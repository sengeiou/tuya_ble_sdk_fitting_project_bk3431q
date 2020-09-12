/**
 * \file tuya_ble_secure.c
 *
 * \brief 
 */
/*
 *  Copyright (C) 2014-2019, Tuya Inc., All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of tuya ble sdk 
 */
#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"
#include "tuya_ble_port.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_event_handler.h"


#define TUYA_BLE_LOG_ERROR(...) 


static uint8_t service_rand[16] = {0};


static bool tuya_ble_device_id_key_generate(uint8_t *key_in,uint16_t key_len,uint8_t *key_out);


static bool tuya_ble_generate_key1(uint8_t *input,uint16_t input_len,uint8_t *output,tuya_ble_parameters_settings_t *current_para)
{
    //
    uint8_t key[16];
    uint8_t iv[16];

    if(input_len%16)
    {
        return false;
    }

    memcpy(key,current_para->auth_settings.auth_key,16);
    memset(iv,0,16);

    tuya_ble_aes128_cbc_encrypt(key,iv,input,input_len,output);
    //
    return true;
}


bool tuya_ble_register_key_generate(uint8_t *output,tuya_ble_parameters_settings_t *current_para)
{

    //
    if(!tuya_ble_aes128_ecb_encrypt(current_para->auth_settings.auth_key,service_rand,16,output))
    {
        TUYA_BLE_LOG_ERROR("register key generate failed!");
        return false;
    }
    else
    {
        return true;
    }

}

bool tuya_ble_device_id_encrypt(uint8_t *key_in,uint16_t key_len,uint8_t *input,uint16_t input_len,uint8_t *output)
{
    uint8_t key[16];
    uint8_t iv[16];
    //
    if(!tuya_ble_device_id_key_generate(key_in,key_len,key))
    {
        TUYA_BLE_LOG_ERROR("device id key generate error!");
        return false;
    }
    else
    {
        memcpy(iv,key,16);
        //TUYA_BLE_LOG_HEXDUMP_DEBUG("device id key :",(u8*)key,16);//
    }

    if(input_len%16)
    {
        return false;
    }


    tuya_ble_aes128_cbc_encrypt(key,iv,input,input_len,output);

    return true;
}




#define KEY_IN_BUFFER_LEN_MAX 64


static bool tuya_ble_key_generate(uint8_t mode,uint8_t *key_out,tuya_ble_parameters_settings_t *current_para,uint8_t *dev_rand)
{
    uint16_t len = 0;
    static uint8_t key_out_hex[16] = {0};
    static uint8_t key_out_key1[48] = {0};
    static uint8_t key_in_buffer[KEY_IN_BUFFER_LEN_MAX] = {0};

    if(mode>=ENCRYPTION_MODE_MAX)
    {
        return false;
    }
    if(mode==ENCRYPTION_MODE_NONE)
    {
        return true;
    }
    
    memset(key_in_buffer,0,sizeof(key_in_buffer));
    memset(key_out_key1,0,sizeof(key_out_key1));
    
    len = 0;
    memcpy(key_in_buffer+len,current_para->auth_settings.auth_key,AUTH_KEY_LEN);
    len += AUTH_KEY_LEN;
    
    if((mode==ENCRYPTION_MODE_KEY_1)||(mode==ENCRYPTION_MODE_KEY_2)||(mode==ENCRYPTION_MODE_KEY_3))
    {
        memcpy(key_in_buffer+len,service_rand,16);
        len += 16;
        if(!tuya_ble_generate_key1(key_in_buffer,AUTH_KEY_LEN+16,key_out_key1,current_para))
        {
            TUYA_BLE_LOG_ERROR("tuya_aes_generate_key1 failed.");
            return false;
        }
        
        memcpy(key_out_hex,key_out_key1+32,16);
        
        if(mode==ENCRYPTION_MODE_KEY_1)
        {
            memcpy(key_out,key_out_hex,16);
            return true;
        }

    }

    memset(key_in_buffer,0,sizeof(key_in_buffer));
    if((mode!=ENCRYPTION_MODE_KEY_1)&&(mode!=ENCRYPTION_MODE_KEY_4))
    {
        if(memcmp(dev_rand,key_in_buffer,PAIR_RANDOM_LEN)==0)
        {
            return false;
        }
    }
    if(mode==ENCRYPTION_MODE_KEY_3)
    {
        if(memcmp(current_para->sys_settings.user_rand,key_in_buffer,PAIR_RANDOM_LEN)==0)  //没有 user_rand
        {
            return false;
        }
    }

    if((16+PAIR_RANDOM_LEN*2)>KEY_IN_BUFFER_LEN_MAX)
    {
        return false;
    }
    
    len = 0;
    switch(mode)
    {
    case ENCRYPTION_MODE_KEY_2:
        memcpy(key_in_buffer,key_out_hex,16);
        len += 16;
        memcpy(key_in_buffer+len,dev_rand,PAIR_RANDOM_LEN);
        len += PAIR_RANDOM_LEN;
        break;
    case ENCRYPTION_MODE_KEY_3:
        memcpy(key_in_buffer,key_out_hex,16);
        len += 16;
        memcpy(key_in_buffer+len,dev_rand,PAIR_RANDOM_LEN);
        len += PAIR_RANDOM_LEN;
        memcpy(key_in_buffer+len,current_para->sys_settings.user_rand,PAIR_RANDOM_LEN);
        len += PAIR_RANDOM_LEN;
        break;
    case ENCRYPTION_MODE_KEY_4:
        memcpy(key_in_buffer+len,current_para->sys_settings.login_key,LOGIN_KEY_LEN);
        len += LOGIN_KEY_LEN;
        break;
    case ENCRYPTION_MODE_SESSION_KEY:
        memcpy(key_in_buffer+len,current_para->sys_settings.login_key,LOGIN_KEY_LEN);
        len += LOGIN_KEY_LEN;
        memcpy(key_in_buffer+len,dev_rand,PAIR_RANDOM_LEN);
        len += PAIR_RANDOM_LEN;
        break;
    default:
        break;
    };
    if(len==0)
    {
        return false;
    }

    memset(key_out_hex,0,sizeof(key_out_hex));

    tuya_ble_md5_crypt(key_in_buffer,len,key_out_hex);

    memcpy(key_out,key_out_hex,16);

    return true;

}


static bool tuya_ble_device_id_key_generate(uint8_t *key_in,uint16_t key_len,uint8_t *key_out)
{
    uint16_t len = 0;

    uint8_t device_id_key_in_buffer[64] = {0};

    memset(device_id_key_in_buffer,0,sizeof(device_id_key_in_buffer));
    len = 0;

    memcpy(device_id_key_in_buffer+len,key_in,key_len);
    len += key_len;

    tuya_ble_md5_crypt(device_id_key_in_buffer,len,key_out);

    return true;
}


static uint16_t tuya_ble_Add_Pkcs(uint8_t *p, uint16_t len)
{
    uint8_t pkcs[16];
    uint16_t out_len;

    if(len%16==0)
    {
        out_len = len;
    }
    else
    {
        uint16_t i = 0, cz = 16-len%16;
        memset(pkcs, 0, 16);
        for( i=0; i<cz; i++ ) {
            pkcs[i]=cz;
        }
        memcpy(p + len, pkcs, cz);
        out_len = len + cz;
    }
    return (out_len);
}

static void hextoascii(uint8_t *hexbuf,uint8_t len,uint8_t *ascbuf)
{
    uint8_t i =0,j =0,temp = 0;

    for(i = 0; i<len; i++) {
        temp = (hexbuf[i]>>4)&0xf;
        if(temp <=9) {
            ascbuf[j] = temp + 0x30;
        }
        else {
            ascbuf[j] = temp + 87;
        }
        j++;
        temp = (hexbuf[i])&0xf;
        if(temp <=9) {
            ascbuf[j] = temp + 0x30;
        }
        else {
            ascbuf[j] = temp + 87;
        }
        j++;
    }
    ascbuf[j] = 0x0;
}

uint8_t  tuya_ble_encrypt_old_with_key(uint8_t *key,uint8_t *in_buf,uint8_t in_len,uint8_t *out_buf)
{
    uint16_t i = 0,tmplen = 0;
    if(in_len>((256)/2-1))
        return 1;//

    hextoascii(in_buf,in_len,out_buf+1);
    tmplen = tuya_ble_Add_Pkcs(out_buf+1,in_len*2+1);

    if(tmplen>255)
        return 2;//

    //tuya_log_v("tmplen=%d-%d",in_len,tmplen);
    tuya_ble_aes128_ecb_encrypt(key,&out_buf[1+i],tmplen,&out_buf[1+i]);
    out_buf[0] = tmplen;

    return 0;
}



uint8_t tuya_ble_encryption(uint8_t encryption_mode,uint8_t *iv,uint8_t *in_buf,uint32_t in_len,uint32_t *out_len,uint8_t *out_buf,
tuya_ble_parameters_settings_t *current_para_data,uint8_t *dev_rand)
{
    uint16_t len = 0;
    uint8_t key[16];
    bool key_ok = false;

    if(encryption_mode>=ENCRYPTION_MODE_MAX)
    {
        return 2;//
    }

    if(encryption_mode==ENCRYPTION_MODE_NONE)
    {
        memcpy(out_buf,in_buf,in_len);
        *out_len = in_len;
        return 0;
    }
    else
    {
        len = tuya_ble_Add_Pkcs(in_buf,in_len);
    }

    memset(key,0,sizeof(key));
    key_ok = tuya_ble_key_generate(encryption_mode,key,current_para_data,dev_rand);

    if(key_ok)
    {

        if(tuya_ble_aes128_cbc_encrypt(key,iv,in_buf,len,out_buf)==true)
        {
            *out_len = len;
            //  
            return 0;//
        }
        else
        {
            return 3;//
        }
    }
    else
    {
        TUYA_BLE_LOG_ERROR("key error!");
        return 4; //key failed
    }

}


uint8_t tuya_ble_decryption(uint8_t const *in_buf,uint32_t in_len,uint32_t *out_len,uint8_t *out_buf,
    tuya_ble_parameters_settings_t *current_para_data,uint8_t *dev_rand)
{
    uint16_t len = 0;
    uint8_t key[16];
    uint8_t IV[16];
    uint8_t mode = 0;
    if(in_len<17)
    {
        return 1;  //
    }
    if(in_buf[0]>=ENCRYPTION_MODE_MAX)
    {
        return 2;//
    }

    if(in_buf[0]==ENCRYPTION_MODE_NONE)
    {
        len = in_len - 1;
        memcpy(out_buf,in_buf+1,len);
        *out_len = len;
        return 0;
    }

    len = in_len-17;

    memset(key,0,sizeof(key));
    memset(IV,0,sizeof(IV));
    mode = in_buf[0];

    if(mode==ENCRYPTION_MODE_KEY_1)
    {
        memcpy(service_rand,in_buf+1,16); //iv==rand
    }
    if(tuya_ble_key_generate(mode,key,current_para_data,dev_rand))
    {

        memcpy(IV,in_buf+1,16);

        if(tuya_ble_aes128_cbc_decrypt(key,IV,(uint8_t *)(in_buf+17),len,out_buf))
        {
 
            *out_len = len;
            return 0;//
        }
        else
        {
            return 3;//
        }
    }
    else
    {

        return 4; //
    }

}


void tuya_ble_event_process(tuya_ble_evt_param_t *tuya_ble_evt)
{
    switch(tuya_ble_evt->hdr.event)
    {
    case TUYA_BLE_EVT_DEVICE_INFO_UPDATE:
        tuya_ble_handle_device_info_update_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_DP_DATA_REPORTED:
        tuya_ble_handle_dp_data_reported_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_FACTORY_RESET:
        tuya_ble_handle_factory_reset_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_OTA_RESPONSE:
        tuya_ble_handle_ota_response_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_DATA_PASSTHROUGH:
        tuya_ble_handle_data_passthrough_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_PRODUCTION_TEST_RESPONSE:
        tuya_ble_handle_data_prod_test_response_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_MTU_DATA_RECEIVE:
        // TUYA_BLE_LOG("ble receive data event\n");
        // tuya_ble_commData_send_mtu(tuya_evt.ble_rev_event.data,tuya_evt.ble_rev_event.len);
        //TUYA_BLE_LOG_HEXDUMP_DEBUG("received ble mtu data",(uint8_t*)tuya_ble_evt->mtu_data.data,tuya_ble_evt->mtu_data.len);//
        tuya_ble_handle_ble_data_evt(tuya_ble_evt->mtu_data.data,tuya_ble_evt->mtu_data.len);
        break;
    case TUYA_BLE_EVT_DP_DATA_WITH_TIME_REPORTED:
        tuya_ble_handle_dp_data_with_time_reported_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_DP_DATA_WITH_TIME_STRING_REPORTED:
        tuya_ble_handle_dp_data_with_time_string_reported_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_UART_CMD:
        tuya_ble_handle_uart_cmd_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_BLE_CMD:
        tuya_ble_handle_ble_cmd_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_NET_CONFIG_RESPONSE:
        tuya_ble_handle_net_config_response_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_CUSTOM:
        tuya_ble_evt->custom_evt.custom_event_handler(tuya_ble_evt->custom_evt.evt_id,tuya_ble_evt->custom_evt.data);
        break;
    case TUYA_BLE_EVT_CONNECT_STATUS_UPDATE:
        tuya_ble_handle_connect_change_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_TIME_REQ:
        tuya_ble_handle_time_request_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_UNBOUND_RESPONSE:
        tuya_ble_handle_unbound_response_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_ANOMALY_UNBOUND_RESPONSE:
        tuya_ble_handle_anomaly_unbound_response_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_DEVICE_RESET_RESPONSE:
        tuya_ble_handle_device_reset_response_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_CONNECTING_REQUEST:
        tuya_ble_handle_connecting_request_evt(tuya_ble_evt);
        break;
	case TUYA_BLE_EVT_GATT_SEND_DATA:
        tuya_ble_evt->hdr.event_handler((void *)tuya_ble_evt);
        break;

    default:
        break;
    }
}


