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
#include "hal_trace.h"
#include "string.h"
#include "ota_multi_image_upgrade.h"
#include "bth_if_cmd_def.h"
#include "bth_if_thread_server.h"

#define  OTA_OP_CODE_WRITE_TIMEOUT   5000
#define  MAX_WRITE_SIZE     120


osSemaphoreDef(ota_m55_write_done_signal);
static osSemaphoreId ota_m55_write_done_signal_id = NULL;


osMutexDef(ota_m55_write);
static osMutexId ota_m55_write_id = NULL;

static uint8_t bth_wr_m55_flash_init_flag = 0;

static uint8_t *flash_read_ptr;
static uint32_t flash_read_inx;


static void ota_m55_write_mutex_init(void)
{
    ota_m55_write_id = osMutexCreate(osMutex(ota_m55_write));
}

static int ota_m55_write_mutex_wait(void)
{
    return osMutexWait(ota_m55_write_id , osWaitForever);
}

static int ota_m55_write_mutex_release(void)
{
    return osMutexRelease(ota_m55_write_id);
}

static void ota_m55_write_done_signal_init(void)
{
    ota_m55_write_done_signal_id = osSemaphoreCreate(osSemaphore(ota_m55_write_done_signal), 0);
}

static int ota_m55_write_done_signal_wait(void)
{
    int ret;
    ret = osSemaphoreAcquire(ota_m55_write_done_signal_id, OTA_OP_CODE_WRITE_TIMEOUT);
    if(ret!= osOK)
    {
        TRACE(3,"%s %d",__func__,ret);
    }
    return ret;
}

static void ota_m55_write_done_signal_set(void)
{
    osSemaphoreRelease(ota_m55_write_done_signal_id);
}


void ota_write_m55_flash_done_handle(void)
{
    ota_m55_write_done_signal_set();
}

int ota_read_m55_flash_done(uint8_t *p_buff , uint32_t ilength)
{
    memcpy(flash_read_ptr+flash_read_inx , p_buff, ilength);
    flash_read_inx += ilength;
    return 0;
}

void ota_multi_image_upgrade_init(void)
{
    ota_m55_write_mutex_init();
    ota_m55_write_done_signal_init();
    bth_wr_m55_flash_init_flag = 1;
}

int ota_multi_image_m55_flash_write(uint32_t address , uint8_t *p_buff , uint32_t ilength)
{
    uint32_t wr_inx = 0;
    uint32_t remain_len = ilength;
    uint32_t write_len=0;
    uint8_t *src_data = p_buff;
    app_ota_bth_to_m55_cmd_t op_param;

    if(!bth_wr_m55_flash_init_flag)
    {
        return -1;
    }

    ota_m55_write_mutex_wait();
    op_param.op_code = OP_CODE_WRITE;
    op_param.address = address;
    op_param.ilength = ilength;
    //send cmd
    bth_if_send_result_to_app(APP_BTH_IF_USER_BT, (uint8_t*)(&op_param), sizeof(op_param), BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_CMD);
    //send data
    while(remain_len>0)
    {
        if(remain_len > MAX_WRITE_SIZE)
        {
            write_len = MAX_WRITE_SIZE;
            bth_if_send_result_to_app(APP_BTH_IF_USER_BT, src_data+wr_inx, write_len, BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_DATA);
        }
        else
        {
            write_len = remain_len;
            bth_if_send_result_to_app(APP_BTH_IF_USER_BT, src_data+wr_inx, write_len, BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_DATA);
        }

        wr_inx+=write_len;
        remain_len-=write_len;
    }
    ota_m55_write_done_signal_wait();
    ota_m55_write_mutex_release();
    return 0;

}

int ota_multi_image_m55_flash_read(uint32_t address , uint32_t ilength , uint8_t *p_buff)
{
    app_ota_bth_to_m55_cmd_t op_param;

    ota_m55_write_mutex_wait();

    flash_read_ptr = p_buff;
    flash_read_inx = 0;

    op_param.op_code = OP_CODE_READ;
    op_param.address = address;
    op_param.ilength = ilength;
    //send cmd
    bth_if_send_result_to_app(APP_BTH_IF_USER_BT, (uint8_t*)(&op_param), sizeof(op_param), BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_CMD);

    ota_m55_write_done_signal_wait();
    ota_m55_write_mutex_release();
    return 0;

}

int ota_multi_image_m55_flash_erase(uint32_t address , uint32_t ilength)
{
    app_ota_bth_to_m55_cmd_t op_param;

    ota_m55_write_mutex_wait();

    op_param.op_code = OP_CODE_ERASE;
    op_param.address = address;
    op_param.ilength = ilength;
    //send cmd
    bth_if_send_result_to_app(APP_BTH_IF_USER_BT, (uint8_t*)(&op_param), sizeof(op_param), BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_CMD);

    ota_m55_write_done_signal_wait();
    ota_m55_write_mutex_release();
    return 0;

}
