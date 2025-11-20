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
#define LCDC_DBG(...)           TR_INFO( TR_MOD(HAL), "//LCDC: " __VA_ARGS__)
#else
#define LCDC_DBG(...)
#endif

#define LCDC_INFO(...)          TR_INFO( TR_MOD(HAL), "//LCDC: " __VA_ARGS__)

static struct LCDC_REG_T * const lcdc = (struct LCDC_REG_T *)LCDC_BASE;

static bool lcdc_video_mode = false;

int hal_lcdc_video_mode_enable(bool enable)
{
    lcdc_video_mode = enable;
    return LCDC_OK;
}

int hal_lcdc_irqn(void)
{
    return DISPLAY_IRQn;
}

int hal_lcdc_write_clut(uint32_t citem, int n)
{
    lcdc->REG_19C = citem;
    lcdc->REG_198 &= ~(LCDC_SRAM_ADDR_MASK | LCDC_SRAM_ADDR_LCDID_MASK |
                       LCDC_SRAM_INIT_WR_RD_MASK);
    lcdc->REG_198 |= LCDC_SRAM_ADDR(n) | LCDC_SRAM_ADDR_LCDID(3) | LCDC_SRAM_INIT_WR_RD(2);
    LCDC_DBG("%s: wrdata 0x%x, sram_addr 0x%x", __func__, citem, n);
    return LCDC_OK;
}

int hal_lcdc_write_gamma(uint32_t citem, int n)
{
    lcdc->REG_19C = citem;

    lcdc->REG_198 &= ~(LCDC_SRAM_ADDR_MASK | LCDC_SRAM_ADDR_LCDID_MASK |
                       LCDC_SRAM_INIT_WR_RD_MASK);
    lcdc->REG_198 |= LCDC_SRAM_ADDR(n) | LCDC_SRAM_ADDR_LCDID(0) | LCDC_SRAM_INIT_WR_RD(2);

    lcdc->REG_198 &= ~LCDC_SRAM_ADDR_LCDID_MASK;
    lcdc->REG_198 |= LCDC_SRAM_ADDR_LCDID(1);

    lcdc->REG_198 &= ~LCDC_SRAM_ADDR_LCDID_MASK;
    lcdc->REG_198 |= LCDC_SRAM_ADDR_LCDID(2);
    LCDC_DBG("%s: wrdata 0x%x, sram_addr 0x%x", __func__, citem, n);
    return LCDC_OK;
}

int hal_lcdc_lcadjcurves_enable(int lid, bool enable)
{
    if (enable)
        lcdc->REG_260 |= LCDC_PN_GAMMA_ENA;
    else
        lcdc->REG_260 &= ~LCDC_PN_GAMMA_ENA;
    LCDC_DBG("%s: %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_axifastmode_enable(bool enable)
{
    /* valid for AXI bus */
#if defined(CHIP_BEST1600) || defined(CHIP_BEST1700) || defined(CHIP_BEST2003) || defined(CHIP_BEST2009)
    if (enable)
        lcdc->REG_260 |= LCDC_ARBFAST_ENA;
    else
        lcdc->REG_260 &= ~LCDC_ARBFAST_ENA;
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
        case LCDC_LAYER_DEC:
            lcdc->REG_2D8 = LCDC_DEC_SA0(addr);
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
        case LCDC_LAYER_DEC:
            addr = GET_BITFIELD(lcdc->REG_2D8, LCDC_DEC_SA0);
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
            lcdc->REG_0FC |= LCDC_GRA_PITCH(pitch);
            break;
        case LCDC_LAYER_DMA:
            lcdc->REG_0E0 |= LCDC_DMA_PITCH_Y(pitch);
            break;
        case LCDC_LAYER_TVG:
            lcdc->REG_03C |= LCDC_TVG_PITCH(pitch);
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_020 |= LCDC_TVD_PITCH_Y(pitch);
            break;
        case LCDC_LAYER_DEC:
            lcdc->REG_03C |= LCDC_TVG_PITCH(pitch);
            break;
        case LCDC_LAYER_WB:
            lcdc->REG_200 |= LCDC_WDMA_IMG_PITCH(pitch);
            break;
        default:
            LCDC_INFO("%s: invalid layer id %d", __func__, lid);
            return LCDC_INVAL;
    }
    LCDC_DBG("%s: layer %d pitch: %d", __func__, lid, pitch);
    return LCDC_OK;
}

int hal_lcdc_lvsepxcount(int start, int end)
{
    lcdc->REG_13C &= ~(LCDC_PN_V_SPXLCNT_MASK | LCDC_PN_V_EPXLCNT_MASK);
    lcdc->REG_13C |= LCDC_PN_V_SPXLCNT(start) | LCDC_PN_V_EPXLCNT(end);
    LCDC_DBG("%s: start %d, end %d", __func__, start, end);
    return LCDC_OK;
}

static int stby_vsync_min = 0x40;
int hal_lcdc_stby_vsync_calc(int width, int pixbit, int pixmhz, int dsimhz, int dsilanes)
{
    int n = dsimhz / pixbit * dsilanes;
    if (n && pixmhz > n)
        stby_vsync_min = (pixmhz - n) * (width + 0x40) / n;
    if (stby_vsync_min < 0x40)
        stby_vsync_min = 0x40;
    LCDC_DBG("%s: standby vsync min val: 0x%x", __func__, stby_vsync_min);
    return LCDC_OK;
}

int hal_lcdc_stby_vsync_set(int stby_len, int start_line, int end_line)
{
    if (stby_len < stby_vsync_min)
        stby_len = stby_vsync_min;

    lcdc->REG_218 = LCDC_VSYNC_START_LINE(start_line) | LCDC_VSYNC_END_LINE(end_line) |
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
            lcdc->REG_040 &= ~(LCDC_TVG_OVSA_HPXL_MASK | LCDC_TVG_OVSA_VLN_MASK);
            lcdc->REG_040 = LCDC_TVG_OVSA_HPXL(hpxl) | LCDC_TVG_OVSA_VLN(vln) |
                            LCDC_TVGFRM_FIX;
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_028 &= ~(LCDC_TVD_OVSA_HPXL_MASK | LCDC_TVD_OVSA_VLN_MASK);
            lcdc->REG_028 = LCDC_TVD_OVSA_HPXL(hpxl) | LCDC_TVD_OVSA_VLN(vln) |
                            LCDC_TVDFRM_FIX;
            break;
        case LCDC_LAYER_DEC:
            lcdc->REG_040 &= ~(LCDC_TVG_OVSA_HPXL_MASK | LCDC_TVG_OVSA_VLN_MASK);
            lcdc->REG_040 = LCDC_TVG_OVSA_HPXL(hpxl) | LCDC_TVG_OVSA_VLN(vln) |
                            LCDC_TVGFRM_FIX;
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
    LCDC_DBG("%s: pn hfp %d, hbp %d", __func__, hfp, hbp);
    return LCDC_OK;
}

int hal_lcdc_bgl_vporch_set(int vfp, int vbp)
{
    lcdc->REG_120 = LCDC_PN_V_FPORCH(vfp) | LCDC_PN_V_BPORCH(vbp);
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
            break;
        case LCDC_LAYER_TVG:
            lcdc->REG_044 = LCDC_TVG_H_PIXEL(w) | LCDC_TVG_V_LINE(h);
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_02C = LCDC_TVD_H_PIXEL(w) | LCDC_TVD_V_LINE(h);
            break;
        case LCDC_LAYER_DEC:
            lcdc->REG_044 = LCDC_TVG_H_PIXEL(w) | LCDC_TVG_V_LINE(h);
            lcdc->REG_2DC = LCDC_DEC_H_PIXEL(w) | LCDC_DEC_V_LINE(h);
            break;
        case LCDC_LAYER_WB:
            lcdc->REG_204 = LCDC_WDMA_IMG_WIDTH(w) | LCDC_WDMA_IMG_HEIGHT(h);
            break;
        case LCDC_LAYER_SPU:
            if (!lcdc_video_mode) {
                w++;
                h++;
            }
            lcdc->REG_118 = LCDC_PN_H_ACTIVE(w) | LCDC_PN_V_ACTIVE(h);
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
            lcdc->REG_048 = LCDC_TVGZM_H_PIXEL(w) | LCDC_TVGZM_V_LINE(h);
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_030 = LCDC_TVDZM_H_PIXEL(w) | LCDC_TVDZM_V_LINE(h);
            break;
        case LCDC_LAYER_DEC:
            lcdc->REG_048 = LCDC_TVGZM_H_PIXEL(w) | LCDC_TVGZM_V_LINE(h);
            lcdc->REG_2D4 = LCDC_DECZM_H_PIXEL(w) | LCDC_DECZM_V_LINE(h) |
                            LCDC_DEC_UNDERFLOW_START_CNT(0xf);
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
        case LCDC_LAYER_DEC:
            if (enable) {
                lcdc->REG_2D0 &= ~LCDC_DEC_HLEN_DR;
                lcdc->REG_2D0 |= LCDC_DEC_ENA;
            } else {
                lcdc->REG_2D0 &= ~LCDC_DEC_ENA;
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
    lcdc->REG_194 &= ~LCDC_PN_VSYNC_MODE_MASK;
    lcdc->REG_194 |= LCDC_PN_VSYNC_MODE(mode);
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
    uint8_t dither_mode[4] = { 6, 4, 5, 7 };
    uint8_t mode = 0;
    uint32_t v = lcdc->REG_1EC;

    v &= ~(LCDC_DITHER_EN_PN | LCDC_DITHER_EN_TV |
           LCDC_DITHER_MODE_PN_MASK | LCDC_DITHER_MODE_TV_MASK);

    if (enable)
        v |= (LCDC_DITHER_EN_PN | LCDC_DITHER_EN_TV);

    if (level > 0) {
        dither_mode[0] = 0;
        dither_mode[1] = 1;
        dither_mode[2] = 2;
        dither_mode[3] = 3;
    }
    if (red == 5 && green == 5 && blue == 5) {
        mode = dither_mode[3];
    } else {
        uint8_t i = red - 4;
        if (i >= 0 && i < 4)
            mode = dither_mode[i];
        else
            mode = dither_mode[0];
    }

    v |= LCDC_DITHER_MODE_PN(mode) | LCDC_DITHER_MODE_TV(mode);

    lcdc->REG_1EC = v;
    LCDC_DBG("%s: dither %s, mode %d", __func__, enable ? "enabled" : "disabled", mode);
    return LCDC_OK;
}

uint32_t hal_lcdc_dither_read(uint8_t index)
{
    uint32_t v;
    lcdc->REG_1EC &= ~LCDC_DITHER_TABLE_INDEX_SEL_MASK;
    lcdc->REG_1EC |= LCDC_DITHER_TABLE_INDEX_SEL(index);
    v = lcdc->REG_1F0;
    LCDC_DBG("%s: table index %d, value 0x%08x", __func__, index, v);
    return v;
}

int hal_lcdc_dither_write(uint8_t index, uint32_t value)
{
    lcdc->REG_1EC &= ~LCDC_DITHER_TABLE_INDEX_SEL_MASK;
    lcdc->REG_1EC |= LCDC_DITHER_TABLE_INDEX_SEL(index);
    lcdc->REG_1F0 = LCDC_DITHER_TBL_DATA(value);
    LCDC_DBG("%s: table index %d, value 0x%08x", __func__, index, value);
    return LCDC_OK;
}

static bool hal_lcdc_cfmt_is_yuv(enum hal_lcdc_color_format_e fmt)
{
    if (fmt == LCDC_CFMT_YUV422_YUYV || fmt == LCDC_CFMT_YUV422_PLANAR ||
        fmt == LCDC_CFMT_YUV420_PLANAR)
        return true;
    return false;
}

static uint8_t hal_lcdc_cfmt_regval(enum hal_lcdc_color_format_e fmt)
{
    switch (fmt) {
        case LCDC_CFMT_RGB565:
            return 0x0;
        case LCDC_CFMT_RGB888:
            return 0x2;
        case LCDC_CFMT_RGBA8888:
            return 0x4;
        case LCDC_CFMT_YUV422_YUYV:
            return 0x5;
        case LCDC_CFMT_YUV422_PLANAR:
            return 0x6;
        case LCDC_CFMT_YUV420_PLANAR:
            return 0x7;
        case LCDC_CFMT_PALETE_4:
            return 0x9;
        case LCDC_CFMT_PALETE_8:
            return 0xA;
        case LCDC_CFMT_ARGB8888:
            return 0xB;
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
            lcdc->REG_080 &= ~(LCDC_TVG_SWAPRB | LCDC_YUV2RGB_TVG | LCDC_TVGFORMAT_MASK);
            lcdc->REG_080 |= LCDC_TVGFORMAT(v) | (swap_rb ? LCDC_TVG_SWAPRB : 0) |
                             (is_yuv ? LCDC_YUV2RGB_TVG : 0);
            break;
        case LCDC_LAYER_TVD:
            lcdc->REG_260 &= ~(LCDC_TVD_SWAPRB | LCDC_YUV2RGB_TVD | LCDC_TVDFORMAT_MASK);
            lcdc->REG_260 |= LCDC_TVDFORMAT(v) | (swap_rb ? LCDC_TVD_SWAPRB : 0) |
                             (is_yuv ? LCDC_YUV2RGB_TVD : 0);
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

int hal_lcdc_lchromakeyenable(int lid, bool enable)
{
    lcdc->REG_194 &= ~LCDC_PN_KEY_MODE_MASK;
    lcdc->REG_084 &= ~LCDC_TV_CKEY_MODE_MASK;
    if (enable) {
        lcdc->REG_194 |= LCDC_PN_KEY_MODE(LCDC_CKEY_RGB_ALL);
        lcdc->REG_084 |= LCDC_TV_CKEY_DMA | LCDC_TV_CKEY_GRA | LCDC_TV_CKEY_TVG |
                         LCDC_TV_CKEY_MODE(LCDC_CKEY_RGB_ALL);
    }
    LCDC_DBG("%s: colorkey mode RGB_ALL %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_lpfc_ltransp(int lid, int mode, uint8_t alpha)
{
    lcdc->REG_194 &= ~(LCDC_PN_ALPHA_MODE_MASK | LCDC_PN_ALPHA_MASK);
    lcdc->REG_194 |= LCDC_PN_ALPHA_MODE(mode) | LCDC_PN_ALPHA(alpha);
    LCDC_DBG("%s: alpha mode %d, alpha value 0x%02x", __func__, mode, alpha);
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

    lcdc->REG_130 = LCDC_PN_CKEY_Y(rr) | LCDC_PN_CKEY_Y1(r) | LCDC_PN_CKEY_Y2(r) |
                    LCDC_PN_ALPHA_Y(ra);
    lcdc->REG_134 = LCDC_PN_CKEY_U(rg) | LCDC_PN_CKEY_U1(g) | LCDC_PN_CKEY_U2(g) |
                    LCDC_PN_ALPHA_U(ra);
    lcdc->REG_138 = LCDC_PN_CKEY_V(rb) | LCDC_PN_CKEY_V1(b) | LCDC_PN_CKEY_V2(b) |
                    LCDC_PN_ALPHA_V(ra);
    LCDC_DBG("%s: colorkey 0x%x, 0x%x, alpha 0x%x", __func__, rgb, rrgb, ra);
    return LCDC_OK;
}

int hal_lcdc_lchromakey_newalpha(int lid, uint8_t newalpha)
{
    lcdc->REG_130 &= ~LCDC_PN_ALPHA_Y_MASK;
    lcdc->REG_130 |= LCDC_PN_ALPHA_Y(newalpha);

    lcdc->REG_134 &= ~LCDC_PN_ALPHA_U_MASK;
    lcdc->REG_134 |= LCDC_PN_ALPHA_U(newalpha);

    lcdc->REG_138 &= ~LCDC_PN_ALPHA_V_MASK;
    lcdc->REG_138 |= LCDC_PN_ALPHA_V(newalpha);

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
    lcdc->REG_210 &= ~LCDC_EXTRA_DELAY_MASK;
    lcdc->REG_210 |= LCDC_EXTRA_DELAY(delay);
    LCDC_DBG("%s: extra delay 0x%x", __func__, delay);
    return LCDC_OK;
}

int hal_lcdc_ol_dmaburst_length_set(uint8_t len)
{
    lcdc->REG_200 &= ~LCDC_WDMA_BURST_LEN_MASK;
    lcdc->REG_200 |= LCDC_WDMA_BURST_LEN(len);
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
        lcdc->REG_1C0 |= (LCDC_DUMB_FRAMEDONE | LCDC_TXC_FRAMEDONE | LCDC_HWC_FRAMEDONE);
    else
        lcdc->REG_1C0 &= ~(LCDC_DUMB_FRAMEDONE | LCDC_TXC_FRAMEDONE | LCDC_HWC_FRAMEDONE);
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
    lcdc->REG_1C4 &= ~(LCDC_DUMB_FRAMEDONE | LCDC_TXC_FRAMEDONE | LCDC_HWC_FRAMEDONE);
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
        return !!(regval & LCDC_DUMB_FRAMEDONE);
    else
        return !!(regval & LCDC_TXC_FRAMEDONE);
}

bool hal_lcdc_irq_is_hwc_framedone(uint32_t regval)
{
    return !!(regval & LCDC_HWC_FRAMEDONE);
}

bool hal_lcdc_irq_is_vsync(uint32_t regval)
{
    if (lcdc_video_mode)
        return !!(regval & LCDC_PN_VSYNC_IRQ);
    else
        return false;
}

bool hal_lcdc_irq_is_framedone_irq0(uint32_t regval)
{
    return !!(regval & (LCDC_GRA_FRAME_IRQ0 | LCDC_DMA_FRAME_IRQ0 |
                        LCDC_TVG_FRAME_IRQ0 | LCDC_TVD_FRAME_IRQ0));
}

bool hal_lcdc_irq_is_framedone_irq1(uint32_t regval)
{
    return !!(regval & (LCDC_GRA_FRAME_IRQ1 | LCDC_DMA_FRAME_IRQ1 |
                        LCDC_TVG_FRAME_IRQ1 | LCDC_TVD_FRAME_IRQ1));
}

bool hal_lcdc_irq_is_error(uint32_t regval)
{
    return !!(regval & (LCDC_ERR_IRQ | LCDC_GRA_FF_UNDERFLOW | LCDC_DMA_FF_UNDERFLOW |
                        LCDC_TVG_FF_UNDERFLOW | LCDC_TVD_FF_UNDERFLOW));
}

int hal_lcdc_error_irq_enable(void)
{
    lcdc->REG_1C0 |= LCDC_ERR_IRQ_ENA | LCDC_GRA_FF_UNDERFLOW_ENA | LCDC_DMA_FF_UNDERFLOW_ENA |
                     LCDC_TVG_FF_UNDERFLOW_ENA | LCDC_TVD_FF_UNDERFLOW_ENA;
    LCDC_DBG("%s: error & underflow irq enabled", __func__);
    return LCDC_OK;
}

int hal_lcdc_error_irqstat_clr(void)
{
    lcdc->REG_1C4 &= ~(LCDC_ERR_IRQ | LCDC_GRA_FF_UNDERFLOW | LCDC_DMA_FF_UNDERFLOW |
                       LCDC_TVG_FF_UNDERFLOW | LCDC_TVD_FF_UNDERFLOW);
    LCDC_DBG("%s: error irq clear", __func__);
    return LCDC_OK;
}

int hal_lcdc_disp_pn_reset(void)
{
    hal_cmu_reset_set(HAL_CMU_MOD_Q_DIS_PN);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_DIS_PN);

    hal_cmu_reset_set(HAL_CMU_MOD_X_DISP);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_X_DISP);

    hal_cmu_reset_set(HAL_CMU_MOD_Q_DIS_TV);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_DIS_TV);
    return LCDC_OK;
}

int hal_lcdc_reset(void)
{
    hal_cmu_reset_set(HAL_CMU_MOD_H_DISPPRE);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_DISPPRE);

    hal_cmu_reset_set(HAL_CMU_MOD_H_LCDC);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_LCDC);

    hal_cmu_reset_set(HAL_CMU_MOD_H_DISP);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_DISP);

    hal_cmu_reset_set(HAL_CMU_MOD_X_DISP);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_X_DISP);

    hal_cmu_reset_set(HAL_CMU_MOD_X_DISPB);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_X_DISPB);

    hal_cmu_reset_set(HAL_CMU_MOD_Q_DIS_PN);
    hal_cmu_reset_set(HAL_CMU_MOD_Q_DIS_TV);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_DIS_PN);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_DIS_TV);

#if 0
    // dsi reset
    hal_cmu_reset_set(HAL_CMU_MOD_Q_LCDC_P);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_LCDC_P);

    hal_cmu_reset_set(HAL_CMU_MOD_Q_DIS_PIX);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_DIS_PIX);

    hal_cmu_reset_set(HAL_CMU_MOD_Q_DIS_DSI);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_DIS_DSI);
#endif
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
#ifdef FPGA
    lcdc->REG_218 = LCDC_STBY_LENGTH(0x200) | LCDC_VSYNC_START_LINE(1) | LCDC_VSYNC_END_LINE(5);
#else
    lcdc->REG_218 = LCDC_STBY_LENGTH(0x80) | LCDC_VSYNC_START_LINE(1) | LCDC_VSYNC_END_LINE(5);
#endif
    lcdc->REG_21C = LCDC_H_FRONT_PORCH_SMPN(0x20) | LCDC_H_BACK_PORCH_SMPN(0x20) |
                    LCDC_VSYNC_NLINE(0xa);
    lcdc->REG_220 = LCDC_VSYNC_START_PIX(0x1e) | LCDC_VSYNC_END_PIX(0x1f);
    LCDC_DBG("%s:", __func__);
    return LCDC_OK;
}

int hal_lcdc_wdma_1fpt_set(void)
{
    lcdc->REG_204 &= ~LCDC_WDMA_FPT_SET_MASK;
    lcdc->REG_204 |= LCDC_WDMA_FPT_SET(1);
    LCDC_DBG("%s:", __func__);
    return LCDC_OK;
}

int hal_lcdc_wdma_toggle_frame(void)
{
    lcdc->REG_204 &= ~LCDC_WDMA_FPT_SET_MASK;
    lcdc->REG_204 |= LCDC_WDMA_FPT_SET(1);
    LCDC_DBG("%s:", __func__);
    return LCDC_OK;
}

int hal_lcdc_select_top(uint8_t lid)
{
    if (lid == LCDC_LAYER_GRA) {
        lcdc->REG_1E8 &= ~LCDC_OVTOP_MASK;
        lcdc->REG_1E8 |= LCDC_OVTOP(0);
    } else {
        lcdc->REG_1E8 &= ~LCDC_OVTOP_MASK;
        lcdc->REG_1E8 |= LCDC_OVTOP(1);
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

int hal_lcdc_swap_burst_length(uint8_t len)
{
    lcdc->REG_1DC &= ~(LCDC_DMA_BURST0TO3_MASK | LCDC_GRA_BURST0TO3_MASK |
                       LCDC_TVG_BURST0TO3_MASK | LCDC_TVD_BURST0TO3_MASK);
    lcdc->REG_1DC |= LCDC_DMA_BURST0TO3(len) | LCDC_GRA_BURST0TO3(len) |
                     LCDC_TVG_BURST0TO3(len) | LCDC_TVD_BURST0TO3(len);
    LCDC_DBG("%s: burst len %d for all layers", __func__, len);
    return LCDC_OK;
}

int hal_lcdc_set_vmirro_enable(uint8_t lid, bool enable)
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

int hal_lcdc_sl_dmaburst_length_set(uint8_t len)
{
    lcdc->REG_1DC &= ~(LCDC_DMA_BURST0TO3_MASK | LCDC_GRA_BURST0TO3_MASK |
                       LCDC_TVG_BURST0TO3_MASK | LCDC_TVD_BURST0TO3_MASK);
    lcdc->REG_1DC |= LCDC_DMA_BURST0TO3(len) | LCDC_GRA_BURST0TO3(len) |
                     LCDC_TVG_BURST0TO3(len) | LCDC_TVD_BURST0TO3(len);
    LCDC_DBG("%s: burst len %d for all layers", __func__, len);
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

int hal_lcdc_dec_swap_rb(bool enable)
{
    if (enable)
        lcdc->REG_2D0 |= LCDC_DEC_SWAP_RB;
    else
        lcdc->REG_2D0 &= ~LCDC_DEC_SWAP_RB;
    LCDC_DBG("%s: decnano swap RB %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

int hal_lcdc_dsi0_layer_select(int layer_fg, int layer_bg)
{
    uint32_t v = lcdc->REG_2C0;

    /* dsi0 always uses data0 */
    v &= ~(LCDC_DSI0_DATA_SEL | LCDC_GRA_IN_VLD_STEREO_L | LCDC_DMA_IN_VLD_STEREO_L |
           LCDC_TVG_IN_VLD_STEREO_L);

    if (layer_fg == LCDC_LAYER_GRA || layer_bg == LCDC_LAYER_GRA)
        v |= LCDC_GRA_IN_VLD_STEREO_L;
    if (layer_fg == LCDC_LAYER_DMA || layer_bg == LCDC_LAYER_DMA)
        v |= LCDC_DMA_IN_VLD_STEREO_L;
    if (layer_fg == LCDC_LAYER_TVG || layer_bg == LCDC_LAYER_TVG)
        v |= LCDC_TVG_IN_VLD_STEREO_L;

    lcdc->REG_2C0 = v;

    LCDC_DBG("%s: DSI0 fg layer %d, bg layer %d", __func__, layer_fg, layer_bg);
    return LCDC_OK;
}

int hal_lcdc_blend(int blend_mode, int alpha_mode, uint8_t alpha)
{
    lcdc->REG_1E8 &= ~LCDC_OVTOP_MASK;
    lcdc->REG_1E8 |= LCDC_OVTOP(blend_mode);

    lcdc->REG_194 &= ~(LCDC_PN_ALPHA_MODE_MASK | LCDC_PN_ALPHA_MASK);
    lcdc->REG_194 |= LCDC_PN_ALPHA_MODE(alpha_mode) | LCDC_PN_ALPHA(alpha);

    LCDC_DBG("%s: blend mode %d, alpha mode %d, const alpha 0x%02x",
            __func__, blend_mode, alpha_mode, alpha);
    return LCDC_OK;
}

int hal_lcdc_dsi0_blend(int blend_mode, int alpha_mode, uint8_t alpha)
{
    return hal_lcdc_blend(blend_mode, alpha_mode, alpha);
}

int hal_lcdc_dsi1_layer_select(int layer_fg, int layer_bg)
{
    uint32_t v1 = lcdc->REG_2C0;
    uint32_t v2 = lcdc->REG_2C8;

    /* dsi1 always uses data1 */
    v1 &= ~(LCDC_GRA_IN_VLD_STEREO_R | LCDC_DMA_IN_VLD_STEREO_R | LCDC_TVG_IN_VLD_STEREO_R);
    v2 &= ~(LCDC_DATA1_GRA_IN_VLD_R | LCDC_DATA1_DMA_IN_VLD_R | LCDC_DATA1_TVG_IN_VLD_R);
    v1 |= LCDC_DSI1_DATA_SEL;

    if (layer_fg == LCDC_LAYER_GRA || layer_bg == LCDC_LAYER_GRA) {
        v1 |= LCDC_GRA_IN_VLD_STEREO_R;
        v2 |= LCDC_DATA1_GRA_IN_VLD_R;
    }
    if (layer_fg == LCDC_LAYER_DMA || layer_bg == LCDC_LAYER_DMA) {
        v1 |= LCDC_DMA_IN_VLD_STEREO_R;
        v2 |= LCDC_DATA1_DMA_IN_VLD_R;
    }
    if (layer_fg == LCDC_LAYER_TVG || layer_bg == LCDC_LAYER_TVG) {
        v1 |= LCDC_TVG_IN_VLD_STEREO_R;
        v2 |= LCDC_DATA1_TVG_IN_VLD_R;
    }

    lcdc->REG_2C0 = v1;
    lcdc->REG_2C8 = v2;

    LCDC_DBG("%s: DSI1 fg layer %d, bg layer %d", __func__, layer_fg, layer_bg);
    return LCDC_OK;
}

int hal_lcdc_dsi1_blend(int blend_mode, int alpha_mode, uint8_t alpha)
{
    uint32_t v = lcdc->REG_2C8;

    v &= ~(LCDC_DATA1_BLEND_MODE_MASK | LCDC_DATA1_PN_ALPHA_PIXEL_MODE |
           LCDC_DATA1_TV_ALPHA_PIXEL_MODE);

    v |= LCDC_DATA1_BLEND_MODE(blend_mode) | LCDC_DATA1_PN_ALPHA(alpha) |
         LCDC_DATA1_TV_ALPHA(alpha);

    if (alpha_mode == LCDC_PIXEL_ALPHA)
        v |= (LCDC_DATA1_PN_ALPHA_PIXEL_MODE | LCDC_DATA1_TV_ALPHA_PIXEL_MODE);

    lcdc->REG_2C8 = v;

    LCDC_DBG("%s: DSI1 blend mode %d, alpha mode %d, const alpha 0x%02x",
            __func__, blend_mode, alpha_mode, alpha);
    return LCDC_OK;
}

int hal_lcdc_rgbif_enable(bool enable, bool clk_inv, bool hsync_inv, bool vsync_inv)
{
    uint32_t v = lcdc->REG_1C0;

    if (enable) {
        v |= LCDC_PARALLEL_CLK_ENA | LCDC_LCDC_PARA0_ENA |
             LCDC_GRA_IN_VLD_STEREO_L | LCDC_DMA_IN_VLD_STEREO_L | LCDC_TVG_IN_VLD_STEREO_L |
             LCDC_GRA_IN_VLD_STEREO_R | LCDC_DMA_IN_VLD_STEREO_R | LCDC_TVG_IN_VLD_STEREO_R;
    } else {
        v &= ~(LCDC_PARALLEL_CLK_ENA | LCDC_LCDC_PARA0_ENA);
    }

    if (clk_inv)
        v |= LCDC_PARALLEL_CLK_INV;
    else
        v &= ~LCDC_PARALLEL_CLK_INV;

    lcdc->REG_2C0 = v;

    if (hsync_inv || vsync_inv) {
        v = lcdc->REG_1B8;
        if (hsync_inv)
            v |= LCDC_PN_INV_HSYNC;
        if (vsync_inv)
            v |= LCDC_PN_INV_VSYNC;
        lcdc->REG_1B8 = v;
    }

    LCDC_DBG("%s: rgb interface %s, clk_inv %d, hsync_inv %d, vsync_inv %d",
            __func__, enable ? "enabled" : "disabled", clk_inv, hsync_inv, vsync_inv);
    return LCDC_OK;
}

int hal_lcdc_dump_regs(void)
{
    volatile uint32_t *reg = (uint32_t *)LCDC_BASE;
    LCDC_INFO("Dump all registers, base %p:", reg);
    for (int i = 0; i < (0x2E0 / 4); i++) {
        LCDC_INFO("  %p = 0x%08x", reg, *reg);
        reg++;
        hal_sys_timer_delay(MS_TO_TICKS(1));
    }
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

int hal_lcdc_rotate_enable(bool enable, uint16_t w, uint16_t h)
{
    if (enable) {
        lcdc->REG_2D4 |= LCDC_GRA_H_PIXEL_ROT(w) | LCDC_GRA_V_LINE_ROT(h);
        lcdc->REG_2D8 |= LCDC_GRARA_ENA | LCDC_ROT_90_DEGREES | LCDC_LONG_WIDTH_ROT | LCDC_GRA_H_PIXEL_ROT_ACTIVE(w);
    } else {
        lcdc->REG_2D4 &= ~( LCDC_GRA_H_PIXEL_ROT_MASK | LCDC_GRA_V_LINE_ROT_MASK);
        lcdc->REG_2D8 &= ~( LCDC_GRARA_ENA | LCDC_ROT_90_DEGREES | LCDC_LONG_WIDTH_ROT | LCDC_GRA_H_PIXEL_ROT_ACTIVE_MASK);
    }
    LCDC_DBG("%s: lcdc rodate %s", __func__, enable ? "enabled" : "disabled");
    return LCDC_OK;
}

#endif
