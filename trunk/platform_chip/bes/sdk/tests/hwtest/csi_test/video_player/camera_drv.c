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
#include "hal_i2c.h"
#include "hal_timer.h"
#include "csi_dsi_conf.h"
#include "video_player.h"
#include "camera_sens_gc2145.h"
#include "camera_sens_sc132gs.h"
#include "camera_drv.h"

enum CAMERA_DEV_STATUS_T {
    CAMERA_DEV_STATUS_NULL = 0x00,
    CAMERA_DEV_STATUS_OPEN_CLOSE = 0x01,
    CAMERA_DEV_STATUS_STREAM_OPEN_CLOSE = 0x02,
    CAMERA_DEV_STATUS_STREAM_START_STOP = 0x04,
    CAMERA_DEV_STATUS_ALLOC_FREE = 0x08,
};

struct CAMERA_DEVICE_DESC_T {
    uint8_t status;                       // device status for controlling
    enum VP_DEVICE_ID_T device_id;        // the device indentify, for CSI controller
    enum VP_DEVICE_T device;              // the device type, specify camera type
    struct VP_DEVICE_BUS_T i2c_bus;       // i2c bus
    struct VP_DEVICE_OPS_T device_ops;    // device operations
    struct VP_DEVICE_CONFIG_T device_cfg; // device configuration
};

static struct CAMERA_DEVICE_DESC_T cam_dev_desc[CAMERA_DESC_NUM];

static enum VP_DEVICE_T cam_dev_list[CAMERA_DEV_NUM] = CAMERA_DEV_LIST;

static int hw_pin_inited = 0;
static volatile int hw_pwron_cnt = 0;
static uint8_t i2c_user_map[CAMERA_I2C_ID_QTY] = {0};

static struct CAMERA_DEVICE_DESC_T *camera_device_get_desc(int id)
{
    ASSERT(id < CAMERA_DEV_NUM, "[%s]: Invalid id %d", __func__, id);

    return &cam_dev_desc[id];
}

static int camera_device_check(enum VP_DEVICE_T cam_device)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(cam_dev_list); i++) {
        if (cam_device == cam_dev_list[i]) {
            return i;
        }
    }
    return -1;
}

static int camera_device_alloc_desc_id(void)
{
    int i;
    struct CAMERA_DEVICE_DESC_T *list = cam_dev_desc;

    for (i = 0; i < CAMERA_DESC_NUM; i++) {
        struct CAMERA_DEVICE_DESC_T *p = &list[i];
        if ((p->status & CAMERA_DEV_STATUS_ALLOC_FREE) == 0) {
            p->status |= CAMERA_DEV_STATUS_ALLOC_FREE;
            return i;
        }
    }
    return -1;
}

static int camera_device_free_desc_id(int desc_id)
{
    struct CAMERA_DEVICE_DESC_T *p = &cam_dev_desc[desc_id];

    if (desc_id >= CAMERA_DESC_NUM) {
        return -1;
    }
    if ((p->status & CAMERA_DEV_STATUS_ALLOC_FREE) != 0) {
        p->status &= ~CAMERA_DEV_STATUS_ALLOC_FREE;
        return 0;
    }
    return -2;
}

static inline void camera_device_active_bus(struct CAMERA_DEVICE_DESC_T *p)
{
    p->device_ops.set_bus(&p->i2c_bus);
}

static struct VP_DEVICE_OPS_T *camera_device_get_ops(enum VP_DEVICE_T cam_device)
{
    struct VP_DEVICE_OPS_T *p = NULL;

    if (0) {
    }
#ifdef CSI_SENS_GC2145
    else if (cam_device == VP_DEVICE_GC2145) {
        p = gc2145_sensor_ops_get();
    }
#endif
#ifdef CSI_SENS_SC132GS
    else if (cam_device == VP_DEVICE_SC132GS) {
        p = sc132gs_sensor_ops_get();
    }
#endif
    else {
        TRACE(1, "[%s]: Bad camera device %d", __func__, cam_device);
        return NULL;
    }
    TRACE(1, "[%s]: device_name: %s", __func__, p->name);
    return p;
}

int camera_device_open(void)
{
    int i;

    // init desc
    for (i = 0; i < ARRAY_SIZE(cam_dev_desc); i++) {
        struct CAMERA_DEVICE_DESC_T *p = &cam_dev_desc[i];
        p->status = CAMERA_DEV_STATUS_OPEN_CLOSE;
        p->device_id = VP_DEVICE_ID_0;
        p->device = VP_DEVICE_NULL;
    }
    // init hw varibles
    for (i = 0; i < CAMERA_I2C_ID_QTY; i++) {
        i2c_user_map[i] = 0;
    }
    hw_pwron_cnt = 0;
    hw_pin_inited = 0;
    TRACE(1, "[%s]: done", __func__);
    return 0;
}

int camera_device_close(void)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(cam_dev_desc); i++) {
        struct CAMERA_DEVICE_DESC_T *p = &cam_dev_desc[i];

        if (p->status & CAMERA_DEV_STATUS_STREAM_START_STOP) {
            camera_device_stream_stop(i);
        }
        if (p->status & CAMERA_DEV_STATUS_STREAM_OPEN_CLOSE) {
            camera_device_stream_close(i);
        }
        p->status &= ~CAMERA_DEV_STATUS_OPEN_CLOSE;
    }
    TRACE(1, "[%s]: done", __func__);
    return 0;
}

int camera_device_get_config(enum VP_DEVICE_T cam_device, struct VP_DEVICE_CONFIG_T *cfg)
{
    int ret = 0;
    struct VP_DEVICE_OPS_T *ops;

    ops = camera_device_get_ops(cam_device);
    ret = ops->get_config(cfg);
    return ret;
}

int camera_device_set_config(enum VP_DEVICE_T cam_device, struct VP_DEVICE_CONFIG_T *cfg)
{
    int ret = 0;
    struct VP_DEVICE_OPS_T *ops;

    ops = camera_device_get_ops(cam_device);
    ret = ops->set_config(cfg);
    return ret;
}

bool camera_device_probe_i2c_bus(enum CAMERA_I2C_ID_T i2c_id, struct VP_DEVICE_BUS_T *i2c_bus,
    enum VP_DEVICE_T *cam_device)
{
    enum VP_DEVICE_T *list = cam_dev_list;
    int i;
    int ret;
    bool success = false;
    uint8_t chip_id[2] = {0};

    ASSERT(i2c_bus != NULL, "[%s]: null i2c bus ptr", __func__);
    ASSERT(cam_device != NULL, "[%s]: null device ptr", __func__);

    for (i = 0; i < ARRAY_SIZE(cam_dev_list); i++) {
        struct VP_DEVICE_OPS_T *ops;
        enum VP_DEVICE_T dev;

        dev = list[i];
        ops = camera_device_get_ops(dev);
        if (ops == NULL) {
            continue;
        }
        // set i2c bus
        ops->get_bus(i2c_bus);
        i2c_bus->id = i2c_id;
        ops->set_bus(i2c_bus);

        // init device
        if (ops->init) {
            ops->init();
        }

        // power on
        ops->power_on(true);

        // get chip id
        chip_id[0] = 0;
        chip_id[1] = 0;
        ret = ops->get_chip_id(chip_id);
        if (!ret) {
            success = true;
            if (cam_device) {
                *cam_device = dev;
            }
        }
        // power off
        ops->power_on(false);
        if (success) {
            break;
        }
    }
    if (!success) {
        TRACE(1, "[%s]: proble failed", __func__);
    } else {
        TRACE(1, "[%s]: probe success, chip id: [%x-%x]", __func__, chip_id[0], chip_id[1]);
    }
    return success;
}

bool camera_device_probe(enum VP_DEVICE_T *cam_device, struct VP_DEVICE_BUS_T *bus)
{
    int id;
    enum VP_DEVICE_T device = VP_DEVICE_NULL;
    struct VP_DEVICE_BUS_T i2c_bus;
    bool success = false;

    for (id = 0; id < CAMERA_I2C_ID_QTY; id++) {
        success = camera_device_probe_i2c_bus((enum CAMERA_I2C_ID_T)id, &i2c_bus, &device);
        if (success) {
            break;
        }
    }
    if (cam_device) {
        *cam_device = device;
    }
    if (bus) {
        *bus = i2c_bus;
    }
    return success;
}

int camera_device_stream_open(enum VP_DEVICE_T cam_device, struct VP_DEVICE_BUS_T *bus,
    struct VP_DEVICE_CONFIG_T *cfg)
{
    int ret = 0;
    int desc_id = 0;
    uint32_t mask;
    struct CAMERA_DEVICE_DESC_T *p;
    struct VP_DEVICE_OPS_T *ops;

    ret = camera_device_check(cam_device);
    if (ret < 0) {
        TRACE(1, "[%s]: Invalid camera device: %d", __func__, cam_device);
        return -1;
    }
    desc_id = camera_device_alloc_desc_id();
    if (desc_id < 0) {
        TRACE(1, "[%s]: alloc desc id failed: %d", __func__, desc_id);
        return -2;
    }
    p = camera_device_get_desc(desc_id);
    mask = CAMERA_DEV_STATUS_OPEN_CLOSE | CAMERA_DEV_STATUS_ALLOC_FREE;
    if (p->status != mask) {
        TRACE(1, "[%s]: error status: %x", __func__, p->status);
        return -3;
    }
    ops = camera_device_get_ops(cam_device);
    if (ops == NULL) {
        TRACE(1, "[%s]: null ops ptr, cam_device: %x", __func__, cam_device);
        return -4;
    }
    p->device = cam_device;
    p->device_ops = *ops;
    if (cfg) {
        p->device_cfg = *cfg;
    } else {
        ret = ops->get_config(&p->device_cfg);
        if (ret) {
            TRACE(1, "[%s]: get device config failed, %d", __func__, ret);
            return -5;
        }
    }
    if (bus) {
        p->i2c_bus = *bus;
    } else {
        ret = ops->get_bus(&p->i2c_bus);
        if (ret) {
            TRACE(1, "[%s]: get bus failed, %d", __func__, ret);
            return -6;
        }
    }

    // device: init & power & open
    camera_device_active_bus(p);
    p->device_ops.init();
    p->device_ops.power_on(true);
    ret = p->device_ops.open();
    if (ret) {
        TRACE(1, "[%s]: open failed, ret=%d", __func__, ret);
        return -7;
    }
    p->status |= CAMERA_DEV_STATUS_STREAM_OPEN_CLOSE;
    TRACE(1, "[%s]: done, device name: %s", __func__, p->device_ops.name);
    return desc_id;
}

int camera_device_stream_close(int desc_id)
{
    int ret = 0;
    uint32_t mask;
    struct CAMERA_DEVICE_DESC_T *p;
    struct VP_DEVICE_OPS_T *ops;

    p = camera_device_get_desc(desc_id);
    mask = CAMERA_DEV_STATUS_OPEN_CLOSE
        | CAMERA_DEV_STATUS_ALLOC_FREE
        | CAMERA_DEV_STATUS_STREAM_OPEN_CLOSE;
    if (p->status != mask) {
        TRACE(1, "[%s]: error status: %x", __func__, p->status);
        return -1;
    }
    camera_device_active_bus(p);
    ops = &p->device_ops;
    ret = ops->close();
    ops->power_on(false);
    p->status &= ~CAMERA_DEV_STATUS_STREAM_OPEN_CLOSE;

    camera_device_free_desc_id(desc_id);
    TRACE(1, "[%s]: desc_id=%d, ret=%d", __func__, desc_id, ret);
    return ret;
}

int camera_device_stream_setup(int desc_id, struct VP_DEVICE_CONFIG_T *cfg)
{
    int ret = 0;
    struct CAMERA_DEVICE_DESC_T *p;

    p = camera_device_get_desc(desc_id);
    p->device_cfg = *cfg;
    TRACE(1, "[%s]: desc_id=%d, ret=%d", __func__, desc_id, ret);
    return ret;
}

int camera_device_stream_start(int desc_id)
{
    int ret = 0;
    uint32_t mask;
    struct CAMERA_DEVICE_DESC_T *p;
    struct VP_DEVICE_OPS_T *ops;

    p = camera_device_get_desc(desc_id);
    mask = CAMERA_DEV_STATUS_OPEN_CLOSE
        | CAMERA_DEV_STATUS_ALLOC_FREE
        | CAMERA_DEV_STATUS_STREAM_OPEN_CLOSE;
    if (p->status != mask) {
        TRACE(1, "[%s]: error status: %x", __func__, p->status);
        return -1;
    }
    camera_device_active_bus(p);
    ops = &p->device_ops;
    ret = ops->start();
    if (ret) {
        TRACE(1, "[%s]: start failed, ret=%d", __func__, ret);
        return -2;
    }
    p->status |= CAMERA_DEV_STATUS_STREAM_START_STOP;

    TRACE(1, "[%s]: desc_id=%d, ret=%d", __func__, desc_id, ret);
    return ret;
}

int camera_device_stream_stop(int desc_id)
{
    int ret = 0;
    uint32_t mask;
    struct CAMERA_DEVICE_DESC_T *p;
    struct VP_DEVICE_OPS_T *ops;

    p = camera_device_get_desc(desc_id);
    mask = CAMERA_DEV_STATUS_OPEN_CLOSE
        | CAMERA_DEV_STATUS_ALLOC_FREE
        | CAMERA_DEV_STATUS_STREAM_START_STOP
        | CAMERA_DEV_STATUS_STREAM_OPEN_CLOSE;
    if (p->status != mask) {
        TRACE(1, "[%s]: error status: %x", __func__, p->status);
        return -1;
    }
    camera_device_active_bus(p);
    ops = &p->device_ops;
    ret = ops->stop();
    if (ret) {
        TRACE(1, "[%s]: stop failed, ret=%d", __func__, ret);
        return -2;
    }
    p->status &= ~CAMERA_DEV_STATUS_STREAM_START_STOP;

    TRACE(1, "[%s]: desc_id=%d, ret=%d", __func__, desc_id, ret);
    return ret;
}

int camera_device_bus_read(int desc_id, uint32_t addr, uint32_t *val)
{
    int ret = 0;
    struct CAMERA_DEVICE_DESC_T *p;
    struct VP_DEVICE_OPS_T *ops;

    p = camera_device_get_desc(desc_id);
    camera_device_active_bus(p);
    ops = &p->device_ops;
    ret = ops->bus_read(addr, val);
    if (ret) {
        TRACE(1, "[%s]: read failed, ret=%d, addr=%x", __func__, ret, addr);
        return -1;
    }
    return ret;
}

int camera_device_bus_write(int desc_id, uint32_t addr, uint32_t val)
{
    int ret;
    struct CAMERA_DEVICE_DESC_T *p;
    struct VP_DEVICE_OPS_T *ops;

    p = camera_device_get_desc(desc_id);
    camera_device_active_bus(p);
    ops = &p->device_ops;
    ret = ops->bus_write(addr, val);
    if (ret) {
        TRACE(1, "[%s]: write failed, ret=%d, addr=%x, val=%c", __func__, ret, addr, val);
        return -1;
    }
    return ret;
}

void camera_device_test_mode(int desc_id, bool enable, uint8_t p0, uint8_t p1, uint8_t p2)
{
    struct CAMERA_DEVICE_DESC_T *p;
    struct VP_DEVICE_OPS_T *ops;
    int mode = p0;

    p = camera_device_get_desc(desc_id);
    camera_device_active_bus(p);
    ops = &p->device_ops;
    ops->test_mode(mode, enable);
}

int camera_device_hw_i2c_open(enum CAMERA_I2C_USER_T user, enum CAMERA_I2C_ID_T id, uint32_t speed)
{
    int ret = 0, set = 0;
    struct HAL_I2C_CONFIG_T cfg;

    TRACE(1, "[%s]: user=%d, id=%d, speed=%d", __func__, user, id, speed);

    if (i2c_user_map[id] == 0) {
        set = 1;
    }
    i2c_user_map[id] |= (1 << user);

    if (set) {
        memset(&cfg, 0, sizeof(cfg));
        cfg.mode = HAL_I2C_API_MODE_TASK;
        cfg.use_dma = 0;
        cfg.use_sync = 1;
        cfg.speed = speed;
        cfg.as_master = 1;

        TRACE(1, "I2C[%d]: speed=%d Hz, mode=%d", id, cfg.speed, cfg.mode);
        if (id == CAMERA_I2C_ID_0) {
            hal_iomux_set_i2c0();
        } else if (id == CAMERA_I2C_ID_1) {
            hal_iomux_set_i2c1();
        } else if (id == CAMERA_I2C_ID_2) {
            hal_iomux_set_i2c2();
        }
        ret = hal_i2c_open((enum HAL_I2C_ID_T)id, &cfg);
        if (ret) {
            TRACE(2,"%s, open i2c failed %d", __func__, ret);
            return ret;
        }
    }
    return 0;
}

int camera_device_hw_i2c_close(enum CAMERA_I2C_USER_T user, enum CAMERA_I2C_ID_T id)
{
    int ret = 0, set = 0;

    TRACE(1, "[%s]: user=%d, id=%d", __func__, user, id);

    i2c_user_map[id] &= ~(1 << user);
    if (i2c_user_map[id] == 0) {
        set = 1;
    }
    if (set) {
        ret = hal_i2c_close(id);
        if (ret) {
            TRACE(1, "[%s]: close i2c failed %d", __func__, ret);
        } else {
            TRACE(1, "[%s]: close i2c success", __func__);
        }
    }
    return 0;
}

int camera_device_hw_i2c_recv(enum CAMERA_I2C_USER_T user, enum CAMERA_I2C_ID_T id,
    uint8_t addr, uint8_t *txbuf, uint16_t txlen, uint8_t *rxbuf, uint16_t rxlen)
{
    int ret;

    ret = hal_i2c_task_recv((enum HAL_I2C_ID_T)id, addr, txbuf, txlen, rxbuf, rxlen, 0, NULL);
    return ret;
}

int camera_device_hw_i2c_send(enum CAMERA_I2C_USER_T user, enum CAMERA_I2C_ID_T id,
    uint8_t addr, uint8_t *txbuf, uint16_t txlen)
{
    int ret;

    ret = hal_i2c_task_send((enum HAL_I2C_ID_T)id, addr, txbuf, txlen, 0, NULL);
    return ret;
}

void camera_device_hw_pin_init(void)
{
#ifdef SIMU
    return;
#endif

    if (hw_pin_inited) {
        return;
    }
    const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux1[] = {
        {CSI_SENS_PDN_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {CSI_SENS_RSTN_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    // pdn, rstn
    hal_iomux_init(pinmux1, ARRAY_SIZE(pinmux1));
    hal_gpio_pin_set_dir(CSI_SENS_PDN_PIN, HAL_GPIO_DIR_OUT, 1); //power down, active high
    hal_gpio_pin_set_dir(CSI_SENS_RSTN_PIN, HAL_GPIO_DIR_OUT, 0); //reset, active low
    hw_pin_inited = 1;
}

static void camera_device_hw_mclk_enable(void)
{
#ifdef SIMU
    return;
#endif

    const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux2[] = {
        {CSI_SENS_MCLK_PIN, HAL_IOMUX_FUNC_CLK_OUT, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };

    hal_iomux_init(pinmux2, ARRAY_SIZE(pinmux2));
    hal_cmu_clock_out_enable(HAL_CMU_CLOCK_OUT_AON_OSC);
}

static void camera_device_hw_mclk_disable(void)
{
#ifdef SIMU
    return;
#endif

    const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux2[] = {
        {CSI_SENS_MCLK_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };

    hal_cmu_clock_out_disable();
    hal_iomux_init(pinmux2, ARRAY_SIZE(pinmux2));
    hal_gpio_pin_set_dir(CSI_SENS_MCLK_PIN, HAL_GPIO_DIR_OUT, 0);
}

void camera_device_hw_power_on(enum VP_DEVICE_ID_T id, enum VP_DEVICE_T device)
{
#ifdef SIMU
    return;
#endif

    if (hw_pwron_cnt == 0) {
        camera_device_hw_mclk_enable();
        hal_sys_timer_delay(US_TO_TICKS(500));

        hal_gpio_pin_set(CSI_SENS_PDN_PIN);
        hal_gpio_pin_clr(CSI_SENS_RSTN_PIN);
        hal_sys_timer_delay(US_TO_TICKS(500));

        hal_gpio_pin_clr(CSI_SENS_PDN_PIN);
        hal_sys_timer_delay(US_TO_TICKS(250));

        hal_gpio_pin_set(CSI_SENS_RSTN_PIN);
        hal_sys_timer_delay(US_TO_TICKS(250));
    }
    hw_pwron_cnt++;
}

void camera_device_hw_power_off(enum VP_DEVICE_ID_T id, enum VP_DEVICE_T device)
{
#ifdef SIMU
    return;
#endif

    if (hw_pwron_cnt == 0) {
        return;
    }
    if (hw_pwron_cnt > 0) {
        hw_pwron_cnt--;
        if (hw_pwron_cnt == 0) {
            hal_gpio_pin_set(CSI_SENS_PDN_PIN);
            hal_sys_timer_delay(US_TO_TICKS(250));

            hal_gpio_pin_clr(CSI_SENS_RSTN_PIN);
            hal_sys_timer_delay(US_TO_TICKS(250));

            camera_device_hw_mclk_disable();
            hal_sys_timer_delay(US_TO_TICKS(500));
        }
    }
}

