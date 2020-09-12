#include "suble_common.h"
#include "lock_dp_report.h"




/*********************************************************************
 * LOCAL CONSTANT
 */
#define BATTERY_SAMPLE_TIME 20

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */
static uint32_t s_sample_idx = 0;
static uint32_t s_adc_value[BATTERY_SAMPLE_TIME] = {0};
static uint32_t s_adc_value_sum = 0;

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/
void suble_battery_init(void)
{
    adc_init(0x08, 0);
}

/*********************************************************
FN: 
*/
void suble_battery_sample_start(void)
{
    s_sample_idx = 0;
    memset(s_adc_value, 0x00, BATTERY_SAMPLE_TIME);
    s_adc_value_sum = 0;
    
    suble_timer_start_0(SUBLE_TIMER103, 97, SUBLE_TIMER_COUNT_ENDLESS);
    SUBLE_PRINTF("suble_battery_sample_start");
}

/*********************************************************
FN: 
*/
void suble_battery_get_value_outtime_handler(void)
{
    if(s_sample_idx < BATTERY_SAMPLE_TIME) {
        s_adc_value[s_sample_idx] = adc_get_value();
    }
    else {
        //排序
        suble_util_shell_sort((void*)s_adc_value, BATTERY_SAMPLE_TIME);
        //求和
        for(uint8_t idx=5; idx<=14; idx++) {
            s_adc_value_sum += s_adc_value[idx];
        }
        //求值
        uint32_t adc_value = s_adc_value_sum/10;
        
        //计算百分比
        int percent_up = adc_value - 528;
        int percent_down = 631 - 528;
        int percent = percent_up*100/percent_down;
        SUBLE_PRINTF("battery_percent: %d", percent);
        //限制百分比的合理范围
        if(percent > 100) {
            percent = 100;
        }
        if(percent <= 1) {
            percent = 1;
        }
        
        //上报
        SUBLE_PRINTF("battery_percent_report: %d", percent);
        lock_state_sync_report(OR_STS_BATTERY_PERCENT, percent);
        
        suble_timer_stop_0(SUBLE_TIMER103);
    }
    s_sample_idx++;
/*
2.5 - 528
2.6 - 549
2.7 - 570
2.8 - 590
2.9 - 611
3.0 - 631
3.1 - 651
3.2 - 671
3.3 - 694
*/
}






