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
static suble_timer_t s_suble_timer[SUBLE_TIMER_MAX-SUBLE_TIMER0] = {0};

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */
static void suble_timer_continue(ke_msg_id_t const timer_id);
static void suble_timer_app_handler(uint32_t timer_id);
static void suble_rtc_handler(void);




/*********************************************************  suble_timer  *********************************************************/

/*********************************************************
FN: 超时处理
*/
void suble_timer_handler(ke_msg_id_t timer_id)
{
    if((timer_id >= SUBLE_TIMER0) && (timer_id < SUBLE_TIMER100)) {
        suble_timer_app_handler(timer_id);
    }
    else {
        switch(timer_id)
        {
            case SUBLE_TIMER100: {
                suble_key_timeout_handler();
            } break;
            
            case SUBLE_TIMER101: {
                suble_key_clear_s_key_press_count();
            } break;
            
            case SUBLE_TIMER102: {
                suble_gpio_led_reverse(ANTILOCK_LED_RED_PIN);
            } break;
            
            case SUBLE_TIMER103: {
                suble_battery_get_value_outtime_handler();
            } break;
            
            case SUBLE_TIMER104: {
                suble_buzzer_timeout_handler();
            } break;
            
            case SUBLE_TIMER105: {
                suble_gpio_rled_blink(1, 100);
            } break;
            
            case SUBLE_TIMER106: {
            } break;
            
            case SUBLE_TIMER107: {
            } break;
            
            case SUBLE_TIMER108: {
            } break;
            
            case SUBLE_TIMER109: {
            } break;
            
            case SUBLE_TIMER200: {
                suble_rtc_handler();
            } break;
            
            default: {
            } break;
        }
    }
    suble_timer_continue(timer_id);
}

/*********************************************************
FN: 启动
*/
void suble_timer_start_0(ke_msg_id_t const timer_id, uint32_t ms, uint32_t count)
{
    if((timer_id < SUBLE_TIMER0) || (timer_id >= SUBLE_TIMER_MAX)) {
        return;
    }
    
    uint32_t idx = timer_id - SUBLE_TIMER0;
    s_suble_timer[idx].delay = ms/10;
    s_suble_timer[idx].count = count;
    ke_timer_set(timer_id, TASK_APPM, s_suble_timer[idx].delay);
}

/*********************************************************
FN: 继续
*/
static void suble_timer_continue(ke_msg_id_t const timer_id)
{
    if((timer_id < SUBLE_TIMER0) || (timer_id >= SUBLE_TIMER_MAX)) {
        return;
    }
    
    uint32_t idx = timer_id-SUBLE_TIMER0;
    //周期性
    if((s_suble_timer[idx].count != SUBLE_TIMER_COUNT_ENDLESS) && (s_suble_timer[idx].count > 0)) {
        s_suble_timer[idx].count--;
    }
    //继续
    if(s_suble_timer[idx].count > 0) {
        ke_timer_set(timer_id, TASK_APPM, s_suble_timer[idx].delay);
    }
}

/*********************************************************
FN: 停止
*/
void suble_timer_stop_0(ke_msg_id_t const timer_id)
{
    if((timer_id < SUBLE_TIMER0) || (timer_id >= SUBLE_TIMER_MAX)) {
        return;
    }
    
    uint32_t idx = timer_id-SUBLE_TIMER0;
    s_suble_timer[idx].count = 0;
    s_suble_timer[idx].delay = 0;
    ke_timer_clear(timer_id, TASK_APPM);
}

/*********************************************************
FN: 是否运行
*/
bool suble_timer_is_running(ke_msg_id_t const timer_id)
{
    if((timer_id < SUBLE_TIMER0) || (timer_id >= SUBLE_TIMER_MAX)) {
        return false;
    }
    
    uint32_t idx = timer_id-SUBLE_TIMER0;
    return (s_suble_timer[idx].count != 0);
}




/*********************************************************  suble_timer app  *********************************************************/

typedef struct {
    uint8_t is_occupy;
    ke_msg_id_t timer_id;
    uint32_t count;
    uint32_t ms;
    suble_timer_handler_t handler;
} suble_timer_item_t;

static suble_timer_item_t m_timer_pool[] = {
    [0]  = { .timer_id = SUBLE_TIMER0},
    [1]  = { .timer_id = SUBLE_TIMER1},
    [2]  = { .timer_id = SUBLE_TIMER2},
    [3]  = { .timer_id = SUBLE_TIMER3},
    [4]  = { .timer_id = SUBLE_TIMER4},
    [5]  = { .timer_id = SUBLE_TIMER5},
    [6]  = { .timer_id = SUBLE_TIMER6},
    [7]  = { .timer_id = SUBLE_TIMER7},
    [8]  = { .timer_id = SUBLE_TIMER8},
    [9]  = { .timer_id = SUBLE_TIMER9},
    [10] = { .timer_id = SUBLE_TIMER10},
    [11] = { .timer_id = SUBLE_TIMER11},
    [12] = { .timer_id = SUBLE_TIMER12},
    [13] = { .timer_id = SUBLE_TIMER13},
    [14] = { .timer_id = SUBLE_TIMER14},
    [15] = { .timer_id = SUBLE_TIMER15},
    [16] = { .timer_id = SUBLE_TIMER16},
    [17] = { .timer_id = SUBLE_TIMER17},
    [18] = { .timer_id = SUBLE_TIMER18},
    [19] = { .timer_id = SUBLE_TIMER19},
};

/*********************************************************
FN: 
*/
static suble_timer_item_t* acquire_timer(uint32_t ms, uint32_t count, suble_timer_handler_t handler)
{
    for(uint8_t i=0; i<SUBLE_TIMER_MAX_NUM; i++) {
        if (m_timer_pool[i].is_occupy == 0) {
            m_timer_pool[i].is_occupy = 1;
            m_timer_pool[i].count = count;
            m_timer_pool[i].ms = ms;
            m_timer_pool[i].handler = handler;
            return &m_timer_pool[i];
        }
    }
    return NULL;
}

/*********************************************************
FN: 
*/
static int32_t release_timer(void* timer_id)
{
    for(uint8_t i=0; i<SUBLE_TIMER_MAX_NUM; i++) {
        if (timer_id == &m_timer_pool[i]) {
            m_timer_pool[i].is_occupy = 0;
            return i;
        }
    }
    return -1;
}

/*********************************************************
FN: 
*/
static int32_t find_timer_ms(void* timer_id, uint32_t *ms)
{
    for(uint8_t i=0; i<SUBLE_TIMER_MAX_NUM; i++) {
        if (timer_id == &m_timer_pool[i]) {
            *ms = m_timer_pool[i].ms;
            return i;
        }
    }
    return -1;
}

/*********************************************************
FN: 
*/
static void suble_timer_app_handler(uint32_t timer_id)
{
    for(uint8_t i=0; i<SUBLE_TIMER_MAX_NUM; i++) {
        if (timer_id == m_timer_pool[i].timer_id) {
            m_timer_pool[i].handler(&m_timer_pool[i]);
            break;
        }
    }
}

/*********************************************************
FN: 
*/
uint32_t suble_timer_create(void** p_timer_id, uint32_t timeout_value_ms, suble_timer_mode_t mode, suble_timer_handler_t timeout_handler)
{
    suble_timer_item_t* timer_item = acquire_timer(timeout_value_ms, (mode==SUBLE_TIMER_SINGLE_SHOT ? 1 : SUBLE_TIMER_COUNT_ENDLESS), timeout_handler);
    if (timer_item == NULL) {
        return SUBLE_ERROR_COMMON;
    }
    
    *p_timer_id = timer_item;
    return SUBLE_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t suble_timer_delete(void* timer_id)
{
    suble_timer_item_t* timer_item = timer_id;
    int id = release_timer(timer_item);
    if (id == -1) {
        return SUBLE_ERROR_COMMON;
    }
    
    suble_timer_stop_0(timer_item->timer_id);
    return SUBLE_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t suble_timer_start(void* timer_id)
{
    uint32_t ms;
    suble_timer_item_t* timer_item = timer_id;
    
    if(find_timer_ms(timer_item, &ms) >= 0)
    {
        suble_timer_start_0(timer_item->timer_id, timer_item->ms, timer_item->count);
        return SUBLE_SUCCESS;
    }
    else {
        SUBLE_PRINTF("suble_timer_start: not find_timer_ms");
        return SUBLE_ERROR_COMMON;
    }
}

/*********************************************************
FN: 
*/
uint32_t suble_timer_stop(void* timer_id)
{
    suble_timer_item_t* timer_item = timer_id;
    suble_timer_stop_0(timer_item->timer_id);
    return SUBLE_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t suble_timer_restart(void* timer_id, uint32_t timeout_value_ms)
{
    uint32_t ms;
    suble_timer_item_t* timer_item = timer_id;
    
    if(find_timer_ms(timer_id, &ms) >= 0)
    {
        suble_timer_stop_0(timer_item->timer_id);
        timer_item->ms = timeout_value_ms;
        suble_timer_start_0(timer_item->timer_id, timer_item->ms, timer_item->count);
        return SUBLE_SUCCESS;
    }
    else {
        SUBLE_PRINTF("suble_timer_restart: not find_timer_ms");
        return SUBLE_ERROR_COMMON;
    }
}




/*********************************************************  RTC  *********************************************************/

static uint32_t s_local_timestamp = 0;
static uint32_t s_local_timestamp_when_update = 0;
static uint32_t s_app_timestamp_when_update = 0;

/*********************************************************
FN: 启动本地时间戳
*/
static void suble_rtc_handler(void)
{
    s_local_timestamp++;
}

/*********************************************************
FN: 启动本地时间戳
*/
static void suble_rtc_start(void)
{
//    rtc_alarm_init(0x01, NULL, 500, suble_rtc_handler);
    
    suble_timer_start_0(SUBLE_TIMER200, 1000, SUBLE_TIMER_COUNT_ENDLESS);
}

/*********************************************************
FN: 启动本地时间戳
*/
void suble_local_timer_start(void)
{
    suble_rtc_start();
}

/*********************************************************
FN: 更新时间戳
*/
void suble_update_timestamp(uint32_t app_timestamp)
{
    s_local_timestamp_when_update = s_local_timestamp;
    s_app_timestamp_when_update = app_timestamp;
}

/*********************************************************
FN: 获取更新时的app时间戳
*/
uint32_t suble_get_app_timestamp_when_update(void)
{
    return s_app_timestamp_when_update;
}

/*********************************************************
FN: 获取本地时间戳
*/
uint32_t suble_get_local_timestamp(void)
{
    return s_local_timestamp;
}

/*********************************************************
FN: 获取当前时间戳（如果没有更新过，即为本地时间戳）
*/
uint32_t suble_get_timestamp(void)
{
    return (s_app_timestamp_when_update + (s_local_timestamp - s_local_timestamp_when_update));
}

/*********************************************************
FN: 获取过去的时间戳（必须在更新时间戳之后使用，否则返回 old_local_timestamp）
*/
uint32_t suble_get_old_timestamp(uint32_t old_local_timestamp)
{
    return (suble_get_timestamp() - (s_local_timestamp - old_local_timestamp));
}




/*********************************************************  delay  *********************************************************/

/*********************************************************
FN: 
*/
void suble_delay_ms(uint32_t ms)
{
    Delay_ms(ms);
}

/*********************************************************
FN: 
*/
void suble_delay_us(uint32_t us)
{
    Delay_us(us);
}











