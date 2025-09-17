
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

//#define LCDC_DEBUG
#ifdef  LCDC_DEBUG
#define LCDC_DBG(...)               TR_INFO( TR_MOD(HAL), "//LCDC: " __VA_ARGS__)
#else
#define LCDC_DBG(...)
#endif
#define LCDC_INFO(...)              TR_INFO( TR_MOD(HAL), "//LCDC: " __VA_ARGS__)

/* Enable QSPI support as default */
#define LCDC_QSPI_SUPPORT           1

/* Enable DSI support if CHIP has DSI */
#ifdef DSI_BASE
#define LCDC_DSI_SUPPORT            1
#ifdef DSI1_BASE
#define LCDC_DUAL_DSI_SUPPORT       1
#endif
#endif

/* Only 1700 support RGB interface for now */
#if defined(CHIP_BEST1700)
#define LCDC_RGB_SUPPORT            1
#endif

static struct lcdc_config_t *lcdc_config = NULL;

#define LAYER_MAX_COUNT             6  /* GRA, DMA, TVG, TVD, DEC, WB */

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

static int lcdc_color_format_bpp(enum hal_lcdc_color_format_e format)
{
    switch (format) {
        case LCDC_CFMT_YUV422_YUYV:
        case LCDC_CFMT_RGB565:
        case LCDC_CFMT_BGR565:
            return 2;
        case LCDC_CFMT_RGB888:
        case LCDC_CFMT_BGR888:
            return 3;
        case LCDC_CFMT_RGBA8888:
        case LCDC_CFMT_ARGB8888:
        case LCDC_CFMT_BGRA8888:
        case LCDC_CFMT_ABGR8888:
            return 4;
        default:
            return 0;
    }
    return 0;
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
        LCDC_DBG("LCDC IRQ 0x%08x, DSI IRQ 0x%08x, DSI FLAGS 0x%08x", irqstat, dsistat, dsiflags);

        error |= (dsiflags & 3);

        if (lcdc_config->irq_cb)
            lcdc_config->irq_cb(irqstat, dsiflags);
#endif
    } else {
        LCDC_DBG("LCDC IRQ 0x%08x", irqstat);

        if (lcdc_config->irq_cb)
            lcdc_config->irq_cb(irqstat, 0);
    }

    if (layer_config[LCDC_LAYER_DEC].enable)
        framedone = hal_lcdc_irq_is_hwc_framedone(irqstat);
    else
        framedone = hal_lcdc_irq_is_framedone(irqstat);

    if (framedone) {
        LCDC_DBG("framedone got!");
        hal_lcdc_framedone_irqstat_clr();

#ifdef LCDC_QSPI_SUPPORT
        if (lcdc_config->bus == LCDC_BUS_QSPI)
            hal_lcdc_qspi_cs_sel(false);
#endif

        if (lcdc_config->framedone_cb)
            lcdc_config->framedone_cb();
    }

    if (hal_lcdc_irq_is_vsync(irqstat)) {
        LCDC_DBG("vsync got!");
        hal_lcdc_vsync_irqstat_clr();

        if (lcdc_config->mode == LCDC_VIDEO_MODE && lcdc_config->vsync_cb)
            lcdc_config->vsync_cb();
    }

    if (hal_lcdc_irq_is_error(irqstat)) {
        LCDC_DBG("error got!");
        hal_lcdc_error_irqstat_clr();

        error |= (irqstat & (LCDC_ERR_IRQ | LCDC_GRA_FF_UNDERFLOW | LCDC_DMA_FF_UNDERFLOW |
                            LCDC_TVG_FF_UNDERFLOW | LCDC_TVD_FF_UNDERFLOW));

        if (lcdc_config->error_cb)
            lcdc_config->error_cb(error);
    }
}

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
    LCDC_DBG("DSI inited.");
    return ret;
}
#endif

#ifdef LCDC_QSPI_SUPPORT
static int lcdc_qspi_init(struct lcdc_config_t *config)
{
    enum HAL_LCDC_QSPI_CFMT_E color_fmt;
    enum HAL_LCDC_QSPI_LINE_MODE_E line_mode;

    if (config->lcd.color_bits == 16)
        color_fmt = LCDC_QSPI_RGB565;
    else if (config->lcd.color_bits == 18)
        color_fmt = LCDC_QSPI_RGB666;
    else if (config->lcd.color_bits == 24)
        color_fmt = LCDC_QSPI_RGB888;
    else
        return LCDC_INVAL;

    if (config->bus_lane == 1)
        line_mode = LCDC_QSPI_1LINE;
    else if (config->bus_lane == 2)
        line_mode = LCDC_QSPI_2LINE;
    else if (config->bus_lane == 4)
        line_mode = LCDC_QSPI_4LINE;
    else
        return LCDC_INVAL;

    hal_lcdc_qspi_initialize(color_fmt, line_mode);
    LCDC_DBG("QSPI inited.");
    return LCDC_OK;
}
#endif

int lcdc_init(struct lcdc_config_t *config)
{
    int ret = LCDC_OK;
    struct lcd_info_t *lcd;
    int htotal, vtotal;

    if (!config) {
        LCDC_INFO("lcdc config is NULL!");
        return LCDC_FAILED;
    }

    if (config->bus == LCDC_BUS_QSPI) {
#ifdef LCDC_QSPI_SUPPORT
        if (config->mode == LCDC_VIDEO_MODE) {
            LCDC_INFO("QSPI doesn't support video mode.");
            return LCDC_INVAL;
        }
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
        hal_iomux_set_rgblcd0();
#endif
    } else {
        LCDC_INFO("invalid display bus type %d", config->bus);
        return LCDC_INVAL;
    }

    if (ret != LCDC_OK) {
        LCDC_INFO("failed to init display bus, ret = %d", ret);
        return LCDC_FAILED;
    }

    lcd = &config->lcd;

    //hal_cmu_lcdc_set_pixmhz(config->pix_mhz);
    hal_lcdc_disp_pn_reset();

    if (config->bus == LCDC_BUS_RGB)
        hal_lcdc_rgbif_enable(true, true, false, false);

    if (config->mode == LCDC_VIDEO_MODE)
        hal_lcdc_video_mode_enable(true);
    else
        hal_lcdc_smpn_enable();

    if (config->bus == LCDC_BUS_DSI || config->bus == LCDC_BUS_DUAL_DSI) {
        hal_lcdc_stby_vsync_set(0x40, 1, 5);
        hal_lcdc_porch_vsync_set(32, 32, 10);
        hal_lcdc_pix_vsync_set(30, 31);
    }

    if (config->bus != LCDC_BUS_QSPI) {
        htotal = lcd->width + lcd->hbp + lcd->hfp + lcd->hsync - 1;
        vtotal = lcd->height + lcd->vbp + lcd->vfp + lcd->vsync - 1;
        if (config->mode == LCDC_VIDEO_MODE) {
            int vstart = lcd->vfp + lcd->hfp;
            hal_lcdc_lvsepxcount(vstart, vstart + 1);

            htotal += 8;
            vtotal += 4;
        }
        hal_lcdc_bgl_hporch_set(lcd->hbp, lcd->hfp);
        hal_lcdc_bgl_vporch_set(lcd->vbp, lcd->vfp);
        hal_lcdc_bgl_vhtotal_set(htotal, vtotal);
    }

    if (config->bus == LCDC_BUS_QSPI)
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

    if (config->bus != LCDC_BUS_QSPI)
        hal_lcdc_disp_mem_enable(true);

    hal_lcdc_panelpath_dmatrans_trigger_enable(LCDC_LAYER_GRA, true);

    hal_lcdc_set_sync_line_pixs(5);
    hal_lcdc_set_sync_lines(0);

    hal_lcdc_smpn_vsync_delaymode_set(1);

    hal_lcdc_disp_extra_delay(0);
    hal_lcdc_sl_dmaburst_length_set(0x0f);
    hal_lcdc_ol_dmaburst_length_set(8);

    /* Configure dither */
    hal_lcdc_dither_s4x8(true);
    hal_lcdc_dither(false, 1, 0, 0, 0);

    if (config->bus == LCDC_BUS_QSPI) {
        hal_lcdc_qspi_swaprb_enable(lcd->swap_rb);
    } else {
        if (lcd->color_bits == 16)
            hal_lcdc_disp_color_fmt(DSI_RGB565);
        else
            hal_lcdc_disp_color_fmt(DSI_RGB888);

        if (lcd->swap_rb)
            hal_lcdc_disp_dsi_data_swap(DSI_BD_BGR);
        else
            hal_lcdc_disp_dsi_data_swap(DSI_BD_RGB);
    }

#if defined(CHIP_BEST1600) || defined(CHIP_BEST1700)
    /* Configure enable axi bus arbiter fast mode */
    hal_lcdc_axifastmode_enable(true);
#endif

    hal_lcdc_disp_avsync_trigger_disable(true);
    hal_lcdc_disp_svsync_trigger_disable(true);

    lcdc_config = config;
    LCDC_INFO("lcdc is initialized.");
    return LCDC_OK;
}

int lcdc_exit(void)
{
    CHECK_CONFIG_RETURN_ERROR;

#ifdef LCDC_QSPI_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_QSPI) {
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

    hal_cmu_lcdc_clock_disable();

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
    if (lcdc_config->bus == LCDC_BUS_QSPI) {
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

    hal_cmu_lcdc_clock_disable();

    /* init lcdc by config */
    ret = lcdc_init(lcdc_config);
    if (ret != LCDC_OK)
        goto exit;

    /* re-init all layers */
    for (int i = 0; i < LAYER_MAX_COUNT; i++) {
        ret = lcdc_layer_config(i, &layer_config[i]);
        if (ret != LCDC_OK)
            break;
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

int lcdc_register_irq_cb(void (*cb)(uint32_t, uint32_t))
{
    CHECK_CONFIG_RETURN_ERROR;

    lcdc_config->irq_cb = cb;
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

int lcdc_layer_config(uint8_t layer_id, struct lcdc_layer_config_t *config)
{
    uint32_t addr;

    CHECK_CONFIG_RETURN_ERROR;

    if (!config) {
        LCDC_INFO("layer config is NULL.");
        return LCDC_FAILED;
    }
    if (layer_id > LAYER_MAX_COUNT) {
        LCDC_INFO("invalid layer id.");
        return LCDC_FAILED;
    }

    hal_lcdc_lsize(layer_id, config->width, config->height);
    hal_lcdc_lzoom_set(layer_id, config->width, config->height);

    if (lcdc_config->bus == LCDC_BUS_QSPI)
        hal_lcdc_lsize(LCDC_LAYER_SPU, config->width - 1, config->height - 1);
    else
        hal_lcdc_lsize(LCDC_LAYER_SPU, config->width, config->height);

    if (layer_id != LCDC_LAYER_DEC)
        hal_lcdc_lpfc_fmt(layer_id, config->format);

    if (lcdc_config->mode == LCDC_VIDEO_MODE || lcdc_config->bus == LCDC_BUS_QSPI)
        hal_lcdc_sponscreen_set(layer_id, 0, 0);
    else
        hal_lcdc_sponscreen_set(layer_id, 1, 1);

    /* calculate start address */
    addr = (uint32_t)config->buffer + (config->offset_y * config->stride) +
            (config->offset_x * lcdc_color_format_bpp(config->format));

    hal_lcdc_lpitch(layer_id, config->stride);
    hal_lcdc_lstartaddr(layer_id, ADDR_CPU_TO_DEV(addr));

    if (layer_id == LCDC_LAYER_DEC) {
        hal_lcdc_dec_set_compress_mode(config->dec_mode);
        hal_lcdc_dec_set_color_format(config->dec_format);
        if (config->dec_mode == LCDC_DEC_HV_SAMPLE)
            hal_lcdc_dec_set_tile_mode(LCDC_DEC_TILED);
        else
            hal_lcdc_dec_set_tile_mode(LCDC_DEC_LINEAR);
    }

    hal_lcdcl_enable(layer_id, config->enable);

    if (config != &layer_config[layer_id])
        memcpy(&layer_config[layer_id], config, sizeof(struct lcdc_layer_config_t));

    LCDC_INFO("layer[%d] is configured.", layer_id);
    return LCDC_OK;
}

int lcdc_layer_resize(uint8_t layer_id, uint16_t width, uint16_t height)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (layer_id > LAYER_MAX_COUNT) {
        LCDC_INFO("invalid layer id.");
        return LCDC_FAILED;
    }

    layer_config[layer_id].width = width;
    layer_config[layer_id].height = height;

    hal_lcdc_lsize(layer_id, width, height);
    hal_lcdc_lzoom_set(layer_id, width, height);

    if (lcdc_config->bus == LCDC_BUS_QSPI)
        hal_lcdc_lsize(LCDC_LAYER_SPU, width - 1, height - 1);
    else
        hal_lcdc_lsize(LCDC_LAYER_SPU, width, height);

    LCDC_INFO("layer[%d] resize to %dx%d.", layer_id, width, height);
    return LCDC_OK;
}

int lcdc_layer_set_buffer(uint8_t layer_id, uint8_t *buffer)
{
    uint32_t addr;

    CHECK_CONFIG_RETURN_ERROR;

    if (layer_id > LAYER_MAX_COUNT) {
        LCDC_INFO("invalid layer id.");
        return LCDC_FAILED;
    }

    layer_config[layer_id].buffer = buffer;

    /* calculate start address */
    addr = (uint32_t)buffer + (layer_config[layer_id].offset_y * layer_config[layer_id].stride) +
           (layer_config[layer_id].offset_x * lcdc_color_format_bpp(layer_config[layer_id].format));

    hal_lcdc_lstartaddr(layer_id, ADDR_CPU_TO_DEV(addr));

    LCDC_DBG("layer[%d] buffer -> %p", layer_id, buffer);
    return LCDC_OK;
}

int lcdc_layer_set_stride(uint8_t layer_id, uint32_t stride)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (layer_id > LAYER_MAX_COUNT) {
        LCDC_INFO("invalid layer id.");
        return LCDC_FAILED;
    }

    layer_config[layer_id].stride = stride;

    hal_lcdc_lpitch(layer_id, stride);

    LCDC_DBG("layer[%d] pitch -> %u", layer_id, stride);
    return LCDC_OK;
}

int lcdc_layer_enable(uint8_t layer_id)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (layer_id > LAYER_MAX_COUNT) {
        LCDC_INFO("invalid layer id.");
        return LCDC_FAILED;
    }
    if (layer_config[layer_id].buffer == NULL) {
        LCDC_INFO("layer[%d] buffer is NULL.", layer_id);
        return LCDC_FAILED;
    }

    layer_config[layer_id].enable = true;

    hal_lcdcl_enable(layer_id, true);

    LCDC_DBG("layer[%d] is enabled.", layer_id);
    return LCDC_OK;
}

int lcdc_layer_disable(uint8_t layer_id)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (layer_id > LAYER_MAX_COUNT) {
        LCDC_INFO("invalid layer id.");
        return LCDC_FAILED;
    }

    layer_config[layer_id].enable = false;

    hal_lcdcl_enable(layer_id, false);

    LCDC_DBG("layer[%d] is disabled.", layer_id);
    return LCDC_OK;
}

int lcdc_layer_select(uint8_t bus_port, uint8_t layer_fg, uint8_t layer_bg)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (bus_port == 1)
        return hal_lcdc_dsi1_layer_select(layer_fg, layer_bg);
    else
        return hal_lcdc_dsi0_layer_select(layer_fg, layer_bg);
}

int lcdc_layer_blend(uint8_t bus_port, int blend_mode, int alpha_mode, uint8_t alpha)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (bus_port == 1)
        return hal_lcdc_dsi1_blend(blend_mode, alpha_mode, alpha);
    else
        return hal_lcdc_dsi0_blend(blend_mode, alpha_mode, alpha);
}

int lcdc_start(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    lcdc_irq_enable();
    lcdc_te_irq_enable();

    if (lcdc_config->mode == LCDC_VIDEO_MODE) {
        hal_lcdcl_dumb_enable(true);
        hal_lcdc_dumb_vsync_mode(2);
        hal_lcdc_source_switch(true);
    } else {
        hal_lcdc_gen_frame_enable(true);
    }

    LCDC_INFO("ready for frame transfer.");
    return LCDC_OK;
}

int lcdc_stop(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (lcdc_config->mode == LCDC_VIDEO_MODE)
        hal_lcdcl_dumb_enable(false);
    else
        hal_lcdc_gen_frame_enable(false);

    lcdc_te_irq_disable();
    lcdc_irq_disable();

    LCDC_INFO("stop frame transfer.");
    return LCDC_OK;
}

int lcdc_xfer_frame(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    if (lcdc_config->mode == LCDC_VIDEO_MODE)
        return LCDC_OK;

#ifdef LCDC_QSPI_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_QSPI) {
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

    hal_lcdcl_enable(LCDC_LAYER_WB, true);
    hal_lcdc_wdma_toggle_frame();
    hal_lcdc_lstartaddr(LCDC_LAYER_WB, ADDR_CPU_TO_DEV((uint32_t)dstbuf));
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
    if (lcdc_config->bus == LCDC_BUS_QSPI)
        hal_lcdc_qspi_sleep();
#endif
    hal_lcdc_sleep();
    return LCDC_OK;
}

int lcdc_wakeup(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    hal_lcdc_wakeup();

#ifdef LCDC_QSPI_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_QSPI)
        hal_lcdc_qspi_wakeup();
#endif
#ifdef LCDC_DSI_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_DSI) {
        hal_dsi_wakeup(lcdc_config->bus_port0);
        hal_dsi_exit_ulps_mode(lcdc_config->bus_port0);
    } else if (lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
        hal_dsi_wakeup(lcdc_config->bus_port0);
        hal_dsi_exit_ulps_mode(lcdc_config->bus_port0);
        hal_dsi_sleep(lcdc_config->bus_port1);
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
    if (lcdc_config->bus == LCDC_BUS_QSPI) {
        ret = hal_lcdc_qspi_send_command(cmd);
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
    if (lcdc_config->bus == LCDC_BUS_QSPI) {
        ret = hal_lcdc_qspi_send_command_list(cmd, param_count, params);
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
    if (lcdc_config->bus == LCDC_BUS_QSPI) {
        ret = hal_lcdc_qspi_read_command(cmd, data, len);
    }
#endif
#ifdef LCDC_DSI_SUPPORT
    if (lcdc_config->bus == LCDC_BUS_DSI || lcdc_config->bus == LCDC_BUS_DUAL_DSI) {
        ret = hal_dsi_read_cmd(bus_port, cmd, data, len);
    }
#endif
    return ret;
}

int lcdc_dither_enable(uint8_t level, uint8_t red, uint8_t green, uint8_t blue)
{
    CHECK_CONFIG_RETURN_ERROR;

    hal_lcdc_dither_s4x8(true);
    hal_lcdc_dither(true, level, red, green, blue);
    return LCDC_OK;
}

int lcdc_dither_disable(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    hal_lcdc_dither_s4x8(false);
    hal_lcdc_dither(false, 0, 0, 0, 0);
    return LCDC_OK;
}

int lcdc_chromakey_enable(uint32_t rgb, uint32_t rrgb, uint8_t ra)
{
    CHECK_CONFIG_RETURN_ERROR;

    hal_lcdc_lchromakey(0, rgb, rrgb, ra);
    hal_lcdc_lchromakeyenable(0, true);
    return LCDC_OK;
}

int lcdc_chromakey_disable(void)
{
    CHECK_CONFIG_RETURN_ERROR;

    hal_lcdc_lchromakeyenable(0, false);
    return LCDC_OK;
}

#endif /* LCDC_BASE */
