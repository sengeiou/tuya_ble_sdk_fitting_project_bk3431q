#include "app_ota.h"




/*********************************************************************
 * LOCAL CONSTANTS
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static volatile uint8_t  s_ota_state = TUYA_BLE_OTA_REQ;
static volatile int32_t  s_pkg_id;
static uint32_t s_data_len;
static uint32_t s_data_crc;
static volatile bool s_ota_success = false;
//file info
static app_ota_file_info_storage_t s_file;
static app_ota_file_info_storage_t s_old_file;

/*********************************************************************
 * LOCAL FUNCTION
 */
static uint32_t app_ota_enter(void);
static uint32_t app_ota_exit(void);
static uint32_t app_ota_get_crc32_in_flash(uint32_t len);
//static void app_ota_setting_write_complete_cb(nrf_fstorage_evt_t* p_evt);
static uint32_t app_ota_req_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp);
static uint32_t app_ota_file_info_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp);
static uint32_t app_ota_file_offset_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp);
static uint32_t app_ota_data_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp);
static uint32_t app_ota_end_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp);
static void app_ota_timer_creat_and_start(void);

/*********************************************************************
 * VARIABLES
 */




/*********************************************************
FN: 
*/
uint32_t app_ota_init(void)
{
    return 0;
}

/*********************************************************
FN: 
*/
void app_ota_handler(tuya_ble_ota_data_t* ota)
{
    tuya_ble_ota_response_t rsp;
    rsp.type = ota->type;
    
    if(ota->type != TUYA_BLE_OTA_DATA)
    {
        SUBLE_PRINTF("ota_cmd_type: %d", ota->type);
        SUBLE_HEXDUMP("ota_cmd_data", ota->p_data, ota->data_len);
    }
    
    switch(ota->type)
    {
        case TUYA_BLE_OTA_REQ: {
            app_ota_req_handler(ota->p_data, ota->data_len, &rsp);
        } break;
        
        case TUYA_BLE_OTA_FILE_INFO: {
            app_ota_file_info_handler(ota->p_data, ota->data_len, &rsp);
        } break;
        
        case TUYA_BLE_OTA_FILE_OFFSET_REQ: {
            app_ota_file_offset_handler(ota->p_data, ota->data_len, &rsp);
        } break;
        
        case TUYA_BLE_OTA_DATA: {
            app_ota_data_handler(ota->p_data, ota->data_len, &rsp);
        } break;
        
        case TUYA_BLE_OTA_END: {
            app_ota_end_handler(ota->p_data, ota->data_len, &rsp);
        } break;
        
        case TUYA_BLE_OTA_UNKONWN: {
        } break;
        
        default: {
        } break;
    }
}

/*********************************************************
FN: 
*/
static uint32_t app_ota_enter(void)
{
    s_pkg_id = -1;
    s_data_len = 0;
    s_data_crc = 0;
    s_ota_success = false;
    memset(&s_file, 0x00, sizeof(app_ota_file_info_storage_t));
    memset(&s_old_file, 0x00, sizeof(app_ota_file_info_storage_t));
    
    suble_gap_conn_param_update(g_conn_info[0].condix, 15, 30, 0, 5000);
//    app_port_ble_conn_evt_ext();
    
    suble_flash_erase(APP_OTA_START_ADDR, APP_OTA_FILE_MAX_LEN/0x1000);
    
    return SUBLE_SUCCESS;
}

/*********************************************************
FN: 
*/
static uint32_t app_ota_exit(void)
{
    app_ota_timer_creat_and_start();
    s_ota_state = TUYA_BLE_OTA_REQ;
    return SUBLE_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t app_ota_get_ota_state(void)
{
    return s_ota_state;
}

/*********************************************************
FN: 
*/
uint32_t app_ota_disconn_handler(void)
{
    if(s_ota_state > TUYA_BLE_OTA_REQ) {
        return app_ota_exit();
    } else {
        return 0;
    }
}

/*********************************************************
FN: 
*/
static uint32_t app_ota_get_crc32_in_flash(uint32_t len)
{
    static uint8_t buf[APP_OTA_PKG_LEN];
    
    if(len == 0)
    {
        return 0;
    }
    
    uint32_t crc_temp = 0;
    uint32_t read_addr = APP_OTA_START_ADDR;
    uint32_t cnt = len/APP_OTA_PKG_LEN;
    uint32_t remainder = len%APP_OTA_PKG_LEN;
    
    for(uint32_t idx=0; idx<cnt; idx++)
    {
        tuya_ble_nv_read(read_addr, buf, APP_OTA_PKG_LEN);
        crc_temp = suble_util_crc32(buf, APP_OTA_PKG_LEN, &crc_temp);
        read_addr += APP_OTA_PKG_LEN;
    }

    if(remainder > 0)
    {
        tuya_ble_nv_read(read_addr, buf, APP_OTA_PKG_LEN);
        crc_temp = suble_util_crc32(buf, remainder, &crc_temp);
        read_addr += remainder;
    }
    
    return crc_temp;
}

/*********************************************************
FN: 
*/
static uint32_t app_ota_rsp(tuya_ble_ota_response_t* rsp, void* rsp_data, uint16_t data_size)
{
    rsp->p_data = rsp_data;
    rsp->data_len = data_size;
    return tuya_ble_ota_response(rsp);
}

/*********************************************************
FN: 
*/
static uint32_t app_ota_req_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp)
{
    //param check
    if(s_ota_state != TUYA_BLE_OTA_REQ)
    {
        SUBLE_PRINTF("Error: TUYA_BLE_OTA_REQ- s_ota_state error");
        //rsp
        app_ota_req_rsp_t req_rsp;
        memset(&req_rsp, 0x00, sizeof(app_ota_req_rsp_t));
        req_rsp.flag = 0x01; //refuse ota
        
        app_ota_rsp(rsp, &req_rsp, sizeof(app_ota_req_rsp_t));
        app_ota_exit();
        return SUBLE_ERROR_COMMON;
    }
    
    //param check
    if((cmd_size != 0x0001) || (*cmd != 0x00))
    {
        SUBLE_PRINTF("Error: TUYA_BLE_OTA_REQ- param error");
        //rsp
        app_ota_req_rsp_t req_rsp;
        memset(&req_rsp, 0x00, sizeof(app_ota_req_rsp_t));
        req_rsp.flag = 0x01; //refuse ota
        
        app_ota_rsp(rsp, &req_rsp, sizeof(app_ota_req_rsp_t));
        app_ota_exit();
        return SUBLE_ERROR_COMMON;
    }
    
    {
        app_ota_enter();

        //rsp
        app_ota_req_rsp_t req_rsp;
        memset(&req_rsp, 0x00, sizeof(app_ota_req_rsp_t));
        req_rsp.flag = 0x00; //accept ota
        req_rsp.ota_version = APP_OTA_VERSION;
        req_rsp.type = 0x00; //firmware info
        req_rsp.version = TUYA_DEVICE_FVER_NUM;
        suble_util_reverse_byte(&req_rsp.version, sizeof(uint32_t));
        req_rsp.package_maxlen = APP_OTA_PKG_LEN;
        suble_util_reverse_byte(&req_rsp.package_maxlen, sizeof(uint16_t));
        
        app_ota_rsp(rsp, &req_rsp, sizeof(app_ota_req_rsp_t));
        s_ota_state = TUYA_BLE_OTA_FILE_INFO;
    }
    return SUBLE_SUCCESS;
}

/*********************************************************
FN: 
*/
static uint32_t app_ota_file_info_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp)
{
    //param check
    if(s_ota_state != TUYA_BLE_OTA_FILE_INFO)
    {
        SUBLE_PRINTF("Error: TUYA_BLE_OTA_FILE_INFO- s_ota_state error");
        //rsp none
        app_ota_exit();
        return SUBLE_ERROR_COMMON;
    }

    //param check
    app_ota_file_info_t* file_info = (void*)cmd;
    if(file_info->type != 0x00)
    {
        SUBLE_PRINTF("Error: TUYA_BLE_OTA_FILE_INFO- file_info->type error");
        //rsp none
        app_ota_exit();
        return SUBLE_ERROR_COMMON;
    }
    
    {
        //file info
        suble_util_reverse_byte(&file_info->version, sizeof(uint32_t));
        suble_util_reverse_byte(&file_info->file_len, sizeof(uint32_t));
        suble_util_reverse_byte(&file_info->crc32, sizeof(uint32_t));
        s_file.len = file_info->file_len;
        s_file.crc32 = file_info->crc32;
        memcpy(s_file.md5, file_info->md5, APP_OTA_FILE_MD5_LEN);
        
        //rsp
        app_ota_file_info_rsp_t file_info_rsp;
        memset(&file_info_rsp, 0x00, sizeof(app_ota_file_info_rsp_t));
        file_info_rsp.type = 0x00; //firmware info
        if(memcmp(file_info->pid, TUYA_DEVICE_PID, 8)) {
            file_info_rsp.state = 0x01; //pid error
        }
        else if(file_info->version <= TUYA_DEVICE_FVER_NUM) {
            file_info_rsp.state = 0x02; //version error
        }
        else if(file_info->file_len > APP_OTA_FILE_MAX_LEN) {
            file_info_rsp.state = 0x03; //size error
        } else {
            file_info_rsp.state = 0x00;
            s_ota_state = TUYA_BLE_OTA_FILE_OFFSET_REQ;
        }
        
        file_info_rsp.old_file_len = s_old_file.len;
        suble_util_reverse_byte(&file_info_rsp.old_file_len, sizeof(uint32_t));
        file_info_rsp.old_crc32 = s_old_file.crc32;
        suble_util_reverse_byte(&file_info_rsp.old_crc32, sizeof(uint32_t));
        memset(file_info_rsp.old_md5, 0x00, APP_OTA_FILE_MD5_LEN);
        app_ota_rsp(rsp, &file_info_rsp, sizeof(app_ota_file_info_rsp_t));
        
        if(file_info_rsp.state != 0x00) {
            SUBLE_PRINTF("Error: TUYA_BLE_OTA_FILE_INFO- errorid: %d", file_info_rsp.state);
            app_ota_exit();
        }
    }
    return SUBLE_SUCCESS;
}

/*********************************************************
FN: 
*/
static uint32_t app_ota_file_offset_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp)
{
    //param check
    if(s_ota_state != TUYA_BLE_OTA_FILE_OFFSET_REQ)
    {
        SUBLE_PRINTF("Error: TUYA_BLE_OTA_FILE_OFFSET_REQ- s_ota_state error");
        //rsp none
        app_ota_exit();
        return SUBLE_ERROR_COMMON;
    }

    //param check
    app_ota_file_offset_t* file_offset = (void*)cmd;
    if(file_offset->type != 0x00)
    {
        SUBLE_PRINTF("Error: TUYA_BLE_OTA_FILE_OFFSET_REQ- file_offset->type error");
        //rsp none
        app_ota_exit();
        return SUBLE_ERROR_COMMON;
    }
    
    {
        suble_util_reverse_byte(&file_offset->offset, sizeof(uint32_t));
        
        //rsp
        app_ota_file_offset_rsp_t file_offset_rsp;
        memset(&file_offset_rsp, 0x00, sizeof(app_ota_file_offset_rsp_t));
        file_offset_rsp.type = 0x00;
        {
            if(file_offset->offset > 0)
            {
                uint8_t md5[APP_OTA_FILE_MD5_LEN];
                if((memcmp(md5, s_file.md5, APP_OTA_FILE_MD5_LEN) == 0) && (app_ota_get_crc32_in_flash(s_data_len) == s_data_crc) && (file_offset->offset >= s_data_len)) {
                    file_offset_rsp.offset = s_data_len;
//                    s_pkg_id = s_data_len/APP_OTA_PKG_LEN;//every time from zero
                } else {
                    file_offset_rsp.offset = 0;
                    s_data_len = 0;
                    s_data_crc = 0;
                }
            }
        }
        suble_util_reverse_byte(&file_offset_rsp.offset, sizeof(uint32_t));
        
        app_ota_rsp(rsp, &file_offset_rsp, sizeof(app_ota_file_offset_rsp_t));
        s_ota_state = TUYA_BLE_OTA_DATA;
    }
    return SUBLE_SUCCESS;
}

/*********************************************************
FN: 
*/
static uint32_t app_ota_data_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp)
{
    //param check
    if(s_ota_state != TUYA_BLE_OTA_DATA)
    {
        SUBLE_PRINTF("Error: TUYA_BLE_OTA_DATA- s_ota_state error");
        //rsp
        app_ota_data_rsp_t ota_data_rsp;
        memset(&ota_data_rsp, 0x00, sizeof(app_ota_data_rsp_t));
        ota_data_rsp.state = 0x04; //unknow error
        
        app_ota_rsp(rsp, &ota_data_rsp, sizeof(app_ota_data_rsp_t));
        app_ota_exit();
        return SUBLE_ERROR_COMMON;
    }

    //param check
    app_ota_data_t* ota_data = (void*)cmd;
    if(ota_data->type != 0x00)
    {
        SUBLE_PRINTF("Error: TUYA_BLE_OTA_DATA- ota_data->type error");
        //rsp
        app_ota_data_rsp_t ota_data_rsp;
        memset(&ota_data_rsp, 0x00, sizeof(app_ota_data_rsp_t));
        ota_data_rsp.state = 0x04; //unknow error
        
        app_ota_rsp(rsp, &ota_data_rsp, sizeof(app_ota_data_rsp_t));
        app_ota_exit();
        return SUBLE_ERROR_COMMON;
    }
    
    {
        suble_util_reverse_byte(&ota_data->pkg_id, sizeof(uint16_t));
        suble_util_reverse_byte(&ota_data->len, sizeof(uint16_t));
        suble_util_reverse_byte(&ota_data->crc16, sizeof(uint16_t));
        
        //rsp
        app_ota_data_rsp_t ota_data_rsp;
        memset(&ota_data_rsp, 0x00, sizeof(app_ota_data_rsp_t));
        ota_data_rsp.type = 0x00;
        if(s_pkg_id+1 != ota_data->pkg_id) {
            ota_data_rsp.state = 0x01; //package id error
        }
        else if(cmd_size-7 != ota_data->len) {
            ota_data_rsp.state = 0x02; //size error
        }
        else if(suble_util_crc16(ota_data->data, ota_data->len, NULL) != ota_data->crc16) {
            ota_data_rsp.state = 0x03; //crc error
        } else {
            ota_data_rsp.state = 0x00;
            
            suble_flash_write(APP_OTA_START_ADDR + s_data_len, ota_data->data, ota_data->len);
            {
                s_data_len += ota_data->len;
                if(s_data_len < s_file.len)
                {
                    SUBLE_PRINTF("s_pkg_id: %d", s_pkg_id);
                    s_ota_state = TUYA_BLE_OTA_DATA;
                }
                else if(s_data_len == s_file.len)
                {
                    s_ota_state = TUYA_BLE_OTA_END;
                }
                else
                {
                    ota_data_rsp.state = 0x04;
                }
                s_pkg_id++;
                
                s_data_crc = suble_util_crc32(ota_data->data, ota_data->len, &s_data_crc);
            }
        }
        app_ota_rsp(rsp, &ota_data_rsp, sizeof(app_ota_data_rsp_t));
        
        if(ota_data_rsp.state != 0x00) {
            SUBLE_PRINTF("Error: TUYA_BLE_OTA_DATA- errorid: %d", ota_data_rsp.state);
            app_ota_exit();
        }
    }
    return SUBLE_SUCCESS;
}

/*********************************************************
FN: 
*/
static uint32_t app_ota_end_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp)
{
    //param check
    if(s_ota_state != TUYA_BLE_OTA_END)
    {
        SUBLE_PRINTF("Error: TUYA_BLE_OTA_END- s_ota_state error");
        //rsp
        app_ota_end_rsp_t end_rsp;
        memset(&end_rsp, 0x00, sizeof(app_ota_end_rsp_t));
        end_rsp.state = 0x03; //unknow error
        
        app_ota_rsp(rsp, &end_rsp, sizeof(app_ota_end_rsp_t));
        app_ota_exit();
        return SUBLE_ERROR_COMMON;
    }

    //param check
    if((cmd_size != 0x0001) || (*cmd != 0x00))
    {
        SUBLE_PRINTF("Error: TUYA_BLE_OTA_END- type error");
        //rsp
        app_ota_end_rsp_t end_rsp;
        memset(&end_rsp, 0x00, sizeof(app_ota_end_rsp_t));
        end_rsp.state = 0x03; //unknow error
        
        app_ota_rsp(rsp, &end_rsp, sizeof(app_ota_end_rsp_t));
        app_ota_exit();
        return SUBLE_ERROR_COMMON;
    }
    
    {
        //rsp
        app_ota_end_rsp_t end_rsp;
        memset(&end_rsp, 0x00, sizeof(app_ota_end_rsp_t));
        end_rsp.type = 0x00;
        if(s_data_len != s_file.len)
        {
            end_rsp.state = 0x01; //total size error
        }
        else if(s_file.crc32 != app_ota_get_crc32_in_flash(s_data_len))
        {
            end_rsp.state = 0x02; //crc error
        }
        else
        {
            uint16_t image_len; //该值在bk的固件中标识固件长度（单位：4字节），在这里做检查，防止越界
            suble_flash_read(APP_OTA_START_ADDR+6, (void*)&image_len, sizeof(uint16_t));
            SUBLE_PRINTF("image_len: %dbyte", image_len*4);
            
            if(image_len > (APP_OTA_FILE_MAX_LEN/4)) {
                suble_flash_erase(APP_OTA_START_ADDR, APP_OTA_FILE_MAX_LEN/0x1000);
                end_rsp.state = 0x03; //unknow error
                SUBLE_PRINTF("APP_OTA_FILE_MAX_LEN ERROR");
            }
            else {
                end_rsp.state = 0x00;
                s_ota_success = true;
                SUBLE_PRINTF("ota success");
            }
        }
        app_ota_rsp(rsp, &end_rsp, sizeof(app_ota_end_rsp_t));
        
        app_ota_exit();
        
        if(end_rsp.state != 0x00) {
            SUBLE_PRINTF("Error: TUYA_BLE_OTA_END- errorid: %d", end_rsp.state);
            app_ota_exit();
        }
    }
    return SUBLE_SUCCESS;
}




/*********************************************************
FN: 
*/
tuya_ble_timer_t app_ota_timer;

static void reset_with_disconn_outtime_cb(tuya_ble_timer_t timer)
{
    suble_gap_disconnect(0, 0x16);
    suble_system_reset();
}


static void app_ota_timer_creat_and_start(void)
{
    tuya_ble_timer_create(&app_ota_timer, 2000, TUYA_BLE_TIMER_SINGLE_SHOT, reset_with_disconn_outtime_cb);
    tuya_ble_timer_start(app_ota_timer);
    
}













