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
/**
 * DOC: The Display Controller operates in Slave mode and DMA mode.
 *
 * Slave mode is for the processor to read/write Smart Panel, and DMA mode is for
 * the Display Controller to move pixels to either the Smart Panel or the Dumb Panels.
 * The <var Product Number> support one panel path and one TV path, for each path,
 * besides background color, there are total of three objects that can be overlaid
 * onto the display screen, video,graphicc, and hardware cursor. When one panel
 * path or TV path is disabled,the three objects can be overlaid on the remaining
 * panel or TV. When all dumb panels are working in DMA mode, smart panel can be
 * accessed through an AHB slave path concurrently. So, a total of three panel
 * interfacesare supported, with two operating in DMA mode and one operating in
 * slave mode. In the following context, the panel path is called panel path, and
 * those signals are mostly called pn_*. Smart panel signals are mostly called
 * *isa* or *smpn*.
 *
 * The following operations will be done during DMA transfer:
 * Color Space Conversion (CSC) on two video paths, supporting SDTV and HDTV, studio
 * and computer, and total of four CSC matrixes.
 * Video enhancement including contrast/saturation/brightness/hue adjust on the
 * two video paths.
 * palette color lookup on the two graphic paths, supporting both 4-bit/pixel and
 *  8-bit/pixel.
 * alpha blending in overlaid area for all paths, with configured alpha, alpha
 * combined with pixel and color key alpha.
 * partial display DMA when part of the picture is covered by another picture,
 * reducing bandwidth (refer to register bit descriptions for more information).
 * multiple DMA requests concatenated together to increase DDR page hit rate.
 * line buffer mode DMA to increase DDR page hit rate and reduce internal SRAM
 * occupation.
 * RGB gamma correction after alpha blending, up to two interfaces.
 * horizontal and vertical scaling and filtering on all of the four video and
 * graphic paths.
 * hardware cursor overlay, each cursor size up to 8192 pixels for 1 bit per pixel.
 * color modification for either Panel path or TV path to enhance the quality
 * of picture colors.
 * selec vertical mirror on all of the four video and graphic objects.
 * dither function enable for either Panel path or TV path.
 * The base addresses of the Display Controller registers is: 0xD420_B000.

 * LCDC hardware blocks:
 *
 *  (FGL source color fmt:RGB565,RGB1555,RGB888packed, RGB888unpacked, ARGB,YUV422packed,
 *   YUV422 planar, YUV420 planar, Palette color 4-bit per pixel, Palette color
 *   8-bit per pixel, RGBA )
 * (BGL source color fmt:RGB565,RGB1555,RGB888 packed,RGB888 unpacked,ARGB,YUV422
 *  packed,Palette color 4-bit per pixel,Palette color 8-bit per pixel,RGB8A )
 *  (OL color fmt:RGB565 RGB888 ARGB888 RGBA888)
 *
 *  |FG Layer(panel video dma layer) |-->|
 *                                       | Alpha_FG (pix/c)
 *                                       V
 *                                      |BLENDING|-->|Smart Panel|-->->|dsi|
 *                                       ^     Alpha_O (pix/c)       |
 *                                       | Alpha_BG (pix/c)          |
 *  |BG Layer(panel graphic dma layer)|->|             |             |
 *                                                     |             |
 *  |TV Path(not used currently by lcdc driver)|-------V             |
 *                                                                   |
 *                                                                   |
 *  |write back layer)|<----------------------------------------------
 */
#include "cmsis.h"
#include "plat_addr_map.h"

#ifdef LCDC_BASE

#include "string.h"
#include "hal_cmu.h"
#include "hal_timer.h"
#include "hal_lcdc.h"
#include "hal_trace.h"
#include "reg_lcdc.h"
#include "hal_psc.h"

//#define LCDC_DEBUG
#ifdef  LCDC_DEBUG
#define LCDC_DBG(...)                   TR_INFO( TR_MOD(HAL), "//LCDC: " __VA_ARGS__)
#else
#define LCDC_DBG(...)
#endif

#define LCDC_INFO(...)                  TR_INFO( TR_MOD(HAL), "//LCDC: " __VA_ARGS__)

#define COMMON_LCDC_FRAME_0_IRQS        (LCDC_GRA_FRAME_IRQ0 | LCDC_DMA_FRAME_IRQ0 | \
                                        LCDC_TVG_FRAME_IRQ0 | LCDC_WB_FRAMEDONE)

#define COMMON_LCDC_FRAME_1_IRQS        (LCDC_GRA_FRAME_IRQ1 | LCDC_DMA_FRAME_IRQ1 | \
                                        LCDC_TVG_FRAME_IRQ1)

#define COMMON_LCDC_ERROR_IRQS          (LCDC_GRA_FF_UNDERFLOW | LCDC_DMA_FF_UNDERFLOW | \
                                        LCDC_TVG_FF_UNDERFLOW | LCDC_WB_FF_UNDERRUN | \
                                        LCDC_WB_FF_OVERRUN | LCDC_ERR_IRQ)

#if defined(CHIP_BEST1701) && defined(CHIP_SUBSYS_CASYS)
#define LCDC_FRAME_0_IRQS               (COMMON_LCDC_FRAME_0_IRQS | LCDC_TVD_FRAME_IRQ0 | \
                                        LCDC_SMT_FRAME_IRQ0)

#define LCDC_FRAME_1_IRQS               (COMMON_LCDC_FRAME_1_IRQS | LCDC_TVD_FRAME_IRQ1 | \
                                        LCDC_SMT_FRAME_IRQ1)

#define LCDC_ERROR_IRQS                 (COMMON_LCDC_ERROR_IRQS | LCDC_TVD_FF_UNDERFLOW |\
                                        LCDC_TFBC_FF_UNDERFLOW | LCDC_TFBC_CRC_MISMATCH | \
                                        LCDC_SMT_FF_UNDERFLOW | LCDC_DEC_ERR_IRQ)

#elif defined(CHIP_BEST2009)
#define LCDC_FRAME_0_IRQS               (COMMON_LCDC_FRAME_0_IRQS)

#define LCDC_FRAME_1_IRQS               (COMMON_LCDC_FRAME_1_IRQS)

#define LCDC_ERROR_IRQS                 (COMMON_LCDC_ERROR_IRQS | LCDC_FRAME_HALT_FLAG)
#else

#define LCDC_FRAME_0_IRQS               (COMMON_LCDC_FRAME_0_IRQS)

#define LCDC_FRAME_1_IRQS               (COMMON_LCDC_FRAME_1_IRQS)

#define LCDC_ERROR_IRQS                 (COMMON_LCDC_ERROR_IRQS)
#endif

#define LCDC_LAYER_FRAME_IRQS           (LCDC_FRAME_0_IRQS | LCDC_FRAME_1_IRQS)

#ifdef FPGA
#define LCDC_VSYNC_STBY_MIN             0x100  // stby > hfp + hsync + hbp
#else
#define LCDC_VSYNC_STBY_MIN             0x48   // stby > hfp + hsync + hbp
#endif

static struct LCDC_REG_T * const lcdc = (struct LCDC_REG_T *)LCDC_BASE;

static bool lcdc_video_mode = false;

static bool alpha_blend_use_new_alg = false;

static bool is_lcdc_swtich_startd = false;

int hal_lcdc_video_mode_enable(bool enable)
{
    lcdc_video_mode = enable;
    return LCDC_OK;
}

int hal_lcdc_irqn(void)
{
    return DISPLAY_IRQn;
}

int hal_lcdc_write_palette(int lid, uint8_t index, uint32_t color)
{
    uint8_t type = 3;
    if (lid >= LCDC_LAYER_TVG)
        type = 7;

    lcdc->REG_19C = LCDC_SRAM_DATA_COLOR(color);
    lcdc->REG_198 = LCDC_SRAM_ADDR(index) | LCDC_SRAM_TYPE(type) | LCDC_SRAM_CTRL(2);
    LCDC_DBG("%s: index %d, color 0x%x", __func__, index, color);
    return LCDC_OK;
}

int hal_lcdc_palette_enable(int lid, bool enable)
{
    if (lid  < LCDC_LAYER_TVG) {
        if (enable)
            lcdc->REG_260 |= LCDC_PN_PALETTE_ENA;
        else
            lcdc->REG_260 &= ~LCDC_PN_PALETTE_ENA;
    }
    else if (lid  < LCDC_LAYER_SMT) {
        if (enable)
            lcdc->REG_080 |= LCDC_TV_PALETTE_ENA;
        else
            lcdc->REG_080 &= ~LCDC_TV_PALETTE_ENA;
    }
    else {
        LCDC_INFO("%s: invalid layer id %d", __func__, lid);
        return LCDC_INVAL;
    }
    LCDC_DBG("%s: %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_write_gamma(uint8_t index, uint8_t value)
{
    lcdc->REG_19C = LCDC_SRAM_DATA_GAMMA(value);

    /* PN Gamma R channel */
    lcdc->REG_198 = LCDC_SRAM_ADDR(index) | LCDC_SRAM_TYPE(0) | LCDC_SRAM_CTRL(2);

    /* PN Gamma G channel */
    lcdc->REG_198 &= ~LCDC_SRAM_TYPE_MASK;
    lcdc->REG_198 |= LCDC_SRAM_TYPE(1);

    /* PN Gamma B channel */
    lcdc->REG_198 &= ~LCDC_SRAM_TYPE_MASK;
    lcdc->REG_198 |= LCDC_SRAM_TYPE(2);

    LCDC_DBG("%s: index %x, value 0x%x", __func__, index, value);
    return LCDC_OK;
}

int hal_lcdc_gamma_enable(bool enable)
{
    if (enable) {
        lcdc->REG_260 |= LCDC_PN_GAMMA_ENA;
        lcdc->REG_080 |= LCDC_TV_GAMMA_ENA;
    } else {
        lcdc->REG_260 &= ~LCDC_PN_GAMMA_ENA;
        lcdc->REG_080 &= ~LCDC_TV_GAMMA_ENA;
    }
    LCDC_DBG("%s: %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_axifastmode_enable(bool enable)
{
#ifdef LCDC_HAS_AXI
    /* valid for AXI bus */
    if (enable)
        lcdc->REG_260 |= LCDC_ARBFAST_ENA;
    else
        lcdc->REG_260 &= ~LCDC_ARBFAST_ENA;
#endif
    return LCDC_OK;
}

int hal_lcdc_axi_outstanding(enum hal_lcdc_outstanding_len_e index)
{
#if defined(CHIP_BEST1701)
    lcdc->REG_1BC = SET_BITFIELD(lcdc->REG_1BC, LCDC_AXI_CTRL, index);
#endif
    return LCDC_OK;
}

int hal_lcdc_lstartaddr(int lid, uint32_t addr)
{
    addr = ADDR_CPU_TO_DEV(addr);
    switch (lid) {
        case LCDC_LAYER_GRA:
            lcdc->REG_0F4 = LCDC_GRA_SA0(addr);
            break;
        case LCDC_LAYER_DMA:
            lcdc->REG_0C0 = LCDC_DMA_SA_Y0(addr);
            break;
        case LCDC_LAYER_TVG:
            lcdc->REG_034 = LCDC_TVG_SA0(addr);
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_000 = LCDC_TVD_SA_Y0(addr);
            break;
        case LCDC_LAYER_SMT:
            lcdc->REG_648 = LCDC_SMT_SA(addr);
            lcdc->REG_618 = LCDC_SMT_SA0(addr);
            break;
        case LCDC_LAYER_DEC:
            lcdc->REG_2D8 = LCDC_DEC_SA0(addr);
            break;
        case LCDC_LAYER_TILED:
            lcdc->REG_334 = LCDC_TILED_SA0(addr);
            break;
        case LCDC_LAYER_WB:
            lcdc->REG_208 = LCDC_WDMA_BASE_ADDR(addr);
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d address: 0x%08x", __func__, lid, addr);
    return LCDC_OK;
}

int hal_lcdc_lstartaddr1(int lid, uint32_t addr)
{
    addr = ADDR_CPU_TO_DEV(addr);
    switch (lid) {
        case LCDC_LAYER_GRA:
            lcdc->REG_0F8 = LCDC_GRA_SA1(addr);
            break;
        case LCDC_LAYER_DMA:
            lcdc->REG_0D0 = LCDC_DMA_SA_Y1(addr);
            break;
        case LCDC_LAYER_TVG:
            lcdc->REG_038 = LCDC_TVG_SA1(addr);
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_010 = LCDC_TVD_SA_Y1(addr);
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d address: 0x%08x", __func__, lid, addr);
    return LCDC_OK;
}

uint32_t hal_lcdc_getcuraddr(int lid)
{
    uint32_t addr = 0;
    switch (lid) {
        case LCDC_LAYER_GRA:
            addr = GET_BITFIELD(lcdc->REG_0F4, LCDC_GRA_SA0);
            break;
        case LCDC_LAYER_DMA:
            addr = GET_BITFIELD(lcdc->REG_0C0, LCDC_DMA_SA_Y0);
            break;
        case LCDC_LAYER_TVG:
            addr = GET_BITFIELD(lcdc->REG_034, LCDC_TVG_SA0);
            break;
        case LCDC_LAYER_TVD:
            addr = GET_BITFIELD(lcdc->REG_000, LCDC_TVD_SA_Y0);
            break;
        case LCDC_LAYER_SMT:
            addr = GET_BITFIELD(lcdc->REG_618, LCDC_SMT_SA0);
            break;
        case LCDC_LAYER_DEC:
            addr = GET_BITFIELD(lcdc->REG_2D8, LCDC_DEC_SA0);
            break;
        case LCDC_LAYER_TILED:
            addr = GET_BITFIELD(lcdc->REG_334, LCDC_TILED_SA0);
            break;
        case LCDC_LAYER_WB:
            addr = GET_BITFIELD(lcdc->REG_208, LCDC_WDMA_BASE_ADDR);
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return 0;
    }
    addr = ADDR_DEV_TO_CPU(addr);
    LCDC_DBG("%s: layer %d address: 0x%08x", __func__, lid, addr);
    return addr;
}

uint32_t hal_lcdc_getcuraddr1(int lid)
{
    uint32_t addr = 0;
    switch (lid) {
        case LCDC_LAYER_GRA:
            addr = GET_BITFIELD(lcdc->REG_0F8, LCDC_GRA_SA1);
            break;
        case LCDC_LAYER_DMA:
            addr = GET_BITFIELD(lcdc->REG_0D0, LCDC_DMA_SA_Y1);
            break;
        case LCDC_LAYER_TVG:
            addr = GET_BITFIELD(lcdc->REG_038, LCDC_TVG_SA1);
            break;
        case LCDC_LAYER_TVD:
            addr = GET_BITFIELD(lcdc->REG_010, LCDC_TVD_SA_Y1);
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return 0;
    }
    addr = ADDR_DEV_TO_CPU(addr);
    LCDC_DBG("%s: layer %d address: 0x%08x", __func__, lid, addr);
    return addr;
}

int hal_lcdc_lpitch(int lid, uint16_t pitch)
{
    switch (lid) {
        case LCDC_LAYER_GRA:
            lcdc->REG_0FC = SET_BITFIELD(lcdc->REG_0FC, LCDC_GRA_PITCH, pitch);
            break;
        case LCDC_LAYER_DMA:
            lcdc->REG_0E0 = SET_BITFIELD(lcdc->REG_0E0, LCDC_DMA_PITCH_Y, pitch);
            break;
        case LCDC_LAYER_TVG:
        case LCDC_LAYER_DEC:
        case LCDC_LAYER_TILED:
            lcdc->REG_03C = SET_BITFIELD(lcdc->REG_03C, LCDC_TVG_PITCH, pitch);
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_020 = SET_BITFIELD(lcdc->REG_020, LCDC_TVD_PITCH_Y, pitch);
            break;
        case LCDC_LAYER_SMT:
            lcdc->REG_64C = SET_BITFIELD(lcdc->REG_64C, LCDC_SMT_PITCH, pitch);
            lcdc->REG_640 = SET_BITFIELD(lcdc->REG_640, LCDC_SMT_PITCH_SP0, pitch);
            break;
        case LCDC_LAYER_WB:
            lcdc->REG_200 = SET_BITFIELD(lcdc->REG_200, LCDC_WDMA_IMG_PITCH, pitch);
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d pitch: %d", __func__, lid, pitch);
    return LCDC_OK;
}

int hal_lcdc_lsplit_disable(int lid)
{
    if (lid == LCDC_LAYER_SMT) {
        hal_lcdc_lsplit_region(lid, 1, 0, 0, 0, 0);
        hal_lcdc_lsplit_region(lid, 2, 0, 0, 0, 0);
        hal_lcdc_lsplit_region(lid, 3, 0, 0, 0, 0);
        hal_lcdc_lsplit_buffer(lid, 1, 0, 0, 0, 0);
        hal_lcdc_lsplit_buffer(lid, 2, 0, 0, 0, 0);
        hal_lcdc_lsplit_buffer(lid, 3, 0, 0, 0, 0);
    }
    else {
        LCDC_INFO("%s: invalid layer id %d", __func__, lid);
        return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d", __func__, lid);
    return LCDC_OK;
}

int hal_lcdc_lsplit_region(int lid, int rid, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if (lid == LCDC_LAYER_SMT) {
        if (rid == 3) {
            lcdc->REG_610 = SET_BITFIELD(lcdc->REG_610, LCDC_SMT_OVSA_HPXL_SP3, x);
            lcdc->REG_610 = SET_BITFIELD(lcdc->REG_610, LCDC_SMT_OVSA_VLN_SP3, y);
            lcdc->REG_614 = SET_BITFIELD(lcdc->REG_614, LCDC_SMT_OVLEN_HPXL_SP3, w);
            lcdc->REG_614 = SET_BITFIELD(lcdc->REG_614, LCDC_SMT_OVLEN_VLN_SP3, h);
        } else if (rid == 2) {
            lcdc->REG_608 = SET_BITFIELD(lcdc->REG_608, LCDC_SMT_OVSA_HPXL_SP2, x);
            lcdc->REG_608 = SET_BITFIELD(lcdc->REG_608, LCDC_SMT_OVSA_VLN_SP2, y);
            lcdc->REG_60C = SET_BITFIELD(lcdc->REG_60C, LCDC_SMT_OVLEN_HPXL_SP2, w);
            lcdc->REG_60C = SET_BITFIELD(lcdc->REG_60C, LCDC_SMT_OVLEN_VLN_SP2, h);
        } else if (rid == 1) {
            lcdc->REG_600 = SET_BITFIELD(lcdc->REG_600, LCDC_SMT_OVSA_HPXL_SP1, x);
            lcdc->REG_600 = SET_BITFIELD(lcdc->REG_600, LCDC_SMT_OVSA_VLN_SP1, y);
            lcdc->REG_604 = SET_BITFIELD(lcdc->REG_604, LCDC_SMT_OVLEN_HPXL_SP1, w);
            lcdc->REG_604 = SET_BITFIELD(lcdc->REG_604, LCDC_SMT_OVLEN_VLN_SP1, h);
        } else if (rid == 0) {
            lcdc->REG_5F8 = SET_BITFIELD(lcdc->REG_5F8, LCDC_SMT_OVSA_HPXL_SP0, x);
            lcdc->REG_5F8 = SET_BITFIELD(lcdc->REG_5F8, LCDC_SMT_OVSA_VLN_SP0, y);
            lcdc->REG_5FC = SET_BITFIELD(lcdc->REG_5FC, LCDC_SMT_OVLEN_HPXL_SP0, w);
            lcdc->REG_5FC = SET_BITFIELD(lcdc->REG_5FC, LCDC_SMT_OVLEN_VLN_SP0, h);
        }
        else {
            LCDC_INFO("%s: layer:%d invalid region id %d", __func__, lid, rid);
            return LCDC_INVAL;
        }
    }
    else {
        LCDC_INFO("%s: invalid layer id %d", __func__, lid);
        return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d region %d: (%d,%d,%d,%d)", __func__, lid, rid, x, y, w, h);
    return LCDC_OK;
}

int hal_lcdc_lsplit_buffer(int lid, int rid, uint32_t buf, uint16_t w, uint16_t h, uint16_t pitch)
{
    if (lid == LCDC_LAYER_SMT) {
        if (rid == 3) {
            lcdc->REG_624 = LCDC_SMT_SA3(buf);
            lcdc->REG_634 = SET_BITFIELD(lcdc->REG_634, LCDC_SMT_H_PIXEL_SA3, w);
            lcdc->REG_634 = SET_BITFIELD(lcdc->REG_634, LCDC_SMT_V_LINE_SA3, h);
            lcdc->REG_644 = SET_BITFIELD(lcdc->REG_644, LCDC_SMT_PITCH_SP3, pitch);
        } else if (rid == 2) {
            lcdc->REG_620 = LCDC_SMT_SA2(buf);
            lcdc->REG_630 = SET_BITFIELD(lcdc->REG_630, LCDC_SMT_H_PIXEL_SA2, w);
            lcdc->REG_630 = SET_BITFIELD(lcdc->REG_630, LCDC_SMT_V_LINE_SA2, h);
            lcdc->REG_644 = SET_BITFIELD(lcdc->REG_644, LCDC_SMT_PITCH_SP2, pitch);
        } else if (rid == 1) {
            lcdc->REG_61C = LCDC_SMT_SA1(buf);
            lcdc->REG_62C = SET_BITFIELD(lcdc->REG_62C, LCDC_SMT_H_PIXEL_SA1, w);
            lcdc->REG_62C = SET_BITFIELD(lcdc->REG_62C, LCDC_SMT_V_LINE_SA1, h);
            lcdc->REG_640 = SET_BITFIELD(lcdc->REG_640, LCDC_SMT_PITCH_SP1, pitch);
        } else if (rid == 0) {
            lcdc->REG_618 = LCDC_SMT_SA0(buf);
            lcdc->REG_628 = SET_BITFIELD(lcdc->REG_628, LCDC_SMT_H_PIXEL_SA0, w);
            lcdc->REG_628 = SET_BITFIELD(lcdc->REG_628, LCDC_SMT_V_LINE_SA0, h);
            lcdc->REG_640 = SET_BITFIELD(lcdc->REG_640, LCDC_SMT_PITCH_SP0, pitch);
        }
        else {
            LCDC_INFO("%s: layer:%d invalid region id %d", __func__, lid, rid);
            return LCDC_INVAL;
        }
    }
    else {
        LCDC_INFO("%s: invalid layer id %d", __func__, lid);
        return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d region %d: buf 0x%x, pitch:%d", __func__, lid, rid, buf, pitch);
    return LCDC_OK;
}

int hal_lcdc_ltfbdc_enable(int lid, bool enable, bool use_qspi, bool vmirror)
{
    if (lid == LCDC_LAYER_GRA) {
        lcdc->REG_690 &= ~LCDC_TFBDC_ENA;
        lcdc->REG_694 &= ~(LCDC_TFBDC_USE_QSPI | LCDC_TFBDC_V_MIRROR | LCDC_TFBDC_UNMSK_VSYNC_INACLK | LCDC_TFBDC_UNMSK_TRIG_INACLK);

        if (enable) {
            lcdc->REG_690 |= LCDC_TFBDC_ENA;
            lcdc->REG_694 |= LCDC_TFBDC_UNMSK_VSYNC_INACLK;
        }
        if (use_qspi) {
            lcdc->REG_694 |= LCDC_TFBDC_USE_QSPI;
        }
        if (vmirror) {
            lcdc->REG_694 |= LCDC_TFBDC_V_MIRROR;
        }
    }
    else {
        LCDC_INFO("%s: invalid layer id %d", __func__, lid);
        return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d enable:%d use_qspi:%d", __func__, lid, enable, use_qspi);
    return LCDC_OK;
}

int hal_lcdc_ltfbdc_set_data_delay(int lid, uint8_t delay)
{
    if (lid == LCDC_LAYER_GRA) {
        lcdc->REG_694 = SET_BITFIELD(lcdc->REG_694, LCDC_TFBDC_DATA_DELAY, delay);
    }
    else {
        LCDC_INFO("%s: invalid layer id %d, delay:0x%x", __func__, lid, delay);
        return LCDC_INVAL;
    }
    LCDC_DBG("%s: tfbdc data delay %d", __func__, delay);
    return LCDC_OK;
}

int hal_lcdc_ltfbdc_set_lossy(int lid, enum hal_lcdc_lossy_type_e lossy)
{
    if (lid == LCDC_LAYER_GRA) {
        lcdc->REG_694 = SET_BITFIELD(lcdc->REG_694, LCDC_TFBDC_REQ_LOSSY, lossy);
    }
    else {
        LCDC_INFO("%s: invalid layer id %d, lossy:0x%x", __func__, lid, lossy);
        return LCDC_INVAL;
    }
    LCDC_DBG("%s: tfbdc lossy %d", __func__, lossy);
    return LCDC_OK;
}

int hal_lcdc_ltfbdc_set_arqos(int lid, uint8_t qos)
{
    if (lid == LCDC_LAYER_GRA) {
        lcdc->REG_694 = SET_BITFIELD(lcdc->REG_694, LCDC_TFBDC_ARQOS, qos);
    }
    else {
        LCDC_INFO("%s: invalid layer id %d, qos:0x%x", __func__, lid, qos);
        return LCDC_INVAL;
    }
    LCDC_DBG("%s: tfbdc qos %d", __func__, qos);
    return LCDC_OK;
}

int hal_lcdc_ltfbdc_size(int lid, uint16_t tile_w, uint16_t tile, uint16_t tile_num, uint16_t xtile, uint16_t xtile_num, uint16_t xtile_diif, uint8_t vmirror)
{
    uint16_t w = tile_w * xtile_num;
    if ((lid == LCDC_LAYER_GRA) && (w <= 0x7fff)) {
        lcdc->REG_690 = SET_BITFIELD(lcdc->REG_690, LCDC_TFBDC_ACTIVE_H_WIDTH_L, w & 0x3ff);
        lcdc->REG_694 &= ~LCDC_TFBDC_ACTIVE_H_WIDTH_H;
        if (w > 0x3ff)
            lcdc->REG_694 |= LCDC_TFBDC_ACTIVE_H_WIDTH_H;

        lcdc->REG_690 = SET_BITFIELD(lcdc->REG_690, LCDC_TFBDC_H_WIDTH_L, w & 0x3ff);
        lcdc->REG_694 &= ~LCDC_TFBDC_H_WIDTH_H;
        if (w > 0x3ff)
            lcdc->REG_694 |= LCDC_TFBDC_H_WIDTH_H;

        lcdc->REG_694 = SET_BITFIELD(lcdc->REG_694, LCDC_TFBDC_REQ_TILE_LINENUM, xtile_num);
        lcdc->REG_698 = SET_BITFIELD(lcdc->REG_698, LCDC_TFBDC_VMIRROR_OVERHEAD, xtile_diif);
        if (vmirror) {
            if (xtile_diif == 0) tile = 1;
            lcdc->REG_6FC = SET_BITFIELD(lcdc->REG_6FC, LCDC_TFBDC_REQ_START_TILE, tile + tile_num - xtile_num);
            lcdc->REG_698 = SET_BITFIELD(lcdc->REG_698, LCDC_TFBDC_REQ_TILE_NUM, tile + tile_num + xtile + xtile_diif);
            lcdc->REG_724 = SET_BITFIELD(lcdc->REG_724, LCDC_TFBDC_VMIRROR_REMAIN_TILES, tile - xtile_diif);
        }
        else {
            lcdc->REG_6FC = SET_BITFIELD(lcdc->REG_6FC, LCDC_TFBDC_REQ_START_TILE, tile);
            lcdc->REG_698 = SET_BITFIELD(lcdc->REG_698, LCDC_TFBDC_REQ_TILE_NUM, tile + tile_num);
        }
    }
    else {
        LCDC_INFO("%s: invalid layer id %d, tile:%d, tile_num:%d, xtile:%d, xtile_num:%d, xtile_diif:%d", __func__, lid, tile, tile_num, xtile, xtile_num, xtile_diif);
        return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d tile:%d, tile_num:%d, xtile:%d, xtile_num:%d, xtile_diif:%d", __func__, lid, w, tile, tile_num, xtile, xtile_num, xtile_diif);
    return LCDC_OK;
}

int hal_lcdc_ltfbdc_startaddr(int lid, uint32_t src_addr, uint16_t src_w, uint16_t src_h, uint16_t bpp)
{
    if (lid == LCDC_LAYER_GRA && src_w <= 0x7fff) {
        uint32_t addr = src_addr;
        lcdc->REG_698 &= ~LCDC_TFBDC_REQ_BASE_ADDR_MSB;
        if (bpp == 2)
            addr += ((src_w * src_h) / (32 *4) * 8);
        else
            addr += ((src_w * src_h) / (16 *4) * 8);

        lcdc->REG_69C = SET_BITFIELD(lcdc->REG_69C, LCDC_TFBDC_REQ_BASE_ADDR, addr >> 4);
    }
    else {
        LCDC_INFO("%s: invalid layer id %d, src_addr:0x%x, src_w:%d, src_h:%d, bpp:%d", __func__, lid, src_addr, src_w, src_h, bpp);
        return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d src_addr:0x%x, src_w:%d, src_h:%d, bpp:%d", __func__, lid, src_addr, src_w, src_h, bpp);
    return LCDC_OK;
}

int hal_lcdc_ltfbdc_set_format(int lid, enum hal_lcdc_tiled_format_e format)
{
    if (lid == LCDC_LAYER_GRA) {
        lcdc->REG_690 &= ~(LCDC_TFBDC_32B | LCDC_TFBDC_24B | LCDC_TFBDC_16B | LCDC_TFBDC_FORMAT_MASK);
        lcdc->REG_694 &= ~(LCDC_TFBDC_REQ_FORMAT_MASK);

        if (format == LCDC_TILED_FMT_16BIT) {
            lcdc->REG_690 |= LCDC_TFBDC_16B | LCDC_TFBDC_FORMAT(2);
            lcdc->REG_694 |= LCDC_TFBDC_REQ_FORMAT(2);
        } else if (format == LCDC_TILED_FMT_24BIT) {
            lcdc->REG_690 |= LCDC_TFBDC_24B | LCDC_TFBDC_FORMAT(1);
            lcdc->REG_694 |= LCDC_TFBDC_REQ_FORMAT(1);
        } else {
            lcdc->REG_690 |= LCDC_TFBDC_32B | LCDC_TFBDC_FORMAT(0);
            lcdc->REG_694 |= LCDC_TFBDC_REQ_FORMAT(0);
        }
    }
    else {
        LCDC_INFO("%s: invalid layer id %d", __func__, lid);
        return LCDC_INVAL;
    }
    LCDC_DBG("%s: set tfbdc format %d", __func__, format);
    return LCDC_OK;
}

int hal_lcdc_ltfbdc_swap_rb(int lid, bool enable)
{
    if (lid == LCDC_LAYER_GRA) {
        lcdc->REG_694 &= ~LCDC_TFBDC_SWAP_RB;
        if (enable) {
            lcdc->REG_694 |= LCDC_TFBDC_SWAP_RB;
        }
    }
    else {
        LCDC_INFO("%s: invalid layer id %d", __func__, lid);
        return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d enable: %d", __func__, lid, enable);
    return LCDC_OK;
}

int hal_lcdc_yuv_fix(int lid, uint16_t width, enum hal_lcdc_color_format_e fmt)
{
    if (fmt == LCDC_CFMT_YUV420_P16) {
        if (width == 410 || width == 454)
            lcdc->REG_724 = SET_BITFIELD(lcdc->REG_724, LCDC_YUV420P_16_LOL_BASE, 0x10);
        else
            lcdc->REG_724 = SET_BITFIELD(lcdc->REG_724, LCDC_YUV420P_16_LOL_BASE, 0x20);
    }
    return LCDC_OK;
}

int hal_lcdc_yuv_lstartaddr(int lid, uint32_t yaddr, uint32_t uaddr, uint32_t vaddr)
{
    yaddr = ADDR_CPU_TO_DEV(yaddr);
    uaddr = ADDR_CPU_TO_DEV(uaddr);
    vaddr = ADDR_CPU_TO_DEV(vaddr);
    switch (lid) {
        case LCDC_LAYER_DMA:
            lcdc->REG_0C0 = LCDC_DMA_SA_Y0(yaddr);
            lcdc->REG_0C4 = LCDC_DMA_SA_U0(uaddr);
            lcdc->REG_0C8 = LCDC_DMA_SA_V0(vaddr);
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_000 = LCDC_TVD_SA_Y0(yaddr);
            lcdc->REG_004 = LCDC_TVD_SA_U0(uaddr);
            lcdc->REG_008 = LCDC_TVD_SA_V0(vaddr);
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d address: y 0x%08x, u 0x%08x, v 0x%08x", __func__, lid, yaddr, uaddr, vaddr);
    return LCDC_OK;
}

int hal_lcdc_yuv_lpitch(int lid, uint16_t ypitch, uint16_t upitch, uint16_t vpitch)
{
    switch (lid) {
        case LCDC_LAYER_DMA:
            lcdc->REG_0E0 = SET_BITFIELD(lcdc->REG_0E0, LCDC_DMA_PITCH_Y, ypitch);
            lcdc->REG_0E4 = LCDC_DMA_PITCH_U(upitch) | LCDC_DMA_PITCH_V(vpitch);
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_020 = SET_BITFIELD(lcdc->REG_020, LCDC_TVD_PITCH_Y, ypitch);
            lcdc->REG_024 = LCDC_TVD_PITCH_U(upitch) | LCDC_TVD_PITCH_V(vpitch);
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d pitch: y %d, u %d, v %d", __func__, lid, ypitch, upitch, vpitch);
    return LCDC_OK;
}

int hal_lcdc_yuvdsp_contrast(int lid, uint32_t contrast)
{
    if (lid == LCDC_LAYER_DMA)
        lcdc->REG_1AC = contrast;
    else if (lid == LCDC_LAYER_TVD)
        lcdc->REG_088 = contrast;
    else
        return LCDC_INVAL;
    LCDC_DBG("%s: layer %d contrast: %d", __func__, lid, contrast);
    return LCDC_OK;
}

int hal_lcdc_yuvdsp_saturation(int lid, uint32_t saturation)
{
    if (lid == LCDC_LAYER_DMA)
        lcdc->REG_1B0 = saturation;
    else if (lid == LCDC_LAYER_TVD)
        lcdc->REG_08C = saturation;
    else
        return LCDC_INVAL;
    LCDC_DBG("%s: layer %d saturation: %d", __func__, lid, saturation);
    return LCDC_OK;
}

int hal_lcdc_yuvdsp_hue(int lid, uint32_t hue)
{
    if (lid == LCDC_LAYER_DMA)
        lcdc->REG_1B4 = hue;
    else if (lid == LCDC_LAYER_TVD)
        lcdc->REG_090 = hue;
    else
        return LCDC_INVAL;
    LCDC_DBG("%s: layer %d hue: %d", __func__, lid, hue);
    return LCDC_OK;
}

int hal_lcdc_yuvdsp_enable(int lid, bool enable)
{
    if (enable) {
        if (lid == LCDC_LAYER_DMA)
            lcdc->REG_260 |= LCDC_PN_CBSH_ENA;
        else if (lid == LCDC_LAYER_TVD)
            lcdc->REG_080 |= LCDC_TV_CBSH_ENA;
        else
            return LCDC_INVAL;
    } else {
        if (lid == LCDC_LAYER_DMA)
            lcdc->REG_260 &= ~LCDC_PN_CBSH_ENA;
        else if (lid == LCDC_LAYER_TVD)
            lcdc->REG_080 &= ~LCDC_TV_CBSH_ENA;
        else
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d %s", __func__, lid, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_yuvdsp_colorspace(int lid, uint8_t csc)
{
    if (lid == LCDC_LAYER_DMA)
        lcdc->REG_1BC = SET_BITFIELD(lcdc->REG_1BC, LCDC_PN_CSC, csc);
    else if (lid == LCDC_LAYER_TVD)
        lcdc->REG_1BC = SET_BITFIELD(lcdc->REG_1BC, LCDC_TV_CSC, csc);
    else
        return LCDC_INVAL;
    LCDC_DBG("%s: layer %d csc: %d", __func__, lid, csc);
    return LCDC_OK;
}

int hal_lcdc_yuv_fast_mode(bool enable, uint16_t rds)
{
    if (enable)
        lcdc->REG_264 |= LCDC_YUV_PLANAR_FAST;
    else
        lcdc->REG_264 &= ~LCDC_YUV_PLANAR_FAST;

    lcdc->REG_264 = SET_BITFIELD(lcdc->REG_264, LCDC_YUV_PLANAR_RDS, rds);
    LCDC_DBG("%s: yuv fast mode %s, rds:%d", __func__, enable ? "enabled" : "disabled", rds);
    return LCDC_OK;
}

int hal_lcdc_lvsepxcount(int start, int end)
{
    lcdc->REG_13C = SET_BITFIELD(lcdc->REG_13C, LCDC_PN_V_SPXLCNT, start);
    lcdc->REG_13C = SET_BITFIELD(lcdc->REG_13C, LCDC_PN_V_EPXLCNT, end);
    LCDC_DBG("%s: start %d, end %d", __func__, start, end);
    return LCDC_OK;
}

int hal_lcdc_stby_vsync_set(int stby_len, int start_line, int end_line)
{
    if (stby_len < LCDC_VSYNC_STBY_MIN)
        stby_len = LCDC_VSYNC_STBY_MIN;

    lcdc->REG_218 &= ~(LCDC_STBY_LENGTH_MASK | LCDC_VSYNC_START_LINE_MASK | LCDC_VSYNC_END_LINE_MASK);
    lcdc->REG_218 |= LCDC_VSYNC_START_LINE(start_line) | LCDC_VSYNC_END_LINE(end_line) |
                    LCDC_STBY_LENGTH(stby_len);
    LCDC_DBG("%s: standby vsync 0x%x, start line %d, end line %d", __func__,
                stby_len, start_line, end_line);
    return LCDC_OK;
}

int hal_lcdc_porch_vsync_set(int hfp, int hbp, int vlines)
{
    lcdc->REG_21C = LCDC_H_FRONT_PORCH_SMPN(hfp) | LCDC_H_BACK_PORCH_SMPN(hbp) |
                    LCDC_VSYNC_NLINE(vlines);
    LCDC_DBG("%s: vsync hfp %d, hbp %d, vsync lines %d", __func__, hfp, hbp, vlines);
    return LCDC_OK;
}

int hal_lcdc_pix_vsync_set(int start_pix, int end_pix)
{
    lcdc->REG_220 = LCDC_VSYNC_START_PIX(start_pix) | LCDC_VSYNC_END_PIX(end_pix);
    LCDC_DBG("%s: vsync start pixel %d, end pixel %d", __func__, start_pix, end_pix);
    return LCDC_OK;
}

int hal_lcdc_sponscreen_set(int lid, uint16_t vln, uint16_t hpxl)
{
    switch (lid) {
        case LCDC_LAYER_GRA:
            lcdc->REG_100 &= ~(LCDC_GRA_OVSA_HPXL_MASK | LCDC_GRA_OVSA_VLN_MASK);
            lcdc->REG_100 = LCDC_GRA_OVSA_HPXL(hpxl) | LCDC_GRA_OVSA_VLN(vln) |
                            LCDC_GRAFRM_FIX;
            break;
        case LCDC_LAYER_DMA:
            lcdc->REG_0E8 &= ~(LCDC_DMA_OVSA_HPXL_MASK | LCDC_DMA_OVSA_VLN_MASK);
            lcdc->REG_0E8 = LCDC_DMA_OVSA_HPXL(hpxl) | LCDC_DMA_OVSA_VLN(vln) |
                            LCDC_DMAFRM_FIX;
            break;
        case LCDC_LAYER_TVG:
        case LCDC_LAYER_DEC:
        case LCDC_LAYER_TILED:
            lcdc->REG_040 &= ~(LCDC_TVG_OVSA_HPXL_MASK | LCDC_TVG_OVSA_VLN_MASK);
            lcdc->REG_040 = LCDC_TVG_OVSA_HPXL(hpxl) | LCDC_TVG_OVSA_VLN(vln) |
                            LCDC_TVGFRM_FIX;
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_028 &= ~(LCDC_TVD_OVSA_HPXL_MASK | LCDC_TVD_OVSA_VLN_MASK);
            lcdc->REG_028 = LCDC_TVD_OVSA_HPXL(hpxl) | LCDC_TVD_OVSA_VLN(vln) |
                            LCDC_TVDFRM_FIX;
            break;
        case LCDC_LAYER_SMT:
            lcdc->REG_5F8 &= ~(LCDC_SMT_OVSA_HPXL_SP0_MASK | LCDC_SMT_OVSA_VLN_SP0_MASK);
            lcdc->REG_5F8 = LCDC_SMT_OVSA_HPXL_SP0(hpxl) | LCDC_SMT_OVSA_VLN_SP0(vln);
            lcdc->REG_04C &= ~(LCDC_TVC_OVSA_HPXL_MASK | LCDC_TVC_OVSA_VLN_MASK);
            lcdc->REG_04C = LCDC_TVC_OVSA_HPXL(hpxl) | LCDC_TVC_OVSA_VLN(vln);
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d hpixel: %d, vln %d", __func__, lid, hpxl, vln);
    return LCDC_OK;
}

int hal_lcdc_bgl_vhtotal_set(int htotal, int vtotal)
{
    lcdc->REG_114 = LCDC_PN_H_TOTAL(htotal) | LCDC_PN_V_TOTAL(vtotal);
    LCDC_DBG("%s: pn htotal %d, vtotal %d", __func__, htotal, vtotal);
    return LCDC_OK;
}

int hal_lcdc_bgl_vtotal_set(int vtotal)
{
    lcdc->REG_114 &= ~LCDC_PN_V_TOTAL_MASK;
    lcdc->REG_114 |= LCDC_PN_V_TOTAL(vtotal);
    LCDC_DBG("%s: pn vtotal %d", __func__, vtotal);
    return LCDC_OK;
}

int hal_lcdc_bgl_htotal_set(int htotal)
{
    lcdc->REG_114 &= ~LCDC_PN_H_TOTAL_MASK;
    lcdc->REG_114 |= LCDC_PN_H_TOTAL(htotal);
    LCDC_DBG("%s: pn htotal %d", __func__, htotal);
    return LCDC_OK;
}

int hal_lcdc_bgl_hporch_set(int hfp, int hbp)
{
    lcdc->REG_11C = LCDC_PN_H_FPORCH(hfp) | LCDC_PN_H_BPORCH(hbp);
    lcdc->REG_05C = LCDC_TV_H_FPORCH(hfp) | LCDC_TV_H_BPORCH(hbp);
    LCDC_DBG("%s: pn hfp %d, hbp %d", __func__, hfp, hbp);
    return LCDC_OK;
}

int hal_lcdc_bgl_vporch_set(int vfp, int vbp)
{
    lcdc->REG_120 = LCDC_PN_V_FPORCH(vfp) | LCDC_PN_V_BPORCH(vbp);
    lcdc->REG_060 = LCDC_TV_V_FPORCH(vfp) | LCDC_TV_V_BPORCH(vbp);
    LCDC_DBG("%s: pn vfp %d, vbp %d", __func__, vfp, vbp);
    return LCDC_OK;
}

int hal_lcdc_lsize(int lid, uint16_t w, uint16_t h)
{
    switch (lid) {
        case LCDC_LAYER_GRA:
            lcdc->REG_104 = LCDC_GRA_H_PIXEL(w) | LCDC_GRA_V_LINE(h);
            break;
        case LCDC_LAYER_DMA:
            lcdc->REG_0EC = LCDC_DMA_H_PIXEL(w) | LCDC_DMA_V_LINE(h);
            if (w < ((GET_BITFIELD(lcdc->REG_1DC, LCDC_DMA_BURST0TO3) + 1) * 64))
                lcdc->REG_26C |= LCDC_WIDTH_SM_BURST_LENGTH_DMA;
            else
                lcdc->REG_26C &= ~LCDC_WIDTH_SM_BURST_LENGTH_DMA;
            if (w <= 24)
                lcdc->REG_724 = SET_BITFIELD(lcdc->REG_724, LCDC_64TOPIXEL_RD_CNT_DMA, 0x18);
            else
                lcdc->REG_724 = SET_BITFIELD(lcdc->REG_724, LCDC_64TOPIXEL_RD_CNT_DMA, 0x20);
            break;
        case LCDC_LAYER_TVG:
            lcdc->REG_044 = LCDC_TVG_H_PIXEL(w) | LCDC_TVG_V_LINE(h);
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_02C = LCDC_TVD_H_PIXEL(w) | LCDC_TVD_V_LINE(h);
            if (w < ((GET_BITFIELD(lcdc->REG_1DC, LCDC_TVD_BURST0TO3) + 1) * 64))
                lcdc->REG_26C |= LCDC_WIDTH_SM_BURST_LENGTH_TVD;
            else
                lcdc->REG_26C &= ~LCDC_WIDTH_SM_BURST_LENGTH_TVD;
            if (w <= 24)
                lcdc->REG_728 = SET_BITFIELD(lcdc->REG_728, LCDC_64TOPIXEL_RD_CNT_TVD, 0x18);
            else
                lcdc->REG_728 = SET_BITFIELD(lcdc->REG_728, LCDC_64TOPIXEL_RD_CNT_TVD, 0x20);
            break;
        case LCDC_LAYER_SMT:
            lcdc->REG_650 = LCDC_SMT_H_PIXEL(w) | LCDC_SMT_V_LINE(h);
            lcdc->REG_628 = LCDC_SMT_H_PIXEL_SA0(w) | LCDC_SMT_V_LINE_SA0(h);
            lcdc->REG_5FC &= ~(LCDC_SMT_OVLEN_HPXL_SP0_MASK | LCDC_SMT_OVLEN_VLN_SP0_MASK);
            lcdc->REG_5FC |= LCDC_SMT_OVLEN_HPXL_SP0(w) | LCDC_SMT_OVLEN_VLN_SP0(h);
            break;
        case LCDC_LAYER_DEC:
            lcdc->REG_044 = LCDC_TVG_H_PIXEL(w) | LCDC_TVG_V_LINE(h);
            lcdc->REG_2DC = LCDC_DEC_H_PIXEL(w) | LCDC_DEC_V_LINE(h);
            break;
        case LCDC_LAYER_TILED:
            lcdc->REG_044 = LCDC_TVG_H_PIXEL(w) | LCDC_TVG_V_LINE(h);
            lcdc->REG_338 = LCDC_TILED_H_PIXEL(w) | LCDC_TILED_V_LINE(h);
            break;
        case LCDC_LAYER_WB:
            lcdc->REG_204 = LCDC_WDMA_IMG_WIDTH(w) | LCDC_WDMA_IMG_HEIGHT(h);
#ifndef LCDC_HAS_NEWALG_BLEND
            lcdc->REG_70C = SET_BITFIELD(lcdc->REG_70C, LCDC_GAMMA_IN_ALPHA_SEL_PN, 2);
            lcdc->REG_70C = SET_BITFIELD(lcdc->REG_70C, LCDC_GAMMA_IN_ALPHA_SEL_TV, 2);
#else
            lcdc->REG_70C = SET_BITFIELD(lcdc->REG_70C, LCDC_GAMMA_IN_ALPHA_SEL_PN, 0);
            lcdc->REG_70C = SET_BITFIELD(lcdc->REG_70C, LCDC_GAMMA_IN_ALPHA_SEL_TV, 0);
#endif
            break;
        case LCDC_LAYER_SPU:
            if (!lcdc_video_mode) {
                w++;
                h++;
            }
            lcdc->REG_118 = LCDC_PN_H_ACTIVE(w) | LCDC_PN_V_ACTIVE(h);
            lcdc->REG_058 = LCDC_TV_H_ACTIVE(w) | LCDC_TV_V_ACTIVE(h);
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d width %d, height %d", __func__, lid, w, h);
    return LCDC_OK;
}

int hal_lcdc_lzoom_set(int lid, uint16_t w, uint16_t h)
{
    switch (lid) {
        case LCDC_LAYER_GRA:
            lcdc->REG_108 = LCDC_GRAZM_H_PIXEL(w) | LCDC_GRAZM_V_LINE(h);
            break;
        case LCDC_LAYER_DMA:
            lcdc->REG_0F0 = LCDC_DMAZM_H_PIXEL(w) | LCDC_DMAZM_V_LINE(h);
            break;
        case LCDC_LAYER_TVG:
        case LCDC_LAYER_DEC:
        case LCDC_LAYER_TILED:
            lcdc->REG_048 = LCDC_TVGZM_H_PIXEL(w) | LCDC_TVGZM_V_LINE(h);
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_030 = LCDC_TVDZM_H_PIXEL(w) | LCDC_TVDZM_V_LINE(h);
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d zoom width %d, zoom height %d", __func__, lid, w, h);
    return LCDC_OK;
}

int hal_lcdcl_enable(int lid, bool enable)
{
    switch (lid) {
        case LCDC_LAYER_GRA:
            if (enable)
                lcdc->REG_264 |= LCDC_GRA_ENA;
            else
                lcdc->REG_264 &= ~LCDC_GRA_ENA;
            break;
        case LCDC_LAYER_DMA:
            if (enable)
                lcdc->REG_190 |= LCDC_DMA_ENA;
            else
                lcdc->REG_190 &= ~LCDC_DMA_ENA;
            break;
        case LCDC_LAYER_TVG:
            if (enable)
                lcdc->REG_080 |= LCDC_TVG_ENA;
            else
                lcdc->REG_080 &= ~LCDC_TVG_ENA;
            break;
        case LCDC_LAYER_TVD:
            if (enable)
                lcdc->REG_260 |= LCDC_TVD_ENA;
            else
                lcdc->REG_260 &= ~LCDC_TVD_ENA;
            break;
        case LCDC_LAYER_SMT:
            if (enable)
                lcdc->REG_63C |= LCDC_SMT_ENA | LCDC_SMT_NOVLAP_SPLIT_ENA;
            else
                lcdc->REG_63C &= ~LCDC_SMT_ENA;
            break;
        case LCDC_LAYER_DEC:
            if (enable) {
                lcdc->REG_2D0 &= ~LCDC_DEC_HLEN_DR;
                lcdc->REG_2D0 |= LCDC_DEC_ENA;
#ifdef LCDC_TVG_WORKAROUND
                lcdc->REG_080 |= LCDC_TVG_ENA;
#endif
            } else {
                lcdc->REG_2D0 &= ~LCDC_DEC_ENA;
#ifdef LCDC_TVG_WORKAROUND
                lcdc->REG_080 &= ~LCDC_TVG_ENA;
#endif
            }
            break;
        case LCDC_LAYER_TILED:
            if (enable) {
                lcdc->REG_32C &= ~LCDC_TILED_HLEN_DR;
                lcdc->REG_32C |= LCDC_TILED_ENA;
#ifdef LCDC_TVG_WORKAROUND
                lcdc->REG_080 |= LCDC_TVG_ENA;
#endif
            } else {
                lcdc->REG_32C &= ~LCDC_TILED_ENA;
#ifdef LCDC_TVG_WORKAROUND
                lcdc->REG_080 &= ~LCDC_TVG_ENA;
#endif
            }
            break;
        case LCDC_LAYER_WB:
            if (enable)
                lcdc->REG_200 |= LCDC_WDMA_ENA;
            else
                lcdc->REG_200 &= ~LCDC_WDMA_ENA;
            break;
        case LCDC_LAYER_SPU:
            if (enable)
                lcdc->REG_188 |= LCDC_SMPN_ENA;
            else
                lcdc->REG_188 &= ~LCDC_SMPN_ENA;
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d %s", __func__, lid, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_test_mode(bool enable)
{
    if (enable)
        lcdc->REG_264 |= LCDC_GRA_TSTMODE;
    else
        lcdc->REG_264 &= ~LCDC_GRA_TSTMODE;
    LCDC_DBG("%s: gra layer test mode %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdcl_dumb_enable(bool enable)
{
    if (enable)
        lcdc->REG_1B8 |= LCDC_DUMB_ENA;
    else
        lcdc->REG_1B8 &= ~LCDC_DUMB_ENA;
    LCDC_DBG("%s: dumb mode %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_dumb_vsync_mode(uint8_t mode)
{
    lcdc->REG_194 = SET_BITFIELD(lcdc->REG_194, LCDC_PN_VSYNC_MODE, mode);
    LCDC_DBG("%s: dumb vsync mode %d", __func__, mode);
    return LCDC_OK;
}

int hal_lcdc_source_switch(bool enable)
{
    if (enable)
        lcdc->REG_210 |= LCDC_DUMB_SOURCE_SEL;
    else
        lcdc->REG_210 &= ~LCDC_DUMB_SOURCE_SEL;
    LCDC_DBG("%s: dumb source switch %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_lcolor(int lid, uint32_t rgb)
{
    if (lid != LCDC_LAYER_SPU) {
        LCDC_INFO("%s: invalid layer %d", __func__, lid);
        return LCDC_INVAL;
    }
    lcdc->REG_124 = LCDC_PN_BLANKCOLOR(rgb);
    lcdc->REG_064 = LCDC_TV_BLANKCOLOR(rgb);
    LCDC_DBG("%s: SPU blank color 0x%08x", __func__, rgb);
    return LCDC_OK;
}

int hal_lcdc_dither_s4x8(bool enable)
{
    if (enable)
        lcdc->REG_1EC |= (LCDC_DITHER_4X8_PN | LCDC_DITHER_4X8_TV);
    else
        lcdc->REG_1EC &= ~(LCDC_DITHER_4X8_PN | LCDC_DITHER_4X8_TV);
    LCDC_DBG("%s: dither 4x8 %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_dither(bool enable, uint8_t level, uint8_t red,
                    uint8_t green, uint8_t blue)
{
    uint32_t v = lcdc->REG_1EC;

    v &= ~(LCDC_DITHER_EN_PN | LCDC_DITHER_EN_TV |
           LCDC_DITHER_MODE_PN_MASK | LCDC_DITHER_MODE_TV_MASK);

    if (enable) {
        v |= (LCDC_DITHER_EN_PN | LCDC_DITHER_EN_TV) |
              LCDC_DITHER_MODE_PN(level) | LCDC_DITHER_MODE_TV(level);
    }

    lcdc->REG_1EC = v;
    LCDC_DBG("%s: dither %s, enable %d", __func__, enable ? "enabled" : "disabled", enable);
    return LCDC_OK;
}

uint32_t hal_lcdc_dither_read(uint8_t index)
{
    uint32_t v;
    lcdc->REG_1EC = SET_BITFIELD(lcdc->REG_1EC, LCDC_DITHER_TABLE_INDEX_SEL, index);
    v = lcdc->REG_1F0;
    LCDC_DBG("%s: table index %d, value 0x%08x", __func__, index, v);
    return v;
}

int hal_lcdc_dither_write(uint8_t index, uint32_t value)
{
    lcdc->REG_1EC = SET_BITFIELD(lcdc->REG_1EC, LCDC_DITHER_TABLE_INDEX_SEL, index);
    lcdc->REG_1F0 = LCDC_DITHER_TBL_DATA(value);
    LCDC_DBG("%s: table index %d, value 0x%08x", __func__, index, value);
    return LCDC_OK;
}

static bool hal_lcdc_cfmt_is_yuv(enum hal_lcdc_color_format_e fmt)
{
    if (fmt == LCDC_CFMT_YUV422_UYVY || fmt == LCDC_CFMT_YUV422_I422 ||
        fmt == LCDC_CFMT_YUV420_I420 || fmt == LCDC_CFMT_YUV420_NV12 ||
        fmt == LCDC_CFMT_YUV420_P16)
        return true;
    return false;
}

static uint8_t hal_lcdc_cfmt_regval(enum hal_lcdc_color_format_e fmt)
{
    switch (fmt) {
        case LCDC_CFMT_RGB565:
            return 0x0;
        case LCDC_CFMT_RGB1555:
            return 0x1;
        case LCDC_CFMT_RGB888:
            return 0x2;
        case LCDC_CFMT_RGBA8888:
            return 0x4;
        case LCDC_CFMT_YUV422_UYVY:
            return 0x5;
        case LCDC_CFMT_YUV422_I422:
            return 0x6;
        case LCDC_CFMT_YUV420_I420:
            return 0x7;
        case LCDC_CFMT_PALETTE_4:
            return 0x9;
        case LCDC_CFMT_PALETTE_8:
            return 0xA;
        case LCDC_CFMT_ARGB8888:
            return 0xB;
        case LCDC_CFMT_YUV420_NV12:
            return 0xC;
        case LCDC_CFMT_YUV420_P16:
            return 0xD;
        case LCDC_CFMT_BGR565:
            return 0x80;
        case LCDC_CFMT_BGR888:
            return 0x82;
        case LCDC_CFMT_BGRA8888:
            return 0x84;
        case LCDC_CFMT_ABGR8888:
            return 0x8B;
        default:
            return 0x4;
    }
}

static uint8_t hal_lcdc_wb_cfmt_regval(enum hal_lcdc_color_format_e fmt)
{
    switch (fmt) {
        case LCDC_CFMT_RGB565:
            return 0x0;
        case LCDC_CFMT_RGB888:
            return 0x1;
        case LCDC_CFMT_RGBA8888:
            return 0x2;
        case LCDC_CFMT_ARGB8888:
            return 0x3;
        default:
            return 0x2;
    }
}

int hal_lcdc_lpfc_fmt(int lid, uint8_t fmt)
{
    uint8_t v = hal_lcdc_cfmt_regval(fmt);
    bool swap_rb = !!(v & 0x80);
    bool is_yuv = hal_lcdc_cfmt_is_yuv(fmt);

    switch (lid) {
        case LCDC_LAYER_GRA:
            lcdc->REG_264 &= ~(LCDC_GRA_SWAPRB | LCDC_YUV2RGB_GRA | LCDC_GRAFORMAT_MASK);
            lcdc->REG_264 |= LCDC_GRAFORMAT(v) | (swap_rb ? LCDC_GRA_SWAPRB : 0) |
                             (is_yuv ? LCDC_YUV2RGB_GRA : 0);
            break;
        case LCDC_LAYER_DMA:
            lcdc->REG_190 &= ~(LCDC_DMA_SWAPRB | LCDC_YUV2RGB_DMA | LCDC_DMAFORMAT_MASK);
            lcdc->REG_190 |= LCDC_DMAFORMAT(v) | (swap_rb ? LCDC_DMA_SWAPRB : 0) |
                             (is_yuv ? LCDC_YUV2RGB_DMA : 0);
            break;
        case LCDC_LAYER_TVG:
        case LCDC_LAYER_TILED:
            lcdc->REG_080 &= ~(LCDC_TVG_SWAPRB | LCDC_YUV2RGB_TVG | LCDC_TVGFORMAT_MASK);
            lcdc->REG_080 |= LCDC_TVGFORMAT(v) | (swap_rb ? LCDC_TVG_SWAPRB : 0) |
                             (is_yuv ? LCDC_YUV2RGB_TVG : 0);
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_260 &= ~(LCDC_TVD_SWAPRB | LCDC_YUV2RGB_TVD | LCDC_TVDFORMAT_MASK);
            lcdc->REG_260 |= LCDC_TVDFORMAT(v) | (swap_rb ? LCDC_TVD_SWAPRB : 0) |
                             (is_yuv ? LCDC_YUV2RGB_TVD : 0);
            break;
        case LCDC_LAYER_SMT:
            if (is_yuv)
            {
                LCDC_INFO("%s: invalid layer id %d not support yuv", __func__, lid);
                return LCDC_INVAL;
            }
            lcdc->REG_63C &= ~(LCDC_SMTFORMAT_MASK);
            lcdc->REG_63C |= LCDC_SMTFORMAT(v);
            break;
        case LCDC_LAYER_WB:
            v = hal_lcdc_wb_cfmt_regval(fmt);
            lcdc->REG_200 &= ~LCDC_WDMA_PIX_FMT_MASK;
            lcdc->REG_200 |= LCDC_WDMA_PIX_FMT(v);
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d color format 0x%x", __func__, lid, v);
    return LCDC_OK;
}

int hal_lcdc_yuv2rgb_enable(int lid, bool enable, bool swap_yu, bool swap_uv)
{
    switch (lid) {
        case LCDC_LAYER_GRA:
            lcdc->REG_264 &= ~(LCDC_YUV2RGB_GRA | LCDC_GRA_SWAPYU | LCDC_GRA_SWAPUV);
            lcdc->REG_264 |= (enable ? LCDC_YUV2RGB_GRA : 0) | (swap_yu ? LCDC_GRA_SWAPYU : 0) |
                             (swap_uv ? LCDC_GRA_SWAPUV : 0);
            break;
        case LCDC_LAYER_DMA:
            lcdc->REG_190 &= ~(LCDC_YUV2RGB_DMA | LCDC_DMA_SWAPYU | LCDC_DMA_SWAPUV);
            lcdc->REG_190 |= (enable ? LCDC_YUV2RGB_DMA : 0) | (swap_yu ? LCDC_DMA_SWAPYU : 0) |
                             (swap_uv ? LCDC_DMA_SWAPUV : 0);
            break;
        case LCDC_LAYER_TVG:
            lcdc->REG_080 &= ~(LCDC_YUV2RGB_TVG | LCDC_TVG_SWAPYU | LCDC_TVG_SWAPUV);
            lcdc->REG_080 |= (enable ? LCDC_YUV2RGB_TVG : 0) | (swap_yu ? LCDC_TVG_SWAPYU : 0) |
                             (swap_uv ? LCDC_TVG_SWAPUV : 0);
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_260 &= ~(LCDC_YUV2RGB_TVD | LCDC_TVD_SWAPYU | LCDC_TVD_SWAPUV);
            lcdc->REG_260 |= (enable ? LCDC_YUV2RGB_TVD : 0) | (swap_yu ? LCDC_TVD_SWAPYU : 0) |
                             (swap_uv ? LCDC_TVD_SWAPUV : 0);
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d yuv2rgb %s, swap_yu %s, swap_uv %s", __func__, lid,
                enable ? "enabled" : "disabled", swap_yu ?  "enabled" : "disabled",
                swap_uv ?  "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_lpfc_ltransp(int lid, enum hal_lcdc_alpha_mode_e mode, uint8_t alpha)
{
#ifdef LCDC_HAS_NEWALG_BLEND
    if (lid == LCDC_LAYER_GRA) {
        lcdc->REG_194 &= ~(LCDC_PN_ALPHA_PIXEL_MODE | LCDC_PN_ALPHA_MASK);
        lcdc->REG_194 |= (mode == LCDC_CONST_ALPHA ? LCDC_PN_ALPHA_CONST_MODE : LCDC_PN_ALPHA_PIXEL_MODE) | LCDC_PN_ALPHA(alpha);
    } else if (lid == LCDC_LAYER_TVG) {
        lcdc->REG_084 &= ~(LCDC_TV_ALPHA_PIXEL_MODE | LCDC_TV_ALPHA_MASK);
        lcdc->REG_084 |= (mode == LCDC_CONST_ALPHA ? LCDC_TV_ALPHA_CONST_MODE : LCDC_TV_ALPHA_PIXEL_MODE) | LCDC_TV_ALPHA(alpha);
    } else if (lid == LCDC_LAYER_TVD) {
        lcdc->REG_2C8 &= ~LCDC_TVD_ALPHA_PIXEL_MODE;
        lcdc->REG_2C8 |= (mode == LCDC_CONST_ALPHA ? LCDC_TVD_ALPHA_CONST_MODE : LCDC_TVD_ALPHA_PIXEL_MODE);
        lcdc->REG_2CC = SET_BITFIELD(lcdc->REG_2CC, LCDC_TVD_ALPHA, alpha);
    } else if (lid == LCDC_LAYER_SMT) {
        lcdc->REG_63C &= ~LCDC_SMT_ALPHA_PIXEL_MODE;
        lcdc->REG_63C |= (mode == LCDC_CONST_ALPHA ? LCDC_SMT_ALPHA_CONST_MODE : LCDC_SMT_ALPHA_PIXEL_MODE);
        lcdc->REG_63C = SET_BITFIELD(lcdc->REG_63C, LCDC_SMT_ALPHA, alpha);
    }
    else {
        LCDC_INFO("%s: invalid layer id %d", __func__, lid);
        return LCDC_INVAL;
    }
    LCDC_DBG("%s:lid %d alpha mode %d, alpha value 0x%02x", __func__, lid, mode, alpha);
#else
    if (lid == LCDC_LAYER_GRA || lid == LCDC_LAYER_DMA) {
        if (mode == LCDC_CONST_ALPHA) {
            lcdc->REG_194 &= ~(LCDC_PN_ALPHA_PIXEL_MODE | LCDC_PN_ALPHA_MASK);
            lcdc->REG_194 |= LCDC_PN_ALPHA(alpha);
        } else {
            lcdc->REG_194 |= LCDC_PN_ALPHA_PIXEL_MODE;
        }
    } else if (lid == LCDC_LAYER_TVG || lid == LCDC_LAYER_TVD) {
        if (mode == LCDC_CONST_ALPHA) {
            lcdc->REG_084 &= ~(LCDC_TV_ALPHA_PIXEL_MODE | LCDC_TV_ALPHA_MASK);
            lcdc->REG_084 |= LCDC_TV_ALPHA(255 - alpha);
        } else {
            lcdc->REG_084 |= LCDC_TV_ALPHA_PIXEL_MODE;
        }
    }
    LCDC_DBG("%s: alpha mode %d, alpha value 0x%02x", __func__, mode, alpha);
#endif
    return LCDC_OK;
}

int hal_lcdc_lchromakeyenable(int lid, bool enable, enum hal_lcdc_colorkey_mode_e mode)
{
    lcdc->REG_194 &= ~LCDC_PN_KEY_MODE_MASK;
    lcdc->REG_084 &= ~(LCDC_TV_CKEY_MODE_MASK | LCDC_TV_CKEY_DMA | LCDC_TV_CKEY_GRA |
                        LCDC_TV_CKEY_TVG | LCDC_TV_CKEY_TVD);
    if (enable) {
        lcdc->REG_194 |= LCDC_PN_KEY_MODE(mode);
        lcdc->REG_084 |= LCDC_TV_CKEY_DMA | LCDC_TV_CKEY_GRA | LCDC_TV_CKEY_TVG |
                         LCDC_TV_CKEY_TVD | LCDC_TV_CKEY_MODE(mode);
    }
    LCDC_DBG("%s: colorkey mode RGB_ALL %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_lchromakey(int lid, uint32_t rgb, uint32_t rrgb, uint8_t ra)
{
    uint8_t r, g, b;
    uint8_t rr, rg, rb;

    r = (rgb & 0xff0000) >> 16;
    g = (rgb & 0x00ff00) >> 8;
    b = (rgb & 0xff);
    rr = (rrgb & 0xff0000) >> 16;
    rg = (rrgb & 0x00ff00) >> 8;
    rb = (rrgb & 0xff);
    if (lid <= LCDC_LAYER_DMA) {
        lcdc->REG_130 = LCDC_PN_CKEY_Y(rr) | LCDC_PN_CKEY_Y1(r) | LCDC_PN_CKEY_Y2(r) |
                        LCDC_PN_ALPHA_Y(ra);
        lcdc->REG_134 = LCDC_PN_CKEY_U(rg) | LCDC_PN_CKEY_U1(g) | LCDC_PN_CKEY_U2(g) |
                        LCDC_PN_ALPHA_U(ra);
        lcdc->REG_138 = LCDC_PN_CKEY_V(rb) | LCDC_PN_CKEY_V1(b) | LCDC_PN_CKEY_V2(b) |
                        LCDC_PN_ALPHA_V(ra);
    } else if (lid == LCDC_LAYER_TVD) {
        lcdc->REG_070 = LCDC_TV_CKEY_Y(rr) | LCDC_TV_CKEY_Y1(r) | LCDC_TV_CKEY_Y2(r) |
                        LCDC_TV_ALPHA_Y(ra);
        lcdc->REG_074 = LCDC_TV_CKEY_U(rg) | LCDC_TV_CKEY_U1(g) | LCDC_TV_CKEY_U2(g) |
                        LCDC_TV_ALPHA_U(ra);
        lcdc->REG_078 = LCDC_TV_CKEY_V(rb) | LCDC_TV_CKEY_V1(b) | LCDC_TV_CKEY_V2(b) |
                        LCDC_TV_ALPHA_V(ra);
    }
    LCDC_DBG("%s: colorkey 0x%x, 0x%x, alpha 0x%x", __func__, rgb, rrgb, ra);
    return LCDC_OK;
}

int hal_lcdc_lchromakey_yr(int lid, uint32_t r2r1ra)
{
    uint8_t r, r1, r2, a;

    r2  = (r2r1ra & 0xff000000) >> 24;
    r1 = (r2r1ra & 0x00ff0000) >> 16;
    r = (r2r1ra & 0x0000ff00) >> 8;
    a = (r2r1ra & 0xff);

    if (lid <= LCDC_LAYER_DMA)
        lcdc->REG_130 = LCDC_PN_CKEY_Y(r) | LCDC_PN_CKEY_Y1(r1) | LCDC_PN_CKEY_Y2(r2) |
                    LCDC_PN_ALPHA_Y(a);
    else if (lid == LCDC_LAYER_TVD)
        lcdc->REG_070 = LCDC_TV_CKEY_Y(r) | LCDC_TV_CKEY_Y1(r1) | LCDC_TV_CKEY_Y2(r2) |
                    LCDC_TV_ALPHA_Y(a);
    else
        return LCDC_INVAL;
    LCDC_DBG("%s: r2r1ra 0x%x", __func__, r2r1ra);
    return LCDC_OK;
}

int hal_lcdc_lchromakey_ug(int lid, uint32_t g2g1ga)
{
    uint8_t g, g1, g2, a;

    g2  = (g2g1ga & 0xff000000) >> 24;
    g1 = (g2g1ga & 0x00ff0000) >> 16;
    g = (g2g1ga & 0x0000ff00) >> 8;
    a = (g2g1ga & 0xff);

    if (lid <= LCDC_LAYER_DMA)
        lcdc->REG_134 = LCDC_PN_CKEY_U(g) | LCDC_PN_CKEY_U1(g1) | LCDC_PN_CKEY_U2(g2) |
                    LCDC_PN_ALPHA_U(a);
    else if (lid == LCDC_LAYER_TVD)
        lcdc->REG_074 = LCDC_TV_CKEY_U(g) | LCDC_TV_CKEY_U1(g1) | LCDC_TV_CKEY_U2(g2) |
                    LCDC_TV_ALPHA_U(a);
    else
        return LCDC_INVAL;
    LCDC_DBG("%s: g2g1ga 0x%x", __func__, g2g1ga);
    return LCDC_OK;
}

int hal_lcdc_lchromakey_vb(int lid, uint32_t b2b1ba)
{
    uint8_t b, b1, b2, a;

    b2  = (b2b1ba & 0xff000000) >> 24;
    b1 = (b2b1ba & 0x00ff0000) >> 16;
    b = (b2b1ba & 0x0000ff00) >> 8;
    a = (b2b1ba & 0xff);

    if (lid <= LCDC_LAYER_DMA)
        lcdc->REG_138 = LCDC_PN_CKEY_V(b) | LCDC_PN_CKEY_V1(b1) | LCDC_PN_CKEY_V2(b2) |
                    LCDC_PN_ALPHA_V(a);
    else if (lid == LCDC_LAYER_TVD)
        lcdc->REG_078 = LCDC_TV_CKEY_V(b) | LCDC_TV_CKEY_V1(b1) | LCDC_TV_CKEY_V2(b2) |
                    LCDC_TV_ALPHA_V(a);
    else
        return LCDC_INVAL;
    LCDC_DBG("%s: b2b1ba 0x%x", __func__, b2b1ba);
    return LCDC_OK;
}

int hal_lcdc_lchromakey_newalpha(int lid, uint8_t newalpha)
{
    if (lid <= LCDC_LAYER_DMA) {
        lcdc->REG_130 = SET_BITFIELD(lcdc->REG_130, LCDC_PN_ALPHA_Y, newalpha);
        lcdc->REG_134 = SET_BITFIELD(lcdc->REG_130, LCDC_PN_ALPHA_U, newalpha);
        lcdc->REG_138 = SET_BITFIELD(lcdc->REG_130, LCDC_PN_ALPHA_V, newalpha);
    } else if (lid == LCDC_LAYER_TVD) {
        lcdc->REG_070 = SET_BITFIELD(lcdc->REG_070, LCDC_TV_ALPHA_Y, newalpha);
        lcdc->REG_074 = SET_BITFIELD(lcdc->REG_074, LCDC_TV_ALPHA_U, newalpha);
        lcdc->REG_078 = SET_BITFIELD(lcdc->REG_078, LCDC_TV_ALPHA_V, newalpha);
    }
    else
        return LCDC_INVAL;
    LCDC_DBG("%s: colorkey alpha 0x%x", __func__, newalpha);
    return LCDC_OK;
}

int hal_lcdc_panelpath_dmatrans_trigger_enable(int lid, bool enable)
{
    if (enable)
        lcdc->REG_194 |= LCDC_FRAME_TRIG;
    else
        lcdc->REG_194 &= ~LCDC_FRAME_TRIG;
    LCDC_DBG("%s: frame trigger %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_disp_avsync_trigger_disable(bool disable)
{
    if (disable)
        lcdc->REG_210 |= LCDC_VSYNC_TRIG_DISABLE_A;
    else
        lcdc->REG_210 &= ~LCDC_VSYNC_TRIG_DISABLE_A;
    LCDC_DBG("%s: avsync %s", __func__, disable ? "disabled" : "enabled");
    return LCDC_OK;
}

int hal_lcdc_disp_svsync_trigger_disable(bool disable)
{
    if (disable)
        lcdc->REG_210 |= LCDC_VSYNC_TRIG_DISABLE_S;
    else
        lcdc->REG_210 &= ~LCDC_VSYNC_TRIG_DISABLE_S;
    LCDC_DBG("%s: svsync %s", __func__, disable ? "disabled" : "enabled");
    return LCDC_OK;
}

int hal_lcdc_disp_mem_enable(bool enable)
{
    if (enable)
        lcdc->REG_210 |= LCDC_DSI_RD_MEM;
    else
        lcdc->REG_210 &= ~LCDC_DSI_RD_MEM;
    LCDC_DBG("%s: disp mem %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_disp_extra_delay(uint16_t delay)
{
    lcdc->REG_210 = SET_BITFIELD(lcdc->REG_210, LCDC_EXTRA_DELAY, delay);
    LCDC_DBG("%s: extra delay 0x%x", __func__, delay);
    return LCDC_OK;
}

int hal_lcdc_ol_dmaburst_length_set(uint8_t len)
{
    lcdc->REG_200 = SET_BITFIELD(lcdc->REG_200, LCDC_WDMA_BURST_LEN, len);
    lcdc->REG_20C &= ~LCDC_SPLIT_WLAST_MASK;
    lcdc->REG_20C |= LCDC_WR_FAST_ENA;// | LCDC_SPLIT_WLAST_MASK;
    LCDC_DBG("%s: wb layer burst length 0x%x", __func__, len);
    return LCDC_OK;
}

int hal_lcdc_smpn_mode_set(int mode)
{
    lcdc->REG_188 &= ~(LCDC_SLV_ENA | LCDC_SMPNMODE_MASK);
    if (lcdc_video_mode)
        lcdc->REG_188 |= LCDC_SLV_ENA | LCDC_SMPNMODE(mode);
    else
        lcdc->REG_188 |= LCDC_SMPNMODE(mode);
    LCDC_DBG("%s: smpn mode 0x%x", __func__, mode);
    return LCDC_OK;
}

int hal_lcdc_irq_disable(void)
{
    lcdc->REG_1C0 = 0;
    LCDC_DBG("%s: all irq disabled", __func__);
    return LCDC_OK;
}

uint32_t hal_lcdc_get_irqstat(void)
{
    return lcdc->REG_1C4;
}

int hal_lcdc_framedone_irq_enable(bool enable)
{
    if (enable)
        lcdc->REG_1C0 |= (LCDC_DUMB_FRAMEDONE | LCDC_TXC_FRAMEDONE | LCDC_SPI_FRAMEDONE);
    else
        lcdc->REG_1C0 &= ~(LCDC_DUMB_FRAMEDONE | LCDC_TXC_FRAMEDONE | LCDC_SPI_FRAMEDONE);
    LCDC_DBG("%s: framedone irq %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_vsync_irq_enable(bool enable)
{
    if (!lcdc_video_mode) {
        LCDC_DBG("%s: no vsync irq for command mode", __func__);
        return LCDC_OK;
    }

    if (enable)
        lcdc->REG_1C0 |= LCDC_VSYNC_IRQ_ENA;
    else
        lcdc->REG_1C0 &= ~LCDC_VSYNC_IRQ_ENA;
    LCDC_DBG("%s: vsync irq %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_framedone_irqstat_clr(void)
{
    lcdc->REG_1C4 &= ~(LCDC_DUMB_FRAMEDONE | LCDC_TXC_FRAMEDONE | LCDC_SPI_FRAMEDONE |
                       LCDC_LAYER_FRAME_IRQS);
    LCDC_DBG("%s: framedone irq clear", __func__);
    return LCDC_OK;
}

int hal_lcdc_vsync_irqstat_clr(void)
{
    lcdc->REG_1C4 &= ~LCDC_PN_VSYNC_IRQ;
    LCDC_DBG("%s: vsync irq clear", __func__);
    return LCDC_OK;
}

bool hal_lcdc_irq_is_framedone(uint32_t regval)
{
    if (lcdc_video_mode)
        return !!((regval & LCDC_DUMB_FRAMEDONE) && (regval & LCDC_LAYER_FRAME_IRQS));
    else
        return !!((regval & LCDC_TXC_FRAMEDONE) && (regval & LCDC_LAYER_FRAME_IRQS));
}

bool hal_lcdc_irq_is_hwc_framedone(uint32_t regval)
{
    return !!((regval & LCDC_SPI_FRAMEDONE) && (regval & LCDC_LAYER_FRAME_IRQS));
}

bool hal_lcdc_irq_is_vsync(uint32_t regval)
{
    return !!(regval & LCDC_PN_VSYNC_IRQ);
}

bool hal_lcdc_irq_is_framedone_irq0(uint32_t regval)
{
    return !!(regval & LCDC_FRAME_0_IRQS);
}

bool hal_lcdc_irq_is_framedone_irq1(uint32_t regval)
{
    return !!(regval & LCDC_FRAME_1_IRQS);
}

bool hal_lcdc_irq_is_error(uint32_t regval)
{
    return !!(regval & LCDC_ERROR_IRQS);
}

uint32_t hal_lcdc_irq_get_error(uint32_t regval)
{
    return regval & LCDC_ERROR_IRQS;
}

int hal_lcdc_error_irq_enable(void)
{
    lcdc->REG_1C0 |= LCDC_ERROR_IRQS;
    LCDC_DBG("%s: error & underflow irq enabled", __func__);
    return LCDC_OK;
}

int hal_lcdc_error_irqstat_clr(void)
{
    lcdc->REG_1C4 &= ~LCDC_ERROR_IRQS;
    LCDC_DBG("%s: error irq clear", __func__);
    return LCDC_OK;
}

int hal_lcdc_disp_pn_reset(void)
{
    hal_cmu_lcdc_disp_pn_reset();
    return LCDC_OK;
}

int hal_lcdc_smpn_enable(void)
{
    lcdc->REG_188 |= LCDC_SMPN_ENA;
    LCDC_DBG("%s: smpn enabled", __func__);
    return LCDC_OK;
}

int hal_lcdc_smpn_vsync_delaymode_set(uint8_t delay)
{
    lcdc->REG_188 &= ~LCDC_SMPNVSYNC_MASK;
    lcdc->REG_188 |= LCDC_SMPNVSYNC(delay);
    LCDC_DBG("%s: smpn vsync mode 0x%x", __func__, delay);
    return LCDC_OK;
}

uint32_t hal_lcdc_smpn_vsync_delaymode_get(void)
{
    return GET_BITFIELD(lcdc->REG_188, LCDC_SMPNVSYNC);
}

int hal_lcdc_gen_frame_enable(bool enable)
{
    if (enable)
        lcdc->REG_210 |= LCDC_REG_GEN_FRAME;
    else
        lcdc->REG_210 &= ~LCDC_REG_GEN_FRAME;
    LCDC_DBG("%s: gen frame %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_set_sync_line_pixs(uint32_t pixs)
{
    lcdc->REG_214 &= ~LCDC_HSYNC_LENGTH_MASK;
    lcdc->REG_214 |= LCDC_HSYNC_LENGTH(pixs);
    LCDC_DBG("%s: hsync pixels %d", __func__, pixs);
    return LCDC_OK;
}

int hal_lcdc_set_sync_lines(uint32_t lines)
{
    lcdc->REG_214 &= ~LCDC_DEL_NLINE_MASK;
    lcdc->REG_214 |= LCDC_DEL_NLINE(lines);
    LCDC_DBG("%s: vsync lines %d", __func__, lines);
    return LCDC_OK;
}

uint32_t hal_lcdc_get_sync_delay_pixs(void)
{
    return GET_BITFIELD(lcdc->REG_214, LCDC_HSYNC_LENGTH);
}

uint32_t hal_lcdc_get_sync_delay_lines(void)
{
    return GET_BITFIELD(lcdc->REG_214, LCDC_DEL_NLINE);
}

int hal_lcdc_start_wb(void)
{
    hal_lcdc_gen_frame_enable(true);
    return hal_lcdc_gen_start();
}

int hal_lcdc_gen_start(void)
{
    lcdc->REG_224 = LCDC_GEN_FRAME_START(1);
    LCDC_DBG("%s: frame transfer start", __func__);
    return LCDC_OK;
}

int hal_lcdc_frame_halt(void)
{
    int h = GET_BITFIELD(lcdc->REG_114, LCDC_PN_H_TOTAL) - 15;
    int v = GET_BITFIELD(lcdc->REG_114, LCDC_PN_V_TOTAL) - 15;
    lcdc->REG_720 = SET_BITFIELD(lcdc->REG_720, LCDC_H_SWITCH_AP_LP, h);
    lcdc->REG_720 = SET_BITFIELD(lcdc->REG_720, LCDC_V_SWITCH_AP_LP, v);
    lcdc->REG_228 |= LCDC_FRAME_HALT_EN;
    lcdc->REG_26C |= LCDC_FRAME_HALT_FROM_LPAP;
    is_lcdc_swtich_startd = false;
    return LCDC_OK;
}

int hal_lcdc_switch_start(void)
{
    int h = GET_BITFIELD(lcdc->REG_114, LCDC_PN_H_TOTAL) - 4;
    int v = GET_BITFIELD(lcdc->REG_118, LCDC_PN_V_ACTIVE);
    lcdc->REG_5DC = SET_BITFIELD(lcdc->REG_5DC, LCDC_V_SWITCH_START, v);
    lcdc->REG_5DC = SET_BITFIELD(lcdc->REG_5DC, LCDC_H_SWITCH_START, h);
    lcdc->REG_5DC |= LCDC_GEN_SWITCH_START_SEL;
    lcdc->REG_5D8 = LCDC_SWITCH_FRAME_START(1);
    LCDC_DBG("%s: frame switch start", __func__);
    is_lcdc_swtich_startd = true;
    return LCDC_OK;
}

bool hal_lcdc_is_switch_started(void)
{
    return is_lcdc_swtich_startd;
}

int hal_lcdc_start_dsi(void)
{
    hal_lcdc_gen_frame_enable(false);
    if (!lcdc_video_mode)
        hal_lcdc_smpn_enable();

    lcdc->REG_200 = 0; /* disable writeback */

    lcdc->REG_254 |= LCDC_TE_USE_SW;
    lcdc->REG_258 = 0;

    LCDC_DBG("%s: start dsi display", __func__);
    return LCDC_OK;
}

int hal_lcdc_te_trigger(void)
{
    lcdc->REG_258 = 0;
    LCDC_DBG("%s: clear reg 258", __func__);
    return LCDC_OK;
}

int hal_lcdc_vsync_porch_set(void)
{
    lcdc->REG_218 &= ~(LCDC_STBY_LENGTH_MASK | LCDC_VSYNC_START_LINE_MASK | LCDC_VSYNC_END_LINE_MASK);
#ifdef FPGA
    lcdc->REG_218 |= LCDC_STBY_LENGTH(0x200) | LCDC_VSYNC_START_LINE(1) | LCDC_VSYNC_END_LINE(5);
#else
    lcdc->REG_218 |= LCDC_STBY_LENGTH(0x80) | LCDC_VSYNC_START_LINE(1) | LCDC_VSYNC_END_LINE(5);
#endif
    lcdc->REG_21C = LCDC_H_FRONT_PORCH_SMPN(0x20) | LCDC_H_BACK_PORCH_SMPN(0x20) |
                    LCDC_VSYNC_NLINE(0xa);
    lcdc->REG_220 = LCDC_VSYNC_START_PIX(0x1e) | LCDC_VSYNC_END_PIX(0x1f);
    LCDC_DBG("%s:", __func__);
    return LCDC_OK;
}

int hal_lcdc_wdma_1fpt_set(void)
{
    lcdc->REG_204 = SET_BITFIELD(lcdc->REG_204, LCDC_WDMA_FPT_SET, 1);
    LCDC_DBG("%s:", __func__);
    return LCDC_OK;
}

int hal_lcdc_wdma_toggle_frame(void)
{
    static bool val = 1;
    lcdc->REG_204 = SET_BITFIELD(lcdc->REG_204, LCDC_WDMA_FPT_SET, val);
    LCDC_INFO("%s: %d", __func__, val);
    val = !val;
    return LCDC_OK;
}

int hal_lcdc_select_top(uint8_t lid)
{
    if (lid == LCDC_LAYER_GRA) {
        lcdc->REG_1E8 = SET_BITFIELD(lcdc->REG_1E8, LCDC_OVTOP, 0);
    } else {
        lcdc->REG_1E8 = SET_BITFIELD(lcdc->REG_1E8, LCDC_OVTOP, 1);
    }
    LCDC_DBG("%s: layer %d on top", __func__, lid);
    return LCDC_OK;
}

int hal_lcdc_swap_path(uint8_t lid)
{
    switch (lid) {
        case LCDC_LAYER_GRA:
            lcdc->REG_1DC |= LCDC_PN_SWAPATH;
            break;
        case LCDC_LAYER_DMA:
            lcdc->REG_1DC &= ~LCDC_PN_SWAPATH;
            break;
        case LCDC_LAYER_TVG:
            lcdc->REG_1DC |= LCDC_TV_SWAPATH;
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_1DC &= ~LCDC_TV_SWAPATH;
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d, swap path set", __func__, lid);
    return LCDC_OK;
}

int hal_lcdc_swap_path_enable(bool enable)
{
    if (enable)
        lcdc->REG_1DC |= LCDC_PN_SWAPATH_ENA | LCDC_TV_SWAPATH_ENA;
    else
        lcdc->REG_1DC &= ~(LCDC_PN_SWAPATH_ENA | LCDC_TV_SWAPATH_ENA);
    LCDC_DBG("%s: swap path %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_set_vmirror_enable(uint8_t lid, bool enable)
{
    uint32_t v = 0;
    switch (lid) {
        case LCDC_LAYER_GRA:
            v = LCDC_GRA_VM_ENA;
            break;
        case LCDC_LAYER_DMA:
            v = LCDC_DMA_VM_ENA;
            break;
        case LCDC_LAYER_TVG:
        case LCDC_LAYER_DEC:
        case LCDC_LAYER_TILED:
            v = LCDC_TVG_VM_ENA;
            break;
        case LCDC_LAYER_TVD:
            v = LCDC_TVD_VM_ENA;
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    if (enable)
        lcdc->REG_1BC |= v;
    else
        lcdc->REG_1BC &= ~v;
    LCDC_DBG("%s: layer %d, vmirror %s", __func__, lid, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_set_hmirror_enable(bool enable, int width, int height)
{
#ifdef LCDC_HAS_HMIRROR
#if defined(CHIP_BEST2009)
    if (enable) {
        lcdc->REG_2D0 = LCDC_HMIRRO_ENA | LCDC_HMIRRO_H_PIXEL(width) |
                        LCDC_HMIRRO_V_LINE(height);
    } else {
        lcdc->REG_2D0 = 0;
    }
#else
    if (enable) {
        lcdc->REG_710 = LCDC_SPU_MIRROR_EN | LCDC_LINE_WIDTH_MIRROR(width) |
                        LCDC_LINE_NUM_MIRROR(height);
    } else {
        lcdc->REG_710 = 0;
    }
#endif
    LCDC_DBG("%s: horizontal mirror is %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
#else
    LCDC_DBG("%s: horizontal mirror is not supported.", __func__);
    return LCDC_NOT_SUPPORT;
#endif
}

int hal_lcdc_disp_color_fmt(enum hal_lcdc_dsi_cfmt_e fmt)
{
    lcdc->REG_228 &= ~(LCDC_DSI_RGB888 | LCDC_DSI_RGB666 | LCDC_DSI_RGB565 | LCDC_DSI_RGB101010);
    switch (fmt) {
        case DSI_RGB888:
            lcdc->REG_228 |= LCDC_DSI_RGB888;
            break;
        case DSI_RGB666:
            lcdc->REG_228 |= LCDC_DSI_RGB666;
            break;
        case DSI_RGB565:
            lcdc->REG_228 |= LCDC_DSI_RGB565;
            break;
        case DSI_RGB101010:
            lcdc->REG_228 |= LCDC_DSI_RGB101010;
            break;
        default:
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: dsi color format %d", __func__, fmt);
    return LCDC_OK;
}

int hal_lcdc_disp_dsi_data_swap(enum hal_lcdc_dsi_dswap_e order)
{
    lcdc->REG_228 &= ~LCDC_DSI_DATA_SWAP_MASK;
    lcdc->REG_228 |= LCDC_DSI_DATA_SWAP(order);
    LCDC_DBG("%s: dsi color order %d", __func__, order);
    return LCDC_OK;
}

int hal_lcdc_sl_dmaburst_length_set(enum hal_lcdc_burst_len_e burst)
{
    lcdc->REG_1DC &= ~(LCDC_DMA_BURST0TO3_MASK | LCDC_GRA_BURST0TO3_MASK |
                       LCDC_TVG_BURST0TO3_MASK | LCDC_TVD_BURST0TO3_MASK);
    lcdc->REG_1DC |= LCDC_DMA_BURST0TO3(burst) | LCDC_GRA_BURST0TO3(burst) |
                     LCDC_TVG_BURST0TO3(burst) | LCDC_TVD_BURST0TO3(burst);

    lcdc->REG_63C &= ~(LCDC_SMT_BURST0TO3_MASK);
    lcdc->REG_63C |= LCDC_SMT_BURST0TO3(burst);

    lcdc->REG_054 &= ~(LCDC_BURSTSIZE_64_128_SEL_64);
    if (burst == LCDC_BURST_64B)
        lcdc->REG_054 |= LCDC_BURSTSIZE_64_128_SEL_64;
    LCDC_DBG("%s: burst len %d for all layers", __func__, burst);
    return LCDC_OK;
}

int hal_lcdc_sleep(void)
{
    hal_cmu_lcdc_sleep();
    return LCDC_OK;
}

int hal_lcdc_wakeup(void)
{
    hal_cmu_lcdc_wakeup();
    return LCDC_OK;
}

int hal_lcdc_dec_set_color_format(enum hal_lcdc_dec_format_e cfmt)
{
    uint32_t v = lcdc->REG_2D0;
    uint8_t m = GET_BITFIELD(v, LCDC_DEC_COMPRESS_MODE);

    v &= ~(LCDC_DEC_FORMAT_MASK | LCDC_DEC_BURST_LEN_SEL_MASK);

    if (cfmt == LCDC_DEC_XRGB) {
        v |= LCDC_DEC_FORMAT(1);
        if (m == 2)
            v |= LCDC_DEC_BURST_LEN_SEL(0); /* XRGB + HV_SAMPLE: burst 16 bytes */
        else if (m == 1)
            v |= LCDC_DEC_BURST_LEN_SEL(1); /* XRGB + H_SAMPLE:  burst 24 bytes */
        else if (m == 0)
            v |= LCDC_DEC_BURST_LEN_SEL(2); /* XRGB + HV_SAMPLE: burst 32 bytes */
    } else { /* ARGB */
        v |= LCDC_DEC_FORMAT(0);
        if (m == 2)
            v |= LCDC_DEC_BURST_LEN_SEL(1); /* XRGB + HV_SAMPLE: burst 24 bytes */
        else if (m == 1)
            v |= LCDC_DEC_BURST_LEN_SEL(2); /* XRGB + H_SAMPLE:  burst 32 bytes */
        else if (m == 0)
            v |= LCDC_DEC_BURST_LEN_SEL(3); /* XRGB + HV_SAMPLE: burst 40 bytes */
    }

    lcdc->REG_2D0 = v;
    LCDC_DBG("%s: decnano color format %d", __func__, cfmt);
    return LCDC_OK;
}

int hal_lcdc_dec_set_size(uint16_t w, uint16_t h)
{
    lcdc->REG_2D4 = LCDC_DEC_BUF_H_PIXEL(w) | LCDC_DEC_BUF_V_LINE(h) |
                    LCDC_DEC_UNDERFLOW_START_CNT(0xf);
    LCDC_DBG("%s: decnano buffer size %dx%d", __func__, w, h);
    return LCDC_OK;
}

int hal_lcdc_dec_set_tile_mode(enum hal_lcdc_dec_tile_mode_e mode)
{
    lcdc->REG_2D0 &= ~LCDC_DEC_TILE_MODE_MASK;
    lcdc->REG_2D0 |= LCDC_DEC_TILE_MODE(mode);
    LCDC_DBG("%s: decnano tile mode %d", __func__, mode);
    return LCDC_OK;
}

int hal_lcdc_dec_set_compress_mode(enum hal_lcdc_dec_compress_mode_e comp)
{
    uint32_t v = lcdc->REG_2D0;
    uint8_t c = GET_BITFIELD(v, LCDC_DEC_FORMAT);

    v &= ~(LCDC_DEC_COMPRESS_MODE_MASK | LCDC_DEC_BURST_LEN_SEL_MASK);

    if (comp == LCDC_DEC_HV_SAMPLE) {
        v |= LCDC_DEC_COMPRESS_MODE(2);
        v |= LCDC_DEC_BURST_LEN_SEL(c ? 0 : 1); /* burst 16 (XRGB) or 24 (ARGB) bytes */
    } else if (comp == LCDC_DEC_H_SAMPLE) {
        v |= LCDC_DEC_COMPRESS_MODE(1);
        v |= LCDC_DEC_BURST_LEN_SEL(c ? 1 : 2); /* burst 24 (XRGB) or 32 (ARGB) bytes */
    } else if (comp == LCDC_DEC_NON_SAMPLE) {
        v |= LCDC_DEC_COMPRESS_MODE(0);
        v |= LCDC_DEC_BURST_LEN_SEL(c ? 2 : 3); /* burst 32 (XRGB) or 40 (ARGB) bytes */
    } else { /* enable bypass mode */
        v |= LCDC_DEC_BYPASS;
        v |= LCDC_DEC_BURST_LEN_SEL(0);
    }

    lcdc->REG_2D0 = v;
    LCDC_DBG("%s: decnano compress mode %d", __func__, comp);
    return LCDC_OK;
}

int hal_lcdc_dec_set_data_delay(uint8_t delay)
{
    lcdc->REG_2D0 = SET_BITFIELD(lcdc->REG_2D0, LCDC_DEC_DATA_DELAY, delay);
    LCDC_DBG("%s: decnano data delay %d", __func__, delay);
    return LCDC_OK;
}

int hal_lcdc_dec_swap_rb(bool enable)
{
    if (enable)
        lcdc->REG_2D0 |= LCDC_DEC_SWAP_RB;
    else
        lcdc->REG_2D0 &= ~LCDC_DEC_SWAP_RB;
    LCDC_DBG("%s: decnano swap RB %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_tiled_set_size(uint16_t w, uint16_t h)
{
    lcdc->REG_330 = LCDC_TILED_BUF_H_PIXEL(w) | LCDC_TILED_BUF_V_LINE(h) |
                    LCDC_TILED_UNDERFLOW_START_CNT(0xf);
    LCDC_DBG("%s: tiled buffer size %dx%d", __func__, w, h);
    return LCDC_OK;
}

int hal_lcdc_tiled_set_format(enum hal_lcdc_tiled_format_e format)
{
    uint32_t v = lcdc->REG_32C;

    v &= ~(LCDC_TILED_32B | LCDC_TILED_24B | LCDC_TILED_16B | LCDC_TILED_BURST_LEN_MASK |
           LCDC_TILED_FORMAT_MASK);

#ifdef LCDC_HAS_AXI
    v |= LCDC_TILED_REQBURST;
#endif
    v |= LCDC_TILED_LINE_END_MASK | LCDC_TILED_USE_QSPI;

    if (format == LCDC_TILED_FMT_16BIT) {
        v |= LCDC_TILED_16B | LCDC_TILED_BURST_LEN(2) | LCDC_TILED_FORMAT(2);
    } else if (format == LCDC_TILED_FMT_24BIT) {
        v |= LCDC_TILED_24B | LCDC_TILED_BURST_LEN(1) | LCDC_TILED_FORMAT(1);
    } else {
        v |= LCDC_TILED_32B | LCDC_TILED_BURST_LEN(6) | LCDC_TILED_FORMAT(0);
    }

    lcdc->REG_32C = v;
    LCDC_DBG("%s: set tiled format %d", __func__, format);
    return LCDC_OK;
}

int hal_lcdc_tiled_swap_rb(bool enable)
{
    if (enable)
        lcdc->REG_32C |= LCDC_TILED_SWAP_RB;
    else
        lcdc->REG_32C &= ~LCDC_TILED_SWAP_RB;
    LCDC_DBG("%s: tiled layer swap RB %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_dsi0_layer_select(uint8_t layermask)
{
    uint32_t v1 = lcdc->REG_2C0;
    uint32_t v2 = lcdc->REG_2C8;

    /* dsi0 always uses data0 */
    v1 &= ~(LCDC_DSI0_DATA1_SEL | LCDC_GRA_IN_VLD_STEREO_L | LCDC_DMA_IN_VLD_STEREO_L |
            LCDC_TVG_IN_VLD_STEREO_L);
    v2 &= ~(LCDC_TVD_IN_VLD_STEREO_L | LCDC_SMT_IN_VLD_STEREO_L);

    if (layermask & (1 << LCDC_LAYER_GRA)) {
        v1 |= LCDC_GRA_IN_VLD_STEREO_L;
    }
    if (layermask & (1 << LCDC_LAYER_DMA)) {
        v1 |= LCDC_DMA_IN_VLD_STEREO_L;
    }
    if (layermask & (1 << LCDC_LAYER_TVG)) {
        v1 |= LCDC_TVG_IN_VLD_STEREO_L;
    }
    if (layermask & (1 << LCDC_LAYER_TVD)) {
        v2 |= LCDC_TVD_IN_VLD_STEREO_L;
    }
    if (layermask & (1 << LCDC_LAYER_SMT)) {
        v2 |= LCDC_SMT_IN_VLD_STEREO_L;
    }

    lcdc->REG_2C0 = v1;
    lcdc->REG_2C8 = v2;
    lcdc->REG_720 |= 0x1f;
    LCDC_DBG("%s: DSI0 selected layer mask: %x", __func__, layermask);
    return LCDC_OK;
}

int hal_lcdc_blend(int blend_mode, int alpha_mode, uint8_t alpha)
{
#ifdef LCDC_HAS_NEWALG_BLEND
    alpha_mode = alpha_mode;
    if (blend_mode == 0)
        blend_mode = LCDC_DRM_BLEND_MODE_DEFAULT;

    for (int lid = LCDC_LAYER_GRA; lid <= LCDC_LAYER_SMT; lid++)
    {
        hal_lcdc_blend_drm(0, lid, blend_mode, alpha);
    }
#else
    lcdc->REG_1E8 &= ~LCDC_OVTOP_MASK;
    lcdc->REG_1E8 |= LCDC_OVTOP(blend_mode);

    if (alpha_mode == LCDC_CONST_ALPHA) {
        lcdc->REG_194 &= ~(LCDC_PN_ALPHA_PIXEL_MODE | LCDC_PN_ALPHA_MASK);
        lcdc->REG_194 |= LCDC_PN_ALPHA(alpha);
        lcdc->REG_084 &= ~(LCDC_TV_ALPHA_PIXEL_MODE | LCDC_TV_ALPHA_MASK);
        lcdc->REG_084 |= LCDC_TV_ALPHA(255 - alpha);
    } else {
        lcdc->REG_194 |= LCDC_PN_ALPHA_PIXEL_MODE;
        lcdc->REG_084 |= LCDC_TV_ALPHA_PIXEL_MODE;
    }

#endif
    LCDC_DBG("%s: blend mode %d, alpha mode %d, const alpha 0x%02x",
            __func__, blend_mode, alpha_mode, alpha);
    return LCDC_OK;
}

int hal_lcdc_layer_alpha(uint8_t bus_port, uint8_t lid, enum hal_lcdc_alpha_mode_e mode, uint8_t alpha)
{
    __IO uint32_t *reg_a;
    uint32_t reg_v;
    uint8_t port_shift;
    if (bus_port == 1) {
        reg_a = &lcdc->REG_708;
        reg_v = lcdc->REG_708;
        port_shift = 5;
    }
    else {
        reg_a = &lcdc->REG_704;
        reg_v = lcdc->REG_704;
        port_shift = 0;
    }

    if (alpha_blend_use_new_alg) {
        if (lid == LCDC_LAYER_SMT) {
            if (bus_port == 1)
                lcdc->REG_70C = SET_BITFIELD(lcdc->REG_70C, LCDC_ALPHA_SMT_NEW_ALG_GLOBAL_R, alpha);
            else
                lcdc->REG_70C = SET_BITFIELD(lcdc->REG_70C, LCDC_ALPHA_SMT_NEW_ALG_GLOBAL_L, alpha);
            if (mode == LCDC_CONST_ALPHA)
                lcdc->REG_700 |= LCDC_ALPHA_SMT_NEW_ALG_SEL_GLOBAL_L << port_shift;
            else
                lcdc->REG_700 &= ~(LCDC_ALPHA_SMT_NEW_ALG_SEL_GLOBAL_L << port_shift);
        } else {
            if (lid == LCDC_LAYER_DMA) {
                reg_v = SET_BITFIELD(reg_v, LCDC_ALPHA_DMA_NEW_ALG_GLOBAL_L, alpha);
                if (mode == LCDC_CONST_ALPHA)
                    lcdc->REG_700 |= LCDC_ALPHA_DMA_NEW_ALG_SEL_GLOBAL_L << port_shift;
                else
                    lcdc->REG_700 &= ~(LCDC_ALPHA_DMA_NEW_ALG_SEL_GLOBAL_L << port_shift);
            } else if (lid == LCDC_LAYER_GRA) {
                reg_v = SET_BITFIELD(reg_v, LCDC_ALPHA_GRA_NEW_ALG_GLOBAL_L, alpha);
                if (mode == LCDC_CONST_ALPHA)
                    lcdc->REG_700 |= LCDC_ALPHA_GRA_NEW_ALG_SEL_GLOBAL_L << port_shift ;
                else
                    lcdc->REG_700 &= ~(LCDC_ALPHA_GRA_NEW_ALG_SEL_GLOBAL_L << port_shift);
            } else if (lid == LCDC_LAYER_TVG) {
                reg_v = SET_BITFIELD(reg_v, LCDC_ALPHA_TVG_NEW_ALG_GLOBAL_L, alpha);
                if (mode == LCDC_CONST_ALPHA)
                    lcdc->REG_700 |= LCDC_ALPHA_TVG_NEW_ALG_SEL_GLOBAL_L << port_shift;
                else
                    lcdc->REG_700 &= ~(LCDC_ALPHA_TVG_NEW_ALG_SEL_GLOBAL_L << port_shift);
            } else if (lid == LCDC_LAYER_TVD) {
                reg_v = SET_BITFIELD(reg_v, LCDC_ALPHA_TVD_NEW_ALG_GLOBAL_L, alpha);
                if (mode == LCDC_CONST_ALPHA)
                    lcdc->REG_700 |= LCDC_ALPHA_TVD_NEW_ALG_SEL_GLOBAL_L << port_shift;
                else
                    lcdc->REG_700 &= ~(LCDC_ALPHA_TVD_NEW_ALG_SEL_GLOBAL_L << port_shift);
            }
            else {
                LCDC_INFO("%s: invalid layer id %d", __func__, lid);
                return LCDC_INVAL;
            }
            *reg_a = reg_v;
        }
    }
    LCDC_DBG("%s:lid %d alpha mode %d, alpha value 0x%02x", __func__, lid, mode, alpha);
    return LCDC_OK;
}

int hal_lcdc_blend_first(uint8_t bus_port, enum hal_lcdc_pix_sel_mode_e both_sel, enum hal_lcdc_alpha_sel_mode_e a_s, \
    enum hal_lcdc_alpha_sel_mode_e a_d, enum hal_lcdc_alpha_sel_mode_e as_o, enum hal_lcdc_alpha_sel_mode_e ad_o, \
    uint8_t o_alpha_mask)
{
    if (bus_port == 0) {
        lcdc->REG_5B0 = SET_BITFIELD(lcdc->REG_5B0, LCDC_SRC_ALPHA_SEL_FIRST_L, a_s);
        lcdc->REG_5B0 = SET_BITFIELD(lcdc->REG_5B0, LCDC_DST_ALPHA_SEL_FIRST_L, a_d);
        lcdc->REG_5B0 = SET_BITFIELD(lcdc->REG_5B0, LCDC_BOTH_PIX_SEL_FIRST_L, both_sel);
        lcdc->REG_5B0 &= ~(LCDC_SRC_PIX_SEL_FIRST_L | LCDC_DST_PIX_SEL_FIRST_L |
            LCDC_ALPHA_SRC_VLD_FIRST_L | LCDC_ALPHA_DST_VLD_FIRST_L | LCDC_ALPHA_BOTH_VLD_FIRST_L);

        lcdc->REG_5D4  &= ~(LCDC_SRC_ALPHA_SEL_OUT_FIRST_L(0x3) | LCDC_DST_ALPHA_SEL_OUT_FIRST_L(0x3));
        lcdc->REG_5D4  |= LCDC_SRC_ALPHA_SEL_OUT_FIRST_L(as_o) | LCDC_DST_ALPHA_SEL_OUT_FIRST_L(ad_o);
        if (o_alpha_mask & (1 << 0))
            lcdc->REG_5B0  |= LCDC_ALPHA_BOTH_VLD_FIRST_L;
        if (o_alpha_mask & (1 << 1))
            lcdc->REG_5B0  |= LCDC_ALPHA_DST_VLD_FIRST_L;
        if (o_alpha_mask & (1 << 2))
            lcdc->REG_5B0  |= LCDC_ALPHA_SRC_VLD_FIRST_L;

        if (both_sel == LCDC_BOTH_SEL_DST_PIXEL)
            lcdc->REG_70C |= LCDC_PRE_COV_SEL_INVERT_FIRST_L;
        else
            lcdc->REG_70C &= ~LCDC_PRE_COV_SEL_INVERT_FIRST_L;
    } else if (bus_port == 1) {
        lcdc->REG_5B8 = SET_BITFIELD(lcdc->REG_5B8, LCDC_SRC_ALPHA_SEL_FIRST_R, a_s);
        lcdc->REG_5B8 = SET_BITFIELD(lcdc->REG_5B8, LCDC_DST_ALPHA_SEL_FIRST_R, a_d);
        lcdc->REG_5B8 = SET_BITFIELD(lcdc->REG_5B8, LCDC_BOTH_PIX_SEL_FIRST_R, both_sel);
        lcdc->REG_5B8 &= ~(LCDC_SRC_PIX_SEL_FIRST_R | LCDC_DST_PIX_SEL_FIRST_R |
            LCDC_ALPHA_SRC_VLD_FIRST_R | LCDC_ALPHA_DST_VLD_FIRST_R | LCDC_ALPHA_BOTH_VLD_FIRST_R);

        lcdc->REG_5D4  &= ~(LCDC_SRC_ALPHA_SEL_OUT_FIRST_R(0x3) | LCDC_DST_ALPHA_SEL_OUT_FIRST_R(0x3));
        lcdc->REG_5D4  |= LCDC_SRC_ALPHA_SEL_OUT_FIRST_R(as_o) | LCDC_DST_ALPHA_SEL_OUT_FIRST_R(ad_o);
        if (o_alpha_mask & (1 << 0))
            lcdc->REG_5B8  |= LCDC_ALPHA_BOTH_VLD_FIRST_R;
        if (o_alpha_mask & (1 << 1))
            lcdc->REG_5B8  |= LCDC_ALPHA_DST_VLD_FIRST_R;
        if (o_alpha_mask & (1 << 2))
            lcdc->REG_5B8  |= LCDC_ALPHA_SRC_VLD_FIRST_R;

        if (both_sel == LCDC_BOTH_SEL_DST_PIXEL)
            lcdc->REG_70C |= LCDC_PRE_COV_SEL_INVERT_FIRST_R;
        else
            lcdc->REG_70C &= ~LCDC_PRE_COV_SEL_INVERT_FIRST_R;
    } else {
        LCDC_INFO("%s: invalid usage, wrong busport:%d", __func__, bus_port);
        return LCDC_INVAL;
    }
    return LCDC_OK;
}

int hal_lcdc_blend_second(uint8_t bus_port, enum hal_lcdc_pix_sel_mode_e both_sel, enum hal_lcdc_alpha_sel_mode_e a_s, \
    enum hal_lcdc_alpha_sel_mode_e a_d, enum hal_lcdc_alpha_sel_mode_e as_o, enum hal_lcdc_alpha_sel_mode_e ad_o, \
    uint8_t o_alpha_mask)
{
    if (bus_port == 0) {
        lcdc->REG_5B0 = SET_BITFIELD(lcdc->REG_5B0, LCDC_SRC_ALPHA_SEL_SECOND_L, a_s);
        lcdc->REG_5B0 = SET_BITFIELD(lcdc->REG_5B0, LCDC_DST_ALPHA_SEL_SECOND_L, a_d);
        lcdc->REG_5B0 = SET_BITFIELD(lcdc->REG_5B0, LCDC_BOTH_PIX_SEL_SECOND_L, both_sel);
        lcdc->REG_5B0 &= ~(LCDC_SRC_PIX_SEL_SECOND_L | LCDC_DST_PIX_SEL_SECOND_L |
            LCDC_ALPHA_SRC_VLD_SECOND_L | LCDC_ALPHA_DST_VLD_SECOND_L | LCDC_ALPHA_BOTH_VLD_SECOND_L);

        lcdc->REG_5D4  &= ~(LCDC_SRC_ALPHA_SEL_OUT_SECOND_L(0x3)| LCDC_DST_ALPHA_SEL_OUT_SECOND_L(0x3));
        lcdc->REG_5D4  |= LCDC_SRC_ALPHA_SEL_OUT_SECOND_L(as_o) | LCDC_DST_ALPHA_SEL_OUT_SECOND_L(ad_o);
        if (o_alpha_mask & (1 << 0))
            lcdc->REG_5B0  |= LCDC_ALPHA_BOTH_VLD_SECOND_L;
        if (o_alpha_mask & (1 << 1))
            lcdc->REG_5B0  |= LCDC_ALPHA_DST_VLD_SECOND_L;
        if (o_alpha_mask & (1 << 2))
            lcdc->REG_5B0  |= LCDC_ALPHA_SRC_VLD_SECOND_L;

        if (both_sel == LCDC_BOTH_SEL_DST_PIXEL)
            lcdc->REG_70C |= LCDC_PRE_COV_SEL_INVERT_SECOND_L;
        else
            lcdc->REG_70C &= ~LCDC_PRE_COV_SEL_INVERT_SECOND_L;
    } else if (bus_port == 1) {
        lcdc->REG_5B8 = SET_BITFIELD(lcdc->REG_5B8, LCDC_SRC_ALPHA_SEL_SECOND_R, a_s);
        lcdc->REG_5B8 = SET_BITFIELD(lcdc->REG_5B8, LCDC_DST_ALPHA_SEL_SECOND_R, a_d);
        lcdc->REG_5B8 = SET_BITFIELD(lcdc->REG_5B8, LCDC_BOTH_PIX_SEL_SECOND_R, both_sel);
        lcdc->REG_5B8 &= ~(LCDC_SRC_PIX_SEL_SECOND_R | LCDC_DST_PIX_SEL_SECOND_R |
            LCDC_ALPHA_SRC_VLD_SECOND_R | LCDC_ALPHA_DST_VLD_SECOND_R | LCDC_ALPHA_BOTH_VLD_SECOND_R);

        lcdc->REG_5D4  &= ~(LCDC_SRC_ALPHA_SEL_OUT_SECOND_R(0x3)| LCDC_DST_ALPHA_SEL_OUT_SECOND_R(0x3));
        lcdc->REG_5D4  |= LCDC_SRC_ALPHA_SEL_OUT_SECOND_R(as_o) | LCDC_DST_ALPHA_SEL_OUT_SECOND_R(ad_o);
        if (o_alpha_mask & (1 << 0))
            lcdc->REG_5B8  |= LCDC_ALPHA_BOTH_VLD_SECOND_R;
        if (o_alpha_mask & (1 << 1))
            lcdc->REG_5B8  |= LCDC_ALPHA_DST_VLD_SECOND_R;
        if (o_alpha_mask & (1 << 2))
            lcdc->REG_5B8  |= LCDC_ALPHA_SRC_VLD_SECOND_R;

        if (both_sel == LCDC_BOTH_SEL_DST_PIXEL)
            lcdc->REG_70C |= LCDC_PRE_COV_SEL_INVERT_SECOND_R;
        else
            lcdc->REG_70C &= ~LCDC_PRE_COV_SEL_INVERT_SECOND_R;
    } else {
        LCDC_INFO("%s: invalid usage, wrong busport:%d", __func__, bus_port);
        return LCDC_INVAL;
    }
    return LCDC_OK;
}

int hal_lcdc_blend_third(uint8_t bus_port, enum hal_lcdc_pix_sel_mode_e both_sel, enum hal_lcdc_alpha_sel_mode_e a_s, \
    enum hal_lcdc_alpha_sel_mode_e a_d, enum hal_lcdc_alpha_sel_mode_e as_o, enum hal_lcdc_alpha_sel_mode_e ad_o, \
    uint8_t o_alpha_mask)
{
    if (bus_port == 0) {
        lcdc->REG_5B4 = SET_BITFIELD(lcdc->REG_5B4, LCDC_SRC_ALPHA_SEL_THIRD_L, a_s);
        lcdc->REG_5B4 = SET_BITFIELD(lcdc->REG_5B4, LCDC_DST_ALPHA_SEL_THIRD_L, a_d);
        lcdc->REG_5B4 = SET_BITFIELD(lcdc->REG_5B4, LCDC_BOTH_PIX_SEL_THIRD_L, both_sel);
        lcdc->REG_5B4 &= ~ (LCDC_SRC_PIX_SEL_THIRD_L | LCDC_DST_PIX_SEL_THIRD_L |
            LCDC_ALPHA_SRC_VLD_THIRD_L | LCDC_ALPHA_DST_VLD_THIRD_L | LCDC_ALPHA_BOTH_VLD_THIRD_L);

        lcdc->REG_5D4  &= ~(LCDC_SRC_ALPHA_SEL_OUT_THIRD_L(0x3)| LCDC_DST_ALPHA_SEL_OUT_THIRD_L(0x3));
        lcdc->REG_5D4  |= LCDC_SRC_ALPHA_SEL_OUT_THIRD_L(as_o) | LCDC_DST_ALPHA_SEL_OUT_THIRD_L(ad_o);
        if (o_alpha_mask & (1 << 0))
            lcdc->REG_5B4  |= LCDC_ALPHA_BOTH_VLD_THIRD_L;
        if (o_alpha_mask & (1 << 1))
            lcdc->REG_5B4  |= LCDC_ALPHA_DST_VLD_THIRD_L;
        if (o_alpha_mask & (1 << 2))
            lcdc->REG_5B4  |= LCDC_ALPHA_SRC_VLD_THIRD_L;

        if (both_sel == LCDC_BOTH_SEL_DST_PIXEL)
            lcdc->REG_70C |= LCDC_PRE_COV_SEL_INVERT_THIRD_L;
        else
            lcdc->REG_70C &= ~LCDC_PRE_COV_SEL_INVERT_THIRD_L;
    } else if (bus_port == 1) {
        lcdc->REG_5BC = SET_BITFIELD(lcdc->REG_5BC, LCDC_SRC_ALPHA_SEL_THIRD_R, a_s);
        lcdc->REG_5BC = SET_BITFIELD(lcdc->REG_5BC, LCDC_DST_ALPHA_SEL_THIRD_R, a_d);
        lcdc->REG_5BC = SET_BITFIELD(lcdc->REG_5BC, LCDC_BOTH_PIX_SEL_THIRD_R, both_sel);
        lcdc->REG_5BC &= ~ (LCDC_SRC_PIX_SEL_THIRD_R | LCDC_DST_PIX_SEL_THIRD_R |
            LCDC_ALPHA_SRC_VLD_THIRD_R | LCDC_ALPHA_DST_VLD_THIRD_R | LCDC_ALPHA_BOTH_VLD_THIRD_R);

        lcdc->REG_5D4  &= ~(LCDC_SRC_ALPHA_SEL_OUT_THIRD_R(0x3)| LCDC_DST_ALPHA_SEL_OUT_THIRD_R(0x3));
        lcdc->REG_5D4  |= LCDC_SRC_ALPHA_SEL_OUT_THIRD_R(as_o) | LCDC_DST_ALPHA_SEL_OUT_THIRD_R(ad_o);
        if (o_alpha_mask & (1 << 0))
            lcdc->REG_5BC  |= LCDC_ALPHA_BOTH_VLD_THIRD_R;
        if (o_alpha_mask & (1 << 1))
            lcdc->REG_5BC  |= LCDC_ALPHA_DST_VLD_THIRD_R;
        if (o_alpha_mask & (1 << 2))
            lcdc->REG_5BC  |= LCDC_ALPHA_SRC_VLD_THIRD_R;

        if (both_sel == LCDC_BOTH_SEL_DST_PIXEL)
            lcdc->REG_70C |= LCDC_PRE_COV_SEL_INVERT_THIRD_R;
        else
            lcdc->REG_70C &= ~LCDC_PRE_COV_SEL_INVERT_THIRD_R;
    } else {
        LCDC_INFO("%s: invalid usage, wrong busport:%d", __func__, bus_port);
        return LCDC_INVAL;
    }
    return LCDC_OK;
}

int hal_lcdc_blend_fourth(uint8_t bus_port, enum hal_lcdc_pix_sel_mode_e both_sel, enum hal_lcdc_alpha_sel_mode_e a_s, \
    enum hal_lcdc_alpha_sel_mode_e a_d, enum hal_lcdc_alpha_sel_mode_e as_o, enum hal_lcdc_alpha_sel_mode_e ad_o, \
    uint8_t o_alpha_mask)
{
    if (bus_port == 0) {
        lcdc->REG_5B4 = SET_BITFIELD(lcdc->REG_5B4, LCDC_SRC_ALPHA_SEL_FOURTH_L, a_s);
        lcdc->REG_5B4 = SET_BITFIELD(lcdc->REG_5B4, LCDC_DST_ALPHA_SEL_FOURTH_L, a_d);
        lcdc->REG_5B4 = SET_BITFIELD(lcdc->REG_5B4, LCDC_BOTH_PIX_SEL_FOURTH_L, both_sel);
        lcdc->REG_5B4 &= ~ (LCDC_SRC_PIX_SEL_FOURTH_L | LCDC_DST_PIX_SEL_FOURTH_L |
            LCDC_ALPHA_SRC_VLD_FOURTH_L | LCDC_ALPHA_DST_VLD_FOURTH_L | LCDC_ALPHA_BOTH_VLD_FOURTH_L);

        lcdc->REG_5D4  &= ~(LCDC_SRC_ALPHA_SEL_OUT_FOURTH_L(0x3)| LCDC_DST_ALPHA_SEL_OUT_FOURTH_L(0x3));
        lcdc->REG_5D4  |= LCDC_SRC_ALPHA_SEL_OUT_FOURTH_L(as_o) | LCDC_DST_ALPHA_SEL_OUT_FOURTH_L(ad_o);
        if (o_alpha_mask & (1 << 0))
            lcdc->REG_5B4  |= LCDC_ALPHA_BOTH_VLD_FOURTH_L;
        if (o_alpha_mask & (1 << 1))
            lcdc->REG_5B4  |= LCDC_ALPHA_DST_VLD_FOURTH_L;
        if (o_alpha_mask & (1 << 2))
            lcdc->REG_5B4  |= LCDC_ALPHA_SRC_VLD_FOURTH_L;

        if (both_sel == LCDC_BOTH_SEL_DST_PIXEL)
            lcdc->REG_70C |= LCDC_PRE_COV_SEL_INVERT_FOURTH_L;
        else
            lcdc->REG_70C &= ~LCDC_PRE_COV_SEL_INVERT_FOURTH_L;
    } else if (bus_port == 1) {
        lcdc->REG_5BC = SET_BITFIELD(lcdc->REG_5BC, LCDC_SRC_ALPHA_SEL_FOURTH_R, a_s);
        lcdc->REG_5BC = SET_BITFIELD(lcdc->REG_5BC, LCDC_DST_ALPHA_SEL_FOURTH_R, a_d);
        lcdc->REG_5BC = SET_BITFIELD(lcdc->REG_5BC, LCDC_BOTH_PIX_SEL_FOURTH_R, both_sel);
        lcdc->REG_5BC &= ~ (LCDC_SRC_PIX_SEL_FOURTH_R | LCDC_DST_PIX_SEL_FOURTH_R |
            LCDC_ALPHA_SRC_VLD_FOURTH_R | LCDC_ALPHA_DST_VLD_FOURTH_R | LCDC_ALPHA_BOTH_VLD_FOURTH_R);

        lcdc->REG_5D4  &= ~(LCDC_SRC_ALPHA_SEL_OUT_FOURTH_R(0x3)| LCDC_DST_ALPHA_SEL_OUT_FOURTH_R(0x3));
        lcdc->REG_5D4  |= LCDC_SRC_ALPHA_SEL_OUT_FOURTH_R(as_o) | LCDC_DST_ALPHA_SEL_OUT_FOURTH_R(ad_o);
        if (o_alpha_mask & (1 << 0))
            lcdc->REG_5BC  |= LCDC_ALPHA_BOTH_VLD_FOURTH_R;
        if (o_alpha_mask & (1 << 1))
            lcdc->REG_5BC  |= LCDC_ALPHA_DST_VLD_FOURTH_R;
        if (o_alpha_mask & (1 << 2))
            lcdc->REG_5BC  |= LCDC_ALPHA_SRC_VLD_FOURTH_R;

        if (both_sel == LCDC_BOTH_SEL_DST_PIXEL)
            lcdc->REG_70C |= LCDC_PRE_COV_SEL_INVERT_FOURTH_R;
        else
            lcdc->REG_70C &= ~LCDC_PRE_COV_SEL_INVERT_FOURTH_R;
    } else {
        LCDC_INFO("%s: invalid usage, wrong busport:%d", __func__, bus_port);
        return LCDC_INVAL;
    }
    return LCDC_OK;
}

int hal_lcdc_blend_nocompose(uint8_t bus_port, uint8_t lid)
{
#ifdef LCDC_HAS_NEWALG_BLEND
    if (lid > LCDC_LAYER_SMT) {
        LCDC_INFO("%s: invalid layer id %d", __func__, lid);
        return LCDC_INVAL;
    }
    hal_lcdc_blend_first(bus_port, LCDC_BOTH_SEL_SRC_PIXEL, LCDC_SEL_CONST_FF, LCDC_SEL_CONST_00, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_CONST_00, 5);
    hal_lcdc_blend_second(bus_port, LCDC_BOTH_SEL_SRC_PIXEL, LCDC_SEL_CONST_FF, LCDC_SEL_CONST_00, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_CONST_00, 5);
    hal_lcdc_blend_third(bus_port, LCDC_BOTH_SEL_SRC_PIXEL, LCDC_SEL_CONST_FF, LCDC_SEL_CONST_00, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_CONST_00, 5);
    hal_lcdc_blend_fourth(bus_port, LCDC_BOTH_SEL_SRC_PIXEL, LCDC_SEL_CONST_FF, LCDC_SEL_CONST_00, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_CONST_00, 5);
    if (lid == LCDC_LAYER_DMA)
        hal_lcdc_blend_first(bus_port, LCDC_BOTH_SEL_SRC_PIXEL, LCDC_SEL_CONST_00, LCDC_SEL_CONST_FF, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_PIXEL_ALPHA, 3);
    else if (lid == LCDC_LAYER_TVG)
        hal_lcdc_blend_second(bus_port, LCDC_BOTH_SEL_SRC_PIXEL, LCDC_SEL_CONST_00, LCDC_SEL_CONST_FF, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_PIXEL_ALPHA, 3);
    else if (lid == LCDC_LAYER_TVD)
        hal_lcdc_blend_third(bus_port, LCDC_BOTH_SEL_SRC_PIXEL, LCDC_SEL_CONST_00, LCDC_SEL_CONST_FF, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_PIXEL_ALPHA, 3);
    else if (lid == LCDC_LAYER_SMT)
        hal_lcdc_blend_fourth(bus_port, LCDC_BOTH_SEL_SRC_PIXEL, LCDC_SEL_CONST_00, LCDC_SEL_CONST_FF, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_PIXEL_ALPHA, 3);
    lcdc->REG_5D0 = 0;
#endif
    return LCDC_OK;
}

int hal_lcdc_zorder_set(uint8_t bus_port, uint8_t zorder)
{
#ifdef LCDC_HAS_NEWALG_BLEND
    lcdc->REG_5C0 |= LCDC_ALPHA_BLEND_USE_NEW_ALG;
    if (bus_port == 1)
        lcdc->REG_5C0 = SET_BITFIELD(lcdc->REG_5C0, LCDC_ALPHA_BLEND_Z_ORDER_R, zorder);
    else
        lcdc->REG_5C0 = SET_BITFIELD(lcdc->REG_5C0, LCDC_ALPHA_BLEND_Z_ORDER_L, zorder);
    alpha_blend_use_new_alg = true;
    LCDC_INFO("%s: bus_port %d, zorder %d", __func__, bus_port, zorder);
#endif
    return LCDC_OK;
}

int hal_lcdc_blend_alpha(uint8_t bus_port, uint8_t zorder_id, enum hal_lcdc_drm_blend_mode_e drm_blend_mode, uint8_t alpha)
{
    __IO uint32_t *reg_a;
    uint32_t reg_v;
    uint8_t port_shift;
    if (bus_port == 1) {
        reg_a = &lcdc->REG_5CC;
        reg_v = lcdc->REG_5CC;
        port_shift = 4;
    } else {
        reg_a = &lcdc->REG_5C8;
        reg_v = lcdc->REG_5C8;
        port_shift = 0;
    }
    /* plane_alpha * fg.rgb + (1 - plane_alpha * fg.aplha) * bg.rgb */
    if (LCDC_DRM_BLEND_MODE_PREMULTI == drm_blend_mode) {
        if (zorder_id == LCDC_ZORDER_FIRST) {
            lcdc->REG_5D0 &= ~((LCDC_ALPHA_BLEND_COVERAGE_FIRST_L | LCDC_ALPHA_BLEND_PREMULTI_FIRST_L) << port_shift);
            lcdc->REG_5D0 |= LCDC_ALPHA_BLEND_PREMULTI_FIRST_L << port_shift;
            reg_v = SET_BITFIELD(reg_v, LCDC_GLOBAL_ALPHA_FIRST_L, alpha);
        } else if (zorder_id == LCDC_ZORDER_SECOND) {
            lcdc->REG_5D0 &= ~((LCDC_ALPHA_BLEND_COVERAGE_SECOND_L | LCDC_ALPHA_BLEND_PREMULTI_SECOND_L) << port_shift);
            lcdc->REG_5D0 |= LCDC_ALPHA_BLEND_PREMULTI_SECOND_L << port_shift;
            reg_v = SET_BITFIELD(reg_v, LCDC_GLOBAL_ALPHA_SECOND_L, alpha);
        } else if (zorder_id == LCDC_ZORDER_THIRD) {
            lcdc->REG_5D0 &= ~((LCDC_ALPHA_BLEND_COVERAGE_THIRD_L | LCDC_ALPHA_BLEND_PREMULTI_THIRD_L) << port_shift);
            lcdc->REG_5D0 |= LCDC_ALPHA_BLEND_PREMULTI_THIRD_L << port_shift;
            reg_v = SET_BITFIELD(reg_v, LCDC_GLOBAL_ALPHA_THIRD_L, alpha);
        } else if (zorder_id == LCDC_ZORDER_FOURTH) {
            lcdc->REG_5D0 &= ~((LCDC_ALPHA_BLEND_COVERAGE_FOURTH_L | LCDC_ALPHA_BLEND_PREMULTI_FOURTH_L) << port_shift);
            lcdc->REG_5D0 |= LCDC_ALPHA_BLEND_PREMULTI_FOURTH_L << port_shift;
            reg_v = SET_BITFIELD(reg_v, LCDC_GLOBAL_ALPHA_FOURTH_L, alpha);
        }
        *reg_a = reg_v;
     }
     /*plane_alpha * fg.aplha * fg.rgb + (1 - plane_alpha * fg.aplha) * bg.rgb */
     else if (LCDC_DRM_BLEND_MODE_COVERAGE == drm_blend_mode) {
        if (zorder_id == LCDC_ZORDER_FIRST) {
            lcdc->REG_5D0 &= ~((LCDC_ALPHA_BLEND_COVERAGE_FIRST_L | LCDC_ALPHA_BLEND_PREMULTI_FIRST_L) << port_shift);
            lcdc->REG_5D0 |= LCDC_ALPHA_BLEND_COVERAGE_FIRST_L << port_shift;
            reg_v = SET_BITFIELD(reg_v, LCDC_GLOBAL_ALPHA_FIRST_L, alpha);
        } else if (zorder_id == LCDC_ZORDER_SECOND) {
            lcdc->REG_5D0 &= ~((LCDC_ALPHA_BLEND_COVERAGE_SECOND_L | LCDC_ALPHA_BLEND_PREMULTI_SECOND_L) << port_shift);
            lcdc->REG_5D0 |= LCDC_ALPHA_BLEND_COVERAGE_SECOND_L << port_shift;
            reg_v = SET_BITFIELD(reg_v, LCDC_GLOBAL_ALPHA_SECOND_L, alpha);
        } else if (zorder_id == LCDC_ZORDER_THIRD) {
            lcdc->REG_5D0 &= ~((LCDC_ALPHA_BLEND_COVERAGE_THIRD_L | LCDC_ALPHA_BLEND_PREMULTI_THIRD_L) << port_shift);
            lcdc->REG_5D0 |= LCDC_ALPHA_BLEND_COVERAGE_THIRD_L << port_shift;
            reg_v = SET_BITFIELD(reg_v, LCDC_GLOBAL_ALPHA_THIRD_L, alpha);
        } else if (zorder_id == LCDC_ZORDER_FOURTH) {
            lcdc->REG_5D0 &= ~((LCDC_ALPHA_BLEND_COVERAGE_FOURTH_L | LCDC_ALPHA_BLEND_PREMULTI_FOURTH_L) << port_shift);
            lcdc->REG_5D0 |= LCDC_ALPHA_BLEND_COVERAGE_FOURTH_L << port_shift;
            reg_v = SET_BITFIELD(reg_v, LCDC_GLOBAL_ALPHA_FOURTH_L, alpha);
        }
        *reg_a = reg_v;
     }
     else {
        if (zorder_id == LCDC_ZORDER_FIRST) {
            lcdc->REG_5D0 &= ~((LCDC_ALPHA_BLEND_COVERAGE_FIRST_L | LCDC_ALPHA_BLEND_PREMULTI_FIRST_L) << port_shift);
        } else if (zorder_id == LCDC_ZORDER_SECOND) {
            lcdc->REG_5D0 &= ~((LCDC_ALPHA_BLEND_COVERAGE_SECOND_L | LCDC_ALPHA_BLEND_PREMULTI_SECOND_L) << port_shift);
        } else if (zorder_id == LCDC_ZORDER_THIRD) {
            lcdc->REG_5D0 &= ~((LCDC_ALPHA_BLEND_COVERAGE_THIRD_L | LCDC_ALPHA_BLEND_PREMULTI_THIRD_L) << port_shift);
        } else if (zorder_id == LCDC_ZORDER_FOURTH) {
            lcdc->REG_5D0 &= ~((LCDC_ALPHA_BLEND_COVERAGE_FOURTH_L | LCDC_ALPHA_BLEND_PREMULTI_FOURTH_L) << port_shift);
        }
     }
     LCDC_DBG("%s: port:%d, zorder id:%d, mode %d, const alpha 0x%02x, new_alg:%d",
            __func__, bus_port, zorder_id, drm_blend_mode, alpha, alpha_blend_use_new_alg);
     return LCDC_OK;
}

int hal_lcdc_blend_drm(uint8_t bus_port, uint8_t lid, enum hal_lcdc_drm_blend_mode_e drm_blend_mode, uint8_t alpha)
{
    if (lid > LCDC_LAYER_SMT) {
        LCDC_INFO("%s: invalid layer id %d", __func__, lid);
        return LCDC_INVAL;
    }
#ifdef LCDC_HAS_NEWALG_BLEND
    if (alpha_blend_use_new_alg) {
        if (lid < 4) {
            hal_lcdc_blend_first(bus_port, LCDC_BOTH_SEL_DST_PIXEL, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_PIXEL_ALPHA, 2);
            hal_lcdc_blend_second(bus_port, LCDC_BOTH_SEL_DST_PIXEL, LCDC_SEL_CONST_FF, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_PIXEL_ALPHA, 4);
            hal_lcdc_blend_third(bus_port, LCDC_BOTH_SEL_DST_PIXEL, LCDC_SEL_CONST_FF, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_PIXEL_ALPHA, 4);
            hal_lcdc_blend_fourth(bus_port, LCDC_BOTH_SEL_SRC_PIXEL, LCDC_SEL_CONST_FF, LCDC_SEL_CONST_00, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_PIXEL_ALPHA, 4);
        }
        else if (lid == 4) {
            hal_lcdc_blend_fourth(bus_port, LCDC_BOTH_SEL_DST_PIXEL, LCDC_SEL_CONST_FF, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_PIXEL_ALPHA, LCDC_SEL_PIXEL_ALPHA, 2);
        }
        else if (lid > 4) {
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
        }

        if (LCDC_DRM_BLEND_MODE_NONE == drm_blend_mode) {
            hal_lcdc_layer_alpha(bus_port, lid, LCDC_CONST_ALPHA, alpha); /* plane_alpha * fg.rgb + (1 - plane_alpha) * bg.rgb */
        } else {
            hal_lcdc_layer_alpha(bus_port, lid, LCDC_PIXEL_ALPHA, alpha); /* fg.alpha * fg.rgb + (1 - fg.alpha) * bg.rgb */
            hal_lcdc_blend_alpha(bus_port, lid, drm_blend_mode, alpha);   /* layer id same as zorder id*/
        }
    } else
#endif
    {
        hal_lcdc_select_top(LCDC_LAYER_GRA);
        if (LCDC_DRM_BLEND_MODE_NONE == drm_blend_mode) {
            hal_lcdc_lpfc_ltransp(lid, LCDC_CONST_ALPHA, alpha);
        } else if (LCDC_DRM_BLEND_MODE_PREMULTI == drm_blend_mode) {
            hal_lcdc_lpfc_ltransp(lid, LCDC_PIXEL_PREMULTI, alpha);
        } else if (LCDC_DRM_BLEND_MODE_COVERAGE == drm_blend_mode) {
            hal_lcdc_lpfc_ltransp(lid, LCDC_PIXEL_COVERAGE, alpha);
        } else {
            hal_lcdc_lpfc_ltransp(lid, LCDC_PIXEL_ALPHA, alpha);
        }
    }
    LCDC_DBG("%s: drm blend mode %d, const alpha 0x%02x, new_alg:%d",
            __func__, drm_blend_mode, alpha, alpha_blend_use_new_alg);
    return LCDC_OK;
}

int hal_lcdc_dsi0_blend(int blend_mode, int alpha_mode, uint8_t alpha)
{
    return hal_lcdc_blend(blend_mode, alpha_mode, alpha);
}

int hal_lcdc_dsi1_layer_select(uint8_t layermask)
{
    uint32_t v1 = lcdc->REG_2C0;
    uint32_t v2 = lcdc->REG_2C8;

    /* dsi1 always uses data1 */
    v1 &= ~(LCDC_GRA_IN_VLD_STEREO_R | LCDC_DMA_IN_VLD_STEREO_R | LCDC_TVG_IN_VLD_STEREO_R);
    v2 &= ~(LCDC_TVD_IN_VLD_STEREO_R | LCDC_SMT_IN_VLD_STEREO_R);
    v1 |= LCDC_DSI1_DATA1_SEL;

    if (layermask & (1 << LCDC_LAYER_GRA)) {
        v1 |= LCDC_GRA_IN_VLD_STEREO_R;
    }
    if (layermask & (1 << LCDC_LAYER_DMA)) {
        v1 |= LCDC_DMA_IN_VLD_STEREO_R;
    }
    if (layermask & (1 << LCDC_LAYER_TVG)) {
        v1 |= LCDC_TVG_IN_VLD_STEREO_R;
    }
    if (layermask & (1 << LCDC_LAYER_TVD)) {
        v2 |= LCDC_TVD_IN_VLD_STEREO_R;
    }
    if (layermask & (1 << LCDC_LAYER_SMT)) {
        v2 |= LCDC_SMT_IN_VLD_STEREO_R;
    }

    lcdc->REG_2C0 = v1;
    lcdc->REG_2C8 = v2;
    lcdc->REG_720 |= 0x1f;
    // write back use dsi 1
    lcdc->REG_6B4 |= LCDC_WDMA_SEL_ALPHA_TV_PN | LCDC_WDMA_SEL_DATA_TV_PN;

    LCDC_DBG("%s: DSI1 selected layer mask: %x", __func__, layermask);
    return LCDC_OK;
}

int hal_lcdc_dsi1_blend(int blend_mode, int alpha_mode, uint8_t alpha)
{
#ifdef LCDC_HAS_NEWALG_BLEND
    alpha_mode = alpha_mode;
    if (blend_mode == 0)
        blend_mode = LCDC_DRM_BLEND_MODE_DEFAULT;

    for (int lid = LCDC_LAYER_GRA; lid <= LCDC_LAYER_SMT; lid++)
    {
        hal_lcdc_blend_drm(1, lid, blend_mode, alpha);
    }
#else
    uint32_t v = lcdc->REG_2C8;

    v &= ~(LCDC_DATA1_BLEND_MODE_MASK | LCDC_DATA1_PN_ALPHA_PIXEL_MODE |
           LCDC_DATA1_TV_ALPHA_PIXEL_MODE);

    v |= LCDC_DATA1_BLEND_MODE(blend_mode) | LCDC_DATA1_PN_ALPHA(alpha) |
         LCDC_DATA1_TV_ALPHA(255 - alpha);

    if (alpha_mode == LCDC_PIXEL_ALPHA)
        v |= (LCDC_DATA1_PN_ALPHA_PIXEL_MODE | LCDC_DATA1_TV_ALPHA_PIXEL_MODE);

    lcdc->REG_2C8 = v;
#endif
    LCDC_DBG("%s: DSI1 blend mode %d, alpha mode %d, const alpha 0x%02x",
            __func__, blend_mode, alpha_mode, alpha);
    return LCDC_OK;
}

#ifdef LCDC_HAS_RGBIF
int hal_lcdc_rgbif_init(struct hal_lcdc_rgbif_config_t *cfg)
{
    uint32_t v;

    if (!cfg)
        return LCDC_FAILED;

    hal_iomux_set_rgblcd0();
    hal_cmu_lcdc_clock_enable(cfg->pix_mhz);

    v = lcdc->REG_2C0;

    v |= LCDC_PARALLEL_CLK_ENA | LCDC_LCDC_PARA0_ENA |
         LCDC_GRA_IN_VLD_STEREO_L | LCDC_DMA_IN_VLD_STEREO_L | LCDC_TVG_IN_VLD_STEREO_L |
         LCDC_GRA_IN_VLD_STEREO_R | LCDC_DMA_IN_VLD_STEREO_R | LCDC_TVG_IN_VLD_STEREO_R;

    if (cfg->clk_inv) {
        v |= LCDC_PARALLEL_CLK_INV;
    } else {
        v &= ~LCDC_PARALLEL_CLK_INV;
    }

    lcdc->REG_2C0 = v;

    if (cfg->hsync_inv) {
        lcdc->REG_1B8 |= LCDC_PN_INV_HSYNC;
        lcdc->REG_094 |= LCDC_TV_INV_HSYNC;
    } else {
        lcdc->REG_1B8 &= ~LCDC_PN_INV_HSYNC;
        lcdc->REG_094 &= ~LCDC_TV_INV_HSYNC;
    }

    if (cfg->vsync_inv) {
        lcdc->REG_1B8 |= LCDC_PN_INV_VSYNC;
        lcdc->REG_094 |= LCDC_TV_INV_VSYNC;
    } else {
        lcdc->REG_1B8 &= LCDC_PN_INV_VSYNC;
        lcdc->REG_094 &= ~LCDC_TV_INV_VSYNC;
    }

    LCDC_INFO("RGBIF: pixmhz:%d, bits:%d, clk/hsync/vsync inv:%d/%d/%d.",
            cfg->pix_mhz, cfg->pix_bits, cfg->clk_inv, cfg->hsync_inv, cfg->vsync_inv);
    return LCDC_OK;
}

void hal_lcdc_rgbif_exit(void)
{
    lcdc->REG_2C0 &= ~(LCDC_PARALLEL_CLK_ENA | LCDC_LCDC_PARA0_ENA);

    hal_cmu_lcdc_clock_disable();
    LCDC_INFO("RGBIF exit.");
}

void hal_lcdc_rgbif_swaprb_enable(bool enable)
{
    if (enable)
        lcdc->REG_1F8 |= LCDC_SMPN_SWAP_RB;
    else
        lcdc->REG_1F8 &= ~LCDC_SMPN_SWAP_RB;
}
#endif /* LCDC_HAS_RGBIF */

int hal_lcdc_dump_regs(void)
{
    volatile uint32_t *reg = (uint32_t *)LCDC_BASE;
    LCDC_INFO("Dump all LCDC registers, base %p:", reg);
    for (int i = 0; i <= (0x728 / 4); i++) {
        LCDC_INFO("  %p = 0x%08x", reg, *reg);
        reg++;
    }
    return LCDC_OK;
}

int hal_lcdc_dump_irqerror(uint32_t irqstat)
{
    uint32_t e_irq = irqstat & LCDC_ERROR_IRQS;
    LCDC_INFO("ERROR: irqstat:0x%x, underflow(DMA:%d, GRA:%d, TVG:%d, TVD:%d, SMT:%d, WB:%d-%d, TFBC:%d-%d)\n", irqstat,
        (e_irq & LCDC_DMA_FF_UNDERFLOW) != 0, (e_irq & LCDC_GRA_FF_UNDERFLOW) != 0, \
        (e_irq & LCDC_TVG_FF_UNDERFLOW) != 0, (e_irq & LCDC_TVD_FF_UNDERFLOW) != 0, \
        (e_irq & LCDC_SMT_FF_UNDERFLOW) != 0, (e_irq & LCDC_WB_FF_UNDERRUN) != 0, \
        (e_irq & LCDC_WB_FF_OVERRUN) != 0, (e_irq & LCDC_TFBC_FF_UNDERFLOW) != 0, \
        (e_irq & LCDC_TFBC_CRC_MISMATCH) != 0);
    return LCDC_OK;
}

int hal_lcdc_ftoggle_enable(int lid, bool enable)
{
    switch (lid) {
        case LCDC_LAYER_GRA:
            if (enable)
                lcdc->REG_264 |= LCDC_GRA_FTOGGLE;
            else
                lcdc->REG_264 &= ~LCDC_GRA_FTOGGLE;
            break;
        case LCDC_LAYER_DMA:
            if (enable)
                lcdc->REG_190 |= LCDC_DMA_FTOGGLE;
            else
                lcdc->REG_190 &= ~LCDC_DMA_FTOGGLE;
            break;
        case LCDC_LAYER_TVG:
            if (enable)
                lcdc->REG_080 |= LCDC_TVG_FTOGGLE;
            else
                lcdc->REG_080 &= ~LCDC_TVG_FTOGGLE;
            break;
        case LCDC_LAYER_TVD:
            if (enable)
                lcdc->REG_260 |= LCDC_TVD_FTOGGLE;
            else
                lcdc->REG_260 &= ~LCDC_TVD_FTOGGLE;
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d %s", __func__, lid, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

#ifdef LCDC_HAS_ROTATE
int hal_lcdc_rotate_enable(int lid, bool enable, int degree, int w, int h, bool tiled)
{
    if (lid != LCDC_LAYER_GRA) {
        LCDC_INFO("%s: layer %d doesn't support 90/270 rotation.",  __func__, lid);
        return LCDC_INVAL;
    }
    if (enable && degree != 90 && degree != 270) {
        LCDC_INFO("%s: only supports 90/270 degrees.", __func__);
        return LCDC_NOT_SUPPORT;
    }
#ifdef CHIP_BEST2009
    if (!enable) {
        lcdc->REG_2D4 = 0;
        lcdc->REG_2D8 = 0;
    } else {
        lcdc->REG_2D4 = LCDC_GRA_H_PIXEL_ROT(h) | LCDC_GRA_V_LINE_ROT(w);
        if (degree == 90)
            lcdc->REG_2D8 = LCDC_GRARA_ENA | LCDC_GRA_ROT_90 | LCDC_GRA_H_PIXEL_ROT_ACTIVE(h);
        else
            lcdc->REG_2D8 = LCDC_GRARA_ENA | LCDC_GRA_ROT_270 | LCDC_GRA_H_PIXEL_ROT_ACTIVE(h);
    }
#else
    if (!enable) {
        lcdc->REG_714 = 0;
        lcdc->REG_718 = 0;
    } else {
        lcdc->REG_714 = LCDC_GRA_H_PIXEL_ROT(h) | LCDC_GRA_V_LINE_ROT(w);
        if (degree == 90)
            lcdc->REG_718 = LCDC_GRARA_ENA | LCDC_GRA_ROT_90 | LCDC_GRA_H_PIXEL_ROT_ACTIVE(h);
        else
            lcdc->REG_718 = LCDC_GRARA_ENA | LCDC_GRA_ROT_270 | LCDC_GRA_H_PIXEL_ROT_ACTIVE(h);
        /* rotation need some blank lines */
        lcdc->REG_21C |= LCDC_VSYNC_NLINE(10);
        /* enable tiled rotation */
        if (tiled)
            lcdc->REG_718 |= LCDC_TILED_ROT_ENA;
    }
#endif
    LCDC_DBG("%s: layer %d, rotate %d %s.", __func__, lid, degree,
                enable ? "enabled" : "disabled");
    return LCDC_OK;
}
#else
int hal_lcdc_rotate_enable(int lid, bool enable, int degree, int w, int h, bool tiled)
{
    LCDC_DBG("%s: rotation is not supported.", __func__);
    return LCDC_NOT_SUPPORT;
}
#endif

#ifdef LCDC_HAS_SCALER
static const int16_t scaler_h_phase[256] = {
     0,    0,    0, 1024,    0,    0,    0,    0,
    -3,   10,  -28, 1022,   30,  -10,    3,    0,
    -6,   19,  -53, 1017,   62,  -22,    7,    0,
    -8,   27,  -76, 1008,   96,  -33,   11,   -1,
   -10,   35,  -97,  996,  132,  -45,   15,   -1,
   -12,   42, -115,  980,  169,  -57,   19,   -2,
   -14,   48, -131,  962,  208,  -69,   23,   -2,
   -15,   53, -145,  940,  248,  -82,   28,   -3,
   -15,   57, -156,  915,  289,  -94,   32,   -4,
   -16,   60, -165,  887,  332, -106,   37,   -5,
   -16,   62, -172,  857,  375, -117,   41,   -6,
   -16,   64, -176,  825,  418, -128,   45,   -7,
   -16,   65, -178,  790,  462, -139,   49,   -8,
   -15,   65, -179,  753,  505, -148,   53,  -10,
   -15,   64, -178,  715,  549, -157,   56,  -11,
   -14,   63, -175,  675,  592, -164,   59,  -12,
   -13,   61, -170,  634,  634, -170,   61,  -13,
   -12,   59, -164,  592,  675, -175,   63,  -14,
   -11,   56, -157,  549,  715, -178,   64,  -15,
   -10,   53, -148,  505,  753, -179,   65,  -15,
    -8,   49, -139,  462,  790, -178,   65,  -16,
    -7,   45, -128,  418,  825, -176,   64,  -16,
    -6,   41, -117,  375,  857, -172,   62,  -16,
    -5,   37, -106,  332,  887, -165,   60,  -16,
    -4,   32,  -94,  289,  915, -156,   57,  -15,
    -3,   28,  -82,  248,  940, -145,   53,  -15,
    -2,   23,  -69,  208,  962, -131,   48,  -14,
    -2,   19,  -57,  169,  980, -115,   42,  -12,
    -1,   15,  -45,  132,  996,  -97,   35,  -10,
    -1,   11,  -33,   96, 1008,  -76,   27,   -8,
     0,    7,  -22,   62, 1017,  -53,   19,   -6,
     0,    3,  -10,   30, 1022,  -28,   10,   -3,
};

static const int16_t scaler_v_phase[128] = {
      0, 1024,    0,    0,
    -23, 1022,   25,   -1,
    -42, 1015,   54,   -3,
    -59, 1005,   84,   -6,
    -74,  991,  118,  -11,
    -85,  973,  153,  -16,
    -95,  951,  190,  -22,
   -103,  927,  228,  -29,
   -108,  900,  268,  -36,
   -112,  870,  309,  -44,
   -113,  838,  351,  -52,
   -114,  804,  394,  -60,
   -113,  768,  437,  -68,
   -110,  730,  480,  -75,
   -106,  690,  523,  -83,
   -102,  650,  566,  -90,
    -96,  608,  608,  -96,
    -90,  566,  650, -102,
    -83,  523,  690, -106,
    -75,  480,  730, -110,
    -68,  437,  768, -113,
    -60,  394,  804, -114,
    -52,  351,  838, -113,
    -44,  309,  870, -112,
    -36,  268,  900, -108,
    -29,  228,  927, -103,
    -22,  190,  951,  -95,
    -16,  153,  973,  -85,
    -11,  118,  991,  -74,
     -6,   84, 1005,  -59,
     -3,   54, 1015,  -42,
     -1,   25, 1022,  -23,
};

#define scaler_min(a, b)   ((a) < (b) ? (a) : (b))
#define scaler_max(a, b)   ((a) < (b) ? (b) : (a))

int hal_lcdc_scaler_enable(bool enable, uint16_t in_w, uint16_t in_h,
                           uint16_t out_w, uint16_t out_h, bool qspi_bus)
{
    volatile uint32_t *reg;
    bool upscale = (out_w > in_w) ? true : false;
    uint16_t in_htotal, out_htotal, start_ptr;
    uint16_t cut[4] = {0, 0, 0, 0}; /* left, right, up, bottom */
    uint32_t h_incr, v_incr, h_init, v_init;
    uint16_t v_active, vln;
    uint16_t stby = 0x2c;           // must > 20
    uint16_t hfp = stby - 4, hbp = 6;
    const int16_t *phase;
    uint16_t clk_mHz = 24;

    if (!enable) {
        lcdc->REG_354 &= ~(LCDC_SCALER_ENA | LCDC_SCALER_VLD_TX_CTRL_SEL);
        hal_cmu_lcdc_scaler_clock_disable();
        LCDC_DBG("%s: scaler is disabled.", __func__);
        return LCDC_OK;
    }

    if (in_w == out_w && in_h == out_h) {
        lcdc->REG_354 &= ~(LCDC_SCALER_ENA | LCDC_SCALER_VLD_TX_CTRL_SEL);
        LCDC_INFO("%s: same in/out size, scaler is disabled.", __func__);
        return LCDC_OK;
    }
    if (out_w >= in_w * 4 || in_w > out_w * 4 ||
        out_h >= in_h * 4 || in_h > out_h * 4) {
        LCDC_INFO("%s: invalid scaling size, only support 1/2 ~ 4.", __func__);
        return LCDC_INVAL;
    }
    if (in_w < 20 || out_w < 20) {
        LCDC_INFO("%s: error, in_w & out_w must larger than 20.", __func__);
        return LCDC_INVAL;
    }
    /*
     * in_htotal/out_htotal ==  out_w/in_w
     * scaler_min(in_htotal, out_htotal) > scaler_max(out_w, in_w)
    */
    int scale_percent = out_w * 100 / in_w;
    if (upscale) {
        uint16_t tmp;
        in_htotal = (out_w + stby + 9)/10 * 10;
        out_htotal = in_htotal * scale_percent / 100;
        tmp = in_htotal;
        in_htotal = out_htotal;
        out_htotal = tmp;
        if ((in_htotal- in_w) > LCDC_SCALER_STBY_LEN_MASK)
        {
            in_htotal = LCDC_SCALER_STBY_LEN_MASK + in_w;
            out_htotal = in_htotal * 100 / scale_percent;
        }
        start_ptr = ((((in_htotal + out_htotal) / 2 + 12) >> 2) + 1) << 2;
        clk_mHz = hal_cmu_lcdc_get_pixmhz() * scale_percent / 100;
    } else {
        out_htotal = (in_w + stby + 9)/10 * 10;
        in_htotal = out_htotal * scale_percent /100;
        if (in_htotal < in_w) {
            in_htotal = in_htotal * 100 / scale_percent;
            out_htotal = in_htotal * 100 / scale_percent;
        }
        if (out_htotal/in_htotal >= 3)
            start_ptr = ((((out_htotal - in_htotal) / 2 - in_htotal + 12) >> 2) + 1) << 2;
        else
            start_ptr = ((((out_htotal - in_htotal) / 2 + 12) >> 2) + 1) << 2;
        clk_mHz = hal_cmu_lcdc_get_pixmhz() * 100 / scale_percent;
    }
    stby = in_htotal- in_w;
    if (stby < 0x2c)
        stby = 0x2c;
    hfp = stby - 4;
#if defined(CHIP_BEST1701) && defined(CHIP_SUBSYS_CASYS)
    start_ptr = 16;
#endif

    h_incr = (in_w - cut[0] - cut[1]) * (1 << 18) / out_w;
    v_incr = (in_h - cut[2] - cut[3]) * (1 << 18) / out_h;
    h_init = (in_w * (1 << 17) / out_w - (1 << 17)) + cut[0] * (1 << 18);
    v_init = (in_h * (1 << 17) / out_h - (1 << 17)) + cut[2] * (1 << 18);

    /* Enable scaler clock */
    hal_cmu_lcdc_scaler_clock_enable(clk_mHz);

    LCDC_DBG("scale %ux%u -> %ux%u. in_htotal:%d,out_htotal:%d,clk_mhz:%d ", in_w, in_h, out_w, out_h, in_htotal, out_htotal, hal_cmu_lcdc_scaler_getmhz());
    ASSERT(((in_htotal/out_htotal) == (out_w/in_w)) && (scaler_min(in_htotal, out_htotal) > scaler_max(out_w, in_w)), \
        "scale %ux%u -> %ux%u. in_htotal:%d,out_htotal:%d", in_w, in_h, out_w, out_h, in_htotal,out_htotal);
    lcdc->REG_354 = LCDC_SCALER_ENA | LCDC_SCALER_FORMAT(2) | LCDC_SCALER_START_PTR(start_ptr) |
                    LCDC_SCALER_R_HORI_EN | LCDC_SCALER_R_VERT_EN | LCDC_SCALER_VLD_TX_CTRL_SEL;
    lcdc->REG_340 = LCDC_SCALER_H_ACTIVE_IN(in_w) | LCDC_SCALER_V_ACTIVE_IN(in_h);
    lcdc->REG_344 = LCDC_SCALER_VSYNC_NLINE(0xd) | LCDC_SCALER_START_LINE(1) |
                    LCDC_SCALER_END_LINE(0xd);
    lcdc->REG_348 = LCDC_SCALER_START_PIX(2) | LCDC_SCALER_END_PIX(in_htotal);
    lcdc->REG_34C = LCDC_SCALER_H_FRONT_PORCH(hfp) | LCDC_SCALER_H_BACK_PORCH(hbp);
    lcdc->REG_350 = LCDC_SCALER_STBY_LEN(stby);
    lcdc->REG_358 = LCDC_SCALER_IMG_IN_HTOTAL(in_htotal) | LCDC_SCALER_IMG_OUT_HTOTAL(out_htotal);
    lcdc->REG_35C = LCDC_SCALER_IMG_IN_WIDTH(in_w) | LCDC_SCALER_IMG_IN_HEIGHT(in_h);
    lcdc->REG_360 = LCDC_SCALER_IMG_OUT_WIDTH(out_w) | LCDC_SCALER_IMG_OUT_HEIGHT(out_h);
    lcdc->REG_364 = LCDC_SCALER_R_HORI_PHASE_INCR(h_incr) | LCDC_SCALER_R_VERT_UP_CUT(cut[2]);
    lcdc->REG_368 = LCDC_SCALER_R_VERT_PHASE_INCR(v_incr) | LCDC_SCALER_R_VERT_BOT_CUT(cut[3]);
    lcdc->REG_36C = LCDC_SCALER_R_HORI_INIT(h_init);
    lcdc->REG_5F0 = LCDC_SCALER_R_VERT_INIT(v_init);

    if (qspi_bus)
        lcdc->REG_354 |= LCDC_SCALER_QSPI_INTR;

    /* config hori phase matrix */
    reg = &lcdc->REG_370;
    phase = scaler_h_phase;
    for (int i = 0; i < 256; i += 8) {
        uint32_t v1 = ((phase[i + 0] & 0xfff) << 0)  |
                      ((phase[i + 1] & 0xfff) << 12) |
                      ((phase[i + 2] & 0x0ff) << 24);
        uint32_t v2 = ((phase[i + 2] & 0xf00) >> 8)  |
                      ((phase[i + 3] & 0xfff) << 4)  |
                      ((phase[i + 4] & 0xfff) << 16) |
                      ((phase[i + 5] & 0x00f) << 28);
        uint32_t v3 = ((phase[i + 5] & 0xff0) >> 4) |
                      ((phase[i + 6] & 0xfff) << 8) |
                      ((phase[i + 7] & 0xfff) << 20);
        *reg++ = v1;
        *reg++ = v2;
        *reg++ = v3;
    }

    /* config vert phase matrix */
    reg = &lcdc->REG_4F0;
    phase = scaler_v_phase;
    for (int i = 0; i < 127; i += 8) {
        uint32_t v1 = ((phase[i + 0] & 0xfff) << 0)  |
                      ((phase[i + 1] & 0xfff) << 12) |
                      ((phase[i + 2] & 0x0ff) << 24);
        uint32_t v2 = ((phase[i + 2] & 0xf00) >> 8)  |
                      ((phase[i + 3] & 0xfff) << 4)  |
                      ((phase[i + 4] & 0xfff) << 16) |
                      ((phase[i + 5] & 0x00f) << 28);
        uint32_t v3 = ((phase[i + 5] & 0xff0) >> 4) |
                      ((phase[i + 6] & 0xfff) << 8) |
                      ((phase[i + 7] & 0xfff) << 20);
        *reg++ = v1;
        *reg++ = v2;
        *reg++ = v3;
    }

    /* reconfig v_active and ovsa_vln */
    v_active = GET_BITFIELD(lcdc->REG_118, LCDC_PN_V_ACTIVE);
    vln = GET_BITFIELD(lcdc->REG_0E8, LCDC_DMA_OVSA_VLN);
    lcdc->REG_5F4 = LCDC_SCALER_V_ACTIVE(v_active) | LCDC_SCALER_OVSA_VLN(vln);
    if (in_w <= 24)
        lcdc->REG_724 = SET_BITFIELD(lcdc->REG_724, LCDC_SCA_64TOPIXEL_RD_CNT_DMA, 0x18);
    else
        lcdc->REG_724 = SET_BITFIELD(lcdc->REG_724, LCDC_SCA_64TOPIXEL_RD_CNT_DMA, 0x20);

    LCDC_DBG("%s: scale %ux%u -> %ux%u.", __func__, in_w, in_h, out_w, out_h);
    return LCDC_OK;
}
#else /* LCDC_HAS_SCALER */
int hal_lcdc_scaler_enable(bool enable, uint16_t in_w, uint16_t in_h,
                           uint16_t out_w, uint16_t out_h, bool qspi_bus)
{
    LCDC_INFO("%s: scaler is not supported.", __func__);
    return LCDC_INVAL;
}
#endif

#ifdef LCDC_HAS_SCALER2
int hal_lcdc_scaler2_enable(bool enable, uint16_t in_w, uint16_t in_h,
                           uint16_t out_w, uint16_t out_h, bool qspi_bus)
{
    volatile uint32_t *reg;
    bool upscale = (out_w > in_w) ? true : false;
    uint16_t in_htotal, out_htotal, start_ptr;
    uint16_t cut[4] = {0, 0, 0, 0}; /* left, right, up, bottom */
    uint32_t h_incr, v_incr, h_init, v_init;
    uint16_t v_active, vln;
    uint16_t stby = 0x2c;           // must > 20
    uint16_t hfp = stby - 4, hbp = 6;
    const int16_t *phase;
    uint16_t clk_mHz = 24;

    if (!enable) {
        lcdc->REG_66C &= ~LCDC_SCALER_ENA;
        lcdc->REG_354 &= ~LCDC_SCALER_VLD_TX_CTRL_SEL;
        hal_cmu_lcdc_scaler2_clock_disable();
        LCDC_DBG("%s: scaler is disabled.", __func__);
        return LCDC_OK;
    }

    if (in_w == out_w && in_h == out_h) {
        lcdc->REG_66C &= ~LCDC_SCALER_ENA;
        lcdc->REG_354 &= ~LCDC_SCALER_VLD_TX_CTRL_SEL;
        LCDC_INFO("%s: same in/out size, scaler is disabled.", __func__);
        return LCDC_OK;
    }
    if (out_w >= in_w * 4 || in_w > out_w * 4 ||
        out_h >= in_h * 4 || in_h > out_h * 4) {
        LCDC_INFO("%s: invalid scaling size, only support 1/2 ~ 4.", __func__);
        return LCDC_INVAL;
    }
    if (in_w < 20 || out_w < 20) {
        LCDC_INFO("%s: error, in_w & out_w must larger than 20.", __func__);
        return LCDC_INVAL;
    }

    /*
     * in_htotal/out_htotal ==  out_w/in_w
     * scaler_min(in_htotal, out_htotal) > scaler_max(out_w, in_w)
     */
    int scale_percent = out_w * 100 / in_w;
    if (upscale) {
        uint16_t tmp;
        in_htotal = (out_w + stby + 9)/10 * 10;
        out_htotal = in_htotal * scale_percent / 100;
        tmp = in_htotal;
        in_htotal = out_htotal;
        out_htotal = tmp;
        if ((in_htotal- in_w) > LCDC_SCALER_STBY_LEN_MASK)
        {
            in_htotal = LCDC_SCALER_STBY_LEN_MASK + in_w;
            out_htotal = in_htotal * 100 / scale_percent;
        }
        start_ptr = ((((in_htotal + out_htotal) / 2 + 12) >> 2) + 1) << 2;
        clk_mHz = hal_cmu_lcdc_get_pixmhz() * scale_percent / 100;
    } else {
        out_htotal = (in_w + stby + 9)/10 * 10;
        in_htotal = out_htotal * scale_percent /100;
        if (in_htotal < in_w) {
            in_htotal = in_htotal * 100 / scale_percent;
            out_htotal = in_htotal * 100 / scale_percent;
        }
        if (out_htotal/in_htotal >= 3)
            start_ptr = ((((out_htotal - in_htotal) / 2 - in_htotal + 12) >> 2) + 1) << 2;
        else
            start_ptr = ((((out_htotal - in_htotal) / 2 + 12) >> 2) + 1) << 2;
        clk_mHz = hal_cmu_lcdc_get_pixmhz() * 100 / scale_percent;
    }
    stby = in_htotal- in_w;
    if (stby < 0x2c)
        stby = 0x2c;
    hfp = stby - 4;
#if defined(CHIP_BEST1701) && defined(CHIP_SUBSYS_CASYS)
    start_ptr = 16;
#endif

    h_incr = (in_w - cut[0] - cut[1]) * (1 << 18) / out_w;
    v_incr = (in_h - cut[2] - cut[3]) * (1 << 18) / out_h;
    h_init = (in_w * (1 << 17) / out_w - (1 << 17)) + cut[0] * (1 << 18);
    v_init = (in_h * (1 << 17) / out_h - (1 << 17)) + cut[2] * (1 << 18);

    /* Enable scaler clock */
    hal_cmu_lcdc_scaler2_clock_enable(clk_mHz);

    LCDC_DBG("scale2 %ux%u -> %ux%u. in_htotal:%d,out_htotal:%d,clk_mhz:%d ", in_w, in_h, out_w, out_h, in_htotal, out_htotal, hal_cmu_lcdc_scaler2_getmhz());
    ASSERT(((in_htotal/out_htotal) == (out_w/in_w)) && (scaler_min(in_htotal, out_htotal) > scaler_max(out_w, in_w)), \
        "scale2 %ux%u -> %ux%u. in_htotal:%d,out_htotal:%d", in_w, in_h, out_w, out_h, in_htotal,out_htotal);
    lcdc->REG_66C = LCDC_SCALER_ENA | LCDC_SCALER_FORMAT(2) | LCDC_SCALER_START_PTR(start_ptr) |
                    LCDC_SCALER_R_HORI_EN | LCDC_SCALER_R_VERT_EN;
    lcdc->REG_354 |= LCDC_SCALER_VLD_TX_CTRL_SEL;
    lcdc->REG_658 = LCDC_SCALER_H_ACTIVE_IN(in_w) | LCDC_SCALER_V_ACTIVE_IN(in_h);
    lcdc->REG_65C = LCDC_SCALER_VSYNC_NLINE(0xd) | LCDC_SCALER_START_LINE(1) |
                    LCDC_SCALER_END_LINE(0xd);
    lcdc->REG_660 = LCDC_SCALER_START_PIX(2) | LCDC_SCALER_END_PIX(in_htotal);
    lcdc->REG_664 = LCDC_SCALER_H_FRONT_PORCH(hfp) | LCDC_SCALER_H_BACK_PORCH(hbp);
    lcdc->REG_668 = LCDC_SCALER_STBY_LEN(stby);
    lcdc->REG_670 = LCDC_SCALER_IMG_IN_HTOTAL(in_htotal) | LCDC_SCALER_IMG_OUT_HTOTAL(out_htotal);
    lcdc->REG_674 = LCDC_SCALER_IMG_IN_WIDTH(in_w) | LCDC_SCALER_IMG_IN_HEIGHT(in_h);
    lcdc->REG_678 = LCDC_SCALER_IMG_OUT_WIDTH(out_w) | LCDC_SCALER_IMG_OUT_HEIGHT(out_h);
    lcdc->REG_67C = LCDC_SCALER_R_HORI_PHASE_INCR(h_incr) | LCDC_SCALER_R_VERT_UP_CUT(cut[2]);
    lcdc->REG_680 = LCDC_SCALER_R_VERT_PHASE_INCR(v_incr) | LCDC_SCALER_R_VERT_BOT_CUT(cut[3]);
    lcdc->REG_684 = LCDC_SCALER_R_HORI_INIT(h_init);
    lcdc->REG_688 = LCDC_SCALER_R_VERT_INIT(v_init);

    if (qspi_bus)
        lcdc->REG_66C |= LCDC_SCALER_QSPI_INTR;

    /* config hori phase matrix */
    reg = &lcdc->REG_370;
    phase = scaler_h_phase;
    for (int i = 0; i < 256; i += 8) {
        uint32_t v1 = ((phase[i + 0] & 0xfff) << 0)  |
                      ((phase[i + 1] & 0xfff) << 12) |
                      ((phase[i + 2] & 0x0ff) << 24);
        uint32_t v2 = ((phase[i + 2] & 0xf00) >> 8)  |
                      ((phase[i + 3] & 0xfff) << 4)  |
                      ((phase[i + 4] & 0xfff) << 16) |
                      ((phase[i + 5] & 0x00f) << 28);
        uint32_t v3 = ((phase[i + 5] & 0xff0) >> 4) |
                      ((phase[i + 6] & 0xfff) << 8) |
                      ((phase[i + 7] & 0xfff) << 20);
        *reg++ = v1;
        *reg++ = v2;
        *reg++ = v3;
    }

    /* config vert phase matrix */
    reg = &lcdc->REG_4F0;
    phase = scaler_v_phase;
    for (int i = 0; i < 127; i += 8) {
        uint32_t v1 = ((phase[i + 0] & 0xfff) << 0)  |
                      ((phase[i + 1] & 0xfff) << 12) |
                      ((phase[i + 2] & 0x0ff) << 24);
        uint32_t v2 = ((phase[i + 2] & 0xf00) >> 8)  |
                      ((phase[i + 3] & 0xfff) << 4)  |
                      ((phase[i + 4] & 0xfff) << 16) |
                      ((phase[i + 5] & 0x00f) << 28);
        uint32_t v3 = ((phase[i + 5] & 0xff0) >> 4) |
                      ((phase[i + 6] & 0xfff) << 8) |
                      ((phase[i + 7] & 0xfff) << 20);
        *reg++ = v1;
        *reg++ = v2;
        *reg++ = v3;
    }

    /* reconfig v_active and ovsa_vln */
    v_active = GET_BITFIELD(lcdc->REG_058, LCDC_PN_V_ACTIVE);
    vln = GET_BITFIELD(lcdc->REG_028, LCDC_DMA_OVSA_VLN);
    lcdc->REG_68C = LCDC_SCALER_V_ACTIVE(v_active) | LCDC_SCALER_OVSA_VLN(vln);
    if (in_w <= 24)
        lcdc->REG_728 = SET_BITFIELD(lcdc->REG_728, LCDC_SCA_64TOPIXEL_RD_CNT_TVD, 0x18);
    else
        lcdc->REG_728 = SET_BITFIELD(lcdc->REG_728, LCDC_SCA_64TOPIXEL_RD_CNT_TVD, 0x20);

    LCDC_DBG("%s: scale %ux%u -> %ux%u.", __func__, in_w, in_h, out_w, out_h);
    return LCDC_OK;
}
#else /* LCDC_HAS_SCALER2 */
int hal_lcdc_scaler2_enable(bool enable, uint16_t in_w, uint16_t in_h,
                           uint16_t out_w, uint16_t out_h, bool qspi_bus)
{
    LCDC_INFO("%s: scaler2 is not supported.", __func__);
    return LCDC_INVAL;
}
#endif

void hal_lcdc_arqos_ctrl_config(int value)
{
    lcdc->REG_2C4 = LCDC_ARQOS_FIX_RDQOS;
    lcdc->REG_2C4 = SET_BITFIELD(lcdc->REG_2C4, LCDC_ARQOS_RDQOS_VALUE, value);
}

void hal_lcdc_aff_watermark_config(int watermark)
{
    lcdc->REG_094 = SET_BITFIELD(lcdc->REG_094, LCDC_AFF_WATERMARK, watermark);
}

#endif
