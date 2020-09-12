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
static uint8_t s_suble_gpio_irq_array[] = 
{
    ANTILOCK_BUTTON_PIN,
};

/*********************************************************************
 * VARIABLE
 */
suble_out_pin_t suble_out_pin_array[] = 
{
    {ANTILOCK_LED_RED_PIN, SUBLE_LEVEL_HIGH},
    {ANTILOCK_BUZZER_EN1, SUBLE_LEVEL_LOW},
    {ANTILOCK_BUZZER_EN2, SUBLE_LEVEL_HIGH},
    {ANTILOCK_BUZZER_DIN, SUBLE_LEVEL_LOW},
};
static uint8_t suble_out_pin_num = sizeof(suble_out_pin_array)/sizeof(suble_out_pin_array[0]);

/*********************************************************************
 * LOCAL FUNCTION
 */
static void suble_gpio_int_config(uint8_t* gpio_array, uint8_t len);
static void suble_gpio_irq_handler(uint32_t pin);
static uint32_t suble_gpio_irq_pin_change_format(uint32_t pin);




/*********************************************************
FN: 输出引脚初始化
*/
void suble_gpio_init_output(uint8_t pin)
{
    gpio_config(pin, OUTPUT, PULL_NONE);
    
    for(uint8_t idx=0; idx<suble_out_pin_num; idx++) {
        if(suble_out_pin_array[idx].pin == pin) {
            gpio_set(pin, suble_out_pin_array[idx].level);
        }
    }
}

/*********************************************************
FN: 输入引脚初始化
*/
void suble_gpio_init_input(uint8_t pin, uint8_t pull_type)
{
    gpio_config(pin, INPUT, (Pull_Type)pull_type);
}

/*********************************************************
FN: 获取输出引脚电平
*/
uint8_t suble_gpio_get_output(uint8_t pin)
{
    for(uint8_t idx=0; idx<suble_out_pin_num; idx++) {
        if(suble_out_pin_array[idx].pin == pin) {
            return suble_out_pin_array[idx].level;
        }
    }
    return SUBLE_LEVEL_INVALID;
}

/*********************************************************
FN: 获取输入引脚电平
*/
uint8_t suble_gpio_get_input(uint8_t pin)
{
    return gpio_get_input(pin);
}

/*********************************************************
FN: 输出引脚设置电平
*/
void suble_gpio_set(uint8_t pin, uint8_t level)
{
    gpio_set(pin, level);
    
    for(uint8_t idx=0; idx<suble_out_pin_num; idx++) {
        if(suble_out_pin_array[idx].pin == pin) {
            suble_out_pin_array[idx].level = level;
        }
    }
}

/*********************************************************
FN: 输出引脚电平翻转
*/
void suble_gpio_reverse(uint8_t pin)
{   
    uint8_t level = suble_gpio_get_output(pin);
    suble_gpio_set(pin, !level);
}




/*********************************************************
FN: 外部中断
*/
static void suble_gpio_int_config(uint8_t* gpio_array, uint8_t len)
{
    for(uint8_t i=0; i<len; i++)
    {
        { //下降沿触发
            suble_gpio_init_input(gpio_array[i], PULL_HIGH);
            REG_APB5_GPIO_WUATOD_TYPE |= 1<<(8*(gpio_array[i]>>4)+(gpio_array[i]&0x0f)); //1<<2 = 4
        }
        REG_APB5_GPIO_WUATOD_STAT |= 1<<(8*(gpio_array[i]>>4)+(gpio_array[i]&0x0f));
        suble_delay_ms(2);
        REG_APB5_GPIO_WUATOD_ENABLE |= 1<<(8*(gpio_array[i]>>4)+(gpio_array[i]&0x0f));
        REG_AHB0_ICU_DEEP_SLEEP0 |= 1<<(8*(gpio_array[i]>>4)+(gpio_array[i]&0x0f));
    }
    REG_AHB0_ICU_INT_ENABLE |= (0x01 << 9);
}

/*********************************************************
FN: 外部中断初始化
*/
void suble_gpio_irq_init(void)
{
    suble_gpio_int_config(s_suble_gpio_irq_array, sizeof(s_suble_gpio_irq_array));
    gpio_cb_register(suble_gpio_irq_handler);
}

/*********************************************************
FN: 外部中断处理
*/
static void suble_gpio_irq_handler(uint32_t pin)
{
    //引脚序号是8进制
    pin = suble_gpio_irq_pin_change_format(pin);
    switch(pin)
    {
        case ANTILOCK_BUTTON_PIN: {
//            if(suble_gpio_get_input(pin) == SUBLE_LEVEL_LOW)
            {
//                SUBLE_PRINTF("ANTILOCK_BUTTON_PIN");
                suble_timer_start_0(SUBLE_TIMER100, 10, SUBLE_TIMER_COUNT_ENDLESS);
            }
        } break;
        
        default: {
        } break;
    }
    suble_gpio_int_config(s_suble_gpio_irq_array, sizeof(s_suble_gpio_irq_array));
}

/*********************************************************
FN: 外部中断引脚转换
*/
static uint32_t suble_gpio_irq_pin_change_format(uint32_t pin)
{
    uint32_t zero_count = 0;
    for(int idx=0; idx<32; idx++) {
        pin = pin>>1;
        if(pin == 0) {
            break;
        }
        zero_count++;
    }
    return (((zero_count/8)*0x10) + (zero_count%8));
}

/*********************************************************
FN: 蜂鸣器
*/
void suble_buzzer_start(uint32_t freq)
{
#define SUBLE_BUZZER_CLOCK 16000000
    
    if(freq < SUBLE_BUZZER_CLOCK/65535) {
        SUBLE_PRINTF("freq not support");
        return;
    }
    
	PWM_DRV_DESC pwm;
	pwm.channel = 1;
	pwm.mode = 0x10; //看注释
//	pwm.pre_divid = 1; //没用到
	pwm.end_value = SUBLE_BUZZER_CLOCK/freq; //定时时间
	pwm.duty_cycle = SUBLE_BUZZER_CLOCK/freq/2;
	pwm_init(&pwm);
	
	pwm_enable(1);
}

/*********************************************************
FN: 
*/
void suble_buzzer_stop(void)
{
	pwm_disable(1);
}

void suble_buzzer_set_mode(uint8_t level1, uint8_t level2)
{
    suble_gpio_set(ANTILOCK_BUZZER_EN1, level1);
    suble_gpio_set(ANTILOCK_BUZZER_EN2, level2);
}

/*********************************************************
FN: 设置蜂鸣器音量
*/
void suble_buzzer_set_sound_volume(uint8_t sound_volume)
{
    switch(sound_volume)
    {
        case 0: {
            suble_buzzer_set_mode(SUBLE_LEVEL_LOW, SUBLE_LEVEL_LOW);
            lock_play_music_cancel();
        } break;
        
        case 1: {
            suble_buzzer_set_mode(SUBLE_LEVEL_LOW, SUBLE_LEVEL_HIGH);
//            lock_play_music(MUSIC_MODE_REPEAT, lock_settings.sound_type);
        } break;
        
        case 2: {
            suble_buzzer_set_mode(SUBLE_LEVEL_HIGH, SUBLE_LEVEL_LOW);
//            lock_play_music(MUSIC_MODE_REPEAT, lock_settings.sound_type);
        } break;
        
        case 3: {
            suble_buzzer_set_mode(SUBLE_LEVEL_HIGH, SUBLE_LEVEL_HIGH);
//            lock_play_music(MUSIC_MODE_REPEAT, lock_settings.sound_type);
        } break;
        
        default: {
        } break;
    }
}




/*********************************************************
FN: 初始化引脚
*/
void suble_gpio_init(void)
{
    //output
    for(uint32_t idx=0; idx<suble_out_pin_num; idx++) {
        suble_gpio_init_output(suble_out_pin_array[idx].pin);
    }
    
    //input
    suble_gpio_init_input(ANTILOCK_BUTTON_PIN, PULL_NONE);
    
    //interrupt
    suble_gpio_irq_init();
}




/*********************************************************
FN: 
*/
void suble_gpio_led_on(uint8_t pin)
{
    suble_gpio_set(pin, SUBLE_LEVEL_LOW);
}

/*********************************************************
FN: 
*/
void suble_gpio_led_off(uint8_t pin)
{
    suble_gpio_set(pin, SUBLE_LEVEL_HIGH);
}

/*********************************************************
FN: 
*/
void suble_gpio_led_reverse(uint8_t pin)
{
    suble_gpio_reverse(pin);
}

/*********************************************************
FN: 
*/
void suble_gpio_rled_blink(uint32_t count, uint32_t ms)
{
    suble_gpio_led_off(ANTILOCK_LED_RED_PIN);
    suble_timer_start_0(SUBLE_TIMER102, ms, count*2);
}

/*********************************************************
FN: 
*/
void suble_gpio_rled_blink_cancel(void)
{
    suble_gpio_led_off(ANTILOCK_LED_RED_PIN);
    suble_timer_stop_0(SUBLE_TIMER102);
}

/*********************************************************
FN: 
*/
void suble_gpio_rled_period_blink(uint32_t count, uint32_t ms)
{
    suble_timer_start_0(SUBLE_TIMER105, ms, count*2);
}

/*********************************************************
FN: 
*/
void suble_gpio_rled_period_blink_cancel(void)
{
    suble_timer_stop_0(SUBLE_TIMER105);
}






































