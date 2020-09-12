/**
****************************************************************************
* @file      master_app.h
* @brief     master_app
* @author    suding
* @version   V1.0.0
* @date      2020-04
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2020 Tuya </center></h2>
*/


#ifndef __MASTER_APP_H__
#define __MASTER_APP_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDE
 */
#include "rwip_config.h"     // SW configuration
#if (BLE_APP_PRESENT)
#include <stdint.h>          // Standard Integer Definition
#include <co_bt.h>           // Common BT Definitions
#include "arch.h"            // Platform Definitions
#include "gap.h"
#include "gapc.h"            // GAPC Definitions
#include "gattc_task.h"
#include <stdbool.h>
#include "ke_task.h"
#include "co_error.h"
#include "attm.h"
#include "gattc_task.h"
#include "prf_utils.h"
#include "ke_mem.h"
#include "sdp_service.h"


#if (BLE_CENTRAL || BLE_OBSERVER)

/*********************************************************************
 * CONSTANT
 */
#define ADV_REPORT_DEV_NUM    5

/*********************************************************************
 * STRUCT
 */
struct adv_addr_list_t
{
    uint8_t nums;
    uint8_t adv_report_num;
    uint8_t addr_type[ADV_REPORT_DEV_NUM];
    struct bd_addr adv_addr[ADV_REPORT_DEV_NUM];
    uint8_t adv_data_len[ADV_REPORT_DEV_NUM];
    uint8_t adv_data[ADV_REPORT_DEV_NUM][31];
};

/*********************************************************************
 * EXTERNAL VARIABLE
 */

/*********************************************************************
 * EXTERNAL FUNCTION
 */
void appm_start_scanning(void);
void appm_stop_scanning(void);
uint8_t appm_start_connencting(struct gap_bdaddr bdaddr);
uint8_t appm_stop_connencting(void);
uint8_t appm_disconnect(uint8_t condix);

void appm_recover_connencting(struct gap_bdaddr bdaddr);
void appm_creat_connenct(void);
uint8_t appc_write_service_data_req(uint8_t conidx,uint16_t handle,uint16_t data_len,uint8_t *data);
uint8_t appc_write_service_ntf_cfg_req(uint8_t conidx,uint16_t handle,uint16_t ntf_cfg);
uint8_t appm_read_service_data_by_uuid_req(uint8_t conidx,uint8_t uuid_len,uint8_t* uuid);
uint8_t appm_read_service_data_by_handle_req(uint8_t conidx,uint16_t handle);
uint8_t appm_read_service_ntf_ind_cfg_by_handle_req(uint8_t conidx,uint16_t handle);
uint8_t appm_read_service_userDesc_by_handle_req(uint8_t conidx,uint16_t handle);
uint8_t sdp_enable_all_server_ntf_ind(uint8_t conidx,uint8_t reset);
void sdp_prf_register_all_atthdl2gatt(void);


#endif //#if (BLE_CENTRAL || BLE_OBSERVER)
#endif //BLE_APP_PRESENT


#ifdef __cplusplus
}
#endif

#endif //__MASTER_APP_H__

