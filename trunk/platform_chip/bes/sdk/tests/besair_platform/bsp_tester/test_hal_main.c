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
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include <stdio.h>
#include <stdarg.h>
#include "stdlib.h"
#include "string.h"
#include "hal_trace.h"
#include "test_hal_api.h"
#include "dhry.h"
#include "pmu.h"

extern int coremark_main(int argc, char *argv[]);
extern void bbk_dhry_test();

void pmu_io_set_high(void)
{
#ifdef CHIP_BEST1600
    pmu_gpio_pin_set_dir(HAL_GPIO_PIN_PMU_P2_1, HAL_GPIO_DIR_OUT, 1);
    pmu_gpio_set_io_pull_select(HAL_GPIO_PIN_PMU_P2_1, HAL_IOMUX_PIN_PULLUP_ENABLE);
    pmu_gpio_pin_set_dir(HAL_GPIO_PIN_PMU_P2_2, HAL_GPIO_DIR_OUT, 1);
    pmu_gpio_set_io_pull_select(HAL_GPIO_PIN_PMU_P2_2, HAL_IOMUX_PIN_PULLUP_ENABLE);
    pmu_gpio_pin_set_dir(HAL_GPIO_PIN_PMU_P2_3, HAL_GPIO_DIR_OUT, 1);
    pmu_gpio_set_io_pull_select(HAL_GPIO_PIN_PMU_P2_3, HAL_IOMUX_PIN_PULLUP_ENABLE);
#endif
}

void test_hal_pattern(void)
{
#if (defined TEST_HAL_PATTERN)

#ifdef TEST_OPENAMP
    test_openamp();
#endif

#ifdef TEST_UART
    test_uart();
#endif

#ifdef TEST_OSTICK
    test_ostick_and_timestamp();
#endif

#ifdef TEST_WDT
    test_wdt();
#endif

#ifdef TEST_TIMER
    test_timer();
#endif

#ifdef TEST_RTC
    test_rtc();
#endif

#ifdef TEST_GPIO
    test_gpio();
#endif

#ifdef TEST_I2C
    test_i2c();
#endif

#ifdef TEST_SPI
    test_spi();
#endif

#ifdef TEST_PWM
    test_pwm();
#endif

#ifdef TEST_DMA
    test_dma();
#endif

#ifdef TEST_TRNG
    test_trng();
#endif

#ifdef TEST_SEC_ENG
    /* fail */
    sec_eng_test();
#endif

#ifdef TEST_GPADC
    test_gpadc();
#endif

#ifdef TEST_NORFLASH
    test_norflash();
#endif

#ifdef TEST_VMIC
    test_vmic();
#endif

#ifdef TEST_I2S
    test_i2s();
#endif

#ifdef TEST_SDIO_HOST_SIGNAL
    test_sdio_host_signal();
#endif

#ifdef TEST_SDIO_HOST
    test_sdio_host();
#endif

#ifdef TEST_PSRAM
    test_psram();
#endif

#ifdef TEST_EMMC
    test_emmc();
#endif

#ifdef TEST_DISPLAY
    pmu_io_set_high();
    test_display();
#endif

#ifdef TEST_LVGL
    pmu_io_set_high();
    extern void lv_main(void);
    lv_main();
#endif

#ifdef TEST_FB
    test_fb();
#endif

#ifdef TEST_LCDC
    test_lcdc();
#endif

#ifdef TEST_BLEND
    test_blend();
#endif

#ifdef TEST_GPU
    test_gpu();
#endif

#ifdef TEST_SPI_SLAVE
    spi_slave_test();
#endif

#ifdef TEST_DHRY
#if 1
    bbk_dhry_test();
#else
    while (1) {
        dhry_main(0, NULL);
    }
#endif
#endif

#ifdef TEST_COREMARK
    coremark_main(0, NULL);
#endif

#ifdef TEST_SENSOR_ENGINE
    test_sensor_engine();
#endif

#ifdef TEST_FREQ_SW_TIME
    test_freq_sw_time();
#endif

#ifdef TEST_MBEDTLS
    test_mbedtls();
#endif

#ifdef TEST_SHANHAI_MBEDTLS
    test_shanhai_mbedtls();
#endif

#ifdef TEST_DMA2D
    test_dma2d();
#endif

#ifdef TEST_ARM2D
    test_arm2d();
#endif

#ifdef TEST_TMA525C
    test_tma525c();
#endif

#ifdef TEST_TIMER_BBK
    extern void cp_timer_test(void);
    cp_timer_test();
#endif

#ifdef TEST_FC
    TRACE(0, "------------------------TEST_FC--------------------");
    run_fc_test();
#endif

#ifdef TEST_USER_GPS_TO_FLASH
    test_user_gps_to_flash();
#endif

#ifdef TEST_DWT
    test_dwt();
#endif

#ifdef TEST_MPU
    test_mpu();
#endif

#ifdef TEST_SYSVIEW
    test_sysview();
#endif

#ifdef TEST_BECO
    beco_test();
#endif

#ifdef CMSIS_TEST
    cmsis_test();
#endif

#endif
}
