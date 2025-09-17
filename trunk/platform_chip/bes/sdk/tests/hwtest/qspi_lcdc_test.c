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
#ifdef QSPI_LCDC_TEST
#include "string.h"
#include "cmsis.h"
#include "hal_psc.h"
#include "hal_cmu.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_lcdc.h"
#include "hal_lcdc_qspi.h"
#include "hal_location.h"

#define LCD_RM69330
//#define LCD_NR240x320 /* A narrow resolution for test */
//#define LCD_9C01
//#define LCD_ICNA3310   /* QSPI read working */

#ifdef LCD_RM69330
#define LCD_WIDTH   454
#define LCD_HEIGHT  454
#endif
#ifdef LCD_NR240x320
#define LCD_WIDTH       240
#define LCD_HEIGHT      320
#endif
#ifdef LCD_9C01
#define LCD_WIDTH   360
#define LCD_HEIGHT  360
#endif
#ifdef LCD_ICNA3310
#define LCD_WIDTH   368
#define LCD_HEIGHT  448
#endif

//#define TEST_PERF
//#define TEST_RGB565

#ifdef TEST_RGB565
#define LCDC_FORMAT     11
#define LCDC_STRIDE     (LCD_WIDTH * 2)
#else
#define LCDC_FORMAT     13
#define LCDC_STRIDE     (LCD_WIDTH * 4)
#endif
#define LCDC_SIZE       (LCDC_STRIDE*LCD_HEIGHT)

#define NCOLORS 10

POSSIBLY_UNUSED
static uint32_t test_colors_RGBA8888[NCOLORS] = {
    0xffffffff, //white
    0xff000000, //black
    0xffff0000, //blue
    0xff00ff00, //green
    0xff0000ff, //red
    0xff90ee90, //light green
    0xffa52a2a, //brown
    0xffadd8e6, //light blue
    0xffee82ee, //violet
    0xff00ffff, //yellow
};

POSSIBLY_UNUSED
static uint16_t test_colors_RGB565[NCOLORS] = {
    0xffff, //white
    0x0000, //black
    0x001f, //red
    0x07e0, //green
    0xf800, //blue
    0x0011, //dark red
    0x8410, //gray
    0x06bf, //gold
    0x8010, //purple
    0x07ff, //yellow
};

static uint8_t ALIGNED(64) sram_buf[LCDC_SIZE];
#ifdef PSRAM_ENABLE
static uint8_t ALIGNED(64) psram_buf[LCDC_SIZE] PSRAM_NC_BSS_LOC;
#endif

static uint32_t frame_start = 0;
static uint32_t frame_count = 0;
static bool framedone_irq_get = false;

/* TE irq */
static int te_gpio_pin = -1;
static bool te_irq_get = false;

/******************************** LCD Init ****************************************/

#define lcd_send_cmd_list   hal_lcdc_qspi_send_command_list
#define lcd_send_cmd_data   hal_lcdc_qspi_send_command_data
#define lcd_send_cmd        hal_lcdc_qspi_send_command
#define lcd_read_cmd_data   hal_lcdc_qspi_read_command

POSSIBLY_UNUSED
static int lcd_read_id(uint32_t *id)
{
    int ret = 0;
    uint8_t *ids = (uint8_t *)id;
    memset(id, 0, sizeof(uint32_t));
    ret = lcd_read_cmd_data(0xda, &ids[0], 1);
    if (ret < 0)
        return -1;
    ret = lcd_read_cmd_data(0xdb, &ids[1], 1);
    if (ret < 0)
        return -1;
    ret = lcd_read_cmd_data(0xdc, &ids[2], 1);
    if (ret < 0)
        return -1;
    return 0;
}

POSSIBLY_UNUSED
static int lcd_read_status(uint8_t *st)
{
    int ret = 0;
    ret = lcd_read_cmd_data(0x0a, st, 1);
    if (ret < 0)
        return -1;
    return 0;
}

#ifdef LCD_RM69330
POSSIBLY_UNUSED
static int lcd_rm69330_init(void)
{
    uint16_t hstart = 14;
    uint16_t hend = LCD_WIDTH-1+hstart;
    uint16_t vstart = 0;
    uint16_t vend = LCD_HEIGHT-1+vstart;
    uint8_t lcd_status = 0;
    uint32_t lcd_id = 0;

    if (lcd_read_status(&lcd_status) < 0) {
        TRACE(0, "Failed to read LCD power status.");
        return -1;
    }
    TRACE(0, "LCD power status: %02x", lcd_status);

    if (lcd_read_id(&lcd_id) < 0) {
        TRACE(0, "Failed to read LCD id.");
        return -1;
    }
    TRACE(0, "LCD id: 0x%06x", lcd_id);

    lcd_send_cmd_data(0xfe, 0x1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xc4, 0x1, 0x80, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x35, 0x1, 0x0, 0x0, 0x0, 0x0);
#ifdef TEST_RGB565
    lcd_send_cmd_data(0x3a, 0x1, 0x55, 0x0, 0x0, 0x0);
#else
    lcd_send_cmd_data(0x3a, 0x1, 0x77, 0x0, 0x0, 0x0);
#endif
    lcd_send_cmd_data(0x36, 0x1, 0x8, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x51, 0x1, 0xcf, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x2a, 4, hstart>>8, hstart&0xff, hend>>8, hend&0xff);
    lcd_send_cmd_data(0x2b, 4, vstart>>8, vstart&0xff, vend>>8, vend&0xff);
    lcd_send_cmd(0x11);
    hal_sys_timer_delay(MS_TO_TICKS(240));
    lcd_send_cmd(0x29);
    hal_sys_timer_delay(MS_TO_TICKS(150));

    TRACE(0, "RM69330 is initialized.");
    return 0;
}
#endif /* LCD_RM69330 */

#ifdef LCD_NR240x320
static int lcd_nr240x320_init(void)
{
    uint16_t hstart = 120;
    uint16_t hend = LCD_WIDTH-1+hstart;
    uint16_t vstart = 66;
    uint16_t vend = LCD_HEIGHT-1+vstart;

    lcd_send_cmd_data(0xfe, 0x1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xc4, 0x1, 0x80, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x35, 0x1, 0x0, 0x0, 0x0, 0x0);
#ifdef TEST_RGB565
    lcd_send_cmd_data(0x3a, 0x1, 0x55, 0x0, 0x0, 0x0);
#else
    lcd_send_cmd_data(0x3a, 0x1, 0x77, 0x0, 0x0, 0x0);
#endif
    lcd_send_cmd_data(0x36, 0x1, 0x8, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x51, 0x1, 0xcf, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x2a, 4, hstart>>8, hstart&0xff, hend>>8, hend&0xff);
    lcd_send_cmd_data(0x2b, 4, vstart>>8, vstart&0xff, vend>>8, vend&0xff);
    lcd_send_cmd(0x11);
    hal_sys_timer_delay(MS_TO_TICKS(240));
    lcd_send_cmd(0x29);
    hal_sys_timer_delay(MS_TO_TICKS(150));

    TRACE(0, "RM69330 [240x320] is initialized.");
    return 0;
}
#endif /* LCD_NR240x320 */

#ifdef LCD_9C01
static int lcd_9c01_init(void)
{
    uint8_t cmd_buf[50];
    uint16_t hstart = 14;
    uint16_t hend = LCD_WIDTH-1+hstart;
    uint16_t vstart = 0;
    uint16_t vend = LCD_HEIGHT-1+vstart;
    uint8_t lcd_status = 0;
    uint32_t lcd_id = 0;

    if (lcd_read_status(&lcd_status) < 0) {
        TRACE(0, "Failed to read LCD power status.");
        return -1;
    }
    TRACE(0, "LCD power status: %02x", lcd_status);

    if (lcd_read_id(&lcd_id) < 0) {
        TRACE(0, "Failed to read LCD id.");
        return -1;
    }
    TRACE(0, "LCD id: 0x%06x", lcd_id);

    lcd_send_cmd(0xfe);
    lcd_send_cmd(0xef);

    lcd_send_cmd_data(0x80, 1, 0x11, 0, 0, 0);
    lcd_send_cmd_data(0x81, 1, 0x70, 0, 0, 0);
    lcd_send_cmd_data(0x82, 1, 0x09, 0, 0, 0);
    lcd_send_cmd_data(0x83, 1, 0x03, 0, 0, 0);
    lcd_send_cmd_data(0x84, 1, 0x20, 0, 0, 0);
    lcd_send_cmd_data(0x85, 1, 0x42, 0, 0, 0);
    lcd_send_cmd_data(0x86, 1, 0xfc, 0, 0, 0);
    lcd_send_cmd_data(0x87, 1, 0x09, 0, 0, 0);
    lcd_send_cmd_data(0x89, 1, 0x10, 0, 0, 0);
    lcd_send_cmd_data(0x8a, 1, 0x4f, 0, 0, 0);
    lcd_send_cmd_data(0x8c, 1, 0x59, 0, 0, 0);
    lcd_send_cmd_data(0x8d, 1, 0x51, 0, 0, 0);
    lcd_send_cmd_data(0x8e, 1, 0xae, 0, 0, 0);
    lcd_send_cmd_data(0x8f, 1, 0xf3, 0, 0, 0);

    lcd_send_cmd_data(0x36, 1, 0x00, 0, 0, 0);
    lcd_send_cmd_data(0x3a, 1, 0x05, 0, 0, 0);
    lcd_send_cmd_data(0xef, 1, 0x70, 0, 0, 0);

    cmd_buf[0] = 0x01;
    cmd_buf[1] = 0x80;
    cmd_buf[2] = 0x00;
    cmd_buf[3] = 0x00;
    cmd_buf[4] = 0x00;
    cmd_buf[5] = 0x00;
    lcd_send_cmd_list(0x74, 6, cmd_buf);

    lcd_send_cmd_data(0x98, 1, 0x3e, 0, 0, 0);
    lcd_send_cmd_data(0x99, 1, 0x3e, 0, 0, 0);
    lcd_send_cmd_data(0xc3, 1, 0x2a, 0, 0, 0);
    lcd_send_cmd_data(0xc4, 1, 0x18, 0, 0, 0);

    lcd_send_cmd_data(0xa1, 2, 0x01, 0x04, 0, 0);
    lcd_send_cmd_data(0xa2, 2, 0x01, 0x04, 0, 0);
    lcd_send_cmd_data(0xa9, 1, 0x1c, 0, 0, 0);
    lcd_send_cmd_data(0xa5, 2, 0x11, 0x09, 0, 0);
    lcd_send_cmd_data(0xb9, 1, 0x8a, 0, 0, 0);
    lcd_send_cmd_data(0xa8, 1, 0x5e, 0, 0, 0);
    lcd_send_cmd_data(0xa7, 1, 0x40, 0, 0, 0);
    lcd_send_cmd_data(0xaf, 1, 0x73, 0, 0, 0);
    lcd_send_cmd_data(0xae, 1, 0x44, 0, 0, 0);
    lcd_send_cmd_data(0xad, 1, 0x38, 0, 0, 0);
    lcd_send_cmd_data(0xa3, 1, 0x5d, 0, 0, 0);

    lcd_send_cmd_data(0xc2, 1, 0x02, 0, 0, 0);
    lcd_send_cmd_data(0xc5, 1, 0x11, 0, 0, 0);
    lcd_send_cmd_data(0xc6, 1, 0x0e, 0, 0, 0);
    lcd_send_cmd_data(0xc7, 1, 0x13, 0, 0, 0);
    lcd_send_cmd_data(0xc8, 1, 0x0d, 0, 0, 0);
    lcd_send_cmd_data(0xcb, 1, 0x02, 0, 0, 0);
    lcd_send_cmd_data(0x7c, 2, 0xb6, 0x26, 0, 0);
    lcd_send_cmd_data(0xac, 1, 0x24, 0, 0, 0);
    lcd_send_cmd_data(0xf6, 1, 0xc0, 0, 0, 0);

    lcd_send_cmd_data(0xb5, 3, 0x09, 0x09, 0x14, 0);
    lcd_send_cmd_data(0x60, 4, 0x38, 0x0b, 0x6b, 0x56);
    lcd_send_cmd_data(0x63, 4, 0x3a, 0xe0, 0x5b, 0x56);

    cmd_buf[0] = 0x38;
    cmd_buf[1] = 0x0D;
    cmd_buf[2] = 0x72;
    cmd_buf[3] = 0xDD;
    cmd_buf[4] = 0x5B;
    cmd_buf[5] = 0x56;
    lcd_send_cmd_list(0x64, 6, cmd_buf);

    cmd_buf[0] = 0x38;
    cmd_buf[1] = 0x11;
    cmd_buf[2] = 0x72;
    cmd_buf[3] = 0xE1;
    cmd_buf[4] = 0x5B;
    cmd_buf[5] = 0x56;
    lcd_send_cmd_list(0x66, 6, cmd_buf);

    cmd_buf[0] = 0x3B;//FLC12 FREQ
    cmd_buf[1] = 0x08;
    cmd_buf[2] = 0x08;
    cmd_buf[3] = 0x00;
    cmd_buf[4] = 0x08;
    cmd_buf[5] = 0x29;
    cmd_buf[6] = 0x5B;
    lcd_send_cmd_list(0x68, 7, cmd_buf);

    cmd_buf[0] = 0x00;//gout1_swap_fw[4:0]
    cmd_buf[1] = 0x00;//gout2_swap_fw[4:0]
    cmd_buf[2] = 0x00;//gout3_swap_fw[4:0]
    cmd_buf[3] = 0x07;//gout4_swap_fw[4:0]
    cmd_buf[4] = 0x01;//gout5_swap_fw[4:0]
    cmd_buf[5] = 0x13;//gout6_swap_fw[4:0]
    cmd_buf[6] = 0x11;//gout7_swap_fw[4:0]
    cmd_buf[7] = 0x0B;//gout8_swap_fw[4:0]
    cmd_buf[8] = 0x09;//gout9_swap_fw[4:0]
    cmd_buf[9] = 0x16;//gout10_swap_fw[4:0]
    cmd_buf[10] = 0x15;//gout11_swap_fw[4:0]
    cmd_buf[11] = 0x1D;//gout12_swap_fw[4:0]
    cmd_buf[12] = 0x1E;//gout13_swap_fw[4:0]
    cmd_buf[13] = 0x00;//gout14_swap_fw[4:0]
    cmd_buf[14] = 0x00;//gout15_swap_fw[4:0]
    cmd_buf[15] = 0x00;//gout16_swap_fw[4:0]
    cmd_buf[16] = 0x00;//gout17_swap_fw[4:0]
    cmd_buf[17] = 0x00;//gout18_swap_fw[4:0]
    cmd_buf[18] = 0x00;//gout19_swap_fw[4:0]
    cmd_buf[19] = 0x1E;//gout20_swap_fw[4:0]
    cmd_buf[20] = 0x1D;//gout21_swap_fw[4:0]
    cmd_buf[21] = 0x15;//gout22_swap_fw[4:0]
    cmd_buf[22] = 0x16;//gout23_swap_fw[4:0]
    cmd_buf[23] = 0x0A;//gout24_swap_fw[4:0]
    cmd_buf[24] = 0x0C;//gout25_swap_fw[4:0]
    cmd_buf[25] = 0x12;//gout26_swap_fw[4:0]
    cmd_buf[26] = 0x14;//gout27_swap_fw[4:0]
    cmd_buf[27] = 0x02;//gout28_swap_fw[4:0]
    cmd_buf[28] = 0x08;//gout29_swap_fw[4:0]
    cmd_buf[29] = 0x00;//gout30_swap_fw[4:0]
    cmd_buf[30] = 0x00;//gout31_swap_fw[4:0]
    cmd_buf[31] = 0x00;//gout32_swap_fw[4:0]
    lcd_send_cmd_list(0x6e, 32, cmd_buf);

    lcd_send_cmd_data(0xbe, 1, 0x11, 0, 0, 0);

    cmd_buf[0] = 0xCC;
    cmd_buf[1] = 0x0C;
    cmd_buf[2] = 0xCC;
    cmd_buf[3] = 0x84;
    cmd_buf[4] = 0xCC;
    cmd_buf[5] = 0x04;
    cmd_buf[6] = 0x50;
    lcd_send_cmd_list(0x6c, 7, cmd_buf);

    lcd_send_cmd_data(0x7d, 1, 0x72, 0, 0, 0);
    lcd_send_cmd_data(0x7e, 1, 0x38, 0, 0, 0);

    cmd_buf[0] = 0x02;//avdd_clk
    cmd_buf[1] = 0x03;//avee_clk
    cmd_buf[2] = 0x09;//vdh_clk
    cmd_buf[3] = 0x05;//vgh_clk
    cmd_buf[4] = 0x0C;//vgl_clk
    cmd_buf[5] = 0x06;//vcl_clk
    cmd_buf[6] = 0x09;//vdh_clk_porch  0E
    cmd_buf[7] = 0x05;//vgh_clk_porch  0E
    cmd_buf[8] = 0x0C;//vgl_clk_porch  0E
    cmd_buf[9] = 0x06;//vcl_clk_porch  0E
    lcd_send_cmd_list(0x70, 10, cmd_buf);

    lcd_send_cmd_data(0x90, 4, 0x06, 0x06, 0x05, 0x06);
    lcd_send_cmd_data(0x93, 3, 0x45, 0xff, 0x00, 0);

    //gamma start
    cmd_buf[0] = 0x45;
    cmd_buf[1] = 0x09;
    cmd_buf[2] = 0x08;
    cmd_buf[3] = 0x08;
    cmd_buf[4] = 0x26;
    cmd_buf[5] = 0x2A;
    lcd_send_cmd_list(0xf0, 6, cmd_buf);

    cmd_buf[0] = 0x43;
    cmd_buf[1] = 0x70;
    cmd_buf[2] = 0x72;
    cmd_buf[3] = 0x36;
    cmd_buf[4] = 0x37;
    cmd_buf[5] = 0x6F;
    lcd_send_cmd_list(0xf1, 6, cmd_buf);

    cmd_buf[0] = 0x45;
    cmd_buf[1] = 0x09;
    cmd_buf[2] = 0x08;
    cmd_buf[3] = 0x08;
    cmd_buf[4] = 0x26;
    cmd_buf[5] = 0x2A;
    lcd_send_cmd_list(0xf2, 6, cmd_buf);

    cmd_buf[0] = 0x43;
    cmd_buf[1] = 0x70;
    cmd_buf[2] = 0x72;
    cmd_buf[3] = 0x36;
    cmd_buf[4] = 0x37;
    cmd_buf[5] = 0x6F;
    lcd_send_cmd_list(0xf3, 6, cmd_buf);

    lcd_send_cmd_data(0x2a, 4, hstart>>8, hstart&0xff, hend>>8, hend&0xff);
    lcd_send_cmd_data(0x2b, 4, vstart>>8, vstart&0xff, vend>>8, vend&0xff);

    lcd_send_cmd_data(0x35, 1, 0x00, 0, 0, 0);

    lcd_send_cmd(0x11);
    hal_sys_timer_delay(MS_TO_TICKS(100));
    lcd_send_cmd(0x29);
    hal_sys_timer_delay(MS_TO_TICKS(100));
    lcd_send_cmd(0x2c);
    return 0;
}
#endif /* LCD_9C01 */

#ifdef LCD_ICNA3310
POSSIBLY_UNUSED
static int lcd_icna3310_init(void)
{
    uint16_t hstart = 14;
    uint16_t hend = LCD_WIDTH - 1 + hstart;
    uint16_t vstart = 0;
    uint16_t vend = LCD_HEIGHT - 1 + vstart;
    uint8_t lcd_status = 0;
    uint32_t lcd_id = 0;

    if (lcd_read_status(&lcd_status) < 0) {
        TRACE(0, "Failed to read LCD power status.");
        return -1;
    }
    if (lcd_status == 0x08)
        TRACE(0, "LCD power status: %02x", lcd_status);
    else
        ASSERT(0, "LCD power status check failed; %02x", lcd_status);

    if (lcd_read_id(&lcd_id) < 0) {
        TRACE(0, "Failed to read LCD id.");
        return -1;
    }
    TRACE(0, "LCD id: 0x%06x", lcd_id);

    lcd_send_cmd_data(0xfe, 0x1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xc4, 0x1, 0x80, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x35, 0x1, 0x0, 0x0, 0x0, 0x0);
#ifdef TEST_RGB565
    lcd_send_cmd_data(0x3a, 0x1, 0x55, 0x0, 0x0, 0x0);
#else
    lcd_send_cmd_data(0x3a, 0x1, 0x77, 0x0, 0x0, 0x0);
#endif
    lcd_send_cmd_data(0x53, 0x1, 0x20, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x36, 0x1, 0x8, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x51, 0x1, 0xff, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x63, 0x1, 0xff, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x2a, 4, hstart>>8, hstart&0xff, hend>>8, hend&0xff);
    lcd_send_cmd_data(0x2b, 4, vstart>>8, vstart&0xff, vend>>8, vend&0xff);
    lcd_send_cmd(0x11);
    hal_sys_timer_delay(MS_TO_TICKS(240));
    lcd_send_cmd(0x29);
    hal_sys_timer_delay(MS_TO_TICKS(150));

    TRACE(0, "ICNA3310 is initialized.");
    return 0;
}
#endif /* LCD_RM69330 */

#ifdef CHIP_BEST1600
#ifdef LCD_ICNA3310
#define GPIO_BL_CTRL HAL_GPIO_PIN_PMU_P1_1
#define GPIO_RST_PIN HAL_GPIO_PIN_PMU_P1_3
#else
#define GPIO_RST_PIN HAL_GPIO_PIN_P4_2
#endif
#define GPIO_1V8_PIN HAL_GPIO_PIN_PMU_P2_1
#define GPIO_2V8_PIN HAL_GPIO_PIN_PMU_P2_2
#define GPIO_3V3_PIN HAL_GPIO_PIN_PMU_P2_3
#elif defined(CHIP_BEST1700)
#ifdef LCD_ICNA3310
#define GPIO_BL_CTRL HAL_GPIO_PIN_PMU_P1_1
#define GPIO_RST_PIN HAL_GPIO_PIN_PMU_P1_3
#else
#define GPIO_RST_PIN HAL_GPIO_PIN_P4_2
#endif
#define GPIO_1V8_PIN HAL_GPIO_PIN_PMU_P2_1
#define GPIO_2V8_PIN HAL_GPIO_PIN_PMU_P2_2
#define GPIO_3V3_PIN HAL_GPIO_PIN_PMU_P2_3
#elif defined(CHIP_BEST1502X)
#define GPIO_RST_PIN HAL_GPIO_PIN_P8_0
#define GPIO_1V8_PIN HAL_GPIO_PIN_P9_2
#ifdef LCD_ICNA3310
#define GPIO_BL_CTRL HAL_GPIO_PIN_P8_3
#endif
#elif defined(CHIP_BEST1503)
#define GPIO_RST_PIN HAL_GPIO_PIN_P8_0
#define GPIO_1V8_PIN HAL_GPIO_PIN_P9_2
#ifdef LCD_ICNA3310
#define GPIO_BL_CTRL HAL_GPIO_PIN_P8_3
#endif
#else
#define GPIO_RST_PIN HAL_GPIO_PIN_P8_4
#endif

static int lcd_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] =
    {
        {(enum HAL_IOMUX_PIN_T)GPIO_RST_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
#ifdef SIMU
    return 0;
#endif

#ifdef GPIO_1V8_PIN
    pinmux[0].pin = (enum HAL_IOMUX_PIN_T)GPIO_1V8_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_1V8_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_1V8_PIN);
#endif
#ifdef GPIO_2V8_PIN
    pinmux[0].pin = (enum HAL_IOMUX_PIN_T)GPIO_2V8_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_2V8_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_2V8_PIN);
#endif
#ifdef GPIO_3V3_PIN
    pinmux[0].pin = (enum HAL_IOMUX_PIN_T)GPIO_3V3_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_3V3_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_3V3_PIN);
#endif
#ifdef GPIO_BL_CTRL
    pinmux[0].pin = (enum HAL_IOMUX_PIN_T)GPIO_BL_CTRL;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_BL_CTRL, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_BL_CTRL);
#endif
    pinmux[0].pin = (enum HAL_IOMUX_PIN_T)GPIO_RST_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_RST_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_RST_PIN);
#if defined(LCD_RM69330) || defined(LCD_NR240x320) || defined(LCD_ICNA3310)
    hal_sys_timer_delay(MS_TO_TICKS(100));
    hal_gpio_pin_clr(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(5));
    hal_gpio_pin_set(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(100));
#endif

#if defined(LCD_RM69330)
    return lcd_rm69330_init();
#elif defined(LCD_NR240x320)
    return lcd_nr240x320_init();
#elif defined(LCD_9C01)
    return lcd_9c01_init();
#elif defined(LCD_ICNA3310)
    return lcd_icna3310_init();
#endif
    return 0;
}

/*********************************** LCDC control **************************************/

static void lcdc_isr(void)
{
    uint32_t irqstat = hal_lcdc_get_irqstat();
    TRACE(0, "lcdc irq %08x", irqstat);
    if (hal_lcdc_irq_is_framedone(irqstat)) {
        hal_lcdc_framedone_irqstat_clr();
        hal_lcdc_qspi_cs_sel(false);
        framedone_irq_get = true;
    }
}

static int lcdc_wait_framedone_irq(void)
{
    TRACE(0, "wait for framedone irq...");
    while (!framedone_irq_get && FAST_TICKS_TO_MS(hal_fast_sys_timer_get() - frame_start) < 500)
        hal_sys_timer_delay_us(100);
    framedone_irq_get = false;
    if (FAST_TICKS_TO_MS(hal_fast_sys_timer_get() - frame_start) >= 500)
        return -1;
    return 0;
}

static void te_gpio_irq(enum HAL_GPIO_PIN_T pin)
{
    te_irq_get = true;
}

static void lcdc_wait_te_irq(void)
{
#ifdef SIMU
    return;
#endif
    TRACE(0, "wait for TE irq...");
    while (!te_irq_get)
        hal_sys_timer_delay_us(10);
    te_irq_get = false;
}

static void lcdc_te_irq_init(void)
{
    static const struct HAL_GPIO_IRQ_CFG_T gpiocfg = {
        .irq_enable = true,
        .irq_debounce = false,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        .irq_handler = te_gpio_irq,
        .irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
    };

    te_gpio_pin = (enum HAL_GPIO_PIN_T)hal_iomux_get_dsi_te_pin();

    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] =
    {
        {(enum HAL_IOMUX_PIN_T)te_gpio_pin, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));

    hal_gpio_pin_set_dir(te_gpio_pin, HAL_GPIO_DIR_IN, 0);
    hal_gpio_setup_irq(te_gpio_pin, &gpiocfg);
}

static void lcdc_te_irq_free(void)
{
    static const struct HAL_GPIO_IRQ_CFG_T gpiocfg = {
        .irq_enable = false,
        .irq_debounce = false,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        .irq_handler = NULL,
        .irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
    };

    hal_gpio_setup_irq(te_gpio_pin, &gpiocfg);
}

static void lcdc_qspi_init(void)
{
    int irq = hal_lcdc_irqn();

    hal_psc_display_enable(HAL_PSC_DISPLAY_MOD_QSPI);

#ifdef TEST_RGB565
    hal_lcdc_qspi_initialize(LCDC_QSPI_RGB565, LCDC_QSPI_4LINE);
#else
    hal_lcdc_qspi_initialize(LCDC_QSPI_RGB888, LCDC_QSPI_4LINE);
#endif

    if (lcd_init())
        ASSERT(0, "LCD init failed.");

    hal_lcdc_disp_pn_reset();

    /* Layer SPU */
    hal_lcdc_lsize(LCDC_LAYER_SPU, LCD_WIDTH-1, LCD_HEIGHT-1);
    hal_lcdc_lcolor(LCDC_LAYER_SPU, 0);
    hal_lcdcl_enable(LCDC_LAYER_SPU, true);

    /* Layer Foreground (0) */
    hal_lcdc_lsize(LCDC_LAYER_LFORE, LCD_WIDTH, LCD_HEIGHT);
    hal_lcdc_lpitch(LCDC_LAYER_LFORE, LCDC_STRIDE);
    hal_lcdc_lzoom_set(LCDC_LAYER_LFORE, LCD_WIDTH, LCD_HEIGHT);
    hal_lcdc_sponscreen_set(LCDC_LAYER_LFORE, 1, 1);
    hal_lcdc_lpfc_fmt(LCDC_LAYER_LFORE, LCDC_FORMAT);
    hal_lcdc_lpfc_ltransp(LCDC_LAYER_LFORE, 0, 0);
    hal_lcdc_select_top(LCDC_LAYER_LFORE);

    hal_lcdc_smpn_enable();
#ifdef TEST_RGB565
    hal_lcdc_smpn_mode_set(5);
#else
    hal_lcdc_smpn_mode_set(3);
#endif

    hal_lcdc_panelpath_dmatrans_trigger_enable(LCDC_LAYER_LFORE, true);

    hal_lcdc_set_sync_line_pixs(5);
    hal_lcdc_set_sync_lines(0);

    hal_lcdc_smpn_vsync_delaymode_set(1);

    hal_lcdc_disp_extra_delay(0);
    hal_lcdc_sl_dmaburst_length_set(0x0f);
    hal_lcdc_ol_dmaburst_length_set(8);

    /* Configure dither */
    hal_lcdc_dither_s4x8(true);
    hal_lcdc_dither(false, 1, 0, 0, 0);

    hal_lcdc_disp_avsync_trigger_disable(true);
    hal_lcdc_disp_svsync_trigger_disable(true);

    hal_lcdcl_enable(LCDC_LAYER_LFORE, true);
    hal_lcdcl_enable(LCDC_LAYER_LBACK, false);

    /* lcdc framedone irq */
    hal_lcdc_irq_disable();
    NVIC_SetVector((IRQn_Type)irq, (uint32_t) lcdc_isr);
    NVIC_SetPriority((IRQn_Type)irq, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ((IRQn_Type)irq);
    NVIC_EnableIRQ((IRQn_Type)irq);

    /* lcd te irq */
    lcdc_te_irq_init();

    hal_lcdc_gen_frame_enable(true);
    hal_lcdc_framedone_irq_enable(true);

    frame_count = 0;
    TRACE(0, "lcdc qspi initialized.");
}

POSSIBLY_UNUSED
static void lcdc_qspi_exit(void)
{
    int irq = hal_lcdc_irqn();

    hal_lcdc_framedone_irq_enable(false);
    hal_lcdc_gen_frame_enable(false);

    hal_lcdc_irq_disable();
    NVIC_ClearPendingIRQ((IRQn_Type)irq);
    NVIC_DisableIRQ((IRQn_Type)irq);

    lcdc_te_irq_free();

    hal_cmu_lcdc_clock_disable();
}

static void lcdc_qspi_sleep(void)
{
    hal_lcdc_qspi_sleep();
    hal_lcdc_sleep();
}

static void lcdc_qspi_wakeup(void)
{
    hal_lcdc_wakeup();
    hal_lcdc_qspi_wakeup();
}

static int lcdc_frame_send(void *framebuffer)
{
    TRACE(0, "frame %u transfer start, buf %p", frame_count, framebuffer);

    lcdc_qspi_wakeup();

    hal_lcdc_lstartaddr(0, (uint32_t)framebuffer);

    lcdc_wait_te_irq();

    hal_lcdc_qspi_gen_start();

    frame_start = hal_fast_sys_timer_get();

    if (lcdc_wait_framedone_irq()) {
        TRACE(0, "frame transfer failed!");
        return -1;
    }

    lcdc_qspi_sleep();

    TRACE(0, "frame %u transfer done, used %uus", frame_count,
                FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start));
    frame_count++;
    return 0;
}

#define TEST_COLOR_BAR

#ifdef TEST_COLOR_BAR
POSSIBLY_UNUSED
static void make_color_bar(uint8_t *buf, uint16_t width, uint16_t height, bool vertical)
{
#ifdef TEST_RGB565
    uint16_t *color_bar = test_colors_RGB565;
    uint16_t *p = (uint16_t *)buf;
#else
    uint32_t *color_bar = test_colors_RGBA8888;
    uint32_t *p = (uint32_t *)buf;
#endif
    int bar_w = width / NCOLORS;
    int bar_h = height / NCOLORS;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int n;
            if (vertical)
                n = col / bar_w;
            else
                n = row / bar_h;
            *p++ = color_bar[n];
        }
    }
}
#else
POSSIBLY_UNUSED
static void fill_color(uint8_t *buf, uint16_t width, uint16_t height, int index)
{
#ifdef TEST_RGB565
    uint16_t *p = (uint16_t *)buf;
    uint16_t color = test_colors_RGB565[index % NCOLORS];
#else
    uint32_t *p = (uint32_t *)buf;
    uint32_t color = test_colors_RGBA8888[index % NCOLORS];
#endif
    for (int i = 0; i < (width * height); i++)
        p[i] = color;
}
#endif

POSSIBLY_UNUSED
static void qspi_perf_test(void)
{
    uint16_t pixmhz[3] = {24, 48, 96};
    for (uint32_t i = 0; i < 3; i++) {
        hal_cmu_lcdc_set_pixmhz(pixmhz[i % 3]);
        hal_cmu_lcdc_set_qspimhz(96);
        TRACE(0, "###### PIXEL: %uMhz, QSPI: %uMhz", hal_cmu_lcdc_get_pixmhz(),
                            hal_cmu_lcdc_get_qspimhz());

        for (int j = 0; j < 10; j++) {
            uint8_t *frame_buf = sram_buf;
#ifdef PSRAM_ENABLE
            if (j % 2 == 1)
                frame_buf = psram_buf;
#endif
#ifdef TEST_COLOR_BAR
            make_color_bar(frame_buf, LCD_WIDTH, LCD_HEIGHT, (j/5) % 2);
#else
            fill_color(frame_buf, LCD_WIDTH, LCD_HEIGHT, (j/5) % 10);
#endif
            lcdc_frame_send(frame_buf);
#ifndef SIMU
        hal_sys_timer_delay(MS_TO_TICKS(100));
#endif
        }
    }

    while (1);
}


void qspi_lcdc_test(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    TRACE(0, "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));

    TRACE(0, "qspi_lcdc_test");

    lcdc_qspi_init();

    hal_iomux_set_clock_out();
    hal_cmu_clock_out_enable(HAL_CMU_CLOCK_OUT_MCU_DISQSPI);

#ifdef TEST_PERF
    qspi_perf_test();
#endif

    TRACE(0, "PIXEL: %uMhz, QSPI: %uMhz", hal_cmu_lcdc_get_pixmhz(),
                        hal_cmu_lcdc_get_qspimhz());

    for (uint32_t i = 0; ;i++) {
        uint8_t *frame_buf = sram_buf;
#ifdef PSRAM_ENABLE
        if (i % 2 == 1)
            frame_buf = psram_buf;
#endif
#ifdef TEST_COLOR_BAR
        make_color_bar(frame_buf, LCD_WIDTH, LCD_HEIGHT, (i/10) % 2);
#else
        fill_color(frame_buf, LCD_WIDTH, LCD_HEIGHT, (i/10) % 10);
#endif
        lcdc_frame_send(frame_buf);
#ifndef SIMU
        hal_sys_timer_delay(MS_TO_TICKS(100));
#endif
    }

    while (1);
}

#endif
