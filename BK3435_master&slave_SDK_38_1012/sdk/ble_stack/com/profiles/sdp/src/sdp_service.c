/**
 ****************************************************************************************
 *
 * @file sdp_service.c
 *
 * @brief Sdp Service Client implementation.
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @addtogroup SDP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#if (BLE_SDP_CLIENT)
#include "ke_mem.h"
#include "gap.h"
#include "ke_env.h"
#include "sdp_service.h"
#include "sdp_service_task.h"
#include "co_math.h"
#include "uart.h"
#include <stdint.h>             // standard definition
#include <stdbool.h>            // standard boolean
#include "arch.h"               // architecture
#include "co_math.h"            // computation utilities
#include "ke_config.h"          // kernel configuration
#include "ke_env.h"             // kernel environment
#include "ke_mem.h"             // kernel memory
#include "RomCallFlash.h"
#if CLI_CONSOLE
#include "cli.h"
#endif
#include "app_api.h"
#include "appc.h"
//uint8_t sdp_need_dis_flag = 0;


struct sdp_env_init_t sdp_env_init;
ble_dev_info slave_device={
	0,   \
	0x11,0x22,0x33,0x44,0x55,0x66,  \
	0,  \
	0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x10, 0x19, 0x00, 0x00,  \
	0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x11, 0x2b, 0x00, 0x00,  \
	0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x10, 0x2b, 0x00, 0x00,  \
};
//	0x10, 0x19,  \
//	0x11, 0x2b,  \
//	0x10, 0x2b,  \

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void sdp_service_init(void)
{
    memset((uint8_t *)&sdp_env_init,0,sizeof(struct sdp_env_init_t));
    for(int i = 0; i < BLE_NB_PROFILES_ADD_MAX ; i++)
    {
        struct prf_sdp_db_env *prf_sdp_db = NULL;
        struct sdp_content *sdp_cont = NULL;
        sdp_env_init.used_status[i] = FREE_STATUS;
        prf_sdp_db = ke_malloc(sizeof(struct prf_sdp_db_env),KE_MEM_NON_RETENTION);
//        UART_PRINTF("struct prf_sdp_db_env size:%d,addr:0x%x \r\n",sizeof(struct prf_sdp_db_env),prf_sdp_db);
        sdp_cont = ke_malloc(sizeof(struct sdp_content),KE_MEM_NON_RETENTION);
//        UART_PRINTF("struct sdp_content size:%d,addr:0x%x \r\n",sizeof(struct sdp_content),sdp_cont);
        sdp_env_init.sdp_env[i].prf_db_env  = prf_sdp_db;
        prf_sdp_db->sdp_cont = sdp_cont;
                
    }
}

uint8_t sdp_service_free_env_find(void)
{
    uint8_t free_idx;
    for(free_idx = 0; free_idx < BLE_NB_PROFILES_ADD_MAX ; free_idx++)
    {   
        if(FREE_STATUS == sdp_env_init.used_status[free_idx])
        {
            return free_idx;
        }
    }
    return free_idx;
}
static uint8_t sdp_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  void* params)
{
    UART_PRINTF("%s,app_task:0x%x \r\n",__func__,app_task);

    struct prf_sdp_db_env *prf_db_env = (struct prf_sdp_db_env *)params;
        
    UART_PRINTF("prf_db_env->prf_idx = %x\r\n",prf_db_env->prf_idx);
    UART_PRINTF("prf_db_env->sdp_cont->chars_nb = %x\r\n",prf_db_env->sdp_cont->chars_nb);
    for(int i =0; i < prf_db_env->sdp_cont->chars_nb; i++)
    {
        UART_PRINTF("char uuid = 0x");
        for(uint8_t len = 0;len < prf_db_env->sdp_cont->chars_descs_inf.chars_inf[i].uuid_len;len++)
        {
            UART_PRINTF("%02x",prf_db_env->sdp_cont->chars_descs_inf.chars_inf[i].uuid[len]);
        }UART_PRINTF("\r\n");
        UART_PRINTF("char_hdl = 0x%02x,val_hdl = 0x%02x,prop = 0x%02x\r\n",prf_db_env->sdp_cont->chars_descs_inf.chars_inf[i].char_hdl,prf_db_env->sdp_cont->chars_descs_inf.chars_inf[i].val_hdl,
                                                                           prf_db_env->sdp_cont->chars_descs_inf.chars_inf[i].prop);
    }
    UART_PRINTF("prf_db_env->sdp_cont.descs_nb = %x\r\n",prf_db_env->sdp_cont->descs_nb);
    for(int i =0; i < prf_db_env->sdp_cont->descs_nb; i++)
    {
        UART_PRINTF("descs uuid = 0x");
        for(uint8_t len = 0;len < prf_db_env->sdp_cont->chars_descs_inf.descs_inf[i].uuid_len;len++)
        {
            UART_PRINTF("%02x",prf_db_env->sdp_cont->chars_descs_inf.descs_inf[i].uuid[len]);
        }UART_PRINTF("\r\n");
        UART_PRINTF("desc_hdl = 0x%02x\r\n",prf_db_env->sdp_cont->chars_descs_inf.descs_inf[i].desc_hdl);
    }
    //-------------------- allocate memory required for the profile  ---------------------
    struct sdp_env_tag* sdp_env = &sdp_env_init.sdp_env[prf_db_env->prf_idx];
    
    // allocate  required environment variable
    env->env = (prf_env_t*) sdp_env;
    sdp_env->prf_env.app_task = app_task
                                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    sdp_env->prf_env.prf_task = env->task | PERM(PRF_MI, ENABLE);
    // initialize environment variable
    env->role                   = PRF_CLIENT;
    env->id                     = KE_BUILD_ID(TASK_ID_SDP,prf_db_env->prf_idx);
    env->desc.idx_max           = SDP_IDX_MAX;
    env->desc.state             = sdp_env->state;
    env->desc.default_handler   = &sdp_default_handler;
     
    sdp_env->operation = NULL;
    // service is ready, go into an Idle state
    ke_state_set(KE_BUILD_ID(env->task, sdp_env->conidx), SDP_IDLE);
    
    if(0 != prf_db_env->sdp_cont->descs_nb)
    {  
        UART_PRINTF("prf_register_atthdl2gatt start_hdl = 0x%x,end_hdl = 0x%x\r\n",sdp_env->prf_db_env->sdp_cont->svc.shdl,sdp_env->prf_db_env->sdp_cont->svc.ehdl);
        prf_register_atthdl2gatt(&(sdp_env->prf_env),sdp_env->conidx, &sdp_env->prf_db_env->sdp_cont->svc);      
    }
    UART_PRINTF("%s end!!\r\n",__func__);
    return GAP_ERR_NO_ERROR;
}
/**
 ****************************************************************************************
 * @brief Destruction of the BASC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void sdp_destroy(struct prf_task_env* env)
{
    UART_PRINTF("sdp_destroy START env->id = 0x%x,role = %x\r\n",env->id,env->role);

    struct sdp_env_tag* sdp_env = (struct sdp_env_tag*) env->env;
    
    if(sdp_env_init.sdp_need_dis_flag[sdp_env->prf_db_env->prf_idx] == SDP_SERVICE_NO_NEED_DISCOVER)
    {
        UART_PRINTF("SDP_SERVICE_NO_NEED_DISCOVER cur connected!!");
        return;  
    }
    
    UART_PRINTF("used_status = 0x%x,prf_idx:%d\r\n",sdp_env_init.used_status[sdp_env->prf_db_env->prf_idx],sdp_env->prf_db_env->prf_idx);
    sdp_env_init.used_status[sdp_env->prf_db_env->prf_idx] = INVALID_STATUS;////

    // cleanup environment variable for each task instances
             
    if(sdp_env->operation != NULL)
    {
        UART_PRINTF("sdp_destroy sdp_env->operation = 0x%x\r\n",sdp_env->operation);
        sdp_env->operation = NULL;
    }        
    // free profile environment variables
    UART_PRINTF("sdp_destroy end\r\n");
}
/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void sdp_create(struct prf_task_env* env, uint8_t conidx)
{
    /* Put BAS Client in Idle state */
    ke_state_set(KE_BUILD_ID(env->task, conidx), SDP_IDLE);
}
/**
 ****************************************************************************************
 * @brief Handles Disconnection
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 ****************************************************************************************
 */
static void sdp_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{

    UART_PRINTF("sdp_cleanup env->id = 0x%x,role = %x,conidx = %x reason = 0x%x\r\n",env->id,env->role,conidx,reason);
    
    struct sdp_env_tag* sdp_env = (struct sdp_env_tag*) env->env;
    
    if(sdp_env->conidx != conidx)
    {
        UART_PRINTF("sdp_cleanup not cur connected!!\r\n");
        ke_state_set(KE_BUILD_ID(env->task, conidx), SDP_FREE);
        return;
    }
    if(sdp_env_init.sdp_need_dis_flag[sdp_env->prf_db_env->prf_idx] == SDP_SERVICE_NO_NEED_DISCOVER)
    {
        UART_PRINTF("SDP_SERVICE_NO_NEED_DISCOVER cur connected!!");
        ke_state_set(KE_BUILD_ID(env->task, conidx), SDP_FREE);
        return;  
    }
    
    UART_PRINTF("used_status = 0x%x,prf_idx:%d\r\n",sdp_env_init.used_status[sdp_env->prf_db_env->prf_idx],sdp_env->prf_db_env->prf_idx);
    sdp_env_init.used_status[sdp_env->prf_db_env->prf_idx] = FREE_STATUS;
    env->id = TASK_ID_INVALID;
    
    uint8_t idx;
    for(idx = 0; idx < SDP_IDX_MAX ; idx++)
    {
        if(0 != sdp_env->prf_db_env->sdp_cont->descs_nb)
        {
            UART_PRINTF("descs_nb:%d\r\n",sdp_env->prf_db_env->sdp_cont->descs_nb); 
            sdp_env->prf_db_env->sdp_cont->descs_nb = 0;
            UART_PRINTF("descs_inf = 0x%02x\r\n",sdp_env->prf_db_env->sdp_cont->chars_descs_inf.descs_inf);
            if(NULL != sdp_env->prf_db_env->sdp_cont->chars_descs_inf.descs_inf)
            {
                ke_free(sdp_env->prf_db_env->sdp_cont->chars_descs_inf.descs_inf);
                sdp_env->prf_db_env->sdp_cont->chars_descs_inf.descs_inf = NULL;
            }
              
        }
    }
    // clean-up environment variable allocated for task instance
    

    if(sdp_env->operation != NULL)
    {
        struct ke_msg *msg = ke_msg2param(sdp_env->operation);
        UART_PRINTF("operation = 0x%08x\r\n",(uint32_t)sdp_env->operation);
        UART_PRINTF("msgid = 0x%02x,dest_id = 0x%02x,src_id = 0x%02x\r\n",msg->id,msg->dest_id,msg->src_id);
                
       // ke_free(sdp_env->operation);
      //  sdp_env->operation = NULL;
                
    }
    if(0 != sdp_env->prf_db_env->sdp_cont->chars_nb)
    {
        UART_PRINTF("chars_nb:%d\r\n",sdp_env->prf_db_env->sdp_cont->chars_nb);
        if(NULL != sdp_env->prf_db_env->sdp_cont->chars_descs_inf.chars_inf)
        {
            UART_PRINTF("chars_inf = 0x%02x\r\n",sdp_env->prf_db_env->sdp_cont->chars_descs_inf.chars_inf);
            ke_free(sdp_env->prf_db_env->sdp_cont->chars_descs_inf.chars_inf);
            sdp_env->prf_db_env->sdp_cont->chars_descs_inf.chars_inf = NULL;
        }
        sdp_env->prf_db_env->sdp_cont->chars_nb = 0;
        
    }
    
    UART_PRINTF("00006\r\n");
    UART_PRINTF("sdp_cleanup end\r\n");
    /* Put BAS Client in Free state */
    ke_state_set(KE_BUILD_ID(env->task, conidx), SDP_FREE);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
/// BASC Task interface required by profile manager
const struct prf_task_cbs sdp_itf =
{
    sdp_init,
    sdp_destroy,
    sdp_create,
    sdp_cleanup,
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
const struct prf_task_cbs* sdp_prf_itf_get(void)
{
    return &sdp_itf;
}

void sdp_discover_all_service(uint8_t conidx)
{
    UART_PRINTF("sdp_discover_all_service conidx:%x\r\n",conidx);
    
    struct gattc_sdp_svc_disc_cmd * svc_req = KE_MSG_ALLOC_DYN(GATTC_SDP_SVC_DISC_CMD,
            KE_BUILD_ID(TASK_GATTC,conidx), KE_BUILD_ID(TASK_APPC,conidx),///TASK_APPM,
            gattc_sdp_svc_disc_cmd, ATT_UUID_16_LEN);
    //gatt request type: by UUID
    svc_req->operation         = GATTC_SDP_DISC_SVC_ALL;
    //start handle;
    svc_req->start_hdl        = ATT_1ST_REQ_START_HDL;
    //end handle
    svc_req->end_hdl          = ATT_1ST_REQ_END_HDL;
    // UUID search
    svc_req->uuid_len = ATT_UUID_16_LEN;
    /// operation sequence number
    //  svc_req->seq_num = 2;
    //set the first two bytes to the value array, LSB to MSB:Health Thermometer Service UUID first
    co_write16p(&(svc_req->uuid[0]), 0x0000);
    //send the message to GATT, which will send back the response when it gets it
    ke_msg_send(svc_req);
}

void sdp_discover_service(uint8_t conidx,uint8_t *uuid,uint8_t uuid_len)
{
    UART_PRINTF("sdp_discover_service conidx:%x\r\n",conidx);

    struct gattc_sdp_svc_disc_cmd * svc_req = KE_MSG_ALLOC_DYN(GATTC_SDP_SVC_DISC_CMD,
            KE_BUILD_ID(TASK_GATTC,conidx), KE_BUILD_ID(TASK_APPC,conidx),
            gattc_sdp_svc_disc_cmd, ATT_UUID_16_LEN);
    //gatt request type: by UUID
    svc_req->operation        = GATTC_SDP_DISC_SVC;
    //start handle;
    svc_req->start_hdl        = ATT_1ST_REQ_START_HDL;
    //end handle
    svc_req->end_hdl          = ATT_1ST_REQ_END_HDL;
    // UUID search
    svc_req->uuid_len = uuid_len; ;
    /// operation sequence number
    svc_req->seq_num = conidx;

    memcpy(&(svc_req->uuid[0]),uuid,uuid_len);
#if UART_DEBUG_EN	
    UART_PRINTF("uuid :");
    for(int i = 0;i < uuid_len;i++)
    {
        UART_PRINTF("%02x",svc_req->uuid[i]);
    }UART_PRINTF("\r\n");
#endif	
    //send the message to GATT, which will send back the response when it gets it
    ke_msg_send(svc_req);
}

//extern struct prf_env_tag prf_env;

uint8_t sdp_extract_svc_info(uint8_t conidx,struct gattc_sdp_svc_ind const *ind)
{
    UART_PRINTF("*************************************************************************************************\r\n");
 
    struct prf_sdp_db_env *prf_db_env;
    uint8_t free_env_idx;
    uint16_t malloc_size;  
    

    UART_PRINTF("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
    UART_PRINTF("%s\r\n",__func__);
    UART_PRINTF("service uuid len = 0x%x,uuid = 0x",ind->uuid_len);
    for(int i = 0; i < ind->uuid_len; i++) {
        UART_PRINTF("%02x",ind->uuid[i]);
    }
    UART_PRINTF("\r\n");
    
    if(ind->uuid_len==16)    
    {
        if(memcmp(slave_device.serv_uuid,ind->uuid,ind->uuid_len) != 0)
        {      
            UART_PRINTF("not slave_device.serv_uuid!!!!\r\n");
            return 0;       
        }
    }
    else
    {
        if(memcmp(&(slave_device.serv_uuid[12]),ind->uuid,ind->uuid_len) != 0)
        {      
            UART_PRINTF("not slave_device.serv_uuid!!!!\r\n");
            return 0;       
        }
    }
    
    uint16_t length =  ind->end_hdl - ind->start_hdl;
    UART_PRINTF("start_hdl  = %02d end_hdl  = %02d length = %d\r\n",ind->start_hdl,ind->end_hdl,length);
    
    //Counters
    uint8_t svc_char,svc_char_cnt;
    uint8_t svc_desc,svc_desc_cnt;
    uint8_t fnd_att;
    /**  search svc_char_cnt and desc_cnt for alloc memory **/
    svc_char_cnt = 0;
    svc_desc_cnt = 0;
    for (fnd_att=0; fnd_att< (ind->end_hdl - ind->start_hdl); fnd_att++)
    {
        if(ind->info[fnd_att].att_type == GATTC_SDP_ATT_CHAR)
        {
            uint16_t val_hdl  = ind->info[fnd_att].att_char.handle;
            // check that value handle is in a valid range
            if((val_hdl <= ind->end_hdl) && (val_hdl > (ind->start_hdl + fnd_att)))
            {
                //Look over requested characteristics
                for (svc_char=0; svc_char < ind->end_hdl -ind->start_hdl ; svc_char++)
                {
                    svc_char_cnt = (svc_char_cnt + 1) % SDP_NB_CHAR_INSTANCES_MAX;
                    // find end of characteristic handle and discover descriptors
                    do
                    {
                        fnd_att++;
                        // found a descriptor
                        if(ind->info[fnd_att].att_type == GATTC_SDP_ATT_DESC)
                        {
                            //Retrieve characteristic descriptor handle using UUID
                            for(svc_desc = 0; svc_desc<ind->end_hdl - ind->start_hdl; svc_desc++)
                            {
                                // check if it's expected descriptor
                                if ( svc_desc == svc_char)
                                {
                                     svc_desc_cnt = (svc_desc_cnt + 1 ) % SDP_NB_DESC_INSTANCES_MAX;
                                    // search for next descriptor
                                      break;
                                }
                            }
                        }
                    }
                    while(((ind->start_hdl+ 1 + fnd_att) <= ind->end_hdl)
                            && (ind->info[fnd_att].att_type != GATTC_SDP_ATT_CHAR)
                            && (ind->info[fnd_att].att_type != GATTC_SDP_INC_SVC));
                    // return to previous valid value
                    fnd_att--;
                    // search next characteristic
                    break;
                }
            }
        }
    }
    /**************************end search ************************************/
    
    malloc_size = ((sizeof(struct prf_char_inf) * svc_char_cnt) + (sizeof(struct prf_char_desc_inf) * svc_desc_cnt));
    if(check_enough_mem_add_service(malloc_size))
    {   
        UART_PRINTF("not enough mem to store profile!!!!!\r\n");
        return GAP_ERR_INSUFF_RESOURCES;
    }
    
    free_env_idx = sdp_service_free_env_find();
    UART_PRINTF("free_env_idx = %d\r\n",free_env_idx);
    if(free_env_idx >= BLE_NB_PROFILES_ADD_MAX)
    {
        UART_PRINTF("not enough env to store profile!!!!!\r\n");
        return GAP_ERR_INSUFF_RESOURCES;                                                         
    }
    
    UART_PRINTF("total need = 0x%x\r\n",sizeof(struct  prf_char_inf ) * svc_char_cnt + svc_desc_cnt * sizeof( struct prf_char_desc_inf ));
    /*************************alloc memory *******************************/
    UART_PRINTF("chars need = 0x%x\r\n",sizeof(struct prf_char_inf) * svc_char_cnt);
    struct prf_char_inf *chars = (struct prf_char_inf *) ke_malloc(sizeof(struct prf_char_inf) * svc_char_cnt,KE_MEM_NON_RETENTION);
    UART_PRINTF("descs need = 0x%x\r\n",sizeof(struct prf_char_desc_inf) * svc_desc_cnt);
    struct prf_char_desc_inf *descs = (struct prf_char_desc_inf *)ke_malloc(sizeof(struct prf_char_desc_inf) * svc_desc_cnt ,KE_MEM_ATT_DB);
    UART_PRINTF("chars_inf = 0x%08x,descs_inf = 0x%08x\r\n",chars,descs);
    UART_PRINTF("alloc memory END\r\n");
   // add_svc_flag = 0;
    if(true == 1)        
    {
       
        prf_db_env = sdp_env_init.sdp_env[free_env_idx].prf_db_env;
        sdp_env_init.sdp_env[free_env_idx].conidx = conidx;
        sdp_env_init.used_status[free_env_idx] = USED_STATUS;
        prf_db_env->prf_idx = free_env_idx;
        prf_db_env->sdp_cont->chars_descs_inf.chars_inf = chars;
        prf_db_env->sdp_cont->chars_descs_inf.descs_inf = descs;
        prf_db_env->sdp_cont->svc.shdl = ind->start_hdl;
        prf_db_env->sdp_cont->svc.ehdl = ind->end_hdl;
        svc_char_cnt = 0;
        svc_desc_cnt = 0;
        prf_db_env->sdp_cont->chars_nb = 0;
        prf_db_env->sdp_cont->char_idx = 0;
        prf_db_env->sdp_cont->descs_nb = 0;
        for (fnd_att=0; fnd_att< (ind->end_hdl - ind->start_hdl); fnd_att++)
        {
            if(ind->info[fnd_att].att_type == GATTC_SDP_ATT_CHAR)
            {
                uint16_t char_hdl = ind->start_hdl + 1 + fnd_att;
                uint16_t val_hdl  = ind->info[fnd_att].att_char.handle;
                uint8_t  val_prop = ind->info[fnd_att].att_char.prop;
                uint8_t  char_idx = fnd_att;
                // check that value handle is in a valid range
                if((val_hdl <= ind->end_hdl) && (val_hdl > (ind->start_hdl + fnd_att)))
                {
                    // retrieve value index
                    uint8_t val_idx = (val_hdl - ind->start_hdl - 1);
                    //Look over requested characteristics
                    for (svc_char=0; svc_char < ind->end_hdl -ind->start_hdl ; svc_char++)
                    {
                        UART_PRINTF("-----------------------------------------------------\r\n");
                        UART_PRINTF("char uuid len = 0x%x ,svc_char = %d ",ind->info[val_idx].att.uuid_len,svc_char);
                      //  UART_PRINTF("char uuid = 0x%04x \r\n",attm_convert_to16((uint8_t *)ind->info[val_idx].att.uuid,ind->info[val_idx].att.uuid_len));
                        UART_PRINTF("char uuid = 0x");
                        for(int i = 0; i < ind->info[val_idx].att.uuid_len; i++)
                        {
                            UART_PRINTF("%02x",ind->info[val_idx].att.uuid[i]);
                        }
                        UART_PRINTF("\r\n\r\n");
                        UART_PRINTF("char_hdl = %d,val_hdl = %d,fnd_att = 0x%x ,val_idx = 0x%x,val_prop = 0x%x\r\n",char_hdl,val_hdl,fnd_att,val_idx,val_prop);
                        {
                            
                            if(ind->uuid_len==16)
                            {
                                if(memcmp(slave_device.write_uuid,ind->info[val_idx].att.uuid,ind->uuid_len) == 0)
                                {
                                    UART_PRINTF("-----------------------------------------------------********************************\r\n");
                                    UART_PRINTF("-----------------------------------------------------********************************\r\n");
                                    appc_env[conidx]->svc_write_handle = val_hdl;
                                    UART_PRINTF("svc_write_handle: 0x%02x\r\n",appc_env[conidx]->svc_write_handle);
                                }
                                else if(memcmp(slave_device.notify_uuid,ind->info[val_idx].att.uuid,ind->uuid_len) == 0)
                                {
                                    UART_PRINTF("-----------------------------------------------------********************************\r\n");
                                    UART_PRINTF("-----------------------------------------------------********************************\r\n");
                                    appc_env[conidx]->svc_notif_handle = val_hdl;
                                    UART_PRINTF("svc_notif_handle: 0x%02x\r\n",appc_env[conidx]->svc_notif_handle);
                                }
                            }
                            else
                            {
                                if(memcmp(&(slave_device.write_uuid[12]),ind->info[val_idx].att.uuid,ind->uuid_len) == 0)
                                {
                                    UART_PRINTF("-----------------------------------------------------********************************\r\n");
                                    UART_PRINTF("-----------------------------------------------------********************************\r\n");
                                    appc_env[conidx]->svc_write_handle = val_hdl;
                                    UART_PRINTF("svc_write_handle: 0x%02x\r\n",appc_env[conidx]->svc_write_handle);
                                }
                                else if(memcmp(&(slave_device.notify_uuid[12]),ind->info[val_idx].att.uuid,ind->uuid_len) == 0)
                                {
                                    UART_PRINTF("-----------------------------------------------------********************************\r\n");
                                    UART_PRINTF("-----------------------------------------------------********************************\r\n");
                                    appc_env[conidx]->svc_notif_handle = val_hdl;
                                    UART_PRINTF("svc_notif_handle: 0x%02x\r\n",appc_env[conidx]->svc_notif_handle);
                                }
                            }
                        
                        }
                        if(0)
                        {
                            UART_PRINTF("Characteristic Properties: 0x%02x\r\n",val_prop);
                            UART_PRINTF("Broadcast :                %d\r\n",PERM_GET(val_prop << 8,BROADCAST));
                            UART_PRINTF("Read :                     %d\r\n",PERM_GET(val_prop << 8,RD));
                            UART_PRINTF("Write Without Response :   %d\r\n",PERM_GET(val_prop << 8,WRITE_COMMAND));
                            UART_PRINTF("Write :                    %d\r\n",PERM_GET(val_prop << 8,WRITE_REQ));
                            UART_PRINTF("Notify :                   %d\r\n",PERM_GET(val_prop << 8,NTF));
                            UART_PRINTF("Indicate :                 %d\r\n",PERM_GET(val_prop << 8,IND));
                            UART_PRINTF("Write Signed :             %d\r\n",PERM_GET(val_prop << 8,WRITE_SIGNED));
                            UART_PRINTF("Extended properties :      %d\r\n",PERM_GET(val_prop << 8,EXT));
                        };
       
                        //Save properties and handles
                        chars[svc_char_cnt].uuid_len = ind->info[val_idx].att.uuid_len;
                        memcpy(chars[svc_char_cnt].uuid,ind->info[val_idx].att.uuid,ind->info[val_idx].att.uuid_len);
                                        
                        chars[svc_char_cnt].char_hdl       = char_hdl;
                        chars[svc_char_cnt].val_hdl        = val_hdl;
                        chars[svc_char_cnt].prop           = val_prop;
                       
                        svc_char_cnt++;
                        prf_db_env->sdp_cont->chars_nb = svc_char_cnt;
                        // find end of characteristic handle and discover descriptors
                        do
                        {
                            fnd_att++;
                            // found a descriptor
                            if(ind->info[fnd_att].att_type == GATTC_SDP_ATT_DESC)
                            {
                                //Retrieve characteristic descriptor handle using UUID
                                for(svc_desc = 0; svc_desc<ind->end_hdl - ind->start_hdl; svc_desc++)
                                {
                                    // check if it's expected descriptor
                                    if ( svc_desc == svc_char)
                                    {
                                        
                                        UART_PRINTF("svc_desc = 0x%x,desc uuid = 0x%02x uuid_len = 0x%x ",svc_desc,
                                        attm_convert_to16((uint8_t*)ind->info[fnd_att].att.uuid,ind->info[fnd_att].att.uuid_len),ind->info[fnd_att].att.uuid_len);
                                        
                                        descs[svc_desc_cnt].uuid_len = ind->info[fnd_att].att.uuid_len;
                                        memcpy(descs[svc_desc_cnt].uuid,ind->info[fnd_att].att.uuid,ind->info[fnd_att].att.uuid_len);
                            
                                        descs[svc_desc_cnt].desc_hdl = ind->start_hdl + 1 + fnd_att;
                                        descs[svc_desc_cnt].char_code =  svc_char_cnt - 1;
                                        UART_PRINTF("desc_hdl = 0x%x desc_char_cnt = 0x%x,val_idx = 0x%x\r\n",ind->start_hdl + 1 + fnd_att,svc_char_cnt - 1,val_idx);
                                        // search for next descriptor
                                        svc_desc_cnt++;
                                        prf_db_env->sdp_cont->descs_nb = svc_desc_cnt;
                                        //break;
                                    }
                                }
                            }
                        } while(((ind->start_hdl+ 1 + fnd_att) <= ind->end_hdl)
                                && (ind->info[fnd_att].att_type != GATTC_SDP_ATT_CHAR)
                                && (ind->info[fnd_att].att_type != GATTC_SDP_INC_SVC));
                        // return to previous valid value
                        fnd_att--;
                        // previous handle was end of the characteristic
                        chars[svc_char_cnt].char_ehdl_off    = fnd_att - char_idx;
                        // search next characteristic
                        break;
                    }
                }
            }
        }
        UART_PRINTF("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
        UART_PRINTF("%s  end\r\n",__func__);
        sdp_add_profiles(conidx,prf_db_env);
    }
    
    return 0;
}

void sdp_add_profiles(uint8_t conidx,struct prf_sdp_db_env *db_env)
{
    UART_PRINTF("%s \r\n",__func__);
    // return;
    struct prf_sdp_db_env *env;
    struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                            TASK_GAPM, KE_BUILD_ID(TASK_APPC,conidx),
                                            gapm_profile_task_add_cmd, sizeof(struct prf_sdp_db_env));
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = 0;//PERM(SVC_AUTH, ENABLE);
    req->prf_task_id = KE_BUILD_ID(TASK_ID_SDP,db_env->prf_idx);
    req->app_task = KE_BUILD_ID(TASK_APPC,conidx);
    req->start_hdl = 0; //req->start_hdl = 0; dynamically allocated
    env = (struct prf_sdp_db_env *)req->param ;
    env->prf_idx  = db_env->prf_idx;
    env->sdp_cont  = db_env->sdp_cont;

    UART_PRINTF("env->sdp_cont.chars_nb = %d,prf_idx :%d\r\n",env->sdp_cont->chars_nb,env->prf_idx);
    ke_msg_send(req);
}
void sdp_enable_rsp_send(struct sdp_env_tag *sdp_env, uint8_t conidx, uint8_t status)
{
    // Send APP the details of the discovered attributes on
    struct sdp_enable_rsp * rsp = KE_MSG_ALLOC(SDP_ENABLE_RSP,
                                  prf_dst_task_get(&(sdp_env->prf_env) ,conidx),
                                  prf_src_task_get(&(sdp_env->prf_env) ,conidx),
                                  sdp_enable_rsp);
    rsp->status = status;
    ke_msg_send(rsp);
} 
uint8_t check_enough_mem_add_service(uint32_t size)
{
    // KE_MEM_BLOCK_MAX
    if( ke_check_malloc(size,KE_MEM_NON_RETENTION))
    {
        return 0;
    }
    else
    {
        return 1;
    }
    
}
#endif /* (BLE_CLIENT) */
/// @} SDP
