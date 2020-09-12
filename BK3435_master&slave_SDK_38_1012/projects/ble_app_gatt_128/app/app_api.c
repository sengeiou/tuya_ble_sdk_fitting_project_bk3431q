
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "rwble.h"
#include "reg_blecore.h"
#include "BK_reg_Protocol.h"
#include "ke_mem.h"
#include "ke_event.h"
#include "compiler.h"
#include "lld_util.h"
#include "llm_util.h"
#include "appm.h"
#include "appm_task.h"
#include "appc.h"
#include "appc_task.h"
#include "master_app.h"
#include "cli.h"
#include "wdt.h"
#include "nvds.h"
//#include "uart.h"
#include "icu.h"
#include "app_electric.h"
#include "app_api.h"
#include "spi.h"
#include "soft_spi.h"

ble_dev_info slave_device={
	0,   \
	0x11,0x22,0x33,0x44,0x55,0x66,  \
	0,  \
	0x79,0x41,0xDC,0x24,0x0E,0xE5,0xA9,0xE0,0x93,0xF3,0xA3,0xB5,0x01,0x00,0x40,0x6E,  \
	0x79,0x41,0xDC,0x24,0x0E,0xE5,0xA9,0xE0,0x93,0xF3,0xA3,0xB5,0x02,0x00,0x40,0x6E,  \
	0x79,0x41,0xDC,0x24,0x0E,0xE5,0xA9,0xE0,0x93,0xF3,0xA3,0xB5,0x03,0x00,0x40,0x6E,  \
};

uint8 ble_dev_name[32] = "BEKEN_BLE";
uint8 scan_rsp_data[32] = "\x0d\x08\x42\x4B\x33\x34\x33\x35\x2D\x47\x41\x54\x54\x34";///{0x0d,0x08,0x42,0x4B,0x33,0x34,0x33,0x35,0x2D,0x47,0x41,0x54,0x54,0x34};
uint8_t dmo_channel = 0;


void ble_name_ini(void)
{
    uint8_t flag = 0;
    
    if(APPM_GET_FIELD(ADV_EN))//if adv on set adv off
    {
        uint8_t  cur_state = ke_state_get(TASK_APPM);
        APPM_SET_FIELD(ADV_EN,0);
        appm_scan_adv_con_schedule();
        UART_PRINTF("adv set off cur_state:%d\r\n",cur_state);
        flag = 1;
    } 

    appm_env.dev_name_len = strlen((char *)ble_dev_name);
    memset(appm_env.dev_name,0,sizeof(appm_env.dev_name));
	memcpy(appm_env.dev_name, ble_dev_name, appm_env.dev_name_len);
    memset(scan_rsp_data,0,sizeof(scan_rsp_data));
	memcpy(scan_rsp_data+2, ble_dev_name, appm_env.dev_name_len);
	scan_rsp_data[0] = appm_env.dev_name_len + 1;//len
	scan_rsp_data[1] = 0x08; 
    
    if((!APPM_GET_FIELD(ADV_EN)) && flag)//adv on
    {
        uint8_t  cur_state = ke_state_get(TASK_APPM);
        UART_PRINTF("adv set on cur_state:%d\r\n",cur_state);
        APPM_SET_FIELD(ADV_EN,1);
        appm_scan_adv_con_schedule();
    }
}

enum appm_error set_ble_name(uint8_t *blename, uint8_t ble_len)
{
    extern struct appm_env_tag appm_env;
    uint8_t len = ble_len;
    
    
    if(ble_len > APP_DEVICE_NAME_MAX_LEN)return APPM_ERROR_STATE;
    if(strlen((const char *)blename) > APP_DEVICE_NAME_MAX_LEN)return APPM_ERROR_STATE;
    if(ble_len > strlen((const char *)blename))len = strlen((const char *)blename);
    
	appm_env.dev_name_len = len;
	//memcpy(appm_env.dev_name, blename, appm_env.dev_name_len);
	
	memset(ble_dev_name,0,sizeof(ble_dev_name));
	memcpy(ble_dev_name, blename, appm_env.dev_name_len);
    
   
    if(nvds_put(NVDS_TAG_DEVICE_NAME,APP_DEVICE_NAME_MAX_LEN,ble_dev_name) != NVDS_OK)return APPM_ERROR_STATE;
//    memset(ble_adv_data, 0x00, sizeof(ble_adv_data));
//    snprintf((char *)ble_adv_data, 10 + strlen((const char *)ble_dev_name),"\x02\x01\x06\x03\x03\xFF\xF0");
//    if(ble_len < 23)
//    {
//        ble_adv_data[7] = ble_len +1;
//        ble_adv_data[8] = 0x08;
//        memcpy(ble_adv_data + 9 , ble_dev_name, ble_len);
//    }
//	if(nvds_put(NVDS_TAG_APP_BLE_ADV_DATA,ADV_DATA_LEN,ble_adv_data) != NVDS_OK)return APPM_ERROR_STATE;
	

//	memset(scan_rsp_data,0,sizeof(scan_rsp_data));
//	memcpy(scan_rsp_data+2, ble_dev_name, appm_env.dev_name_len);
//	scan_rsp_data[0] = appm_env.dev_name_len + 1;//len
//	scan_rsp_data[1] = 0x08;  
    ble_name_ini();
    
//    if(nvds_put(NVDS_TAG_APP_BLE_SCAN_RESP_DATA,SCAN_RSP_DATA_LEN,scan_rsp_data) != NVDS_OK)return APPM_ERROR_STATE;  
    return APPM_ERROR_NO_ERROR;
}

enum appm_error read_ble_mac(uint8_t *mac)
{
    //uint8_t mac[6];

    uint32_t bdl,bdu;
    uint8_t *p_mac_l = (uint8_t *)&bdl;
    uint8_t *p_mac_u = (uint8_t *)&bdu;
    bdl = ble_bdaddrl_getf();
    bdu = ble_bdaddru_getf();

    mac[5] = p_mac_u[1];
    mac[4] = p_mac_u[0];
    mac[3] = p_mac_l[3];
    mac[2] = p_mac_l[2];
    mac[1] = p_mac_l[1];
    mac[0] = p_mac_l[0];
    
    USER_PRINTF("\r\n+MAC: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
            //p_mac_l[0], p_mac_l[1], p_mac_l[2], p_mac_l[3], p_mac_u[0], p_mac_u[1]);
    p_mac_u[1], p_mac_u[0], p_mac_l[3], p_mac_l[2], p_mac_l[1], p_mac_l[0]);
   
    return APPM_ERROR_NO_ERROR;
}

//enum appm_error set_ble_mac(uint8_t *mac)
enum appm_error set_ble_mac(struct gap_bdaddr mac)
{
    uint8_t flag = 0;
    struct bd_addr b_addr;
    
    for(int i=0;i<6;i++)
    {
        b_addr.addr[i] = mac.addr.addr[5-i];
    }
    if(nvds_put(NVDS_TAG_BD_ADDRESS,6,b_addr.addr) != NVDS_OK)///(uint8_t *)&mac[0]
    {            
        return APPM_ERROR_STATE;
    }
        
    if(APPM_GET_FIELD(ADV_EN))//if adv on set adv off
    {
        uint8_t  cur_state = ke_state_get(TASK_APPM);
        APPM_SET_FIELD(ADV_EN,0);
        appm_scan_adv_con_schedule();
        UART_PRINTF("adv set off cur_state:%d\r\n",cur_state);
        flag = 1;
    } 
    //memcpy(b_addr.addr,mac,6);
    if(mac.addr_type == ADDR_PUBLIC)
    {
        co_default_bdaddr_type = GAPM_CFG_ADDR_PUBLIC;
        llm_util_set_public_addr(&b_addr);
        lld_util_set_bd_address(&b_addr,ADDR_PUBLIC);        
    }
    else //if(mac.addr_type == ADDR_RAND)
    {
        co_default_bdaddr_type = GAPM_CFG_ADDR_PRIVATE;
        llm_util_set_rand_addr(&b_addr);
        lld_util_set_bd_address(&b_addr,ADDR_RAND);
    }
       

    USER_PRINTF("\r\n+SET MAC: %02x-%02x-%02x-%02x-%02x-%02x\r\n\r\nOK\r\n",
            mac.addr.addr[0], mac.addr.addr[1], mac.addr.addr[2], mac.addr.addr[3], mac.addr.addr[4], mac.addr.addr[5]);
          //mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);

    if((!APPM_GET_FIELD(ADV_EN)) && flag)//adv on
    {
        uint8_t  cur_state = ke_state_get(TASK_APPM);
        UART_PRINTF("adv set on cur_state:%d\r\n",cur_state);
        APPM_SET_FIELD(ADV_EN,1);
        appm_scan_adv_con_schedule();
    }
    return APPM_ERROR_NO_ERROR;    
}
#if CLI_CONSOLE
static uint8_t hex(char c);
static void hexstr2bin(const char *macstr, uint8_t *mac, int len);

static void help_cmd(char *buf, int len, int argc, char **argv);
static void version_cmd(char *buf, int len, int argc, char **argv);
static void echo_cmd(char *buf, int len, int argc, char **argv);

static void reboot_cmd(char *buf, int len, int argc, char **argv);
static void scan_cmd(char *buf, int len, int argc, char **argv);
static void connect_cmd(char *buf, int len, int argc, char **argv);
static void data_send_cmd(char *buf, int len, int argc, char **argv);
static void disconn_cmd(char *buf, int len, int argc, char **argv);
static void test_cmd(char *buf, int len, int argc, char **argv);
static void check_info_cmd(char *buf, int len, int argc, char **argv);
static void set_baud_cmd(char *buf, int len, int argc, char **argv);
static void set_name_cmd(char *buf, int len, int argc, char **argv);
static void mac_cmd(char *buf, int len, int argc, char **argv);
static void connect(char *buf, int len, int argc, char **argv);
static void adv(char *buf, int len, int argc, char **argv);
static void dmo_cmd(char *buf, int len, int argc, char **argv);
static void qdmo_cmd(char *buf, int len, int argc, char **argv);


const struct cli_command built_ins[BUILD_INS_NUM] = {
	
    {"AT+HELP","    help",              help_cmd},
    {"AT+VER","     version",           version_cmd},
    {"AT+ECHO","    echo on/off",       echo_cmd},
#if 0    
    {"\x01","       bk reg w/r",        bk_reg_cmd},    
#endif    
		
    {"AT+CONN","    connect device",    connect},
    {"AT+ADV","     adv on/off",        adv},
    {"AT+MAC","     get/set mac",       mac_cmd},
    {"AT+DMO","      Direct Mode",       dmo_cmd},
    {"AT+QDMO","     quit DMO",          qdmo_cmd},

    {"AT+REBOOT","  reboot system",     reboot_cmd},
    {"AT+SCAN","    scan device",       scan_cmd},
    {"AT+LECCONN"," connect ble device",connect_cmd},
    {"AT+LESEND","  send data",         data_send_cmd},
    {"AT+LEDISC","  disconnect device", disconn_cmd},
    {"AT+TEST","    test cmd",          test_cmd},
    {"AT+CHINFO","  check info",        check_info_cmd},
    {"AT+BDBAUD","  set baudrate",      set_baud_cmd},
    {"AT+NAME","    set name",          set_name_cmd},		
};

/* Built-in "help" command: prints all registered commands and their help
 * text string, if any.
 */
static void help_cmd(char *buf, int len, int argc, char **argv)
{
    int i, n;
    uint32_t build_in_count = sizeof(built_ins) / sizeof(struct cli_command);

    aos_cli_printf( "====Build-in Commands====\r\n" );
    for (i = 0, n = 0; i < MAX_COMMANDS && n < cli->num_commands; i++) {
        if (cli->commands[i]->name) {
            aos_cli_printf("%s: %s\r\n", cli->commands[i]->name,
                           cli->commands[i]->help ?
                           cli->commands[i]->help : "");
            n++;
            if ( n == build_in_count - USER_COMMANDS_NUM ) {
                aos_cli_printf("\r\n");
                aos_cli_printf("====User Commands====\r\n");
            }
        }
    }
}

static void version_cmd(char *buf, int len, int argc, char **argv)
{
    uint8_t fw_version[4];
    uint8_t hw_version[4]; 
    rwble_version(fw_version,hw_version);
    aos_cli_printf("rw hw version :v%d.%d.%d   build :%s\r\n",hw_version[3],hw_version[2],hw_version[1],(hw_version[0] == 0) ? "BLE v4.2":"BLE v4.2 + 2Mbps LE");
    aos_cli_printf("rw fw version :v%d.%d.%d.%d \r\n",fw_version[3],fw_version[2],fw_version[1],fw_version[0]);
}

static void echo_cmd(char *buf, int len, int argc, char **argv)
{
    if (argc == 1) {
        aos_cli_printf("Usage: echo on/off. Echo is currently %s\r\n",
                       cli->echo_disabled ? "Disabled" : "Enabled");
        return;
    }

    if (!strcmp(argv[1], "on")) {
        aos_cli_printf("Enable echo\r\n");
        cli->echo_disabled = 0;
    } else if (!strcmp(argv[1], "off")) {
        aos_cli_printf("Disable echo\r\n");
        cli->echo_disabled = 1;
    }
}

static uint8_t hex(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;
    return 0;
}

static void hexstr2bin(const char *macstr, uint8_t *mac, int len)
{
    int i;
    for (i=0;i < len && macstr[2 * i];i++) {
        mac[i] = hex(macstr[2 * i]) << 4;
        mac[i] |= hex(macstr[2 * i + 1]);
    }
}

static void reboot_cmd(char *buf, int len, int argc, char **argv)
{
#ifdef CLI_DEBUG
  aos_cli_printf("reboot cmd\r\n");
#endif
	
	aos_cli_printf("\r\nOK\r\n");
	

}

void enter_DMO_mode(uint8_t channal)
{
    cliexit = 1;
	uart_ringbuf_clean();
	dmo_channel = channal;
    aos_cli_printf("\r\nOK\r\n");
}

void quit_DMO_mode(void)
{
    cliexit = 0;
    uart_ringbuf_clean();
    aos_cli_printf("\r\nOK\r\n");
}

static void dmo_cmd(char *buf, int len, int argc, char **argv)
{
#ifdef CLI_DEBUG	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif
    
  if (argc != 2)goto EXIT;
	
	cliexit = 1;
	uart_ringbuf_clean();
	dmo_channel = argv[1][0]-'0';
	aos_cli_printf("\r\nOK\r\n");
	
	return;
	
EXIT:
	aos_cli_printf("\r\nERROR\r\n");
}

static void qdmo_cmd(char *buf, int len, int argc, char **argv)
{
    aos_cli_printf("\r\nERROR\r\n");
}

/*
ble device 1 mac, addr type(pub or random),device name
ble device 2 mac, addr type(pub or random),device name
......
ble device n mac, addr type(pub or random),device name
*/
static void scan_cmd(char *buf, int len, int argc, char **argv)
{
	
#ifdef CLI_DEBUG	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif
    
    if ((!strcmp(argv[1], "1")) || (!strcmp(argv[1], "2")))
    {
        if(APPM_GET_FIELD(SCAN_EN))
        {
            //aos_cli_printf("scan currently is on\r\n");
        }
        else
        {
            //aos_cli_printf("scan set on\r\n");
            uint8_t  cur_state = ke_state_get(TASK_APPM);
            APPM_SET_FIELD(SCAN_EN,1);
            //aos_cli_printf("scan set on cur_state:%d\r\n",cur_state);
            appm_scan_adv_con_schedule();
        }       
    }
    else if (!strcmp(argv[1], "0"))
    {
        if(APPM_GET_FIELD(SCAN_EN) == 0)
        {
            //aos_cli_printf("scan currently is off\r\n");
        }
        else
        {
            APPM_SET_FIELD(SCAN_EN,0);
            uint8_t  cur_state = ke_state_get(TASK_APPM);
            appm_scan_adv_con_schedule();
            //aos_cli_printf("scan set off cur_state:%d\r\n",cur_state);
        }         
    }
    
	//aos_cli_printf("\r\n+SCAN{\r\n112233445566,0,bk_ble_D01\r\n223344556677,1,bk_ble_D02\r\n+SCAN}\r\n");

}

uint8_t free_channel_search(void)
{
    uint8_t num = 0;
    
    for(int i = 0;i < APPC_IDX_MAX;i++)
    {
        
        if((ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_LINK_CONNECTED) ||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SDP_DISCOVERING)\
            ||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SERVICE_CONNECTED) )
        {
            num++; 
        }
        else break;
    }
    return num;
}
#include "appc.h"
uint8_t mac_search(uint8_t *bdaddr)
{
    uint8_t num = 0;
    
    for(int i = 0;i < APPC_IDX_MAX;i++)
    {
        
        if((ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_LINK_CONNECTED) ||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SDP_DISCOVERING)\
            ||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SERVICE_CONNECTED) )
        {
            if(memcmp((const void *)&(appc_env[i]->con_dev_addr.addr.addr[0]),(const void *)&(bdaddr[0]),6)==0)
            {
                num++; 
                break;
            } 
        }        
    }
    return num;
}    
static void connect_cmd(char *buf, int len, int argc, char **argv)
{
	uint8_t addr_type;

	struct gap_bdaddr bdaddr;
	
#ifdef CLI_DEBUG1	
	uint8_t i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif
    
	if (argc != 5)
    {
        //aos_cli_printf("Usage: parm1:mac+type,parm2:service uuid,parm3:write uuid,parm4:notify uuid\r\n");
		goto EXIT;
	}
	addr_type = argv[1][strlen(argv[1])-1] - '0';
	if(addr_type <= 1)slave_device.type = 	addr_type;
	else goto EXIT;
	if((strlen(argv[1])==13) && (strlen(argv[2]) <= 32) && (strlen(argv[3]) <= 32) && (strlen(argv[4]) <= 32))
	{
		hexstr2bin(argv[1],slave_device.mac,strlen(argv[1])-1);//mac
		hexstr2bin(argv[2],slave_device.serv_uuid,strlen(argv[2]));//service uuid
		hexstr2bin(argv[3],slave_device.write_uuid,strlen(argv[3]));//write uuid
		hexstr2bin(argv[4],slave_device.notify_uuid,strlen(argv[4]));//write uuid
	}
	else goto EXIT;	
	if(mac_search(slave_device.mac)) goto EXIT;
	//add connect function	
	
#ifdef CLI_DEBUG	
	aos_cli_printf("%s argc %d\r\n",__func__,argc);	
	for(i=1;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
	aos_cli_printf("change :\r\n");
	for(i=0;i<6;i++)
	{
		aos_cli_printf("%x ",slave_device.mac[i]);
	}aos_cli_printf("\r\n");
	for(i=0;i<16;i++)
	{
		aos_cli_printf("%x ",slave_device.serv_uuid[i]);
	}aos_cli_printf("\r\n");
	for(i=0;i<16;i++)
	{
		aos_cli_printf("%x ",slave_device.write_uuid[i]);
	}aos_cli_printf("\r\n");
	for(i=0;i<16;i++)
	{
		aos_cli_printf("%x ",slave_device.notify_uuid[i]);
	}aos_cli_printf("\r\n");
#endif

    hexstr2bin(argv[1], bdaddr.addr.addr, GAP_BD_ADDR_LEN);	
    bdaddr.addr_type = addr_type;
    
    if(appm_start_connencting( bdaddr) != APPM_ERROR_NO_ERROR)
    {
        goto EXIT;
    }
    appm_env.recon_num = APPM_RECONNENCT_DEVICE_NUM;
    aos_cli_printf("\r\n+GATTSTAT=%d,2\r\n",free_channel_search());
    
    return;
    
EXIT:
    aos_cli_printf("\r\nERROR\r\n");	
}

static void data_send_cmd(char *buf, int len, int argc, char **argv)
{
	uint8_t device_id;
	uint16_t packet_len;
	uint8_t packet[128];
	
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif
	if(argc != 4)goto EXIT;
	if(strlen(argv[1]) != 1)goto EXIT;
	device_id = atoi(argv[1]);// argv[1][0];///-'0';
//	if(BLE_WORK_STA[device_id] != BLE_CONN){UART_PRINTF("BLE_WORK_STA[%d] = %s\r\n",dmo_channel,BLE_WORK_STA[dmo_channel]?"BLE_DISCONN":"BLE_CONN");goto EXIT;}
	
	packet_len = atoi(argv[2]);
	memcpy(packet,argv[3],packet_len);
	//send data to device_id
	//aos_cli_printf("devID:%d, len:%d, packet:%s\r\n",device_id,packet_len,packet);
    //aos_cli_printf("appc_env[device_id]->svc_write_handle:%x\r\n",appc_env[device_id]->svc_write_handle);

    if(app_send_ble_data(device_id,packet_len,packet) != APPM_ERROR_NO_ERROR)goto EXIT;
	aos_cli_printf("\r\nOK\r\n");		
	return;
    
EXIT:
	aos_cli_printf("\r\nERROR\r\n");
}

static int8_t channel_search(uint8_t *mac)
{
	for(int i = 0;i < APPC_IDX_MAX;i++)
	{			
		if((ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_LINK_CONNECTED) ||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SDP_DISCOVERING)\
				||(ke_state_get(KE_BUILD_ID(TASK_APPC,i)) == APPC_SERVICE_CONNECTED) )
		{
			if(memcmp(mac,appc_env[i]->con_dev_addr.addr.addr,GAP_BD_ADDR_LEN)==0)
			{
				//UART_PRINTF("i:%x\r\n",i);
				return i;
			}
		}
	}
	return -1;
}

static void disconn_cmd(char *buf, int len, int argc, char **argv)
{
	int8_t device_id;
	uint8_t mac[6];
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif
	if((argc != 2)||((strlen(argv[1]) != 1)&&(strlen(argv[1]) != 12)))
	{
		//UART_PRINTF("argv[1]len:%x\r\n",strlen(argv[1]));
		goto EXIT;
	}
	if(strlen(argv[1]) == 1)
	{
		device_id  = atoi(argv[1]);///-'0';
	}
	else
	{
		hexstr2bin(argv[1],mac,strlen(argv[1]));
		device_id = channel_search(mac);
		if(device_id == -1)goto EXIT;
	}
	//add disconnect ble device function
	if(appm_disconnect(device_id) != APPM_ERROR_NO_ERROR)
    {
        goto EXIT;
    }
	//aos_cli_printf("\r\n+GATTSTAT=%s,1\r\n",argv[1]);///+CATTSTAT
	
	return;
EXIT:
	aos_cli_printf("\r\nERROR\r\n");	
}

static void test_cmd(char *buf, int len, int argc, char **argv)
{	
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif
    if(argc == 2)
    { 
        uint8_t send_data[255];
        uint8_t len = strlen(argv[1])/2;
        hexstr2bin(argv[1], send_data, len);

#if SOFT_SPI_EN
        spi_test1(send_data, len);
#else
        hw_spi_test1(send_data, len);///hw_spi_test(send_data, len);
#endif    
    }
    else
    {
        
    }
	aos_cli_printf("\r\nOK\r\n");	
}

static void check_info_cmd(char *buf, int len, int argc, char **argv)
{	
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif    
    appc_connect_device_info_get();	
}

static void set_baud_cmd(char *buf, int len, int argc, char **argv)
{
	uint32_t baudrate=0;
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}
#endif	
	if(argc != 2)
	{
		goto EXIT;
	}
	baudrate = atoi(argv[1]); 
	
	if(uart_init(baudrate))goto EXIT;  
	if(nvds_put(NVDS_TAG_UART_BAUDRATE,sizeof(baudrate),(uint8_t *)&baudrate) != NVDS_OK)
	{
		
		goto EXIT;
	}
	aos_cli_printf("\r\n+BAUD=%s\r\n\r\nOK\r\n",argv[1]);
		
	return;
EXIT:
	aos_cli_printf("\r\nERROR\r\n");		
}

#include "appm.h"
extern struct appm_env_tag appm_env;

static void set_name_cmd(char *buf, int len, int argc, char **argv)
{
	
#ifdef CLI_DEBUG1	
	int i;
	aos_cli_printf("%s argc %d\r\n",__func__,argc);
	for(i=0;i<argc;i++)
	{
		aos_cli_printf("%s\r\n",argv[i]);
	}	
#endif
	if(argc > 2)
	{
		goto EXIT;
	}

    if(argc == 2)
    {
        if(set_ble_name((uint8_t *)argv[1] , strlen(argv[1])) != APPM_ERROR_NO_ERROR)goto EXIT;
        aos_cli_printf("\r\n+NAME=%s\r\nOK\r\n",argv[1]);
    }
	else
        aos_cli_printf("\r\n+NAME=%s\r\nOK\r\n",ble_dev_name);
	
	return;
    
EXIT:
    aos_cli_printf("\r\nERROR\r\n");
}

static void mac_cmd(char *buf, int len, int argc, char **argv)
{
    struct gap_bdaddr bdaddr;
//    uint8_t mac[6];
//    uint8_t type;

    if (argc == 1)
    {    
        read_ble_mac(bdaddr.addr.addr);
    }
    else if(argc == 2)
    { 
        hexstr2bin(argv[1], bdaddr.addr.addr, 6);
        if((strlen(argv[1]) == 13) && ((0x30 <= argv[1][12])&&(argv[1][12] <= 0x31)))
        {
            bdaddr.addr_type = argv[1][12] - '0';
        }
        
        if(set_ble_mac(bdaddr) != APPM_ERROR_NO_ERROR) goto EXIT;
    }
    else
    {
      //aos_cli_printf("invalid cmd\r\n");
			goto EXIT;
    }
	return;
EXIT:
		aos_cli_printf("\r\nERROR\r\n");		
}

static void connect(char *buf, int len, int argc, char **argv)
{    
    UART_PRINTF("%s\r\n",__func__);
    struct gap_bdaddr bdaddr;
    uint8_t mac[6];
    if (argc != 2)goto EXIT;

    hexstr2bin(argv[1], mac, GAP_BD_ADDR_LEN);
    for(int i = 0; i < 6; i++)
    {
        bdaddr.addr.addr[i] = mac[5-i];
    }
    if(strlen(argv[1]) == 13) slave_device.type = argv[1][12] - '0';
    if(mac_search(bdaddr.addr.addr)) goto EXIT;
    
    UART_PRINTF("con address: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
            bdaddr.addr.addr[0], bdaddr.addr.addr[1], bdaddr.addr.addr[2], bdaddr.addr.addr[3], bdaddr.addr.addr[4], bdaddr.addr.addr[5]);
    bdaddr.addr_type = slave_device.type;///ADDR_PUBLIC;

    if(appm_start_connencting( bdaddr) != APPM_ERROR_NO_ERROR)goto EXIT;

    appm_env.recon_num = APPM_RECONNENCT_DEVICE_NUM;
    aos_cli_printf("\r\n+GATTSTAT=%d,2\r\n",free_channel_search());
 
	return;
EXIT:
    aos_cli_printf("\r\nERROR\r\n");   
}

static void adv(char *buf, int len, int argc, char **argv)
{      
    UART_PRINTF("%s\r\n",__func__);
    if (argc == 1)
    {       
        UART_PRINTF("Usage: Adv on/off. Adv is currently %s\r\n",
            APPM_GET_FIELD(SCAN_EN) ? "On" : "Off");
        goto EXIT;
    }
    else if(argc == 2)
    {
        if ((!strcmp(argv[1], "on"))||(!strcmp(argv[1], "ON"))) 
        {
            if(APPM_GET_FIELD(ADV_EN))
            {
                UART_PRINTF("adv currently is on\r\n");
            }else
            {
                uint8_t  cur_state = ke_state_get(TASK_APPM);
                UART_PRINTF("adv set on cur_state:%d\r\n",cur_state);
                APPM_SET_FIELD(ADV_EN,1);
                appm_scan_adv_con_schedule();
            }       
        } 
        else if ((!strcmp(argv[1], "off"))||(!strcmp(argv[1], "OFF")))
        {
            if(APPM_GET_FIELD(ADV_EN) == 0)
            {
                UART_PRINTF("adv currently is off\r\n");
            }else
            {
                uint8_t  cur_state = ke_state_get(TASK_APPM);
                APPM_SET_FIELD(ADV_EN,0);
                appm_scan_adv_con_schedule();
                UART_PRINTF("adv set off cur_state:%d\r\n",cur_state);
            }         
        }    
    }
    else goto EXIT;
    aos_cli_printf("\r\nOK\r\n");
	return;
EXIT:
    aos_cli_printf("\r\nERROR\r\n");
}

#endif
//extern uint8_t send_data_enable_flag[BLE_CONNECTION_MAX];
uint8_t app_send_ble_data(uint8_t device_id,uint16_t packet_len,uint8_t* packet)
{
//	if((device_id >= BLE_CONNECTION_MAX) || (appc_env[device_id] == NULL) || (!send_data_enable_flag[device_id])) goto EXIT;

	if(appc_env[device_id]->role == ROLE_SLAVE)
    {
//        if(app_electric_tx_send(device_id,packet_len,packet) != APPM_ERROR_NO_ERROR)goto EXIT;
    }
    else 
    {        
        if(appc_write_service_data_req(device_id,appc_env[device_id]->svc_write_handle,packet_len,packet)!= APPM_ERROR_NO_ERROR)
            goto EXIT;
    }
//    send_data_enable_flag[device_id] = 0;
	return APPM_ERROR_NO_ERROR;
    
EXIT:	
	return APPM_ERROR_STATE;
}

