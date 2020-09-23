/**
****************************************************************************
* @file      suble_common.h
* @brief     suble_common
* @author    suding
* @version   V1.0.0
* @date      2020-04
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2020 Tuya </center></h2>
*/


#ifndef __SUBLE_COMMON_H__
#define __SUBLE_COMMON_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDE
 */
//c lib
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
//stack
#include "gapc_task.h" // GAP Controller Task API
#include "gapm_task.h" // GAP Manager Task API
#include "ke_timer.h"
#include "co_utils.h"
#include "appm.h"
#include "appc.h"
#include "appm_task.h"
#include "appc_task.h"
#include "app_fff0.h"
#include "slave_app.h"
#include "master_app.h"
//peripheral
#include "gpio.h"
#include "uart.h"
#include "flash.h"
#include "pwm.h"
#include "adc.h"
#include "rtc.h"
#include "rf.h"

//cpt
#include "elog.h"
#include "sf_port.h"
//tuya_ble_sdk
#include "tuya_ble_log.h"
#include "tuya_ble_api.h"
#include "tuya_ble_app_demo.h"




/*********************************************************************
 * CONSTANT
 */
/* suble_common
 **************************************************/
#define SUBLE_DEBUG_EN 1

#if (SUBLE_DEBUG_EN)
    #define SUBLE_PRINTF(...)                   TUYA_APP_LOG_INFO(__VA_ARGS__)
    #define SUBLE_HEXDUMP(...)                  TUYA_APP_LOG_HEXDUMP_INFO(__VA_ARGS__)
#else
    #define SUBLE_PRINTF(...)
    #define SUBLE_HEXDUMP(...)
#endif

typedef enum {
    SUBLE_SUCCESS = 0x00,
    SUBLE_ERROR_COMMON,
} suble_status_t;

/* suble_adv_scan
 **************************************************/
#define  SUBLE_ADV_DATA_MAX_LEN                (31)

#define  SUBLE_ADV_INTERVAL_MIN                (TUYA_ADV_INTERVAL)
#define  SUBLE_ADV_INTERVAL_MAX                (TUYA_ADV_INTERVAL)

typedef enum {
    SUBLE_SCAN_EVT_ADV_REPORT = 0x00,
    SUBLE_SCAN_EVT_SCAN_TIMEOUT,
} suble_scan_result_t;

/* suble_gap
 **************************************************/
#define SUBLE_BT_MAC_LEN                       (BD_ADDR_LEN)
#define SUBLE_BT_MAC_STR_LEN                   (SUBLE_BT_MAC_LEN*2)

#define SUBLE_CONN_INTERVAL_MIN                (TUYA_CONN_INTERVAL_MIN) //最小可接受的连接间隔
#define SUBLE_CONN_INTERVAL_MAX                (TUYA_CONN_INTERVAL_MAX)
#define SUBLE_SLAVE_LATENCY                    (0)
#define SUBLE_CONN_SUP_TIMEOUT                 (5000)

typedef enum {
    SUBLE_GAP_EVT_CONNECTED = 0x00,
    SUBLE_GAP_EVT_DISCONNECTED,
    SUBLE_GAP_EVT_CONNECT_TIMEOUT,
} suble_gap_result_t;

/* suble_svc
 **************************************************/
typedef enum {
    SUBLE_SVC_C_EVT_DISCOVERY_COMPLETE = 0x00,
    SUBLE_SVC_C_EVT_RECEIVE_DATA_FROM_SLAVE,
} suble_svc_result_t;

/* suble_gpio
 **************************************************/
#define ANTILOCK_LED_RED_PIN                    0x12
#define ANTILOCK_BUTTON_PIN                     0x10
#define ANTILOCK_BUZZER_EN1                     0x31
#define ANTILOCK_BUZZER_EN2                     0x32
#define ANTILOCK_BUZZER_DIN                     0x11

enum
{
    SUBLE_LEVEL_INVALID = 0xFF,
    SUBLE_LEVEL_LOW  = 0,
    SUBLE_LEVEL_HIGH = 1,
};

/* suble_uart
 **************************************************/

/* suble_flash
 **************************************************/
#define SUBLE_FLASH_START_ADDR                 0x44000
#define SUBLE_FLASH_END_ADDR                   0x64000

//ota
#define SUBLE_FLASH_OTA_START_ADDR             SUBLE_FLASH_START_ADDR
#define SUBLE_FLASH_OTA_END_ADDR               0x64000
//mac
#define SUBLE_FLASH_BT_MAC_ADDR                0x7F000

/* suble_timer
 **************************************************/
#define SUBLE_TIMER_MAX_NUM                    20

#define SUBLE_TIMER_COUNT_ENDLESS              0xFFFFFFFF

typedef enum {
    SUBLE_TIMER_SINGLE_SHOT,
    SUBLE_TIMER_REPEATED,
} suble_timer_mode_t;

/* suble_key
 **************************************************/

/* suble_battery
 **************************************************/

/* suble_music
 **************************************************/
#define LOCK_MUSIC_MAX_NUM 6

typedef enum
{
    MUSIC_FREEDOM_1 = 0,
    MUSIC_FREEDOM_2,
    MUSIC_FREEDOM_3,
    MUSIC_FREEDOM_4,
    MUSIC_FREEDOM_5,
    MUSIC_FREEDOM_6,
    MUSIC_DI_1,
    MUSIC_DI_2,
    MUSIC_LONG_DI,
    MUSIC_NOTIFY_1,
    MUSIC_NOTIFY_2,
} music_enum_t;

typedef enum
{
    MUSIC_MODE_ONCE = 0,
    MUSIC_MODE_REPEAT,
} music_mode_t;

/* suble_util
 **************************************************/

/* suble_test
 **************************************************/




/*********************************************************************
 * STRUCT
 */
#pragma pack(1)
/* suble_common
 **************************************************/

/* suble_adv_scan
 **************************************************/
typedef struct
{
    double  adv_interval_min; //ms
    double  adv_interval_max; //ms
    uint8_t adv_type;
    uint8_t adv_power;
    uint8_t adv_channal_map;
} adv_param_t;

typedef struct
{
    uint32_t len;
    uint8_t  value[SUBLE_ADV_DATA_MAX_LEN];
} adv_data_t;

typedef void (*suble_scan_result_handler_t)(uint32_t evt, uint8_t* buf, uint32_t size);

/* suble_gap
 **************************************************/
typedef struct
{
    uint16_t condix;
    uint8_t  role;
    struct gap_bdaddr mac;
} conn_info_t;

typedef void (*suble_connect_result_handler_t)(uint32_t evt, uint8_t* buf, uint32_t size);

/* suble_svc
 **************************************************/
typedef void (*suble_svc_result_handler_t)(uint32_t evt, uint8_t* buf, uint32_t size);

/* suble_gpio
 **************************************************/
typedef struct
{
    uint8_t pin;
    uint8_t level;
} suble_out_pin_t;

/* suble_uart
 **************************************************/

/* suble_flash
 **************************************************/

/* suble_timer
 **************************************************/
typedef struct
{
    uint32_t count;
    uint32_t delay;
} suble_timer_t;

typedef void (*suble_timer_handler_t)(void*);

/* suble_key
 **************************************************/

/* suble_battery
 **************************************************/

/* suble_music
 **************************************************/

/* suble_util
 **************************************************/

/* suble_test
 **************************************************/

#pragma pack()




/*********************************************************************
 * EXTERNAL VARIABLE
 */
/* suble_common
 **************************************************/
extern volatile bool g_system_sleep;

/* suble_adv_scan
 **************************************************/
extern adv_data_t      g_adv_data;
extern adv_data_t      g_scan_rsp;
extern adv_param_t     g_adv_param;

//更新广播参数
extern volatile bool g_adv_restart_glag;

/* suble_gap
 **************************************************/
extern conn_info_t     g_conn_info[];

/* suble_svc
 **************************************************/

/* suble_gpio
 **************************************************/

/* suble_uart
 **************************************************/

/* suble_flash
 **************************************************/

/* suble_timer
 **************************************************/

/* suble_key
 **************************************************/

/* suble_battery
 **************************************************/

/* suble_music
 **************************************************/

/* suble_util
 **************************************************/

/* suble_test
 **************************************************/





/*********************************************************************
 * EXTERNAL FUNCTION
 */
/* suble_common
 **************************************************/
void suble_init_func(uint8_t location);
void suble_mainloop(void);
void suble_system_reset(void);
void suble_enter_critical(void);
void suble_exit_critical(void);

void suble_log_init(void);
void suble_log_hexdump(const char *name, uint8_t *buf, uint16_t size);
void suble_log_hexdump_for_tuya_ble_sdk(const char *name, uint8_t width, uint8_t *buf, uint16_t size);
void suble_log_hexdump_empty(const char *name, uint8_t width, uint8_t *buf, uint16_t size);

/* suble_adv_scan
 **************************************************/
void suble_adv_start(void);
void suble_adv_stop(void);
void suble_adv_update_advDataAndScanRsp(void);
void suble_adv_param_set(void);

void suble_scan_evt_handler(struct adv_report const *p_param);
void suble_scan_timeout_handler(void);
void suble_scan_init(void);
void suble_scan_start(suble_scan_result_handler_t handler);
void suble_scan_stop(void);
void suble_scan_param_set(void);

/* suble_gap
 **************************************************/
uint32_t suble_gap_connect(struct gap_bdaddr bdaddr, suble_connect_result_handler_t handler);
void suble_gap_disconnect(uint16_t condix, uint8_t hci_status_code);
void suble_gap_disconnect_for_tuya_ble_sdk(void);
void suble_gap_conn_param_update(uint16_t condix, uint16_t cMin, uint16_t cMax, uint16_t latency, uint16_t timeout);
void suble_gap_init_bt_mac(void);
void suble_gap_set_bt_mac(uint8_t *pMac);
void suble_gap_get_bt_mac(uint8_t *pMac, uint32_t size);

void suble_gap_conn_handler(void);
void suble_gap_disconn_handler(void);
void suble_gap_master_conn_handler(void);
void suble_gap_master_disconn_handler(void);
void suble_gap_master_connect_timeout_handler(void);

/* suble_svc
 **************************************************/
void suble_svc_init(void);
void suble_svc_receive_data(uint8_t* buf, uint32_t size);
void suble_svc_send_data_complete(void);
void suble_svc_notify(uint8_t* buf, uint32_t size);
void suble_svc_notify_handler(void);

void suble_svc_c_init(void);
void suble_svc_c_handle_assign(uint16_t conn_handle);
void suble_db_discovery_init(void);
void suble_db_discovery_start(suble_svc_result_handler_t handler);
uint32_t suble_svc_c_send_data(uint16_t condix, uint8_t* pBuf, uint16_t len);
void suble_svc_c_discovery_complete(void);
void suble_svc_c_receive_data_from_slave(void* buf, uint32_t size);

/* suble_gpio
 **************************************************/
void suble_gpio_init_output(uint8_t pin);
void suble_gpio_init_input(uint8_t pin, uint8_t pull_type);
uint8_t suble_gpio_get_input(uint8_t pin);
uint8_t suble_gpio_get_output(uint8_t pin);
void suble_gpio_set(uint8_t pin, uint8_t level);
void suble_gpio_reverse(uint8_t pin);

void suble_gpio_irq_init(void);

void suble_buzzer_start(uint32_t freq);
void suble_buzzer_stop(void);
void suble_buzzer_set_sound_volume(uint8_t sound_volume);

void    suble_gpio_init(void);

void suble_gpio_led_on(uint8_t pin);
void suble_gpio_led_off(uint8_t pin);
void suble_gpio_led_reverse(uint8_t pin);
void suble_gpio_rled_blink(uint32_t count, uint32_t ms);
void suble_gpio_rled_blink_cancel(void);
void suble_gpio_rled_period_blink(uint32_t count, uint32_t ms);
void suble_gpio_rled_period_blink_cancel(void);

/* suble_uart
 **************************************************/
void suble_uart1_init(void);
void suble_uart2_init(void);
void suble_uart1_send(const uint8_t* buf, uint32_t size);
void suble_uart2_send(const uint8_t* buf, uint32_t size);

/* suble_flash
 **************************************************/
void suble_flash_init(void);
void suble_flash_read(uint32_t addr, uint8_t *buf, uint32_t size);
void suble_flash_write(uint32_t addr, uint8_t *buf, uint32_t size);
void suble_flash_erase(uint32_t addr, uint32_t num);

/* suble_timer
 **************************************************/
void suble_timer_handler(ke_msg_id_t timer_id);
void suble_timer_start_0(ke_msg_id_t const timer_id, uint32_t ms, uint32_t count);
void suble_timer_stop_0(ke_msg_id_t const timer_id);
bool suble_timer_is_running(ke_msg_id_t const timer_id);

uint32_t suble_timer_create(void** p_timer_id, uint32_t timeout_value_ms, suble_timer_mode_t mode, suble_timer_handler_t timeout_handler);
uint32_t suble_timer_delete(void* timer_id);
uint32_t suble_timer_start(void* timer_id);
uint32_t suble_timer_stop(void* timer_id);
uint32_t suble_timer_restart(void* timer_id, uint32_t timeout_value_ms);

void suble_local_timer_start(void);
void suble_update_timestamp(uint32_t app_timestamp);
uint32_t suble_get_app_timestamp_when_update(void);
uint32_t suble_get_local_timestamp(void);
uint32_t suble_get_timestamp(void);
uint32_t suble_get_old_timestamp(uint32_t old_local_timestamp);

void suble_delay_ms(uint32_t ms);
void suble_delay_us(uint32_t us);

/* suble_key
 **************************************************/
void suble_key_timeout_handler(void);
void suble_key_clear_s_key_press_count(void);

void suble_gpio_open_with_common_pwd(uint8_t hardid, uint16_t slaveid);
void suble_gpio_open_with_tmp_pwd(uint8_t hardid, uint16_t slaveid);
void set_s_master_scan_is_running(void);

/* suble_battery
 **************************************************/
void suble_battery_init(void);
void suble_battery_sample_start(void);
void suble_battery_get_value_outtime_handler(void);

/* suble_music
 **************************************************/
void suble_buzzer_timeout_handler(void);
void lock_play_music(uint8_t mode, uint32_t music_idx);
void lock_play_music_cancel(void);
uint32_t lock_music_maxnum(uint32_t music_idx);

/* suble_util
 **************************************************/
uint8_t  suble_util_check_sum8(uint8_t *buf, uint32_t size);
uint16_t suble_util_check_sum16(uint8_t *buf, uint32_t size);
uint16_t suble_util_crc16(uint8_t* buf, uint32_t size, uint16_t* p_crc);
uint32_t suble_util_crc32(uint8_t* buf, uint32_t size, uint32_t* p_crc);
void     suble_util_reverse_byte(void* buf, uint32_t size);
uint32_t suble_util_numarray2int(uint8_t *num_array, uint32_t start_idx, uint32_t size);
uint32_t suble_util_shell_sort(int* buf, int size);

uint8_t  suble_util_str_hexchar2int(uint8_t hexchar);
uint8_t  suble_util_str_int2hexchar(uint8_t int_num);
uint32_t suble_util_str_hexstr2int(uint8_t* hexstr, uint32_t size, int* sum);
uint32_t suble_util_str_intstr2int(uint8_t* intstr, uint32_t size, int* sum);
uint32_t suble_util_str_hexstr2hexarray(uint8_t* hexstr, uint32_t size, uint8_t* hexarray);
uint32_t suble_util_str_hexarray2hexstr(uint8_t* hexarray, uint32_t size, uint8_t* hexstr);

int source_to_object(int source_notation, uint8_t source_array[], uint32_t source_array_len, int object_notation, uint8_t object_array[]);

/* suble_test
 **************************************************/
void suble_test_func(void);


#ifdef __cplusplus
}
#endif

#endif //__SUBLE_COMMON_H__
