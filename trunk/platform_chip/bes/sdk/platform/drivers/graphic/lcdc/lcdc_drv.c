
/***************************************************************************
 * Copyright 2015-2024 BES.
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
#include "plat_addr_map.h"

#ifdef LCDC_BASE

#include "string.h"
#ifndef NOSTD
#include "math.h"
#endif
#include "hal_cmu.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "reg_lcdc.h"
#include "hal_psc.h"
#include "hal_lcdc.h"
#include "hal_dsi.h"
#include "hal_lcdc_qspi.h"

#include "lcdc_drv.h"
#include "lcdc_iommu.h"

//#define LCDC_DEBUG
#ifdef  LCDC_DEBUG
#define LCDC_DBG(...)               TR_INFO( TR_MOD(HAL), "//LCDC: " __VA_ARGS__)
#else
#define LCDC_DBG(...)
#endif
#define LCDC_INFO(...)              TR_INFO( TR_MOD(HAL), "//LCDC: " __VA_ARGS__)

#if defined(FPGA) || defined(SIMU)
#define LCDC_DUMP                   LCDC_INFO
#else
#define LCDC_DUMP                   LCDC_DBG
#endif

/* Enable QSPI support as default */
#if !defined(LCDC_NO_QSPI)
#define LCDC_QSPI_SUPPORT           1
#endif

/* Enable DSI support if CHIP has DSI */
#ifdef DSI_BASE
#define LCDC_DSI_SUPPORT            1
#ifdef DSI1_BASE
#define LCDC_DUAL_DSI_SUPPORT       1
#endif
#endif

/* Enable RGB support if CHIP has RGB interface */
#ifdef LCDC_HAS_RGBIF
#define LCDC_RGB_SUPPORT            1
#endif

/* Enable MIP support if CHIP has MIP interface */
#ifdef CHIP_HAS_MIP
#include "pmu.h"
#define LCDC_MIP_SUPPORT            1
#endif

#ifdef CHIP_BEST1503P
#define QSPI_USE_DSI_PATH           1
#endif

#define QSPI_BUS(bus)               ((bus == LCDC_BUS_QSPI)        || \
                                     (bus == LCDC_BUS_QSPI_DTR)    || \
                                     (bus == LCDC_BUS_MIP)         || \
                                     (bus == LCDC_BUS_DUAL_QSPI)   || \
                                     (bus == LCDC_BUS_DUAL_QSPI_DTR))

static struct lcdc_config_t *lcdc_config = NULL;
static bool lcdc_started;

#define LAYER_MAX_COUNT             8  /* GRA, DMA, TVG, TVD, SMT, DEC, TILED, WB */

static struct lcdc_layer_config_t layer_config[LAYER_MAX_COUNT] = {
    {
        .enable = false,
        .buffer = NULL,
    },
    {
        .enable = false,
        .buffer = NULL,
    },
    {
        .enable = false,
        .buffer = NULL,
    },
    {
        .enable = false,
        .buffer = NULL,
    },
    {
        .enable = false,
        .buffer = NULL,
    },
    {
        .enable = false,
        .buffer = NULL,
    },
    {
        .enable = false,
        .buffer = NULL,
    },
};

#define CHECK_CONFIG_RETURN \
    do { \
        if (!lcdc_config) { \
            LCDC_INFO("lcdc is not opened!"); \
            return; \
        } \
    } while (0)

#define CHECK_CONFIG_RETURN_ERROR \
    do { \
        if (!lcdc_config) { \
            LCDC_INFO("lcdc is not opened!"); \
            return LCDC_INVAL; \
        } \
    } while (0)

#define CHECK_YUV_FORMAT(n) \
    ((n == LCDC_CFMT_YUV422_I422) || (n == LCDC_CFMT_YUV420_I420) || \
     (n == LCDC_CFMT_YUV420_NV12) || (n == LCDC_CFMT_YUV420_P16))

#define CHECK_TILED_FORMAT(n) \
    ((n == LCDC_CFMT_RGBA8888) || (n == LCDC_CFMT_BGRA8888) || \
     (n == LCDC_CFMT_RGB565) || (n == LCDC_CFMT_BGR565) || \
     (n == LCDC_CFMT_RGB888) || (n == LCDC_CFMT_BGR888))

static int lcdc_color_format_bpp(enum hal_lcdc_color_format_e format)
{
    switch (format) {
        case LCDC_CFMT_PALETTE_4:
            return 4;
        case LCDC_CFMT_PALETTE_8:
            return 8;
        case LCDC_CFMT_YUV422_UYVY:
        case LCDC_CFMT_RGB1555:
        case LCDC_CFMT_RGB565:
        case LCDC_CFMT_BGR565:
            return 16;
        case LCDC_CFMT_RGB888:
        case LCDC_CFMT_BGR888:
            return 24;
        case LCDC_CFMT_RGBA8888:
        case LCDC_CFMT_ARGB8888:
        case LCDC_CFMT_BGRA8888:
        case LCDC_CFMT_ABGR8888:
            return 32;
        default:
            return 8;
    }
    return 8;
}

static void lcdc_irq_handler(void)
{
    uint32_t error = 0;
    uint32_t irqstat = 0;
    bool framedone = false;

    CHECK_CONFIG_RETURN;

    irqstat = hal_lcdc_get_irqstat();

    if (lcdc_config->bus == LCDC_BUS_DSI || lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
#ifdef LCDC_DSI_SUPPORT
        POSSIBLY_UNUSED uint32_t dsistat = hal_dsi_get_irqstate(lcdc_config->bus_port0);
        uint32_t dsiflags = hal_dsi_get_unov_flags(lcdc_config->bus_port0);
        LCDC_DUMP("LCDC IRQ 0x%08x, DSI IRQ 0x%08x, DSI FLAGS 0x%08x", irqstat, dsistat, dsiflags);

        error |= (dsiflags & 3);
#else
        LCDC_DUMP("LCDC IRQ 0x%08x", irqstat);
#endif
    } else {
        LCDC_DUMP("LCDC IRQ 0x%08x", irqstat);
    }

    if (hal_lcdc_irq_is_vsync(irqstat)) {
        LCDC_DBG("vsync got!");
        hal_lcdc_vsync_irqstat_clr();

        if (lcdc_config->mode == LCDC_VIDEO_MODE && lcdc_config->vsync_cb)
            lcdc_config->vsync_cb();
    }

    if (hal_lcdc_irq_is_error(irqstat) &&
       !(layer_config[LCDC_LAYER_DEC].enable && (irqstat & LCDC_TVG_FF_UNDERFLOW)) /* Ignore TVG underflow for decnano */
       ) {
        hal_lcdc_dump_irqerror(irqstat);
        hal_lcdc_error_irqstat_clr();

        error |= hal_lcdc_irq_get_error(irqstat);

        if (lcdc_config->error_cb)
            lcdc_config->error_cb(error);
    }

    /* Handle framedone IRQ in last, LCDC can enter lowpower in framedone_cb() */
    if (QSPI_BUS(lcdc_config->bus))
        framedone = hal_lcdc_irq_is_hwc_framedone(irqstat);
    else
        framedone = hal_lcdc_irq_is_framedone(irqstat);
    if (framedone) {
        LCDC_DBG("framedone got!");
        hal_lcdc_framedone_irqstat_clr();

#ifdef LCDC_QSPI_SUPPORT
        if (QSPI_BUS(lcdc_config->bus))
            hal_lcdc_qspi_cs_sel(false);
#endif
#ifdef LCDC_HAS_DUAL_LCDC
        if (hal_lcdc_is_switch_started()) {
            static uint8_t switch_trigger_frame = 0;
            if (++switch_trigger_frame == 3) {
                hal_lcdc_frame_halt();
                switch_trigger_frame = 0;
            }
        }
#endif
        if (lcdc_config->bus != LCDC_BUS_MIP && lcdc_config->framedone_cb)
            lcdc_config->framedone_cb();
    }
}

#ifdef LCDC_MIP_SUPPORT
static void lcdc_mip_irq_handler(void)
{
    if (lcdc_config->framedone_cb)
        lcdc_config->framedone_cb();
}
#endif

static void lcdc_irq_enable(void)
{
    int irq = hal_lcdc_irqn();

    CHECK_CONFIG_RETURN;

    hal_lcdc_irq_disable();

    NVIC_SetVector((IRQn_Type)irq, (uint32_t) lcdc_irq_handler);
    NVIC_SetPriority((IRQn_Type)irq, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ((IRQn_Type)irq);
    NVIC_EnableIRQ((IRQn_Type)irq);

    hal_lcdc_framedone_irq_enable(true);
    hal_lcdc_vsync_irq_enable(true);
    hal_lcdc_error_irq_enable();

    LCDC_DBG("IRQ enabled.");
}

static void lcdc_irq_disable(void)
{
    int irq = hal_lcdc_irqn();

    CHECK_CONFIG_RETURN;

    hal_lcdc_irq_disable();

    NVIC_ClearPendingIRQ((IRQn_Type)irq);
    NVIC_DisableIRQ((IRQn_Type)irq);

    LCDC_DBG("IRQ disabled.");
}

static void lcdc_te_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    CHECK_CONFIG_RETURN;

    LCDC_DBG("TE IRQ Got!");

    if (lcdc_config->vsync_cb)
        lcdc_config->vsync_cb();
}

POSSIBLY_UNUSED
static void lcdc_te_irq_enable(void)
{
    static const struct HAL_GPIO_IRQ_CFG_T gpiocfg = {
        .irq_enable = true,
        .irq_debounce = false,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        .irq_handler = lcdc_te_irq_handler,
        .irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
    };

    CHECK_CONFIG_RETURN;

    if (lcdc_config->mode != LCDC_COMMAND_MODE)
        return;
    if (lcdc_config->te_pin < 0)
        return;

    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] =
    {
        {(enum HAL_IOMUX_PIN_T) lcdc_config->te_pin,
         HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));

    hal_gpio_pin_set_dir(lcdc_config->te_pin, HAL_GPIO_DIR_IN, 0);
    hal_gpio_setup_irq(lcdc_config->te_pin, &gpiocfg);

    LCDC_DBG("TE IRQ enabled.");
}

POSSIBLY_UNUSED
static void lcdc_te_irq_disable(void)
{
    static const struct HAL_GPIO_IRQ_CFG_T gpiocfg = {
        .irq_enable = false,
        .irq_debounce = false,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        .irq_handler = NULL,
        .irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
    };

    CHECK_CONFIG_RETURN;

    if (lcdc_config->mode != LCDC_COMMAND_MODE)
        return;
    if (lcdc_config->te_pin < 0)
        return;

    hal_gpio_setup_irq(lcdc_config->te_pin, &gpiocfg);

    LCDC_DBG("TE IRQ disabled.");
}

#ifdef LCDC_DSI_SUPPORT
static int lcdc_dsi_stby_vsync_calc(int width, int pixbit, int pixmhz, int dsimhz, int dsilanes)
{
    int v = 0;
    int n = dsimhz / pixbit * dsilanes;
    if (n && pixmhz > n)
        v = (pixmhz - n) * (width + 0x20) / n;
    if (v < 0x40)
        v = 0x40;
    return v;
}

static int lcdc_dsi_init(struct lcdc_config_t *config)
{
    int ret = LCDC_OK;
    enum HAL_DSI_MODE_E dsi_mode;
    enum hal_lcdc_dsi_cfmt_e dsi_fmt;
    enum HAL_DSI_LANE_E dsi_lane;

    if (config->mode == LCDC_VIDEO_MODE)
        dsi_mode = DSI_VIDEO_MODE;
    else
        dsi_mode = DSI_COMMAND_MODE;

    if (config->lcd.color_bits == 16)
        dsi_fmt = DSI_RGB565;
    else if (config->lcd.color_bits == 18)
        dsi_fmt = DSI_RGB666;
    else if (config->lcd.color_bits == 24)
        dsi_fmt = DSI_RGB888;
    else
        return LCDC_INVAL;

    if (config->bus_lane == 1)
        dsi_lane = DSI_LANE_1;
    else if (config->bus_lane == 2)
        dsi_lane = DSI_LANE_2;
    else if (config->bus_lane == 4)
        dsi_lane = DSI_LANE_4;
    else
        return LCDC_INVAL;

    if (config->bus == LCDC_BUS_DUAL_DSI) {
        ret = hal_dsi_init(config->bus_port0, config->lcd.width, dsi_mode, dsi_lane, dsi_fmt);
        if (!ret) {
            ret = hal_dsi_init(config->bus_port1, config->lcd.width,
                    dsi_mode, dsi_lane, dsi_fmt);
        }
    } else {
        ret = hal_dsi_init(config->bus_port0, config->lcd.width, dsi_mode, dsi_lane, dsi_fmt);
    }

    config->bus_mhz = hal_dsi_get_pllmhz(config->bus_port0);
    config->pix_mhz = hal_dsi_get_pixmhz(config->bus_port0);

    if (config->vsync_stby == 0) {
        config->vsync_stby = lcdc_dsi_stby_vsync_calc(config->lcd.width,
                    (dsi_fmt == DSI_RGB565) ? 16 : 24, /* RGB666 and RGB888 both use 24bits */
                    config->pix_mhz, config->bus_mhz, config->bus_lane);
    }
    LCDC_DBG("DSI inited.");
    return ret;
}
#endif

#ifdef LCDC_QSPI_SUPPORT
static int lcdc_qspi_stby_vsync_calc(int width, int pixbit, int pixmhz, int qspimhz, bool dtr)
{
    int v, n;
    if (pixbit == 16)
        n = dtr ? 2 : 4;
    else
        n = dtr ? 3 : 6;
    v = (pixmhz * width) / (qspimhz / 2 / n) - width;
    return v;
}

static int lcdc_qspi_init(struct lcdc_config_t *config)
{
    struct HAL_LCDC_QSPI_CONFIG_T qspi_cfg;

    qspi_cfg.pix_mhz = config->pix_mhz;
    qspi_cfg.bus_mhz = config->bus_mhz;

    if (config->lcd.color_bits == 16)
        qspi_cfg.color_fmt = LCDC_QSPI_RGB565;
    else if (config->lcd.color_bits == 18)
        qspi_cfg.color_fmt = LCDC_QSPI_RGB666;
    else if (config->lcd.color_bits == 24)
        qspi_cfg.color_fmt = LCDC_QSPI_RGB888;
    else
        return LCDC_INVAL;

    if (config->bus == LCDC_BUS_MIP)
        qspi_cfg.line_mode = LCDC_QSPI_MIP;
    else if (config->bus_lane == 1)
        qspi_cfg.line_mode = LCDC_QSPI_1LINE;
    else if (config->bus_lane == 2)
        qspi_cfg.line_mode = LCDC_QSPI_2LINE;
    else if (config->bus_lane == 4)
        qspi_cfg.line_mode = LCDC_QSPI_4LINE;
    else
        return LCDC_INVAL;

    qspi_cfg.dtr_enable = (config->bus == LCDC_BUS_QSPI_DTR) ||
                          (config->bus == LCDC_BUS_DUAL_QSPI_DTR);
    qspi_cfg.dual_enable = (config->bus == LCDC_BUS_DUAL_QSPI) ||
                           (config->bus == LCDC_BUS_DUAL_QSPI_DTR);

    hal_lcdc_qspi_init(config->bus_port0, &qspi_cfg);

    config->bus_mhz = hal_lcdc_qspi_get_mhz();
    config->pix_mhz = hal_lcdc_qspi_get_pixmhz();

    if (config->vsync_stby == 0) {
        config->vsync_stby = lcdc_qspi_stby_vsync_calc(config->lcd.width, config->lcd.color_bits,
                    config->pix_mhz, config->bus_mhz, (config->bus == LCDC_BUS_QSPI_DTR));
    }
    LCDC_DBG("QSPI inited.");
    return LCDC_OK;
}
#endif

#ifdef LCDC_RGB_SUPPORT
static int lcdc_rgbif_init(struct lcdc_config_t *config)
{
    struct lcd_info_t *lcd = &config->lcd;
    struct hal_lcdc_rgbif_config_t rgbif_cfg;

    rgbif_cfg.pix_mhz = config->pix_mhz;
    rgbif_cfg.pix_bits = lcd->color_bits;
    rgbif_cfg.clk_inv = lcd->clk_inv;
    rgbif_cfg.hsync_inv = lcd->hsync_inv;
    rgbif_cfg.vsync_inv = lcd->vsync_inv;

    hal_lcdc_rgbif_init(&rgbif_cfg);

    config->pix_mhz = hal_cmu_lcdc_get_pixmhz();

    LCDC_DBG("RGBIF inited.");
    return LCDC_OK;
}
#endif

int lcdc_init_param(struct lcdc_config_t *config)
{
    int htotal, vtotal;
    struct lcd_info_t *lcd = &config->lcd;

    hal_lcdc_disp_pn_reset();

    /* first disable frame auto-send. */
    hal_lcdc_gen_frame_enable(true);

    if (config->mode == LCDC_VIDEO_MODE) {
        hal_lcdc_video_mode_enable(true);
        hal_lcdc_dumb_vsync_mode(2);
        hal_lcdc_source_switch(true);
    } else {
        hal_lcdc_smpn_enable();
    }

    if (config->bus == LCDC_BUS_DSI || config->bus == LCDC_BUS_DUAL_DSI) {
        hal_lcdc_stby_vsync_set(config->vsync_stby, 1, 5);
        hal_lcdc_porch_vsync_set(32, 32, 10);
        hal_lcdc_pix_vsync_set(30, 31);
    }

    if (!QSPI_BUS(config->bus)) {
        htotal = lcd->width + lcd->hbp + lcd->hfp + lcd->hsync - 1;
        vtotal = lcd->height + lcd->vbp + lcd->vfp + lcd->vsync - 1;
        if (config->mode == LCDC_VIDEO_MODE) {
            int vstart = lcd->vfp + lcd->hfp;
            hal_lcdc_lvsepxcount(vstart, vstart + 1);

            htotal += 16 * config->bus_lane;
            vtotal += 4;
            hal_lcdc_bgl_hporch_set(lcd->hfp, lcd->hbp);
            hal_lcdc_bgl_vporch_set(lcd->vfp, lcd->vbp);
        }
        hal_lcdc_bgl_vhtotal_set(htotal, vtotal);
    }

    if (QSPI_BUS(config->bus))
        hal_lcdc_lsize(LCDC_LAYER_SPU, lcd->width - 1, lcd->height - 1);
    else
        hal_lcdc_lsize(LCDC_LAYER_SPU, lcd->width, lcd->height);

    if (config->mode == LCDC_COMMAND_MODE)
        hal_lcdcl_enable(LCDC_LAYER_SPU, true);

    hal_lcdc_lcolor(LCDC_LAYER_SPU, 0);

    if (lcd->color_bits == 16)
        hal_lcdc_smpn_mode_set(5);
    else
        hal_lcdc_smpn_mode_set(3);

    if (!QSPI_BUS(config->bus))
        hal_lcdc_disp_mem_enable(true);

#ifdef QSPI_USE_DSI_PATH
    if (QSPI_BUS(config->bus)) {
        hal_lcdc_disp_mem_enable(true);
        hal_lcdc_stby_vsync_set(config->vsync_stby, 1, 5);
        hal_lcdc_lsize(LCDC_LAYER_SPU, lcd->width, lcd->height);
    }
#endif

    hal_lcdc_arqos_ctrl_config(0xc);

    hal_lcdc_panelpath_dmatrans_trigger_enable(LCDC_LAYER_GRA, true);

    hal_lcdc_set_sync_line_pixs(5);
    hal_lcdc_set_sync_lines(0);

    hal_lcdc_smpn_vsync_delaymode_set(1);

    hal_lcdc_disp_extra_delay(0);
    hal_lcdc_sl_dmaburst_length_set(LCDC_BURST_128B);
#ifdef FPGA
    hal_lcdc_ol_dmaburst_length_set(2);
#else
    hal_lcdc_ol_dmaburst_length_set(4);
#endif

    /* Configure dither */
    hal_lcdc_dither_s4x8(true);
    hal_lcdc_dither(false, 1, 0, 0, 0);
#ifdef LCDC_QSPI_SUPPORT
    if (QSPI_BUS(config->bus)) {
        hal_lcdc_qspi_swaprb_enable(lcd->swap_rb);
    } else
#endif
    {
        if (lcd->color_bits == 16)
            hal_lcdc_disp_color_fmt(DSI_RGB565);
        else if (lcd->color_bits == 18)
            hal_lcdc_disp_color_fmt(DSI_RGB666);
        else
            hal_lcdc_disp_color_fmt(DSI_RGB888);

        if (lcd->swap_rb)
            hal_lcdc_disp_dsi_data_swap(DSI_BD_BGR);
        else
            hal_lcdc_disp_dsi_data_swap(DSI_BD_RGB);
    }

    /* Configure enable axi bus arbiter fast mode */
    hal_lcdc_axifastmode_enable(true);
    hal_lcdc_yuv_fast_mode(true, 0xffff);
    hal_lcdc_axi_outstanding(LCDC_OUTSTANDING_32);
    hal_lcdc_disp_avsync_trigger_disable(true);
    hal_lcdc_disp_svsync_trigger_disable(true);

    hal_lcdc_zorder_set(config->bus_port0, config->zorder);

    lcdc_config = config;
    lcdc_layer_select(config->bus_port0, 0xFF); // dsi0 select all layer
    if (config->mode == LCDC_VIDEO_MODE)
        lcdc_switch_init();

    LCDC_INFO("lcdc is initialized.");
    return LCDC_OK;
}

int lcdc_init(struct lcdc_config_t *config)
{
    int ret = LCDC_OK;

    if (!config) {
        LCDC_INFO("lcdc config is NULL!");
        return LCDC_FAILED;
    }

    if (QSPI_BUS(config->bus)) {
#ifdef LCDC_QSPI_SUPPORT
        if (config->mode == LCDC_VIDEO_MODE) {
            LCDC_INFO("QSPI doesn't support video mode.");
            return LCDC_INVAL;
        }
#ifdef LCDC_MIP_SUPPORT
        /* initialize mip convertor */
        if (config->bus == LCDC_BUS_MIP)
            pmu_mip_open(lcdc_mip_irq_handler);
#endif
        /* initialize qspi bus */
        ret = lcdc_qspi_init(config);
#else
        LCDC_INFO("QSPI is not supported!");
        return LCDC_NOT_SUPPORT;
#endif
    } else if (config->bus == LCDC_BUS_DSI) {
#ifdef LCDC_DSI_SUPPORT
        ret = lcdc_dsi_init(config);
#else
        LCDC_INFO("DSI is not supported!");
        return LCDC_NOT_SUPPORT;
#endif
    }
    else if (config->bus == LCDC_BUS_DUAL_DSI) {
#ifdef LCDC_DUAL_DSI_SUPPORT
        if (config->bus_port0 == config->bus_port1) {
            LCDC_INFO("DUAL-DSI is enabled, but bus port0 is same with port1.");
            return LCDC_FAILED;
        }
        ret = lcdc_dsi_init(config);
#else
        LCDC_INFO("DUAL-DSI is not supported!");
        return LCDC_NOT_SUPPORT;
#endif
    } else if (config->bus == LCDC_BUS_RGB) {
#ifdef LCDC_RGB_SUPPORT
        if (config->mode == LCDC_COMMAND_MODE) {
            LCDC_INFO("RGB interface doesn't support command mode.");
            return LCDC_INVAL;
        }
        ret = lcdc_rgbif_init(config);
#endif
    } else {
        LCDC_INFO("invalid display bus type %d", config->bus);
        return LCDC_INVAL;
    }

    if (ret != LCDC_OK) {
        LCDC_INFO("failed to init display bus, ret = %d", ret);
        return LCDC_FAILED;
    }

    return lcdc_init_param(config);
}

int lcdc_exit(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    /* Disable all lcdc layers */
    for (int i = 0; i < LAYER_MAX_COUNT; i++) {
        if (layer_config[i].enable)
            lcdc_layer_disable((enum hal_lcdc_layer_e)i);
    }

    /* if lcdc is working, stop it */
    if (lcdc_started) {
        uint32_t lk = int_lock();
        if (lcdc_config->mode == LCDC_VIDEO_MODE) {
            /* FIXME, hal_lcdcl_dumb_enable(false) is not safe, it
             * may cause LCDC bus ack error, so do nothing in here. */
        } else {
            lcdc_te_irq_disable();
            hal_lcdcl_enable(LCDC_LAYER_SPU, false);
        }
        lcdc_irq_disable();
        lcdc_started = false;
        int_unlock(lk);

#ifdef LCDC_MIP_SUPPORT
        if (lcdc_config->bus == LCDC_BUS_MIP)
            pmu_mip_stop();
#endif
    }

#ifdef LCDC_QSPI_SUPPORT
    if (QSPI_BUS(lcdc_config->bus)) {
        hal_lcdc_qspi_exit();
#ifdef LCDC_MIP_SUPPORT
        if (lcdc_config->bus == LCDC_BUS_MIP)
            pmu_mip_close();
#endif
    }
#endif
#ifdef LCDC_DSI_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_DSI) {
        hal_dsi_exit(lcdc_config->bus_port0);
    } else if (lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
        hal_dsi_exit(lcdc_config->bus_port0);
        hal_dsi_exit(lcdc_config->bus_port1);
    }
#endif
#ifdef LCDC_RGB_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_RGB) {
        hal_lcdc_rgbif_exit();
    }
#endif

    for (int i = 0; i < LAYER_MAX_COUNT; i++)
        memset(&layer_config[i], 0, sizeof(struct lcdc_layer_config_t));

    lcdc_config = NULL;
    LCDC_INFO("lcdc exit.");
    return LCDC_OK;
}

int lcdc_reset(void)
{
    int ret = LCDC_OK;

    CHECK_CONFIG_RETURN_ERROR;

#ifdef LCDC_QSPI_SUPPORT
    if (QSPI_BUS(lcdc_config->bus)) {
        hal_lcdc_qspi_exit();
    }
#endif
#ifdef LCDC_DSI_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_DSI) {
        hal_dsi_exit(lcdc_config->bus_port0);
    } else if (lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
        hal_dsi_exit(lcdc_config->bus_port0);
        hal_dsi_exit(lcdc_config->bus_port1);
    }
#endif
#ifdef LCDC_RGB_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_RGB) {
        hal_lcdc_rgbif_exit();
    }
#endif

    /* init lcdc by config */
    ret = lcdc_init(lcdc_config);
    if (ret != LCDC_OK)
        goto exit;

    /* re-init all layers */
    for (int i = 0; i < LAYER_MAX_COUNT; i++) {
        if (layer_config[i].enable) {
            ret = lcdc_layer_config((enum hal_lcdc_layer_e)i, &layer_config[i]);
            if (ret != LCDC_OK)
                break;
        }
    }

exit:
    LCDC_INFO("lcdc reset %s", ret ? "failed" : "okay");
    return ret;
}

int lcdc_reset_simple(void)
{
    int ret = LCDC_OK;

    CHECK_CONFIG_RETURN_ERROR;

    /* init lcdc param only by config */
    ret = lcdc_init_param(lcdc_config);
    if (ret != LCDC_OK)
        goto exit;

    /* re-init all layers */
    for (int i = 0; i < LAYER_MAX_COUNT; i++) {
        if (layer_config[i].enable) {
            ret = lcdc_layer_config((enum hal_lcdc_layer_e)i, &layer_config[i]);
            if (ret != LCDC_OK)
                break;
        }
    }

exit:
    LCDC_INFO("lcdc reset %s", ret ? "failed" : "okay");
    return ret;
}

int lcdc_get_config(struct lcdc_config_t **config)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (!config)
        return LCDC_FAILED;

    *config = lcdc_config;
    return LCDC_OK;
}

int lcdc_register_framedone_cb(void (*cb)(void))
{
    CHECK_CONFIG_RETURN_ERROR;

    lcdc_config->framedone_cb = cb;
    return LCDC_OK;
}

int lcdc_register_vsync_cb(void (*cb)(void))
{
    CHECK_CONFIG_RETURN_ERROR;

    lcdc_config->vsync_cb = cb;
    return LCDC_OK;
}

int lcdc_register_error_cb(void (*cb)(int))
{
    CHECK_CONFIG_RETURN_ERROR;

    lcdc_config->error_cb = cb;
    return LCDC_OK;
}

int lcdc_set_bgcolor(uint32_t color)
{
    CHECK_CONFIG_RETURN_ERROR;

    hal_lcdc_lcolor(LCDC_LAYER_SPU, color);
    return LCDC_OK;
}

int lcdc_display_resize(uint16_t width, uint16_t height)
{
    CHECK_CONFIG_RETURN_ERROR;

#ifdef QSPI_USE_DSI_PATH
    hal_lcdc_lsize(LCDC_LAYER_SPU, width, height);
#else
    if (QSPI_BUS(lcdc_config->bus))
        hal_lcdc_lsize(LCDC_LAYER_SPU, width - 1, height - 1);
    else
        hal_lcdc_lsize(LCDC_LAYER_SPU, width, height);
#endif
#ifdef LCDC_DSI_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
        hal_dsi_config_width(lcdc_config->bus_port0, width);
        hal_dsi_config_width(lcdc_config->bus_port1, width);
    } else if (lcdc_config->bus == LCDC_BUS_DSI) {
        hal_dsi_config_width(lcdc_config->bus_port0, width);
    }
#endif
    return LCDC_OK;
}

int lcdc_layer_config(uint8_t layer_id, struct lcdc_layer_config_t *config)
{
    int ret;

    CHECK_CONFIG_RETURN_ERROR;

    if (!config) {
        LCDC_INFO("layer config is NULL.");
        return LCDC_FAILED;
    }
    if (layer_id >= LAYER_MAX_COUNT) {
        LCDC_INFO("invalid layer id.");
        return LCDC_FAILED;
    }
    if (config->scale_percent > 0 &&
        (config->scale_percent <= 25 || config->scale_percent >= 400)) {
        LCDC_INFO("scale percent must in 25%% ~ 400%%.");
        return LCDC_FAILED;
    }
    if (config->rotate % 90) {
        LCDC_INFO("layer rotation only supports 90/180/270.");
        return LCDC_FAILED;
    }
    if ((config->rotate % 180 == 90) && layer_id != LCDC_LAYER_GRA) {
        LCDC_INFO("layer rotation (90/270) only supported by LCDC_LAYER_GRA.");
        return LCDC_FAILED;
    }
    if (config->tiled && config->rotate % 180 == 0 && (layer_id != LCDC_LAYER_TILED
        && config->tile_type != LCDC_TILED_TYPE_TFBDC)) {
        LCDC_INFO("tiled + rotation 0/180 only works with LCDC_LAYER_TILED.");
        return LCDC_FAILED;
    }
    if (config->tiled && config->rotate % 180 == 90 && layer_id != LCDC_LAYER_GRA) {
        LCDC_INFO("tiled + rotation 90/270 only works with LCDC_LAYER_GRA.");
        return LCDC_FAILED;
    }
    if (layer_id == LCDC_LAYER_TILED && !CHECK_TILED_FORMAT(config->format)) {
        LCDC_INFO("TILED layer only supports RGBA888/RGB888/RGB565.");
        return LCDC_FAILED;
    }

    if (config->rotate == 180) {
        config->vmirror = true;
        config->hmirror = true;
    }
    if (config->buf_width == 0)
        config->buf_width = config->width;
    if (config->buf_height == 0)
        config->buf_height = config->height;

    if (config->rotate && (config->buf_width % 8)) {
        LCDC_INFO("layer rotation need buffer width aligned to 8 pixels.");
        return LCDC_FAILED;
    }

    /* Save layer config first */
    memcpy(&layer_config[layer_id], config, sizeof(struct lcdc_layer_config_t));
    config = &layer_config[layer_id];

    hal_lcdc_yuv2rgb_enable(layer_id, true, false, false);

    if (CHECK_YUV_FORMAT(config->format)) {
        ret = lcdc_layer_set_yuv_buffer(layer_id, config->y_buffer, config->u_buffer,
                            config->v_buffer, config->stride, config->height);
    } else {
        ret = lcdc_layer_set_buffer(layer_id, (uint8_t *)(uintptr_t)config->buffer, config->stride);
    }
    if (ret != LCDC_OK) {
        LCDC_INFO("failed to configure layer buffer.");
        return LCDC_FAILED;
    }

    hal_lcdc_lsize(layer_id, config->width, config->height);

    if (layer_id != LCDC_LAYER_DEC)
        hal_lcdc_lpfc_fmt(layer_id, config->format);

#ifdef QSPI_USE_DSI_PATH
    hal_lcdc_sponscreen_set(layer_id, config->screen_y + 1, config->screen_x + 1);
#else
    if (QSPI_BUS(lcdc_config->bus) || lcdc_config->mode == LCDC_VIDEO_MODE)
        hal_lcdc_sponscreen_set(layer_id, config->screen_y, config->screen_x);
    else
        hal_lcdc_sponscreen_set(layer_id, config->screen_y + 1, config->screen_x + 1);
#endif

    if (layer_id == LCDC_LAYER_DEC) {
        hal_lcdc_dec_set_size(config->buf_width, config->buf_height);
        if (config->dec_mode == LCDC_DEC_HV_SAMPLE)
            hal_lcdc_dec_set_tile_mode(LCDC_DEC_TILED);
        else
            hal_lcdc_dec_set_tile_mode(LCDC_DEC_LINEAR);
        hal_lcdc_dec_set_color_format(config->dec_format);
        hal_lcdc_dec_set_compress_mode(config->dec_mode);
        hal_lcdc_dec_swap_rb(true);
        if (QSPI_BUS(lcdc_config->bus))
            hal_lcdc_dec_set_data_delay(1);
        else
            hal_lcdc_dec_set_data_delay(2);
    } else if (layer_id == LCDC_LAYER_TILED || config->tile_type == LCDC_TILED_TYPE_TFBDC) {
        int tiled_fmt = LCDC_TILED_FMT_32BIT;
        if (lcdc_color_format_bpp(config->format) == 16)
            tiled_fmt = LCDC_TILED_FMT_16BIT;
        else if (lcdc_color_format_bpp(config->format) == 24)
            tiled_fmt = LCDC_TILED_FMT_24BIT;
        if (config->tile_type == LCDC_TILED_TYPE_TFBDC) {
            uint8_t bpp = lcdc_color_format_bpp(config->format) / 8;
            uint16_t tiled_size = config->tile_w * config->tile_h;
            uint32_t x_tile_diff = (config->buf_width  - config->width) / config->tile_w;
            uint32_t xtile = config->offset_x / config->tile_w;
            uint32_t tile = config->offset_y * config->stride / (tiled_size * bpp) + xtile;
            uint32_t tile_num = config->buf_width * config->height / tiled_size - x_tile_diff;
            uint32_t xtile_num = config->width/config->tile_w;
            POSSIBLY_UNUSED uint32_t header_size = config->buf_width * config->buf_height / 8;
            config->buffer = TFBC_MMAP(config->buffer, config->stride * config->buf_height + header_size, config->mmu_flags, config->mmu_enable);
            LCDC_INFO("x:%d,y:%d,w:%d,h:%d, buf_width:%d, tile:%d, tile_num:%d, xtile:%d, xtile_num:%d, xtile_diff:%d", config->offset_x , config->offset_y, \
                config->width, config->height, config->buf_width, tile, tile_num, xtile, xtile_num, x_tile_diff);
            hal_lcdc_ltfbdc_startaddr(layer_id, (uint32_t)config->buffer, config->buf_width, config->buf_height, bpp);
            hal_lcdc_ltfbdc_size(layer_id, config->tile_w, tile, tile_num, xtile, xtile_num, x_tile_diff, config->vmirror);
            hal_lcdc_ltfbdc_set_format(layer_id, tiled_fmt);
            hal_lcdc_ltfbdc_swap_rb(layer_id, !lcdc_config->lcd.swap_rb);
            hal_lcdc_ltfbdc_set_data_delay(layer_id, config->tfbdc_cfg->tfbdc_delay);
            hal_lcdc_ltfbdc_set_lossy(layer_id, config->tfbdc_cfg->tfbdc_lossy);
            hal_lcdc_ltfbdc_set_arqos(layer_id, config->tfbdc_cfg->tfbdc_qos);
        } else {
            hal_lcdc_tiled_set_size(config->buf_width, config->buf_height);
            hal_lcdc_tiled_set_format(tiled_fmt);
            hal_lcdc_tiled_swap_rb(true);
        }
    }

    hal_lcdc_set_vmirror_enable(layer_id, config->vmirror);
    hal_lcdc_set_hmirror_enable(config->hmirror, config->width, config->height);

    if (config->rotate % 180 == 90) {
        hal_lcdc_rotate_enable(layer_id, true, config->rotate,
                               config->width, config->height, config->tiled);
    }

    if (config->scale_percent == 0 || config->scale_percent == 100) {
        hal_lcdc_lzoom_set(layer_id, config->width, config->height);
    } else {
        uint16_t zoom_w = config->width * config->scale_percent / 100;
        uint16_t zoom_h = config->height * config->scale_percent / 100;
        if (layer_id == LCDC_LAYER_DMA || layer_id == LCDC_LAYER_TVD)
            ret = lcdc_layer_scaler_enable(layer_id, config->scale_percent);
        else
            ret = hal_lcdc_lzoom_set(layer_id, zoom_w, zoom_h);
        if (ret != LCDC_OK) {
            LCDC_INFO("layer:%d failed to configure scaler or zoom.", layer_id);
        }
    }

    hal_lcdc_lpfc_ltransp(layer_id, LCDC_PIXEL_ALPHA, config->alpha);

    if (config->blend_mode == 0)
        hal_lcdc_blend_nocompose(config->port, layer_id);
    else
        hal_lcdc_blend_drm(config->port, layer_id, config->blend_mode, config->alpha);

    if (layer_id == LCDC_LAYER_SMT)
        lcdc_layer_split(layer_id, config->split_cfg, true);

    hal_lcdcl_enable(layer_id, config->enable);

    if (config->tile_type == LCDC_TILED_TYPE_TFBDC)
        hal_lcdc_ltfbdc_enable(layer_id, config->enable, QSPI_BUS(lcdc_config->bus), config->vmirror);

    LCDC_INFO("layer[%d] is configured.", layer_id);
    return LCDC_OK;
}

int lcdc_layer_resize(uint8_t layer_id, uint16_t width, uint16_t height)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (layer_id >= LAYER_MAX_COUNT) {
        LCDC_INFO("invalid layer id.");
        return LCDC_FAILED;
    }

    layer_config[layer_id].width = width;
    layer_config[layer_id].height = height;

    hal_lcdc_lsize(layer_id, width, height);
    hal_lcdc_lzoom_set(layer_id, width, height);

    LCDC_INFO("layer[%d] resize to %dx%d.", layer_id, width, height);
    return LCDC_OK;
}

int lcdc_layer_set_buffer(uint8_t layer_id, uint8_t *buffer, uint32_t stride)
{
    uint32_t addr;
    uint8_t bpp;
    uint16_t w, h, x, y;

    CHECK_CONFIG_RETURN_ERROR;

    if (layer_id >= LAYER_MAX_COUNT) {
        LCDC_INFO("invalid layer id.");
        return LCDC_FAILED;
    }
    if (layer_id == LCDC_LAYER_SMT && layer_config[layer_id].format == LCDC_CFMT_YUV422_UYVY) {
        LCDC_INFO("SMT layer not support YUV format.");
        return LCDC_FAILED;
    }

    bpp = lcdc_color_format_bpp(layer_config[layer_id].format);
    w = layer_config[layer_id].width;
    h = layer_config[layer_id].height;
    x = layer_config[layer_id].offset_x;
    y = layer_config[layer_id].offset_y;

    if (layer_config[layer_id].tile_type != LCDC_TILED_TYPE_TFBDC)
        buffer = FB_MMAP(buffer, stride * h, layer_config[layer_id].mmu_flags, layer_config[layer_id].mmu_enable);

    layer_config[layer_id].buffer = buffer;
    layer_config[layer_id].stride = stride;

    /* calculate start address */
    uint8_t tile_w = layer_config[layer_id].tile_w;
    uint8_t tile_h = layer_config[layer_id].tile_h;
    if (layer_config[layer_id].tiled) {
        if (layer_config[layer_id].tile_type == LCDC_TILED_TYPE_TFBDC) {
            /* check tfbc size is in display area */
            if (layer_config[layer_id].screen_x + w > lcdc_config->lcd.width) {
                w = lcdc_config->lcd.width - layer_config[layer_id].screen_x;
            }
            if (layer_config[layer_id].screen_y + h > lcdc_config->lcd.height) {
                h = lcdc_config->lcd.height - layer_config[layer_id].screen_y;
            }
        }
        x = ((x + tile_w - 1)/tile_w) * tile_w;
        y = ((y + tile_h - 1)/tile_h) * tile_h;
        w = w & ~(tile_w - 1);
        h = h & ~(tile_h - 1);
        layer_config[layer_id].offset_x = x;
        layer_config[layer_id].offset_y = y;
        lcdc_layer_resize(layer_id, w, h);
    }
    if (layer_config[layer_id].vmirror) {
        uint16_t offs = (layer_config[layer_id].tiled) ? (h - tile_h) : h - 1;
        addr = (uint32_t)buffer + ((y + offs) * stride) + (x * bpp / 8);
    } else {
        if (layer_config[layer_id].format == LCDC_CFMT_YUV422_UYVY)
            hal_lcdc_yuv2rgb_enable(layer_id, true, false, x & 1);

        addr = (uint32_t)buffer + (y * stride) + (x * bpp / 8);
    }

    hal_lcdc_lpitch(layer_id, stride);
    hal_lcdc_lstartaddr(layer_id, addr);

    LCDC_DBG("layer[%d] (x:%d,y:%d) buffer -> %08x, pitch -> %u", layer_id, x, y, addr, stride);
    return LCDC_OK;
}

int lcdc_layer_set_yuv_buffer(uint8_t layer_id, uint8_t *y_buffer, uint8_t *u_buffer,
                              uint8_t *v_buffer, uint16_t width, uint16_t height)
{
    uint32_t y_pitch, uv_pitch;
    uint16_t x, y;

    CHECK_CONFIG_RETURN_ERROR;

    if (layer_id != LCDC_LAYER_DMA && layer_id != LCDC_LAYER_TVD) {
        LCDC_INFO("only DMA layer support YUV planar format.");
        return LCDC_FAILED;
    }
    if (!y_buffer) {
        LCDC_INFO("Y buffer is NULL.");
        return LCDC_FAILED;
    }

    x = layer_config[layer_id].offset_x;
    y = layer_config[layer_id].offset_y;

    if (layer_config[layer_id].format == LCDC_CFMT_YUV420_P16) {
        if (!u_buffer)
            u_buffer = y_buffer + width * height * 2;
        if (!v_buffer)
            v_buffer = u_buffer + width * height / 2;
        y_pitch = width * 2;
        uv_pitch = width;
        y_buffer += y * y_pitch + x * 2;
        u_buffer += y / 2 * uv_pitch + x / 2;
        v_buffer += y / 2 * uv_pitch + x / 2;
        y_buffer = FB_MMAP(y_buffer, width * height * 2, layer_config[layer_id].mmu_flags, layer_config[layer_id].mmu_enable);
        u_buffer = FB_MMAP(u_buffer, width * height / 2, layer_config[layer_id].mmu_flags, layer_config[layer_id].mmu_enable);
        v_buffer = FB_MMAP(v_buffer, width * height / 2, layer_config[layer_id].mmu_flags, layer_config[layer_id].mmu_enable);
        hal_lcdc_yuv_fix(layer_id, width, LCDC_CFMT_YUV420_P16);
    } else if (layer_config[layer_id].format == LCDC_CFMT_YUV420_NV12) {
        if (!u_buffer)
            u_buffer = y_buffer + width * height;
        y_pitch = width;
        uv_pitch = width;
        y_buffer += y * y_pitch + x;
        u_buffer += y / 2 * uv_pitch + x;
        hal_lcdc_yuv2rgb_enable(layer_id, true, false, x & 1);
        y_buffer = FB_MMAP(y_buffer, width * height, layer_config[layer_id].mmu_flags, layer_config[layer_id].mmu_enable);
        u_buffer = FB_MMAP(u_buffer, width * height, layer_config[layer_id].mmu_flags, layer_config[layer_id].mmu_enable);
    } else if (layer_config[layer_id].format == LCDC_CFMT_YUV420_I420) {
        if (!u_buffer)
            u_buffer = y_buffer + width * height;
        if (!v_buffer)
            v_buffer = u_buffer + width * height / 4;
        y_pitch = width;
        uv_pitch = width / 2;
        y_buffer += y * y_pitch + x;
        u_buffer += y / 2 * uv_pitch + x / 2;
        v_buffer += y / 2 * uv_pitch + x / 2;
        y_buffer = FB_MMAP(y_buffer, width * height, layer_config[layer_id].mmu_flags, layer_config[layer_id].mmu_enable);
        u_buffer = FB_MMAP(u_buffer, width * height / 2, layer_config[layer_id].mmu_flags, layer_config[layer_id].mmu_enable);
        v_buffer = FB_MMAP(v_buffer, width * height / 2, layer_config[layer_id].mmu_flags, layer_config[layer_id].mmu_enable);
    } else if (layer_config[layer_id].format == LCDC_CFMT_YUV422_I422) {
        if (!u_buffer)
            u_buffer = y_buffer + width * height;
        if (!v_buffer)
            v_buffer = u_buffer + width * height / 2;
        y_pitch = width;
        uv_pitch = width / 2;
        y_buffer += y * y_pitch + x;
        u_buffer += y * uv_pitch + x / 2;
        v_buffer += y * uv_pitch + x / 2;
        y_buffer = FB_MMAP(y_buffer, width * height, layer_config[layer_id].mmu_flags, layer_config[layer_id].mmu_enable);
        u_buffer = FB_MMAP(u_buffer, width * height / 2, layer_config[layer_id].mmu_flags, layer_config[layer_id].mmu_enable);
        v_buffer = FB_MMAP(v_buffer, width * height / 2, layer_config[layer_id].mmu_flags, layer_config[layer_id].mmu_enable);
    } else {
        LCDC_INFO("%s: only support YUV422/420 planar formats.", __func__);
        return LCDC_FAILED;
    }

    layer_config[layer_id].y_buffer = y_buffer;
    layer_config[layer_id].u_buffer = u_buffer;
    layer_config[layer_id].v_buffer = v_buffer;
    layer_config[layer_id].stride = width;

    /* update buffer address for vertical mirror */
    if (layer_config[layer_id].vmirror) {
        uint32_t uv_offs = 0;
        if (layer_config[layer_id].format == LCDC_CFMT_YUV422_I422) {
            uv_offs = uv_pitch * (height - 1);
        } else {  /* YUV420 */
            uv_offs = uv_pitch * (height / 2 - 1);
        }
        y_buffer += y_pitch * (height - 1);;
        u_buffer += uv_offs;
        if (v_buffer)
            v_buffer += uv_offs;
    }

    hal_lcdc_yuv_lpitch(layer_id, y_pitch, uv_pitch, uv_pitch);
    hal_lcdc_yuv_lstartaddr(layer_id, (uint32_t)y_buffer, (uint32_t)u_buffer, (uint32_t)v_buffer);
    LCDC_DBG("layer[%d] (x:%d,y:%d) buffer -> Y %p, U %p, V %p, %ux%u",
                layer_id, x, y, y_buffer, u_buffer, v_buffer, width, height);
    return LCDC_OK;
}

int lcdc_layer_split(int layer_id, struct lcdc_split_config_t *split_cfg, bool enable)
{
    uint32_t addr;
    uint8_t bpp;
    uint16_t x, y;
    uint16_t tst_starty = 100;
    bool next_region_needed = false;
    POSSIBLY_UNUSED bool mmu_enable = layer_config[layer_id].mmu_enable;
    POSSIBLY_UNUSED uint32_t mmu_flags = layer_config[layer_id].mmu_flags;

    if (layer_id != LCDC_LAYER_SMT) {
        LCDC_INFO("lcdc_layer_split invalid layer id:%d.", layer_id);
        return LCDC_FAILED;
    }
    if (split_cfg == NULL || !enable) {
        hal_lcdc_lsplit_disable(layer_id);
        if (split_cfg != NULL) {
            if (split_cfg->region1.buffer != NULL)
                FB_UNMAP(split_cfg->region1.buffer, mmu_flags, mmu_enable);
            if (split_cfg->region2.buffer != NULL)
                FB_UNMAP(split_cfg->region2.buffer, mmu_flags, mmu_enable);
            if (split_cfg->region3.buffer != NULL)
                FB_UNMAP(split_cfg->region3.buffer, mmu_flags, mmu_enable);
            if (split_cfg->region4.buffer != NULL)
                FB_UNMAP(split_cfg->region4.buffer, mmu_flags, mmu_enable);
        }
        return LCDC_OK;
    }

    layer_config[layer_id].split_cfg = split_cfg;
    bpp = lcdc_color_format_bpp(layer_config[layer_id].format) / 8;
    struct lcdc_split_region_t *region = &split_cfg->region4;
    if (region->buffer != NULL) {
        uint32_t stride = region->pix_stride * bpp;
        x = region->buffer_x;
        y = region->buffer_y;
        region->buffer = FB_MMAP(region->buffer, region->buffer_h * stride, mmu_flags, mmu_enable);
        addr = (uint32_t)region->buffer + (y * stride) + (x * bpp);
        hal_lcdc_lsplit_region(layer_id, 3, region->screen_x + 1, region->screen_y + 1, region->screen_w, region->screen_h);
        hal_lcdc_lsplit_buffer(layer_id, 3, addr, region->buffer_w , region->buffer_h, stride);
        next_region_needed = true;
        tst_starty = region->screen_y;
    }
    region = &split_cfg->region3;
    if (region->buffer != NULL) {
        uint32_t stride = region->pix_stride * bpp;
        x = region->buffer_x;
        y = region->buffer_y;
        if (tst_starty < y + region->screen_y)
        {
            LCDC_INFO("lcdc_layer_split invalid region3 y:%d, tst_starty:%d", y, tst_starty);
            return LCDC_FAILED;
        }
        region->buffer = FB_MMAP(region->buffer, region->buffer_h * stride, mmu_flags, mmu_enable);
        addr = (uint32_t)region->buffer + (y * stride) + (x * bpp);
        hal_lcdc_lsplit_region(layer_id, 2, region->screen_x + 1, region->screen_y + 1, region->screen_w, region->screen_h);
        hal_lcdc_lsplit_buffer(layer_id, 2, addr, region->buffer_w , region->buffer_h, stride);
        next_region_needed = true;
        tst_starty = region->screen_y;
    } else if (next_region_needed) {
        LCDC_INFO("lcdc_layer_split invalid region, region3 not set.");
        return LCDC_FAILED;
    }
    region = &split_cfg->region2;
    if (region->buffer != NULL) {
        uint32_t stride = region->pix_stride * bpp;
        x = region->buffer_x;
        y = region->buffer_y;
        if (tst_starty < y + region->screen_y)
        {
            LCDC_INFO("lcdc_layer_split invalid region2 y:%d, tst_starty:%d", y, tst_starty);
            return LCDC_FAILED;
        }
        region->buffer = FB_MMAP(region->buffer, region->buffer_h * stride, mmu_flags, mmu_enable);
        addr = (uint32_t)region->buffer + (y * stride) + (x * bpp);
        hal_lcdc_lsplit_region(layer_id, 1, region->screen_x + 1, region->screen_y + 1, region->screen_w, region->screen_h);
        hal_lcdc_lsplit_buffer(layer_id, 1, addr, region->buffer_w , region->buffer_h, stride);
        next_region_needed = true;
        tst_starty = region->screen_y;
    } else if (next_region_needed) {
        LCDC_INFO("lcdc_layer_split invalid region, region2 not set.");
        return LCDC_FAILED;
    }
    region = &split_cfg->region1;
    if (region->buffer != NULL) {
        uint32_t stride = region->pix_stride * bpp;
        x = region->buffer_x;
        y = region->buffer_y;
        if (tst_starty < y + region->screen_y)
        {
            LCDC_INFO("lcdc_layer_split invalid region1 y:%d, tst_starty:%d", y, tst_starty);
            return LCDC_FAILED;
        }
        region->buffer = FB_MMAP(region->buffer, region->buffer_h * stride, mmu_flags, mmu_enable);
        addr = (uint32_t)region->buffer + (y * stride) + (x * bpp);
        hal_lcdc_lsplit_region(layer_id, 0, region->screen_x + 1, region->screen_y + 1, region->screen_w, region->screen_h);
        hal_lcdc_lsplit_buffer(layer_id, 0, addr, region->buffer_w , region->buffer_h, stride);
    } else if (next_region_needed) {
        LCDC_INFO("lcdc_layer_split invalid region, region1 not set.");
        return LCDC_FAILED;
    }
    LCDC_DBG("layer[%d] split", layer_id);
    return LCDC_OK;
}

int lcdc_layer_enable(uint8_t layer_id)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (layer_id >= LAYER_MAX_COUNT) {
        LCDC_INFO("invalid layer id.");
        return LCDC_FAILED;
    }
    if (layer_config[layer_id].buffer == NULL) {
        LCDC_INFO("layer[%d] buffer is NULL.", layer_id);
        return LCDC_FAILED;
    }

    layer_config[layer_id].enable = true;

    hal_lcdcl_enable(layer_id, true);
    if (layer_config[layer_id].tile_type == LCDC_TILED_TYPE_TFBDC)
        hal_lcdc_ltfbdc_enable(layer_id, true, QSPI_BUS(lcdc_config->bus), layer_config[layer_id].vmirror);

    LCDC_DBG("layer[%d] is enabled.", layer_id);
    return LCDC_OK;
}

int lcdc_layer_disable(uint8_t layer_id)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (layer_id >= LAYER_MAX_COUNT) {
        LCDC_INFO("invalid layer id.");
        return LCDC_FAILED;
    }

    struct lcdc_layer_config_t *config = &layer_config[layer_id];
    config->enable = false;
    hal_lcdcl_enable(layer_id, false);

    if ((layer_id == LCDC_LAYER_DMA || layer_id == LCDC_LAYER_TVD) &&
        layer_config[layer_id].scale_percent != 0 &&
        layer_config[layer_id].scale_percent != 100) {
        lcdc_layer_scaler_disable(layer_id);
    }

    if (layer_id == LCDC_LAYER_SMT)
        lcdc_layer_split(layer_id, config->split_cfg, false);

    if (config->tile_type == LCDC_TILED_TYPE_TFBDC)
        hal_lcdc_ltfbdc_enable(layer_id, false, QSPI_BUS(lcdc_config->bus), false);

    if (CHECK_YUV_FORMAT(config->format)) {
        if (config->y_buffer)
            FB_UNMAP(config->y_buffer, config->mmu_flags, config->mmu_enable);
        if (config->u_buffer)
            FB_UNMAP(config->u_buffer, config->mmu_flags, config->mmu_enable);
        if (config->v_buffer)
            FB_UNMAP(config->v_buffer, config->mmu_flags, config->mmu_enable);
    }
    else {
        if (config->tile_type == LCDC_TILED_TYPE_TFBDC)
            TFBC_UNMAP(config->buffer, config->mmu_enable);
        else
            FB_UNMAP(config->buffer, config->mmu_flags, config->mmu_enable);
    }

    LCDC_DBG("layer[%d] is disabled.", layer_id);
    return LCDC_OK;
}

int lcdc_layer_chromakey_enable(uint8_t layer_id, uint32_t key_color, uint32_t new_color,
                                uint8_t new_alpha)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (layer_id >= LAYER_MAX_COUNT) {
        LCDC_INFO("invalid layer id.");
        return LCDC_FAILED;
    }

    hal_lcdc_lchromakey(layer_id, key_color, new_color, new_alpha);
    hal_lcdc_lchromakeyenable(layer_id, true, LCDC_CKEY_RGB_ALL);
    return LCDC_OK;
}

int lcdc_layer_chromakey_disable(uint8_t layer_id)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (layer_id >= LAYER_MAX_COUNT) {
        LCDC_INFO("invalid layer id.");
        return LCDC_FAILED;
    }

    hal_lcdc_lchromakeyenable(layer_id, false, 0);
    return LCDC_OK;
}

int lcdc_layer_scaler_enable(uint8_t layer_id, uint16_t scale_percent)
{
    int ret = LCDC_FAILED;
    POSSIBLY_UNUSED int format;
    int x, y, offset_x, offset_y;
    uint16_t w, h;
    uint16_t zoom_w, zoom_h;

    CHECK_CONFIG_RETURN_ERROR;

    if (layer_id != LCDC_LAYER_DMA && layer_id != LCDC_LAYER_TVD) {
        LCDC_INFO("scaler only support DMA/TVD layer.");
        return LCDC_FAILED;
    }
    if (scale_percent <= 25 || scale_percent >= 400) {
        LCDC_INFO("scaler only suppports: 25%% ~ 400%%, now get %u%%.", scale_percent);
        return LCDC_FAILED;
    }

    format = layer_config[layer_id].format;
    x = layer_config[layer_id].screen_x;
    y = layer_config[layer_id].screen_y;
    w = layer_config[layer_id].width;
    h = layer_config[layer_id].height;
    zoom_w = w * scale_percent / 100;
    zoom_h = h * scale_percent / 100;
    offset_x = layer_config[layer_id].offset_x;
    offset_y = layer_config[layer_id].offset_y;
    if (x < 0 || y < 0) {
        LCDC_INFO("scaler layer need screen_x/y >= 0.");
        return LCDC_FAILED;
    }

    /* check zoom size is in display area */
    if (zoom_w + x > lcdc_config->lcd.width) {
        int d = zoom_w + x - lcdc_config->lcd.width;
        w -= d * 100 / scale_percent + 1;
    }
    if (zoom_h + y > lcdc_config->lcd.height) {
        int d = zoom_h + y - lcdc_config->lcd.height;
        h -= d * 100 / scale_percent + 1;
    }
#if defined(CHIP_BEST1701)
    w &= ~1;
    h &= ~1;
#else
    /* force alignment, YUV format 16 pixels, RGB format 4 pixels */
    if (CHECK_YUV_FORMAT(format)) {
        w &= 0xfff0;
        h &= 0xfff0;
    } else {
        w &= 0xfffc;
        h &= 0xfffc;
    }
#endif

    /* reconfig layer width/height */
    if (w != layer_config[layer_id].width || h != layer_config[layer_id].height) {
        lcdc_layer_resize(layer_id, w, h);

        zoom_w = w * scale_percent / 100;
        zoom_h = h * scale_percent / 100;
    }
    zoom_w &= ~1;
    zoom_h &= ~1;
    LCDC_INFO("scaler screen(%d,%d),buf(%d,%d)in_w:%d,in_h:%d,out_w=%d,out_h=%d,scale:%d", x,y,offset_x,offset_y,w,h,zoom_w,zoom_h,scale_percent);
    if (layer_id == LCDC_LAYER_DMA)
        ret = hal_lcdc_scaler_enable(true, w, h, zoom_w, zoom_h, QSPI_BUS(lcdc_config->bus));
    else if (layer_id == LCDC_LAYER_TVD)
        ret = hal_lcdc_scaler2_enable(true, w, h, zoom_w, zoom_h, QSPI_BUS(lcdc_config->bus));
    if (ret == LCDC_OK)
        layer_config[layer_id].scale_percent = scale_percent;
    return ret;
}

int lcdc_layer_scaler_disable(uint8_t layer_id)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (layer_id != LCDC_LAYER_DMA && layer_id != LCDC_LAYER_TVD) {
        LCDC_INFO("scaler only support DMA/TVD layer.");
        return LCDC_FAILED;
    }
    layer_config[layer_id].scale_percent = 0;
    if (layer_id == LCDC_LAYER_DMA)
        return hal_lcdc_scaler_enable(false, 0, 0, 0, 0, false);
    else if (layer_id == LCDC_LAYER_TVD)
        return hal_lcdc_scaler2_enable(false, 0, 0, 0, 0, false);
    return LCDC_FAILED;
}

int lcdc_layer_select(uint8_t bus_port, uint8_t layermask)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (bus_port == 1)
        return hal_lcdc_dsi1_layer_select(layermask);
    else
        return hal_lcdc_dsi0_layer_select(layermask);
}

int lcdc_layer_blend(uint8_t bus_port, int blend_mode, int alpha_mode, uint8_t alpha)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (bus_port == 1)
        return hal_lcdc_dsi1_blend(blend_mode, alpha_mode, alpha);
    else
        return hal_lcdc_dsi0_blend(blend_mode, alpha_mode, alpha);
}

bool lcdc_is_started(void)
{
    return lcdc_started;
}

int lcdc_start(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    uint32_t lk = int_lock();
    lcdc_irq_enable();

    if (lcdc_config->mode == LCDC_VIDEO_MODE) {
        hal_lcdcl_dumb_enable(true);
    } else {
        lcdc_te_irq_enable();
        hal_lcdcl_enable(LCDC_LAYER_SPU, true);
    }
    lcdc_started = true;
    int_unlock(lk);

#ifdef LCDC_MIP_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_MIP)
        pmu_mip_start();
#endif

    LCDC_INFO("ready for frame transfer.");
    return LCDC_OK;
}

int lcdc_stop(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    uint32_t lk = int_lock();
    if (lcdc_config->mode == LCDC_VIDEO_MODE) {
        hal_lcdcl_dumb_enable(false);
    } else {
        lcdc_te_irq_disable();
        hal_lcdcl_enable(LCDC_LAYER_SPU, false);
    }

    lcdc_irq_disable();
    lcdc_started = false;
    int_unlock(lk);

#ifdef LCDC_MIP_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_MIP)
        pmu_mip_stop();
#endif

    LCDC_INFO("stop frame transfer.");
    return LCDC_OK;
}

int lcdc_stop_simple(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    uint32_t lk = int_lock();
    lcdc_te_irq_disable();
    lcdc_irq_disable();
    lcdc_started = false;
    int_unlock(lk);

    LCDC_INFO("stop frame transfer.");
    return LCDC_OK;
}

int lcdc_switch_init(void)
{
#if defined(LCDC_HAS_DUAL_LCDC) && defined(LCDC_DSI_SUPPORT)
#ifdef LCDC_FIRST_USED_BY_AP
    hal_dsi_select_ap(lcdc_config->bus_port0);
    if (lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
        hal_dsi_select_ap(lcdc_config->bus_port1);
    }
#ifndef CHIP_SUBSYS_CASYS
    hal_lcdc_frame_halt();
#endif
#else
    hal_dsi_select_lp(lcdc_config->bus_port0);
    if (lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
        hal_dsi_select_lp(lcdc_config->bus_port1);
    }
#ifdef CHIP_SUBSYS_CASYS
    hal_lcdc_frame_halt();
#endif
#endif
#endif
    return LCDC_OK;
}

int lcdc_switch_start(bool toAP)
{
#if defined(LCDC_HAS_DUAL_LCDC) && defined(LCDC_DSI_SUPPORT)
    /* TODO: first IPC to peer to do lcdc_start() */
    if (lcdc_config->bus == LCDC_BUS_DSI) {
        hal_dsi_switch_to_ap(lcdc_config->bus_port0, toAP);
        hal_cmu_dsi_switch(toAP);
    } else if (lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
        hal_dsi_switch_to_ap(lcdc_config->bus_port0, toAP);
        hal_dsi_switch_to_ap(lcdc_config->bus_port1, toAP);
        hal_cmu_dsi_switch(toAP);
        hal_cmu_dsi1_switch(toAP);
    }
    hal_lcdc_switch_start();
#endif
    return LCDC_OK;
}

int lcdc_xfer_frame(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (lcdc_config->mode == LCDC_VIDEO_MODE)
        return LCDC_OK;

#ifdef LCDC_QSPI_SUPPORT
    if (QSPI_BUS(lcdc_config->bus)) {
        hal_lcdc_qspi_gen_start();

        LCDC_DBG("start frame transfer.");
        return LCDC_OK;
    }
#endif

#ifdef LCDC_DSI_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_DSI) {
        hal_dsi_start_hs_clock(lcdc_config->bus_port0);
    } else if (lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
        hal_dsi_start_hs_clock(lcdc_config->bus_port0);
        hal_dsi_start_hs_clock(lcdc_config->bus_port1);
    }
#endif

    hal_lcdc_gen_start();

    LCDC_DBG("start frame transfer.");
    return LCDC_OK;
}

int lcdc_start_writeback(uint8_t *dstbuf)
{
    CHECK_CONFIG_RETURN_ERROR;

    hal_lcdc_lsize(LCDC_LAYER_WB, lcdc_config->lcd.width, lcdc_config->lcd.height);
    if (lcdc_config->lcd.color_bits == 16) {
        hal_lcdc_lpfc_fmt(LCDC_LAYER_WB, LCDC_CFMT_RGB565);
        hal_lcdc_lpitch(LCDC_LAYER_WB, lcdc_config->lcd.width * 2);
    } else {
        hal_lcdc_lpfc_fmt(LCDC_LAYER_WB, LCDC_CFMT_RGBA8888);
        hal_lcdc_lpitch(LCDC_LAYER_WB, lcdc_config->lcd.width * 4);
    }
    if (layer_config[LCDC_LAYER_WB].mmu_enable) {
        POSSIBLY_UNUSED uint16_t stride, h;
        stride =  layer_config[LCDC_LAYER_WB].stride;
        h = layer_config[LCDC_LAYER_WB].height;
        dstbuf = FB_MMAP(dstbuf, stride * h, layer_config[LCDC_LAYER_WB].mmu_flags, true);
    }
    hal_lcdcl_enable(LCDC_LAYER_WB, true);
    hal_lcdc_wdma_toggle_frame();
    if (dstbuf != NULL)
        hal_lcdc_lstartaddr(LCDC_LAYER_WB, (uint32_t)dstbuf);
    hal_lcdc_start_wb();

    LCDC_DBG("start blend/blit layers.");
    return LCDC_OK;
}

int lcdc_sleep(void)
{
    CHECK_CONFIG_RETURN_ERROR;

#ifdef LCDC_DSI_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_DSI) {
        hal_dsi_enter_ulps_mode(lcdc_config->bus_port0);
        hal_dsi_sleep(lcdc_config->bus_port0);
    } else if (lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
        hal_dsi_enter_ulps_mode(lcdc_config->bus_port0);
        hal_dsi_sleep(lcdc_config->bus_port0);
        hal_dsi_enter_ulps_mode(lcdc_config->bus_port1);
        hal_dsi_sleep(lcdc_config->bus_port1);
    }
#endif
#ifdef LCDC_QSPI_SUPPORT
    if (QSPI_BUS(lcdc_config->bus)) {
        hal_lcdc_qspi_sleep();
#ifdef LCDC_MIP_SUPPORT
        if (lcdc_config->bus == LCDC_BUS_MIP)
            pmu_mip_sleep();
#endif
    }
#endif
    hal_lcdc_sleep();
    return LCDC_OK;
}

int lcdc_wakeup(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    hal_lcdc_wakeup();

#ifdef LCDC_QSPI_SUPPORT
    if (QSPI_BUS(lcdc_config->bus)) {
        hal_lcdc_qspi_wakeup();
#ifdef LCDC_MIP_SUPPORT
        if (lcdc_config->bus == LCDC_BUS_MIP)
            pmu_mip_wakeup();
#endif
    }
#endif
#ifdef LCDC_DSI_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_DSI) {
        hal_dsi_wakeup(lcdc_config->bus_port0);
        hal_dsi_exit_ulps_mode(lcdc_config->bus_port0);
    } else if (lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
        hal_dsi_wakeup(lcdc_config->bus_port0);
        hal_dsi_exit_ulps_mode(lcdc_config->bus_port0);
        hal_dsi_wakeup(lcdc_config->bus_port1);
        hal_dsi_exit_ulps_mode(lcdc_config->bus_port1);
    }
#endif
    return LCDC_OK;
}

int lcdc_send_cmd(uint8_t bus_port, uint8_t cmd)
{
    int ret = LCDC_FAILED;

    CHECK_CONFIG_RETURN_ERROR;

#ifdef LCDC_QSPI_SUPPORT
    if (QSPI_BUS(lcdc_config->bus)) {
        ret = hal_lcdc_qspi_send_command(bus_port, cmd);
    }
#endif
#ifdef LCDC_DSI_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_DSI || lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
        ret = hal_dsi_send_cmd(bus_port, cmd);
    }
#endif
    return ret;
}

int lcdc_send_cmd_list(uint8_t bus_port, uint8_t cmd, uint8_t param_count, uint8_t *params)
{
    int ret = LCDC_FAILED;

    CHECK_CONFIG_RETURN_ERROR;

#ifdef LCDC_QSPI_SUPPORT
    if (QSPI_BUS(lcdc_config->bus)) {
        ret = hal_lcdc_qspi_send_command_list(bus_port, cmd, param_count, params);
    }
#endif
#ifdef LCDC_DSI_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_DSI || lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
        ret = hal_dsi_send_cmd_list(bus_port, cmd, param_count, params);
    }
#endif
    return ret;
}

int lcdc_read_cmd(uint8_t bus_port, uint8_t cmd, uint8_t *data, int len)
{
    int ret = LCDC_FAILED;

    CHECK_CONFIG_RETURN_ERROR;

#ifdef LCDC_QSPI_SUPPORT
    if (QSPI_BUS(lcdc_config->bus)) {
        ret = hal_lcdc_qspi_read_command(bus_port, cmd, data, len);
    }
#endif
#ifdef LCDC_DSI_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_DSI || lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
        ret = hal_dsi_read_cmd(bus_port, cmd, data, len);
    }
#endif
    return ret;
}

int lcdc_dither_enable(uint8_t level, const uint8_t *lut_data, uint8_t lut_count)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (lut_data) {
        for (int i = 0; i < lut_count; i++)
            hal_lcdc_dither_write(i, lut_data[i]);
    }

    hal_lcdc_dither_s4x8(true);
    hal_lcdc_dither(true, level, 0, 0, 0);
    return LCDC_OK;
}

int lcdc_dither_disable(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    hal_lcdc_dither_s4x8(false);
    hal_lcdc_dither(false, 0, 0, 0, 0);
    return LCDC_OK;
}

int lcdc_gamma_enable(float gamma)
{
#ifdef NOSTD
    ASSERT(0, "%s: need math.h, set NOSTD=0", __func__);
    return LCDC_FAILED;
#else
    CHECK_CONFIG_RETURN_ERROR;

    for (int i = 0; i < 256; i++) {
        float f = (float)i / 255.0f;
        uint8_t v = (uint8_t)(255.0f * powf(f, 1.0f / gamma));
        hal_lcdc_write_gamma(i, v);
    }

    hal_lcdc_gamma_enable(true);
    return LCDC_OK;
#endif
}

int lcdc_gamma_enable_by_lut(const uint8_t *lut_data, int lut_count)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (!lut_data || lut_count == 0)
        return LCDC_FAILED;

    if (lut_count > 256)
        lut_count = 256;

    for (int i = 0; i < lut_count; i++)
        hal_lcdc_write_gamma(i, lut_data[i]);

    hal_lcdc_gamma_enable(true);
    return LCDC_OK;
}

int lcdc_gamma_disable(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    hal_lcdc_gamma_enable(false);
    return LCDC_OK;
}

int lcdc_palette_enable(int format, const uint32_t *colors, int count)
{
    CHECK_CONFIG_RETURN_ERROR;

    return lcdc_layer_palette_enable(LCDC_LAYER_GRA, format, colors, count);
}

int lcdc_palette_disable(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    return lcdc_layer_palette_disable(LCDC_LAYER_GRA);
}

int lcdc_layer_palette_enable(uint8_t layer_id, int format, const uint32_t *colors, int count)
{
    CHECK_CONFIG_RETURN_ERROR;
    if (layer_id  > LCDC_LAYER_TVD)
        return LCDC_FAILED;
    if (format != LCDC_CFMT_PALETTE_4 && format != LCDC_CFMT_PALETTE_8)
        return LCDC_FAILED;
    if (!colors || count == 0)
        return LCDC_FAILED;

    if (format == LCDC_CFMT_PALETTE_8) {
        if (count > 256)
            count = 256;
        for (int i = 0; i < count; i++)
            hal_lcdc_write_palette(layer_id, i, colors[i]);

    } else if (format == LCDC_CFMT_PALETTE_4) {
        if (count > 16)
            count = 16;
        for (int i = 0; i < count; i++)
            hal_lcdc_write_palette(layer_id, (i << 4), colors[i]);
    }
    hal_lcdc_palette_enable(layer_id, true);
    return LCDC_OK;
}

int lcdc_layer_palette_disable(uint8_t layer_id)
{
    CHECK_CONFIG_RETURN_ERROR;
    if (layer_id  > LCDC_LAYER_TVD)
        return LCDC_FAILED;

    hal_lcdc_palette_enable(layer_id, false);
    return LCDC_OK;
}

int lcdc_mip_init_config(void *data)
{
#ifdef LCDC_MIP_SUPPORT
    pmu_mip_config(data);
    return LCDC_OK;
#else
    return LCDC_NOT_SUPPORT;
#endif
}

#endif /* LCDC_BASE */
