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
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "plat_addr_map.h"
#if defined(CSI_BASE) || defined(CSI1_BASE) || defined(CSI2_BASE)
#include "plat_types.h"
#include "reg_csi.h"
#include "mipiphy.h"
#include "cmsis_nvic.h"
#include "hal_cmu.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_csi.h"
#include "pmu.h"

#define CSI_MIPI_2LANE_MODE

#define CSI_PHY_CK_RANGE_VALUE (CSI_PHY_CLK_RANGE_200MHZ)
#define CSI_PHY_CK_DELAY_VALUE (3)
#define CSI_PHY_D0_DELAY_VALUE (0)
#define CSI_PHY_D1_DELAY_VALUE (0)

//#define CSI_PHY_NO_CALIB

#define CSI_PHY_CALIB_SKIP_CNT (1)

#define CSI_PHY_D0_CALIB_START (0x00)
#define CSI_PHY_D0_CALIB_END   (0x0F)
#define CSI_PHY_D0_CALIB_COEF  (0x4)

#define CSI_CALIB_MAX_TM_CNT   (CSI_PHY_D0_CALIB_END - CSI_PHY_D0_CALIB_START) / 3
#define CSI_CALIB_MAX_WAIT_CNT (20)
#define CSI_CALIB_WAIT_DLY_MS  (5)

//#define CSI_PHY_D0_CALIB_USE_FIRST_VALUE

#define CSI_PHY_D0_CALIB_CHECK_ERR (CSI_R_INVALID_WC | CSI_R_DTYPE_ERR | CSI_R_MULTI_ECC_ERR)

#define CSI_PRPR_TIME    0x15
#define CSI_PWR_INI_DLY  0x0
#define CSI_R_HDLY_0     0x1A0
#define CSI_R_HDLY_1     0x30F

#define CSI_PIX_CLK_MHZ 50

//#define CSI_IRQ_DEBUG

#define CSI_DMA_MIN_IRQ_INTERVAL_MS 2

///#define CSI_STOP_WHEN_IRQ_INTERVAL_ERR

#define CSI_REG_IRQ_EN_DMA_DONE (1<<6)
#define DLY_CYC 16
#define TRY_CNT 10

#if defined(CONFIG_CSI2_2LANE_MODE) && defined(CSI2_BASE)
#define CSI_BANK 2
#else
#define CSI_BANK 0
#endif

static struct CSI_REG_T * const csi[] = {
    (struct CSI_REG_T *)CSI_BASE,
#ifdef CSI1_BASE
    (struct CSI_REG_T *)CSI1_BASE,
#endif
#ifdef CSI2_BASE
    (struct CSI_REG_T *)CSI2_BASE,
#endif
};


struct HAL_CSI_CNTL_T {
    struct HAL_CSI_CONFIG_T cfg;
    HAL_CSI_STREAM_HANDLER_T stream_handler;
    volatile int pp_index;
    uint32_t pp_data_len;
};

struct CSI_CALIB_CNTL_T {
    uint32_t enable : 1;
    uint32_t done   : 1;
    uint32_t good   : 1;
    uint32_t rsv    : 29;

    uint32_t good_map;
    uint32_t err_status;
    uint32_t skip_cnt;
    uint32_t pos;
    uint32_t ck_delay;
    uint32_t d0_delay;
    uint32_t d1_delay;
};

static struct HAL_CSI_CNTL_T csi_cntl;
static struct CSI_CALIB_CNTL_T calib_cntl;
static bool csi_calib_status = false;
#ifndef FPGA
static bool csi_dma_trig_calib = false;
#endif
static bool csi_inited = false;
static uint32_t dma_irq_cnt = 0;
static uint32_t csi_err_status = 0;

/* CSI CALIB CONFIG */
static struct HAL_CSI_CALIB_CONFIG_T csi_calib_cfg = {
    .flag = 0,
    .clock_range = CSI_PHY_CK_RANGE_VALUE,
    .clock_delay = CSI_PHY_CK_DELAY_VALUE,
    .data_delay  = {CSI_PHY_D0_DELAY_VALUE, CSI_PHY_D1_DELAY_VALUE},
    .pix_clk_mhz = CSI_PIX_CLK_MHZ,
    .pwr_ini_dly = CSI_PWR_INI_DLY,
    .prpr_time   = CSI_PRPR_TIME,
    .r_hdly      = {CSI_R_HDLY_0, CSI_R_HDLY_1},
};

static HAL_CSI_CALIB_WAIT_CALLBACK_T csi_calib_wait_callback = NULL;

#ifdef CHIP_BEST2003
static void hal_csi_clr_error(void);
#endif

static uint32_t hal_csi_clr_irq_status(void);
static void hal_csi_dump_regs(void);
static int hal_csi_calib_irq_handler(int error);

#ifndef FPGA
static void hal_csi_calib_to_phy_cfg(struct CSI_PHY_CONFIG_T *cfg)
{
    cfg->range = csi_calib_cfg.clock_range;
    cfg->param.clock_delay = csi_calib_cfg.clock_delay;
    cfg->param.data_delay[0] = csi_calib_cfg.data_delay[0];
    cfg->param.data_delay[1] = csi_calib_cfg.data_delay[1];
}

static void hal_csi_phy_param_to_calib_cfg(struct CSI_PHY_TIMING_PARAM_T *param)
{
    csi_calib_cfg.clock_delay = param->clock_delay;
    csi_calib_cfg.data_delay[0] = param->data_delay[0];
    csi_calib_cfg.data_delay[1] = param->data_delay[1];
}
#endif

static void hal_csi_delay_us(uint32_t us)
{
    hal_sys_timer_delay(US_TO_TICKS(us));
}

static void hal_csi_delay(uint32_t cycle)
{
    int i = (int)cycle >> 2;

    do {
        __NOP();__NOP();__NOP();__NOP();
    } while (--i > 0);
}

void hal_csi_init(void)
{
    if (csi_inited) {
        return;
    }
    hal_cmu_csi_clock_enable(csi_calib_cfg.pix_clk_mhz);
    hal_cmu_csi_reset_clear();

#ifndef FPGA
    struct CSI_PHY_CONFIG_T phy_cfg;
    hal_csi_calib_to_phy_cfg(&phy_cfg);
    csiphy_open(&phy_cfg);
    csiphy_dump_regs();
#endif

    csi[CSI_BANK]->REG_000 = CSI_R_PWR_INI_DELAY(csi_calib_cfg.pwr_ini_dly) | CSI_R_HS_PRPR_TIME(csi_calib_cfg.prpr_time) |
#ifdef CSI_CLK_HIGH_SPEED
    CSI_R_CKLANE_HS_EN |
#endif
#ifdef CSI_MIPI_2LANE_MODE
    CSI_R_LANE_NUM(1)
#else
    CSI_R_LANE_NUM(0)
#endif
    ;
    csi[CSI_BANK]->REG_00C = CSI_R_USER_DTYPE_BN3(0x2) | CSI_R_USER_DTYPE_BN2(0x2) |
        CSI_R_USER_DTYPE_BN1(0x2) | CSI_R_USER_DTYPE_BN0(0x2) |
        CSI_R_HDLY0(csi_calib_cfg.r_hdly[0]) | CSI_R_HDLY1(csi_calib_cfg.r_hdly[1]);

#ifdef CONFIG_CSI2_2LANE_MODE
    csi[CSI_BANK]->REG_09C &= ~CSI2_REG_CSI_CHCK1_CKX_SEL ;
    csi[CSI_BANK]->REG_09C = CSI2_REG_DATA_SWAP;
#endif
    csi_inited = true;
}

void hal_csi_deinit(void)
{
    if (!csi_inited) {
        return;
    }
    csi[CSI_BANK]->REG_07C = 0;
#ifndef FPGA
    csiphy_close();
#endif

    hal_cmu_csi_reset_set();
    hal_cmu_csi_clock_disable();
    csi_inited = false;
}
#ifdef CHIP_BEST2003
static void hal_csi_clr_error(void)
{
    hal_csi_dump_regs();
}
#endif

POSSIBLY_UNUSED static void hal_csi_dma_done_irq_enable(bool enable)
{
    uint32_t i, mask, val;

    if (enable) {
        mask = CSI_REG_IRQ_EN_DMA_DONE;
    } else {
        mask = 0;
    }
    for (i = 0; i < TRY_CNT; i++) {
        csi[CSI_BANK]->REG_074 &= ~CSI_REG_IRQ_EN_DMA_DONE;
        val = csi[CSI_BANK]->REG_074;
        if ((val & CSI_REG_IRQ_EN_DMA_DONE) == mask) {
            break;
        }
        hal_csi_delay(DLY_CYC);
    }
}

uint32_t hal_csi_get_err_status(void)
{
    uint32_t clr_status;

    csi_err_status = csi[CSI_BANK]->REG_008;
    clr_status = csi[CSI_BANK]->REG_004;
    clr_status = clr_status;
    return csi_err_status;
}

static uint32_t hal_csi_clr_irq_status(void)
{
    POSSIBLY_UNUSED uint32_t val[2], i;
    uint32_t err_status = 0;
    uint32_t irq_status = 0;

    err_status = hal_csi_get_err_status();
    err_status = err_status;

    irq_status = csi[CSI_BANK]->REG_078;

#if 0
    // FIXME: workaround for CSI IRQ mis-trigger
    csi[CSI_BANK]->REG_060 &= ~CSI_DMA_ENA;
    hal_csi_delay(DLY_CYC);
    csi[CSI_BANK]->REG_060 |= CSI_DMA_ENA;
#endif

    for (i = 0; i < TRY_CNT; i++) {
        val[0] = csi[CSI_BANK]->REG_078;
        csi[CSI_BANK]->REG_078 = 0;
        hal_csi_delay(DLY_CYC);
        val[1] = csi[CSI_BANK]->REG_078;
        hal_csi_delay(DLY_CYC);
        if (val[1] == 0) {
            break;
        }
    }
#ifdef CSI_IRQ_DEBUG
    TRACE(0, "reg_78:[%x]->[%x], err_status=%x", val[0], val[1], err_status);
#endif
    return irq_status;
}

void hal_csi_irq_enable(uint32_t handler)
{
    NVIC_SetVector(CSIDMA_IRQn, handler);
    NVIC_SetPriority(CSIDMA_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(CSIDMA_IRQn);
    NVIC_EnableIRQ(CSIDMA_IRQn);
}

void hal_csi_irq_disable(void)
{
    NVIC_DisableIRQ(CSIDMA_IRQn);
}

int hal_csi_trigger_calib_process(int param)
{
    int r = 0;

    if (csi_calib_status) {
        r = hal_csi_calib_irq_handler(param);
    }
    return r;
}

bool hal_csi_calib_status(void)
{
    return csi_calib_status;
}

#ifdef CHIP_BEST2003
static void csi_dma_irq_handler(void)
{
    POSSIBLY_UNUSED static uint32_t cur_time = 0, last_time = 0, diff_time = 0;
    int err = 0;
    uint32_t lock;
    uint32_t irq_status = 0;

    // check irq interval time
    cur_time = hal_sys_timer_get();
    if (dma_irq_cnt == 0) {
        last_time = cur_time;
    }
    diff_time = cur_time - last_time;
    last_time = cur_time;
    if ((dma_irq_cnt > 0) && (TICKS_TO_MS(diff_time) < CSI_DMA_MIN_IRQ_INTERVAL_MS)) {
        err = 1;
    } else {
        err = 0;
    }
    dma_irq_cnt++;

    if (csi_calib_status && csi_dma_trig_calib) {
        hal_csi_trigger_calib_process(err);
    }

    irq_status = hal_csi_clr_irq_status();
    if (irq_status & CSI_DMA_IRQ_MASK_OVERFLOW) {
        TRACE(0, "[%s]: WARNING: DMA FIFO is overflow, %x", __func__, irq_status);
    }
    if (irq_status & CSI_DMA_IRQ_MASK_UNDFLOW) {
        TRACE(0, "[%s]: WARNING: DMA FIFO is underflow, %x", __func__, irq_status);
    }

    // process error occur
    if (err && (!csi_calib_status)) {
        TRACE(0, "[%4u][CSI_IRQ]: cnt=%d, IRQ status not cleared? ", TICKS_TO_MS(diff_time), dma_irq_cnt);
        hal_csi_clr_error();
#ifdef CSI_STOP_WHEN_IRQ_INTERVAL_ERR
        hal_csi_irq_disable();
        hal_csi_stop();
#endif
#ifdef CSI_IRQ_DEBUG
    } else {
        TRACE(0, "[%4u][CSI_IRQ]: cnt=%d", TICKS_TO_MS(diff_time), dma_irq_cnt);
#endif
    }
    if (csi_calib_status) {
        return;
    }
    // invoke stream handler
    HAL_CSI_STREAM_HANDLER_T handler = csi_cntl.stream_handler;
    if (handler) {
        uint8_t *buf;
        int pp_index;
        uint32_t len;
        uint32_t errcode = csi_err_status & 0xFFFF;

        lock = int_lock();

        pp_index = csi_cntl.pp_index;
        len = csi_cntl.pp_data_len;
        if (pp_index == 0) {
            buf = (uint8_t *)(csi_cntl.cfg.data_ptr);
        } else {
            buf = (uint8_t *)(csi_cntl.cfg.data_ptr) + len;
        }
        if (csi_cntl.cfg.pp_mode) {
            csi_cntl.pp_index ^= 1;
        }

        int_unlock(lock);

        handler(buf, len, errcode);
    }
}
#else
static void csi_dma_irq_handler(void)
{
    uint32_t lock;
    uint32_t irq_status = 0;
    irq_status = hal_csi_clr_irq_status();
    if (irq_status & CSI_ECC_ERR_IRQ) {
        TRACE(0, "[%s]: WARNING:csi irq ecc err, %x", __func__, irq_status);
        TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_008, csi[CSI_BANK]->REG_008);
        TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_094, csi[CSI_BANK]->REG_094);
        TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_0A4, csi[CSI_BANK]->REG_0A4);
    }
    if (irq_status & CSI_CSIRX_FIFO_FULL) {
        TRACE(0, "[%s]: WARNING: csi pix is too small or rhdely too big, %x", __func__, irq_status);
    }
    if (irq_status & CSI_BAD_FRAME_IRQ) {
        TRACE(0, "[%s]: WARNING: this frame is err, %x", __func__, irq_status);
    }
    if (irq_status & CSI_DMA_IRQ_MASK_OVERFLOW) {
        TRACE(0, "[%s]: WARNING: DMA FIFO is overflow, %x", __func__, irq_status);
    }
    if (irq_status & CSI_DMA_IRQ_EN_DONE) {
        TRACE(0, "[%s]: csi frame done, %x", __func__, irq_status);
    }

    // invoke stream handler
    HAL_CSI_STREAM_HANDLER_T handler = csi_cntl.stream_handler;
    if (handler) {
        uint8_t *buf;
        int pp_index;
        uint32_t len;
        uint32_t errcode = csi_err_status & 0xFFFF;

        lock = int_lock();

        pp_index = csi_cntl.pp_index;
        len = csi_cntl.pp_data_len;
        if (pp_index == 0) {
            buf = (uint8_t *)(csi_cntl.cfg.data_ptr);
        } else {
            buf = (uint8_t *)(csi_cntl.cfg.data_ptr) + len;
        }
        if (csi_cntl.cfg.pp_mode) {
            csi_cntl.pp_index ^= 1;
        }

        int_unlock(lock);

        handler(buf, len, errcode);
    }
}
#endif

void hal_csi_open(struct HAL_CSI_CONFIG_T *cfg)
{
    enum CSI_PIX_FMT_T fmt = cfg->fmt;
    uint32_t width = cfg->width;
    uint32_t height = cfg->height;
    uint32_t pitch;
    uint32_t addr, bsize;

    ASSERT(cfg != NULL, "[%s]: null ptr", __func__);
    ASSERT(cfg->data_size > 0, "[%s]: zero data_size", __func__);
    ASSERT(cfg->data_ptr != NULL, "[%s]: null data ptr", __func__);
    ASSERT(cfg->handler != NULL, "[%s]: null handler ptr", __func__);

    hal_csi_init();

    if (fmt == CSI_PIX_FMT_YUV422) {
        pitch = width * 2;//UYVY, 2B/pix
    } else if (fmt == CSI_PIX_FMT_RGB565) {
        pitch = width * 2;//BGR, 2B/pix
    } else if (fmt == CSI_PIX_FMT_RGB888) {
        pitch = width * 3;//BGR, 3B/pix
    } else if (fmt == CSI_PIX_FMT_RGBA888) {
        pitch = width * 4;
    } else {
        ASSERT(0, "unsupport fmt");
    }

    bsize = pitch * height;
    if (cfg->pp_mode) {
        ASSERT(cfg->data_size >= bsize, "[%s]: Bad data_size=%d, should be larger %d",
        __func__, cfg->data_size, bsize);
    }
    addr = (uint32_t)cfg->data_ptr;
    addr &= ~3;
//    bsize = 0x100000; //FIXME!
    TRACE(1, "addr=0x%x, bsize=0x%x", addr, bsize);
    csi[CSI_BANK]->REG_068 = CSI_DMA_BASE_ADDR(addr);
    csi[CSI_BANK]->REG_080 = CSI_DMA_BASE_ADDR(addr + bsize);
    csi[CSI_BANK]->REG_07C = 0;
    csi[CSI_BANK]->REG_078 = 0;
    //csi[CSI_BANK]->REG_070 |= (1<<6); //DMA DONE status, read clear
    csi[CSI_BANK]->REG_008 = (((3<<16)|(7<<21)|(1<<28)|(1<<30)));
#ifdef CHIP_BEST2003
    csi[CSI_BANK]->REG_074 |= (CSI_DMA_IRQ_EN_DONE | CSI_DMA_IRQ_EN_UNDFLOW | CSI_DMA_IRQ_EN_OVERFLOW);
#else
    csi[CSI_BANK]->REG_074 = 0;
    csi[CSI_BANK]->REG_074 |= (CSI_ECC_ERR_IRQ_EN | CSI_CSIRX_FIFO_FULL_EN | CSI_DMA_IRQ_EN_DONE | CSI_BAD_FRAME_IRQ_EN | CSI_DMA_IRQ_EN_OVERFLOW);
#endif
    csi[CSI_BANK]->REG_01C &= ~(0xF<<28); //i2c irq mask, low power escape irq mask

    csi_cntl.cfg = *cfg;
    csi_cntl.pp_data_len = bsize;
    csi_cntl.stream_handler = cfg->handler;
    csi_cntl.pp_index = 0;
    dma_irq_cnt = 0;

    hal_csi_irq_enable((uint32_t)csi_dma_irq_handler);

    csi[CSI_BANK]->REG_064 = CSI_IMG_HEIGHT(height) | CSI_IMG_WIDTH(width);
    csi[CSI_BANK]->REG_06C = CSI_DMAC_AWQOS(0x4) | CSI_DMAC_ARQOS(0x4) |
        CSI_DMAC_MAX_REQ_NUM(0x4) | CSI_DMAC_RST_N_PWR |
        CSI_REG_DMA_ENA_SYNC_ACLK | CSI_REG_DMA_ENA_SYNC_FCLK;

    csi[CSI_BANK]->REG_060 = CSI_IMG_PITCH(pitch) | CSI_BURST_LEN(0x8) | CSI_PIX_FMT(fmt); //BURST_LEN: 8 is 128B, 4 is 64B
    if (cfg->pp_mode) {
        csi[CSI_BANK]->REG_060 |= CSI_WDMA_ADDR_SEL;
    }
}

void hal_csi_close(void)
{
    hal_csi_deinit();
}

void hal_csi_start(void)
{
    csi[CSI_BANK]->REG_060 |= CSI_DMA_ENA;
}

void hal_csi_stop(void)
{
    csi[CSI_BANK]->REG_060 &= ~CSI_DMA_ENA;
}

void hal_csi_restart(void)
{
    csi[CSI_BANK]->REG_060 &= ~CSI_DMA_ENA;
    hal_csi_delay_us(100);
    csi[CSI_BANK]->REG_060 |= CSI_DMA_ENA;
}

POSSIBLY_UNUSED static void hal_csi_dump_regs(void)
{
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_008, csi[CSI_BANK]->REG_008);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_000, csi[CSI_BANK]->REG_000);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_00C, csi[CSI_BANK]->REG_00C);

    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_010, csi[CSI_BANK]->REG_010);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_014, csi[CSI_BANK]->REG_014);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_018, csi[CSI_BANK]->REG_018);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_01C, csi[CSI_BANK]->REG_01C);

    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_020, csi[CSI_BANK]->REG_020);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_024, csi[CSI_BANK]->REG_024);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_028, csi[CSI_BANK]->REG_028);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_02C, csi[CSI_BANK]->REG_02C);

    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_060, csi[CSI_BANK]->REG_060);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_064, csi[CSI_BANK]->REG_064);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_06C, csi[CSI_BANK]->REG_06C);

    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_070, csi[CSI_BANK]->REG_070);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_078, csi[CSI_BANK]->REG_078);

    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_068, csi[CSI_BANK]->REG_068);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_080, csi[CSI_BANK]->REG_080);

    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_084, csi[CSI_BANK]->REG_084);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_088, csi[CSI_BANK]->REG_088);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_08C, csi[CSI_BANK]->REG_08C);

    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_094, csi[CSI_BANK]->REG_094);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_098, csi[CSI_BANK]->REG_098);
    TRACE(0, "CSI[%x]=%x", (int)&csi[CSI_BANK]->REG_0A4, csi[CSI_BANK]->REG_0A4);

    hal_trace_flush_output();
}

#ifndef FPGA
static void hal_csi_calib_ready(uint32_t ck_delay, uint32_t d0_delay, uint32_t d1_delay)
{
    calib_cntl.good = 0;
    calib_cntl.done = 0;
    calib_cntl.err_status = 0;
    calib_cntl.skip_cnt = CSI_PHY_CALIB_SKIP_CNT;
    calib_cntl.pos = d0_delay;
    calib_cntl.ck_delay = ck_delay;
    calib_cntl.d0_delay = d0_delay;
    calib_cntl.d1_delay = d1_delay;
    calib_cntl.enable = 1;
}

static void hal_csi_calib_init(void)
{
    calib_cntl.good_map = 0;
}
#endif

static int hal_csi_calib_irq_handler(int error)
{
    uint32_t mask, err_status;

    if (!calib_cntl.enable) {
        return 1;
    }
    if (calib_cntl.done) {
        return 0;
    }

    if (calib_cntl.skip_cnt > 0) {
        calib_cntl.skip_cnt -= 1;
        TRACE(0, "[CSI_CALIB]: skip one frame, cnt=%d", calib_cntl.skip_cnt);
    } else {
        err_status = hal_csi_get_err_status();
        TRACE(0, "[CSI_CALIB]: checking frame, status=%x", err_status);
        mask = CSI_PHY_D0_CALIB_CHECK_ERR;
        if (err_status & mask) {
            calib_cntl.good = 0;
        } else {
            calib_cntl.good = 1;
            calib_cntl.good_map |= (1 << calib_cntl.pos);
        }
        calib_cntl.err_status = err_status;
        calib_cntl.done = 1;
    }
    return 0;
}

void hal_csi_setup_calib_wait_callback(HAL_CSI_CALIB_WAIT_CALLBACK_T callback)
{
    csi_calib_wait_callback = callback;
}

#ifndef FPGA
static int hal_csi_calib_wait(void)
{
    volatile int timeout = CSI_CALIB_MAX_WAIT_CNT;

    while (1) {
        if (calib_cntl.done) {
            calib_cntl.enable = 0;
            break;
        }
        if (csi_calib_wait_callback) {
            csi_calib_wait_callback();
        }
        osDelay(CSI_CALIB_WAIT_DLY_MS);
        timeout--;
        if (timeout == 0) {
            TRACE(0, "[%s]: timeout", __func__);
            return 1;
        }
    }
    if (calib_cntl.good) {
        TRACE(1, "GOOD[%2d %2d %2d]+++++++++++++++++++++++> status=%08x",
            calib_cntl.ck_delay, calib_cntl.d0_delay, calib_cntl.d1_delay, calib_cntl.err_status);
    } else {
        TRACE(1, "FAIL[%2d %2d %2d]-----------------------> status=%08x",
            calib_cntl.ck_delay, calib_cntl.d0_delay, calib_cntl.d1_delay, calib_cntl.err_status);
    }
    return 0;
}

static int hal_csi_calib_get_value(struct CSI_PHY_TIMING_PARAM_T *tim)
{
    uint32_t i, k = 0, a = 0, b = 0;
    uint32_t map = calib_cntl.good_map;
    uint32_t msk = 0;

    TRACE(1, "CSI_PHY_D0_CALIB_START:    %d",CSI_PHY_D0_CALIB_START);
    TRACE(1, "CSI_PHY_D0_CALIB_END:      %d",CSI_PHY_D0_CALIB_END);
    TRACE(1, "CSI_PHY_D0_CALIB_COEF:     %d",CSI_PHY_D0_CALIB_COEF);

    if (map == 0) {
        TRACE(0, "[%s]: zero map", __func__);
        return 1;
    }
    for (i = CSI_PHY_D0_CALIB_START; i <= CSI_PHY_D0_CALIB_END; i++) {
        msk = (1<<i);
        if (msk & map) {
            a = i;
            break;
        }
    }
#ifdef CSI_PHY_D0_CALIB_USE_FIRST_VALUE
    TRACE(1, "CSI_PHY_D0_CALIB_USE_FIRST_VALUE");
    k = a;
#else
    b = a;
    for (i = a; i < CSI_PHY_D0_CALIB_END; i++) {
        msk = (1<<i);
        if (!(msk & map)) {
            break;
        } else {
            b = i;
        }
    }
    if (b > a) {
        k = (a + b) / CSI_PHY_D0_CALIB_COEF;
        if (k < a) {
            k = a;
        }
    } else {
        k = a;
    }
#endif
    if (tim) {
        tim->data_delay[0] = k;
        tim->data_delay[1] = k;
    }
    TRACE(1, "[%s]: a=%d,b=%d,k=%d,map=%x", __func__, a, b, k, map);
    return 0;
}
#endif

int hal_csi_get_calib_value(struct HAL_CSI_CALIB_CONFIG_T *cfg)
{
    if (cfg) {
        *cfg = csi_calib_cfg;
    }
    return 0;
}

int hal_csi_set_calib_value(struct HAL_CSI_CALIB_CONFIG_T *cfg)
{
    csi_calib_cfg.clock_range = cfg->clock_range,
    csi_calib_cfg.clock_delay = cfg->clock_delay,
    csi_calib_cfg.data_delay[0] = cfg->data_delay[0],
    csi_calib_cfg.data_delay[1] = cfg->data_delay[1],
    csi_calib_cfg.pix_clk_mhz = cfg->pix_clk_mhz;
    csi_calib_cfg.pwr_ini_dly = cfg->pwr_ini_dly;
    csi_calib_cfg.prpr_time   = cfg->prpr_time;
    csi_calib_cfg.r_hdly[0]   = cfg->r_hdly[0];
    csi_calib_cfg.r_hdly[1]   = cfg->r_hdly[1];

    return 0;
}

int hal_csi_start_calib(bool force_calib, bool gpio_trigger)
{
#ifndef FPGA
    struct CSI_PHY_TIMING_PARAM_T phy_tim;
    uint32_t i, j;
    uint32_t lock;
    int err = 0, tmcnt = 0;
    uint16_t *clk_dly = NULL;
    uint32_t clk_num = 0;

    uint16_t clk_dly03_list[] = {0x3, 0x2, 0x4, 0x1, 0x5, 0x0, 0x6, 0x7, 0x8};
    uint16_t clk_dly02_list[] = {0x2, 0x3, 0x1, 0x4, 0x0, 0x5, 0x6, 0x7, 0x8};
    uint16_t clk_dly01_list[] = {0x1, 0x0, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
    uint16_t clk_dly14_list[] = {0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14};
    uint16_t clk_dlyoh_list[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};

#ifdef CSI_PHY_NO_CALIB
    return 0;
#endif

    TRACE(1, "[%s]: force_calib=%d", __func__, force_calib);

    if ((csi_calib_cfg.flag & CSI_CALIB_CFG_FLAG_DONE) && !force_calib) {
        return 0;
    }
    csi_dma_trig_calib = !gpio_trigger;
    csi_calib_status = true;

    switch (csi_calib_cfg.clock_delay) {
    case 0x03:
        clk_dly = clk_dly03_list;
        clk_num = ARRAY_SIZE(clk_dly03_list);
        break;
    case 0x02:
        clk_dly = clk_dly02_list;
        clk_num = ARRAY_SIZE(clk_dly02_list);
        break;
    case 0x01:
        clk_dly = clk_dly01_list;
        clk_num = ARRAY_SIZE(clk_dly01_list);
        break;
    case 0x14:
        clk_dly = clk_dly14_list;
        clk_num = ARRAY_SIZE(clk_dly14_list);
        break;
    default:
        clk_dly = clk_dlyoh_list;
        clk_num = ARRAY_SIZE(clk_dlyoh_list);
        break;
    }
    TRACE(1, "clock_delay=0x%x, clk_num=%d", csi_calib_cfg.clock_delay, clk_num);

    for (j = 0; j < clk_num; j++) {
        phy_tim.clock_delay = clk_dly[j];
        csiphy_set_clock_delay(phy_tim.clock_delay);
        csiphy_dump_regs();
        hal_csi_calib_init();
        for (i = CSI_PHY_D0_CALIB_START; i <= CSI_PHY_D0_CALIB_END; i++) {
            lock = int_lock();
            phy_tim.data_delay[0] = i;
            phy_tim.data_delay[1] = i;
            hal_csi_calib_ready(phy_tim.clock_delay, phy_tim.data_delay[0], phy_tim.data_delay[1]);
            csiphy_set_data_chan_delay(CSI_PHY_DATA_CH0, phy_tim.data_delay[0]);
            csiphy_set_data_chan_delay(CSI_PHY_DATA_CH1, phy_tim.data_delay[1]);
            int_unlock(lock);
            err = hal_csi_calib_wait();
            csiphy_dump_regs();
            if (err) {
                tmcnt++;
                break;
            }
        }
        err = hal_csi_calib_get_value(&phy_tim);
        if (!err) {
            lock = int_lock();
            hal_csi_phy_param_to_calib_cfg(&phy_tim);
            csi_calib_cfg.flag |= CSI_CALIB_CFG_FLAG_DONE;
            int_unlock(lock);
            csiphy_apply_timing_param(&phy_tim);
            csiphy_dump_regs();
            TRACE(1, "CSI_CALIB_RESULT: [ck d0 d1]=[%2d %2d %2d]",
                phy_tim.clock_delay, phy_tim.data_delay[0], phy_tim.data_delay[1]);
            TRACE(1, "CSIPHY apply new timing done");
            break;
        }
        if (tmcnt > CSI_CALIB_MAX_TM_CNT) {
            TRACE(1, "[%s]: reboot system ...", __func__);
            osDelay(20);
            pmu_reboot();
        }
    }
    ASSERT(err == 0, "[%s]: CSI calib error %d", __func__, err);

    csi_calib_status = false;
#endif
    return 0;
}

#endif /*CSI_BASE CSI1_BASE CSI2_BASE*/
