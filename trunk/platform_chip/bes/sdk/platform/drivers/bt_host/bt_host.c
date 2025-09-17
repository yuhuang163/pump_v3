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
#include "bt_host.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_cmu.h"
#include "hal_norflash.h"
#include "hal_rmt_ipc.h"
#include "hal_psc.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "rx_bth_trc.h"
#include "string.h"
#include "system_subsys.h"

#ifndef BTH_FLASH_BOOT_STRUCT_ADDR
#define BTH_FLASH_BOOT_STRUCT_ADDR          FLASH1_NC_BASE
#endif

extern uint32_t __bt_host_code_start_flash[];
extern uint32_t __bt_host_code_end_flash[];

static HAL_RMT_IPC_EP_T rmt_ipc_ep;

#if defined(BTH_TRC_TO_SYS) && defined(RMT_TRC_IN_MSG_CHAN)
static BT_HOST_RX_IRQ_HANDLER app_rx_hdlr;

static unsigned int general_rx_handler(const void *data, unsigned int len)
{
    if (bt_host_trace_rx_handler(data, len)) {
        return len;
    }

    if (app_rx_hdlr) {
        return app_rx_hdlr(data, len);
    }

    return len;
}
#endif

int bt_host_open(BT_HOST_RX_IRQ_HANDLER rx_hdlr, BT_HOST_TX_IRQ_HANDLER tx_hdlr)
{
    uint32_t code_start;
    POSSIBLY_UNUSED int ret;
    uint32_t entry;
    uint32_t sp;

    hal_psc_bth_enable();
    hal_cmu_bth_clock_enable();
    hal_cmu_bth_reset_clear();

    osDelay(2);

    hal_cmu_bth_ram_cfg();

#ifdef BTH_FLASH_BOOT
    enum HAL_FLASH_ID_T id;

    id = hal_norflash_addr_to_id(BTH_FLASH_BOOT_STRUCT_ADDR);
    ASSERT(id < HAL_FLASH_ID_NUM, "%s: addr not in flash: 0x%08X", __func__, BTH_FLASH_BOOT_STRUCT_ADDR);
    if (id != HAL_FLASH_ID_0) {
        hal_norflash_force_wakeup(id);
    }

    ret = subsys_check_boot_struct(BTH_FLASH_BOOT_STRUCT_ADDR, &code_start);
    if (ret) {
        ASSERT(false, "%s: Bad subsys boot struct: %d", __func__, ret);
        return 1;
    }
#else
    uint32_t addr;
    uint32_t len;
    int check_magic;

#ifdef BTH_LOAD_FLASH_ADDR
    addr = BTH_LOAD_FLASH_ADDR;
    len = BTH_LOAD_FLASH_SIZE;
    check_magic = true;
#else
    addr = (uint32_t)__bt_host_code_start_flash;
    len = (uint32_t)__bt_host_code_end_flash - addr;
    check_magic = false;
#endif

    ret = subsys_loader_load_image(addr, len, check_magic, NULL, &code_start);
    if (ret) {
        ASSERT(false, "%s: Failed to load subsys simple image: %d", __func__, ret);
        return 2;
    }
#endif

#ifndef NO_BTH_IPC_WHEN_OPEN
#if defined(BTH_TRC_TO_SYS) && defined(RMT_TRC_IN_MSG_CHAN)
    app_rx_hdlr = rx_hdlr;
    rx_hdlr = general_rx_handler;
#endif
    ret = hal_rmt_ipc_open(HAL_RMT_IPC_CORE_BTH_C0, HAL_RMT_IPC_CHAN_0, rx_hdlr, tx_hdlr, false, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_open failed: %d", ret);

    ret = hal_rmt_ipc_start_recv(rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_start_recv failed: %d", ret);

#ifdef BTH_TRC_TO_SYS
    bt_host_trace_server_open();
#endif
#endif

    sp = BTH_MAILBOX_BASE;
    entry = code_start | 1;

    hal_cmu_bth_start_cpu(sp, entry);

    return 0;
}

int bt_host_opened(void)
{
    return hal_rmt_ipc_opened(rmt_ipc_ep);
}

int bt_host_close(void)
{
    TR_INFO(0, "%s", __FUNCTION__);

#ifdef BTH_TRC_TO_SYS
    bt_host_trace_server_close();
#endif

    hal_rmt_ipc_close(rmt_ipc_ep);

    hal_cmu_bth_reset_set();
    hal_cmu_bth_clock_disable();
    hal_psc_bth_disable();

    return 0;
}

int bt_host_send_seq(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_rmt_ipc_send_seq(rmt_ipc_ep, data, len, seq);
}

int bt_host_send(const void *data, unsigned int len)
{
    return hal_rmt_ipc_send(rmt_ipc_ep, data, len);
}

int bt_host_tx_active(unsigned int seq)
{
    return hal_rmt_ipc_tx_active(rmt_ipc_ep, seq);
}
