
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
    LCDC_BUS_RGB        = 0,
    LCDC_BUS_QSPI       = 1,
    LCDC_BUS_DSI        = 2,
    LCDC_BUS_DUAL_DSI   = 3,
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
};

struct lcdc_config_t {
    enum lcdc_mode_e mode;

    uint16_t pix_mhz;

    enum lcdc_bus_type_e bus;
    uint16_t bus_mhz;
    uint8_t  bus_lane;
    uint8_t  bus_port0;
    uint8_t  bus_port1;

    struct lcd_info_t lcd;

    int  te_pin;

    void (*irq_cb)(uint32_t irqstat, uint32_t flags);
    void (*framedone_cb)(void);
    void (*vsync_cb)(void);
    void (*error_cb)(int error);
};

struct lcdc_layer_config_t {
    bool enable;

    enum hal_lcdc_color_format_e format;

    uint16_t width;
    uint16_t height;

    uint16_t offset_x;
    uint16_t offset_y;

    uint8_t *buffer;
    uint32_t stride;

    /* for decnano layer */
    enum hal_lcdc_dec_compress_mode_e dec_mode;
    enum hal_lcdc_dec_format_e dec_format;
};

/**
 * @brief Initialize LCDC/DSI/QSPI.
 *
 * @param config LCDC & LCD panel parameters.
 */
int lcdc_init(struct lcdc_config_t *config);

/**
 * @brief Disable LCDC/DSI/QSPI.
 */
int lcdc_exit(void);

/**
 * @brief Re-init LCDC/DSI/QSPI.
 */
int lcdc_reset(void);

/**
 * @brief Get the currenct lcdc_config pointer.
 */
int lcdc_get_config(struct lcdc_config_t **config);

/**
 * @brief Register LCDC irq callback function.
 */
int lcdc_register_irq_cb(void (*cb)(uint32_t, uint32_t));

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
 */
int lcdc_layer_set_buffer(uint8_t layer_id, uint8_t *buffer);

/**
 * @brief Update layer buffer stride.
 *
 * @param layer_id The layer ID, 0 ~ 5, GRA/DMA/TVG/TVD/DEC/WB;
 */
int lcdc_layer_set_stride(uint8_t layer_id, uint32_t stride);

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
 * @brief Select layers for DSI0/DSI1.
 *
 * @param bus_port For which port, 0 or 1;
 * @param layer_fg The foregound layer, 0 ~ 5, GRA/DMA/TVG/TVD/DEC/WB;
 * @param layer_bg The backgound layer, 0 ~ 5, GRA/DMA/TVG/TVD/DEC/WB;
 */
int lcdc_layer_select(uint8_t bus_port, uint8_t layer_fg, uint8_t layer_bg);

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
 */
int lcdc_dither_enable(uint8_t level, uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Disable dither support.
 */
int lcdc_dither_disable(void);

/**
 * @brief Enable chromakey support.
 */
int lcdc_chromakey_enable(uint32_t rgb, uint32_t rrgb, uint8_t ra);

/**
 * @brief Disable chromakey support.
 */
int lcdc_chromakey_disable(void);

#ifdef __cplusplus
}
#endif

#endif /* __LCDC_DRV_H__ */
