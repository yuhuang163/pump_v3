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
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_iomux.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hwtimer_list.h"
#ifdef __ARMCC_VERSION
#include "link_sym_armclang.h"
#endif
#include "main_entry.h"
#include "mpu_cfg.h"

#ifdef SENSOR_TEST
#include "sensor_test.h"
#endif

#ifdef SPI_ROM_ONLY
#include "pmu_rom.h"
#else
#include "hal_analogif.h"
#include "hal_chipid.h"
#include "pmu.h"
#endif

// The following macros can be defined in command line by TC=...
// E.g., TC=I2S_TEST,SPI_TEST

extern uint32_t __rom_text0_end[];
extern uint32_t __rom_ramx_start[];
extern uint32_t __rom_ramx_end[];

#ifdef ROM_CACHE_MEM
#include "hal_cache.h"

extern uint32_t __ram_extra_text_data_load_start[];
extern uint32_t __ram_extra_text_data_start[];
extern uint32_t __ram_extra_text_data_end[];
extern uint32_t __ram_extra_bss_start[];
extern uint32_t __ram_extra_bss_end[];
#endif

void hwtest_register_key_callback(int (*cb)(uint32_t, uint8_t))
{
}

void clock_init(void)
{
#ifdef NO_CLOCK_INIT
    hal_sys_timer_open();
#else
    hal_cmu_rom_setup();
#endif

    // Enable OSC X2/X4
#ifdef SPI_ROM_ONLY
#ifndef NO_PMU
    pmu_rom_open();
    pmu_rom_update_pll_config();
    pmu_rom_enable_osc_x2();
#endif
#else
    hal_analogif_open();
#ifndef NO_PMU
    hal_chipid_init();
    pmu_boot_init();
#endif
#endif

#ifndef NO_CLOCK_INIT
    hal_cmu_module_init_state();

    hal_cmu_osc_x2_enable();
    hal_cmu_osc_x4_enable();

#ifdef NO_LPU_26M
    while (hal_cmu_lpu_init(HAL_CMU_LPU_CLK_NONE) == -1);
#else
    while (hal_cmu_lpu_init(HAL_CMU_LPU_CLK_26M) == -1);
#endif
    hal_cmu_rom_enable_pll();
#endif
}

int MAIN_ENTRY(void)
{
#ifndef __ARM_ARCH_ISA_ARM
    uint32_t *dst, *src;

    for (dst = __rom_ramx_start, src = __rom_text0_end;
            dst < __rom_ramx_end;
            dst++, src++) {
        *dst = *src;
    }

#ifdef ROM_CACHE_MEM
#ifndef NO_LOAD_EXTRA_RAM
    for (dst = __ram_extra_text_data_start, src = __ram_extra_text_data_load_start;
            dst < __ram_extra_text_data_end;
            dst++, src++) {
        *dst = *src;
    }
#endif

    for (dst = __ram_extra_bss_start; dst < __ram_extra_bss_end; dst++) {
        *dst = 0;
    }
#endif
#endif

    clock_init();

#ifdef MPU_INIT_TABLE
    // Init memory map
    mpu_boot_cfg();
#endif

#ifdef ROM_CACHE_MEM
    // Enable cache after hal_cmu_rom_setup() and mpu_boot_cfg(), for
    // 1) hal_cmu_rom_setup() will disable caches
    // 2) mpu_boot_cfg() will identify the address range to cache

    // Enable icache
    hal_cache_enable(HAL_CACHE_ID_I_CACHE);
    // Enable write buffer
    hal_cache_writebuffer_enable(HAL_CACHE_ID_D_CACHE);
    // Enable write back
    hal_cache_writeback_enable(HAL_CACHE_ID_D_CACHE);
    // Enable dcache
    hal_cache_enable(HAL_CACHE_ID_D_CACHE);
#endif

    hal_dma_open();

#ifdef DEBUG
#if (DEBUG_PORT == 1)
    hal_iomux_set_uart0();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART0);
#endif
#if (DEBUG_PORT == 2)
    hal_iomux_set_uart1();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART1);
#endif
#if (DEBUG_PORT == 3)
    hal_iomux_set_uart2();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART2);
#endif
#endif

    hwtimer_init();

    hal_sleep_start_stats(10000, 10000);

#ifndef NO_CLOCK_INIT
#ifdef NO_PLL
    // CAUTION: x2/x4 might not be ready
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M);
#else
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_78M);
#endif
#endif

    TRACE(0, "hwtest starts");

#ifdef BT_HOST_TEST
    bt_host_test();
#endif

#ifdef DSP_HIFI4_TEST
    dsp_hifi4_test();
#endif

#ifdef CRC_TEST
    crc_test();
#endif

#ifdef CHECKSUM_TEST
    checksum_test();
#endif

#ifdef SBC_TEST
    sbc_test();
#endif

#ifdef FM_SIMU
    fm_test();
#endif

#ifdef DPD_SIMU
    dpd_test();
#endif

#ifdef CODEC_SIMU
    codec_test();
#endif

#ifdef SPDIF_TEST
    spdif_test();
#endif

#ifdef I2C_TEST
    i2c_test();
#endif

#ifdef I2C_DEBUG_SLAVE_TEST
    i2c_debug_slave_test();
#endif

#ifdef SENSOR_TEST
    sensor_test();
#endif

#ifdef I2C_SLAVE_TEST
    i2c_slave_test();
#endif

#ifdef I2S_TEST
    i2s_test();
#endif

#ifdef I3C_TEST
    i3c_test();
#endif

#ifdef AF_TEST
    af_test();
#endif

#ifdef SPI_TEST
    spi_test();
#endif

#ifdef SPI2SDIO_TEST
    spi2sdio_test();
#endif

#ifdef PCIE_TEST
    pcie_test();
#endif

#ifdef SPI_DEBUG_SLAVE_TEST
    spi_debug_slave_test();
#endif

#ifdef EFUSE_TEST
    efuse_test();
#endif

#ifdef ISPI_DMA_TEST
    ispi_dma_test();
#endif

#ifdef TIMER_TEST
    timer_test();
#endif

#ifdef USB_SERIAL_SPEED_TEST
    usb_serial_speed_test();
#endif

#ifdef USB_SERIAL_TEST
    usb_serial_test();
#endif

#ifdef USB_SERIAL_DIRECT_XFER_TEST
    usb_serial_direct_xfer_test();
#endif

#ifdef USB_AUDIO_TEST
    usb_audio_test();
#endif

#ifdef RSA_TEST
    rsa_test();
#endif

#ifdef UART_BRIDGE_TEST
    uart_bridge();
#endif

#ifdef INTERSYS_COMM_TEST
    intersys_comm_test();
#endif

#ifdef FLASH_TEST
    flash_test();
#endif

#ifdef NANDFLASH_TEST
    nandflash_test();
#endif

#ifdef FULLMAP_MMU_TEST
    fullmap_mmu_test();
#endif

#ifdef PSRAM_TEST
    psram_test();
#endif

#ifdef PSRAM_WINDOW_TEST
    psram_window_test();
#endif

#ifdef PSRAM_DUAL_PORT_TEST
    psram_dual_port_test();
#endif

#ifdef MBW_TEST
    mbw_test();
#endif

#ifdef PSRAMUHS_TEST
    psramuhs_test();
#endif

#ifdef PSRAMUHS_WINDOW_TEST
    psramuhs_window_test();
#endif

#ifdef USBHOST_TEST
    usbhost_test();
#endif

#ifdef BT_CONN_TEST
    bt_conn_test();
#endif

#ifdef BT_CONN_SIMU
    bt_conn_simu();
#endif

#ifdef PWM_TEST
    pwm_test();
#endif

#ifdef BREATHING_LED_TEST
    breathing_led_test();
#endif

#ifdef RF_CALIB_TEST
    rf_calib_test();
#endif

#ifdef A7_DSP_TEST
    a7_dsp_test();
#endif

#ifdef TRANSQ_TEST
    transq_test();
#endif

#ifdef PATCH_TEST
    patch_test();
#endif

#ifdef HWFFT_TEST
    hwfft_test();
#endif

#ifdef PMU_WDT_TEST
    pmu_wdt_test();
#endif

#ifdef CP_TEST
    cp_test();
#endif

#ifdef CP_SUBSYS_TEST
    cp_subsys_test();
#endif

#ifdef CSP_SUBSYS_TEST
    csp_subsys_test();
#endif

#ifdef WDT_TEST
    wdt_test();
#endif

#ifdef SEC_ENG_TEST
    sec_eng_test();
#endif

#ifdef SENSOR_HUB_TEST
    sensor_hub_test();
#endif

#ifdef DSI_TEST
    dsi_test();
#endif

#ifdef QSPI_LCDC_TEST
    qspi_lcdc_test();
#endif

#ifdef QSPI_TEST
    qspi_test();
#endif

#ifdef GPU_TEST
    gpu_test();
#endif

#ifdef PNG_TEST
    png_test();
#endif

#ifdef JPEG_TEST
    jpeg_test();
#endif

#ifdef DMA2D_TEST
    dma2d_test();
#endif

#ifdef GAMU_TEST
    gamu_test();
#endif

#ifdef DMA2D_TEST
    run_graphic_test_cases();
#endif
#ifdef TRNG_TEST
    trng_test();
#endif

#ifdef SDIO_DEVICE_TEST
    sdio_device_test();
#endif

#ifdef SDIO_HOST_TEST
    sdio_host_test();
#endif

#ifdef SDIO_HOST_SIGNAL_TEST
    sdio_host_signal_test();
#endif

#ifdef SDIO_HOST_FIRMWARE_TEST
    sdio_host_firmware_test();
#endif

#ifdef SDIO_HOST_PROGRAMMER_TEST
    sdio_host_programmer_test();
#endif

#ifdef SPI_NORFLASH_TEST
    spi_norflash_test();
#endif

#ifdef SDMMC_TEST
    sdmmc_test();
#endif

#ifdef SDEMMC_TEST
    sdemmc_test();
#endif

#ifdef EMMC_TEST
    emmc_test();
#endif

#ifdef UART_TEST
    uart_test();
#endif

#ifdef SCI_TEST
    sci_test();
#endif

#ifdef SENSOR_ENGINE_TEST
    sensor_engine_test();
#endif

#ifdef GPIO_TEST
    gpio_test();
#endif

#ifdef GPADC_TEST
    gpadc_test();
#endif

#ifdef HEAP_TEST
    heap_test();
#endif

#ifdef CHARGER_TEST
    charger_test();
#endif

#ifdef COREMARK_TEST
    coremark_test();
#endif

#ifdef DHRYSTONE_TEST
    dhrystone_test();
#endif

#ifdef MBEDTLS_TEST
    mbedtls_test();
#endif

#ifdef SHANHAI_TEST
    shanhai_test();
#endif

#ifdef JIAYU_TEST
    jiayu_test();
#endif

#ifdef WIFI_TEST
    wifi_test();
#endif

#ifdef CPU_MAX_POWER_TEST
    cpu_max_power_test();
#endif

    // SLEEP_TEST is shared by other cases so it must be the last one
#ifdef SLEEP_TEST
    sleep_test();
#endif

    SAFE_PROGRAM_STOP();
    return 0;
}

