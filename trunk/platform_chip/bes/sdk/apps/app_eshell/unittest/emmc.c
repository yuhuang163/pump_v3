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

#if defined(UTILS_ESHELL_EN) && defined(__SYS_AS_MAIN__) && defined(CHIP_HAS_EMMC)
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "host.h"
#include "error.h"
#include "emmc_config.h"
#include "sdhci_emphy.h"
#include "sdhci.h"
#include "card.h"
#include "host.h"
#include "heap_api.h"
#include "heap_api.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "cmsis_os.h"
#include "eshell.h"
#include "app_eshell.h"

/**
 * work_clk = mod_clk/2*(div+1)
 * 24M   oscx2   0
 * 48M   oscx4   0
 * 200M  BPLL    0
 * according to work_clk select mod_clk
 */

// mod_clk out GPIO5  word_clk out R126
// size bigger the faster the speed
#define CLKOUT_IOMUX_INDEX 5

// to data invert
static int invert = 0;

#if (RAM_SIZE <= 256 * 1024)
#define EMMC_XBUF_SIZE 2048
#else
#define EMMC_XBUF_SIZE (64 * 1024)
#endif
// SRAM
SYNC_FLAGS_LOC static uint32_t emmc_src_buf[EMMC_XBUF_SIZE / 4];
SYNC_FLAGS_LOC static uint32_t emmc_dst_buf[EMMC_XBUF_SIZE / 4];

#define EMMC_XBUF_SIZE2 (10 * 1024 * 1024) // psram 10M

typedef enum
{
    EMMC_DEV_INFO,    // Command output Information about the currently selected MMC info device.
    EMMC_STATUS_INFO, // Command to query the working status of the MMC.
    EMMC_CSD,         // Command is used to query the CSD register information of the MMC.
    EMMC_CID,         // Command is used to query the CID register information of the MMC.
    EMMC_SPEED, // Command is used to MMC device in DS | SDR52 | DDR52 | HS200 speed mode, speaking,
    // reading and writing test block;
    EMMC_WRITE, // Command to write data to the MMC device;
    EMMC_READ,  // Commands are used to read the data of the MMC.
    EMMC_ERASE, // Command is used to erase the specified block of the MMC device.
    EMMC_INIT,  // Commands is used to initialize MMC devices.
} EMMC_INSTRUCTION;

#define RETRY_CNT_MAX 5
#define CARD_BLOCK_LENGTH 512
#define MULTI_TEST_SIZE (2 * 1024 * 1024) // 2MBytes

#define MULTI_READ_CNT 10
static struct mmc_host emmc_test_op;
// #define EMMC_XBUF_SIZE 1024
static volatile uint8_t dma_done_flag = 0;
#define MULTI_TEST_CNT (MULTI_TEST_SIZE / EMMC_XBUF_SIZE)

enum EMMC_TEST_TYPE { SDHCI_ONCE_TEST, SDHCI_LOOP_TEST };

static void emmc_delay(uint32_t ms)
{
    osDelay(ms);
}

static volatile uint8_t error_flag = 0;

static void emmc_host_error(enum EMMC_HOST_ERR error)
{
    error_flag = 1;
    eshell_putstring("******EMMC HOST, error: %d\r\n", error);
}

static void emmc_host_dma_done(void)
{
    dma_done_flag = 1;
}

// emmc_config
static struct emmc_config emmc_cfg =
{
    /*pmu*/
    .vmin_begin = 1800,

    /*test*/
    .controller = DEV_TYPE_EMMC,
    .test_id = SDHCI_STRESS_TEST,

    .subtest_id = 0,
    .real_card = 1,

    /*platform*/
    .src_region = 0,
    .dst_region = 0,
    .aligned = 4,

    .rst_pin = HAL_IOMUX_PIN_P3_6,
    /* BES EVB Core Board V1.2/1.3... */
    .pwr_3p3_pin = HAL_IOMUX_PIN_PMU_P2_3,
    .pwr_1p8_pin = HAL_IOMUX_PIN_PMU_P2_1,
    .pwr_on_level = 1,
    .clk_drv = 2,
    .cmd_drv = 2,
    .dat_drv = 2,
    .vcc_main = 1800,

    /*data transfer*/
    .blksz = CARD_BLOCK_LENGTH, // max 4095

    .blks = EMMC_XBUF_SIZE / CARD_BLOCK_LENGTH, // max 65535

    .card_offset = 0,
    .seed = 0x12345678,
    .pattern = MMC_DATA_PATTERN_RANDOM,

    /*host mode*/

    .dma_mode = MM4_SDMA,

    .dma_bdry = MM4_HOST_DMA_BDRY_512KB,
    .auto_cmd_type = MM4_AUTO_CMD12, // SDMA does not support AutoCMD23

    .desc_type = ADMA2_DESC_FIXED_SZ,

    .desc_sz = ADMA2_DESC_MAX_LEN,

    .desc_nr = EMMC_XBUF_SIZE / ADMA2_DESC_MAX_LEN ? EMMC_XBUF_SIZE / ADMA2_DESC_MAX_LEN : 1,

    /*bus io*/
    .timing = DS,
    .khz = F_DS,
    .width = MMC_8BIT_WIDTH,
    .voltage = 1800,
    .strobe = 0, // 0 or 1 can, the software will automatically modify the value according to the
    // parameters of the EMMC.
    .quirks = MMC_QUIRKS_HW_TUNING | MMC_QUIRKS_PERFORMANCE,

    /*delay*/
    .delay = emmc_delay,

    /*callback*/
    .emmc_host_error = emmc_host_error,

    .emmc_host_dma_done = emmc_host_dma_done,
};

static void emmc_poweron_device(struct mmc_host *host)
{
    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    // emmc 3.3v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_3p3_pin) < HAL_IOMUX_PIN_PMU_NUM)
    {
        if (cfg->pwr_on_level)
        {
            hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(cfg->pwr_3p3_pin));
        }
        else
        {
            hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(cfg->pwr_3p3_pin));
        }
    }

    // emmc 1.8v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_1p8_pin) < HAL_IOMUX_PIN_PMU_NUM)
    {
        if (cfg->pwr_on_level)
        {
            hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(cfg->pwr_1p8_pin));
        }
        else
        {
            hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(cfg->pwr_1p8_pin));
        }
    }

    eshell_putstring("EMMC POWER ON DONE\r\n");
}

static void emmc_poweroff_device(struct mmc_host *host)
{
    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    // emmc 3.3v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_3p3_pin) < HAL_IOMUX_PIN_PMU_NUM)
    {
        if (cfg->pwr_on_level)
        {
            hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(cfg->pwr_3p3_pin));
        }
        else
        {
            hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(cfg->pwr_3p3_pin));
        }
    }

    // emmc 1.8v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_1p8_pin) < HAL_IOMUX_PIN_PMU_NUM)
    {
        if (cfg->pwr_on_level)
        {
            hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(cfg->pwr_1p8_pin));
        }
        else
        {
            hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(cfg->pwr_1p8_pin));
        }
    }

    eshell_putstring("EMMC POWER OFF DONE\r\n");
}

static void emmc_rst_pwr_pin_init(struct mmc_host *host)
{
    struct emmc_config *cfg;
    struct HAL_IOMUX_PIN_FUNCTION_MAP iomux[] =
    {
        {HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    cfg = (struct emmc_config *)host->private;

    // emmc reset pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->rst_pin) < HAL_IOMUX_PIN_NUM)
    {
        iomux[0].pin = (enum HAL_IOMUX_PIN_T)(cfg->rst_pin);
        hal_iomux_init(iomux, ARRAY_SIZE(iomux));
    }

    if ((enum HAL_IOMUX_PIN_T)(cfg->rst_pin) < HAL_IOMUX_PIN_PMU_NUM)
    {
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)(cfg->rst_pin), HAL_GPIO_DIR_OUT, 1);
    }

    // emmc 3.3v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_3p3_pin) < HAL_IOMUX_PIN_NUM)
    {
        iomux[0].pin = (enum HAL_IOMUX_PIN_T)(cfg->pwr_3p3_pin);
        hal_iomux_init(iomux, ARRAY_SIZE(iomux));
    }

    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_3p3_pin) < HAL_IOMUX_PIN_PMU_NUM)
    {
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)(cfg->pwr_3p3_pin), HAL_GPIO_DIR_OUT, 1);
    }

    // emmc 1.8v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_1p8_pin) < HAL_IOMUX_PIN_NUM)
    {
        iomux[0].pin = (enum HAL_IOMUX_PIN_T)(cfg->pwr_1p8_pin);
        hal_iomux_init(iomux, ARRAY_SIZE(iomux));
    }

    if ((enum HAL_IOMUX_PIN_T)(cfg->pwr_1p8_pin) < HAL_IOMUX_PIN_PMU_NUM)
    {
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)(cfg->pwr_1p8_pin), HAL_GPIO_DIR_OUT, 1);
    }
}

static void emmc_reset_device(struct mmc_host *host)
{
    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(cfg->rst_pin));
    hal_sys_timer_delay_us(5);
    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(cfg->rst_pin));

    eshell_putstring("EMMC DEVICE RESET DONE\r\n");
}

static void mmc_test_info(struct mmc_host *host)
{
    eshell_putstring("Card Type                   : %s\r\n", mmc_card_type(host->card->type));
    eshell_putstring("Data Transfer Bit-Width     : %s\r\n", mmc_bitwidth_type(host->ios.width));
    eshell_putstring("Data Transfer Bus Mode      : %s\r\n", mmc_timing_type(host->ios.timing));
    eshell_putstring("Data Transfer ClkRate       : %dMHz\r\n", (host->ios.khz) / 1000);
    eshell_putstring("Data Transfer Size(Blocks)  : %d\r\n", host->blks);
    eshell_putstring("Data Transfer Auto CMD Type : %s\r\n", mmc_autocmd_type(host->auto_cmd_type));
}

static int sdhci_data_xfer_setup(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;
    uint32_t time_base;
    uint32_t emmc_init_time[5];
    struct mmc_card_info card_info;

    emmc_poweron_device(host);
    emmc_rst_pwr_pin_init(host);
    emmc_reset_device(host);

    time_base = hal_fast_sys_timer_get();
    emmc_init_time[0] = time_base;
    // card type
    ret = sdhci_platform_init(true, host);
    eshell_putstring("sdhci_platform_init\r\n");
    if (ret != MMC_NO_ERR)
        return ret;
    emmc_init_time[1] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_base);

    time_base = hal_fast_sys_timer_get();
    ret = sdhci_host_init(true, host);
    eshell_putstring("\r\n");
    if (ret != MMC_NO_ERR)
        return ret;

    emmc_init_time[2] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_base);

    time_base = hal_fast_sys_timer_get();
    ret = detect_card(host);
    eshell_putstring("detect_card done\r\n");
    if (ret != MMC_NO_ERR)
        return ret;

    emmc_init_time[3] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_base);

    time_base = hal_fast_sys_timer_get();
    ret = init_card(host);
    eshell_putstring("initialize card device done\r\n");
    if (ret != MMC_NO_ERR)
        return ret;

    emmc_init_time[4] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_base);
    emmc_init_time[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - emmc_init_time[0]);

    eshell_putstring("emmc platform init time: %dus\r\n", emmc_init_time[1]);
    eshell_putstring("emmc host init time    : %dus\r\n", emmc_init_time[2]);
    eshell_putstring("emmc detect card time  : %dus\r\n", emmc_init_time[3]);
    eshell_putstring("emmc init card time    : %dus\r\n", emmc_init_time[4]);

    get_card_info(&card_info);
    eshell_putstring("card type                : %d(%s)\r\n", card_info.cardtype,
                     card_info.cardtype ? "high capacity" : "low capacity");
    eshell_putstring("card class               : 0x%X\r\n", card_info.cardclass);
    eshell_putstring("card blocknbr            : 0x%X\r\n", card_info.blocknbr);
    eshell_putstring("card blocksize           : 0x%X\r\n", card_info.blocksize);
    eshell_putstring("Product Revision         : %d.%d\r\n", (card_info.cid.prv >> 4),
                     (card_info.cid.prv & 0xF));
    POSSIBLY_UNUSED uint32_t val =
        (uint32_t)(((uint64_t)card_info.blocknbr * card_info.blocksize * 100) / 1073741824);
    eshell_putstring("Card capacity            : %d.%dGB\r\n", val / 100, val % 100);

    return ret;
}

static int sdhci_data_xfer_test(struct mmc_host *host, bool cfg_card)
{
    int ret = MMC_NO_ERR;
    POSSIBLY_UNUSED uint8_t retry_cnt;
    uint32_t i;
    uint64_t val;
    uint32_t start_addr;

    uint32_t size = host->blks * host->blksz;
    uint32_t len = size / sizeof(uint32_t);

    uint32_t *src_mem = (uint32_t *)emmc_src_buf;
    uint32_t *dst_mem = (uint32_t *)emmc_dst_buf;
    // PSRAM_NC
    // uint32_t *src_mem = (uint32_t *)PSRAM_NC_BASE;
    // uint32_t *dst_mem = (uint32_t *)PSRAM_NC_BASE;

    uint32_t *src = (uint32_t *)src_mem;
    uint32_t *dst = (uint32_t *)dst_mem;

    uint32_t timeb, time[7] = {0};
    // get addr to get location
    // eshell_putstring("src=%x, dst=%x\r\n", (uint32_t)src, (uint32_t)dst);

    memset(src, 0x0, len);
    memset(dst, 0x0, len);

    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    timeb = hal_fast_sys_timer_get();
    if (cfg_card)
    {
        ret = config_card(host, cfg->timing, cfg->khz, cfg->width);
        if (ret != MMC_NO_ERR)
        {
            eshell_putstring("config card failed, %d\r\n", ret);
            goto _exit;
        }
    }
    time[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);

    mmc_test_info(host);
    // fill src,dst data (write random )
    if (invert)
    {
        // fill src,dst data
        for (i = 0; i < len; i++)
        {
            src[i] = 0xAAAAAAAA + (uint32_t)rand();
            dst[i] = 0x0;
        }
    }
    else
    {
        for (i = 0; i < len; i++)
        {
            src[i] = 0x55555555 + (uint32_t)rand();
            dst[i] = 0x0;
        }
    }
    sleep_card(host);
    host->delay(10);
    awake_card(host);

    retry_cnt = 0;
    error_flag = 0;
    dma_done_flag = 0;
    do
    {
        eshell_putstring("*******writing card*******\r\n");
        timeb = hal_fast_sys_timer_get();

        ret = write_card_dma_mode(host, cfg->card_offset, (void *)src, size);
        if (error_flag)
        {
            error_flag = 0;

            retry_cnt++;
            if (retry_cnt > RETRY_CNT_MAX)
            {
                goto write_error;
            }
            host->delay(1000);
            continue;
        }
        else
        {
            while (!dma_done_flag)
            {
            }
            dma_done_flag = 0;
        }
write_error:
        time[1] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
        eshell_putstring("multi write start,  total size:%uMB, write size:%d, cnt:%d\r\n",
                         MULTI_TEST_SIZE / 1024 / 1024, size, MULTI_TEST_CNT);
        timeb = hal_fast_sys_timer_get();
        for (i = 0; i < MULTI_TEST_CNT - 1; i++)
        {
            start_addr = cfg->card_offset + i * size / CARD_BLOCK_LENGTH;
            ret = write_card_dma_mode(host, start_addr, (void *)src, size);
            if (error_flag)
            {
                error_flag = 0;

                retry_cnt++;
                if (retry_cnt > RETRY_CNT_MAX)
                {
                    break;
                }
                host->delay(1000);
                continue;
            }
            else
            {
                while (!dma_done_flag)
                {
                }
                dma_done_flag = 0;
            }
        }
        time[2] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);

        if (ret != MMC_NO_ERR)
        {
            eshell_putstring("write card failed, ret=%d\r\n", ret);
        }
        else
        {
            eshell_putstring("write card complete\r\n");
            break;
        }
        if (ret != MMC_NO_ERR)
        {
            eshell_putstring("halt for write card error\r\n");
            while (1)
                ;
        }
    }
    while (1);

    sleep_card(host);
    host->delay(10);
    awake_card(host);
    retry_cnt = 0;
    error_flag = 0;
    dma_done_flag = 0;
    do
    {
        eshell_putstring("single read start, total size:%u, read size:%u, cnt:1\r\n", size, size);
        timeb = hal_fast_sys_timer_get();
        ret = read_card_dma_mode(host, cfg->card_offset, (void *)dst, size);

        if (error_flag)
        {
            error_flag = 0;

            retry_cnt++;
            if (retry_cnt > RETRY_CNT_MAX)
            {
                goto read_error;
            }
            host->delay(1000);
            continue;
        }
        else
        {
            while (!dma_done_flag)
            {
            }
            dma_done_flag = 0;
        }
read_error:
        retry_cnt = 0;
        error_flag = 0;
        dma_done_flag = 0;

        time[3] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
        eshell_putstring("multi read start,  total size:%uMB, read size:%d, cnt:%d\r\n",
                         MULTI_TEST_SIZE / 1024 / 1024, size, MULTI_TEST_CNT);
        timeb = hal_fast_sys_timer_get();
        for (i = 0; i < MULTI_TEST_CNT - 1; i++)
        {
            start_addr = cfg->card_offset + i * size / CARD_BLOCK_LENGTH;
            ret = read_card_dma_mode(host, start_addr, (void *)dst, size);
            if (error_flag)
            {
                error_flag = 0;

                retry_cnt++;
                if (retry_cnt > RETRY_CNT_MAX)
                {
                    break;
                }
                host->delay(1000);
                continue;
            }
            else
            {
                while (!dma_done_flag)
                {
                }
                dma_done_flag = 0;
            }
        }
        time[4] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
        if (ret != MMC_NO_ERR)
        {
            eshell_putstring("ead card failed, ret=%d\r\n", ret);
        }
        else
        {
            eshell_putstring("ead card complete\r\n");
            break;
        }

        if (ret != MMC_NO_ERR)
        {
            eshell_putstring("halt for read card error\r\n");
            while (1)
                ;
        }
    }
    while (1);
    invert = !invert;
    eshell_putstring("checking card data\r\n");
    do
    {
        timeb = hal_fast_sys_timer_get();
        ret = memcmp((uint32_t *)src, (uint32_t *)dst, len);
        time[5] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
        if (ret != MMC_NO_ERR)
        {
            eshell_putstring("check data failed, ret=%d, line=%d\r\n", ret, __LINE__);
            while (1)
                ;
        }

        timeb = hal_fast_sys_timer_get();
        for (i = 0; i < MULTI_TEST_CNT - 1; i++)
        {
            start_addr = cfg->card_offset + i * size / CARD_BLOCK_LENGTH;

            ret = read_card_dma_mode(host, start_addr, (void *)dst, size);
            if (error_flag)
            {
                error_flag = 0;

                retry_cnt++;
                if (retry_cnt > RETRY_CNT_MAX)
                {
                    break;
                }
                host->delay(1000);
                continue;
            }
            else
            {
                while (!dma_done_flag)
                {
                }
                dma_done_flag = 0;
            }
            ret = memcmp((uint32_t *)src, (uint32_t *)dst, len);
            if (ret != MMC_NO_ERR)
            {
                eshell_putstring("check data failed, ret=%d, line=%d\r\n", ret, __LINE__);
                eshell_putstring("halt for check card data error\r\n");
                while (1)
                    ;
            }
        }
        time[6] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
        eshell_putstring(" data check is OK!!!\r\n");

        if (!ret)
        {
            // calc speed

            eshell_putstring("==========read and write success==========\r\n");
            eshell_putstring("******************************************\r\n");
            // eshell_putstring("TIME COST        : cfg        [%uus]\r\n", time[0]);
            eshell_putstring("TIME COST        : read       [%uus]\r\n", time[3]);
            eshell_putstring("TIME COST        : multi read [%uus]\r\n", time[4]);
            eshell_putstring("TIME COST        : write      [%uus]\r\n", time[1]);
            eshell_putstring("TIME COST        : multi write[%uus]\r\n", time[2]);
            eshell_putstring("TIME COST        : cmp        [%uus]\r\n", time[5]);
            eshell_putstring("TIME COST        : multi cmp  [%uus]\r\n", time[6]);
            eshell_putstring("******************************************\r\n");
            eshell_putstring("read size        :%ubytes\r\n", size);
            val = (uint64_t)size * MULTI_TEST_CNT / 1024;
            eshell_putstring("multi read size  :%uKbytes=%uMbytes\r\n", (uint32_t)val,
                             (uint32_t)(val / 1024));
            eshell_putstring("write size       :%ubytes\r\n", size);
            eshell_putstring("multi write size :%uKbytes=%uMbytes\r\n", (uint32_t)val,
                             (uint32_t)(val / 1024));
            eshell_putstring("cmp size         :%ubytes\r\n", size);
            eshell_putstring("multi cmp size   :%uKbytes=%uMbytes\r\n", (uint32_t)val,
                             (uint32_t)(val / 1024));

            time[3] = (uint32_t)((uint64_t)size * 95400 / 1000 / time[3]); // read
            time[4] =
                (uint32_t)((uint64_t)size * 95400 * MULTI_TEST_CNT / 1000 / time[4]); // multi read
            time[1] = (uint32_t)((uint64_t)size * 95400 / 1000 / time[1]);            // write
            time[2] =
                (uint32_t)((uint64_t)size * 95400 * MULTI_TEST_CNT / 1000 / time[2]); // multi write

            // eshell_putstring(1, "emmc cache       :%s", card_info.cache_ctrl ? "ON" : "OFF");
            eshell_putstring("read speed       :%u.%uMB/s\r\n", time[3] / 100, time[3] % 100);
            eshell_putstring("multi read speed :%u.%uMB/s\r\n", time[4] / 100, time[4] % 100);
            eshell_putstring("write speed      :%u.%uMB/s\r\n", time[1] / 100, time[1] % 100);
            eshell_putstring("multi write speed:%u.%uMB/s\r\n", time[2] / 100, time[2] % 100);
            break;
        }
        else
        {
            eshell_putstring("check data failed, ret=%d\r\n", ret);
        }

        if (ret != MMC_NO_ERR)
        {
            eshell_putstring("halt for check card data error\r\n");
            while (1)
                ;
        }
    }
    while (1);
_exit:
    return ret;
}

static int sdhci_data_xfer_clean(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    eshell_putstring("SDHCI DATA transfer clean\r\n");

    return ret;
}

static int mmc_test_start(enum EMMC_TEST_TYPE test_type, struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    uint32_t run_cnt = 0;
    switch (test_type)
    {
    case 0:
    {
        eshell_putstring("SDHCI DATA transfer test\r\n");
        ret = sdhci_data_xfer_setup(host);
        if (ret != MMC_NO_ERR)
            return ret;

        ret = sdhci_data_xfer_test(host, true);
        if (ret != MMC_NO_ERR)
            return ret;

        ret = sdhci_data_xfer_clean(host);
        if (ret != MMC_NO_ERR)
            return ret;
        break;
    }
    case 1:
    {
        eshell_putstring("SDHCI DATA stress test");
        ret = sdhci_data_xfer_setup(host);
        if (ret != MMC_NO_ERR)
            return ret;

        for (int i = 0; i < 255; i++)
        {
            ret = sdhci_data_xfer_test(host, true);
            if (ret != MMC_NO_ERR)
                return ret;
            // eshell_putstring("before sleep, card status:0x%x", mmc_card_status(host));
            sleep_card(host);
            eshell_putstring("after sleep,  card status:sleep\r\n");

            sdhci_host_emphy_sleep(host);

            host->delay(5000);

            sdhci_host_emphy_wakeup(host);
            host->delay(2);
            eshell_putstring("emmc phy enable\r\n");
            awake_card(host);
            // eshell_putstring("---after awake,  card status:0x%x", mmc_card_status(host));

            run_cnt++;
            eshell_putstring("******run cnt:%d******\r\n", run_cnt);
            host->delay(100);
        }
    }
    sdhci_host_init(false, host);
    emmc_poweroff_device(host);
    break;

    default:
    {
        eshell_putstring("%s:%d,emmc test id error", __FUNCTION__, __LINE__);
        ret = MMC_INVALID_ERR;
        break;
    }
    }
    return ret;
}

static void mmc_init_host(struct mmc_host *host)
{
    host->id = emmc_cfg.controller;
    host->dma_mode = emmc_cfg.dma_mode;
    host->auto_cmd_type = emmc_cfg.auto_cmd_type;
    host->dma_bdry = emmc_cfg.dma_bdry;
    host->desc_sz = emmc_cfg.desc_sz;
    host->desc_nr = emmc_cfg.desc_nr;
    host->desc_type = emmc_cfg.desc_type;
    host->blksz = emmc_cfg.blksz;
    host->blks = emmc_cfg.blks;
    host->quirks = emmc_cfg.quirks;
    host->strobe = emmc_cfg.strobe;
    host->vmin_begin = emmc_cfg.vmin_begin;
    host->speed_mode = emmc_cfg.timing;
    host->delay = emmc_cfg.delay;
    host->emmc_host_error = emmc_cfg.emmc_host_error;
    host->emmc_host_dma_done = emmc_cfg.emmc_host_dma_done;
    host->private = (void *)&emmc_cfg;
    host->error_code = MMC_NO_ERR;
}

static int mmc_read_card_info(EMMC_INSTRUCTION emmc_inst)
{
    struct mmc_host *host = &emmc_test_op;
    struct mmc_card_info card_info;
    char *cbx[3] = {"Device(removable)", "BGA(Discrete embedded)", "POP"};
    char *pnm;
    uint32_t val = 0;

    if (host->id == 0)
    {
        int ret = MMC_NO_ERR;
        memset((void *)host, 0, sizeof(struct mmc_host));

        mmc_init_host(host);

        emmc_poweron_device(host);
        emmc_rst_pwr_pin_init(host);
        emmc_reset_device(host);

        // card type
        ret = sdhci_platform_init(true, host);
        if (ret != MMC_NO_ERR)
            return ret;

        ret = sdhci_host_init(true, host);
        if (ret != MMC_NO_ERR)
            return ret;

        ret = detect_card(host);

        if (ret != MMC_NO_ERR)
            return ret;

        ret = init_card(host);

        if (ret != MMC_NO_ERR)
            return ret;
    }

    get_card_info(&card_info);
    pnm = (char *)host->card->cid.pnm;

    switch (emmc_inst)
    {
    case EMMC_DEV_INFO:
        eshell_putstring("  \r\n");
        eshell_putstring("********************************** \r\n");
        eshell_putstring("dev info: \r\n");
        eshell_putstring("card type                : %d(%s) \r\n", card_info.cardtype,
                         card_info.cardtype ? "high capacity" : "low capacity");
        eshell_putstring("card class               : 0x%X \r\n", card_info.cardclass);
        eshell_putstring("card relcardAdd          : 0x%X \r\n", card_info.relcardadd);
        eshell_putstring("card blocknbr            : 0x%X \r\n", card_info.blocknbr);
        eshell_putstring("card blocksize           : 0x%X \r\n", card_info.blocksize);
        eshell_putstring("card logblocknbr         : 0x%X\r\n", card_info.logblocknbr);
        eshell_putstring("card logllocksize        : 0x%X\r\n", card_info.logllocksize);
        eshell_putstring("card ext_csd_rev         : 0x%X\r\n", card_info.ext_csd_rev);
        eshell_putstring("card firmware version_h32: 0x%X\r\n", card_info.firmware_version_h32);
        eshell_putstring("card firmware version_l32: 0x%X\r\n", card_info.firmware_version);
        eshell_putstring("card device version      : 0x%X\r\n", card_info.device_version);
        val = (uint32_t)(((uint64_t)card_info.blocknbr * card_info.blocksize * 100) / 1073741824);
        eshell_putstring("Card capacity            : %d.%dGB\r\n", val / 100, val % 100);
        eshell_putstring("********************************************\r\n");
        break;
    case EMMC_CID:
        eshell_putstring("********************************** \r\n");
        eshell_putstring("dev cid info: \r\n");
        eshell_putstring("EMMC Manufacturer ID : 0x%x \r\n", host->card->cid.mid);
        if (host->card->cid.cbx == 0)
            eshell_putstring("EMMC Device/BGA : %s \r\n", cbx[0]);
        else if (host->card->cid.cbx == 1)
            eshell_putstring("EMMC Device/BGA : %s \r\n", cbx[1]);
        else if (host->card->cid.cbx == 2)
            eshell_putstring("EMMC Device/BGA : %s \r\n", cbx[2]);
        eshell_putstring("EMMC OEM/Application ID : %u \r\n", host->card->cid.oid);
        eshell_putstring("EMMC Product name : %s \r\n", pnm);
        eshell_putstring("EMMC Product Revision : %d.%d \r\n", (host->card->cid.prv >> 4),
                         (host->card->cid.prv & 0xF));
        eshell_putstring("EMMC Product serial number : 0x%x \r\n", host->card->cid.psn);
        eshell_putstring("EMMC Manufacturing date : %u \r\n", host->card->cid.mdt);
        eshell_putstring("Manufacture Date : %d/%d \r\n", 2000 + (card_info.cid.mdt >> 4),
                         card_info.cid.mdt & 0xF);
        eshell_putstring("********************************** \r\n");
        break;
    case EMMC_CSD:
        eshell_putstring("  \r\n");
        eshell_putstring("********************************************\r\n");
        eshell_putstring("csd reg info: \r\n");
        eshell_putstring("csd.csd_structure        : 0x%X\r\n", card_info.csd.csd_structure);
        eshell_putstring("csd.spec_vers            : 0x%X\r\n", card_info.csd.spec_vers);
        eshell_putstring("csd.taac                 : 0x%X\r\n", card_info.csd.taac);
        eshell_putstring("csd.nsac                 : 0x%X\r\n", card_info.csd.nsac);
        eshell_putstring("csd.tran_speed           : 0x%X\r\n", card_info.csd.tran_speed);
        eshell_putstring("csd.ccc                  : 0x%X\r\n", card_info.csd.ccc);
        eshell_putstring("csd.read_bl_len          : 0x%X\r\n", card_info.csd.read_bl_len);
        eshell_putstring("csd.read_bl_partial      : 0x%X\r\n", card_info.csd.read_bl_partial);
        eshell_putstring("csd.write_blk_misalign   : 0x%X\r\n", card_info.csd.write_blk_misalign);
        eshell_putstring("csd.read_blk_misalign    : 0x%X\r\n", card_info.csd.read_blk_misalign);
        eshell_putstring("csd.dsr_imp              : 0x%X\r\n", card_info.csd.dsr_imp);
        eshell_putstring("csd.c_size               : 0x%X\r\n", card_info.csd.c_size);
        eshell_putstring("csd.vdd_r_curr_min       : 0x%X\r\n", card_info.csd.vdd_r_curr_min);
        eshell_putstring("csd.vdd_r_curr_max       : 0x%X\r\n", card_info.csd.vdd_r_curr_max);
        eshell_putstring("csd.vdd_w_curr_min       : 0x%X\r\n", card_info.csd.vdd_w_curr_min);
        eshell_putstring("csd.vdd_w_curr_max       : 0x%X\r\n", card_info.csd.vdd_w_curr_max);
        eshell_putstring("csd.c_size_mult          : 0x%X\r\n", card_info.csd.c_size_mult);
        eshell_putstring("csd.erase_grp_size       : 0x%X\r\n", card_info.csd.erase_grp_size);
        eshell_putstring("csd.erase_grp_mult       : 0x%X\r\n", card_info.csd.erase_grp_mult);
        eshell_putstring("csd.wp_grp_size          : 0x%X\r\n", card_info.csd.wp_grp_size);
        eshell_putstring("csd.wp_grp_enable        : 0x%X\r\n", card_info.csd.wp_grp_enable);
        eshell_putstring("csd.default_ecc          : 0x%X\r\n", card_info.csd.default_ecc);
        eshell_putstring("csd.r2w_factor           : 0x%X\r\n", card_info.csd.r2w_factor);
        eshell_putstring("csd.write_bl_len         : 0x%X\r\n", card_info.csd.write_bl_len);
        eshell_putstring("csd.write_bl_partial     : 0x%X\r\n", card_info.csd.write_bl_partial);
        eshell_putstring("csd.content_prot_app     : 0x%X\r\n", card_info.csd.content_prot_app);
        eshell_putstring("csd.file_format_grp      : 0x%X\r\n", card_info.csd.file_format_grp);
        eshell_putstring("csd.copy                 : 0x%X\r\n", card_info.csd.copy);
        eshell_putstring("csd.perm_write_protect   : 0x%X\r\n", card_info.csd.perm_write_protect);
        eshell_putstring("csd.tmp_write_protect    : 0x%X\r\n", card_info.csd.tmp_write_protect);
        eshell_putstring("csd.file_format          : 0x%X\r\n", card_info.csd.file_format);
        eshell_putstring("csd.ecc                  : 0x%X\r\n", card_info.csd.ecc);
        eshell_putstring("********************************************\r\n");
        eshell_putstring("  \r\n");
        break;
    case EMMC_STATUS_INFO:
        eshell_putstring("********************************** \r\n");
        eshell_putstring("emmc base: 0x%X \r\n", EMMC_BASE);
        eshell_putstring("VOL min begin  : %d(mV) \r\n", emmc_cfg.vmin_begin);
        eshell_putstring("MMC Controller : %d   \r\n", emmc_cfg.controller);
        eshell_putstring("Test Number    : %d   \r\n", emmc_cfg.test_id);
        eshell_putstring("Sub Test #     : %d   \r\n", emmc_cfg.subtest_id);
        eshell_putstring("Real Card      : %s   \r\n", (emmc_cfg.real_card == 1) ? "yes" : "no");
        eshell_putstring("SRC Memory     : %d   \r\n", emmc_cfg.src_region);
        eshell_putstring("TGT Memory     : %d   \r\n", emmc_cfg.dst_region);
        eshell_putstring("Memory align   : %d   \r\n", emmc_cfg.aligned);
        eshell_putstring("Emmc reset pin : %d   \r\n", emmc_cfg.rst_pin);
        eshell_putstring("Emmc 3.3v pin  : %d   \r\n", emmc_cfg.pwr_3p3_pin);
        eshell_putstring("Emmc 1.8v pin  : %d   \r\n", emmc_cfg.pwr_1p8_pin);
        eshell_putstring("Emmc pwr on lev: %d   \r\n", emmc_cfg.pwr_on_level);
        eshell_putstring("Driver [CLK CMD DAT CDWP] = [%d %d %d] \r\n", emmc_cfg.clk_drv,
                         emmc_cfg.cmd_drv, emmc_cfg.dat_drv);
        eshell_putstring("VCC Main       : %d(mV) \r\n", emmc_cfg.vcc_main);
        eshell_putstring("Block Size     : %d   \r\n", emmc_cfg.blksz);
        eshell_putstring("Block Count    : %d   \r\n", emmc_cfg.blks);
        eshell_putstring("Card Offset    : 0x%0X \r\n", emmc_cfg.card_offset);
        eshell_putstring("Seed           : 0x%0X \r\n", emmc_cfg.seed);
        eshell_putstring("Pattern        : 0x%0X \r\n", emmc_cfg.pattern);
        eshell_putstring("DMA Mode       : 0x%0X \r\n", emmc_cfg.dma_mode);
        eshell_putstring("DMA BDRY       : %d \r\n", emmc_cfg.dma_bdry);
        eshell_putstring("Auto CMD Type  : 0x%0X \r\n", emmc_cfg.auto_cmd_type);
        eshell_putstring("ADMA Desc Type : 0x%0X \r\n", emmc_cfg.desc_type);
        eshell_putstring("ADMA Desc SZ   : 0x%0X \r\n", emmc_cfg.desc_sz);
        eshell_putstring("ADMA Desc BR   : 0x%0X  \r\n", emmc_cfg.desc_nr);
        eshell_putstring("Bus Mode       : 0x%0X \r\n", emmc_cfg.timing);
        eshell_putstring("Bus Frequency  : %d(KHz) \r\n", emmc_cfg.khz);
        if (emmc_cfg.width == MMC_1BIT_WIDTH)
        {
            eshell_putstring("Bus Width      : 1bit \r\n");
        }
        else if (emmc_cfg.width == MMC_4BIT_WIDTH)
        {
            eshell_putstring("Bus Width      : 4bit \r\n");
        }
        else if (emmc_cfg.width == MMC_8BIT_WIDTH)
        {
            eshell_putstring("Bus Width      : 8bit \r\n");
        }
        eshell_putstring("Bus Voltage    : %d(mV) \r\n", emmc_cfg.voltage);
        eshell_putstring("Bus strobe     : 0x%0X \r\n", emmc_cfg.strobe);
        eshell_putstring("Bus quirks     : 0x%0X \r\n", emmc_cfg.quirks);
        eshell_putstring("********************************** \r\n");
        break;
    default:
        break;
    }

    sdhci_host_init(false, host);
    emmc_poweroff_device(host);

    return 0;
}

static int mmc_card_init()
{
    struct mmc_host *host = &emmc_test_op;
    struct mmc_card_info card_info;
    if (host->id != 0)
    {
        sdhci_host_init(false, host);
        emmc_poweroff_device(host);
    }
    int ret = MMC_NO_ERR;
    memset((void *)host, 0, sizeof(struct mmc_host));
    mmc_init_host(host);
    eshell_putstring("-------------------\r\n");
    emmc_poweron_device(host);
    emmc_rst_pwr_pin_init(host);
    emmc_reset_device(host);

    // card type
    ret = sdhci_platform_init(true, host);
    if (ret != MMC_NO_ERR)
    {
        eshell_putstring("eMMC platform init error\r\n");
        return ret;
    }
    else
    {
        eshell_putstring("eMMC BASE      : 0x%0X \r\n", host->base);
        eshell_putstring("eMMC ctrl ver  : 0x%0X \r\n", host->version);
        eshell_putstring("eMMC irq       : 0x%X \r\n", host->irq);
        eshell_putstring("eMMC platform init success\r\n");
    }

    ret = sdhci_host_init(true, host);
    if (ret != MMC_NO_ERR)
    {
        eshell_putstring("eMMC host init error\r\n");
        return ret;
    }
    else
        eshell_putstring("eMMC host init success\r\n");

    ret = detect_card(host);
    if (ret != MMC_NO_ERR)
    {
        if (ret == MMC_CMD_ERR)
            eshell_putstring("eMMC init: CMD0 GO IDLE STATE ERROR\r\n");
        else if (ret == MMC_DETECT_CARD_ERR)
            eshell_putstring("eMMC init: CMD1 SEND OP COND ERROR\r\n");
        return ret;
    }
    else
    {
        eshell_putstring("eMMC init:detect_card done,CMD0,CMD1 SEND SUCCESS\r\n");
    }
    init_card_info(host);
    host->card->cid.mid = 0;
    host->card_info->relcardadd = 0;
    host->card_info->cardclass = 0;
    host->card_info->ext_csd_rev = 0;
    host->card->width = MMC_8BIT_WIDTH;
    ret = init_card(host);
    get_card_info(&card_info);
    // eshell_putstring("host->card->cid.mid=%u,host->card_info->relcardadd =
    // %u,host->card_info->cardclass=
    // %u,host->card_info->ext_csd_rev=%u",host->card->cid.mid,host->card_info->relcardadd
    // ,host->card_info->cardclass,host->card_info->ext_csd_rev);
    if (ret != MMC_NO_ERR)
    {
        if (ret == MMC_DETECT_CARD_ERR)
            eshell_putstring("Unknown Card Type\r\n");
        if (ret == MMC_CMD_ERR)
        {
            if (!host->card->cid.mid)
                eshell_putstring("eMMC init: CMD2 ALL SEND CID ERROR\r\n");
            else if (host->card_info->relcardadd == 0)
                eshell_putstring("eMMC init: CMD3 SET RELATIVE ADDR ERROR\r\n");
            else if (ret == MMC_CARD_STATUS_ERR && !host->card_info->cardclass)
                eshell_putstring("eMMC init: Card is NOT in STAND-BY mode!!\r\n");
            else if (host->card_info->cardclass == 0)
                eshell_putstring("eMMC init: CMD9 SEND CSD ERROR\r\n");
            else if (ret == MMC_CARD_STATUS_ERR)
                eshell_putstring("eMMC init: Card is NOT in TRANS-STATE mode!!\r\n");
            else if (host->card_info->ext_csd_rev == 0)
                eshell_putstring("eMMC init: CMD8 SEND EXT CSD ERROR\r\n");
            else if (host->card->width != MMC_1BIT_WIDTH)
                eshell_putstring("eMMC init: CD16 SET BLOCKLEN ERROR\r\n");
            else
                eshell_putstring("eMMC init: CMD7 SELECT CARD ERROR\r\n");
        }

        return ret;
    }
    else
    {
        eshell_putstring("eMMC init: initialize card device done\r\n");
        eshell_putstring("eMMC init: CMD2,CMD3,CMD7,CMD8,CMD9,CMD16 SEND SUCCESS\r\n");
        eshell_putstring("eMMC init: success\r\n");
    }
    eshell_putstring("-------------------\r\n");
    sdhci_host_init(false, host);
    emmc_poweroff_device(host);
    return 0;
}

static void unitest_emmc_usage(void)
{
    eshell_putstring("Usage: \r\n");
    eshell_putstring(
        "  test_Dev_Info : < utest_emmc dev >         Obtaining Device Information;\r\n");
    eshell_putstring(
        "  test_CID_Info : < utest_emmc cid >         Obtain device CID register information;\r\n");
    eshell_putstring(
        "  test_CSD_Info : < utest_emmc csd >         Obtain device CSD register information;\r\n");
    eshell_putstring("  test_HOST_Info: < utest_emmc host_cfg >    Obtain device host "
                     "configuration information;\r\n");
    eshell_putstring("  test_SPEED   : < utest_emmc speed_mode once/circulation "
                     "DS/SDR52/DDR52/HS200) >   select "
                     "DS|SDR52|DDR52|HS200 speed mode to test;\r\n");
    eshell_putstring("  test_INIT     : < utest_emmc init >        Initialize the EMMC device, and "
                     "Print error message;\r\n");
    eshell_putstring("  test_write    : < utest_emmc write  start_addr  blk speed(0~3-DS~HS200) "
                     "width(1,4,8) >      EMMC write test;\r\n");
    eshell_putstring("  test_read     : < utest_emmc read   start_addr  blk speed(0~3-DS~HS200) "
                     "width(1,4,8) >      EMMC read test;\r\n");
    eshell_putstring("  test_earse    : < utest_emmc erase  start_addr  blk speed(0~3-DS~HS200) "
                     "width(1,4,8) >     EMMC erase test;\r\n");
    eshell_putstring("  sleep_awake   : < utest_emmc  sleep_awaken"
                     "sleep 10s to mesure sleep status "
                     "then awake to once write read;\r\n");
    eshell_putstring("defaule use 48M Fpclk  test sdr/dd OSC_26M_X4_AUD2BB=1 \
                      test hs200 EMMC_USE_PLL=1");

}

static int emmc_test(int cmd, uint32_t speed_mode, int out_clk)
{
    int ret = MMC_NO_ERR;
    struct mmc_host *host = &emmc_test_op;
    switch (speed_mode)
    {
    case 0:
        emmc_cfg.timing = DS;
        emmc_cfg.khz = F_DS;
        break;

    case 1:
        emmc_cfg.timing = SDR52;
        emmc_cfg.khz = F_SDR52;
        break;

    case 2:
        emmc_cfg.timing = DDR52;
        emmc_cfg.khz = F_DDR52;
        break;

    case 3:
        emmc_cfg.timing = HS200;
        emmc_cfg.khz = F_HS200;
        break;

    default:
        break;
    }
    eshell_putstring("Start emmc speed test \r\n");
    eshell_putstring("------emmc test started------\r\n");
    memset((void *)host, 0, sizeof(struct mmc_host));
    mmc_init_host(host);

    if (host->vmin_begin)
    {
        switch (emmc_cfg.timing)
        {
        case DS:
            eshell_putstring("EMMC DS mode\r\n");
            break;
        case SDR52:
            eshell_putstring("EMMC SDR52 mode\r\n");
            break;
        case DDR52:
            eshell_putstring("EMMC DDR52 mode\r\n");
            break;
        case HS200:
            eshell_putstring("EMMC HS200 mode\r\n");
            break;
        case HS400:
        case EHS400:
            eshell_putstring("EMMC HS400 mode\r\n");
            break;
        default:
            break;
        }
    }
    eshell_putstring("emmc base: 0x%X\r\n", EMMC_BASE);
    eshell_putstring("    MMC Controller : %d  \r\n", emmc_cfg.controller);
    eshell_putstring("    Real Card      : %s  \r\n", (emmc_cfg.real_card == 1) ? "yes" : "no");
    eshell_putstring("    SRC Memory     : %d  \r\n", emmc_cfg.src_region);
    eshell_putstring("    TGT Memory     : %d  \r\n", emmc_cfg.dst_region);
    eshell_putstring("    Driver [CLK CMD DAT CDWP] = [%d %d %d]\r\n", emmc_cfg.clk_drv,
                     emmc_cfg.cmd_drv, emmc_cfg.dat_drv);
    eshell_putstring("    Block Size     : %d  \r\n", emmc_cfg.blksz);
    eshell_putstring("    Block Count    : %d  \r\n", emmc_cfg.blks);
    eshell_putstring("    DMA Mode       : 0x%0X\r\n", emmc_cfg.dma_mode);
    eshell_putstring("    Bus Mode       : 0x%0X\r\n", emmc_cfg.timing);
    eshell_putstring("    Bus Frequency  : %d(KHz)\r\n", emmc_cfg.khz);
    if (emmc_cfg.width == MMC_1BIT_WIDTH)
    {
        eshell_putstring("    Bus Width      : 1bit\r\n");
    }
    else if (emmc_cfg.width == MMC_4BIT_WIDTH)
    {
        eshell_putstring("    Bus Width      : 4bit\r\n");
    }
    else if (emmc_cfg.width == MMC_8BIT_WIDTH)
    {
        eshell_putstring("    Bus Width      : 8bit\r\n");
    }
    eshell_putstring("    Bus Voltage    : %d(mV)\r\n", emmc_cfg.voltage);
    eshell_putstring("    Bus strobe     : 0x%0X\r\n", emmc_cfg.strobe);
    eshell_putstring("    Bus quirks     : 0x%0X\r\n", emmc_cfg.quirks);

    // emmc clock output
    if (out_clk)
    {
        hal_iomux_set_clock_out();
        hal_cmu_clock_out_enable(HAL_CMU_CLOCK_OUT_SYS_EMMC);
    }
    // read_write random
    srand(hal_sys_timer_get() * hal_fast_sys_timer_get());
    // do this
    if (cmd)
        ret = mmc_test_start(SDHCI_LOOP_TEST, host);
    else
        ret = mmc_test_start(SDHCI_ONCE_TEST, host);

    eshell_putstring("emmc test error code:%d\r\n", host->error_code);
    eshell_putstring("emmc test results:%d\r\n", ret);
    eshell_putstring("----------emmc test completed----------\r\n");

    // to test other
    sdhci_host_init(false, host);
    emmc_poweroff_device(host);
    return 0;
}

static int sdhci_data_readwrite_test(EMMC_INSTRUCTION emmc_inst, struct mmc_host *host,
                                     uint32_t addr, uint32_t cnt, uint32_t speed, uint32_t width)
{
    int ret = MMC_NO_ERR;
    POSSIBLY_UNUSED uint8_t retry_cnt;
    uint32_t i;

    uint32_t size = cnt * host->blksz;
    uint32_t len = size / sizeof(uint32_t);

    uint32_t *src_mem = (uint32_t *)emmc_src_buf;
    uint32_t *dst_mem = (uint32_t *)emmc_dst_buf;

    uint32_t *src = (uint32_t *)src_mem;
    uint32_t *dst = (uint32_t *)dst_mem;

    eshell_putstring("%s: src=%x, dst=%x\r\n", __FUNCTION__, (uint32_t)src, (uint32_t)dst);
    memset(src, 0x0, len);
    memset(dst, 0x0, len);

    struct emmc_config *cfg;
    cfg = (struct emmc_config *)host->private;

    switch (speed)
    {
    case 0:
        cfg->timing = DS;
        cfg->khz = F_DS;
        eshell_putstring("EMMC DS mode\r\n");
        break;
    case 1:
        cfg->timing = SDR52;
        cfg->khz = F_SDR52;
        eshell_putstring("EMMC SDR52 mode\r\n");
        break;
    case 2:
        cfg->timing = DDR52;
        cfg->khz = F_DDR52;
        eshell_putstring("EMMC DDR52 mode\r\n");
        break;
    case 3:
        cfg->timing = HS200;
        cfg->khz = F_HS200;
        eshell_putstring("EMMC HS200 mode\r\n");
        break;

    default:
        break;
    }

    switch (width)
    {
    case 1:
        cfg->width = MMC_1BIT_WIDTH;
        break;
    case 4:
        cfg->width = MMC_4BIT_WIDTH;
        break;
    case 8:
        cfg->width = MMC_8BIT_WIDTH;
        break;
    default:
        break;
    }

    ret = config_card(host, cfg->timing, cfg->khz, cfg->width);
    if (ret != MMC_NO_ERR)
    {
        eshell_putstring("%s: config card failed, %d\r\n", __FUNCTION__, ret);
        goto _exit;
    }

    mmc_test_info(host);

    // fill src,dst data
    for (i = 0; i < len; i++)
    {
        src[i] = 0xAA55AA55;
        dst[i] = 0x0;
    }

    retry_cnt = 0;
    error_flag = 0;
    cfg->card_offset = addr;

    if (emmc_inst == EMMC_WRITE)
    {
        eshell_putstring("  \r\n");
        eshell_putstring("=================> writing card\r\n");
        do
        {
            eshell_putstring("++++++card_offset=0x%x, buf=0x%x, sz=%d\r\n", cfg->card_offset,
                             (uint32_t)src, size);

            ret = write_card_dma_mode(host, cfg->card_offset, (void *)src, size);

            if (error_flag)
            {
                error_flag = 0;

                retry_cnt++;
                if (retry_cnt > RETRY_CNT_MAX)
                {
                    goto write_error;
                }
                eshell_putstring("******line: %d,tx/rx error, try again: %d\r\n", __LINE__,
                                 retry_cnt);
                host->delay(1000);
                eshell_putstring("  ");
                continue;
            }
            else
            {
                while (!dma_done_flag)
                {
                }
                dma_done_flag = 0;
            }

write_error:
            if (ret != MMC_NO_ERR)
            {
                eshell_putstring("write card failed, ret=%d\r\n", ret);
                while (1)
                    ;
            }
            else
            {
                eshell_putstring("write card success !!!\r\n");
                break;
            }

        }
        while (1);
    }
    else if (emmc_inst == EMMC_READ)
    {
        eshell_putstring("==================> reading card\r\n");
        retry_cnt = 0;
        error_flag = 0;
        dma_done_flag = 0;
        do
        {
            ret = read_card_dma_mode(host, cfg->card_offset, (void *)dst, size);
            if (error_flag)
            {
                error_flag = 0;
                retry_cnt++;
                if (retry_cnt > RETRY_CNT_MAX)
                    goto read_error;

                eshell_putstring("******line: %d,tx/rx error, try again: %d\r\n", __LINE__,
                                 retry_cnt);
                host->delay(1000);
                eshell_putstring("  ");
                continue;
            }
            else
            {
                while (!dma_done_flag)
                {
                }
                dma_done_flag = 0;
            }

read_error:
            if (ret != MMC_NO_ERR)
            {
                eshell_putstring("read card failed, ret=%d\r\n", ret);
                while (1)
                    ;
            }
            else
            {
                for (int i = 0; i < len; i++)
                    eshell_putstring("addr: 0x%x,read data=0x%x\r\n",
                                     cfg->card_offset + i * sizeof(uint32_t), dst[i]);
                eshell_putstring("read card success !!!\r\n");
                break;
            }
        }
        while (1);

    }
    else
    {
        ret = erase_card(host, cfg->card_offset, cfg->card_offset + size);
        if (ret != MMC_NO_ERR)
            eshell_putstring("erase card failed, ret=%d\r\n", ret);
        else
            eshell_putstring("erase card success \r\n");
    }

_exit:
    return ret;
}

static int emmc_readwrite_test(EMMC_INSTRUCTION emmc_inst, uint32_t addr, uint32_t cnt,
                               uint32_t speed, uint32_t width)
{
    int ret = MMC_NO_ERR;
    struct mmc_host *host = &emmc_test_op;
    switch (speed)
    {
    case 0:
        emmc_cfg.timing = DS;
        emmc_cfg.khz = F_DS;
        eshell_putstring("EMMC DS mode\r\n");
        break;
    case 1:
        emmc_cfg.timing = SDR52;
        emmc_cfg.khz = F_SDR52;
        eshell_putstring("EMMC SDR52 mode\r\n");
        break;
    case 2:
        emmc_cfg.timing = DDR52;
        emmc_cfg.khz = F_DDR52;
        eshell_putstring("EMMC DDR52 mode\r\n");
        break;
    case 3:
        emmc_cfg.timing = HS200;
        emmc_cfg.khz = F_HS200;
        eshell_putstring("EMMC HS200 mode\r\n");
        break;
    default:
        break;
    }
    eshell_putstring("------emmc test started------\r\n");
    memset((void *)host, 0, sizeof(struct mmc_host));

    mmc_init_host(host);

    eshell_putstring("SDHCI DATA test\r\n");
    ret = sdhci_data_xfer_setup(host);
    if (ret != MMC_NO_ERR)
        return ret;

    ret = sdhci_data_readwrite_test(emmc_inst, host, addr, cnt, speed, width);
    if (ret != MMC_NO_ERR)
    {
        return ret;
    }

    ret = sdhci_data_xfer_clean(host);
    if (ret != MMC_NO_ERR)
        return ret;

    sdhci_host_init(false, host);
    emmc_poweroff_device(host);
    return 0;
}

static int mmc_test_sleep_awake(void)
{
    int ret = MMC_NO_ERR;
    struct mmc_host *host = &emmc_test_op;
    memset((void *)host, 0, sizeof(struct mmc_host));
    mmc_init_host(host);
    ret = sdhci_data_xfer_setup(host);
    if (ret != MMC_NO_ERR)
        return ret;

    eshell_putstring("emmc sleep\r\n");
    sleep_card(host);
    sdhci_host_emphy_sleep(host);
    host->delay(10000);
    eshell_putstring("delay 10s to measure\r\n");
    sdhci_host_emphy_wakeup(host);
    host->delay(2);
    eshell_putstring("emmc phy enable\r\n");
    awake_card(host);
    ret = sdhci_data_xfer_test(host, true);
    if (ret != MMC_NO_ERR)
        return ret;

    ret = sdhci_data_xfer_clean(host);
    if (ret != MMC_NO_ERR)
        return ret;
    eshell_putstring("emmc sleep again \r\n");
    sleep_card(host);
    sdhci_force_clk_off(host);
    eshell_putstring("close clk \r\n");
    sdhci_host_init(false, host);
    eshell_putstring("close emmc \r\n");

    return 0;
}

static void utest_emmc(int argc, char *argv[])
{
    if (argc < 2)
        goto usage;

    // Obtaining Device Information
    if (strncmp(argv[1], "dev", 3) == 0)
    {
        eshell_putstring("Start obtaining dev information \r\n");
        mmc_read_card_info(EMMC_DEV_INFO);
        return;
    }

    // Obtain device CID register information
    if (strncmp(argv[1], "cid", 3) == 0)
    {
        eshell_putstring("Start obtaining cid information \r\n");
        mmc_read_card_info(EMMC_CID);
        return;
    }

    // Obtain device CSD register informatio
    if (strncmp(argv[1], "csd", 3) == 0)
    {
        eshell_putstring("Start obtaining csd information \r\n");
        mmc_read_card_info(EMMC_CSD);
        return;
    }

    // Obtain device host configuration informatio
    if (strncmp(argv[1], "host_cfg", 8) == 0)
    {
        eshell_putstring("Start obtaining host information \r\n");
        mmc_read_card_info(EMMC_STATUS_INFO);
        return;
    }

    // Command is used to MMC device in DS | SDR52 | DDR52 | HS200 speed mode, speaking, reading and
    // writing test block;
    if (strncmp(argv[1], "speed", 5) == 0)
    {
        uint32_t rotate = 0;
        if (argc != 4)
            goto usage;
        rotate = atoi(argv[2]);

        if (!rotate)
        {
            if (strncmp(argv[3], "DS", 2) == 0)
                emmc_test(0, 0, 0);
            else if (strncmp(argv[3], "SDR52", 5) == 0)
                emmc_test(0, 1, 0);
            else if (strncmp(argv[3], "DDR52", 5) == 0)
                emmc_test(0, 2, 0);
            else if (strncmp(argv[3], "HS200", 5) == 0)
                emmc_test(0, 3, 0);
            else
                goto usage;
        }
        else
        {
            if (strncmp(argv[3], "DS", 2) == 0)
                emmc_test(1, 0, 1);
            else if (strncmp(argv[3], "SDR52", 5) == 0)
                emmc_test(1, 1, 1);

            else if (strncmp(argv[3], "DDR52", 5) == 0)
                emmc_test(1, 2, 1);
            else if (strncmp(argv[3], "HS200", 5) == 0)
                emmc_test(1, 3, 1);
            else
                goto usage;
        }
        return;
    }

    // Initialize the EMMC device, and Print error message
    if (strncmp(argv[1], "init", 4) == 0)
    {
        eshell_putstring("Start emmc init test \r\n");
        mmc_card_init();
        return;
    }

    // emmc write test
    if (strncmp(argv[1], "write", 5) == 0)
    {
        uint32_t addr;
        uint32_t cnt;
        uint32_t speed;
        uint32_t width;

        if (argc != 6)
            goto usage;
        addr = atoi(argv[2]);
        cnt = atoi(argv[3]);
        speed = atoi(argv[4]);
        width = atoi(argv[5]);
        if ((width == 1 && speed > 1) || (width != 1 && width != 4 && width != 8) || speed > 3)
            goto usage;
        eshell_putstring("Start emmc write test \r\n");
        emmc_readwrite_test(EMMC_WRITE, addr, cnt, speed, width);
        return;
    }

    // emmc read test
    if (strncmp(argv[1], "read", 4) == 0)
    {
        uint32_t addr;
        uint32_t cnt;
        uint32_t speed;
        uint32_t width;

        if (argc != 6)
            goto usage;
        addr = atoi(argv[2]);
        cnt = atoi(argv[3]);
        speed = atoi(argv[4]);
        width = atoi(argv[5]);
        if ((width == 1 && speed > 1) || (width != 1 && width != 4 && width != 8) || speed > 3)
            goto usage;
        eshell_putstring("Start emmc read test \r\n");
        emmc_readwrite_test(EMMC_READ, addr, cnt, speed, width);
        return;
    }

    // emmc erase test
    if (strncmp(argv[1], "erase", 5) == 0)
    {
        uint32_t addr;
        uint32_t cnt;
        uint32_t speed;
        uint32_t width;

        if (argc != 6)
            goto usage;
        addr = atoi(argv[2]);
        cnt = atoi(argv[3]);
        speed = atoi(argv[4]);
        width = atoi(argv[5]);
        if ((width == 1 && speed > 1) || (width != 1 && width != 4 && width != 8) || speed > 3)
            goto usage;
        uint32_t Multiple = addr / 512;
        uint32_t remainder = addr % 512;
        if (remainder)
        {
            if (Multiple)
            {
                addr = Multiple * 512;
                cnt = cnt + 1;
            }
            else
            {
                addr = 0;
                cnt = cnt + 1;
            }
        }

        eshell_putstring("Start emmc erase test \r\n");
        emmc_readwrite_test(EMMC_ERASE, addr, cnt, speed, width);
        return;
    }

    // emmc sleep awake
    if (strncmp(argv[1], "sleep_awake", 11) == 0)
    {
        eshell_putstring("emmc sleep awake test start");
        mmc_test_sleep_awake();
        return;
    }

    if (strncmp(argv[1], "write_read", 10) == 0)
    {
        eshell_putstring("emmc write read test start");
        mmc_test_sleep_awake();
        return;
    }

usage:
    unitest_emmc_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_emmc", "usage: utest_emmc help", utest_emmc);
#endif
