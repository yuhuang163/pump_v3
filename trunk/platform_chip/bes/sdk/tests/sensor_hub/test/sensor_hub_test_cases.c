/***************************************************************************
 *
 * Copyright 2022-2023 BES.
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
#include "hal_sysfreq.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hwtimer_list.h"
#include "app_sensor_hub.h"
#include "sensor_hub.h"
#include "sensor_hub_core.h"
#include "sens_comp_test.h"
#include "string.h"

#ifdef CHIP_FUNCTION_TEST
#include "chip_function_test.h"
#endif
#ifdef MINIMA_TEST
#include "minima_test.h"
#endif
#ifdef PT_APP
#include "pt_app.h"
#endif
#ifdef MCU2SENS_MSG_TEST
#include "rmt_msg_test.h"
#endif
#ifdef TEST_HAL_PATTERN
#include "test_hal_main.h"
#endif
#ifdef SENSOR_TEST
#include "sensor_test.h"
#endif
#ifdef VD_TEST
#include "voice_detector.h"
#endif

#if defined(BT_CONN_TEST) || defined(I2S_TEST) || defined(SENSOR_ENGINE_TEST) || \
    defined(CPU_MAX_POWER_TEST) || defined(UART_TEST)  || defined(BECO_TEST)  || defined(I2C_TEST) || \
    defined(SBC_TEST) || defined(COREMARK_TEST) || defined(DHRYSTONE_TEST)
#include "hwtest.h"
#endif

#ifdef MCU2SENS_MSG_TEST
static void mcu2sens_msg_test_client(void)
{
    sensor_hub_core_register_rx_irq_handler(CORE_IRQ_HDLR_PRIO_LOW, rmt_msg_test_rx_handler);
    sensor_hub_core_register_tx_done_irq_handler(CORE_IRQ_HDLR_PRIO_LOW, rmt_msg_test_tx_handler);
    rmt_msg_test(true, sensor_hub_send, NULL);
}
#endif

#if defined(SLEEP_TEST) && !defined(NO_TIMER)
static HWTIMER_ID wakeup_timer;
static uint32_t wakeup_cnt;
static const uint32_t timer_ms = 11;

static void timer_handler(void *param)
{
    sensor_hub_send(&wakeup_cnt, sizeof(wakeup_cnt));
    wakeup_cnt++;
    hwtimer_start(wakeup_timer, MS_TO_TICKS(timer_ms));
}
#endif

#if defined(APP_COREMARK_TEST)
typedef struct {
    uint32_t coremark_test_mode; //2: stay at freq and do spi reading 1: coremark test; 0: wfi
    enum HAL_CMU_FREQ_T freq;
} power_test_param;

#include "hal_spi.h"
#include "hal_iomux.h"
#define SPI_OP_TEST_PERIOD_MS   100
#define SPI_OP_TEST_LOG_PRINT_PERIOD_MS 1000

static const osTimerAttr_t TimerAttr_sensorhub_spi_op_test = {
    .name = "SENSORHUB_SPI_OP_TEST_TIMER",
    .attr_bits = 0U,
    .cb_mem = NULL,
    .cb_size = 0U,
};

static osTimerId spi_op_test_timer_id = NULL;

static void spi_op_test_init(void)
{
    struct HAL_SPI_CFG_T spi_cfg;

    POSSIBLY_UNUSED uint32_t val;
    uint32_t error = 0;

    TRACE(0, "------SPI master test started------");

    memset(&spi_cfg, 0, sizeof(spi_cfg));

    spi_cfg.clk_delay_half = true;
    spi_cfg.clk_polarity = false;

    spi_cfg.slave = false;

    spi_cfg.dma_rx = true;
    spi_cfg.dma_tx = true;

    spi_cfg.rx_sep_line = false;
    spi_cfg.cs = 0;

    spi_cfg.rate = 12 * 1000 * 1000;

    spi_cfg.rx_bits = 8;
    spi_cfg.tx_bits = 8;
    spi_cfg.rx_frame_bits = 0;

    TRACE(0, "  ");
    TRACE(1, "spi_cfg.clk_delay_half:%d", spi_cfg.clk_delay_half);
    TRACE(1, "spi_cfg.clk_polarity  :%d", spi_cfg.clk_polarity);
    TRACE(1, "spi_cfg.slave         :%d", spi_cfg.slave);
    TRACE(1, "spi_cfg.dma_rx        :%d", spi_cfg.dma_rx);
    TRACE(1, "spi_cfg.dma_tx        :%d", spi_cfg.dma_tx);
    TRACE(1, "spi_cfg.rx_sep_line   :%d", spi_cfg.rx_sep_line);
    TRACE(1, "spi_cfg.cs            :%d", spi_cfg.cs);
    TRACE(1, "spi_cfg.rate          :%d", spi_cfg.rate);
    TRACE(1, "spi_cfg.rx_bits       :%d", spi_cfg.rx_bits);
    TRACE(1, "spi_cfg.tx_bits       :%d", spi_cfg.tx_bits);
    TRACE(1, "spi_cfg.rx_frame_bits :%d", spi_cfg.rx_frame_bits);
    TRACE(0, "  ");

    hal_iomux_set_sens_spi();

    error = hal_spi_open(&spi_cfg);
    ASSERT(!error, "%s:%d,spi open error:%d", __func__, __LINE__, error);
}

static int32_t spi_op_test_read_master(uint8_t *cmd, uint8_t *pbuf, uint16_t len, uint8_t enable_dma)
{
    int32_t ret = 0;
    uint8_t outBuf[len + 1];

    if (enable_dma) {
        ret = hal_spi_dma_recv(cmd, outBuf, len + 1, NULL);
    } else {
        ret = hal_spi_recv(cmd, outBuf, len + 1);
    }

    memcpy(pbuf, outBuf + 1, len); //remove head
    return ret;
}

static void spi_op_test_timer_handler(void* param)
{
    uint32_t spi_rx_data = 0;
    int32_t ret_rx;
    uint8_t buffer[5] = {0xFF};

    ret_rx = spi_op_test_read_master(&buffer[0], (uint8_t *)&spi_rx_data, sizeof(spi_rx_data), true);
    spi_rx_data = __REV(spi_rx_data);

    static uint32_t test_timer_count = 0;
    if (0 == (test_timer_count++ % (SPI_OP_TEST_LOG_PRINT_PERIOD_MS/SPI_OP_TEST_PERIOD_MS)))
    {
        TRACE(2, "spi master rx data:0x%X, ret:%d", spi_rx_data, ret_rx);
    }
}

static void sensorhub_application_test_handler(void)
{
    // start a periodic timer to do SPI write and read test
    if (!spi_op_test_timer_id)
    {
        spi_op_test_timer_id = osTimerNew(spi_op_test_timer_handler, osTimerPeriodic, NULL, &TimerAttr_sensorhub_spi_op_test);
    }
    spi_op_test_init();
    osTimerStart(spi_op_test_timer_id, SPI_OP_TEST_PERIOD_MS);
}

static void app_sensor_hub_coremark_wfi_power_test_receive_handler(uint8_t *buf, uint16_t len)
{
    power_test_param *test_param = (power_test_param *)buf;
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, test_param->freq);
    TRACE(0, "[Sensor Hub Power Test] Freq %uM", (hal_sys_timer_calc_cpu_freq(5, 0) / 1000000));
    if (2 == test_param->coremark_test_mode) {
        sensorhub_application_test_handler();
        return;
    }
    else if (1 == test_param->coremark_test_mode) {
        coremark_test();
    } else {
        while (1) {
            __WFI();
        }
    }
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_POWER_TEST,
                                "power test cmd received",
                                NULL,
                                app_sensor_hub_coremark_wfi_power_test_receive_handler,
                                0,
                                NULL,
                                NULL,
                                NULL);

#endif /* COREMARK_TEST */

void sensor_hub_test_cases(void)
{
#if defined(SLEEP_TEST) || !defined(NOAPP) || defined(SLEEP_PD_CURRENT_TEST)
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
#else
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M);
#endif
    hal_sysfreq_print_user_freq();

#ifdef BECO_TEST
    beco_test();
#endif

#ifdef BT_CONN_TEST
    bt_conn_test();
#endif

#ifdef CHIP_FUNCTION_TEST
    chip_function_test();
#endif

#if defined(COREMARK_TEST) && !defined(APP_COREMARK_TEST)
    coremark_test();
#endif

#ifdef DHRYSTONE_TEST
    dhrystone_test();
#endif

#ifdef I2C_TEST
    i2c_test();
#endif

#ifdef I2S_TEST
    i2s_test();
#endif

#ifdef CPU_MAX_POWER_TEST
    cpu_max_power_test();
#endif

#ifdef MCU2SENS_MSG_TEST
    mcu2sens_msg_test_client();
#endif

#ifdef MINIMA_TEST
    minima_test();
#endif

#ifdef PT_APP
    pt_app_entry(NULL);
#endif

#ifdef SBC_TEST
    sbc_test();
#endif

#ifdef SENSOR_ENGINE_TEST
    sensor_engine_test();
#endif

#ifdef SENSOR_HUB_MISC_TEST
    sensor_hub_misc_test();
#endif

#ifdef SENSOR_TEST
    sensor_test();
#endif

#ifdef TEST_HAL_PATTERN
    test_hal_pattern();
#endif

#ifdef UART_TEST
    uart_test();
#endif

#ifdef VAD_CODEC_TEST
    vad_codec_test();
#endif

#ifdef VD_TEST
    vad_test_env_init();
    voice_detector_test();
#endif

#if defined(VPU_CFG_TEST)
    extern int32_t vpu_cfg_init(void);
    vpu_cfg_init();
#endif

#if defined(SLEEP_TEST) && !defined(NO_TIMER)
    wakeup_timer = hwtimer_alloc(timer_handler, NULL);
    ASSERT(wakeup_timer, "Failed to alloc wakeup_timer");

    int ret = hwtimer_start(wakeup_timer, MS_TO_TICKS(timer_ms));
    ASSERT(ret == 0, "Failed to start timer: %d", ret);
#endif
}
