#include "rwip_config.h"             // SW configuration
#include <string.h>
#include "ke_mem.h"
#include "ke_timer.h"
#include "gattc_task.h"
#include "appc.h"
#include "appc_task.h"
#include "uart.h"
#include "cli.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */
//GAP Controller task descriptor
static const struct ke_task_desc TASK_DESC_APPC = {NULL, &appc_default_handler, appc_state, APPC_STATE_MAX, APPC_IDX_MAX};

/*********************************************************************
 * VARIABLE
 */
//GAP Controller环境变量
struct appc_env_tag *appc_env[APPC_IDX_MAX];

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/
void appc_init(void)
{
    for (uint8_t conidx=0; conidx<APPC_IDX_MAX; conidx++) {
        appc_env[conidx] = NULL;
    }
    ke_task_create(TASK_APPC, &TASK_DESC_APPC);
}

/*********************************************************
FN: 为新连接的设备创建一个环境变量
*/
void appc_create(uint8_t conidx, struct gapc_connection_req_ind const* con_req)
{
    //环境变量 申请内存
    if(appc_env[conidx] == NULL) {
        appc_env[conidx] = (void*)ke_malloc(sizeof(struct appc_env_tag), KE_MEM_ENV);
    }
    memset((void*)appc_env[conidx], 0, sizeof(struct appc_env_tag));

    //记录设备信息
    appc_env[conidx]->role = con_req->con_role;
    appc_env[conidx]->con_dev_addr.addr_type = con_req->peer_addr_type;
    memcpy(&(appc_env[conidx]->con_dev_addr.addr.addr[0]), &(con_req->peer_addr.addr[0]), 6);

    //设备处于就绪状态
    ke_state_set(KE_BUILD_ID(TASK_APPC, conidx), APPC_LINK_CONNECTED);
    
    ke_timer_set(APPC_CHECK_LINK_TIMER, KE_BUILD_ID(TASK_APPC, conidx), con_req->con_interval*1.25);
}

/*********************************************************
FN: 
*/
void appc_cleanup(uint8_t conidx)
{
    ASSERT_ERR(conidx < APPC_IDX_MAX);

    ke_free(appc_env[conidx]);
    
    appc_env[conidx] = NULL;

    // set device into a ready state
    ke_state_set(KE_BUILD_ID(TASK_APPC, conidx), APPC_LINK_IDLE);
}

/*********************************************************
FN: 
*/
void appc_gatt_mtu_change(uint8_t conidx)
{
    if((ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_LINK_CONNECTED) \
        ||(ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SDP_DISCOVERING) \
        ||(ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SERVICE_CONNECTED))
    {
        struct gattc_exc_mtu_cmd *cmd = KE_MSG_ALLOC(GATTC_EXC_MTU_CMD, KE_BUILD_ID(TASK_GATTC, conidx), 
                                            KE_BUILD_ID(TASK_APPC, conidx), gattc_exc_mtu_cmd);
        cmd->operation = GATTC_MTU_EXCH;
        cmd->seq_num = 0;
        ke_msg_send(cmd);
    }
    else {
        SUBLE_PRINTF("%s can't pro. cur state :%d",ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)));
    }
}

/*********************************************************
FN: 
*/
void appc_le_data_length_update_req(uint8_t conidx,uint16_t max_tx_octets,uint16_t max_tx_time)
{
    if((ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_LINK_CONNECTED) \
        ||(ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SDP_DISCOVERING) \
        ||(ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SERVICE_CONNECTED))
    {
        // Prepare the GAPC_SET_LE_PKT_SIZE_CMD message
        struct gapc_set_le_pkt_size_cmd *cmd = KE_MSG_ALLOC(GAPC_SET_LE_PKT_SIZE_CMD,
                                                     KE_BUILD_ID(TASK_GAPC, conidx),KE_BUILD_ID(TASK_APPC,conidx) , //KE_BUILD_ID(TASK_APPM,conidx)
                                                     gapc_set_le_pkt_size_cmd);
        cmd->operation  = GAPC_SET_LE_PKT_SIZE;
        cmd->tx_octets   = max_tx_octets;
        cmd->tx_time   = max_tx_time;
        SUBLE_PRINTF("appc_le_data_length_update_req tx_octets-%d,tx_time-%d", max_tx_octets, max_tx_time);
        // Send the message
        ke_msg_send(cmd);
    }
    else {
        SUBLE_PRINTF("%s can't pro. cur state :%d", ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)));
    }
}

/*********************************************************
FN: 
*/
void appc_update_param(uint8_t conidx,uint16_t intv_min,uint16_t intv_max,uint16_t latency,uint16_t time_out)
{
    if((ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_LINK_CONNECTED) ||(ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SDP_DISCOVERING)\
            ||(ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)) == APPC_SERVICE_CONNECTED) )
    {
        // Prepare the GAPC_PARAM_UPDATE_CMD message
        struct gapc_param_update_cmd *cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD,
                                                     KE_BUILD_ID(TASK_GAPC, conidx), KE_BUILD_ID(TASK_APPC,conidx),
                                                     gapc_param_update_cmd);
        cmd->operation  = GAPC_UPDATE_PARAMS;
        cmd->intv_min   = intv_min;
        cmd->intv_max   = intv_max;
        cmd->latency    = latency;
        cmd->time_out   = time_out;
        // not used by a slave device
        cmd->ce_len_min = 0xFFFF;
        cmd->ce_len_max = 0xFFFF;
//        SUBLE_PRINTF("intv_min = %d,intv_max = %d,latency = %d,time_out = %d",
//        cmd->intv_min,cmd->intv_max,cmd->latency,cmd->time_out);
        // Send the message
        ke_msg_send(cmd);
    }
    else {
        SUBLE_PRINTF("%s can't pro. cur state :%d",ke_state_get(KE_BUILD_ID(TASK_APPC,conidx)));
    }
}

/*********************************************************
FN: 
    /// GAP request type:
    /// - GAPC_GET_PEER_NAME: Retrieve name of peer device.
    /// - GAPC_GET_PEER_VERSION: Retrieve peer device version info.
    /// - GAPC_GET_PEER_FEATURES: Retrieve peer device features.
    /// - GAPC_GET_CON_RSSI: Retrieve connection RSSI.
    /// - GAPC_GET_CON_CHANNEL_MAP: Retrieve Connection Channel MAP.
    /// - GAPC_GET_PEER_APPEARANCE: Get Peer device appearance
    /// - GAPC_GET_PEER_SLV_PREF_PARAMS: Get Peer device Slaved Preferred Parameters
    /// - GAPC_GET_ADDR_RESOL_SUPP: Address Resolution Supported
    /// - GAPC_GET_LE_PING_TIMEOUT: Retrieve LE Ping Timeout Value
*/
uint8_t appc_get_peer_dev_info(uint8_t conidx, uint8_t type)
{
    // Send security request
    struct gapc_get_info_cmd *cmd = KE_MSG_ALLOC(GAPC_GET_INFO_CMD,
                                    KE_BUILD_ID(TASK_GAPC, conidx),KE_BUILD_ID(TASK_APPC,conidx),
                                    gapc_get_info_cmd);
    cmd->operation = type;
    // copy name to provided pointer
    ke_msg_send(cmd);

    return 0;
}

