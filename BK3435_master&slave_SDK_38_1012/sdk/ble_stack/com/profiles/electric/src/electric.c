/**
 ****************************************************************************************
 *
 * @file electir.c
 *
 * @brief electir Server Implementation.
 *
 * Copyright (C) beken 2019-2022
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"

#if 1//(BLE_ELECTRIC_SERVER)
#include "attm.h"
#include "electric.h"
#include "electric_task.h"
#include "prf_utils.h"
#include "prf.h"
#include "ke_mem.h"
#include "uart.h"



/*
 * FFF0 ATTRIBUTES DEFINITION
 ****************************************************************************************
 */

uint8_t electric_serv_uuid[16] = ELECTRIC_SERV_UUID_128;

//  LO_UINT16( uuid ), HI_UINT16( uuid )
/// Full electric Database Description - Used to add attributes into the database
const struct attm_desc_128 electric_att_db[ELECTRIC_IDX_NB] =
{
    // Service Declaration
    [ELECTRIC_IDX_SVC]      =   {{LO_UINT16( ATT_DECL_PRIMARY_SERVICE ), HI_UINT16( ATT_DECL_PRIMARY_SERVICE )}, PERM(RD, ENABLE), 0, 0},

    // RX Characteristic Declaration
    [ELECTRIC_IDX_RX_CHAR]  =   {{LO_UINT16( ATT_DECL_CHARACTERISTIC ), HI_UINT16( ATT_DECL_CHARACTERISTIC )}, PERM(RD, ENABLE), 0, 0},    
    // RX Characteristic Value
    [ELECTRIC_IDX_RX_VAL]   =   {ELECTRIC_CHAR_RX_UUID_128,PERM(WRITE_COMMAND, ENABLE) , PERM(UUID_LEN, UUID_128)|PERM(RI, ENABLE),ELECTRIC_RX_DATA_LEN *sizeof(uint8_t) },// 

    
	// TX Characteristic Declaration   
    [ELECTRIC_IDX_TX_CHAR]  =   {{LO_UINT16( ATT_DECL_CHARACTERISTIC ), HI_UINT16( ATT_DECL_CHARACTERISTIC )}, PERM(RD, ENABLE), 0, 0},
    //  TX Characteristic Value
    [ELECTRIC_IDX_TX_VAL]   =   {ELECTRIC_CHAR_TX_UUID_128,PERM(RD, ENABLE), PERM(UUID_LEN, UUID_128)|PERM(RI, ENABLE), ELECTRIC_TX_DATA_LEN * sizeof(uint8_t)},
	//  TX Characteristic - Client Characteristic Configuration Descriptor
    [ELECTRIC_IDX_TX_NTF_CFG] = {{LO_UINT16( ATT_DESC_CLIENT_CHAR_CFG ), HI_UINT16( ATT_DESC_CLIENT_CHAR_CFG )}, PERM(RD, ENABLE)|PERM(NTF, ENABLE)| PERM(WRITE_REQ, ENABLE), 0, 0},

};/// Macro used to retrieve permission value from access and rights on attribute.



static uint8_t electric_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct electric_db_cfg* params)
{
    uint16_t shdl;
    struct electric_env_tag* electric_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    
    //-------------------- allocate memory required for the profile  ---------------------
    electric_env = (struct electric_env_tag* ) ke_malloc(sizeof(struct electric_env_tag), KE_MEM_ATT_DB);
    memset(electric_env, 0 , sizeof(struct electric_env_tag));

    // Service content flag
    uint8_t cfg_flag = ELECTRIC_CFG_FLAG_MANDATORY_MASK;

    // Save database configuration
    electric_env->features |= (params->features) ;
   
    shdl = *start_hdl;

    //Create in the DB
    //------------------ create the attribute database for the profile -------------------
   
    status = attm_svc_create_db_128(&(shdl), electric_serv_uuid, (uint8_t *)&cfg_flag, ELECTRIC_IDX_NB,
                               NULL, env->task,&electric_att_db[0],(sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS | PERM_MASK_SVC_MI | PERM_MASK_SVC_UUID_LEN)));

    //Set optional permissions
    if (status == GAP_ERR_NO_ERROR)
    {
        //Set optional permissions
        if(params->features == ELECTRIC_TX_NTF_SUP)
        {
            // Battery Level characteristic value permissions
            uint16_t perm = PERM(RD, ENABLE) | PERM(NTF, ENABLE);

            attm_att_set_permission(shdl + ELECTRIC_IDX_TX_VAL, perm, 0);
        }
    }

    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {

        // allocate BASS required environment variable
        env->env = (prf_env_t*) electric_env;
        *start_hdl = shdl;
        electric_env->start_hdl = *start_hdl;
        electric_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        electric_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_ELECTRIC;
        env->desc.idx_max           = ELECTRIC_IDX_MAX;
        env->desc.state             = electric_env->state;
        env->desc.default_handler   = &electric_default_handler;

        // service is ready, go into an Idle state
        ke_state_set(env->task, ELECTRIC_IDLE);
    }
    else if(electric_env != NULL)
    {
        ke_free(electric_env);
    }
     
    return (status);
}


static void electric_destroy(struct prf_task_env* env)
{
    struct electric_env_tag* electric_env = (struct electric_env_tag*) env->env;

    // clear on-going operation
    if(electric_env->operation != NULL)
    {
        ke_free(electric_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    ke_free(electric_env);
}

static void electric_create(struct prf_task_env* env, uint8_t conidx)
{
    struct electric_env_tag* electric_env = (struct electric_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    electric_env->ntf_cfg[conidx] = 0;
}


static void electric_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct electric_env_tag* electric_env = (struct electric_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
    electric_env->ntf_cfg[conidx] = 0;
}


void electric_tx_notify(struct electric_env_tag* env, struct electric_tx_upd_req const *param)
{
    // Allocate the GATT notification message
    struct gattc_send_evt_cmd *tx_value = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KE_BUILD_ID(TASK_GATTC, param->conidx), prf_src_task_get(&(env->prf_env),0),
            gattc_send_evt_cmd, sizeof(uint8_t)* (param->length));

    // Fill in the parameter structure
    tx_value->operation = GATTC_NOTIFY;
    tx_value->handle = electric_get_att_handle(ELECTRIC_IDX_TX_VAL);
    // pack measured value in database
    tx_value->length = param->length;
	memcpy(&tx_value->value[0],&param->tx_value[0],param->length);
    // send notification to peer device
    ke_msg_send(tx_value);
}



/// Task interface required by profile manager
const struct prf_task_cbs electric_itf =
{
        (prf_init_fnct) electric_init,
        electric_destroy,
        electric_create,
        electric_cleanup,
};


const struct prf_task_cbs* electric_prf_itf_get(void)
{
   return &electric_itf;
}


uint16_t electric_get_att_handle( uint8_t att_idx)
{
		
    struct electric_env_tag *electric_env = PRF_ENV_GET(ELECTRIC, electric);
    uint16_t handle = ATT_INVALID_HDL;
   
    handle = electric_env->start_hdl;

    // increment index according to expected index
    if(att_idx < ELECTRIC_IDX_TX_NTF_CFG)
    {
        handle += att_idx;
    }
    //  notification
    else if((att_idx == ELECTRIC_IDX_TX_NTF_CFG) && (((electric_env->features ) & 0x01) == ELECTRIC_TX_NTF_SUP))
    {
        handle += ELECTRIC_IDX_TX_NTF_CFG;			
    }		      
    else
    {
        handle = ATT_INVALID_HDL;
    }
    

    return handle;
}

uint8_t electric_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct electric_env_tag* electric_env = PRF_ENV_GET(ELECTRIC, electric);
    uint16_t hdl_cursor = electric_env->start_hdl;
    uint8_t status = PRF_APP_ERROR;

    // Browse list of services
    // handle must be greater than current index 
    // check if it's a mandatory index
    if(handle <= (hdl_cursor + ELECTRIC_IDX_TX_VAL))
    {
        *att_idx = handle -hdl_cursor;
        status = GAP_ERR_NO_ERROR;
        
    }
    hdl_cursor += ELECTRIC_IDX_TX_VAL;

    // check if it's a notify index
    if(((electric_env->features ) & 0x01) == ELECTRIC_TX_NTF_SUP)
    {
        hdl_cursor++;
        if(handle == hdl_cursor)
        {
            *att_idx = ELECTRIC_IDX_TX_NTF_CFG;
            status = GAP_ERR_NO_ERROR;
        }
    }
    hdl_cursor++;
    
    return (status);
}


#endif // (BLE_ELECTRIC_SERVER)


 
