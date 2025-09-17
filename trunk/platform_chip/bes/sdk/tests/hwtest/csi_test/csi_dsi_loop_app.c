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
#ifdef VIDEO_PLAYER
#include "hal_trace.h"
#include "ap_vplayer_app.h"
#include "dsp_vplayer_app.h"
#include "video_stream_rpc.h"
#include "csi_dsi_loop_app.h"

#if defined(CHIP_BEST2003_DSP)
#define CSI_DSI_DUMMY_APP
#endif

static enum VPLAYER_APP_MODE_T g_app_mode = VPLAYER_APP_MODE_NONE;

#ifdef CSI_DSI_DUMMY_APP
void dummy_vplayer_app_open(void)
{
    TRACE(1, "[%s]", __func__);
}

void dummy_vplayer_app_on(bool on)
{
    TRACE(1, "[%s]", __func__);
}
#endif

void csi_dsi_loop_app_init(void)
{
#ifdef CAMERA_TEST_ONLY
    g_app_mode = VPLAYER_APP_MODE_CSI;
#elif defined(DISPLAY_TEST_ONLY)
    g_app_mode = VPLAYER_APP_MODE_DSI;
#elif defined(BOOT_IMAGE_ONLY)
    g_app_mode = VPLAYER_APP_MODE_DSI;
#else
    g_app_mode = VPLAYER_APP_MODE_CSI_DSI_LOOP;
#endif

#if defined(CHIP_ROLE_AP)
    ap_vplayer_app_open(g_app_mode);
#elif defined(CHIP_ROLE_DSP)
    dsp_vplayer_app_open(g_app_mode);
#elif defined(CSI_DSI_DUMMY_APP)
    dummy_vplayer_app_open();
#else
    ap_vplayer_app_init(g_app_mode);
#endif
}

void csi_dsi_loop_app_on(bool on)
{
#if defined(CHIP_ROLE_AP)
    ap_vplayer_app_trigger(on);
#elif defined(CHIP_ROLE_DSP)
    dsp_vplayer_app_trigger(on);
#elif defined(CSI_DSI_DUMMY_APP)
    dummy_vplayer_app_on(on);
#else
    ap_vplayer_app_on(on);
#endif
}

void csi_dsi_loop_app_run(void)
{
#if !defined(RTOS)
#if !defined(CHIP_ROLE_AP) && !defined(CHIP_ROLE_DSP) && !defined(CSI_DSI_DUMMY_APP)
extern void vplayer_thread(void const *arguments);
    vplayer_thread(NULL);
#endif
#endif
}

void csi_dsi_loop_app_rx_cmd_handler(char *data, uint32_t len)
{
#if defined(CHIP_ROLE_AP) || defined(CHIP_ROLE_DSP)
    vplayer_rpc_msg_rx_handler(data, len);
#endif
}

void csi_dsi_loop_app_tx_cmd_handler(char *data, uint32_t len)
{
#if defined(CHIP_ROLE_AP) || defined(CHIP_ROLE_DSP)
    vplayer_rpc_msg_tx_handler(data, len);
#endif
}
#endif /* VIDEO_PLAYER */
