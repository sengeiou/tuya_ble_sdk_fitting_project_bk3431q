/**
 ****************************************************************************************
 *
 * @file sdp_service.h
 *
 * @brief Header file - sdp  Role.
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _SDP_SERVICE_H_
#define _SDP_SERVICE_H_

/**
 ****************************************************************************************
 * @addtogroup SDP_SERVICE  
 * @ingroup SDP
 * @brief SDP Service 
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#if 1 //(BLE_SDP_CLIENT)

#include <stdint.h>
#include <stdbool.h>
#include "ke_task.h"
#include "co_utils.h"
#include "prf_types.h"
#include "prf_utils.h"
#include "sdp_service_task.h"
#include "appm.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#define INVALID_STATUS  (0xFFFF)
#define FREE_STATUS     (0xFFFF)
#define USED_STATUS     (0xA5A5)


#define BLE_NB_PROFILES_ADD_MAX (BLE_NB_PROFILES - APPM_SVC_LIST_STOP)
/*
 * ENUMERATIONS
 ****************************************************************************************
 */

enum
{
       
    SDP_SERVICE_NEED_DISCOVER,
    
    SDP_SERVICE_NO_NEED_DISCOVER,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
 
 
struct sdp_chars_descs_inf
{
    struct prf_char_inf *chars_inf;
    
    //struct prf_char_desc_inf *descs;
    struct prf_char_desc_inf *descs_inf;
};
///Structure containing the characteristics handles, value handles and descriptors
struct sdp_content
{
    /// service info
    struct prf_svc svc;

    /// Characteristic Info:
    uint8_t chars_nb;
    uint8_t char_idx;    
    /// Descriptor handles:
    uint8_t descs_nb;   
    
    struct sdp_chars_descs_inf chars_descs_inf;
     
};


struct prf_sdp_db_env
{
	 ///index of  instances added
	uint8_t prf_idx;
   
    struct sdp_content *sdp_cont;
    
};

/// SDP 'Profile' Client environment variable
struct sdp_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    
    uint8_t conidx;	
    
    uint16_t use_status;
		/// on-going operation
    struct ke_msg * operation;
    /// Environment variable pointer for db
    struct prf_sdp_db_env  *prf_db_env;
    /// State of different task instances
    ke_state_t state[SDP_IDX_MAX];
};


struct sdp_env_init_t
{
    uint16_t used_status[BLE_NB_PROFILES_ADD_MAX];
    uint8_t sdp_need_dis_flag[BLE_NB_PROFILES_ADD_MAX];
    struct sdp_env_tag sdp_env[BLE_NB_PROFILES_ADD_MAX];
};


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern struct sdp_env_init_t sdp_env_init;
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Retrieve BAS client profile interface
 *
 * @return BAS client profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* sdp_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send ATT DB discovery results to BASC host.
 ****************************************************************************************
 */
void sdp_discover_service(uint8_t conidx,uint8_t *uuid,uint8_t uuid_len);
void sdp_discover_all_service(uint8_t conidx);
void sdp_service_init(void);
void sdp_add_profiles(uint8_t conidx,struct prf_sdp_db_env *env);
uint8_t sdp_extract_svc_info(uint8_t conidx,struct gattc_sdp_svc_ind const *param);

void sdp_enable_rsp_send(struct sdp_env_tag *basc_env, uint8_t conidx, uint8_t status);

uint8_t check_enough_mem_add_service(uint32_t size);

#endif /* (BLE_BATT_CLIENT) */

/// @} BASC

#endif /* _BASC_H_ */
