/**
 ****************************************************************************************
 *
 * @file sdp_service_task.h
 *
 * @brief Header file - SDP Service Client Role Task.
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */


#ifndef _SDP_SERVICE_TASK_H_
#define _SDP_SERVICE_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup SDP Service Client Task
 * @ingroup SDP
 * @brief SDP Service Client Task
 *
 * The SDPTASK is responsible for handling the messages coming in and out of the
 * @ref SDP block of the BLE Host.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_SDP_CLIENT)
#include "ke_task.h"
#include "prf_types.h"
#include "rwip_task.h" // Task definitions
/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of SDP Client task instances
#define SDP_IDX_MAX     (BLE_NB_PROFILES_ADD_MAX)



#define SDP_NB_SERVICE_INSTANCES_MAX      (BLE_NB_PROFILES_ADD_MAX)
#define SDP_NB_CHAR_INSTANCES_MAX         (20)
#define SDP_NB_DESC_INSTANCES_MAX         (SDP_NB_CHAR_INSTANCES_MAX)
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// Parameters for the database creation
struct sdp_db_cfg
{
    /// Number of sdp to add
    uint8_t sdp_idx;

};

/// Possible states of the SDP task
enum sdp_state
{
    /// Disconnected state
    SDP_FREE,
    /// IDLE state
    SDP_IDLE,
    /// Busy State

    SDP_DISCOVING,
    SDP_BUSY,

    /// Number of defined states.
    SDP_STATE_MAX
};

enum sdp_msg_id
{
    /// Start the Battery Service Client Role - at connection
    SDP_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_SDP),
    ///Confirm that cfg connection has finished with discovery results, or that normal cnx started
    SDP_ENABLE_RSP,

    /// Read Characteristic xxxx Request
    SDP_READ_INFO_REQ,
    /// Read Characteristic xxxx Response
    SDP_READ_INFO_RSP,

    /// Write Characteristic Value Request
    SDP_WRITE_VALUE_INFO_REQ,
    ///  Write Characteristic Value response
    SDP_WRITE_VALUE_INFO_RSP,

    /// Write  Notification Configuration Value request
    SDP_WRITE_NTF_CFG_REQ,
    /// Write  Notification Configuration Value response
    SDP_WRITE_NTF_CFG_RSP,

    /// Indicate to APP that the  ntf value has been received
    SDP_NTF_IND,
};


/// Peer service info that can be read
enum sdpc_info
{
    /// SERVICE Char  value
    SDPC_CHAR_VAL,
    ///  Client Characteristic Configuration
    SDPC_CHAR_NTF_CFG,
	  ///  Client Characteristic User Description
	SDPC_CHAR_USER_DESC_VAL,
    /// Characteristic Presentation Format
    SDPC_CHAR_PRES_FORMAT,

    SDPC_INFO_MAX,
};

enum sdpc_type
{
    /// SERVICE Char  value
    SDPC_OPERATE_HANDLE,
    SDPC_OPERATE_UUID,
};
/*
 * APIs Structure
 ****************************************************************************************
 */


/// Parameters of the @ref SDP_ENABLE_REQ message
struct sdp_enable_req
{
    uint8_t conidx;
    ///Connection type
    uint8_t con_type;


};

/// Parameters of the @ref SDP_ENABLE_RSP message
struct sdp_enable_rsp
{
    uint8_t conidx;
    /// Status
    uint8_t status;

};


///Parameters of the @ref SDP_READ_INFO_REQ message
struct sdp_read_info_req
{
    uint8_t conidx;
    ///Characteristic info @see enum sdpc_info
    uint8_t info;
    uint8_t type;
    uint16_t handle;
    uint8_t uuid_len;
    uint8_t uuid[ATT_UUID_128_LEN];

};

///Parameters of the @ref  SDP_READ_INFO_RSP message
struct sdp_read_info_rsp
{
    uint8_t conidx;
    /// status of the request
    uint8_t status;
    ///Characteristic info @see enum sdpc_info
    uint8_t info;
    uint16_t length;
    /// Information data
    uint8_t data[__ARRAY_EMPTY];
};

///Parameters of the @ref  SDP_WRITE_INFO_REQ message
struct sdp_write_info_req
{
    uint8_t conidx;
    uint8_t info;
    uint8_t operation;
    uint16_t handle;
    uint16_t length;
    uint8_t data[__ARRAY_EMPTY];
};

///Parameters of the @ref SDP_W_NTF_CFG_REQ message
struct sdp_write_ntf_cfg_req
{
    uint8_t conidx;
    uint8_t info;
    uint16_t handle;
    ///Notification Configuration
    uint16_t ntf_cfg;

};

///Parameters of the @ref SDP_W_NTF_CFG_RSP message
struct sdp_write_ntf_cfg_rsp
{
    uint8_t conidx;
    ///Status
    uint8_t status;

};



/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

extern const struct ke_state_handler sdp_default_handler;

#endif // (BLE_SDP_CLIENT)

/// @} SDPTASK

#endif /* _SDP_SERVICE_TASK_H_ */
