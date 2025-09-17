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
#if defined(BT_SERVICE_ON) && defined(BT_SERVICE_TEST) && defined(UTILS_ESHELL_EN)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "plat_types.h"

#include "bt_service.h"
#include "eshell.h"

static bts_status_t bts_hfp_hf_callback(bts_hfp_hf_event_t event, bts_hfp_hf_event_param_t *param)
{
    bts_dev_addr_t *addr  = NULL;
    bts_hfp_hf_t *hfp_hf = NULL;

    hfp_hf = param->hfp_hf;

    switch (event) {
        case BTS_HFP_HF_EVENT_SERVICE_CONNECTED:
        {
            addr = param->p.service_connected.addr;
            eshell_putstring_nl("hfp hf : service connected: addr=0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], addr->addr[4], addr->addr[5]);
            eshell_putstring_nl("hfp hf : service_connected: hfp_hf=0x%p", hfp_hf);
        }
        break;
        case BTS_HFP_HF_EVENT_SERVICE_DISCONNECTED:
        {
            eshell_putstring_nl("hfp hf : sevice disconnected: hfp_hf=0x%p", hfp_hf);
        }
        break;
        case BTS_HFP_HF_EVENT_RING_IND:
        {
            eshell_putstring_nl("hfp hf : ring ind: hfp_hf=0x%p", hfp_hf);
        }
        break;
        case BTS_HFP_HF_EVENT_CALLER_ID_IND:
        {
            eshell_putstring_nl("hfp hf : caller id ind: hfp_hf=0x%p,number=%s", hfp_hf, param->p.caller_id_ind.number);
        }
        break;
        case BTS_HFP_HF_EVENT_AUDIO_CONNECTED:
        {
            eshell_putstring_nl("hfp hf : audio connected : hfp_hf=0x%p", hfp_hf);
        }
        break;
        case BTS_HFP_HF_EVENT_AUDIO_DISCONNECTED:
        {
            eshell_putstring_nl("hfp hf : audio disconnected : hfp_hf=0x%p", hfp_hf);
        }
        break;
        default:
        break;
    }
    return BTS_ST_SUCCESS;
}

// bts_hfp_hf_test_start
static void bts_hfp_hf_test_start_cmd(int argc, char *argv[])
{
    eshell_putstring_nl("bts_hfp_hf_test_start...");
    bts_hfp_hf_init();
    bts_hfp_hf_register_callback(bts_hfp_hf_callback);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_HFP_CLT, "bts_hfp_hf_test_start", "start bts hfp_hf test", bts_hfp_hf_test_start_cmd);

#endif