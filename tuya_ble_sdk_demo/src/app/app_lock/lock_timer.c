#include "lock_timer.h"




/*********************************************************************
 * LOCAL CONSTANTS
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static void* lock_timer[LOCK_TUMER_MAX];

/*********************************************************************
 * LOCAL FUNCTION
 */

/*********************************************************************
 * VARIABLES
 */
int16_t g_timezone = 0;




/*********************************************************
FN: 
*/
void conn_param_update_outtime_cb_handler(void)
{
    app_port_conn_param_update(SUBLE_CONN_INTERVAL_MIN, SUBLE_CONN_INTERVAL_MAX, SUBLE_SLAVE_LATENCY, SUBLE_CONN_SUP_TIMEOUT);
}
static void conn_param_update_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_0);
}

/*********************************************************
FN: 
*/
void delay_report_outtime_cb_handler(void)
{
    delay_report_outtime_handler();
}
static void delay_report_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_1);
}

/*********************************************************
FN: 
*/
void bonding_conn_outtime_cb_handler(void)
{
    lock_offline_evt_report(0xFF);
    suble_battery_sample_start();
}
static void bonding_conn_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_2);
}

/*********************************************************
FN: 
*/
void reset_with_disconn_outtime_cb_handler(void)
{
    app_port_ble_gap_disconnect();
    lock_timer_start(LOCK_TIMER_RESET_WITH_DISCONN2);
}
static void reset_with_disconn_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_3);
}

/*********************************************************
FN: 
*/
void app_test_outtime_cb_handler(void)
{
    app_test_outtime_handler();
}
static void app_test_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_5);
}

/*********************************************************
FN: 
*/
void app_test_reset_outtime_cb_handler(void)
{
    app_port_device_reset();
}
static void app_test_reset_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_6);
}

/*********************************************************
FN: 
*/
void app_active_report_outtime_cb_handler(void)
{
    app_active_report_stop(ACTICE_REPORT_STOP_STATE_OUTTIME);
}
static void app_active_report_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_7);
}

/*********************************************************
FN: 
*/
void reset_with_disconn2_outtime_cb_handler(void)
{
    app_port_device_reset();
}
static void reset_with_disconn2_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_8);
}

/*********************************************************
FN: 
*/
void communication_monitor_outtime_cb_handler(void)
{
    TUYA_APP_LOG_INFO("ble disconncet because communication monitor timer timeout.");
    app_port_ble_gap_disconnect();
}
static void communication_monitor_outtime_cb(tuya_ble_timer_t timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_9);
}

/*********************************************************
FN: 
*/
uint32_t lock_timer_creat(void)
{
    uint32_t ret = 0;
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_CONN_PARAM_UPDATE], 1000, SUBLE_TIMER_SINGLE_SHOT, conn_param_update_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_DELAY_REPORT], 200, SUBLE_TIMER_SINGLE_SHOT, delay_report_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_BONDING_CONN], 1000, SUBLE_TIMER_SINGLE_SHOT, bonding_conn_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_RESET_WITH_DISCONN], 1000, SUBLE_TIMER_SINGLE_SHOT, reset_with_disconn_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_APP_TEST_OUTTIME], APP_TEST_MODE_ENTER_OUTTIME_MS, SUBLE_TIMER_SINGLE_SHOT, app_test_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_APP_TEST_RESET_OUTTIME], 500, SUBLE_TIMER_SINGLE_SHOT, app_test_reset_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_ACTIVE_REPORT], 30000, SUBLE_TIMER_SINGLE_SHOT, app_active_report_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_RESET_WITH_DISCONN2], 1000, SUBLE_TIMER_SINGLE_SHOT, reset_with_disconn2_outtime_cb);
	ret += app_port_timer_create(&lock_timer[LOCK_TIMER_COMMUNICATION_MONITOR], 120000, TUYA_BLE_TIMER_SINGLE_SHOT, communication_monitor_outtime_cb);
    //tuya_ble_xtimer_connect_monitor
    return ret;
}

/*********************************************************
FN: 
*/
uint32_t lock_timer_delete(lock_timer_t p_timer)
{
    if(p_timer >= LOCK_TUMER_MAX)
    {
        return APP_PORT_ERROR_COMMON;
    }
    
    return app_port_timer_delete(lock_timer[p_timer]);
}

/*********************************************************
FN: 
*/
uint32_t lock_timer_start(lock_timer_t p_timer)
{
    if(p_timer >= LOCK_TUMER_MAX)
    {
        return APP_PORT_ERROR_COMMON;
    }
    
    return app_port_timer_start(lock_timer[p_timer]);
}

/*********************************************************
FN: 
*/
uint32_t lock_timer_stop(lock_timer_t p_timer)
{
    if(p_timer >= LOCK_TUMER_MAX)
    {
        return APP_PORT_ERROR_COMMON;
    }
    
    return app_port_timer_stop(lock_timer[p_timer]);
}




#pragma pack(1)
typedef struct
{
	uint32_t total_start;
	uint32_t total_end;
	uint8_t  cycle_type;
	uint32_t cycle_flag;
	uint8_t  day_start_h;
	uint8_t  day_start_m;
	uint8_t  day_end_h;
	uint8_t  day_end_m;
} time_parser_t;
#pragma pack()

/*********************************************************
FN: 
*/
uint32_t lock_timer_time_is_valid(void* time, uint32_t current_timestamp)
{
    uint32_t ret = 0;
    
    time_parser_t* p_time = time;
    suble_util_reverse_byte(&p_time->total_start, sizeof(uint32_t));
    suble_util_reverse_byte(&p_time->total_end, sizeof(uint32_t));
    suble_util_reverse_byte(&p_time->cycle_flag, sizeof(uint32_t));
    
    SUBLE_PRINTF("total_start: %d", p_time->total_start);
    SUBLE_PRINTF("total_end:   %d", p_time->total_end);
    SUBLE_PRINTF("cycle_type:  %d", p_time->cycle_type);
    SUBLE_PRINTF("cycle_flag:  %04x", p_time->cycle_flag);
    SUBLE_PRINTF("day_start_h: %d", p_time->day_start_h);
    SUBLE_PRINTF("day_start_m: %d", p_time->day_start_m);
    SUBLE_PRINTF("day_end_h:   %d", p_time->day_end_h);
    SUBLE_PRINTF("day_end_m:   %d", p_time->day_end_m);
    
    if(current_timestamp >= p_time->total_start && current_timestamp <= p_time->total_end) {
            
        tuya_ble_time_struct_data_t current_time;
        current_timestamp += (g_timezone*36); //g_timezone*3600/100
        tuya_ble_utc_sec_2_mytime(current_timestamp, &current_time, 0);
        SUBLE_PRINTF("current_time.nYear:    %d", current_time.nYear);
        SUBLE_PRINTF("current_time.nMonth:   %d", current_time.nMonth);
        SUBLE_PRINTF("current_time.nDay:     %d", current_time.nDay);
        SUBLE_PRINTF("current_time.nHour:    %d", current_time.nHour);
        SUBLE_PRINTF("current_time.nMin:     %d", current_time.nMin);
        SUBLE_PRINTF("current_time.nSec:     %d", current_time.nSec);
        SUBLE_PRINTF("current_time.DayIndex: %d", current_time.DayIndex);
        
        switch(p_time->cycle_type)
        {
            case 0x00: {
                ret = 1;
            } break;
            
            //天循环
            case 0x01: {
                if(((current_time.nHour >= p_time->day_start_h) && (current_time.nHour <= p_time->day_end_h))
                    &&((current_time.nMin >= p_time->day_start_m) && (current_time.nMin <= p_time->day_end_m)))
                {
                    ret = 1;
                }
            } break;
            
            //周循环
            case 0x02: {
                if(((1<<current_time.DayIndex) & (p_time->cycle_flag)) != 0) {
                    if(((current_time.nHour >= p_time->day_start_h) && (current_time.nHour <= p_time->day_end_h))
                        &&((current_time.nMin >= p_time->day_start_m) && (current_time.nMin <= p_time->day_end_m)))
                    {
                        ret = 1;
                    }
                }
            } break;
             
            //月循环
            case 0x03: {
                if(((1<<(current_time.nDay-1)) & (p_time->cycle_flag)) != 0) {
                    if(((current_time.nHour >= p_time->day_start_h) && (current_time.nHour <= p_time->day_end_h))
                        &&((current_time.nMin >= p_time->day_start_m) && (current_time.nMin <= p_time->day_end_m)))
                    {
                        ret = 1;
                    }
                }
            } break;
            
            default: {
            } break;
        }
    }
    
    SUBLE_PRINTF("ret: %d", ret);
    
    return ret;
}


























