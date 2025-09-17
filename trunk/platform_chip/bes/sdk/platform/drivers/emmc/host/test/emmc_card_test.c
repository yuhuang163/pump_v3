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
#include "stdlib.h"
#include "core.h"
#include "host.h"
#include "card.h"
#include "error.h"
#include "sdhci.h"
#include "card_mmc.h"
#include "card_sdio.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "sdhci_emphy.h"
#include "hal_location.h"
#include "emmc_platform.h"

#define EMMC_TRACE_SLE  1
#if   (EMMC_TRACE_SLE == 1)
    #define EMMC_TRACE(n, s, ...)       TRACE(n, s , ##__VA_ARGS__)
#elif (EMMC_TRACE_SLE == 2)
    #include "hal_def.h"
    #define EMMC_TRACE(n, s, ...)       log_hal(n, s , ##__VA_ARGS__)
#else
    #define EMMC_TRACE(n, s, ...)       TRACE_DUMMY(n, s, ##__VA_ARGS__)
#endif

/*
sdcard support speed mode
-------------------------------------------------
| Number |  Mode  |  Clk  |  .timing  |  .khz   |
|    1   |  SDR12 |  24M  |    SDR12  | F_SDR12 |
|    2   |  SDR25 |  48M  |    SDR25  | F_SDR25 |
|    3   |  DDR50 |  48M  |    DDR50  | F_DDR50 |
|    4   |  SDR50 |  96M  |    SDR50  | F_SDR50 |
|    5   |  SDR104|  192M |    SDR104 | F_SDR104|
-------------------------------------------------
*/
//#define SDCARD_TEST
//#define SDIO_TEST

/* reading and writing function selection */
//#define EMMC_RANDOM_WRITE_ENABLE
//#define EMMC_CONTINUOUS_READ_ENABLE       /* Test the fixed address read function */
//#define EMMC_CONTINUOUS_ONLY_READ_ENABLE  /* Test random read or continuous read functionality */
//#define EMMC_SINGLE_READ_WRITE_ENABLE
//#define EMMC_3V3_PWROFF_TEST              /* 3.3v power-down test of emmc device */
//#define EMMC_PWROFF_NOTIFICATION_TEST     /* emmc power off notification */

/* used to test different interfaces */
//#define EMMC_TEST_BLOCKING_MODE
#define EMMC_TEST_DMA_MODE
#define EMMC_CALLBACK_ENABLE

/* Used for digital simulation and grasping waveforms */
//#define SAVE_RESULT_FOR_SIMU
//#define EMMC_WRITE_TRIGGER_ENABLE
//#define TRIGGER_GPIO    HAL_GPIO_PIN_P3_0

/* Used to test the reading and writing function after opening EMMC's internal cache */
//#define EMMC_CACHE_ENABLE

/* Used to test the descriptor scatter function of ADMA2 */
//#define EMMC_ADMA_DESC_MODE

/* Used to modify the number of clks before stopping the output of clks */
//#define EMMC_SET_CLK_CNT_BF_GATE

/* Test the read and write performance of EMMC after waking up from sleep on the chip */
#define EMMC_CORE_SLEEP_POWER_DOWN_TEST

/* Data read and write tests that are smaller than one block */
//#define EMMC_WRITE_SMALL_DATA_TEST

/* RST_n_FUNCTION function settings
   Note: This modification is one-time and permanent
*/
//#define RST_N_FUNCTION_SET
//#define RST_N_FUNCTION_MODE     EMMC_RST_N_PERM_ENABLE
//#define RST_N_FUNCTION_MODE     EMMC_RST_N_PERM_DISABLE

/* emmc OCLK output: gpio30 */
//#define EMMC_OCLK_OUT_ENABLE

/* emmc input clk output: clkout */
#define EMMC_CLOCK_OUTPUT

/* write and read all blocks */
//#define EMMC_ALL_BLOCK_TEST_ENABLE

/* flag used by slt/ft to record the process */
//#define EMMC_SLT_FLAG_SET_ENABLE

/* big file test example exceeding 31.9MB */
//#define EMMC_BIG_FILE_TEST_ENABLE

/* Erase function test */
//#define EMMC_ERASE_TEST_ENABLE

#if defined(EMMC_ADMA_DESC_MODE) && defined(CORE_SLEEP_POWER_DOWN)
    #error "The ADMA test does not support in the PD mode, because the memory is too small"
#endif

#if defined(EMMC_TEST_BLOCKING_MODE) && defined(EMMC_TEST_DMA_MODE)
    #error "Blocking mode and dma mode cannot be used at the same time"
#elif !defined(EMMC_TEST_BLOCKING_MODE) && !defined(EMMC_TEST_DMA_MODE) && defined(EMMC_CALLBACK_ENABLE)
    #error "EMMC_CALLBACK_ENABLE cannot be defined in this mode"
#endif

#if defined(EMMC_CONTINUOUS_READ_ENABLE) && defined(EMMC_CONTINUOUS_ONLY_READ_ENABLE)
    #error "These 2 macros cannot be used at the same time"
#endif

#if defined(CHIP_BEST2009)
    #define GPIO_PIN_NUM_MAX    HAL_IOMUX_PIN_NUM
#else
    #define GPIO_PIN_NUM_MAX    HAL_IOMUX_PIN_PMU_NUM
#endif

enum EMMC_TEST_TAG_T {
    EMMC_TEST_TAG_PLAT_INIT_OK      = 0,
    EMMC_TEST_TAG_HOST_INIT_OK      = 1,
    EMMC_TEST_TAG_DET_CARD_OK       = 2,
    EMMC_TEST_TAG_INIT_CARD_OK      = 3,
    EMMC_TEST_TAG_CONFIG_CARD_OK    = 4,
    EMMC_TEST_TAG_SINGLE_WR_OK      = 5,
    EMMC_TEST_TAG_MULTI_WR_OK       = 6,
    EMMC_TEST_TAG_SINGLE_RD_OK      = 7,
    EMMC_TEST_TAG_MULTI_RD_OK       = 8,
    EMMC_TEST_TAG_DATA_CHECK_OK     = 9,
    EMMC_TEST_TAG_SUCCESS           = 10,
};

#ifdef EMMC_USE_HEAP
    #define EMMC_XBUF_SIZE 2048
#else
    #if (RAM_SIZE <= 256 * 1024)
        #define EMMC_XBUF_SIZE 2048
    #else
        #define EMMC_XBUF_SIZE (64 * 1024)
    #endif
    SYNC_FLAGS_LOC static uint32_t emmc_src_buf[EMMC_XBUF_SIZE / 4];
    SYNC_FLAGS_LOC static uint32_t emmc_dst_buf[EMMC_XBUF_SIZE / 4];
#endif
#if (EMMC_XBUF_SIZE < CARD_BLOCK_LENGTH)
    #error "EMMC_XBUF_SIZE cannot be less than 512"
#endif
static struct mmc_host emmc_test_op;
static struct mmc_card_info card_info;

static void emmc_delay(uint32_t ms)
{
    osDelay(ms);
}

static volatile uint8_t error_flag = 0;
POSSIBLY_UNUSED static void emmc_host_error(enum EMMC_HOST_ERR error)
{
    error_flag = 1;
    EMMC_TRACE(1, "******emmc error:0x%X", error);
}

#ifdef EMMC_CALLBACK_ENABLE
static volatile uint8_t dma_done_flag = 0;
static void emmc_host_dma_done(void)
{
    dma_done_flag = 1;

    //log is closed, otherwise the read and write speed will be affected
    //EMMC_TRACE(1, "------%s, emmc dma tx/rx done, flag: %d", __FUNCTION__, dma_done_flag);
}
#endif

static struct emmc_config emmc_cfg = {
    /*pmu*/
    .vmin_begin = 1800,

    /*test*/
#ifdef SDCARD_TEST
    .controller = DEV_TYPE_SD,
#elif defined SDIO_TEST
    .controller = DEV_TYPE_SDIO,
#else
    .controller = DEV_TYPE_EMMC,
#endif
#ifdef EMMC_STRESS_TEST
    .test_id = SDHCI_STRESS_TEST,
#else
    .test_id = SDHCI_DATA_XFER_TEST,
#endif
    .subtest_id = 0,
    .real_card = 1,

    /*platform*/
    .src_region = 0,
    .dst_region = 0,
    .aligned = 4,

    /* The Host's GPIO pin connects with emmc device RSTn pin */
    /*
    evb core board     : HAL_IOMUX_PIN_P3_6
    socket board       : HAL_IOMUX_PIN_P8_1
    mi watch board     : HAL_IOMUX_PIN_PMU_P1_4
    mi Evaluation board: HAL_IOMUX_PIN_P4_6
    */
    .rst_pin = HAL_IOMUX_PIN_P3_6,
#ifdef EMMC_EXTENDED_USE
    .pwr_3p3_pin = HAL_IOMUX_PIN_PMU_P6_5,
    .pwr_1p8_pin = HAL_IOMUX_PIN_PMU_P5_6,
    .pwr_on_level = 1,
#else
    /* BES EVB Core Board V1.2/1.3... */
#if defined(CHIP_BEST2009)
    .pwr_3p3_pin = GPIO_PIN_NUM_MAX,
    .pwr_1p8_pin = GPIO_PIN_NUM_MAX,
#else
    .pwr_3p3_pin = HAL_IOMUX_PIN_PMU_P2_3,
    .pwr_1p8_pin = HAL_IOMUX_PIN_PMU_P2_1,
#endif
    .pwr_on_level = 1,
#endif
    .clk_drv = 2,
    .cmd_drv = 2,
    .dat_drv = 2,
    .vcc_main = 1800,

    /*data transfer*/
    .blksz = CARD_BLOCK_LENGTH,
    .blks = EMMC_XBUF_SIZE / CARD_BLOCK_LENGTH,
    .card_offset = 0,
    .seed = 0x12345678,
    .pattern = MMC_DATA_PATTERN_RANDOM,

    /*host mode*/
#ifdef EMMC_ADMA_DESC_MODE
    .dma_mode = MM4_32_BIT_ADMA_2,//EMMC_ADMA_DESC_CNT should be big enough
#else
    .dma_mode = MM4_SDMA,
#endif
    .dma_bdry = MM4_HOST_DMA_BDRY_512KB,
    .auto_cmd_type = MM4_AUTO_CMD12,//SDMA does not support AutoCMD23
#ifdef EMMC_ADMA_DESC_MODE
    .desc_type = ADMA2_DESC_SCATTER,
#else
    .desc_type = ADMA2_DESC_FIXED_SZ,
#endif
    .desc_sz = ADMA2_DESC_MAX_LEN,
#ifdef EMMC_USE_HEAP
    .desc_nr = 1,
#else
    .desc_nr = EMMC_XBUF_SIZE / ADMA2_DESC_MAX_LEN ? EMMC_XBUF_SIZE / ADMA2_DESC_MAX_LEN : 1,
#endif
    .clk_gating_en = 1,
    .resp_polling_mode = 1,

    /*bus io*/
#if defined(SDIO_TEST) || defined(SDCARD_TEST)
#ifdef EMMC_DDR_MODE
    .timing = DDR50,
    .khz = F_DDR50,
#else
#if defined(EMMC_USE_PLL)
    .timing = SDR104,
    .khz = F_SDR104,
#elif defined(EMMC_HS_MODE)
    .timing = SDR25,
    .khz = F_SDR25,
#else
    .timing = DS,
    .khz = F_DS,
#endif
#endif
#else
#ifdef EMMC_DDR_MODE
#if defined(EMMC_USE_PLL)
    .timing = HS400,
    .khz = F_HS400,
#else
    .timing = DDR52,
    .khz = F_DDR52,
#endif
#else
#if defined(EMMC_USE_PLL)
    .timing = HS200,
    .khz = F_HS200,
#elif defined(EMMC_HS_MODE)
    .timing = SDR52,
    .khz = F_SDR52,
#else
    .timing = DS,
    .khz = F_DS,
#endif
#endif    //EMMC_DDR_MODE
#endif    //SDIO_TEST || SDCARD_TEST
#if defined SDCARD_TEST || defined SDIO_TEST
    .width = MMC_4BIT_WIDTH,
#else
    .width = MMC_8BIT_WIDTH,
#endif
    .voltage = 1800,
    .strobe = 0,//0 or 1 can, the software will automatically modify the value according to the parameters of the EMMC.
    .quirks = MMC_QUIRKS_HW_TUNING | MMC_QUIRKS_PERFORMANCE,

    /*delay*/
    .delay = emmc_delay,

    /*callback*/
    .emmc_host_error = emmc_host_error,
#ifdef EMMC_CALLBACK_ENABLE
    .emmc_host_dma_done = emmc_host_dma_done,
#else
    .emmc_host_dma_done = NULL,
#endif
};

static void emmc_rst_pwr_pin_init(struct mmc_host *host)
{
    struct emmc_config *cfg;
    struct HAL_IOMUX_PIN_FUNCTION_MAP iomux[] = {
        {HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    cfg = (struct emmc_config *)host->private;

    //emmc reset pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->rst_pin) < HAL_IOMUX_PIN_NUM) {
        iomux[0].pin = (enum HAL_IOMUX_PIN_T)(cfg->rst_pin);
        hal_iomux_init(iomux, ARRAY_SIZE(iomux));
    } else if ((enum HAL_IOMUX_PIN_T)(cfg->rst_pin) < GPIO_PIN_NUM_MAX) {
        //pmu gpio does not require hal_iomux_init
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)(cfg->rst_pin), HAL_GPIO_DIR_OUT, 1);
    }

    //emmc 3.3v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_3p3_pin) < HAL_IOMUX_PIN_NUM) {
        iomux[0].pin = (enum HAL_IOMUX_PIN_T)(cfg->pwr_3p3_pin);
        hal_iomux_init(iomux, ARRAY_SIZE(iomux));
    } else if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_3p3_pin) < GPIO_PIN_NUM_MAX) {
        //pmu gpio does not require hal_iomux_init
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)(cfg->pwr_3p3_pin), HAL_GPIO_DIR_OUT, 0);
    }

    //emmc 1.8v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_1p8_pin) < HAL_IOMUX_PIN_NUM) {
        iomux[0].pin = (enum HAL_IOMUX_PIN_T)(cfg->pwr_1p8_pin);
        hal_iomux_init(iomux, ARRAY_SIZE(iomux));
    } else if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_1p8_pin) < GPIO_PIN_NUM_MAX) {
        //pmu gpio does not require hal_iomux_init
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)(cfg->pwr_1p8_pin), HAL_GPIO_DIR_OUT, 0);
    }
}

static void emmc_reset_pin_pull_down(struct mmc_host *host)
{
    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(cfg->rst_pin));

    EMMC_TRACE(1, "%s: done", __FUNCTION__);
}

static void emmc_reset_pin_pull_up(struct mmc_host *host)
{
    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(cfg->rst_pin));

    EMMC_TRACE(1, "%s: done", __FUNCTION__);
}

static void emmc_reset_device(struct mmc_host *host)
{
    emmc_reset_pin_pull_down(host);
    hal_sys_timer_delay_us(20);
    emmc_reset_pin_pull_up(host);

    EMMC_TRACE(1, "%s: done", __FUNCTION__);
}

static void emmc_poweron_device(struct mmc_host *host)
{
    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    //emmc 3.3v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_3p3_pin) < GPIO_PIN_NUM_MAX) {
        if (cfg->pwr_on_level) {
            hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(cfg->pwr_3p3_pin));
        } else {
            hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(cfg->pwr_3p3_pin));
        }
    }

    //emmc 1.8v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_1p8_pin) < GPIO_PIN_NUM_MAX) {
        if (cfg->pwr_on_level) {
            hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(cfg->pwr_1p8_pin));
        } else {
            hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(cfg->pwr_1p8_pin));
        }
    }

    EMMC_TRACE(1, "%s: done", __FUNCTION__);
}

POSSIBLY_UNUSED static void emmc_poweron_device_3v3(struct mmc_host *host)
{
    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    //emmc 3.3v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_3p3_pin) < GPIO_PIN_NUM_MAX) {
        if (cfg->pwr_on_level) {
            hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(cfg->pwr_3p3_pin));
        } else {
            hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(cfg->pwr_3p3_pin));
        }
    }

    EMMC_TRACE(1, "%s: done", __FUNCTION__);
}

static void emmc_poweroff_device(struct mmc_host *host)
{
    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    //emmc 3.3v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_3p3_pin) < GPIO_PIN_NUM_MAX) {
        if (cfg->pwr_on_level) {
            hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(cfg->pwr_3p3_pin));
        } else {
            hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(cfg->pwr_3p3_pin));
        }
    }

    //emmc 1.8v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_1p8_pin) < GPIO_PIN_NUM_MAX) {
        if (cfg->pwr_on_level) {
            hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(cfg->pwr_1p8_pin));
        } else {
            hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(cfg->pwr_1p8_pin));
        }
    }

    EMMC_TRACE(1, "%s: done", __FUNCTION__);
}

POSSIBLY_UNUSED static void emmc_poweroff_device_3v3(struct mmc_host *host)
{
    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    //emmc 3.3v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_3p3_pin) < GPIO_PIN_NUM_MAX) {
        if (cfg->pwr_on_level) {
            hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(cfg->pwr_3p3_pin));
        } else {
            hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(cfg->pwr_3p3_pin));
        }
    }

    EMMC_TRACE(1, "%s: done", __FUNCTION__);
}

POSSIBLY_UNUSED static void emmc_dump32(const char *str, uint32_t *buf, uint32_t size)
{
    uint32_t i;

    TRACE_FLUSH();
    if (str) {
        EMMC_TRACE(1, "%s, buf=%x, size=%d", str, (uint32_t)buf, size);
    }
    EMMC_TRACE(0, " ");
    for (i = 0; i < size / 4; i += 8) {
        EMMC_TRACE(1, "%08X %08X %08X %08X %08X %08X %08X %08X",
                   buf[i + 0], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4], buf[i + 5], buf[i + 6], buf[i + 7]);
    }
    EMMC_TRACE(1, " ");
    TRACE_FLUSH();
}

static void mmc_test_info(struct mmc_host *host)
{
    EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);
    EMMC_TRACE(1, "-------------------------------------------------------");
    EMMC_TRACE(1, "Card Type                   : %s", mmc_card_type(host->card->type));
    EMMC_TRACE(1, "Data Transfer Bit-Width     : %s", mmc_bitwidth_type(host->ios.width));
    EMMC_TRACE(1, "Data Transfer Bus Mode      : %s", mmc_timing_type(host->ios.timing));
    EMMC_TRACE(1, "Data Transfer ClkRate       : %dMHz", (host->ios.khz) / 1000);
    EMMC_TRACE(1, "Data Transfer Size(Blocks)  : %d", host->blks);
    EMMC_TRACE(1, "Data Transfer Auto CMD Type : %s", mmc_autocmd_type(host->auto_cmd_type));
    if (host->dma_mode == MM4_32_BIT_ADMA_2) {
        EMMC_TRACE(1, "ADMA Desc Type              : %s", mmc_adma2_desc_type(host->desc_type));
    }
    EMMC_TRACE(1, "-------------------------------------------------------");
}

static int sdhci_data_xfer_setup(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;
    uint32_t time_base;
    uint32_t emmc_init_time[5];

    EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);

    emmc_rst_pwr_pin_init(host);
    emmc_poweron_device(host);
    emmc_reset_device(host);
    host->delay(100);//Wait for emmc to power on and stabilize

    time_base = hal_fast_sys_timer_get();
    emmc_init_time[0] = time_base;
    ret = sdhci_platform_init(true, host);
    if (ret != MMC_NO_ERR) {
        return ret;
    }
#ifdef EMMC_SLT_FLAG_SET_ENABLE
    hal_cmu_simu_tag(EMMC_TEST_TAG_PLAT_INIT_OK);
#endif
    emmc_init_time[1] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_base);

    time_base = hal_fast_sys_timer_get();
    ret = sdhci_host_init(true, host);
    if (ret != MMC_NO_ERR) {
        return ret;
    }
#ifdef EMMC_SLT_FLAG_SET_ENABLE
    hal_cmu_simu_tag(EMMC_TEST_TAG_HOST_INIT_OK);
#endif
    emmc_init_time[2] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_base);

#ifdef EMMC_SET_CLK_CNT_BF_GATE
    uint8_t clkcnt;

    clkcnt = sdhci_get_clk_cnt_bf_gate(host);
    EMMC_TRACE(0, "###old clk cnt before gate:%d", clkcnt);

    sdhci_set_clk_cnt_bf_gate(host, 0xF);
    EMMC_TRACE(0, "set clk cnt before gate:0xF");
    clkcnt = sdhci_get_clk_cnt_bf_gate(host);
    EMMC_TRACE(0, "###new clk cnt before gate:%d", clkcnt);
#endif

    time_base = hal_fast_sys_timer_get();
    ret = detect_card(host);
    if (ret != MMC_NO_ERR) {
        return ret;
    }
#ifdef EMMC_SLT_FLAG_SET_ENABLE
    hal_cmu_simu_tag(EMMC_TEST_TAG_DET_CARD_OK);
#endif
    emmc_init_time[3] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_base);

    time_base = hal_fast_sys_timer_get();
    ret = init_card(host);
    if (ret != MMC_NO_ERR) {
        return ret;
    }
#ifdef EMMC_SLT_FLAG_SET_ENABLE
    hal_cmu_simu_tag(EMMC_TEST_TAG_INIT_CARD_OK);
#endif
    emmc_init_time[4] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_base);
    emmc_init_time[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - emmc_init_time[0]);

#ifdef EMMC_CACHE_ENABLE
    uint8_t cache_value;

    cache_value = host->card->ext_csd[EXT_CSD__CACHE_CTRL];
    EMMC_TRACE(0, "  ");
    EMMC_TRACE(0, "------CACHE_CTRL before:%d", cache_value);
    ret = card_cache_control(host, 1);
    if (ret) {
        while (1) {
            EMMC_TRACE(0, "emmc open cache:(%s)", ret ? "fail" : "success");
            host->delay(1000);
        }
    }

    ret = mmc_send_ext_csd(host);
    if (ret) {
        while (1) {
            EMMC_TRACE(0, "******send ext csd err:%d, line:%d", ret, __LINE__);
            host->delay(1000);
        }
    }

    cache_value = host->card->ext_csd[EXT_CSD__CACHE_CTRL];
    EMMC_TRACE(0, "------CACHE_CTRL after :%d", cache_value);
    if (cache_value == 1) {
        EMMC_TRACE(0, "++++++emmc open cache success");
    } else {
        while (1) {
            EMMC_TRACE(0, "******emmc open cache fail");
            host->delay(1000);
        }
    }
#endif

#ifdef RST_N_FUNCTION_SET
    ret = mmc_rst_n_mode_set(host, RST_N_FUNCTION_MODE);
    EMMC_TRACE(0, "+++mmc_rst_n_mode_set:0x%x", ret);
#endif

#ifdef EMMC_OCLK_OUT_ENABLE
    hal_iomux_set_emmc_tport();
    sdhci_set_oclk_out(host);
#endif

    EMMC_TRACE(0, "  ");
    EMMC_TRACE(0, "**********************************");
    EMMC_TRACE(0, "emmc platform init time: %dus", emmc_init_time[1]);
    EMMC_TRACE(0, "emmc host init time    : %dus", emmc_init_time[2]);
    EMMC_TRACE(0, "emmc detect card time  : %dus", emmc_init_time[3]);
    EMMC_TRACE(0, "emmc init card time    : %dus", emmc_init_time[4]);
    EMMC_TRACE(0, "emmc init total time   : %dus", emmc_init_time[0]);
    EMMC_TRACE(0, "**********************************");
    EMMC_TRACE(0, "  ");

    get_card_info(&card_info);
    EMMC_TRACE(0, "********************************************");
    EMMC_TRACE(1, "card type                : %s", card_info.cardtype ? "1(high capacity)" : "0(low capacity)");
    EMMC_TRACE(1, "card class               : 0x%X", card_info.cardclass);
    EMMC_TRACE(1, "card relcardAdd          : 0x%X", card_info.relcardadd);
    EMMC_TRACE(1, "card blocknbr            : 0x%X", card_info.blocknbr);
    EMMC_TRACE(1, "card blocksize           : 0x%X", card_info.blocksize);
    EMMC_TRACE(1, "card logblocknbr         : 0x%X", card_info.logblocknbr);
    EMMC_TRACE(1, "card logllocksize        : 0x%X", card_info.logllocksize);
    EMMC_TRACE(1, "card ext_csd_rev         : 0x%X", card_info.ext_csd_rev);
    EMMC_TRACE(1, "card firmware version_h32: 0x%X", card_info.firmware_version_h32);
    EMMC_TRACE(1, "card firmware version_l32: 0x%X", card_info.firmware_version);
    EMMC_TRACE(1, "card device version      : 0x%X", card_info.device_version);
    EMMC_TRACE(1, "card cache size          : 0x%X", card_info.cache_size);
    EMMC_TRACE(1, "card cache ctrl          : %s", card_info.cache_ctrl ? "1(enabled)" : "0(disable)");
    EMMC_TRACE(1, "card cache flush policy  : %s", card_info.cache_flush_policy ? "1(fifo mode)" : "0(unknown)");
    EMMC_TRACE(1, "card generic cmd6 time   : %dms", card_info.generic_cmd6_time);
    EMMC_TRACE(1, "csd.csd_structure        : 0x%X", card_info.csd.csd_structure);
    EMMC_TRACE(1, "csd.spec_vers            : 0x%X", card_info.csd.spec_vers);
    EMMC_TRACE(1, "csd.taac                 : 0x%X", card_info.csd.taac);
    EMMC_TRACE(1, "csd.nsac                 : 0x%X", card_info.csd.nsac);
    EMMC_TRACE(1, "csd.tran_speed           : 0x%X", card_info.csd.tran_speed);
    EMMC_TRACE(1, "csd.ccc                  : 0x%X", card_info.csd.ccc);
    EMMC_TRACE(1, "csd.read_bl_len          : 0x%X", card_info.csd.read_bl_len);
    EMMC_TRACE(1, "csd.read_bl_partial      : 0x%X", card_info.csd.read_bl_partial);
    EMMC_TRACE(1, "csd.write_blk_misalign   : 0x%X", card_info.csd.write_blk_misalign);
    EMMC_TRACE(1, "csd.read_blk_misalign    : 0x%X", card_info.csd.read_blk_misalign);
    EMMC_TRACE(1, "csd.dsr_imp              : 0x%X", card_info.csd.dsr_imp);
    EMMC_TRACE(1, "csd.c_size               : 0x%X", card_info.csd.c_size);
    EMMC_TRACE(1, "csd.vdd_r_curr_min       : 0x%X", card_info.csd.vdd_r_curr_min);
    EMMC_TRACE(1, "csd.vdd_r_curr_max       : 0x%X", card_info.csd.vdd_r_curr_max);
    EMMC_TRACE(1, "csd.vdd_w_curr_min       : 0x%X", card_info.csd.vdd_w_curr_min);
    EMMC_TRACE(1, "csd.vdd_w_curr_max       : 0x%X", card_info.csd.vdd_w_curr_max);
    EMMC_TRACE(1, "csd.c_size_mult          : 0x%X", card_info.csd.c_size_mult);
    EMMC_TRACE(1, "csd.erase_grp_size       : 0x%X", card_info.csd.erase_grp_size);
    EMMC_TRACE(1, "csd.erase_grp_mult       : 0x%X", card_info.csd.erase_grp_mult);
    EMMC_TRACE(1, "csd.wp_grp_size          : 0x%X", card_info.csd.wp_grp_size);
    EMMC_TRACE(1, "csd.wp_grp_enable        : 0x%X", card_info.csd.wp_grp_enable);
    EMMC_TRACE(1, "csd.default_ecc          : 0x%X", card_info.csd.default_ecc);
    EMMC_TRACE(1, "csd.r2w_factor           : 0x%X", card_info.csd.r2w_factor);
    EMMC_TRACE(1, "csd.write_bl_len         : 0x%X", card_info.csd.write_bl_len);
    EMMC_TRACE(1, "csd.write_bl_partial     : 0x%X", card_info.csd.write_bl_partial);
    EMMC_TRACE(1, "csd.content_prot_app     : 0x%X", card_info.csd.content_prot_app);
    EMMC_TRACE(1, "csd.file_format_grp      : 0x%X", card_info.csd.file_format_grp);
    EMMC_TRACE(1, "csd.copy                 : 0x%X", card_info.csd.copy);
    EMMC_TRACE(1, "csd.perm_write_protect   : 0x%X", card_info.csd.perm_write_protect);
    EMMC_TRACE(1, "csd.tmp_write_protect    : 0x%X", card_info.csd.tmp_write_protect);
    EMMC_TRACE(1, "csd.file_format          : 0x%X", card_info.csd.file_format);
    EMMC_TRACE(1, "csd.ecc                  : 0x%X", card_info.csd.ecc);
    EMMC_TRACE(1, "Manufacturer ID          : 0x%X", card_info.cid.mid);
    EMMC_TRACE(1, "OEM/Application ID       : 0x%X", card_info.cid.oid & 0xFF);
    EMMC_TRACE(1, "Device/BGA               : 0x%X", card_info.cid.cbx);
    EMMC_TRACE(1, "Product Name             : %c%c%c%c%c%c",  card_info.cid.pnm[0], \
                                                              card_info.cid.pnm[1], \
                                                              card_info.cid.pnm[2], \
                                                              card_info.cid.pnm[3], \
                                                              card_info.cid.pnm[4], \
                                                              (card_info.cid.pnm[5] & 0x7E) < 0x20 ? ' ' : (card_info.cid.pnm[5] & 0x7E));//character range
    EMMC_TRACE(1, "Product Revision         : %d.%d", (card_info.cid.prv >> 4), (card_info.cid.prv & 0xF));
    EMMC_TRACE(1, "Product Serial No        : 0x%X", card_info.cid.psn);
    EMMC_TRACE(1, "Manufacture Date         : %d/%d", 2000 + (card_info.cid.mdt >> 4), card_info.cid.mdt & 0xF);
    POSSIBLY_UNUSED uint32_t val = (uint32_t)(((uint64_t)card_info.blocknbr * card_info.blocksize * 100) / 1073741824);
    EMMC_TRACE(1, "Card capacity            : %d.%dGB", val / 100, val % 100);
    EMMC_TRACE(0, "********************************************");
    EMMC_TRACE(0, "  ");

    return ret;
}

//Initialization example
POSSIBLY_UNUSED static int emmc_open(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;
    struct emmc_config *cfg;

    emmc_rst_pwr_pin_init(host);
    emmc_poweron_device(host);
    emmc_reset_device(host);
    host->delay(10);//Wait for emmc to power on and stabilize

    ret = sdhci_platform_init(true, host);
    if (ret != MMC_NO_ERR) {
        EMMC_TRACE(0, "%s: sdhci platform init failed, %d", __FUNCTION__, ret);
        return ret;
    }

    ret = sdhci_host_init(true, host);
    if (ret != MMC_NO_ERR) {
        EMMC_TRACE(0, "%s: sdhci host init failed, %d", __FUNCTION__, ret);
        return ret;
    }

    ret = detect_card(host);
    if (ret != MMC_NO_ERR) {
        EMMC_TRACE(0, "%s: detect card failed, %d", __FUNCTION__, ret);
        return ret;
    }

    ret = init_card(host);
    if (ret != MMC_NO_ERR) {
        EMMC_TRACE(0, "%s: init card failed, %d", __FUNCTION__, ret);
        return ret;
    }

    cfg = (struct emmc_config *)host->private;
    ret = config_card(host, cfg->timing, cfg->khz, cfg->width);
    if (ret != MMC_NO_ERR) {
        EMMC_TRACE(0, "%s: config card failed, %d", __FUNCTION__, ret);
        return ret;
    }

    switch (host->speed_mode) {
        case DS:
            EMMC_TRACE(0, "EMMC DS mode");
            break;
        case SDR52:
            EMMC_TRACE(0, "EMMC SDR52 mode");
            break;
        case DDR26:
            EMMC_TRACE(0, "EMMC DDR26 mode");
            break;
        case DDR52:
            EMMC_TRACE(0, "EMMC DDR52 mode");
            break;
        case HS25:
            EMMC_TRACE(0, "EMMC HS25 mode");
            break;
        case HS50:
            EMMC_TRACE(0, "EMMC HS50 mode");
            break;
        case HS100:
            EMMC_TRACE(0, "EMMC HS100 mode");
            break;
        case HS200:
            EMMC_TRACE(0, "EMMC HS200 mode");
            break;
        case HS400:
            EMMC_TRACE(0, "EMMC HS400 mode");
            break;
        case EHS400:
            EMMC_TRACE(0, "EMMC EHS400 mode");
            break;
        default:
            break;
    }

    return ret;
}

static int emmc_close(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    emmc_poweroff_device(host);     //close emmc device
    emmc_reset_pin_pull_down(host); //reset pin pull down

    ret = sdhci_host_init(false, host);//close emmc controller
    if (ret != MMC_NO_ERR) {
        EMMC_TRACE(0, "%s: sdhci host init failed %d", __FUNCTION__, ret);
        return ret;
    }

    ret = sdhci_host_emphy_close(NULL);//close phy
    if (ret != MMC_NO_ERR) {
        EMMC_TRACE(0, "%s: sdhci host emphy close failed %d", __FUNCTION__, ret);
        return ret;
    }

    return ret;
}

POSSIBLY_UNUSED static void rand_generator(uint8_t *dest, size_t size)
{
    while (size--) {
        *dest++ = (uint8_t)rand();
    }
}

#if (EMMC_XBUF_SIZE <= CARD_BLOCK_LENGTH)
    #define MULTI_TEST_SIZE         CARD_BLOCK_LENGTH
    #define MULTI_TEST_CNT          1
#else
    #define MULTI_TEST_SIZE         (2 * 1024 * 1024)//2MBytes
    #define MULTI_TEST_CNT          (MULTI_TEST_SIZE / EMMC_XBUF_SIZE)
#endif
#define RETRY_CNT_MAX               3

//For digital simulation
#define RESULT_WRITE_PASS           0xAAAA
#define RESULT_WRITE_FAIL           0x95AA
#define RESULT_READ_PASS            0x5555
#define RESULT_READ_FAIL            0xAA55
#define RESULT_ADDR_WRITE           0x808
#define RESULT_ADDR_READ            0x80C
#define HW_WRITE_REG(_Reg,_Value)   (*((volatile uint32_t*)(_Reg)) = (uint32_t)(_Value))

#ifdef SDIO_TEST
    static uint8_t  sdio_data_buff[32768];
#endif

static int sdhci_data_xfer_test(struct mmc_host *host, bool cfg_card)
{
    int ret = MMC_NO_ERR;
    POSSIBLY_UNUSED uint8_t retry_cnt;
    uint32_t i;
    uint64_t val;
    uint32_t start_addr;

    uint32_t size = host->blks * host->blksz;
    uint32_t len = size / sizeof(uint32_t);

#ifdef EMMC_USE_HEAP
    uint32_t *src_mem = (uint32_t *)emmc_heap_malloc(size);
    uint32_t *dst_mem = (uint32_t *)emmc_heap_malloc(size);
#else
    uint32_t *src_mem = (uint32_t *)emmc_src_buf;
    uint32_t *dst_mem = (uint32_t *)emmc_dst_buf;
#endif
    uint32_t *src = (uint32_t *)src_mem;
    uint32_t *dst = (uint32_t *)dst_mem;
    uint32_t timeb, time[7] = {0};

    EMMC_TRACE(1, "%s: src=%x, dst=%x", __FUNCTION__, (uint32_t)src, (uint32_t)dst);

    memset(src, 0x0, len);
    memset(dst, 0x0, len);

    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    if (host->speed_mode == HS400 || host->speed_mode == EHS400) {
        hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_208M);
        TR_INFO(TR_MOD(MAIN), "CPU freq change to: %u", hal_sys_timer_calc_cpu_freq(5, 0));
    }

#ifdef EMMC_RANDOM_WRITE_ENABLE
    uint32_t sector_count;

    sector_count = (uint32_t)(*(uint32_t *)&host->card->ext_csd[212]);//get max sector conut
    EMMC_TRACE(0, "------sector conut[SEC_COUNT]:0x%X", sector_count);

    cfg->card_offset = (uint32_t)((rand() % RAND_MAX) / (double)RAND_MAX * sector_count);
    EMMC_TRACE(0, "------random card offset:0x%X", cfg->card_offset);
    if ((uint64_t)cfg->card_offset * 512 + MULTI_TEST_SIZE > (uint64_t)sector_count * 512) {
        cfg->card_offset = (uint32_t)((uint64_t)sector_count * 512 - MULTI_TEST_SIZE) / 512;
    }
    EMMC_TRACE(0, "------final card offset :0x%X", cfg->card_offset);
#endif

    timeb = hal_fast_sys_timer_get();
    if (cfg_card) {
        ret = config_card(host, cfg->timing, cfg->khz, cfg->width);
        if (ret != MMC_NO_ERR) {
            EMMC_TRACE(1, "%s: config card failed, %d", __FUNCTION__, ret);
            goto _exit;
        }
    }
    time[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);

#ifdef EMMC_SLT_FLAG_SET_ENABLE
    hal_cmu_simu_tag(EMMC_TEST_TAG_CONFIG_CARD_OK);
#endif
    mmc_test_info(host);

    // fill src,dst data
    for (i = 0; i < len; i++) {
#ifdef EMMC_RANDOM_WRITE_ENABLE
        src[i] = 0xAA55AA55 + (uint32_t)rand();
#else
        src[i] = 0xAA55AA55;
#endif
        dst[i] = 0x0;
    }
#ifdef EMMC_TEST_DUMP_DATA
    emmc_dump32("dump1 src", src, len);
    emmc_dump32("dump1 dst", dst, len);
#endif

    POSSIBLY_UNUSED uint32_t card_status;
#ifndef SDIO_TEST
    card_status = mmc_card_status(host);
    EMMC_TRACE(0, "------card before sleep, card current state:0x%x", card_status);
    card_status = mmc_card_raw_status(host);
    EMMC_TRACE(0, "------card before sleep, card raw status   :0x%x", card_status);
    sleep_card(host);
    card_status = mmc_card_sleep_status(host);
    EMMC_TRACE(0, "------card sleep status                    :%d(%s)", card_status, card_status ? "sleep" : "awake");
    host->delay(10);
    awake_card(host);
    card_status = mmc_card_status(host);
    EMMC_TRACE(0, "------card after awake,  card current state:0x%x", card_status);
    card_status = mmc_card_raw_status(host);
    EMMC_TRACE(0, "------card after awake,  card raw status   :0x%x", card_status);
    card_status = mmc_card_sleep_status(host);
    EMMC_TRACE(0, "------card sleep status                    :%d(%s)", card_status, card_status ? "sleep" : "awake");

    EMMC_TRACE(0, "  ");
    EMMC_TRACE(0, "=================> writing card");
    EMMC_TRACE(1, "------card offset=0x%x, buf=0x%x, buf sz=%u", cfg->card_offset, (uint32_t)src, size);
    retry_cnt = 0;
    error_flag = 0;
#ifdef EMMC_CALLBACK_ENABLE
    dma_done_flag = 0;
#endif
#else
    uint8_t  response_r5;
    POSSIBLY_UNUSED  uint8_t buf_cnt = 0;
    POSSIBLY_UNUSED uint16_t device_rx_buf_len_max;
    POSSIBLY_UNUSED uint16_t real_write_len = 0;
    uint16_t blocksize = 0;
    uint8_t capability = 0;
    uint8_t bus_speed_mode = 0;
    sdio_get_block_size(host, SDIO_FUNC_1, &blocksize);
    EMMC_TRACE(0, "------sdio block size:%d", blocksize);
    sdio_get_capability(host, &capability);
    EMMC_TRACE(0, "------sdio device capability:0x%X", capability);
    sdio_get_bus_speed(host, &bus_speed_mode);
    EMMC_TRACE(0, "------sdio speed mode:%d", bus_speed_mode);
    while (1) {
        uint32_t time0, time1, time2;
        buf_cnt = 0;
        memset(sdio_data_buff, 0xAA, sizeof(sdio_data_buff));
        sdio_host_gen_int_to_device(host);
        host->delay(20);
        sdio_host_func_int(host, 1);
        time0 = hal_fast_sys_timer_get();
        sdio_io_rw_extended(host, SDIO_DEVICE_READ, SDIO_FUNC_1, 0,
                            &response_r5, 1, (uint8_t *)sdio_data_buff, 8192, BLOCKING_MODE_IRQ_ON, 1);
        time1 = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time0);

        if (!buf_cnt) {
            memset(sdio_data_buff, 0xAA, sizeof(sdio_data_buff));
            sdio_host_get_func1_rx_buf_cfg(host, &buf_cnt, &device_rx_buf_len_max);

            EMMC_TRACE(2, "get rx buf cfg to write %d", buf_cnt);
        }

        if (buf_cnt) {
            real_write_len = 8192;

            time0 = hal_fast_sys_timer_get();
            sdio_io_rw_extended(host, SDIO_DEVICE_WRITE, SDIO_FUNC_1, real_write_len,
                                &response_r5, 1, (uint8_t *)sdio_data_buff, real_write_len, BLOCKING_MODE_IRQ_ON, 1);
            time2 = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time0);
            host->delay(20);
            time1 = (uint32_t)(95400 * 8192 / 1000 / time1);
            EMMC_TRACE(1, "read speed    :%u.%uMB/s", time1 / 100, time1 % 100);

            time2 = (uint32_t)(95400 * 8192 / 1000 / time2);
            EMMC_TRACE(1, "write speed     :%u.%uMB/s", time2 / 100, time2 % 100);// send abort fun1
        }

    }
#endif  //SDIO_TEST
    do {
        //single write test
        EMMC_TRACE(1, "------single write start, total size:%u, write size:%u, cnt:1", size, size);
#ifdef EMMC_WRITE_TRIGGER_ENABLE
        hal_gpio_pin_set(TRIGGER_GPIO);
#endif
        timeb = hal_fast_sys_timer_get();
#if defined(EMMC_TEST_BLOCKING_MODE)
        ret = write_card_blocking_mode(host, cfg->card_offset, (void *)src, size);
#elif defined(EMMC_TEST_DMA_MODE)
#ifdef EMMC_ADMA_DESC_MODE
#define DATA_SIZE   1024
        uint8_t desc_cnt = size / DATA_SIZE;
        if (!desc_cnt) {
            desc_cnt = 1;
        }
        for (i = 0; i < desc_cnt; i++) {
            if (!i) {
                if (desc_cnt == 1) {
                    if ((size > DATA_SIZE) && (size % DATA_SIZE)) {
                        mmc_adma_init_desc(host, (void *)((uint32_t)src + DATA_SIZE * i), DATA_SIZE, (void *)((uint32_t)src + DATA_SIZE * (i + 1)));
                        mmc_adma_init_desc(host, (void *)((uint32_t)src + DATA_SIZE * (i + 1)), size % DATA_SIZE, NULL);
                    } else {
                        mmc_adma_init_desc(host, src, size, NULL);
                    }
                } else {
                    mmc_adma_init_desc(host, src, DATA_SIZE, (void *)((uint32_t)src + DATA_SIZE * (i + 1)));
                }
            } else if (i < desc_cnt - 1) {
                mmc_adma_init_desc(host, (void *)((uint32_t)src + DATA_SIZE * i), DATA_SIZE, (void *)((uint32_t)src + DATA_SIZE * (i + 1)));
            } else if (i == desc_cnt - 1) {
                if (size % DATA_SIZE) {
                    mmc_adma_init_desc(host, (void *)((uint32_t)src + DATA_SIZE * i), DATA_SIZE, (void *)((uint32_t)src + DATA_SIZE * (i + 1)));
                    mmc_adma_init_desc(host, (void *)((uint32_t)src + DATA_SIZE * (i + 1)), size % DATA_SIZE, NULL);
                } else {
                    mmc_adma_init_desc(host, (void *)((uint32_t)src + DATA_SIZE * i), DATA_SIZE, NULL);
                }
            }
        }
#endif
        ret = write_card_dma_mode(host, cfg->card_offset, (void *)src, size);
#ifdef EMMC_CALLBACK_ENABLE
        if (error_flag) {
            error_flag = 0;

            retry_cnt++;
            if (retry_cnt > RETRY_CNT_MAX) {
                goto write_error;
            }

            EMMC_TRACE(2, "******line: %d,tx/rx error, try again: %d", __LINE__, retry_cnt);
            host->delay(1000);
            EMMC_TRACE(0, "  ");
            continue;
        } else {
            while (!dma_done_flag) {
            }
            dma_done_flag = 0;
        }
#endif
#else
        ret = write_card(host, cfg->card_offset, (void *)src, size);
#endif

#ifdef EMMC_SLT_FLAG_SET_ENABLE
        hal_cmu_simu_tag(EMMC_TEST_TAG_SINGLE_WR_OK);
#endif
#if defined(EMMC_TEST_DMA_MODE) && defined(EMMC_CALLBACK_ENABLE)
write_error:
#endif
        time[1] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
#ifdef EMMC_WRITE_TRIGGER_ENABLE
        hal_gpio_pin_clr(TRIGGER_GPIO);
#endif
        //multi write test
        EMMC_TRACE(1, "------multi write start,  total size:%uMB, write size:%d, cnt:%d", MULTI_TEST_SIZE / 1024 / 1024, size, MULTI_TEST_CNT);
        timeb = hal_fast_sys_timer_get();
        for (i = 0; i < MULTI_TEST_CNT - 1; i++) {
            start_addr = cfg->card_offset + i * size / host->blksz;
            //EMMC_TRACE(1, "------write addr:0x%X, size:%d, i:%d", start_addr, size, i);
#if defined(EMMC_TEST_BLOCKING_MODE)
            ret = write_card_blocking_mode(host, start_addr, (void *)src, size);
#elif defined(EMMC_TEST_DMA_MODE)
#ifdef EMMC_ADMA_DESC_MODE
            //There is no need to configure here, because the above configuration will not change, you can use it directly
#endif
            ret = write_card_dma_mode(host, start_addr, (void *)src, size);
#ifdef EMMC_CALLBACK_ENABLE
            if (error_flag) {
                error_flag = 0;

                retry_cnt++;
                if (retry_cnt > RETRY_CNT_MAX) {
                    break;
                }

                EMMC_TRACE(2, "******line: %d,tx/rx error, try again: %d", __LINE__, retry_cnt);
                host->delay(1000);
                EMMC_TRACE(0, "  ");
                continue;
            } else {
                while (!dma_done_flag) {
                }
                dma_done_flag = 0;
            }
#endif
#else
            ret = write_card(host, start_addr, (void *)src, size);
#endif
        }
        if (MULTI_TEST_CNT > 1) {
            time[2] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
        } else {
            time[2] = 0;
        }

        if (ret != MMC_NO_ERR) {
#ifdef SAVE_RESULT_FOR_SIMU
            HW_WRITE_REG(EMMC_BASE + RESULT_ADDR_WRITE, RESULT_WRITE_FAIL);
#endif
            EMMC_TRACE(1, "******write card failed, ret=%d", ret);
        } else {
#ifdef SAVE_RESULT_FOR_SIMU
            HW_WRITE_REG(EMMC_BASE + RESULT_ADDR_WRITE, RESULT_WRITE_PASS);
#endif
            EMMC_TRACE(1, "------write card complete");
            break;
        }

        if (ret != MMC_NO_ERR) {
            EMMC_TRACE(1, "******halt for write card error");
            while (1);
        }
    } while (1);
#ifdef EMMC_SLT_FLAG_SET_ENABLE
    hal_cmu_simu_tag(EMMC_TEST_TAG_MULTI_WR_OK);
#endif

    EMMC_TRACE(0, "  ");
    card_status = mmc_card_status(host);
    EMMC_TRACE(0, "------card before sleep, card current state:0x%x", card_status);
    card_status = mmc_card_raw_status(host);
    EMMC_TRACE(0, "------card before sleep, card raw status   :0x%x", card_status);
    sleep_card(host);
    card_status = mmc_card_sleep_status(host);
    EMMC_TRACE(0, "------card sleep status                    :%d(%s)", card_status, card_status ? "sleep" : "awake");
    host->delay(10);
    awake_card(host);
    card_status = mmc_card_status(host);
    EMMC_TRACE(0, "------card after awake,  card current state:0x%x", card_status);
    card_status = mmc_card_raw_status(host);
    EMMC_TRACE(0, "------card after awake,  card raw status   :0x%x", card_status);
    card_status = mmc_card_sleep_status(host);
    EMMC_TRACE(0, "------card sleep status                    :%d(%s)", card_status, card_status ? "sleep" : "awake");

    EMMC_TRACE(0, "  ");
    EMMC_TRACE(1, "=================> reading card");
    EMMC_TRACE(1, "------card offset=0x%x, buf=0x%x, buf sz=%u", cfg->card_offset, (uint32_t)dst, size);
    retry_cnt = 0;
    error_flag = 0;
#ifdef EMMC_CALLBACK_ENABLE
    dma_done_flag = 0;
#endif
    do {
        //single read test
        EMMC_TRACE(1, "------single read start, total size:%u, read size:%u, cnt:1", size, size);
#ifdef EMMC_WRITE_TRIGGER_ENABLE
        hal_gpio_pin_set(TRIGGER_GPIO);
#endif
        timeb = hal_fast_sys_timer_get();
#if defined(EMMC_TEST_BLOCKING_MODE)
        ret = read_card_blocking_mode(host, cfg->card_offset, (void *)dst, size);
#elif defined(EMMC_TEST_DMA_MODE)
#ifdef EMMC_ADMA_DESC_MODE
        mmc_adma_init_desc(host, dst, size, NULL);
#endif
        ret = read_card_dma_mode(host, cfg->card_offset, (void *)dst, size);
#ifdef EMMC_CALLBACK_ENABLE
        if (error_flag) {
            error_flag = 0;

            retry_cnt++;
            if (retry_cnt > RETRY_CNT_MAX) {
                goto read_error;
            }

            EMMC_TRACE(2, "******line: %d,tx/rx error, try again: %d", __LINE__, retry_cnt);
            host->delay(1000);
            EMMC_TRACE(0, "  ");
            continue;
        } else {
            while (!dma_done_flag) {
            }
            dma_done_flag = 0;
        }
#endif
#else
        ret = read_card(host, cfg->card_offset, (void *)dst, size);
#endif

#ifdef EMMC_SLT_FLAG_SET_ENABLE
        hal_cmu_simu_tag(EMMC_TEST_TAG_SINGLE_RD_OK);
#endif
#if defined(EMMC_TEST_DMA_MODE) && defined(EMMC_CALLBACK_ENABLE)
read_error:
#endif
        retry_cnt = 0;
        error_flag = 0;
#ifdef EMMC_CALLBACK_ENABLE
        dma_done_flag = 0;
#endif
        time[3] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
#ifdef EMMC_WRITE_TRIGGER_ENABLE
        hal_gpio_pin_clr(TRIGGER_GPIO);
#endif
        //multi read test
        EMMC_TRACE(1, "------multi read start,  total size:%uMB, read size:%d, cnt:%d", MULTI_TEST_SIZE / 1024 / 1024, size, MULTI_TEST_CNT);
        timeb = hal_fast_sys_timer_get();
        for (i = 0; i < MULTI_TEST_CNT - 1; i++) {
            start_addr = cfg->card_offset + i * size / host->blksz;
#if defined(EMMC_TEST_BLOCKING_MODE)
            ret = read_card_blocking_mode(host, start_addr, (void *)dst, size);
#elif defined(EMMC_TEST_DMA_MODE)
#ifdef EMMC_ADMA_DESC_MODE
            mmc_adma_init_desc(host, dst, size, NULL);
#endif
            ret = read_card_dma_mode(host, start_addr, (void *)dst, size);
#ifdef EMMC_CALLBACK_ENABLE
            if (error_flag) {
                error_flag = 0;

                retry_cnt++;
                if (retry_cnt > RETRY_CNT_MAX) {
                    break;
                }

                EMMC_TRACE(2, "******line: %d,tx/rx error, try again: %d", __LINE__, retry_cnt);
                host->delay(1000);
                EMMC_TRACE(0, "  ");
                continue;
            } else {
                while (!dma_done_flag) {
                }
                dma_done_flag = 0;
            }
#endif
#else
            ret = read_card(host, start_addr, (void *)dst, size);
#endif
        }
        if (MULTI_TEST_CNT > 1) {
            time[4] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
        } else {
            time[4] = 0;
        }

        if (ret != MMC_NO_ERR) {
#ifdef SAVE_RESULT_FOR_SIMU
            HW_WRITE_REG(EMMC_BASE + RESULT_ADDR_READ, RESULT_READ_FAIL);
#endif
            EMMC_TRACE(1, "******read card failed, ret=%d", ret);
#ifdef EMMC_TEST_DUMP_DATA
            emmc_dump32("dump2 dst", dst, len);
#endif
        } else {
#ifdef SAVE_RESULT_FOR_SIMU
            HW_WRITE_REG(EMMC_BASE + RESULT_ADDR_READ, RESULT_READ_PASS);
#endif
            EMMC_TRACE(1, "------read card complete");
            break;
        }

        if (ret != MMC_NO_ERR) {
            EMMC_TRACE(1, "******halt for read card error");
            while (1);
        }
    } while (1);
#ifdef EMMC_SLT_FLAG_SET_ENABLE
    hal_cmu_simu_tag(EMMC_TEST_TAG_MULTI_RD_OK);
#endif

    EMMC_TRACE(0, "  ");
    EMMC_TRACE(1, "=================> checking card data");
    do {
        timeb = hal_fast_sys_timer_get();
        ret = memcmp((uint32_t *)src, (uint32_t *)dst, len);
        time[5] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
        if (ret != MMC_NO_ERR) {
            EMMC_TRACE(1, "******check data failed, ret=%d, line=%d", ret, __LINE__);
            emmc_dump32("dump3 src", src, len);
            emmc_dump32("dump3 dst", dst, len);
            EMMC_TRACE(1, "******halt for check card data error");
            while (1);
        }

        timeb = hal_fast_sys_timer_get();
        for (i = 0; i < MULTI_TEST_CNT - 1; i++) {
            start_addr = cfg->card_offset + i * size / host->blksz;
#if defined(EMMC_TEST_BLOCKING_MODE)
            ret = read_card_blocking_mode(host, start_addr, (void *)dst, size);
#elif defined(EMMC_TEST_DMA_MODE)
#ifdef EMMC_ADMA_DESC_MODE
            mmc_adma_init_desc(host, dst, size, NULL);
#endif
            ret = read_card_dma_mode(host, start_addr, (void *)dst, size);
#ifdef EMMC_CALLBACK_ENABLE
            if (error_flag) {
                error_flag = 0;

                retry_cnt++;
                if (retry_cnt > RETRY_CNT_MAX) {
                    break;
                }

                EMMC_TRACE(2, "******line: %d,tx/rx error, try again: %d", __LINE__, retry_cnt);
                host->delay(1000);
                EMMC_TRACE(0, "  ");
                continue;
            } else {
                while (!dma_done_flag) {
                }
                dma_done_flag = 0;
            }
#endif
#else
            ret = read_card(host, start_addr, (void *)dst, size);
#endif
            ret = memcmp((uint32_t *)src, (uint32_t *)dst, len);
            if (ret != MMC_NO_ERR) {
                EMMC_TRACE(1, "******check data failed, ret=%d, line=%d", ret, __LINE__);
                emmc_dump32("dump4 src", src, len);
                emmc_dump32("dump4 dst", dst, len);
                EMMC_TRACE(1, "******halt for check card data error");
                while (1);
            }
        }
        if (MULTI_TEST_CNT > 1) {
            time[6] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
        } else {
            time[6] = 0;
        }
        EMMC_TRACE(1, "=================$ data check is OK!!!");

        TRACE_FLUSH();
        if (!ret) {
            //calc speed
            EMMC_TRACE(1, "  ");
            EMMC_TRACE(1, "==========read and write success==========");
            EMMC_TRACE(1, "******************************************");
            EMMC_TRACE(1, "TIME COST        : cfg        [%uus]", time[0]);
            EMMC_TRACE(1, "TIME COST        : read       [%uus]", time[3]);
            EMMC_TRACE(1, "TIME COST        : multi read [%uus]", time[4]);
            EMMC_TRACE(1, "TIME COST        : write      [%uus]", time[1]);
            EMMC_TRACE(1, "TIME COST        : multi write[%uus]", time[2]);
            EMMC_TRACE(1, "TIME COST        : cmp        [%uus]", time[5]);
            EMMC_TRACE(1, "TIME COST        : multi cmp  [%uus]", time[6]);
            EMMC_TRACE(1, "  ");
            EMMC_TRACE(1, "******************************************");
            EMMC_TRACE(1, "read size        :%ubytes", size);
            val = (uint64_t)size * MULTI_TEST_CNT / 1024;
            EMMC_TRACE(1, "multi read size  :%uKbytes=%uMbytes", (uint32_t)val, (uint32_t)(val / 1024));
            EMMC_TRACE(1, "write size       :%ubytes", size);
            EMMC_TRACE(1, "multi write size :%uKbytes=%uMbytes", (uint32_t)val, (uint32_t)(val / 1024));
            EMMC_TRACE(1, "cmp size         :%ubytes", size);
            EMMC_TRACE(1, "multi cmp size   :%uKbytes=%uMbytes", (uint32_t)val, (uint32_t)(val / 1024));
            EMMC_TRACE(1, "  ");

            //speed=MB/s
            //     =size/1024/1024MB/(time*10^-6)s
            //     =size/1024/1024*1000000/time (MB/s)
            //     =size*1000000/1024/1024/time (MB/s)
            //     =size*0.954/time (MB/s)
            //     =size*954/1000/time (MB/s)
            //     =size*95400/1000/time (MB/s) //The result is magnified 100 times and used to display 2 decimal places
            time[3] = (uint32_t)((uint64_t)size * 95400 / 1000 / time[3]);                  //read
            if (time[4]) {
                time[4] = (uint32_t)((uint64_t)size * 95400 * MULTI_TEST_CNT / 1000 / time[4]); //multi read
            } else {
                time[4] = time[3];//multi time = single time
            }
            time[1] = (uint32_t)((uint64_t)size * 95400 / 1000 / time[1]);                  //write
            if (time[2]) {
                time[2] = (uint32_t)((uint64_t)size * 95400 * MULTI_TEST_CNT / 1000 / time[2]); //multi write
            } else {
                time[2] = time[1];//multi time = single time
            }

            EMMC_TRACE(1, "emmc cache       :%s", card_info.cache_ctrl ? "ON" : "OFF");
            EMMC_TRACE(1, "  ");
            EMMC_TRACE(1, "read speed       :%u.%uMB/s", time[3] / 100, time[3] % 100);
            EMMC_TRACE(1, "multi read speed :%u.%uMB/s", time[4] / 100, time[4] % 100);
            EMMC_TRACE(1, "write speed      :%u.%uMB/s", time[1] / 100, time[1] % 100);
            EMMC_TRACE(1, "multi write speed:%u.%uMB/s", time[2] / 100, time[2] % 100);
            EMMC_TRACE(1, "******************************************");
            EMMC_TRACE(1, "  ");
#ifdef EMMC_SLT_FLAG_SET_ENABLE
            hal_cmu_simu_tag(EMMC_TEST_TAG_DATA_CHECK_OK);
#endif
            break;
        } else {
            EMMC_TRACE(1, "******check data failed, ret=%d", ret);

            emmc_dump32("dump5 src", src, len);
            emmc_dump32("dump5 dst", dst, len);
        }

        if (ret != MMC_NO_ERR) {
            EMMC_TRACE(1, "******halt for check card data error");
            while (1);
        }
    } while (1);

_exit:
#ifdef EMMC_USE_HEAP
    emmc_heap_free(src_mem);
    emmc_heap_free(dst_mem);
#endif

    return ret;
}

static int sdhci_data_xfer_clean(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    EMMC_TRACE(1, "%s", __FUNCTION__);
    EMMC_TRACE(1, "SDHCI DATA transfer clean");

    return ret;
}

static int mmc_test_start(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;
    struct emmc_config *cfg;

    EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);

    cfg = (struct emmc_config *)host->private;
    switch (cfg->test_id) {
        case SDHCI_DATA_XFER_TEST: {
            EMMC_TRACE(1, "SDHCI DATA transfer test");
            ret = sdhci_data_xfer_setup(host);
            if (ret != MMC_NO_ERR) {
                return ret;
            }
            ret = sdhci_data_xfer_test(host, true);
            if (ret != MMC_NO_ERR) {
                return ret;
            }
            ret = sdhci_data_xfer_clean(host);
            if (ret != MMC_NO_ERR) {
                return ret;
            }
            break;
        }
        case SDHCI_STRESS_TEST: {
            uint32_t run_cnt = 0;
            EMMC_TRACE(1, "SDHCI DATA stress test");

#if defined(EMMC_CORE_SLEEP_POWER_DOWN_TEST) && defined(RTOS)
            uint16_t random_delay;

            //Must be turned off, otherwise this function cannot be tested during sleep wake-up
            host->clk_gating_en = 0;

            ret = sdhci_data_xfer_setup(host);
            if (ret != MMC_NO_ERR) {
                return ret;
            }

            hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
            if (host->speed_mode == HS400 || host->speed_mode == EHS400) {
                hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_208M);
            } else {
                hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_104M);
            }
            while (1) {
                ret = sdhci_data_xfer_test(host, true);
                if (ret != MMC_NO_ERR) {
                    return ret;
                }

                EMMC_TRACE(0, "  ");
                EMMC_TRACE(0, "---emmc phy sleep");
                sdhci_host_emphy_sleep(host);
                random_delay = rand() % (1000 - 1 + 1) + 1;//[1 ~ 1000] ms,[a,b]=rand()%(b-a+1)+a
                EMMC_TRACE(0, "---mcu sleep:%dms", random_delay);
                hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_32K);
#ifdef RTOS
                host->delay(random_delay);//sleep
#else
                hal_sleep_enter_sleep();
#endif
                if (host->speed_mode == HS400 || host->speed_mode == EHS400) {
                    hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_208M);
                } else {
                    hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_104M);
                }
                EMMC_TRACE(0, "---mcu wakeup");
                EMMC_TRACE(1, "---cpu freq:%uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
                sdhci_host_emphy_wakeup(host);
                EMMC_TRACE(0, "---emmc phy wakeup");

                random_delay = rand() % (10 - 1 + 1) + 1;//[1 ~ 10] ms,[a,b]=rand()%(b-a+1)+a
#ifdef RTOS
                host->delay(random_delay);
#else
                hal_sleep_enter_sleep();
#endif
                EMMC_TRACE(0, "---wait for %dms before reading and writing", random_delay);

                run_cnt++;
                EMMC_TRACE(0, "-------------------------------->>>>>>>>>>>>>>>>>run cnt:%d", run_cnt);
                EMMC_TRACE(0, "  ");
                EMMC_TRACE(0, "  ");
            }
#else
            ret = sdhci_data_xfer_setup(host);
            if (ret != MMC_NO_ERR) {
                return ret;
            }
            while (1) {
                ret = sdhci_data_xfer_test(host, true);
                if (ret != MMC_NO_ERR) {
                    return ret;
                }

                EMMC_TRACE(0, "---before sleep, card current state:0x%x", mmc_card_status(host));
                EMMC_TRACE(0, "---before sleep, card raw status   :0x%x", mmc_card_raw_status(host));
                sleep_card(host);
                EMMC_TRACE(0, "---after sleep,  card status       :sleep");

                EMMC_TRACE(0, "---emmc phy sleep");
                sdhci_host_emphy_sleep(host);

                EMMC_TRACE(0, "  ");
                EMMC_TRACE(0, "******It is now possible to measure the current after emmc+phy sleep******");
                EMMC_TRACE(0, "You have 5 seconds to measure");
                EMMC_TRACE(0, "  ");

                host->delay(5000);

                sdhci_host_emphy_wakeup(host);
                host->delay(2);
                EMMC_TRACE(0, "---emmc phy wakeup");

                EMMC_TRACE(0, "---before awake, card status       :sleep");
                awake_card(host);
                EMMC_TRACE(0, "---after awake,  card current state:0x%x", mmc_card_status(host));
                EMMC_TRACE(0, "---after awake,  card raw status   :0x%x", mmc_card_raw_status(host));

                run_cnt++;
                EMMC_TRACE(0, "-------------------------------->>>>>>>>>>>>>>>>>run cnt:%d", run_cnt);
                EMMC_TRACE(0, "  ");
                EMMC_TRACE(0, "  ");
                host->delay(100);
            }
#endif
        }
        default: {
            EMMC_TRACE(1, "%s:%d,emmc test id error", __FUNCTION__, __LINE__);
            ret = MMC_INVALID_ERR;
            break;
        }
    }
    return ret;
}

int emmc_sleep_pd_test(void)
{
    return sdhci_data_xfer_test(&emmc_test_op, 0);
}

static int sdhci_emmc_read_write(struct mmc_host *host, uint32_t addr, const uint8_t *buf, uint32_t buf_len, uint8_t read0_write1)
{
    int ret = MMC_NO_ERR;
    POSSIBLY_UNUSED uint8_t retry_cnt;
    uint32_t size = buf_len;
    POSSIBLY_UNUSED uint32_t len = size / sizeof(uint32_t);
    uint32_t *pdata = (uint32_t *)buf;
    POSSIBLY_UNUSED uint32_t timeb, time = 0xffffffff, time_split[2] = {0};
    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;
    cfg->card_offset = addr;

    EMMC_TRACE(0, "  ");
    EMMC_TRACE(1, "emmc blk addr=0x%x, src/dst=0x%x, len=%d, %s", addr, (uint32_t)pdata, size, read0_write1 ? "write" : "read");
    if (read0_write1) {
        EMMC_TRACE(0, "=================> writing card");
        EMMC_TRACE(1, "------card_offset=0x%x, buf=0x%x, buf sz=%d", cfg->card_offset, (uint32_t)pdata, size);

        time_split[0] = 0;
        time_split[1] = 0;
        retry_cnt = 0;
        error_flag = 0;
#ifdef EMMC_CALLBACK_ENABLE
        dma_done_flag = 0;
#endif
        do {
            //single write test
            EMMC_TRACE(1, "------single write start, total size:%u, write size:%u, cnt:1", size, size);
#ifdef EMMC_WRITE_TRIGGER_ENABLE
            hal_gpio_pin_set(TRIGGER_GPIO);
#endif
            timeb = hal_fast_sys_timer_get();
#if defined(EMMC_TEST_BLOCKING_MODE)
            ret = write_card_blocking_mode(host, cfg->card_offset, (void *)pdata, size);
#elif defined(EMMC_TEST_DMA_MODE)
            ret = write_card_dma_mode(host, cfg->card_offset, (void *)pdata, size);
#ifdef EMMC_CALLBACK_ENABLE
            if (error_flag) {
                error_flag = 0;

                retry_cnt++;
                if (retry_cnt > RETRY_CNT_MAX) {
                    goto write_error;
                }

                EMMC_TRACE(2, "******line: %d,tx/rx error, try again: %d", __LINE__, retry_cnt);
                host->delay(1000);
                EMMC_TRACE(0, "  ");
                continue;
            } else {
                time_split[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
                while (!dma_done_flag) {
                }
                time_split[1] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb) - time_split[0];
                dma_done_flag = 0;
            }
#endif
#else
            ret = write_card(host, cfg->card_offset, (void *)pdata, size);
#endif

#if defined(EMMC_TEST_DMA_MODE) && defined(EMMC_CALLBACK_ENABLE)
write_error:
#endif
            time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
#ifdef EMMC_WRITE_TRIGGER_ENABLE
            hal_gpio_pin_clr(TRIGGER_GPIO);
#endif
            if (ret != MMC_NO_ERR) {
#ifdef SAVE_RESULT_FOR_SIMU
                HW_WRITE_REG(EMMC_BASE + RESULT_ADDR_WRITE, RESULT_WRITE_FAIL);
#endif
                EMMC_TRACE(1, "******write card failed, ret=%d", ret);
            } else {
#ifdef SAVE_RESULT_FOR_SIMU
                HW_WRITE_REG(EMMC_BASE + RESULT_ADDR_WRITE, RESULT_WRITE_PASS);
#endif
                EMMC_TRACE(1, "------write card complete");
                break;
            }

            if (ret != MMC_NO_ERR) {
                EMMC_TRACE(1, "******halt for write card error");
                while (1);
            }
        } while (1);

        EMMC_TRACE(1, "******************************************");
        EMMC_TRACE(1, "write size     :%dbytes", size);
        EMMC_TRACE(1, "TIME COST      :write[%dus]", time);
        time = (uint32_t)(95400 * size / 1000 / time);
        EMMC_TRACE(1, "write speed    :%u.%uMB/s", time / 100, time % 100);
        EMMC_TRACE(1, "  ");
        if (time_split[0]) {
            EMMC_TRACE(1, "TIME COST      :cmd+resp  [%dus]", time_split[0]);
        }
        if (time_split[1]) {
            EMMC_TRACE(1, "TIME COST      :write data[%dus]", time_split[1]);
        }
        EMMC_TRACE(1, "******************************************");
        EMMC_TRACE(1, "  ");
    }

    if (!read0_write1) {
        EMMC_TRACE(1, "=================> reading card");
        EMMC_TRACE(1, "------card_offset=0x%x, buf=0x%x, buf sz=%d", cfg->card_offset, (uint32_t)pdata, size);

        time_split[0] = 0;
        time_split[1] = 0;
        retry_cnt = 0;
        error_flag = 0;
#ifdef EMMC_CALLBACK_ENABLE
        dma_done_flag = 0;
#endif
        do {
            //single write test
            EMMC_TRACE(1, "------single read start, total size:%u, read size:%u, cnt:1", size, size);
#ifdef EMMC_WRITE_TRIGGER_ENABLE
            hal_gpio_pin_set(TRIGGER_GPIO);
#endif
            timeb = hal_fast_sys_timer_get();
#if defined(EMMC_TEST_BLOCKING_MODE)
            ret = read_card_blocking_mode(host, cfg->card_offset, (void *)pdata, size);
#elif defined(EMMC_TEST_DMA_MODE)
            ret = read_card_dma_mode(host, cfg->card_offset, (void *)pdata, size);
#ifdef EMMC_CALLBACK_ENABLE
            if (error_flag) {
                error_flag = 0;

                retry_cnt++;
                if (retry_cnt > RETRY_CNT_MAX) {
                    goto read_error;
                }

                EMMC_TRACE(2, "******line: %d,tx/rx error, try again: %d", __LINE__, retry_cnt);
                host->delay(1000);
                EMMC_TRACE(0, "  ");
                continue;
            } else {
                time_split[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
                while (!dma_done_flag) {
                }
                time_split[1] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb) - time_split[0];
                dma_done_flag = 0;
            }
#endif
#else
            ret = read_card(host, cfg->card_offset, (void *)pdata, size);
#endif

#if defined(EMMC_TEST_DMA_MODE) && defined(EMMC_CALLBACK_ENABLE)
read_error:
#endif
            time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
#ifdef EMMC_WRITE_TRIGGER_ENABLE
            hal_gpio_pin_clr(TRIGGER_GPIO);
#endif
            if (ret != MMC_NO_ERR) {
#ifdef SAVE_RESULT_FOR_SIMU
                HW_WRITE_REG(EMMC_BASE + RESULT_ADDR_READ, RESULT_READ_FAIL);
#endif
                EMMC_TRACE(1, "******read card failed, ret=%d", ret);
#ifdef EMMC_TEST_DUMP_DATA
                emmc_dump32("dump2 dst", pdata, len);
#endif
            } else {
#ifdef SAVE_RESULT_FOR_SIMU
                HW_WRITE_REG(EMMC_BASE + RESULT_ADDR_READ, RESULT_READ_PASS);
#endif
                EMMC_TRACE(1, "------read card complete");
                break;
            }

            if (ret != MMC_NO_ERR) {
                EMMC_TRACE(1, "******halt for read card error");
                while (1);
            }
        } while (1);

        EMMC_TRACE(1, "******************************************");
        EMMC_TRACE(1, "read size      :%dbytes", size);
        EMMC_TRACE(1, "TIME COST      :read[%dus]", time);
        time = (uint32_t)(95400 * size / 1000 / time);
        EMMC_TRACE(1, "read speed     :%u.%uMB/s", time / 100, time % 100);
        EMMC_TRACE(1, "  ");
        if (time_split[0]) {
            EMMC_TRACE(1, "TIME COST      :cmd+resp [%dus]", time_split[0]);
        }
        if (time_split[1]) {
            EMMC_TRACE(1, "TIME COST      :read data[%dus]", time_split[1]);
        }
        EMMC_TRACE(1, "******************************************");
        EMMC_TRACE(1, "  ");
    }

    return ret;
}

int emmc_read_write(uint32_t addr, const uint8_t *buf, uint32_t buf_len, uint8_t read0_write1)
{
    struct mmc_host *host = &emmc_test_op;

    return sdhci_emmc_read_write(host, addr, buf, buf_len, read0_write1);
}

#ifndef EMMC_USE_HEAP
#ifdef EMMC_RPMB_ENABLE
int emmc_card_rpmb_test(void)
{
    int ret = MMC_NO_ERR;
    uint32_t result = 0;
    struct mmc_host *host = &emmc_test_op;
    uint32_t *src_mem = (uint32_t *)emmc_src_buf;
    uint32_t *dst_mem = (uint32_t *)emmc_dst_buf;
    uint32_t *src = (uint32_t *)src_mem;
    uint32_t *dst = (uint32_t *)dst_mem;

    EMMC_TRACE(1, "%s: src=%x, dst=%x", __FUNCTION__, (uint32_t)src, (uint32_t)dst);
    memset(src, 0x55, 512);
    memset(dst, 0x0, 512);

    EMMC_TRACE(1, " ");
    EMMC_TRACE(1, "===========================================================");
    EMMC_TRACE(1, "eMMC5.1/SD3.0/SDIO3.0/CE-ATA1.x (SD Host) Controller Module");
    EMMC_TRACE(1, "===========================================================");

    memset((void *)host, 0, sizeof(struct mmc_host));
    host->id       = emmc_cfg.controller;
    host->dma_mode = emmc_cfg.dma_mode;
    host->auto_cmd_type = emmc_cfg.auto_cmd_type;
    host->dma_bdry  = emmc_cfg.dma_bdry;
    host->desc_sz   = emmc_cfg.desc_sz;
    host->desc_nr   = emmc_cfg.desc_nr;
    host->desc_type = emmc_cfg.desc_type;
    host->blksz     = emmc_cfg.blksz;
    host->blks      = emmc_cfg.blks;
    host->quirks    = emmc_cfg.quirks;
    host->strobe    = emmc_cfg.strobe;
    host->vmin_begin = emmc_cfg.vmin_begin;
    host->speed_mode = emmc_cfg.timing;
    host->delay      = emmc_cfg.delay;
    host->clk_gating_en     = emmc_cfg.clk_gating_en;
    host->resp_polling_mode = emmc_cfg.resp_polling_mode;
    host->emmc_host_error   = emmc_cfg.emmc_host_error;
    host->emmc_host_dma_done = emmc_cfg.emmc_host_dma_done;
    host->private    = (void *)&emmc_cfg;
    host->error_code = MMC_NO_ERR;

    EMMC_TRACE(1, "SDHCI DATA transfer test");
    ret = sdhci_data_xfer_setup(host);
    if (ret != MMC_NO_ERR) {
        return ret;
    }

    //emmc switch to rpmb
    ret = mmc_switch_to_rpmb(host, EMMC_RPMB_PART);
    if (ret != MMC_NO_ERR) {
        EMMC_TRACE(1, "emmc switch to rpmb error,ret = %d", ret);
        return ret;
    }

    //emmc set key
    const uint8_t key[32] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32};
    ret = mmc_rpmb_set_key(host, (uint8_t *)key, &result);
    if (ret != MMC_NO_ERR || result != 0) {
        EMMC_TRACE(1, "emmc set rpmb key error,ret = %d, result = 0x%x", ret, result);
        // return ret;
    }

    //emmc get write counter
    uint32_t write_counter;
    ret = mmc_rpmb_get_counter(host, &write_counter, &result);
    if (ret != MMC_NO_ERR || result != 0) {
        EMMC_TRACE(1, "emmc get counter to rpmb error,ret = %d, result = 0x%x", ret, result);
        return ret;
    }

    //emmc write data to rpmb
    ret = mmc_rpmb_write(host, write_counter, 0x0, src, 512, key, &result);
    if (ret != MMC_NO_ERR || result != 0) {
        EMMC_TRACE(1, "emmc write to rpmb error,ret = %d, result = 0x%x", ret, result);
        return ret;
    }

    //emmc read data from rpmb
    ret = mmc_rpmb_read(host, 0x0, dst, 512, &result);
    if (ret != MMC_NO_ERR || result != 0) {
        EMMC_TRACE(1, "emmc read to rpmb error,ret = %d, result = 0x%x", ret, result);
        return ret;
    }

    if (memcmp(src, dst, 512) != 0) {
        EMMC_TRACE(1, "emmc read to rpmb error");
        return 1;
    }

    //emmc switch to general partition
    ret = mmc_switch_to_rpmb(host, NO_ACCESS_BOOT_DEFAULT);
    if (ret != MMC_NO_ERR) {
        EMMC_TRACE(1, "emmc switch to rpmb error,ret = %d", ret);
        return ret;
    }

    return 0;
}
#endif
#endif

int emmc_card_test(int cmd)
{
    int ret = MMC_NO_ERR;
    struct mmc_host *host = &emmc_test_op;

    EMMC_TRACE(1, " ");
    EMMC_TRACE(1, "===========================================================");
    EMMC_TRACE(1, "eMMC5.1/SD3.0/SDIO3.0/CE-ATA1.x (SD Host) Controller Module");
    EMMC_TRACE(1, "===========================================================");

    memset((void *)host, 0, sizeof(struct mmc_host));
    host->id       = emmc_cfg.controller;
    host->dma_mode = emmc_cfg.dma_mode;
    host->auto_cmd_type = emmc_cfg.auto_cmd_type;
    host->dma_bdry  = emmc_cfg.dma_bdry;
    host->desc_sz   = emmc_cfg.desc_sz;
    host->desc_nr   = emmc_cfg.desc_nr;
    host->desc_type = emmc_cfg.desc_type;
    host->blksz     = emmc_cfg.blksz;
    host->blks      = emmc_cfg.blks;
    host->quirks    = emmc_cfg.quirks;
    host->strobe    = emmc_cfg.strobe;
    host->vmin_begin = emmc_cfg.vmin_begin;
    host->speed_mode = emmc_cfg.timing;
    host->delay      = emmc_cfg.delay;
    host->clk_gating_en     = emmc_cfg.clk_gating_en;
    host->resp_polling_mode = emmc_cfg.resp_polling_mode;
    host->emmc_host_error   = emmc_cfg.emmc_host_error;
    host->emmc_host_dma_done = emmc_cfg.emmc_host_dma_done;
    host->private    = (void *)&emmc_cfg;
    host->error_code = MMC_NO_ERR;
    /**************/
    //change voltage
    /**************/
    if (host->vmin_begin) {
        switch (emmc_cfg.timing) {
            case DS:
                EMMC_TRACE(1, "EMMC DS mode");
                break;
            case SDR52:
                EMMC_TRACE(1, "EMMC SDR52 mode");
                break;
            case DDR26:
                EMMC_TRACE(1, "EMMC DDR26 mode");
                break;
            case DDR52:
                EMMC_TRACE(1, "EMMC DDR52 mode");
                break;
            case HS25:
                EMMC_TRACE(1, "EMMC HS25 mode");
                break;
            case HS50:
                EMMC_TRACE(1, "EMMC HS50 mode");
                break;
            case HS100:
                EMMC_TRACE(1, "EMMC HS100 mode");
                break;
            case HS200:
                EMMC_TRACE(1, "EMMC HS200 mode");
                break;
            case HS400:
                EMMC_TRACE(1, "EMMC HS400 mode");
                break;
            case EHS400:
                EMMC_TRACE(1, "EMMC EHS400 mode");
                break;
            default:
                break;
        }
    }

    EMMC_TRACE(1, "emmc base: 0x%X", EMMC_BASE);
    EMMC_TRACE(1, " ");
    EMMC_TRACE(1, "    ---------------------");
    EMMC_TRACE(1, "    VOL min begin  : %d(mV)", emmc_cfg.vmin_begin);
    EMMC_TRACE(1, "    ---------------------");
    EMMC_TRACE(1, "    MMC Controller : %d  ", emmc_cfg.controller);
    EMMC_TRACE(1, "    Test Number    : %d  ", emmc_cfg.test_id);
    EMMC_TRACE(1, "    Sub Test #     : %d  ", emmc_cfg.subtest_id);
    EMMC_TRACE(1, "    Real Card      : %s  ", (emmc_cfg.real_card == 1) ? "yes" : "no");
    EMMC_TRACE(1, "    ---------------------");
    EMMC_TRACE(1, "    SRC Memory     : %d  ", emmc_cfg.src_region);
    EMMC_TRACE(1, "    TGT Memory     : %d  ", emmc_cfg.dst_region);
    EMMC_TRACE(1, "    Memory align   : %d  ", emmc_cfg.aligned);
    EMMC_TRACE(1, "    Emmc reset pin : %d  ", emmc_cfg.rst_pin);
    EMMC_TRACE(1, "    Emmc 3.3v pin  : %d  ", emmc_cfg.pwr_3p3_pin);
    EMMC_TRACE(1, "    Emmc 1.8v pin  : %d  ", emmc_cfg.pwr_1p8_pin);
    EMMC_TRACE(1, "    Emmc pwr on lev: %d  ", emmc_cfg.pwr_on_level);
    EMMC_TRACE(1, "    Driver [CLK CMD DAT CDWP] = [%d %d %d]", emmc_cfg.clk_drv, emmc_cfg.cmd_drv, emmc_cfg.dat_drv);
    EMMC_TRACE(1, "    VCC Main       : %d(mV)", emmc_cfg.vcc_main);
    EMMC_TRACE(1, "    -------------------");
    EMMC_TRACE(1, "    Block Size     : %d  ", emmc_cfg.blksz);
    EMMC_TRACE(1, "    Block Count    : %d  ", emmc_cfg.blks);
    EMMC_TRACE(1, "    Card Offset    : 0x%0X", emmc_cfg.card_offset);
    EMMC_TRACE(1, "    Seed           : 0x%0X", emmc_cfg.seed);
    EMMC_TRACE(1, "    Pattern        : 0x%0X", emmc_cfg.pattern);
    EMMC_TRACE(1, "    -------------------");
    EMMC_TRACE(1, "    DMA Mode       : 0x%0X", emmc_cfg.dma_mode);
    EMMC_TRACE(1, "    DMA BDRY       : %d", emmc_cfg.dma_bdry);
    EMMC_TRACE(1, "    Auto CMD Type  : 0x%0X", emmc_cfg.auto_cmd_type);
    EMMC_TRACE(1, "    ADMA Desc Type : 0x%0X", emmc_cfg.desc_type);
    EMMC_TRACE(1, "    ADMA Desc SZ   : 0x%0X", emmc_cfg.desc_sz);
    EMMC_TRACE(1, "    ADMA Desc BR   : 0x%0X ", emmc_cfg.desc_nr);
    EMMC_TRACE(1, "    clock gating   : %d(%s) ", emmc_cfg.clk_gating_en, emmc_cfg.clk_gating_en ? "enable" : "disable");
    EMMC_TRACE(1, "    resp polling   : %d(%s) ", emmc_cfg.resp_polling_mode, emmc_cfg.resp_polling_mode ? "enable" : "disable");
    EMMC_TRACE(1, "    -------------------");
    EMMC_TRACE(1, "    Bus Mode       : 0x%0X", emmc_cfg.timing);
    EMMC_TRACE(1, "    Bus Frequency  : %d(KHz)", emmc_cfg.khz);
    if (emmc_cfg.width == MMC_1BIT_WIDTH) {
        EMMC_TRACE(1, "    Bus Width      : 1bit");
    } else if (emmc_cfg.width == MMC_4BIT_WIDTH) {
        EMMC_TRACE(1, "    Bus Width      : 4bit");
    } else if (emmc_cfg.width == MMC_8BIT_WIDTH) {
        EMMC_TRACE(1, "    Bus Width      : 8bit");
    }
    EMMC_TRACE(1, "    Bus Voltage    : %d(mV)", emmc_cfg.voltage);
    EMMC_TRACE(1, "    Bus strobe     : 0x%0X", emmc_cfg.strobe);
    EMMC_TRACE(1, "    Bus quirks     : 0x%0X", emmc_cfg.quirks);
    EMMC_TRACE(1, "    -------------------");

#ifdef EMMC_CLOCK_OUTPUT
    hal_iomux_set_clock_out();
    hal_cmu_clock_out_enable(HAL_CMU_CLOCK_OUT_SYS_EMMC);
#endif

    srand(hal_sys_timer_get() * hal_fast_sys_timer_get());

#ifdef EMMC_WRITE_TRIGGER_ENABLE
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] = {
        {TRIGGER_GPIO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));
    hal_gpio_pin_set_dir(TRIGGER_GPIO, HAL_GPIO_DIR_OUT, 0);
#endif

    ret = mmc_test_start(host);
    EMMC_TRACE(1, "emmc test error code:%d", host->error_code);
    EMMC_TRACE(1, "emmc test results:%d", ret);

#ifdef EMMC_SINGLE_READ_WRITE_ENABLE
    emmc_read_write(1024, (const uint8_t *)emmc_src_buf, EMMC_XBUF_SIZE, 1);//write test
    emmc_read_write(8192, (const uint8_t *)emmc_dst_buf, EMMC_XBUF_SIZE, 0);//read test
#endif

#ifdef EMMC_CONTINUOUS_READ_ENABLE
    uint32_t pass_cnt = 0;
    uint32_t fail_cnt = 0;
    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    EMMC_TRACE(0, "  ");
    EMMC_TRACE(1, "----------emmc continuous read test start----------");
#ifdef EMMC_RANDOM_WRITE_ENABLE
    uint32_t sector_count;

    sector_count = (uint32_t)(*(uint32_t *)&host->card->ext_csd[212]);//get max sector conut
    EMMC_TRACE(0, "------sector conut[SEC_COUNT]:0x%X", sector_count);

    cfg->card_offset = (uint32_t)((rand() % RAND_MAX) / (double)RAND_MAX * sector_count);
    EMMC_TRACE(0, "------random card offset:0x%X", cfg->card_offset);
    if (cfg->card_offset > sector_count) {
        cfg->card_offset -= 1;
    }
    EMMC_TRACE(0, "------final card offset :0x%X", cfg->card_offset);
#else
    cfg->card_offset = 0;
#endif

    emmc_read_write(cfg->card_offset, (uint8_t *)emmc_src_buf, sizeof(emmc_src_buf), 1);
    while (1) {
        memset(emmc_dst_buf, 0, sizeof(emmc_dst_buf));
        emmc_read_write(cfg->card_offset, (uint8_t *)emmc_dst_buf, sizeof(emmc_dst_buf), 0);
        ret = memcmp((uint32_t *)emmc_dst_buf, (uint32_t *)emmc_src_buf, sizeof(emmc_dst_buf));
        if (ret) {
            fail_cnt++;
            EMMC_TRACE(0, "******************emmc read: compare error, ret:%d, fail cnt:%d, pass cnt:%d", ret, fail_cnt, pass_cnt);
        } else {
            pass_cnt++;
            EMMC_TRACE(0, "++++++++++++++++++emmc writing/reading comparison: Success, fail cnt:%d, pass cnt:%d", fail_cnt, pass_cnt);
        }
        host->delay(200);
    }
#endif

#ifdef EMMC_CONTINUOUS_ONLY_READ_ENABLE
    uint32_t sector_count;
    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    EMMC_TRACE(0, "  ");
    EMMC_TRACE(1, "----------emmc continuous only read test start----------");
    sector_count = (uint32_t)(*(uint32_t *)&host->card->ext_csd[212]);//get max sector conut
    EMMC_TRACE(0, "------sector conut[SEC_COUNT]:0x%X", sector_count);

#ifndef EMMC_RANDOM_WRITE_ENABLE
    cfg->card_offset = (uint32_t)((rand() % RAND_MAX) / (double)RAND_MAX * sector_count);
    if (cfg->card_offset > sector_count) {
        cfg->card_offset -= 1;
    }
#endif
    while (1) {
#ifdef EMMC_RANDOM_WRITE_ENABLE
        cfg->card_offset = (uint32_t)((rand() % RAND_MAX) / (double)RAND_MAX * sector_count);
        EMMC_TRACE(0, "------random card offset:0x%X", cfg->card_offset);
        if (cfg->card_offset > sector_count) {
            cfg->card_offset -= 1;
        }
        EMMC_TRACE(0, "------final card offset :0x%X", cfg->card_offset);
#endif

        emmc_read_write(cfg->card_offset, (uint8_t *)emmc_dst_buf, sizeof(emmc_dst_buf), 0);

#ifndef EMMC_RANDOM_WRITE_ENABLE
        cfg->card_offset += sizeof(emmc_dst_buf);
        if (cfg->card_offset > sector_count) {
            cfg->card_offset = 0;
        }
#endif
        host->delay(200);
    }
#endif

#ifdef EMMC_3V3_PWROFF_TEST
    int err;
    uint32_t pass_cnt = 0;
    uint32_t fail_cnt = 0;
    uint32_t card_status;
    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    EMMC_TRACE(0, "  ");
    EMMC_TRACE(1, "----------emmc 3.3v power-down test----------");
#ifdef EMMC_RANDOM_WRITE_ENABLE
    uint32_t sector_count;

    sector_count = (uint32_t)(*(uint32_t *)&host->card->ext_csd[212]);//get max sector conut
    EMMC_TRACE(0, "------sector conut[SEC_COUNT]:0x%X", sector_count);
#endif

#ifdef EMMC_PWROFF_NOTIFICATION_TEST
    uint8_t pon_value;

    pon_value = host->card->ext_csd[EXT_CSD__PWR_OFF_NOTIFY];
    EMMC_TRACE(0, "------PWR_OFF_NOTIFY before:%d", pon_value);
    mmc_power_off_notification_enable(host);
    err = mmc_send_ext_csd(host);
    if (err) {
        while (1) {
            EMMC_TRACE(0, "******send ext csd err:%d, line:%d", err, __LINE__);
            host->delay(1000);
        }
    }

    pon_value = host->card->ext_csd[EXT_CSD__PWR_OFF_NOTIFY];
    EMMC_TRACE(0, "------PWR_OFF_NOTIFY after :%d", pon_value);
    if (pon_value == 1) {
        EMMC_TRACE(0, "++++++power off notification enable success");
    } else {
        while (1) {
            EMMC_TRACE(0, "******power off notification enable fail");
            host->delay(1000);
        }
    }
#endif
    while (1) {
#ifdef EMMC_RANDOM_WRITE_ENABLE
        cfg->card_offset = (uint32_t)((rand() % RAND_MAX) / (double)RAND_MAX * sector_count);
        EMMC_TRACE(0, "------random card offset:0x%X", cfg->card_offset);
        if (cfg->card_offset > sector_count) {
            cfg->card_offset -= 1;
        }
        EMMC_TRACE(0, "------final card offset :0x%X", cfg->card_offset);
#else
        cfg->card_offset = 0;
#endif
        for (int i = 0; i < sizeof(emmc_src_buf) / sizeof(uint32_t); i++) {
            emmc_src_buf[i] = 0xAA55AA55 + (uint32_t)rand();
        }

        //write
        emmc_read_write(cfg->card_offset, (uint8_t *)emmc_src_buf, sizeof(emmc_src_buf), 1);

#ifdef EMMC_PWROFF_NOTIFICATION_TEST
        err = mmc_send_ext_csd(host);
        if (err) {
            while (1) {
                EMMC_TRACE(0, "******send ext csd err:%d, line:%d", err, __LINE__);
                host->delay(1000);
            }
        }
        pon_value = host->card->ext_csd[EXT_CSD__PWR_OFF_NOTIFY];
        EMMC_TRACE(0, "------PWR_OFF_NOTIFY after awake:%d", pon_value);

        err = mmc_power_off_notification_config(host, EMMC_SLEEP_NOTIFICATION);
        if (err) {
            while (1) {
                EMMC_TRACE(0, "******power off notification config err:%d, line:%d", err, __LINE__);
                host->delay(1000);
            }
        }

#if 0
        /*
            If host continues to send commands (e.g., CMD8) to the device after switching to the power off setting
            (POWER_OFF_LONG, POWER_OFF_SHORT or SLEEP_NOTIFICATION) or performs HPI during
            its busy condition, the device shall restore the POWER_OFF_NOTIFICATION byte to POWERED_ON
        */
        err = mmc_send_ext_csd(host);
        if (err) {
            while (1) {
                EMMC_TRACE(0, "******send ext csd err:%d, line:%d", err, __LINE__);
                host->delay(1000);
            }
        }

        pon_value = host->card->ext_csd[EXT_CSD__PWR_OFF_NOTIFY];
        EMMC_TRACE(0, "------PWR_OFF_NOTIFY after :%d", pon_value);
        if (pon_value == EMMC_SLEEP_NOTIFICATION) {
            EMMC_TRACE(0, "++++++power off notification config:%d success", EMMC_SLEEP_NOTIFICATION);
        } else {
            while (1) {
                EMMC_TRACE(0, "******power off notification config:%d fail", EMMC_SLEEP_NOTIFICATION);
                host->delay(1000);
            }
        }
#endif
#endif
        card_status = mmc_card_status(host);
        EMMC_TRACE(0, "------card before sleep, card current state:0x%x", card_status);
        sleep_card(host);
        emmc_poweroff_device_3v3(host);

        host->delay(2000);//The length of the delay depends on the time it takes for the capacitor to discharge
        EMMC_TRACE(0, "------delay......");
        emmc_poweron_device_3v3(host);
        awake_card(host);
        card_status = mmc_card_status(host);
        EMMC_TRACE(0, "------card after awake,  card current state:0x%x", card_status);

        //read and check
        memset(emmc_dst_buf, 0, sizeof(emmc_dst_buf));
        emmc_read_write(cfg->card_offset, (uint8_t *)emmc_dst_buf, sizeof(emmc_dst_buf), 0);
        ret = memcmp((uint32_t *)emmc_dst_buf, (uint32_t *)emmc_src_buf, sizeof(emmc_dst_buf));
        if (ret) {
            fail_cnt++;
            EMMC_TRACE(0, "******************emmc read: compare error, ret:%d, fail cnt:%d, pass cnt:%d", ret, fail_cnt, pass_cnt);
        } else {
            pass_cnt++;
            EMMC_TRACE(0, "++++++++++++++++++emmc writing/reading comparison: Success, fail cnt:%d, pass cnt:%d", fail_cnt, pass_cnt);
        }
    }
#endif

#ifdef EMMC_WRITE_SMALL_DATA_TEST
#define WRITE_SMALL_DATA_SIZE       32
    EMMC_TRACE(0, "  ");
    EMMC_TRACE(0, "----------emmc write and read %dbytes test----------", WRITE_SMALL_DATA_SIZE);

    //data buffer populates 0x55
    memset((uint8_t *)emmc_src_buf, 0x55, sizeof(emmc_src_buf));

    //write
    emmc_read_write(0, (uint8_t *)emmc_src_buf, 512, 1);

    //read
    emmc_read_write(0, (uint8_t *)emmc_dst_buf, 512, 0);
    EMMC_TRACE(0, "---------------------------write %d bytes:0x55", 512);
    emmc_dump32("origin emmc data", emmc_dst_buf, 512);

    //data buffer populates 0xAA
    memset((uint8_t *)emmc_src_buf, 0xAA, sizeof(emmc_src_buf));

    //write
    emmc_read_write(0, (uint8_t *)emmc_src_buf, WRITE_SMALL_DATA_SIZE, 1);

    //read
    emmc_read_write(0, (uint8_t *)emmc_dst_buf, WRITE_SMALL_DATA_SIZE, 0);
    EMMC_TRACE(0, "---------------------------write %d bytes:0xAA", WRITE_SMALL_DATA_SIZE);
    emmc_dump32("new emmc data", emmc_dst_buf, 512);
#endif

#ifdef EMMC_ALL_BLOCK_TEST_ENABLE
    uint8_t pt_index;
    uint32_t emmc_cap;
    uint32_t cur_emmc_cap;
    uint32_t blk_offset;
    uint32_t pattern_data[] = {
        0xAAAAAAAA,
        0x55555555,
        0x12345678,
        0xA5A5A5A5,
        0x5A5A5A5A,
        0xFEFEFEFE,
        0xEFFEEFFE,
        0x00000000,
        0xFFFFFFFF,
        0x11111111,
    };

    EMMC_TRACE(0, "  ");
    EMMC_TRACE(0, "----------emmc all block test start----------");
    emmc_cap = (uint32_t)(((uint64_t)card_info.blocknbr * card_info.blocksize * 100) / 1073741824);
#if 1
    for (blk_offset = 0; blk_offset < card_info.blocknbr; blk_offset++) {
#else
    for (blk_offset = 0; blk_offset < 10; blk_offset++) {
#endif
        for (pt_index = 0; pt_index < ARRAY_SIZE(pattern_data); pt_index++) {
            for (int i = 0; i < CARD_BLOCK_LENGTH / sizeof(uint32_t); i++) {
                emmc_src_buf[i] = pattern_data[pt_index];
            }

            //write
            emmc_read_write(blk_offset, (uint8_t *)emmc_src_buf, CARD_BLOCK_LENGTH, 1);

            //read
            memset((uint8_t *)emmc_dst_buf, 0xCC, CARD_BLOCK_LENGTH);
            emmc_read_write(blk_offset, (uint8_t *)emmc_dst_buf, CARD_BLOCK_LENGTH, 0);
            emmc_dump32("emmc data", emmc_dst_buf, CARD_BLOCK_LENGTH);

            //compare
            ret = memcmp((void *)emmc_dst_buf, (void *)emmc_src_buf, CARD_BLOCK_LENGTH);
            if (ret) {
                EMMC_TRACE(0, "******emmc compare error, ret:%d, pattern:0x%X, block offset:%d", ret, pattern_data[pt_index], blk_offset);
                emmc_dump32("emmc data", emmc_dst_buf, CARD_BLOCK_LENGTH);
                EMMC_TRACE(0, "******enter while(1)");
                while (1);
            } else {
                cur_emmc_cap = (uint32_t)(((uint64_t)blk_offset * card_info.blocksize * 100) / 1073741824);
                EMMC_TRACE(0, "++++++emmc write,read,compare: Success, pattern:0x%X, block offset:%d, %d.%d/%d.%dGB",
                           pattern_data[pt_index], blk_offset,
                           cur_emmc_cap / 100, cur_emmc_cap % 100,
                           emmc_cap / 100, emmc_cap % 100);
            }
        }
        TRACE_FLUSH();
    }

#if 0
    for (blk_offset = 0; blk_offset < ARRAY_SIZE(pattern_data); blk_offset++) {
        //read
        memset((uint8_t *)emmc_dst_buf, 0xCC, CARD_BLOCK_LENGTH);
        emmc_read_write(blk_offset, (uint8_t *)emmc_dst_buf, CARD_BLOCK_LENGTH, 0);

        EMMC_TRACE(0, "block offset:%d", blk_offset);
        emmc_dump32("emmc data", emmc_dst_buf, CARD_BLOCK_LENGTH);
    }
#endif

    EMMC_TRACE(0, "  ");
    EMMC_TRACE(0, "*******************************************");
    EMMC_TRACE(0, "       emmc all block check:PASS");
    EMMC_TRACE(0, "*******************************************");
    EMMC_TRACE(0, "  ");
    EMMC_TRACE(0, "----------emmc all block test end----------");
#endif

#ifdef EMMC_BIG_FILE_TEST_ENABLE
    EMMC_TRACE(0, "  ");
    EMMC_TRACE(0, "----------emmc write and read 31.9Mbytes test----------");
    EMMC_TRACE(0, "simulation test, you need to modify CARD_XFER_SIZE_MAX to 1024");

    //data buffer populates random
    rand_generator((uint8_t *)emmc_src_buf, sizeof(emmc_src_buf));

    //write
    EMMC_TRACE(0, "----------write %d bytes", sizeof(emmc_src_buf));
    EMMC_TRACE(0, "----------src buf addr:0x%X", (uint32_t)emmc_src_buf);
    write_card_ext(host, 0, emmc_src_buf, sizeof(emmc_src_buf));

    //read
    EMMC_TRACE(0, "----------read %d bytes", sizeof(emmc_dst_buf));
    EMMC_TRACE(0, "----------dst buf addr:0x%X", (uint32_t)emmc_dst_buf);
    read_card_ext(host, 0, (uint8_t *)emmc_dst_buf, sizeof(emmc_dst_buf));
    emmc_dump32("emmc data", emmc_dst_buf, 512);

    if (memcmp(emmc_src_buf, emmc_dst_buf, sizeof(emmc_src_buf)) == 0) {
        EMMC_TRACE(0, "----------big file test successful");
    } else {
        EMMC_TRACE(0, "**********big file test fail");
    }
    EMMC_TRACE(0, " ");
#endif

#ifdef EMMC_ERASE_TEST_ENABLE
#define ERASE_START_BLK     (0)
#define ERASE_BLKS_CNT      (3)
#define SECTOR_SIZE         (CARD_BLOCK_LENGTH)

    EMMC_TRACE(0, "  ");
    EMMC_TRACE(0, "--------------------emmc earse test--------------------");

    //data buffer populates random
    rand_generator((uint8_t *)emmc_src_buf, sizeof(emmc_src_buf));

    //write
    EMMC_TRACE(0, "----------write %d bytes", sizeof(emmc_src_buf));
    write_card_ext(host, 0, emmc_src_buf, sizeof(emmc_src_buf));

    //read
    EMMC_TRACE(0, "----------read %d bytes", sizeof(emmc_dst_buf));
    read_card_ext(host, 0, (uint8_t *)emmc_dst_buf, sizeof(emmc_dst_buf));
    emmc_dump32("emmc data", (uint32_t *)((uint32_t)emmc_dst_buf + SECTOR_SIZE * 0), SECTOR_SIZE);
    emmc_dump32("emmc data", (uint32_t *)((uint32_t)emmc_dst_buf + SECTOR_SIZE * 1), SECTOR_SIZE);
    emmc_dump32("emmc data", (uint32_t *)((uint32_t)emmc_dst_buf + SECTOR_SIZE * 2), SECTOR_SIZE);
    emmc_dump32("emmc data", (uint32_t *)((uint32_t)emmc_dst_buf + SECTOR_SIZE * 3), SECTOR_SIZE);

    //erase
    EMMC_TRACE(0, "----------erase blk: %d ~ %d", ERASE_START_BLK, ERASE_START_BLK + ERASE_BLKS_CNT - 1);
    ret = erase_card(host, ERASE_START_BLK, ERASE_BLKS_CNT);
    if (ret) {
        EMMC_TRACE(0, "----------erase fail, ret:%d", ret);
    } else {
        //read
        EMMC_TRACE(0, "----------read %d bytes", sizeof(emmc_dst_buf));
        memset((void *)emmc_dst_buf, 0, sizeof(emmc_dst_buf));
        read_card_ext(host, 0, (uint8_t *)emmc_dst_buf, sizeof(emmc_dst_buf));
        emmc_dump32("emmc data", (uint32_t *)((uint32_t)emmc_dst_buf + SECTOR_SIZE * 0), SECTOR_SIZE);
        emmc_dump32("emmc data", (uint32_t *)((uint32_t)emmc_dst_buf + SECTOR_SIZE * 1), SECTOR_SIZE);
        emmc_dump32("emmc data", (uint32_t *)((uint32_t)emmc_dst_buf + SECTOR_SIZE * 2), SECTOR_SIZE);
        emmc_dump32("emmc data", (uint32_t *)((uint32_t)emmc_dst_buf + SECTOR_SIZE * 3), SECTOR_SIZE);
        EMMC_TRACE(0, "----------erase test successful");
    }
    EMMC_TRACE(0, " ");
#endif

#ifndef CORE_SLEEP_POWER_DOWN
#ifdef EMMC_SLEEP_ENABLE
    EMMC_TRACE(0, "  ");
    EMMC_TRACE(0, "+++before sleep, card current state:0x%x", mmc_card_status(host));
    EMMC_TRACE(0, "+++before sleep, card raw status   :0x%x", mmc_card_raw_status(host));
    sleep_card(host);
    EMMC_TRACE(0, "+++after sleep,  card status       :sleep");
    sdhci_force_clk_off(host);
    sdhci_host_init(false, host);//emmc close
#else
    if (emmc_cfg.timing != HS400 && emmc_cfg.timing != HS200 && emmc_cfg.timing != HS100
        && emmc_cfg.timing != HS50 && emmc_cfg.timing != HS25) {
        sdhci_force_clk_on(host);//Always open clk forcibly, only for testing
    }
#endif
#endif

    ret = emmc_close(host);
#ifdef EMMC_SLT_FLAG_SET_ENABLE
    hal_cmu_simu_tag(EMMC_TEST_TAG_SUCCESS);
#endif
    EMMC_TRACE(0, "----------emmc test completed----------");

    return ret;
}
