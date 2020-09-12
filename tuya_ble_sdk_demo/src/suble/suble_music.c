#include "suble_common.h"




/*********************************************************************
 * LOCAL CONSTANTS
 */
//低音——C调
//#define NTD0 100
//#define NTD1 131
//#define NTD2 147
//#define NTD3 165
//#define NTD4 175
//#define NTD5 196
//#define NTD6 221
//#define NTD7 248
//低音——D调
//#define NTD0 100
//#define NTD1 147
//#define NTD2 165
//#define NTD3 175
//#define NTD4 196
//#define NTD5 221
//#define NTD6 248
//#define NTD7 278

//中音——C调
#define NTD0 100
#define NTD1 262
#define NTD2 294
#define NTD3 330
#define NTD4 350
#define NTD5 393
#define NTD6 441
#define NTD7 495
//中音——D调
#define NTD0_D 100
#define NTD1_D 294
#define NTD2_D 330
#define NTD3_D 350
#define NTD4_D 393
#define NTD5_D 441
#define NTD6_D 495
#define NTD7_D 556

//高音——C调
//#define NTD0 100
//#define NTD1 525
//#define NTD2 589
//#define NTD3 661
//#define NTD4 700
//#define NTD5 786
//#define NTD6 882
//#define NTD7 990
//高音——D调
//#define NTD0 100
//#define NTD1 589
//#define NTD2 661
//#define NTD3 700
//#define NTD4 786
//#define NTD5 882
//#define NTD6 990
//#define NTD7 1112

/*********************************************************************
 * LOCAL STRUCT
 */
#pragma pack(1)
//open with bt
typedef struct
{
    const uint16_t* pTune;
    const float* pDurt;
    uint32_t    size;
} music_tune_t;
#pragma pack()

/*********************************************************************
 * LOCAL VARIABLES
 */
/*********************************************************  欢乐颂  *********************************************************/
//根据简谱列出各频率
static const uint16_t tune1[]= {
    NTD3,NTD3,NTD4,NTD5,
    NTD5,NTD4,NTD3,NTD2,
    NTD1,NTD1,NTD2,NTD3,
    NTD3,NTD2,NTD2,
    NTD3,NTD3,NTD4,NTD5,
    NTD5,NTD4,NTD3,NTD2,
    NTD1,NTD1,NTD2,NTD3,
    NTD2,NTD1,NTD1,
    NTD2,NTD2,NTD3,NTD1,
    NTD2,NTD3,NTD4,NTD3,NTD1,
    NTD2,NTD3,NTD4,NTD3,NTD2,
    NTD1,NTD2,NTD5,NTD0,
    NTD3,NTD3,NTD4,NTD5,
    NTD5,NTD4,NTD3,NTD4,NTD2,
    NTD1,NTD1,NTD2,NTD3,
    NTD2,NTD1,NTD1
};
//根据简谱列出各节拍
static const float durt1[]= {
    1,1,1,1,
    1,1,1,1,
    1,1,1,1,
    1+0.5,0.5,1+1,
    1,1,1,1,
    1,1,1,1,
    1,1,1,1,
    1+0.5,0.5,1+1,
    1,1,1,1,
    1,0.5,0.5,1,1,
    1,0.5,0.5,1,1,
    1,1,1,1,
    1,1,1,1,
    1,1,1,0.5,0.5,
    1,1,1,1,
    1+0.5,0.5,1+1,
};

/*********************************************************  一闪一闪亮晶晶  *********************************************************/
//根据简谱列出各频率
static const uint16_t tune2[]= {
    NTD1,NTD1,NTD5,NTD5,NTD6,NTD6,NTD5,
    NTD4,NTD4,NTD3,NTD3,NTD2,NTD2,NTD1,
    NTD5,NTD5,NTD4,NTD4,NTD3,NTD3,NTD2,
    NTD5,NTD5,NTD4,NTD4,NTD3,NTD3,NTD2,
    NTD1,NTD1,NTD5,NTD5,NTD6,NTD6,NTD5,
    NTD4,NTD4,NTD3,NTD3,NTD2,NTD2,NTD1,
};
//根据简谱列出各节拍
static const float durt2[]= {
    1,1,1,1,1,1,1+1,
    1,1,1,1,1,1,1+1,
    1,1,1,1,1,1,1+1,
    1,1,1,1,1,1,1+1,
    1,1,1,1,1,1,1+1,
    1,1,1,1,1,1,1+1,
};

/*********************************************************  提示音1  *********************************************************/
//根据简谱列出各频率
static const uint16_t tune3[]= {
    NTD1,NTD2,NTD1,NTD3,NTD2,NTD1,NTD4,NTD3,NTD5,
};
//根据简谱列出各节拍
float durt3[]= {
    0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5+1,0.5,
};

/*********************************************************  提示音2  *********************************************************/
//根据简谱列出各频率
static const uint16_t tune4[]= {
    NTD1,NTD3,NTD5,NTD1,NTD1,NTD5,NTD3,NTD1
};
//根据简谱列出各节拍
static const float durt4[]= {
    0.5,0.5,0.5,0.5+1,0.5,0.5,0.5,0.5+1
};

/*********************************************************  歌曲1  *********************************************************/
//根据简谱列出各频率
static const uint16_t tune5[]= {
    NTD3,NTD3,NTD6,NTD1,NTD1,
};
//根据简谱列出各节拍
float durt5[]= {
    0.3,0.3,0.5,0.3,0.3,
};

/*********************************************************  歌曲2  *********************************************************/
//根据简谱列出各频率
static const uint16_t tune6[]= {
    NTD7,NTD1,NTD7,NTD7,NTD2
};
//根据简谱列出各节拍
float durt6[]= {
    0.2,0.5,0.2,0.2,0.5,
};

/*********************************************************  歌曲3  *********************************************************/
//根据简谱列出各频率
static const uint16_t tune7[]= {
    NTD1,NTD4,
};
//根据简谱列出各节拍
float durt7[]= {
    0.1,0.5,
};

/*********************************************************  歌曲4  *********************************************************/
//根据简谱列出各频率
static const uint16_t tune8[]= {
    NTD4,NTD3,NTD1,NTD5,NTD4,NTD3,NTD1,NTD5,
};
//根据简谱列出各节拍
float durt8[]= {
    0.4,0.3,0.2,0.5,0.4,0.3,0.2,0.5,
};

/*********************************************************  歌曲5  *********************************************************/
//根据简谱列出各频率
static const uint16_t tune9[]= {
    NTD1,NTD7,NTD7,NTD1,NTD7,
};
//根据简谱列出各节拍
float durt9[]= {
    0.1,0.5,0.5,0.1,0.5,
};

/*********************************************************  歌曲6  *********************************************************/
//根据简谱列出各频率
static const uint16_t tune10[]= {
    NTD1,NTD2,NTD3,NTD4,NTD5,NTD6,NTD7,NTD7,NTD6,NTD5,NTD4,NTD3,NTD2,NTD1
};
//根据简谱列出各节拍
float durt10[]= {
    0.3,0.3,0.2,0.2,0.1,0.1,0.1,0.1,0.1,0.1,0.2,0.2,0.3,0.3,
};

/*********************************************************  嘀1声  *********************************************************/
//根据简谱列出各频率
static const uint16_t tune11[]= {
    NTD1
};
//根据简谱列出各节拍
float durt11[]= {
    1
};

/*********************************************************  嘀2声  *********************************************************/
//根据简谱列出各频率
static const uint16_t tune12[]= {
    NTD1,0,NTD1
};
//根据简谱列出各节拍
float durt12[]= {
    1,0.8,1
};

/*********************************************************  长嘀1声  *********************************************************/
//根据简谱列出各频率
static const uint16_t tune13[]= {
    NTD1
};
//根据简谱列出各节拍
float durt13[]= {
    10
};

//抖音。电音乐谱
/*********************************************************  音乐指针  *********************************************************/
static music_tune_t music[]= {
//    { tune1, durt1, sizeof(tune1)/sizeof(tune1[0]) },
//    { tune2, durt2, sizeof(tune2)/sizeof(tune2[0]) },
    { tune5, durt5, sizeof(tune5)/sizeof(tune5[0]) },
    { tune6, durt6, sizeof(tune6)/sizeof(tune6[0]) },
    { tune7, durt7, sizeof(tune7)/sizeof(tune7[0]) },
    { tune8, durt8, sizeof(tune8)/sizeof(tune8[0]) },
    { tune9, durt9, sizeof(tune9)/sizeof(tune9[0]) },
    { tune10, durt10, sizeof(tune10)/sizeof(tune10[0]) },
    { tune11, durt11, sizeof(tune11)/sizeof(tune11[0]) },
    { tune12, durt12, sizeof(tune12)/sizeof(tune12[0]) },
    { tune13, durt13, sizeof(tune13)/sizeof(tune13[0]) },
    { tune3, durt3, sizeof(tune3)/sizeof(tune3[0]) },
    { tune4, durt4, sizeof(tune4)/sizeof(tune4[0]) },
};
static uint32_t s_music_idx = 0;
static uint32_t s_tune_idx = 0;
static uint8_t s_music_mode = MUSIC_MODE_ONCE;

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/
void suble_buzzer_timeout_handler(void)
{
    s_tune_idx++;
    if(s_tune_idx < music[s_music_idx].size) {
        suble_buzzer_stop();
        suble_delay_ms(10);
        
        uint16_t tune = music[s_music_idx].pTune[s_tune_idx];
        float durt = music[s_music_idx].pDurt[s_tune_idx];
        
        suble_buzzer_start(tune*10);
        if(s_music_mode == MUSIC_MODE_ONCE) {
            suble_timer_start_0(SUBLE_TIMER104, (100*durt), 1);
//            APP_DEBUG_PRINTF("count: %d, delay: %d", s_tune_idx, (100*durt));
        }
        else {
            suble_timer_start_0(SUBLE_TIMER104, (500*durt), 1);
//            APP_DEBUG_PRINTF("count: %d, delay: %d", s_tune_idx, (500*durt));
        }
    }
    else {
        if(s_music_mode == MUSIC_MODE_ONCE) {
            suble_buzzer_stop();
            g_system_sleep = true;
            SUBLE_PRINTF("suble_buzzer_stop");
        }
        else {
            lock_play_music(MUSIC_MODE_REPEAT, s_music_idx);
        }
    }
}

/*********************************************************
FN: 
*/
void lock_play_music(uint8_t mode, uint32_t music_idx)
{
    SUBLE_PRINTF("music_idx: %d", music_idx);
    
    if(mode == MUSIC_MODE_REPEAT) {
        music_idx = lock_music_maxnum(music_idx);
    }
    
    if((suble_gpio_get_output(ANTILOCK_BUZZER_EN1) == SUBLE_LEVEL_HIGH)
        || (suble_gpio_get_output(ANTILOCK_BUZZER_EN2) == SUBLE_LEVEL_HIGH))
    {
        g_system_sleep = false;
        
        s_music_mode = mode;
        s_tune_idx = 0;
        s_music_idx = music_idx;
        
        uint16_t tune = music[s_music_idx].pTune[s_tune_idx];
        float durt = music[s_music_idx].pDurt[s_tune_idx];
        
        suble_buzzer_start(tune*10);
        if(s_music_mode == MUSIC_MODE_ONCE) {
            suble_timer_start_0(SUBLE_TIMER104, (100*durt), 1);
//            APP_DEBUG_PRINTF("count: %d, delay: %d", s_tune_idx, (100*durt));
        }
        else {
            suble_timer_start_0(SUBLE_TIMER104, (500*durt), 1);
//            APP_DEBUG_PRINTF("count: %d, delay: %d", s_tune_idx, (500*durt));
        }
    }
}

/*********************************************************
FN: 
*/
void lock_play_music_cancel(void)
{
    s_tune_idx = music[s_music_idx].size;
    s_music_mode = MUSIC_MODE_ONCE;
}

uint32_t lock_music_maxnum(uint32_t music_idx)
{
    if(music_idx >= LOCK_MUSIC_MAX_NUM) {
        return 0;
    }
    return music_idx;
}






