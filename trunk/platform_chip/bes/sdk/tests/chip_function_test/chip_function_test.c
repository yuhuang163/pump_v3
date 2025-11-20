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
#include "string.h"
#include "cmsis.h"
#include "hal_timer.h"
#include "hal_timer_fast_irq.h"
#include "hal_timer_user_irq.h"
#include "hal_trace.h"
#include "hal_sleep.h"
#include "pmu.h"
#include "hal_gpio.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_sysfreq.h"
#include "chip_function_test.h"
void chip_function_test(void)
{
#ifdef CHIP_FUNCTION_TEST_CURRENT
    function_test_current();
#endif

#ifdef CHIP_FUNCTION_TEST_NORFLASH
    function_test_norflash();
#endif

#ifdef CHIP_FUNCTION_TEST_HWTIMER
    function_test_hwtimer();
#endif

#ifdef CHIP_FUNCTION_TEST_RTC
    function_test_rtc();
#endif

#ifdef CHIP_FUNCTION_TEST_I2S
    function_test_i2s();
#endif

#ifdef CHIP_FUNCTION_TEST_DMA
    function_test_dma();
#endif

#ifdef CHIP_FUNCTION_TEST_POWER_RESET
    function_test_power_reset();
#endif

#ifdef CHIP_FUNCTION_TEST_GPIO
    function_test_gpio();
#endif

#ifdef CHIP_FUNCTION_TEST_SAM
    function_test_sam();
#endif

#ifdef CHIP_FUNCTION_TEST_SDMMC
    function_test_sdmmc();
#endif

#ifdef CHIP_FUNCTION_TEST_I2C
    function_test_i2c();
#endif


}

