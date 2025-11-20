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
#include "host.h"
#include "core.h"
#include "card.h"
#include "sdhci.h"
#include "error.h"
#include "core_if.h"
#include "emmc_dbg.h"
#include "hal_sleep.h"
#include "cmsis_nvic.h"
#include "sdhci_emphy.h"
#include "hal_location.h"
#include "emmc_platform.h"
#include "sdhci_unifiedAl.h"
#ifdef EMMC_EXTENDED_USE
    #include <nuttx/ioexpander/ioexpander.h>
#endif

#ifndef EMMC_ADMA_DESC_CNT
    #define EMMC_ADMA_DESC_CNT     4  /* 3 desc + nop desc = 64K * 3 = 192K, a descriptor (8 -byte) can manage 64KB data */
#endif

#if EMMC_ADMA_DESC_CNT < 2
    #error "EMMC_ADMA_DESC_CNT cannot be less than 2"
#endif

#ifndef EMMC_USE_HEAP
    #ifndef EMMC_EXTENDED_USE
        SYNC_FLAGS_LOC static uint32_t emmc_buf1[MMC_MAX_XFER_SZ / 4];//cache alignment user data
        SYNC_FLAGS_LOC static uint32_t emmc_buf2[MMC_MAX_XFER_SZ / 4];//save the address of the user data
    #endif
    SYNC_FLAGS_LOC static struct mmc_adma_desc __attribute__((aligned(4))) emmc_buf3[EMMC_ADMA_DESC_CNT];//for adma2 desc
#endif

#ifdef EMMC_TEST_WAIT
#define EMMC_TEST_CMD_GO   (CMU_SIMU_RES_PASSED)
void emmc_test_wait_cmd(const char *s, int mark, bool show)
{
    uint32_t addr = CMU_BASE + 0xF4, val;
    uint32_t tick = 0;

    hal_cmu_simu_set_val(mark);

    if (show) {
        TRACE(1, " ");
        TRACE(1, "%s", s);
        TRACE(1, "Wait until you write cmd: [0x%X] = 0x%x to continue", addr, EMMC_TEST_CMD_GO);
    }
    do {
        val = hal_cmu_simu_get_val();
        if ((val & EMMC_TEST_CMD_GO) == EMMC_TEST_CMD_GO) {
            break;
        }
        hal_sys_timer_delay(MS_TO_TICKS(10));
        tick++;
        if ((tick % 200) == 0) {
            if (show) {
                TRACE(1, "waiting for you %ds...", tick / 100);
            }
        }
    } while (1);

    if (show) {
        TRACE(1, "wait done");
    }
}
#endif

static void sdhci_emmc_clock_init(struct mmc_host *host)
{
    uint32_t val;
    uint32_t src_clk_khz;

    if (0) {
    // EMMC
    } else if (host->speed_mode == EHS400 || host->speed_mode == HS400) {
        src_clk_khz = F_HS400;
    } else if (host->speed_mode == HS200) {
        src_clk_khz = F_HS200;
    } else if (host->speed_mode == HS100) {
        src_clk_khz = F_HS100;
    } else if (host->speed_mode == HS50) {
        src_clk_khz = F_HS50;
    } else if (host->speed_mode == HS25) {
        src_clk_khz = F_HS25;
    } else if (host->speed_mode == DDR52) {
        src_clk_khz = F_DDR52;
    } else if (host->speed_mode == DDR26) {
        src_clk_khz = F_DDR26;
    } else if (host->speed_mode == SDR52) {
        src_clk_khz = F_SDR52;
    // SD/SDIO
    } else if (host->speed_mode == SDR104) {
        src_clk_khz = F_SDR104;
    } else if (host->speed_mode == SDR50) {
        src_clk_khz = F_SDR50;
    } else if (host->speed_mode == DDR50) {
        src_clk_khz = F_DDR50;
    } else if (host->speed_mode == HS || host->speed_mode == SDR25) {
        src_clk_khz = F_SDR25;
    } else {
        src_clk_khz = F_DS;
    }

#if !defined(CHIP_BEST1600) && !defined(AUD_PLL_DOUBLE)
    if (host->speed_mode == EHS400 || host->speed_mode == HS400) {
#ifdef EMMC_ASSERT
        HAL_EMMC_ASSERT(false, "%s:%d, This speed mode is not supported", __FUNCTION__, __LINE__);
#else
        HAL_EMMC_ERROR(0, "%s:%d, WARNING: This speed mode is not supported", __FUNCTION__, __LINE__);
#endif
        return;
    } else if (host->speed_mode != HS200 && host->speed_mode != SDR104)
#endif
    {
        //because div is at least 2, doubling is required here
        src_clk_khz *= 2;
    }

    if (host->speed_mode == HS100 || host->speed_mode == HS50 ||
        host->speed_mode == HS25 || host->speed_mode == SDR50) {
#ifdef BBPLL_MHZ
        src_clk_khz = BBPLL_MHZ * 1000;
#else
#ifdef AUD_PLL_DOUBLE
        src_clk_khz = 2 * F_HS200;
#else
        src_clk_khz = F_HS200;
#endif
#endif
    }

    host->clk_in_khz = src_clk_khz;
    // enable emmc clock
    hal_cmu_emmc_clock_enable(src_clk_khz * 1000);

#if defined(CHIP_BEST1600)
    //Remove the old version of the emmc ip netlist
    val = sdhciReadSdhc_bypass_netlist(host);
    val &= ~(7 << 7);
    val |= (5 << 7);
    sdhciWriteSdhc_bypass_netlist(host, val);
#else
    val = sdhciReadSdhc_bypass_netlist(host);
    val &= ~EMMC_RX_BYPASS_CLK;
    sdhciWriteSdhc_bypass_netlist(host, val);
#endif

#ifdef EMMC_CLOCK_REVERSE
    sdhci_sel_work_clk(host);// reverse clock edge, BIWIN device need this
#endif
}

static void sdhci_emmc_clock_deinit(struct mmc_host *host)
{
    // disable emmc clock
    hal_cmu_emmc_clock_disable();
}

static struct mmc_host *host_irq = NULL;
static void sdhci_emmc_irq_handler(void)
{
    if (host_irq) {
        mmc_irq((void *)host_irq);
    } else {
#ifdef EMMC_ASSERT
        HAL_EMMC_ASSERT(false, "%s:%d, host_irq cannot be NULL", __FUNCTION__, __LINE__);
#else
        HAL_EMMC_ERROR(0, "%s:%d, error:host_irq cannot be NULL", __FUNCTION__, __LINE__);
#endif
    }
}

static int sdhci_emmc_int_init(void *private, bool enable)
{
    HAL_EMMC_TRACE(2, "%s:%d", __FUNCTION__, __LINE__);

    if (enable) {
        host_irq = (struct mmc_host *)private;
        NVIC_SetVector(EMMC_IRQn, (uint32_t)sdhci_emmc_irq_handler);
        NVIC_SetPriority(EMMC_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(EMMC_IRQn);
        NVIC_EnableIRQ(EMMC_IRQn);
    } else {
        NVIC_DisableIRQ(EMMC_IRQn);
        NVIC_ClearPendingIRQ(EMMC_IRQn);
    }

    return 0;
}

int sdhci_platform_init(bool init, struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    HAL_EMMC_TRACE(1, "%s: init=%d", __FUNCTION__, init);

    if (init) {
#ifdef EMMC_ASSERT
        HAL_EMMC_ASSERT(host->blksz <= EMMC_BLK_SIZE_MAX, "%s:%d, %d > The maximum block size is EMMC_BLK_SIZE_MAX", __FUNCTION__, __LINE__, host->blksz);
        HAL_EMMC_ASSERT(host->blks <= EMMC_BLK_CNT_MAX, "%s:%d, %d > The maximum number of blocks is EMMC_BLK_CNT_MAX", __FUNCTION__, __LINE__, host->blks);
#else
        if (host->blksz > EMMC_BLK_SIZE_MAX) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d > The maximum block size is EMMC_BLK_SIZE_MAX", __FUNCTION__, __LINE__, host->blksz);
            return MMC_INVALID_ERR;
        }
        if (host->blks > EMMC_BLK_CNT_MAX) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d > The maximum number of blocks is EMMC_BLK_CNT_MAX", __FUNCTION__, __LINE__, host->blks);
            return MMC_INVALID_ERR;
        }
#endif
        switch (host->id) {
            case DEV_TYPE_EMMC:
            case DEV_TYPE_SD:
            case DEV_TYPE_SDIO:
                host->base    = EMMC_BASE;
                host->version = 0x50;
                host->irq     = EMMC_IRQn;
                break;
            default:
#ifdef EMMC_ASSERT
                HAL_EMMC_ASSERT(false, "%s:%d, Invalid Controller %d", __FUNCTION__, __LINE__, host->id);
#else
                HAL_EMMC_ERROR(0, "%s:%d, error:Invalid Controller %d", __FUNCTION__, __LINE__, host->id);
                return MMC_INVALID_ERR;
#endif
                break;
        }

        HAL_EMMC_TRACE(1, "    -------------------");
        HAL_EMMC_TRACE(1, "    eMMC BASE      : 0x%0X", host->base);
        HAL_EMMC_TRACE(1, "    eMMC ctrl ver  : 0x%0X", host->version);
        HAL_EMMC_TRACE(1, "    eMMC irq       : 0x%X",  host->irq);
        HAL_EMMC_TRACE(1, "    -------------------");
    }

    return ret;
}

#ifdef CORE_SLEEP_POWER_DOWN
struct SAVED_EMMC_REGS_T {
    //uint32_t REG_000;//not used
    //uint32_t REG_004;
    //uint32_t REG_008;
    //uint32_t REG_00C;
    //uint32_t REG_020;
    uint32_t REG_028;
    uint32_t REG_02C;
    uint32_t REG_034;
    uint32_t REG_038;
    uint32_t REG_03C;
    //uint32_t REG_058;
    //uint32_t REG_05C;
    uint32_t REG_0E0;
    uint32_t REG_104;
    uint32_t REG_108;
    uint32_t REG_10C;
    uint32_t REG_110;
    uint32_t REG_114;
    uint32_t REG_118;
    uint32_t REG_11C;
    uint32_t REG_120;
    uint32_t REG_124;
    uint32_t REG_128;
    uint32_t REG_130;
    uint32_t REG_134;
    uint32_t REG_800;
    uint32_t REG_818;
    uint32_t REG_81C;
    uint32_t REG_820;
    uint32_t REG_834;
};

SRAM_BSS_LOC static struct SAVED_EMMC_REGS_T saved_emmc_regs;
#endif

#ifdef EMMC_RUN_RECORD
SRAM_BSS_LOC volatile uint8_t mmc_sleep_wakeup_record[EMMC_RUN_RECORD_CNT] = {0};
SRAM_BSS_LOC volatile uint16_t mmc_sleep_wakeup_record_index = 0;
#endif

static int hal_emmc_pm_notif_handler(enum HAL_PM_STATE_T state)
{
    if (state == HAL_PM_STATE_NORMAL_SLEEP || state == HAL_PM_STATE_POWER_DOWN_SLEEP) {
        //phy sleep,Interface does not power off, do not save PHY register values
        sdhci_host_emphy_sleep(NULL);
        hal_cmu_emmc_clock_pause();//oclk

#ifdef EMMC_RUN_RECORD
        mmc_sleep_wakeup_record[mmc_sleep_wakeup_record_index++] = 0xA0;
        if (mmc_sleep_wakeup_record_index >= EMMC_RUN_RECORD_CNT) {
            mmc_sleep_wakeup_record_index = 0;
        }
#endif

#ifdef CORE_SLEEP_POWER_DOWN
        if (state == HAL_PM_STATE_POWER_DOWN_SLEEP) {
            //save digital side registers
            //saved_emmc_regs.REG_000 = sdhciReadSdh_regs(EMMC_BASE + 0x000);//not used
            //saved_emmc_regs.REG_004 = sdhciReadSdh_regs(EMMC_BASE + 0x004);
            //saved_emmc_regs.REG_008 = sdhciReadSdh_regs(EMMC_BASE + 0x008);
            //saved_emmc_regs.REG_00C = sdhciReadSdh_regs(EMMC_BASE + 0x00C);
            //saved_emmc_regs.REG_020 = sdhciReadSdh_regs(EMMC_BASE + 0x020);
            saved_emmc_regs.REG_028 = sdhciReadSdh_regs(EMMC_BASE + 0x028);
            saved_emmc_regs.REG_02C = sdhciReadSdh_regs(EMMC_BASE + 0x02C);
            saved_emmc_regs.REG_034 = sdhciReadSdh_regs(EMMC_BASE + 0x034);
            saved_emmc_regs.REG_038 = sdhciReadSdh_regs(EMMC_BASE + 0x038);
            saved_emmc_regs.REG_03C = sdhciReadSdh_regs(EMMC_BASE + 0x03C);
            //saved_emmc_regs.REG_058 = sdhciReadSdh_regs(EMMC_BASE + 0x058);
            //saved_emmc_regs.REG_05C = sdhciReadSdh_regs(EMMC_BASE + 0x05C);
            saved_emmc_regs.REG_0E0 = sdhciReadSdh_regs(EMMC_BASE + 0x0E0);
            saved_emmc_regs.REG_104 = sdhciReadSdh_regs(EMMC_BASE + 0x104);
            saved_emmc_regs.REG_108 = sdhciReadSdh_regs(EMMC_BASE + 0x108);
            saved_emmc_regs.REG_10C = sdhciReadSdh_regs(EMMC_BASE + 0x10C);
            saved_emmc_regs.REG_110 = sdhciReadSdh_regs(EMMC_BASE + 0x110);
            saved_emmc_regs.REG_114 = sdhciReadSdh_regs(EMMC_BASE + 0x114);
            saved_emmc_regs.REG_118 = sdhciReadSdh_regs(EMMC_BASE + 0x118);
            saved_emmc_regs.REG_11C = sdhciReadSdh_regs(EMMC_BASE + 0x11C);
            saved_emmc_regs.REG_120 = sdhciReadSdh_regs(EMMC_BASE + 0x120);
            saved_emmc_regs.REG_124 = sdhciReadSdh_regs(EMMC_BASE + 0x124);
            saved_emmc_regs.REG_128 = sdhciReadSdh_regs(EMMC_BASE + 0x128);
            saved_emmc_regs.REG_130 = sdhciReadSdh_regs(EMMC_BASE + 0x130);
            saved_emmc_regs.REG_134 = sdhciReadSdh_regs(EMMC_BASE + 0x134);
            saved_emmc_regs.REG_800 = sdhciReadSdh_regs(EMMC_BASE + 0x800);
#if !defined(CHIP_BEST1600)
            saved_emmc_regs.REG_818 = sdhciReadSdh_regs(EMMC_BASE + 0x818);
            saved_emmc_regs.REG_81C = sdhciReadSdh_regs(EMMC_BASE + 0x81C);
            saved_emmc_regs.REG_820 = sdhciReadSdh_regs(EMMC_BASE + 0x820);
#endif
        }
#endif
    } else if (state == HAL_PM_STATE_NORMAL_WAKEUP || state == HAL_PM_STATE_POWER_DOWN_WAKEUP) {
        sdhci_host_emphy_wakeup(NULL);  //wakeup phy
        hal_sys_timer_delay_us(20);     //phy's ldo power on time
        hal_cmu_emmc_clock_continue();  //oclk
        //hal_cmu_clock_enable(HAL_CMU_MOD_X_EMMC);//xclk

#ifdef EMMC_RUN_RECORD
        mmc_sleep_wakeup_record[mmc_sleep_wakeup_record_index++] = 0xA1;
        if (mmc_sleep_wakeup_record_index >= EMMC_RUN_RECORD_CNT) {
            mmc_sleep_wakeup_record_index = 0;
        }
#endif

#ifdef CORE_SLEEP_POWER_DOWN
        if (state == HAL_PM_STATE_POWER_DOWN_WAKEUP) {
            //restore digital side registers
            //sdhciWriteSdh_regs(EMMC_BASE + 0x000, saved_emmc_regs.REG_000);
            //sdhciWriteSdh_regs(EMMC_BASE + 0x004, saved_emmc_regs.REG_004);
            //sdhciWriteSdh_regs(EMMC_BASE + 0x008, saved_emmc_regs.REG_008);
            //sdhciWriteSdh_regs(EMMC_BASE + 0x00C, saved_emmc_regs.REG_00C);
            //sdhciWriteSdh_regs(EMMC_BASE + 0x020, saved_emmc_regs.REG_020);
            sdhciWriteSdh_regs(EMMC_BASE + 0x028, saved_emmc_regs.REG_028);
            sdhciWriteSdh_regs(EMMC_BASE + 0x02C, saved_emmc_regs.REG_02C);
            sdhciWriteSdh_regs(EMMC_BASE + 0x03C, saved_emmc_regs.REG_03C);
            //sdhciWriteSdh_regs(EMMC_BASE + 0x058, saved_emmc_regs.REG_058);
            //sdhciWriteSdh_regs(EMMC_BASE + 0x05C, saved_emmc_regs.REG_05C);
            sdhciWriteSdh_regs(EMMC_BASE + 0x0E0, saved_emmc_regs.REG_0E0);
            sdhciWriteSdh_regs(EMMC_BASE + 0x104, saved_emmc_regs.REG_104);
            sdhciWriteSdh_regs(EMMC_BASE + 0x108, saved_emmc_regs.REG_108);
            sdhciWriteSdh_regs(EMMC_BASE + 0x10C, saved_emmc_regs.REG_10C);
            sdhciWriteSdh_regs(EMMC_BASE + 0x110, saved_emmc_regs.REG_110);
            sdhciWriteSdh_regs(EMMC_BASE + 0x114, saved_emmc_regs.REG_114);
            sdhciWriteSdh_regs(EMMC_BASE + 0x118, saved_emmc_regs.REG_118);
            sdhciWriteSdh_regs(EMMC_BASE + 0x11C, saved_emmc_regs.REG_11C);
            sdhciWriteSdh_regs(EMMC_BASE + 0x120, saved_emmc_regs.REG_120);
            sdhciWriteSdh_regs(EMMC_BASE + 0x124, saved_emmc_regs.REG_124);
            sdhciWriteSdh_regs(EMMC_BASE + 0x128, saved_emmc_regs.REG_128);
            sdhciWriteSdh_regs(EMMC_BASE + 0x130, saved_emmc_regs.REG_130);
            sdhciWriteSdh_regs(EMMC_BASE + 0x134, saved_emmc_regs.REG_134);
            sdhciWriteSdh_regs(EMMC_BASE + 0x800, saved_emmc_regs.REG_800);
#if !defined(CHIP_BEST1600)
            sdhciWriteSdh_regs(EMMC_BASE + 0x818, saved_emmc_regs.REG_818);
            sdhciWriteSdh_regs(EMMC_BASE + 0x81C, saved_emmc_regs.REG_81C);
            sdhciWriteSdh_regs(EMMC_BASE + 0x820, saved_emmc_regs.REG_820);
#endif
            sdhciWriteSdh_regs(EMMC_BASE + 0x030, 0xFFFF);//clear int flag
            sdhciWriteSdh_regs(EMMC_BASE + 0x034, saved_emmc_regs.REG_034);
            sdhciWriteSdh_regs(EMMC_BASE + 0x038, saved_emmc_regs.REG_038);
        }
#endif
    }

    return 0;
}

int sdhci_host_init(bool init, struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    if (init) {
        hal_pm_notif_register(HAL_PM_USER_HAL, hal_emmc_pm_notif_handler);

        host->in_use = 0;
        host->irq_en = true;
        host->irq_en_cp = host->irq_en;
        host->tran_status = 0;
        host->tuning_status = 0;
        host->internal_gating_en = 1;//IP internal default is opened
        host->clk_src = (enum EMMC_CLK_SRC_SEL_T)0;

        sdhci_emmc_clock_init(host);
        sdhci_emmc_int_init((void *)host, true);
        emmc_core_if_init();

        // init emmc controller and  phy
        ret = sdhci_host_emphy_init(true, host);
        if (ret != MMC_NO_ERR) {
            return ret;
        }

        // After initialization, turn on auto gating
        if (host->clk_gating_en) {
            host->clk_gating_status = 0;
            card_controller_clk_pause(host);
        }

        // alloc host buffer
        /* Allocate Aligned internal buffer for host */
#ifdef EMMC_USE_HEAP
        host->aligned_buf   = (void *)emmc_heap_malloc(MMC_MAX_XFER_SZ);
        host->actual_buf    = (void *)emmc_heap_malloc(MMC_MAX_XFER_SZ);
        host->desc          = (unsigned int *)emmc_heap_malloc(EMMC_ADMA_DESC_CNT * sizeof(struct mmc_adma_desc));

        host->aligned_buf_copy   = (uint32_t)host->aligned_buf;
        host->actual_buf_copy    = (uint32_t)host->actual_buf;
        host->desc_copy          = (uint32_t)host->desc;

        HAL_EMMC_TRACE(1, "---host->aligned_buf_copy =0x%x", host->aligned_buf_copy);
        HAL_EMMC_TRACE(1, "---host->actual_buf_copy  =0x%x", host->actual_buf_copy);
        HAL_EMMC_TRACE(1, "---host->desc_copy        =0x%x", host->desc_copy);
#else
#ifdef EMMC_EXTENDED_USE
        host->alloc_p = malloc(0x10000);
        host->alloc_p += 32 - (((uint32_t)host->alloc_p) % 32);
#else
        host->aligned_buf   = (void *)emmc_buf1;
        host->actual_buf    = (void *)emmc_buf2;
#endif
        host->desc          = (unsigned int *)emmc_buf3;
#endif
        host->desc_len      = EMMC_ADMA_DESC_CNT * sizeof(struct mmc_adma_desc);
        host->desc_cnt      = EMMC_ADMA_DESC_CNT - 1;
        host->desc_index    = 0;
        host->desc_ctrl_max_len = (host->desc_len / sizeof(struct mmc_adma_desc) - 1) * 65536;//Reserve a nop desc

        HAL_EMMC_TRACE(3, "init done: id=%d, base=%x,irq=%d",
                       host->id, host->base, host->irq);
    } else {
#ifdef EMMC_USE_HEAP
        emmc_heap_free((void *)host->aligned_buf_copy);
        emmc_heap_free((void *)host->actual_buf_copy);
        emmc_heap_free((void *)host->desc_copy);
#endif
        sdhci_host_emphy_init(false, host); //close emmc phy,interface does not power off
        sdhci_emmc_clock_deinit(NULL);      //close emmc host clock
        sdhci_emmc_int_init(NULL, false);   //close emmc nvic
        if (host->clk_gating_en) {
            host->clk_gating_status = 0;
            card_controller_clk_pause(host);
        }

        hal_pm_notif_deregister(HAL_PM_USER_HAL, hal_emmc_pm_notif_handler);
        HAL_EMMC_TRACE(1, "deinit done");
    }
    return ret;
}

int sdhci_host_all_init(struct mmc_host *host, bool enable)
{
    int ret = MMC_NO_ERR;

    if (!host) {
        HAL_EMMC_ERROR(0, "%s: host is NULL", __FUNCTION__);
        return MMC_INVALID_ERR;
    }

    ret = sdhci_platform_init(enable, host);
    if (ret) {
        HAL_EMMC_ERROR(0, "sdhci_platform_init err:%d", ret);
        return ret;
    }

    ret = sdhci_host_init(enable, host);
    if (ret) {
        HAL_EMMC_ERROR(0, "sdhci_host_init err:%d", ret);
        return ret;
    }

    return ret;
}

int sdhci_int_init(struct mmc_host *host, bool enable)
{
    if (!host) {
        HAL_EMMC_ERROR(0, "%s: host is NULL", __FUNCTION__);
        return MMC_INVALID_ERR;
    }

    return sdhci_emmc_int_init((void *)host, enable);
}
