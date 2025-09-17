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
#include "cp_subsys.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_cmu.h"
#include "hal_rmt_ipc.h"
#include "hal_sleep.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "rx_cp_subsys_trc.h"
#include "string.h"
#include "system_subsys.h"
#include "hal_overlay_subsys.h"
#if defined(APP_RPC_MCU_CP_EN)
#include "app_rpc_api.h"
#endif
#ifndef CP_SUBSYS_FLASH_BOOT_STRUCT_ADDR
#define CP_SUBSYS_FLASH_BOOT_STRUCT_ADDR    FLASH1_NC_BASE
#endif

extern uint32_t __cp_subsys_code_start_flash[];
extern uint32_t __cp_subsys_code_end_flash[];
extern void cp_wakeup_handler(void);

static HAL_RMT_IPC_EP_T rmt_ipc_ep;

int cp_subsys_open(CP_SUBSYS_RX_IRQ_HANDLER rx_hdlr, CP_SUBSYS_TX_IRQ_HANDLER tx_hdlr)
{
    uint32_t code_start;
    POSSIBLY_UNUSED int ret;
    uint32_t entry;
    uint32_t sp;

    sp = CP_SUBSYS_MAILBOX_BASE;
    hal_cmu_cp_subsys_enable(sp);

#ifdef CP_SUBSYS_FLASH_BOOT
    ret = subsys_check_boot_struct(CP_SUBSYS_FLASH_BOOT_STRUCT_ADDR, &code_start);
    if (ret) {
        ASSERT(false, "%s: Bad subsys boot struct: %d", __func__, ret);
        return 1;
    }
#else
    uint32_t addr;
    uint32_t len;
    int check_magic;
    SUBSYS_EXEC_ADDR_CONV_CB conv_cb = NULL;

#ifdef CP_SUBSYS_LOAD_FLASH_ADDR
    addr = CP_SUBSYS_LOAD_FLASH_ADDR;
    len = CP_SUBSYS_LOAD_FLASH_SIZE;
    check_magic = true;
#else
    addr = (uint32_t)__cp_subsys_code_start_flash;
    len = (uint32_t)__cp_subsys_code_end_flash - addr;
    check_magic = false;
#endif

#ifdef CP_ADDR_CPU_TO_DEV
    conv_cb = cp_addr_cpu_to_dev;
#endif

    ret = subsys_loader_load_image(addr, len, check_magic, conv_cb, &code_start);
    if (ret) {
        ASSERT(false, "%s: Failed to load subsys image: %d", __func__, ret);
        return 2;
    }
#endif

#ifndef NO_OVERLAY
    hal_overlay_subsys_init(addr, OVERLAY_M55, conv_cb);
#endif

#ifndef NO_CP_SUBSYS_IPC_WHEN_OPEN
#if defined(APP_RPC_MCU_CP_EN)
    app_rpc_ctx_init(APP_RPC_CORE_MCU_CP);
#else
    ret = hal_rmt_ipc_open(HAL_RMT_IPC_CORE_CORE1, HAL_RMT_IPC_CHAN_0, rx_hdlr, tx_hdlr, false, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_open failed: %d", ret);

    ret = hal_rmt_ipc_start_recv(rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_start_recv failed: %d", ret);
#endif

#ifdef CP_SUBSYS_TRC_TO_MCU
    cp_subsys_trace_server_open();
#endif
#endif

    entry = code_start;
    hal_cmu_cp_start_cpu(sp, entry);

    hal_bus_wake_lock(HAL_BUS_WAKE_LOCK_USER_CP);

    return 0;
}

int cp_subsys_opened(void)
{
    return hal_rmt_ipc_opened(rmt_ipc_ep);
}

int cp_subsys_close(void)
{
    TR_INFO(0, "%s", __FUNCTION__);

#ifdef CP_SUBSYS_TRC_TO_MCU
    cp_subsys_trace_server_close();
#endif

    hal_rmt_ipc_close(rmt_ipc_ep);

    hal_cmu_cp_subsys_disable();

    hal_bus_wake_unlock(HAL_BUS_WAKE_LOCK_USER_CP);

    return 0;
}

int cp_subsys_send_seq(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_rmt_ipc_send_seq(rmt_ipc_ep, data, len, seq);
}

int cp_subsys_send(const void *data, unsigned int len)
{
    int ret = hal_rmt_ipc_send(rmt_ipc_ep, data, len);
    cp_wakeup_handler();
    return ret;
}

int cp_subsys_tx_active(unsigned int seq)
{
    return hal_rmt_ipc_tx_active(rmt_ipc_ep, seq);
}
