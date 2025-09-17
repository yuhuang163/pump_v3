/***************************************************************************
 *
 * Copyright 2015-2023 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary cfgrmation of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#include "plat_addr_map.h"

#if defined(CRC_BASE)

#include "reg_crc.h"
#include "hal_crc.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_timer.h"

#ifdef RTOS
#include "cmsis_os.h"
static osSemaphoreId_t g_crc_sem;
#endif

static HAL_CRC_RUN_CB_T run_cb;

static struct CRC_T * const crc = (struct CRC_T *)CRC_BASE;

static void hal_crc_sem_init(void)
{
#ifdef RTOS
    if (g_crc_sem == NULL)
        g_crc_sem = osSemaphoreNew(1, 0, NULL);
#endif
}

static void hal_crc_sem_release(void)
{
#ifdef RTOS
    if (g_crc_sem)
        osSemaphoreRelease(g_crc_sem);
#endif
}

static int hal_crc_get_result(uint32_t *res)
{
    uint32_t val;
    uint32_t crc_mode;

    if (crc->DMA_STATUS & CRC_DMA_TRANS_ERR)
        return -1;

    crc_mode = crc->CRC_CFG;
    val = crc->CRC_RES;

    if ((crc_mode & CRC_CRC16_32_MASK) == 0) {
        if ((crc_mode & CRC_CRC_REFOUT_MASK) == 0 && (crc_mode & CRC_CRC_REFIN_MASK) == 0)
            val = val & 0xffff;
        else
            val = (val >> 16) & 0xffff;
    }
    *res = val;
    return 0;
}

static void hal_crc_intr_clr(void)
{
    crc->CRC_CFG |= CRC_INTR_CLEAR;
}

static void hal_crc_reset(void)
{
    crc->DMA_REQ &= ~CRC_DMA_EN;
    crc->DMA_CFG &= ~CRC_DMA_RST;
    crc->DMA_CFG |= CRC_DMA_RST;
}

static void hal_crc_config(const struct HAL_CRC_CFG_T* cfg, void *data, uint16_t len)
{
    uint32_t val;

    crc->DMA_ADDR = ADDR_CPU_TO_DEV((uint32_t)data);
    crc->DMA_LEN_BST = CRC_DMA_LEN((uint32_t)len);
    crc->CRC_INIT_VAL = cfg->init;
    crc->CRC_POLY = cfg->poly;

    val = crc->CRC_CFG;
    val |= CRC_INTR_MASK | CRC_INTR_CLEAR | CRC_CRC_INI;
    val &= ~(CRC_CRC16_32_MASK | CRC_CRC_REFOUT_MASK | CRC_CRC_REFIN_MASK | CRC_CRC_RESULT_XOR_MASK);
    val = SET_BITFIELD(val, CRC_CRC16_32, cfg->type_crc32);
    val = SET_BITFIELD(val, CRC_CRC_REFOUT, cfg->refout);
    val = SET_BITFIELD(val, CRC_CRC_REFIN, cfg->refin);
    val = SET_BITFIELD(val, CRC_CRC_RESULT_XOR, cfg->xorout_flag);
    crc->CRC_CFG = val;
}

static void hal_crc_begin(void)
{
    crc->DMA_REQ |= CRC_DMA_EN;
}

static void hal_crc_irq_handler(void)
{
    uint32_t crc_result;
    uint32_t state;

    state = crc->DMA_STATUS;
    hal_crc_intr_clr();

    if (run_cb) {
        if (hal_crc_get_result(&crc_result) == 0)
            run_cb(crc_result, 0);
        else
            run_cb(state, 1);
    } else {
        hal_crc_sem_release();
    }
}

bool hal_crc_busy(void)
{
    if ((crc->DMA_STATUS & CRC_CRC_REQING) || (crc->DMA_STATUS & CRC_CRC_WORKING))
        return true;
    return false;
}

static void hal_crc_wait(void)
{
#ifdef RTOS
    if (g_crc_sem)
        osSemaphoreAcquire(g_crc_sem, osWaitForever);
#else
    while (hal_crc_busy());
#endif
}

static void hal_crc_irq_enable(void)
{
    NVIC_SetVector(CRC_IRQn, (uint32_t)hal_crc_irq_handler);
    NVIC_SetPriority(CRC_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(CRC_IRQn);
    NVIC_EnableIRQ(CRC_IRQn);
}

void hal_crc_open(void)
{
    hal_crc_sem_init();
    hal_cmu_crc_clock_enable();
    hal_crc_irq_enable();
}

void hal_crc_close(void)
{
    NVIC_DisableIRQ(CRC_IRQn);
    hal_cmu_crc_clock_disable();
}

uint32_t hal_crc_start(const struct HAL_CRC_CFG_T* cfg, void *data, uint16_t len, HAL_CRC_RUN_CB_T cb)
{
    uint32_t res;

    hal_crc_reset();
    hal_crc_config(cfg, data, len);
    run_cb = cb;
    hal_crc_begin();

    if (run_cb) {
        return 0;
    }

    hal_crc_wait();
    ASSERT(hal_crc_get_result(&res) == 0,"state error,error state = %#x\n",crc->DMA_STATUS);

    return res;
}

uint16_t hal_crc16_ibm(void *data, uint16_t len)
{
    uint16_t res;

    struct HAL_CRC_CFG_T cfg;
    cfg.type_crc32 = 0;
    cfg.poly = 0x8005;
    cfg.init = 0x0000;
    cfg.refin = true;
    cfg.refout = true;
    cfg.xorout_flag = 0;
    res = hal_crc_start(&cfg, data, len, NULL);

    return res;
}

uint16_t hal_crc16_maxim(void *data, uint16_t len)
{
    uint16_t res;

    struct HAL_CRC_CFG_T cfg;
    cfg.type_crc32 = 0;
    cfg.poly = 0x8005;
    cfg.init = 0x0000;
    cfg.refin = true;
    cfg.refout = true;
    cfg.xorout_flag = 1;
    res = hal_crc_start(&cfg, data, len, NULL);

    return res;
}

uint16_t hal_crc16_usb(void *data, uint16_t len)
{
    uint16_t res;

    struct HAL_CRC_CFG_T cfg;
    cfg.type_crc32 = 0;
    cfg.poly = 0x8005;
    cfg.init = 0xffff;
    cfg.refin = true;
    cfg.refout = true;
    cfg.xorout_flag = 1;
    res = hal_crc_start(&cfg, data, len, NULL);

    return res;
}

uint16_t hal_crc16_modbus(void *data, uint16_t len)
{
    uint16_t res;

    struct HAL_CRC_CFG_T cfg;
    cfg.type_crc32 = 0;
    cfg.poly = 0x8005;
    cfg.init = 0xffff;
    cfg.refin = true;
    cfg.refout = true;
    cfg.xorout_flag = 0;
    res = hal_crc_start(&cfg, data, len, NULL);

    return res;
}

uint16_t hal_crc16_ccitt(void *data, uint16_t len)
{
    uint16_t res;

    struct HAL_CRC_CFG_T cfg;
    cfg.type_crc32 = 0;
    cfg.poly = 0x1021;
    cfg.init = 0x0000;
    cfg.refin = true;
    cfg.refout = true;
    cfg.xorout_flag = 0;
    res = hal_crc_start(&cfg, data, len, NULL);

    return res;
}

uint16_t hal_crc16_ccitt_false(void *data, uint16_t len)
{
    uint16_t res;

    struct HAL_CRC_CFG_T cfg;
    cfg.type_crc32 = 0;
    cfg.poly = 0x1021;
    cfg.init = 0xffff;
    cfg.refin = false;
    cfg.refout = false;
    cfg.xorout_flag = 0;
    res = hal_crc_start(&cfg, data, len, NULL);

    return res;
}

uint16_t hal_crc16_x25(void *data, uint16_t len)
{
    uint16_t res;

    struct HAL_CRC_CFG_T cfg;
    cfg.type_crc32 = 0;
    cfg.poly = 0x1021;
    cfg.init = 0xffff;
    cfg.refin = true;
    cfg.refout = true;
    cfg.xorout_flag = 1;
    res = hal_crc_start(&cfg, data, len, NULL);

    return res;
}

uint16_t hal_crc16_xmodem(void *data, uint16_t len)
{
    uint16_t res;

    struct HAL_CRC_CFG_T cfg;
    cfg.type_crc32 = 0;
    cfg.poly = 0x1021;
    cfg.init = 0x0000;
    cfg.refin = false;
    cfg.refout = false;
    cfg.xorout_flag = 0;
    res = hal_crc_start(&cfg, data, len, NULL);

    return res;
}

uint16_t hal_crc16_dnp(void *data, uint16_t len)
{
    uint16_t res;

    struct HAL_CRC_CFG_T cfg;
    cfg.type_crc32 = 0;
    cfg.poly = 0x3d65;
    cfg.init = 0x0000;
    cfg.refin = true;
    cfg.refout = true;
    cfg.xorout_flag = 1;
    res = hal_crc_start(&cfg, data, len, NULL);

    return res;
}

uint32_t hal_crc32(void *data, uint16_t len)
{
    uint32_t res;

    struct HAL_CRC_CFG_T cfg;
    cfg.type_crc32 = 1;
    cfg.poly = 0x04c11db7;
    cfg.init = 0xffffffff;
    cfg.refin = true;
    cfg.refout = true;
    cfg.xorout_flag = 1;
    res = hal_crc_start(&cfg, data, len, NULL);

    return res;
}

uint32_t hal_crc32_mpeg(void *data, uint16_t len)
{
    uint32_t res;

    struct HAL_CRC_CFG_T cfg;
    cfg.type_crc32 = 1;
    cfg.poly = 0x04c11db7;
    cfg.init = 0xffffffff;
    cfg.refin = false;
    cfg.refout = false;
    cfg.xorout_flag = 0;
    res = hal_crc_start(&cfg, data, len, NULL);

    return res;
}

#endif
