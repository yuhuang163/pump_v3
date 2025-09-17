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
#ifndef NOAPP
#include "string.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hwtimer_list.h"
#include "mcu_dsp_m55_app.h"
#include "app_thread.h"
#include "dsp_m55_msg.h"
#include "dsp_m55_core.h"
#include "dsp_m55_core_app.h"
#include "app_dsp_m55.h"

/*
 * SENSOR HUB CORE PING MSG
 ********************************************************
 */

#ifndef CORE_BRIDGE_NO_PING_MSG
static void app_dsp_m55_core_transmit_ping_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(MCU_DSP_M55_TASK_CMD_PING, ptr, len);
}

static void app_dsp_m55_ping_bth(void)
{
    app_dsp_m55_bridge_send_cmd(MCU_DSP_M55_TASK_CMD_PING, NULL, 0);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_DSP_M55_TASK_CMD_PING,
                                "ping mcu",
                                app_dsp_m55_core_transmit_ping_handler,
                                NULL,
                                0,
                                NULL,
                                NULL,
                                NULL);
#endif

/*
 * CORE BRIDGE DEMO MSG
 ********************************************************
 */

#ifdef CORE_BRIDGE_DEMO_MSG
#define CORE_BRIDGE_DEMO_REQ_DATA       0x55
#define CORE_BRIDGE_DEMO_REQ_RSP_DATA   0xAA

typedef struct
{
    uint8_t reqData;
} APP_DSP_M55_CORE_DEMO_REQ_T;

typedef struct
{
    uint8_t rspData;
} APP_DSP_M55_CORE_DEMO_RSP_T;

static void app_dsp_m55_core_transmit_demo_no_rsp_cmd_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(MCU_DSP_M55_TASK_CMD_DEMO_REQ_NO_RSP, ptr, len);
}

static void app_dsp_m55_core_demo_no_rsp_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "Get demo no rsp command from mcu:");
    DUMP8("%02x ", ptr, len);
}

static void app_dsp_m55_core_demo_no_rsp_cmd_tx_done_handler(uint16_t cmdCode,
    uint8_t* ptr, uint16_t len)
{
    TRACE(0, "cmdCode 0x%x tx done", cmdCode);
    DUMP8("%02x ", ptr, len);
}

void app_dsp_m55_core_send_demo_req_no_rsp(void)
{
    APP_DSP_M55_CORE_DEMO_REQ_T req;
    req.reqData = CORE_BRIDGE_DEMO_REQ_DATA;
    app_dsp_m55_bridge_send_cmd(MCU_DSP_M55_TASK_CMD_DEMO_REQ_NO_RSP,
        (uint8_t *)&req, sizeof(req));
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_DSP_M55_TASK_CMD_DEMO_REQ_NO_RSP,
                                "demo no rsp req to mcu",
                                app_dsp_m55_core_transmit_demo_no_rsp_cmd_handler,
                                app_dsp_m55_core_demo_no_rsp_cmd_received_handler,
                                0,
                                NULL,
                                NULL,
                                app_dsp_m55_core_demo_no_rsp_cmd_tx_done_handler);

static void app_dsp_m55_core_transmit_demo_wait_rsp_cmd_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(MCU_DSP_M55_TASK_CMD_DEMO_REQ_WITH_RSP, ptr, len);
}

static void app_dsp_m55_core_demo_wait_rsp_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "Get demo with rsp command from mcu:");
    DUMP8("%02x ", ptr, len);
    APP_DSP_M55_CORE_DEMO_RSP_T rsp;
    rsp.rspData = CORE_BRIDGE_DEMO_REQ_RSP_DATA;
    app_dsp_m55_bridge_send_rsp(MCU_DSP_M55_TASK_CMD_DEMO_REQ_WITH_RSP, (uint8_t *)&rsp, sizeof(rsp));
}

static void app_dsp_m55_core_demo_wait_rsp_cmd_wait_rsp_timeout(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "sensor core waiting for rsp to demo cmd timeout.");
}

static void app_dsp_m55_core_demo_wait_rsp_cmd_rsp_received_handler(uint8_t* ptr, uint16_t len)
{
    APP_DSP_M55_CORE_DEMO_RSP_T* ptrRsp = (APP_DSP_M55_CORE_DEMO_RSP_T *)ptr;
    TRACE(0, "Sensor hub gets rsp 0x%x from mcu:", ptrRsp->rspData);
}

static void app_dsp_m55_core_demo_with_rsp_cmd_tx_done_handler(uint16_t cmdCode,
    uint8_t* ptr, uint16_t len)
{
    TRACE(0, "cmdCode 0x%x tx done", cmdCode);
}

void app_dsp_m55_core_send_demo_req_with_rsp(void)
{
    APP_DSP_M55_CORE_DEMO_REQ_T req;
    req.reqData = CORE_BRIDGE_DEMO_REQ_DATA;
    app_dsp_m55_bridge_send_cmd(MCU_DSP_M55_TASK_CMD_DEMO_REQ_WITH_RSP,
        (uint8_t *)&req, sizeof(req));
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_DSP_M55_TASK_CMD_DEMO_REQ_WITH_RSP,
                                "demo with rsp req to mcu",
                                app_dsp_m55_core_transmit_demo_wait_rsp_cmd_handler,
                                app_dsp_m55_core_demo_wait_rsp_cmd_received_handler,
                                APP_DSP_M55_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS,
                                app_dsp_m55_core_demo_wait_rsp_cmd_wait_rsp_timeout,
                                app_dsp_m55_core_demo_wait_rsp_cmd_rsp_received_handler,
                                app_dsp_m55_core_demo_with_rsp_cmd_tx_done_handler);

static void app_dsp_m55_core_transmit_demo_instant_req_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_instant_cmd_data(MCU_DSP_M55_INSTANT_CMD_DEMO_REQ,
        ptr, len);
}

static void app_dsp_m55_core_demo_instant_req_handler(uint8_t* ptr, uint16_t len)
{
    // for test purpose, we add log print here.
    // but as instant cmd handler will be directly called in intersys irq context,
    // for realistic use, should never do log print
    TRACE(0, "Get demo instant req command from mcu:");
    DUMP8("%02x ", ptr, len);
}

CORE_BRIDGE_INSTANT_COMMAND_TO_ADD(MCU_DSP_M55_INSTANT_CMD_DEMO_REQ,
                                app_dsp_m55_core_transmit_demo_instant_req_handler,
                                app_dsp_m55_core_demo_instant_req_handler);

#endif /* CORE_BRIDGE_DEMO_MSG */

static void app_dsp_m55_core_transmit_set_freq_req_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_instant_cmd_data(MCU_DSP_M55_INSTANT_CMD_AXI_SYSFREQ_REQ, ptr, len);
}

CORE_BRIDGE_INSTANT_COMMAND_TO_ADD(MCU_DSP_M55_INSTANT_CMD_AXI_SYSFREQ_REQ,
                                app_dsp_m55_core_transmit_set_freq_req_handler,
                                NULL);

static void app_dsp_m55_core_receive_disable_m55_handler(uint8_t* ptr, uint16_t len)
{
    // Avoid other task to run when the BTH need to disable M55
    POSSIBLY_UNUSED uint32_t lock = int_lock();
    while(1) {
        __WFI();
    }
}

CORE_BRIDGE_INSTANT_COMMAND_TO_ADD(MCU_DSP_M55_INSTANT_CMD_DISABLE_M55,
                                NULL,
                                app_dsp_m55_core_receive_disable_m55_handler);

#ifndef CORE_BRIDGE_NO_PING_MSG
#define APP_DSP_M55_PING_MCU_TIMER_PERIOD_MS    (10000)

static void dsp_m55_ping_mcu_timer_handler(void const *param);
osTimerDef(dsp_m55_ping_mcu_timer, dsp_m55_ping_mcu_timer_handler);
static osTimerId dsp_m55_ping_mcu_timer_id = NULL;

static void dsp_m55_ping_mcu_timer_handler(void const *param)
{
    app_dsp_m55_ping_bth();
}

static void app_dsp_m55_timer_init(void)
{
    TRACE(0, "%s:", __func__);

    if (dsp_m55_ping_mcu_timer_id == NULL) {
        dsp_m55_ping_mcu_timer_id = osTimerCreate(osTimer(dsp_m55_ping_mcu_timer), osTimerPeriodic, NULL);
    }
    if (dsp_m55_ping_mcu_timer_id != NULL) {
        osTimerStart(dsp_m55_ping_mcu_timer_id, APP_DSP_M55_PING_MCU_TIMER_PERIOD_MS);
    }
}
#endif

void dsp_m55_core_deinit(void)
{
    // stub, do nothing
}

void dsp_m55_core_app_init(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_52M);

    app_dsp_m55_bridge_init();

    dsp_m55_core_register_rx_irq_handler(CORE_IRQ_HDLR_PRIO_LOW, app_dsp_m55_bridge_data_received);

    dsp_m55_core_register_tx_done_irq_handler(CORE_IRQ_HDLR_PRIO_LOW, app_dsp_m55_bridge_data_tx_done);

    app_dsp_m55_notify_tx_thread_status(true);

#ifndef CORE_BRIDGE_NO_PING_MSG
    app_dsp_m55_ping_bth();

    app_dsp_m55_timer_init();
#endif
}
#endif
