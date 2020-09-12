#include <string.h>
#include "uart.h"
#include "gpio.h"
#include "master_app.h"
#include "interface.h"
uint8_t char_to_num(uint8_t ch);
uint8_t char_to_hex(uint8_t *ch);
extern volatile unsigned char uart_rx_buf[];
extern uint32_t con_peer_idx;
extern struct gap_bdaddr  con_bdaddr;


void interface_data_pro(uint8_t *buf,uint8_t len)
{
    /*
    UART_PRINTF("interface_data_pro\r\n");
    uint8_t param_idx;
    if( strncmp((char*)buf,SCAN_OPEN,sizeof(SCAN_OPEN)-1) == 0 )
    {
        UART_PRINTF("SCAN OPEN OK!!!\r\n");
        appm_start_scanning();
    }
    else if(strncmp((char*)buf,SCAN_CLOSE,sizeof(SCAN_CLOSE)-1) == 0 )
    {
        UART_PRINTF("SCAN_CLOSE OK!!!\r\n");
        appm_stop_scanning();
    }
    else if(strncmp((char*)buf,CON_LIST,sizeof(CON_LIST)-1) == 0 )
    {
        uint8_t param;
        param_idx = sizeof(CON_LIST)- 1;
        param = char_to_hex(&buf[param_idx]);
        con_peer_idx = param;
        appm_start_connencting(con_bdaddr);
        UART_PRINTF("CON_LIST OK!!!,param_idx = 0x%x,param = 0x%x\r\n",param_idx,param);
    }
    else if(strncmp((char*)buf,CON_ADDR,sizeof(CON_ADDR)-1) == 0 ) //CMD+CON+ADDR+addr_type+addr
    {
        uint8_t addr[6];
        param_idx = sizeof(CON_ADDR)- 1;
        con_bdaddr.addr_type = char_to_hex(&buf[param_idx]);
        param_idx += 3;
        UART_PRINTF("CON_ADDR OK!!!,param_idx = 0x%x,con_bdaddr.addr_type = 0x%x\r\n",param_idx,con_bdaddr.addr_type);
        UART_PRINTF("addr = 0x");
        for(int i = 0; i < 6; i ++)
        {
            addr[i] = char_to_hex(&buf[param_idx]);
            con_bdaddr.addr.addr[i] = char_to_hex(&buf[param_idx]);
            param_idx+=2;
            UART_PRINTF("%x ",addr[i]);
        }
        UART_PRINTF("\r\n");
        appm_start_connencting(con_bdaddr);
    }
    else if(strncmp((char*)buf,PRF_WR_DATA,sizeof(PRF_WR_DATA)-1) == 0 )//CMD+PRF+WRD+uuid16+len+data" // uuid 16
    {
        uint8_t len;
        uint8_t w_data[20];
        uint16_t uuid16;
        param_idx = sizeof(PRF_WR_DATA)- 1;
        UART_PRINTF("PRF_WR_DATA OK!!!,param_idx = 0x%x\r\n",param_idx);
        uuid16 = char_to_hex(&buf[param_idx]) << 8 | char_to_hex(&buf[param_idx + 2]);
        param_idx+=5;
        UART_PRINTF("uuid16 = 0x%x\r\n",uuid16);
        len = char_to_hex(&buf[param_idx]);
        param_idx+=3;
        UART_PRINTF("len = 0x%x\r\n",len);
        UART_PRINTF("data = 0x");
        for(int i = 0; i < len; i++)
        {
            w_data[i] = char_to_hex(&buf[param_idx]);
            UART_PRINTF("%x ",w_data[i]);
            param_idx+=2;
        }
        UART_PRINTF("\r\n");
        appm_write_uuid_data_req(uuid16,len,w_data);
    }
    else if(strncmp((char*)buf,PRF_WR_CFG,sizeof(PRF_WR_CFG)-1) == 0 )//CMD+PRF+WRC+uuid16+len+data" // uuid 16
    {
        uint8_t len;
        uint16_t uuid16;
        uint8_t cfg;
        param_idx = sizeof(PRF_WR_CFG)- 1;
        UART_PRINTF("PRF_WR_CFG OK!!!,param_idx = 0x%x\r\n",param_idx);
        uuid16 = char_to_hex(&buf[param_idx]) << 8 | char_to_hex(&buf[param_idx + 2]);
        param_idx+=5;
        UART_PRINTF("uuid16 = 0x%x\r\n",uuid16);
        len = char_to_hex(&buf[param_idx]);
        param_idx+=3;
        UART_PRINTF("len = 0x%x\r\n",len);
        cfg = char_to_hex(&buf[param_idx]);
        UART_PRINTF("data = 0x");
        UART_PRINTF("%s\r\n",&buf[param_idx]);
        UART_PRINTF("\r\n");
        appm_write_ntf_cfg_req(uuid16,cfg);
    }
    else if(strncmp((char*)buf,PRF_RD_DATA,sizeof(PRF_RD_DATA)-1) == 0 )//CMD+PRF+RDD+uuid16" // uuid 16
    {
        uint16_t uuid16;
        param_idx = sizeof(PRF_RD_DATA)- 1;
        UART_PRINTF("PRF_RD_DATA OK!!!,param_idx = 0x%x\r\n",param_idx);
        uuid16 = char_to_hex(&buf[param_idx]) << 8 | char_to_hex(&buf[param_idx + 2]);
        UART_PRINTF("uuid16 = 0x%x\r\n",uuid16);
        appm_read_uuid_data_req(uuid16);
    }
    else if(strncmp((char*)buf,PRF_RD_CFG,sizeof(PRF_RD_CFG)-1) == 0 )//CMD+PRF+RDC+uuid16" // uuid 16
    {
        uint16_t uuid16;
        param_idx = sizeof(PRF_RD_CFG)- 1;
        UART_PRINTF("PRF_RD_CFG OK!!!,param_idx = 0x%x\r\n",param_idx);
        uuid16 = char_to_hex(&buf[param_idx]) << 8 | char_to_hex(&buf[param_idx + 2]);
        UART_PRINTF("uuid16 = 0x%x\r\n",uuid16);
        appm_read_uuid_cfg_req(uuid16);
    }
    else if(strncmp((char*)buf,PRF_RD_DESC,sizeof(PRF_RD_DESC)-1) == 0 )//CMD+PRF+RDS+uuid16" // uuid 16
    {
        uint16_t uuid16;
        param_idx = sizeof(PRF_RD_DESC)- 1;
        UART_PRINTF("PRF_RD_DESC OK!!!,param_idx = 0x%x\r\n",param_idx);
        uuid16 = char_to_hex(&buf[param_idx]) << 8 | char_to_hex(&buf[param_idx + 2]);
        UART_PRINTF("uuid16 = 0x%x\r\n",uuid16);
        appm_read_uuid_userDesc_req(uuid16);
    }
    {
        UART_PRINTF("%s\r\n",buf);
    }*/
}
 
uint8_t char_to_num(uint8_t ch)
{
    uint8_t value;
    if( (ch>='0')&&(ch<='9'))
    {
        value=(ch-'0');
    }
    else if( (ch>='a')&&(ch<='f'))
    {
        value=(ch-'a'+10);
    }
    else if ( (ch>='A')&&(ch<='F'))
    {
        value=(ch-'A'+10);
    }
    return value;
}

uint8_t char_to_hex(uint8_t *ch)
{
    uint8_t value;
    value = (char_to_num(ch[0])<< 4) | (char_to_num(ch[1]));
    return value;
}


