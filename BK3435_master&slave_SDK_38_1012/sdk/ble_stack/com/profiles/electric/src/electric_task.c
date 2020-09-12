/**
 ****************************************************************************************
 *
 * @file   electric_task.c
 *
 * @brief  Server Role Task Implementation.
 *
 * Copyright (C) Beken 2019-2022
 *
 *
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if  1//(BLE_ELECTRIC_SERVER)

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "atts.h"
#include "co_utils.h"
#include "electric.h"
#include "electric_task.h"
#include "uart.h"
#include "prf_utils.h"


uint8_t send_data_enable_flag[BLE_CONNECTION_MAX] = {1, 1, 1, 1, 1};

static int electric_tx_upd_req_handler(ke_msg_id_t const msgid,
                                            struct electric_tx_upd_req const *param,
                                            ke_task_id_t const dest_id,
                                            ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_SAVED;
    uint8_t state = ke_state_get(dest_id);
	
    // check state of the task
    if(state == ELECTRIC_IDLE)
    {
        struct electric_env_tag *electric_env = PRF_ENV_GET(ELECTRIC, electric);

        if(electric_env->ntf_cfg[param->conidx]!= 0)
        {
            // put task in a busy state
            ke_state_set(dest_id, ELECTRIC_BUSY);						
            electric_tx_notify(electric_env, param);             
            msg_status = KE_MSG_CONSUMED;	
        }        
    }

    return (msg_status);
  }


  
static int gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gattc_att_info_req_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{

    struct gattc_att_info_cfm * cfm;
    uint8_t  att_idx = 0;
    // retrieve handle information
    uint8_t status = electric_get_att_idx(param->handle, &att_idx);

    //Send write response
    cfm = KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    cfm->handle = param->handle;
    UART_PRINTF("[elect] gattc_att_info_req_ind_handler:att_idx:0x%x\r\n",att_idx);
    if(status == GAP_ERR_NO_ERROR)
    {
        // check if it's a client configuration char
        if(att_idx == ELECTRIC_IDX_TX_NTF_CFG)
        {
            // CCC attribute length = 2
            cfm->length = 2;
        }
        // not expected request
        else
        {
            cfm->length = 0;
            status = GAP_ERR_NO_ERROR;//ATT_ERR_WRITE_NOT_PERMITTED;
        }
    }

    cfm->status = status;
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}



static int gattc_write_req_ind_handler(ke_msg_id_t const msgid, struct gattc_write_req_ind const *param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_write_cfm * cfm;
    uint8_t att_idx = 0;
    uint8_t conidx = KE_IDX_GET(src_id);

    UART_PRINTF("[electric] %s msgid:0x%x,dest_id:0x%x,src_id:0x%x,conidx:%x\r\n",__func__,msgid,dest_id,src_id,conidx);
    // retrieve handle information
    uint8_t status = electric_get_att_idx(param->handle, &att_idx);
		
    UART_PRINTF("param->handle:%d,att_idx:%d,status:%d\r\n",param->handle,att_idx,status);
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct electric_env_tag *electric_env = PRF_ENV_GET(ELECTRIC, electric);
		
        // Extract value before check
        uint16_t ntf_cfg = co_read16p(&param->value[0]);

        // Only update configuration if value for stop or notification enable
        if ((att_idx == ELECTRIC_IDX_TX_NTF_CFG) // ELECTRIC_IDX_TX_NTF_CFG == 5
                && ((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg == PRF_CLI_START_NTF)))
        {

            // Conserve information in environment
            if (ntf_cfg == PRF_CLI_START_NTF)
            {
                // Ntf cfg bit set to 1
                electric_env->ntf_cfg[conidx] |= (ELECTRIC_TX_NTF_SUP );
            }
            else
            {
                // Ntf cfg bit set to 0
                electric_env->ntf_cfg[conidx] &= ~(ELECTRIC_TX_NTF_SUP );
            }

            // Inform APP of configuration change
            struct electric_tx_ntf_cfg_ind * ind = KE_MSG_ALLOC(ELECTRIC_TX_NTF_CFG_IND,
                    prf_dst_task_get(&(electric_env->prf_env), conidx), dest_id,
                    electric_tx_ntf_cfg_ind);
            ind->conidx = conidx;
            ind->ntf_cfg = electric_env->ntf_cfg[conidx];
						
            ke_msg_send(ind);			
        }
		else if (att_idx == ELECTRIC_IDX_RX_VAL)
		{
			// Allocate the alert value change indication
			struct electric_rx_writer_ind *ind = KE_MSG_ALLOC(ELECTRIC_RX_WRITER_REQ_IND,
			        prf_dst_task_get(&(electric_env->prf_env), conidx),
			        dest_id, electric_rx_writer_ind);
			
			// Fill in the parameter structure	
			memcpy(ind->rx_value,&param->value[0],param->length);
			ind->conidx = conidx;
			ind->length = param->length;
            
            UART_PRINTF("dest:0x%x,ind->length:%d\r\n",prf_dst_task_get(&(electric_env->prf_env), conidx),ind->length);
			
			// Send the message
			ke_msg_send(ind);
		}
        else
        {
            status = PRF_APP_ERROR;
        }

    }

    //Send write response
    cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
    cfm->handle = param->handle;
    cfm->status = status;
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}   



static int gattc_read_req_ind_handler(ke_msg_id_t const msgid, struct gattc_read_req_ind const *param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_read_cfm * cfm;
    uint8_t  att_idx = 0;
    uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = electric_get_att_idx(param->handle, &att_idx);
    uint16_t length = 0;
    
    struct electric_env_tag *electric_env = PRF_ENV_GET(ELECTRIC, electric);
    uint16_t ntf_cfg = electric_env->ntf_cfg[conidx];
    
    UART_PRINTF("%s,dest:0x%x,src_id:0x%x,conidx:%d,handle:%d\r\n",__func__,dest_id,src_id,conidx,param->handle);
    
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status != GAP_ERR_NO_ERROR)
    {
        // read notification information             
         status = PRF_APP_ERROR; 
        
    }
    
    if (att_idx == ELECTRIC_IDX_TX_NTF_CFG)
    {
       length =2;
       memcpy(cfm->value,&ntf_cfg,length);
    }
    //Send write response
    cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
    cfm->handle = param->handle;
    cfm->status = status;
    cfm->length = length;
    
   
    

    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}   


static int gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    if(param->operation == GATTC_NOTIFY)
    {	
        
        // go back in to idle mode
        ke_state_set(dest_id, ke_state_get(dest_id) & ~ELECTRIC_BUSY);
        
      	uint8_t conidx = KE_IDX_GET(src_id);
      	struct electric_env_tag *electric_env = PRF_ENV_GET(ELECTRIC, electric);

	    struct electric_tx_upd_rsp *rsp = KE_MSG_ALLOC(ELECTRIC_TX_UPD_RSP,
                                                         prf_dst_task_get(&(electric_env->prf_env), conidx),
                                                         dest_id, electric_tx_upd_rsp);
            
        rsp->conidx = conidx;
        rsp->status = param->status;			
        ke_msg_send(rsp);
        
		send_data_enable_flag[conidx] = 1;
    }	
	
    return (KE_MSG_CONSUMED);
}

/// Default State handlers definition
const struct ke_msg_handler electric_default_state[] =
{
    {ELECTRIC_TX_UPD_REQ,           (ke_msg_func_t) electric_tx_upd_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (ke_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_WRITE_REQ_IND,           (ke_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (ke_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t) gattc_cmp_evt_handler},
};

/// Specifies the message handlers that are common to all states.
const struct ke_state_handler electric_default_handler = KE_STATE_HANDLER(electric_default_state);

#endif /* #if (BLE_ELECTRIC_SERVER) */


