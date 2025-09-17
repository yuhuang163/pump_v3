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
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "string.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_csi.h"
#include "hal_dsi.h"
#include "video_misc.h"
#include "csi_dsi_conf.h"
#include "heap_api.h"
#ifndef MEDIA_HEAP_USE_STATIC_BUFF
#include "stdlib.h"
#endif

#include "reg_lcdc.h"
#include "reg_dsi.h"
#include "reg_csi.h"

#define DEBUG_REGS_DUMP_FLAG 0

#ifndef ALIGNED4
#define ALIGNED4                        ALIGNED(4)
#endif

#ifdef MEDIA_HEAP_ENABLE
#define MEDIA_HEAP_ALIGN_SIZE      0x200
#define MEDIA_HEAP_HDR_SIZE        0x24
#define MEDIA_HEAP_ADDR_LIST_SIZE  8

struct MEDIA_HEAP_ADDR_LIST_T {
    bool used;
    uint8_t *raw_ptr;
    uint8_t *align_ptr;
    uint32_t size;
};

#ifdef MEDIA_HEAP_USE_STATIC_BUFF
static heap_handle_t media_heap_handle = NULL;
static uint32_t media_heap_buff_size = MEDIA_HEAP_SIZE;
static ALIGNED4 uint8_t media_heap_buff[MEDIA_HEAP_SIZE];
#endif /* MEDIA_HEAP_USE_STATIC_BUFF */

static struct MEDIA_HEAP_ADDR_LIST_T heap_addr_list[MEDIA_HEAP_ADDR_LIST_SIZE];
#endif /* MEDIA_HEAP_ENABLE */

#ifdef CSI_FPS_STAT_SLOW
#define FPS_STAT_SLOW_ENABLE 1
#else
#define FPS_STAT_SLOW_ENABLE 0
#endif

static struct VIDEO_FPS_STAT_T g_video_fps_stat[VIDEO_FPS_STAT_ID_QTY];

void dsi_debug_regs_dump(int flag);

static struct VIDEO_FPS_STAT_T *video_fps_stat_get(enum VIDEO_FPS_STAT_ID_T id)
{
    ASSERT(id < VIDEO_FPS_STAT_ID_QTY, "[%s]: Invalid id %d", __func__, id);
    return &g_video_fps_stat[id];
}

void video_fps_stat_print(enum VIDEO_FPS_STAT_ID_T id)
{
    struct VIDEO_FPS_STAT_T *s = video_fps_stat_get(id);

    if (id == VIDEO_FPS_STAT_ID_CSI) {
        if (s->fps_en) {
            TRACE(1, "CSI_FPS_INFO: fps=%d, p=%d, dt=%d, n=%d", s->fps, s->p, s->dt, s->n);
        }
        if (s->los_en) {
            TRACE(1, "CSI_IRQ_LOSS: ratio=%d %%, cnt=%d / %d, rst=%d", s->lr, s->lc, s->ln + s->lc, s->lrst);
        }
        if (s->crc_en) {
            TRACE(1, "CSI_CRC_ERR : ratio=%d %%, cnt=%d / %d, ecode=%x", s->crc_er, s->crc_ec, s->crc_n, s->ecode);
        }
        if (s->frm_en) {
            TRACE(1, "CSI_FRM_ERR : ratio=%d %%, cnt=%d / %d, ecode=%x", s->frm_er, s->frm_ec, s->frm_n, s->ecode);
        }
        dsi_debug_regs_dump(DEBUG_REGS_DUMP_FLAG);

    } else {
        if (s->fps_en) {
            TRACE(1, "DSI_FPS_INFO: fps=%d, p=%d, dt=%d, n=%d", s->fps, s->p, s->dt, s->n);
        }
        if (s->los_en) {
            TRACE(1, "DSI_IRQ_LOSS: ratio=%d %%, cnt=%d / %d, rst=%d", s->lr, s->lc, s->ln + s->lc, s->lrst);
        }
        if (s->crc_en) {
            TRACE(1, "DSI_CRC_ERR : ratio=%d %%, cnt=%d / %d, ecode=%x", s->crc_er, s->crc_ec, s->crc_n, s->ecode);
        }
        if (s->frm_en) {
            TRACE(1, "DSI_FRM_ERR : ratio=%d %%, cnt=%d / %d, ecode=%x", s->frm_er, s->frm_ec, s->frm_n, s->ecode);
        }
    }
}

void video_fps_stat_calc(enum VIDEO_FPS_STAT_ID_T id, uint32_t errcode)
{
    bool print = false;
    struct VIDEO_FPS_STAT_T *s = video_fps_stat_get(id);
    uint32_t time = TICKS_TO_MS(hal_sys_timer_get());

    s->n++;
    if (s->n > 1) {
        if (time - s->pt >= 1000) {
            s->n = 1;
        }
    }
    if (s->crc_n == 0) {
        s->crc_ec = 0;
        s->crc_er = 0;
        s->crc_n  = 1;
    }
    if (s->frm_n == 0) {
        s->frm_ec = 0;
        s->frm_er = 0;
        s->frm_n  = 1;
    }
    if (s->ln == 0) {
        s->lc = 0;
        s->lr = 0;
        s->ln = 1;
        s->lrst = 0;
    }
    if (s->n == 1) {
        s->bt = time;
        s->pt = time;
        s->st = 0;
        return;
    }

    s->ct = time;
    s->dt = s->ct - s->bt;
    s->st = s->st + s->dt;
    s->bt = s->ct;
    s->p = s->st / (s->n-1);

    // calc fps
    if (s->fps_en) {
        if (s->p == 0) {
            s->p = 60;
            TRACE(1, "[%s]: WARNING: zero p", __func__);
        }
        s->fps = (1000 + (s->p + 1) / 2) / s->p;
    }

    if (id == VIDEO_FPS_STAT_ID_CSI) {
        //calc crc error
        if (s->crc_en) {
            s->crc_n++;
            if (errcode & 0x400) {
                s->crc_ec++;
            }
            if (s->crc_n == 0) {
                s->crc_n  = 1;
                s->crc_ec = 0;
                s->crc_er = 0;
            }
            s->crc_er = (100 * s->crc_ec) / (s->crc_n); //assume first time no CRC error
        }
        //calc frame error
        if (s->frm_en) {
            s->frm_n++;
            if (errcode & 0x8000) {
                s->frm_ec++;
            }
            if (s->frm_n == 0) {
                s->frm_n  = 1;
                s->frm_ec = 0;
                s->frm_er = 0;
            }
            s->frm_er = (100 * s->frm_ec) / (s->frm_n); //assume first time no FRM error
        }
    }
    //calc lost ratio
    if (s->los_en) {
        s->ln++;
        if (s->ln == 0) {
            s->lc = 0;
            s->lr = 0;
            s->ln = 1;
            s->lrst = 0;
        }
        if (s->ln > 10) {
            volatile uint32_t cp = s->dt;
            volatile uint32_t ep = s->p;
            volatile uint32_t lp = ep + ep * 2 / 5; //lp = 1.4 * ep
            volatile uint32_t hp = ep + ep * 4 / 5; //hp = 1.8 * ep
            volatile uint32_t i = 0;

            while (1) {
                if (cp >= hp) {
                    cp = cp - ep;
                    s->lc++;
                } else if (cp >= lp && cp < hp) {
                    cp = cp - ep;
                    s->lc++;
                } else { //cp < lp
                    break;
                }
                i++;
                if (i >= 10) {
                    s->lc = 0;
                    s->lr = 0;
                    s->ln = 1;
                    s->lrst++;
                    TRACE(1, "[%s]: WARNING: lost ratio error", __func__);
                    break;
                }
            }
            uint32_t m = s->lc;
            uint32_t n = s->ln;
            s->lr = (m * 100) / (n + m);
        }
    }
    s->ecode = errcode;

    if (s->slow_en) {
        if (s->n == 2) {
            print = true;
        } else {
            print = false;
        }
    } else {
        print = true;
    }
    if (print) {
        video_fps_stat_print(id);
    }
}

void video_fps_stat_open(enum VIDEO_FPS_STAT_ID_T id)
{
    struct VIDEO_FPS_STAT_T *s = video_fps_stat_get(id);

    if (!s->open) {
        if (id == VIDEO_FPS_STAT_ID_CSI) {
            s->fps_en = 1;
            s->crc_en = 1;
            s->frm_en = 1;
            s->los_en = 1;
            s->slow_en = FPS_STAT_SLOW_ENABLE;
        } else {
            s->fps_en = 1;
            s->crc_en = 0;
            s->frm_en = 0;
            s->los_en = 1;
            s->slow_en = FPS_STAT_SLOW_ENABLE;
        }
        s->open = 1;
    }
}

void video_fps_stat_close(enum VIDEO_FPS_STAT_ID_T id)
{
    struct VIDEO_FPS_STAT_T *s = video_fps_stat_get(id);
    if (s->open) {
        s->open = 0;
    }
}

void video_fps_stat_enable(enum VIDEO_FPS_STAT_ID_T id, bool enable)
{
    struct VIDEO_FPS_STAT_T *s = video_fps_stat_get(id);

    if (enable) {
        s->n = 0;
        s->bt = 0;
        s->ct = 0;
        s->dt = 0;
        s->st = 0;
        s->fps = 0;
        s->enable = 1;
    } else {
        s->enable = 0;
    }
}

static enum GPIO_TRIG_STATE_T csi_trig_state;
static bool csi_trig_enable = false;
static struct GPIO_TRIG_CONFIG_T gpio_trig_cfg;

static void gpio_trigger_csi_callback(int pin, enum GPIO_TRIG_STATE_T state)
{
    GPIO_TRIG_CALLBACK_T callback = gpio_trig_cfg.callback;

    if (callback) {
        callback(pin, state);
    }
}

static void csi_sync_pin_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    static struct HAL_GPIO_IRQ_CFG_T gpiocfg;

    enum GPIO_TRIG_STATE_T next_state = TRIG_STATE_SYNC_BEGIN;

    switch (csi_trig_state) {
    case TRIG_STATE_SYNC_END:
        next_state = TRIG_STATE_SYNC_BEGIN;
        gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
        break;
    case TRIG_STATE_SYNC_BEGIN:
    default:
        next_state = TRIG_STATE_SYNC_END;
        gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
        break;
    }
    if (csi_trig_enable) {
        gpio_trigger_csi_callback(pin, csi_trig_state);
    }
//    TRACE(1, "csi_trig_state:[%d]->[%d]", csi_trig_state, next_state);
    csi_trig_state = next_state;

    gpiocfg.irq_enable   = true;
    gpiocfg.irq_debounce = false;
    gpiocfg.irq_handler  = csi_sync_pin_irq_handler;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE;
    hal_gpio_setup_irq(CSI_SYNC_IN_PIN, &gpiocfg);
}

static void csi_sync_out_pin_init(void)
{
    uint32_t addr, val;

    const struct HAL_IOMUX_PIN_FUNCTION_MAP csi_pin_map[] = {
        {CSI_SYNC_OUT_PIN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
    };

    hal_iomux_init(csi_pin_map, ARRAY_SIZE(csi_pin_map));
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)CSI_SYNC_OUT_PIN, HAL_GPIO_DIR_OUT, 0);

    TRACE(0, "[%s]: ", __func__);

    // ana_test_dir
    addr = 0x40086014;
    val = *(volatile uint32_t *)addr;
    TRACE(0, "RD iomux_reg[%x]=%x", addr, val);
    val |= (1<<24);
//    val = 0x01000000;
    *(volatile uint32_t *)addr = val;
    TRACE(0, "WR iomux_reg[%x]=%x", addr, val);

    // ana_test_sel
    addr = 0x40086018;
    val = *(volatile uint32_t *)addr;
    TRACE(0, "RD iomux_reg[%x]=%x", addr, val);
    val &= ~0xF;
    val |= 0xB;
//    val = 0x0000000B;
    *(volatile uint32_t *)addr = val;
    TRACE(0, "WR iomux_reg[%x]=%x", addr, val);

    // ana tport sel
    addr = 0x40086010;
    val = *(volatile uint32_t *)addr;
    TRACE(0, "RD iomux_reg[%x]=%x", addr, val);
    val &= ~0x0F;
    val |= 0x0E; //select ana tport
    *(volatile uint32_t *)addr = val;
    TRACE(0, "WR iomux_reg[%x]=%x", addr, val);
}

static void csi_sync_in_pin_init(void)
{
    const struct HAL_IOMUX_PIN_FUNCTION_MAP csi_pin_map[] = {
        {CSI_SYNC_IN_PIN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
    };

    TRACE(0, "[%s]: ", __func__);

    hal_iomux_init(csi_pin_map, ARRAY_SIZE(csi_pin_map));
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)CSI_SYNC_IN_PIN, HAL_GPIO_DIR_IN, 1);
}

void gpio_trigger_csi_open(struct GPIO_TRIG_CONFIG_T *cfg)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;

    TRACE(0, "[%s]: ", __func__);

    csi_sync_in_pin_init();
    csi_sync_out_pin_init();

    csi_trig_enable = false;
    csi_trig_state = TRIG_STATE_SYNC_BEGIN;
    gpio_trig_cfg = *cfg;

    gpiocfg.irq_enable   = true;
    gpiocfg.irq_debounce = false;
    gpiocfg.irq_handler  = csi_sync_pin_irq_handler;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE;
    hal_gpio_setup_irq(CSI_SYNC_IN_PIN, &gpiocfg);
}

void gpio_trigger_csi_close(void)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;

    TRACE(0, "[%s]: ", __func__);

    gpiocfg.irq_enable   = false;
    gpiocfg.irq_debounce = false;
    gpiocfg.irq_handler  = csi_sync_pin_irq_handler;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE;
    hal_gpio_setup_irq(CSI_SYNC_IN_PIN, &gpiocfg);
}

void gpio_trigger_csi_enable(bool en)
{
    TRACE(0, "[%s]: en=%d", __func__, en);

    csi_trig_enable = en;
}

#ifdef MEDIA_HEAP_ENABLE
void media_heap_addr_list_init(void)
{
    uint32_t i;
    struct MEDIA_HEAP_ADDR_LIST_T *p;

    for (i = 0; i < MEDIA_HEAP_ADDR_LIST_SIZE; i++) {
        p = &heap_addr_list[i];
        p->used = false;
        p->raw_ptr = NULL;
        p->align_ptr = NULL;
        p->size = 0;
    }
}

int media_heap_addr_list_set(uint8_t *raw_ptr, uint8_t *align_ptr, uint32_t size)
{
    uint32_t i;
    struct MEDIA_HEAP_ADDR_LIST_T *p;

    for (i = 0; i < MEDIA_HEAP_ADDR_LIST_SIZE; i++) {
        p = &heap_addr_list[i];
        if (p->used == false) {
            p->raw_ptr = raw_ptr;
            p->align_ptr = align_ptr;
            p->size = size;
            p->used = true;
            return 0;
        }
    }
    return 1;
}

uint8_t *media_heap_addr_list_get_raw_ptr(uint8_t *align_ptr)
{
    uint32_t i;
    struct MEDIA_HEAP_ADDR_LIST_T *p;

    for (i = 0; i < MEDIA_HEAP_ADDR_LIST_SIZE; i++) {
        p = &heap_addr_list[i];
        if ((p->used == true) && (p->align_ptr == align_ptr)) {
            return p->raw_ptr;
        }
    }
    return NULL;
}

int media_heap_addr_list_clr_raw_ptr(uint8_t *align_ptr)
{
    uint32_t i;
    struct MEDIA_HEAP_ADDR_LIST_T *p;

    for (i = 0; i < MEDIA_HEAP_ADDR_LIST_SIZE; i++) {
        p = &heap_addr_list[i];
        if ((p->used == true) && (p->align_ptr == align_ptr)) {
            p->used = false;
            return 0;
        }
    }
    return 1;
}

void media_heap_init(void)
{
#ifdef MEDIA_HEAP_USE_STATIC_BUFF
    uint8_t *buf = (uint8_t *)media_heap_buff;
    uint32_t len = media_heap_buff_size;
    uint32_t align_addr, end_addr;

    TRACE(1, "[%s]: buf=0x%x, len=0x%x", __func__, (int)buf, len);

#ifdef MEDIA_HEAP_ADDR
    TRACE(1, "MEDIA_HEAP_ADDR=0x%x",MEDIA_HEAP_ADDR);
#endif
#ifdef MEDIA_HEAP_SIZE
    TRACE(1, "MEDIA_HEAP_SIZE=0x%x",MEDIA_HEAP_SIZE);
#endif
    TRACE(1, "MEDIA_HEAP_ALIGN_SIZE=0x%x, MEDIA_HEAP_HDR_SIZE=0x%x",
        MEDIA_HEAP_ALIGN_SIZE, MEDIA_HEAP_HDR_SIZE);
    align_addr = ((uint32_t)media_heap_buff + MEDIA_HEAP_ALIGN_SIZE) & (~(MEDIA_HEAP_ALIGN_SIZE-1));
#if !defined(CHIP_ROLE_DSP) && !defined(CHIP_ROLE_AP)
    align_addr = (align_addr - MEDIA_HEAP_HDR_SIZE);
#elif defined(CHIP_ROLE_DSP)
    align_addr = ((align_addr - MEDIA_HEAP_HDR_SIZE) & (~0xFF000000)) | PSRAMUHS_NC_BASE;
    //align_addr = (align_addr - MEDIA_HEAP_HDR_SIZE);
#elif defined(CHIP_ROLE_AP)
    align_addr = (align_addr - MEDIA_HEAP_HDR_SIZE);
#endif
    len = ((uint32_t)(buf+len) & (~0xFF000000)) - (align_addr & (~0xFF000000));
    end_addr = align_addr + len;
    TRACE(1, "heap loc addr=[%x - %x], size=0x%x B", align_addr, end_addr, len);
    media_heap_handle = heap_register((void *)align_addr, (size_t)len);
    ASSERT(media_heap_handle != NULL, "[%s]: heap_register failed", __func__);
#endif /* MEDIA_HEAP_USE_STATIC_BUFF */

    media_heap_addr_list_init();
}

uint8_t *media_heap_malloc(uint32_t size)
{
    int ret;
    uint8_t *raw_ptr = NULL, *align_ptr = NULL;

    size = size + MEDIA_HEAP_ALIGN_SIZE;
#ifdef MEDIA_HEAP_USE_STATIC_BUFF
    raw_ptr = heap_malloc(media_heap_handle, (size_t)size);
    ASSERT(raw_ptr != NULL, "[%s]: null ptr", __func__);
#else
    raw_ptr = malloc((size_t)size);
#endif
    align_ptr = (uint8_t *)(((uint32_t)raw_ptr+MEDIA_HEAP_ALIGN_SIZE)&(~(MEDIA_HEAP_ALIGN_SIZE-1)));
    ret = media_heap_addr_list_set(raw_ptr, align_ptr, size);
    ASSERT(ret==0, "[%s]: set addr list failed, ret=%d", __func__, ret);
    TRACE(1, "[%s]: raw_ptr=%x, align_ptr=%x, size=%d", __func__, (int)raw_ptr, (int)align_ptr, size);
    return align_ptr;
}

void media_heap_free(uint8_t *ptr)
{
    if (ptr) {
        uint8_t *raw_ptr;
        raw_ptr = media_heap_addr_list_get_raw_ptr(ptr);
        ASSERT(raw_ptr != NULL, "[%s]: raw_ptr is null", __func__);
#ifdef MEDIA_HEAP_USE_STATIC_BUFF
        heap_free(media_heap_handle, raw_ptr);
#else
        free(raw_ptr);
#endif
        media_heap_addr_list_clr_raw_ptr(ptr);
        TRACE(1, "[%s]: raw_ptr=%x, align_ptr=%x", __func__, (int)raw_ptr, (int)ptr);
    }
}
#endif /* MEDIA_HEAP_ENABLE */

void dsi_debug_regs_dump(int flag)
{
#define DSI_REG_BASE  DSI_BASE
#define LCDC_REG_BASE LCDC_BASE
#define CSI_REG_BASE  CSI_BASE
#define __DUMP_DELAY() hal_sys_timer_delay_us(10)

    uint32_t i;
    uint32_t dsi_reg_num = sizeof(struct DSI_REG_T) / 4;
    uint32_t lcdc_reg_num = sizeof(struct LCDC_REG_T) / 4;
    uint32_t csi_reg_num = sizeof(struct CSI_REG_T) / 4;
    volatile uint32_t *dsi_reg = (volatile uint32_t *)DSI_REG_BASE;
    volatile uint32_t *lcdc_reg = (volatile uint32_t *)LCDC_REG_BASE;
    volatile uint32_t *csi_reg = (volatile uint32_t *)CSI_REG_BASE;

    if (flag == 0) {
        return;
    }
    if (flag & 0x1) {
        TRACE(1, " ");
        for (i = 0; i < csi_reg_num; i++) {
            TRACE(1, "CSI_REG[%x] = %x", (uint32_t)&csi_reg[i], csi_reg[i]);
            __DUMP_DELAY();
        }
    }
    if (flag & 0x2) {
        TRACE(1, " ");
        for (i = 0; i < dsi_reg_num; i++) {
            TRACE(1, "DSI_REG[%x] = %x", (uint32_t)&dsi_reg[i], dsi_reg[i]);
            __DUMP_DELAY();
        }
    }
    if (flag & 0x4) {
        TRACE(1, " ");
        for (i = 0; i < lcdc_reg_num; i++) {
            TRACE(1, "LCDC_REG[%x] = %x", (uint32_t)&lcdc_reg[i], lcdc_reg[i]);
            __DUMP_DELAY();
        }
    }
    TRACE(1, " ");
}

