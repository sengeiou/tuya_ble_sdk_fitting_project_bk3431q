/**
****************************************************************************
* @file      appc_task.h
* @brief     appc_task
* @author    suding
* @version   V1.0.0
* @date      2020-04
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2020 Tuya </center></h2>
*/


#ifndef __APPC_TASK_H__
#define __APPC_TASK_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDE
 */
#include "rwip_config.h"    // SW configuration
#include <stdint.h>         // Standard Integer
#include "rwip.h"
#include "rwip_task.h"      // Task definitions
#include "ke_task.h"        // Kernel Task

/*********************************************************************
 * CONSTANT
 */
//最大的实例数量
#define APPC_IDX_MAX                 (BLE_CONNECTION_MAX)

enum appc_state
{
    APPC_LINK_IDLE,
    APPC_LINK_CONNECTED,
    APPC_SDP_DISCOVERING, 
    APPC_SERVICE_CONNECTED,
    APPC_STATE_MAX
};

enum appc_msg
{
    APPC_DUMMY_MSG = TASK_FIRST_MSG(TASK_ID_APPC),
    /// Timer used to automatically stop advertising
    APPC_CHECK_LINK_TIMER,
};

/*********************************************************************
 * STRUCT
 */

/*********************************************************************
 * EXTERNAL VARIABLE
 */

/*********************************************************************
 * EXTERNAL FUNCTION
 */
extern const struct ke_state_handler appc_default_handler;
extern ke_state_t appc_state[APPC_IDX_MAX];


#ifdef __cplusplus
}
#endif

#endif //__APPC_TASK_H__

