/**
****************************************************************************
* @file      appm_task.h
* @brief     appm_task
* @author    suding
* @version   V1.0.0
* @date      2020-04
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2020 Tuya </center></h2>
*/


#ifndef __APPM_TASK_H__
#define __APPM_TASK_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDE
 */
#include "rwip_config.h"    // SW configuration
#if (BLE_APP_PRESENT)
#include <stdint.h>         // Standard Integer
#include "rwip.h"
#include "rwip_task.h"      // Task definitions
#include "ke_task.h"        // Kernel Task


/*********************************************************************
 * CONSTANT
 */
//最大的实例数量
#define APPM_IDX_MAX                (0x01)

#define APPM_MASTER_CON_MAX         (0x01)
#define APPM_SLAVE_CON_MAX          (0x01)

enum appm_state
{
    APPM_INIT                   = 0,
    APPM_CREATE_DB              = 1,
    APPM_IDLE                   = 2,
    APPM_ADVERTISING            = 3,
    APPM_WAIT_ADVERTISTING_END  = 4,
    APPM_ADVERTISTING_END       = 5,
    APPM_SCANNING               = 6,
    APPM_WAIT_SCAN_END          = 7,
    APPM_SCAN_END               = 8,
    APPM_CONNECTING             = 9,
    APPM_LINK_CONNECTED         = 10,//for slave
    APPM_SDP_DISCOVERING        = 11,//for master
    APPM_CONNECTED              = APPM_IDLE,//Prf Connected //for master
    APPM_DISCONNECT             = 13,
    APPM_STATE_MAX
};

enum appm_msg
{
    APPM_DUMMY_MSG = TASK_FIRST_MSG(TASK_ID_APPM),
    APPM_SCAN_TIMEOUT_TIMER,
    APPM_CON_TIMEOUT_TIMER,

	APP_PERIOD_TIMER,
	APP_SEND_SMPREQ_TIMER,

    SUBLE_TIMER0,
    SUBLE_TIMER1,
    SUBLE_TIMER2,
    SUBLE_TIMER3,
    SUBLE_TIMER4,
    SUBLE_TIMER5,
    SUBLE_TIMER6,
    SUBLE_TIMER7,
    SUBLE_TIMER8,
    SUBLE_TIMER9,
    SUBLE_TIMER10,
    SUBLE_TIMER11,
    SUBLE_TIMER12,
    SUBLE_TIMER13,
    SUBLE_TIMER14,
    SUBLE_TIMER15,
    SUBLE_TIMER16,
    SUBLE_TIMER17,
    SUBLE_TIMER18,
    SUBLE_TIMER19,
    SUBLE_TIMER100,
    SUBLE_TIMER101,
    SUBLE_TIMER102,
    SUBLE_TIMER103,
    SUBLE_TIMER104,
    SUBLE_TIMER105,
    SUBLE_TIMER106,
    SUBLE_TIMER107,
    SUBLE_TIMER108,
    SUBLE_TIMER109,
    SUBLE_TIMER200,
    SUBLE_TIMER_MAX,
};

#define SUBLE_TIMERX_HANDLER(x) \
                static int suble_timer##x##_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id) { \
                    suble_timer_handler(SUBLE_TIMER##x); \
                    return KE_MSG_CONSUMED; \
                }

/*********************************************************************
 * STRUCT
 */

/*********************************************************************
 * EXTERNAL VARIABLE
 */

/*********************************************************************
 * EXTERNAL FUNCTION
 */
extern const struct ke_state_handler appm_default_handler;
extern ke_state_t appm_state[APPM_IDX_MAX];

#endif //(BLE_APP_PRESENT)

#ifdef __cplusplus
}
#endif

#endif //__APPM_TASK_H__

