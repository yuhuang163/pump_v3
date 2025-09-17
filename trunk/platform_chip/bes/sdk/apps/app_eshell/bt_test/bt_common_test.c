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
#ifdef UTILS_ESHELL_BT_TEST

#include "stdlib.h"
#include "string.h"
#include "eshell.h"
#include "app_eshell.h"
#include "bt_drv.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_common_test.h"
#include "bt_drv_common_test_case.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_key.h"
#include "pmu.h"

static bool init = false;
static bool sleep_flag;
#define BTCT_INIT_CHECK(n) if (!init) { \
                                _btct_init(); \
                            }

static void _btct_init(void)
{
    if (init) {
        return;
    }
    btdrv_start_bt();

    btdrv_hciopen();
    btdrv_ins_patch_test_init();
    bt_drv_reg_op_key_gen_after_reset(false);
    btdrv_hci_reset();

    btdrv_sleep_config(0);
    osDelay(2000);
    btdrv_testmode_start();
    btdrv_feature_default();
    bt_drv_config_after_hci_reset();

    btdrv_common_test_cmd_set_bt_sleep(true);
    init = true;
    eshell_putstring("%s init done", __func__);
}

static void _btct_deinit(void)
{
    if (!init) {
        return;
    }
    btdrv_hcioff();
    btdrv_stop_bt();
    init = false;
    eshell_putstring("%s deinit done", __func__);
}

static void btct_init(int argc, char *argv[])
{
    _btct_init();
    eshell_putstring("%s init flag [%d]", __func__, init);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "btct_init", "none", btct_init);

static void btct_deinit(int argc, char *argv[])
{
    _btct_deinit();
    eshell_putstring("%s init flag [%d]", __func__, init);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "btct_deinit", "none", btct_deinit);

static void btct_pscan_standard_en(int argc, char *argv[])
{
    if (argc < 3) {
        eshell_putstring("%s param err! e.g.[btct_pscan_standard_en interval window]", __func__);
        return;
    }
    BTCT_INIT_CHECK();
    uint16_t interval = atoi(argv[1]);
    uint16_t window = atoi(argv[2]);
    btdrv_common_pscan_standard_enable_test(interval, window);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "btct_pscan_standard_en", "none", btct_pscan_standard_en);

static void btct_pscan_interlaced_en(int argc, char *argv[])
{
    if (argc < 3) {
        eshell_putstring("%s param err! e.g.[btct_pscan_interlaced_en interval window]", __func__);
        return;
    }
    BTCT_INIT_CHECK();
    uint16_t interval = atoi(argv[1]);
    uint16_t window = atoi(argv[2]);
    btdrv_common_pscan_interlaced_enable_test(interval, window);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "btct_pscan_interlaced_en", "none", btct_pscan_interlaced_en);

static void btct_pscan_bothscan_en(int argc, char *argv[])
{
    if (argc < 3) {
        eshell_putstring("%s param err! e.g.[btct_pscan_bothscan_en interval window]", __func__);
        return;
    }
    BTCT_INIT_CHECK();
    uint16_t interval = atoi(argv[1]);
    uint16_t window = atoi(argv[2]);
    btdrv_common_bothscan_enable_test(interval, window);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "btct_pscan_bothscan_en", "none", btct_pscan_bothscan_en);

static void btct_create_connect(int argc, char *argv[])
{
    BTCT_INIT_CHECK();
    btdrv_common_bt_create_connect_test();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "btct_create_connect", "none", btct_create_connect);

static void btct_enter_sniff(int argc, char *argv[])
{
    if (argc < 5) {
        eshell_putstring("%s param err! e.g.[btct_enter_sniff conhandle sniff_interval attempt timeout]", __func__);
        return;
    }
    BTCT_INIT_CHECK();
    uint16_t conhandle = atoi(argv[1]);
    uint16_t sniff_interval = atoi(argv[2]);
    uint16_t attempt = atoi(argv[3]);
    uint16_t timeout = atoi(argv[4]);
    btdrv_common_bt_enter_sniff_test(conhandle, sniff_interval, attempt, timeout);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "btct_enter_sniff", "none", btct_enter_sniff);

static void btct_ble_adv_en(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("%s param err! e.g.[btct_ble_adv_en interval]", __func__);
        return;
    }
    BTCT_INIT_CHECK();
    uint16_t interval = atoi(argv[1]);
    eshell_putstring("%s interval %d", __func__, interval);
    btdrv_common_ble_adv_enable_test(interval);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "btct_ble_adv_en", "none", btct_ble_adv_en);

static void btct_ble_create_connect(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("%s param err! e.g.[btct_ble_create_connect interval]", __func__);
        return;
    }
    BTCT_INIT_CHECK();
    uint16_t interval = atoi(argv[1]);
    btdrv_common_ble_create_connect_test(interval);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "btct_ble_create_connect", "none", btct_ble_create_connect);

static void btct_ble_adv_with_data_en(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("%s param err! e.g.[btct_ble_adv_with_data_en interval]", __func__);
        return;
    }
    BTCT_INIT_CHECK();
    uint16_t interval = atoi(argv[1]);
    btdrv_common_ble_adv_enable_with_data_test(interval);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "btct_ble_adv_with_data_en", "none", btct_ble_adv_with_data_en);

static void btct_set_bt_sleep(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("%s param err! e.g.[btct_ble_adv_with_data_en enable]", __func__);
        return;
    }
    BTCT_INIT_CHECK();
    uint8_t enable = atoi(argv[1]);
    btdrv_common_test_cmd_set_bt_sleep(enable);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "btct_set_bt_sleep", "none", btct_set_bt_sleep);

static int key_event_process(uint32_t key_code, uint8_t key_event)
{
    eshell_putstring("wakeup by power key");
    hal_trace_rx_wakeup();
    sleep_flag = false;
    return 0;
}

#define PIN_WAKEUP                  (HAL_IOMUX_PIN_P1_5)
#define PIN_TRIGGER                 (HAL_IOMUX_PIN_P1_3)
static void btct_gpio_irq_wakeup_handler(enum HAL_GPIO_PIN_T pin)
{
    eshell_putstring("wakeup pin:%o", pin);
    hal_trace_rx_wakeup();
}

static void btct_gpio_irq_wakeup_set(bool en)
{
    uint32_t i;
    static bool gpio_inited = false;
    struct HAL_GPIO_IRQ_CFG_T cfg = {0,};
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pin_wakeup_table[] = {
        {PIN_WAKEUP, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLDOWN_ENABLE},
    };

    eshell_putstring("%s en:%d", __func__, en);
    cfg.irq_enable = en;
    if (en) {
        cfg.irq_handler = btct_gpio_irq_wakeup_handler;
    } else {
        cfg.irq_handler = NULL;
    }
    cfg.irq_debounce = true;
    cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    cfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;

    if (gpio_inited == false) {
        hal_iomux_init(pin_wakeup_table, ARRAY_SIZE(pin_wakeup_table));
        for (i = 0; i < ARRAY_SIZE(pin_wakeup_table); i++){
            hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)pin_wakeup_table[i].pin, HAL_GPIO_DIR_IN, 1);
        }
        gpio_inited = true;
    }
    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)PIN_WAKEUP, &cfg);
}

static void btct_gpio_trigger_set(void)
{
    const struct HAL_IOMUX_PIN_FUNCTION_MAP tri_gpio_cfg[] = {
	{PIN_TRIGGER, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
    };
	hal_iomux_init(tri_gpio_cfg, sizeof(tri_gpio_cfg) / sizeof(struct HAL_IOMUX_PIN_FUNCTION_MAP));
    hal_gpio_pin_set_dir(PIN_TRIGGER, HAL_GPIO_DIR_OUT, 1);
}

static void btct_sys_sleep(int argc, char *argv[])
{
    btct_gpio_irq_wakeup_set(true);
    btct_gpio_trigger_set();
    hal_trace_rx_sleep();

    hal_sleep_start_stats(5000, 5000);
    // pmu_sleep_en(true);
    enum HAL_CMU_FREQ_T sys_freq = hal_sysfreq_get();
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);

    sleep_flag = true;
    while (sleep_flag) {
        hal_sleep_enter_sleep();
    };

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, sys_freq);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "btct_sys_sleep", "none", btct_sys_sleep);

static void btct_key_open(int argc, char *argv[])
{
    hal_key_open(false, key_event_process);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "btct_key_open", "none", btct_key_open);

static void bt_tport(int argc, char *argv[])
{
    if(strncmp(argv[1],"set",3) == 0){
        hal_iomux_set_bt_tport();
        eshell_putstring("enable tport");
    }else if(strncmp(argv[1],"clear",5) == 0){
        hal_iomux_clear_bt_tport();
        eshell_putstring("disable tport");
    }else{
        eshell_putstring("%s error command:%d", __func__, init);
    }
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "btct_tport", "none", bt_tport);

#endif
