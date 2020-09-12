#include "rwip_config.h"             // SW configuration
#if (BLE_APP_PRESENT)
#include "ke_timer.h"
#include "appm.h"
#include "appm_task.h"                // Application task Definition
#include "appc_task.h"                // Application task Definition
#include "master_app.h"                     // Application Definition
#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manager Task API
#include "gapc_task.h"               // GAP Controller Task API
#include "gattc_task.h"
#include "co_bt.h"                   // Common BT Definition
#include "co_math.h"                 // Common Maths Definition
#include "sdp_service_task.h"
#include "sdp_service.h"
#include "uart.h"
#include "gpio.h"




#if (BLE_CENTRAL || BLE_OBSERVER)
/*********************************************************************
 * LOCAL CONSTANT
 */
#define APPM_SCAN_TIME_10MS   1000

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 开始扫描
*/
void appm_start_scanning(void)
{
    SUBLE_PRINTF("%s",__func__);
    
    //处于就绪状态
    if (ke_state_get(TASK_APPM) == APPM_IDLE)
    {
        //准备消息队列，消息类型: GAPM_START_SCAN_CMD
        struct gapm_start_scan_cmd *cmd = KE_MSG_ALLOC(GAPM_START_SCAN_CMD,
                                          TASK_GAPM, TASK_APPM,
                                          gapm_start_scan_cmd);
        //扫描参数
        cmd->op.addr_src    = GAPM_STATIC_ADDR;     //源地址为Public或者Static
        cmd->op.code        = GAPM_SCAN_ACTIVE;     //扫描类型
        cmd->interval       = 5*8/5;                //扫描间隔 单位: 0.625ms
        cmd->window         = 5*8/5;                //扫描窗口 单位: 0.625ms
        cmd->mode           = GAP_OBSERVER_MODE;    //观察者模式 GAP_OBSERVER_MODE GAP_GEN_DISCOVERY
        cmd->filt_policy    = SCAN_ALLOW_ADV_ALL;   //不过滤
        cmd->filter_duplic  = SCAN_FILT_DUPLIC_DIS; //不过滤重复的 SCAN_FILT_DUPLIC_EN SCAN_FILT_DUPLIC_DIS
        //发送message
        ke_msg_send(cmd);
        
        //设置任务状态
        ke_state_set(TASK_APPM, APPM_SCANNING);
        
        if (!ke_timer_active(APPM_SCAN_TIMEOUT_TIMER, TASK_APPM)) {
            ke_timer_set(APPM_SCAN_TIMEOUT_TIMER, TASK_APPM, APPM_SCAN_TIME_10MS);
        }
        else {
            SUBLE_PRINTF("scanning restart");
        }
    }
    else {
        SUBLE_PRINTF("ke_state_get(TASK_APPM) = %x",ke_state_get(TASK_APPM));
    }
}

/*********************************************************
FN: 
*/
void appm_stop_scanning(void)
{
    SUBLE_PRINTF("%s",__func__);
    if (ke_state_get(TASK_APPM) == APPM_SCANNING)
    {
        // Prepare the GAPM_CANCEL_CMD message
        struct gapm_cancel_cmd *cmd = KE_MSG_ALLOC(GAPM_CANCEL_CMD,
                                      TASK_GAPM, TASK_APPM,
                                      gapm_cancel_cmd);
        cmd->operation = GAPM_CANCEL;
        // Send the message
        ke_msg_send(cmd);
        // Go in ready state
        ke_state_set(TASK_APPM, APPM_WAIT_SCAN_END);
    }
    else
    {
        SUBLE_PRINTF("stop_scanning fail ke_state_get(TASK_APP) = %x",ke_state_get(TASK_APPM));
    }
}

/*********************************************************
FN: 
*/
uint8_t appm_start_connencting(struct gap_bdaddr bdaddr)
{
    uint8_t ret = APPM_ERROR_NO_ERROR;
    
    if (ke_state_get(TASK_APPM) == APPM_CONNECTING) {
        return APPM_ERROR_STATE;
    }
    
    if(appm_env.master_num < APPM_MASTER_CON_MAX)
    {
        appm_env.recon_num = APPM_RECONNENCT_DEVICE_NUM;

        if ((ke_state_get(TASK_APPM) == APPM_IDLE))
        {
            // Prepare the GAPM_START_SCAN_CMD message
            struct gapm_start_connection_cmd *cmd = KE_MSG_ALLOC_DYN(GAPM_START_CONNECTION_CMD,
                                                    TASK_GAPM, TASK_APPM,
                                                    gapm_start_connection_cmd, sizeof(struct gap_bdaddr));
            cmd->op.addr_src = GAPM_STATIC_ADDR;
            cmd->op.code = GAPM_CONNECTION_DIRECT;
            cmd->scan_interval = 5*8/5;
            cmd->scan_window  = 5*8/5;
            cmd->con_intv_min = 20;
            cmd->con_intv_max = 24;
            cmd->con_latency = 0;
            cmd->superv_to = 500;
            cmd->ce_len_min = 10;
            cmd->ce_len_max = 20;
            cmd->nb_peers = 1;

            cmd->peers[0].addr_type = bdaddr.addr_type;
            memcpy(&cmd->peers[0].addr.addr[0],&(bdaddr.addr.addr[0]),6);
            
            SUBLE_HEXDUMP("addr", &cmd->peers[0].addr.addr[0], 6);
            SUBLE_PRINTF("Start connect!");
            
            // Send the message
            ke_msg_send(cmd);
            // Set the state of the task to APPM_SCANNING
            ke_state_set(TASK_APPM, APPM_CONNECTING);
            ke_timer_set(APPM_CON_TIMEOUT_TIMER,TASK_APPM,APPM_CONNENCT_DEVICE_CONTIU_TIME); // 4S,call con timeout func to stop connecting
            //  set_flash_clk(0xa);
        }
        else {
            SUBLE_PRINTF("cur state = %x, can't start connecting ,delay !!!",ke_state_get(TASK_APPM));
            appm_env.con_dev_addr = bdaddr;
            appm_env.con_dev_flag = 1;

            appm_field_save_clean(); //APPM_FIELD_SAVE_CLEAN();

        }
    } else
    {
        ret = APPM_ERROR_LINK_MAX;
    }

    return ret;
}

/*********************************************************
FN: 
*/
uint8_t appm_stop_connencting(void)
{
    SUBLE_PRINTF("func %s",__func__);
    uint8_t ret = APPM_ERROR_NO_ERROR;
    if (ke_state_get(TASK_APPM) == APPM_CONNECTING)
    {
        // Go in ready state
        // Prepare the GAPM_CANCEL_CMD message
        struct gapm_cancel_cmd *cmd = KE_MSG_ALLOC(GAPM_CANCEL_CMD,
                                      TASK_GAPM, TASK_APPM,
                                      gapm_cancel_cmd);
        cmd->operation = GAPM_CANCEL;
        // Send the message
        ke_msg_send(cmd);
    } else
    {
        ret = APPM_ERROR_STATE;
    }

    return ret;
}

/*********************************************************
FN: 
*/
uint8_t appm_disconnect(uint8_t conidx)
{
    SUBLE_PRINTF("func %s",__func__);
    uint8_t ret = APPM_ERROR_NO_ERROR;
    if((ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_LINK_CONNECTED) ||(ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SDP_DISCOVERING)||(ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SERVICE_CONNECTED) )
    {
        struct gapc_disconnect_cmd *cmd = KE_MSG_ALLOC(GAPC_DISCONNECT_CMD,
                                          KE_BUILD_ID(TASK_GAPC, conidx), KE_BUILD_ID(TASK_APPC,conidx),
                                          gapc_disconnect_cmd);

        cmd->operation = GAPC_DISCONNECT;
        cmd->reason    = CO_ERROR_REMOTE_USER_TERM_CON;

        // Send the message
        ke_msg_send(cmd);
    }
    else
    {
        ret = APPM_ERROR_LINK_LOSS;
        SUBLE_PRINTF("ke_state_get(TASK_APP) = %x",ke_state_get(TASK_APPM));
    }

    return ret;
}

/*********************************************************
FN: 
*/
uint8_t appc_write_service_data_req(uint8_t conidx,uint16_t handle,uint16_t data_len,uint8_t *data)
{
    uint8_t ret = APPM_ERROR_NO_ERROR;
    if((ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SERVICE_CONNECTED))
    {
        struct sdp_write_info_req *req = KE_MSG_ALLOC_DYN(SDP_WRITE_VALUE_INFO_REQ,
                                         prf_get_task_from_id(KE_BUILD_ID(TASK_ID_SDP,conidx)), KE_BUILD_ID(TASK_APPC,conidx),
                                         sdp_write_info_req, data_len);
        // Fill in the parameter structure
        req->conidx = conidx;
        req->handle = handle;
        req->length = data_len;
        memcpy(req->data,data,data_len);
        // Send the message
        ke_msg_send(req);
    }
    else
    {
        ret = APPM_ERROR_STATE;
        SUBLE_PRINTF("ke_state_get(TASK_APP) = %x",ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)));
    }

    return ret;
}

/*********************************************************
FN: 
*/
uint8_t appc_write_service_ntf_cfg_req(uint8_t conidx,uint16_t handle,uint16_t ntf_cfg)
{
//    SUBLE_PRINTF("func %s,state = %x,",__func__,ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)));
//    SUBLE_PRINTF("conidx:%d,handle = 0x%04x,ntf_cfg = 0x%x",conidx,handle,ntf_cfg);

    uint8_t ret = APPM_ERROR_NO_ERROR;
    if((ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) != APPC_LINK_IDLE))
    {
        struct sdp_write_ntf_cfg_req *req = KE_MSG_ALLOC(SDP_WRITE_NTF_CFG_REQ,
                                            prf_get_task_from_id(KE_BUILD_ID(TASK_ID_SDP,conidx)), KE_BUILD_ID(TASK_APPC,conidx),
                                            sdp_write_ntf_cfg_req);
//        SUBLE_PRINTF("dest = 0x%04x",prf_get_task_from_id(KE_BUILD_ID(TASK_ID_SDP,conidx)));
        // Fill in the parameter structure
        req->conidx = conidx;
        req->handle = handle;
        req->ntf_cfg = ntf_cfg;
        // Send the message
        ke_msg_send(req);
    }
    else
    {
        ret = APPM_ERROR_STATE;
        SUBLE_PRINTF("ke_state_get(TASK_APPC) = %x",ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)));
    }

    return ret;
}

/*********************************************************
FN: 
*/
uint8_t appm_read_service_data_by_uuid_req(uint8_t conidx,uint8_t uuid_len,uint8_t* uuid)
{
    SUBLE_PRINTF("func %s",__func__);

    uint8_t ret = APPM_ERROR_NO_ERROR;
    if((ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SERVICE_CONNECTED))
    {
        struct sdp_read_info_req *req = KE_MSG_ALLOC(SDP_READ_INFO_REQ,
                                        prf_get_task_from_id(TASK_ID_SDP), TASK_APPM,
                                        sdp_read_info_req);
        // Fill in the parameter structure
        req->uuid_len = uuid_len;
        memcpy(req->uuid,uuid,uuid_len);

        req->info = SDPC_CHAR_VAL;
        req->type = SDPC_OPERATE_UUID;
        // Send the message
        ke_msg_send(req);
    }
    else
    {
        ret = APPM_ERROR_STATE;
        SUBLE_PRINTF("ke_state_get(TASK_APP) = %x",ke_state_get(TASK_APPM));
    }
    return ret;
}

/*********************************************************
FN: 
*/
uint8_t appm_read_service_data_by_handle_req(uint8_t conidx,uint16_t handle)
{
    SUBLE_PRINTF("func %s",__func__);

    uint8_t ret = APPM_ERROR_NO_ERROR;
    if((ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SERVICE_CONNECTED))
    {
        struct sdp_read_info_req *req = KE_MSG_ALLOC(SDP_READ_INFO_REQ,
                                        prf_get_task_from_id(TASK_ID_SDP), TASK_APPM,
                                        sdp_read_info_req);
        // Fill in the parameter structure
        req->handle = handle;
        req->info = SDPC_CHAR_VAL;
        req->type = SDPC_OPERATE_HANDLE;
        // Send the message
        ke_msg_send(req);
    }
    else
    {
        ret = APPM_ERROR_STATE;
        SUBLE_PRINTF("ke_state_get(TASK_APP) = %x",ke_state_get(TASK_APPM));
    }
    return ret;
}

/*********************************************************
FN: 
*/
uint8_t appm_read_service_ntf_ind_cfg_by_handle_req(uint8_t conidx,uint16_t handle)
{
    SUBLE_PRINTF("func %s",__func__);

    uint8_t ret = APPM_ERROR_NO_ERROR;
    if((ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SERVICE_CONNECTED))
    {
        struct sdp_read_info_req *req = KE_MSG_ALLOC(SDP_READ_INFO_REQ,
                                        prf_get_task_from_id(TASK_ID_SDP), TASK_APPM,
                                        sdp_read_info_req);
        // Fill in the parameter structure
        req->handle = handle;
        req->info = SDPC_CHAR_NTF_CFG;
        req->type = SDPC_OPERATE_HANDLE;
        // Send the message
        ke_msg_send(req);
    }
    else
    {
        ret = APPM_ERROR_STATE;
        SUBLE_PRINTF("ke_state_get(TASK_APP) = %x",ke_state_get(TASK_APPM));
    }
    return ret;
}

/*********************************************************
FN: 
*/
uint8_t appm_read_service_userDesc_by_handle_req(uint8_t conidx,uint16_t handle)
{
    SUBLE_PRINTF("func %s",__func__);

    uint8_t ret = APPM_ERROR_NO_ERROR;
    if((ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SERVICE_CONNECTED))
    {
        struct sdp_read_info_req *req = KE_MSG_ALLOC(SDP_READ_INFO_REQ,
                                        prf_get_task_from_id(TASK_ID_SDP), TASK_APPM,
                                        sdp_read_info_req);
        // Fill in the parameter structure
        req->handle = handle;
        req->info = SDPC_CHAR_USER_DESC_VAL;
        req->type = SDPC_OPERATE_HANDLE;
        // Send the message
        ke_msg_send(req);
    }
    else
    {
        ret = APPM_ERROR_STATE;
        SUBLE_PRINTF("ke_state_get(TASK_APP) = %x",ke_state_get(TASK_APPM));
    }
    return ret;
}

/*********************************************************
FN: 
*/
uint8_t sdp_enable_all_server_ntf_ind(uint8_t conidx,uint8_t  reset)
{
    SUBLE_PRINTF("func %s",__func__);
    bool more_enable = false;
    static uint8_t server_num = 0,chars_num = 0;
    if(reset == 1)
    {
        server_num = 0;
        chars_num = 0;
    }
    for(; server_num < SDP_NB_SERVICE_INSTANCES_MAX;)
    {
        //SUBLE_PRINTF("server_num = 0x%x,use_status = 0x%x\r\n",server_num,sdp_env_init.used_status[server_num] );
        //SUBLE_PRINTF("p_use_status = 0x%x\r\n",&Sdp_env[server_num].use_status);
        if( sdp_env_init.used_status[server_num] == USED_STATUS)
        {
            //SUBLE_PRINTF("server_num = %d,chars_nb = %d\r\n",server_num,sdp_env_init.sdp_env[server_num].prf_db_env->sdp_cont->chars_nb);
            for(; chars_num < sdp_env_init.sdp_env[server_num].prf_db_env->sdp_cont->chars_nb;)
            {
                SUBLE_PRINTF("server_num = %d,chars_num = %d,prop = 0x%x",server_num,chars_num,sdp_env_init.sdp_env[server_num].prf_db_env->sdp_cont->chars_descs_inf.chars_inf[chars_num].prop);
                if(sdp_env_init.sdp_env[server_num].prf_db_env->sdp_cont->chars_descs_inf.chars_inf[chars_num].prop & ATT_CHAR_PROP_IND)
                {

                    appc_write_service_ntf_cfg_req(conidx,sdp_env_init.sdp_env[server_num].prf_db_env->sdp_cont->chars_descs_inf.chars_inf[chars_num].val_hdl,PRF_CLI_START_IND);

                    more_enable = true;
                    chars_num++;
                    return more_enable;
                }
                else if(sdp_env_init.sdp_env[server_num].prf_db_env->sdp_cont->chars_descs_inf.chars_inf[chars_num].prop & ATT_CHAR_PROP_NTF)
                {

                    appc_write_service_ntf_cfg_req(conidx,sdp_env_init.sdp_env[server_num].prf_db_env->sdp_cont->chars_descs_inf.chars_inf[chars_num].val_hdl,PRF_CLI_START_NTF);
                    more_enable = true;
                    chars_num++;
                    return more_enable;
                }
                else
                {
                    chars_num++;
                }
            }
            server_num++;
            chars_num = 0;
        }
        else
        {
            more_enable = false;
            return more_enable;
        }
    }
    return more_enable;
}

/*********************************************************
FN: 
*/
void sdp_prf_register_all_atthdl2gatt(void)
{
    uint8_t idx = 0;
    for(idx = 0; idx < SDP_NB_SERVICE_INSTANCES_MAX; idx++)
    {
        struct sdp_env_tag* sdp_env = (struct sdp_env_tag*)&sdp_env_init.sdp_env[idx];

        if(0 != sdp_env->prf_db_env->sdp_cont->descs_nb)
        {
            SUBLE_PRINTF("prf_register_atthdl2gatt start_hdl = 0x%x,end_hdl = 0x%x",sdp_env->prf_db_env->sdp_cont->svc.shdl,sdp_env->prf_db_env->sdp_cont->svc.ehdl);
            prf_register_atthdl2gatt(&(sdp_env->prf_env),sdp_env->conidx, &sdp_env->prf_db_env->sdp_cont->svc);
        }
    }
}

#endif // (#if (BLE_CENTRAL || BLE_OBSERVER))
#endif  //#if (BLE_APP_PRESENT)

