/**
 ****************************************************************************************
 *
 * @file electric_task.h
 *
 * @brief Header file - electric Service Server Role Task.
 *
 * Copyright (C) RivieraWaves 2019-2022
 *
 *
 ****************************************************************************************
 */


#ifndef _ELECTRIC_TASK_H_
#define _ELECTRIC_TASK_H_


#include "rwprf_config.h"
#if 1//(BLE_ELECTRIC_SERVER)
#include <stdint.h>
#include "rwip_task.h" // Task definitions
#include "uart.h"
/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of Server task instances
#define ELECTRIC_IDX_MAX     0x01

///Maximal number of  that can be added in the DB
#define  ELECTRIC_RX_DATA_LEN  2048///244
#define  ELECTRIC_TX_DATA_LEN  2048///244
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Possible states of the task
enum electric_state
{
    /// Idle state
    ELECTRIC_IDLE,
    /// busy state
    ELECTRIC_BUSY,
    /// Number of defined states.
    ELECTRIC_STATE_MAX
};

/// Messages for FFF0 Server
enum electric_msg_id
{
    /// Start the  Server - at connection used to restore bond data
	ELECTRIC_CREATE_DB_REQ   = TASK_FIRST_MSG(TASK_ID_ELECTRIC),
	
	/// Value Update Request
	ELECTRIC_TX_UPD_REQ,
	/// Inform APP if TX value has been notified or not
	ELECTRIC_TX_UPD_RSP,
	/// Inform APP that TX Level Notification Configuration has been changed - use to update bond data
	ELECTRIC_TX_NTF_CFG_IND,
	
	ELECTRIC_RX_WRITER_REQ_IND,
	
};

/// Features Flag Masks
enum electric_features
{
    /// TX Characteristic doesn't support notifications
    ELECTRIC_TX_NTF_NOT_SUP,
    /// TX Characteristic support notifications
    ELECTRIC_TX_NTF_SUP,
};
/*
 * APIs Structures
 ****************************************************************************************
 */

/// Parameters for the database creation
struct electric_db_cfg
{
    /// Number of electric to add
    uint8_t electric_nb;
    /// Features of each electric instance
    uint8_t features;
};



///Parameters of the @ref ELECTRIC_TX_UPD_REQ message
struct electric_tx_upd_req
{
    /// instance
    uint8_t conidx;
	
	uint16_t length;
    /// fff1 Level
    uint8_t tx_value[ELECTRIC_TX_DATA_LEN];
};

///Parameters of the @ref ELECTRIC_TX_UPD_RSP message
struct electric_tx_upd_rsp
{
    uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref ELECTRIC_TX_NTF_CFG_IND message
struct electric_tx_ntf_cfg_ind
{
    /// connection index
    uint8_t  conidx;
    ///Notification Configuration
    uint16_t ntf_cfg;
};


/// Parameters of the @ref ELECTRIC_RX_WRITER_REQ_IND message
struct electric_rx_writer_ind
{
   
    /// Connection index
    uint8_t conidx;
    
    uint16_t length;
    /// Alert level
    uint8_t rx_value[ELECTRIC_TX_DATA_LEN];
		
};


/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

extern const struct ke_state_handler electric_default_handler;
#endif // BLE_ELECTRIC_SERVER


#endif /* _ELECTRIC_TASK_H_ */

