/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "plat_addr_map.h"

#ifdef BT_CMU_BASE

#include "hwtest.h"
#ifdef BT_DRV_TEST
#include "bt_drv_interface.h"
#endif
#include "cmsis.h"
#include "hal_analogif.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_intersys.h"
#include "hal_psc.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hwtimer_list.h"
#include "stdbool.h"
#include "string.h"

#ifdef BTPCM_TEST
#include "hal_btpcm.h"
#endif

#define BT_RX_WAKE_LOCK                 HAL_CPU_WAKE_LOCK_USER_10
#define BT_TX_WAKE_LOCK                 HAL_CPU_WAKE_LOCK_USER_11
#define BT_TIMER_WAKE_LOCK              HAL_CPU_WAKE_LOCK_USER_12

enum BT_CONN_STATE_T {
    BT_CONN_WAIT_START,
    BT_CONN_SEND_HCI_RESET,
    BT_CONN_HCI_RESET_ACK,
    BT_CONN_SEND_LINK_POLICY,
    BT_CONN_LINK_POLICY_ACK,
    BT_CONN_SEND_CHANGE_BDADDR,
    BT_CONN_CHANGE_BDADDR_ACK,
    BT_CONN_SEND_SET_SCAN_INTERVAL,
    BT_CONN_SET_SCAN_INTERVAL_ACK,
    BT_CONN_SEND_SCAN,
    BT_CONN_SCAN_ACK,
    BT_CONN_LINK_STATUS,
    BT_CONN_SEND_ACCEPT,
    BT_CONN_ACCEPT_ACK,
    BT_CONN_CONN_STATUS,
    BT_CONN_TEST_EVT,
    BT_CONN_SNIFFER,
    BT_CONN_WAIT_SEND_DATA,
    BT_CONN_SEND_DATA,
    BT_CONN_DATA_ACK,
    BT_CONN_SCO_CONN_REQ,
    BT_CONN_SEND_SCO_CONN_ACCEPT,
    BT_CONN_SCO_CONN_ACCEPT_ACK,
    BT_CONN_DONE,
};

static const enum HAL_INTERSYS_ID_T conn_intersys = HAL_INTERSYS_ID_0;

static enum BT_CONN_STATE_T bt_state = BT_CONN_WAIT_START;

static HWTIMER_ID timer_id;

static bool bt_sleep_only = false;

static volatile bool bt_rx_done = false;
static volatile bool bt_tx_done = false;
static volatile bool timer_evt = false;

static unsigned char bt_rx_data[128];
static unsigned int bt_rx_len = 0;

static unsigned char cmd_data[0x500];
static unsigned int cmd_data_len;

static const unsigned char cmd_hci_reset[] = {
    0x01, 0x03, 0x0c, 0x00,
};
static const unsigned char ack_hci_reset[] = {
    0x04, 0x0e, 0x04, 0x05, 0x03, 0x0c, 0x00,
};
static const unsigned char cmd_link_policy[] = {
    0x01, 0x0f, 0x08, 0x02, 0x05, 0x00,
};
static const unsigned char ack_link_policy[] = {
    0x04, 0x0e, 0x04, 0x05, 0x0f, 0x08, 0x00,
};
static const unsigned char cmd_change_bdaddr[] = {
    0x01, 0x40, 0xfc, 0x06, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
};
static const unsigned char ack_change_bdaddr[] = {
    0x04, 0x0E, 0x04, 0x05, 0x40, 0xFC, 0x00,
};
static const unsigned char cmd_set_scan_interval[] = {
    0x01, 0x1c, 0x0c, 0x04, 0x20, 0x00, 0x12, 0x00,
};
static const unsigned char ack_set_scan_interval[] = {
    0x04, 0x0e, 0x04, 0x05, 0x1c, 0x0c, 0x00,
};
static const unsigned char cmd_scan[] = {
    0x01, 0x1a, 0x0c, 0x01, 0x03,
};
static const unsigned char ack_scan[] = {
    0x04, 0x0e, 0x04, 0x05, 0x1a, 0x0c, 0x00,
};
static const unsigned char evt_link_status[] = {
    0x04, 0x04, 0x0A, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0x00, 0x00, 0x00, 0x01,
};
static const unsigned char cmd_accept[] = {
    0x01, 0x09, 0x04, 0x07, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0x01,
};
static const unsigned char ack_accept[] = {
    0x04, 0x0f, 0x04, 0x00, 0x05, 0x09, 0x04,
};
static const unsigned char evt_conn_status[] = {
    0x04, 0x03, 0x0B, 0x00, 0x80, 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0x01, 0x00,
};
static const unsigned char evt_conn_attributs[] = {
    0x04, 0x1b, 0x03, 0x80, 0x00, 0x05,
};
static const unsigned char evt_sniffer[] = {
    0x04, 0x14, 0x06, 0x00, 0x80, 0x00, 0x02, 0x20, 0x00,
};
static const unsigned char ack_data[] = {
    0x04, 0x13, 0x05, 0x01, 0x80, 0x00, 0x01, 0x00,
};
static const unsigned char evt_sco_conn_req[] = {
    0x04, 0x04, 0x0A, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x00, 0x00, 0x00, 0x02,
};
static const unsigned char cmd_sco_conn_accept[] = {
    0x01, 0x29, 0x04, 0x15, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0x40, 0x1f, 0x00,
    0x00, 0x40, 0x1f, 0x00, 0x00, 0x1e, 0x00, 0x40, 0x00, 0x01, 0xc8, 0x03,
};

static void timer_expired(void *param)
{
    timer_evt = true;
    hal_cpu_wake_lock(BT_TIMER_WAKE_LOCK);
}

static unsigned int bt_rx(const unsigned char *data, unsigned int len)
{
    unsigned int processed = len;

    hal_intersys_stop_recv(conn_intersys);
    if (bt_rx_len + processed > sizeof(bt_rx_data)) {
        TRACE(3,"bt_rx data too long: bt_rx_len=%d, len=%d, limit=%d", bt_rx_len, processed, sizeof(bt_rx_data));
        processed = sizeof(bt_rx_data) - bt_rx_len;
    }
    if (processed > 0) {
        memcpy(bt_rx_data + bt_rx_len, data, processed);
        bt_rx_len += processed;
        bt_rx_done = true;
        hal_cpu_wake_lock(BT_RX_WAKE_LOCK);
    }
    return len;
}

static void bt_tx(const unsigned char *data, unsigned int len)
{
    bt_tx_done = true;
    hal_cpu_wake_lock(BT_TX_WAKE_LOCK);

    if (len) {
        unsigned int dump_len = (len <= 16) ? len : 16;
        TRACE(TR_ATTR_NO_LF, "[TX] ");
        DUMP8("%02X ", data, dump_len);
    }
}

POSSIBLY_UNUSED
static void bt_hci_send_cmd_hci_reset(void)
{
    int ret;

    ret = hal_intersys_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_hci_reset, sizeof(cmd_hci_reset));
    ASSERT(ret == 0, "Failed to send hci reset cmd: %d", ret);
    bt_state = BT_CONN_SEND_HCI_RESET;
}

static void bt_hci_send_cmd_link_policy(void)
{
    int ret;

    ret = hal_intersys_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_link_policy, sizeof(cmd_link_policy));
    ASSERT(ret == 0, "Failed to send link policy cmd: %d", ret);
    bt_state = BT_CONN_SEND_LINK_POLICY;
}

static void bt_hci_send_cmd_data(void)
{
    int ret;

    ret = hal_intersys_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_data, cmd_data_len);
    ASSERT(ret == 0, "Failed to send data cmd: %d", ret);
    bt_state = BT_CONN_SEND_DATA;
}

static void handle_timer_evt(void)
{
    TRACE(1,"Timer event: bt_state=%2d", bt_state);

    switch (bt_state) {
        case BT_CONN_WAIT_START:
#ifdef BT_DRV_TEST
            // Avoid HCI-RESET cmd, otherwise btdrv_sleep_config() will be overwritten
            // and BT cannot enter sleep
            bt_hci_send_cmd_link_policy();
#else
            bt_hci_send_cmd_hci_reset();
#endif
            break;
        case BT_CONN_WAIT_SEND_DATA:
            bt_hci_send_cmd_data();
            break;
        default:
            ASSERT(false, "%s: Invalid bt_state=%2d", __FUNCTION__, bt_state);
            break;
    }
}

#ifdef BTPCM_TEST

#define AUDIO_BUFFER_SIZE 2048

struct HAL_BTPCM_CONFIG_T btpcm_cfg;
struct HAL_DMA_CH_CFG_T dma_cfg;
struct HAL_DMA_DESC_T play_desc;
struct HAL_DMA_DESC_T record_desc;
uint32_t play_buffer[AUDIO_BUFFER_SIZE / 4];
uint32_t record_buffer[AUDIO_BUFFER_SIZE / 4];

static void start_btpcm(void)
{
    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.src_periph = (enum HAL_DMA_PERIPH_T)0;
    dma_cfg.handler = NULL;
    dma_cfg.dst_periph = HAL_AUDMA_BTPCM_TX;
    dma_cfg.type = HAL_DMA_FLOW_M2P_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.dst_periph, HAL_DMA_HIGH_PRIO);

    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.dst_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.src_tsize = AUDIO_BUFFER_SIZE/2; /* cause of half word width */
    dma_cfg.try_burst = 1;

    dma_cfg.src = (uint32_t)play_buffer;
    hal_audma_init_desc(&play_desc, &dma_cfg, &play_desc, 0);
    hal_audma_sg_start(&play_desc, &dma_cfg);

    dma_cfg.dst_periph = (enum HAL_DMA_PERIPH_T)0;
    dma_cfg.handler = NULL;
    dma_cfg.src_periph = HAL_AUDMA_BTPCM_RX;
    dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);

    dma_cfg.dst = (uint32_t)record_buffer;
    hal_audma_init_desc(&record_desc, &dma_cfg, &record_desc, 0);
    hal_audma_sg_start(&record_desc, &dma_cfg);

    hal_btpcm_open(HAL_BTPCM_ID_0, AUD_STREAM_PLAYBACK);
    hal_btpcm_open(HAL_BTPCM_ID_0, AUD_STREAM_CAPTURE);
    memset(&btpcm_cfg, 0, sizeof(btpcm_cfg));
    btpcm_cfg.use_dma = 1;
    hal_btpcm_setup_stream(HAL_BTPCM_ID_0, AUD_STREAM_PLAYBACK, &btpcm_cfg);
    hal_btpcm_setup_stream(HAL_BTPCM_ID_0, AUD_STREAM_CAPTURE, &btpcm_cfg);
    hal_btpcm_start_stream(HAL_BTPCM_ID_0, AUD_STREAM_PLAYBACK);
    hal_btpcm_start_stream(HAL_BTPCM_ID_0, AUD_STREAM_CAPTURE);
}

#endif // BTPCM_TEST

#ifdef DPD_BT_TEST

static unsigned char dpd_bt_buf[4096] __attribute__((aligned(0x100)));
static struct HAL_DMA_DESC_T dpd_bt_desc;

void dpd_bt_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
#ifdef CHIP_BEST1000
    //REG(DPD_RX_BASE + 0x30) = 0;
    //REG(DPD_RX_BASE) = 0;
#endif
}

void dpd_bt_test(void)
{
    struct HAL_DMA_CH_CFG_T dma_cfg;

    // FM simulation
    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_16;
    //dma_cfg.dst_periph = HAL_AUDMA_PERIPH_QTY; // useless
    dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
    dma_cfg.handler = dpd_bt_handler;
    dma_cfg.src = 0; // useless
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_periph = HAL_AUDMA_DPD_RX;
    dma_cfg.src_tsize = sizeof(dpd_bt_buf) / 4;
    dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);

    dma_cfg.dst = (uint32_t)&dpd_bt_buf[0];
    hal_audma_init_desc(&dpd_bt_desc, &dma_cfg, &dpd_bt_desc, 1);
    hal_audma_sg_start(&dpd_bt_desc, &dma_cfg);

    REG(0xD03300f0) |= (1 << 0);
    REG(0xD0350208) |= (1 << 16);
    REG(0xD0350228) |= (1 << 16);

#ifdef CHIP_BEST1000
    REG(DPD_RX_BASE + 0x30) = 1;
    REG(DPD_RX_BASE) = 3;
#endif
}

#endif // DPD_BT_TEST

static void handle_rx_done(const unsigned char *data, unsigned int len)
{
    int ret;

    TRACE(1,"Rx done: bt_state=%2d len=%2u", bt_state, len);
    if (len) {
        unsigned int dump_len = (len <= 16) ? len : 16;
        TRACE(TR_ATTR_NO_LF, "[RX] ");
        DUMP8("%02X ", data, dump_len);
    }

    switch (bt_state) {
        case BT_CONN_HCI_RESET_ACK:
            ASSERT(len == sizeof(ack_hci_reset), "Invalid hci reset ack len: %d", len);
            ret = memcmp(ack_hci_reset, data, len);
            ASSERT(ret == 0, "Recv invalid hci reset ack");
            if (bt_sleep_only) {
                bt_state = BT_CONN_DONE;
                TRACE(0, "%s: Bt starts to sleep ...", __func__);
            } else {
                bt_hci_send_cmd_link_policy();
            }
            break;
        case BT_CONN_LINK_POLICY_ACK:
            ASSERT(len == sizeof(ack_link_policy), "Invalid link policy ack len: %d", len);
            ret = memcmp(ack_link_policy, data, len);
            ASSERT(ret == 0, "Recv invalid link policy ack");
            if (0) {
                ret = hal_intersys_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_change_bdaddr, sizeof(cmd_change_bdaddr));
                ASSERT(ret == 0, "Failed to send change bdaddr cmd: %d", ret);
                bt_state = BT_CONN_SEND_CHANGE_BDADDR;
            } else {
                ret = hal_intersys_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_set_scan_interval, sizeof(cmd_set_scan_interval));
                ASSERT(ret == 0, "Failed to send set scan interval cmd: %d", ret);
                bt_state = BT_CONN_SEND_SET_SCAN_INTERVAL;
            }
            break;
        case BT_CONN_CHANGE_BDADDR_ACK:
            ASSERT(len == sizeof(ack_change_bdaddr), "Invalid change bdaddr ack len: %d", len);
            ret = memcmp(ack_change_bdaddr, data, len);
            ASSERT(ret == 0, "Recv invalid change bdaddr ack");
            ret = hal_intersys_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_scan, sizeof(cmd_scan));
            ASSERT(ret == 0, "Failed to send scan cmd: %d", ret);
            bt_state = BT_CONN_SEND_SCAN;
            break;
        case BT_CONN_SET_SCAN_INTERVAL_ACK:
            ASSERT(len == sizeof(ack_set_scan_interval), "Invalid set scan interval ack len: %d", len);
            ret = memcmp(ack_set_scan_interval, data, len);
            ASSERT(ret == 0, "Recv invalid set scan interval ack");
            ret = hal_intersys_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_scan, sizeof(cmd_scan));
            ASSERT(ret == 0, "Failed to send scan cmd: %d", ret);
            bt_state = BT_CONN_SEND_SCAN;
            break;
        case BT_CONN_SCAN_ACK:
            ASSERT(len == sizeof(ack_scan), "Invalid scan ack len: %d", len);
            ret = memcmp(ack_scan, data, len);
            ASSERT(ret == 0, "Recv invalid scan ack");
#ifdef DPD_BT_TEST
            bt_state = BT_CONN_DONE;
            hal_sys_timer_delay(MS_TO_TICKS(50));
            dpd_bt_test();
#else
            bt_state = BT_CONN_LINK_STATUS;
#endif
            break;
        case BT_CONN_LINK_STATUS:
            ASSERT(len == sizeof(evt_link_status), "Invalid link status evt len: %d", len);
            ret = memcmp(evt_link_status, data, len);
            ASSERT(ret == 0, "Recv invalid link status evt");
            ret = hal_intersys_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_accept, sizeof(cmd_accept));
            ASSERT(ret == 0, "Failed to send accept cmd: %d", ret);
            bt_state = BT_CONN_SEND_ACCEPT;
            break;
        case BT_CONN_ACCEPT_ACK:
            ASSERT(len == sizeof(ack_accept), "Invalid accept ack len: %d", len);
            ret = memcmp(ack_accept, data, len);
            ASSERT(ret == 0, "Recv invalid accept ack");
            bt_state = BT_CONN_CONN_STATUS;
            break;
        case BT_CONN_CONN_STATUS:
            ASSERT(len == sizeof(evt_conn_status), "Invalid conn status evt len: %d", len);
            ret = memcmp(evt_conn_status, data, len);
            ASSERT(ret == 0, "Recv invalid conn status evt");
            bt_state = BT_CONN_TEST_EVT;
            break;
        case BT_CONN_TEST_EVT:
            if (len == sizeof(evt_conn_attributs) && memcmp(evt_conn_attributs, data, len) == 0) {
                TRACE(0,"Recv conn attrib evt while waiting for test evt");
            } else if (len == sizeof(evt_sniffer) && memcmp(evt_sniffer, data, len) == 0) {
                // Current state is BT_CONN_SNIFFER
                hwtimer_start(timer_id, MS_TO_TICKS(60));
                bt_state = BT_CONN_WAIT_SEND_DATA;
            } else if (len == sizeof(evt_sco_conn_req) && memcmp(evt_sco_conn_req, data, len) == 0) {
                // Current state is BT_CONN_SCO_CONN_REQ
                ret = hal_intersys_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_sco_conn_accept, sizeof(cmd_sco_conn_accept));
                ASSERT(ret == 0, "Failed to send sync conn accept cmd: %d", ret);
                bt_state = BT_CONN_SEND_SCO_CONN_ACCEPT;
            } else {
                TRACE(3,"Recv invalid test evt: len=%d data={%08x%08x}",
                    len, data[0]<<24|data[1]<<16|data[2]<<8|data[3], data[4]<<24|data[5]<<16|data[6]<<8|data[7]);
            }
            break;
        case BT_CONN_DATA_ACK:
            ASSERT(len == sizeof(ack_data), "Invalid data ack len: %d", len);
            ret = memcmp(ack_data, data, len);
            ASSERT(ret == 0, "Recv invalid data ack");
            bt_state = BT_CONN_DONE;
            hal_cmu_simu_pass();
            break;
        case BT_CONN_SCO_CONN_ACCEPT_ACK:
            TRACE(4,"SYNC ACK len=%d {%08x%08x%08x}", len,
                data[0]<<24|data[1]<<16|data[2]<<8|data[3],
                data[4]<<24|data[5]<<16|data[6]<<8|data[7],
                data[8]<<24|data[9]<<16|data[10]<<8|data[11]);
            bt_state = BT_CONN_DONE;
#ifdef BTPCM_TEST
            start_btpcm();
#endif
            hal_cmu_simu_pass();
            break;
        case BT_CONN_DONE:
            TRACE(1,"Rx after test done: len=%d", len);
            break;
        default:
            ASSERT(false, "%s: Invalid bt_state=%2d", __FUNCTION__, bt_state);
            break;
    }
}

static void handle_tx_done(void)
{
    TRACE(1,"Tx done: bt_state=%2d", bt_state);

    switch (bt_state) {
        case BT_CONN_SEND_HCI_RESET:
            bt_state = BT_CONN_HCI_RESET_ACK;
            break;
        case BT_CONN_SEND_LINK_POLICY:
            bt_state = BT_CONN_LINK_POLICY_ACK;
            break;
        case BT_CONN_SEND_CHANGE_BDADDR:
            bt_state = BT_CONN_CHANGE_BDADDR_ACK;
            break;
        case BT_CONN_SEND_SET_SCAN_INTERVAL:
            bt_state = BT_CONN_SET_SCAN_INTERVAL_ACK;
            break;
        case BT_CONN_SEND_SCAN:
            bt_state = BT_CONN_SCAN_ACK;
            break;
        case BT_CONN_SEND_ACCEPT:
            bt_state = BT_CONN_ACCEPT_ACK;
            break;
        case BT_CONN_SEND_DATA:
            bt_state = BT_CONN_DATA_ACK;
            break;
        case BT_CONN_SEND_SCO_CONN_ACCEPT:
            bt_state = BT_CONN_SCO_CONN_ACCEPT_ACK;
            break;
        default:
            ASSERT(false, "%s: Invalid bt_state=%2d", __FUNCTION__, bt_state);
            break;
    }
}

static void loop_worker(void)
{
    uint32_t lock;
    bool proc_timer;
    bool proc_rx;
    bool proc_tx;

    proc_timer = false;
    proc_rx = false;
    proc_tx = false;

    lock = int_lock();
    if (timer_evt) {
        timer_evt = false;
        proc_timer = true;
        hal_cpu_wake_unlock(BT_TIMER_WAKE_LOCK);
    }
    if (bt_rx_done) {
        bt_rx_done = false;
        proc_rx = true;
        hal_cpu_wake_unlock(BT_RX_WAKE_LOCK);
    }
    if (bt_tx_done) {
        bt_tx_done = false;
        proc_tx = true;
        hal_cpu_wake_unlock(BT_TX_WAKE_LOCK);
    }
    int_unlock(lock);

    if (proc_tx) {
        handle_tx_done();
    }
    if (proc_rx) {
        if (bt_rx_len > 0) {
            handle_rx_done(bt_rx_data, bt_rx_len);
            bt_rx_len = 0;
        }
        hal_intersys_start_recv(conn_intersys);
    }
    if (proc_timer) {
        handle_timer_evt();
    }
}

void bt_conn_test(void)
{
    int ret;
    int i;

    TRACE(0, "%s", __func__);

#ifdef BT_DRV_TEST
    btdrv_start_bt();
    btdrv_enable_tports();
#ifdef BT_DRV_TEST_BRIDGE_MODE
    btdrv_uart_bridge_loop();
#else
    btdrv_hciopen();
    btdrv_sleep_config(1);
    btdrv_hcioff();
#endif //BT_DRV_TEST_BRIDGE_MODE
#else
#ifdef AON_PSC_BASE
    hal_psc_bt_enable();
#endif
    hal_cmu_bt_clock_enable();
    hal_cmu_bt_reset_clear();
    hal_sys_timer_delay(MS_TO_TICKS(5));
#endif

    sleep_test_register_loop_callback(loop_worker);

    ret = hal_intersys_open(conn_intersys, HAL_INTERSYS_MSG_HCI, bt_rx, bt_tx, false);
    if (ret) {
        ASSERT(false, "Failed to open intersys");
    }
    hal_intersys_start_recv(conn_intersys);

    cmd_data[0] = 0x02;
    cmd_data[1] = 0x80;
    cmd_data[2] = 0x20;
    cmd_data[3] = 0xfd;
    cmd_data[4] = 0x03;
    for (i = 0; i < 0x03fd; i++) {
        cmd_data[5 + i] = i;
    }
    cmd_data_len = 5 + 0x03fd;

    timer_id = hwtimer_alloc(timer_expired, NULL);
    ASSERT(timer_id, "Failed to alloc timer_id");

#ifdef BT_SLEEP_ONLY
    bt_sleep_only = true;
#endif

#if defined(BT_DRV_TEST) && defined(BT_SLEEP_ONLY)
    TRACE(0, "%s: Bt starts to sleep ...", __func__);
#else
    hwtimer_start(timer_id, MS_TO_TICKS(15));

    TRACE(0,"%s: Start to wait timer ...", __func__);
#endif
}

void bt_conn_loop(void)
{
    loop_worker();
}

bool bt_conn_test_done(void)
{
    return (bt_state == BT_CONN_DONE) ? true : false;
}

#endif

