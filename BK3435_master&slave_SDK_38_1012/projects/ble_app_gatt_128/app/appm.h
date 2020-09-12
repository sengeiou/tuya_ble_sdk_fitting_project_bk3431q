#ifndef APPM_H_
#define APPM_H_

#include "rwip_config.h"     // SW configuration
#if (BLE_APP_PRESENT)
#include <stdint.h>          // Standard Integer Definition
#include <co_bt.h>           // Common BT Definitions
#include "arch.h"            // Platform Definitions
#include "gapc.h"            // GAPC Definitions
#include "nvds.h"
#include "appm_task.h"

/*
 * DEFINES
 ****************************************************************************************
 */
/// Maximal length of the Device Name value
#define APP_DEVICE_NAME_MAX_LEN      NVDS_LEN_DEVICE_NAME///(18)

#define APPM_CONNENCT_DEVICE_CONTIU_TIME    500  // 4S

//重连次数
#define APPM_RECONNENCT_DEVICE_NUM   10


/*
enum appm_fields
{
    APPM_ADV_EN = 0,
    APPM_ADV_EN_MASK = 0x01,
    APPM_SCAN_EN = 1,
    APPM_SCAN_EN_MASK = 0x02,
};
*/

/// Set appm configuration field, 设置bit0-adv, bit1-scan
#define APPM_SET_FIELD(field, value) \
                            (appm_env.fields) = ((appm_env.fields) & (~APPM_##field##_MASK)) \
                                     | (((value) << APPM_##field) & (APPM_##field##_MASK))
/// Get appm configuration field, 查询bit0-adv, bit1-scan
#define APPM_GET_FIELD(field) \
                            (((appm_env.fields) & (APPM_##field##_MASK)) >> APPM_##field)

/// appm configuration field
#define APPM_FIELD_SAVE_CLEAN()  appm_env.fields_bck = appm_env.fields; \
                                 appm_env.fields = 0;
#define APPM_FIELD_RECOVER()    (appm_env.fields = appm_env.fields_bck)
#define APPM_FIELD_CLEAN()      (appm_env.fields = 0)


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
enum appm_error
{
    APPM_ERROR_NO_ERROR,
    APPM_ERROR_LINK_LOSS,
    APPM_ERROR_STATE,
    APPM_ERROR_NTFIND_DISABLE,
    APPM_ERROR_LINK_MAX
};    

/// List of service to add in the database
enum appm_svc_list
{
    APPM_SVC_FFF0,
    APPM_SVC_LIST_STOP ,
};

/// List of Application NVDS TAG identifiers
enum app_nvds_tag
{
    /// BLE Application Advertising data
    NVDS_TAG_APP_BLE_ADV_DATA           = 0x0B,
    NVDS_LEN_APP_BLE_ADV_DATA           = 32,
    /// BLE Application Scan response data
    NVDS_TAG_APP_BLE_SCAN_RESP_DATA     = 0x0C,
    NVDS_LEN_APP_BLE_SCAN_RESP_DATA     = 32,
    /// Mouse Sample Rate
    NVDS_TAG_MOUSE_SAMPLE_RATE          = 0x38,
    NVDS_LEN_MOUSE_SAMPLE_RATE          = 1,
    /// Peripheral Bonded
    NVDS_TAG_PERIPH_BONDED              = 0x39,
    NVDS_LEN_PERIPH_BONDED              = 1,
    /// Mouse NTF Cfg
    NVDS_TAG_MOUSE_NTF_CFG              = 0x3A,
    NVDS_LEN_MOUSE_NTF_CFG              = 2,
    /// Mouse Timeout value
    NVDS_TAG_MOUSE_TIMEOUT              = 0x3B,
    NVDS_LEN_MOUSE_TIMEOUT              = 2,
    /// Peer Device BD Address
    NVDS_TAG_PEER_BD_ADDRESS            = 0x3C,
    NVDS_LEN_PEER_BD_ADDRESS            = 7,
    /// Mouse Energy Safe
    NVDS_TAG_MOUSE_ENERGY_SAFE          = 0x3D,
    NVDS_LEN_MOUSE_SAFE_ENERGY          = 2,
    /// EDIV (2bytes), RAND NB (8bytes),  LTK (16 bytes), Key Size (1 byte)
    NVDS_TAG_LTK                        = 0x3E,
    NVDS_LEN_LTK                        = 28,
    /// PAIRING
    NVDS_TAG_PAIRING                    = 0x3F,
    NVDS_LEN_PAIRING                    = 54,
};

enum app_loc_nvds_tag
{
    /// Audio mode 0 task
    NVDS_TAG_AM0_FIRST                  = NVDS_TAG_APP_SPECIFIC_FIRST, // 0x90
    NVDS_TAG_AM0_LAST                   = NVDS_TAG_APP_SPECIFIC_FIRST+16, // 0xa0

    /// Local device Identity resolving key
    NVDS_TAG_LOC_IRK,
    /// Peer device Resolving identity key (+identity address)
    NVDS_TAG_PEER_IRK,

    /// size of local identity resolving key
    NVDS_LEN_LOC_IRK                    = KEY_LEN,
    /// size of Peer device identity resolving key (+identity address)
    NVDS_LEN_PEER_IRK                   = sizeof(struct gapc_irk),
};

///Advertising report structure
struct adv_rsp_report
{
    ///Event type:
    /// - ADV_CONN_UNDIR: Connectable Undirected advertising
    /// - ADV_CONN_DIR: Connectable directed advertising
    /// - ADV_DISC_UNDIR: Discoverable undirected advertising
    /// - ADV_NONCONN_UNDIR: Non-connectable undirected advertising
    uint8_t        evt_type;
    ///Advertising address type: public/random
    uint8_t        adv_addr_type;
    ///Advertising address value
    struct bd_addr adv_addr;
    ///Data length in advertising packet
    uint8_t        adv_data_len;
    ///Data of advertising packet
    uint8_t        adv_data[ADV_DATA_LEN];   
        ///Data length in advertising packet
    uint8_t        rsp_data_len;
    ///Data of advertising packet
    uint8_t        rsp_data[ADV_DATA_LEN];
    ///RSSI value for advertising packet
    int8_t        rssi;
};

/// Application environment structure
struct appm_env_tag
{
    /// Connection handle
    uint16_t conhdl;
    /// Connection Index
    uint8_t  conidx;
   
    /// Scan filtering Array
    struct adv_rsp_report* scan_filter;
    
    uint8_t recon_num; 
    /// Last initialized profile
    uint8_t next_svc; 
    
    uint8_t master_num; 
         
    uint8_t slave_num; 
    
    uint8_t adv_num;
    
    uint8_t scan_num; // min 3    
    
    uint16_t features;
    
    /// Configuration fields (@see enum appm_fields)
    uint16_t fields;
    uint16_t fields_bck;

    /// Bonding status
    bool bonded;

    /// Device Name length
    uint8_t dev_name_len;
    /// Device Name
    uint8_t dev_name[APP_DEVICE_NAME_MAX_LEN];

    /// Local device IRK
    uint8_t loc_irk[KEY_LEN];
        
    uint8_t con_dev_flag; 
    /// Address information about a device address
    struct gap_bdaddr con_dev_addr;
    
    uint8_t svc_uuid_len;
    /// UUID
    uint8_t svc_uuid[16]; 
    
    uint8_t write_uuid_len;
    /// UUID
    uint8_t write_uuid[16]; 
    
    uint8_t notif_uuid_len;
    /// UUID
    uint8_t notif_uuid[16]; 
	
};

enum appm_fields
{
    APPM_ADV_EN = 0,
    APPM_ADV_EN_MASK = 0x01,
    APPM_SCAN_EN = 1,
    APPM_SCAN_EN_MASK = 0x02,
};

enum appm_features
{
    APPM_SLAVE_MODE = 0x01,
    APPM_MASTER_MODE = 0x02,
    APPM_ALL_MODE = 0x03,
};


extern struct appm_env_tag appm_env;


void appm_init(void);
bool appm_add_svc(void);
void appm_start_advertising(void);
void appm_stop_advertising(void);
uint8_t appm_filter_adv_report(struct adv_report const *p_param);
uint8_t appm_get_dev_name(uint8_t* name);
void appm_scan_adv_con_schedule(void);

void appm_field_save_clean(void);
void appm_field_recover(void);

#endif //(BLE_APP_PRESENT)

#endif // APP_H_

