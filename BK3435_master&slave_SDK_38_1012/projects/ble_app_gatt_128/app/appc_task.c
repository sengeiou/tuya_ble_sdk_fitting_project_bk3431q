#include "rwip_config.h"          // SW configuration
#include "gapc_task.h"            // GAP Controller Task API
#include "gapm_task.h"          // GAP Manager Task API
#include "gattc_task.h"
#include "ke_timer.h"
#include "app_sec.h"
#include "appc.h"
#include "appc_task.h"
#include "appm.h"
#include "appm_task.h"
#include "master_app.h"
#include "sdp_service.h"
#include "uart.h"




/*********************************************************
FN: Handles reception of all messages sent from the lower layers to the application
PM: msgid     Id of the message received.
    param     Pointer to the parameters of the message.
    dest_id   ID of the receiving task instance.
    src_id    ID of the sending task instance.
RT: If the message was consumed or not.
*/
static int appc_msg_handler(ke_msg_id_t const msgid,
                            void *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    SUBLE_PRINTF("%s", __func__);
    SUBLE_PRINTF("conidx:%x",conidx);
    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: Handles GAP manager command complete events.
*/
static int gapm_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gapm_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
//    SUBLE_PRINTF("%s", __func__);
//    SUBLE_PRINTF("conidx:%x",conidx);
//	SUBLE_PRINTF(" operation = 0x%x, status = 0x%x", param->operation, param->status);
    
    switch(param->operation)
    {
        case (GAPM_PROFILE_TASK_ADD): {
//            SUBLE_PRINTF("GAPM_PROFILE_TASK_ADD");
        } break;
        
        default: {
        } break;
    }

    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: Handles GAP controller command complete events.
*/
static int gapc_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gapc_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
//	SUBLE_PRINTF("gapc_cmp_evt_handler operation-%x,status-%x", param->operation, param->status);
    
	switch(param->operation)
	{
    	case (GAPC_UPDATE_PARAMS):  //0x09
    	{
			if (param->status != GAP_ERR_NO_ERROR)
        	{
            	SUBLE_PRINTF("gapc update params fail !");
			}
			else
			{
				SUBLE_PRINTF("gapc update params ok !");
			}
			
    	} break;

		case (GAPC_SECURITY_REQ): //0x0c
		{
			if (param->status != GAP_ERR_NO_ERROR)
	        {
	            SUBLE_PRINTF("gapc security req fail !");
	        }
	        else
	        {
	            SUBLE_PRINTF("gapc security req ok !");
	        }
		}break;
		case (GAPC_BOND): // 0xa
    	{
	        if (param->status != GAP_ERR_NO_ERROR)
	        {
	            SUBLE_PRINTF("gapc bond fail !");
	        }
	        else
	        {
	            SUBLE_PRINTF("gapc bond ok !");
	        }
    	}break;
		
		case (GAPC_ENCRYPT): // 0xb
		{
			if (param->status != GAP_ERR_NO_ERROR)
			{
				SUBLE_PRINTF("gapc encrypt start fail !");
			}
			else
			{
				SUBLE_PRINTF("gapc encrypt start ok !");
			}
		}
		break;
        
        case (GAPC_GET_PEER_VERSION): // 0x3
		{
			if (param->status != GAP_ERR_NO_ERROR)
			{
				SUBLE_PRINTF("GAPC_GET_PEER_VERSION fail ! status:0x%x",param->status);
			}
			else
			{
//				SUBLE_PRINTF("GAPC_GET_PEER_VERSION ok !");
			}
            appc_get_peer_dev_info(conidx,GAPC_GET_PEER_FEATURES);
		}
		break;
        
         case (GAPC_GET_PEER_FEATURES): // 0x4
		{
			if (param->status != GAP_ERR_NO_ERROR)
			{
				SUBLE_PRINTF("GAPC_GET_PEER_FEATURES fail ! status:0x%x",param->status);
			}
			else
			{
				SUBLE_PRINTF("GAPC_GET_PEER_FEATURES ok !");
			}  
		}
		break;
		

    	default:
    	  break;
    }

    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: 
*/
static int gattc_sdp_svc_ind_handler(ke_msg_id_t const msgid,
                                     struct gattc_sdp_svc_ind const *ind,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
//    SUBLE_PRINTF("%s", __func__);
    
    sdp_extract_svc_info(conidx,ind);
    
    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: 完成事件
*/
static int gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
//    SUBLE_PRINTF("%s, operation-%d, status-%d", __func__, param->operation, param->status);
  
    uint8_t conidx = KE_IDX_GET(src_id);
    
    if(param->operation == GATTC_MTU_EXCH) {
    }
    
    if(param->operation == GATTC_WRITE) {
    }
    
    if(param->operation == GATTC_WRITE_NO_RESPONSE) {
    }
    
    if(param->operation == GATTC_SDP_DISC_SVC) {
        if(param->status == ATT_ERR_NO_ERROR) {
            SUBLE_PRINTF("all service discovered !!!");
            
            ke_state_set(TASK_APPM, APPM_CONNECTED);
            ke_state_set(KE_BUILD_ID(TASK_APPC,conidx),APPC_SERVICE_CONNECTED);
            appm_scan_adv_con_schedule();
            
//            appc_update_param(conidx, BLE_UAPDATA_MIN_INTVALUE, BLE_UAPDATA_MAX_INTVALUE, 0, 1000);
        }
    }
    else if(param->operation == GATTC_SDP_DISC_SVC_ALL) {
        if(param->status == ATT_ERR_NO_ERROR) {
//            SUBLE_PRINTF("all service discovered !!!");
            
            ke_state_set(TASK_APPM, APPM_CONNECTED);
            ke_state_set(KE_BUILD_ID(TASK_APPC,conidx),APPC_SERVICE_CONNECTED);
            appm_scan_adv_con_schedule();

//            appc_update_param(conidx, BLE_UAPDATA_MIN_INTVALUE, BLE_UAPDATA_MAX_INTVALUE, 0, 1000);
//            SUBLE_PRINTF("all service discovered !!!");
        }
    }
    
    if(param->operation == GATTC_REGISTER) {
        appc_write_service_ntf_cfg_req(conidx,appc_env[conidx]->svc_notif_handle, 0x01);//if slave devce no response, Cause problems
    }
    
    if(param->operation == GATTC_WRITE) {
        suble_svc_c_discovery_complete();
    }
    
    if(param->operation == GATTC_UNREGISTER) {
    }
    
    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: Handles profile add indication from the GAP.
*/
static int gapm_profile_added_ind_handler(ke_msg_id_t const msgid,
                                          struct gapm_profile_added_ind *param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);

//    SUBLE_PRINTF("%s", __func__);
    return KE_MSG_CONSUMED;
}

/*********************************************************
FN: Handles disconnection complete event from the GAP.
*/
static int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_disconnect_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    SUBLE_PRINTF("%s", __func__);
    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: 
*/
static int gapc_peer_version_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_peer_version_ind const *version_ind,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
//    SUBLE_PRINTF("%s", __func__);
    uint8_t conidx = KE_IDX_GET(src_id);

//    // LMP version
//    SUBLE_PRINTF("lmp_vers:%x,%d",version_ind->lmp_vers,conidx);
//    // LMP subversion
//    SUBLE_PRINTF("lmp_subvers:%x",version_ind->lmp_subvers);
//    // Manufacturer name
//    SUBLE_PRINTF("compid:%x",version_ind->compid);  
    
    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: 
*/
static int gapc_peer_features_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_peer_features_ind const *features_ind,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
//    SUBLE_PRINTF("%s", __func__);
    uint8_t conidx = KE_IDX_GET(src_id);

    //features
//    SUBLE_HEXDUMP("features", (void*)features_ind->features, LE_FEATS_LEN);

    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: 
*/
#include "cli.h"
extern ble_dev_info slave_device;
//extern void appc_le_data_length_update_req(uint8_t conidx,uint16_t max_tx_octets,uint16_t max_tx_time);
static int appc_check_device_link_timer_handler(ke_msg_id_t const msgid, void const *param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
//    SUBLE_PRINTF("%s", __func__);
    uint8_t conidx = KE_IDX_GET(dest_id);
    
    if(ke_state_get(KE_BUILD_ID(TASK_APPC, conidx)) == APPC_LINK_CONNECTED)
    {
        //底层长度扩展
//        appc_le_data_length_update_req(appm_env.conidx, BLE_MAX_OCTETS, BLE_MAX_TIME_4_2);
        //更新mtu
//        appc_gatt_mtu_change(conidx);
        
        if(appc_env[conidx]->role == ROLE_MASTER) {
            appc_get_peer_dev_info(conidx, GAPC_GET_PEER_VERSION);
//            sdp_discover_service(conidx,slave_device.serv_uuid, ATT_UUID_128_LEN);
            sdp_discover_all_service(conidx);
            ke_state_set(TASK_APPM, APPM_SDP_DISCOVERING);
            ke_state_set(KE_BUILD_ID(TASK_APPC, conidx), APPC_SDP_DISCOVERING);
        }
    }
    
    return (KE_MSG_CONSUMED);
}




//消息管理列表：消息 - 处理函数
const struct ke_msg_handler appc_default_state[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KE_MSG_DEFAULT_HANDLER,    	(ke_msg_func_t)appc_msg_handler},
    
    {GAPM_CMP_EVT,             		(ke_msg_func_t)gapm_cmp_evt_handler},
    {GAPC_CMP_EVT,             		(ke_msg_func_t)gapc_cmp_evt_handler},
    
    {GAPC_DISCONNECT_IND,       	(ke_msg_func_t)gapc_disconnect_ind_handler},
    {GAPC_PEER_VERSION_IND,         (ke_msg_func_t)gapc_peer_version_ind_handler},
    {GAPC_PEER_FEATURES_IND,        (ke_msg_func_t)gapc_peer_features_ind_handler},
    {GAPM_PROFILE_ADDED_IND ,       (ke_msg_func_t)gapm_profile_added_ind_handler},
    
    {GATTC_SDP_SVC_IND,             (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t)gattc_cmp_evt_handler},
    
    {APPC_CHECK_LINK_TIMER,         (ke_msg_func_t)appc_check_device_link_timer_handler},
   
};
//指定消息管理列表
const struct ke_state_handler appc_default_handler = KE_STATE_HANDLER(appc_default_state);

/* Defines the place holder for the states of all the task instances. */
ke_state_t appc_state[APPC_IDX_MAX];



