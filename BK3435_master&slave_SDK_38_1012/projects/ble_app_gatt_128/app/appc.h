/**
****************************************************************************
* @file      appc.h
* @brief     appc
* @author    suding
* @version   V1.0.0
* @date      2020-04
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2020 Tuya </center></h2>
*/


#ifndef __APPC_H__
#define __APPC_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDE
 */
#include <stdint.h>          // Standard Integer Definition
#include <co_bt.h>           // Common BT Definitions
#include "arch.h"            // Platform Definitions
#include "gapc.h"            // GAPC Definitions
#include "appc_task.h"

/*********************************************************************
 * CONSTANT
 */

/*********************************************************************
 * STRUCT
 */
/// Application environment structure
struct appc_env_tag
{
    uint16_t conhdl;
    uint8_t  conidx;
    uint8_t  role;
    bool     bonded;
    uint8_t  loc_irk[KEY_LEN]; //Local device IRK
    struct   gap_bdaddr con_dev_addr;
    uint16_t svc_write_handle;
    uint16_t svc_notif_handle;
};

/*********************************************************************
 * EXTERNAL VARIABLE
 */
extern struct appc_env_tag *appc_env[APPC_IDX_MAX];

/*********************************************************************
 * EXTERNAL FUNCTION
 */
void appc_update_param(uint8_t conidx,uint16_t intv_min,uint16_t intv_max,uint16_t latency,uint16_t time_out);
uint8_t appc_get_peer_dev_info(uint8_t conidx,uint8_t type);

void appc_init(void);
void appc_create(uint8_t conidx,struct gapc_connection_req_ind const*con_req);
void appc_cleanup(uint8_t conidx);
void appc_le_data_length_update_req(uint8_t conidx,uint16_t max_tx_octets,uint16_t max_tx_time);
void appc_gatt_mtu_change(uint8_t conidx);


#ifdef __cplusplus
}
#endif

#endif //__APPC_H__

