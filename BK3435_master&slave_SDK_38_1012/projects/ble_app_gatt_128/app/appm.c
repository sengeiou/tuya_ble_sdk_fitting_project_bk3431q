#include "rwip_config.h"             // SW configuration
#if (BLE_APP_PRESENT)
#include <string.h>
#include "rwapp_config.h"
#include "appm_task.h"                // Application task Definition
#include "appm.h"                     // Application Definition
#include "appc.h" 
#include "appc_task.h"
#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manager Task API
#include "gapc_task.h"               // GAP Controller Task API
//#if CLI_CONSOLE
#include "cli.h"
//#endif
#include "app_api.h"
#include "co_bt.h"                   // Common BT Definition
#include "co_math.h"                 // Common Maths Definition
#include "ke_timer.h"
#include "app_fff0.h"                // Application security Definition
#include "app_dis.h"                 // Device Information Service Application Definitions
#include "app_batt.h"                // Battery Application Definitions
#include "app_oads.h"                 // Application oads Definition
#include "master_app.h" 
#include "app_electric.h"
#include "nvds.h"                    // NVDS Definitions
#include "rf.h"
#include "uart.h"
#include "adc.h"
#include "gpio.h"
#include "wdt.h"
#include "app_sec.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */
//Application Task Descriptor
static const struct ke_task_desc TASK_DESC_APPM = {NULL, &appm_default_handler, appm_state, APPM_STATE_MAX, APPM_IDX_MAX};

//添加服务列表
typedef void (*appm_add_svc_func_t)(void);
static const appm_add_svc_func_t appm_add_svc_func_list[APPM_SVC_LIST_STOP] = {
    (appm_add_svc_func_t)app_fff0_add_fff0s,
};

/*********************************************************************
 * VARIABLE
 */
//Application环境变量
struct appm_env_tag appm_env;

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/
void appm_init()
{
    uint8_t key_len = KEY_LEN;
    
    //初始化环境变量
    memset(&appm_env, 0, sizeof(struct appm_env_tag));
    //设置默认名称
    appm_env.dev_name_len = strlen(APP_DFLT_DEVICE_NAME);
    memcpy(appm_env.dev_name, APP_DFLT_DEVICE_NAME, appm_env.dev_name_len);

    //创建任务
    ke_task_create(TASK_APPM, &TASK_DESC_APPM);
    //初始化任务状态
    ke_state_set(TASK_APPM, APPM_INIT);

    //支持主从机
    appm_env.features = APPM_MASTER_MODE | APPM_SLAVE_MODE;
    
    //启动扫描/广播
//    APPM_SET_FIELD(SCAN_EN, 1);
//    APPM_SET_FIELD(ADV_EN, 1);

    if(nvds_get(NVDS_TAG_LOC_IRK, &key_len, appm_env.loc_irk) != NVDS_OK)
    {
        //generate a new IRK
        for (uint8_t idx = 0; idx < KEY_LEN; idx++) {
            appm_env.loc_irk[idx] = (uint8_t)co_rand_word();
        }

        //Store the generated value in NVDS
        if(nvds_put(NVDS_TAG_LOC_IRK, KEY_LEN, (uint8_t *)&(appm_env.loc_irk)) != NVDS_OK) {
            ASSERT_INFO(0, NVDS_TAG_LOC_IRK, 0);
        }
    }
    key_len = NVDS_LEN_DEVICE_NAME;
    
    appc_init();
    //初始化服务
    app_fff0_init();
    sdp_service_init();
}

/*********************************************************
FN: 
*/
bool appm_add_svc(void)
{
    // Indicate if more services need to be added in the database
    bool more_svc = false;

    // Check if another should be added in the database
    if (appm_env.next_svc != APPM_SVC_LIST_STOP)
    {
        ASSERT_INFO(appm_add_svc_func_list[app_env.next_svc] != NULL, appm_env.next_svc, 1);

        // Call the function used to add the required service
        appm_add_svc_func_list[appm_env.next_svc]();

        // Select following service to add
        appm_env.next_svc++;
        more_svc = true;
    }

    return more_svc;
}

/*********************************************************
FN: 
RT: name length
*/
uint8_t appm_get_dev_name(uint8_t* name)
{
    memcpy(name, appm_env.dev_name, appm_env.dev_name_len);
    return appm_env.dev_name_len;
}

/*********************************************************
FN: 
*/
void appm_scan_adv_con_schedule(void)
{
    uint8_t cur_state = ke_state_get(TASK_APPM);
//    SUBLE_PRINTF("state:%x",cur_state);
    
    switch(cur_state)
    {
        case APPM_CREATE_DB: { // 1
            if(APPM_GET_FIELD(ADV_EN)) {
                ke_state_set(TASK_APPM, APPM_IDLE);
                appm_start_advertising();
            }
            else if(APPM_GET_FIELD(SCAN_EN)) {
                ke_state_set(TASK_APPM, APPM_IDLE);
                appm_start_scanning();
            }
        } break;
        
        case APPM_IDLE: { // 2
            if(APPM_GET_FIELD(ADV_EN)) {
                appm_start_advertising();
            }
            else if(APPM_GET_FIELD(SCAN_EN)) {
                appm_start_scanning();
            }
        } break;
       
        case APPM_ADVERTISING:  { // 3
            if(APPM_GET_FIELD(SCAN_EN)) {
                appm_stop_advertising();// first stop adv 
            }
            else {
                if(APPM_GET_FIELD(ADV_EN)) {
                    // only adv 
                } else {
                    appm_stop_advertising();//
                }
            }
        } break;
       
        case APPM_WAIT_ADVERTISTING_END: { //4
        } break;  

        case APPM_ADVERTISTING_END: { // 5
            if(APPM_GET_FIELD(SCAN_EN))
            {
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_start_scanning();
            }
            else if(APPM_GET_FIELD(ADV_EN))
            {
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_start_advertising();
            }
            else {
                SUBLE_PRINTF("adv && scan off");
                ke_state_set(TASK_APPM,APPM_IDLE);
                if(appm_env.con_dev_flag == 1)
                {
                    ke_state_set(TASK_APPM,APPM_IDLE);
                    appm_start_connencting(appm_env.con_dev_addr);
                }
            }
        } break;
       
        case APPM_SCANNING: { //6
            if(APPM_GET_FIELD(ADV_EN)) {
                appm_stop_scanning();
            }
            else {
                if(APPM_GET_FIELD(SCAN_EN)) {
                    SUBLE_PRINTF("continue scaning");
                } else {
                    appm_stop_scanning();
                }
            }
        } break; 
 
        case APPM_WAIT_SCAN_END: { // 7
        } break;
       
        case APPM_SCAN_END: { // 8
            if(APPM_GET_FIELD(ADV_EN)) {
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_start_advertising();
            }
            else if(APPM_GET_FIELD(SCAN_EN)) {
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_start_scanning();             
            }
            else {
                SUBLE_PRINTF("scan && adv off");
                ke_state_set(TASK_APPM,APPM_IDLE);
                if(appm_env.con_dev_flag == 1)
                {
                    ke_state_set(TASK_APPM,APPM_IDLE);
                    appm_start_connencting(appm_env.con_dev_addr);
                }
            }
        } break;
        
        case APPM_CONNECTING: { // 9
            if(!ke_timer_active(APPM_CON_TIMEOUT_TIMER,TASK_APPM))
            {
                appm_stop_connencting();
                
                SUBLE_PRINTF("case APPM_CONNECTING");
                if(APPM_GET_FIELD(ADV_EN))//test 20190806
                {
                    ke_state_set(TASK_APPM,APPM_IDLE);
                    appm_start_advertising();
                }
                else if(APPM_GET_FIELD(SCAN_EN))
                {
                    ke_state_set(TASK_APPM,APPM_IDLE);
                    appm_start_scanning();
                }
            }
           // appm_stop_connencting();
        } break;
        
        case APPM_LINK_CONNECTED: { //10
            #if 0
            if(APPM_GET_FIELD(ADV_EN)) {
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_start_advertising();
            }
            else if(APPM_GET_FIELD(SCAN_EN)) {
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_start_scanning();
            }
            #endif
        } break;

        case APPM_DISCONNECT: { //13
            if(APPM_GET_FIELD(ADV_EN)) {
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_start_advertising();
            }
            else if(APPM_GET_FIELD(SCAN_EN)) {
                ke_state_set(TASK_APPM,APPM_IDLE);
                appm_start_scanning();
            }
            else {
                SUBLE_PRINTF("scan && adv off");
                ke_state_set(TASK_APPM,APPM_IDLE);
                if(appm_env.con_dev_flag == 1)
                {
                    ke_state_set(TASK_APPM,APPM_IDLE);
                    appm_start_connencting(appm_env.con_dev_addr);
                }
            }
        } break;

        default: {
        } break;
   }
}

static bool appm_field_flag = false;

void appm_field_save_clean(void)
{
    if(!appm_field_flag) {
        appm_field_flag = true;
        
        appm_env.fields_bck = appm_env.fields;
        appm_env.fields = 0;
    }
}

void appm_field_recover(void)
{
    if(appm_field_flag) {
        appm_field_flag = false;
        
        appm_env.fields = appm_env.fields_bck;
    }
}


/*********************************************************
FN: 过滤广播
*/
//uint8_t appm_filter_adv_report(struct adv_report const *p_param)
//{
//    struct adv_report* param = (void*)p_param;
//    
////    SUBLE_HEXDUMP("mac", param->adv_addr.addr, 6);
//    
//    //adv report
//    if(param->adv_addr.addr[0] == 0xE8)
//    {
//        SUBLE_PRINTF("evt_type: %d, rssi: %d, data_len: %d", param->evt_type, param->rssi, param->data_len);
//        SUBLE_HEXDUMP("mac", param->adv_addr.addr, 6);
//        SUBLE_HEXDUMP("adv_data", param->data, param->data_len);
//        
//        APPM_SET_FIELD(SCAN_EN, 0);
//        appm_scan_adv_con_schedule();
//        
//        struct gap_bdaddr mac;
//        mac.addr_type = param->adv_addr_type;
//        memcpy(mac.addr.addr, param->adv_addr.addr, 6);
//        appm_start_connencting(mac);
//    }
//    return 0;
//}

#endif //(BLE_APP_PRESENT)

/// @} APP


