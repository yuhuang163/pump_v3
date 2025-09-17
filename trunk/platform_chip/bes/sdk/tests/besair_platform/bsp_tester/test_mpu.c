/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifdef TEST_MPU
#include "hal_trace.h"
#include "dwt.h"
#include "hal_timer.h"
#include "string.h"
#include "mpu.h"

#define BUF_LEN 1024
static __attribute__((aligned(32))) uint8_t mpu_mem[BUF_LEN] = {0};


void test_mpu(void)
{
    int ret = 0;

    memset(mpu_mem, 0x01, sizeof(mpu_mem));
    TRACE(0,"mpu_mem:%p ", mpu_mem);

    /**
     * 必须满足以下条件
     * 1. 起始 addr 按 32Bytes 对齐
     * 2. len 必须大于等于 32Bytes
     */
    ret = mpu_set((uint32_t)mpu_mem, BUF_LEN, MPU_ATTR_EXEC, MEM_ATTR_INT_SRAM);
    if(!ret) {
        TRACE(0,"mpu_set success addr:%p len=%d", mpu_mem, BUF_LEN);
    } else {
        TRACE(0,"mpu_set fail ret=%d", ret);
    }

    TRACE(0, "============= mpu_region_dump ============");
    mpu_region_dump();
    TRACE(0, "========== mpu_region_dump done ==========\n");

    hal_sys_timer_delay(MS_TO_TICKS(1000));
    TRACE(0, "mpu_clear, moidfy data, do not trigger crash");
    ret = mpu_clear((uint32_t)mpu_mem, BUF_LEN);
    if (ret) {
        TRACE(0, "mpu_clear fail, ret=%d", ret);
    }
    mpu_mem[50] = 0x02;
    TRACE(0,"modify mpu_mem[50] = %x", mpu_mem[50]);

    hal_sys_timer_delay(MS_TO_TICKS(1000));
    TRACE(0, "mpu_set, moidfy data, trigger crash");
    ret = mpu_set((uint32_t)mpu_mem, BUF_LEN, MPU_ATTR_EXEC, MEM_ATTR_INT_SRAM);
    if (ret) {
        TRACE(0, "mpu_set fail, ret=%d", ret);
    }
    mpu_mem[50] = 0x03;
}

#endif
