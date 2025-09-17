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
#include "stdio.h"
#include "stdlib.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_bootmode.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_location.h"
#include "hal_norflash.h"
#include "hal_sleep.h"
#include "hal_wdt.h"
#include "hal_trng.h"
#include "hal_uart.h"
#include "hwtimer_list.h"
#include "mpu_cfg.h"
#include "mpu.h"
#include "pmu.h"
#include "analog.h"
#include "norflash_api.h"
#include "app_utils.h"
#include "app_thread.h"
#include "watchdog/watchdog.h"
#include "audioflinger.h"
#include "aiot_generic.h"
#include "heap_api.h"
#ifdef CP_SUBSYS_TEST
#include "app_cp_subsys.h"
#endif

#ifdef APP_RPC_BTH_DSP_EN
#include "app_rpc_api.h"
#include "rpc_bth_dsp.h"
#define APP_RPC_CORE_REMOTE APP_RPC_CORE_BTH_DSP
#endif

#ifdef APP_RPC_MCU_SENSOR_EN
#include "app_rpc_api.h"
#include "app_sensor_hub.h"
#define APP_RPC_CORE_REMOTE APP_RPC_CORE_MCU_SENSOR
#endif

#ifdef ARM_CMNS
#include "tz_trace_ns.h"
#endif
#if defined(UTILS_ESHELL_EN)
#include "eshell.h"
#include "app_eshell.h"
#endif
#ifdef NET_EN
#include "net_main.h"
#endif

#include "apps.h"
#include "factory_section.h"

#ifdef __ALSA_EN__
#include "alsa.h"
#endif

#ifdef BTH_IF_COMM_RPC_EN
#include "bth_if_thread_client.h"
#include "bth_bt_if_client.h"
#include "bth_ota_rx_handler.h"
#endif

#include "sensor_hub.h"
#include "bt_host.h"
#ifdef DSP_ENABLE
#include "mcu_dsp_hifi4_app.h"
#endif

#if defined(MBEDTLS_INIT_SUPPORT)
#include "mbedtls/mbedtls_init.h"
#endif

#ifdef VOWLAN_SUPPORT
#include "vowlan_main.h"
#endif

#ifdef LOAD_LINUX
#include "fw_linux_loader.h"
#endif

#ifdef USE_TRACE_ID
const char *unkonw_str __attribute__((section(".trc_str"))) = "unknow string";
#endif

#ifndef MULTIMEDIA_DISABLE
#if !defined(BT_RPC_EN) && !defined(BT_NATIVE_SUPPORT) && !defined(BT_CLOSE) && defined(__BT_APP__)
#define MULTIMEDIA_DISABLE
#endif
#endif

static osThreadId main_thread_tid = NULL;

#define PMU_RESERVED_REG (0x39)
#define UART_SWITCH_FLAG_BIT (15)
static bool uart0_is_i2c = false;
static void uart0_i2c_switch_timer_handler(void const *param);
osTimerDef(UART0_I2C_SWITCH_TIMER, uart0_i2c_switch_timer_handler);
static osTimerId uart0_i2c_switch_timer = NULL;

static void uart0_i2c_switch_timer_handler(void const *param)
{
    uint16_t reg16 = 0;
    pmu_read(PMU_RESERVED_REG, &reg16);
    if (uart0_is_i2c) {
        if (!(reg16 & (1 << UART_SWITCH_FLAG_BIT))) {
            hal_iomux_set_uart0();
            uart0_is_i2c = false;
            osTimerStop(uart0_i2c_switch_timer);
            osTimerDelete(uart0_i2c_switch_timer);
            uart0_i2c_switch_timer = NULL;
        }
    }
}

/*
call this funtion will swith uart0 to i2c
if use i2c tools write pmu 0x39 bit15, it'll switch i2c to uart0
*/
int hal_uart0_i2c_switch(void)
{
    uint16_t reg16 = 0;
#if (DEBUG_PORT != 1)
    return -1;
#endif
    if (!uart0_is_i2c) {
        //switch uart to i2c
        hal_iomux_set_analog_i2c();
        uart0_is_i2c = true;
        pmu_read(PMU_RESERVED_REG, &reg16);
        reg16 |= (1 << UART_SWITCH_FLAG_BIT);
        pmu_write(PMU_RESERVED_REG, reg16);
        if (!uart0_i2c_switch_timer) {
            uart0_i2c_switch_timer = osTimerCreate(osTimer(UART0_I2C_SWITCH_TIMER), osTimerPeriodic, NULL);
            if (uart0_i2c_switch_timer) {
                osTimerStart(uart0_i2c_switch_timer, 500);
            }
        }
    }
    return 0;
}

void hal_uart0_i2c_switch_init(void)
{
    uint16_t reg16 = 0;
    //should set after bt power on  or it'll down
    uart0_is_i2c = false;
    pmu_read(PMU_RESERVED_REG, &reg16);
    reg16 &= ~(1 << UART_SWITCH_FLAG_BIT);
    pmu_write(PMU_RESERVED_REG, reg16);
    uart0_i2c_switch_timer = NULL;
}

int system_shutdown(void)
{
    TR_INFO(TR_MOD(MAIN), "system_shutdown!!");
    osThreadSetPriority(main_thread_tid, osPriorityRealtime);
    osSignalSet(main_thread_tid, 0x4);
    while (1) {
        TRACE(0, "system_shutdown");
        osDelay(1000);
    }
    return 0;
}

int system_reset(void)
{
    osThreadSetPriority(main_thread_tid, osPriorityRealtime);
    osSignalSet(main_thread_tid, 0x8);
    while (1) {
        TRACE(0, "system_reset");
        osDelay(1000);
    }
    return 0;
}

int signal_send_to_main_thread(uint32_t signals)
{
    osSignalSet(main_thread_tid, signals);
    return 0;
}

int platform_trace_disable(void)
{
    hal_trace_flush_buffer();
    hal_trace_close();
    return 0;
}

int platform_trace_enable(void)
{
#ifdef DEBUG
#if (DEBUG_PORT == 1)
    hal_iomux_set_uart0();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART0);
#endif

#if (DEBUG_PORT == 2)
    hal_iomux_set_analog_i2c();
    hal_iomux_set_uart1();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART1);
#endif

#if (DEBUG_PORT == 3)
    hal_iomux_set_analog_i2c();
    hal_iomux_set_uart2();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART2);
#endif

    hal_sleep_start_stats(10000, 10000);

#ifdef ARM_CMNS
    cmns_trace_init();
#endif
#endif

    return 0;
}

int platform_driver_init(int flags)
{
    int ret = 0;

#if (defined(PSRAM_ENABLE) || defined(PSRAMUHS_ENABLE)) && defined(PSRAM_INIT_CHECK)
    hal_wdt_stop(HAL_WDT_ID_0);
#endif

#ifdef __WATCHER_DOG_RESET__
    if (flags & PLAT_INIT_FLAG_WDG_EN)
        app_wdt_open(30);
#endif

#ifdef TRNG_BASE
    struct HAL_TRNG_CFG_T trng_cfg = {0x100, HAL_TRNG_RND_SRC_SHORTEST};
    hal_trng_open(&trng_cfg);
#endif

#if defined(ROM_UTILS_ON)
    rom_utils_init();
#endif

    main_thread_tid = osThreadGetId();

    hwtimer_init();

    hal_dma_set_delay_func((HAL_DMA_DELAY_FUNC)osDelay);
    hal_audma_open();
    hal_gpdma_open();
    norflash_api_init();
#if defined(DUMP_LOG_ENABLE)
    log_dump_init();
#endif
#if (defined(DUMP_CRASH_LOG) || defined(TOTA_CRASH_DUMP_TOOL_ENABLE))
    crash_dump_init();
#endif

#if defined(RAM_DUMP_TO_FLASH)
    ramdump_to_flash_init();
#endif

#ifdef CORE_DUMP_TO_FLASH
    coredump_to_flash_init();
#endif

    platform_trace_enable();

    hal_iomux_ispi_access_init();

    uint8_t flash_id[HAL_NORFLASH_DEVICE_ID_LEN];
    hal_norflash_get_id(HAL_FLASH_ID_0, flash_id, ARRAY_SIZE(flash_id));
    TR_INFO(TR_MOD(MAIN), "FLASH_ID: %02X-%02X-%02X", flash_id[0], flash_id[1], flash_id[2]);
    hal_norflash_show_calib_result(HAL_FLASH_ID_0);
    ASSERT(hal_norflash_opened(HAL_FLASH_ID_0), "Failed to init flash: %d", hal_norflash_get_open_state(HAL_FLASH_ID_0));

    // Software will load the factory data and user data from the bottom TWO sectors from the flash,
    // the FLASH_SIZE defined is the common.mk must be equal or greater than the actual chip flash size,
    // otherwise the ota will load the wrong information
    uint32_t actualFlashSize = hal_norflash_get_flash_total_size(HAL_FLASH_ID_0);
    if (FLASH_SIZE > actualFlashSize) {
        TRACE_IMM(0,"Wrong FLASH_SIZE defined in target.mk!");
        TRACE_IMM(2,"FLASH_SIZE is defined as 0x%x while the actual chip flash size is 0x%x!", FLASH_SIZE, actualFlashSize);
        TRACE_IMM(1,"Please change the FLASH_SIZE in common.mk to 0x%x to enable the OTA feature.", actualFlashSize);
        ASSERT(false, " ");
    }

    mpu_cfg();
#if !defined(__SUBSYS_ONLY__)
    pmu_open();
    analog_open();
#endif

    return ret;
}

#ifdef BTH_IF_COMM_RPC_EN
POSSIBLY_UNUSED static bth_server_state_monitor_cb bth_alive_state_report = NULL;
#endif

#if defined(APP_RPC_ENABLE) || defined(APP_RPC_MCU_SENSOR_EN)
#define BTH_PING_TIMEOUT_MAX_NUM        (3)
static uint16_t bth_ping_timeout_cnt = 0;
static uint8_t sys_ping_bth_timeout_handle(uint8_t status)
{
    bth_ping_timeout_cnt++;
    TRACE(1, "sys ping bth timeout:%d", bth_ping_timeout_cnt);
    if (bth_ping_timeout_cnt >= BTH_PING_TIMEOUT_MAX_NUM) {
        bth_ping_timeout_cnt = 0;
#ifdef BTH_IF_COMM_RPC_EN
        if (bth_alive_state_report) {
            bth_alive_state_report(false);
        }
#endif
    }
    return 0;
}

static uint8_t sys_ping_bth_rsp_handle(uint8_t status)
{
    if (status) {
        bth_ping_timeout_cnt = 0;
        TRACE(0, "sys ping bth success");
    } else {
        bth_ping_timeout_cnt++;
        TRACE(1, "sys ping bth failure:%d", bth_ping_timeout_cnt);
    }
#ifdef BTH_IF_COMM_RPC_EN
    if (bth_ping_timeout_cnt >= BTH_PING_TIMEOUT_MAX_NUM) {
        bth_ping_timeout_cnt = 0;
        if (bth_alive_state_report) {
            bth_alive_state_report(false);
        }
    } else if (bth_ping_timeout_cnt == 0) {
        if (bth_alive_state_report) {
            bth_alive_state_report(true);
        }
    }
#endif
    return 0;
}

static void app_bth_conn_ping_timer_handler(void const *param);
osTimerDef (APP_PING_BTH_CONN_TIMER, app_bth_conn_ping_timer_handler);
static osTimerId app_bth_conn_ping_timer_id = NULL;
#define BTH_PING_PERIOD_MS              (5000)
static void app_bth_conn_ping_timer_handler(void const *param)
{
#if defined(APP_RPC_ENABLE)
    app_rpc_ping_cmd_send_handle(APP_RPC_CORE_BTH_DSP);
#elif defined(APP_RPC_MCU_SENSOR_EN)
    app_rpc_ping_cmd_send_handle(APP_RPC_CORE_MCU_SENSOR);
#endif
    osTimerStart(app_bth_conn_ping_timer_id, BTH_PING_PERIOD_MS);
}
#endif

WEAK void aiot_platform_pre_initializing_callback(void)
{

}

WEAK void aiot_platform_initialized_callback(void)
{

}

void sensor_hub_tx_done(const void* data, unsigned int len)
{

}

unsigned int sensor_hub_data_received(const void* data, unsigned int len)
{
    return len;
}

#if defined(BT_RPC_EN) || defined(BT_HOST)
POSSIBLY_UNUSED static void bt_host_tx_done(const void* data, unsigned int len)
{
#ifdef APP_RPC_BTH_DSP_EN
    app_rpc_tx_handler_process(APP_RPC_CORE_BTH_DSP, data, len);
#endif

}

POSSIBLY_UNUSED static unsigned int bt_host_data_received(const void* data, unsigned int len)
{
#ifdef APP_RPC_BTH_DSP_EN
    app_rpc_rx_handler_process(APP_RPC_CORE_BTH_DSP, data, len);
#endif
    return len;
}
#endif

#ifdef NORMAL_TEST_MODE_SWITCH
extern void app_factorymode_bt_signalingtest_handler(void);
extern void app_factorymode_bt_nosignalingtest_handler(void);
#endif

int platform_framework_init(int flags)
{
    int ret = 0;

    aiot_platform_pre_initializing_callback();

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    TRACE(1,"sys freq calc : %ld\n", (long)hal_sys_timer_calc_cpu_freq(5, 0));

    main_thread_tid = osThreadGetId();

    srand(hal_sys_timer_get());

    syspool_init();

    factory_section_init();
    factory_section_open();

    /* load eshell module */
#if defined(UTILS_ESHELL_EN)
    if (flags & PLAT_INIT_FLAG_ESHELL_EN) {
#if defined(APP_RPC_ENABLE)
        app_eshell_rpc_init();
        eshell_rpc_run_cmd_func_cb_register(app_eshell_rpc_send_op_cmd_func, app_eshell_rpc_send_op_result_func);
        eshell_open(HAL_UART_ID_1, false);
#else
        eshell_open(HAL_UART_ID_1, true);
#endif
    }
#endif

#ifdef LOAD_LINUX
    ap_open();
#endif

    /* load NET module */
#ifdef NET_EN
    if (flags & PLAT_INIT_FLAG_NET_EN) {
        net_system_init();
    }
#endif
#if defined(MBEDTLS_INIT_SUPPORT)
    mbedtls_init();
#endif
    /* load multimedia module */
#ifndef MULTIMEDIA_DISABLE
    af_open();
#ifdef __ALSA_EN__
    alsa_init();
#endif
#endif

#ifdef CP_SUBSYS_TEST
    cp_subsys_init();
#endif

#ifdef BTH_IF_COMM_RPC_EN
    app_bth_if_client_init(&bth_alive_state_report);
    app_bth_bt_if_client_init();
    app_bth_a2dp_if_client_init();
    app_bth_hfp_cli_if_client_init();
    app_bth_a2dp_sink_if_client_init();
    app_bth_ble_if_client_init();
    app_bth_gatt_if_client_init();
    app_bth_ota_thread_init();
#endif

#ifdef BT_HOST
    bt_host_open(bt_host_data_received, bt_host_tx_done);
#endif

#if defined(APP_RPC_ENABLE) || defined(APP_RPC_MCU_SENSOR_EN)
    app_rpc_ping_handle_cb_regist(sys_ping_bth_timeout_handle, sys_ping_bth_rsp_handle);
#endif

#ifdef SENSOR_HUB
    extern void app_sensor_hub_init(void);
    app_sensor_hub_init();
#endif

    /* bt rpc module */
#ifdef BT_RPC_EN

#if defined(APP_RPC_ENABLE) && defined(APP_RPC_BTH_DSP_EN)
    app_rpc_ctx_init(APP_RPC_CORE_BTH_DSP);
    //app_rpc_core_open(APP_RPC_CORE_BTH_DSP);
#endif
#elif defined(BT_NATIVE_SUPPORT)
    if (flags & PLAT_INIT_FLAG_BT_EN) {
        extern void app_aiot_bt_init(void);
        app_aiot_bt_init();
#if defined(BTUSB_AUDIO_MODE)
        app_usbaudio_entry();
#endif // BTUSB_AUDIO_MODE
    }
#else

#if !defined(BT_CLOSE) && defined(__BT_APP__)
    app_init();
#elif defined(CHIP_BEST1502X)
    extern void btdrv_start_bt(void);
    extern void app_sensor_hub_init(void);
    btdrv_start_bt();
#ifdef SENSOR_HUB
    app_sensor_hub_init();
#endif
#endif
#endif

    /* app bootstrap */
#ifdef APP_BOOTSTRAP_EN
    INIT_APP_CALL(feature);
#endif

#if defined(APP_RPC_ENABLE) || defined(APP_RPC_MCU_SENSOR_EN)
    if (app_bth_conn_ping_timer_id == NULL) {
        app_bth_conn_ping_timer_id = osTimerCreate(osTimer(APP_PING_BTH_CONN_TIMER), osTimerOnce, NULL);
        //osTimerStart(app_bth_conn_ping_timer_id, 1000);
    }
#endif

#if defined(OPENAMP_ENABLE)
    void amp_dsp2mcu_init(void);
    amp_dsp2mcu_init();
#endif

#ifdef __A7_DSP_ENABLE__
    if (flags & PLAT_INIT_FLAG_A7_EN) {
        app_os_init();
        extern void a7_dsp_boot(void);
        a7_dsp_boot();
    }
#endif

#ifdef USB_DISK_TEST
    extern void app_usb_host_disk_test(void);
    app_usb_host_disk_test();
#endif

#ifdef VOWLAN_SUPPORT
    vowlan_app_init();
#endif

#ifdef NORMAL_TEST_MODE_SWITCH
    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE)
    {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MASK);
        app_factorymode_bt_nosignalingtest_handler();
    }
    else if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_SIGNALINGMODE)
    {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MASK);
        app_factorymode_bt_signalingtest_handler();
    }
#endif

    /* release freq request after init */
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);

    aiot_platform_initialized_callback();

#ifdef DSP_ENABLE
    app_dsp_hifi4_init(APP_DSP_HIFI4_USER_AIOT);
#endif

    return ret;
}

void platform_deamon_start(void)
{
    uint8_t sys_case = 0;

    while (1) {
        osEvent evt;
#ifndef __POWERKEY_CTRL_ONOFF_ONLY__
        osSignalClear (main_thread_tid, 0x0f);
#endif
        //wait any signal
        evt = osSignalWait(0x0, osWaitForever);

        //get role from signal value
        if (evt.status == osEventSignal) {
            if (evt.value.signals & 0x04) {
                sys_case = 1;
                break;
            } else if (evt.value.signals & 0x08) {
                sys_case = 2;
                break;
            }
        } else {
            sys_case = 1;
            break;
        }
    }

    system_shutdown_wdt_config(10);
    TR_INFO(TR_MOD(MAIN), "byebye~~~ %d\n", sys_case);
    if ((sys_case == 1) || (sys_case == 0)) {
        TR_INFO(TR_MOD(MAIN), "shutdown\n");
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
        pmu_shutdown();
    } else if (sys_case == 2) {
        TR_INFO(TR_MOD(MAIN), "reset\n");
        pmu_reboot();
    }
}

#ifndef USE_OS_MAIN
int WEAK main(void)
{
    platform_driver_init(PLAT_INIT_FLAG_ALL_EN);
    platform_framework_init(PLAT_INIT_FLAG_ALL_EN);
    platform_deamon_start();
    return 0;
}
#endif
