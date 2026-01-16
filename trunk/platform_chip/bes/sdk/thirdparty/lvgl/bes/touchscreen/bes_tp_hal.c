/***************************************************************************
 *
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
#include "hal_iomux.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include "hal_i2c.h"
#include "hal_gpio.h"
#include "bes_tp_hal.h"

//#define TP_DEBUG
#ifdef TP_DEBUG
#define TP_DBG(...)         TR_INFO( TR_MOD(HAL), "//TP: "  __VA_ARGS__)
#else
#define TP_DBG(...)
#endif
#define TP_LOG(...)         TR_INFO( TR_MOD(HAL), "//TP: "  __VA_ARGS__)
#define TP_ERR(...)         TR_INFO( TR_MOD(HAL), "//TP: "  __VA_ARGS__)

#define POWER_ON_DELAY      200
#define POWER_OFF_DELAY     70

#ifdef LVGL_TP_ZINITIX
static struct i2c_dev zinitix_dev = {
    .name     = "zinitix",
    .bus      = (enum HAL_I2C_ID_T )HAL_I2C_ID_1,
    .addr     = ZTW_I2C_ADDR,
    .speed    = I2C_SPEED,
    .fn_probe = ztw_probe,
    .fn_coor  = ztw_read_coord,
    .fn_init  = ztw_init,
};
#endif

#ifdef LVGL_TP_TMA525C
static struct i2c_dev tma525c_dev = {
    .name     = "tma525c",
#if defined(CHIP_BEST1600) || defined(CHIP_BEST1700)
    .bus      = (enum HAL_I2C_ID_T )HAL_I2C_ID_0,
#elif defined(CHIP_BEST1502X) || defined(CHIP_BEST1306P)
    .bus      = (enum HAL_I2C_ID_T )HAL_I2C_ID_2,
#elif defined(CHIP_BEST1502P)
    .bus      = (enum HAL_I2C_ID_T )HAL_I2C_ID_3,
#elif defined(CHIP_BEST1503)
    .bus      = (enum HAL_I2C_ID_T )HAL_I2C_ID_5,
#elif defined(CHIP_BEST3601)
    .bus      = (enum HAL_I2C_ID_T )HAL_I2C_ID_4,
#endif
    .addr     = TMA525C_I2C_ADDR,
    .speed    = I2C_SPEED,
    .fn_probe = tma525c_probe,
    .fn_coor  = tma525c_read_coord,
    .fn_init  = tma525c_init,
};
#endif

#ifdef LVGL_TP_CHSC6X
static struct i2c_dev chsc6x_dev = {
    .name     = "chsc6x",
    .bus      = (enum HAL_I2C_ID_T )HAL_I2C_ID_3,
    .addr     = CHSC6X_I2C_ADDR,
    .speed    = I2C_SPEED,
    .fn_probe = chsc6x_probe,
    .fn_coor  = chsc6x_read_coord,
    .fn_init  = tp_chsc6x_open,
};
#endif

static i2c_dev_t* probed_dev = NULL;
static tp_sample_t g_sample;
static struct i2c_dev *tp_i2c_dev_list[] = {
#ifdef LVGL_TP_ZINITIX
    &zinitix_dev,
#endif
#ifdef LVGL_TP_TMA525C
    &tma525c_dev,
#endif
#ifdef LVGL_TP_CHSC6X
    &chsc6x_dev,
#endif
};

static int tp_i2c_write(uint32_t i2c_bus, uint32_t slave, uint8_t *pd, uint32_t dlen)
{
    int ret;

    TP_DBG("%s, bus:%d, addr:0x%x", __func__, i2c_bus, slave);

    ret = hal_i2c_simple_send(i2c_bus, slave, pd, dlen);
    if (ret) {
        TP_ERR("%s, send failed, ret=%d", __func__, ret);
    }

    TP_DBG("%s, done with ret %d", __func__, ret);
    return ret;
}

static int tp_i2c_read(uint32_t i2c_bus, uint32_t slave, uint8_t *reg_addr, uint32_t reg_len,
                uint8_t *pd, uint32_t dlen)
{
    int ret;

    TP_DBG("%s, dev_addr=0x%x", __func__, slave);

    ret = hal_i2c_simple_recv(i2c_bus,slave, reg_addr, reg_len, pd, dlen);
    if (ret) {
        TP_ERR("%s, read failed, ret=0x%x, dlen=%d", __func__, ret, dlen);
    }

    TP_DBG("%s, done with ret %d", __func__, ret);
    return ret;
}

static void tp_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    if (probed_dev) {
        if (probed_dev->fn_coor(&g_sample) != 0) {
            TP_ERR("Failed to read touch event.");
        }
    }
}

static void bes_tp_irq_enable(bool enable, uint32_t int_pin, uint32_t int_mode)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg = { 0 };

    gpiocfg.irq_polarity = int_mode ;
    gpiocfg.irq_type     = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;

    if (enable) {
        gpiocfg.irq_enable   = true;
        gpiocfg.irq_debounce = true;
        gpiocfg.irq_handler  = tp_irq_handler;
    } else {
        gpiocfg.irq_enable = false;
        gpiocfg.irq_debounce = false;
        gpiocfg.irq_handler = NULL;
    }

    hal_gpio_setup_irq(int_pin, &gpiocfg);
}

static void touch_hw_reset(bool enable, uint32_t rst_pin)
{
    if (enable) {
        hal_gpio_pin_set(rst_pin);
        hal_sys_timer_delay_us(200 * 1000);
        hal_gpio_pin_clr(rst_pin);
        hal_sys_timer_delay_us(100 * 1000);
        hal_gpio_pin_set(rst_pin);
        hal_sys_timer_delay_us(POWER_ON_DELAY * 1000);
    } else {
        hal_gpio_pin_clr(rst_pin);
        hal_sys_timer_delay_us(POWER_OFF_DELAY * 1000);
    }
}

static void touch_poweron(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP iomux_cfg[] = {
        { TP_RST_PIN, HAL_IOMUX_FUNC_AS_GPIO,
            HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE },
        { TP_INT_PIN, HAL_IOMUX_FUNC_AS_GPIO,
            HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL },
    };
    hal_iomux_init(iomux_cfg, sizeof(iomux_cfg) / sizeof(iomux_cfg[0]));

    hal_gpio_pin_set_dir(TP_RST_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set_dir(TP_INT_PIN, HAL_GPIO_DIR_IN, 0);
#if defined(CHIP_BEST1503)
    hal_gpio_pin_set_dir(TP_PWR_PIN, HAL_GPIO_DIR_OUT, 1);
#endif

#ifdef LVGL_TP_CHSC6X
    chsc6x_tp_reset();
#else
    touch_hw_reset(true, TP_RST_PIN);
#endif
    bes_tp_irq_enable(false, TP_INT_PIN, HAL_GPIO_IRQ_POLARITY_LOW_FALLING);
}

static void touch_poweroff(void)
{
    hal_gpio_pin_set_dir(TP_RST_PIN, HAL_GPIO_DIR_OUT, 0);
}

static i2c_dev_t *tp_i2c_probe(void)
{
    i2c_dev_t *dev = NULL;
    int32_t ret = 0;
    struct HAL_I2C_CONFIG_T cfg;

    memset(&cfg, 0, sizeof(cfg));
    cfg.mode      = HAL_I2C_API_MODE_SIMPLE;
    cfg.use_dma   = 0;
    cfg.use_sync  = 1;
    cfg.as_master = 1;

    for(int i = 0; i < sizeof(tp_i2c_dev_list) / sizeof(tp_i2c_dev_list[0]); i++) {
        cfg.speed = tp_i2c_dev_list[i]->speed;
        ret = hal_i2c_open(tp_i2c_dev_list[i]->bus, &cfg);
        if(ret != 0)
            continue;

        ret = tp_i2c_dev_list[i]->fn_probe(tp_i2c_dev_list[i]->bus);
        if(ret != 0) {
            hal_i2c_close(tp_i2c_dev_list[i]->bus);
            continue;
        }

        dev = tp_i2c_dev_list[i];
        break;
    }

    if(!dev) {
        TP_ERR("%s: failed to probe touchscreen device.", __func__);
    } else {
        TP_LOG("%s: touchscreen %s, bus %d, probed.", __func__, dev->name, dev->bus);
    }
    return dev;
}

bool bes_tp_is_pressed(void)
{
    return !((g_sample.contact == CONTACT_NONE)||(g_sample.contact == CONTACT_UP));
}

void bes_tp_get_xy(int32_t * x, int32_t * y)
{
    TP_DBG("%s: read xy %d,%d.", __func__, g_sample.x, g_sample.y);
    *x = (int32_t)g_sample.x;
    *y = (int32_t)g_sample.y;
}

int bes_tp_hal_init(void)
{
    touch_poweron();

    /* register i2c read and write callback to target device */
#ifdef LVGL_TP_ZINITIX
    ztw_register_i2c_rw(tp_i2c_write, tp_i2c_read);
#endif

#ifdef LVGL_TP_TMA525C
    tma525c_register_i2c_rw(tp_i2c_write, tp_i2c_read);
#endif

#ifdef LVGL_TP_CHSC6X
    chsc6x_register_i2c_rw(tp_i2c_write, tp_i2c_read);
#endif

    probed_dev = tp_i2c_probe();
    if (probed_dev == NULL) {
        return -1;
    }

    /* initialize the touchscreen */
    probed_dev->fn_init(probed_dev->addr);

    bes_tp_irq_enable(true, TP_INT_PIN, HAL_GPIO_IRQ_POLARITY_LOW_FALLING);
    return 0;
}

void bes_tp_hal_exit(void)
{
    bes_tp_irq_enable(false, TP_INT_PIN, HAL_GPIO_IRQ_POLARITY_LOW_FALLING);

    touch_poweroff();
}
