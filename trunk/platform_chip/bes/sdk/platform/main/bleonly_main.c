#include "cmsis.h"
#include "cmsis_os.h"
#include "cmsis_nvic.h"
#include "analog.h"
#include "pmu.h"
#include "mpu_cfg.h"
#include "hwtimer_list.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_bootmode.h"
#include "hal_uart.h"
#include "tool_msg.h"
#include "bt_drv_interface.h"
#include "norflash_api.h"
#include "nvrecord_appmode.h"
#include "nvrecord_bt.h"
#include "nvrecord_dev.h"
#include "nvrecord_env.h"
#include "factory_section.h"
#include "bluetooth_ble_api.h"
#include "app_custom.h"
#include "app_ble.h"

#if defined(BLE_ONLY_LT)
#include "cqueue.h"
#ifdef TEST_HAL_PATTERN
#include "test_hal_main.h"
#endif

#ifdef BES_OTA
#include "ota_bes.h"
#include "ota_control.h"
#include "ota_config.h"
#endif

#ifdef OTA_ENABLE
#include "ota_basic.h"
#endif
#endif

#include "watchdog/watchdog.h"

#define STACK_READY_BT  0x01
#define STACK_READY_BLE 0x02

static osThreadId main_thread_tid = NULL;

#if defined(BLE_ONLY_LT)
#ifdef FIRMWARE_REV
#define SYS_STORE_FW_VER(x) \
      if(fw_rev_##x) { \
        *fw_rev_##x = fw.softwareRevByte##x; \
      }

typedef struct
{
    uint8_t softwareRevByte0;
    uint8_t softwareRevByte1;
    uint8_t softwareRevByte2;
    uint8_t softwareRevByte3;
} FIRMWARE_REV_INFO_T;

static const FIRMWARE_REV_INFO_T fwRevInfoInFlash = {0, 0, 1, 0};
FIRMWARE_REV_INFO_T fwRevInfoInRam;

void system_get_info(uint8_t *fw_rev_0, uint8_t *fw_rev_1, uint8_t *fw_rev_2, uint8_t *fw_rev_3)
{
  FIRMWARE_REV_INFO_T fw = fwRevInfoInFlash;

  SYS_STORE_FW_VER(0);
  SYS_STORE_FW_VER(1);
  SYS_STORE_FW_VER(2);
  SYS_STORE_FW_VER(3);
}
#endif

int system_reset(void)
{
    osThreadSetPriority(main_thread_tid, osPriorityRealtime);
    osThreadFlagsSet(main_thread_tid, 0x8);
    return 0;
}

int app_reset(void)
{
    TRACE(0, "Retention bit when reset happens is 0x%x", hal_sw_bootmode_get());
    system_reset();
    return 0;
}

static void app_postponed_reset_timer_handler(void const *param);
osTimerDef(APP_POSTPONED_RESET_TIMER, app_postponed_reset_timer_handler);
static osTimerId app_postponed_reset_timer = NULL;
#define APP_RESET_PONTPONED_TIME_IN_MS  2000
static void app_postponed_reset_timer_handler(void const *param)
{
    app_reset();
}

void app_start_postponed_reset(void)
{
    if (NULL == app_postponed_reset_timer)
    {
        app_postponed_reset_timer = osTimerCreate(osTimer(APP_POSTPONED_RESET_TIMER), osTimerOnce, NULL);
    }

#ifndef FLASH_REMAP
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
#endif
    osTimerStart(app_postponed_reset_timer, APP_RESET_PONTPONED_TIME_IN_MS);
}
#else
#define BOOT_MAGIC_NUMBER               0xBE57EC1C
#define BOOT_WORD_A                     0xAAAAAAAA
#define BOOT_WORD_B                     0xBBBBBBBB

typedef struct
{
    uint32_t magicNumber; // NORMAL_BOOT or COPY_NEW_IMAGE
    uint32_t imageSize;
    uint32_t imageCrc;
    uint32_t newImageFlashOffset;
    uint32_t boot_word;
} FLASH_OTA_BOOT_INFO_T;

typedef void (*FLASH_ENTRY)(void);
FLASH_OTA_BOOT_INFO_T otaBootInfoInFlash __attribute((section(".ota_boot_info"))) = {
    BOOT_MAGIC_NUMBER, 0, 0, OTA_CODE_OFFSET, BOOT_WORD_A
};

static const struct boot_struct_t *boot_struct = (struct boot_struct_t *)(FLASH_BASE + OTA_CODE_OFFSET);

static bool app_is_ble_mode(void)
{
    uint32_t bootmode = hal_sw_bootmode_get();

    return (bootmode & HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
}

void SelectEnterApp(void)
{
	FLASH_ENTRY entry;

    if (app_is_ble_mode()) {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
        return;
    }

    if (BOOT_MAGIC_NUMBER == boot_struct->hdr.magic)
    {
        NVIC_DisableAllIRQs();
        entry = (FLASH_ENTRY)(&boot_struct->hdr + 1);
        entry = (FLASH_ENTRY)((uint32_t)entry | 1);
        entry();
    }
}
void boot_loader_pre_init_hook(void) __attribute__((alias("SelectEnterApp")));
#endif

int signal_send_to_main_thread(uint32_t signals)
{
    osThreadFlagsSet(main_thread_tid, signals);
    return 0;
}

bool app_is_power_off_in_progress(void)
{
    return false;
}

static uint8_t stack_ready_flag = 0;
void app_notify_stack_ready(uint8_t ready_flag)
{
    TRACE(2,"app_notify_stack_ready %d %d", stack_ready_flag, ready_flag);

    stack_ready_flag |= ready_flag;

#ifdef __IAG_BLE_INCLUDE__
    if(stack_ready_flag == (STACK_READY_BT|STACK_READY_BLE))
#endif
    {
        TRACE(0, "signal_send_to_main_thread %d", STACK_READY_BLE | STACK_READY_BT);
        signal_send_to_main_thread(STACK_READY_BLE | STACK_READY_BT);
    }
}

static void app_stack_ready_cb(void)
{
    TRACE(0,"stack init done");
}

static void POSSIBLY_UNUSED app_wait_stack_ready(void)
{
    uint32_t stime, etime;
    stime = hal_sys_timer_get();
    osSignalWait(0x3, osWaitForever);
    etime = hal_sys_timer_get();
    TRACE(1,"app_wait_stack_ready: wait:%d ms", TICKS_TO_MS(etime - stime));

    app_stack_ready_cb();
}

static void init_bt_thread(const void *arg);
osThreadDef(init_bt_thread, osPriorityHigh, 1, (2048), "init_bt_thread");
static osThreadId init_bt_thread_id;

static void init_bt_thread(const void *arg)
{
    btdrv_start_bt();

    bes_bt_thread_init();

    osThreadExit();
}

static void app_start_bt_module_thread(void)
{
    init_bt_thread_id =
        osThreadCreate(osThread(init_bt_thread), NULL);

    osThreadId app_thread_id = osThreadGetId();

    osThreadSetPriority(app_thread_id, osPriorityAboveNormal);
}

static void app_earbud_mode_init()
{
    bes_ble_gap_stub_user_init();
}

static void app_default_mode_init(nvrec_appmode_e mode)
{
    bes_ble_app_init(mode);

    app_earbud_mode_init();
}

int app_bluetooth_application_init(void)
{
    nvrec_appmode_e mode = nv_record_appmode_get();

    TRACE(0, "%s: mode %d", __func__, mode);

    switch (mode)
    {
        case NV_APP_EARBUDS_MONO:
        case NV_APP_EARBUDS_STEREO_ONE_CIS:
        case NV_APP_EARBUDS_STEREO_TWO_CIS:
        case NV_APP_HEADSET_STEREO_ONE_CIS:
        case NV_APP_HEADSET_STEREO_TWO_CIS:
            app_default_mode_init(mode);
            break;
        case NV_APP_WALKIE_TALKIE:
            break;
        default:
            break;
    }

    bes_ble_gap_ready_and_init_done(mode);

    return 0;
}


/***************************************************************/
#include "hal_sleep.h"
#include "intersyshci.h"
#include "bt_drv_reg_op.h"
#define APP_FACT_CPU_WAKE_LOCK              HAL_CPU_WAKE_LOCK_USER_3

enum TEST_MODE_TIMER_TYEP_T{
   BT_NONSIGNALING_TX_TYPE = 0,
   BT_NONSIGNALING_RX_TYPE,
   LE_TEST_TX_TYPE,
   LE_TEST_RX_TYPE,
   TEST_ERROR_TYPE,
};

static uint8_t test_end_timer_argument = TEST_ERROR_TYPE;

static void app_bt_nonsignalingtest_end_timer_handler(void const *param);
osTimerDef(bt_nonsignalingtest_end_timer, app_bt_nonsignalingtest_end_timer_handler);
static osTimerId bt_nonsignalingtest_end_timer_id = NULL;


static void bt_error_check_timer_handler(void const *param);
osTimerDef(bt_error_check_timer, bt_error_check_timer_handler);
static osTimerId bt_error_check_timer_id = NULL;
uint8_t test_mode_type=0;

static void bt_error_check_timer_handler(void const *param)
{
    //dump rssi
    bt_drv_rssi_dump_handler();
    //adjust parameter for testmode
    bt_drv_testmode_adaptive_adjust_param();

    //check BT core status
    if(bt_drv_error_check_handler())
    {
        if(test_mode_type==1)
        {
            hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE|HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
        }
        else if(test_mode_type==2)
        {
            hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE|HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE);
        }
        pmu_reboot();
    }
}

static void app_bt_nonsignalingtest_end_timer_handler(void const *param)
{
    uint8_t timer_type_param = *((uint8_t *)param);
    if(timer_type_param == LE_TEST_RX_TYPE)
    {
        btdrv_le_test_end();
    }
    else if(timer_type_param == BT_NONSIGNALING_RX_TYPE)
    {
        btdrv_nosig_stop_rx_test_set();
    }
    else
    {
        hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
        nv_record_flash_flush();
        pmu_reboot();
    }

    test_end_timer_argument = TEST_ERROR_TYPE;
}

static const uint8_t test_instrument_addr[6] = {0x51, 0x33, 0x33, 0x22, 0x11, 0x11};

const uint8_t* app_bt_factorty_get_test_instrument_addr(void)
{
    return test_instrument_addr;
}

static void app_bt_start_nonsignalingtest_end_timer(uint32_t timeout, uint8_t timer_type)
{
    TRACE(1,"app factory: timeout =%d", timeout);

    do {
        if(timeout < 2000)
        {
            break;
        }

        test_end_timer_argument = timer_type;
        if (!bt_nonsignalingtest_end_timer_id){
            bt_nonsignalingtest_end_timer_id = osTimerCreate(osTimer(bt_nonsignalingtest_end_timer),
                osTimerOnce, (void *)(&test_end_timer_argument));
        }

        if (bt_nonsignalingtest_end_timer_id != NULL) {
            osTimerStart(bt_nonsignalingtest_end_timer_id, timeout);
        }
    } while(0);
}

static bool app_bt_nonsignaling_test_end_hander(uint8_t status, uint16_t nb_packet_received, uint16_t hec_nb, uint16_t crc_nb)
{
    do {
        if(status == 0 && nb_packet_received == 0 && hec_nb == 0xffff && crc_nb == 0xffff)
        {
           break;
        }
        TRACE(0, "APP factory:BT non-singnalint test end, status=0x%x, packet_received =%d,hec error =%d, crc error=%d",
            status, nb_packet_received, hec_nb, crc_nb);

        nv_record_update_factory_bt_nonsignaling_test_result((status == NONSIGNALING_TEST_OK), nb_packet_received, hec_nb, crc_nb);

        hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
        nv_record_flash_flush();
        osDelay(500);
        pmu_reboot();
    } while(0);


    return true;
}

static bool app_le_test_end_hander(uint8_t status, uint16_t nb_packet_received)
{
    TRACE(0, "APP factory:LE test end, status=%d, packet_received =%d", status, nb_packet_received);
    nv_record_update_factory_le_rx_test_result((status == 0), nb_packet_received);

    //if you want to reconnect mobile,|HAL_SW_BOOTMODE_TEST_NORMAL_MODE
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
    nv_record_flash_flush();
    osDelay(500);
    pmu_reboot();

    return true;
}

void app_bt_factory_bt_nonsign_rx_test_ind(uint32_t test_end_timeout, struct nvrecord_env_t *nvrecord_env)
{
    app_bt_start_nonsignalingtest_end_timer(test_end_timeout, BT_NONSIGNALING_RX_TYPE);
    btdrv_regist_bt_nonsignalint_test_end_callback(app_bt_nonsignaling_test_end_hander);
    if(nvrecord_env != NULL)
    {

        bt_nonsignaling_test_t* param = &nvrecord_env->bt_nonsignaling_env;
        btdrv_nosig_rx_test_set(param->hopping_mode, param->whitening_mode, param->tx_freq,
            param->rx_freq, param->power_level, app_bt_factorty_get_test_instrument_addr(), param->lt_addr, param->edr_enabled,
            param->packet_type, param->payload_pattern, param->payload_length);
    }
}

void app_bt_factory_bt_nonsign_tx_test_ind(uint32_t test_end_timeout, struct nvrecord_env_t *nvrecord_env)
{
    app_bt_start_nonsignalingtest_end_timer(test_end_timeout, BT_NONSIGNALING_TX_TYPE);
    if(nvrecord_env != NULL)
    {
        uint8_t local_addr[6] = {0x55,0x55,0x55,0x55,0x55,0x55};
        const uint8_t* factory_addr = app_bt_factorty_get_test_instrument_addr();
        if(factory_addr != NULL)
        {
            memcpy(local_addr, factory_addr, 6);
        }

        bt_nonsignaling_test_t* param = &nvrecord_env->bt_nonsignaling_env;
        btdrv_nosig_tx_test_set(param->hopping_mode, param->whitening_mode, param->tx_freq,
            param->rx_freq, param->power_level, local_addr, param->lt_addr, param->edr_enabled,
            param->packet_type, param->payload_pattern, param->payload_length, param->tx_packet_num);
    }
}

void app_factory_tota_nonsignalingtest_handler(void)
{
    uint8_t test_type = NV_BT_ERROR_TEST_MODE;
    uint32_t test_end_timeout = 0;
    //fetch test mode in NV
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);

    if(nvrecord_env->factory_test_env.test_type != NV_BT_ERROR_TEST_MODE)
    {
        test_type = nvrecord_env->factory_test_env.test_type;
        test_end_timeout = nvrecord_env->factory_test_env.test_end_timeout;
    }

    switch (test_type)
    {
        case NV_BT_NONSIGNALING_MODE:
            break;

        case NV_BT_NONSIGNALING_RX_MODE:
            app_bt_factory_bt_nonsign_rx_test_ind(test_end_timeout, nvrecord_env);
            break;

        case NV_BT_NONSIGNALING_TX_MODE:
            app_bt_factory_bt_nonsign_tx_test_ind(test_end_timeout, nvrecord_env);
            break;

        case NV_LE_NONSIGNALING_TX_MODE:
            app_bt_start_nonsignalingtest_end_timer(test_end_timeout, LE_TEST_TX_TYPE);
        //fall through
        case NV_LE_NONSIGNALING_CONT_TX_MODE:
            btdrv_le_tx_test_v2(nvrecord_env->le_tx_env.tx_channel, nvrecord_env->le_tx_env.data_len,
                nvrecord_env->le_tx_env.pkt_payload, nvrecord_env->le_tx_env.phy);
            break;

        case NV_LE_NONSIGNALING_RX_MODE:
            app_bt_start_nonsignalingtest_end_timer(test_end_timeout, LE_TEST_RX_TYPE);
            btdrv_regist_le_teset_end_callback(app_le_test_end_hander);
        //fall through
        case NV_LE_NONSIGNALING_CONT_RX_MODE:
            btdrv_le_rx_test_v2(nvrecord_env->le_rx_env.rx_channel, nvrecord_env->le_rx_env.phy,
                nvrecord_env->le_rx_env.mod_idx);
            break;

        default:
            TRACE(1,"app fatorty:error type, test type=0x%x", test_type);
            break;
    }

    //clear the test mode in nv
    nv_record_reset_factory_test_env();
}

void app_factorymode_bt_signalingtest_handler(void)
{
    dev_addr_name devinfo;
    TRACE(1,"%s",__func__);
#ifdef __WATCHER_DOG_RESET__
    app_wdt_close();
#endif
    hal_cpu_wake_lock(APP_FACT_CPU_WAKE_LOCK);
    if (!bt_error_check_timer_id){
        bt_error_check_timer_id = osTimerCreate(osTimer(bt_error_check_timer), osTimerPeriodic, NULL);
    }
    if (bt_error_check_timer_id != NULL) {
        osTimerStart(bt_error_check_timer_id, 1000);
    }
    test_mode_type = 1;
    pmu_sleep_en(0);
    BESHCI_Close();
    btdrv_hciopen();
    btdrv_ins_patch_test_init();
    btdrv_hci_reset();

    btdrv_sleep_config(0);
    osDelay(2000);
    btdrv_testmode_start();
    btdrv_feature_default();
    devinfo.btd_addr = bt_get_local_address();
    devinfo.ble_addr = bt_get_ble_local_address();
    devinfo.localname = bt_get_local_name();
    devinfo.ble_name= bt_get_local_name();
    nvrec_dev_localname_addr_init(&devinfo);
    btdrv_write_localinfo((char *)devinfo.localname, strlen(devinfo.localname) + 1, devinfo.btd_addr);
    bt_drv_config_after_hci_reset();
    btdrv_enable_dut();
}

void app_factorymode_bt_signalingtest(APP_KEY_STATUS *status, void *param)
{
    app_factorymode_bt_signalingtest_handler();
}

void app_factorymode_bt_nosignalingtest_handler(void)
{
    dev_addr_name devinfo;
    TRACE(1,"%s",__func__);
#ifdef __WATCHER_DOG_RESET__
    app_wdt_close();
#endif
    hal_cpu_wake_lock(APP_FACT_CPU_WAKE_LOCK);
    if (!bt_error_check_timer_id){
        bt_error_check_timer_id = osTimerCreate(osTimer(bt_error_check_timer), osTimerPeriodic, NULL);
    }
    if (bt_error_check_timer_id != NULL) {
        osTimerStart(bt_error_check_timer_id, 1000);
    }
    test_mode_type = 2;
    pmu_sleep_en(0);
    BESHCI_Close();
    btdrv_hciopen();
    btdrv_ins_patch_test_init();
    bt_drv_reg_op_key_gen_after_reset(false);
    btdrv_hci_reset();
    btdrv_sleep_config(0);
    btdrv_testmode_start();
    btdrv_feature_default();
    devinfo.btd_addr = bt_get_local_address();
    devinfo.ble_addr = bt_get_ble_local_address();
    devinfo.localname = bt_get_local_name();
    devinfo.ble_name= bt_get_local_name();
    nvrec_dev_localname_addr_init(&devinfo);
    btdrv_write_localinfo((char *)devinfo.localname, strlen(devinfo.localname) + 1, devinfo.btd_addr);
    bt_drv_config_after_hci_reset();
    app_factory_tota_nonsignalingtest_handler();

    btdrv_hcioff();
    btdrv_uart_bridge_loop();
}

void app_factorymode_bt_nosignalingtest(APP_KEY_STATUS *status, void *param)
{
    app_factorymode_bt_nosignalingtest_handler();
}

void app_enter_signalingtest_mode(void)
{
    TRACE(0, "APP: enter signaling test");
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE|HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
    pmu_reboot();
}

void app_enter_non_signalingtest_mode(void)
{
    TRACE(0, "APP: enter non-signaling test");
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE|HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE);
    pmu_reboot();
}
/***************************************************************/
#if 0
static int key_event_process(uint32_t key_code, uint8_t key_event)
{
    TRACE(2,"%s: key_code:%04x key_event:%d", __func__, key_code, key_event);

    if (key_event == HAL_KEY_EVENT_CLICK) {
        TRACE(0, "powerkey click!");
        hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
        app_enter_signalingtest_mode();
    } else if (key_event == HAL_KEY_EVENT_DOUBLECLICK) {
        TRACE(0, "powerkey double click! Enter default mode");
        // pmu_reboot();
        signal_send_to_main_thread(0x08);
    } else if (key_event == HAL_KEY_EVENT_LONGPRESS) {
        TRACE(0, "powerkey longpress!");
        hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
        app_enter_non_signalingtest_mode();
    }

    return 0;
}
#endif

static void app_ble_stub_user_data_fill_handler(void *param)
{
    TRACE(0, "%s", __func__);

    app_ble_data_fill_enable(USER_STUB, true);
}

void ble_app_init(void)
{
    TRACE(0,"ble_app_init\n");

    bes_bt_app_init();
    app_start_bt_module_thread();

#if defined(BES_OTA)
    bes_ota_init_handler();
#endif

    app_wait_stack_ready();

    bes_bt_call_custom_function_in_bt_thread((uint32_t)0, 0, (uint32_t)app_bluetooth_application_init);

#if defined(OTA_ENABLE)
    ota_basic_env_init();
#endif

#ifdef BES_OTA
#if !defined(OTA_OVER_TOTA_ENABLED)
    bes_ota_init();
#endif
#endif

    TRACE(1,"\n\n\nbt_stack_init_done:\n\n\n");
    bes_bt_sleep_init();
}

void ble_app_deinit(void)
{
    TRACE(0,"ble_app_deinit\n");
}

void app_ble_enable_and_start_adv(void);
void ble_start_adv(void)
{
    bes_ble_gap_register_data_fill_handle(USER_STUB, app_ble_stub_user_data_fill_handler, false);
    app_ble_enable_and_start_adv();
    app_ble_start_connectable_adv(500);
}

#if CFG_CUSTOM_APP
extern void platform_log_init(void);
extern void root_main(int argc, const char *argv[]);
#endif
int main(void)
{
    app_wdt_open(15);
    app_wdt_hung_task_check_enable(true);

    hwtimer_init();
    hal_sys_timer_open();
    hal_audma_open();
    hal_gpdma_open();

    // hal_key_open(0, key_event_process);

#if (DEBUG_PORT == 1)
    hal_iomux_set_uart0();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART0);
#endif
#if (DEBUG_PORT == 2)
    //hal_iomux_set_analog_i2c();
    hal_iomux_set_uart1();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART1);
#endif

#ifdef TRACE_LVL_DEBUG
    hal_trace_set_log_level(LOG_LEVEL_DEBUG);
#endif

#if CFG_CUSTOM_APP
    pmu_power_key_hw_reset_enable(10);
    platform_log_init();
#else
    hal_trace_set_log_level(TR_LEVEL_DEBUG);
    TRACE(0, "-->-->no custom app log level: debug");
#endif

    hal_norflash_show_id_state(HAL_FLASH_ID_0, true);

    hal_iomux_ispi_access_init();

    mpu_cfg();

    pmu_open();
    analog_open();

    TRACE(0,"\n\nble mode enter\n\n");

#ifdef USE_NORFLASH_API
    enum NORFLASH_API_RET_T result;
    result = norflash_api_init();
    if (result != NORFLASH_API_OK) {
        TRACE(0, "norflash_api_init fail ret=%d", result);
    }
#endif

    factory_section_open();
    nv_record_init();
    factory_section_init();
    nv_record_env_init();
    nvrec_dev_data_open();

#ifdef TEST_HAL_PATTERN
    test_hal_pattern();
#endif

    // hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    TR_INFO(TR_MOD(MAIN), "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));
    hal_sleep_start_stats(10000, 10000);

    main_thread_tid = osThreadGetId();
    TRACE(1,"main_thread_tid=%x", (uint32_t)main_thread_tid);

    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_MODE) {
        TRACE(0, "-->-->bootmode test mode!!!");
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MODE);
        btdrv_start_bt();

#ifdef __WATCHER_DOG_RESET__
        app_wdt_close();
#endif
        // app_factorymode_key_init();
        if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_SIGNALINGMODE){
            hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MASK);
            app_factorymode_bt_signalingtest(NULL, NULL);
        }
        if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE){
            hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MASK);
            app_factorymode_bt_nosignalingtest(NULL, NULL);
        }
    } else {
        ble_app_init();
        ble_start_adv();
#if CFG_CUSTOM_APP
        root_main(0, NULL);
#else
        TRACE(0, "no custom app enter");
#endif
    }
 
    uint32_t flags = 0;
    while (1) {
        flags = osThreadFlagsWait(CMSIS_V1_SIGNAL_ANY, osFlagsWaitAny, 5000);
        if ((flags > 0U) && (flags < 0x80000000U)) {
            if (flags & 0x08) {
                break;
            }
        }
    #if CFG_CUSTOM_APP
    #else
        TRACE(0, "-->-->no custom app loop");
    #endif
    }

    ble_app_deinit();
    pmu_reboot();
}