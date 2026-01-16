
/***************************************************************************
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

#ifndef __HAL_LCDC_H
#define __HAL_LCDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

enum hal_lcdc_result_e {
    LCDC_OK             = 0,
    LCDC_FAILED         = -1,
    LCDC_INVAL          = -2,
    LCDC_ERROR          = -3,
    LCDC_TIMEOUT        = -4,
    LCDC_NOT_SUPPORT    = -5,
};

enum hal_lcdc_layer_e {
    LCDC_LAYER_GRA = 0,
    LCDC_LAYER_DMA,
    LCDC_LAYER_TVG,
    LCDC_LAYER_TVD,
    LCDC_LAYER_SMT,
    LCDC_LAYER_DEC,
    LCDC_LAYER_TILED,
    LCDC_LAYER_WB,
    LCDC_LAYER_SPU,

    /* compatible with old version codes. */
    LCDC_LAYER_LFORE = LCDC_LAYER_GRA,
    LCDC_LAYER_LBACK = LCDC_LAYER_DMA,

    LCDC_LAYER_NONE = -1,
};

enum hal_lcdc_zorder_id_e {
    LCDC_ZORDER_BG = 0,
    LCDC_ZORDER_FIRST,
    LCDC_ZORDER_SECOND,
    LCDC_ZORDER_THIRD,
    LCDC_ZORDER_FOURTH,
};

enum hal_lcdc_color_format_e {
    LCDC_CFMT_PALETTE_4     = 5,
    LCDC_CFMT_PALETTE_8     = 6,
    LCDC_CFMT_RGB1555       = 10,
    LCDC_CFMT_RGB565        = 11,
    LCDC_CFMT_RGB888        = 12,
    LCDC_CFMT_RGBA8888      = 13,
    LCDC_CFMT_ARGB8888      = 21,
    LCDC_CFMT_YUV422_UYVY   = 45,  /* Packed UYVY */
    LCDC_CFMT_YUV422_I422   = 60,  /* Planar, Y:U:V */
    LCDC_CFMT_YUV420_I420   = 51,  /* Planar, Y:U:V */
    LCDC_CFMT_YUV420_NV12   = 52,  /* Semi-Planar, Y:UVUV */
    LCDC_CFMT_YUV420_P16    = 61,  /* Planar, I420 with 16 bits color component. */
    LCDC_CFMT_BGR565        = 63,
    LCDC_CFMT_BGR888        = 64,
    LCDC_CFMT_BGRA8888      = 65,
    LCDC_CFMT_ABGR8888      = 66,

    LCDC_CFMT_YUV422_PLANAR = LCDC_CFMT_YUV422_I422,
    LCDC_CFMT_YUV420_PLANAR = LCDC_CFMT_YUV420_I420,
};

enum hal_lcdc_colorkey_mode_e {
    LCDC_CKEY_NONE      = 0x0,
    LCDC_CKEY_Y         = 0x1,
    LCDC_CKEY_U         = 0x2,
    LCDC_CKEY_V         = 0x4,
    LCDC_CKEY_YUV_ALL   = 0x5,
    LCDC_CKEY_G         = 0x6,
    LCDC_CKEY_B         = 0x7,
    LCDC_CKEY_RGB_ALL   = 0x3,
};

enum hal_lcdc_dsi_cfmt_e {
    DSI_RGB888 = 28,
    DSI_RGB666,
    DSI_RGB565,
    DSI_RGB101010,
};

enum hal_lcdc_dsi_dswap_e {
    DSI_BD_RGB = 0,
    DSI_BD_GRB = 1,
    DSI_BD_RBG = 2,
    DSI_BD_GBR = 3,
    DSI_BD_BRG = 4,
    DSI_BD_BGR = 5,
};

enum hal_lcdc_dec_tile_mode_e {
    LCDC_DEC_LINEAR = 0,
    LCDC_DEC_TILED,
};

enum hal_lcdc_dec_compress_mode_e {
    LCDC_DEC_DISABLE = 0,
    LCDC_DEC_NON_SAMPLE,
    LCDC_DEC_H_SAMPLE,
    LCDC_DEC_HV_SAMPLE,
};

enum hal_lcdc_dec_format_e {
    LCDC_DEC_RGBA8888 = 0,
    LCDC_DEC_RGBX8888 = 1,

    LCDC_DEC_ARGB = LCDC_DEC_RGBA8888,
    LCDC_DEC_XRGB = LCDC_DEC_RGBX8888,
};

enum hal_lcdc_tiled_type_e {
    LCDC_TILED_TYPE_NODEC = 0,
    LCDC_TILED_TYPE_TFBDC,
    LCDC_TILED_TYPE_DEC,
};

enum hal_lcdc_lossy_type_e {
    LCDC_LOSSY_TYPE_LOSSLESS = 0,
    LCDC_LOSSY_TYPE_LOSS_75,
    LCDC_LOSSY_TYPE_LOSS_50,
    LCDC_LOSSY_TYPE_LOSS_25,
};

enum hal_lcdc_tiled_format_e {
    LCDC_TILED_FMT_32BIT = 0,
    LCDC_TILED_FMT_24BIT,
    LCDC_TILED_FMT_16BIT,
};

enum hal_lcdc_alpha_mode_e {
    LCDC_CONST_ALPHA = 0,
    LCDC_PIXEL_ALPHA = 1,
    LCDC_PIXEL_PREMULTI = 2,
    LCDC_PIXEL_COVERAGE = 3,
};

enum hal_lcdc_blend_mode_e {
    LCDC_BLEND_MODE0 = 0,  /* (1-a) * dma-pix + a * gra-pix */
    LCDC_BLEND_MODE1 = 1,  /* (1-a) * dma-pix + 1 * gra-pix */
    LCDC_BLEND_MODE2 = 2,  /* a * dma-pix + 1 * gra-pix */
};

enum hal_lcdc_alpha_sel_mode_e {
    LCDC_SEL_PIXEL_ALPHA = 0,   /* src/dst alpha use src/dst pix alpha*/
    LCDC_SEL_CONST_FF = 1,      /* src/dst alpha use const 0xff */
    LCDC_SEL_CONST_00 = 2,      /* src/dst alpha use const 0x00*/
};

enum hal_lcdc_pix_sel_mode_e {
    LCDC_BOTH_SEL_SRC_PIXEL = 0,   /* both pixel use src pix */
    LCDC_BOTH_SEL_DST_PIXEL = 1,   /* both pixel use dst pix */
    LCDC_BOTH_SEL_CONST_00  = 2,   /* both pixel set to 0x00 */
    LCDC_BOTH_SEL_CONST_FF  = 3,   /* both pixel set to 0xff */
};

/*
input a_s can be LCDC_SEL_CONST_FF/LCDC_SEL_CONST_00/LCDC_SEL_PIXEL_ALPHA
input a_d can be LCDC_SEL_CONST_FF/LCDC_SEL_CONST_00/LCDC_SEL_PIXEL_ALPHA
a_src = a_s * ( 1 - a_d )
a_dst = a_d * ( 1 - a_s )
a_both = a_s * a_d
Pix_result = a_src * src_pix + a_dst * dst_pix + a_both * (src_pix || dst_pix)
Alpha_result = a_src * (0 || 1) + a_dst * (0 || 1) + a_both * (0 || 1)

Pix_result both sel src = a_s * src_pix + (1 - a_s) * a_d * dst_pix
or
Pix_result both sel dst = a_d * dst_pix + (1 - a_d) * a_s * src_pix
*/
enum hal_lcdc_porter_duff_mode_e {
    LCDC_PORTER_DUFF_MODE_SRC = 1,
    LCDC_PORTER_DUFF_MODE_ATOP,
    LCDC_PORTER_DUFF_MODE_OVER,
    LCDC_PORTER_DUFF_MODE_IN,
    LCDC_PORTER_DUFF_MODE_OUT,
    LCDC_PORTER_DUFF_MODE_DEST,
    LCDC_PORTER_DUFF_MODE_DESTATOP,
    LCDC_PORTER_DUFF_MODE_DESTOVER,
    LCDC_PORTER_DUFF_MODE_DESTIN,
    LCDC_PORTER_DUFF_MODE_DESTOUT,
    LCDC_PORTER_DUFF_MODE_CLEAR,
    LCDC_PORTER_DUFF_MODE_XOR,
};

/* plane_alpha = global_alpha = layer alpha */
enum hal_lcdc_drm_blend_mode_e {
    LCDC_DRM_BLEND_MODE_NONE = 1,       /* plane_alpha * fg.rgb + (1 - plane_alpha) * bg.rgb */
    LCDC_DRM_BLEND_MODE_PREMULTI = 2,   /* plane_alpha * fg.rgb + (1 - plane_alpha * fg.aplha) * bg.rgb */
    LCDC_DRM_BLEND_MODE_COVERAGE = 3,   /* plane_alpha * fg.aplha * fg.rgb + (1 - plane_alpha * fg.aplha) * bg.rgb */
    LCDC_DRM_BLEND_MODE_DEFAULT = 4,    /* fg.aplha * fg.rgb + (1 - fg.aplha) * bg.rgb */
};

enum hal_lcdc_burst_len_e {
    LCDC_BURST_64B = 0,
    LCDC_BURST_128B,
    LCDC_BURST_192B,
    LCDC_BURST_256B
};

enum hal_lcdc_outstanding_len_e {
    LCDC_OUTSTANDING_4 = 0,
    LCDC_OUTSTANDING_8,
    LCDC_OUTSTANDING_16,
    LCDC_OUTSTANDING_32,
    LCDC_OUTSTANDING_64,
    LCDC_OUTSTANDING_128
};

struct hal_lcdc_rgbif_config_t {
    uint16_t pix_mhz;    /* Pixel clock freq in Mhz */
    uint16_t pix_bits;   /* Bits per Pixel */

    bool clk_inv;        /* Inverse clock pulse */
    bool hsync_inv;      /* Inverse hsync pulse */
    bool vsync_inv;      /* Inverse vsync pulse */
};

/**
 * hal_lcdc_irqn - get lcdc irq  number
 */
int hal_lcdc_irqn(void);

/**
 * hal_lcdc_write_palette - write palette color table into lcdc memory
 * @index  : color index
 * @color  : color value
 */
int hal_lcdc_write_palette(int lid, uint8_t index, uint32_t color);

/**
 * hal_lcdc_palette_enable - enable/disalbe palette for layer lid
 */
int hal_lcdc_palette_enable(int lid, bool enable);

/**
 * hal_lcdc_write_gamma - write gamma lut table into lcdc memory
 * @index   : gamma index
 * @value   : gamma value
 */
int hal_lcdc_write_gamma(uint8_t index, uint8_t value);

/**
 * hal_lcdc_gamma_enable - enable/disalbe gamma curves function
 */
int hal_lcdc_gamma_enable(bool enable);

/**
 * hal_lcdc_axifastmode_enable - enable/disalbe  axi bus fast mode function
 */
int hal_lcdc_axifastmode_enable(bool enable);

int hal_lcdc_axi_outstanding(enum hal_lcdc_outstanding_len_e index);

/**
 * hal_lcdc_lstartaddr - set lcdc layer dma memory start address
 * @lid    : Layer id
 * @addr   : memory address
 */
int hal_lcdc_lstartaddr(int lid, uint32_t addr);

/**
 * hal_lcdc_lstartaddr1 - set lcdc layer dma memory start address1
 * @lid    : Layer id
 * @addr   : memory address
 */
int hal_lcdc_lstartaddr1(int lid, uint32_t addr);

/**
 * hal_lcdc_getcuraddr - get lcdc layer dma memory start address
 * @lid    : Layer id
 */
uint32_t hal_lcdc_getcuraddr(int lid);

/**
 * hal_lcdc_getcuraddr1 - get lcdc layer dma memory start address1
 * @lid    : Layer id
 */
uint32_t hal_lcdc_getcuraddr1(int lid);

/**
 * hal_lcdc_lpitch - set lcdc rgb layer pitch (stride)
 * @lid    : Layer id
 * @pitch  : pitch in bytes
 */
int hal_lcdc_lpitch(int lid, uint16_t pitch);

int hal_lcdc_lsplit_disable(int lid);

int hal_lcdc_lsplit_region(int lid, int rid, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

int hal_lcdc_lsplit_buffer(int lid, int rid, uint32_t buf, uint16_t w, uint16_t h, uint16_t pitch);

int hal_lcdc_ltfbdc_enable(int lid, bool enable, bool use_qspi, bool vmirror);

int hal_lcdc_ltfbdc_set_data_delay(int lid, uint8_t delay);

int hal_lcdc_ltfbdc_set_lossy(int lid, enum hal_lcdc_lossy_type_e lossy);

int hal_lcdc_ltfbdc_set_arqos(int lid, uint8_t qos);

int hal_lcdc_ltfbdc_size(int lid, uint16_t tile_w, uint16_t tile, uint16_t tile_num, uint16_t xtile, uint16_t xtile_num, uint16_t xtile_diif, uint8_t vmirror);

int hal_lcdc_ltfbdc_startaddr(int lid, uint32_t src_addr, uint16_t src_w, uint16_t src_h, uint16_t bpp);

int hal_lcdc_ltfbdc_set_format(int lid, enum hal_lcdc_tiled_format_e format);

int hal_lcdc_ltfbdc_swap_rb(int lid, bool enable);

/**
 * hal_lcdc_yuv_lstartaddr1 - set lcdc yuv layer Y/U/V memory start addresses
 * @lid    : Layer id, only support DMA layer
 * @yaddr  : Y buffer address
 * @uaddr  : U buffer address
 * @vaddr  : V buffer address
 */
int hal_lcdc_yuv_lstartaddr(int lid, uint32_t yaddr, uint32_t uaddr, uint32_t vaddr);

/**
 * hal_lcdc_yuv_lpitch - set lcdc yuv layer Y/U/V pitchs
 * @lid    : Layer id, only support DMA layer
 * @ypitch : Y pitch in bytes
 * @Upitch : U pitch in bytes
 * @Vpitch : V pitch in bytes
 */
int hal_lcdc_yuv_lpitch(int lid, uint16_t ypitch, uint16_t upitch, uint16_t vpitch);

int hal_lcdc_yuv_fix(int lid, uint16_t width, enum hal_lcdc_color_format_e fmt);
int hal_lcdc_yuvdsp_enable(int lid, bool enable);
int hal_lcdc_yuvdsp_contrast(int lid, uint32_t contrast);
int hal_lcdc_yuvdsp_saturation(int lid, uint32_t saturation);
int hal_lcdc_yuvdsp_hue(int lid, uint32_t hue);
int hal_lcdc_yuvdsp_colorspace(int lid, uint8_t csc);

int hal_lcdc_yuv_fast_mode(bool enable, uint16_t rds);

/**
 * hal_lcdc_lvsepxcount - set lcdc vsync start/end pix count
 * @start   : stby length
 * @end     : start line
 */
int hal_lcdc_lvsepxcount(int start, int end);

/**
 * hal_lcdc_stby_vsync_set - set stby vsync
 * @stby_len    : stby length
 * @start_line  : start line
 * @end_line    : end line
 */
int hal_lcdc_stby_vsync_set(int stby_len, int start_line, int end_line);

/**
 * hal_lcdc_porch_vsync_set - set vsync porch in pixes
 * @hf      : hfront porch
 * @hb      : hback proch
 * @vlines  : vlines
 */
int hal_lcdc_porch_vsync_set(int hfp, int hbp, int vlines);

/**
 * hal_lcdc_pix_vsync_set - set vsync position in pixes
 * @start_pix  : start pixels
 * @end_pix    : end pixels
 */
int hal_lcdc_pix_vsync_set(int start_pix, int end_pix);

/**
 * hal_lcdc_sponscreen_set - set  F/B Layer screen start position in pixes
 * @lid    : Layer id
 * @vln    : y in line
 * @hpxl   : x in pixels
 */
int hal_lcdc_sponscreen_set(int lid, uint16_t vln, uint16_t hpxl);

/**
 * hal_lcdc_bgl_vhtotal_set - set  F/B Layer PN_V/H_TOTAL in pix
 * @htotal    : height total
 * @vtotal    : width total
 */
int hal_lcdc_bgl_vhtotal_set(int htotal, int vtotal);

/**
 * hal_lcdc_bgl_vtotal_set - set  F/B Layer PN_V_TOTAL in pixes
 * @vtotal  : width total
 */
int hal_lcdc_bgl_vtotal_set(int vtotal);

/**
 * hal_lcdc_bgl_htotal_set - set  F/B Layer PN_H_TOTAL in pixes
 * @htotal    : height total
 */
int hal_lcdc_bgl_htotal_set(int htotal);

/**
 * hal_lcdc_bgl_hporch_set - set  F/B Layer HPORCH in pixes
 * @hfp     : hfront porch
 * @hbp     : hback porch
 */
int hal_lcdc_bgl_hporch_set(int hfp, int hbp);

/**
 * hal_lcdc_bgl_vporch_set - set  F/B Layer VPORCH in pixes
 *
 * Input Parameters:
 * @vfp  : vfront porch
 * @vbp  : vback porch
 */
int hal_lcdc_bgl_vporch_set(int vfp, int vbp);

/**
 * hal_lcdc_lsize - set  Layer image size in pixes
 * @lid   : LCDC_LAYER_SPU only
 * @w     : Layer width
 * @h     : Layer height
 */
int hal_lcdc_lsize(int lid, uint16_t w, uint16_t h);

/**
 * hal_lcdc_lzoom_set - set  F/B Layer zoom in pixes
 * @lid  : Layer id
 * @w    : window active area width
 * @h    : window active area height
 */
int hal_lcdc_lzoom_set(int lid, uint16_t w, uint16_t h);

/**
 * hal_lcdcl_enable - enable/disable lcdc Layer
 * @lid     : Layer index
 * @enable  : enable/disable
 */
int hal_lcdcl_enable(int lid, bool enable);

/**
 * hal_lcdc_lcolor - set  background Layer color
 * @lid    : LCDC_LAYER_SPU only
 * @rgb    : color in RGB888
 */
int hal_lcdc_lcolor(int lid, uint32_t rgb);

/**
 * hal_lcdc_dither_s4x8 - enable/disable lcdc  DITHER_4X8_PN
 * @enable   : enable/disable
 */
int hal_lcdc_dither_s4x8(bool enable);

/**
 * hal_lcdc_dither - lcdc  dither configuration
 * @enable   : enable/disable
 * @level    : 0: low  1: high
 * @red      : color width(4,5,6,5)
 * @green    : color width(4,6,6,5)
 * @blue     : color width(4,5,6,5)
 */
int hal_lcdc_dither(bool enable, uint8_t level, uint8_t red,
             uint8_t green, uint8_t blue);

/**
 * hal_lcdc_dither_read - read  lcdc  dither table memory
 * @index   : table memory index
 *
 * Returns: mem value of dither table
 */
uint32_t hal_lcdc_dither_read(uint8_t index);

/**
 * hal_lcdc_dither_write - write  lcdc  dither table memory
 * @index   : table memory index
 * @value   : write value to dither table
 */
int hal_lcdc_dither_write(uint8_t index, uint32_t value);

/**
 * hal_lcdc_lpfc_fmt - Configure layer color format
 * @lid  : Layer id
 * @fmt  : LCDC_CFMT_XXX
 */
int hal_lcdc_lpfc_fmt(int lid, uint8_t fmt);

/**
 * hal_lcdc_lchromakeyenable - Enable colorkey
 */
int hal_lcdc_lchromakeyenable(int lid, bool enable, enum hal_lcdc_colorkey_mode_e mode);

/**
 * hal_lcdc_lpfc_ltransp - Configure layer blend function
 * @lid     : Layer id
 * @mode    : Pixel alpha mode, LCDC_CONST_ALPHA or LCDC_PIXEL_ALPHA
 * @alpha   : Layer alpha
 */
int hal_lcdc_lpfc_ltransp(int lid, enum hal_lcdc_alpha_mode_e mode, uint8_t alpha);

/**
 * hal_lcdc_lchromakey - foreground and background color key Configure
 * @lid   : Layer id
 * @rgb   : chroma key
 * @rrgb  : color to replace chroma key
 * @ra    : new alpha for replaced pixes
 *
 * Set foreground and background color key Configure, real color will be
 * replaced , if it's value is equal to the chroma key
 */
int hal_lcdc_lchromakey(int lid, uint32_t rgb, uint32_t rrgb, uint8_t ra);
int hal_lcdc_lchromakey_yr(int lid, uint32_t r2r1ra);
int hal_lcdc_lchromakey_ug(int lid, uint32_t g2g1ga);
int hal_lcdc_lchromakey_vb(int lid, uint32_t b2b1ba);

/**
 * hal_lcdc_lchromakey_newalpha - foreground and background color key Configure
 * @lid    : Layer id
 * @alpha  : new alpha for replaced pixes
 *
 * set foreground and background color key Configure, real color alpha will be
 * replaced , if it's color value is equal to the chroma key
 */
int hal_lcdc_lchromakey_newalpha(int lid, uint8_t newalpha);

/**
 * hal_lcdc_panelpath_dmatrans_trigger_enable - enable/disable lcdc panel dma trigger
 * @lid    : Layer id
 * @ena    : enable/disable
 */
int hal_lcdc_panelpath_dmatrans_trigger_enable(int lid, bool enable);

/**
 * hal_lcdc_disp_avsync_trigger_disable - enable/disable lcdc disp avsync trigger
 * @dis     : enable/disable
 */
int hal_lcdc_disp_avsync_trigger_disable(bool disable);

/**
 * hal_lcdc_disp_svsync_trigger_disable - enable/disable lcdc disp svsync trigger
 * @dis   : enable/disable
 */
int hal_lcdc_disp_svsync_trigger_disable(bool disable);

/**
 * hal_lcdc_disp_mem_enable - enable/disable lcdc disp memory
 * @dis  : enable/disable
 */
int hal_lcdc_disp_mem_enable(bool enable);

/**
 * hal_lcdc_disp_extra_delay - set display extra delays
 */
int hal_lcdc_disp_extra_delay(uint16_t delay);

/**
 * hal_lcdc_ol_dmaburst_length_set - set lcdc wb path dma burst length
 * @len     :  burst length
 */
int hal_lcdc_ol_dmaburst_length_set(uint8_t len);

/**
 * hal_lcdc_smpn_mode_set - set lcdc smpn mode
 * @mode : smpn mode (0x0 = 3 cycles per pixel;
 *                    0x1 = RGB666, 3 cycles per pixel
 *                    0x2 = RGB565, 2 cycles per pixel;
 *                    0x3 = 1 cycle per pixel;
 *                    0x4 = RGB666, 1 cycle per pixel;
 *                    0x5 = RGB565 1 cycle per pixel )
 */
int hal_lcdc_smpn_mode_set(int mode);

/**
 * hal_lcdc_irq_disable - mask all lcdc_irqs
 */
int hal_lcdc_irq_disable(void);

/**
 * hal_lcdc_get_irqstat - get irq state.
 */
uint32_t hal_lcdc_get_irqstat(void);

/**
 * hal_lcd_framedone_irq_enable - enable/disable framedone irq.
 * @enable    : true or false
 */
int hal_lcdc_framedone_irq_enable(bool enable);

/**
 * hal_lcd_vsync_irq_enable - enable/disable vsync irq.
 * @enable    : true or false
 */
int hal_lcdc_vsync_irq_enable(bool enable);

/**
 * hal_lcdc_framedone_irqstat_clr  - clear framedone irq status.
 */
int hal_lcdc_framedone_irqstat_clr(void);

/**
 * hal_lcdc_vsync_irqstat_clr - clear vsync irq status.
 */
int hal_lcdc_vsync_irqstat_clr(void);

/**
 * hal_lcdc_irq_is_framedone - check get framedone irq status or not.
 * @regval   : lcdc ISR value
 */
bool hal_lcdc_irq_is_framedone(uint32_t regval);

/**
 * hal_lcdc_irq_is_hwc_framedone - check get hwc framedone irq status or not.
 * @regval   : lcdc ISR value
 */
bool hal_lcdc_irq_is_hwc_framedone(uint32_t regval);

/**
 * hal_lcdc_irq_is_framedone_irq0 - check layer framedone irq0.
 * @regval   : lcdc ISR value
 */
bool hal_lcdc_irq_is_framedone_irq0(uint32_t regval);

/**
 * hal_lcdc_irq_is_framedone_irq1 - check layer framedone irq1.
 * @regval   : lcdc ISR value
 */
bool hal_lcdc_irq_is_framedone_irq1(uint32_t regval);

/**
 * hal_lcdc_irq_is_framedone - check get vsync irq status or not.
 * @regval   : lcdc ISR value
 */
bool hal_lcdc_irq_is_vsync(uint32_t regval);

/**
 * hal_lcdc_irq_is_error - check error irq status.
 * @regval  : lcdc ISR value
 */
bool hal_lcdc_irq_is_error(uint32_t regval);

/**
 * hal_lcdc_irq_get_error - return current error irq only.
 * @regval  : lcdc ISR value
 */
uint32_t hal_lcdc_irq_get_error(uint32_t regval);

/**
 * hal_lcdc_error_irq_enable - enable all lcdc_error irqs
 */
int hal_lcdc_error_irq_enable(void);

/**
 * hal_lcdc_error_irqstat_clr - enable  all lcdc_error irqs
 */
int hal_lcdc_error_irqstat_clr(void);

/**
 * hal_cmu_disp_pn_reset   - reset display pannel
 */
int hal_lcdc_disp_pn_reset(void);

/**
 * hal_lcdc_smpn_enable - lcdc smpn module enable.
 */
int hal_lcdc_smpn_enable(void);

/**
 * hal_lcdc_smpn_vsync_delaymode_set - set lcdc smpn module vsync delay.
 */
int hal_lcdc_smpn_vsync_delaymode_set(uint8_t delay);

/**
 * hal_lcdc_smpn_vsync_delaymode_get - get lcdc smpn module vsync delay.
 */
uint32_t hal_lcdc_smpn_vsync_delaymode_get(void);

/**
 * hal_lcdc_smpn_enable - lcdc smpn module enable/disable.
 */
int hal_lcdc_gen_frame_enable(bool enable);

/**
 * hal_lcdc_set_sync_line_pixes - set lcdc sync pixs.
 */
int hal_lcdc_set_sync_line_pixs(uint32_t pixs);

/**
 * hal_lcdc_set_sync_lines  - set lcdc sync lines.
 */
int hal_lcdc_set_sync_lines(uint32_t lines);

/**
 * hal_lcdc_get_sync_delay_pixs - set lcdc delay pixes.
 */
uint32_t hal_lcdc_get_sync_delay_pixs(void);

/**
 * hal_lcdc_get_sync_delay_lines - get lcdc delay lines.
 */
uint32_t hal_lcdc_get_sync_delay_lines(void);

/**
 * hal_lcdc_start_wb  - Starts the wblayer dma2d transfer process
 */
int hal_lcdc_start_wb(void);

/**
 * hal_lcdc_gen_start  - Starts the  dma2d transfer process by write register
 */
int hal_lcdc_gen_start(void);

int hal_lcdc_frame_halt(void);

int hal_lcdc_switch_start(void);

bool hal_lcdc_is_switch_started(void);

/**
 * hal_lcdc_start_dsi - Starts the lcdc transfer process.
 */
int hal_lcdc_start_dsi(void);

/**
 * hal_lcdc_te_trigger - Enable TE trigger.
 */
int hal_lcdc_te_trigger(void);

/**
 * hal_lcdc_vsync_porch_set  - set vsync porch with default value
 */
int hal_lcdc_vsync_porch_set(void);

/**
 * hal_lcdc_wdma_1fpt_set - set write back dma in one frame per trigger mode
 */
int hal_lcdc_wdma_1fpt_set(void);

/**
 * hal_lcdc_wdma_toggle_frame - toggle signal to start a write back dma frame
 */
int hal_lcdc_wdma_toggle_frame(void);

/**
 * hal_lcdc_select_top - select f/b dma path  as top layer to blender
 */
int hal_lcdc_select_top(uint8_t lid);

/**
 * hal_lcdc_blend - set blend and alpha mode for pn layers.
 */
int hal_lcdc_blend(int blend_mode, int alpha_mode, uint8_t alpha);
int hal_lcdc_zorder_set(uint8_t bus_port, uint8_t zorder);
int hal_lcdc_layer_alpha(uint8_t bus_port, uint8_t lid, enum hal_lcdc_alpha_mode_e mode, uint8_t alpha);
int hal_lcdc_blend_drm(uint8_t bus_port, uint8_t lid, enum hal_lcdc_drm_blend_mode_e drm_blend_mode, uint8_t alpha);
int hal_lcdc_blend_nocompose(uint8_t bus_port, uint8_t lid);

/**
 * hal_lcdc_swap_path - select f/b dma path
 */
int hal_lcdc_swap_path(uint8_t lid);

/**
 * hal_lcdc_swap_path_enable  - enable selected f/b dma path
 */
int hal_lcdc_swap_path_enable(bool enable);

/**
 * hal_lcdc_set_vmirror_enable - enable/disable selected layer vertical mirror function
 * @lid : LCDC_LAYER_GRA/DMA/TVG
 */
int hal_lcdc_set_vmirror_enable(uint8_t lid, bool enable);

/**
 * hal_lcdc_set_hmirror_enable - enable/disable horizontal mirror for all layers.
 */
int hal_lcdc_set_hmirror_enable(bool enable, int width, int height);

/**
 * hal_lcdc_disp_color_fmt - set color ouput format for display interface
 * @fmt  : ctrl bit:
 *         DSI_RGB888                (28)
 *         DSI_RGB666                (29)
 *         DSI_RGB565                (30)
 *         DSI_RGB101010             (31)
 */
int hal_lcdc_disp_color_fmt(enum hal_lcdc_dsi_cfmt_e fmt);

/**
 * hal_lcdc_disp_dsi_data_swap - set dsi output data byte-order
 * @order  : ctrl fmt
 *            DSI_BD_BGR : 0
 *            DSI_BD_BRG : 1
 *            DSI_BD_GBR : 2
 *            DSI_BD_RBG : 3
 *            DSI_BD_GRB : 4
 *            DSI_BD_RGB : 5
 */
int hal_lcdc_disp_dsi_data_swap(enum hal_lcdc_dsi_dswap_e order);

/**
 * hal_lcdc_sl_dmaburst_length_set - set lcdc source path dma read burst length
 * @order  : burst fmt
 *            LCDC_BURST_64B : 0
 *            LCDC_BURST_128B : 1
 *            LCDC_BURST_192B : 2
 *            LCDC_BURST_256B : 3
 */
int hal_lcdc_sl_dmaburst_length_set(enum hal_lcdc_burst_len_e burst);

/**
 * hal_lcdc_sleep - lcdc enter sleep mode.
 */
int hal_lcdc_sleep(void);

/**
 * hal_lcdc_wakeup - lcdc wakeup from sleep mode.
 */
int hal_lcdc_wakeup(void);

/**
 * hal_lcdc_video_mode_enable - enable lcdc setting for DSI video mode.
 * call it in front of other lcdc init functions.
 */
int hal_lcdc_video_mode_enable(bool enable);

/**
 * hal_lcdcl_dumb_enable - enable video frame transfer.
 */
int hal_lcdcl_dumb_enable(bool enable);

/**
 * hal_lcdc_dumb_vsync_mode - enable VSYNC sigal on dumb panel.
 */
int hal_lcdc_dumb_vsync_mode(uint8_t mode);

/**
 * hal_lcdc_source_switch - switch the dumb panel as source
 */
int hal_lcdc_source_switch(bool enable);

/**
 * hal_lcdc_dec_set_size - config decnano buffer width & height
 */
int hal_lcdc_dec_set_size(uint16_t w, uint16_t h);

/**
 * hal_lcdc_dec_set_color_format - set decnano color format
 */
int hal_lcdc_dec_set_color_format(enum hal_lcdc_dec_format_e cfmt);

/**
 * hal_lcdc_dec_set_tile_mode - set decnano tile mode
 */
int hal_lcdc_dec_set_tile_mode(enum hal_lcdc_dec_tile_mode_e mode);

/**
 * hal_lcdc_dec_set_compress_mode - set decnano tile mode
 */
int hal_lcdc_dec_set_compress_mode(enum hal_lcdc_dec_compress_mode_e comp);

/**
 * hal_lcdc_dec_set_data_delay - set decnano data delay
 */
int hal_lcdc_dec_set_data_delay(uint8_t delay);

/**
 * hal_lcdc_yuv2rgb_enable - enable YUV to RGB convert
 */
int hal_lcdc_yuv2rgb_enable(int lid, bool enable, bool swap_yu, bool swap_uv);

/**
 * hal_lcdc_dec_swap_rb - swap decnano red & blue
 */
int hal_lcdc_dec_swap_rb(bool enable);

/**
 * hal_lcdc_tiled_set_size - config tiled buffer width & height
 */
int hal_lcdc_tiled_set_size(uint16_t w, uint16_t h);

/**
 * hal_lcdc_tiled_set_format - set tiled layer format type
 */
int hal_lcdc_tiled_set_format(enum hal_lcdc_tiled_format_e format);

/**
 * hal_lcdc_tiled_swap_rb - swap tiled layer red & blue
 */
int hal_lcdc_tiled_swap_rb(bool enable);

/**
 * hal_lcdc_test_mode - enable test mode for gra layer.
 */
int hal_lcdc_test_mode(bool enable);

/**
 * hal_lcdc_dsi0_layer_select - select layers for DSI0
 */
int hal_lcdc_dsi0_layer_select(uint8_t layermask);

/**
 * hal_lcdc_dsi0_blend - configure dsi0 blend and alpha mode.
 */
int hal_lcdc_dsi0_blend(int blend_mode, int alpha_mode, uint8_t alpha);

/**
 * hal_lcdc_dsi1_layer_select - select layers for DSI1
 */
int hal_lcdc_dsi1_layer_select(uint8_t layermask);

/**
 * hal_lcdc_dsi1_blend - configure dsi1 blend and alpha mode.
 */
int hal_lcdc_dsi1_blend(int blend_mode, int alpha_mode, uint8_t alpha);

/**
 * @brief hal_lcdc_rgbif_init - initialize RGB interface sub-module.
 *
 * @param cfg: hal_lcdc_rgbif_config_t pointer.
 */
int hal_lcdc_rgbif_init(struct hal_lcdc_rgbif_config_t *cfg);

/**
 * @brief hal_lcdc_rgbif_exit - exit RGB interface sub-module, disable lcdc/rgbif clock.
 */
void hal_lcdc_rgbif_exit(void);

/**
 * @brief hal_lcdc_rgbif_swaprb_enable - Swap red & blue order.
 */
void hal_lcdc_rgbif_swaprb_enable(bool enable);

/**
 * hal_lcdc_dump_regs - dump lcdc all registers.
 */
int hal_lcdc_dump_regs(void);

int hal_lcdc_dump_irqerror(uint32_t irqstat);

/**
 * hal_lcdc_ftoggle_enable - enable/disable lcdc ftoggle
 * @lid     : Layer index
 * @enable  : enable/disable
 */
int hal_lcdc_ftoggle_enable(int lid, bool enable);

/**
 * hal_lcdc_rotate_enable - enable/disable layer rotation.
 * @lid     : layer id, only support GRA layer for now
 * @enable  : enable/disable
 * @degree  : rotation degrees, 90 or 270
 * @width   : layer orignal width
 * @height  : layer orignal height
 */
int hal_lcdc_rotate_enable(int lid, bool enable, int degree, int w, int h, bool tiled);

/**
 * hal_lcdc_scaler_enable - enable/disable scaler module
 * @enable : enable or disable
 * @in_w   : source image width
 * @in_h   : source image height
 * @out_w  : output image width
 * @out_h  : output image height
 */
int hal_lcdc_scaler_enable(bool enable, uint16_t in_w, uint16_t in_h,
                           uint16_t out_w, uint16_t out_h, bool qspi_bus);

int hal_lcdc_scaler2_enable(bool enable, uint16_t in_w, uint16_t in_h,
                           uint16_t out_w, uint16_t out_h, bool qspi_bus);

void hal_lcdc_aff_watermark_config(int watermark);
void hal_lcdc_arqos_ctrl_config(int value);
#ifdef __cplusplus
}
#endif
#endif /* __HAL_LCDC_H */
