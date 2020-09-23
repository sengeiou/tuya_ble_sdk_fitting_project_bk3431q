#include "rwip_config.h"          // SW configuration
#include "rwapp_config.h"
#include <string.h>
#include "master_app.h"
#include "appm.h"                      // Application Manager Definition
#include "appm_task.h"              // Application Manager Task API
#include "appc.h"
#include "appc_task.h"
#include "gapc_task.h"            // GAP Controller Task API
#include "gapm_task.h"          // GAP Manager Task API
#include "gattc_task.h"
#include "arch.h"                    // Platform Definitions
#include "ke_timer.h"             // Kernel timer
#include "app_fff0.h"              // fff0 Module Definition
#include "fff0s_task.h"
#include "app_sec.h"
#include "app_electric.h"
#include "electric_task.h"
#include "gpio.h"
#include "audio.h"
#include "uart.h"
#include "BK3435_reg.h"
#include "icu.h"
#include "reg_ble_em_cs.h"
#include "lld.h"
#include "wdt.h"
#include "user_config.h"
#include "cli.h"
#include "sdp_service.h"
#include "electric_task.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */
static uint8_t con_dev_str_name0[] = "BK3435-GATT0";

/*********************************************************************
 * VARIABLE
 */
uint8_t discover_flag = 0;

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/
static uint8_t appm_get_handler(const struct ke_state_handler *handler_list,
                                ke_msg_id_t msgid, void *param, ke_task_id_t src_id)
{
    uint8_t counter;

    // Get the message handler function by parsing the message table
    for (counter = handler_list->msg_cnt; 0 < counter; counter--)
    {
        struct ke_msg_handler handler = (*(handler_list->msg_table + counter - 1));

        if ((handler.id == msgid) ||(handler.id == KE_MSG_DEFAULT_HANDLER)) {
            // If handler is NULL, message should not have been received in this state
            ASSERT_ERR(handler.func);
            return (uint8_t)(handler.func(msgid, param, TASK_APPM, src_id));
        }
    }
    // If we are here no handler has been found, drop the message
    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN:
*/
SUBLE_TIMERX_HANDLER(0);
SUBLE_TIMERX_HANDLER(1);
SUBLE_TIMERX_HANDLER(2);
SUBLE_TIMERX_HANDLER(3);
SUBLE_TIMERX_HANDLER(4);
SUBLE_TIMERX_HANDLER(5);
SUBLE_TIMERX_HANDLER(6);
SUBLE_TIMERX_HANDLER(7);
SUBLE_TIMERX_HANDLER(8);
SUBLE_TIMERX_HANDLER(9);
SUBLE_TIMERX_HANDLER(10);
SUBLE_TIMERX_HANDLER(11);
SUBLE_TIMERX_HANDLER(12);
SUBLE_TIMERX_HANDLER(13);
SUBLE_TIMERX_HANDLER(14);
SUBLE_TIMERX_HANDLER(15);
SUBLE_TIMERX_HANDLER(16);
SUBLE_TIMERX_HANDLER(17);
SUBLE_TIMERX_HANDLER(18);
SUBLE_TIMERX_HANDLER(19);
SUBLE_TIMERX_HANDLER(100);
SUBLE_TIMERX_HANDLER(101);
SUBLE_TIMERX_HANDLER(102);
SUBLE_TIMERX_HANDLER(103);
SUBLE_TIMERX_HANDLER(104);
SUBLE_TIMERX_HANDLER(105);
SUBLE_TIMERX_HANDLER(106);
SUBLE_TIMERX_HANDLER(107);
SUBLE_TIMERX_HANDLER(108);
SUBLE_TIMERX_HANDLER(109);
SUBLE_TIMERX_HANDLER(200);

/*********************************************************
FN: 
*/
static int app_adv_timeout_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
#if (BLE_APP_HID)
#else
    // Stop advertising
#if (BLE_PERIPHERAL)
    appm_stop_advertising();
#endif
#endif
    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: Handles ready indication from the GAP. - Reset the stack
PM: msgid     Id of the message received.
    param     Pointer to the parameters of the message.
    dest_id   ID of the receiving task instance (TASK_GAP).
    src_id    ID of the sending task instance.
RT: If the message was consumed or not.
*/
static int gapm_device_ready_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    // Application has not been initialized
    ASSERT_ERR(ke_state_get(dest_id) == APPM_INIT);

    // Reset the stack
    struct gapm_reset_cmd* cmd = KE_MSG_ALLOC(GAPM_RESET_CMD,
                                 TASK_GAPM, TASK_APPM,
                                 gapm_reset_cmd);


    cmd->operation = GAPM_RESET;

    ke_msg_send(cmd);

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
    switch(param->operation)
    {
        case (GAPM_RESET): {
            if(param->status == GAP_ERR_NO_ERROR)
            {
                // Set Device configuration
                struct gapm_set_dev_config_cmd* cmd = KE_MSG_ALLOC(GAPM_SET_DEV_CONFIG_CMD,
                                                      TASK_GAPM, TASK_APPM,
                                                      gapm_set_dev_config_cmd);
                // Set the operation
                cmd->operation = GAPM_SET_DEV_CONFIG;
                // Set the device role - Peripheral
                cmd->role      = GAP_ROLE_ALL;//GAP_ROLE_ALL;//GAP_ROLE_PERIPHERAL;//GAP_ROLE_ALL;

                cmd->addr_type = GAPM_CFG_ADDR_PUBLIC;///GAPM_CFG_ADDR_PUBLIC;
//                memcpy(cmd->addr.addr, &co_default_bdaddr, 6);

                cmd->sugg_max_tx_octets = BLE_MIN_OCTETS;
                cmd->sugg_max_tx_time   = BLE_MAX_TIME_4_2;

                cmd->max_mtu = BLE_MIN_OCTETS;
                //Do not support secure connections
                cmd->pairing_mode = GAPM_PAIRING_LEGACY;

                // load IRK
                memcpy(cmd->irk.key, appm_env.loc_irk, KEY_LEN);

                appm_env.next_svc = 0;

                // Send message
                ke_msg_send(cmd);
            }
        } break;

        case (GAPM_CANCEL): {
            if(param->status == GAP_ERR_COMMAND_DISALLOWED) {
                SUBLE_PRINTF("GAP_ERR_COMMAND_DISALLOWED");
//                ke_state_set(dest_id, APPM_IDLE);
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_scan_adv_con_schedule();
            }
        } break;
        
        case (GAPM_PROFILE_TASK_ADD): {
            if (ke_state_get(dest_id) == APPM_CREATE_DB)
            {
                //添加后续的服务
                if (!appm_add_svc())
                {
                    ke_state_set(dest_id, APPM_IDLE);
                    
                    suble_init_func(2);
                    
                    appm_scan_adv_con_schedule();
                }
            }
            else if(ke_state_get(dest_id) == APPM_CONNECTED)
            {
                uint8_t find;
                //  find = sdp_enable_all_server_ntf_ind(1);
                SUBLE_PRINTF("------------------find--- = %d",find);
            }
        } break;
        
        case (GAPM_SET_DEV_CONFIG): {
            //添加第一个服务
            ke_state_set(TASK_APPM, APPM_CREATE_DB);
            appm_add_svc();
        } break;

        case (GAPM_ADV_NON_CONN):
        case (GAPM_ADV_UNDIRECT):
        case (GAPM_ADV_DIRECT):
        case (GAPM_UPDATE_ADVERTISE_DATA):
        case (GAPM_ADV_DIRECT_LDC): {
            
            if(param->status != GAP_ERR_COMMAND_DISALLOWED) {
                SUBLE_PRINTF("adv stoped");
            }
            
            if (param->status == GAP_ERR_CANCELED)
            {
                if(ke_state_get(TASK_APPM) == APPM_WAIT_ADVERTISTING_END) {
                    ke_timer_clear(APPM_STOP_ADV_TIMER, TASK_APPM);
                    
                    ke_state_set(TASK_APPM, APPM_ADVERTISTING_END);
                    SUBLE_PRINTF("APPM_WAIT_ADVERTISTING_END");
                    appm_scan_adv_con_schedule();
                    
                    if(g_adv_restart_glag)
                    {
                        SUBLE_PRINTF("restart adv");
                        g_adv_restart_glag = false;
                        suble_adv_start();
                    }
                } else {
                    SUBLE_PRINTF("state error state:0x%x!!!",ke_state_get(TASK_APPM));
                }
            }
            
            if (param->status == GAP_ERR_NO_ERROR) {
                ke_state_set(TASK_APPM, APPM_ADVERTISTING_END);
                appm_scan_adv_con_schedule();
            }

            if (param->status == GAP_ERR_TIMEOUT) {
                ke_state_set(TASK_APPM, APPM_IDLE);
            }
        } break;
        
        case (GAPM_SCAN_ACTIVE):
        case(GAPM_SCAN_PASSIVE): {
            if (param->status == GAP_ERR_CANCELED)
            {
                SUBLE_PRINTF("scan stoped");
                if (ke_state_get(TASK_APPM) == APPM_WAIT_SCAN_END)
                {
                    ke_state_set(TASK_APPM, APPM_SCAN_END);
                    appm_scan_adv_con_schedule();
                }
            }
        }
        break;
        
        case (GAPM_CONNECTION_DIRECT): {
            if (param->status == GAP_ERR_NO_ERROR) {
                ke_state_set(TASK_APPM, APPM_IDLE);
//                SUBLE_PRINTF("GAPM_CONNECTION_DIRECT");
//                appm_scan_adv_con_schedule();
            }
            else if (param->status == GAP_ERR_CANCELED) {
                appm_env.con_dev_flag = 0;
                ke_state_set(TASK_APPM, APPM_IDLE);
                SUBLE_PRINTF("GAP_ERR_CANCELED");
//                suble_adv_start();
//                appm_scan_adv_con_schedule();
            }
            else if(param->status == LL_ERR_ACL_CON_EXISTS) {
                ke_state_set(TASK_APPM, APPM_IDLE);
            }
//            appm_scan_adv_con_schedule();
        } break;
    
        default: {
        } break;
    }
    return (KE_MSG_CONSUMED);
}

static int gapc_get_dev_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gapc_get_dev_info_req_ind const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    switch(param->req)
    {
        case GAPC_DEV_NAME: {
            struct gapc_get_dev_info_cfm * cfm = KE_MSG_ALLOC_DYN(GAPC_GET_DEV_INFO_CFM,
                                                 src_id, dest_id,
                                                 gapc_get_dev_info_cfm, APP_DEVICE_NAME_MAX_LEN);
            cfm->req = param->req;
            cfm->info.name.length = appm_get_dev_name(cfm->info.name.value);
            // Send message
            ke_msg_send(cfm);
            UART_PRINTF("get_dev_info GAPC_DEV_NAME\r\n");
        } break;

        case GAPC_DEV_APPEARANCE: {
            // Allocate message
            struct gapc_get_dev_info_cfm *cfm = KE_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM,
                                                    src_id, dest_id,
                                                    gapc_get_dev_info_cfm);
            cfm->req = param->req;
            
            // No appearance
            cfm->info.appearance = 0;

            // Send message
            ke_msg_send(cfm);
        } break;

        case GAPC_DEV_SLV_PREF_PARAMS: {
            // Allocate message
            struct gapc_get_dev_info_cfm *cfm = KE_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM,
                    								src_id, dest_id,
                                                    gapc_get_dev_info_cfm);
            cfm->req = param->req;
            // Slave preferred Connection interval Min
            cfm->info.slv_params.con_intv_min = 8;
            // Slave preferred Connection interval Max
            cfm->info.slv_params.con_intv_max = 10;
            // Slave preferred Connection latency
            cfm->info.slv_params.slave_latency = 180;
            // Slave preferred Link supervision timeout
            cfm->info.slv_params.conn_timeout  = 600;  // 6s (600*10ms)

            // Send message
            ke_msg_send(cfm);
        } break;

        default: { /* Do Nothing */
        } break;
    }
    
    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: 每包大小
*/
static int gapc_le_pkt_size_ind_handler (ke_msg_id_t const msgid,
        const struct gapc_le_pkt_size_ind  *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    SUBLE_PRINTF("le_pkt_size: conidx-%d, rx_octets-%d, rx_time-%d, tx_octets-%d, tx_time-%d", \
        conidx, param->max_rx_octets, param->max_rx_time, param->max_tx_octets, param->max_tx_time);
    return KE_MSG_CONSUMED;
}

/*********************************************************
FN: 更新连接参数请求
*/
static int gapc_param_update_req_ind_handler(ke_msg_id_t const msgid,
        struct gapc_param_update_req_ind const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    SUBLE_PRINTF("%s", __func__);
    // Prepare the GAPC_PARAM_UPDATE_CFM message
    struct gapc_param_update_cfm *cfm = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CFM,
                                        src_id, dest_id, gapc_param_update_cfm);
    cfm->ce_len_max = 0xffff;
    cfm->ce_len_min = 0xffff;
    cfm->accept = true;

    // Send message
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: 更新连接参数
*/
static int gapc_param_updated_ind_handler (ke_msg_id_t const msgid,
        const struct gapc_param_updated_ind  *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    SUBLE_PRINTF("conn_param_update: conidx-%d, min-%dms, max-%dms, latency-%d, timeout-%dms", \
        conidx,                                 \
        (uint16_t)(param->con_interval*1.25),   \
        (uint16_t)(param->con_interval*1.25),   \
        (uint16_t)(param->con_latency),         \
        (uint16_t)(param->sup_to*10) );
    return KE_MSG_CONSUMED;
}

/*********************************************************
FN: 更新mtu
*/
static int gattc_mtu_changed_ind_handler(ke_msg_id_t const msgid,
        struct gattc_mtu_changed_ind const *ind,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    SUBLE_PRINTF("mtu_changed: conidx-%d, mtu-%d, seq-%d", \
        conidx, ind->mtu, ind->seq_num);
    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: Handles connection complete event from the GAP. Enable all required profiles
*/
static int gapc_connection_req_ind_handler(ke_msg_id_t const msgid, struct gapc_connection_req_ind const *param,
                                            ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    //检查连接序号是否有效
    appm_env.conidx = KE_IDX_GET(src_id);
    if (appm_env.conidx != GAP_INVALID_CONIDX)
    {
        //Send connection confirmation
        struct gapc_connection_cfm *cfm = KE_MSG_ALLOC(GAPC_CONNECTION_CFM, KE_BUILD_ID(TASK_GAPC, appm_env.conidx), TASK_APPM, gapc_connection_cfm);
        cfm->auth = GAP_AUTH_REQ_NO_MITM_NO_BOND;
        ke_msg_send(cfm);

        appc_create(appm_env.conidx, param);
        
        
        if(param->con_role == ROLE_SLAVE) //从机
        {
            ke_state_set(TASK_APPM, APPM_CONNECTED);
            
            {
                SUBLE_PRINTF("ROLE_SLAVE Connected");
                g_conn_info[0].role = ROLE_SLAVE;
                g_conn_info[0].condix = appm_env.conidx;
                memcpy(&g_conn_info[0].mac, &param->peer_addr, sizeof(struct gap_bdaddr));
                
                suble_gap_conn_handler();
            }
        }
        else //主机
        {
            ke_state_set(TASK_APPM, APPM_LINK_CONNECTED);
            
            //关闭连接标志
            appm_env.con_dev_flag = 0;
            //清除连接超时
            if(ke_timer_active(APPM_CON_TIMEOUT_TIMER, TASK_APPM)) {
                ke_timer_clear(APPM_CON_TIMEOUT_TIMER, TASK_APPM);
            }
            
            {
                SUBLE_PRINTF("ROLE_MASTER Connected");
                g_conn_info[1].role = ROLE_MASTER;
                g_conn_info[1].condix = appm_env.conidx;
                memcpy(&g_conn_info[1].mac, &param->peer_addr, sizeof(struct gap_bdaddr));
                
                suble_gap_master_conn_handler();
            }
            
//            appm_field_recover(); //APPM_FIELD_RECOVER();
        }
        
        //关闭广播
        suble_adv_stop();
    }
    else {
        SUBLE_PRINTF("No connection has been establish");
    }
    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: Handles disconnection complete event from the GAP.
*/
static int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                       struct gapc_disconnect_ind const *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    int idx = KE_IDX_GET(src_id);
    struct appc_env_tag *env = appc_env[idx];

    
    if(env->role == ROLE_SLAVE) //从机
    {
        {
            SUBLE_PRINTF("ROLE_SLAVE Disconnected: 0x%02x", param->reason);
            g_conn_info[0].role = ROLE_END;
            g_conn_info[0].condix = GAP_INVALID_CONIDX;
            memset(&g_conn_info[0].mac, 0, sizeof(struct gap_bdaddr));
            
            suble_gap_disconn_handler();
        }
    }
    else //主机
    {
        if(param->reason == CO_ERROR_CONN_FAILED_TO_BE_EST) //0x3E
        {
            if(appm_env.recon_num > 0) {
                appm_env.recon_num--;
                SUBLE_PRINTF("RECONNECTING");
                set_s_master_scan_is_running();
            } else {
                SUBLE_PRINTF("recon_num %d can't connect peer dev!!!",APPM_RECONNENCT_DEVICE_NUM);
            }
        }
        
        {
            SUBLE_PRINTF("ROLE_MASTER Disconnected: 0x%02x", param->reason);
            g_conn_info[1].role = ROLE_END;
            g_conn_info[1].condix = GAP_INVALID_CONIDX;
            memset(&g_conn_info[1].mac, 0, sizeof(struct gap_bdaddr));
            
            suble_gap_master_disconn_handler();
        }
    }
    
//    if(ke_state_get(TASK_APPM) !=  APPM_CONNECTING)
//    {
//        appm_scan_adv_con_schedule();
//    }
    
    appc_cleanup(idx);

    ke_state_set(TASK_APPM, APPM_DISCONNECT);

    suble_adv_start();
    
    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: Handles profile add indication from the GAP.
*/
static int gapm_profile_added_ind_handler(ke_msg_id_t const msgid,
        struct gapm_profile_added_ind *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    return KE_MSG_CONSUMED;
}

/*********************************************************
FN: app period timer process
*/
static int app_period_timer_handler(ke_msg_id_t const msgid,
                                    void const *param,
                                    ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    return KE_MSG_CONSUMED;
}

/*********************************************************
FN: Handles reception of all messages sent from the lower layers to the application
*/
static int appm_msg_handler(ke_msg_id_t const msgid,
                            void *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id)
{
    ke_task_id_t src_task_id = MSG_T(msgid);
    uint8_t      msg_pol     = KE_MSG_CONSUMED;
    
    switch (src_task_id)
    {
        case (TASK_ID_GAPC): {
        } break;

        case (TASK_ID_GATTC): {
        } break;

        case (TASK_ID_FFF0S): {
            // Call the Health Thermometer Module
            msg_pol = appm_get_handler(&app_fff0_table_handler, msgid, param, src_id);
        } break;

        default: {
        } break;
    }
    return (msg_pol);
}

/*********************************************************
FN: 发现设备
*/
static int gapm_adv_report_ind_handler(ke_msg_id_t const msgid, struct adv_report const *param,
                                       ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    //状态检查
    if(APPM_GET_FIELD(SCAN_EN) == 0) {
//        SUBLE_PRINTF("scan stoped!!!!!");
        return KE_MSG_CONSUMED;
    }

//    appm_filter_adv_report(param);
    
    suble_scan_evt_handler(param);

    return KE_MSG_CONSUMED;
}

/*********************************************************
FN: 
*/
static int app_adv_evt_end_handler(ke_msg_id_t const msgid,
                                   void const *ind,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
//    SUBLE_PRINTF("app_adv_evt_end_handler");
    appm_scan_adv_con_schedule();
    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: 
*/
static int app_scan_evt_end_handler(ke_msg_id_t const msgid,
                                    void const *ind,
                                    ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
//    appm_scan_adv_con_schedule(); //扫描期间不开启广播
    return (KE_MSG_CONSUMED);
}

/*********************************************************
FN: 
*/
static int appm_scan_dev_timerout_handler(ke_msg_id_t const msgid,
        struct gapm_profile_added_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)

{
    extern volatile bool g_scan_success_flag;
    if(g_scan_success_flag) {
        g_scan_success_flag = false;
    }
    else {
        //停止扫描
        suble_scan_stop();
        
        suble_scan_timeout_handler();
    }
    
    return (KE_MSG_CONSUMED);
}


/*********************************************************
FN: 
*/
extern uint8_t free_channel_search(void);
static int appm_con_dev_timerout_handler(ke_msg_id_t const msgid,
        struct gapm_profile_added_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)

{
    SUBLE_PRINTF("%s", __func__);
    if (ke_state_get(TASK_APPM) == APPM_CONNECTING)
    {
//        appm_field_recover(); //APPM_FIELD_RECOVER();
//        appm_scan_adv_con_schedule();
        appm_stop_connencting();
        suble_gap_master_connect_timeout_handler();

        suble_adv_start();
    }

    return (KE_MSG_CONSUMED);
}

static int appm_stop_adv_timerout_handler(ke_msg_id_t const msgid,
        struct gapm_profile_added_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)

{
    SUBLE_PRINTF("%s", __func__);
    
    ke_state_set(TASK_APPM, APPM_ADVERTISTING_END);
    SUBLE_PRINTF("APPM_WAIT_ADVERTISTING_END");
    appm_scan_adv_con_schedule();
    
    if(g_adv_restart_glag)
    {
        SUBLE_PRINTF("restart adv");
        g_adv_restart_glag = false;
        suble_adv_start();
    }
    
    return (KE_MSG_CONSUMED);
}




//消息管理列表：消息 - 处理函数
const struct ke_msg_handler appm_default_state[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KE_MSG_DEFAULT_HANDLER,    	(ke_msg_func_t)appm_msg_handler},

    {GAPM_DEVICE_READY_IND,         (ke_msg_func_t)gapm_device_ready_ind_handler},
    {GAPM_PROFILE_ADDED_IND,    	(ke_msg_func_t)gapm_profile_added_ind_handler},
    {GAPM_CMP_EVT,             		(ke_msg_func_t)gapm_cmp_evt_handler},

    {GAPC_GET_DEV_INFO_REQ_IND, 	(ke_msg_func_t)gapc_get_dev_info_req_ind_handler},
    {GAPC_CONNECTION_REQ_IND,   	(ke_msg_func_t)gapc_connection_req_ind_handler},
    {GAPC_DISCONNECT_IND,       	(ke_msg_func_t)gapc_disconnect_ind_handler},

    {GAPC_LE_PKT_SIZE_IND,			(ke_msg_func_t)gapc_le_pkt_size_ind_handler},
    {GAPC_PARAM_UPDATE_REQ_IND, 	(ke_msg_func_t)gapc_param_update_req_ind_handler},
    {GAPC_PARAM_UPDATED_IND,		(ke_msg_func_t)gapc_param_updated_ind_handler},
    {GATTC_MTU_CHANGED_IND,         (ke_msg_func_t)gattc_mtu_changed_ind_handler},
    {GAPM_ADV_REPORT_IND,           (ke_msg_func_t)gapm_adv_report_ind_handler},
    {APP_PERIOD_TIMER,              (ke_msg_func_t)app_period_timer_handler},
    {USER_LLD_EVT_ADV_END,          (ke_msg_func_t)app_adv_evt_end_handler},
    {USER_LLD_EVT_SCAN_END,         (ke_msg_func_t)app_scan_evt_end_handler},

    {APPM_SCAN_TIMEOUT_TIMER,       (ke_msg_func_t)appm_scan_dev_timerout_handler},
    {APPM_CON_TIMEOUT_TIMER,        (ke_msg_func_t)appm_con_dev_timerout_handler},
    {APPM_STOP_ADV_TIMER,           (ke_msg_func_t)appm_stop_adv_timerout_handler},

    {SUBLE_TIMER0,                  (ke_msg_func_t)suble_timer0_handler},
    {SUBLE_TIMER1,                  (ke_msg_func_t)suble_timer1_handler},
    {SUBLE_TIMER2,                  (ke_msg_func_t)suble_timer2_handler},
    {SUBLE_TIMER3,                  (ke_msg_func_t)suble_timer3_handler},
    {SUBLE_TIMER4,                  (ke_msg_func_t)suble_timer4_handler},
    {SUBLE_TIMER5,                  (ke_msg_func_t)suble_timer5_handler},
    {SUBLE_TIMER6,                  (ke_msg_func_t)suble_timer6_handler},
    {SUBLE_TIMER7,                  (ke_msg_func_t)suble_timer7_handler},
    {SUBLE_TIMER8,                  (ke_msg_func_t)suble_timer8_handler},
    {SUBLE_TIMER9,                  (ke_msg_func_t)suble_timer9_handler},
    {SUBLE_TIMER10,                 (ke_msg_func_t)suble_timer10_handler},
    {SUBLE_TIMER11,                 (ke_msg_func_t)suble_timer11_handler},
    {SUBLE_TIMER12,                 (ke_msg_func_t)suble_timer12_handler},
    {SUBLE_TIMER13,                 (ke_msg_func_t)suble_timer13_handler},
    {SUBLE_TIMER14,                 (ke_msg_func_t)suble_timer14_handler},
    {SUBLE_TIMER15,                 (ke_msg_func_t)suble_timer15_handler},
    {SUBLE_TIMER16,                 (ke_msg_func_t)suble_timer16_handler},
    {SUBLE_TIMER17,                 (ke_msg_func_t)suble_timer17_handler},
    {SUBLE_TIMER18,                 (ke_msg_func_t)suble_timer18_handler},
    {SUBLE_TIMER19,                 (ke_msg_func_t)suble_timer19_handler},
    {SUBLE_TIMER100,                (ke_msg_func_t)suble_timer100_handler},
    {SUBLE_TIMER101,                (ke_msg_func_t)suble_timer101_handler},
    {SUBLE_TIMER102,                (ke_msg_func_t)suble_timer102_handler},
    {SUBLE_TIMER103,                (ke_msg_func_t)suble_timer103_handler},
    {SUBLE_TIMER104,                (ke_msg_func_t)suble_timer104_handler},
    {SUBLE_TIMER105,                (ke_msg_func_t)suble_timer105_handler},
    {SUBLE_TIMER106,                (ke_msg_func_t)suble_timer106_handler},
    {SUBLE_TIMER107,                (ke_msg_func_t)suble_timer107_handler},
    {SUBLE_TIMER108,                (ke_msg_func_t)suble_timer108_handler},
    {SUBLE_TIMER109,                (ke_msg_func_t)suble_timer109_handler},
    {SUBLE_TIMER200,                (ke_msg_func_t)suble_timer200_handler},
};
//指定消息管理列表
const struct ke_state_handler appm_default_handler = KE_STATE_HANDLER(appm_default_state);

/* Defines the place holder for the states of all the task instances. */
ke_state_t appm_state[APPM_IDX_MAX];





