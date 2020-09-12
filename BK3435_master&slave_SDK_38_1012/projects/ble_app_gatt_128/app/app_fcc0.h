/**
 ****************************************************************************************
 *
 * @file app_ffc0.c
 *
 * @brief findt Application Module entry point
 *
 * @auth  HAI.XU
 *
 * @date  2016.05.31
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */
#ifndef APP_FCC0_H_
#define APP_FCC0_H_
/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief ffc0 Application Module entry point
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

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

/// fff0s Application Module Environment Structure
struct app_fcc0_env_tag
{
    /// Connection handle
    uint8_t conidx;
};
/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

/// fff0s Application environment
extern struct app_fcc0_env_tag app_fcc0_env;

/// Table of message handlers
extern const struct ke_state_handler app_fcc0_table_handler;
/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 *
 * fff0s Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize fff0s Application Module
 ****************************************************************************************
 */
void app_fcc0_init(void);
/**
 ****************************************************************************************
 * @brief Add a fff0 Service instance in the DB
 ****************************************************************************************
 */
void app_fcc0_add_fcc0s(void);




#endif // APP_BATT_H_
