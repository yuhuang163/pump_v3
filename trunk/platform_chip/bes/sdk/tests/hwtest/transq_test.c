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
#include "hwtest.h"
#include "hal_transq.h"
#include "cmsis.h"
#include "string.h"

#ifdef TRANSQ0_BASE

#ifdef RX_ROLE
static volatile int rx_cnt;
#else
static const uint8_t *normal_data = (const uint8_t *)0x20010001;
static uint32_t normal_len = 0x1;

static const uint8_t *high_data = (const uint8_t *)0x20020001;
static uint32_t high_len = 0x1;
#endif

static enum HAL_TRANSQ_ID_T transq_id =
#ifdef TRANSQ_ID
TRANSQ_ID
#else
HAL_TRANSQ_ID_0
#endif
;

static void get_rx_data(enum HAL_TRANSQ_PRI_T pri)
{
    enum HAL_TRANSQ_RET_T ret;
    const uint8_t *data;
    uint32_t len;
    uint32_t cnt = 0;

    ret = hal_transq_rx_first(transq_id, pri, &data, &len);
    ASSERT(ret == HAL_TRANSQ_RET_OK, "hal_transq_rx_first failed: pri=%d ret=%d", pri, ret);

    while (ret == HAL_TRANSQ_RET_OK && data) {
        cnt++;
        TRACE(4,"    rx-%u: pri=%d data=%p len=%u", cnt, pri, data, len);

#ifdef RX_ROLE
        ret = hal_transq_tx(transq_id, pri, data, len);
        TRACE(3,"    reply-%u: pri=%d ret=%d", cnt, pri, ret);
#endif

        ret = hal_transq_rx_next(transq_id, pri, &data, &len);
    }

    if (ret == HAL_TRANSQ_RET_RX_EMPTY) {
        TRACE(2,"    rx-%u empty: pri=%d", cnt, pri);
    } else {
        ASSERT(false, "    rx-%u failed: pri=%d ret=%d", cnt, pri, ret);
    }
}

static void rx_handler(enum HAL_TRANSQ_PRI_T pri)
{
    TRACE(2,"%s: pri=%d", __func__, pri);

#ifdef RX_ROLE
    if (pri == HAL_TRANSQ_PRI_NORMAL) {
        rx_cnt++;
    } else
#endif
    {
        get_rx_data(pri);
    }
}

static void tx_handler(enum HAL_TRANSQ_PRI_T pri, const uint8_t *data, uint32_t len)
{
    TRACE(4,"%s: pri=%d data=%p len=%u", __func__, pri, data, len);
}

#endif // TRANSQ0_BASE

void transq_test(void)
{

    TRACE(1,"%s", __FUNCTION__);

#ifdef TRANSQ0_BASE

    enum HAL_TRANSQ_RET_T ret;
    struct HAL_TRANSQ_CFG_T cfg;

    memset(&cfg, 0, sizeof(cfg));

#ifdef RX_ROLE
    cfg.slot.rx_num[HAL_TRANSQ_PRI_HIGH] = 2;
    cfg.slot.rx_num[HAL_TRANSQ_PRI_NORMAL] = 4;

    cfg.slot.tx_num[HAL_TRANSQ_PRI_HIGH] = 1;
    cfg.slot.tx_num[HAL_TRANSQ_PRI_NORMAL] = 3;
#else
    cfg.slot.rx_num[HAL_TRANSQ_PRI_HIGH] = 1;
    cfg.slot.rx_num[HAL_TRANSQ_PRI_NORMAL] = 3;

    cfg.slot.tx_num[HAL_TRANSQ_PRI_HIGH] = 2;
    cfg.slot.tx_num[HAL_TRANSQ_PRI_NORMAL] = 4;
#endif

    cfg.rx_handler = rx_handler;
    cfg.tx_handler = tx_handler;

    ret = hal_transq_open(transq_id, &cfg);
    ASSERT(ret == HAL_TRANSQ_RET_OK, "hal_transq_open failed: %d", ret);

#ifdef RX_ROLE

    for (volatile int delay = 0; delay < 7000; delay++);

    uint32_t lock;

    while (1) {
        if (rx_cnt > 0) {
            lock = int_lock();
            rx_cnt--;
            int_unlock(lock);

            get_rx_data(HAL_TRANSQ_PRI_NORMAL);
        }
    }

#else // !RX_ROLE

    int i;

    for (i = 0; i < 6; i++) {
        ret = hal_transq_tx(transq_id, HAL_TRANSQ_PRI_NORMAL, normal_data, normal_len);
        TRACE(3,"tx normal: %p - %u : ret=%d", normal_data, normal_len, ret);
        if (ret == HAL_TRANSQ_RET_OK) {
            normal_data++;
            normal_len++;
        }
    }

    for (i = 0; i < 4; i++) {
        ret = hal_transq_tx(transq_id, HAL_TRANSQ_PRI_HIGH, high_data, high_len);
        TRACE(3,"tx high: %p - %u : ret=%d", high_data, high_len, ret);
        if (ret == HAL_TRANSQ_RET_OK) {
            high_data++;
            high_len++;
        }
    }

    for (volatile int delay = 0; delay < 7000 * 3; delay++);

    for (i = 0; i < 6; i++) {
        ret = hal_transq_tx(transq_id, HAL_TRANSQ_PRI_NORMAL, normal_data, normal_len);
        TRACE(3,"tx normal: %p - %u : ret=%d", normal_data, normal_len, ret);
        if (ret == HAL_TRANSQ_RET_OK) {
            normal_data++;
            normal_len++;
        }
    }

    for (i = 0; i < 4; i++) {
        ret = hal_transq_tx(transq_id, HAL_TRANSQ_PRI_HIGH, high_data, high_len);
        TRACE(3,"tx high: %p - %u : ret=%d", high_data, high_len, ret);
        if (ret == HAL_TRANSQ_RET_OK) {
            high_data++;
            high_len++;
        }
    }

#endif // !RX_ROLE

#else // !TRANSQ0_BASE

    TRACE(1,"%s: Chip has no transq!", __func__);

#endif // !TRANSQ0_BASE
}


