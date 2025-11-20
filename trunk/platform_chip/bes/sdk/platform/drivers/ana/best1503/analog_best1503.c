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
#include "analog.h"
#include CHIP_SPECIFIC_HDR(reg_analog)
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_chipid.h"
#include "hal_cmu.h"
#include "hal_codec.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "pmu.h"
#include "string.h"
#include "tgt_hardware.h"

#define VCM_ON
//#define VCM_ALWAYS_LP

// Not using 1uF
#define VCM_CAP_100NF

#define AUDPLL_SHUTDOWN_WORKAROUD

#define DAC_DC_CALIB_BIT_WIDTH              14

#define DAC_DC_ADJUST_STEP                  90

//adc dc calib
//ana_76 reg_codec_adcB_ibsel_offset[3:0] | ana_77 reg_codec_adcB_offset_bit[0:12]
//0x4                                     | {5,5,10,10,20,40,40,80,160,320,320,640,1280};
//0x8                                     | {5,5,10,10,20,40,40,80,160,320,320,640,1280}*2;
//0xF                                     | {5,5,10,10,20,40,40,80,160,320,320,640,1280}*4;
#define CODEC_ADC_IBSEL_OFFSET     0xF
#define CODEC_ADC_OFFSET_BIT       12

#define DEFAULT_ANC_FF_ADC_GAIN_DB          6
#define DEFAULT_ANC_FB_ADC_GAIN_DB          6
#define DEFAULT_ANC_TT_ADC_GAIN_DB          6
#define DEFAULT_VOICE_ADC_GAIN_DB           12

#ifndef ANALOG_ADC_A_GAIN_DB
#if defined(ANC_APP) && defined(ANC_FF_ENABLED) && ((ANC_FF_MIC_CH_L == AUD_CHANNEL_MAP_CH0) || (ANC_FF_MIC_CH_R == AUD_CHANNEL_MAP_CH0))
#define ANALOG_ADC_A_GAIN_DB                DEFAULT_ANC_FF_ADC_GAIN_DB
#elif defined(ANC_APP) && defined(ANC_FB_ENABLED) && ((ANC_FB_MIC_CH_L == AUD_CHANNEL_MAP_CH0) || (ANC_FB_MIC_CH_R == AUD_CHANNEL_MAP_CH0))
#define ANALOG_ADC_A_GAIN_DB                DEFAULT_ANC_FB_ADC_GAIN_DB
#elif defined(ANC_APP) && (defined(AUDIO_ANC_TT_HW)||defined(PSAP_APP)) && ((ANC_TT_MIC_CH_L == AUD_CHANNEL_MAP_CH0) || (ANC_TT_MIC_CH_R == AUD_CHANNEL_MAP_CH0))
#define ANALOG_ADC_A_GAIN_DB                DEFAULT_ANC_TT_ADC_GAIN_DB
#else
#define ANALOG_ADC_A_GAIN_DB                DEFAULT_VOICE_ADC_GAIN_DB
#endif
#endif

#ifndef ANALOG_ADC_B_GAIN_DB
#if defined(ANC_APP) && defined(ANC_FF_ENABLED) && ((ANC_FF_MIC_CH_L == AUD_CHANNEL_MAP_CH1) || (ANC_FF_MIC_CH_R == AUD_CHANNEL_MAP_CH1))
#define ANALOG_ADC_B_GAIN_DB                DEFAULT_ANC_FF_ADC_GAIN_DB
#elif defined(ANC_APP) && defined(ANC_FB_ENABLED) && ((ANC_FB_MIC_CH_L == AUD_CHANNEL_MAP_CH1) || (ANC_FB_MIC_CH_R == AUD_CHANNEL_MAP_CH1))
#define ANALOG_ADC_B_GAIN_DB                DEFAULT_ANC_FB_ADC_GAIN_DB
#elif defined(ANC_APP) && (defined(AUDIO_ANC_TT_HW)||defined(PSAP_APP)) && ((ANC_TT_MIC_CH_L == AUD_CHANNEL_MAP_CH1) || (ANC_TT_MIC_CH_R == AUD_CHANNEL_MAP_CH1))
#define ANALOG_ADC_B_GAIN_DB                DEFAULT_ANC_TT_ADC_GAIN_DB
#else
#define ANALOG_ADC_B_GAIN_DB                DEFAULT_VOICE_ADC_GAIN_DB
#endif
#endif

#ifndef ANALOG_ADC_C_GAIN_DB
#if defined(ANC_APP) && defined(ANC_FF_ENABLED) && ((ANC_FF_MIC_CH_L == AUD_CHANNEL_MAP_CH2) || (ANC_FF_MIC_CH_R == AUD_CHANNEL_MAP_CH2))
#define ANALOG_ADC_C_GAIN_DB                DEFAULT_ANC_FF_ADC_GAIN_DB
#elif defined(ANC_APP) && defined(ANC_FB_ENABLED) && ((ANC_FB_MIC_CH_L == AUD_CHANNEL_MAP_CH2) || (ANC_FB_MIC_CH_R == AUD_CHANNEL_MAP_CH2))
#define ANALOG_ADC_C_GAIN_DB                DEFAULT_ANC_FB_ADC_GAIN_DB
#elif defined(ANC_APP) && (defined(AUDIO_ANC_TT_HW)||defined(PSAP_APP))  && ((ANC_TT_MIC_CH_L == AUD_CHANNEL_MAP_CH2) || (ANC_TT_MIC_CH_R == AUD_CHANNEL_MAP_CH2))
#define ANALOG_ADC_C_GAIN_DB                DEFAULT_ANC_TT_ADC_GAIN_DB
#else
#define ANALOG_ADC_C_GAIN_DB                DEFAULT_VOICE_ADC_GAIN_DB
#endif
#endif

#ifndef LINEIN_ADC_GAIN_DB
#define LINEIN_ADC_GAIN_DB                  0
#endif

#ifndef CFG_HW_AUD_MICKEY_DEV
#define CFG_HW_AUD_MICKEY_DEV               (AUD_VMIC_MAP_VMIC1)
#endif

#ifndef ANC_VMIC_CFG
#define ANC_VMIC_CFG                        (AUD_VMIC_MAP_VMIC1)
#endif

#define CODEC_BBPLL_DAC_EN                  RESERVED_ANA_15_0(1 << 0)
#define CODEC_BBPLL_ADC_EN                  RESERVED_ANA_15_0(1 << 1)
#define CODEC_TX_EN_ANA_LDAC                RESERVED_ANA_15_0(1 << 2)

#define PMU_EFUSE_PAGE_DCCALIB2_L           PMU_EFUSE_PAGE_DCDC2_VOLT
#define PMU_EFUSE_PAGE_DCCALIB2_L_LP        PMU_EFUSE_PAGE_DCDC3_VOLT
#define PMU_EFUSE_PAGE_DCCALIB_L            PMU_EFUSE_PAGE_CP_WAFER_XY_CTX
#define PMU_EFUSE_PAGE_DCCALIB_L_LP         PMU_EFUSE_PAGE_CP_WAFER_ID

enum ANA_CODEC_USER_T {
    ANA_CODEC_USER_DAC          = (1 << 0),
    ANA_CODEC_USER_ADC          = (1 << 1),

    ANA_CODEC_USER_CODEC        = (1 << 2),
    ANA_CODEC_USER_MICKEY       = (1 << 3),

    ANA_CODEC_USER_ANC_FF       = (1 << 4),
    ANA_CODEC_USER_ANC_FB       = (1 << 5),
    ANA_CODEC_USER_ANC_TT       = (1 << 6),

    ANA_CODEC_USER_VAD          = (1 << 7),
    ANA_CODEC_USER_RPCSVR_ADC   = (1 << 8),
};

enum ANA_CODEC_INIT_T {
    ANA_CODEC_INIT_FIRST          = (1 << 0),
    ANA_CODEC_INIT_FULL           = (1 << 1),
    ANA_CODEC_INIT_QUICK          = (1 << 2),
};

enum ANA_OSC_CLK_USER_T {
    ANA_OSC_CLK_USER_CODEC      = (1 << 0),
    ANA_OSC_CLK_USER_VAD        = (1 << 1),
};

struct ANALOG_PLL_CFG_T {
    uint32_t freq;
    uint8_t div;
    uint64_t val;
};

#ifdef ANC_PROD_TEST
#define OPT_TYPE
#else
#define OPT_TYPE                        const
#endif

static OPT_TYPE uint16_t vcodec_mv = (uint16_t)(VCODEC_VOLT * 1000);

static bool ana_spk_req;
static bool ana_spk_muted;
static bool ana_spk_enabled;

static bool anc_calib_mode;

static enum ANA_CODEC_USER_T adc_map[MAX_ANA_MIC_CH_NUM];
static enum ANA_CODEC_USER_T vmic_map[MAX_VMIC_CH_NUM];
static enum ANA_CODEC_USER_T codec_common_map;
static enum ANA_CODEC_USER_T adda_common_map;
static enum ANA_CODEC_USER_T bbpll_common_map;

static enum ANA_AUD_PLL_USER_T ana_aud_pll_map;
static enum ANA_OSC_CLK_USER_T osc_clk_map = 0;


#ifdef ANC_APP
#ifndef DYN_ADC_GAIN
#define DYN_ADC_GAIN
#endif
#if defined(ANC_FF_MIC_CH_L)
#if defined(ANC_TT_MIC_CH_L)
static int8_t anc_tt_gain_db_l;
#endif
#endif
#if defined(ANC_FF_MIC_CH_R)
#if defined(ANC_TT_MIC_CH_R)
static int8_t anc_tt_gain_db_r;
#endif
#endif
#endif

static const int8_t adc_db[] = { -9, -6, -3, 0, 3, 6, 9, 12, };

static const int8_t tgt_adc_db[MAX_ANA_MIC_CH_NUM] = {
    ANALOG_ADC_A_GAIN_DB, ANALOG_ADC_B_GAIN_DB,
    ANALOG_ADC_C_GAIN_DB,
};

#ifdef DYN_ADC_GAIN
static int8_t dyn_adc_gain_db[MAX_ANA_MIC_CH_NUM];
STATIC_ASSERT(sizeof(dyn_adc_gain_db) == sizeof(tgt_adc_db), "dyn_adc_gain_db should have the same type and size as tgt_adc_db");
#endif

#ifdef LOW_POWER_ADC
static bool anc_enabled;
static const int8_t low_bias_offset = -12;
#endif

// Max allowed total tune ratio (5000ppm)
#define MAX_TOTAL_TUNE_RATIO                0.005000

static struct ANALOG_PLL_CFG_T ana_pll_cfg[2];
static int pll_cfg_idx;

void analog_aud_freq_pll_config(uint32_t freq, uint32_t div)
{
    // CODEC_FREQ is likely 24.576M (48K series) or 22.5792M (44.1K series)
    // PLL_nominal = CODEC_FREQ * CODEC_DIV
    // PLL_cfg_val = ((CODEC_FREQ * CODEC_DIV) / OSC) * (1 << 28)

    int i, j;
    uint64_t PLL_cfg_val;
    uint32_t crystal;

    if (pll_cfg_idx < ARRAY_SIZE(ana_pll_cfg) &&
            ana_pll_cfg[pll_cfg_idx].freq == freq &&
            ana_pll_cfg[pll_cfg_idx].div == div) {
        return;
    }

    crystal = hal_cmu_get_crystal_freq();

    j = ARRAY_SIZE(ana_pll_cfg);
    for (i = 0; i < ARRAY_SIZE(ana_pll_cfg); i++) {
        if (ana_pll_cfg[i].freq == freq && ana_pll_cfg[i].div == div) {
            break;
        }
        if (j == ARRAY_SIZE(ana_pll_cfg) && ana_pll_cfg[i].freq == 0) {
            j = i;
        }
    }

    if (i < ARRAY_SIZE(ana_pll_cfg)) {
        pll_cfg_idx = i;
        PLL_cfg_val = ana_pll_cfg[pll_cfg_idx].val;
    } else {
        if (j < ARRAY_SIZE(ana_pll_cfg)) {
            pll_cfg_idx = j;
        } else {
            pll_cfg_idx = 0;
        }
        if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_6){
            PLL_cfg_val = ((uint64_t)(1 << 25) * freq * div + crystal / 2) / crystal;
        }else{
            PLL_cfg_val = ((uint64_t)(1 << 28) * freq * div + crystal / 2) / crystal;
        }
        ana_pll_cfg[pll_cfg_idx].freq = freq;
        ana_pll_cfg[pll_cfg_idx].div = div;
        ana_pll_cfg[pll_cfg_idx].val = PLL_cfg_val;
    }

    bbpll_pll_update(PLL_cfg_val, true);
    bbpll_set_codec_div(div);
}

void analog_aud_pll_tune(float ratio)
{
    // CODEC_FREQ is likely 24.576M (48K series) or 22.5792M (44.1K series)
    // PLL_nominal = CODEC_FREQ * CODEC_DIV
    // PLL_cfg_val = ((CODEC_FREQ * CODEC_DIV) / 26M) * (1 << 28)
    // Delta = ((SampleDiff / Fs) / TimeDiff) * PLL_cfg_val

    int64_t delta, new_pll;

    if (pll_cfg_idx >= ARRAY_SIZE(ana_pll_cfg) ||
            ana_pll_cfg[pll_cfg_idx].freq == 0) {
        ANALOG_INFO_TRACE(1,"%s: WARNING: aud pll config cache invalid. Skip tuning", __FUNCTION__);
        return;
    }

    if (ABS(ratio) > MAX_TOTAL_TUNE_RATIO) {
        ANALOG_INFO_TRACE(1,"\n------\nWARNING: TUNE: ratio=%d is too large and will be cut\n------\n", FLOAT_TO_PPB_INT(ratio));
        if (ratio > 0) {
            ratio = MAX_TOTAL_TUNE_RATIO;
        } else {
            ratio = -MAX_TOTAL_TUNE_RATIO;
        }
    }

    ANALOG_INFO_TRACE(2,"%s: ratio=%d", __FUNCTION__, FLOAT_TO_PPB_INT(ratio));

    new_pll = (int64_t)ana_pll_cfg[pll_cfg_idx].val;
    delta = (int64_t)(new_pll * ratio);

    new_pll += delta;

    bbpll_pll_update(new_pll, false);
}

void analog_aud_osc_clk_enable(enum ANA_OSC_CLK_USER_T user, bool enable)
{
    uint32_t lock;
    bool set = false;

    lock = int_lock();
    if (enable) {
        if (osc_clk_map == 0) {
            set = true;
        }
        osc_clk_map |= user;
    } else {
        osc_clk_map &= ~user;
        if (osc_clk_map == 0) {
            set = true;
        }
    }
    int_unlock(lock);

    if (set) {
        uint16_t val;

        if (enable) {
            analog_read(ANA_REG_84, &val);
#ifdef AUDPLL_SHUTDOWN_WORKAROUD
            val |= REG_CRYSTAL_SEL_LV;
#else
            val |= REG_PU_OSC | REG_CRYSTAL_SEL_LV;
#endif
            analog_write(ANA_REG_84, val);
#if 0
            analog_read(ANA_REG_41, &val);
            val |= CFG_TX_CLK_INV;
            analog_write(ANA_REG_41, val);
#endif
    } else {
#if 0
            analog_read(ANA_REG_41, &val);
            val &= ~CFG_TX_CLK_INV;
            analog_write(ANA_REG_41, val);
#endif
            analog_read(ANA_REG_84, &val);
#ifdef AUDPLL_SHUTDOWN_WORKAROUD
            val &= ~(REG_CRYSTAL_SEL_LV);
#else
            val &= ~(REG_PU_OSC | REG_CRYSTAL_SEL_LV);
#endif
            analog_write(ANA_REG_84, val);
        }
    }
}

void analog_aud_pll_open(enum ANA_AUD_PLL_USER_T user)
{
    if (user >= ANA_AUD_PLL_USER_END) {
        return;
    }

#ifdef __AUDIO_RESAMPLE__
    if (user == ANA_AUD_PLL_USER_CODEC &&
            hal_cmu_get_audio_resample_status()) {

        analog_aud_osc_clk_enable(ANA_OSC_CLK_USER_CODEC, true);
        return;
    }
#endif

    if (ana_aud_pll_map == 0) {
        bbpll_codec_clock_enable(true);
        hal_cmu_pll_enable(HAL_CMU_PLL_BB, HAL_CMU_PLL_USER_AUD);
    }
    ana_aud_pll_map |= user;
}

void analog_aud_pll_close(enum ANA_AUD_PLL_USER_T user)
{
    if (user >= ANA_AUD_PLL_USER_END) {
        return;
    }

#ifdef __AUDIO_RESAMPLE__
    if (user == ANA_AUD_PLL_USER_CODEC &&
            hal_cmu_get_audio_resample_status()) {

        analog_aud_osc_clk_enable(ANA_OSC_CLK_USER_CODEC, false);
        return;
    }
#endif

    ana_aud_pll_map &= ~user;
    if (ana_aud_pll_map == 0) {
        bbpll_codec_clock_enable(false);
        hal_cmu_pll_disable(HAL_CMU_PLL_BB, HAL_CMU_PLL_USER_AUD);
    }
}

static void analog_aud_enable_bbpll_common(enum ANA_CODEC_USER_T user, bool en)
{
    uint32_t lock;
    bool set = false;
    uint16_t val;

    lock = int_lock();
    if (en) {
        if (bbpll_common_map == 0) {
            set = true;
        }
        bbpll_common_map |= user;
    } else {
        bbpll_common_map &= ~user;
        if (bbpll_common_map == 0) {
            set = true;
        }
    }
    int_unlock(lock);

    if (set) {
        analog_read(ANA_REG_85, &val);
        if (bbpll_common_map) {
            val |= CODEC_BBPLL_ADC_EN | CODEC_BBPLL_DAC_EN;
        } else {
            val &= ~(CODEC_BBPLL_ADC_EN | CODEC_BBPLL_DAC_EN);
        }
        analog_write(ANA_REG_85, val);
    }
}

static void analog_aud_enable_dac_with_classab(uint32_t dac, bool switch_pa)
{
    uint16_t val_41;
    uint16_t val_10d;
    uint16_t val_10e;
    uint16_t val_112;
    uint16_t val_115;
    uint16_t val_116;
    uint16_t val_11e;

    analog_read(ANA_REG_41, &val_41);
    analog_read(ANA_REG_10D, &val_10d);
    analog_read(ANA_REG_10E, &val_10e);
    analog_read(ANA_REG_112, &val_112);
    analog_read(ANA_REG_115, &val_115);
    analog_read(ANA_REG_116, &val_116);
    analog_read(ANA_REG_11E, &val_11e);

    if (dac & (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)) {
        val_10e |= REG_CODEC_TX_EAR_DR_EN | REG_CODEC_TX_EAR_ENBIAS | REG_CODEC_TX_EAR_LPBIAS;
        analog_write(ANA_REG_10E, val_10e);
        osDelay(1);
        val_116 |= CFG_TX_TREE_EN;
        analog_write(ANA_REG_116, val_116);
        osDelay(1);

        if (dac & AUD_CHANNEL_MAP_CH0) {
            val_115 |= REG_CODEC_TX_EN_EARPA_L | REG_CODEC_TX_EN_LCLK;
            if (!switch_pa) {
                val_41 |= REG_CODEC_TX_EN_LDAC;
                val_11e |= DIG_CODEC_TX_EN_LDAC_ANA;
            }
        }

        val_10d |= REG_CODEC_TX_PWR_SW;
        analog_write(ANA_REG_10D, val_10d);
        analog_write(ANA_REG_41, val_41);
        analog_write(ANA_REG_115, val_115);
        val_112 |= REG_CODEC_TX_EN_DACLDO;
        analog_write(ANA_REG_112, val_112);
        val_116 |= REG_CODEC_TX_EN_LPPA;
        analog_write(ANA_REG_116, val_116);
        osDelay(1);
        val_115 |= REG_CODEC_TX_EN_S1PA;
        analog_write(ANA_REG_115, val_115);
        analog_write(ANA_REG_11E, val_11e);
        analog_aud_enable_bbpll_common(ANA_CODEC_USER_DAC, true);
        // Ensure 1ms delay before enabling dac_pa
        osDelay(1);
    } else {
        // Ensure 1ms delay after disabling dac_pa
        osDelay(1);
        val_115 &= ~REG_CODEC_TX_EN_S1PA;
        analog_write(ANA_REG_115, val_115);
        osDelay(1);

        val_115 &= ~(REG_CODEC_TX_EN_EARPA_L | REG_CODEC_TX_EN_LCLK);
        if (!switch_pa) {
            val_41 &= ~REG_CODEC_TX_EN_LDAC;
            val_11e &= ~DIG_CODEC_TX_EN_LDAC_ANA;
        }

        analog_write(ANA_REG_41, val_41);
        val_10d &= ~REG_CODEC_TX_PWR_SW;
        analog_write(ANA_REG_10D, val_10d);
        analog_write(ANA_REG_115, val_115);
        analog_write(ANA_REG_11E, val_11e);
        val_112 &= ~REG_CODEC_TX_EN_DACLDO;
        analog_write(ANA_REG_112, val_112);
        val_116 &= ~REG_CODEC_TX_EN_LPPA;
        analog_write(ANA_REG_116, val_116);
        osDelay(1);

        val_116 &= ~CFG_TX_TREE_EN;
        analog_write(ANA_REG_116, val_116);
        osDelay(1);

        val_10e &= ~(REG_CODEC_TX_EAR_DR_EN | REG_CODEC_TX_EAR_ENBIAS | REG_CODEC_TX_EAR_LPBIAS);
        analog_write(ANA_REG_10E, val_10e);
        analog_aud_enable_bbpll_common(ANA_CODEC_USER_DAC, false);
    }
}

static void analog_aud_enable_dac_pa_classab(uint32_t dac)
{
    uint16_t val_115;

    analog_read(ANA_REG_115, &val_115);

    if (dac & (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)) {
        val_115 |= REG_CODEC_TX_EN_S4PA;
    } else {
        val_115 &= ~REG_CODEC_TX_EN_S4PA;
    }
    analog_write(ANA_REG_115, val_115);
}

static void analog_aud_enable_dac(uint32_t dac)
{
    analog_aud_enable_dac_with_classab(dac, false);
}

static void analog_aud_enable_dac_pa_internal(uint32_t dac)
{
    analog_aud_enable_dac_pa_classab(dac);
}

static void analog_aud_enable_dac_pa(uint32_t dac)
{
    if (dac & (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)) {
        analog_aud_enable_dac_pa_internal(dac);

#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
        hal_codec_dac_sdm_reset_clear();
#endif
    } else {
#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
        hal_codec_dac_sdm_reset_set();
#endif

        analog_aud_enable_dac_pa_internal(dac);
    }
}

static void analog_aud_enable_codec_vcm_buffer(bool en)
{
    uint16_t val;

    analog_read(ANA_REG_82, &val);
    if (en) {
        val |= REG_CODEC_EN_VCM_BUFFER;
    } else {
        val &= ~REG_CODEC_EN_VCM_BUFFER;
    }
    analog_write(ANA_REG_82, val);
}

void analog_aud_codec_enable_capsensor_rc(bool en)
{
    uint16_t val;

    analog_read(ANA_REG_5A, &val);
    if (en) {
        val |= (REG_CLK_CAPSENSOR_GATE_EN | REG_CLK_32K_CAPSENSOR_GATE_EN | SOFT_RESETN_CAPSENSOR | SOFT_RESETN_CAPSENSOR_REG);
        val = SET_BITFIELD(val, REG_CFG_CAPSENSOR_DIV, 0x8);
    } else {
        val &= ~(REG_CLK_CAPSENSOR_GATE_EN | REG_CLK_32K_CAPSENSOR_GATE_EN | SOFT_RESETN_CAPSENSOR | SOFT_RESETN_CAPSENSOR_REG);
        val = SET_BITFIELD(val, REG_CFG_CAPSENSOR_DIV, 0x0);
    }
    analog_write(ANA_REG_5A, val);
}

void analog_aud_codec_enable_capsensor_osc(bool en)
{
    uint16_t val;

    analog_read(ANA_REG_5A, &val);
    if (en) {
        val |= (REG_CLK_CAPSENSOR_GATE_EN | REG_CLK_32K_CAPSENSOR_GATE_EN | SOFT_RESETN_CAPSENSOR | SOFT_RESETN_CAPSENSOR_REG | REG_SEL_CAPSENSOR_OSC);
        val = SET_BITFIELD(val, REG_CFG_CAPSENSOR_DIV, 0x27);
    } else {
        val &= ~(REG_CLK_CAPSENSOR_GATE_EN | REG_CLK_32K_CAPSENSOR_GATE_EN | SOFT_RESETN_CAPSENSOR | SOFT_RESETN_CAPSENSOR_REG | REG_SEL_CAPSENSOR_OSC);
        val = SET_BITFIELD(val, REG_CFG_CAPSENSOR_DIV, 0x0);
    }
    analog_write(ANA_REG_5A, val);
}

static void analog_aud_enable_codec_bias_lp(bool en)
{
#ifndef RC_CLK_ENABLE
    uint16_t val;

    analog_read(ANA_REG_80, &val);
    if (en) {
        val |= REG_CODEC_EN_BIAS_LP;
    } else {
        val &= ~REG_CODEC_EN_BIAS_LP;
    }
    analog_write(ANA_REG_80, val);
#endif
}

#if defined(VOICE_DETECTOR_EN) && defined(VAD_USE_SAR_ADC)
#define SAR_ADC_CLK_DIV (512*3-2)
static enum ANA_CODEC_USER_T sar_adc_map = 0;
static void analog_aud_enable_sar_adc(enum ANA_CODEC_USER_T user, bool en)
{
    uint16_t val;
    enum ANA_CODEC_USER_T old_map;
    bool set = false;

    if (en) {
        if (sar_adc_map == 0) {
            set = true;
        }
        sar_adc_map |= user;
    } else {
        old_map = sar_adc_map;
        sar_adc_map &= ~user;
        if (old_map != 0 && sar_adc_map == 0) {
            set = true;
        }
    }

    if (set) {
        if (sar_adc_map) {
            // enable clock
            val = REG_SAR_CLK_EN | REG_SAR_EN_PREAMP | REG_SAR_DELAY_BIT(1);
            analog_write(ANA_REG_266, val);

            val = REG_SAR_PU_START_DLY_CNT(0x32) | REG_SAR_PU_VREF_IN;
            analog_write(ANA_REG_269, val); val = REG_CLK_SAR_MUX; //[15]: 1=osc clk, 0=rc clk

            val |= REG_SAR_CALI_LSB_SCRN | REG_SAR_CALI_CNT(2) | REG_SAR_INIT_CALI_BIT(9);
            analog_write(ANA_REG_261, val);

            val = REG_SAR_CLOSE_ON_DET | REG_SAR_TO_SIGN | SEL_SARCLK_DIV2;
            analog_write(ANA_REG_2AF, val);

            //sample rate=24M/SAR_CLK_OUT_DIV = 16K
            val = REG_SAR_CLK_OUT_DIV(SAR_ADC_CLK_DIV);
            analog_write(ANA_REG_268, val);

            val = REG_SAR_ADC_ON | REG_SAR_RESULT_SEL | REG_SAR_CLK_INT_DIV(0x16) | REG_SAR_OFFSET_CALI_CNT(2);
            analog_write(ANA_REG_265, val);
        } else {
            analog_read(ANA_REG_265, &val);
            val &= ~REG_SAR_ADC_ON;
            analog_write(ANA_REG_265, val);

            analog_read(ANA_REG_266, &val);
            val &= ~REG_SAR_CLK_EN;
            analog_write(ANA_REG_266, val);
        }
        analog_aud_enable_codec_bias_lp(!!sar_adc_map);
    }
}
#endif

static void analog_aud_enable_adc(enum ANA_CODEC_USER_T user, enum AUD_CHANNEL_MAP_T ch_map, bool en)
{
    int i;
    uint16_t val;
    enum ANA_CODEC_USER_T old_map;
    bool set;
#if !defined(CODEC_TX_PEAK_DETECT)
    bool global_update = false;
#endif

    ANALOG_DEBUG_TRACE(3,"[%s] user=%d ch_map=0x%x", __func__, user, ch_map);

#if defined(VOICE_DETECTOR_EN) && defined(VAD_USE_SAR_ADC)
    if (ch_map == AUD_CHANNEL_MAP_CH5) {
        analog_aud_enable_sar_adc(user, true);
        return;
    }
#endif

    for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
        if (ch_map & (AUD_CHANNEL_MAP_CH0 << i)) {
            set = false;
            if (en) {
                if (adc_map[i] == 0) {
                    set = true;
                }
                adc_map[i] |= user;
            } else {
                old_map = adc_map[i];
                adc_map[i] &= ~user;
                if (old_map != 0 && adc_map[i] == 0) {
                    set = true;
                }
            }
            if (set) {
#if !defined(CODEC_TX_PEAK_DETECT)
                if (!global_update) {
                    global_update = true;
                    if (en) {
                        analog_aud_enable_codec_vcm_buffer(true);
                        analog_aud_enable_bbpll_common(ANA_CODEC_USER_ADC, true);
                    }
                }
#endif
                if (i == 4) {
#ifdef VOICE_DETECTOR_EN
                    if (!vad_started)
#endif
                    {
                        analog_aud_enable_codec_bias_lp(!!adc_map[i]);
                    }
                }
                analog_read(ANA_REG_80, &val);
                if (adc_map[i]) {
                    val |= (REG_CODEC_EN_ADCA << i);
                } else {
                    val &= ~(REG_CODEC_EN_ADCA << i);
                }
                analog_write(ANA_REG_80, val);
            }
        }
    }

#if !defined(CODEC_TX_PEAK_DETECT)
    if (global_update && !en) {
        for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
            if (adc_map[i]) {
                break;
            }
        }
        if (i >= MAX_ANA_MIC_CH_NUM) {
            analog_aud_enable_codec_vcm_buffer(false);
            analog_aud_enable_bbpll_common(ANA_CODEC_USER_ADC, false);
        }
    }
#endif
}

static uint32_t db_to_adc_gain(int db)
{
    int i;
    uint8_t cnt;
    const int8_t *list;

    list = adc_db;
    cnt = ARRAY_SIZE(adc_db);

    for (i = 0; i < cnt - 1; i++) {
        if (db < list[i + 1]) {
            break;
        }
    }

    if (i == cnt - 1) {
        return i;
    }
    else if (db * 2 < list[i] + list[i + 1]) {
        return i;
    } else {
        return i + 1;
    }
}

static int8_t get_chan_adc_gain(uint32_t i)
{
    int8_t gain;

#ifdef DYN_ADC_GAIN
    gain = dyn_adc_gain_db[i];
#else
    gain = tgt_adc_db[i];
#endif

    return gain;
}

static void analog_aud_set_adc_gain(enum AUD_IO_PATH_T input_path, enum AUD_CHANNEL_MAP_T ch_map)
{
    int i;
    int gain;
    uint16_t gain_val;
    uint16_t val;

#if defined(VOICE_DETECTOR_EN) && defined(VAD_USE_SAR_ADC)
    if (ch_map == AUD_CHANNEL_MAP_CH5) {
        //NO any gain need to be set for SAR ADC
        return;
    }
#endif

    for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
        if (ch_map & (AUD_CHANNEL_MAP_CH0 << i)) {
            if (0) {
#ifdef ANC_APP
#ifdef ANC_FF_ENABLED
            } else if ((ANC_FF_MIC_CH_L | ANC_FF_MIC_CH_R) & (AUD_CHANNEL_MAP_CH0 << i)) {
                gain = get_chan_adc_gain(i);
#endif
#ifdef ANC_FB_ENABLED
            } else if ((ANC_FB_MIC_CH_L | ANC_FB_MIC_CH_R) & (AUD_CHANNEL_MAP_CH0 << i)) {
                gain = get_chan_adc_gain(i);
#endif
#endif
            } else if (input_path == AUD_INPUT_PATH_LINEIN) {
                gain = LINEIN_ADC_GAIN_DB;
            } else {
                gain = get_chan_adc_gain(i);
            }
#ifdef LOW_POWER_ADC
            if (!anc_enabled) {
                gain += low_bias_offset;
            }
#endif
            gain_val = db_to_adc_gain(gain);
            analog_read(ANA_REG_01 + 0x10 * i, &val);
            val = SET_BITFIELD(val, REG_CODEC_ADCA_GAIN_BIT, gain_val);
            analog_write(ANA_REG_01 + 0x10 * i, val);
        }
    }
}

void analog_aud_set_adc_gain_direct(enum AUD_CHANNEL_MAP_T ch_map, int gain)
{
    int i;
    uint16_t gain_val;
    uint16_t val;

    for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
        if (ch_map & (AUD_CHANNEL_MAP_CH0 << i)) {
            gain_val = db_to_adc_gain(gain);
            analog_read(ANA_REG_01 + 0x10 * i, &val);
            val = SET_BITFIELD(val, REG_CODEC_ADCA_GAIN_BIT, gain_val);
            analog_write(ANA_REG_01 + 0x10 * i, val);
        }
    }
}

#ifdef ANC_APP
static void _update_anc_dyn_adc_gain(bool set, enum ANC_TYPE_T type, int16_t offset_l, int16_t offset_r)
{
    enum ANC_TYPE_T single_type;
    enum AUD_CHANNEL_MAP_T ch_map;
    uint32_t l, r;
    int8_t org_l, adj_l;
    int8_t org_r, adj_r;
    int8_t gain_l, gain_r;

    if (set) {
        // qdb to db
        gain_l = DEFAULT_ANC_FF_ADC_GAIN_DB + offset_l / 4;
        gain_r = DEFAULT_ANC_FF_ADC_GAIN_DB + offset_r / 4;
    } else {
        gain_l = gain_r = 0;
    }

    while (type) {
        l = get_msb_pos(type);
        single_type = (1 << l);
        type &= ~single_type;

        ch_map = 0;
        l = r = 32;
        if (0) {
#if defined(ANC_FF_MIC_CH_L) && defined(ANC_FF_MIC_CH_R)
        } else if (single_type == ANC_FEEDFORWARD) {
            if (ANC_FF_MIC_CH_L) {
                ch_map |= ANC_FF_MIC_CH_L;
                l = get_msb_pos(ANC_FF_MIC_CH_L);
            }
            if (ANC_FF_MIC_CH_R) {
                ch_map |= ANC_FF_MIC_CH_R;
                r = get_msb_pos(ANC_FF_MIC_CH_R);
            }
#endif

#if defined(ANC_FB_MIC_CH_L) && defined(ANC_FB_MIC_CH_R)
        } else if (single_type == ANC_FEEDBACK) {
            if (ANC_FB_MIC_CH_L) {
                ch_map |= ANC_FB_MIC_CH_L;
                l = get_msb_pos(ANC_FB_MIC_CH_L);
            }
            if (ANC_FB_MIC_CH_R) {
                ch_map |= ANC_FB_MIC_CH_R;
                r = get_msb_pos(ANC_FB_MIC_CH_R);
            }
#endif

#if defined(ANC_TT_MIC_CH_L) && defined(ANC_TT_MIC_CH_R)
        } else if (single_type == ANC_TALKTHRU) {
            if (ANC_TT_MIC_CH_L) {
                ch_map |= ANC_TT_MIC_CH_L;
                l = get_msb_pos(ANC_TT_MIC_CH_L);
            }
            if (ANC_TT_MIC_CH_R) {
                ch_map |= ANC_TT_MIC_CH_R;
                r = get_msb_pos(ANC_TT_MIC_CH_R);
            }
#if defined(ANC_FF_MIC_CH_L) && defined(ANC_FF_MIC_CH_R)
            if (set) {
                anc_tt_gain_db_l = gain_l;
                anc_tt_gain_db_r = gain_r;
            }
            if (adda_common_map & ANA_CODEC_USER_ANC_FF) {
                if (ANC_TT_MIC_CH_L & (ANC_FF_MIC_CH_L)) {
                    ch_map &= ~ANC_TT_MIC_CH_L;
                    l = 32;
                }
                if (ANC_TT_MIC_CH_R & (ANC_FF_MIC_CH_R)) {
                    ch_map &= ~ANC_TT_MIC_CH_R;
                    r = 32;
                }
            }
#endif
#endif
        } else {
            continue;
        }

        if (set) {
            ANALOG_INFO_TRACE(0, "ana: set anc adc gain: type=%d", single_type);
        } else {
            if (l >= MAX_ANA_MIC_CH_NUM && r >= MAX_ANA_MIC_CH_NUM) {
                continue;
            }
            if (l < MAX_ANA_MIC_CH_NUM) {
                gain_l = tgt_adc_db[l];
#if defined(ANC_TT_MIC_CH_L)
#if defined(ANC_FF_MIC_CH_L)
                if ((single_type & ANC_FEEDFORWARD) && (adda_common_map & ANA_CODEC_USER_ANC_TT) &&
                        (ANC_TT_MIC_CH_L & (ANC_FF_MIC_CH_L))) {
                    gain_l = anc_tt_gain_db_l;
                }
#endif
#endif
            }
            if (r < MAX_ANA_MIC_CH_NUM) {
                gain_r = tgt_adc_db[r];
#if defined(ANC_TT_MIC_CH_R)
#if defined(ANC_FF_MIC_CH_R)
                if ((single_type & ANC_FEEDFORWARD) && (adda_common_map & ANA_CODEC_USER_ANC_TT) &&
                        (ANC_TT_MIC_CH_R & (ANC_FF_MIC_CH_R))) {
                    gain_r = anc_tt_gain_db_r;
                }
#endif
#endif
            }
            ANALOG_INFO_TRACE(0, "ana: clear anc adc gain: type=%d", single_type);
        }

        if ((l >= MAX_ANA_MIC_CH_NUM || dyn_adc_gain_db[l] == gain_l) &&
                (r >= MAX_ANA_MIC_CH_NUM || dyn_adc_gain_db[r] == gain_r)) {
            continue;
        }
        if (l < MAX_ANA_MIC_CH_NUM) {
            dyn_adc_gain_db[l] = gain_l;
        }
        if (r < MAX_ANA_MIC_CH_NUM) {
            dyn_adc_gain_db[r] = gain_r;
        }
        ANALOG_INFO_TRACE(0, "ana: update anc adc gain: type=%d gain=%d/%d", single_type, gain_l, gain_r);

        org_l = adj_l = 0;
        if (l < MAX_ANA_MIC_CH_NUM) {
            if (dyn_adc_gain_db[l] != tgt_adc_db[l]) {
                org_l = adc_db[db_to_adc_gain(tgt_adc_db[l])];
                adj_l = adc_db[db_to_adc_gain(dyn_adc_gain_db[l])];
            }
        }

        org_r = adj_r = 0;
        if (r < MAX_ANA_MIC_CH_NUM) {
            if (dyn_adc_gain_db[r] != tgt_adc_db[r]) {
                org_r = adc_db[db_to_adc_gain(tgt_adc_db[r])];
                adj_r = adc_db[db_to_adc_gain(dyn_adc_gain_db[r])];
            }
        }

        hal_codec_apply_anc_adc_gain_offset(single_type, (org_l - adj_l), (org_r - adj_r));
        analog_aud_set_adc_gain(AUD_INPUT_PATH_MAINMIC, ch_map);
    }
}

void analog_aud_apply_anc_adc_gain_offset(enum ANC_TYPE_T type, int16_t offset_l, int16_t offset_r)
{
    _update_anc_dyn_adc_gain(true, type, offset_l, offset_r);
}

void analog_aud_restore_anc_dyn_adc_gain(enum ANC_TYPE_T type)
{
    _update_anc_dyn_adc_gain(false, type, 0, 0);
}
#endif

#ifdef DYN_ADC_GAIN
void analog_aud_apply_dyn_adc_gain(enum AUD_CHANNEL_MAP_T ch_map, int16_t gain)
{
    enum AUD_CHANNEL_MAP_T map;
    int i;

#ifdef ANC_APP
#ifdef ANC_FF_ENABLED
    ch_map &= ~(ANC_FF_MIC_CH_L | ANC_FF_MIC_CH_R);
#endif
#ifdef ANC_FB_ENABLED
    ch_map &= ~(ANC_FB_MIC_CH_L | ANC_FB_MIC_CH_R);
#endif
#if defined(ANC_TT_MIC_CH_L)
    ch_map &= ~ANC_TT_MIC_CH_L;
#endif
#if defined(ANC_TT_MIC_CH_R)
    ch_map &= ~ANC_TT_MIC_CH_R;
#endif
#endif

    if (ch_map) {
        map = ch_map;

        while (map) {
            i = get_msb_pos(map);
            map &= ~(1 << i);
            if (i < MAX_ANA_MIC_CH_NUM) {
                dyn_adc_gain_db[i] = gain;
            }
        }

        ANALOG_INFO_TRACE(2,"ana: apply adc gain: ch_map=0x%X gain=%d", ch_map, gain);

        analog_aud_set_adc_gain(AUD_INPUT_PATH_MAINMIC, ch_map);
    }
}
#endif

void analog_aud_set_dac_gain(int32_t v)
{
}

uint32_t analog_codec_get_dac_gain(void)
{
    return 0;
}

uint32_t analog_codec_dac_gain_to_db(int32_t gain)
{
    return 0;
}

int32_t analog_codec_dac_max_attn_db(void)
{
    return 0;
}

static int POSSIBLY_UNUSED dc_calib_checksum_valid(uint32_t efuse)
{
    int i;
    uint32_t cnt = 0;
    uint32_t chksum_mask = (1 << (16 - DAC_DC_CALIB_BIT_WIDTH)) - 1;

    for (i = 0; i < DAC_DC_CALIB_BIT_WIDTH; i++) {
        if (efuse & (1 << i)) {
            cnt++;
        }
    }

    return (((~cnt) & chksum_mask) == ((efuse >> DAC_DC_CALIB_BIT_WIDTH) & chksum_mask));
}

static int16_t dc_calib_val_decode(int16_t val)
{
#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
    uint32_t sign_bit = (1 << (DAC_DC_CALIB_BIT_WIDTH - 1));
    uint32_t num_mask = sign_bit - 1;

    if (val & sign_bit) {
        val = -(val & num_mask);
    }
#endif
    return val;
}

static void analog_aud_get_dc_calib_value_ext(bool classd, int16_t *dc_l, int16_t *dc_r)
{
    static const uint8_t EFUSE_PAGE_CLASSAB_1P8V = PMU_EFUSE_PAGE_DCCALIB2_L;
    static const uint8_t EFUSE_PAGE_CLASSAB_1P7V = PMU_EFUSE_PAGE_DCCALIB2_L_LP;
    static const uint8_t EFUSE_PAGE_CLASSD_1P8V = PMU_EFUSE_PAGE_DCCALIB_L;
    static const uint8_t EFUSE_PAGE_CLASSD_1P7V = PMU_EFUSE_PAGE_DCCALIB_L_LP;
    uint8_t page;
    uint16_t efuse;

    union DC_EFUSE_T {
        struct DC_VALUE_T {
            int16_t dc          : DAC_DC_CALIB_BIT_WIDTH;
            uint16_t checksum   : (16 - DAC_DC_CALIB_BIT_WIDTH);
        } val;
        uint16_t reg;
    };
    union DC_EFUSE_T dc;

    if (vcodec_mv >= 1800) {
        page = classd ? EFUSE_PAGE_CLASSD_1P8V : EFUSE_PAGE_CLASSAB_1P8V;
    } else {
        page = classd ? EFUSE_PAGE_CLASSD_1P7V : EFUSE_PAGE_CLASSAB_1P7V;
    }

    pmu_get_efuse(page, &efuse);
    if (dc_calib_checksum_valid(efuse)) {
        ANALOG_INFO_TRACE(1,"Dc calib L OK: 0x%04x", efuse);
        dc.reg = efuse;
        *dc_l = dc.val.dc;
    } else {
        ANALOG_INFO_TRACE(1,"Warning: Bad dc calib efuse L: 0x%04x", efuse);
        *dc_l = 0;
    }

    *dc_r = 0;

    ANALOG_INFO_TRACE(2,"ANA: DC CALIB L=0x%04hX/%d", *dc_l, dc_calib_val_decode(*dc_l));

#if defined(ANA_DC_CALIB_L) || defined(ANA_DC_CALIB_R)
#ifdef ANA_DC_CALIB_L
    *dc_l = ANA_DC_CALIB_L;
#endif
    ANALOG_INFO_TRACE(2,"ANA: OVERRIDE DC CALIB L=0x%04hX/%d", *dc_l, dc_calib_val_decode(*dc_l));
#endif

    return;
}

void analog_aud_get_dc_calib_value(int16_t *dc_l, int16_t *dc_r)
{
    bool classd = false;
    analog_aud_get_dc_calib_value_ext(classd, dc_l, dc_r);
}

bool analog_aud_dc_calib_valid(void)
{
    return false;
}

uint16_t analog_aud_dac_dc_diff_to_val(int32_t diff)
{
    uint16_t val;

    // BIT 13: SIGN
    // BIT 12: x256
    // BIT 11: x128
    // BIT 10: x64
    // BIT  9: x64
    // BIT  8: x32
    // BIT  7: x16
    // BIT  6: x8
    // BIT  5: x8
    // BIT  4: x4
    // BIT  3: x2
    // BIT  2: x2
    // BIT  1: x1
    // BIT  1: x1

    val = 0;
    if (diff < 0) {
        val |= (1 << 13);
        diff = -diff;
    }
    if (diff & (1 << 8)) {
        val |= (1 << 12);
    }
    if (diff & (1 << 7)) {
        val |= (1 << 11);
    }
    if (diff & (1 << 6)) {
        val |= (1 << 9);
    }
    if (diff & (1 << 5)) {
        val |= (1 << 8);
    }
    if (diff & (1 << 4)) {
        val |= (1 << 7);
    }
    if (diff & (1 << 3)) {
        val |= (1 << 5);
    }
    if (diff & (1 << 2)) {
        val |= (1 << 4);
    }
    if (diff & (1 << 1)) {
        val |= (1 << 2);
    }
    if (diff & (1 << 0)) {
        val |= (1 << 0);
    }

    return val;
}

uint16_t analog_aud_dc_calib_val_to_efuse(uint16_t val)
{
    int i;
    uint32_t cnt = 0;
    uint32_t chksum_mask = (1 << (16 - DAC_DC_CALIB_BIT_WIDTH)) - 1;
    uint32_t val_mask = (1 << DAC_DC_CALIB_BIT_WIDTH) - 1;

    for (i = 0; i < DAC_DC_CALIB_BIT_WIDTH; i++) {
        if (val & (1 << i)) {
            cnt++;
        }
    }

    return (((~cnt) & chksum_mask) << DAC_DC_CALIB_BIT_WIDTH) | (val & val_mask);
}

int16_t analog_aud_dac_dc_get_step(void)
{
    return DAC_DC_ADJUST_STEP;
}

void analog_aud_save_dc_calib(uint16_t val)
{
}

void analog_aud_dc_calib_set_value(uint16_t dc_l, uint16_t dc_r)
{
    uint16_t val;

    analog_read(ANA_REG_112, &val);
    val = SET_BITFIELD(val, REG_CODEC_TX_EAR_OFF_BITL, dc_l);
    analog_write(ANA_REG_112, val);
}

void analog_aud_dc_calib_get_cur_value(uint16_t *dc_l, uint16_t *dc_r)
{
    uint16_t val;

    if (dc_l) {
        analog_read(ANA_REG_112, &val);
        *dc_l = GET_BITFIELD(val, REG_CODEC_TX_EAR_OFF_BITL);
    }
    if (dc_r) {
        *dc_r = 0;
    }
}

bool analog_aud_dc_calib_get_large_ana_dc_value(int16_t *ana_dc, int cur_dig_dc, int tgt_dig_dc,
    int chan, bool init)
{
#define DAC_LARGE_ANA_DC_WEIGHT_INIT_R (0x0)
#define DAC_LARGE_ANA_DC_WEIGHT_INIT_L (0x20)  //DC=230uv
#define DAC_LARGE_ANA_DC_WEIGHT_STEP   (0x10)  //DC=100uv
#define DAC_DIG_DC_THRES_MIN    (3000)

    bool success = false;
    int16_t dc;
    int comp_dig_dc;
    if (!ana_dc) {
        return success;
    }
    comp_dig_dc = tgt_dig_dc - cur_dig_dc;
    if (init) {
        if (chan == 0) {
            *ana_dc = DAC_LARGE_ANA_DC_WEIGHT_INIT_L;
        } else {
            *ana_dc = DAC_LARGE_ANA_DC_WEIGHT_INIT_R;
        }

        dc = ABS(*ana_dc);
        if (comp_dig_dc > 0) {
            dc = -dc;
        }
        *ana_dc = dc;
    } else {
        if (ABS(comp_dig_dc) >= DAC_DIG_DC_THRES_MIN) {
            success = true;
        } else {
            dc = ABS(*ana_dc);
            dc += DAC_LARGE_ANA_DC_WEIGHT_STEP;
            if (comp_dig_dc > 0) {
                dc = -dc;
            }
            *ana_dc = dc;
        }
    }
    return success;
}

#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
static void analog_aud_dc_calib_init(void)
{
    uint16_t val;
    int16_t dc_l, dc_r;

    analog_aud_get_dc_calib_value_ext(false, &dc_l, &dc_r);

    analog_read(ANA_REG_112, &val);
    val = SET_BITFIELD(val, REG_CODEC_TX_EAR_OFF_BITL, dc_l);
    analog_write(ANA_REG_112, val);
}
#endif

void analog_aud_dc_calib_enable(bool en)
{
    uint16_t val;

    analog_read(ANA_REG_112, &val);
    if (en) {
        val |= REG_CODEC_TX_EAR_OFFEN;
    } else {
        val &= ~REG_CODEC_TX_EAR_OFFEN;
    }
    analog_write(ANA_REG_112, val);
}

static void analog_aud_dac_dc_backup_regs(bool save)
{
    uint32_t i;
    uint16_t regaddr[] = {
        ANA_REG_01,
        ANA_REG_06,
        ANA_REG_09};

    static uint16_t regval[ARRAY_SIZE(regaddr)] = {0};
    static bool regs_saved = false;

    if (save) {
        if (!regs_saved) {
            for(i = 0; i < ARRAY_SIZE(regaddr); i++) {
                analog_read(regaddr[i], &regval[i]);
            }
            regs_saved = true;
        }
    } else {
        if (regs_saved) {
            for(i = 0; i < ARRAY_SIZE(regaddr); i++) {
                analog_write(regaddr[i], regval[i]);
            }
            regs_saved = false;
        }
    }
}

#define analog_aud_dac_dc_save_regs()    analog_aud_dac_dc_backup_regs(true)
#define analog_aud_dac_dc_restore_regs() analog_aud_dac_dc_backup_regs(false)

void analog_aud_dac_dc_auto_calib_enable(void)
{
    uint16_t val;

    analog_aud_dac_dc_save_regs();

    analog_aud_enable_dac(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
    analog_aud_enable_dac_pa(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
    analog_aud_set_adc_gain_direct((AUD_CHANNEL_MAP_CH1 | AUD_CHANNEL_MAP_CH2), -3);
    analog_aud_enable_adc(ANA_CODEC_USER_ADC, (AUD_CHANNEL_MAP_CH1 | AUD_CHANNEL_MAP_CH2), false);
    analog_aud_enable_adc(ANA_CODEC_USER_ADC, (AUD_CHANNEL_MAP_CH1 | AUD_CHANNEL_MAP_CH2), true);

    // Force ADC precharge = 1
    analog_read(ANA_REG_01, &val);
    val |= REG_CODEC_ADCA_PRE_CHARGE;
    analog_write(ANA_REG_01, val);

    // precharge DR = 1
    analog_read(ANA_REG_01, &val);
    val |= CFG_PRE_CHARGE_ADCA_DR;
    analog_write(ANA_REG_01, val);
    osDelay(200);

    // precharge DR = 0
    analog_read(ANA_REG_01, &val);
    val &= ~CFG_PRE_CHARGE_ADCA_DR;
    analog_write(ANA_REG_01, val);

    // Force ADC precharge = 0
    analog_read(ANA_REG_01, &val);
    val &= ~REG_CODEC_ADCA_PRE_CHARGE;
    analog_write(ANA_REG_01, val);
}

void analog_aud_dac_dc_auto_calib_disable(void)
{
    analog_aud_dac_dc_auto_calib_set_mode(ANA_DAC_DC_CALIB_MODE_NORMAL);

    analog_aud_enable_adc(ANA_CODEC_USER_ADC, (AUD_CHANNEL_MAP_CH1 | AUD_CHANNEL_MAP_CH2), false);
    analog_aud_enable_dac_pa(0);
    analog_aud_enable_dac(0);

    analog_aud_dac_dc_restore_regs();
}

void analog_aud_dac_dc_auto_calib_set_mode(enum ANA_DAC_DC_CALIB_MODE_T mode)
{
    uint16_t val;

    analog_read(ANA_REG_06, &val);
    if (mode == ANA_DAC_DC_CALIB_MODE_ADC_ONLY) {
        val = SET_BITFIELD(val, REG_CODEC_ADCA_OFFSET_CAL_S, 0x06);
    } else if (mode == ANA_DAC_DC_CALIB_MODE_DAC_TO_ADC) {
        val = SET_BITFIELD(val, REG_CODEC_ADCA_OFFSET_CAL_S, 0x12);
    } else {
        val = SET_BITFIELD(val, REG_CODEC_ADCA_OFFSET_CAL_S, 0x12);
    }
    analog_write(ANA_REG_06, val);

    analog_read(ANA_REG_09, &val);
    val = SET_BITFIELD(val, REG_CODEC_ADCA_CH_SEL_INT, 4); // 0x0100:adc->dac_1
    analog_write(ANA_REG_09, val);
}

static void analog_aud_enable_common_internal(enum ANA_CODEC_USER_T user, bool en)
{
    uint32_t lock;
    uint16_t val_82;
    uint16_t val_83;
    bool set = false;

    lock = int_lock();
    if (en) {
        if (codec_common_map == 0) {
            set = true;
        }
        codec_common_map |= user;
    } else {
        codec_common_map &= ~user;
        if (codec_common_map == 0) {
            set = true;
        }
    }
    int_unlock(lock);

    if (set) {
        analog_read(ANA_REG_82, &val_82);
        analog_read(ANA_REG_83, &val_83);
        if (codec_common_map) {
            val_82 |= REG_CODEC_EN_VCM;
        } else {
            val_82 &= ~REG_CODEC_EN_VCM;
            val_83 &= ~REG_CODEC_VCM_EN_LPF;
        }
        if (codec_common_map) {
            // RTOS application startup time is long enougth for VCM charging
#if !(defined(VCM_ON) && defined(RTOS)) || defined(RC_CLK_ENABLE)
            // VCM fully stable time is about 60ms/1.95V or 150ms/1.7V
            // Quick startup:
            // 1) Disable VCM LPF and target to a higher voltage than the required one
            // 2) Wait for a short time when VCM is in quick charge (high voltage)
            // 3) Enable VCM LPF and target to the required VCM LPF voltage
            analog_write(ANA_REG_82, SET_BITFIELD(val_82, REG_CODEC_VCM_LOW_VCM, 0));
            uint32_t delay;

#if defined(VCM_CAP_100NF)
            if (vcodec_mv >= 1900) {
                delay = 6;
            } else {
                delay = 10;
            }
#else
            if (vcodec_mv >= 1900) {
                delay = 25;
            } else {
                delay = 100;
            }
#endif
            osDelay(delay);
#endif // !(VCM_ON && RTOS)
            val_83 |= REG_CODEC_VCM_EN_LPF;
#ifdef VCM_ALWAYS_LP
            val_82 |= REG_CODEC_LP_VCM;
#endif
        }
        analog_write(ANA_REG_82, val_82);
        analog_write(ANA_REG_83, val_83);
    }
}

static void analog_aud_enable_codec_common(enum ANA_CODEC_USER_T user, bool en)
{
#ifndef VCM_ON
    analog_aud_enable_common_internal(user, en);
#endif
}

static void analog_aud_enable_adda_common(enum ANA_CODEC_USER_T user, bool en)
{
    uint32_t lock;
    uint16_t val_80;
    bool set = false;

    lock = int_lock();
    if (en) {
        if (adda_common_map == 0) {
            set = true;
        }
        adda_common_map |= user;
    } else {
        adda_common_map &= ~user;
        if (adda_common_map == 0) {
            set = true;
        }
    }
    int_unlock(lock);

    if (set) {
        analog_read(ANA_REG_80, &val_80);
        if (adda_common_map) {
            val_80 |= REG_CODEC_EN_BIAS;
            // bypass or pu tx regulator
        } else {
            val_80 &= ~REG_CODEC_EN_BIAS;
        }
        analog_write(ANA_REG_80, val_80);

#if defined(CODEC_TX_PEAK_DETECT)
        analog_aud_enable_codec_vcm_buffer(!!adda_common_map);
#endif
    }
}

static void analog_aud_enable_vmic(enum ANA_CODEC_USER_T user, uint32_t dev, bool en)
{
    uint32_t lock;
    enum ANA_CODEC_USER_T old_map;
    bool set = false;
    bool global_update = false;
    int i;
    uint32_t pmu_map = 0;

    lock = int_lock();

    for (i = 0; i < MAX_VMIC_CH_NUM; i++) {
        if ((dev & (AUD_VMIC_MAP_VMIC1 << i)) == 0) {
            continue;
        }
        set = false;
        if (en) {
            if (vmic_map[i] == 0) {
                set = true;
            }
            vmic_map[i] |= user;
        } else {
            old_map = vmic_map[i];
            vmic_map[i] &= ~user;
            if (old_map != 0 && vmic_map[i] == 0) {
                set = true;
            }
        }

        if (set) {
            pmu_map |= (AUD_VMIC_MAP_VMIC1 << i);
            global_update = true;
        }
    }

    int_unlock(lock);

    if (global_update) {
        pmu_codec_mic_bias_enable(pmu_map, en);
#ifdef VOICE_DETECTOR_EN
        pmu_codec_mic_bias_lowpower_mode(pmu_map, en);
#endif
        if (pmu_map) {
            osDelay(1);
        }
    }
}

uint32_t analog_aud_get_max_dre_gain(void)
{
    return 0x10;
}

int analog_reset(void)
{
    return 0;
}

static void analog_low_power_adc_enable(bool en)
{
    uint16_t i;
    uint16_t val;

    if (en) {
        for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
            val = REG_CODEC_ADCA_CAP_BIT1(0x7F) | REG_CODEC_ADCA_CAP_BIT2(0x1F);
            analog_write(ANA_REG_02 + 0x10 * i, val);
            val = REG_CODEC_ADCA_CAP_BIT3(0x1F) | REG_CODEC_ADCA_REG_VSEL(3);
            analog_write(ANA_REG_03 + 0x10 * i, val);
            val = REG_CODEC_ADCA_IBSEL_OFFSET(8) | REG_CODEC_ADCA_IBSEL_OP1(0x4) | REG_CODEC_ADCA_IBSEL_OP2(8) | REG_CODEC_ADCA_IBSEL_OP3(8);
            analog_write(ANA_REG_04 + 0x10 * i, val);
            val = REG_CODEC_ADCA_IBSEL_OP4(8) | REG_CODEC_ADCA_IBSEL_REG(8) | REG_CODEC_ADCA_IBSEL_VCOMP(1) | REG_CODEC_ADCA_IBSEL_VREF(0xc);
            analog_write(ANA_REG_05 + 0x10 * i, val);
            val = REG_CODEC_ADCA_IBSEL_VREFBUF(4) | REG_CODEC_ADCA_IBSEL_IDAC2(8) | REG_CODEC_ADCA_OFFSET_CURRENT_SEL;
            analog_write(ANA_REG_06 + 0x10 * i, val);
            val = REG_CODEC_ADCA_PU_REG | REG_CODEC_ADCA_DVDD_SEL;
            analog_write(ANA_REG_07 + 0x10 * i, val);
            val = REG_CODEC_ADCA_VREF_SEL(3) | REG_CODEC_ADCA_VREFBUF_BIT(8) | ADCA_TIMER_RSTN_DLY(0x10);
            analog_write(ANA_REG_08 + 0x10 * i, val);
            val = ADCA_PRE_CHARGE_TIMER_DLY(0x14) | REG_CODEC_ADCA_CH_SEL_INT(2);//normal:0x0010 ; 0x0001 unused
            analog_write(ANA_REG_09 + 0x10 * i, val);
        }
    } else {
        for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
            val = REG_CODEC_ADCA_CAP_BIT1(0x7F) | REG_CODEC_ADCA_CAP_BIT2(0x1F);
            analog_write(ANA_REG_02 + 0x10 * i, val);
            val = REG_CODEC_ADCA_CAP_BIT3(0x1F) | REG_CODEC_ADCA_REG_VSEL(3);
            analog_write(ANA_REG_03 + 0x10 * i, val);
            val = REG_CODEC_ADCA_IBSEL_OFFSET(8) | REG_CODEC_ADCA_IBSEL_OP1(8) | REG_CODEC_ADCA_IBSEL_OP2(8) | REG_CODEC_ADCA_IBSEL_OP3(8);
            analog_write(ANA_REG_04 + 0x10 * i, val);
            val = REG_CODEC_ADCA_IBSEL_OP4(8) | REG_CODEC_ADCA_IBSEL_REG(8) | REG_CODEC_ADCA_IBSEL_VCOMP(1) | REG_CODEC_ADCA_IBSEL_VREF(0xc);
            analog_write(ANA_REG_05 + 0x10 * i, val);
            val = REG_CODEC_ADCA_IBSEL_VREFBUF(4) | REG_CODEC_ADCA_IBSEL_IDAC2(8) | REG_CODEC_ADCA_OFFSET_CURRENT_SEL;
            analog_write(ANA_REG_06 + 0x10 * i, val);
            val = REG_CODEC_ADCA_PU_REG | REG_CODEC_ADCA_DVDD_SEL;
            analog_write(ANA_REG_07 + 0x10 * i, val);
            val = REG_CODEC_ADCA_VREF_SEL(3) | REG_CODEC_ADCA_VREFBUF_BIT(8) | ADCA_TIMER_RSTN_DLY(0x10);
            analog_write(ANA_REG_08 + 0x10 * i, val);
            val = ADCA_PRE_CHARGE_TIMER_DLY(0x14) | REG_CODEC_ADCA_CH_SEL_INT(2) | REG_CODEC_ADCA_OP1_HP_MODE | REG_CODEC_ADCA_HP_MODE;
            analog_write(ANA_REG_09 + 0x10 * i, val);
        }
    }
}

void analog_open(void)
{
    uint16_t val;
    bool low_power_adc = false;

#ifdef DYN_ADC_GAIN
    memcpy(dyn_adc_gain_db, tgt_adc_db, sizeof(dyn_adc_gain_db));
#endif

#ifdef LOW_POWER_ADC
    low_power_adc = true;
#endif
    analog_low_power_adc_enable(low_power_adc);

    val = 0;
    analog_write(ANA_REG_50, val);

    analog_write(ANA_REG_80, val);

    val = REG_CODEC_BIAS_IBSEL(9) | REG_CODEC_BIAS_IBSEL_TX(4) | REG_CODEC_BIAS_IBSEL_VOICE(8);
    analog_write(ANA_REG_81, val);

    val = REG_CODEC_VCM_LOW_VCM(7) | REG_CODEC_VCM_LOW_VCM_LP(7) | REG_CODEC_VCM_LOW_VCM_LPF(7);
    analog_write(ANA_REG_82, val);

    val = REG_CODEC_BUF_LOWVCM(4);
    analog_write(ANA_REG_83, val);

    val = REG_TX_REGULATOR_BIT(8);
#ifdef CODEC_TX_PEAK_DETECT
    val |= REG_CODEC_TX_PEAK_NL_EN | REG_CODEC_TX_PEAK_PL_EN;
#endif
#ifdef AUDPLL_SHUTDOWN_WORKAROUD
    val |= REG_PU_OSC;
#endif
    val |= REG_EXTPLL_SEL;
    analog_write(ANA_REG_84, val);

    val = REG_CODEC_TX_EAR_DRE_GAIN_L(0x10) | REG_CODEC_TX_EAR_OCEN;
#ifdef DAC_DRE_GAIN_DC_UPDATE
    val |= REG_CODEC_TX_EAR_DRE_GAIN_L_UPDATE;
#endif
    analog_write(ANA_REG_10E, val);

    val = REG_CODEC_TX_DAC_VREF_L(5);
    analog_write(ANA_REG_10F, val);

    val = REG_CODEC_TX_EAR_COMP1(0xf1);
    analog_write(ANA_REG_110, val);

    val = REG_CODEC_TX_EAR_IBSEL(2);
    analog_write(ANA_REG_111, val);

    val = REG_CODEC_TX_EAR_SOFTSTART(8) | REG_CODEC_TX_EAR_OUTPUTSEL(1) |
        REG_CODEC_TX_EAR_DR_ST(2) | REG_OCP_SEL(3);
    analog_write(ANA_REG_114, val);

    val = REG_BYPASS_TX_REGULATOR | REG_CODEC_DAC_CLK_EDGE_SEL | REG_CODEC_TX_EAR_GAIN(1);
    analog_write(ANA_REG_115, val);

    // DAC dre gain select dig ctrl mode
    val = DRE_GAIN_SEL_L | REG_CODEC_TX_EAR_VCM(2);
    analog_write(ANA_REG_116, val);

    val = REG_CODEC_TX_CASN_L(2) | REG_CODEC_TX_CASP_L(2) | REG_CODEC_TX_IB_SEL_ST2(2);
    analog_write(ANA_REG_11B, val);

    val = REG_CODEC_TX_VREFBUF_CAS_L(1) | REG_CODEC_TX_VREFBUF_LOWGAIN(3) |
        REG_CODEC_TX_VREFBUF_CSEL(3);
    analog_write(ANA_REG_11C, val);

    val = REG_CODEC_TX_OUTPUT_L_LP_DR | REG_CODEC_TX_VCMO_SEL;
    analog_write(ANA_REG_11D, val);

#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
    analog_aud_dc_calib_init();
    analog_aud_dc_calib_enable(true);
#endif

#ifdef VCM_ON
    analog_aud_enable_common_internal(ANA_CODEC_USER_DAC, true);
#endif


    // bbpll medfreq calib, freq_med = ((44.1k + 48k) /2) *512 = 23577600
    //bbpll_auto_calib_spd(23577600, CODEC_PLL_DIV);
}

void analog_sleep(void)
{
    uint16_t val;

#ifndef VCM_ALWAYS_LP
#ifndef VCM_ON
    if (codec_common_map)
#endif
    {
        analog_read(ANA_REG_82, &val);
        val |= REG_CODEC_LP_VCM;
        analog_write(ANA_REG_82, val);
    }
#endif

#ifdef AUDPLL_SHUTDOWN_WORKAROUD
#ifdef VOICE_DETECTOR_EN
    if (!rpcsvr_osc_enable)
#endif
    {
        analog_read(ANA_REG_84, &val);
        val &= ~REG_PU_OSC;
        analog_write(ANA_REG_84, val);
    }
#endif
}

void analog_wakeup(void)
{
    uint16_t val;

#ifdef AUDPLL_SHUTDOWN_WORKAROUD
    analog_read(ANA_REG_84, &val);
    val |= REG_PU_OSC;
    analog_write(ANA_REG_84, val);
#endif

#ifndef VCM_ALWAYS_LP
#ifndef VCM_ON
    if (codec_common_map)
#endif
    {
        analog_read(ANA_REG_82, &val);
        val &= ~REG_CODEC_LP_VCM;
        analog_write(ANA_REG_82, val);
    }
#endif
}

void analog_aud_codec_anc_enable(enum ANC_TYPE_T type, bool en)
{
    enum ANA_CODEC_USER_T user;
    enum AUD_CHANNEL_MAP_T ch_map;

    user = 0;
    ch_map = 0;

#if defined(ANC_FF_MIC_CH_L) || defined(ANC_FF_MIC_CH_R)
    if (type & ANC_FEEDFORWARD) {
        user |= ANA_CODEC_USER_ANC_FF;
        ch_map |= ANC_FF_MIC_CH_L | ANC_FF_MIC_CH_R;
    }
#endif

#if defined(ANC_FB_MIC_CH_L) || defined(ANC_FB_MIC_CH_R)
    if (type & ANC_FEEDBACK) {
        user |= ANA_CODEC_USER_ANC_FB;
        ch_map |= ANC_FB_MIC_CH_L | ANC_FB_MIC_CH_R;
    }
#endif

#if defined(ANC_TT_MIC_CH_L) || defined(ANC_TT_MIC_CH_R)
    if (type & ANC_TALKTHRU) {
        user |= ANA_CODEC_USER_ANC_TT;
        ch_map |= ANC_TT_MIC_CH_L | ANC_TT_MIC_CH_R;
    }
#endif

    ANALOG_DEBUG_TRACE(0, "%s: type=%d en=%d ch_map=0x%x", __func__, type, en, ch_map);

    if (en) {
#ifdef LOW_POWER_ADC
        if ((adda_common_map & (ANA_CODEC_USER_ANC_FF |
                ANA_CODEC_USER_ANC_FB | ANA_CODEC_USER_ANC_TT)) == 0) {
            analog_low_power_adc_enable(false);
            anc_enabled = true;
            analog_aud_set_adc_gain(AUD_INPUT_PATH_MAINMIC, ((1 << MAX_ANA_MIC_CH_NUM) - 1));
        }
#endif
        analog_aud_enable_vmic(user, ANC_VMIC_CFG, true);
        analog_aud_enable_codec_common(user, true);
        analog_aud_enable_adda_common(user, true);
        analog_aud_set_adc_gain(AUD_INPUT_PATH_MAINMIC, ch_map);
        analog_aud_enable_adc(user, ch_map, true);
    } else {
        analog_aud_restore_anc_dyn_adc_gain(type);
        analog_aud_enable_adc(user, ch_map, false);
        analog_aud_enable_adda_common(user, false);
        analog_aud_enable_codec_common(user, false);
        analog_aud_enable_vmic(user, ANC_VMIC_CFG, false);
#ifdef LOW_POWER_ADC
        if ((adda_common_map & (ANA_CODEC_USER_ANC_FF |
                ANA_CODEC_USER_ANC_FB | ANA_CODEC_USER_ANC_TT)) == 0) {
            anc_enabled = false;
            analog_aud_set_adc_gain(AUD_INPUT_PATH_MAINMIC, ((1 << MAX_ANA_MIC_CH_NUM) - 1));
            analog_low_power_adc_enable(true);
        }
#endif
    }
}

void analog_aud_codec_anc_boost(bool en)
{
}

void analog_aud_mickey_enable(bool en)
{
    if (en) {
        analog_aud_enable_vmic(ANA_CODEC_USER_MICKEY, CFG_HW_AUD_MICKEY_DEV, true);
        analog_aud_enable_codec_common(ANA_CODEC_USER_MICKEY, true);
    } else {
        analog_aud_enable_codec_common(ANA_CODEC_USER_MICKEY, false);
        analog_aud_enable_vmic(ANA_CODEC_USER_MICKEY, CFG_HW_AUD_MICKEY_DEV, false);
    }
}

void analog_aud_codec_adc_enable(enum AUD_IO_PATH_T input_path, enum AUD_CHANNEL_MAP_T ch_map, bool en)
{
    uint32_t dev;

    dev = hal_codec_get_input_path_cfg(input_path);

    if (en) {
        // Enable vmic first to overlap vmic stable time with codec vcm stable time
        analog_aud_enable_vmic(ANA_CODEC_USER_ADC, dev, true);
        analog_aud_enable_codec_common(ANA_CODEC_USER_ADC, true);
        analog_aud_enable_adda_common(ANA_CODEC_USER_ADC, true);
        analog_aud_set_adc_gain(input_path, ch_map);
        analog_aud_enable_adc(ANA_CODEC_USER_ADC, ch_map, true);
    } else {
        analog_aud_enable_adc(ANA_CODEC_USER_ADC, ch_map, false);
        analog_aud_enable_adda_common(ANA_CODEC_USER_ADC, false);
        analog_aud_enable_codec_common(ANA_CODEC_USER_ADC, false);
        analog_aud_enable_vmic(ANA_CODEC_USER_ADC, dev, false);
    }
}

static void analog_aud_codec_config_speaker(void)
{
    bool en;

    if (ana_spk_req && !ana_spk_muted) {
        en = true;
    } else {
        en = false;
    }

    if (ana_spk_enabled != en) {
        ana_spk_enabled = en;
        if (en) {
            analog_aud_enable_dac_pa(CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV);
        } else {
            analog_aud_enable_dac_pa(0);
        }
    }
}

void analog_aud_codec_speaker_enable(bool en)
{
    ana_spk_req = en;
    analog_aud_codec_config_speaker();
}

void analog_aud_codec_dac_enable(bool en)
{
    if (en) {
        analog_aud_enable_codec_common(ANA_CODEC_USER_DAC, true);
        analog_aud_enable_adda_common(ANA_CODEC_USER_DAC, true);
        analog_aud_enable_dac(CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV);
#if !defined(AUDIO_OUTPUT_DC_CALIB_ANA) && !defined(AUDIO_OUTPUT_DC_CALIB)
        osDelay(1);
        analog_aud_codec_speaker_enable(true);
#endif
    } else {
#if !defined(AUDIO_OUTPUT_DC_CALIB_ANA) && !defined(AUDIO_OUTPUT_DC_CALIB)
        analog_aud_codec_speaker_enable(false);
        osDelay(1);
#endif
        analog_aud_enable_dac(0);
        analog_aud_enable_adda_common(ANA_CODEC_USER_DAC, false);
        analog_aud_enable_codec_common(ANA_CODEC_USER_DAC, false);
    }
}

void analog_aud_codec_open(void)
{
}

void analog_aud_codec_close(void)
{
    analog_aud_codec_speaker_enable(false);
    osDelay(1);
    analog_aud_codec_dac_enable(false);
}

void analog_aud_codec_mute(void)
{
#ifndef AUDIO_OUTPUT_DC_CALIB
    //analog_codec_tx_pa_gain_sel(0);
#endif

    ana_spk_muted = true;
    analog_aud_codec_config_speaker();
}

void analog_aud_codec_nomute(void)
{
    ana_spk_muted = false;
    analog_aud_codec_config_speaker();

#ifndef AUDIO_OUTPUT_DC_CALIB
    //analog_aud_set_dac_gain(dac_gain);
#endif
}

int analog_debug_config_audio_output(bool diff)
{
    return 0;
}

int analog_debug_config_codec(uint16_t mv)
{
#ifdef ANC_PROD_TEST
    int ret;

    ret = pmu_debug_config_codec(mv);
    if (ret) {
        return ret;
    }

    vcodec_mv = mv;
#endif
    return 0;
}

int analog_debug_config_low_power_adc(bool enable)
{
    return 0;
}

void analog_debug_config_anc_calib_mode(bool enable)
{
    anc_calib_mode = enable;
}

bool analog_debug_get_anc_calib_mode(void)
{
    return anc_calib_mode;
}

void analog_aud_adc_dc_auto_calib_enable(enum AUD_CHANNEL_MAP_T ch_map, bool on)
{
    uint16_t val;
    int i;

    for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
        if (ch_map & (AUD_CHANNEL_MAP_CH0 << i)) {
            analog_read(ANA_REG_09 + 0x10 * i, &val);
            if (on) {
                val = SET_BITFIELD(val, REG_CODEC_ADCA_CH_SEL_INT, 0);
            } else {
                val = SET_BITFIELD(val, REG_CODEC_ADCA_CH_SEL_INT, 2);
            }
            analog_write(ANA_REG_09 + 0x10 * i, val);
            osDelay(10);
        }
    }
}

void analog_aud_adc_dc_calib_set_step(enum AUD_CHANNEL_MAP_T ch_map, uint16_t step)
{
    int i;
    uint16_t val;

    for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
        if (ch_map & (AUD_CHANNEL_MAP_CH0 << i)) {
            analog_read(ANA_REG_04 + i * 0x10, &val);
            val = SET_BITFIELD(val, REG_CODEC_ADCA_IBSEL_OFFSET, step);
            analog_write(ANA_REG_04 + i * 0x10, val);
        }
    }
}

void analog_aud_adc_dc_calib_offset_update_enable(enum AUD_CHANNEL_MAP_T ch_map, bool on)
{
    int i;
    uint16_t val;

    for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
        if (ch_map & (AUD_CHANNEL_MAP_CH0 << i)) {
            analog_read(ANA_REG_06 + i * 0x10, &val);
            if (on) {
                val |= REG_CODEC_ADCA_OFFSET_CURRENT_EN;
                val &= ~(REG_CODEC_ADCA_OFFSET_CURRENT_SEL);
            } else {
                val |= REG_CODEC_ADCA_OFFSET_CURRENT_SEL;
                val &= ~(REG_CODEC_ADCA_OFFSET_CURRENT_EN);
            }
            analog_write(ANA_REG_06 + i * 0x10, val);
        }
    }
}

void analog_aud_adc_dc_calib_offset_update(enum AUD_CHANNEL_MAP_T ch_map, uint16_t value)
{
    int i;
    uint16_t val;
    for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
        value &= REG_CODEC_ADCA_OFFSET_BIT_MASK;
        if (ch_map & (AUD_CHANNEL_MAP_CH0 << i)) {
            analog_read(ANA_REG_07 + i * 0x10, &val);
            val = SET_BITFIELD(val, REG_CODEC_ADCA_OFFSET_BIT, value);
            analog_write(ANA_REG_07 + i * 0x10, val);
        }
    }
}

void analog_capsensor_open(void) //0x82, 0x7779 \ 0x83, 0x0081 \ 0x84,0x0240
{
    analog_aud_enable_codec_bias_lp(true);  //0x80, 0x4000
    // analog_aud_codec_enable_capsensor_osc(true); //0x5A 0xF04f
    analog_aud_codec_enable_capsensor_rc(true);  //0x5A 0xF010
}

void analog_capsensor_reset_set(void)
{
    uint16_t val;

    analog_read(ANA_REG_5A, &val);
    val &= ~(SOFT_RESETN_CAPSENSOR | SOFT_RESETN_CAPSENSOR_REG);
    analog_write(ANA_REG_5A, val);
}

void analog_capsensor_reset_clear(void)
{
    uint16_t val;

    analog_read(ANA_REG_5A, &val);
    val |= SOFT_RESETN_CAPSENSOR | SOFT_RESETN_CAPSENSOR_REG;
    analog_write(ANA_REG_5A, val);
}

void analog_capsensor_clk_gate_on(void)
{
    uint16_t val;

    analog_read(ANA_REG_5A, &val);
    val &= ~(REG_CLK_CAPSENSOR_GATE_EN | REG_CLK_32K_CAPSENSOR_GATE_EN);
    analog_write(ANA_REG_5A, val);
}

void analog_capsensor_clk_gate_off(void)
{
    uint16_t val;

    analog_read(ANA_REG_5A, &val);
    val |= REG_CLK_CAPSENSOR_GATE_EN | REG_CLK_32K_CAPSENSOR_GATE_EN;
    analog_write(ANA_REG_5A, val);
}
