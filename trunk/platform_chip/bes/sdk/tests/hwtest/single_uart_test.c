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
#ifdef SINGLE_WIRE_UART_TEST
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_gpio.h"
#include "hal_key.h"
#include "hwtest.h"
#include "communication_svr.h"
#include "cmsis_os.h"

/* make
* 1700
* make T=noapp_test -j BTH_AS_MAIN_MCU=1 CHIP_SUBSYS=bth CHIP=best1700 SINGLE_WIRE_UART_TEST=1 SINGLE_WIRE_UART_PMU=1 SINGLE_WIRE_UART_PORT=2 SLEEP_TEST=1
* make T=noapp_test -j BTH_AS_MAIN_MCU=1 CHIP_SUBSYS=bth CHIP=best1700 SINGLE_WIRE_UART_TEST=1 SINGLE_WIRE_UART_PMU=0 SINGLE_WIRE_UART_PORT=1 SLEEP_TEST=1 UART1_IOMUX_INDEX=30
*
* 1603
* make T=noapp_test -j BTH_AS_MAIN_MCU=1 CHIP_SUBSYS=bth CHIP=best1603 SINGLE_WIRE_UART_TEST=1 SINGLE_WIRE_UART_PMU=1 SINGLE_WIRE_UART_PORT=1 SLEEP_TEST=1
*
* 1600C TWS chips
* make T=noapp_test -j BTH_AS_MAIN_MCU=1 CHIP_SUBSYS=bth CHIP=best1600 SINGLE_WIRE_UART_TEST=1 SINGLE_WIRE_UART_PMU=1 SINGLE_WIRE_UART_PORT=1 SLEEP_TEST=1
*
* 1600B TWS chips, single wire uart needs work on M55. Call pmu_uart_enable() in BTH.
* make T=dsp_m55 -j DSP_M55_TRC_TO_MCU=0 DEBUG_PORT=2 SINGLE_WIRE_UART_PMU=1 SINGLE_WIRE_UART_PORT=0 TEST_SINGLE_WIRE_COM=1
* make T=noapp_test -j BTH_AS_MAIN_MCU=1 CHIP_SUBSYS=bth CHIP=best1600 DEBUG_PORT=2 DSP_M55_TEST=1 DSP_M55_TRC_TO_MCU=0 RMT_TRC_IN_MSG_CHAN=0 SLEEP_TEST=1
*
* 1501 + 1803/1805
* make T=noapp_test CHIP=best1501 -j SINGLE_WIRE_UART_TEST=1 SINGLE_WIRE_UART_PMU=1 SINGLE_WIRE_UART_PORT=2 SLEEP_TEST=1
*
* 1501 + 1802/1807
* make T=noapp_test CHIP=best1501 -j SINGLE_WIRE_UART_TEST=1 SINGLE_WIRE_UART_PMU=1 SINGLE_WIRE_UART_PORT=1 SLEEP_TEST=1
*
* 1501p/1502x/1305/1306/1503
* make T=noapp_test -j SINGLE_WIRE_UART_TEST=1 SLEEP_TEST=1 CHIP=bestx
*
*/

#define SINGLE_UART_TX_PWRKEY

static uint32_t tx_cnt;

static uint8_t test_buf[2] = {0x5a, 0xa5};

#ifdef SINGLE_UART_TX_PWRKEY
static int single_uart_test_key_event_process(uint32_t key_code, uint8_t key_event)
{
    TRACE(0, "%s: code=0x%X event=%u", __FUNCTION__, key_code, key_event);

    if (key_code == HAL_KEY_CODE_PWR && key_event == HAL_KEY_EVENT_CLICK) {
        communication_send_buf(test_buf, ARRAY_SIZE(test_buf));
        tx_cnt++;
    }

    return 0;
}
#else
static void single_uart_test_tx(void)
{
    communication_send_buf(test_buf, ARRAY_SIZE(test_buf));
    tx_cnt++;
}
#endif

void single_wire_uart_test(void)
{
    TRACE(0, "%s enter, if no data is received within 15 seconds, the single-wire port will be configured as GPIO with irq enable", __func__);

    communication_init();

#ifdef SINGLE_UART_TX_PWRKEY
    hwtest_register_key_callback(single_uart_test_key_event_process);
#endif

    while(1) {
#ifdef SINGLE_UART_TX_PWRKEY
        TRACE(0, "%s Click power key TX",  __func__);
#else
        single_uart_test_tx();
#endif
        TRACE(0, "%s tx_cnt=%d", __func__, tx_cnt);
        osSignalWait(0x0, 1000 * 10);
    }
}
#endif
