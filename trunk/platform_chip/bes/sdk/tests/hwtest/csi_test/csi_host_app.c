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
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "string.h"
#include "hal_location.h"
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_key.h"
#include "hal_i2c.h"
#include "hal_dsi.h"
#include "hal_lcdc.h"
#include "hal_csi.h"
#include "csi_dsi_conf.h"
#include "csi_host.h"
#include "csi_host_app.h"
#include "camera_drv.h"
#include "video_player.h"
#ifdef VIDEO_PLAYER
#include "csi_dsi_loop_app.h"
#endif
#ifdef QRCODE_TEST
#include "ap_qrcode_app.h"
#endif

static int csi_reboot_cnt = 0;

static int g_cam_desc_id = 0;
static enum VP_DEVICE_T g_cam_device;
static struct VP_DEVICE_CONFIG_T g_cam_config;
static struct VP_DEVICE_BUS_T g_cam_i2c_bus;

bool csi_host_app_is_calibed(void)
{
    if (csi_reboot_cnt > 0) {
        return true;
    }
    return false;
}

void csi_host_app_init(void)
{
    bool success;

    camera_device_open();

    success = camera_device_probe(&g_cam_device, &g_cam_i2c_bus);
    if (!success) {
        ASSERT(false, "[%s]: probe device failed", __func__);
    }

    TRACE(0, "%s: device=%d", __func__, g_cam_device);

    camera_device_get_config(g_cam_device, &g_cam_config);

#ifndef __ARM_ARCH_ISA_ARM
#if defined(CHIP_HAS_A7_DSP) && !defined(__A7_DSP_ENABLE__) && defined(CSI_TEST)
    hal_cmu_dsp_clock_enable();
    hal_cmu_dsp_reset_clear();
#endif
#endif

    TRACE(0, "[%s]: CSI HOST APP INIT SUCCESS", __func__);
}

void csi_host_app_on(bool on)
{
    TRACE(0, "[%s]: on=%d", __func__, on);

    if (on == csi_host_status_get()) {
        TRACE(0, "already done");
        return;
    }

    if (on) {
        csi_host_state_notify(0);

        g_cam_desc_id = camera_device_stream_open(g_cam_device, &g_cam_i2c_bus, &g_cam_config);

        csi_host_setup_param(g_cam_desc_id, g_cam_device);
        csi_host_open();

        camera_device_stream_start(g_cam_desc_id);

        csi_host_start();

        if (g_cam_config.op_flag & CAMERA_OP_FLAG_CALIB_PHY) {
            csi_host_calib_start();
        }
        csi_host_state_notify(1);
    } else {
        csi_host_state_notify(2);

        camera_device_stream_stop(g_cam_desc_id);

        csi_host_stop();

        camera_device_stream_close(g_cam_desc_id);

        csi_host_close();

        csi_host_state_notify(3);
    }
    csi_reboot_cnt++;
    csi_host_status_set(on);
    TRACE(0, "DONE");
}

void csi_host_app_restart(void)
{
    POSSIBLY_UNUSED static uint32_t last_time = 0;

    uint32_t cur_time = hal_sys_timer_get();
    bool restart = false;

    if (last_time == 0) {
        restart = true;
    } else if (TICKS_TO_MS(cur_time - last_time) > 1000) {
        restart = true;
    }
    if (restart) {
        csi_host_app_on(false);
        csi_host_app_on(true);
        last_time = cur_time;
    }
}

void csi_host_app_cmd_help(void)
{
    TRACE(1, "CSI OFF      : AT+CSI=%d  "            , CSI_TEST_CMD_OFF      );
    TRACE(1, "CSI ON       : AT+CSI=%d  "            , CSI_TEST_CMD_ON       );
    TRACE(1, "CSI I2C READ : AT+CSI=%d,[reg] "       , CSI_TEST_CMD_I2C_READ );
    TRACE(1, "CSI I2C WRITE: AT+CSI=%d,[reg],[val] " , CSI_TEST_CMD_I2C_WRITE);
    TRACE(1, "CSI REBOOT   : AT+CSI=%d  "            , CSI_TEST_CMD_REBOOT   );
    TRACE(1, "CSI TEST PATT: AT+CSI=%d  "            , CSI_TEST_CMD_TEST_PATT);
    TRACE(1, "CSI DUMP DATA: AT+CSI=%d,[bytes]  "    , CSI_TEST_CMD_DUMP_DATA);
    TRACE(1, "CSI SET PAGE : AT+CSI=%d,[page]  "     , CSI_TEST_CMD_SET_PAGE );
    TRACE(1, "CSI GET CHIP ID : AT+CSI=%d  "         , CSI_TEST_CMD_GET_CHIP_ID);
    TRACE(1, "CSI TRACE ON : AT+CSI=%d  "            , CSI_TEST_CMD_TRACE_ON );
    TRACE(1, "CSI TRACE OFF: AT+CSI=%d  "            , CSI_TEST_CMD_TRACE_OFF);
    TRACE(1, "I2C SLAVE ON : AT+CSI=%d  "            , CSI_TEST_CMD_I2C_SLAVE);
    TRACE(1, "CSI READ MEM : AT+CSI=%d,[addr],[n]  " , CSI_TEST_CMD_RD_MEM);
    TRACE(1, "CSI WRITE MEM: AT+CSI=%d,[addr],[val] ", CSI_TEST_CMD_WR_MEM);
    TRACE(1, "CSI POLL REG : AT+CSI=%d,[addr]"       , CSI_TEST_CMD_POLL_REG_ON);
    TRACE(1, "CSI POLL OFF : AT+CSI=%d"              , CSI_TEST_CMD_POLL_REG_OFF);
    TRACE(1, "CSI DSI LOOP ON  : AT+CSI=%d"          , CSI_TEST_CMD_LOOP_APP_ON);
    TRACE(1, "CSI DSI LOOP OFF : AT+CSI=%d"          , CSI_TEST_CMD_LOOP_APP_OFF);
}

int csi_host_app_cmd_handler(int cmd, int argc, int arg[])
{
    int ret = 0;
    int show_help = 0;
    if ((cmd == CSI_TEST_CMD_I2C_READ)
        || (cmd == CSI_TEST_CMD_I2C_WRITE)
        || (cmd == CSI_TEST_CMD_SET_PAGE)
        || (cmd == CSI_TEST_CMD_GET_CHIP_ID)
        || (cmd == CSI_TEST_CMD_DUMP_DATA)) {
        csi_host_cmd_trace_quiet(true);
    }
    switch (cmd) {
    case CSI_TEST_CMD_OFF:
        csi_host_app_on(false);
        break;
    case CSI_TEST_CMD_ON:
    {
        csi_host_app_init();
        if (csi_reboot_cnt == 0) {
            csi_host_app_on(true);
            osDelay(100);
            csi_host_app_on(false);
            osDelay(200);
        }
        csi_host_app_on(true);
    }
        break;
    case CSI_TEST_CMD_I2C_READ:
    {
        uint32_t addr = 0xf0, val = 0;
        if (argc > 2) {
            addr = arg[0];
            camera_device_bus_read(g_cam_desc_id, addr, &val);
            TRACE(1, "I2C RD: [%02X] = %02X", addr, val);
        } else {
            camera_device_probe(&g_cam_device, &g_cam_i2c_bus);
        }
    }
        break;
    case CSI_TEST_CMD_I2C_WRITE:
    {
        uint32_t addr = arg[0], val = 0;
        if (argc > 3) {
            addr = arg[0];
            val = arg[1];
            camera_device_bus_write(g_cam_desc_id, addr, val);
            TRACE(1, "I2C WR: [%02X] = %02X", addr, val);
        } else {
            TRACE(1, "I2C PARAM ERROR!");
            show_help = 1;
        }
    }
        break;
    case CSI_TEST_CMD_REBOOT:
        csi_host_app_on(false);
        csi_host_app_init();
        csi_host_app_on(true);
        break;
    case CSI_TEST_CMD_TRACE_ON:
        csi_host_cmd_trace_quiet(false);
        break;
    case CSI_TEST_CMD_TRACE_OFF:
        csi_host_cmd_trace_quiet(true);
        break;
    case CSI_TEST_CMD_TEST_PATT:
        camera_device_test_mode(g_cam_desc_id, true, 0, 0, 0);
        break;
    case CSI_TEST_CMD_DUMP_DATA:
        if (argc > 3) {
            csi_host_cmd_dump_data(arg[0], arg[1]);
        } else {
            show_help = 1;
        }
        break;
    case CSI_TEST_CMD_SET_PAGE:
        if (argc > 2) {
            csi_host_cmd_set_page(arg[0]);
        }
        break;
    case CSI_TEST_CMD_GET_CHIP_ID:
        camera_device_probe(&g_cam_device, &g_cam_i2c_bus);
        break;
    case CSI_TEST_CMD_I2C_SLAVE:
        TRACE(1, "SWITCH UART0 TO I2C ...");
        hal_trace_flush_output();
        hal_iomux_set_analog_i2c();
        break;
    case CSI_TEST_CMD_RD_MEM:
        if (argc > 2) {
            csi_host_cmd_access_mem(0, arg[0], arg[1]);
        } else {
            show_help = 1;
        }
        break;
    case CSI_TEST_CMD_WR_MEM:
        if (argc > 3) {
            csi_host_cmd_access_mem(1, arg[0], arg[1]);
        } else {
            show_help = 1;
        }
        break;
    case CSI_TEST_CMD_POLL_REG_ON:
        if (argc > 2) {
            csi_host_cmd_poll_reg(1, arg[0]);
        } else {
            show_help = 1;
        }
        break;
    case CSI_TEST_CMD_POLL_REG_OFF:
        csi_host_cmd_poll_reg(0, 0);
        break;
#ifdef VIDEO_PLAYER
    case CSI_TEST_CMD_LOOP_APP_ON: //enable csi_dsi_loop_app
#ifndef M33_H264_TEST
    csi_dsi_loop_app_init();
#endif
        //csi_dsi_loop_app_init();
        csi_dsi_loop_app_on(true);
#ifdef QRCODE_TEST
        ap_qrcode_task_init();
#endif
        break;
    case CSI_TEST_CMD_LOOP_APP_OFF: //disable csi_dsdi_loop_app
        csi_dsi_loop_app_on(false);
#ifdef QRCODE_TEST
        ap_qrcode_task_deinit();
#endif
        break;
#endif
    default:
        TRACE(1, "unknown cmd:%d",cmd);
        show_help = 1;
        break;
    }
    if (show_help) {
        csi_host_app_cmd_help();
    }
    return ret;
}
