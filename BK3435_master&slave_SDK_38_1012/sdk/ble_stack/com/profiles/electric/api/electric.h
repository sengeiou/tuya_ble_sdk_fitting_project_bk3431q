/**
 ****************************************************************************************
 *
 * @file electric.h
 *
 * @brief Header file - electric Service Server Role
 *
 * Copyright (C) beken 2019-2022
 *
 *
 ****************************************************************************************
 */
#ifndef _ELECTRIC_H_ 
#define _ELECTRIC_H_

/**
 ****************************************************************************************
 * @addtogroup  electric 'Profile' Server
 * @ingroup electric
 * @brief electric 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwprf_config.h"

#if 1//(BLE_ELECTRIC_SERVER)

#include "electric_task.h"
#include "atts.h"
#include "prf_types.h"
#include "prf.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#define ELECTRIC_CFG_FLAG_MANDATORY_MASK       (0x3F)

#define ELECTRIC_CFG_FLAG_NTF_SUP_MASK          (0x10)

#define ELECTRIC_FLAG_NTF_CFG_BIT             (0x05)



#define ELECTRIC_SERV_UUID_128     {0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E}

#define ELECTRIC_CHAR_RX_UUID_128  {0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E}

#define ELECTRIC_CHAR_TX_UUID_128  {0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E}


/// ELECTRIC Service Attributes Indexes
enum
{
	ELECTRIC_IDX_SVC,

	ELECTRIC_IDX_RX_CHAR,
	ELECTRIC_IDX_RX_VAL,

	ELECTRIC_IDX_TX_CHAR,
	ELECTRIC_IDX_TX_VAL,
	ELECTRIC_IDX_TX_NTF_CFG,

	ELECTRIC_IDX_NB,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// ELECTRIC 'Profile' Server environment variable
struct electric_env_tag
{
    /// profile environment
    prf_env_t prf_env;
   
    /// On-going operation
    struct ke_msg * operation;
    ///  Services Start Handle
    uint16_t start_hdl;
    /// rx  
//    uint8_t rx_value[ELECTRIC_RX_DATA_LEN];
	
//	uint8_t tx_value[ELECTRIC_TX_DATA_LEN];
    /// BASS task state
    ke_state_t state[ELECTRIC_IDX_MAX];
    /// Notification configuration of peer devices.
    uint16_t ntf_cfg[BLE_CONNECTION_MAX];
    /// Database features
    uint8_t features;

};



/**
 ****************************************************************************************
 * @brief Retrieve service profile interface
 *
 * @return  service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* electric_prf_itf_get(void);

uint16_t electric_get_att_handle(uint8_t att_idx);

uint8_t  electric_get_att_idx(uint16_t handle, uint8_t *att_idx);

void electric_tx_notify(struct electric_env_tag* env, struct electric_tx_upd_req const *param);

#endif /* #if (BLE_ELECTRIC_SERVER) */



#endif /*  _ELECTRIC_H_ */



