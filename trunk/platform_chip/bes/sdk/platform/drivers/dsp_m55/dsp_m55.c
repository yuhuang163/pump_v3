/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "dsp_m55.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_cmu.h"
#include "hal_psc.h"
#include "hal_rmt_ipc.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "rx_dsp_m55_trc.h"
#include "string.h"
#include "system_subsys.h"

#ifdef DSP_M55_FLASH_BOOT
#include "hal_norflash.h"
#endif

#ifndef NO_OVERLAY
#include "hal_overlay_subsys.h"
#endif

#ifndef DSP_M55_CODE_OFFSET
#ifdef DSP_M55_ALT_BOOT_FLASH
#define DSP_M55_CODE_OFFSET                 (FLASH_SIZE / 2)
#else
#define DSP_M55_CODE_OFFSET                 0
#endif
#endif

#ifdef DSP_M55_ALT_BOOT_FLASH
#define DSP_M55_FLASH_BOOT_STRUCT_ADDR      (FLASH_BASE + DSP_M55_CODE_OFFSET)
#else
#define DSP_M55_FLASH_BOOT_STRUCT_ADDR      (FLASH1_BASE + DSP_M55_CODE_OFFSET)
#endif

#if defined(CHIP_BEST1600) || defined(CHIP_BEST1603)
#define DSP_M55_RUN_AFTER_RESET_CLEAR
#endif

extern uint32_t __dsp_m55_code_start_flash[];
extern uint32_t __dsp_m55_code_end_flash[];

static HAL_RMT_IPC_EP_T rmt_ipc_ep;

#if defined(DSP_M55_TRC_TO_MCU) && defined(RMT_TRC_IN_MSG_CHAN)
static DSP_M55_RX_IRQ_HANDLER app_rx_hdlr;

static unsigned int general_rx_handler(const void *data, unsigned int len)
{
    if (dsp_m55_trace_rx_handler(data, len)) {
        return len;
    }

    if (app_rx_hdlr) {
        return app_rx_hdlr(data, len);
    }

    return len;
}
#endif

#if defined(DSP_M55_RUN_AFTER_RESET_CLEAR) && !defined(M55C0_MAIN_RAM_USE_SYS_RAM)
static void sys_ram_set_value_32(uint32_t addr, uint32_t val)
{
    uint32_t new_addr;
    uint32_t POSSIBLY_UNUSED offset;

    ASSERT((addr & 3) == 0, "%s: Bad addr=0x%X", __func__, addr);

    new_addr = addr;

    if (0) {
#if (DSP_M55_ITCM_SIZE > 0)
    } else if (DSP_M55_ITCM_BASE <= addr && addr < (DSP_M55_ITCM_BASE + DSP_M55_ITCM_SIZE)) {
        new_addr -= DSP_M55_MIN_ITCM_BASE;
        offset = new_addr & (SHR_RAM_BLK_SIZE / 2 - 1);
        if (offset) {
            if (offset < SHR_RAM_BLK_SIZE / 4) {
                offset *= 2;
            } else {
                offset = (offset - SHR_RAM_BLK_SIZE / 4) * 2 + 4;
            }
            new_addr = (new_addr & ~(SHR_RAM_BLK_SIZE / 2 - 1)) + offset;
        }
        new_addr += SYS_RAM0_BASE;
#endif
#if (DSP_M55_DTCM_SIZE > 0)
    } else if (DSP_M55_DTCM_BASE <= addr && addr < (DSP_M55_DTCM_BASE + DSP_M55_DTCM_SIZE)) {
        new_addr -= DSP_M55_MIN_DTCM_BASE;
        offset = new_addr & (SHR_RAM_BLK_SIZE - 1);
        if (offset) {
            if (offset & 8) {
                offset = offset / 16 * 8 + (offset & (8 - 1)) + SHR_RAM_BLK_SIZE / 2;
            } else {
                offset = offset / 16 * 8 + (offset & (8 - 1));
            }
            new_addr = (new_addr & ~(SHR_RAM_BLK_SIZE - 1)) + offset;
        }
        new_addr = (SYS_RAM2_BASE - SHR_RAM_BLK_SIZE) -
            new_addr / SHR_RAM_BLK_SIZE * SHR_RAM_BLK_SIZE +
            offset;
#endif
    }

    *(volatile uint32_t *)new_addr = val;
}

static void dsp_m55_prepare_wfi_codes(uint32_t vector, uint32_t sp)
{
    static const uint16_t m55_wfi_code[2] = {
        0xBF30, // WFI
        0xE7FD, // b (pc - 6)
    };

    // Change tcm to sys ram
    hal_cmu_sys_m55_dtcm_cfg(false);

    sys_ram_set_value_32(vector + 0, sp);
    sys_ram_set_value_32(vector + 4, (vector + 8) | 1);
    sys_ram_set_value_32(vector + 8, (m55_wfi_code[1] << 16) | m55_wfi_code[0]);
    __DSB();

    // Restore tcm cfg
    hal_cmu_sys_m55_dtcm_cfg(true);
}
#endif

static void dsp_m55_init_vectors(volatile uint32_t vectors[], uint32_t sp, uint32_t entry)
{
    uint32_t array[2];

    array[0] = sp;
    array[1] = entry;

#ifdef SUBSYS_CUSTOMIZED_COPY
    int ret;

    ret = subsys_customized_copy((void *)vectors, array, sizeof(array));
    if (ret == 0) {
        return;
    }
#endif

    vectors[0] = array[0];
    vectors[1] = array[1];
    __DSB();
}

int dsp_m55_open(DSP_M55_RX_IRQ_HANDLER rx_hdlr, DSP_M55_TX_IRQ_HANDLER tx_hdlr)
{
    int ret;
    uint32_t code_start;
    volatile uint32_t *vector_base;
    uint32_t sp;

    TR_INFO(0, "%s", __FUNCTION__);

    TR_INFO(0, "M55-ITCM BASE:%08x SIZE:%08x", DSP_M55_ITCM_BASE, DSP_M55_ITCM_SIZE);
    TR_INFO(0, "M55-DTCM BASE:%08x SIZE:%08x", DSP_M55_DTCM_BASE, DSP_M55_DTCM_SIZE);
    TR_INFO(0, "M55-SRAM BASE:%08x SIZE:%08x", DSP_M55_SYS_RAM_BASE, DSP_M55_SYS_RAM_SIZE);

    hal_psc_sys_m55_enable();
    hal_cmu_m55_clock_enable();
    hal_cmu_m55_reset_clear();

    osDelay(2);

#if defined(DSP_M55_RUN_AFTER_RESET_CLEAR) && !defined(M55C0_MAIN_RAM_USE_SYS_RAM)
    dsp_m55_prepare_wfi_codes(DSP_M55_DTCM_BASE, DSP_M55_MAILBOX_BASE);
#endif

#ifdef M55C0_MAIN_RAM_USE_SYS_RAM
    sp = DSP_M55_SYS_RAM_BASE + DSP_M55_SYS_RAM_SIZE;
    vector_base = (volatile uint32_t *)(sp - 128);
#else
    sp = DSP_M55_DTCM_BASE + 128;
#ifdef DSP_M55_ADDR_CPU_TO_DEV
    vector_base = (volatile uint32_t *)dsp_m55_addr_cpu_to_dev(DSP_M55_DTCM_BASE);
#else
    vector_base = (volatile uint32_t *)DSP_M55_DTCM_BASE;
#endif
#endif

#if !(defined(DSP_M55_RUN_AFTER_RESET_CLEAR) && defined(M55C0_MAIN_RAM_USE_SYS_RAM))
    hal_cmu_m55_cpu_reset_clear((uint32_t)vector_base);
#endif

#ifdef DSP_M55_FLASH_BOOT
#if !defined(DSP_M55_ALT_BOOT_FLASH) && defined(FLASH1_CTRL_BASE)
    hal_cmu_sys_flash_io_enable();
    hal_cmu_clock_enable(HAL_CMU_MOD_H_FLASH1);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_FLASH1);
    hal_norflash_force_wakeup(HAL_FLASH_ID_1);
#endif

    ret = subsys_check_boot_struct(DSP_M55_FLASH_BOOT_STRUCT_ADDR, &code_start);
    if (ret) {
        ASSERT(false, "%s: Bad subsys boot struct: %d", __func__, ret);
        return 1;
    }

#else // !DSP_M55_FLASH_BOOT
    uint32_t addr;
    uint32_t len;
    int check_magic;
    SUBSYS_EXEC_ADDR_CONV_CB conv_cb = NULL;

#ifdef DSP_M55_LOAD_FLASH_ADDR
    addr = DSP_M55_LOAD_FLASH_ADDR;
    len = DSP_M55_LOAD_FLASH_SIZE;
    check_magic = true;
#else
    addr = (uint32_t)__dsp_m55_code_start_flash;
    len = (uint32_t)__dsp_m55_code_end_flash - addr;
    check_magic = false;
#endif

#ifdef DSP_M55_ADDR_CPU_TO_DEV
    conv_cb = dsp_m55_addr_cpu_to_dev;
#endif

    ret = subsys_loader_load_image(addr, len, check_magic, conv_cb, &code_start);
    if (ret) {
        ASSERT(false, "%s: Failed to load subsys segment image: %d", __func__, ret);
        return 2;
    }

#ifndef NO_OVERLAY
    hal_overlay_subsys_init(addr, OVERLAY_M55, conv_cb);
#endif

#endif // !DSP_M55_FLASH_BOOT

#if defined(DSP_M55_TRC_TO_MCU) && defined(RMT_TRC_IN_MSG_CHAN)
    app_rx_hdlr = rx_hdlr;
    rx_hdlr = general_rx_handler;
#endif
    ret = hal_rmt_ipc_open(HAL_RMT_IPC_CORE_SYS_M55C0, HAL_RMT_IPC_CHAN_0, rx_hdlr, tx_hdlr, false, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_open failed: %d", ret);

    ret = hal_rmt_ipc_start_recv(rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_start_recv failed: %d", ret);

#ifdef DSP_M55_TRC_TO_MCU
    dsp_m55_trace_server_open();
#endif

    dsp_m55_init_vectors(vector_base, sp, (code_start | 1));
#ifdef DSP_M55_RUN_AFTER_RESET_CLEAR
#ifndef M55C0_MAIN_RAM_USE_SYS_RAM
    hal_cmu_m55_cpu_reset_set();
#endif
    hal_cmu_m55_cpu_reset_clear((uint32_t)vector_base);
#endif
    hal_cmu_m55_start_cpu();

    return 0;
}

int dsp_m55_close(void)
{
    TR_INFO(0, "%s", __FUNCTION__);

#ifdef DSP_M55_TRC_TO_MCU
    dsp_m55_trace_server_close();
#endif

    hal_rmt_ipc_close(rmt_ipc_ep);

    hal_cmu_m55_reset_set();
    hal_cmu_m55_clock_disable();
    hal_psc_sys_m55_disable();

    return 0;
}

int dsp_m55_send_seq(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_rmt_ipc_send_seq(rmt_ipc_ep, data, len, seq);
}

int dsp_m55_send(const void *data, unsigned int len)
{
    return hal_rmt_ipc_send(rmt_ipc_ep, data, len);
}

int dsp_m55_tx_active(unsigned int seq)
{
    return hal_rmt_ipc_tx_active(rmt_ipc_ep, seq);
}
