/**
 ****************************************************************************************
 *
 * @file app_electric.c
 *
 * @brief electric Application Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2019.05.16
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "app_electric.h"              // Battery Application Module Definitions
#include "appm.h"                    // Application Definitions
#include "appc.h" 
#include "appm_task.h"             // application task definitions
#include "electric.h"
#include "electric_task.h"           // health thermometer functions
#include "co_bt.h"
#include "prf_types.h"             // Profile common types definition
#include "arch.h"                    // Platform Definitions
#include "prf.h"
#include "ke_timer.h"
#include "uart.h"
#include "flash.h"
#include "Co_utils.h"
#include "gpio.h"
#include "electric_task.h"
/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
ble_sta_t  BLE_WORK_STA[5]={BLE_DISCONN,BLE_DISCONN,BLE_DISCONN,BLE_DISCONN,BLE_DISCONN};/// = BLE_CONN;
/// Application Module Environment Structure
struct app_electric_env_tag app_electric_env;

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */


void app_electric_init(void)
{
    // Reset the environment
    memset(&app_electric_env, 0, sizeof(struct app_electric_env_tag));
}

void app_add_electric(void)
{

   struct electric_db_cfg *db_cfg;
		
   struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                  TASK_GAPM, TASK_APPM,
                                                  gapm_profile_task_add_cmd, sizeof(struct electric_db_cfg));
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl =   PERM(SVC_UUID_LEN, UUID_128);
    req->prf_task_id = TASK_ID_ELECTRIC;
    req->app_task = TASK_APPM;
    req->start_hdl = 0; 
	 
    // Set parameters
    db_cfg = (struct electric_db_cfg* ) req->param;
	 
    // Sending of notifications is supported
    db_cfg->features = ELECTRIC_TX_NTF_SUP;
    // Send the message
    ke_msg_send(req);
}


uint8_t app_electric_tx_send(uint8_t conidx,uint8_t len,uint8_t* buf)
{
    uint8_t ret = APPM_ERROR_NO_ERROR;
    if((ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_LINK_CONNECTED))
    {
    
        if(app_electric_env.ntf_cfg[conidx] != PRF_CLI_STOP_NTFIND)
        {        
                // Allocate the message
            struct electric_tx_upd_req * req = KE_MSG_ALLOC(ELECTRIC_TX_UPD_REQ,
                                                            prf_get_task_from_id(TASK_ID_ELECTRIC),
                                                            TASK_APPM,
                                                            electric_tx_upd_req);
            // Fill in the parameter structure	

            req->conidx = conidx;
            req->length = len;
            memcpy(req->tx_value, buf, len);
            // Send the message
            ke_msg_send(req);
        }else
        {
            ret = APPM_ERROR_NTFIND_DISABLE;
        }
        
    }else
    {
        ret = APPM_ERROR_STATE;
    }
    return ret;
}

/**
 ****************************************************************************************
 * @brief
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int app_electric_msg_dflt_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    UART_PRINTF("%s msgid:0x%x,dest_id:0x%x,src_id:0x%x\r\n",__func__,msgid,dest_id,src_id);
    // Drop the message
    return (KE_MSG_CONSUMED);
}

static int app_electric_tx_ntf_cfg_ind_handler(ke_msg_id_t const msgid,
                                               struct electric_tx_ntf_cfg_ind const *param,
                                               ke_task_id_t const dest_id,
                                               ke_task_id_t const src_id)
{
	UART_PRINTF("%s param->ntf_cfg = %x %x\r\n",__func__,param->ntf_cfg,param->conidx);
    app_electric_env.ntf_cfg[param->conidx] = param->ntf_cfg;
	if(param->ntf_cfg == PRF_CLI_STOP_NTFIND)
	{
		
	}else
	{
		
	}
    
    return (KE_MSG_CONSUMED);
}

static int app_electric_rx_writer_ind_handler(ke_msg_id_t const msgid,
                                     struct electric_rx_writer_ind *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    UART_PRINTF("%s msgid:0x%x,dest_id:0x%x,src_id:0x%x conidx:%d\r\n",__func__,msgid,dest_id,src_id,conidx);    
    UART_PRINTF("ind->conidx = 0x%x\r\n",param->conidx);
    
    // Drop the message
#if UART_PRINTF_EN	
//    if(param->rx_value[0] == 0x01)
//    {
//        gpio_config(PIN_STA_O, OUTPUT, PULL_NONE);
//        gpio_set(PIN_STA_O, 1);
//        UART_PRINTF("on");

//    }

//    if(param->rx_value[0] == 0x00)
//    {
//        gpio_config(PIN_STA_O, OUTPUT, PULL_NONE);
//        gpio_set(PIN_STA_O, 0);
//        UART_PRINTF("off");
//    }
//    if(!strcmp((const char*)param->rx_value,"LED_ON"))
//    {
//        gpio_config(PIN_STA_O, OUTPUT, PULL_NONE);
//        gpio_set(PIN_STA_O, 1);
//        UART_PRINTF("on");
//    }
//    else if(!strcmp((const char*)param->rx_value,"LED_OFF"))
//    {
//        gpio_config(PIN_STA_O, OUTPUT, PULL_NONE);
//        gpio_set(PIN_STA_O, 0);
//        UART_PRINTF("off");
//    }
//    UART_PRINTF("param->value = 0x ");
//    for(uint8_t i = 0;i < param->length;i++)
//    {
//        UART_PRINTF("%02x ",param->rx_value[i]);
//    }

//    UART_PRINTF("\r\n");
#endif

		{
            USER_PRINTF("+DATA=%d,%d,",param->conidx,param->length);
		}
		uart_data_send(param->rx_value,param->length);
		
		{			
            USER_PRINTF("\r\n");
        }
		
#if UART_PRINTF_EN		
		//app_electric_tx_send(param->conidx,param->length,param->rx_value);
#endif		
    return (KE_MSG_CONSUMED);
}
//uint8_t ttbuf[240];
static int app_electric_tx_upd_rsp_handler(ke_msg_id_t const msgid,
                                      struct electric_tx_upd_rsp const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
   // UART_PRINTF("%s msgid:0x%x,dest_id:0x%x,src_id:0x%x\r\n",__func__,msgid,dest_id,src_id);
	if(param->status == GAP_ERR_NO_ERROR)
	{
       
        if(app_electric_env.ntf_cfg[param->conidx] != 0)
        {
            //app_electric_tx_send(ttbuf,20,param->conidx);
            //ttbuf[0]++;
            //ttbuf[19]--;
        }

	}
    return (KE_MSG_CONSUMED);
}

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct ke_msg_handler app_electric_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KE_MSG_DEFAULT_HANDLER,        (ke_msg_func_t)app_electric_msg_dflt_handler},
    {ELECTRIC_RX_WRITER_REQ_IND,    (ke_msg_func_t)app_electric_rx_writer_ind_handler},
    {ELECTRIC_TX_NTF_CFG_IND  ,     (ke_msg_func_t)app_electric_tx_ntf_cfg_ind_handler},
    {ELECTRIC_TX_UPD_RSP  ,         (ke_msg_func_t)app_electric_tx_upd_rsp_handler},
};

const struct ke_state_handler app_electric_table_handler =
    {&app_electric_msg_handler_list[0], (sizeof(app_electric_msg_handler_list)/sizeof(struct ke_msg_handler))};


