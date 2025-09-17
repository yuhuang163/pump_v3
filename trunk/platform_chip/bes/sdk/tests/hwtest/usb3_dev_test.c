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
#include "plat_addr_map.h"
#ifdef USB3_BASE
#include "hal_cmu.h"
#include "hwtest.h"

#ifdef USB3_CDC_TEST
#include "usb3_cdc.h"

static void usb3_serial_break_handler(uint16_t ms);
static void usb3_serial_state_handler(enum USB_SERIAL_STATE_EVENT_T event);

static void usb3_dev_env_init(void)
{
}

static const struct USB_SERIAL_CFG_T cdc_cfg = {
    .break_callback = usb3_serial_break_handler,
    .state_callback = usb3_serial_state_handler,
    .mode = USB_SERIAL_API_BLOCKING,
};

static void usb3_serial_break_handler(uint16_t ms)
{
    TRACE(1,"Detect break: %u ms", ms);
}

static void usb3_serial_state_handler(enum USB_SERIAL_STATE_EVENT_T event)
{
    TRACE(1,"state change : %u ", event);
}

#define ALIGNED4 ALIGNED(4)
static uint8_t ALIGNED4 usb_serial_data[USB_SERIAL_DIRECT_XFER_SIZE];

// USB_SERIAL_DIRECT_XFER_TEST
void usb3_serial_test(void)
{
    usb3_dev_env_init();
    usb3_serial_open(&cdc_cfg);
    TRACE(1,"[0x%X] Open USB serial", hal_sys_timer_get());

    usb3_serial_flush_recv_buffer();
    usb3_serial_init_xfer(USB_SERIAL_XFER_TYPE_ALL);

    usb3_bulk_epn_recv(usb_serial_data, USB_SERIAL_DIRECT_XFER_SIZE);
    while (1) {
        usb3_msleep(10);
    }
}
#endif

#ifdef USB3_SPEED_TEST
#include "usb3_general.h"

/**
 * The larger DWC_REQ_SIZ is, the less software work in thransfer, and transfer
 * speed is much closer to physics speed
 * NOTE: DWC_NUM_REQ * DWC_REQ_SIZ must be <= DWC_BUF_SIZ !
 */
#define DWC_NUM_REQ    1
#define DWC_REQ_SIZ    (32 * 1024)

// NOTE: 2005 RAMSIZE 1 or 2M, can't afford too large buffers
#define DWC_BUF_SIZ    (DWC_REQ_SIZ * DWC_NUM_REQ)

/** Transfer buffers - at least 1K each*/
static uint8_t g_in_buf[DWC_BUF_SIZ] ={0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8 };
static uint8_t g_out_buf[DWC_BUF_SIZ];
static void usb3_speed_test_send_complete(usb_ep_t *ep, usb_request_t *req);
static void usb3_speed_test_recv_complete(usb_ep_t *ep, usb_request_t *req);
static void usb3_lpbk_send_complete(usb_ep_t *ep, usb_request_t *req);
static void usb3_lpbk_recv_complete(usb_ep_t *ep, usb_request_t *req);

void usb3_prepare_request(uint8_t ep_dir, DWC_USB3_CMPL_CALLBACK cb)
{
    uint8_t i = 0;
    if (ep_dir == UE_DIR_IN) {
        for (i = 0; i < DWC_NUM_REQ; i++) {
            usb3_general_send_epn(g_in_buf + DWC_REQ_SIZ * i,
                DWC_REQ_SIZ - 1, cb);
        }
    } else if (ep_dir == UE_DIR_OUT) {
        for (i = 0; i < DWC_NUM_REQ; i++) {
            usb3_general_recv_epn(g_out_buf + DWC_REQ_SIZ * i,
                DWC_REQ_SIZ, cb);
        }
    }

    return;
}

static void usb3_speed_test_send_complete(usb_ep_t *ep, usb_request_t *req)
{
    int ret;
    if (req->status) {
        TRACE(1,"send failed! status[%d]", req->status);
        dwc_usb3_free_request(ep, req);
        return;
    }

    req->length = DWC_REQ_SIZ - 1;
    ret = dwc_usb3_requeue(ep, req);
    if (ret) {
        TRACE(1,"send requeue failed! ret[%d]", ret);
        dwc_usb3_free_request(ep, req);
    }
}

static void usb3_speed_test_recv_complete(usb_ep_t *ep, usb_request_t *req)
{
    int ret;
    if (req->status) {
        TRACE(1,"recv failed! status[%d]", req->status);
        dwc_usb3_free_request(ep, req);
        return;
    }

    req->length = DWC_REQ_SIZ;
    ret = dwc_usb3_requeue(ep, req);
    if (ret) {
        TRACE(1,"recv requeue failed! ret[%d]", ret);
        dwc_usb3_free_request(ep, req);
    }
}

// mcuclk384M, buffer32k, read&write throughput 3Gbps(396MB/s)
void usb3_speed_test(void)
{
    usb3_general_open();
    usb3_prepare_request(UE_DIR_IN, usb3_speed_test_send_complete);
    usb3_prepare_request(UE_DIR_OUT, usb3_speed_test_recv_complete);

    while (1) {
        usb3_msleep(10);
    }
}

static uint8_t next0 = 0;
static void usb3_lpbk_recv_complete(usb_ep_t *ep, usb_request_t *req)
{
    int ret;
    TRACE(1,"OUT req, req->actual:%d, req->length:%d", req->actual, req->length);
    usb_ep_t *in_ep = usb3_general_get_ep(UE_DIR_IN);
    int maxp = usb3_general_get_maxp();
    /* Handle 0-length marker packet*/
    if (req->actual && (req->actual & (maxp - 1)) == 0)
        next0 = 1;

    /* Echo packet back to host on IN EP */
    req->length = req->actual;
    req->complete = usb3_lpbk_send_complete;
    ret = dwc_usb3_requeue(in_ep, req);
    TRACE(1,"echo pkt req add to queue,ret:%d --- IN req ready! --- ", ret);
    if (ret == 0)
        return;

    next0 = 0;
    TRACE(1,"Failed to queue IN req, requeuing on OUT\n");
    /* Handle 0-length marker packet */
    if (next0) {
        next0 = 0;
        req->length = 0;
        req->complete = usb3_lpbk_send_complete;
        ret = dwc_usb3_requeue(in_ep, req);
        TRACE(1,"zero-length pkt send! ret:%d", ret);
        if (ret == 0)
            return;
        TRACE(1,"Failed to queue IN 0-length req, requeuing on OUT\n");
    }

    /* Requeue for a future OUT EP transfer */
    req->length = DWC_REQ_SIZ;
    req->complete = usb3_lpbk_recv_complete;
    ret = dwc_usb3_requeue(ep, req);
    if (ret == 0)
        return;
    TRACE(1,"Failed to queue OUT req\n");
    dwc_usb3_free_request(ep, req);
}

static void usb3_lpbk_send_complete(usb_ep_t *ep, usb_request_t *req)
{
    int ret;
    usb_ep_t *out_ep = usb3_general_get_ep(UE_DIR_OUT);
    /* Handle 0-length marker packet */
    if (next0) {
        next0 = 0;
        req->length = 0;
        ret = dwc_usb3_requeue(ep, req);
        TRACE(1,"zero-length pkt send! ret:%d", ret);
        if (ret == 0)
            return;
        TRACE(1,"Failed to queue IN 0-length req, requeuing on OUT\n");
    }

    /* Requeue for a future OUT EP transfer */
    req->length = DWC_REQ_SIZ;
    req->complete = usb3_lpbk_recv_complete;
    ret = dwc_usb3_requeue(out_ep, req);
    if (ret == 0)
        return;
    TRACE(1,"Failed to queue OUT req\n");
    dwc_usb3_free_request(out_ep, req);
}

void usb3_lpbk_test(void)
{
    usb3_general_open();
    usb3_prepare_request(UE_DIR_OUT, usb3_lpbk_recv_complete);

    while (1) {
        usb3_msleep(10);
    }
}

#endif

#endif

