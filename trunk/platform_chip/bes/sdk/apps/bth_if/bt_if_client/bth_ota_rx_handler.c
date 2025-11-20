/***************************************************************************
 *
 * Copyright 2015-2022 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

#include "cmsis_os.h"
#include "cmsis.h"
#include "string.h"
#include "hal_trace.h"
#include "heap_api.h"
#include "bth_if_cmd_def.h"
#include "hal_norflash.h"
#include "pmu.h"
#include "hal_timer.h"
#include "bth_bt_if_client.h"
#include "bth_ota_rx_handler.h"
#include "ota_multi_image_upgrade.h"

#define MAX_READ_SIZE   (120)
uint32_t cur_op_inx;

static uint8_t bth_ota_flash_busy = 0;
static PARAM_M55_FLASH_OP bth_ota_param;

uint8_t flash_read_cache[MAX_READ_SIZE];

osThreadId bth_ota_flash_tid = NULL;
static void bth_ota_thread(const void *arg);
// stacksize is 672 * 4 =  2688 bytes;
osThreadDef(bth_ota_thread, osPriorityHigh, 1, (2688), "bth_ota_flash");

#define BTH_OTA_BUFFER_MAX_LEN  (512)
#define BTH_OTA_MAILBOX_MAX (15)

osMailQId bth_ota_mq_id;
osMailQDef (bth_ota_mailbox, BTH_OTA_MAILBOX_MAX, FLASH_BLOCK_T);

static heap_handle_t bth_ota_mailbox_heap;
static uint8_t bth_ota_mailbox_heap_pool[BTH_OTA_BUFFER_MAX_LEN*(BTH_OTA_MAILBOX_MAX/3)];

static void bth_ota_op_code_handle(uint8_t *ptr_data , uint32_t ilength)
{

    int lock;
    uint32_t iread_size;

    if(bth_ota_flash_busy)
    {
        TRACE(3,"%s busy!",__func__);
        return;
    }

    memcpy((void*)&bth_ota_param , ptr_data , sizeof(bth_ota_param));
    switch(bth_ota_param.op_code)
    {
        case OP_CODE_WRITE:
            TRACE(3,"rec bth write m55 flash addr=0x%x" , bth_ota_param.address);
            bth_ota_flash_busy = 1;
            cur_op_inx = 0;
        break;
        case OP_CODE_READ:
            bth_ota_flash_busy = 1;
            cur_op_inx = 0;
            TRACE(3,"rec bth read m55 flash , ");
            while(bth_ota_param.ilength > cur_op_inx)
            {
                if((bth_ota_param.ilength - cur_op_inx)>MAX_READ_SIZE)
                {
                    iread_size = MAX_READ_SIZE;
                }
                else
                {
                    iread_size = (bth_ota_param.ilength - cur_op_inx);
                }
                lock = int_lock();
                hal_norflash_read(HAL_FLASH_ID_0, (bth_ota_param.address + cur_op_inx),
                    flash_read_cache, iread_size);
                int_unlock(lock);
                cur_op_inx += iread_size;

                //send data to bth -- data to be read
                app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_M55_TO_BTH_OTA_RSP_DATA, flash_read_cache, iread_size);
            }
            bth_ota_flash_busy = 0;

            //send cmd to bth -- read
            app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_M55_TO_BTH_OTA_RSP_CMD, (uint8_t*)"done", sizeof("done"));
        break;
        case OP_CODE_ERASE:
            bth_ota_flash_busy = 1;
            TRACE(3,"rec bth erase m55 flash 0x%x,%d", bth_ota_param.address, bth_ota_param.ilength);
            lock = int_lock();
            pmu_flash_write_config();
            hal_norflash_erase(HAL_FLASH_ID_0, bth_ota_param.address, bth_ota_param.ilength);
            pmu_flash_read_config();
            int_unlock(lock);
            bth_ota_flash_busy = 0;
            //send cmd to m55 -- erase done
            app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_M55_TO_BTH_OTA_RSP_CMD, (uint8_t*)"done", sizeof("done"));
        break;
        default:
        break;
    }
}

static void bth_ota_flash_data_handle(uint8_t *ptr_data , uint32_t ilength)
{
    int lock;
    uint8_t rsp_temp[4]={0};

    switch(bth_ota_param.op_code)
    {
        case OP_CODE_WRITE:
            lock = int_lock();
            pmu_flash_write_config();
            hal_norflash_write(HAL_FLASH_ID_0, (bth_ota_param.address + cur_op_inx),
                ptr_data, ilength);
            pmu_flash_read_config();
            int_unlock(lock);
            //send data to bth -- received data
            app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_M55_TO_BTH_DATA, rsp_temp, 4);
            cur_op_inx += ilength;
            if(cur_op_inx >= bth_ota_param.ilength)
            {
                bth_ota_flash_busy = 0;
                // send cmd to bth -- write done
                app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_M55_TO_BTH_OTA_RSP_CMD, (uint8_t*)"done", sizeof("done"));
            }
        break;
        default:
        break;
    }
}

static void bth_ota_mailbox_heap_init(void)
{
    bth_ota_mailbox_heap = heap_register(bth_ota_mailbox_heap_pool, sizeof(bth_ota_mailbox_heap_pool));
    TRACE(0,"%s",__func__);
}

POSSIBLY_UNUSED static uint32_t bth_ota_mailbox_heap_available_space(void)
{
    if (bth_ota_mailbox_heap)
    {
        return heap_free_size(bth_ota_mailbox_heap);
    }
    else
    {
        return 0;
    }
}

static void *bth_ota_mailbox_heap_malloc(uint32_t size)
{
    void *ptr =NULL;
    if (size){
        ptr = heap_malloc(bth_ota_mailbox_heap,size);
        ASSERT(ptr, "%s size:%d", __func__, size);
    }
    return ptr;
}

static void bth_ota_mailbox_heap_free(void *rmem)
{
    if (rmem){
        heap_free(bth_ota_mailbox_heap,rmem);
    }
}

static int bth_ota_mailbox_get(FLASH_BLOCK_T** msg_p)
{
    osEvent evt;
    evt = osMailGet(bth_ota_mq_id, osWaitForever);
    if (evt.status == osEventMail)
    {
        *msg_p = (FLASH_BLOCK_T *)evt.value.p;
        return 0;
    }
    return -1;
}

static int bth_ota_mailbox_free(FLASH_BLOCK_T* msg_p)
{
    osStatus status;
    bth_ota_mailbox_heap_free((void *)msg_p->cmd_buff.cmd_buffer);
    status = osMailFree(bth_ota_mq_id, msg_p);
    return (int)status;
}

static int bth_ota_mailbox_init(void)
{
    bth_ota_mq_id = osMailCreate(osMailQ(bth_ota_mailbox), NULL);
    if (bth_ota_mq_id == NULL)
    {
        TRACE(0,"Failed to Create bth_ota_mailbox\n");
        return -1;
    }
    return 0;
}

static int bth_ota_mailbox_put(FLASH_BLOCK_T* msg_src)
{
    FLASH_BLOCK_T *msg_p = NULL;
    osStatus status;
    msg_p = (FLASH_BLOCK_T*)osMailAlloc(bth_ota_mq_id , 0);
    if(!msg_p){
        return -1;
    }
    msg_p->evt = msg_src->evt;
    msg_p->arg = msg_src->arg;
    msg_p->arg1 = msg_src->arg1;
    msg_p->system_time = hal_sys_timer_get();
    msg_p->cmd_buff = msg_src->cmd_buff;
    status = osMailPut(bth_ota_mq_id, msg_p);
    return (int)status;
}

//operation cmd
void app_bth_ota_op_code_send(uint8_t* data , uint32_t ilen)
{
    FLASH_BLOCK_T msg_src;
    memset(&msg_src , 0x00 , sizeof(msg_src));
    msg_src.evt = MSG_OP_CODE;
    msg_src.cmd_buff.cmd_buffer = bth_ota_mailbox_heap_malloc(ilen);
    memcpy(msg_src.cmd_buff.cmd_buffer , data , ilen);
    msg_src.cmd_buff.cmd_len = ilen;
    bth_ota_mailbox_put(&msg_src);
}

//data packet
void app_bth_ota_data_send(uint8_t* data , uint32_t ilen)
{
    FLASH_BLOCK_T msg_src;
    memset(&msg_src , 0x00 , sizeof(msg_src));
    msg_src.evt = MSG_FLASH_DATA;
    msg_src.cmd_buff.cmd_buffer = bth_ota_mailbox_heap_malloc(ilen);
    msg_src.cmd_buff.cmd_len = ilen;
    memcpy(msg_src.cmd_buff.cmd_buffer , data , ilen);
    bth_ota_mailbox_put(&msg_src);
}

void app_bth_ota_thread_init(void)
{
    TRACE(0,"%s",__func__);
    bth_ota_mailbox_heap_init();
    bth_ota_mailbox_init();
    if (bth_ota_flash_tid == NULL) {
        bth_ota_flash_tid = osThreadCreate(osThread(bth_ota_thread), NULL);
    }
}

static void bth_ota_thread(const void *arg)
{
    FLASH_BLOCK_T *msg_p = NULL;
    while(1)
    {
        bth_ota_mailbox_get(&msg_p);
        switch(msg_p->evt)
        {
            case MSG_OP_CODE:
                bth_ota_op_code_handle(msg_p->cmd_buff.cmd_buffer , msg_p->cmd_buff.cmd_len);
            break;
            case MSG_FLASH_DATA:
                bth_ota_flash_data_handle(msg_p->cmd_buff.cmd_buffer , msg_p->cmd_buff.cmd_len);
            break;
        }
        bth_ota_mailbox_free(msg_p);
    }
}