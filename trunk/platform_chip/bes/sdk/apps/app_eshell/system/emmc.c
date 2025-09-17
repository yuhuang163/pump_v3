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
#if defined(UTILS_ESHELL_EN) && defined(CHIP_HAS_EMMC) && defined(__SYS_AS_MAIN__)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "core.h"
#include "host.h"
#include "card.h"
#include "error.h"
#include "sdhci.h"
#include "card_mmc.h"
#include "sdhci_emphy.h"
#include "hal_location.h"
#include "emmc_platform.h"
#include "eshell.h"
#include "app_eshell.h"

//#define EMMC_DEBUG_ENABLE

#define EMMC_TRACE_SLE  2
#if   (EMMC_TRACE_SLE == 1)
    #define EMMC_TRACE(s, ...)       TRACE(1, s , ##__VA_ARGS__)
#elif (EMMC_TRACE_SLE == 2)
    #define EMMC_TRACE(s, ...)       eshell_putstring(s"\r\n" , ##__VA_ARGS__)
#else
    #define EMMC_TRACE(s, ...)
#endif

#ifdef EMMC_DEBUG_ENABLE
#define EMMC_DEBUG  EMMC_TRACE
#else
#define EMMC_DEBUG(...)
#endif

#if defined(EMMC_HS_MODE) && defined(EMMC_USE_PLL)
#define DEF_BUS_MODE    HS200
#else
#define DEF_BUS_MODE    SDR52
#endif
#define DEF_BUS_WIDTH   MMC_8BIT_WIDTH /* MMC_4BIT_WIDTH, MMC_1BIT_WIDTH */

/* used to test different interfaces */
//#define EMMC_TEST_BLOCKING_MODE
#define EMMC_TEST_DMA_MODE
#define EMMC_CALLBACK_ENABLE

/* Used to test the reading and writing function after opening EMMC's internal cache */
//#define EMMC_CACHE_ENABLE

/* Used to test the descriptor scatter function of ADMA2 */
//#define EMMC_ADMA_DESC_MODE

#if defined(EMMC_ADMA_DESC_MODE) && defined(CORE_SLEEP_POWER_DOWN)
    #error "The ADMA test does not support in the PD mode, because the memory is too small"
#endif

#if defined(EMMC_TEST_BLOCKING_MODE) && defined(EMMC_TEST_DMA_MODE)
    #error "Blocking mode and dma mode cannot be used at the same time"
#elif !defined(EMMC_TEST_BLOCKING_MODE) && !defined(EMMC_TEST_DMA_MODE) && defined(EMMC_CALLBACK_ENABLE)
    #error "EMMC_CALLBACK_ENABLE cannot be defined in this mode"
#endif

static struct mmc_host emmc_host_dev;
static struct mmc_card_info card_info;

static void emmc_delay(uint32_t ms)
{
    osDelay(ms);
}

static volatile uint8_t host_error_flag = 0;
static void emmc_host_error(enum EMMC_HOST_ERR error)
{
    host_error_flag = 1;
    EMMC_TRACE("%s: error = %d", __FUNCTION__, error);
}

#ifdef EMMC_CALLBACK_ENABLE
static volatile uint8_t dma_done_flag = 0;
static void emmc_host_dma_done(void)
{
    dma_done_flag = 1;
}
#endif


#define EMMC_RESET_PIN  HAL_IOMUX_PIN_P3_6
#ifdef EMMC_EXTENDED_USE
#define EMMC_3P3_PIN    HAL_IOMUX_PIN_PMU_P6_5
#define EMMC_1P8_PIN    HAL_IOMUX_PIN_PMU_P5_6
#define EMMC_PWR_ON_LEVEL 1
#else
#define EMMC_3P3_PIN    HAL_IOMUX_PIN_PMU_NUM
#define EMMC_1P8_PIN    HAL_IOMUX_PIN_PMU_NUM
#define EMMC_PWR_ON_LEVEL 0
#endif

static void emmc_rst_pwr_pin_init(struct mmc_host *host)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP iomux[] = {
        {HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };

    //emmc reset pin
    if (EMMC_RESET_PIN < HAL_IOMUX_PIN_NUM) {
        iomux[0].pin = EMMC_RESET_PIN;
        hal_iomux_init(iomux, ARRAY_SIZE(iomux));
    }

    if (EMMC_RESET_PIN < HAL_IOMUX_PIN_PMU_NUM)
        hal_gpio_pin_set_dir(EMMC_RESET_PIN, HAL_GPIO_DIR_OUT, 1);

    //emmc 3.3v power contrl pin
    if (EMMC_3P3_PIN < HAL_IOMUX_PIN_NUM) {
        iomux[0].pin = EMMC_3P3_PIN;
        hal_iomux_init(iomux, ARRAY_SIZE(iomux));
    }

    if (EMMC_3P3_PIN < HAL_IOMUX_PIN_PMU_NUM)
        hal_gpio_pin_set_dir(EMMC_3P3_PIN, HAL_GPIO_DIR_OUT, 1);

    //emmc 1.8v power contrl pin
    if (EMMC_1P8_PIN < HAL_IOMUX_PIN_NUM) {
        iomux[0].pin = EMMC_1P8_PIN;
        hal_iomux_init(iomux, ARRAY_SIZE(iomux));
    }

    if (EMMC_1P8_PIN < HAL_IOMUX_PIN_PMU_NUM)
        hal_gpio_pin_set_dir(EMMC_1P8_PIN, HAL_GPIO_DIR_OUT, 1);
}

static void emmc_reset_device(struct mmc_host *host)
{
    hal_gpio_pin_clr(EMMC_RESET_PIN);
    hal_sys_timer_delay_us(5);
    hal_gpio_pin_set(EMMC_RESET_PIN);

    EMMC_DEBUG("%s: done", __FUNCTION__);
}

static void emmc_poweron_device(struct mmc_host *host)
{
    //emmc 3.3v power contrl pin
    if (EMMC_3P3_PIN < HAL_IOMUX_PIN_PMU_NUM) {
        if (EMMC_PWR_ON_LEVEL)
            hal_gpio_pin_set(EMMC_3P3_PIN);
        else
            hal_gpio_pin_clr(EMMC_3P3_PIN);
    }

    //emmc 1.8v power contrl pin
    if (EMMC_1P8_PIN < HAL_IOMUX_PIN_PMU_NUM) {
        if (EMMC_PWR_ON_LEVEL)
            hal_gpio_pin_set(EMMC_1P8_PIN);
        else
            hal_gpio_pin_clr(EMMC_1P8_PIN);
    }

    EMMC_DEBUG("%s: done", __FUNCTION__);
}

POSSIBLY_UNUSED static void emmc_poweroff_device(struct mmc_host *host)
{
    //emmc 3.3v power contrl pin
    if (EMMC_3P3_PIN < HAL_IOMUX_PIN_PMU_NUM) {
        if (EMMC_PWR_ON_LEVEL)
            hal_gpio_pin_clr(EMMC_3P3_PIN);
        else
            hal_gpio_pin_set(EMMC_3P3_PIN);
    }

    //emmc 1.8v power contrl pin
    if (EMMC_1P8_PIN < HAL_IOMUX_PIN_PMU_NUM) {
        if (EMMC_PWR_ON_LEVEL)
            hal_gpio_pin_clr(EMMC_1P8_PIN);
        else
            hal_gpio_pin_set(EMMC_1P8_PIN);
    }

    EMMC_DEBUG("%s: done", __FUNCTION__);
}

static void emmc_host_info_dump(struct mmc_host *host)
{
    EMMC_TRACE("-------------------EMMC HOST---------------------------");
    EMMC_TRACE("Card Type                   : %s", mmc_card_type(host->card->type));
    EMMC_TRACE("Data Transfer Bit-Width     : %s", mmc_bitwidth_type(host->ios.width));
    EMMC_TRACE("Data Transfer Bus Mode      : %s", mmc_timing_type(host->ios.timing));
    EMMC_TRACE("Data Transfer ClkRate       : %dMHz", (host->ios.khz) / 1000);
    EMMC_TRACE("Data Transfer Size(Blocks)  : %d", host->blks);
    EMMC_TRACE("Data Transfer Auto CMD Type : %s", mmc_autocmd_type(host->auto_cmd_type));
    if (host->dma_mode == MM4_32_BIT_ADMA_2)
        EMMC_TRACE("ADMA Desc Type              : %s", mmc_adma2_desc_type(host->desc_type));
}

static void emmc_card_info_dump(struct mmc_card_info *info)
{
    uint32_t capacity_val = 0;

    EMMC_TRACE("-------------------EMMC CARD---------------------------");
    EMMC_TRACE("Card type                : %s", info->cardtype ? "1(high capacity)" : "0(low capacity)");
    EMMC_TRACE("Card class               : 0x%X", info->cardclass);
    EMMC_DEBUG("Card relcardAdd          : 0x%X", info->relcardadd);
    EMMC_TRACE("Card blocknbr            : 0x%X", info->blocknbr);
    EMMC_TRACE("Card blocksize           : 0x%X", info->blocksize);
    EMMC_DEBUG("Card logblocknbr         : 0x%X", info->logblocknbr);
    EMMC_DEBUG("Card logllocksize        : 0x%X", info->logllocksize);
    EMMC_TRACE("Card ext_csd_rev         : 0x%X", info->ext_csd_rev);
    EMMC_DEBUG("Card firmware version_h32: 0x%X", info->firmware_version_h32);
    EMMC_DEBUG("Card firmware version_l32: 0x%X", info->firmware_version);
    EMMC_DEBUG("Card device version      : 0x%X", info->device_version);
    EMMC_DEBUG("Card cache size          : 0x%X", info->cache_size);
    EMMC_DEBUG("Card cache ctrl          : %s", info->cache_ctrl ? "1(enabled)" : "0(disable)");
    EMMC_DEBUG("Card cache flush policy  : %s", info->cache_flush_policy ? "1(fifo mode)" : "0(unknown)");
    EMMC_DEBUG("Card generic cmd6 time   : %dms", info->generic_cmd6_time);
    EMMC_DEBUG("csd.csd_structure        : 0x%X", info->csd.csd_structure);
    EMMC_DEBUG("csd.spec_vers            : 0x%X", info->csd.spec_vers);
    EMMC_DEBUG("csd.taac                 : 0x%X", info->csd.taac);
    EMMC_DEBUG("csd.nsac                 : 0x%X", info->csd.nsac);
    EMMC_DEBUG("csd.tran_speed           : 0x%X", info->csd.tran_speed);
    EMMC_DEBUG("csd.ccc                  : 0x%X", info->csd.ccc);
    EMMC_DEBUG("csd.read_bl_len          : 0x%X", info->csd.read_bl_len);
    EMMC_DEBUG("csd.read_bl_partial      : 0x%X", info->csd.read_bl_partial);
    EMMC_DEBUG("csd.write_blk_misalign   : 0x%X", info->csd.write_blk_misalign);
    EMMC_DEBUG("csd.read_blk_misalign    : 0x%X", info->csd.read_blk_misalign);
    EMMC_DEBUG("csd.dsr_imp              : 0x%X", info->csd.dsr_imp);
    EMMC_DEBUG("csd.c_size               : 0x%X", info->csd.c_size);
    EMMC_DEBUG("csd.vdd_r_curr_min       : 0x%X", info->csd.vdd_r_curr_min);
    EMMC_DEBUG("csd.vdd_r_curr_max       : 0x%X", info->csd.vdd_r_curr_max);
    EMMC_DEBUG("csd.vdd_w_curr_min       : 0x%X", info->csd.vdd_w_curr_min);
    EMMC_DEBUG("csd.vdd_w_curr_max       : 0x%X", info->csd.vdd_w_curr_max);
    EMMC_DEBUG("csd.c_size_mult          : 0x%X", info->csd.c_size_mult);
    EMMC_DEBUG("csd.erase_grp_size       : 0x%X", info->csd.erase_grp_size);
    EMMC_DEBUG("csd.erase_grp_mult       : 0x%X", info->csd.erase_grp_mult);
    EMMC_DEBUG("csd.wp_grp_size          : 0x%X", info->csd.wp_grp_size);
    EMMC_DEBUG("csd.wp_grp_enable        : 0x%X", info->csd.wp_grp_enable);
    EMMC_DEBUG("csd.default_ecc          : 0x%X", info->csd.default_ecc);
    EMMC_DEBUG("csd.r2w_factor           : 0x%X", info->csd.r2w_factor);
    EMMC_DEBUG("csd.write_bl_len         : 0x%X", info->csd.write_bl_len);
    EMMC_DEBUG("csd.write_bl_partial     : 0x%X", info->csd.write_bl_partial);
    EMMC_DEBUG("csd.content_prot_app     : 0x%X", info->csd.content_prot_app);
    EMMC_DEBUG("csd.file_format_grp      : 0x%X", info->csd.file_format_grp);
    EMMC_DEBUG("csd.copy                 : 0x%X", info->csd.copy);
    EMMC_DEBUG("csd.perm_write_protect   : 0x%X", info->csd.perm_write_protect);
    EMMC_DEBUG("csd.tmp_write_protect    : 0x%X", info->csd.tmp_write_protect);
    EMMC_DEBUG("csd.file_format          : 0x%X", info->csd.file_format);
    EMMC_DEBUG("csd.ecc                  : 0x%X", info->csd.ecc);
    EMMC_TRACE("Manufacturer ID          : 0x%X", info->cid.mid);
    EMMC_DEBUG("OEM/Application ID       : 0x%X", info->cid.oid & 0xFF);
    EMMC_DEBUG("Device/BGA               : 0x%X", info->cid.cbx);
    EMMC_TRACE("Product Name             : %c%c%c%c%c%c",
                                        info->cid.pnm[0],
                                        info->cid.pnm[1],
                                        info->cid.pnm[2],
                                        info->cid.pnm[3],
                                        info->cid.pnm[4],
                                        (info->cid.pnm[5] & 0x7E) < 0x20 ? ' ' : (info->cid.pnm[5] & 0x7E)
                                        );
    EMMC_TRACE("Product Revision         : %d.%d", (info->cid.prv >> 4), (info->cid.prv & 0xF));
    EMMC_TRACE("Product Serial No        : 0x%X", info->cid.psn);
    EMMC_TRACE("Manufacture Date         : %d/%d", 2000 + (info->cid.mdt >> 4), info->cid.mdt & 0xF);

    capacity_val = (uint32_t)(((uint64_t)info->blocknbr * info->blocksize * 100) / 1073741824);
    EMMC_TRACE("Card capacity            : %d.%dGB", capacity_val / 100, capacity_val % 100);
}

static int emmc_host_init(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    emmc_rst_pwr_pin_init(host);
    emmc_poweron_device(host);
    emmc_reset_device(host);

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

#ifdef EMMC_CACHE_ENABLE
    ret = card_cache_control(host, 1);
    EMMC_DEBUG("EMMC cache open result: %d(%s)", ret, ret ? "failed" : "success");
#endif
    EMMC_DEBUG("EMMC host init okay!");
    return ret;
}

static int emmc_host_read(struct mmc_host *host, uint32_t offset,
            uint32_t size, uint8_t *buf)
{
    int ret = MMC_NO_ERR;
    POSSIBLY_UNUSED uint32_t timeb, time = 0xffffffff;

    EMMC_TRACE("EMMC Read: offset 0x%x, size 0x%x, to address %p",
                offset, size, buf);

    host_error_flag = 0;
#ifdef EMMC_CALLBACK_ENABLE
    dma_done_flag = 0;
#endif

    timeb = hal_fast_sys_timer_get();

#if defined(EMMC_TEST_BLOCKING_MODE)
    ret = read_card_blocking_mode(host, offset, (void *)buf, size);
#elif defined(EMMC_TEST_DMA_MODE)
    ret = read_card_dma_mode(host, offset, (void *)buf, size);
#ifdef EMMC_CALLBACK_ENABLE
    if (ret == MMC_NO_ERR) {
        if (host_error_flag) {
            EMMC_TRACE("EMMC Read: DMA xfer error.");
            ret = -1;
        } else {
            int timeout_ms = 2000;
            while (!dma_done_flag && timeout_ms-- > 0)
                osDelay(1);
            if (timeout_ms <= 0)
                ret = -1;
        }
    }
#endif
#else
        ret = read_card(host, offset, (void *)buf, size);
#endif

    time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);

    if (ret != MMC_NO_ERR) {
        EMMC_TRACE("EMMC Read: failed, ret = %d", ret);
    } else {
            EMMC_DEBUG("EMMC Read: completed!");
    }

    EMMC_DEBUG("---------------EMMC Read------------------------");
    EMMC_DEBUG("Size      :%d bytes", size);
    EMMC_DEBUG("Time      :read[%dus]", time);
    time = (uint32_t)(95400 * size / 1000 / time);
    EMMC_DEBUG("Speed     :%u.%uMB/s", time / 100, time % 100);
    EMMC_DEBUG("------------------------------------------------");
    return ret;
}

static int emmc_host_write(struct mmc_host *host, uint32_t offset,
                uint32_t size, const uint8_t *buf)
{
    int ret = MMC_NO_ERR;
    POSSIBLY_UNUSED uint32_t timeb, time = 0xffffffff;

    EMMC_TRACE("EMMC Write: offset 0x%x, size 0x%x, from address %p",
                offset, size, buf);

    host_error_flag = 0;
#ifdef EMMC_CALLBACK_ENABLE
    dma_done_flag = 0;
#endif

    timeb = hal_fast_sys_timer_get();

#if defined(EMMC_TEST_BLOCKING_MODE)
    ret = write_card_blocking_mode(host, offset, (void *)buf, size);
#elif defined(EMMC_TEST_DMA_MODE)
    ret = write_card_dma_mode(host, offset, (void *)buf, size);
#ifdef EMMC_CALLBACK_ENABLE
    if (ret == MMC_NO_ERR) {
        if (host_error_flag) {
            EMMC_TRACE("EMMC Write: DMA xfer error.");
            ret = -1;
        } else {
            int timeout_ms = 2000;
            while (!dma_done_flag && timeout_ms-- > 0)
                osDelay(1);
            if (timeout_ms <= 0)
                ret = -1;
        }
    }
#endif
#else
    ret = write_card(host, offset, (void *)buf, size);
#endif

    time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);

    if (ret != MMC_NO_ERR) {
        EMMC_TRACE("EMMC Write: failed, ret = %d", ret);
    } else {
        EMMC_DEBUG("EMMC Write: completed!");
    }

    EMMC_DEBUG("---------------------EMMC Write--------------------");
    EMMC_DEBUG("Size      :%d bytes", size);
    EMMC_DEBUG("Time      :%dus", time);
    time = (uint32_t)(95400 * size / 1000 / time);
    EMMC_DEBUG("Speed     :%u.%uMB/s", time / 100, time % 100);
    EMMC_DEBUG("---------------------------------------------------");
    return ret;
}

static int emmc_host_erase(struct mmc_host *host, uint32_t offset,
                uint32_t size)
{
    int ret = MMC_NO_ERR;
    POSSIBLY_UNUSED uint32_t timeb, time = 0xffffffff;

    EMMC_TRACE("EMMC Erase: offset 0x%x, size 0x%x", offset, size);

    timeb = hal_fast_sys_timer_get();

    ret = erase_card(host, offset, offset+size);

    time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - timeb);
    if (ret != MMC_NO_ERR)
        EMMC_TRACE("EMMC Erase: failed, ret = %d", ret);
    else
        EMMC_DEBUG("EMMC Erase: completed!");

    EMMC_DEBUG("---------------------EMMC Erase--------------------");
    EMMC_DEBUG("Size     :%d bytes", size);
    EMMC_DEBUG("Time     :%dus", time);
    time = (uint32_t)(95400 * size / 1000 / time);
    EMMC_DEBUG("Speed    :%u.%uMB/s", time / 100, time % 100);
    EMMC_DEBUG("---------------------------------------------------");
    return ret;
}

/*********************************************************/

static bool emmc_inited = false;

bool emmc_is_inited(void)
{
    return emmc_inited;
}

uint32_t emmc_block_count(void)
{
    if (emmc_inited)
        return card_info.blocknbr;
    return 0;
}

uint32_t emmc_block_size(void)
{
    if (emmc_inited)
        return card_info.blocksize;
    return 0;
}

int emmc_write(uint32_t offset, uint32_t size, uint32_t addr)
{
    struct mmc_host *host = &emmc_host_dev;

    if (!emmc_inited) {
        eshell_putstring("EMMC is not inited.\r\n");
        return -1;
    }

    return emmc_host_write(host, offset, size, (const uint8_t *)addr);
}

int emmc_read(uint32_t offset, uint32_t size, uint32_t addr)
{
    struct mmc_host *host = &emmc_host_dev;

    if (!emmc_inited) {
        eshell_putstring("EMMC is not inited.\r\n");
        return -1;
    }

    return emmc_host_read(host, offset, size, (uint8_t *)addr);
}

int emmc_erase(uint32_t offset, uint32_t size)
{
    struct mmc_host *host = &emmc_host_dev;

    if (!emmc_inited) {
        eshell_putstring("EMMC is not inited.\r\n");
        return -1;
    }
    return emmc_host_erase(host, offset, size);
}

void emmc_info(void)
{
    struct mmc_host *host = &emmc_host_dev;

    if (!emmc_inited) {
        eshell_putstring("EMMC is not inited.\r\n");
        return;
    }

    emmc_host_info_dump(host);
    emmc_card_info_dump(&card_info);
}

int emmc_init(uint32_t emmc_mode, uint32_t emmc_width)
{
    int emmc_khz = 0;
    int ret = MMC_NO_ERR;
    struct mmc_host *host = &emmc_host_dev;

    if (emmc_inited) {
        EMMC_TRACE("EMMC is inited!");
        return 0;
    }

    EMMC_TRACE("===========================================================");
    EMMC_TRACE("eMMC5.1/SD3.0/SDIO3.0/CE-ATA1.x (SD Host) Controller Module");
    EMMC_TRACE("===========================================================");

    memset((void *)host, 0, sizeof(struct mmc_host));
    host->id       = DEV_TYPE_EMMC;
#ifdef EMMC_ADMA_DESC_MODE
    host->dma_mode = MM4_32_BIT_ADMA_2;
#else
    host->dma_mode = MM4_SDMA;
#endif
    host->auto_cmd_type = MM4_AUTO_CMD12;
    host->dma_bdry  = MM4_HOST_DMA_BDRY_512KB;
    host->desc_sz   = ADMA2_DESC_MAX_LEN;
    host->desc_nr   = 1;
#ifdef EMMC_ADMA_DESC_MODE
    host->desc_type = ADMA2_DESC_SCATTER;
#else
    host->desc_type = ADMA2_DESC_FIXED_SZ;
#endif
    /* data transfer */
    host->blksz     = CARD_BLOCK_LENGTH;
    host->blks      = 32;
    host->quirks    = MMC_QUIRKS_HW_TUNING | MMC_QUIRKS_PERFORMANCE;
    host->strobe    = 0;
    host->vmin_begin = 1800;
    /* callback */
    host->delay      = emmc_delay;
    host->emmc_host_error   = emmc_host_error;
    host->emmc_host_dma_done = emmc_host_dma_done;
    host->private    = NULL;
    host->error_code = MMC_NO_ERR;
    host->speed_mode = emmc_mode;

    EMMC_DEBUG("emmc base: 0x%X", EMMC_BASE);
    EMMC_DEBUG("    ---------------------");
    EMMC_DEBUG("    VOL min begin  : %d(mV)", host->vmin_begin);
    EMMC_DEBUG("    ---------------------");
    EMMC_DEBUG("    MMC Controller : %d  ", host->id);
    EMMC_DEBUG("    ---------------------");
    EMMC_DEBUG("    Emmc reset pin : %d  ", EMMC_RESET_PIN);
    EMMC_DEBUG("    Emmc 3.3v pin  : %d  ", EMMC_3P3_PIN);
    EMMC_DEBUG("    Emmc 1.8v pin  : %d  ", EMMC_1P8_PIN);
    EMMC_DEBUG("    Emmc pwr on lev: %d  ", EMMC_PWR_ON_LEVEL);
    EMMC_DEBUG("    -------------------");
    EMMC_DEBUG("    Block Size     : %d  ", host->blksz);
    EMMC_DEBUG("    Block Count    : %d  ", host->blks);
    EMMC_DEBUG("    -------------------");
    EMMC_DEBUG("    DMA Mode       : 0x%0X", host->dma_mode);
    EMMC_DEBUG("    DMA BDRY       : %d", host->dma_bdry);
    EMMC_DEBUG("    Auto CMD Type  : 0x%0X", host->auto_cmd_type);
    EMMC_DEBUG("    ADMA Desc Type : 0x%0X", host->desc_type);
    EMMC_DEBUG("    ADMA Desc SZ   : 0x%0X", host->desc_sz);
    EMMC_DEBUG("    ADMA Desc BR   : 0x%0X ", host->desc_nr);
    EMMC_DEBUG("    -------------------");
    EMMC_DEBUG("    Bus Mode       : 0x%0X", host->speed_mode);
    switch(host->speed_mode) {
        case DS:
            EMMC_DEBUG("          EMMC DS mode");
            emmc_khz = F_DS;
            break;
        case SDR52:
            EMMC_DEBUG("          EMMC SDR52 mode");
            emmc_khz = F_SDR52;
            break;
        case DDR52:
            EMMC_DEBUG("          EMMC DDR52 mode");
            emmc_khz = F_DDR52;
            break;
        case HS200:
            EMMC_DEBUG("          EMMC HS200 mode");
            emmc_khz = F_HS200;
            break;
        case HS400:
        case EHS400:
            EMMC_DEBUG("          EMMC HS400 mode");
            emmc_khz = F_HS400;
            break;
        default:
            EMMC_DEBUG("          Wrong EMMC bus mode, abort!");
            return -1;
    }
    switch (emmc_width) {
        case MMC_1BIT_WIDTH:
            EMMC_DEBUG("    Bus Width      : 1bit");
            break;
        case MMC_4BIT_WIDTH:
            EMMC_DEBUG("    Bus Width      : 4bit");
            break;
        case MMC_8BIT_WIDTH:
            EMMC_DEBUG("    Bus Width      : 8bit");
            break;
        default:
            EMMC_DEBUG("    Wrong EMMC bus width, abort!");
            return -1;
    }
    EMMC_DEBUG("    Bus strobe     : 0x%0X", host->strobe);
    EMMC_DEBUG("    Bus quirks     : 0x%0X", host->quirks);
    EMMC_DEBUG("    -------------------");

#ifdef EMMC_CLOCK_OUTPUT
    hal_iomux_set_clock_out();
    hal_cmu_clock_out_enable(HAL_CMU_CLOCK_OUT_SYS_EMMC);
#endif

    ret = emmc_host_init(host);
    if (ret != MMC_NO_ERR) {
        EMMC_TRACE("EMMC Init: failed to init host, ret = %d, error_code = %d",
                ret, host->error_code);
        return ret;
    }

    ret = config_card(host, host->speed_mode, emmc_khz, emmc_width);
    if (ret != MMC_NO_ERR) {
        EMMC_TRACE("EMMC Init: failed to config card, ret = %d, error_code = %d",
                ret, host->error_code);
        return ret;
    }

    memset(&card_info, 0, sizeof(card_info));
    ret = get_card_info(&card_info);
    if (ret != MMC_NO_ERR) {
        EMMC_TRACE("EMMC Init: faild to get card info, ret = %d, error_code = %d",
                ret, host->error_code);
        return ret;
    }

    emmc_host_info_dump(host);
    emmc_card_info_dump(&card_info);

    EMMC_TRACE("  ");
    EMMC_TRACE("EMMC Init: completed!");
    emmc_inited = true;
    return ret;
}

int emmc_init_default(void)
{
    return emmc_init(DEF_BUS_MODE, DEF_BUS_WIDTH);
}

static void emmc_cmd_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  emmc init [bus_mode bus_width]\r\n");
    eshell_putstring("      bus_mode: SDR52|DDR52|HS200|HS400, SDR52 is default\r\n");
    eshell_putstring("      bus_width: 8|4|1, 8 bits is default\r\n");
    eshell_putstring("  emmc info\r\n");
    eshell_putstring("  emmc write offset size addr\r\n");
    eshell_putstring("      offset: write start offset, HEX\r\n");
    eshell_putstring("      size:   write size, HEX\r\n");
    eshell_putstring("      addr:   source address in memory, HEX\r\n");
    eshell_putstring("  emmc read offset size addr\r\n");
    eshell_putstring("      offset: read start offset, HEX\r\n");
    eshell_putstring("      size:   read size, HEX\r\n");
    eshell_putstring("      addr:   destination address in memory, HEX\r\n");
    eshell_putstring("  emmc erase offset size\r\n");
    eshell_putstring("      offset: erase start offset, HEX\r\n");
    eshell_putstring("      size:   erase size, HEX\r\n");
}

static void emmc_cmd(int argc, char *argv[])
{
    uint32_t offset = 0;
    uint32_t size = 0;
    uint32_t addr = 0;

    if (argc < 2)
        goto usage;

    if (strncmp(argv[1], "init", 4) == 0) {
        uint32_t mode = DEF_BUS_MODE;
        uint32_t width = DEF_BUS_WIDTH;
        if (argc > 2) {
            if (strncmp(argv[2], "SDR52", 5) == 0
                || strncmp(argv[2], "sdr52", 5) == 0)
                mode = SDR52;
            else if (strncmp(argv[2], "DDR52", 5) == 0
                    || strncmp(argv[2], "ddr52", 5) == 0)
                mode = DDR52;
            else if (strncmp(argv[2], "HS200", 5) == 0
                    || strncmp(argv[2], "HS200", 5) == 0)
                mode = HS200;
            else if (strncmp(argv[2], "HS400", 5) == 0
                    || strncmp(argv[2], "HS400", 5) == 0)
                mode = HS400;
            else
                goto usage;
        }
        if (argc > 3) {
            if (strncmp(argv[3], "8", 1) == 0)
                width = MMC_8BIT_WIDTH;
            else if (strncmp(argv[3], "4", 1) == 0)
                width = MMC_4BIT_WIDTH;
            else if (strncmp(argv[3], "1", 1) == 0)
                width = MMC_1BIT_WIDTH;
            else
                goto usage;
        }

        emmc_init(mode, width);

    } else if (strncmp(argv[1], "info", 4) == 0) {
        emmc_info();

    } else if (strncmp(argv[1], "write", 5) == 0) {
        if (argc < 5)
            goto usage;

        offset = strtoul(argv[2], NULL, 16);
        size = strtoul(argv[3], NULL, 16);
        addr = strtoul(argv[4], NULL, 16);

        emmc_write(offset, size, addr);

    } else if (strncmp(argv[1], "read", 4) == 0) {
        if (argc < 5)
            goto usage;

        offset = strtoul(argv[2], NULL, 16);
        size = strtoul(argv[3], NULL, 16);
        addr = strtoul(argv[4], NULL, 16);

        emmc_read(offset, size, addr);

    } else if (strncmp(argv[1], "erase", 5) == 0) {
        if (argc < 4)
            goto usage;

        sscanf(argv[2], "%x", &offset);
        sscanf(argv[3], "%x", &size);

        emmc_erase(offset, size);

    } else {
        goto usage;
    }

    return;
usage:
    emmc_cmd_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "emmc", "emmc help", emmc_cmd);

#endif //UTILS_ESHELL_EN
