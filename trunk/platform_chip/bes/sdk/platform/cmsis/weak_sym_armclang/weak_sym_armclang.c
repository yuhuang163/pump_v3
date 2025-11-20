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
#ifdef __ARMCC_VERSION

#include "analog.h"
#include "hal_cmd.h"
#include "hal_codec.h"
#include "hal_spi.h"
#include "hal_trace.h"
#include "codec_int.h"
#include "norflash_drv.h"
#include "pmu.h"

// Stupid armlink
WEAK uint32_t __cp_text_sram_start_flash__[1];
WEAK uint32_t __cp_text_sram_start[1];
WEAK uint32_t __cp_text_sram_end[1];
WEAK uint32_t __cp_data_sram_start_flash__[1];
WEAK uint32_t __cp_data_sram_start[1];
WEAK uint32_t __cp_data_sram_end[1];
WEAK uint32_t __cp_bss_sram_start[1];
WEAK uint32_t __cp_bss_sram_end[1];

WEAK void analog_aud_xtal_tune(float ratio) {}
WEAK void analog_aud_restore_anc_dyn_adc_gain(enum ANC_TYPE_T type) {}
WEAK void analog_aud_apply_anc_adc_gain_offset(enum ANC_TYPE_T type, int16_t offset_l, int16_t offset_r) {}

WEAK void hal_codec_dac_gain_m60db_check(enum HAL_CODEC_PERF_TEST_POWER_T type) {}
WEAK void hal_codec_set_noise_reduction(bool enable) {}
WEAK void hal_codec_set_anc_boost_gain_attn(float attn) {}
WEAK void hal_codec_tune_both_resample_rate(float ratio) {}
WEAK void hal_codec_set_bt_trigger_callback(HAL_CODEC_BT_TRIGGER_CALLBACK callback) {}
WEAK int hal_codec_bt_trigger_start(void) {return 0;}
WEAK int hal_codec_bt_trigger_stop(void) {return 0;}
WEAK void hal_codec_sync_dac_enable(enum HAL_CODEC_SYNC_TYPE_T type) {}
WEAK void hal_codec_sync_dac_disable(void) {}
WEAK void hal_codec_sync_adc_enable(enum HAL_CODEC_SYNC_TYPE_T type) {}
WEAK void hal_codec_sync_adc_disable(void) {}
WEAK void hal_codec_sync_dac_resample_rate_enable(enum HAL_CODEC_SYNC_TYPE_T type) {}
WEAK void hal_codec_sync_dac_resample_rate_disable(void) {}
WEAK void hal_codec_sync_adc_resample_rate_enable(enum HAL_CODEC_SYNC_TYPE_T type) {}
WEAK void hal_codec_sync_adc_resample_rate_disable(void) {}
WEAK void hal_codec_sync_dac_gain_enable(enum HAL_CODEC_SYNC_TYPE_T type) {}
WEAK void hal_codec_sync_dac_gain_disable(void) {}
WEAK void hal_codec_sync_adc_gain_enable(enum HAL_CODEC_SYNC_TYPE_T type) {}
WEAK void hal_codec_sync_adc_gain_disable(void) {}
WEAK void hal_codec_dac_dre_init_calib_cfg(void) {}
WEAK struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *hal_codec_dac_dre_get_calib_cfg(uint32_t *nr) {return NULL;}
WEAK int codec_anc_open(enum ANC_TYPE_T type, enum AUD_SAMPRATE_T dac_rate, enum AUD_SAMPRATE_T adc_rate, CODEC_ANC_HANDLER hdlr) {return 0;}
WEAK int codec_anc_close(enum ANC_TYPE_T type) {return 0;}

WEAK void hal_sys_timer_calib_start(void) {}
WEAK int hal_sys_timer_calib_end(void) {return 0;}
WEAK float hal_sys_timer_systick_hz_float(void) {return 0.0;}
WEAK void hal_cmu_programmer_enable_pll(void) {}
WEAK void hal_cmu_rom_clock_init(void) {}
WEAK uint32_t hal_cmu_cp_get_entry_addr(void) {return 0;}
WEAK int hal_cmu_emmc_set_freq(enum HAL_CMU_PERIPH_FREQ_T freq) {return 0;}
WEAK void hal_cmu_usb_rom_set_clock_source(enum HAL_CMU_USB_CLOCK_SEL_T sel) {}

WEAK int hal_cmd_run (hal_cmd_rx_status_t status) {return 0;}

WEAK int pmu_codec_volt_ramp_up(void) {return 0;}
WEAK int pmu_codec_volt_ramp_down(void) {return 0;}

WEAK void system_cp_reset_handler(void) {}

#ifdef ROM_BUILD
#if 1 //ndef ROM_IN_FLASH
WEAK void norflash_reset(enum HAL_FLASH_ID_T id) {}
WEAK int norflash_get_total_size(enum HAL_FLASH_ID_T id, uint32_t *total_size) {return HAL_NORFLASH_OK;}
WEAK int norflash_set_mode(enum HAL_FLASH_ID_T id, uint32_t op) {return HAL_NORFLASH_OK;}
WEAK int norflash_pre_operation(enum HAL_FLASH_ID_T id) {return HAL_NORFLASH_OK;}
WEAK int norflash_post_operation(enum HAL_FLASH_ID_T id) {return HAL_NORFLASH_OK;}
WEAK int norflash_init_sample_delay_by_div(enum HAL_FLASH_ID_T id, uint32_t div) {return HAL_NORFLASH_OK;}
WEAK void norflash_set_sample_delay_index(enum HAL_FLASH_ID_T id, uint32_t index) {}
WEAK int norflash_sample_delay_calib(enum HAL_FLASH_ID_T id, enum DRV_NORFLASH_CALIB_T type) {return HAL_NORFLASH_OK;}
WEAK int norflash_init_div(enum HAL_FLASH_ID_T id, const struct HAL_NORFLASH_CONFIG_T *cfg) {return HAL_NORFLASH_OK;}
WEAK int norflash_match_chip(enum HAL_FLASH_ID_T id, const uint8_t *dev_id, uint32_t len) {return HAL_NORFLASH_OK;}
WEAK int norflash_get_id(enum HAL_FLASH_ID_T id, uint8_t *value, uint32_t len) {return HAL_NORFLASH_OK;}
WEAK int norflash_get_unique_id(enum HAL_FLASH_ID_T id, uint8_t *value, uint32_t len) {return HAL_NORFLASH_OK;}
WEAK enum HAL_NORFLASH_RET_T norflash_erase(enum HAL_FLASH_ID_T id, uint32_t start_address, enum DRV_NORFLASH_ERASE_T type, int suspend) {return HAL_NORFLASH_OK;}
WEAK enum HAL_NORFLASH_RET_T norflash_write(enum HAL_FLASH_ID_T id, uint32_t start_address, const uint8_t *buffer, uint32_t len, int suspend) {return HAL_NORFLASH_OK;}
WEAK int norflash_read(enum HAL_FLASH_ID_T id, uint32_t start_address, uint8_t *buffer, uint32_t len) {return HAL_NORFLASH_OK;}
WEAK uint32_t norflash_get_sample_delay_index(enum HAL_FLASH_ID_T id) {return 0;}
WEAK void norflash_show_calib_result(enum HAL_FLASH_ID_T id) {}
WEAK int norflash_read_sfdp_reg(enum HAL_FLASH_ID_T id, uint8_t addr, uint8_t *value, uint32_t len) {return 0;}
WEAK int norflash_dual_chip_read_sfdp_reg(enum HAL_FLASH_ID_T id, uint8_t addr, uint8_t *value0, uint8_t *value1, uint32_t len) {return 0;}
WEAK void norflash_set_dual_chip_mode(enum HAL_FLASH_ID_T id, int flash, int sec_reg, int tx_copy) {}
WEAK void norflash_enter_4byteaddr_mode(enum HAL_FLASH_ID_T id) {}
WEAK void norflash_exit_4byteaddr_mode(enum HAL_FLASH_ID_T id) {}
WEAK int norflash_set_block_protection(enum HAL_FLASH_ID_T id, uint32_t bp) {return 0;}
WEAK int norflash_get_page_size(enum HAL_FLASH_ID_T id, uint32_t *page_size) {return 0;}
WEAK int norflash_pre_operation_suspend(enum HAL_FLASH_ID_T id, int suspend) {return 0;}
WEAK int norflash_post_operation_suspend(enum HAL_FLASH_ID_T id, int suspend, int skip_mode_change) {return 0;}
WEAK void norflash_force_sleep(enum HAL_FLASH_ID_T id) {}
WEAK void norflash_force_wakeup(enum HAL_FLASH_ID_T id) {}
WEAK void norflash_match_sfdp(enum HAL_FLASH_ID_T id, const uint8_t *dev_id, uint8_t len) {}
WEAK void norflash_show_sfdp_check_param(enum HAL_FLASH_ID_T id) {}
#endif

WEAK void pmu_boot_init(void) {}
WEAK void pmu_sleep(void) {}
WEAK void pmu_wakeup(void) {}
WEAK void pmu_flash_freq_config(uint32_t freq) {}
WEAK void pmu_fir_freq_config(uint32_t freq) {}
WEAK void pmu_iir_freq_config(uint32_t freq) {}
WEAK void pmu_iir_eq_freq_config(uint32_t freq) {}
WEAK void pmu_rs_freq_config(uint32_t freq) {}
WEAK void pmu_rs_adc_freq_config(uint32_t freq) {}
WEAK void pmu_led_set_voltage_domains(enum HAL_IOMUX_PIN_T pin, enum HAL_IOMUX_PIN_VOLTAGE_DOMAINS_T volt) {}
WEAK void pmu_led_set_pull_select(enum HAL_IOMUX_PIN_T pin, enum HAL_IOMUX_PIN_PULL_SELECT_T pull_sel) {}
WEAK void pmu_big_bandgap_enable(enum PMU_BIG_BANDGAP_USER_T user, int enable) {}
WEAK uint32_t pmu_gpio_set_io_pull_select(enum HAL_IOMUX_PIN_T pin, enum HAL_IOMUX_PIN_PULL_SELECT_T pull_sel) {return 0;}
WEAK uint32_t pmu_gpio_set_io_drv(enum HAL_IOMUX_PIN_T pin, uint32_t val) {return 0;}
WEAK int pmu_get_gpadc_data_bits(void) {return 0;}
WEAK int pmu_gpadc_clear_chan_irq(unsigned short irq) {return 0;}
WEAK int pmu_gpadc_enable_chan(int ch, int enable) {return 0;}
WEAK int pmu_gpadc_div_ctrl(enum HAL_GPADC_CHAN_T channel, int enable) {return 0;}
WEAK int pmu_set_irq_unified_handler(enum PMU_IRQ_TYPE_T type, PMU_IRQ_UNIFIED_HANDLER_T hdlr) {return 0;}
WEAK void pmu_itn_psram_pu_ce(int dual) {}

#ifdef SPI_ROM_ONLY
WEAK int hal_ispi_open(const struct HAL_SPI_CFG_T *cfg) {return 0;}
WEAK int hal_ispi_send(const void *data, uint32_t len) {return 0;}
WEAK int hal_ispi_recv(const void *cmd, void *data, uint32_t len) {return 0;}
WEAK void hal_ispi_activate_cs(uint32_t cs) {}
WEAK int hal_ispi_enable_ctrl(const struct HAL_SPI_CTRL_T *ctrl) {return 0;}
WEAK int hal_ispi_restore_ctrl(void) {return 0;}
WEAK int hal_ispi_send_ex(const struct HAL_SPI_CTRL_T *ctrl, const void *data, uint32_t len) {return 0;}
WEAK int hal_ispi_recv_ex(const struct HAL_SPI_CTRL_T *ctrl, const void *cmd, void *data, uint32_t len) {return 0;}
#endif

WEAK void analog_sleep(void) {}
WEAK void analog_wakeup(void) {}
WEAK void analog_aud_freq_pll_config(uint32_t freq, uint32_t div) {}
WEAK void analog_aud_pll_open(enum ANA_AUD_PLL_USER_T user) {}
WEAK void analog_aud_pll_close(enum ANA_AUD_PLL_USER_T user) {}
WEAK void analog_aud_codec_speaker_enable(bool en) {}
WEAK void analog_aud_set_dac_gain(int32_t v) {}
WEAK uint32_t analog_aud_get_max_dre_gain(void) {return 0;}
WEAK void analog_aud_vad_adc_enable(bool en) {}
WEAK void analog_aud_vad_enable(enum AUD_VAD_TYPE_T type, bool en) {}
#endif // ROM_BUILD

#endif // __ARMCC_VERSION

