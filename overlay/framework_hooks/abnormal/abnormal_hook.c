/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "abnormal_manager_api.h"
#include "root_common.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "root_log_define.h"
#include "time_utils.h" // IWYU pragma: keep
#include "windows_manager_api.h" // IWYU pragma: keep
#include "power_manager_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define ABNORMAL_MONITOR_POLL_MS    5000 //5s
#define ABNORMAL_HOOK_DEBUG     1
/// 电池温度异常/恢复需连续确认次数（4 × 5s ≈ 20s）
#define BAT_TEMP_ABNORMAL_DEBOUNCE_CNT    4

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
/// 当前显示的异常类型（用于优先级管理）
static uint8_t s_current_displayed_abnormal = SYS_ABNORMAL_TYPE_MAX;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int abnormal_hook_get_highest_priority_abnormal(abnormal_manager_context_t *abnormal)
{
    EXCEPTION_CHECK_INT(abnormal);
    
    int highest_priority = -1;
    uint8_t highest_priority_type = SYS_ABNORMAL_TYPE_MAX;
    
    for (uint8_t i = 0; i < SYS_ABNORMAL_TYPE_MAX; i++) {
        if (abnormal->is_abnormal_active(i)) {
            uint8_t priority = abnormal->get_abnormal_severity(i);
            if (priority > highest_priority) {
                highest_priority = (int)priority;
                highest_priority_type = i;
            }
        }
    }
    
    return (int)highest_priority_type;
}

static void abnormal_dispatch_system_abnormal_window(void)
{
    window_manager_context_t *wm =
        (window_manager_context_t *)find_module(ID_MODULE_WM);
    if (wm == NULL) {
        return;
    }
    // 如果已经在系统异常窗口，不重复调度
    if (wm->is_current_window(wm, ID_WINDOW_SYSTEM_ABNORMAL)) {
        return;
    }
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_SYSTEM_ABNORMAL;
    wm->dispatch_new_window_args(wm, &msg);
}

/**
 * @brief 异常弹窗调度 — 根据优先级决定显示/切换/解除弹窗
 *
 * 决策流程：
 *  1. 无活动异常              → 解除当前弹窗，notify CLEARED
 *  2. 当前无弹窗              → 直接弹出 highest，notify ACTIVE
 *  3. current == highest     → 同优先级后出现替换先出现：若 triggered_type
 *                              是另一个活动的同优先级异常，切换显示
 *  4. current != highest     → new >= current 优先级才切换（低优先不覆盖高优）
 *
 * @param triggered_type  触发本次调度的异常类型，SYS_ABNORMAL_TYPE_MAX 表示非特定触发
 */
static void trigger_abnormal_popup(uint8_t triggered_type)
{
    abnormal_manager_context_t *abnormal =
        (abnormal_manager_context_t *)find_module(ID_MODULE_SYS_ABNORMAL);
    EXCEPTION_CHECK_VOID(abnormal);

    int highest_type = abnormal_hook_get_highest_priority_abnormal(abnormal);

    // 如果没有活动异常
    if (highest_type == (int)SYS_ABNORMAL_TYPE_MAX) {
        // 所有异常恢复，解除设备限制并恢复可运行状态
        if (s_current_displayed_abnormal != SYS_ABNORMAL_TYPE_MAX) {
            LOG_MISC("all abnormal cleared, restore device state");
            s_current_displayed_abnormal = SYS_ABNORMAL_TYPE_MAX;
            // 触发解除弹窗和设备恢复逻辑
            sys_abnormal_state_info_t info = {
                .type = SYS_ABNORMAL_TYPE_MAX,
                .state = SYS_ABNORMAL_STATE_CLEARED
            };
            abnormal->notify_abnormal_state_change(&info);
        }
        return;
    }

    // 如果当前没有显示任何弹窗，直接显示最高优先级异常
    if (s_current_displayed_abnormal == SYS_ABNORMAL_TYPE_MAX) {
        LOG_MISC("show abnormal popup, type=%u", (uint8_t)highest_type);
        s_current_displayed_abnormal = (uint8_t)highest_type;
        sys_abnormal_state_info_t info = {
            .type = (uint8_t)highest_type,
            .state = SYS_ABNORMAL_STATE_ACTIVE
        };
        abnormal->notify_abnormal_state_change(&info);
        abnormal_dispatch_system_abnormal_window();
        return;
    }

    // 如果当前显示的异常就是最高优先级异常，检查是否被同优先级新异常触发
    if (s_current_displayed_abnormal == (uint8_t)highest_type) {
        // 同优先级后出现替换先出现：如果触发源是另一个活动的同优先级异常，切换显示
        if (triggered_type != SYS_ABNORMAL_TYPE_MAX &&
            triggered_type != s_current_displayed_abnormal &&
            abnormal->is_abnormal_active(triggered_type)) {
            uint8_t current_priority = abnormal->get_abnormal_severity(s_current_displayed_abnormal);
            uint8_t triggered_priority = abnormal->get_abnormal_severity(triggered_type);
            if (triggered_priority == current_priority) {
                LOG_MISC("switch to same-priority abnormal, from type=%u to type=%u(sev=%u)",
                    s_current_displayed_abnormal, triggered_type, triggered_priority);
                s_current_displayed_abnormal = triggered_type;
                sys_abnormal_state_info_t info = {
                    .type = triggered_type,
                    .state = SYS_ABNORMAL_STATE_ACTIVE
                };
                abnormal->notify_abnormal_state_change(&info);
                abnormal_dispatch_system_abnormal_window();
            }
        }
        return;
    }

    // 如果有更高优先级或同优先级的异常出现，切换显示
    uint8_t current_priority = abnormal->get_abnormal_severity(s_current_displayed_abnormal);
    uint8_t new_priority = abnormal->get_abnormal_severity((uint8_t)highest_type);

    if (new_priority >= current_priority) {
        LOG_MISC("switch to abnormal, from type=%u(sev=%u) to type=%u(sev=%u)",
            s_current_displayed_abnormal, current_priority,
            (uint8_t)highest_type, new_priority);
        s_current_displayed_abnormal = (uint8_t)highest_type;
        sys_abnormal_state_info_t info = {
            .type = (uint8_t)highest_type,
            .state = SYS_ABNORMAL_STATE_ACTIVE
        };
        abnormal->notify_abnormal_state_change(&info);
        abnormal_dispatch_system_abnormal_window();
    }
    // 低优先级异常不覆盖高优先级异常，低优先级保留在后台检测和记录
}
static void check_flash_failure_abnormal(void)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_VOID(pbus);
    
    rt_device_node_t *flash_dev = (rt_device_node_t *)pbus->find_device(ID_DEVICE_FLASH);
    if (flash_dev) {
        bool flash_is_ready = true;
        DEVICE_IOCTL_ARGS_NO_RET(flash_dev, OP_CODE_FLASH_GET_FAIL_STATS, &flash_is_ready);

        if (!flash_is_ready) {
            LOGE("sys_abnormal_report_storage_failure");
            sys_abnormal_report_storage_failure(1);
        }
    }
}

static void check_peripheral_communication_abnormal(void)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_VOID(pbus);
    
    uint8_t comm_error = 0;
    int ret;
    
    // 检查气压传感器状态
    #if IS_SUPPORT_DEVICE_AIR_PRESSURE
    {
        uint8_t pressure_is_ready_status[AIR_PRESSURE_NUM] = {0};
        rt_device_node_t *air_pressure_dev = (rt_device_node_t *)pbus->find_device(ID_DEVICE_AIR_PRESSURE);
        if (air_pressure_dev) {
            DEVICE_IOCTL_ARGS_NO_RET(air_pressure_dev, OP_CODE_AIR_PRESSURE_IS_READY, pressure_is_ready_status);
            for (uint8_t i = 0; i < AIR_PRESSURE_NUM; i++) {
                if (pressure_is_ready_status[i] == 0) {
                    comm_error = 1;
                    LOGE("sys_abnormal_report_communication_erro  air");
                    break;
                }
            }
        }
    }
    #endif
    
    // 检查电池状态
    #if IS_SUPPORT_DEVICE_CHARGE
    {
        rt_device_node_t *battery_dev = (rt_device_node_t *)pbus->find_device(ID_DEVICE_BATTERY);
        if (battery_dev) {
            DEVICE_IOCTL_HAS_RET(battery_dev, OP_CODE_BAT_IS_READY, ret);
            if (ret <= 0) {
                comm_error = 1;
                LOGE("sys_abnormal_report_communication_erro  charge");
            }
        }
    }
    #endif
    
    // 检查触摸按键状态
    #if IS_SUPPORT_DEVICE_TOUCH_KEY
    {
        uint8_t touch_is_ready_status = 0;
        rt_device_node_t *input_key_dev = (rt_device_node_t *)pbus->find_device(ID_DEVICE_INPUT_KEY);
        if (input_key_dev) {
            DEVICE_IOCTL_ARGS_NO_RET(input_key_dev, OP_CODE_INPUT_TOUCH_IS_READY, &touch_is_ready_status);
            if (touch_is_ready_status == 0) {
                comm_error = 1;
                LOGE("sys_abnormal_report_communication_erro  key");
            }
        }
    }
    #endif
    
    // 检查LED状态
    #if IS_SUPPORT_DEVICE_LED
    {
        uint8_t led_is_ready_status = 0;
        rt_device_node_t *led_dev = (rt_device_node_t *)pbus->find_device(ID_DEVICE_LED);
        if (led_dev) {
            DEVICE_IOCTL_ARGS_NO_RET(led_dev, OP_CODE_LED_IS_READY, &led_is_ready_status);
            if (led_is_ready_status == 0) {
                comm_error = 1;
                LOGE("sys_abnormal_report_communication_erro  led");
            }
        }
    }
    #endif
    
    // 检查PD充电状态  pd如果检测异常会弹适配器异常界面，也暂时埋点不上报异常
    /*#if IS_SUPPORT_PD_CHARGE
    {
        rt_device_node_t *battery_dev = (rt_device_node_t *)pbus->find_device(ID_DEVICE_BATTERY);
        if (battery_dev) {
            DEVICE_IOCTL_HAS_RET(battery_dev, OP_CODE_BAT_PD_IS_READY, charge_pd_is_ready_status);
            if (charge_pd_is_ready_status == false) {
                comm_error = 1;
                LOGE("sys_abnormal_report_communication_erro charge pd");
            }
        }
    }
    #endif*/

    // 检查LCD状态
    #if IS_SUPPORT_DEVICE_LCD
    {
        int lcd_is_ready_status = 0;
        rt_device_node_t *lcd_dev = (rt_device_node_t *)pbus->find_device(ID_DEVICE_DISPLAY_PANEL);
        if (lcd_dev) {
            lcd_is_ready_status = lcd_dev->ioctl(lcd_dev, OP_CODE_PANEL_IS_READY, NULL);
            if (lcd_is_ready_status == 0) {
                comm_error = 1;
                LOGE("sys_abnormal_report_communication_error lcd");
            }
        }
    }
    #endif
    
    // 报告通信异常
    sys_abnormal_report_communication_error(comm_error);
    ///触发弹窗 永久
    if (comm_error) {
        LOGE("sys_abnormal_report_communication_error");
    }
   
}

static void check_battery_temperature_abnormal(abnormal_manager_context_t *abnormal)
{
    ///读取电池温度
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_VOID(pbus);
    struct rt_device_node *battery = (struct rt_device_node *)pbus->find_device(ID_DEVICE_BATTERY);
    EXCEPTION_CHECK_VOID(battery);

    DEVICE_IOCTL_HAS_RET(battery, OP_CODE_BAT_GET_CHARGE_TEMPERATURE, temperature);

    static uint8_t prev_temp_abnormal = 0;
    static uint8_t debounce_cnt = 0;
    uint8_t sample_temp_abnormal = prev_temp_abnormal;
    ///temperature = 70;//测试用
    if (temperature >= SYS_ABNORMAL_BAT_TEMP_HIGH_THRESHOLD_C) {
        sample_temp_abnormal = 1;
    } else if (temperature <= SYS_ABNORMAL_BAT_TEMP_NORMAL_THRESHOLD_C) {
        sample_temp_abnormal = 0;
    } else {
        /// 滞回区间内不累计，避免阈值附近抖动推进状态
        debounce_cnt = 0;
        return;
    }

    if (sample_temp_abnormal == prev_temp_abnormal) {
        debounce_cnt = 0;
        return;
    }

    debounce_cnt++;
    if (debounce_cnt < BAT_TEMP_ABNORMAL_DEBOUNCE_CNT) {
        LOG_MISC("battery temp debounce %u/%u: %d℃, pending=%u",
                 debounce_cnt, BAT_TEMP_ABNORMAL_DEBOUNCE_CNT,
                 temperature, sample_temp_abnormal);
        return;
    }
    debounce_cnt = 0;

    ///判断是否异常（连续确认后才触发）
    if (sample_temp_abnormal && !prev_temp_abnormal) {
        ///异常 - 连续超过高温阈值
        abnormal->set_abnormal_state(SYS_ABNORMAL_BATTERY_TEMP_HIGH, SYS_ABNORMAL_STATE_ACTIVE);
        ///弹窗
        LOGE("SYS_ABNORMAL_BATTERY_TEMP_HIGH: %d℃", temperature);
        ///填入埋点
        DATA_BURY_EVENT_TRIGGER_EXCEPTION(DATA_BURY_ERROR_BATTERY_TEMP_OUT_OF_RANGE, temperature);
        trigger_abnormal_popup(SYS_ABNORMAL_BATTERY_TEMP_HIGH);
    } else if (!sample_temp_abnormal && prev_temp_abnormal) {
        ///恢复正常 - 连续低于恢复阈值
        abnormal->set_abnormal_state(SYS_ABNORMAL_BATTERY_TEMP_HIGH, SYS_ABNORMAL_STATE_CLEARED);
        ///解除弹窗
        LOG_MISC("battery temperature normal: %d℃", temperature);
        trigger_abnormal_popup(SYS_ABNORMAL_BATTERY_TEMP_HIGH);
    }

    prev_temp_abnormal = sample_temp_abnormal;
}

static void abnormal_monitor_thread(void *user_data)
{
    abnormal_manager_context_t *abnormal = (abnormal_manager_context_t *)user_data;

    EXCEPTION_CHECK_VOID(abnormal);
    ///如果在产测窗口，则不检查异常
    window_manager_context_t *wm =
        (window_manager_context_t *)find_module(ID_MODULE_WM);
    if (wm && wm->is_current_window(wm, ID_WINDOW_PRODUCT_TEST_MODE)) {
        return;
    }
    ///如果系统在suspend状态，则不检查异常
    power_manager_context_t *pm =
        (power_manager_context_t *)find_module(ID_MODULE_POWER);
    if (pm && pm->is_pm_suspend()) {
        return;
    }
    
    /// 检查电池温度异常
    check_battery_temperature_abnormal(abnormal);
    
    /// 检查外设通信异常
    check_peripheral_communication_abnormal();
    
    /// 检查Flash存储失败次数异常
    check_flash_failure_abnormal();
    #if ABNORMAL_HOOK_DEBUG
    uint32_t rtc_timestamp = platform_get_utc_time();
    utc_time_t utc_time = {0};
    timestamp_to_utc_time(rtc_timestamp, &utc_time);
    LOG_MISC("rtc_time: %u, %04u-%02u-%02u %02u:%02u:%02u, "
         "abnormal: bat_temp=%u storage=%u comm=%u displayed=%u forbidden=%u locked=%u",
         (unsigned int)rtc_timestamp, utc_time.year + 2000, utc_time.month, utc_time.day,
         utc_time.hour, utc_time.minute, utc_time.second,
         abnormal->is_abnormal_active(SYS_ABNORMAL_BATTERY_TEMP_HIGH),
         abnormal->is_abnormal_active(SYS_ABNORMAL_STORAGE_FAILURE),
         abnormal->is_abnormal_active(SYS_ABNORMAL_COMMUNICATION_ERROR),
         s_current_displayed_abnormal,
         abnormal->is_new_task_forbidden(),
         abnormal->is_core_locked());
    #endif
}

static int on_post_abnormal_init_route(abnormal_manager_context_t *ctx)
{
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);

    EXCEPTION_CHECK_INT(ctx);
    if (misc) {
        misc->add_misc_task(ctx, MISC_TASK_ID_SYS_ABNORMAL_POLLING, ABNORMAL_MONITOR_POLL_MS,
            MISC_WORK_MSG_ID_TIMER, abnormal_monitor_thread);
    }
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_abnormal_hook(abnormal_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);
    ctx->hook = u_malloz(sizeof(struct __abnormal_manager_hook));
    ctx->hook->on_post_init_route = on_post_abnormal_init_route;
}

void sys_abnormal_report_storage_failure(uint8_t active)
{
    abnormal_manager_context_t *abnormal =
        (abnormal_manager_context_t *)find_module(ID_MODULE_SYS_ABNORMAL);

    if (!abnormal) {
        return;
    }

    uint8_t was_active = abnormal->is_abnormal_active(SYS_ABNORMAL_STORAGE_FAILURE);
    if (was_active == active) {
        return;
    }

    abnormal->set_abnormal_state(SYS_ABNORMAL_STORAGE_FAILURE,
        active ? SYS_ABNORMAL_STATE_ACTIVE : SYS_ABNORMAL_STATE_CLEARED);
    if (active) {
        ///填入埋点
        DATA_BURY_EVENT_TRIGGER_EXCEPTION(DATA_BURY_ERROR_STORAGE_FAILURE, 0X00);
    }
    trigger_abnormal_popup(SYS_ABNORMAL_STORAGE_FAILURE);
}

void sys_abnormal_report_communication_error(uint8_t active)
{
    abnormal_manager_context_t *abnormal =
        (abnormal_manager_context_t *)find_module(ID_MODULE_SYS_ABNORMAL);

    if (!abnormal) {
        return;
    }

    uint8_t was_active = abnormal->is_abnormal_active(SYS_ABNORMAL_COMMUNICATION_ERROR);
    if (was_active == active) {
        return;
    }

    abnormal->set_abnormal_state(SYS_ABNORMAL_COMMUNICATION_ERROR,
        active ? SYS_ABNORMAL_STATE_ACTIVE : SYS_ABNORMAL_STATE_CLEARED);
    if (active) {
        ///填入埋点
        DATA_BURY_EVENT_TRIGGER_EXCEPTION(DATA_BURY_ERROR_COMMUNICATION, 0X00);
    }
    trigger_abnormal_popup(SYS_ABNORMAL_COMMUNICATION_ERROR);
}
