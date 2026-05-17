#ifndef __BSP_CHARGE_H__
#define __BSP_CHARGE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_internal.h" // IWYU pragma: keep

/**@brief Firmware information define*/

/**@brief
 * 充电芯片充电电压设置寄存器
 */
#define CHARGE_VBAT_SET_REG                  0x00

/// 内阻补偿使能控制
#define IRCOMP_POS                           6
typedef enum {
    IRCOMP_0MOHM   = 0x00 << IRCOMP_POS,   /**< 0mΩ */
    IRCOMP_20MOHM  = 0x01 << IRCOMP_POS, /**< 20mΩ */
    IRCOMP_40MOHM  = 0x02 << IRCOMP_POS, /**< 40mΩ */
    IRCOMP_80MOHM  = 0x03 << IRCOMP_POS, /**< 80mΩ */
    IRCOMP_DEFAULT = IRCOMP_0MOHM, /**< 默认 */
} ircomp_e;

/// 充电芯片选择寄存器
#define CSEL_POS                           3
typedef enum {
    CSEL_1_CELL  = 0x00 << CSEL_POS, /**< 1节 */
    CSEL_2_CELL  = 0x01 << CSEL_POS, /**< 2节 */
    CSEL_3_CELL  = 0x02 << CSEL_POS, /**< 3节 */
    CSEL_4_CELL  = 0x03 << CSEL_POS, /**< 4节 */
    CSEL_DEFAULT = CSEL_1_CELL, /**< 默认 */
} csel_e;

/// 充电芯片电池电压寄存器
#define VCELL_POS                           0
typedef enum {
    VCELL_4_1V   = 0x00 << VCELL_POS, /**< 4.1V */
    VCELL_4_2V   = 0x01 << VCELL_POS, /**< 4.2V */
    VCELL_4_25V  = 0x02 << VCELL_POS, /**< 4.25V */
    VCELL_4_3V   = 0x03 << VCELL_POS, /**< 4.3V */
    VCELL_4_35V  = 0x04 << VCELL_POS, /**< 4.35V */
    VCELL_4_4V   = 0x05 << VCELL_POS, /**< 4.4V */
    VCELL_4_45V  = 0x06 << VCELL_POS, /**< 4.45V */
    VCELL_4_5V   = 0x07 << VCELL_POS, /**< 4.5V */
    VCELL_DEFAULT = VCELL_4_2V, /**< 默认 */
} vcell_e;

/**@brief
 * 充电芯片输入电压限制寄存器
 * @note 该寄存器用于设置输入电压限制, 10BIT
 * VBUS_SCALE=0, LSB = 20mV
 * VBUS_SCALE=1, LSB = 40mV
 */
#define CHARGE_VINDPM_H_REG                  0x01
#define CHARGE_VINDPM_L_REG                  0x02
/**@brief
 * 充电芯片输入电流限制寄存器
 * @note 该寄存器用于设置输入电流限制, 8BIT, 0x00 ~ 0xFF
 * IBUS_SCALE=0b, LSB = 25mA
 * IBUS_SCALE=1b, LSB = 50mA
 */
#define CHARGE_IIN_LIMIT_REG                 0x03
/**@brief
 * 充电芯片充电电流限制寄存器
 * @note 该寄存器用于设置充电电流限制, 8BIT, 0x00 ~ 0xFF
 * IBAT_SCALE=0b, LSB = 25mA
 * IBAT_SCALE=1b, LSB = 50mA
 */
#define CHARGE_IBAT_CHG_REG                  0x04

/**@brief
 * 充电芯片OTG电压寄存器
 * @note 该寄存器用于设置OTG电压, 10BIT
 * VBUS_SCALE=0, VOTG = (VOTG_L + VOTG_H × 256) ×20mV
 * VBUS_SCALE=1, VOTG = (VOTG_L + VOTG_H × 256) ×40mV
 */
#define CHARGE_VOTG_H_REG                    0x05
#define CHARGE_VOTG_L_REG                    0x06

/**@brief
 * 充电芯片OTG电压反馈寄存器
 * @note 该寄存器用于设置OTG电压反馈, 10BIT
 * VOTG_FB_SEL=1b, VOTG_FB = (VOTG_FB_L + VOTG_FB_H ×256) × 2mV
 */
#define CHARGE_VOTG_FB_H_REG                 0x07
#define CHARGE_VOTG_FB_L_REG                 0x08

/**@brief
 * 充电芯片OTG电流寄存器
 * @note 该寄存器用于设置OTG电流, 8BIT, 0x00 ~ 0xFF
 * IBUS_SCALE = 0, LSB = 25mA,
 * IBUS_SCALE = 1, LSB = 50mA,
 */
#define CHARGE_IOTG_REG                      0x09

/**@brief
 * 充电芯片电池放电电流寄存器
 * @note 该寄存器用于设置电池放电电流, 8BIT, 0x00 ~ 0xFF
 * IBAT_SCALE = 0, LSB = 25mA,
 * IBAT_SCALE = 1, LSB = 50mA,
 */
#define CHARGE_IBAT_DCHG_REG                 0x0A

/**@brief
 * 充电芯片控制寄存器0
 * @note 该寄存器用于配置充电芯片的控制状态
 */
#define CHARGE_CTRL0_REG                     0x0B

/// OTG使能控制
#define EN_OTG_POS                           7
typedef enum {
    OTG_DISABLE = 0x00 << EN_OTG_POS, /**< 关闭 */
    OTG_ENABLE  = 0x01 << EN_OTG_POS, /**< 开启 */
    OTG_DEFAULT = OTG_DISABLE, /**< 默认 */
} otg_en_e;

/// PWM频率选择寄存器
#define PWM_FREQ_POS                         4
typedef enum {
    PWM_FREQ_300KHZ  = 0x00 << PWM_FREQ_POS, /**< 300kHz */
    PWM_FREQ_350KHZ  = 0x01 << PWM_FREQ_POS, /**< 360kHz */
    PWM_FREQ_450KHZ  = 0x02 << PWM_FREQ_POS, /**< 450kHz */
    PWM_FREQ_500KHZ  = 0x03 << PWM_FREQ_POS, /**< 500kHz */
    PWM_FREQ_DEFAULT = PWM_FREQ_450KHZ, /**< 默认 */
} pwm_freq_e;

/// FCCM使能控制
#define EN_FCCM_POS                         3
typedef enum {
    FCCM_DISABLE = 0x00 << EN_FCCM_POS, /**< 关闭 */
    FCCM_ENABLE  = 0x01 << EN_FCCM_POS, /**< 开启 */
    FCCM_DEFAULT = FCCM_DISABLE, /**< 默认 */
} en_fccm_en_e;

/// 输入电源检测功能
#define INDET_CTRL_POS                       2
typedef enum {
    INDET_CTRL_DISABLE = 0x00 << INDET_CTRL_POS, /**< 关闭 */
    INDET_CTRL_ENABLE  = 0x01 << INDET_CTRL_POS, /**< 开启 */
    INDET_CTRL_DEFAULT = INDET_CTRL_DISABLE, /**< 默认 */
} indet_ctrl_e;

/// 抖动使能控制
#define EN_DITHER_POS                        1
typedef enum {
    EN_DITHER_DISABLE = 0x00 << EN_DITHER_POS, /**< 关闭 */
    EN_DITHER_ENABLE  = 0x01 << EN_DITHER_POS, /**< 开启 */
    EN_DITHER_DEFAULT = EN_DITHER_DISABLE, /**< 默认 */
} en_dither_en_e;

/**@brief
 * 充电芯片控制寄存器1
 * @note 该寄存器用于配置充电芯片的控制状态
 */
#define CHARGE_CTRL1_REG                     0x0C
/// 电池电压反馈选择寄存器
#define VBAT_FB_SEL_POS                      7
typedef enum {
    VBAT_FB_SEL_INTERNAL_SETTING  = 0x00 << VBAT_FB_SEL_POS, /**< 内部 */
    VBAT_FB_SEL_EXTERNAL_SETTING  = 0x01 << VBAT_FB_SEL_POS, /**< 外部 */
    VBAT_FB_SEL_DEFAULT  = VBAT_FB_SEL_INTERNAL_SETTING,
} vbat_fb_sel_e;

/// 充电电流反馈选择寄存器
#define ICHG_SEL_POS                         6
typedef enum {
    ICHG_SEL_BUS_CURRENT  = 0x00 << ICHG_SEL_POS, /**< 总线电流 */
    ICHG_SEL_BATTERY_CURRENT  = 0x01 << ICHG_SEL_POS, /**< 电池电流 */
    ICHG_SEL_DEFAULT  = ICHG_SEL_BUS_CURRENT,
} ichg_sel_e;

/// 充电电流涓流阈值寄存器
#define VTRICKLE_TH_POS                      5
typedef enum {
    VTRICKLE_TH_70_PERCENT  = 0x00 << VTRICKLE_TH_POS, /**< 70% of setting battery voltage */
    VTRICKLE_TH_60_PERCENT  = 0x01 << VTRICKLE_TH_POS, /**< 60% of setting battery voltage */
    VTRICKLE_TH_DEFAULT  = VTRICKLE_TH_70_PERCENT,
} vtrickle_th_e;

/// 充电电流涓流使能控制
#define EN_TRICKLE_POS                        4
typedef enum {
    EN_TRICKLE_DISABLE = 0x00 << EN_TRICKLE_POS,
    EN_TRICKLE_ENABLE  = 0x01 << EN_TRICKLE_POS,
    EN_TRICKLE_DEFAULT = EN_TRICKLE_DISABLE,
} en_trickle_en_e;

/// 充电截止电流设置寄存器
#define ITERM_SET_POS                         1
typedef enum {
    ITERM_SET_150MA  = 0x01 << ITERM_SET_POS, /**< 150mA */
    ITERM_SET_200MA  = 0x02 << ITERM_SET_POS, /**< 200mA */
    ITERM_SET_250MA  = 0x03 << ITERM_SET_POS, /**< 250mA */
    ITERM_SET_300MA  = 0x04 << ITERM_SET_POS, /**< 300mA */
    ITERM_SET_350MA  = 0x05 << ITERM_SET_POS, /**< 350mA */
    ITERM_SET_400MA  = 0x06 << ITERM_SET_POS, /**< 400mA */
    ITERM_SET_450MA  = 0x07 << ITERM_SET_POS, /**< 450mA */
    ITERM_SET_DEFAULT  = ITERM_SET_150MA,
} iterm_set_current_e;

/**@brief
 * 充电芯片控制寄存器2
 * @note 该寄存器用于配置充电芯片的控制状态
 */
#define CHARGE_CTRL2_REG                     0x0D

/// 输出过压反馈选择寄存器
#define VOTG_FB_SEL_POS                      7
typedef enum {
    VOTG_FB_SEL_INTERNAL_SETTING  = 0x00 << VOTG_FB_SEL_POS, /**< 内部 */
    VOTG_FB_SEL_EXTERNAL_SETTING  = 0x01 << VOTG_FB_SEL_POS, /**< 外部 */
    VOTG_FB_SEL_DEFAULT  = VOTG_FB_SEL_INTERNAL_SETTING,
} votg_fb_sel_e;

/// 看门狗超时时间设置寄存器
#define WDTMR_SET_POS                        5
typedef enum {
    WDTMR_SET_TIMEOUT_10_SEC   = 0x00 << WDTMR_SET_POS, /**< 10秒 */
    WDTMR_SET_TIMEOUT_20_SEC   = 0x01 << WDTMR_SET_POS, /**< 20秒 */
    WDTMR_SET_TIMEOUT_180_SEC  = 0x02 << WDTMR_SET_POS, /**< 180秒 */
    WDTMR_SET_TIMEOUT_DISABLE  = 0x03 << WDTMR_SET_POS, /**< 关闭 */
    WDTMR_SET_DEFAULT  = WDTMR_SET_TIMEOUT_DISABLE,
} wdtmr_set_timeout_e;

/// 看门狗超时重置控制
#define WDTMR_RESET_POS                      4
typedef enum {
    WDTMR_RESET_IGNORE   = 0x00 << WDTMR_RESET_POS, /**< 不重置 */
    WDTMR_RESET_ENABLE   = 0x01 << WDTMR_RESET_POS, /**< 重置 */
    WDTMR_RESET_DEFAULT  = WDTMR_RESET_IGNORE,
} wdtmr_reset_en_e;

/// 输出过流保护使能控制
#define EN_OTGOCP_POS                        3
typedef enum {
    EN_OTGOCP_DISABLE = 0x00 << EN_OTGOCP_POS,
    EN_OTGOCP_ENABLE  = 0x01 << EN_OTGOCP_POS,
    EN_OTGOCP_DEFAULT = EN_OTGOCP_DISABLE,
} en_otgocp_en_e;

/// 电池欠压保护选择寄存器 
#define BATUVP_SEL_POS                        2
typedef enum {
    BATUVP_SEL_SINGLE_THRESHOLD  = 0x00 << BATUVP_SEL_POS, /**< 单阈值 rising: 3.0V, falling: 2.5V.*/
    BATUVP_SEL_MULTI_THRESHOLD  = 0x01 << BATUVP_SEL_POS, /**< 多阈值 rising: 3.0V/cell, falling: 2.5V/cell.*/
    BATUVP_SEL_DEFAULT  = BATUVP_SEL_SINGLE_THRESHOLD,
} batuvp_sel_e;

/// 输入电流限制寄存器
#define IL_LIMIT_POS                          1
typedef enum {
    IL_LIMIT_15A  = 0x00 << IL_LIMIT_POS, /**< 15A */
    IL_LIMIT_21A  = 0x01 << IL_LIMIT_POS, /**< 21A */
    IL_LIMIT_DEFAULT  = IL_LIMIT_15A,
} il_limit_e;

/**@brief
 * 充电芯片控制寄存器3
 * @note 该寄存器用于配置充电芯片的控制状态
 */
#define CHARGE_CTRL3_REG                     0x0E
/// 充电超时时间设置
#define CHG_TIMER_OUT_TIME_POS               6
typedef enum {
    CHG_TIMER_OUT_TIME_DISABLE  = 0x00 << CHG_TIMER_OUT_TIME_POS, /**< 关闭 */
    CHG_TIMER_OUT_TIME_12_HOURS = 0x01 << CHG_TIMER_OUT_TIME_POS, /**< 12小时 */
    CHG_TIMER_OUT_TIME_24_HOURS = 0x02 << CHG_TIMER_OUT_TIME_POS, /**< 24小时 */
    CHG_TIMER_OUT_TIME_48_HOURS = 0x03 << CHG_TIMER_OUT_TIME_POS, /**< 48小时 */
    CHG_TIMER_OUT_TIME_DEFAULT = CHG_TIMER_OUT_TIME_DISABLE,
} chg_timer_out_time_e;

/// PGATE1控制
#define PGATE1_CTRL_POS                      5
typedef enum {
    PGATE1_CTRL_LOGIC_LOW_OUTPUT   = 0x00 << PGATE1_CTRL_POS, /**< 逻辑低输出 */
    PGATE1_CTRL_OPEN_DRAIN_OUTPUT  = 0x01 << PGATE1_CTRL_POS, /**< 开漏输出 */
    PGATE1_CTRL_DEFAULT = PGATE1_CTRL_OPEN_DRAIN_OUTPUT,
} pgate1_ctrl_e;

/// PGATE2控制
#define PGATE2_CTRL_POS                      4
typedef enum {
    PGATE2_CTRL_LOGIC_LOW_OUTPUT   = 0x00 << PGATE2_CTRL_POS, /**< 逻辑低输出 */
    PGATE2_CTRL_OPEN_DRAIN_OUTPUT  = 0x01 << PGATE2_CTRL_POS, /**< 开漏输出 */
    PGATE2_CTRL_DEFAULT = PGATE2_CTRL_OPEN_DRAIN_OUTPUT,
} pgate2_ctrl_e;

/// BUS_DCHG控制
#define BUS_DCHG_CTRL_POS                      3
typedef enum {
    BUS_DCHG_CTRL_IGNORE  = 0x00 << BUS_DCHG_CTRL_POS, /**< 忽略 */
    BUS_DCHG_CTRL_ENABLE  = 0x01 << BUS_DCHG_CTRL_POS, /**< 开启 */
    BUS_DCHG_CTRL_DEFAULT  = BUS_DCHG_CTRL_IGNORE,
} bus_dchg_ctrl_e;

/// RESET寄存器设置
#define RESET_REGSET_POS                      2
typedef enum {
    RESET_REGSET_IGNORE   = 0x00 << RESET_REGSET_POS, /**< 忽略 */
    RESET_REGSET_ENABLE   = 0x01 << RESET_REGSET_POS, /**< 开启 */
    RESET_REGSET_DEFAULT  = RESET_REGSET_IGNORE,
} reset_regset_e;

/// 充电超时重置控制
#define CHG_TIMER_RESET_POS                      1
typedef enum {
    CHG_TIMER_RESET_IGNORE  = 0x00 << CHG_TIMER_RESET_POS, /**< 忽略 */
    CHG_TIMER_RESET_ENABLE  = 0x01 << CHG_TIMER_RESET_POS, /**< 开启 */
    CHG_TIMER_RESET_DEFAULT  = CHG_TIMER_RESET_IGNORE,
} chg_timer_reset_en_e;

/**@brief
 * 充电芯片NTC设置寄存器
 */
#define CHARGE_NTC_SET_REG                   0x0F
/// NTC充电高温截止阈值
#define NTC_CHG_H_POS                        6
typedef enum {
    NTC_CHG_H_THRESHOLD_45_C     = 0x00 << NTC_CHG_H_POS, /**< 45℃ */
    NTC_CHG_H_THRESHOLD_55_C     = 0x01 << NTC_CHG_H_POS, /**< 55℃ */
    NTC_CHG_H_THRESHOLD_60_C     = 0x02 << NTC_CHG_H_POS, /**< 60℃ */
    NTC_CHG_H_THRESHOLD_DISABLE  = 0x03 << NTC_CHG_H_POS, /**< 关闭 */
} ntc_chg_h_threshold_e;

/// NTC充电低温截止阈值
#define NTC_CHG_L_POS                        4
typedef enum {
    NTC_CHG_L_THRESHOLD_0_C                = 0x00 << NTC_CHG_L_POS, /**< 0℃ */
    NTC_CHG_L_THRESHOLD_BELOW_ZERO_5_C     = 0x01 << NTC_CHG_L_POS, /**< -5℃ */
    NTC_CHG_L_THRESHOLD_BELOW_ZERO_10_C    = 0x02 << NTC_CHG_L_POS, /**< -10℃ */
    NTC_CHG_L_THRESHOLD_DISABLE  = 0x03 << NTC_CHG_L_POS, /**< 关闭 */
} ntc_chg_l_threshold_e;

/// NTC输出过温截止阈值
#define NTC_OTG_H_POS                        2
typedef enum {
    NTC_OTG_H_THRESHOLD_55_C     = 0x00 << NTC_OTG_H_POS, /**< 55℃ */
    NTC_OTG_H_THRESHOLD_60_C     = 0x01 << NTC_OTG_H_POS, /**< 60℃ */
    NTC_OTG_H_THRESHOLD_65_C     = 0x02 << NTC_OTG_H_POS, /**< 65℃ */
    NTC_OTG_H_THRESHOLD_DISABLE  = 0x03 << NTC_OTG_H_POS, /**< 关闭 */
} ntc_otg_h_threshold_e;

/// NTC输出低温截止阈值
#define NTC_OTG_L_POS                        0
typedef enum {
    NTC_OTG_L_THRESHOLD_BELOW_ZERO_5_C     = 0x00 << NTC_OTG_L_POS, /**< -5℃ */
    NTC_OTG_L_THRESHOLD_BELOW_ZERO_10_C    = 0x01 << NTC_OTG_L_POS, /**< -10℃ */
    NTC_OTG_L_THRESHOLD_BELOW_ZERO_20_C    = 0x02 << NTC_OTG_L_POS, /**< -20℃ */
    NTC_OTG_L_THRESHOLD_DISABLE = 0x03 << NTC_OTG_L_POS, /**< 关闭 */
} ntc_otg_l_threshold_e;

/**@brief
 * 充电芯片ADC设置寄存器
 */
#define CHARGE_ADC_SET_REG                   0x10

/// ADC启动使能
#define AD_START_POS                         7
typedef enum {
    AD_START_DISABLE = 0x00 << AD_START_POS,
    AD_START_ENABLE  = 0x01 << AD_START_POS,
    AD_START_DEFAULT = AD_START_DISABLE,
} ad_start_en_e;

/// 总线电压使能
#define EN_ADC_VBUS_POS                       6
typedef enum {
    ADC_VBUS_DISABLE = 0x00 << EN_ADC_VBUS_POS,
    ADC_VBUS_ENABLE  = 0x01 << EN_ADC_VBUS_POS,
    ADC_VBUS_DEFAULT = ADC_VBUS_ENABLE,
} en_adc_vbus_en_e;

/// 总线电流使能
#define EN_ADC_IBUS_POS                       5
typedef enum {
    ADC_IBUS_DISABLE = 0x00 << EN_ADC_IBUS_POS,
    ADC_IBUS_ENABLE  = 0x01 << EN_ADC_IBUS_POS,
    ADC_IBUS_DEFAULT = ADC_IBUS_ENABLE,
} en_adc_ibus_en_e;

/// 电池电压使能
#define EN_ADC_VBAT_POS                       4
typedef enum {
    ADC_VBAT_DISABLE = 0x00 << EN_ADC_VBAT_POS,
    ADC_VBAT_ENABLE  = 0x01 << EN_ADC_VBAT_POS,
    ADC_VBAT_DEFAULT = ADC_VBAT_ENABLE,
} en_adc_vbat_en_e;

/// 充电电流使能
#define EN_ADC_IBAT_POS                       3
typedef enum {
    ADC_IBAT_DISABLE = 0x00 << EN_ADC_IBAT_POS,
    ADC_IBAT_ENABLE  = 0x01 << EN_ADC_IBAT_POS,
    ADC_IBAT_DEFAULT = ADC_IBAT_ENABLE,
} en_adc_ibat_en_e;

/// NTC使能
#define EN_ADC_NTC_POS                        2
typedef enum {
    ADC_NTC_DISABLE = 0x00 << EN_ADC_NTC_POS,
    ADC_NTC_ENABLE  = 0x01 << EN_ADC_NTC_POS,
    ADC_NTC_DEFAULT = ADC_NTC_ENABLE,
} en_adc_ntc_en_e;

/**@brief
 * 充电芯片ADC比例寄存器
 */
#define CHARGE_ADC_SCALE_REG                 0x11

/// 总线电压比例
#define VBUS_SCALE_POS                       7
typedef enum {
    VBUS_SCALE_1 = 0x00 << VBUS_SCALE_POS,
    VBUS_SCALE_2 = 0x01 << VBUS_SCALE_POS,
    VBUS_SCALE_DEFAULT = VBUS_SCALE_1,
} vbus_scale_e;

/// 总线电流比例
#define IBUS_SCALE_POS                       6
typedef enum {
    IBUS_SCALE_1 = 0x00 << IBUS_SCALE_POS,
    IBUS_SCALE_2 = 0x01 << IBUS_SCALE_POS,
    IBUS_SCALE_DEFAULT = IBUS_SCALE_2,
} ibus_scale_e;

/// 电池电压比例
#define VBAT_SCALE_POS                       5
typedef enum {
    VBAT_SCALE_1 = 0x00 << VBAT_SCALE_POS,
    VBAT_SCALE_2 = 0x01 << VBAT_SCALE_POS,
    VBAT_SCALE_DEFAULT = VBAT_SCALE_1,
} vbat_scale_e;

/// 充电电流比例
#define IBAT_SCALE_POS                       4
typedef enum {
    IBAT_SCALE_1 = 0x00 << IBAT_SCALE_POS,
    IBAT_SCALE_2 = 0x01 << IBAT_SCALE_POS,
    IBAT_SCALE_DEFAULT = IBAT_SCALE_2,
} ibat_scale_e;

/**@brief
 * 充电芯片ADC寄存器
 * @note 该寄存器用于读取总线电压的ADC值, 10BIT
 * VBUS_SCALE = 0, LSB = 20mV,
 * VBUS_SCALE = 1, LSB = 40mV,
 */
#define CHARGE_ADC_VBUS_L_REG                0x12
#define CHARGE_ADC_VBUS_H_REG                0x13
/**@brief
 * 充电芯片ADC寄存器
 * @note 该寄存器用于读取总线电流的ADC值, 10BIT
 * IBUS_SCALE = 0, LSB = 6.25mA,
 * IBUS_SCALE = 1, LSB = 12.5mV,
 */
#define CHARGE_ADC_IBUS_L_REG                0x14
#define CHARGE_ADC_IBUS_H_REG                0x15
/**@brief
 * 充电芯片ADC寄存器
 * @note 该寄存器用于读取电池电压的ADC值, 10BIT
 * VBAT_SCALE = 0, LSB = 20mV,
 * VBAT_SCALE = 1, LSB = 40mV,
 */
#define CHARGE_ADC_VBAT_L_REG                0x16
#define CHARGE_ADC_VBAT_H_REG                0x17
/**@brief
 * 充电芯片ADC寄存器
 * @note 该寄存器用于读取充电电流的ADC值, 10BIT
 * IBAT_SCALE = 0, LSB = 6.25mA, 
 * VBAT_SCALE = 1, LSB = 12.5mV,
 */
#define CHARGE_ADC_IBAT_L_REG                0x18
#define CHARGE_ADC_IBAT_H_REG                0x19

/**@brief
 * 充电芯片ADC寄存器
 * @note 该寄存器用于读取充电芯片的ADC值, 10BIT
 * LSB = 2mV, NTC_MAX = 1023
 */
#define CHARGE_ADC_NTC_L_REG                 0x1A
#define CHARGE_ADC_NTC_H_REG                 0x1B

/**@brief
 * 充电芯片状态寄存器0
 * @note 该寄存器用于读取充电芯片的状态
 */
#define CHARGE_STATUS0_REG                   0x1C
/// AC 适配器检测状态
#define AC_PRESENT_STATUS_POS                (7 + 8)
typedef enum {
    CHARGE_AC_PRESENT_NO_TRIGGER = 0x00 << AC_PRESENT_STATUS_POS,
    CHARGE_AC_PRESENT_TRIGGER  = 0x01 << AC_PRESENT_STATUS_POS,
} charge_ac_present_trig_e;

/// 插入检测状态
#define INDET_STATUS_POS                     (5 + 8)
typedef enum {
    CHARGE_INDET_NO_TRIGGER = 0x00 << INDET_STATUS_POS,
    CHARGE_INDET_TRIGGER  = 0x01 << INDET_STATUS_POS,
} charge_indet_trig_e;

/// 总线欠压保护充电状态
#define BUSUVP_CHG_STATUS_POS                (3 + 8)
typedef enum {
    CHARGE_BUSUVP_CHG_NO_TRIGGER = 0x00 << BUSUVP_CHG_STATUS_POS,
    CHARGE_BUSUVP_CHG_TRIGGER  = 0x01 << BUSUVP_CHG_STATUS_POS,
} charge_busuvp_chg_trig_e;

/// 电池过压保护状态
#define BATOVP_STATUS_POS                    (2 + 8)
typedef enum {
    CHARGE_BATOVP_NO_TRIGGER = 0x00 << BATOVP_STATUS_POS,
    CHARGE_BATOVP_TRIGGER  = 0x01 << BATOVP_STATUS_POS,
} charge_batovp_trig_e;

/// 电池满电状态
#define BAT_FULL_STATUS_POS                  (1 + 8)
typedef enum {
    CHARGE_BAT_FULL_NO_TRIGGER = 0x00 << BAT_FULL_STATUS_POS,
    CHARGE_BAT_FULL_TRIGGER  = 0x01 << BAT_FULL_STATUS_POS,
} charge_bat_full_trig_e;

/// 重新充电状态
#define RE_CHG_STATUS_POS                    (0 + 8)
typedef enum {
    CHARGE_RE_CHG_NO_TRIGGER = 0x00 << RE_CHG_STATUS_POS,
    CHARGE_RE_CHG_TRIGGER  = 0x01 << RE_CHG_STATUS_POS,
} charge_re_chg_trig_e;

/**@brief
 * 充电芯片状态寄存器1
 * @note 该寄存器用于读取充电芯片的状态
 */
#define CHARGE_STATUS1_REG                   0x1D

/// 输出过压保护状态
#define OTGOVP_STATUS_POS                     7
typedef enum {
    CHARGE_OTGOVP_NO_FAULT = 0x00 << OTGOVP_STATUS_POS,
    CHARGE_OTGOVP_FAULT  = 0x01 << OTGOVP_STATUS_POS,
} charge_otgovp_status_e;

/// 输出过流保护状态
#define OTGOC_STATUS_POS                     6
typedef enum {
    CHARGE_OTGOC_NO_FAULT = 0x00 << OTGOC_STATUS_POS,
    CHARGE_OTGOC_FAULT  = 0x01 << OTGOC_STATUS_POS,
} charge_otgoc_status_e;

/// 电池欠压保护状态
#define BAT_EMPTY_STATUS_POS                 5
typedef enum {
    CHARGE_BAT_EMPTY_NO_FAULT = 0x00 << BAT_EMPTY_STATUS_POS,
    CHARGE_BAT_EMPTY_FAULT  = 0x01 << BAT_EMPTY_STATUS_POS,
} charge_bat_empty_status_e;

/// 充电超时状态
#define CHG_TIMEOUT_STATUS_POS               4
typedef enum {
    CHARGE_CHG_TIMEOUT_NO_FAULT = 0x00 << CHG_TIMEOUT_STATUS_POS,
    CHARGE_CHG_TIMEOUT_FAULT  = 0x01 << CHG_TIMEOUT_STATUS_POS,
} charge_chg_timeout_status_e;

/// 看门狗超时状态
#define WD_TIMEOUT_STATUS_POS                3
typedef enum {
    CHARGE_WD_TIMEOUT_NO_FAULT = 0x00 << WD_TIMEOUT_STATUS_POS,
    CHARGE_WD_TIMEOUT_FAULT  = 0x01 << WD_TIMEOUT_STATUS_POS,
} charge_wd_timeout_status_e;

/// 过温关机状态
#define TSHUT_STATUS_POS                     2
typedef enum {
    CHARGE_TSHUT_NO_FAULT = 0x00 << TSHUT_STATUS_POS,
    CHARGE_TSHUT_FAULT  = 0x01 << TSHUT_STATUS_POS,
} charge_tshut_status_e;

/// NTC故障状态
#define NTC_FAULT_STATUS_POS                 1
typedef enum {
    CHARGE_NTC_FAULT_NO_FAULT = 0x00 << NTC_FAULT_STATUS_POS,
    CHARGE_NTC_FAULT_FAULT  = 0x01 << NTC_FAULT_STATUS_POS,
} charge_ntc_fault_status_e;

/**@brief
 * 充电芯片中断使能寄存器0
 * @note 该寄存器用于配置充电芯片的中断使能状态
 */
#define CHARGE_INT_EN0_REG                   0x1E
/// AC 适配器检测中断
#define AC_PRESENT_INT_EN_POS                7
typedef enum {
    CHARGE_AC_PRESENT_INT_DISABLE = 0x00 << AC_PRESENT_INT_EN_POS,
    CHARGE_AC_PRESENT_INT_ENABLE  = 0x01 << AC_PRESENT_INT_EN_POS,
    CHARGE_AC_PRESENT_INT_DEFAULT = CHARGE_AC_PRESENT_INT_ENABLE,
} charge_ac_present_int_en_e;

/// 插入检测中断
#define INDET_INT_EN_POS                     5
typedef enum {
    CHARGE_INDET_INT_DISABLE = 0x00 << INDET_INT_EN_POS,
    CHARGE_INDET_INT_ENABLE  = 0x01 << INDET_INT_EN_POS,
    CHARGE_INDET_INT_DEFAULT = CHARGE_INDET_INT_ENABLE,
} charge_indet_int_en_e;

/// 总线欠压保护充电中断
#define BUSUVP_CHG_INT_EN_POS                3
typedef enum {
    CHARGE_BUSUVP_CHG_INT_DISABLE = 0x00 << BUSUVP_CHG_INT_EN_POS,
    CHARGE_BUSUVP_CHG_INT_ENABLE  = 0x01 << BUSUVP_CHG_INT_EN_POS,
    CHARGE_BUSUVP_CHG_INT_DEFAULT = CHARGE_BUSUVP_CHG_INT_ENABLE,
} charge_busuvp_chg_int_en_e;

/// 电池过压保护中断
#define BATOVP_INT_EN_POS                     2
typedef enum {
    CHARGE_BATOVP_INT_DISABLE = 0x00 << BATOVP_INT_EN_POS,
    CHARGE_BATOVP_INT_ENABLE  = 0x01 << BATOVP_INT_EN_POS,
    CHARGE_BATOVP_INT_DEFAULT = CHARGE_BATOVP_INT_ENABLE,
} charge_batovp_int_en_e;

/// 电池满电中断
#define BAT_FULL_INT_EN_POS                   1
typedef enum {
    CHARGE_BAT_FULL_INT_DISABLE = 0x00 << BAT_FULL_INT_EN_POS,
    CHARGE_BAT_FULL_INT_ENABLE  = 0x01 << BAT_FULL_INT_EN_POS,
    CHARGE_BAT_FULL_INT_DEFAULT = CHARGE_BAT_FULL_INT_ENABLE,
} charge_bat_full_int_en_e;

/// 重新充电中断
#define RE_CHG_INT_EN_POS                     0
typedef enum {
    CHARGE_RE_CHG_INT_DISABLE = 0x00 << RE_CHG_INT_EN_POS,
    CHARGE_RE_CHG_INT_ENABLE  = 0x01 << RE_CHG_INT_EN_POS,
    CHARGE_RE_CHG_INT_DEFAULT = CHARGE_RE_CHG_INT_ENABLE,
} charge_re_chg_int_en_e;

/**@brief
 * 充电芯片中断使能寄存器1
 * @note 该寄存器用于配置充电芯片的中断使能状态
 */
#define CHARGE_INT_EN1_REG                   0x1F

/// 输出过压保护中断
#define OTGOVP_INT_EN_POS                     7
typedef enum {
    CHARGE_OTGOVP_INT_DISABLE = 0x00 << OTGOVP_INT_EN_POS,
    CHARGE_OTGOVP_INT_ENABLE  = 0x01 << OTGOVP_INT_EN_POS,
    CHARGE_OTGOVP_INT_DEFAULT = CHARGE_OTGOVP_INT_ENABLE,
} charge_otgovp_int_en_e;

/// 输出过流保护中断
#define OTGOC_INT_EN_POS                     6
typedef enum {
    CHARGE_OTGOC_INT_DISABLE = 0x00 << OTGOC_INT_EN_POS,
    CHARGE_OTGOC_INT_ENABLE  = 0x01 << OTGOC_INT_EN_POS,
    CHARGE_OTGOC_INT_DEFAULT = CHARGE_OTGOC_INT_ENABLE,
} charge_otgoc_int_en_e;

/// 电池欠压保护中断
#define BAT_EMPTY_INT_EN_POS                 5
typedef enum {
    CHARGE_BAT_EMPTY_INT_DISABLE = 0x00 << BAT_EMPTY_INT_EN_POS,
    CHARGE_BAT_EMPTY_INT_ENABLE  = 0x01 << BAT_EMPTY_INT_EN_POS,
    CHARGE_BAT_EMPTY_INT_DEFAULT = CHARGE_BAT_EMPTY_INT_ENABLE,
} charge_bat_empty_int_en_e;

/// 充电超时中断
#define CHG_TIMEOUT_INT_EN_POS               4
typedef enum {
    CHARGE_CHG_TIMEOUT_INT_DISABLE = 0x00 << CHG_TIMEOUT_INT_EN_POS,
    CHARGE_CHG_TIMEOUT_INT_ENABLE  = 0x01 << CHG_TIMEOUT_INT_EN_POS,
    CHARGE_CHG_TIMEOUT_INT_DEFAULT = CHARGE_CHG_TIMEOUT_INT_ENABLE,
} charge_chg_timeout_int_en_e;

/// 看门狗超时中断
#define WD_TIMEOUT_INT_EN_POS                3
typedef enum {
    CHARGE_WD_TIMEOUT_INT_DISABLE = 0x00 << WD_TIMEOUT_INT_EN_POS,
    CHARGE_WD_TIMEOUT_INT_ENABLE  = 0x01 << WD_TIMEOUT_INT_EN_POS,
    CHARGE_WD_TIMEOUT_INT_DEFAULT = CHARGE_WD_TIMEOUT_INT_ENABLE,
} charge_wd_timeout_int_en_e;

/// 过温关机中断
#define TSHUT_INT_EN_POS                     2
typedef enum {
    CHARGE_TSHUT_INT_DISABLE = 0x00 << TSHUT_INT_EN_POS,
    CHARGE_TSHUT_INT_ENABLE  = 0x01 << TSHUT_INT_EN_POS,
    CHARGE_TSHUT_INT_DEFAULT = CHARGE_TSHUT_INT_ENABLE,
} charge_tshut_int_en_e;

/// NTC故障中断
#define NTC_FAULT_INT_EN_POS                  1
typedef enum {
    CHARGE_NTC_FAULT_INT_DISABLE = 0x00 << NTC_FAULT_INT_EN_POS,
    CHARGE_NTC_FAULT_INT_ENABLE  = 0x01 << NTC_FAULT_INT_EN_POS,
    CHARGE_NTC_FAULT_INT_DEFAULT = CHARGE_NTC_FAULT_INT_ENABLE,
} charge_ntc_fault_int_en_e;

/**@brief
 * 充电芯片ID寄存器
 * @note 该寄存器用于读取充电芯片的ID
 */
#define CHARGE_ID_REG                        0xFE

/// 充电芯片ID值
#define CHARGE_ID_VALUE                      0x04

/// 充电芯片状态
typedef struct {
    uint16_t vbus; /// 总线电压 mV
    uint16_t ibus; /// 总线电流 mA
    uint16_t vbat; /// 电池电压 mV
    uint16_t ibat; /// 电池电流 mA
    uint16_t status; /// 状态寄存器
    float ntc_resistance; /// ntc电阻值 KΩ
    uint8_t charge_full; /// 满电状态
    uint8_t plug_in; /// 插拔状态
    float temp; /// 温度℃
} charge_chip_status_t;

/// 输入电流限制
#define IBUS_SCALE_CFG                      IBUS_SCALE_2
#define CHARGE_INPUT_CURRENT_LIMIT          3000
#if IBUS_SCALE_CFG == IBUS_SCALE_2
#define CHARGE_INPUT_CURRENT_LIMIT_SCALE    50
#define CHARGE_ADC_CURRENT_LIMIT_SCALE      12.5f
#else
#define CHARGE_INPUT_CURRENT_LIMIT_SCALE    25
#define CHARGE_ADC_CURRENT_LIMIT_SCALE      6.25f
#endif

/// 输入电压限制
#define VBUS_SCALE_CFG                      VBUS_SCALE_2
#define CHARGE_INPUT_VOLTAGE_LIMIT          4200
#if VBUS_SCALE_CFG == VBUS_SCALE_2
#define CHARGE_INPUT_VOLTAGE_LIMIT_SCALE    40
#else
#define CHARGE_INPUT_VOLTAGE_LIMIT_SCALE    20
#endif

/// 电池输入/输出电流限制
#define IBAT_SCALE_CFG                      IBAT_SCALE_2
#define BATT_INPUT_CURRENT_LIMIT            2000
#define BATT_OUTPUT_CURRENT_LIMIT           3000
#define TEMP_ABNORMAL_CHARGING_CURRENT      600
#if IBAT_SCALE_CFG == IBAT_SCALE_2
#define BATT_CURRENT_LIMIT_SCALE            50
#define BATT_ADC_CURRENT_LIMIT_SCALE        12.5f
#else
#define BATT_CURRENT_LIMIT_SCALE            25
#define BATT_ADC_CURRENT_LIMIT_SCALE        6.25f
#endif

/// 电池输入/输出电压限制
#define VBAT_SCALE_CFG                      VBAT_SCALE_2
#if VBAT_SCALE_CFG == VBAT_SCALE_2
#define BATT_VOLTAGE_SCALE                  40
#else
#define BATT_VOLTAGE_SCALE                  20
#endif

/// ntc电压比例
#define NTC_VOLTAGE_SCALE                   2
/// ntc电流系数
#define NTC_CURRENT_COEFF                   30.0f

#define CHARGE_NORMAL_TEMP_MIN              15
#define CHARGE_NORMAL_TEMP_MAX              45
#define CHARGE_DERATED_TEMP_MIN             0
#define CHARGE_DERATED_TEMP_MAX             60

typedef enum {
    CHARGE_TEMP_UNKNOW,
    CHARGE_TEMP_NORMAL,   /// 正常电流充电
    CHARGE_TEMP_DERATED,  /// 降低电流充电
    CHARGE_TEMP_DISABLE,  /// 禁止充电
} charging_temp_status_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 
