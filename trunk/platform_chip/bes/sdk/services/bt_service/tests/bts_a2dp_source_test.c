/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifdef BT_SOURCE
#if defined(BT_SERVICE_ON) && defined(BT_SERVICE_TEST) && defined(UTILS_ESHELL_EN)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "plat_types.h"
#include "bt_service.h"
#include "eshell.h"

static bts_status_t bts_a2dp_source_callback(bts_a2dp_source_event_t event, bts_a2dp_source_event_param_t *param)
{
    bts_dev_addr_t *addr = NULL;
    switch (event) {
        case BTS_A2DP_SOURCE_EVENT_SEARCH_RESULT:
            addr = param->p.search_result.addr;
            eshell_putstring_nl("found device, addr=0x%x 0x%x 0x%x 0x%x 0x%x 0x%x,name=%s",
                addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], addr->addr[4], addr->addr[5],
                param->p.search_result.name);
        break;
        case BTS_A2DP_SOURCE_EVENT_SEARCH_COMPLETE:
            eshell_putstring_nl("search complete");
        break;
        case BTS_A2DP_SOURCE_EVENT_CONNECT_FAIL:
            addr = param->p.connect_fail.addr;
            eshell_putstring_nl("connect fail: addr=0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], addr->addr[4], addr->addr[5]);
        break;
        case BTS_A2DP_SOURCE_EVENT_STREAM_OPEN:
            addr = param->p.stream_open.addr;
            eshell_putstring_nl("stream open: addr=0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], addr->addr[4], addr->addr[5]);
        break;
        case BTS_A2DP_SOURCE_EVENT_STREAM_CLOSE:
            addr = param->p.stream_close.addr;
            eshell_putstring_nl("stream close: addr=0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], addr->addr[4], addr->addr[5]);
        break;
        default:
        break;
    }
    return BTS_ST_SUCCESS;
}

// bts_a2dp_source_test_start
static void bts_a2dp_source_test_start_cmd(int argc, char *argv[])
{
    eshell_putstring_nl("bts_a2dp_source_test_start...");
    bts_a2dp_source_init();
    bts_a2dp_source_register_callback(bts_a2dp_source_callback);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_A2DP_SRC, "bts_a2dp_source_test_start", "start bts a2dp_source test", bts_a2dp_source_test_start_cmd);

// search
static void bts_a2dp_source_search_cmd(int argc, char *argv[])
{
    bts_status_t ret = 0;
    ret = bts_a2dp_source_search();
    eshell_putstring_nl("bts_a2dp_source_search,ret=%d", ret);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_A2DP_SRC, "bts_a2dp_source_search", "bts a2dp source search", bts_a2dp_source_search_cmd);

// connect 11:22:33:aa:bb:cc
static void bts_a2dp_source_connect_cmd(int argc, char *argv[])
{
    bts_status_t ret = 0;
    bts_dev_addr_t addr;
    char *param = argv[1];
    ESHELL_PARAM_CHECK_COUNT(param,1,"bts_a2dp_source_connect");

    eshell_param_macstr_get_array(param, 0, (unsigned char *)&addr);
    ret = bts_a2dp_source_connect(&addr);
    eshell_putstring_nl("bts_a2dp_source_connect 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x, ret=%d",
        addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3], addr.addr[4], addr.addr[5], ret);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_A2DP_SRC, "bts_a2dp_source_connect", "bts a2dp source connect", bts_a2dp_source_connect_cmd);

#endif
#endif