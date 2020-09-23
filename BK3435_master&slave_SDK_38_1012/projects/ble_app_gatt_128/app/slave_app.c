#include "rwip_config.h"             // SW configuration
#if (BLE_APP_PRESENT)
#include <string.h>
#include "rwapp_config.h"
#include "appm_task.h"                // Application task Definition
#include "appm.h"                     // Application Definition
#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manager Task API
#include "gapc_task.h"               // GAP Controller Task API
#include "co_bt.h"                   // Common BT Definition
#include "co_math.h"                 // Common Maths Definition
#include "ke_timer.h"
#include "app_fcc0.h"                 // Application security Definition
#include "app_dis.h"                 // Device Information Service Application Definitions
#include "app_batt.h"                // Battery Application Definitions
#include "app_oads.h"                 // Application oads Definition
#include "nvds.h"                    // NVDS Definitions
#include "rf.h"
#include "uart.h"
#include "adc.h"
#include "gpio.h"
#include "wdt.h"
#include "gattc_task.h"




extern uint8_t ble_dev_name[];
extern uint8_t scan_rsp_data[];




/*********************************************************
FN: 开始广播
*/
void appm_start_advertising(void)
{
    //处于就绪状态
    if (ke_state_get(TASK_APPM) == APPM_IDLE)
    {
        //准备消息队列，消息类型：GAPM_START_ADVERTISE_CMD
        struct gapm_start_advertise_cmd *cmd = KE_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                               TASK_GAPM, TASK_APPM,
                                               gapm_start_advertise_cmd);
        //广播参数
        cmd->op.addr_src    = GAPM_STATIC_ADDR;                     //源地址为Public或者Static
        cmd->op.code        = g_adv_param.adv_type;                 //广播类型 - 不定向可连接
        cmd->channel_map    = g_adv_param.adv_channal_map;          //广播通道
        cmd->intv_min 		= ((g_adv_param.adv_interval_min*8)/5); //广播间隔 - min
        cmd->intv_max 		= ((g_adv_param.adv_interval_max*8)/5); //广播间隔 - max
        cmd->info.host.mode = GAP_GEN_DISCOVERABLE;                 //广播模式 - 通用可发现模式

 		// 初始化广播数据
        cmd->info.host.adv_data_len = g_adv_data.len-3;
        memcpy(&cmd->info.host.adv_data, &g_adv_data.value[3], g_adv_data.len-3);
        // 初始化扫描响应数据
        cmd->info.host.scan_rsp_data_len = g_scan_rsp.len;
        memcpy(&cmd->info.host.scan_rsp_data, g_scan_rsp.value, g_scan_rsp.len);

        //发送message
        ke_msg_send(cmd);
	 	SUBLE_PRINTF("start advertising");

        //设置任务状态
        ke_state_set(TASK_APPM, APPM_ADVERTISING);	
    }
    else {
        SUBLE_PRINTF("ke_state_get(TASK_APPM) = %x",ke_state_get(TASK_APPM));
    }
}

/*********************************************************
FN: 停止广播
*/
void appm_stop_advertising(void)
{
    if (ke_state_get(TASK_APPM) == APPM_ADVERTISING)
    {
        //准备消息队列，消息类型：GAPM_CANCEL_CMD
        struct gapm_cancel_cmd *cmd = KE_MSG_ALLOC(GAPM_CANCEL_CMD,
                                                   TASK_GAPM, TASK_APPM,
                                                   gapm_cancel_cmd);
        cmd->operation = GAPM_CANCEL;

        //发送message
        ke_msg_send(cmd);
	 	SUBLE_PRINTF("appm stop advertising");

        //设置任务状态
        ke_state_set(TASK_APPM, APPM_WAIT_ADVERTISTING_END);
        ke_timer_set(APPM_STOP_ADV_TIMER,TASK_APPM,500);
    }
}

/*********************************************************
FN: 更新广播数据
*/
void appm_update_adv_data( uint8_t* adv_buff, uint8_t adv_len, uint8_t* scan_buff, uint8_t scan_len)
{
    if (ke_state_get(TASK_APPM) == APPM_ADVERTISING
            && (adv_len <= ADV_DATA_LEN) && (scan_len <= ADV_DATA_LEN))
    {
        struct gapm_update_advertise_data_cmd *cmd =  KE_MSG_ALLOC(
                    GAPM_UPDATE_ADVERTISE_DATA_CMD,
                    TASK_GAPM,
                    TASK_APPM,
                    gapm_update_advertise_data_cmd);

        cmd->operation = GAPM_UPDATE_ADVERTISE_DATA;
        cmd->adv_data_len = adv_len-3;
        cmd->scan_rsp_data_len = scan_len;

        //memcpy
        memcpy(&cmd->adv_data[0], &adv_buff[3], adv_len-3);
        memcpy(&cmd->scan_rsp_data[0], scan_buff, scan_len);

        // Send the message
        ke_msg_send(cmd);
    }
}

/*********************************************************
FN: 更新连接参数
*/
void appm_update_param(struct gapc_conn_param *conn_param)
{
    // Prepare the GAPC_PARAM_UPDATE_CMD message
    struct gapc_param_update_cmd *cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD,
                                                     KE_BUILD_ID(TASK_GAPC, appm_env.conidx), TASK_APPM,
                                                     gapc_param_update_cmd);
    cmd->operation  = GAPC_UPDATE_PARAMS;
    cmd->intv_min   = conn_param->intv_min;
    cmd->intv_max   = conn_param->intv_max;
    cmd->latency    = conn_param->latency;
    cmd->time_out   = conn_param->time_out;
    // not used by a slave device
    cmd->ce_len_min = 0xFFFF;
    cmd->ce_len_max = 0xFFFF;
//    SUBLE_PRINTF("appm_update_param: intv_min = %d, intv_max = %d, latency = %d, time_out = %d",
//		cmd->intv_min,cmd->intv_max,cmd->latency,cmd->time_out);
    // Send the message
    ke_msg_send(cmd);
}


#endif //BLE_APP_PRESENT


