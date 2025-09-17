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
#ifdef MCU2CP_COMM_DEMO_TEST

#include "plat_types.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include "mcu2cp_comm_test.h"
#include "cp_subsys.h"
#include "cp_subsys_msg.h"

static uint32_t* in_buf_addr = NULL;
static uint32_t* out_buf_addr = NULL;
static uint32_t in_buf_size = 0;
static uint32_t out_buf_size = 0;
static volatile uint32_t process_start = false;

//CRC-32: x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1
static uint32_t demo_crc32(uint8_t *data, uint32_t length)
{
    uint8_t i;
    uint32_t crc = 0xffffffff;        // Initial value
    while (length--) {
        crc ^= *data++;                // crc ^= *data; data++;
        for (i = 0; i < 8; ++i) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;// 0xEDB88320= reverse 0x04C11DB7
            else
                crc = (crc >> 1);
        }
    }
    return ~crc;
}

void cp_subsys_data_process_handle(void)
{
    //TODO: handle the input buffer data and return the result.
    if (process_start) {
        uint32_t start_tick = hal_fast_sys_timer_get();
        uint32_t crc32_ret, ii;
        for (ii = 0; ii < 2000; ii++) {
            crc32_ret = demo_crc32((uint8_t*)in_buf_addr, in_buf_size*sizeof(uint32_t));
        }
        memcpy((uint8_t*)out_buf_addr, (uint8_t*)&crc32_ret, sizeof(crc32_ret));
        out_buf_size = sizeof(crc32_ret);
        uint32_t end_tick = hal_fast_sys_timer_get();
        TRACE(1, "CP_SUBSYS: process done:%d(us)", FAST_TICKS_TO_US(end_tick - start_tick));

        //TODO: cp subsys return the result to mcu core.
#ifdef NOAPP
        static uint32_t cnt = 0;
        if (cnt++%100 == 0) {
            cp_subsys_msg_send(CP_SUBSYS_MSG_ID_DEMO_PROCESS_RET, 0, NULL, 0);
        }
#endif
    }
}

unsigned int mcu2cp_msg_test_rx_handler(const void *data, unsigned int len)
{
    if (len == sizeof(struct CP_SUBSYS_MSG_T)) {
        const struct CP_SUBSYS_MSG_T *msg = (const struct CP_SUBSYS_MSG_T *)data;
        if (msg->hdr.id == CP_SUBSYS_MSG_ID_DEMO_PROCESS_START && !msg->hdr.reply) {
            cp_subsys_proc_buf_t* proc_buf = (cp_subsys_proc_buf_t*)msg->param[0];
            in_buf_addr = proc_buf->in_buf;
            out_buf_addr = proc_buf->out_buf;
            in_buf_size = proc_buf->in_size;
            process_start = true;
            TRACE(0, "CP_SUBSYS: process start, seq_cnt=%d", msg->seq);
        } else if (msg->hdr.id == CP_SUBSYS_MSG_ID_DEMO_PROCESS_STOP && !msg->hdr.reply) {
            process_start = false;
            TRACE(0, "CP_SUBSYS: process stop");
        }
    }

    return len;
}

POSSIBLY_UNUSED void mcu2cp_msg_test_tx_done_handler(const void *data, unsigned int len)
{

}

#endif
