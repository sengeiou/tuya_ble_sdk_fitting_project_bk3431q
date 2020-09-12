/**
 ****************************************************************************************
 *
 * @file app_electric.c
 *
 * @brief findt Application Module entry point
 *
 * @auth  
 *
 * @date  2016.05.31
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */
#ifndef APP_ELECTRIC_H_
#define APP_ELECTRIC_H_
/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief  Application Module entry point
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration


#include <stdint.h>          // Standard Integer Definition
#include "ke_task.h"         // Kernel Task Definition

#include "appm_task.h"

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */
typedef enum
{
    BLE_CONN      = 0,
    BLE_DISCONN   = 1,
}ble_sta_t;

///  Application Module Environment Structure
struct app_electric_env_tag
{
    /// Connection handle
    uint8_t conidx;
    
    uint16_t ntf_cfg[BLE_CONNECTION_MAX];///[APPM_SLAVE_CON_MAX];
};
/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */
/// fff0s Application environment
extern struct app_electric_env_tag app_electric_env;

/// Table of message handlers
extern const struct ke_state_handler app_electric_table_handler;
/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 *
 *  Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize fff0s Application Module
 ****************************************************************************************
 */
void app_electric_init(void);
/**
 ****************************************************************************************
 * @brief Add a fff0 Service instance in the DB
 ****************************************************************************************
 */
void app_add_electric(void);


uint8_t app_electric_tx_send(uint8_t conidx,uint8_t len,uint8_t* buf);


#endif // APP_ELECTRIC_H_
