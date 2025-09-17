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
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#include "hal_location.h"
#include "hal_rmt_ipc_pri.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif

#define RMT_IPC_UNIFIED_IRQ

#define RMT_IPC_RX_VERBOSE_TRACE            0 //(1 << 0)
#define RMT_IPC_TX_VERBOSE_TRACE            0 //(1 << 0)

SRAM_BSS_LOC
static uint32_t chan_wake_alloc_map;
SRAM_BSS_LOC
static uint32_t chan_tx_busy_map;

static void _rmt_ipc_tx_busy(uint32_t wake_lock_idx, bool busy)
{
    uint32_t mask;

    mask = (1 << wake_lock_idx);

    if (busy) {
        if (chan_tx_busy_map == 0) {
            hal_bus_wake_lock(HAL_BUS_WAKE_LOCK_USER_RMT_IPC);
        }
        chan_tx_busy_map |= mask;
    } else {
        chan_tx_busy_map &= ~mask;
        if (chan_tx_busy_map == 0) {
            hal_bus_wake_unlock(HAL_BUS_WAKE_LOCK_USER_RMT_IPC);
        }
    }
}

void hal_rmt_ipc_rx_irq_handler(const struct HAL_RMT_IPC_CFG_T *cfg)
{
    uint32_t chan;
    struct HAL_RMT_IPC_CH_CFG_T *all_chan_cfg;
    const struct HAL_RMT_IPC_MSG_T *msg_ptr;
    struct HAL_RMT_IPC_MSG_T *next;
    unsigned int len;
    const void *data;
    unsigned int processed;

    all_chan_cfg = ADDR_DEV_TO_CPU(cfg->chan_cfg);

    for (chan = 0; chan < cfg->chan_num; chan++) {
#ifdef RMT_IPC_UNIFIED_IRQ
        if (!(NVIC_GetEnableIRQ(cfg->rx_irq_id[chan]) && cfg->irq_active(chan, HAL_RMT_IPC_IRQ_DATA_IND))) {
            continue;
        }
#else
        bool irq_pending = false;

        if (!NVIC_GetActive(cfg->rx_irq_id[chan])) {
            // Check pending IRQ for manual IRQ handler loop inside int_lock env
            if (NVIC_GetPendingIRQ(cfg->rx_irq_id[chan]) && NVIC_GetEnableIRQ(cfg->rx_irq_id[chan])) {
                irq_pending = true;
            } else {
                continue;
            }
        }
#endif

        cfg->rx_irq_suspend(chan);

        if (!all_chan_cfg[chan].chan_opened) {
            goto _clear_pending_irq;
        }

        if (all_chan_cfg[chan].recv_pending_head.data) {
            // Previous unfinished message
            msg_ptr = &all_chan_cfg[chan].recv_pending_head;
        } else if (all_chan_cfg[chan].recv_pending_head.next) {
            // Previous unprocessed message
            msg_ptr = all_chan_cfg[chan].recv_pending_head.next;
        } else {
            // New message
            if (!all_chan_cfg[chan].recv_msg_list_pp) {
                TR_INFO(0, "%s-RX:%d: Got peer=%p (%p)", cfg->name, chan, cfg->peer_cfg_pp, *cfg->peer_cfg_pp);

                all_chan_cfg[chan].recv_msg_list_pp = (const struct HAL_RMT_IPC_MSG_T **)
                    &(*cfg->peer_cfg_pp)->chan_cfg[chan].send_msg_list_p;
                TR_INFO(0, "%s-RX:%d: Got rxPtr=%p", cfg->name, chan, all_chan_cfg[chan].recv_msg_list_pp);
            }
            msg_ptr = *all_chan_cfg[chan].recv_msg_list_pp;
        }
        while (msg_ptr) {
            next = msg_ptr->next;
            len = msg_ptr->len;
            data = msg_ptr->data;
            if (RMT_IPC_RX_VERBOSE_TRACE & (1 << chan)) {
                TR_INFO(0, "%s-RX:%d: msg=%p len=%u data=%p content=0x%08X 0x%08X",
                    cfg->name, chan, msg_ptr, len, data, data ? *(uint32_t *)data : 0,
                    data ? *(uint32_t *)(data + 4) : 0);
            }
            if (all_chan_cfg[chan].rx_irq_handler) {
                all_chan_cfg[chan].chan_rx_busy = true;
                all_chan_cfg[chan].chan_in_rx_hdlr = true;
                processed = all_chan_cfg[chan].rx_irq_handler(data, len);
                all_chan_cfg[chan].chan_in_rx_hdlr = false;
                // Check if flow control needed
                if (all_chan_cfg[chan].chan_rx_busy && data && processed < len) {
                    all_chan_cfg[chan].recv_pending_head.next = next;
                    all_chan_cfg[chan].recv_pending_head.len = len - processed;
                    all_chan_cfg[chan].recv_pending_head.data = data + processed;
                    break;
                }
            } else {
                // Error
                ASSERT(false, "%s-RX:%d: Handler missing", cfg->name, chan);
                break;
            }
            msg_ptr = next;
        }

        if (msg_ptr == NULL) {
            all_chan_cfg[chan].recv_pending_head.next = NULL;
            all_chan_cfg[chan].recv_pending_head.data = NULL;
            // Auto rx done, or already rx done in rx handler
            if (!all_chan_cfg[chan].chan_manual_rx_done || !all_chan_cfg[chan].chan_rx_busy) {
                all_chan_cfg[chan].chan_rx_busy = false;
                cfg->rx_done(chan);
            }
        }

_clear_pending_irq:;
#ifndef RMT_IPC_UNIFIED_IRQ
        if (irq_pending) {
            // Clear NVIC pending state at last (after clearing the IRQ source)
            NVIC_ClearPendingIRQ(cfg->rx_irq_id[chan]);
        }
#endif
    }
}

static void _rmt_ipc_tx_irq_handler_by_chan(const struct HAL_RMT_IPC_CFG_T *cfg, uint32_t chan, struct HAL_RMT_IPC_CH_CFG_T *chan_cfg)
{
    struct HAL_RMT_IPC_MSG_T *msg_ptr;

#ifdef RMT_IPC_UNIFIED_IRQ
    if (!(NVIC_GetEnableIRQ(cfg->tx_irq_id[chan]) && cfg->irq_active(chan, HAL_RMT_IPC_IRQ_DATA_DONE))) {
        return;
    }
#else
    bool irq_pending = false;

    if (!NVIC_GetActive(cfg->tx_irq_id[chan])) {
        // Check pending IRQ for manual IRQ handler loop inside int_lock env
        if (NVIC_GetPendingIRQ(cfg->tx_irq_id[chan]) && NVIC_GetEnableIRQ(cfg->tx_irq_id[chan])) {
            irq_pending = true;
        } else {
            return;
        }
    }
#endif

    cfg->local_tx_irq_clear(chan);

    if (!chan_cfg->chan_opened) {
        goto _clear_pending_irq;
    }

    msg_ptr = chan_cfg->send_msg_list_p;
    while (msg_ptr) {
        msg_ptr = ADDR_DEV_TO_CPU(msg_ptr);
        if (chan_cfg->tx_irq_handler) {
#ifdef RMT_IPC_SEND_DATA_RELAY
            if (cfg->send_data_relay) {
                msg_ptr->data = msg_ptr->org_data;
            }
#endif
            chan_cfg->tx_irq_handler(ADDR_DEV_TO_CPU(msg_ptr->data), msg_ptr->len);
        };
        CONTAINER_OF(msg_ptr, struct HAL_RMT_IPC_SEND_RECORD_T, msg)->in_use = false;
        msg_ptr = msg_ptr->next;
    }

    if (chan_cfg->send_pending_list_p) {
#ifdef RMT_IPC_SEND_DATA_RELAY
        if (cfg->send_data_relay) {
            struct HAL_RMT_IPC_MSG_T *cur;
            int ret;

            cur = chan_cfg->send_pending_list_p;
            do {
                cur->org_data = cur->data;
                ret = cfg->send_data_relay(chan_cfg, cur);
                if (ret == 0) {
                    break;
                }
                CONTAINER_OF(cur, struct HAL_RMT_IPC_SEND_RECORD_T, msg)->in_use = false;
                cur = cur->next;
            } while (cur);
            if (cur) {
                chan_cfg->send_msg_list_p = ADDR_CPU_TO_DEV(cur);
                chan_cfg->send_pending_list_p = cur->next;
                cur->next = NULL;
                goto _set_peer_irq;
            } else {
                chan_cfg->send_pending_list_p = NULL;
                goto _no_pending_msg;
            }
        }
#endif
#ifdef CPU_TO_DEV_ADDR_REMAP
        struct HAL_RMT_IPC_MSG_T *cur;
        struct HAL_RMT_IPC_MSG_T *next;

        cur = chan_cfg->send_pending_list_p;
        while (cur->next) {
            next = cur->next;
            cur->next = ADDR_CPU_TO_DEV(next);
            cur = next;
        }
#endif
        chan_cfg->send_msg_list_p = ADDR_CPU_TO_DEV(chan_cfg->send_pending_list_p);
        chan_cfg->send_pending_list_p = NULL;
_set_peer_irq: POSSIBLY_UNUSED;
        __DSB();
        cfg->peer_tx_irq_set(chan);
    } else {
_no_pending_msg: POSSIBLY_UNUSED;
        chan_cfg->send_msg_list_p = NULL;
        if (cfg->wake_lock_en) {
            // Allow sleep
            _rmt_ipc_tx_busy(chan_cfg->wake_lock_idx, false);
        }
    }

_clear_pending_irq:;
#ifndef RMT_IPC_UNIFIED_IRQ
    if (irq_pending) {
        // Clear NVIC pending state at last (after clearing the IRQ source)
        NVIC_ClearPendingIRQ(cfg->tx_irq_id[chan]);
    }
#endif
}

void hal_rmt_ipc_tx_irq_handler(const struct HAL_RMT_IPC_CFG_T *cfg)
{
    uint32_t chan;
    struct HAL_RMT_IPC_CH_CFG_T *all_chan_cfg;

    all_chan_cfg = ADDR_DEV_TO_CPU(cfg->chan_cfg);

    for (chan = 0; chan < cfg->chan_num; chan++) {
        _rmt_ipc_tx_irq_handler_by_chan(cfg, chan, &all_chan_cfg[chan]);
    }
}

#ifdef CORE_SLEEP_POWER_DOWN
int hal_rmt_ipc_pm_notif_handler(const struct HAL_RMT_IPC_CFG_T *cfg, enum HAL_PM_STATE_T state)
{
    struct HAL_RMT_IPC_CH_CFG_T *chan_cfg;

    chan_cfg = ADDR_DEV_TO_CPU(cfg->chan_cfg);

    if (state == HAL_PM_STATE_POWER_DOWN_WAKEUP) {
        for (uint32_t i = 0; i < cfg->chan_num; i++) {
            if (chan_cfg[i].chan_opened) {
                cfg->irq_init(i);
            }
        }
    }

    return 0;
}
#endif

static int _rmt_ipc_check_config(const struct HAL_RMT_IPC_CFG_T *cfg)
{
    if (!cfg->name) {
        return -1;
    }
    if (!cfg->peer_cfg_pp) {
        return -2;
    }
    if (!cfg->local_cfg_pp) {
        return -3;
    }
    if (!cfg->irq_init) {
        return -4;
    }
    if (!cfg->peer_tx_irq_set) {
        return -5;
    }
    if (!cfg->local_tx_irq_clear) {
        return -6;
    }
    if (!cfg->rx_irq_suspend) {
        return -7;
    }
    if (!cfg->rx_irq_resume) {
        return -8;
    }
    if (!cfg->rx_done) {
        return -9;
    }
    if (!cfg->irq_active) {
        return -10;
    }
    if (!cfg->rx_irq_entry) {
        return -11;
    }
    if (!cfg->tx_irq_entry) {
        return -12;
    }
    if (!cfg->chan_num) {
        return -13;
    }
    if (!cfg->rec_num_per_chan) {
        return -14;
    }
    if (!cfg->chan_cfg) {
        return -15;
    }
    if (!cfg->send_record) {
        return -16;
    }
    if (!cfg->rx_irq_id) {
        return -17;
    }
    if (!cfg->tx_irq_id) {
        return -18;
    }
    return 0;
}

int hal_rmt_ipc_open(enum HAL_RMT_IPC_CORE_T core, enum HAL_RMT_IPC_CHAN_T chan,
                     HAL_RMT_IPC_RX_IRQ_HANDLER rxhandler, HAL_RMT_IPC_TX_IRQ_HANDLER txhandler,
                     int manual_rx_done, HAL_RMT_IPC_EP_T *ep_out)
{
    RMT_IPC_OPEN_CFG_QUAL struct HAL_RMT_IPC_CFG_T *cfg;
    int ret;
    uint32_t lock;
    uint32_t wake_lock_idx = ~0UL;

    cfg = (RMT_IPC_OPEN_CFG_QUAL struct HAL_RMT_IPC_CFG_T *)hal_rmt_ipc_get_cfg(core);
    if (cfg == NULL) {
        ASSERT(false, "RMT_IPC-OPEN: Failed to get cfg: core=%d", core);
        return 1;
    }

    ret = _rmt_ipc_check_config(cfg);
    if (ret) {
        return ret;
    }

    if (chan >= cfg->chan_num) {
        return 2;
    }

    manual_rx_done = !!manual_rx_done;

    if (cfg->chan_cfg[chan].chan_opened) {
        ASSERT(cfg->chan_cfg[chan].chan_manual_rx_done == manual_rx_done, "%s-OPEN:%d: manual_rx_done=%d (should be %d)",
            cfg->name, chan, manual_rx_done, cfg->chan_cfg[chan].chan_manual_rx_done);
        return 3;
    }

    if (cfg->wake_lock_en) {
        lock = int_lock();
        for (wake_lock_idx = 0; wake_lock_idx < sizeof(chan_wake_alloc_map) * 8; wake_lock_idx++) {
            if ((chan_wake_alloc_map & (1 << wake_lock_idx)) == 0) {
                chan_wake_alloc_map |= (1 << wake_lock_idx);
                break;
            }
        }
        int_unlock(lock);
        if (wake_lock_idx >= sizeof(chan_wake_alloc_map) * 8) {
            ASSERT(false, "%s-OPEN:%d: Failed to alloc wake lock idx", cfg->name, chan);
            return 4;
        }
    }

    // Stop IRQs by default
    NVIC_DisableIRQ(cfg->rx_irq_id[chan]);
    NVIC_DisableIRQ(cfg->tx_irq_id[chan]);

    NVIC_SetVector(cfg->rx_irq_id[chan], (uint32_t)cfg->rx_irq_entry);
    NVIC_SetPriority(cfg->rx_irq_id[chan], IRQ_PRIORITY_NORMAL);

    NVIC_SetVector(cfg->tx_irq_id[chan], (uint32_t)cfg->tx_irq_entry);
    NVIC_SetPriority(cfg->tx_irq_id[chan], IRQ_PRIORITY_NORMAL);

    cfg->irq_init(chan);

    memset(&cfg->chan_cfg[chan], 0, sizeof(cfg->chan_cfg[chan]));

    cfg->chan_cfg[chan].wake_lock_idx = wake_lock_idx;
    cfg->chan_cfg[chan].chan_manual_rx_done = manual_rx_done;
    cfg->chan_cfg[chan].send_record = cfg->send_record + chan * cfg->rec_num_per_chan;
    cfg->chan_cfg[chan].send_rec_num = cfg->rec_num_per_chan;

    memset(cfg->chan_cfg[chan].send_record, 0,
        sizeof(*cfg->chan_cfg[chan].send_record) * cfg->chan_cfg[chan].send_rec_num);

#ifdef CORE_SLEEP_POWER_DOWN
    if (cfg->pm_notif_handler) {
        uint32_t i;

        for (i = 0; i < cfg->chan_num; i++) {
            if (cfg->chan_cfg[i].chan_opened) {
                break;
            }
        }
        if (i >= cfg->chan_num) {
            hal_pm_notif_register(HAL_PM_USER_HAL, cfg->pm_notif_handler);
        }
    }
#endif

    cfg->chan_cfg[chan].rx_irq_handler = rxhandler;
    cfg->chan_cfg[chan].tx_irq_handler = txhandler;
    cfg->chan_cfg[chan].cfg = cfg;
    cfg->chan_cfg[chan].chan = chan;
    cfg->chan_cfg[chan].chan_opened = true;

    if (ep_out) {
        *ep_out = &cfg->chan_cfg[chan];
    }

    // Enable TX IRQ
    NVIC_ClearPendingIRQ(cfg->tx_irq_id[chan]);
    NVIC_EnableIRQ(cfg->tx_irq_id[chan]);

    *cfg->local_cfg_pp = ADDR_CPU_TO_DEV(cfg);
#ifdef CPU_TO_DEV_ADDR_REMAP
    cfg->chan_cfg = ADDR_CPU_TO_DEV(cfg->chan_cfg);
#endif
    __DSB();

    TR_INFO(0, "%s-OPEN:%d: peer=%p (n/a) local=%p (%p)",
        cfg->name, chan, cfg->peer_cfg_pp, cfg->local_cfg_pp, *cfg->local_cfg_pp);
    TR_INFO(0, "%s-OPEN:%d: rxPtr=(n/a) txPtr=%p manualRxDone=%d",
        cfg->name, chan, &cfg->chan_cfg[chan].send_msg_list_p, cfg->chan_cfg[chan].chan_manual_rx_done);

    return 0;
}

static struct HAL_RMT_IPC_CH_CFG_T *_rmt_ipc_ep_to_chan_cfg(HAL_RMT_IPC_EP_T ep)
{
    struct HAL_RMT_IPC_CH_CFG_T *chan_cfg;

    chan_cfg = (struct HAL_RMT_IPC_CH_CFG_T *)ep;
    return chan_cfg;
}

int hal_rmt_ipc_close(HAL_RMT_IPC_EP_T ep)
{
    uint32_t lock;
    struct HAL_RMT_IPC_CH_CFG_T *chan_cfg;
    const struct HAL_RMT_IPC_CFG_T *cfg;
    uint32_t chan;

    if (ep == NULL) {
        return -100;
    }

    chan_cfg = _rmt_ipc_ep_to_chan_cfg(ep);
    if (!chan_cfg->chan_opened) {
        return 2;
    }

    cfg = chan_cfg->cfg;
    chan = chan_cfg->chan;

    // Stop IRQs by default
    NVIC_DisableIRQ(cfg->rx_irq_id[chan]);
    NVIC_DisableIRQ(cfg->tx_irq_id[chan]);

    // Clear IRQs generated by local
    cfg->irq_init(chan);

    lock = int_lock();
    if (cfg->wake_lock_en) {
        // Release tx busy state
        _rmt_ipc_tx_busy(chan_cfg->wake_lock_idx, false);
        chan_wake_alloc_map &= ~(1 << chan_cfg->wake_lock_idx);
    }
    int_unlock(lock);

    memset(chan_cfg->send_record, 0,
        sizeof(*chan_cfg->send_record) * chan_cfg->send_rec_num);
    memset(chan_cfg, 0, sizeof(*chan_cfg));

#ifdef CORE_SLEEP_POWER_DOWN
    if (cfg->pm_notif_handler) {
        uint32_t i;
        const struct HAL_RMT_IPC_CH_CFG_T *all_chan_cfg;

        all_chan_cfg = ADDR_DEV_TO_CPU(cfg->chan_cfg);
        for (i = 0; i < cfg->chan_num; i++) {
            if (all_chan_cfg[i].chan_opened) {
                break;
            }
        }
        if (i >= cfg->chan_num) {
            hal_pm_notif_deregister(HAL_PM_USER_HAL, cfg->pm_notif_handler);
        }
    }
#endif

    return 0;
}

int hal_rmt_ipc_start_recv(HAL_RMT_IPC_EP_T ep)
{
    struct HAL_RMT_IPC_CH_CFG_T *chan_cfg;
    const struct HAL_RMT_IPC_CFG_T *cfg;
    uint32_t lock;
    uint32_t chan;

    if (ep == NULL) {
        return -100;
    }

    chan_cfg = _rmt_ipc_ep_to_chan_cfg(ep);
    if (!chan_cfg->chan_opened) {
        return 2;
    }

    cfg = chan_cfg->cfg;
    chan = chan_cfg->chan;

    NVIC_ClearPendingIRQ(cfg->rx_irq_id[chan]);
    NVIC_EnableIRQ(cfg->rx_irq_id[chan]);

    lock = int_lock();
    // Check if there is any previous unfinished message
    if (chan_cfg->recv_pending_head.data) {
        cfg->rx_irq_resume(chan);
    }
    int_unlock(lock);

    return 0;
}

int hal_rmt_ipc_stop_recv(HAL_RMT_IPC_EP_T ep)
{
    struct HAL_RMT_IPC_CH_CFG_T *chan_cfg;
    const struct HAL_RMT_IPC_CFG_T *cfg;
    uint32_t chan;

    if (ep == NULL) {
        return -100;
    }

    chan_cfg = _rmt_ipc_ep_to_chan_cfg(ep);
    if (!chan_cfg->chan_opened) {
        return 2;
    }

    cfg = chan_cfg->cfg;
    chan = chan_cfg->chan;

    NVIC_DisableIRQ(cfg->rx_irq_id[chan]);

    return 0;
}

int hal_rmt_ipc_send_seq(HAL_RMT_IPC_EP_T ep, const void *data, unsigned int len, unsigned int *seq)
{
    uint32_t lock;
    int ret;
    struct HAL_RMT_IPC_CH_CFG_T *chan_cfg;
    const struct HAL_RMT_IPC_CFG_T *cfg;
    struct HAL_RMT_IPC_SEND_RECORD_T *record;
    struct HAL_RMT_IPC_MSG_T *next;
    uint32_t chan;
    int i;

    if (ep == NULL) {
        return -100;
    }

    chan_cfg = _rmt_ipc_ep_to_chan_cfg(ep);
    if (!chan_cfg->chan_opened) {
        return 2;
    }

    cfg = chan_cfg->cfg;
    chan = chan_cfg->chan;

    ret = -1;
    record = chan_cfg->send_record;

    lock = int_lock();
    for (i = 0; i < chan_cfg->send_rec_num; i++) {
        if (record->in_use) {
            record++;
            continue;
        }
        record->in_use = true;
        record->msg.next = NULL;
        record->msg.len = len;
        record->msg.data = ADDR_CPU_TO_DEV(data);
        if (RMT_IPC_TX_VERBOSE_TRACE & (1 << chan)) {
            TR_INFO(0, "%s-TX:%d: msg=%p len=%u data=%p content=0x%08X 0x%08X",
                cfg->name, chan, &record->msg, len, data,
                data ? *(uint32_t *)data : 0, data ? *(uint32_t *)(data + 4) : 0);
        }
        if (chan_cfg->send_msg_list_p == NULL) {
#ifdef RMT_IPC_SEND_DATA_RELAY
            if (cfg->send_data_relay) {
                record->msg.org_data = record->msg.data;
                ret = cfg->send_data_relay(chan_cfg, &record->msg);
                if (ret) {
                    record->in_use = false;
                    break;
                }
            }
#endif
            chan_cfg->send_msg_list_p = ADDR_CPU_TO_DEV(&record->msg);
            __DSB();
            cfg->peer_tx_irq_set(chan);
        } else if (chan_cfg->send_pending_list_p == NULL) {
            chan_cfg->send_pending_list_p = &record->msg;
        } else {
            next = chan_cfg->send_pending_list_p;
            while (next->next) {
                next = next->next;
            }
            next->next = &record->msg;
        }
        ret = 0;
        if (seq) {
            *seq = i;
        }
        if (cfg->wake_lock_en) {
            // Prohibit sleep here
            _rmt_ipc_tx_busy(chan_cfg->wake_lock_idx, true);
        }
        break;
    }
    int_unlock(lock);

#ifdef RMT_IPC_SEND_CALLBACK
    HAL_RMT_IPC_SEND_CALLBACK send_cb = chan_cfg->send_cb;
    if (send_cb) {
        send_cb();
    }
#endif

    return ret;
}

int hal_rmt_ipc_send(HAL_RMT_IPC_EP_T ep, const void *data, unsigned int len)
{
    return hal_rmt_ipc_send_seq(ep, data, len, NULL);
}

int hal_rmt_ipc_rx_done(HAL_RMT_IPC_EP_T ep)
{
    struct HAL_RMT_IPC_CH_CFG_T *chan_cfg;
    const struct HAL_RMT_IPC_CFG_T *cfg;
    uint32_t chan;
    uint32_t lock;

    if (ep == NULL) {
        return -100;
    }

    chan_cfg = _rmt_ipc_ep_to_chan_cfg(ep);
    if (!chan_cfg->chan_opened) {
        return 2;
    }

    cfg = chan_cfg->cfg;
    chan = chan_cfg->chan;

    lock = int_lock();
    if (chan_cfg->chan_in_rx_hdlr) {
        // Tell IRQ to ignore the return value of rx handler
        chan_cfg->chan_rx_busy = false;
    } else {
        // Clean up previous unfinished message if any
        chan_cfg->recv_pending_head.data = NULL;
        if (chan_cfg->recv_pending_head.next) {
            // Resume if there is any previous unprocessed message
            cfg->rx_irq_resume(chan);
        } else if (chan_cfg->chan_rx_busy) {
            chan_cfg->chan_rx_busy = false;
            cfg->rx_done(chan);
        }
    }
    int_unlock(lock);

    return 0;
}

int hal_rmt_ipc_tx_irq_run(HAL_RMT_IPC_EP_T ep)
{
    struct HAL_RMT_IPC_CH_CFG_T *chan_cfg;
    const struct HAL_RMT_IPC_CFG_T *cfg;
    uint32_t chan;
    uint32_t lock;

    if (ep == NULL) {
        return -100;
    }

    chan_cfg = _rmt_ipc_ep_to_chan_cfg(ep);

    if (!chan_cfg->chan_opened) {
        return 2;
    }

    cfg = chan_cfg->cfg;
    chan = chan_cfg->chan;

    lock = int_lock();
    _rmt_ipc_tx_irq_handler_by_chan(cfg, chan, chan_cfg);
    int_unlock(lock);

    return 0;
}

int hal_rmt_ipc_tx_active(HAL_RMT_IPC_EP_T ep, unsigned int seq)
{
    struct HAL_RMT_IPC_CH_CFG_T *chan_cfg;

    if (ep == NULL) {
        return -100;
    }

    chan_cfg = _rmt_ipc_ep_to_chan_cfg(ep);

    if (!chan_cfg->chan_opened) {
        return 2;
    }

    if (seq < chan_cfg->send_rec_num) {
        return chan_cfg->send_record[seq].in_use;
    } else if (seq == HAL_RMT_IPC_ALL_SEND_SEQ) {
        uint32_t i;
        uint32_t lock;

        lock = int_lock();
        for (i = 0; i < chan_cfg->send_rec_num; i++) {
            if (chan_cfg->send_record[i].in_use) {
                break;
            }
        }
        int_unlock(lock);
        if (i < chan_cfg->send_rec_num) {
            return true;
        }
    }

    return false;
}

int hal_rmt_ipc_opened(HAL_RMT_IPC_EP_T ep)
{
    struct HAL_RMT_IPC_CH_CFG_T *chan_cfg;

    if (ep == NULL) {
        return false;
    }

    chan_cfg = _rmt_ipc_ep_to_chan_cfg(ep);

    return chan_cfg->chan_opened;
}

int hal_rmt_ipc_peer_opened(HAL_RMT_IPC_EP_T ep)
{
    struct HAL_RMT_IPC_CH_CFG_T *chan_cfg;

    if (ep == NULL) {
        return false;
    }

    chan_cfg = _rmt_ipc_ep_to_chan_cfg(ep);
    if (!chan_cfg->chan_opened) {
        return false;
    }

    // If rx peer msg
    return !!chan_cfg->recv_msg_list_pp;
}

int hal_rmt_ipc_get_ep(enum HAL_RMT_IPC_CORE_T core, enum HAL_RMT_IPC_CHAN_T chan, HAL_RMT_IPC_EP_T *ep_out)
{
    const struct HAL_RMT_IPC_CFG_T *cfg;

    if (ep_out == NULL) {
        return -1;
    }

    cfg = hal_rmt_ipc_get_cfg(core);
    if (cfg == NULL) {
        ASSERT(false, "rmt_ipc_get_id: Failed to get cfg: core=%d", core);
        return 1;
    }

    if (chan >= cfg->chan_num) {
        return 2;
    }

    *ep_out = (HAL_RMT_IPC_EP_T)ADDR_DEV_TO_CPU(&cfg->chan_cfg[chan]);
    return 0;
}

int hal_rmt_ipc_update_cfg_loc(enum HAL_RMT_IPC_CORE_T core, void *peer_loc, void *local_loc)
{
    struct HAL_RMT_IPC_CFG_T *cfg;

    cfg = (struct HAL_RMT_IPC_CFG_T *)hal_rmt_ipc_get_cfg(core);
    if (cfg == NULL) {
        ASSERT(false, "rmt_ipc_update_cfg_ptr: Failed to get cfg: core=%d", core);
        return 1;
    }

    cfg->peer_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)peer_loc;
    cfg->local_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)local_loc;
    return 0;
}

#ifdef RMT_IPC_SEND_CALLBACK
int hal_rmt_ipc_register_send_callback(HAL_RMT_IPC_EP_T ep, HAL_RMT_IPC_SEND_CALLBACK cb)
{
    struct HAL_RMT_IPC_CH_CFG_T *chan_cfg;

    if (ep == NULL) {
        return -100;
    }

    chan_cfg = _rmt_ipc_ep_to_chan_cfg(ep);

    chan_cfg->send_cb = cb;
    return 0;
}
#endif

#ifdef OPENAMP_ENABLE
int hal_rmt_ipc_get_rsc_table(HAL_RMT_IPC_EP_T ep, uint32_t *rsc_addr, uint32_t *rsc_sz)
{
    const struct HAL_RMT_IPC_CFG_T *cfg;
    struct HAL_RMT_IPC_CH_CFG_T *chan_cfg;
    uint32_t start, end;
    uint32_t cnt = 100;

    if (!ep) {
        return -1;
    }

    while (!hal_rmt_ipc_peer_opened(ep) && cnt) {
        osDelay(10);
        cnt--;
    }
    if (cnt == 0) {
        TR_INFO(0, "Error: IPC can not open peer");
        return -2;
    }

    chan_cfg = _rmt_ipc_ep_to_chan_cfg(ep);
    if (!chan_cfg) {
        ASSERT(false, "rmt_ipc_update_cfg_ptr: Failed to get cfg");
        return -1;
    }
    cfg = chan_cfg->cfg;

    start = (*cfg->peer_cfg_pp)->openamp_rsc_start;
    end = (*cfg->peer_cfg_pp)->openamp_rsc_end;
    *rsc_addr  = start;
    *rsc_sz  = end - start;

    return 0;
}
#endif
