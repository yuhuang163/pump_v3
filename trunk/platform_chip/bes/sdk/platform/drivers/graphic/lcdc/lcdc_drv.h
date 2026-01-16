
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

#ifndef __LCDC_DRV_H__
#define __LCDC_DRV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "hal_lcdc.h"
#include "hal_lcdc_qspi.h"
#include "hal_dsi.h"

enum lcdc_mode_e {
    LCDC_VIDEO_MODE     = 0,
    LCDC_COMMAND_MODE   = 1,
};

enum lcdc_bus_type_e {
    LCDC_BUS_RGB            = 0,
    LCDC_BUS_QSPI_SDR       = 1,
    LCDC_BUS_DSI            = 2,
    LCDC_BUS_DUAL_DSI       = 3,
    LCDC_BUS_MIP            = 4,
    LCDC_BUS_QSPI_DTR       = 5,
    LCDC_BUS_DUAL_QSPI      = 6,
    LCDC_BUS_DUAL_QSPI_DTR  = 7,

    LCDC_BUS_QSPI           = LCDC_BUS_QSPI_SDR,
};

enum lcdc_error_no_e {
    LCDC_ERROR_NONE           = 0,

    LCDC_ERROR_DSI_OVERFLOW   = (1 << 0),
    LCDC_ERROR_DSI_UNDERFLOW  = (1 << 1),
    LCDC_ERROR_IRQ            = (1 << 16),
    LCDC_ERROR_GRA_UNDERFLOW  = (1 << 25),
    LCDC_ERROR_DMA_UNDERFLOW  = (1 << 29),
    LCDC_ERROR_TVG_UNDERFLOW  = (1 << 9),
    LCDC_ERROR_TVD_UNDERFLOW  = (1 << 13),
};

struct lcd_info_t {
    uint16_t width;
    uint16_t height;

    uint16_t hfp;
    uint16_t hbp;
    uint16_t hsync;

    uint16_t vfp;
    uint16_t vbp;
    uint16_t vsync;

    uint8_t  color_bits; /* 8/16/18/24 */
    bool     swap_rb;    /* Swap red & blue order */

    /* RGB panel clk/hsync/vsync inverse */
    bool clk_inv;
    bool hsync_inv;
    bool vsync_inv;
};

struct lcdc_config_t {
    enum lcdc_mode_e mode;
    uint8_t zorder;

    uint16_t pix_mhz;

    enum lcdc_bus_type_e bus;
    uint16_t bus_mhz;
    uint8_t  bus_lane;
    uint8_t  bus_port0;
    uint8_t  bus_port1;

    struct lcd_info_t lcd;

    int te_pin;

    /* vsync standby delay between lines, in pixel clock cycles. */
    int vsync_stby;

    /* LCDC framedone IRQ callback function.
     * It's safe to enter low-power mode in here. */
    void (*framedone_cb)(void);
    /* TE or LCDC vsync IRQ callback function. */
    void (*vsync_cb)(void);
    /* LCDC error IRQ callback function. */
    void (*error_cb)(int error);
};

struct lcdc_layer_config_t {
    bool enable;
    bool mmu_enable;
    uint16_t mmu_flags;

    uint8_t alpha;
    uint8_t blend_mode;
    uint8_t port;
    enum hal_lcdc_color_format_e format;

    /* The display area width & height */
    uint16_t width;
    uint16_t height;

    /* The display area offset in the whole buffer */
    uint16_t offset_x;
    uint16_t offset_y;

    /* RGB and YUV packed pixel buffer,
     * Or YUV planar Y pixel buffer. */
    union {
        uint8_t *buffer;
        uint8_t *y_buffer;
    };
    /* YUV planar U/V pixel buffer, config it to a pointer or NULL.
     * If NULL, U/V buffer will be calculated by Y buffer. */
    uint8_t *u_buffer;
    uint8_t *v_buffer;

    /* For RGB or YUV packed format, stride is buffer width in bytes.
     * For YUV planar format, stride is Y width in pixels. */
    uint32_t stride;

    /* for decnano layer */
    enum hal_lcdc_dec_compress_mode_e dec_mode;
    enum hal_lcdc_dec_format_e dec_format;

    /* The offset in the screen */
    int16_t screen_x;
    int16_t screen_y;

    /* vertical mirror */
    bool vmirror;
    /* horizontal mirror */
    bool hmirror;

    /* rotation 90/180/270 */
    uint16_t rotate;

    /* tiled data (4x4), (16*4), supported by TILED & GRA layers.
     * GRA layer:  90 and 270 rotation;
     * TILED layer: no rotation, h & v mirror;
     */
    bool tiled;
    enum hal_lcdc_tiled_type_e tile_type;
    uint8_t tile_w;
    uint8_t tile_h;

    /* the buffer actual size for decnano & tiled layer */
    uint16_t buf_width;
    uint16_t buf_height;

    /* EXPERIMENT, BE CAREFUL!!
     * Enable scaling may cause frame transfer issue.
     * Scale percent for width & height, 25 ~ 400, 0/100 is disabled.
     */
    uint16_t scale_percent;

    struct lcdc_split_config_t *split_cfg;
    struct lcdc_tfbdc_config_t *tfbdc_cfg;
};

struct lcdc_split_region_t {
    /* The region in the screen */
    int16_t screen_x;
    int16_t screen_y;
    int16_t screen_w;
    int16_t screen_h;

    /* The display region in the buffer */
    int16_t buffer_x;
    int16_t buffer_y;
    uint16_t buffer_w;
    uint16_t buffer_h;

    /* stride is buffer total width in pixel. */
    uint32_t pix_stride;
    /* RGB pixel buffer. */
    uint8_t *buffer;
};

struct lcdc_split_config_t {
    struct lcdc_split_region_t region1;
    struct lcdc_split_region_t region2;
    struct lcdc_split_region_t region3;
    struct lcdc_split_region_t region4;
};

struct lcdc_tfbdc_config_t {
    enum hal_lcdc_tiled_format_e tfbdc_fmt;
    enum hal_lcdc_lossy_type_e tfbdc_lossy;
    uint8_t tfbdc_qos;
    uint8_t tfbdc_delay;
};

/**
 * @brief Initialize LCDC/DSI/QSPI.
 *
 * @param config LCDC & LCD panel parameters.
 */
int lcdc_init(struct lcdc_config_t *config);
int lcdc_init_param(struct lcdc_config_t *config);
/**
 * @brief Disable LCDC/DSI/QSPI.
 */
int lcdc_exit(void);

/**
 * @brief Re-init LCDC/DSI/QSPI.
 */
int lcdc_reset(void);

/**
 * @brief Re-init LCDC config psram.
 */
int lcdc_reset_simple(void);

/**
 * @brief Get the currenct lcdc_config pointer.
 */
int lcdc_get_config(struct lcdc_config_t **config);

/**
 * @brief Register LCDC framedone callback function.
 */
int lcdc_register_framedone_cb(void (*cb)(void));

/**
 * @brief Register LCDC vsync callback function.
 */
int lcdc_register_vsync_cb(void (*cb)(void));

/**
 * @brief Register LCDC error callback function.
 */
int lcdc_register_error_cb(void (*cb)(int));

/**
 * @brief Set default background color.
 *
 * @param color The background color value.
 */
int lcdc_set_bgcolor(uint32_t color);

/**
 * @brief Resize the display area for LCD panel.
 *
 * @param width  The display area width.
 * @param height The display area height.
 */
int lcdc_display_resize(uint16_t width, uint16_t height);

/**
 * @brief Configure layer parameters.
 *
 * @param layer_id The layer ID, 0 ~ 5, GRA/DMA/TVG/TVD/DEC/WB;
 * @param config The layer config pointer;
 */
int lcdc_layer_config(uint8_t layer_id, struct lcdc_layer_config_t *config);

/**
 * @brief Resize layer width & height.
 *
 * @param layer_id The layer ID, 0 ~ 5, GRA/DMA/TVG/TVD/DEC/WB;
 */
int lcdc_layer_resize(uint8_t layer_id, uint16_t width, uint16_t height);

/**
 * @brief Update layer buffer address.
 *
 * @param layer_id The layer ID, 0 ~ 5, GRA/DMA/TVG/TVD/DEC/WB;
 * @param buffer The layer buffer pointer;
 * @param stride The buffer stride, bufWidthInPixel * bytesInPixel;
 */
int lcdc_layer_set_buffer(uint8_t layer_id, uint8_t *buffer, uint32_t stride);

/**
 * @brief Update YUV layer buffer addresses.
 *
 * @param layer_id The layer ID, only support DMA/TVD (1/3) layer;
 * @param y_buffer The Y buffer pointer;
 * @param u_buffer The U buffer pointer, if NULL, lcdc will calculate it from y_buffer;
 * @param v_buffer The V buffer pointer, if NULL, lcdc will calculate it from u/y_buffer;
 * @param width    The Y buffer width in pixel;
 * @param height   The Y buffer height in pixel;
 */
int lcdc_layer_set_yuv_buffer(uint8_t layer_id, uint8_t *y_buffer, uint8_t *u_buffer,
                              uint8_t *v_buffer, uint16_t width, uint16_t height);

int lcdc_layer_split(int layer_id, struct lcdc_split_config_t *split_cfg, bool enable);

/**
 * @brief Enable layer for frame transfer.
 *
 * @param layer_id The layer ID, 0 ~ 5, GRA/DMA/TVG/TVD/DEC/WB;
 */
int lcdc_layer_enable(uint8_t layer_id);

/**
 * @brief Disable layer.
 */
int lcdc_layer_disable(uint8_t layer_id);

/**
 * @brief Enable layer chromakey.
 *
 * @param layer_id The layer ID, 0 ~ 3, GRA/DMA/TVG/TVD;
 * @param key_color The key color need be replace;
 * @param new_color The new color value;
 * @param new_alpha The new alpha value, 0 for tranparent;
 */
int lcdc_layer_chromakey_enable(uint8_t layer_id, uint32_t key_color,
                                uint32_t new_color, uint8_t new_alpha);

/**
 * @brief Disable chromakey support.
 */
int lcdc_layer_chromakey_disable(uint8_t layer_id);

/**
 * @brief Enable scaler to replace layer zoom.
 *
 * @param layer_id The layer ID, for now only support LCDC_LAYER_DMA;
 * @param scale_percent The scaling num in percent, 25 < n < 400;
 */
int lcdc_layer_scaler_enable(uint8_t layer_id, uint16_t scale_percent);

/**
 * @brief Disable scaler.
 */
int lcdc_layer_scaler_disable(uint8_t layer_id);

/**
 * @brief Select layers for DSI0/DSI1.
 *
 * @param bus_port For which port, 0 or 1;
 * @param layermask The layer mask with DMA/GRA/TVG/TVD/SMT/DEC/WB;
 */
int lcdc_layer_select(uint8_t bus_port, uint8_t layermask);

/**
 * @brief Configure layers blending mode.
 *
 * @param blend_mode The blending mode, LCDC_BLEND_MODE0/1/2;
 * @param alpha_mode The alpha mode, LCDC_CONST_ALPHA/LCDC_PIXEL_ALPHA;
 * @param alpha The alpha value for const alpha mode;
 */
int lcdc_layer_blend(uint8_t bus_port, int blend_mode, int alpha_mode, uint8_t alpha);

/**
 * @brief Enable(Ready for) frame transfer.
 *
 * For video mode, this function will enable frame auto-transfer;
 * For command mode, after this function, can use lcdc_xfer_frame() to send frame;
 */
int lcdc_start(void);

/**
 * @brief Disable frame transfer.
 */
int lcdc_stop(void);

/**
 * @brief Disable frame irq only.
 */
int lcdc_stop_simple(void);

/**
 * @brief Is lcdc is started or not stoped.
 */
bool lcdc_is_started(void);

/**
 * @brief Manually transfer one frame.
 *
 * Only for command mode;
 */
int lcdc_xfer_frame(void);

/**
 * @brief Blend/blit all enabled layers to writeback layer.
 *
 * @param dstbuf The writeback destination buffer;
 */
int lcdc_start_writeback(uint8_t *dstbuf);

/**
 * @brief LCDC/DSI/QSPI enter low power mode.
 */
int lcdc_sleep(void);

/**
 * @brief LCDC/DSI/QSPI exit low power mode.
 */
int lcdc_wakeup(void);

/**
 * @brief DSI/QSPI send command.
 *
 * @param bus_port The DSI/QSPI port number, 0 ~ 1;
 * @param cmd The command code;
 */
int lcdc_send_cmd(uint8_t bus_port, uint8_t cmd);

/**
 * @brief DSI/QSPI send command with parameters.
 *
 * @param bus_port The DSI/QSPI port number, 0 ~ 1;
 * @param cmd The command code;
 * @param param_count The parameter count in bytes;
 * @param params The parameter data array;
 */
int lcdc_send_cmd_list(uint8_t bus_port, uint8_t cmd, uint8_t param_count, uint8_t *params);

/**
 * @brief DSI/QSPI read data from panel.
 *
 * @param bus_port The DSI/QSPI port number, 0 ~ 1;
 * @param cmd The command code;
 * @param data The received data buffer;
 * @param len The received data length in bytes;
 */
int lcdc_read_cmd(uint8_t bus_port, uint8_t cmd, uint8_t *data, int len);

/**
 * @brief Enable dither support.
 *
 * @param level The dither level, 0~7;
 * @param lut_data The dither matrix data;
 * @param lut_count The matirx table count;
 */
int lcdc_dither_enable(uint8_t level, const uint8_t *lut_data, uint8_t lut_count);

/**
 * @brief Disable dither support.
 */
int lcdc_dither_disable(void);

/**
 * @brief Enable gamma adjustment.
 *
 * @param gamma The gamma value in float;
 */
int lcdc_gamma_enable(float gamma);

/**
 * @brief Enable gamma adjustment, with gamma LUT data.
 *
 * @param lut_data The LUT table data;
 * @param lut_count The table count;
 */
int lcdc_gamma_enable_by_lut(const uint8_t *lut_data, int lut_count);

/**
 * @brief Disable gamma adjustment.
 */
int lcdc_gamma_disable(void);

/**
 * @brief Enable palette for GRA/DMA layers with LCDC_CFMT_PALETTE_4/8 format.
 *
 * @param format LCDC_CFMT_PALETTE_4, or LCDC_CFMT_PALETTE_8;
 * @param colors The palette colors;
 * @param count  The color count;
 */
int lcdc_palette_enable(int format, const uint32_t *colors, int count);
int lcdc_layer_palette_enable(uint8_t layer_id, int format, const uint32_t *colors, int count);

/**
 * @brief Disable palette for GRA/DMA layers.
 */
int lcdc_palette_disable(void);
int lcdc_layer_palette_disable(uint8_t layer_id);

/**
 * @brief Initialize MIP bus configuration.
 *
 * Must be called before lcdc_init().
 */
int lcdc_mip_init_config(void *data);

/**
 * @brief Display switch between LP and AP.
 *
 */
int lcdc_switch_init(void);
int lcdc_switch_start(bool toAP);

#ifdef __cplusplus
}
#endif

#endif /* __LCDC_DRV_H__ */
