#include "suble_common.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */
static uint8_t s_bt_mac_default[SUBLE_BT_MAC_LEN] = {0x55, 0x44, 0x33, 0x22, 0x11, 0xFC};
static uint8_t s_bt_mac_invalid[SUBLE_BT_MAC_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

//0-slave, 1-master
conn_info_t g_conn_info[2] = {
    {
        .condix = GAP_INVALID_CONIDX,
        .role = ROLE_END,
    },
    {
        .condix = GAP_INVALID_CONIDX,
        .role = ROLE_END,
    },
};

static suble_connect_result_handler_t suble_connect_result_handler;

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 发起连接
*/
uint32_t suble_gap_connect(struct gap_bdaddr bdaddr, suble_connect_result_handler_t handler)
{
    suble_connect_result_handler = handler;
    return appm_start_connencting(bdaddr);
}

/*********************************************************
FN: 断开连接
*/
void suble_gap_disconnect(uint16_t condix, uint8_t hci_status_code)
{
    appm_disconnect(condix);
}

/*********************************************************
FN: 
*/
void suble_gap_disconnect_for_tuya_ble_sdk(void)
{
    appm_disconnect(g_conn_info[0].condix);
    SUBLE_PRINTF("suble_gap_disconnect_for_tuya_ble_sdk");
}

/*********************************************************
FN: 
*/
void suble_gap_conn_param_update(uint16_t condix, uint16_t cMin, uint16_t cMax, uint16_t latency, uint16_t timeout)
{
	struct gapc_conn_param  up_param;
	up_param.intv_min   = (cMin*4)/5;
	up_param.intv_max   = (cMax*4)/5;
	up_param.latency    = latency;
	up_param.time_out   = timeout/10;
//	appm_update_param(&up_param);
    appc_update_param(condix, up_param.intv_min, up_param.intv_max, up_param.latency, up_param.time_out);
}

/*********************************************************
FN: 
*/
void suble_gap_init_bt_mac(void)
{
    uint8_t tmp_bt_mac_str[SUBLE_BT_MAC_STR_LEN] = TUYA_DEVICE_MAC;
    suble_util_str_hexstr2hexarray(tmp_bt_mac_str, SUBLE_BT_MAC_STR_LEN, s_bt_mac_default);
    suble_util_reverse_byte(s_bt_mac_default, SUBLE_BT_MAC_LEN);
    memcpy(co_default_bdaddr.addr, s_bt_mac_default, SUBLE_BT_MAC_LEN);
    
    uint8_t tmp_bt_mac[SUBLE_BT_MAC_LEN];
    suble_flash_read(SUBLE_FLASH_BT_MAC_ADDR, tmp_bt_mac, SUBLE_BT_MAC_LEN);
    if(0 != memcmp(tmp_bt_mac, s_bt_mac_invalid, SUBLE_BT_MAC_LEN))
    {
        memcpy(co_default_bdaddr.addr, tmp_bt_mac, SUBLE_BT_MAC_LEN);
    }
    SUBLE_HEXDUMP("bt_mac", co_default_bdaddr.addr, SUBLE_BT_MAC_LEN);
}

/*********************************************************
FN: 
*/
void suble_gap_set_bt_mac(uint8_t *pMac)
{
    suble_flash_erase(SUBLE_FLASH_BT_MAC_ADDR, 1);
    suble_flash_write(SUBLE_FLASH_BT_MAC_ADDR, pMac, SUBLE_BT_MAC_LEN);
    memcpy(co_default_bdaddr.addr, pMac, SUBLE_BT_MAC_LEN);
}

/*********************************************************
FN: 
*/
void suble_gap_get_bt_mac(uint8_t *pMac, uint32_t size)
{
    if((pMac == NULL) || (size < SUBLE_BT_MAC_LEN)) {
        SUBLE_PRINTF("suble_gap_get_bt_mac: param error");
        return;
    }
    
    uint8_t tmp_bt_mac[SUBLE_BT_MAC_LEN];
    suble_flash_read(SUBLE_FLASH_BT_MAC_ADDR, tmp_bt_mac, SUBLE_BT_MAC_LEN);
    memcpy(pMac, tmp_bt_mac, SUBLE_BT_MAC_LEN);
}

/*********************************************************
FN: 
*/
void suble_gap_conn_handler(void)
{
    tuya_ble_app_evt_send(APP_EVT_CONNECTED);
}

/*********************************************************
FN: 
*/
void suble_gap_disconn_handler(void)
{
    tuya_ble_app_evt_send(APP_EVT_DISCONNECTED);
}

/*********************************************************
FN: 
*/
void suble_gap_master_conn_handler(void)
{
    suble_connect_result_handler(SUBLE_GAP_EVT_CONNECTED, NULL, 0);
}

/*********************************************************
FN: 
*/
void suble_gap_master_disconn_handler(void)
{
    suble_connect_result_handler(SUBLE_GAP_EVT_DISCONNECTED, NULL, 0);
}

/*********************************************************
FN: 
*/
void suble_gap_master_connect_timeout_handler(void)
{
    suble_connect_result_handler(SUBLE_GAP_EVT_CONNECT_TIMEOUT, NULL, 0);
}
















