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
 * DOC: a dumy board lcd driver
 *  setup lcd display pannel
 *  Initialize the framebuffer video hardware associated with the display
 *
 */

#include <stdio.h>
#include "string.h"
#include "fb.h"
#include "lcdc.h"
#include "hal_dsi.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_timer.h"
#include "hal_cmu.h"
#include "hal_trace.h"
#include "hal_lcdc.h"
#include "hal_lcdc_qspi.h"
#include "hal_psc.h"
#include "hal_location.h"
#include "lpcmd.h"

#ifdef USE_GAMU_FB
#include "hal_gamu.h"
#include <math.h>
#endif

#ifdef CONFIG_LCDC_QSPI
#define lcd_send_cmd_data   hal_lcdc_qspi_send_command_data
#define lcd_send_cmd_list   hal_lcdc_qspi_send_command_list
#define lcd_send_cmd        hal_lcdc_qspi_send_command
#define lcd_read_cmd        hal_lcdc_qspi_read_command
#elif defined(CONFIG_LCDC_DSI)
#ifdef CONFIG_LCDC_USE_DSI1
static enum HAL_DSI_ID_E dsi_id = HAL_DSI_ID_1;
#else
static enum HAL_DSI_ID_E dsi_id = HAL_DSI_ID_0;
#endif
#define lcd_send_cmd_data(...)   hal_dsi_send_cmd_data(dsi_id, ##__VA_ARGS__)
#define lcd_send_cmd_list(...)   hal_dsi_send_cmd_list(dsi_id, ##__VA_ARGS__)
#define lcd_send_cmd(...)        hal_dsi_send_cmd(dsi_id, ##__VA_ARGS__)
#define lcd_read_cmd(...)        hal_dsi_read_cmd(dsi_id, ##__VA_ARGS__)
#else
#error "Must define CONFIG_LCDC_DSI or CONFIG_LCDC_QSPI for graphics driver."
#endif

#define LCD_INFO(...)  TR_INFO( TR_MOD(TEST), "[LCD]"  __VA_ARGS__)

#ifndef SCREEN_OFFSET
    #define SCREEN_OFFSET            0
#endif

#ifndef SCREEN_WIDTH
    #define SCREEN_WIDTH             BOARD_LCDC_WIDTH
#endif

#ifndef SCREEN_HEIGTH
    #define SCREEN_HEIGTH            BOARD_LCDC_HEIGHT
#endif

#ifdef USE_GAMU_FB
/*
 * modify the value to the size parameter of get_fb_phymem()
*/
    #if (CONFIG_LCDC_FB_BPP == 24)
        #define FB_BUF_SIZE    497948
    #elif (CONFIG_LCDC_FB_BPP == 32)
        #define FB_BUF_SIZE    834624
    #else
        #define FB_BUF_SIZE    834624
    #endif
#else
    #define FB_BUF_SIZE  (CONFIG_LCDC_FB_SIZE)
#endif

#if defined(CHIP_BEST2003)
#define fb_buf 0x3c000000
#else
#ifdef PSRAM_FB
#ifdef USE_GAMU_FB
PSRAM_BSS_LOC
//SRAM_BSS_LOC
#else
PSRAM_NC_BSS_LOC
#endif
#endif
static char fb_buf[FB_BUF_SIZE] ALIGNED(64) ;
#endif

#ifdef USE_GAMU_FB
    #ifdef CONFIG_FB_DOUBLE_BUFFER
       #ifdef PSRAM_FB
           PSRAM_BSS_LOC
           /*PSRAM_NC_BSS_LOC*/
       #endif
       static char gamu_mem_buf[FB_BUF_SIZE] ALIGNED(64) ;
    #endif
#endif
//#define SOCKET_BOARD

#ifdef SOCKET_BOARD
#define GPIO_RST_PIN HAL_GPIO_PIN_PMU_P1_3
#define GPIO_1V8_PIN HAL_GPIO_PIN_PMU_P2_1
#define GPIO_2V8_PIN HAL_GPIO_PIN_PMU_P2_2
#define GPIO_3V3_PIN HAL_GPIO_PIN_PMU_P2_3
#else
#if defined(CHIP_BEST2003)
#define GPIO_RST_PIN HAL_GPIO_PIN_P0_3
#define GPIO_LED_PIN HAL_GPIO_PIN_P2_6
#elif defined(CHIP_BEST1502X)
#define GPIO_RST_PIN HAL_GPIO_PIN_P8_0
#define GPIO_1V8_PIN HAL_GPIO_PIN_P9_1
#elif defined(CHIP_BEST1503)
#define GPIO_RST_PIN HAL_GPIO_PIN_P8_0
#define GPIO_1V8_PIN HAL_GPIO_PIN_P9_2
#elif defined(CHIP_BEST1600)
#ifdef DISPLAY_GC9503NP
#define GPIO_RST_PIN HAL_GPIO_PIN_PMU_P1_3
#else
#define GPIO_RST_PIN HAL_GPIO_PIN_P4_2 //HAL_GPIO_PIN_PMU_P1_3
#endif
#define GPIO_1V8_PIN HAL_GPIO_PIN_PMU_P2_1
#define GPIO_2V8_PIN HAL_GPIO_PIN_PMU_P2_2
#define GPIO_3V3_PIN HAL_GPIO_PIN_PMU_P2_3
#elif defined(CHIP_BEST1700)
#define GPIO_RST_PIN HAL_GPIO_PIN_P4_2
#define GPIO_1V8_PIN HAL_GPIO_PIN_PMU_P2_1
#define GPIO_2V8_PIN HAL_GPIO_PIN_PMU_P2_2
#define GPIO_3V3_PIN HAL_GPIO_PIN_PMU_P2_3
#else
#define GPIO_RST_PIN HAL_GPIO_PIN_P8_4
#endif
#endif

/**
 * Pre-processor Definitions
 */

#ifdef USE_GAMU_FB
static uint32_t get_fb_phymem(uint8_t id, uint32_t size, uint8_t aligns);

static gamu_fb_info_t  gamu_fb = {
    .id = 0,
    .width = BOARD_LCDC_WIDTH,  // must be 512 pixels for 1502x
    .height = BOARD_LCDC_WIDTH,
    .bpp = CONFIG_LCDC_FB_BPP,
    .align_malloc = get_fb_phymem,
    .sqrtf = sqrtf,
    // width = xoffset + 2* radius
    // height = yoffset + 2* radius
    .radius = SCREEN_WIDTH/2,//256,//,128,//48,
    .yoffset = SCREEN_OFFSET, // line offset
    .used_vblocks = 0,
    .vmem = 0,
};

#ifdef CONFIG_FB_DOUBLE_BUFFER
static gamu_fb_info_t  gamu_fb_01 = {
    .id = 1,
    .width = BOARD_LCDC_WIDTH,  // must be 512 pixels for 1502x
    .height = BOARD_LCDC_WIDTH,
    .bpp = CONFIG_LCDC_FB_BPP,
    .align_malloc = get_fb_phymem,
    .sqrtf = sqrtf,
    // width = xoffset + 2* radius
    // height = yoffset + 2* radius
    .radius = SCREEN_WIDTH/2,//256,//,128,//48,
    .yoffset = SCREEN_OFFSET, // line offset
    .used_vblocks = 0,
    .vmem = 0,
};
#endif
#endif

/**
 * Private Data
 */
POSSIBLY_UNUSED
static int putarea(fb_coord_t row_start, fb_coord_t row_end,
               fb_coord_t col_start, fb_coord_t col_end,
               FAR const uint8_t *buffer)
{
    uint16_t width,heigh;
    uint32_t fbmem = (uint32_t)buffer;
    col_start &= 0xfffe;
    row_start &= 0xfffe;
    width = (col_end - col_start +1)&0xfffe;
    heigh = (row_end - row_start +1)&0xfffe;

    uint16_t hstart = 14 + col_start;
    uint16_t hend = hstart+width -1;
    uint16_t vstart = row_start;
    uint16_t vend = vstart+heigh-1;

#ifdef CONFIG_LCDC_DSI
    hal_dsi_reset(dsi_id, width);
#endif
    lcd_send_cmd_data(0x2a, 4, hstart>>8, hstart&0xff, hend>>8, hend&0xff);
    lcd_send_cmd_data(0x2b, 4, vstart>>8, vstart&0xff, vend>>8, vend&0xff);

#ifdef CONFIG_FB_VMIRRO
    if(in_vmirro_mode()) {
        fbmem += (row_end*BOARD_LCDC_WIDTH*CONFIG_LCDC_FB_BPP/8)+
                  col_start*CONFIG_LCDC_FB_BPP/8;
    }
    else
#endif
    {
        fbmem += (row_start*BOARD_LCDC_WIDTH*CONFIG_LCDC_FB_BPP/8)+
                  col_start*CONFIG_LCDC_FB_BPP/8;
    }

    /* Configure Layer start addr register */
    hal_lcdc_lstartaddr(LCDC_LAYER_LFORE, fbmem);
    /* Configure Layer size register */
    hal_lcdc_lsize(LCDC_LAYER_LFORE, width,heigh);
    hal_lcdc_lsize(LCDC_LAYER_SPU, width,heigh);
    hal_lcdc_lzoom_set(LCDC_LAYER_LFORE, width,heigh);

    return 0;
}

static int lcdc_getplaneinfo(FAR struct lcd_dev_s *dev,
                               unsigned int planeno,
                               FAR struct lcd_planeinfo_s *pinfo)
{

  pinfo->putarea = putarea;
  return OK;
}

static int rm69330_getstatus_reg(int flag)
{
    lpcmd_node_t status[] = { //{ 0xFE, 1, LPCMD_TYPE_WRITE, { 0x00 } },
                              { 0x0A, 1, LPCMD_TYPE_READ, { 0x0 } } };

    lpcmds_register_cmds(status, ARRAY_SIZE(status), flag);
    TRACE(0,"[lcd] panel status register = 0x%x\n", status[0].params[0]);
    return status[0].params[0];
}

static void rm69330_entery_idle_mode(void)
{
    lpcmd_node_t idlemode[] = { { 0xFF, 1, LPCMD_TYPE_WRITE, { 0x10 } },
                            { 0xFB, 1, LPCMD_TYPE_WRITE, { 0x01 } },
                            { 0x39, 0, LPCMD_TYPE_WRITE, { 0x0 } },
                            { 0x51, 1, LPCMD_TYPE_WRITE, { 128 } }};

    lpcmds_register_cmds(idlemode, ARRAY_SIZE(idlemode), LPCMDS_NONBLOCK);
    if (0x9c == rm69330_getstatus_reg(LPCMDS_BLOCK))
    {
        TRACE(0,"[lcd] enter idle mode failed, will retry 1 times.");
        lpcmds_register_cmds(idlemode, ARRAY_SIZE(idlemode), LPCMDS_NONBLOCK);
    }
    TRACE(0,"[lcd] rm69330 enter idle mode, current power = %d\n",
        idlemode[3].params[0]);
}

static void rm69330_exit_idle_mode(void)
{
    lpcmd_node_t idlemode[] = { { 0xFF, 1, LPCMD_TYPE_WRITE, { 0x10 } },
                            { 0xFB, 1, LPCMD_TYPE_WRITE, { 0x01 } },
                            { 0x38, 0, LPCMD_TYPE_WRITE, { 0x0 } },
                            { 0x51, 1, LPCMD_TYPE_WRITE, { 128} } };

    lpcmds_register_cmds(idlemode, ARRAY_SIZE(idlemode), LPCMDS_NONBLOCK);
    if (0xdc == rm69330_getstatus_reg(LPCMDS_BLOCK))
    {
        TRACE(0,"[lcd] exit idle mode failed, will retry 1 times.");
        lpcmds_register_cmds(idlemode, ARRAY_SIZE(idlemode), LPCMDS_NONBLOCK);
    }

    TRACE(0,"[lcd] rm69330 exit idle mode, current power = %d\n",
        idlemode[3].params[0]);
}

static void rm69330_enter_deepstandby(void)
{
    lpcmd_node_t enterstandby[] = { //{ 0xFE, 1, LPCMD_TYPE_WRITE, { 0x00 } },
                                    { 0x51, 1, LPCMD_TYPE_WRITE, { 0x0 } },
                                    { 0x28, 0, LPCMD_TYPE_WRITE, { 0x0 } },
                                    { 0x10, 0, LPCMD_TYPE_WRITE, { 0x0 } },
                                  };

    lpcmds_register_cmds(enterstandby, ARRAY_SIZE(enterstandby),
                          LPCMDS_NONBLOCK);
    TRACE(0,"[lcd] rm69330 enter deep standby\n");
    while (0x88 != rm69330_getstatus_reg(LPCMDS_BLOCK))
    {
        TRACE(0,"[lcd] enter deep mode failed, will retry 1 times.");
        lpcmds_register_cmds(enterstandby, ARRAY_SIZE(enterstandby),
                              LPCMDS_NONBLOCK);
    }
}
int board_lcd_initialize(void);

static void rm69330_exit_deepstandby(void)
{
    lpcmd_node_t enterstandby[] = { //{ 0xFE, 1, LPCMD_TYPE_WRITE, { 0x00 } },
                                    { 0x51, 1, LPCMD_TYPE_WRITE, { 0xff } },
                                    { 0x11, 0, LPCMD_TYPE_WRITE, { 0x0 } },
                                    { 6, 0, LPCMD_TYPE_DELAY, { 0x0 } },
                                    { 0x29, 0, LPCMD_TYPE_WRITE, { 0x0 } },
                                  };

    lpcmds_clear_buffer();
    lpcmds_register_cmds(enterstandby, ARRAY_SIZE(enterstandby),
                          LPCMDS_NONBLOCK);
    TRACE(0,"[lcd] rm69330 exit deep standby\n");
    /* direct flush just can do from task for exit deepsleep*/
    lpcmds_flush();

    while (0x9c != rm69330_getstatus_reg(LPCMDS_BLOCK|LPCMDS_FLUSH))
    {
        TRACE(0,"[lcd] exit deep mode failed, will retry 1 times.");
        lpcmds_register_cmds(enterstandby, ARRAY_SIZE(enterstandby),
                              LPCMDS_NONBLOCK);
        lpcmds_flush();
    }
}


/****************************************************************************
* Name:  rm69330_setpower
****************************************************************************/
static int rm69330_setpower(FAR struct lcd_dev_s *dev, int power)
{
 TRACE(0,"set power: %d\n", power);
 if(power > 0)
 {
    power = power & 0xff;
 }
 else
 {
   power = 0;
 }
 lpcmd_node_t setlight[] =
 {
   { 0x51, 1, LPCMD_TYPE_WRITE, {power } },
 };
 lpcmds_register_cmds(setlight, ARRAY_SIZE(setlight), LPCMDS_NONBLOCK);
 return OK;
}
static void rm69330_init();

static int rm69330_ioctl(FAR struct lcd_dev_s *dev, PANNEL_IOCTL_CMD cmd)
{
 TRACE(0,"ioctl: %d\n", cmd);
 switch(cmd) {
    case PANNEL_ENTER_IDLE:
        rm69330_entery_idle_mode();
        break;
    case PANNEL_EXIT_IDLE:
        rm69330_exit_idle_mode();
        break;
    case PANNEL_ENTER_DEEPSLEEP:
        rm69330_enter_deepstandby();
        break;
    case PANNEL_EXIT_DEEPSLEEP:
        rm69330_exit_deepstandby();
        break;
    case PANNEL_READSTATUS:
        return rm69330_getstatus_reg(LPCMDS_BLOCK);
        break;
    default :
        break;
  }
 return OK;
}


struct lcd_dev_s g_lcd = {0};


/**
 * Private Functions
 */

/**
 * lcd_initialize - Initialize the  LCD controller to the RGB interface mode.
 *
 */
#define RM69330_XRES       454
#define RM69330_YRES       454
POSSIBLY_UNUSED
static void rm69330_init()
{
    uint16_t hstart = 14;
    uint16_t hend = RM69330_XRES-1+hstart;
    uint16_t vstart = 0;
    uint16_t vend = RM69330_YRES-1+vstart;
    g_lcd.getplaneinfo = lcdc_getplaneinfo;
    g_lcd.setpower     = rm69330_setpower;
    g_lcd.pannel_ioctl = rm69330_ioctl;

    lcd_send_cmd_data(0xfe, 0x1, 0x0, 0x0, 0x0, 0x0);
#ifdef CONFIG_DSI_VIDEO_MODE
    lcd_send_cmd_data(0xc2, 0x1, 0x3, 0x0, 0x0, 0x0);
#else
#ifdef CONFIG_LCDC_QSPI
    lcd_send_cmd_data(0xc4, 0x1, 0x80, 0x0, 0x0, 0x0);
#endif
    lcd_send_cmd_data(0x35, 0x1, 0x0, 0x0, 0x0, 0x0);
#endif
#if (CONFIG_LCDC_FB_BPP == 16)
    lcd_send_cmd_data(0x3a, 0x1, 0x55, 0x0, 0x0, 0x0);
#endif
    lcd_send_cmd_data(0x36, 0x1, 0x8, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x51, 0x1, 0xcf, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x2a, 4, hstart>>8, hstart&0xff, hend>>8, hend&0xff);
    lcd_send_cmd_data(0x2b, 4, vstart>>8, vstart&0xff, vend>>8, vend&0xff);
    lcd_send_cmd(0x11);
    hal_sys_timer_delay(MS_TO_TICKS(10));
    lcd_send_cmd(0x29);
}

POSSIBLY_UNUSED
static void rm69090_init(void)
{
     //DSI init-2
    lcd_send_cmd_data(0xfe, 0x1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xfe, 0x1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xfe, 0x1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x35, 0x1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x35, 0x1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x35, 0x1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x51, 0x1, 0xcf, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x51, 0x1, 0xcf, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x51, 0x1, 0xcf, 0x0, 0x0, 0x0);
    //lcd_send_cmd_data(0x2a, 4, 0x0, 0x10, 0x01, 0x7f);
    //lcd_send_cmd_data(0x2b, 4, 0x0, 0x0, 0x01, 0xbf);
    lcd_send_cmd_data(0x2a, 4, 0x0, 0xe, 0x1, 0xd3);
    lcd_send_cmd_data(0x2a, 4, 0x0, 0xe, 0x1, 0xd3);
    lcd_send_cmd_data(0x2b, 4, 0x0, 0x0, 0x1, 0xc5);
    lcd_send_cmd_data(0x2b, 4, 0x0, 0x0, 0x1, 0xc5);
    //DSI sleep out
    lcd_send_cmd(0x11);
    lcd_send_cmd(0x11);

    hal_sys_timer_delay(MS_TO_TICKS(240));

    //DSI disp on
    lcd_send_cmd(0x29);
    lcd_send_cmd(0x29);

    hal_sys_timer_delay(MS_TO_TICKS(150));
}

#define SCAN_LINE               0
#define REGFLAG_DELAY           0xFC
#define REGFLAG_END_OF_TABLE    0xFD    /* END OF REGISTERS MARKER */

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char params[64];
};

POSSIBLY_UNUSED
static struct LCM_setting_table lcm_st7701sn_initialization_setting[] = {
#if defined(DISPLAY_ST7785M)
    {REGFLAG_DELAY, 120, {}},
    {0x11, 0, {0}},
    {REGFLAG_DELAY, 120, {}},
    { 0x36, 2, {0x00, 0x00}},
    { 0x3A, 2, {0x00, 0x55}},
#ifndef CONFIG_DSI_VIDEO_MODE
    { 0xB0, 2, {0x00, 0xF1}},  //command mode
#else
    { 0xB0, 2, {0x00, 0x10}},  //video mode
#endif
    { 0xB2, 10, {0x00, 0x0C, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x33, 0x00, 0x33}},
    { 0xB7, 2, {0x00, 0x05}},
    { 0xBB, 2, {0x00, 0x11}},
    { 0xC0, 2, {0x00, 0x2C}},
    { 0xC2, 2, {0x00, 0x01}},
    { 0xC3, 2, {0x00, 0x13}},
    { 0xC6, 2, {0x00, 0x0F}},
    { 0xD0, 2, {0x00, 0xA7}},
    { 0xD0, 4, {0x00, 0xA4, 0x00, 0xA1}},
    { 0xD6, 2, {0x00, 0xA1}},
    { 0xE0, 28, {0x00, 0xF0, 0x00, 0x01, 0x00, 0x08, 0x00, 0x08, 0x00, 0x06, 0x00, 0x35, 0x00, 0x34, 0x00, 0x44, 0x00, 0x4A, 0x00, 0x38, 0x00, 0x13, 0x00, 0x11, 0x00, 0x2E, 0x00, 0x34}},
    { 0xE1, 28, {0x00, 0xF0, 0x00, 0x0A, 0x00, 0x0F, 0x00, 0x0E, 0x00, 0x0C, 0x00, 0x07, 0x00, 0x33, 0x00, 0x44, 0x00, 0x4A, 0x00, 0x26, 0x00, 0x10, 0x00, 0x12, 0x00, 0x2C, 0x00, 0x32}},
    { 0x29, 0, {}},
    { 0x21, 0, {}},
    { 0x2A, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF}},
    { 0x2B, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x3F}},
    { 0x2C,  0, {}},
#else
    {0XFF, 5, {0x77, 0x01 ,0x00, 0x00, 0x13}},
    {0xEF, 1, {0x08}},
    {0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x10}},
    {0xC0, 2, {0x4F, 0x00}},
    {0xC1, 2, {0x07, 0x02}},
    {0xC2, 2, {0x31, 0x07}},
    {0xCC, 1, {0x10}},
    {0xB0, 16, {0x00, 0x0A, 0x11, 0x0C, 0x10, 0x05, 0x00, 0x08, 0x08, 0x1F, 0x07, 0x13, 0x10, 0xA9, 0x30, 0x18}},
    {0xB1, 16, {0x00, 0x0B, 0x11, 0x0D, 0x0F, 0x05, 0x02, 0x07, 0x06, 0x20, 0x05, 0x15, 0x13, 0xA9, 0x30, 0x18}},
    {0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x11}},
    {0xB0, 1, {0x53}},
    {0xB1, 1, {0x60}},
    {0xB2, 1, {0x87}},
    {0xB3, 1, {0x80}},
    {0xB5, 1, {0x49}},
    {0xB7, 1, {0x85}},
    {0xB8, 1, {0x21}},
    {0xBB, 1, {0x33}},
    {0xBC, 1, {0x33}},
    {0xC1, 1, {0x78}},
    {0xC2, 1, {0x78}},
    {0xD0, 1, {0x88}},
    {REGFLAG_DELAY, 10, {}},
    {0xE0, 3, {0x00, 0x00, 0x02}},
    {0xE1, 11, {0x03, 0xA0, 0x00, 0x00, 0x02, 0xA0, 0x00, 0x00, 0x00, 0x33, 0x33}},
    {0xE2, 12, {0x22, 0x22, 0x33, 0x33, 0x88, 0xA0, 0x00, 0x00, 0x87, 0xA0, 0x00, 0x00}},
    {0xE3, 4, {0x00, 0x00, 0x22, 0x22}},
    {0xE4, 2, {0x44, 0x44}},
    {0xE5, 16, {0x04, 0x84, 0xA0, 0xA0, 0x06, 0x86, 0xA0, 0xA0, 0x08, 0x88, 0xA0, 0xA0, 0x0A, 0x8A, 0xA0, 0xA0}},
    {0xE6, 4, {0x00, 0x00, 0x22, 0x22}},
    {0xE7, 2, {0x44, 0x44}},
    {0xE8, 16, {0x03, 0x83, 0xA0, 0xA0, 0x05, 0x85, 0xA0, 0xA0, 0x07, 0x87, 0xA0, 0xA0, 0x09, 0x89, 0xA0, 0xA0}},
    {0xEB, 7, {0x00, 0x01, 0xE4, 0xE4, 0x88, 0x00, 0x40}},
    {0xEC, 2, {0x3C, 0x01}},
    {0xED, 16, {0xAB, 0x89, 0x76, 0x54, 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x45, 0x67, 0x98, 0xBA}},
    {0xEF, 6, {0x10, 0x0D, 0x04, 0x08, 0x3F, 0x1F}},
    {0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x13}},
    {0xE6, 2, {0x16, 0x7C}},
    {0xE8, 2, {0x00, 0x0E}},
    {0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x00}},
    {0x11, 0, {0}},
    {REGFLAG_DELAY, 120, {0}},
    {0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x13}},
    {0xE8, 2, {0x00, 0x0C}},
    {REGFLAG_DELAY, 10, {0}},
    {0xE8, 2, {0x00, 0x00}},
    {0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x00}},
    {0x29, 0, {0}},
    {REGFLAG_DELAY, 10, {}},
#if 0
    {0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x12}},
    {0xD1, 1, {0x81}},
    {0xD2, 1, {0x06}},
#endif
#endif
};

POSSIBLY_UNUSED
static struct LCM_setting_table lcm_gc9503np_initialization_setting[] = {
    {0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x00}},
    {0xF6, 2, {0x5A, 0x87}},
    {0xC1, 1, {0x3F}},
    {0xC2, 1, {0x0E}},
    {0xC6, 1, {0xF8}},
    {0xC9, 1, {0x10}},
    {0xCD, 1, {0x25}},
    {0x86, 4, {0x88, 0xA3, 0xA3, 0x31}},
    {0x87, 3, {0x04, 0x03, 0x66}},
    {0xAC, 1, {0x65}},
    {0xF8, 1, {0x8A}},
    {0xA7, 1, {0x47}},
    {0xA0, 1, {0xFF}},
    {0xB1, 1, {0x04}},
    {0xFA, 4, {0x08, 0x08, 0x08, 0x04}},
    {0x71, 1, {0x48}},
    {0x72, 1, {0x48}},
    {0x73, 2, {0x00, 0x44}},
    {0xA3, 1, {0xEE}},
    {0xFD, 3, {0x3C, 0x3C, 0x00}},
    {0x97, 1, {0xEE}},
    {0x83, 1, {0x93}},
    {0x77, 2, {0x00, 0x08}},
    {0x9A, 1, {0x98}},
    {0x9B, 1, {0x98}},
    {0x82, 2, {0x6F, 0x6F}},
    {0x80, 1, {0x0D}},
    {0x7A, 2, {0x13, 0x1A}},
    {0x7B, 2, {0x13, 0x1A}},
    {0x6D, 32, {0x1E, 0x1D, 0x1D, 0x1D, 0x1D, 0x1E, 0x02, 0x1E, 0x19, 0x1E, 0x1A, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1A, 0x1E, 0x19, 0x1E, 0x01, 0x1E, 0x1D, 0x1D, 0x1D, 0x1D, 0x1E}},
    {0x60, 8, {0x18, 0x08, 0x10, 0x70, 0x18, 0x07, 0x10, 0x70}},
    {0x64, 16, {0x18, 0x06, 0x03, 0x22, 0x70, 0x03, 0x18, 0x05, 0x03, 0x23, 0x70, 0x03, 0x10, 0x70, 0x10, 0x70}},
    {0x65, 16, {0x18, 0x04, 0x03, 0x23, 0x70, 0x03, 0x18, 0x03, 0x03, 0x23, 0x70, 0x03, 0x10, 0x70, 0x10, 0x70}},
    {0x66, 16, {0x18, 0x02, 0x03, 0x23, 0x70, 0x03, 0x18, 0x01, 0x03, 0x23, 0x70, 0x03, 0x10, 0x70, 0x10, 0x70}},
    {0x67, 16, {0x18, 0x00, 0x03, 0x23, 0x70, 0x03, 0x10, 0x01, 0x03, 0x23, 0x70, 0x03, 0x10, 0x70, 0x10, 0x70}},
    {0x68, 13, {0x00, 0x08, 0x1D, 0x08, 0x1C, 0x00, 0x00, 0x08, 0x11, 0x08, 0x10, 0x00, 0x00}},
    {0x69, 7, {0x14, 0x22, 0x14, 0x22, 0x14, 0x22, 0x08}},
    {0x6A, 8, {0x08, 0x20, 0x48, 0x16, 0x01, 0x23, 0x45, 0x67}},
    {0x6B, 1, {0xA7}},
    {0xD1, 52, {0x00, 0x00, 0x00, 0x0D, 0x00, 0x25, 0x00, 0x3D, 0x00, 0x58, 0x00, 0x9D, 0x00, 0xDE, 0x01, 0x2E, 0x01, 0x68, 0x01, 0xB5, 0x01, 0xEA, 0x02, 0x3A, 0x02, 0x74, 0x02, 0x76, 0x02, 0xAB, 0x02, 0xE0, 0x03, 0x01, 0x03, 0x26, 0x03, 0x40, 0x03, 0x65, 0x03, 0x7E, 0x03, 0x9F, 0x03, 0xB2, 0x03, 0xCA, 0x03, 0xE4, 0x03, 0xFF}},
    {0xD2, 52, {0x00, 0x00, 0x00, 0x05, 0x00, 0x11, 0x00, 0x23, 0x00, 0x3E, 0x00, 0x7D, 0x00, 0xB7, 0x01, 0x0D, 0x01, 0x45, 0x01, 0x93, 0x01, 0xCC, 0x02, 0x20, 0x02, 0x5D, 0x02, 0x5F, 0x02, 0x95, 0x02, 0xCB, 0x02, 0xEB, 0x03, 0x13, 0x03, 0x2F, 0x03, 0x58, 0x03, 0x73, 0x03, 0x97, 0x03, 0xAD, 0x03, 0xC6, 0x03, 0xE3, 0x03, 0xFF}},
    {0xD3, 52, {0x00, 0x00, 0x00, 0x02, 0x00, 0x07, 0x00, 0x10, 0x00, 0x1A, 0x00, 0x53, 0x00, 0x98, 0x00, 0xF7, 0x01, 0x2F, 0x01, 0x7B, 0x01, 0xB4, 0x02, 0x01, 0x02, 0x3C, 0x02, 0x3E, 0x02, 0x6F, 0x02, 0xA2, 0x02, 0xBA, 0x02, 0xDC, 0x02, 0xF0, 0x03, 0x0D, 0x03, 0x22, 0x03, 0x42, 0x03, 0x5D, 0x03, 0x85, 0x03, 0xBC, 0x03, 0xFF}},
    {0xD4, 52, {0x00, 0x00, 0x00, 0x0D, 0x00, 0x25, 0x00, 0x3D, 0x00, 0x58, 0x00, 0x9D, 0x00, 0xDE, 0x01, 0x2E, 0x01, 0x68, 0x01, 0xB5, 0x01, 0xEA, 0x02, 0x3A, 0x02, 0x74, 0x02, 0x76, 0x02, 0xAB, 0x02, 0xE0, 0x03, 0x01, 0x03, 0x26, 0x03, 0x40, 0x03, 0x65, 0x03, 0x7E, 0x03, 0x9F, 0x03, 0xB2, 0x03, 0xCA, 0x03, 0xE4, 0x03, 0xFF}},
    {0xD5, 52, {0x00, 0x00, 0x00, 0x05, 0x00, 0x11, 0x00, 0x23, 0x00, 0x3E, 0x00, 0x7D, 0x00, 0xB7, 0x01, 0x0D, 0x01, 0x45, 0x01, 0x93, 0x01, 0xCC, 0x02, 0x20, 0x02, 0x5D, 0x02, 0x5F, 0x02, 0x95, 0x02, 0xCB, 0x02, 0xEB, 0x03, 0x13, 0x03, 0x2F, 0x03, 0x58, 0x03, 0x73, 0x03, 0x97, 0x03, 0xAD, 0x03, 0xC6, 0x03, 0xE3, 0x03, 0xFF}},
    {0xD6, 52, {0x00, 0x00, 0x00, 0x02, 0x00, 0x07, 0x00, 0x10, 0x00, 0x1A, 0x00, 0x53, 0x00, 0x98, 0x00, 0xF7, 0x01, 0x2F, 0x01, 0x7B, 0x01, 0xB4, 0x02, 0x01, 0x02, 0x3C, 0x02, 0x3E, 0x02, 0x6F, 0x02, 0xA2, 0x02, 0xBA, 0x02, 0xDC, 0x02, 0xF0, 0x03, 0x0D, 0x03, 0x22, 0x03, 0x42, 0x03, 0x5D, 0x03, 0x85, 0x03, 0xBC, 0x03, 0xFF}},
    {0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    {0x6D, 32, {0x1E, 0x10, 0x0E, 0x0C, 0x0A, 0x1E, 0x02, 0x1E, 0x19, 0x1E, 0x1A, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1A, 0x1E, 0x19, 0x1E, 0x01, 0x1E, 0x09, 0x0B, 0x0D, 0x0F, 0x1E}},
    {0x77, 2, {0x00, 0x00}},
    {0xA6, 1, {0x56}},
    {0x29, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},
};

POSSIBLY_UNUSED
static int push_table(struct LCM_setting_table *table, int count)
{
  for(int i = 0; i < count; i++)
  {
    switch (table[i].cmd)
    {
    case REGFLAG_DELAY:
      //usleep(table[i].count * 1000);
      hal_sys_timer_delay(MS_TO_TICKS(table[i].count));
      break;
    /*case REG_READ_DELAY:
       {    uint8_t buf;
            lcd_read_cmd(table[i-1].cmd,&buf,table[i].count);
            if(buf == table[i-1].params[0]){
              break;
            }else{
              LCD_ERR("INIT ERR!!!!!!\n");
              return -1;
              break;
            }
       }*/
    default:
      lcd_send_cmd_list(table[i].cmd, table[i].count, table[i].params);
      break;
    }
  }
  return 0;
}

POSSIBLY_UNUSED
static void gc9503np_init(void)
{
    g_lcd.setpower     = rm69330_setpower;
    g_lcd.pannel_ioctl = rm69330_ioctl;
    push_table(lcm_gc9503np_initialization_setting,
          sizeof(lcm_gc9503np_initialization_setting) / sizeof(struct LCM_setting_table));
}

POSSIBLY_UNUSED
static void st7701sn_init(void)
{
    push_table(lcm_st7701sn_initialization_setting,
          sizeof(lcm_st7701sn_initialization_setting) / sizeof(struct LCM_setting_table));
}

#if defined(PROGRAMMER) && defined(CONFIG_LCDC_DSI)
void hal_dsi_send_hscmd(enum HAL_DSI_ID_E id, uint8_t cmd);

void dsi_programmer_test(void)
{
#define DSI_TST_CMD 0x29
    LCD_INFO("[%s]: \n", __func__);
    lcd_send_cmd(DSI_TST_CMD);
    osDelay(1);
    hal_dsi_send_hscmd(dsi_id, DSI_TST_CMD);
}

#define DSI_PROGRAMMER_TEST_START() dsi_programmer_test();
#define DSI_PROGRAMMER_TEST_STOP() {for(;;);}
#endif

#if defined(CONFIG_CLKP_REPAIR_GPIO) && (CONFIG_CLKP_REPAIR_GPIO != 0xff)
static void dsi_clkp_repair_gpio(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] =
    {
        {CONFIG_CLKP_REPAIR_GPIO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(CONFIG_CLKP_REPAIR_GPIO, HAL_GPIO_DIR_OUT, 0);
    hal_dsi_start_hs_clock(dsi_id);
    hal_sys_timer_delay(US_TO_TICKS(1));
    hal_gpio_pin_set_dir(CONFIG_CLKP_REPAIR_GPIO, HAL_GPIO_DIR_IN, 0);
}
#endif

#ifdef DISPLAY_A064
extern void panel_a064_init(void);
#endif

static int lcd_initialize(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] =
    {
        {GPIO_RST_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };

    LCD_INFO("[%s]: start", __func__);

    LCD_INFO("Init gpio pin ...");
#ifdef GPIO_LED_PIN
    pinmux[0].pin = GPIO_LED_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_LED_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_LED_PIN);
    LCD_INFO("LED_PIN:%d ",GPIO_LED_PIN);
#endif

#ifdef GPIO_1V8_PIN
    pinmux[0].pin = GPIO_1V8_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_1V8_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_1V8_PIN);
    LCD_INFO("1V8_PIN:%d ",GPIO_1V8_PIN);
#endif

#ifdef GPIO_2V8_PIN
    pinmux[0].pin = GPIO_2V8_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_2V8_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_2V8_PIN);
    LCD_INFO("2V8_PIN:%d ",GPIO_2V8_PIN);
#endif

#ifdef GPIO_3V3_PIN
    pinmux[0].pin = GPIO_3V3_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_3V3_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_3V3_PIN);
    LCD_INFO("3V3_PIN:%d ",GPIO_3V3_PIN);
#endif

    pinmux[0].pin = GPIO_RST_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_RST_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_RST_PIN);
    LCD_INFO("RST_PIN:%d ",GPIO_RST_PIN);

#if defined(DISPLAY_RM69330)
    LCD_INFO("reset device: RM69330");
    hal_sys_timer_delay(MS_TO_TICKS(100));
    hal_gpio_pin_clr(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(1));
    hal_gpio_pin_set(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(100));
#elif defined(DISPLAY_GC9503NP)
    LCD_INFO("reset device: GC9503NP");
    hal_sys_timer_delay(MS_TO_TICKS(20));
    hal_gpio_pin_clr(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(20));
    hal_gpio_pin_set(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(20));
#elif defined(DISPLAY_ST7701SN) || defined(DISPLAY_ST7785M)
    LCD_INFO("reset device: ST7701SN");
    hal_sys_timer_delay(MS_TO_TICKS(10));
    hal_gpio_pin_clr(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(20));
    hal_gpio_pin_set(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(120));
#endif

#ifdef CONFIG_LCDC_QSPI
    uint8_t cfmt = (CONFIG_LCDC_FB_BPP == 16) ? LCDC_QSPI_RGB565 : LCDC_QSPI_RGB888;
    LCD_INFO("init QSPI interface ...");
    hal_lcdc_qspi_initialize(cfmt, LCDC_QSPI_4LINE);
#elif defined(CONFIG_LCDC_DSI)
#ifdef CONFIG_DSI_VIDEO_MODE
    uint8_t mode = DSI_VIDEO_MODE;
#else
    uint8_t mode = DSI_COMMAND_MODE;
#endif
#ifdef CONFIG_DSI_2LANE_MODE
    uint8_t lane = DSI_LANE_2;
#else
    uint8_t lane = DSI_LANE_1;
#endif
    uint8_t cfmt = (CONFIG_LCDC_FB_BPP == 16) ? DSI_RGB565 : DSI_RGB888;
    LCD_INFO("init DSI interface ...");
    //hal_dsi_init(dsi_id, BOARD_LCDC_WIDTH, mode, lane, cfmt);
    hal_dsi_init(dsi_id, SCREEN_WIDTH, mode, lane, cfmt);
#if defined(CONFIG_CLKP_REPAIR_GPIO) && (CONFIG_CLKP_REPAIR_GPIO != 0xff)
    dsi_clkp_repair_gpio();
#endif
#endif

#if defined(PROGRAMMER) && defined(CONFIG_LCDC_DSI)
    DSI_PROGRAMMER_TEST_START();
#else /* !PROGRAMMER */

#if defined(DISPLAY_RM69330)
    LCD_INFO("init device RM69330...");
    rm69330_init();
#elif defined(DISPLAY_GC9503NP)
    LCD_INFO("init device GC9503NP...");
    gc9503np_init();
#elif defined(DISPLAY_ST7701SN) || defined(DISPLAY_ST7785M)
    LCD_INFO("init device ST7701SN...");
    st7701sn_init();
#elif defined(DISPLAY_A064)
    LCD_INFO("init device a064...");
    panel_a064_init();
#endif
#endif

    uint8_t mid=0,version=0,id=0,pwr=0;
    lcd_read_cmd(0xda, &mid, 1);
    lcd_read_cmd(0xdb, &version, 1);
    lcd_read_cmd(0xdc, &id, 1);
    lcd_read_cmd(0x0a, &pwr, 1);
    LCD_INFO("mid=%d,version=%d,id=%d,pwr=0x%x\n",mid,version,id,pwr);

    hal_sys_timer_delay(MS_TO_TICKS(100));
#ifdef CONFIG_LCDC_DSI
    LCD_INFO("start DSI interface");
    hal_dsi_start(dsi_id);
#endif
    LCD_INFO("[%s]: Done", __func__);

#if defined(PROGRAMMER) && defined(CONFIG_LCDC_DSI)
    DSI_PROGRAMMER_TEST_STOP();
#endif

  return OK;
}

/**
 * Public Functions
 */

/**
 * board_lcd_uninitialize - Uninitialize the LCD Device.
 *
 */
#define LCD_SLEEP_IN    0x10
#define LCD_DISPLAY_OFF 0x28
void board_lcd_uninitialize(void)
{
  /* display off */
  lcdc_exit_lowpower();
  lcd_send_cmd(LCD_DISPLAY_OFF);
  hal_sys_timer_delay(MS_TO_TICKS(10));
}


/**
 * board_lcd_getdev - Return a reference to the LCD object for the specified LCD Device.
 *   This allows support for multiple LCD devices.
 * @lcddev - Number of the LDC Device.
 *
 * Returns: Reference to the LCD object if exist otherwise NULL
 *
 */

FAR struct lcd_dev_s *board_lcd_getdev(int lcddev)
{
  return &g_lcd;
}

/**
 * board_lcd_initialize
 *
 *   Initialize the LCD video hardware. The initial state of the LCD is
 *   fully initialized, display memory cleared, and the LCD ready to use, but
 *   with the power setting at 0 (full off).
 *
 */

int board_lcd_initialize(void)
{
  /* check if always initialized */

  /* Initialize the sub driver structure */
  lcd_initialize();

  return OK;
}

/**
 * up_fbinitialize - Initialize the framebuffer video hardware associated with the display.
 * @display : In the case of hardware with multiple displays, this
 *     specifies the display.  Normally this is zero.
 *
 * Returns: Zero is returned on success; a negated errno value is returned on any
 *   failure.
 */

static bool initialized = false;
int up_fbinitialize(int display)
{
  int ret = OK;

  if (!initialized)
    {
      board_lcd_initialize();
      LCD_INFO("FBSIZE: %d\n",FB_BUF_SIZE );
      LCD_INFO("FBW: %d\n",BOARD_LCDC_WIDTH );
      LCD_INFO("FBH: %d\n",BOARD_LCDC_HEIGHT);

#ifdef USE_GAMU_FB

      if (gamu_fb.vmem == 0) {
          hal_gamu_setup(&gamu_fb);
      }
      #ifdef CONFIG_FB_DOUBLE_BUFFER)
          if (gamu_fb_01.vmem == 0) {
              hal_gamu_setup(&gamu_fb_01);
          }
      #endif
      ret = lcdc_initialize(gamu_fb.vmem, CONFIG_LCDC_FB_SIZE);
#else
      ret = lcdc_initialize((void *)fb_buf, FB_BUF_SIZE);
#endif
      initialized = (ret >= OK);

#ifdef DSI_BASE
      LCD_INFO("DSI: %X\n", DSI_BASE);
      //for(int i=0;i<30;i++)
      //    LCD_INFO("%x: %X\n", DSI_BASE+i*4, ((uint32_t *)DSI_BASE)[i]);
#endif
      LCD_INFO("LCDC: %X\n", LCDC_BASE);
      //for(int i=0;i<256;i++)
      //    LCD_INFO("%x: %X\n", LCDC_BASE+i*4, ((uint32_t *)LCDC_BASE)[i]);
    }

  return ret;
}

/**
 * up_fbgetvplane -  Return a a reference to the framebuffer object for the specified video
 *   plane of the specified plane.  Many OSDs support multiple planes of video.
 * @display : In the case of hardware with multiple displays, this
 *     specifies the display.  Normally this is zero.
 * @vplane : Identifies the plane being queried.
 *
 * Returns: A non-NULL pointer to the frame buffer access structure is returned on
 *   success; NULL is returned on any failure.
 *
 */

FAR struct fb_vtable_s *up_fbgetvplane(int display, int vplane)
{
  return lcdcgetvplane(vplane);
}

/**
 * up_fbuninitialize - Uninitialize the framebuffer support for the specified display.
 * @display : In the case of hardware with multiple displays, this
 *     specifies the display.  Normally this is zero.
 *
 */

void up_fbuninitialize(int display)
{
  if (initialized) {
      board_lcd_uninitialize();
      lcdcuninitialize();
      initialized = false;
  }
}


/**
 * get_fb_base - Get the fb buffer base address.
 *
 */

uint32_t get_fb_base(void)
{
#ifdef USE_GAMU_FB
      if (gamu_fb.vmem == 0) {
          hal_gamu_setup(&gamu_fb);
      }
      #ifdef CONFIG_FB_DOUBLE_BUFFER)
          if (gamu_fb_01.vmem == 0) {
              hal_gamu_setup(&gamu_fb_01);
          }
      #endif
      return (uint32_t)gamu_fb.vmem;
#else
    return (uint32_t)fb_buf;
#endif
}

#ifdef USE_GAMU_FB

/**
 * get_fb_phymem - Get the physical fb buffer address.
 *
 */
static uint32_t get_fb_phymem(uint8_t id, uint32_t size, uint8_t aligns)
{
    TRACE(0,"%s %d",__func__,size);
    if (id == 0) {
        return (uint32_t)fb_buf;
#ifdef CONFIG_FB_DOUBLE_BUFFER)
    } else if (id == 1) {
        return (uint32_t)gamu_mem_buf;
#endif
    }else{
        TRACE(0,"%s invalid gamu buffer id:%d",id);
        return (uint32_t)0;
    }
}

#endif
