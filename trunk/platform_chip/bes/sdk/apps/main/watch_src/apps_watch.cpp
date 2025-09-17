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
#include "stdio.h"
#include "cmsis_os.h"
#include "list.h"
#include "string.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_bootmode.h"
#include "hal_sleep.h"
#include "hal_uart.h"
#include "pmu.h"
#include "audioflinger.h"
#include "apps.h"
#include "app_thread.h"
#include "app_key.h"
#include "bluetooth_bt_api.h"
#include "app_pwl.h"
#include "app_overlay.h"
#include "app_trace_rx.h"
#include "app_utils.h"
#include "bt_drv_interface.h"
#include "norflash_api.h"
#include "nvrecord_bt.h"
#include "nvrecord_dev.h"
#include "nvrecord_env.h"
#include "crash_dump_section.h"
#include "log_section.h"
#include "factory_section.h"
#include "watchdog/watchdog.h"
#include "main_api.h"

#include "app_audio.h"
#include "app_bt_audio.h"
#include "audio_manager.h"

#ifdef BTH_TRC_TO_SYS
#include "bth_trc_to_sys.h"
#endif

#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#endif

#if defined(BT_WATCH_MASTER) || defined(BT_WATCH_SLAVE)
#include "app_bt_watch.h"
#endif

#ifdef PC_CMD_UART_ID
#include "app_cmd.h"
#endif

#ifdef __FACTORY_MODE_SUPPORT__
#include "app_factory.h"
#include "app_factory_bt.h"
#endif

#ifdef PROMPT_IN_FLASH
#include "nvrecord_prompt.h"
#endif

#ifdef BT_USB_AUDIO_DUAL_MODE
#include "btusb_audio.h"
#endif

#ifdef SENSOR_HUB
#include "mcu_sensor_hub_app.h"
#endif

#ifdef DSP_M55
#include "mcu_dsp_m55_app.h"
#endif

#ifdef DSP_HIFI4
#include "dsp_loader.h"
#endif

#if defined(APP_RPC_ENABLE) || defined(APP_RPC_MCU_SENSOR_EN)
#include "app_rpc_api.h"
#ifdef APP_RPC_BTH_DSP_EN
#include "rpc_bth_dsp.h"
#endif
#ifdef APP_RPC_MCU_SENSOR_EN
#include "app_sensor_hub.h"
#endif
#endif

#if defined(UTILS_ESHELL_EN)
#include "eshell.h"
#include "app_eshell.h"
#endif

#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
#include "codec_calib.h"
#endif

#ifdef APP_UART_MODULE
#include "app_uart_dma_thread.h"
#endif

#ifdef MSD_MODE
#include "sdmmc_msd.h"
#endif

#ifdef MTEST_BUILTIN
#include "mem_scan.h"
#endif

#ifdef BLE_WALKIE_TALKIE
#include "walkie_talkie_test.h"
#include "app_walkie_talkie.h"
#endif

#ifdef BTH_IF_COMM_RPC_EN
#include "bth_if_thread_server.h"
#include "bth_bt_if_server.h"
#endif

#if defined(__HCI_BRIDGE_LOCAL_MODE__) || defined(__HCI_BRIDGE_REMOTE_MODE__)
#include "app_hci_bridge.h"
#endif

#define POWERON_PRESSMAXTIME_THRESHOLD_MS (5000)
enum APP_POWERON_CASE_T {
    APP_POWERON_CASE_NORMAL = 0,
    APP_POWERON_CASE_DITHERING,
    APP_POWERON_CASE_REBOOT,
    APP_POWERON_CASE_ALARM,
    APP_POWERON_CASE_CALIB,
    APP_POWERON_CASE_BOTHSCAN,
    APP_POWERON_CASE_CHARGING,
    APP_POWERON_CASE_FACTORY,
    APP_POWERON_CASE_TEST,
    APP_POWERON_CASE_INVALID,
    APP_POWERON_CASE_NUM
};

static uint8_t app_poweroff_flag = 0;
static enum APP_POWERON_CASE_T g_pwron_case = APP_POWERON_CASE_INVALID;
static uint8_t stack_ready_flag = 0;
#if defined(APP_RPC_ENABLE) || defined(APP_RPC_MCU_SENSOR_EN)
static uint8_t app_stack_ready_rpc_notify_send_timeout(uint8_t status)
{
    TRACE(0, "bth ready indication send timeout");
    return 0;
}

static uint8_t app_stack_ready_rpc_notify_recv_response(uint8_t status)
{
    TRACE(0, "bth ready indication recv peer confirm");
    return 0;
}
#endif
static void app_stack_ready_rpc_notify_send_handler(void)
{
#if defined(APP_RPC_ENABLE)
    app_rpc_send_data_waiting_rsp(APP_RPC_CORE_BTH_DSP, BTH_TASK_CMD_STACK_READY_IND, NULL, 0);
#elif defined(APP_RPC_MCU_SENSOR_EN)
    app_rpc_send_data_waiting_rsp(APP_RPC_CORE_MCU_SENSOR, BTH_TASK_CMD_STACK_READY_IND, NULL, 0);
#endif
}

void app_notify_stack_ready(uint8_t ready_flag)
{
    TRACE(2, "app_notify_stack_ready %d %d", stack_ready_flag, ready_flag);

    stack_ready_flag |= ready_flag;
#ifdef __IAG_BLE_INCLUDE__
    if (stack_ready_flag == (STACK_READY_BT | STACK_READY_BLE))
#endif
    {
        signal_send_to_main_thread(STACK_READY_BLE | STACK_READY_BT);
        app_stack_ready_rpc_notify_send_handler();
    }
}

bool app_is_stack_ready(void)
{
    bool ret = false;

    if (stack_ready_flag == (STACK_READY_BT
#ifdef __IAG_BLE_INCLUDE__
                             | STACK_READY_BLE
#endif
                             ))
    {
        ret = true;
    }

    return ret;
}

static void app_stack_ready_cb(void)
{
    TRACE(0, "stack init done");
#ifdef __IAG_BLE_INCLUDE__
    bes_ble_app_init(NV_APP_EARBUDS_MONO);

#ifdef BLE_HOST_PTS_TEST_ENABLED
    // Noting to do
#else

#if !(BLE_AUDIO_ENABLED)
    bes_ble_gap_stub_user_init();
#endif

#ifndef BLE_STACK_NEW_DESIGN
#ifndef BLE_ADV_NOT_START_DEFAULT
    bes_ble_gap_start_connectable_adv(BLE_ADVERTISING_INTERVAL);
#endif
#endif

#endif // BLE_HOST_PTS_TEST_ENABLED
#endif //__IAG_BLE_INCLUDE__
}

static bool g_pwron_finished = false;
static void app_poweron_normal(APP_KEY_STATUS *status, void *param)
{
    TRACE(3, "%s %d,%d", __func__, status->code, status->event);
    g_pwron_case = APP_POWERON_CASE_NORMAL;
    signal_send_to_main_thread(0x2);
}

static void app_poweron_scan(APP_KEY_STATUS *status, void *param)
{
    TRACE(3, "%s %d,%d", __func__, status->code, status->event);
    g_pwron_case = APP_POWERON_CASE_BOTHSCAN;
    signal_send_to_main_thread(0x2);
}

static void app_poweron_finished(APP_KEY_STATUS *status, void *param)
{
    TRACE(3, "%s %d,%d", __func__, status->code, status->event);
    g_pwron_finished = true;
    signal_send_to_main_thread(0x2);
}

static void app_poweron_wait_finished(void)
{
    if (!g_pwron_finished) {
        osSignalWait(0x2, osWaitForever);
    }
}

const APP_KEY_HANDLE pwron_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR, APP_KEY_EVENT_INITUP}, "power on: normal", app_poweron_normal, NULL},
    {{APP_KEY_CODE_PWR, APP_KEY_EVENT_INITLONGPRESS}, "power on: both scan", app_poweron_scan, NULL},
    {{APP_KEY_CODE_PWR, APP_KEY_EVENT_INITFINISHED}, "power on: finished", app_poweron_finished, NULL},
};

static void app_poweron_key_init(void)
{
    uint8_t i = 0;
    TRACE(1, "%s", __func__);

    for (i = 0; i < ARRAY_SIZE(pwron_key_handle_cfg); i++) {
        app_key_handle_registration(&pwron_key_handle_cfg[i]);
    }
}

static uint8_t app_poweron_wait_case(void)
{
    uint32_t stime = 0, etime = 0;

    if (g_pwron_case == APP_POWERON_CASE_INVALID) {
        stime = hal_sys_timer_get();
        osSignalWait(0x2, POWERON_PRESSMAXTIME_THRESHOLD_MS);
        etime = hal_sys_timer_get();
    }
    TRACE(2, "powon raw case:%d time:%d", g_pwron_case, TICKS_TO_MS(etime - stime));
    return g_pwron_case;
}

static void POSSIBLY_UNUSED app_wait_stack_ready(void)
{
    uint32_t stime, etime;
    stime = hal_sys_timer_get();
    osSignalWait(0x3, osWaitForever);
    etime = hal_sys_timer_get();
    TRACE(1, "app_wait_stack_ready: wait:%d ms", TICKS_TO_MS(etime - stime));

    app_stack_ready_cb();
}

void app_bt_key_shutdown(APP_KEY_STATUS *status, void *param)
{
    TRACE(3, "%s %d,%d", __func__, status->code, status->event);
    app_shutdown();
}

static void app_bt_key(APP_KEY_STATUS *status, void *param)
{
    bool continue_handle = true;
    TRACE(3, "%s %d,%d", __func__, status->code, status->event);
    switch (status->event) {
    case APP_KEY_EVENT_CLICK:
        TRACE(0, "first blood!");
        if (status->code == APP_KEY_CODE_PWR) {
        }
        break;
    case APP_KEY_EVENT_DOUBLECLICK:
        TRACE(0, "double kill");
        if (status->code == APP_KEY_CODE_PWR) {
        }
        break;
    case APP_KEY_EVENT_TRIPLECLICK:
        TRACE(0, "triple kill");
        if (status->code == APP_KEY_CODE_PWR) {
        }
        break;
    case APP_KEY_EVENT_ULTRACLICK:
        TRACE(0, "ultra kill");
        break;
    case APP_KEY_EVENT_RAMPAGECLICK:
        TRACE(0, "rampage kill!you are crazy!");
        break;
    }
    if (continue_handle) {
        bes_bt_me_send_bt_key(status);
    }
}

static const APP_KEY_HANDLE app_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR, APP_KEY_EVENT_LONGLONGPRESS}, "bt function key", app_bt_key_shutdown, NULL},
    {{APP_KEY_CODE_PWR, APP_KEY_EVENT_LONGPRESS}, "bt function key", app_bt_key, NULL},
    {{APP_KEY_CODE_PWR, APP_KEY_EVENT_CLICK}, "bt function key", app_bt_key, NULL},
    {{APP_KEY_CODE_PWR, APP_KEY_EVENT_DOUBLECLICK}, "bt function key", app_bt_key, NULL},
    {{APP_KEY_CODE_PWR, APP_KEY_EVENT_TRIPLECLICK}, "bt function key", app_bt_key, NULL},
};

static void app_key_init(void)
{
    uint8_t i = 0;
    TRACE(1, "%s", __func__);

    app_key_handle_clear();
    for (i = 0; i < ARRAY_SIZE(app_key_handle_cfg); i++) {
        app_key_handle_registration(&app_key_handle_cfg[i]);
    }
}

#ifdef OS_THREAD_TIMING_STATISTICS_ENABLE
extern "C" void rtx_show_all_threads_usage(void);
static void cpu_usage_timer_handler(void const *param);
osTimerDef(cpu_usage_timer, cpu_usage_timer_handler);
static osTimerId cpu_usage_timer_id = NULL;
static void cpu_usage_timer_handler(void const *param)
{
    rtx_show_all_threads_usage();
}
#endif

#if defined(APP_RPC_ENABLE) || defined(APP_RPC_MCU_SENSOR_EN)
static uint8_t app_rpc_ping_cmd_recv_handle_cb(uint8_t status)
{
    uint8_t state = app_is_stack_ready();
    return state;
}
#endif

static POSSIBLY_UNUSED int app_nvrecord_rebuild(void)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    nv_record_sector_clear();
    nv_record_env_init();
    nv_record_update_factory_tester_status(NVRAM_ENV_FACTORY_TESTER_STATUS_TEST_PASS);
    nv_record_env_set(nvrecord_env);
    nv_record_flash_flush();
    return 0;
}

static int app_power_reboot_mode_handler(void)
{
    bes_bt_sleep_init();
    app_key_init();
#ifdef __IAG_BLE_INCLUDE__
    bes_ble_app_reinit();
#endif
    return 0;
}

static int app_power_test_mode_handler(void)
{
#ifdef __ENGINEER_MODE_SUPPORT__
    app_factorymode_set(true);
#ifdef __WATCHER_DOG_RESET__
    app_wdt_close();
#endif
    TRACE(0, "!!!!!ENGINEER_MODE!!!!!\n");
    app_nvrecord_rebuild();
    app_factorymode_key_init();
    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_SIGNALINGMODE) {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MASK);
        app_factorymode_bt_signalingtest(NULL, NULL);
    }
    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE) {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MASK);
        app_factorymode_bt_nosignalingtest(NULL, NULL);
    }
#endif
    return 0;
}

static void app_watch_bt_init(void)
{
    if (bes_bt_me_bes_cfg_get(BT_BES_CFG_BT_SINK_ENABLED)) {
        bes_bt_init_global_handle();
    }
}

static void app_rpc_enable(void)
{
#if defined(APP_RPC_ENABLE) && defined(APP_RPC_BTH_DSP_EN)
    app_rpc_ctx_init(APP_RPC_CORE_BTH_DSP);
    app_rpc_core_open(APP_RPC_CORE_BTH_DSP);
#endif

#ifdef BTH_IF_COMM_RPC_EN
    bth_if_thread_server_init();
    app_bth_bt_if_init();
    app_bth_a2dp_src_if_init();
    app_bth_hfp_cli_if_init();
    app_bth_a2dp_sink_if_init();
#ifdef BLE_HOST_SUPPORT
    app_bth_ble_if_init();
    app_bth_gatt_if_init();
#endif
#endif

#if defined(APP_RPC_ENABLE) || defined(APP_RPC_MCU_SENSOR_EN)
    app_rpc_ping_recv_handle_cb_regist(app_rpc_ping_cmd_recv_handle_cb);
    app_rpc_bth_ready_ind_send_handler_cb_regist(app_stack_ready_rpc_notify_send_timeout,
                                                    app_stack_ready_rpc_notify_recv_response);
#endif
}


extern uint32_t __coredump_section_start[];
extern uint32_t __ota_upgrade_log_start[];
extern uint32_t __log_dump_start[];
extern uint32_t __crash_dump_start[];
extern uint32_t __custom_parameter_start[];
extern uint32_t __lhdc_license_start[];
extern uint32_t __aud_start[];
extern uint32_t __userdata_start[];
extern uint32_t __factory_start[];
int app_init(void)
{
    int nRet = 0;
    struct nvrecord_env_t *nvrecord_env;
    bool need_check_key = false;
    uint8_t pwron_case = APP_POWERON_CASE_INVALID;

#ifndef RAM_NV_RECORD
    TRACE(0, "please check all sections sizes and heads is correct ........");
    TRACE(2, "__coredump_section_start: %p length: 0x%x", __coredump_section_start, CORE_DUMP_SECTION_SIZE);
    TRACE(2, "__ota_upgrade_log_start: %p length: 0x%x", __ota_upgrade_log_start, OTA_UPGRADE_SECTION_SIZE);
    TRACE(2, "__log_dump_start: %p length: 0x%x", __log_dump_start, LOG_DUMP_SECTION_SIZE);
    TRACE(2, "__crash_dump_start: %p length: 0x%x", __crash_dump_start, CRASH_DUMP_SECTION_SIZE);
    TRACE(2, "__custom_parameter_start: %p length: 0x%x", __custom_parameter_start, CUSTOM_PARAMETER_SECTION_SIZE);
    TRACE(2, "__lhdc_license_start: %p length: 0x%x", __lhdc_license_start, LHDC_LICENSE_SECTION_SIZE);
    TRACE(2, "__userdata_start: %p length: 0x%x", __userdata_start, USERDATA_SECTION_SIZE * 2);
    TRACE(2, "__aud_start: %p length: 0x%x", __aud_start, AUD_SECTION_SIZE);
    TRACE(2, "__factory_start: %p length: 0x%x", __factory_start, FACTORY_SECTION_SIZE);
#endif
    TRACE(0, "app_watch_init\n");

#ifdef APP_COREMARK_TEST
    app_rpc_enable();
    goto exit;
#endif

#ifdef FLASH_UNIQUE_ID
    uint8_t flash_unique_id[HAL_NORFLASH_UNIQUE_ID_LEN] = {0};
    hal_norflash_get_unique_id(HAL_FLASH_ID_0, flash_unique_id, ARRAY_SIZE(flash_unique_id));
    TRACE(0, "GET_FLASH_UNIQUE_ID:");
    DUMP8("%02X ", flash_unique_id, ARRAY_SIZE(flash_unique_id));
#endif

#ifdef APP_TRACE_RX_ENABLE
    app_trace_rx_open();
    bes_bt_me_init_cmd();
#endif
    bes_bt_me_add_string_test_table();

    nv_record_init();
#if !defined(__NuttX__)
    factory_section_init();
#endif

    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_104M);

#if defined(MCU_HIGH_PERFORMANCE_MODE)
    TRACE(1, "sys freq calc : %d\n", hal_sys_timer_calc_cpu_freq(5, 0));
#endif
#ifdef OS_THREAD_TIMING_STATISTICS_ENABLE
    cpu_usage_timer_id = osTimerCreate(osTimer(cpu_usage_timer), osTimerPeriodic, NULL);
    if (cpu_usage_timer_id != NULL)
        osTimerStart(cpu_usage_timer_id, OS_THREAD_TIMING_STATISTICS_PEROID_MS);
#endif

    list_init();
    nRet = app_os_init();
    if (nRet)
        goto exit;

#ifdef BTADDR_FOR_DEBUG
    bes_bt_me_gen_addr_for_debug();
#endif

    app_rpc_enable();

#ifdef FORCE_SIGNALINGMODE
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE | HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
#elif defined FORCE_NOSIGNALINGMODE
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE | HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE);
#endif

#if defined (__HCI_BRIDGE_LOCAL_MODE__)
    app_hci_bridge_init(false);
#elif defined (__HCI_BRIDGE_REMOTE_MODE__)
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE);
    app_hci_bridge_init(true);
#endif

    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_REBOOT_FROM_CRASH) {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT_FROM_CRASH);
        TRACE(0, "Crash happened!!!");
    }

    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_REBOOT) {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
        pwron_case = APP_POWERON_CASE_REBOOT;
        need_check_key = false;
        TRACE(0, "Initiative REBOOT happens!!!");
    }

    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_MODE) {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MODE);
        pwron_case = APP_POWERON_CASE_TEST;
        need_check_key = false;
        TRACE(0, "To enter test mode!!!");
    }

#if !defined(IS_CUSTOM_UART_APPLICATION_ENABLED)
#ifdef APP_UART_MODULE
    app_uart_init();
#endif
#endif

    if (app_key_open(need_check_key)) {
        TRACE(0, "PWR KEY DITHER!");
        nRet = -1;
        goto exit;
    }

    hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
    app_poweron_key_init();

    bes_bt_app_init();
    amgr_init();
    //app_audio_open();
    //app_audio_manager_open();
    app_overlay_open();
    app_overlay_subsys_open();

    nv_record_env_init();
    nvrec_dev_data_open();
    nv_record_env_get(&nvrecord_env);

#ifdef PC_CMD_UART_ID
    app_cmd_open();
#endif

#if defined(UTILS_ESHELL_EN)
#ifdef APP_RPC_ENABLE
    app_eshell_rpc_init();
    eshell_rpc_run_cmd_func_cb_register(app_eshell_rpc_send_op_cmd_func, app_eshell_rpc_send_op_result_func);
#endif
    eshell_open(HAL_UART_ID_1, false);
#endif

#ifdef BLE_WALKIE_TALKIE
    walkie_talkie_uart_cmd_init();
#endif

    // enable and start btc
    btdrv_start_bt();
    if (pwron_case != APP_POWERON_CASE_TEST) {
        bes_bt_thread_init();
        app_wait_stack_ready();
#if defined(SENSOR_HUB) && !defined(__NuttX__)
        app_sensor_hub_init();
#endif
        bt_drv_config_after_hci_reset();
        bes_bt_me_gen_ecdh_key_pair();

        bes_bt_call_custom_function_in_bt_thread((uint32_t)0,
            0, (uint32_t)app_watch_bt_init);
    }

    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_52M);
    TRACE(1, "\n\n\nbt_stack_init_done:%d\n\n\n", pwron_case);

    app_application_ready_to_start_callback();
    if (pwron_case == APP_POWERON_CASE_REBOOT) {
        nRet = app_power_reboot_mode_handler();
    } else if (pwron_case == APP_POWERON_CASE_TEST) {
        nRet = app_power_test_mode_handler();
    } else {
        if (need_check_key)
            pwron_case = app_poweron_wait_case();
        else
            pwron_case = APP_POWERON_CASE_NORMAL;

        if (pwron_case != APP_POWERON_CASE_INVALID && pwron_case != APP_POWERON_CASE_DITHERING) {
            TRACE(1, "power on case:%d\n", pwron_case);
            nRet = 0;
            bes_bt_sleep_init();
            switch (pwron_case) {
            case APP_POWERON_CASE_CALIB:
                break;
            case APP_POWERON_CASE_BOTHSCAN:
                break;
            case APP_POWERON_CASE_NORMAL:
                /* FALL THROUGH*/
            case APP_POWERON_CASE_REBOOT:
            case APP_POWERON_CASE_ALARM:
            default:
                break;
            }

            if (need_check_key)
                app_poweron_wait_finished();

            app_key_init();
        } else {
            app_key_close();
            nRet = -1;
        }
    }

exit:
    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_32K);
    return nRet;
}

int app_deinit(int deinit_case)
{
    int nRet = 0;
    TRACE(2, "%s case:%d", __func__, deinit_case);

#if defined(SENSOR_HUB) && !defined(__NuttX__)
    app_sensor_hub_deinit();
#endif
#ifdef DSP_M55
    app_dsp_m55_force_deinit();
#endif
#ifdef PC_CMD_UART_ID
    app_cmd_close();
#endif

    if (!deinit_case) {
        app_poweroff_flag = 1;
        bes_bt_me_acl_disc_all_bt_links(true);
        osDelay(200);
#ifndef RAM_NV_RECORD
        nv_record_flash_flush();
        norflash_api_flush_all(true);
#if defined(DUMP_LOG_ENABLE)
        log_dump_flush_all();
#endif
#endif
    }

    return nRet;
}

int app_init_btc(void)
{
    struct nvrecord_env_t *nvrecord_env;

    int nRet = 0;
    POSSIBLY_UNUSED uint8_t pwron_case = APP_POWERON_CASE_INVALID;

    TRACE(0, "please check all sections sizes and heads is correct ........");
    TRACE(2, "__coredump_section_start: %p length: 0x%x", __coredump_section_start, CORE_DUMP_SECTION_SIZE);
    TRACE(2, "__ota_upgrade_log_start: %p length: 0x%x", __ota_upgrade_log_start, OTA_UPGRADE_SECTION_SIZE);
    TRACE(2, "__log_dump_start: %p length: 0x%x", __log_dump_start, LOG_DUMP_SECTION_SIZE);
    TRACE(2, "__crash_dump_start: %p length: 0x%x", __crash_dump_start, CRASH_DUMP_SECTION_SIZE);
    TRACE(2, "__custom_parameter_start: %p length: 0x%x", __custom_parameter_start, CUSTOM_PARAMETER_SECTION_SIZE);
    TRACE(2, "__lhdc_license_start: %p length: 0x%x", __lhdc_license_start, LHDC_LICENSE_SECTION_SIZE);
    TRACE(2, "__userdata_start: %p length: 0x%x", __userdata_start, USERDATA_SECTION_SIZE * 2);
    TRACE(2, "__aud_start: %p length: 0x%x", __aud_start, AUD_SECTION_SIZE);
    TRACE(2, "__factory_start: %p length: 0x%x", __factory_start, FACTORY_SECTION_SIZE);
    TRACE(0, "app_init\n");

    nv_record_init();
    factory_section_init();

    nv_record_env_init();
    nvrec_dev_data_open();
    factory_section_open();
    nv_record_env_get(&nvrecord_env);

    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_104M);
    list_init();
    nRet = app_os_init();
    if (nRet)
        goto exit;

#ifdef FORCE_SIGNALINGMODE
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE | HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
#elif defined FORCE_NOSIGNALINGMODE
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE | HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE);
#endif

    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_REBOOT) {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
        pwron_case = APP_POWERON_CASE_REBOOT;
        TRACE(0, "Initiative REBOOT happens!!!");
    }

    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_MODE) {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MODE);
        pwron_case = APP_POWERON_CASE_TEST;
        TRACE(0, "To enter test mode!!!");
    }

    hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
    btdrv_start_bt();

    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_52M);
    if (pwron_case == APP_POWERON_CASE_TEST) {
        nRet = app_power_test_mode_handler();
    }

exit:
    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_32K);
    return nRet;
}

int app_deinit_btc(int deinit_case)
{
    int nRet = 0;
    TRACE(2, "%s case:%d", __func__, deinit_case);
    if (!deinit_case)
        app_poweroff_flag = 1;

    return nRet;
}

int app_shutdown(void)
{
    system_shutdown();
    return 0;
}

int app_reset(void)
{
    TRACE(0, "Retention bit when reset happens is 0x%x", hal_sw_bootmode_get());
    system_reset();
    return 0;
}
void app_enter_signalingtest_mode(void)
{
    TRACE(0, "APP: enter signaling test");
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE | HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
    app_reset();
}

void app_enter_non_signalingtest_mode(void)
{
    TRACE(0, "APP: enter non-signaling test");
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE | HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE);
    app_reset();
}

bool app_is_power_off_in_progress(void)
{
    return app_poweroff_flag ? TRUE : FALSE;
}

void app_trace_port_config(uint32_t bootmode)
{
#ifdef BTH_TRC_TO_SYS
    bth_trace_to_sys();
#endif

#ifdef DEBUG
#if (DEBUG_PORT == 1)
    hal_iomux_set_uart0();
    if (!(bootmode & HAL_SW_BOOTMODE_FACTORY)) {
        hal_trace_open(HAL_TRACE_TRANSPORT_UART0);
    }
#endif

#if (DEBUG_PORT == 2)
    if (!(bootmode & HAL_SW_BOOTMODE_FACTORY)) {
        //hal_iomux_set_analog_i2c();
    }
    hal_iomux_set_uart1();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART1);
#endif

#if (DEBUG_PORT == 3)
    if (!(bootmode & HAL_SW_BOOTMODE_FACTORY)) {
        //hal_iomux_set_analog_i2c();
    }
    hal_iomux_set_uart2();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART2);
#endif

    hal_sleep_start_stats(10000, 10000);
    hal_trace_set_log_level(TR_LEVEL_DEBUG);
#endif
}

WEAK void app_application_ready_to_start_callback(void)
{
}
