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
#ifndef __HWTEST_H__
#define __HWTEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_addr_map.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hwtimer_list.h"

#define REG(a)                          *(volatile uint32_t *)(a)

void sleep_test_register_timer_callback(HWTIMER_CALLBACK_T cb);

void sleep_test_register_loop_callback(void (*cb)(void));

void hwtest_register_key_callback(int (*cb)(uint32_t, uint8_t));

void fm_test(void);

void dpd_test(void);

void codec_test(void);

void spdif_test(void);

void i2c_test(void);

void i2c_debug_slave_test(void);

void i2c_slave_test(void);

void i3c_test(void);

void i2s_test(void);

void af_test(void);

void pcie_test(void);

void spi_test(void);

void ispi_dma_test(void);

void spi2sdio_test(void);

void spi_modem_test(void);

void qspi_1501_test(void);

void qspi_lcdc_test(void);

void qspi_test(void);

void spi_norflash_test(void);

void spi_debug_slave_test(void);

void efuse_test(void);

void timer_test(void);

void usb_serial_speed_test(void);

void usb_serial_test(void);

void usb_serial_direct_xfer_test(void);

void usb_audio_test(void);

void usb_mtp_test(void);

void usb_general_speed_test(void);

void usb3_serial_test(void);

void usb3_speed_test(void);

void usb3_lpbk_test(void);

void uart_bridge(void);

void intersys_comm_test(void);

void sleep_test(void);

void flash_test(void);

void nandflash_test(void);

void usbhost_test(void);

void bt_conn_test(void);

void bt_conn_simu(void);

void pwm_test(void);

void breathing_led_test(void);

void psram_test(void);

void psram_window_test(void);

uint32_t psram_window_scan_test(void);

void psram_dual_port_test();

void mbw_test();

void psramuhs_test(void);

void psramuhs_window_test(void);

void ddr_test(void);

void rf_calib_test(void);

void transq_test(void);

void patch_test(void);

void hwfft_test(void);

void wdt_test(void);

void pmu_wdt_test(void);

void cp_test(void);

void cp_subsys_test(void);

void csp_subsys_test(void);

void tdm_test(void);

void sec_eng_test(void);

void a7_dsp_test(void);

void a55_test(void);

void sensor_hub_test(void);

void dsp_m55_test(void);

void dsp_hifi4_test(void);

void bt_host_test(void);

void dsi_test(void);

void csi_test(void);

void beco_test(void);

void rsa_test(void);

void trng_test(void);

void tz_test(void);

void ir_test(void);

void ft_test_main(void);

void dac_test(void);

void adc2_test(void);

void sdio_device_test(void);

void sdio_host_test(void);

void sdio_host_signal_test(void);

void sdio_host_firmware_test(void);

void sdio_host_programmer_test(void);

void sdmmc_test(void);

void sdemmc_test(void);

void uart_test(void);

void sci_test(void);

void gpu_test(void);

void gamu_test();

int png_test(void);

int run_graphic_test_cases(void);

void emmc_test(void);

void sensor_engine_test();

void cpu_max_power_test();

void gpio_test();

void gpadc_test();

void heap_test();

void cmsis_test(void);

void lc3_test(void);

void sbc_test(void);

void sbc_rom_test(void);

void aac_dec_test(void);

void charger_test(void);

void coremark_test(void);

void dhrystone_test(void);

void mbedtls_test(void);

void shanhai_test(void);

void jiayu_test(void);

void fullmap_mmu_test(void);

void wifi_test(void);

void pvt_test(void);

void single_wire_uart_test(void);

void ext_crystal_test(void);

void rtc_test(void);

void boot_reason_test(void);

void power_protect_test(void);

void cpudump_test(void);

void crc_test(void);

void checksum_test(void);

void jpeg_test(void);

void dma2d_test(void);

void osc_wakeup_test(void);

#ifdef __cplusplus
}
#endif

#endif

