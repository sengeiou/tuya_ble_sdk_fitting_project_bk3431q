#include "suble_common.h"
#include "lock_timer.h"
#include "tuya_ble_master_port.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

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
static void tmp_key_handler(tuya_ble_master_operation_t operation);




/*********************************************************
FN: 
*/



/*********************************************************************
 * LOCAL CONSTANTS
 */
#define KEY_COUNT_1  (4/1)
#define KEY_COUNT_2  (300/1)
#define KEY_COUNT_3  (1000/1)

enum
{
    KEY_STATE_READY = 0,
    KEY_STATE_PRESSED_1,
    KEY_STATE_PRESSED_2,
    KEY_STATE_RELEASE,
    KEY_STATE_RELEASE_0, //4
    KEY_STATE_RELEASE_1,
    KEY_STATE_RELEASE_2,
};

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint32_t s_key_press_count = 0;




/*********************************************************
FN: 按键处理函数
*/
static void suble_key_state_handler(uint8_t state)
{
//    SUBLE_PRINTF("KEY_STATE: %d\r\n", state);
    switch(state)
    {
        case KEY_STATE_PRESSED_1: {
        } break;
        
        case KEY_STATE_PRESSED_2: {
        } break;
        
        case KEY_STATE_RELEASE: {
            lock_factory_handler();
        } break;
        
        case KEY_STATE_RELEASE_1: {
            if(s_key_press_count < 1) {
                s_key_press_count++;
                suble_timer_start_0(SUBLE_TIMER101, 500, 1);
            }
            else {
                s_key_press_count = 0;
                suble_timer_stop_0(SUBLE_TIMER101);
                
                //双击
                SUBLE_PRINTF("hit 2");
                tmp_key_handler(TUYA_BLE_MASTER_OPERATION_CLOSE);
            }
        } break;
        
        default: {
        } break;
    }
}

/*********************************************************
FN: 按键超时处理
*/
void suble_key_timeout_handler(void)
{
    #define SUBLE_KEY_TIME      5
    #define SUBLE_VALID_LEVEL   SUBLE_LEVEL_LOW
    static uint8_t  key_log[SUBLE_KEY_TIME];
    static uint32_t key_log_idx = 0;
    uint32_t key_log_sum = 0;
    static uint32_t key_count   = 0;
    static bool     key_pressed = true;
    static int      key_state   = KEY_STATE_READY;
    
    //按键计数
    key_count++;
    //中间消抖计数
    key_log_idx++;
    if(key_log_idx == SUBLE_KEY_TIME)
    {
        key_log_idx = 0;
    }
    
    //记录前10次的状态
    if(suble_gpio_get_input(ANTILOCK_BUTTON_PIN) == SUBLE_VALID_LEVEL) {
        key_log[key_log_idx] = 0;
    } else {
        key_log[key_log_idx] = 1;
    }
    //统计前10次的状态
    for(uint8_t idx=0; idx<SUBLE_KEY_TIME; idx++) {
        key_log_sum += key_log[idx];
    }
    //无效次数大于7次
    if(key_log_sum > 4) {
        key_pressed = false;
        memset(key_log, 0, SUBLE_KEY_TIME);
    }
    
//    SUBLE_PRINTF(": %d\r\n", key_pressed);
    if(key_pressed == true)
    {
        if(key_count == KEY_COUNT_1) {
            if(key_state == KEY_STATE_READY) {
                key_state = KEY_STATE_PRESSED_1;
                suble_key_state_handler(key_state);
            }
        }
        else if(key_count == KEY_COUNT_2) {
            if(key_state == KEY_STATE_PRESSED_1) {
                key_state = KEY_STATE_PRESSED_2;
                suble_key_state_handler(key_state);
            }
        }
        else if(key_count == KEY_COUNT_3) {
            if(key_state == KEY_STATE_PRESSED_2) {
                //超时释放
                key_state = KEY_STATE_RELEASE;
            }
        }
    }
    else {
        if(key_count <= KEY_COUNT_1) {
            if(key_state == KEY_STATE_READY) {
                key_state = KEY_STATE_RELEASE_0;
            }
        }
        else if((key_count > KEY_COUNT_1) && (key_count <= KEY_COUNT_2)) {
            if(key_state == KEY_STATE_PRESSED_1) {
                key_state = KEY_STATE_RELEASE_1;
            }
        }
        else if((key_count > KEY_COUNT_2) && (key_count <= KEY_COUNT_3)) {
            if(key_state == KEY_STATE_PRESSED_2) {
                key_state = KEY_STATE_RELEASE_2;
            }
        }
    }
    
    //释放
    if(key_state >= KEY_STATE_RELEASE) {
        suble_key_state_handler(key_state);
        
        key_count = 0;
        key_pressed = true;
        key_state = KEY_STATE_READY;
        suble_timer_stop_0(SUBLE_TIMER100);
    }
}

void suble_key_clear_s_key_press_count(void)
{
    if(s_key_press_count == 1) {
        SUBLE_PRINTF("hit 1");
        
        tmp_key_handler(TUYA_BLE_MASTER_OPERATION_OPEN);
    }
    s_key_press_count = 0;
}













#include "app_port.h"

#pragma pack(1)
typedef struct
{
    uint8_t  open_meth;
    uint8_t  operation;
    uint8_t  hardid;
    uint8_t  open_meth_info_len;
    uint8_t  open_meth_info[64];
} open_record_info_t;
#pragma pack()


static open_record_info_t s_open_record_info;
static volatile bool s_master_scan_is_running = false;




/*********************************************************
FN: 
*/
void master_bonding_slave_handler(uint32_t evt, uint8_t* buf, uint32_t size)
{
    switch (evt)
    {
        case TUYA_BLE_MASTER_EVT_TIMEOUT: {
            SUBLE_PRINTF("TUYA_BLE_MASTER_EVT_TIMEOUT");
            s_master_scan_is_running = false;
        } break;
        
        case TUYA_BLE_MASTER_EVT_SLAVEID_INVALID: {
            SUBLE_PRINTF("TUYA_BLE_MASTER_EVT_SLAVEID_INVALID");
            s_master_scan_is_running = false;
        } break;
        
        case TUYA_BLE_MASTER_EVT_SCAN_TIMEOUT: {
            SUBLE_PRINTF("TUYA_BLE_MASTER_EVT_SCAN_TIMEOUT");
            s_master_scan_is_running = false;
            
            g_open_fail_count = 0;
        } break;
        
        case TUYA_BLE_MASTER_EVT_CONNECT_TIMEOUT: {
            SUBLE_PRINTF("TUYA_BLE_MASTER_EVT_CONNECT_TIMEOUT");
            s_master_scan_is_running = false;
        } break;
        
        case TUYA_BLE_MASTER_EVT_BONDING: {
            //开门
            tuya_ble_master_open_with_master(s_open_record_info.operation, 
                                                s_open_record_info.open_meth, 
                                                s_open_record_info.open_meth_info, 
                                                s_open_record_info.open_meth_info_len);
        } break;
        
        case TUYA_BLE_MASTER_EVT_OPEN_WITH_MASTER_SUCCESS: {
            //实际上会断开连接，此处上报只为存储开锁记录，待手机连接后再上报
            if(s_open_record_info.open_meth == OR_LOG_OPEN_WITH_KEY) {
                TUYA_APP_LOG_INFO("open with key success");
            }
            else if(s_open_record_info.open_meth == OR_LOG_OPEN_WITH_PW) {
                TUYA_APP_LOG_INFO("open with common password success");
            }
            else if(s_open_record_info.open_meth == OR_LOG_OPEN_WITH_TMP_PWD) {
                TUYA_APP_LOG_INFO("open with temp password success");
            }
            
            open_with_master_record_report_info_t* info = (void*)buf;
            lock_open_record_report(info->timestamp, s_open_record_info.open_meth, s_open_record_info.hardid, info->slaveid);
            
            suble_gap_disconnect(g_conn_info[1].condix, 0x13);
            
            g_open_fail_count = 0;
        } break;
        
        case TUYA_BLE_MASTER_EVT_OPEN_WITH_MASTER_FAILURE: {
            suble_gap_disconnect(g_conn_info[1].condix, 0x13);
            
            g_open_fail_count = 0;
        } break;
        
        case TUYA_BLE_MASTER_EVT_DISCONNECT: {
            SUBLE_PRINTF("TUYA_BLE_MASTER_EVT_DISCONNECT");
            s_master_scan_is_running = false;
        } break;
        
        default: {
        } break;
    }
}

/*********************************************************
FN: 
*/
static void tmp_key_handler(tuya_ble_master_operation_t operation)
{
    s_open_record_info.open_meth = OR_LOG_OPEN_WITH_KEY;
    s_open_record_info.hardid = 0x00;
    s_open_record_info.open_meth_info_len = 6;
    s_open_record_info.open_meth_info[0] = 0;
    s_open_record_info.open_meth_info[1] = 0;
    s_open_record_info.open_meth_info[2] = 0;
    s_open_record_info.open_meth_info[3] = 0;
    s_open_record_info.open_meth_info[4] = 0;
    s_open_record_info.open_meth_info[5] = 0;
    s_open_record_info.operation = operation;
    
    if(!s_master_scan_is_running) {
        s_master_scan_is_running = true;
        tuya_ble_master_scan_start(-1, master_bonding_slave_handler);
    }
    else {
        SUBLE_PRINTF("__________________s_master_scan_is_running");
    }
}

/*********************************************************
FN: 
*/
void suble_gpio_open_with_common_pwd(uint8_t hardid, uint16_t slaveid)
{
    s_open_record_info.open_meth = OR_LOG_OPEN_WITH_PW;
    s_open_record_info.hardid = hardid;
    s_open_record_info.open_meth_info_len = 6;
    s_open_record_info.open_meth_info[0] = 0;
    s_open_record_info.open_meth_info[1] = 0;
    s_open_record_info.open_meth_info[2] = 0;
    s_open_record_info.open_meth_info[3] = hardid;
    s_open_record_info.open_meth_info[4] = slaveid>>8;
    s_open_record_info.open_meth_info[5] = slaveid&0xFF;
    s_open_record_info.operation = TUYA_BLE_MASTER_OPERATION_OPEN;
    
    if(!s_master_scan_is_running) {
        s_master_scan_is_running = true;
        tuya_ble_master_scan_start(slaveid, master_bonding_slave_handler);
    }
    else {
        SUBLE_PRINTF("__________________s_master_scan_is_running");
    }
}

/*********************************************************
FN: 
*/
void suble_gpio_open_with_tmp_pwd(uint8_t hardid, uint16_t slaveid)
{
    s_open_record_info.open_meth = OR_LOG_OPEN_WITH_TMP_PWD;
    s_open_record_info.hardid = hardid;
    s_open_record_info.open_meth_info_len = 6;
    s_open_record_info.open_meth_info[0] = 0;
    s_open_record_info.open_meth_info[1] = 0;
    s_open_record_info.open_meth_info[2] = 0;
    s_open_record_info.open_meth_info[3] = hardid;
    s_open_record_info.open_meth_info[4] = slaveid>>8;
    s_open_record_info.open_meth_info[5] = slaveid&0xFF;
    s_open_record_info.operation = TUYA_BLE_MASTER_OPERATION_OPEN;
    
    if(!s_master_scan_is_running) {
        s_master_scan_is_running = true;
        tuya_ble_master_scan_start(slaveid, master_bonding_slave_handler);
    }
    else {
        SUBLE_PRINTF("__________________s_master_scan_is_running");
    }
}

void set_s_master_scan_is_running(void)
{
    s_master_scan_is_running = true;
}


















