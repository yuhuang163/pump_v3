/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __PMU_BEST1306P_H__
#define __PMU_BEST1306P_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_cmu.h"

#define CAP_REG(r)                          (((r) & 0xFFF) | 0x0000)
#define ANA_REG(r)                          (((r) & 0xFFF) | 0x1000)
#define RF_REG(r)                           (((r) & 0xFFF) | 0x2000)
#define PMU_REG(r)                          (((r) & 0xFFF) | 0x3000)

#define ISPI_PMU_REG(reg)                   (((reg) & 0xFFF) | 0x3000)

#define MAX_VMIC_CH_NUM                     2

#define PMU_RF_BBPLL_FREQWORD_SHIFT         (1 << 25)

enum PMU_EFUSE_PAGE_T {
    PMU_EFUSE_PAGE_SECURITY         = 0x0,
    PMU_EFUSE_PAGE_BOOT             = 0x1,
    PMU_EFUSE_PAGE_FEATURE          = 0x2,
    PMU_EFUSE_PAGE_BATTER_LV        = 0x3,
    PMU_EFUSE_PAGE_BATTER_HV        = 0x4,
    PMU_EFUSE_PAGE_SW_CFG           = 0x5,
    PMU_EFUSE_PAGE_PROD_TEST        = 0x6,
    PMU_EFUSE_PAGE_RESERVED_7       = 0x7,
    PMU_EFUSE_PAGE_EXT_GPADC_LV     = 0x8,
    PMU_EFUSE_PAGE_EXT_GPADC_HV     = 0x9,
    PMU_EFUSE_PAGE_TEMPERATURE      = 0xA,
    PMU_EFUSE_PAGE_CP_WAFER_ID      = 0xB,
    PMU_EFUSE_PAGE_CP_WAFER_XY_CTX  = 0xC,
    PMU_EFUSE_PAGE_VBG_VRTC_CALIB   = 0xD,
    PMU_EFUSE_PAGE_DCDC_VOLT_CALIB  = 0xE,
    PMU_EFUSE_PAGE_RF_TX_PWR        = 0xF,

    PMU_EFUSE_PAGE_QTY
};

enum PMU_IRQ_TYPE_T {
    PMU_IRQ_TYPE_GPADC,
    PMU_IRQ_TYPE_RTC,
    PMU_IRQ_TYPE_CHARGER,
    PMU_IRQ_TYPE_GPIO,
    PMU_IRQ_TYPE_WDT,
    PMU_IRQ_TYPE_PWRKEY,

    PMU_IRQ_TYPE_QTY
};

enum PMU_PLL_DIV_TYPE_T {
    PMU_PLL_DIV_DIG,
    PMU_PLL_DIV_CODEC,
};

enum PMU_BIG_BANDGAP_USER_T {
    PMU_BIG_BANDGAP_USER_GPADC              = (1 << 0),
    PMU_BIG_BANDGAP_USER_SAR_VREF_OUTPUT    = (1 << 1),
};

enum PMU_EXT_CRYSTAL_CLKOUT_PIN_T {
    PMU_EXT_CRYSTAL_CLKOUT_PIN_0            = (1 << 0),
};

enum PMU_NTC_USER_T {
    PMU_NTC_USER_BT,
    PMU_NTC_USER_PMU,

    PMU_NTC_USER_QTY,
};

enum PMU_PACKAGE_TYPE_T {
    PMU_PACKAGE_TYPE_IHC_V0 = 0,
    PMU_PACKAGE_TYPE_IA,
    PMU_PACKAGE_TYPE_IHC_V1_I2S,
    PMU_PACKAGE_TYPE_IHC_V1_NO_I2S,
};

union SECURITY_VALUE_T {
    struct {
        unsigned short security_en      :1;
        unsigned short mode             :1;
        unsigned short sig_type         :1;
        unsigned short reserved         :1;
        unsigned short key_id           :2;
        unsigned short vendor_id        :5;
        unsigned short flash_id         :1;
        unsigned short chksum           :4;
    } root;
    struct {
        unsigned short security_en      :1;
        unsigned short mode             :1;
        unsigned short sig_type         :1;
        unsigned short skip_romkey      :1;
        unsigned short key_id           :2;
        unsigned short reg_base         :2;
        unsigned short reg_size         :2;
        unsigned short reg_offset       :1;
        unsigned short flash_id         :1;
        unsigned short chksum           :4;
    } otp;
    unsigned short reg;
};
#define SECURITY_VALUE_T                    SECURITY_VALUE_T

struct RTC_CALENDAR_FORMAT_T {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t week;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

enum PMU_CLK_SEL_RC_USER_T {
    PMU_CLK_SEL_RC_USER_SYS      = (1 << 0),
    PMU_CLK_SEL_RC_USER_CAP      = (1 << 1),
};

struct PMU_NTC_MONITOR_CTX_T {
    uint32_t interval_ms;
    bool enable;
};

struct PMU_NTC_CTX_T {
    int temperature;
    uint32_t temperature_update_ms;
    uint32_t monitor_interval_ms;
};

enum HAL_FLASH_ID_T pmu_get_boot_flash_ctrl_id(void);

uint8_t pmu_gpio_setup_irq(enum HAL_GPIO_PIN_T pin, const struct HAL_GPIO_IRQ_CFG_T *cfg);

void pmu_codec_mic_bias_enable(uint32_t map, int enable);

void pmu_codec_mic_bias_lowpower_mode(uint32_t map, int enable);

int pmu_codec_volt_ramp_up(void);

int pmu_codec_volt_ramp_down(void);

void pmu_pll_div_reset_set(enum HAL_CMU_PLL_T pll);

void pmu_pll_div_reset_clear(enum HAL_CMU_PLL_T pll);

void pmu_pll_div_set(enum HAL_CMU_PLL_T pll, enum PMU_PLL_DIV_TYPE_T type, uint32_t div);

void pmu_pll_freq_reg_set(uint16_t low, uint16_t high, uint16_t high2);

void pmu_led_set_hiz(enum HAL_GPIO_PIN_T pin);

void pmu_led_uart_enable(enum HAL_IOMUX_PIN_T pin);

void pmu_led_uart_disable(enum HAL_IOMUX_PIN_T pin);

void pmu_rf_ana_init(void);

void pmu_big_bandgap_enable(enum PMU_BIG_BANDGAP_USER_T user, int enable);

void pmu_bt_reconn(bool en);

void pmu_bt_2v5pa(bool en);

bool pmu_dig_set_volt_value(uint16_t dcdc_volt_mv, uint16_t logic_volt_mv, uint16_t ldo_volt_mv);

void bbpll_pll_update(uint64_t pll_cfg_val, bool calib);

void bbpll_set_codec_div(uint32_t div);

void bbpll_codec_clock_enable(bool en);

void pmu_vio_3v3(bool en);

void pmu_rc_clk_test(void);

int pmu_rc_clk_calib(void);

void pmu_rc_clk_enable(enum PMU_CLK_SEL_RC_USER_T user);

int pmu_volt2temperature(const uint16_t volt);

int pmu_ntc_monitor_register(enum PMU_NTC_USER_T user, struct PMU_NTC_MONITOR_CTX_T *ctx);

int pmu_ntc_monitor_deregister(enum PMU_NTC_USER_T user);

int pmu_ntc_ctx_get(struct PMU_NTC_CTX_T *ctx);

int pmu_wdt_load_test(void);

void pmu_external_crystal_enable(void);

void pmu_external_crystal_clkout_enable(enum PMU_EXT_CRYSTAL_CLKOUT_PIN_T pin);

void pmu_external_crystal_clkout_disable(enum PMU_EXT_CRYSTAL_CLKOUT_PIN_T pin);

void pmu_external_crystal_capbit_set(uint32_t capbit);

void pmu_external_crystal_capbit_get(uint32_t *p_capbit);

int pmu_external_crystal_calib(uint32_t *p_wait_ms);

void pmu_sar_adc_vref_enable(void);

void pmu_sar_adc_vref_disable(void);

int pmu_wdt_reboot(void);

int pmu_sar_input_buf_enable(int enable);

void pmu_ldo_vsensor_off_pull_down(void);

void pmu_ldo_vsensor_on_lp_dr(bool dr_lp);

void pmu_get_vbat_calib_value(unsigned short *val_lv, unsigned short *val_hv);

void pmu_get_ext_gpadc_calib_value(unsigned short *val_lv, unsigned short *val_hv);

int pmu_get_acin_volt(uint16_t *volt);

enum PMU_PACKAGE_TYPE_T pmu_get_chip_package_type(void);

//=========================================================================
// APIs for pmu rtc
//=========================================================================

typedef void (*PMU_RTC_CALENDAR_IRQ_HANDLER_T)(struct RTC_CALENDAR_FORMAT_T *calendar);

int pmu_rtc_calendar_alarm_irq_handler_set(PMU_RTC_CALENDAR_IRQ_HANDLER_T handler);

int pmu_rtc_calendar_alarm_clear(void);

int pmu_rtc_calendar_alarm_set(struct RTC_CALENDAR_FORMAT_T *alarm_set);

int pmu_rtc_calendar_alarm_get(struct RTC_CALENDAR_FORMAT_T *alarm_get);

int pmu_rtc_calendar_set(struct RTC_CALENDAR_FORMAT_T *calendar_set);

int pmu_rtc_calendar_get(struct RTC_CALENDAR_FORMAT_T *calendar_get);

int pmu_rtc_calendar_pwron_enable(bool en);

#ifdef __cplusplus
}
#endif

#endif

