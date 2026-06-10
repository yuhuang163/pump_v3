#include "root_common_api.h" // IWYU pragma: keep
#include "app_protocol_handler.h"
#include "app_protocol_pump_ctrl_server.h"
#include "app_protocol_broadcast_to_window.h"
#include "app_protocol_key_simulation.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_CTRL_TASK_DELAY_TIME 500
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
/**
 * @brief 静态变量用于延迟任务的数据传递
 * @note 使用静态变量避免内存泄漏，但需要注意：
 *       1. 如果同名任务被重复投递，旧任务会被删除，新任务使用更新后的数据
 *       2. 如果删除操作失败，旧任务可能使用新数据（概率很低）
 *       3. 这些变量在整个程序生命周期存在，但任务删除后不会使用
 */
static app_old_comm_pump_ctrl pump_ctrl_data_temp = {0};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static bool app_old_comm_check_custom_payload_len(uint8_t cal, uint16_t offset, uint8_t cid, uint8_t *cab)
{
    if ((uint16_t)offset + 5 >= cal) {
        LOG_SRV("custom mode payload too short, offset:%d, len:%d", offset, cal);
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NACK, cid, 4, cab);
        return false;
    }
    return true;
}

static bool app_old_comm_check_custom_policy_capacity(uint8_t step_index, uint8_t need_count, uint8_t cid, uint8_t *cab)
{
    if ((uint16_t)step_index + need_count > ROOT_BREAST_PUMP_STEP_MAX) {
        LOG_SRV("custom mode policy overflow, step:%d, need:%d, max:%d",
            step_index, need_count, ROOT_BREAST_PUMP_STEP_MAX);
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NACK, cid, 4, cab);
        return false;
    }
    return true;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

static uint8_t app_old_comm_get_adaptive_dimming_switch(void)
{
#if IS_SUPPORT_ADAPTIVE_DIMMING
//     return (uint8_t)adaptive_dimming_global_switch_get();
// #else
    return 0;
#endif
}

static void app_pump_safe_start_key_sim_task(void)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_VOID(wm);
    /* Only simulate key when pump window is active to avoid stale UI context access. */
    if (wm->is_current_window(wm, ID_WINDOW_PUMP)) {
        app_start_key_simulation();
    }
}

static uint8_t app_old_comm_get_night_light_level(void)
{
#if IS_SUPPORT_NIGHT_LIGHT
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_UINT(led && led->hook && led->hook->get_night_light_level);
    return led->hook->get_night_light_level(led);
#else
    return 0;
#endif
}

static uint8_t app_old_comm_get_night_light_onoff(void)
{
#if IS_SUPPORT_NIGHT_LIGHT
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_UINT(led && led->hook && led->hook->get_night_light_onoff);
    return led->hook->get_night_light_onoff(led);
#else
    return 0;
#endif
}

static uint8_t app_old_comm_get_screen_key_light_level(void)
{
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    //TODO:
    //EXCEPTION_CHECK_UINT(led && led->hook && led->hook->get_key_backlight_level);
    return 10; //TODO:
}

static uint8_t app_old_comm_get_sys_status(servers_old_protocol_publish_data_id_e id, void *user_data)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_UINT(pbus);
    struct rt_device_node *battery = pbus->find_device(ID_DEVICE_BATTERY);
    uint8_t chage_state = 0;
    if (battery) {
        chage_state = DEVICE_IOCTL(battery, OP_CODE_BAT_VBUS_IS_PLUG_IN);
    } else {
        chage_state = CHARGE_STATE_UNCHARGE;
    }

    int pump_status = 0;
    READ_PUMP_WORK_STATUS(pump_ctx, pump_status);
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(pump_ctx, pump_mode);
    uint8_t sys_status = APP_OLD_COMM_SYS_FINISH;
    motor_manager_context_t *motor_ctx = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    EXCEPTION_CHECK_UINT(motor_ctx);

    /// 强制上报系统状态复位
    if (id == OLD_PUBLISH_DATA_SYSTEM_STATUS_RESET) {
        LOG_SRV("APP_OLD_COMM_SYS_RESET");
        return APP_OLD_COMM_SYS_RESET;
    }

    /// 强制上报泵状态结束
    if (id == OLD_PUBLISH_DATA_PUMP_STATUS_FINISH || motor_ctx->get_timed_policy_finish_flag() == 1) {
        USB_IS_PLUGIN(chage_state) ? sys_status = APP_OLD_COMM_PUMP_FINISH_IN_CHARGE : APP_OLD_COMM_SYS_FINISH;
       if (sys_status == APP_OLD_COMM_PUMP_FINISH_IN_CHARGE) {
            LOG_SRV("APP_OLD_COMM_PUMP_FINISH_IN_CHARGE");
       } else {
            LOG_SRV("APP_OLD_COMM_SYS_FINISH");
       } 
        return sys_status;
    }

    if (USB_IS_PLUGIN(chage_state)) {
        LOG_SRV("APP_OLD_COMM_SYS_CHANRGE");
        sys_status = APP_OLD_COMM_SYS_CHANRGE;
    }

    if (BREAST_PUMP_START == pump_status || BREAST_PUMP_CONTINUE == pump_status) {
        LOG_SRV("APP_OLD_COMM_SYS_RUN");
        sys_status = APP_OLD_COMM_SYS_RUN;
    }

    if ((BREAST_PUMP_START == pump_status || BREAST_PUMP_CONTINUE == pump_status)
        && USB_IS_PLUGIN(chage_state)) {
        LOG_SRV("APP_OLD_COMM_PUMP_RUN_IN_CHARGE");
        sys_status = APP_OLD_COMM_PUMP_RUN_IN_CHARGE;
    }

    if (BREAST_PUMP_IDLE == pump_status || BREAST_PUMP_FINISH == pump_status) {
        LOG_SRV("APP_OLD_COMM_SYS_FINISH");
        sys_status = APP_OLD_COMM_SYS_FINISH;
    }

    if (BREAST_PUMP_PAUSE == pump_status) {
        LOG_SRV("APP_OLD_COMM_SYS_PAUSE");
        sys_status = APP_OLD_COMM_SYS_PAUSE;
    }

    if ((BREAST_PUMP_IDLE == pump_status || BREAST_PUMP_FINISH == pump_status)
        && USB_IS_PLUGIN(chage_state)) {
        LOG_SRV("APP_OLD_COMM_PUMP_FINISH_IN_CHARGE");
        sys_status = APP_OLD_COMM_PUMP_FINISH_IN_CHARGE;
    }

    if ((BREAST_PUMP_PAUSE == pump_status)
        && USB_IS_PLUGIN(chage_state)) {
        LOG_SRV("APP_OLD_COMM_PUMP_PAUSE_IN_CHARGE");
        sys_status = APP_OLD_COMM_PUMP_PAUSE_IN_CHARGE;
    }

    if (BREAST_PUMP_RESET == pump_status ) {
        LOG_SRV("APP_OLD_COMM_SYS_RESET");
        sys_status = APP_OLD_COMM_SYS_RESET;
    }

    return sys_status;
}

POSSIBLY_UNUSED static uint8_t app_old_comm_get_pump_running_status(void)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    int is_pump_working = 0; 
    READ_PUMP_IS_WORKING(pump_ctx, is_pump_working);
    LOG_SRV("pump is working = %d", is_pump_working);
    return is_pump_working;
}

static uint8_t app_old_comm_get_battery_percent(void)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_UINT(pbus);
    struct rt_device_node *battery = pbus->find_device(ID_DEVICE_BATTERY);
    uint32_t percent = 0;
    if (battery) {
        DEVICE_READ_ARGS(battery, &percent, sizeof(percent));
    } else {
        percent = 100;
    }
    LOG_SRV("percent=%d", percent);
    return (uint8_t)percent;
}

static bool app_old_comm_check_if_low_battery(void)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_UINT(pbus);
    struct rt_device_node *battery = pbus->find_device(ID_DEVICE_BATTERY);
    int percent = 0;
    if (battery) {
        DEVICE_READ_ARGS(battery, &percent, sizeof(percent));
    } else {
        percent = 100;
    }
    LOG_SRV("percent=%d", percent);
    if (percent <= LOW_BAT_PERCENTAGE) {
        return 1;
    }
    return 0;
}

static uint8_t app_old_comm_get_current_pump_mode(void)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(pump_ctx, pump_mode);
    char *mode_name = "unknow";
    READ_PUMP_WORK_MODE_NAME(pump_ctx, &mode_name);
    LOG_SRV("pump mode: %s", mode_name);
    return (uint8_t)pump_mode;
}

static uint8_t app_old_comm_get_current_custom_pump_step_mode(servers_old_protocol_publish_data_id_e id)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);

    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(pump_ctx, time_index);
    /// 当前不是运行的不是自定义模式步骤，默认上报第一步参数,启动自定义模式从第一步开始
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(pump_ctx, pump_mode);
    if (pump_mode != BREAST_PUMP_CUSTOM_MODE) {
        time_index = 0;
    }
    /// 获取自定义模式参数
    uint8_t time_step_mode = APP_OLD_COMM_STEP_MODE_SHORT;
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    pump_mode_steps_info_t steps_info ={0};
    storage_ctx->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_MODE, 0, &steps_info);
    uint8_t time_step_index = 0;    
    for (uint8_t i = 0; i < steps_info.policy_num; i++) {
        if (steps_info.data[i].policy_type == MOTOR_TIME_POLICY) {
            if (time_step_index == time_index) {
                if (steps_info.data[i].mode_type == SUCTION_MODE_SHORT) {
                    time_step_mode = APP_OLD_COMM_STEP_MODE_SHORT;
                } else if (steps_info.data[i].mode_type == SUCTION_MODE_LONG) {
                    time_step_mode = APP_OLD_COMM_STEP_MODE_LONG;
                } else if (steps_info.data[i].mode_type == SUCTION_MODE_MIX_SHORT || 
                    steps_info.data[i].mode_type == SUCTION_MODE_MIX_LONG) {
                    time_step_mode = steps_info.data[i].loop_num == 5 ? APP_OLD_COMM_STEP_MODE_MIX1 : APP_OLD_COMM_STEP_MODE_MIX2;
                } else {
                    time_step_mode = APP_OLD_COMM_STEP_MODE_RELAX;
                }
                break;
            }
            time_step_index++;
        }
    }
    LOG_SRV("custom_steps_parameter.data[%d].mode=%d", time_index, time_step_mode);
    return (uint8_t)time_step_mode;
}

static uint8_t app_old_comm_get_current_mode_pump_level(app_old_commd_l_r_ctrl_e lr_ct, servers_old_protocol_publish_data_id_e id)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(pump_ctx, time_index);
    ///当前系统在处于自定义模式步骤，应该上报什么，就默认上报吸乳第一步参数
    pump_setting_t setting;
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(pump_ctx, pump_mode);
    READ_PUMP_SETTING_INFO(pump_ctx, time_index, setting); 
    if (pump_mode == BREAST_PUMP_CUSTOM_MODE) {
        LOG_SRV("setting.level [%d] =%d", lr_ct, setting.level[lr_ct]);
        return (uint8_t)setting.level[lr_ct];
    } else {
        LOG_SRV("setting.level [%d] =%d", lr_ct, setting.level[lr_ct]);
        return (uint8_t)setting.level[lr_ct];
    }
}

static uint8_t app_old_comm_get_current_custom_pump_level(app_old_commd_l_r_ctrl_e lr_ct, servers_old_protocol_publish_data_id_e id)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(pump_ctx, time_index);
    pump_setting_t setting;
    /// 当前系统在处于自定义模式步骤
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(pump_ctx, pump_mode);
    if (pump_mode == BREAST_PUMP_CUSTOM_MODE) {
        READ_PUMP_SETTING_INFO(pump_ctx, time_index, setting); 
        LOG_SRV("setting.level [%d] =%d", lr_ct, setting.level[lr_ct]);
        return (uint8_t)setting.level[lr_ct];
    } else { 
        /// 默认上报flash中自定义模式第一步参数,启动自定义模式从第一步开始
        storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        EXCEPTION_CHECK_UINT(storage_ctx);
        storage_ctx->get_store_ram_val(STORE_ID_PUMP_SETTING_INFO, BREAST_PUMP_CUSTOM_MODE, 0, &setting);
        LOG_SRV("setting.level [%d] =%d", lr_ct, setting.level[lr_ct]);
        return (uint8_t)setting.level[lr_ct];
    }
}

/// 左右需要一致，不区分
static uint8_t app_old_comm_get_current_mode_pump_speed(servers_old_protocol_publish_data_id_e id)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);

    EXCEPTION_CHECK_UINT(pump_ctx);
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(pump_ctx, time_index);
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    int last_store_mode = BREAST_PUMP_PUMP_MODE_DEFAULT;
    storage_ctx->get_store_ram_val(STORE_ID_BREAST_PUMP_MODE, 0, 0, &last_store_mode);
    /// 当前系统在处于自定义模式步骤，应该上报什么，就默认上报吸乳第一步参数
    pump_setting_t setting;
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(pump_ctx, pump_mode);
    if (pump_mode == BREAST_PUMP_CUSTOM_MODE) {
        storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        EXCEPTION_CHECK_UINT(storage_ctx);
        storage_ctx->get_store_ram_val(STORE_ID_PUMP_SETTING_INFO, last_store_mode, 0, &setting);
        LOG_SRV("setting.speed =%d", setting.speed);
        return (uint8_t)setting.speed;
    } else {
        READ_PUMP_SETTING_INFO(pump_ctx, time_index, setting); 
        LOG_SRV("setting.speed =%d", setting.speed);
        return (uint8_t)setting.speed;
    }
}

static uint8_t app_old_comm_get_current_custom_pump_speed(servers_old_protocol_publish_data_id_e id)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(pump_ctx, time_index);
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    /// 当前不是运行的不是自定义模式步骤，默认上报第一步参数,启动自定义模式从第一步开始
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(pump_ctx, pump_mode);
    if (pump_mode != BREAST_PUMP_CUSTOM_MODE) {
        time_index = 0;
    }
    /// 不允许修改自定义模式频率，只能修改等级，所以从app设置缓存中获取
    uint8_t pump_speed = 0;
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    pump_setting_t setting;
    storage_ctx->get_store_ram_val(STORE_ID_PUMP_SETTING_INFO, BREAST_PUMP_CUSTOM_MODE, 0, &setting);
    pump_speed = setting.speed;
    LOG_SRV("setting.speed=%d", pump_speed);
    return (uint8_t)pump_speed;
}

static uint32_t app_old_comm_get_current_pump_mode_run_time(void)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    const pump_result_info_t *result_info = NULL;
    READ_PUMP_RESULT_INFO(pump_ctx, BREAST_PUMP_RESULT_PUMPING, &result_info);
    LOG_SRV("run time = %d", result_info->current_mode_work_time);
    return (uint32_t)result_info->current_mode_work_time/1000;
}

static uint32_t app_old_comm_get_pump_mode_run_total_time(servers_old_protocol_publish_data_id_e id)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    const pump_result_info_t *result_info = NULL;
    READ_PUMP_RESULT_INFO(pump_ctx, BREAST_PUMP_RESULT_PUMPING, &result_info);
    ///如果pump已经运行完成，则返回0
    int work_status = 0;
    READ_PUMP_WORK_STATUS(pump_ctx, work_status);
    ///如果pump已经运行完成，则返回0,如果id为SYSTEM_STATUS_RESET，则返回当前运行时间
    if ((work_status == BREAST_PUMP_FINISH || work_status == BREAST_PUMP_IDLE) && id != OLD_PUBLISH_DATA_SYSTEM_STATUS_RESET) {
        LOG_SRV("pump already finished or idle, total time = 0");
        return 0;
    } else {
        LOG_SRV("total time = %d", result_info->work_valid_time);
        return (uint32_t)result_info->work_valid_time/1000;
    }
}

static uint32_t app_old_comm_get_current_custom_pump_mode_total_time(void)
{
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    uint32_t pump_time = 0;
    storage_ctx->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, BREAST_PUMP_CUSTOM_MODE, 0, &pump_time);
    uint32_t custom_total_time = pump_time;
    LOG_SRV("custom total time= %d", custom_total_time);
    return (uint32_t)custom_total_time;
}

static uint8_t app_old_comm_get_current_running_mode_step(void)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(pump_ctx, time_index);
    LOG_SRV("mode_step= %d", time_index);
    return (uint8_t)time_index;
}

static uint16_t app_old_comm_get_current_running_mode_step_time(void)
{
    motor_manager_context_t *motor_ctx = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    EXCEPTION_CHECK_UINT(motor_ctx);
    uint16_t step_index_time = (uint16_t)motor_ctx->get_time_policy_remaining_time();
    //if (motor_ctx->get_timed_policy_finish_flag()) {
    //    step_index_time = 0;
    //}
    LOG_SRV("step_index_time= %d", step_index_time);
    return (uint16_t)step_index_time;
}

static uint8_t app_old_comm_get_system_is_airplane_mode(void)
{
    airplane_mode_context_t *ap = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    EXCEPTION_CHECK_UINT(ap);
    int air_play_mode = ap->is_on_airplane_mode();
    return (uint8_t)air_play_mode;
}

static void pump_copy_trial_mode_to_custom_mode(void)
{
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(storage_ctx);
    // 复制BREAST_PUMP_CUSTOM_TRIAL_MODE的数据到BREAST_PUMP_CUSTOM_MODE
    pump_mode_steps_info_t trial_steps_info;
    storage_ctx->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_TRIAL_MODE, 0, &trial_steps_info);
    storage_ctx->put_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_MODE, 0, &trial_steps_info);
    LOG_SRV("copy custom trial steps info to custom mode");
    pump_setting_t trial_setting;
    for (uint8_t i = 0; i < ROOT_BREAST_PUMP_STEP_MAX; i++) {
        if (i >= trial_steps_info.policy_num) {
            break; // 如果步数大于最大步数，则跳出
        }
        storage_ctx->get_store_ram_val(STORE_ID_PUMP_SETTING_INFO, BREAST_PUMP_CUSTOM_TRIAL_MODE, i, &trial_setting);
        storage_ctx->put_store_ram_val(STORE_ID_PUMP_SETTING_INFO, BREAST_PUMP_CUSTOM_MODE, i, &trial_setting);
    }
    LOG_SRV("copy custom trial setting info to custom mode");

    motor_manager_context_t *motor_ctx = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    EXCEPTION_CHECK_VOID(motor_ctx);
    LOG_SRV("copy custom trial steps info to custom mode ram");
    motor_ctx->hook->generate_motor_para(motor_ctx, BREAST_PUMP_CUSTOM_MODE, &trial_steps_info, sizeof(trial_steps_info));
    
    char mode_name[PUMP_MODE_NAME_LEN];   /// 模式名称
    storage_ctx->get_store_ram_val(STORE_ID_PUMP_MODE_NAME_INFO, BREAST_PUMP_CUSTOM_TRIAL_MODE, 0, mode_name);
    storage_ctx->put_store_ram_val(STORE_ID_PUMP_MODE_NAME_INFO, BREAST_PUMP_CUSTOM_MODE, 0, mode_name);
}

static uint8_t pump_custom_trial_mode_update_by_app(pump_mode_steps_info_t pump_steps_info, char *name)
{
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    motor_manager_context_t *motor_ctx = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(storage_ctx);
    EXCEPTION_CHECK_UINT(motor_ctx);
    EXCEPTION_CHECK_UINT(pump_ctx);
    //更新自定义模式名称
    storage_ctx->put_store_ram_val(STORE_ID_PUMP_MODE_NAME_INFO, BREAST_PUMP_CUSTOM_TRIAL_MODE, 0, name);
    ///是否需要区分设备是否在用户自定义模式下运行，如果运行，是否直接替换用户自定义模式参数
    storage_ctx->put_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_TRIAL_MODE, 0, &pump_steps_info);
    LOG_SRV("put custom trial steps info to ram");
    motor_ctx->hook->generate_motor_para(motor_ctx, BREAST_PUMP_CUSTOM_TRIAL_MODE, &pump_steps_info, sizeof(pump_steps_info));
    //custom_trial_mode_update_by_steps_info(&pump_steps_info);
    pump_setting_t setting;
    for (uint8_t i = 0; i < ROOT_BREAST_PUMP_STEP_MAX; i++) {
        if (i >= pump_steps_info.policy_num) {
            break; // 如果步数大于最大步数，则跳出
        }
        setting.speed = pump_steps_info.data[i].freq;
        for (uint8_t j = 0; j < BREAST_PUMP_NUM; j++) {
            setting.level[j] = pump_steps_info.data[i].level;
        }
        storage_ctx->put_store_ram_val(STORE_ID_PUMP_SETTING_INFO, BREAST_PUMP_CUSTOM_TRIAL_MODE, i, &setting);
    }

    ///同步参数到custom
    pump_copy_trial_mode_to_custom_mode();
    ///更新同步标志
    storage_ctx->put_store_ram_val(STORE_ID_MODE_NEW_STEP_INFO_FLAG, BREAST_PUMP_CUSTOM_MODE, true, 0);
    ///更新激活标识 自定义模式激活
    WRITE_PUMP_MODE_ACTIVATE(pump_ctx, BREAST_PUMP_CUSTOM_MODE, 1);
    LOG_SRV("update custom trial mode activate state");
    return true;
}

static uint8_t pump_custom_mode_update_by_app(pump_mode_steps_info_t pump_steps_info, char *name)
{
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    motor_manager_context_t *motor_ctx = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(storage_ctx);
    EXCEPTION_CHECK_UINT(motor_ctx);
    EXCEPTION_CHECK_UINT(pump_ctx);

    //更新自定义模式名称
    storage_ctx->put_store_ram_val(STORE_ID_PUMP_MODE_NAME_INFO, BREAST_PUMP_CUSTOM_MODE, 0, name);
    ///是否需要区分设备是否在用户自定义模式下运行，如果运行，是否直接替换用户自定义模式参数
    storage_ctx->put_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_MODE, 0, &pump_steps_info);
    LOG_SRV("put custom steps info to ram");
    motor_ctx->hook->generate_motor_para(motor_ctx, BREAST_PUMP_CUSTOM_MODE, &pump_steps_info, sizeof(pump_steps_info));
    //custom_mode_update_by_steps_info(&pump_steps_info);
    pump_setting_t setting;
    for (uint8_t i = 0; i < ROOT_BREAST_PUMP_STEP_MAX; i++) {
        if (i >= pump_steps_info.policy_num) {
            break; // 如果步数大于最大步数，则跳出
        }
        setting.speed = pump_steps_info.data[i].freq;
        for (uint8_t j = 0; j < BREAST_PUMP_NUM; j++) {
            setting.level[j] = pump_steps_info.data[i].level;
        }
        storage_ctx->put_store_ram_val(STORE_ID_PUMP_SETTING_INFO, BREAST_PUMP_CUSTOM_MODE, i, &setting);
    }
    ///更新激活标识 自定义模式激活
    WRITE_PUMP_MODE_ACTIVATE(pump_ctx, BREAST_PUMP_CUSTOM_MODE, 1);

    return true;
}

static uint8_t app_old_comm_set_nommal_pump_level(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(pump_ctx, time_index);
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    pump_setting_t setting; /// 当前泵奶设置信息.
    READ_PUMP_SETTING_INFO(pump_ctx, time_index, setting); 
    if (app_old_comm_pump_ctrl_date->pump_mode_lr_or_all == APP_OLD_COMM_LEFT_CT) {
        if (app_old_comm_pump_ctrl_date->normal_pump_level >= 0) {
            ///相同不进行设置
            if (setting.level[0] == app_old_comm_pump_ctrl_date->normal_pump_level) {
                LOG_SRV("left pump level no change %d", setting.level[0]);
                return false;
            }
            setting.level[0] = app_old_comm_pump_ctrl_date->normal_pump_level;
            LOG_SRV("left pump level %d",setting.level[0]);
            app_broadcast_pump_level_to_window(setting.level[0], OLD_BROADCAST_LEFT_CT);
        }
    } else if (app_old_comm_pump_ctrl_date->pump_mode_lr_or_all == APP_OLD_COMM_RIGHT_CT) {
        if (app_old_comm_pump_ctrl_date->normal_pump_level <= BREAST_PUMP_LEVEL_15) {
            ///相同不进行设置
            if (setting.level[1] == app_old_comm_pump_ctrl_date->normal_pump_level) {
                LOG_SRV("right pump level no change %d", setting.level[1]);
                return false;
            }
            setting.level[1] = app_old_comm_pump_ctrl_date->normal_pump_level;
            LOG_SRV("right pump level %d",setting.level[1]);
            app_broadcast_pump_level_to_window(setting.level[1], OLD_BROADCAST_RIGHT_CT);
        }
    } else {
        if (app_old_comm_pump_ctrl_date->normal_pump_level >= 0
            && app_old_comm_pump_ctrl_date->normal_pump_level <= BREAST_PUMP_LEVEL_15) {
            ///相同不进行设置
            if (setting.level[0] == app_old_comm_pump_ctrl_date->normal_pump_level && setting.level[1] == app_old_comm_pump_ctrl_date->normal_pump_level) {
                LOG_SRV("all pump level no change %d", setting.level[0]);
                return false;
            }
            setting.level[1] = app_old_comm_pump_ctrl_date->normal_pump_level;
            setting.level[0] = app_old_comm_pump_ctrl_date->normal_pump_level;
            LOG_SRV("all pump level %d, %d", setting.level[0], setting.level[1]);
            app_broadcast_pump_level_to_window(setting.level[0], OLD_BROADCAST_LEFT_CT);
            app_broadcast_pump_level_to_window(setting.level[1], OLD_BROADCAST_RIGHT_CT);
            ///双边模式不要设置，通过EA设置
            return false;
        }
    }
    WRITE_PUMP_SETTING_INFO(pump_ctx, time_index, true, setting);
    return true;
}

static uint8_t app_old_comm_set_nommal_pump_speed(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(pump_ctx, time_index);
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    pump_setting_t setting; /// 当前泵奶设置信息.
    READ_PUMP_SETTING_INFO(pump_ctx, time_index, setting); 
    //是否变动speed
    if (setting.speed == app_old_comm_pump_ctrl_date->normal_pump_speed) {
        LOG_SRV("pump speed no change %d", setting.speed);
        return false;
    }
    //只允许常规模式设置 speed，并且左右同步控制
    setting.speed = app_old_comm_pump_ctrl_date->normal_pump_speed;
    LOG_SRV("pump speed  %d", setting.speed);
    WRITE_PUMP_SETTING_INFO(pump_ctx, time_index, true, setting);
    app_broadcast_pump_speed_to_window(setting.speed ,OLD_BROADCAST_ALL_CT);

    return true;
}

static uint8_t app_old_comm_set_pump_control_type(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
{
    uint8_t control_type = BREAST_PUMP_CONTROL_TYPE_BILATERAL;
    if (app_old_comm_pump_ctrl_date->pump_mode_lr_or_all == APP_OLD_COMM_LEFT_CT) {
        control_type = BREAST_PUMP_CONTROL_TYPE_LEFT;
    } else if (app_old_comm_pump_ctrl_date->pump_mode_lr_or_all == APP_OLD_COMM_RIGHT_CT) {
        control_type = BREAST_PUMP_CONTROL_TYPE_RIGHT;
    } else {
        control_type = BREAST_PUMP_CONTROL_TYPE_BILATERAL;
    }

    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump);

    int current_control_type = 0;
    READ_PUMP_LEVEL_CONTROL_TYPE(pump, current_control_type);
    if (current_control_type == control_type) {
        LOG_SRV("pump control type no change %d", control_type);
        return false;
    }
    WRITE_PUMP_LEVEL_CONTROL_TYPE(pump, control_type, 1);
    
    /*led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    if (led && led->hook && led->hook->set_key_backlight) {
        led->hook->set_key_backlight(led, 100);
    }*/
    LOG_SRV("set pump control type %d", control_type);
    return true;
}

static uint8_t app_old_comm_get_pump_control_type(void)
{
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump);
    int control_type = 0;
    READ_PUMP_LEVEL_CONTROL_TYPE(pump, control_type);
    LOG_SRV("get pump control type %d", control_type);
    return control_type;
}

static uint8_t app_old_comm_set_custom_pump_level(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(pump_ctx, time_index);
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    pump_setting_t setting; /// 当前泵奶设置信息.
    READ_PUMP_SETTING_INFO(pump_ctx, time_index, setting); 
    if (app_old_comm_pump_ctrl_date->pump_mode_lr_or_all == APP_OLD_COMM_LEFT_CT) {
        if (app_old_comm_pump_ctrl_date->user_define_pump_level >= 0) {
            ///相同不进行设置
            if (setting.level[0] == app_old_comm_pump_ctrl_date->user_define_pump_level) {
                LOG_SRV("left pump level no change %d", setting.level[0]);
                return false;
            }
            setting.level[0] = app_old_comm_pump_ctrl_date->user_define_pump_level;
            LOG_SRV("left pump level %d",setting.level[0]);
            app_broadcast_pump_level_to_window(setting.level[0] ,OLD_BROADCAST_LEFT_CT);
        }
    } else if (app_old_comm_pump_ctrl_date->pump_mode_lr_or_all == APP_OLD_COMM_RIGHT_CT) {
        if (app_old_comm_pump_ctrl_date->user_define_pump_level <= BREAST_PUMP_LEVEL_15) {
            ///相同不进行设置
            if (setting.level[1] == app_old_comm_pump_ctrl_date->user_define_pump_level) {
                LOG_SRV("right pump level no change %d", setting.level[1]);
                return false;
            }
            setting.level[1] = app_old_comm_pump_ctrl_date->user_define_pump_level;
            LOG_SRV("right pump level %d",setting.level[1]);
            app_broadcast_pump_level_to_window(setting.level[1] ,OLD_BROADCAST_RIGHT_CT);
        }
    } else {
        if (app_old_comm_pump_ctrl_date->user_define_pump_level >= 0
            && app_old_comm_pump_ctrl_date->user_define_pump_level <= BREAST_PUMP_LEVEL_15) {
            ///相同不进行设置
            if (setting.level[0] == app_old_comm_pump_ctrl_date->user_define_pump_level && setting.level[1] == app_old_comm_pump_ctrl_date->user_define_pump_level) {
                LOG_SRV("all pump level no change %d", setting.level[0]);
                return false;
            }
            setting.level[1] = app_old_comm_pump_ctrl_date->user_define_pump_level;
            setting.level[0] = app_old_comm_pump_ctrl_date->user_define_pump_level;
            LOG_SRV("all pump level %d, %d", setting.level[0], setting.level[1]);
            app_broadcast_pump_level_to_window(setting.level[0] ,OLD_BROADCAST_LEFT_CT);
            app_broadcast_pump_level_to_window(setting.level[1] ,OLD_BROADCAST_RIGHT_CT);
            ///双边模式不要设置，通过EA设置
            return false;
        }
    }
    WRITE_PUMP_SETTING_INFO(pump_ctx, time_index, true, setting);
    return true;
}

static uint8_t app_old_comm_set_custom_pump_speed(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(pump_ctx, time_index);
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    pump_setting_t setting; /// 当前泵奶设置信息.
    READ_PUMP_SETTING_INFO(pump_ctx, time_index, setting); 
    //是否变动speed
    if (setting.speed == app_old_comm_pump_ctrl_date->user_define_pump_speed) {
        LOG_SRV("pump speed no change %d", setting.speed);
        return false;
    }
    //左右同步控制
    setting.speed = app_old_comm_pump_ctrl_date->user_define_pump_speed;
    LOG_SRV("pump speed  %d", setting.speed);
    WRITE_PUMP_SETTING_INFO(pump_ctx, time_index, true, setting);
    app_broadcast_pump_speed_to_window(setting.speed ,OLD_BROADCAST_ALL_CT);

    return true;
}

static uint8_t app_old_comm_pump_control_handler(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(pump_ctx, time_index);
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    //appbug，就是切模式把之前目标模式的参数，也带下来了，但是切模式都要从第一步运行，要以我这边为准
    //如果模式不变，说明只设置当前模式参数,
    if (app_old_comm_pump_ctrl_date->pump_mode == app_old_comm_get_current_pump_mode()) {
        //设置控制类型
        app_old_comm_set_pump_control_type(app_old_comm_pump_ctrl_date);
        //设置参数
        switch (app_old_comm_get_current_pump_mode()) {
            case BREAST_PUMP_LACTATION_MODE:
            case BREAST_PUMP_STIMULATION_MODE: // 传统模式
            case BREAST_PUMP_RELAX_MODE: // 放松模式
            case BREAST_PUMP_MIX_MODE: 
            case BREAST_PUMP_REGULAR_PUMPING_MODE:
            case BREAST_PUMP_EMPTYING_BOOST_MODE:
            case BREAST_PUMP_PP_MILK_TRACK_MODE:
            case BREAST_PUMP_MILK_BOOST_MODE:
            case BREAST_PUMP_LACTATION_START_MODE: {
                app_old_comm_set_nommal_pump_speed(app_old_comm_pump_ctrl_date);
                app_old_comm_set_nommal_pump_level(app_old_comm_pump_ctrl_date);
                break;
            }
            case BREAST_PUMP_CUSTOM_MODE: {
                app_old_comm_set_custom_pump_speed(app_old_comm_pump_ctrl_date);
                app_old_comm_set_custom_pump_level(app_old_comm_pump_ctrl_date); //设置运行中的参数
                //todo                                                          //将缓存的cust para也更新下
                break;
            }
            default:
                break;
        }
    } else {
        /// 如果模式改变，则改变模式,忽略参数
        WRITE_PUMP_WORK_MODE(pump_ctx, app_old_comm_pump_ctrl_date->pump_mode, true, 1);
        app_broadcast_pump_mode_to_window(app_old_comm_pump_ctrl_date->pump_mode,OLD_BROADCAST_ALL_CT);
        /// prd 模式记忆
        LOG_SRV("app change pump_mode %d",app_old_comm_pump_ctrl_date->pump_mode);
    }
    return true;
}

static uint8_t app_old_comm_pump_run_status_ctrl(uint8_t status)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    LOG_SRV("status: %d", status);
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(pump_ctx, pump_status);
    if (pump_status == BREAST_PUMP_FINISH || pump_status == BREAST_PUMP_IDLE) {
        if (status == APP_OLD_COMM_PUMP_CTRL_RUN) {
            //WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_START, 1);
            app_broadcast_pump_status_to_window(BREAST_PUMP_START, OLD_BROADCAST_ALL_CT);
            app_pump_safe_start_key_sim_task();
            WRITE_PUMP_IS_START_FROM_APP(pump_ctx, true);
        }
    } else if (pump_status == BREAST_PUMP_START || pump_status == BREAST_PUMP_CONTINUE) {
        if (status == APP_OLD_COMM_PUMP_CTRL_STOP) {
            //WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_PAUSE, 1);
            app_broadcast_pump_status_to_window(BREAST_PUMP_PAUSE,OLD_BROADCAST_ALL_CT);
            app_pump_safe_start_key_sim_task();
        }
    } else if (pump_status == BREAST_PUMP_PAUSE) {
        if (status == APP_OLD_COMM_PUMP_CTRL_RUN) {
            //WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_CONTINUE, 1);
            app_broadcast_pump_status_to_window(BREAST_PUMP_CONTINUE,OLD_BROADCAST_ALL_CT);
            app_pump_safe_start_key_sim_task();
        }
    }
    return true;
}

static void app_pump_ctrl_delay_task_handler(hdr_post_args_t *args)
{
    int status = (int)args->arg1;
    app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date = (app_old_comm_pump_ctrl *)args->user_data;
    EXCEPTION_CHECK_VOID(app_old_comm_pump_ctrl_date);
    ///app_old_comm_pump_control_handler(app_old_comm_pump_ctrl_date);
    app_old_comm_pump_run_status_ctrl(status);
    LOG_SRV("status: %d", status);
}

static void app_pump_custom_trial_mode_start_handler(void)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_VOID(pump_ctx);
    int is_pump_working = 0; 
    READ_PUMP_IS_WORKING(pump_ctx, is_pump_working);
    if (app_old_comm_get_current_pump_mode() == BREAST_PUMP_CUSTOM_TRIAL_MODE && is_pump_working) {
        WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_PAUSE, 0);
        app_broadcast_pump_status_to_window(BREAST_PUMP_PAUSE, OLD_BROADCAST_ALL_CT);
        WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_START, 0);
        app_broadcast_pump_status_to_window(BREAST_PUMP_START, OLD_BROADCAST_ALL_CT);
    }
}

static void app_pump_custom_mode_start_handler(void)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_VOID(pump_ctx);
    int is_pump_working = 0; 
    READ_PUMP_IS_WORKING(pump_ctx, is_pump_working);
    if (app_old_comm_get_current_pump_mode() == BREAST_PUMP_CUSTOM_MODE && is_pump_working) {
        WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_PAUSE, 0);
        app_broadcast_pump_status_to_window(BREAST_PUMP_PAUSE, OLD_BROADCAST_ALL_CT);
        WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_START, 0);
        app_broadcast_pump_status_to_window(BREAST_PUMP_START, OLD_BROADCAST_ALL_CT);
    }
}

static void app_pump_user_define_delay_task_handler(hdr_post_args_t *args)
{
    app_pump_custom_mode_start_handler();
}

static void app_pump_trial_user_define_delay_task_handler(hdr_post_args_t *args)
{
    app_pump_custom_trial_mode_start_handler();
}

static void app_pump_broadcast_app_completed_delay_task_handler(hdr_post_args_t *args)
{
    /// 发送APP强制完成广播
    if (is_broadcast_inited()) {
        SEND_BROADCAST_ARGS1(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_APP_COMPLETED);
        LOG_SRV("APP_COMPLETED broadcast sent");
    } else {
        LOG_SRV("Broadcast system not initialized, skip sending APP_COMPLETED broadcast");
    }
}

static bool app_old_comm_report_start_block_abnormal(uint8_t abnormal_id)
{
    servers_publish_data_args_t args_temp = {
        .arg1 = abnormal_id,
        .arg2 = OLD_PUBLISH_ABNORMAL_STATE_ACTIVE,
        .val = NULL,
    };
    app_old_comm_handle_dev_abnormal_report(&args_temp);
    return true;
}

///低电不足运行一次不允许启动
static bool app_old_comm_check_if_start_block_abnormal(uint8_t ctrl_status)
{
    int pump_status = 0;
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    battery_manager_context_t *batt_ctx = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    airplane_mode_context_t *air_ctx = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    EXCEPTION_CHECK_UINT(pump_ctx);
    EXCEPTION_CHECK_UINT(batt_ctx);
    EXCEPTION_CHECK_UINT(air_ctx);
    READ_PUMP_WORK_STATUS(pump_ctx, pump_status);

    if (ctrl_status != APP_OLD_COMM_PUMP_CTRL_RUN) {
        return false;
    }

    if (pump_status != BREAST_PUMP_FINISH && pump_status != BREAST_PUMP_IDLE) {
        return false;
    }

    if ((app_old_comm_get_battery_percent() <= FORBID_START_BAT_PERCENTAGE) && !batt_ctx->is_vbus_plug_in) {
        return app_old_comm_report_start_block_abnormal(OLD_PUBLISH_ABNORMAL_LOW_BAT_CANNOT_START);
    }

    ///适配器不适配，运行一次不允许启动
    if (batt_ctx->get_charge_status() == CHARGE_STATE_ERROR) {
        return app_old_comm_report_start_block_abnormal(OLD_PUBLISH_ABNORMAL_ADAPTER_ERROR);
    }

    ///旅行锁
    if (air_ctx->is_on_airplane_mode()) {
        return app_old_comm_report_start_block_abnormal(OLD_PUBLISH_ABNORMAL_TRAVEL_LOCK);
    }

    //如果在引导窗口，则不允许启动
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_UINT(wm);
    if (wm->is_current_window(wm, ID_WINDOW_GUIDE) || wm->is_current_window(wm, ID_WINDOW_POWER_OFF)
        || wm->is_current_window(wm, ID_WINDOW_CHARGE)) {
        return app_old_comm_report_start_block_abnormal(OLD_PUBLISH_ABNORMAL_GUIDE_WINDOW);
    }

    return false;
}

static void app_old_comm_append_u16_le(uint8_t *buf, uint16_t *len, uint32_t value)
{
    EXCEPTION_CHECK_VOID(buf && len);
    buf[(*len)++] = (uint8_t)((value >> 0) & 0xFF);
    buf[(*len)++] = (uint8_t)((value >> 8) & 0xFF);
}

static breast_pump_mode_e app_old_comm_recv_mode_to_breast_pump_mode(uint8_t recv_mode)
{
    switch ((app_old_comm_pump_mode)recv_mode) {
    case APP_OLD_COMM_MODE_MASSAGE:
        return BREAST_PUMP_STIMULATION_MODE;
    case APP_OLD_COMM_MODE_SUCTION:
        return BREAST_PUMP_LACTATION_MODE;
    case APP_OLD_COMM_MODE_MIXED:
        return BREAST_PUMP_MIX_MODE;
    case APP_OLD_COMM_MODE_REGULAR_PUMPING:
        return BREAST_PUMP_REGULAR_PUMPING_MODE;
    case APP_OLD_COMM_MODE_MILK_BOOST:
        return BREAST_PUMP_MILK_BOOST_MODE;
    case APP_OLD_COMM_MODE_PP_MILK_TRACK:
        return BREAST_PUMP_PP_MILK_TRACK_MODE;
    case APP_OLD_COMM_MODE_LACTATION_START:
        return BREAST_PUMP_LACTATION_START_MODE;
    case APP_OLD_COMM_MODE_EMPTYING_BOOST:
        return BREAST_PUMP_EMPTYING_BOOST_MODE;
    case APP_OLD_COMM_MODE_CUSTOM:
        return BREAST_PUMP_CUSTOM_MODE;
    case APP_OLD_COMM_MODE_CUSTOM_TRIAL:
        return BREAST_PUMP_CUSTOM_TRIAL_MODE;
    case APP_OLD_COMM_MODE_RELAX:
        return BREAST_PUMP_RELAX_MODE;
    default:
        LOG_SRV("invalid app pump mode: 0x%02X", recv_mode);
        return BREAST_PUMP_MODE_INVALID;
    }
}

static uint8_t breast_pump_mode_to_app_old_comm_mode(breast_pump_mode_e mode)
{
    switch (mode) {
    case BREAST_PUMP_STIMULATION_MODE:
        return APP_OLD_COMM_MODE_MASSAGE;
    case BREAST_PUMP_LACTATION_MODE:
        return APP_OLD_COMM_MODE_SUCTION;
    case BREAST_PUMP_MIX_MODE:
        return APP_OLD_COMM_MODE_MIXED;
    case BREAST_PUMP_REGULAR_PUMPING_MODE:
        return APP_OLD_COMM_MODE_REGULAR_PUMPING;
    case BREAST_PUMP_MILK_BOOST_MODE:
        return APP_OLD_COMM_MODE_MILK_BOOST;
    case BREAST_PUMP_PP_MILK_TRACK_MODE:
        return APP_OLD_COMM_MODE_PP_MILK_TRACK;
    case BREAST_PUMP_LACTATION_START_MODE:
        return APP_OLD_COMM_MODE_LACTATION_START;
    case BREAST_PUMP_EMPTYING_BOOST_MODE:
        return APP_OLD_COMM_MODE_EMPTYING_BOOST;
    case BREAST_PUMP_CUSTOM_MODE:
        return APP_OLD_COMM_MODE_CUSTOM;
    case BREAST_PUMP_CUSTOM_TRIAL_MODE:
        return APP_OLD_COMM_MODE_CUSTOM_TRIAL;
    case BREAST_PUMP_RELAX_MODE:
        return APP_OLD_COMM_MODE_RELAX;
    default:
        LOG_SRV("invalid breast pump mode for app report: %d", mode);
        return 0;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

//0xd0
void app_old_comm_report_device_status(app_old_commd_l_r_ctrl_e lr_ct, servers_old_protocol_publish_data_id_e id, void *val)
{
    LOG_SRV(" Reporting device status id %d", id);
    uint8_t report_data[45] = {0};
    uint16_t data_len = 0;
    uint32_t mode_total_time = app_old_comm_get_pump_mode_run_total_time(id);
    uint32_t mode_run_time = app_old_comm_get_current_pump_mode_run_time();
    uint32_t custom_mode_total_time = app_old_comm_get_current_custom_pump_mode_total_time();
    uint32_t mode_step_time = app_old_comm_get_current_running_mode_step_time();

    report_data[data_len++] = app_old_comm_get_battery_percent();
    report_data[data_len++] = app_old_comm_get_sys_status(id, val);
    report_data[data_len++] = lr_ct;
    report_data[data_len++] = breast_pump_mode_to_app_old_comm_mode(
        (breast_pump_mode_e)app_old_comm_get_current_pump_mode());
    report_data[data_len++] = app_old_comm_get_current_mode_pump_level(lr_ct, id); //0x01~0x0F（1档~15档）
    report_data[data_len++] = app_old_comm_get_current_custom_pump_step_mode(id);
    report_data[data_len++] = app_old_comm_get_current_custom_pump_level(lr_ct, id); //0x01~0x0F（1档~15档）
    report_data[data_len++] = app_old_comm_get_current_custom_pump_speed(id) + 1;     //频次（1-低 2-中 3-高）（1字节）
    app_old_comm_append_u16_le(report_data, &data_len, mode_total_time); //单位s
    app_old_comm_append_u16_le(report_data, &data_len, mode_run_time); //单位s
    report_data[data_len++] = 0; //v3无储奶检测
    report_data[data_len++] = app_old_comm_check_if_low_battery();
    app_old_comm_append_u16_le(report_data, &data_len, custom_mode_total_time);
    report_data[data_len++] = app_old_comm_get_system_is_airplane_mode();
    report_data[data_len++] = app_old_comm_get_current_mode_pump_speed(id) + 1; //normal speed //频次（1-低 2-中 3-高）（1字节）
    report_data[data_len++] = 0x00; //预留
    report_data[data_len++] = 0x00; //预留
    report_data[data_len++] = app_old_comm_get_current_running_mode_step();
    app_old_comm_append_u16_le(report_data, &data_len, mode_step_time);
    report_data[data_len++] = 0;
    report_data[data_len++] = 0;
    report_data[data_len++] = 0;
    report_data[data_len++] = app_old_comm_get_pump_control_type();
    report_data[data_len++] = 0;
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NOTIFY, APP_OLD_COMM_CID_NOTIFY_STATUS, data_len, report_data);
}

//0xed
void app_old_comm_report_custom_mode_id(app_old_commd_l_r_ctrl_e lr_ct)
{
    pump_mode_steps_info_t trial_steps_info;
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(storage_ctx);
    storage_ctx->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_MODE, 0, &trial_steps_info);
    uint8_t report_data[4] = {
        (uint8_t)((trial_steps_info.id.dat32 >> 0) & 0xFFU),
        (uint8_t)((trial_steps_info.id.dat32 >> 8) & 0xFFU),
        (uint8_t)((trial_steps_info.id.dat32 >> 16) & 0xFFU),
        (uint8_t)((trial_steps_info.id.dat32 >> 24) & 0xFFU),
    };
    LOG_SRV("Reporting custom mode id notify: 0x%08lX", (unsigned long)trial_steps_info.id.dat32);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NOTIFY, APP_OLD_COMM_CID_NOTIFY_CUSTOM_MODE_ID,
                                               sizeof(report_data), report_data);
}

//0xb0
void app_old_comm_handle_get_status_req(app_old_commd_l_r_ctrl_e lr_ct)
{
    LOG_SRV(" Handling get device status request");
    uint8_t ack_data[45] = {0};
    uint16_t data_len = 0;
    uint32_t mode_total_time = app_old_comm_get_pump_mode_run_total_time(OLD_PUBLISH_DATA_POLL_REPORT);
    uint32_t mode_run_time = app_old_comm_get_current_pump_mode_run_time();
    uint32_t custom_mode_total_time = app_old_comm_get_current_custom_pump_mode_total_time();
    uint32_t mode_step_time = app_old_comm_get_current_running_mode_step_time();

    ack_data[data_len++] = app_old_comm_get_battery_percent();
    ack_data[data_len++] = app_old_comm_get_sys_status(OLD_PUBLISH_DATA_POLL_REPORT, NULL);
    ack_data[data_len++] = lr_ct;
    ack_data[data_len++] = breast_pump_mode_to_app_old_comm_mode(
        (breast_pump_mode_e)app_old_comm_get_current_pump_mode());
    ack_data[data_len++] = app_old_comm_get_current_mode_pump_level(lr_ct, OLD_PUBLISH_DATA_POLL_REPORT);
    ack_data[data_len++] = app_old_comm_get_current_custom_pump_step_mode(OLD_PUBLISH_DATA_POLL_REPORT);
    ack_data[data_len++] = app_old_comm_get_current_custom_pump_level(lr_ct, OLD_PUBLISH_DATA_POLL_REPORT);
    ack_data[data_len++] = app_old_comm_get_current_custom_pump_speed(OLD_PUBLISH_DATA_POLL_REPORT) + 1;
    app_old_comm_append_u16_le(ack_data, &data_len, mode_total_time);
    app_old_comm_append_u16_le(ack_data, &data_len, mode_run_time);
    ack_data[data_len++] = 0; //v3无储奶检测
    ack_data[data_len++] = app_old_comm_check_if_low_battery();
    app_old_comm_append_u16_le(ack_data, &data_len, custom_mode_total_time);
    ack_data[data_len++] = app_old_comm_get_system_is_airplane_mode();
    ack_data[data_len++] = app_old_comm_get_current_mode_pump_speed(OLD_PUBLISH_DATA_POLL_REPORT) + 1; //normal speed
    ack_data[data_len++] = 0x00; //预留
    ack_data[data_len++] = 0x00; //预留
    ack_data[data_len++] = app_old_comm_get_current_running_mode_step();
    app_old_comm_append_u16_le(ack_data, &data_len, mode_step_time);
    ack_data[data_len++] = 0;
    ack_data[data_len++] = 0;
    ack_data[data_len++] = 0;
    ack_data[data_len++] = app_old_comm_get_pump_control_type();
    ack_data[data_len++] = 0;
    LOG_SRV(" Sending device status ACK (data len: %d)", data_len);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_STATUS, data_len, ack_data);
}

//0xc00
void app_old_comm_handle_ctrl_dev_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling device control request");
    EXCEPTION_CHECK_VOID(cab);
    if (cal < 0x0A) {
        LOG_SRV(" Invalid data length for CTRL_DEV (expected >=0X0A, got %d)", cal);
    }
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_VOID(pump_ctx);
    // TODO: Send parameters to pump valve component
    app_old_comm_pump_ctrl app_old_comm_pump_ctrl_date;
    app_old_comm_pump_ctrl_date.pump_ctrl_status = cab[0];//0X00 -暂停  0x01 启动
    app_old_comm_pump_ctrl_date.pump_mode =
        (uint8_t)app_old_comm_recv_mode_to_breast_pump_mode(cab[1]);
    app_old_comm_pump_ctrl_date.normal_pump_level = cab[2];
    app_old_comm_pump_ctrl_date.user_define_pump_step_mode = cab[3];
    app_old_comm_pump_ctrl_date.user_define_pump_level = cab[4];
    app_old_comm_pump_ctrl_date.user_define_pump_speed = cab[5] - 1; //频次（1-低 2-中 3-高）（1字节）
    app_old_comm_pump_ctrl_date.pump_mode_lr_or_all = cab[6];//0:左 1:右 2:双
    app_old_comm_pump_ctrl_date.normal_pump_speed = cab[7] - 1; //频次（1-低 2-中 3-高）（1字节）

    ///低电不足运行一次不允许启动
    if (app_old_comm_check_if_start_block_abnormal(cab[0])) {
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NACK, APP_OLD_COMM_CID_CTRL_DEV, 1, cab);
        return;
    }
    ///查看窗口，是否需要唤醒pump窗口
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_PUMP)) {
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_CTRL_DEV, 1, &cab[6]);
        app_old_comm_pump_control_handler(&app_old_comm_pump_ctrl_date);
        app_old_comm_pump_run_status_ctrl(cab[0]);
    } else if (wm->is_current_window(wm, ID_WINDOW_CHARGE) || wm->is_current_window(wm, ID_WINDOW_DUMMY)
        || wm->is_current_window(wm, ID_WINDOW_HOME)) {
        LOG_SRV("charge or dummy, dispatch pump window.");
        multi_func_msg_data_t msg = {0};       
        ///先把参数和模式发给pump组件，再dispatch窗口
        app_old_comm_pump_control_handler(&app_old_comm_pump_ctrl_date);
        msg.union_body.extra.window_id = ID_WINDOW_PUMP;
        wm->dispatch_new_window_args(wm, &msg);
        ///等待窗口弹出延时投递处理
        // 注意：POST_TASK_HANDLER_DELAYED_ARG2 会先删除同名旧任务，然后投递新任务
        // 使用静态变量避免内存泄漏，但如果删除失败，旧任务可能使用新数据
        memcpy(&pump_ctrl_data_temp, &app_old_comm_pump_ctrl_date, sizeof(app_old_comm_pump_ctrl));
        POST_TASK_HANDLER_DELAYED_ARG2(app_pump_ctrl_delay_task_handler, PUMP_CTRL_TASK_DELAY_TIME, cab[0], &pump_ctrl_data_temp,
                                       "app_pump_ctrl_delay_task");
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_CTRL_DEV, 1, &cab[6]);
    } else {
        LOG_SRV("not in pump window or charge or dummy window");
    }
    LOG_HEX("device control hex", cab, cal);
}

//0xc4
void app_old_comm_handle_set_user_define_req(uint8_t cal, uint8_t *cab)
{
    uint8_t section_num = 0, speed = 0, mode_type = 0, level = 0;
    uint16_t time_policy_duration = 0;
	pump_mode_steps_info_t pump_steps_info = {0};
    char mode_name[PUMP_MODE_NAME_LEN];   /// 模式名称
    EXCEPTION_CHECK_VOID(cab);
    //memcpy(pump_steps_info.id.dat, cab, 4);
    pump_steps_info.id.dat[0] = cab[0];
    pump_steps_info.id.dat[1] = cab[1];
    pump_steps_info.id.dat[2] = cab[2];
    pump_steps_info.id.dat[3] = cab[3];
	LOG_SRV("cmd ud-mode id = %d", pump_steps_info.id.dat32);
	if (cab[4] <= ROOT_BREAST_PUMP_STEP_MAX) {
		section_num = cab[4];
		pump_steps_info.time_policy_num = section_num;
        LOG_SRV("time_policy_num = %d", pump_steps_info.time_policy_num);
	}
	else { //超出范围，发送NACK
		LOG_SRV("section_num: %d, out of range", section_num);
		app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NACK, APP_OLD_COMM_CID_USER_DEFINE, 4, cab);
		return;
	}

    ///名称标号
    if ( cab[5] < 10) {  
        snprintf(mode_name, sizeof(mode_name), "0%d", cab[5]);
    } else {
        snprintf(mode_name, sizeof(mode_name), "%d", cab[5]);
    }
    LOG_SRV("mode_name %s", mode_name);

	pump_steps_info.total_time = 0;
    uint8_t step_index = 0;
    uint16_t n = 0;
	for (uint8_t i = 0; i < section_num; i++) {
		n = 6 + 10 * i;
        if (!app_old_comm_check_custom_payload_len(cal, n, APP_OLD_COMM_CID_USER_DEFINE, cab)) {
            return;
        }
        if (cab[n] > ROOT_BREAST_PUMP_STEP_MAX) {
            LOG_SRV("section index out of range:%d", cab[n]);
            app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NACK, APP_OLD_COMM_CID_USER_DEFINE, 4, cab);
            return;
        }
        mode_type = cab[n + 1];
        level = cab[n + 2] >= BREAST_PUMP_LEVEL_MAX ? BREAST_PUMP_LEVEL_15 : cab[n + 2];
        speed = (cab[n + 3] - 1) >= BREAST_PUMP_SPEED_MAX ? BREAST_PUMP_SPEED_HIGH : (cab[n + 3] - 1);
        time_policy_duration = cab[n + 4] << 8 | cab[n + 5];
        if (mode_type == APP_OLD_COMM_STEP_MODE_SHORT) {
                if (!app_old_comm_check_custom_policy_capacity(step_index, 1, APP_OLD_COMM_CID_USER_DEFINE, cab)) {
                    return;
                }
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_SHORT;
                pump_steps_info.data[step_index].level = level;
                pump_steps_info.data[step_index].freq = speed;
                pump_steps_info.data[step_index].loop_num = 1;
                pump_steps_info.data[step_index].duration = time_policy_duration;
                pump_steps_info.data[step_index].policy_type = MOTOR_TIME_POLICY;
                step_index++;
            } else if (mode_type == APP_OLD_COMM_STEP_MODE_LONG) {
                if (!app_old_comm_check_custom_policy_capacity(step_index, 1, APP_OLD_COMM_CID_USER_DEFINE, cab)) {
                    return;
                }
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_LONG;
                pump_steps_info.data[step_index].level = level;
                pump_steps_info.data[step_index].freq = speed;
                pump_steps_info.data[step_index].loop_num = 1;
                pump_steps_info.data[step_index].duration = time_policy_duration;
                pump_steps_info.data[step_index].policy_type = MOTOR_TIME_POLICY;
                step_index++;
            } else if (mode_type == APP_OLD_COMM_STEP_MODE_MIX1) {
                if (!app_old_comm_check_custom_policy_capacity(step_index, 2, APP_OLD_COMM_CID_USER_DEFINE, cab)) {
                    return;
                }
                /// 混合1短吸
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_MIX_SHORT;
                pump_steps_info.data[step_index].level = level;
                pump_steps_info.data[step_index].freq = speed;
                pump_steps_info.data[step_index].loop_num = 5; 
                pump_steps_info.data[step_index].duration = time_policy_duration;
                pump_steps_info.data[step_index].policy_type = MOTOR_TIME_POLICY;
                /// 混合1长吸
                pump_steps_info.data[step_index + 1].mode_type = SUCTION_MODE_MIX_LONG;
                pump_steps_info.data[step_index + 1].level = level;
                pump_steps_info.data[step_index + 1].freq = speed;
                pump_steps_info.data[step_index + 1].loop_num = 1;
                pump_steps_info.data[step_index + 1].duration = 0;
                pump_steps_info.data[step_index + 1].policy_type = MOTOR_TIME_SUB_POLICY;
                step_index += 2;
            } else if (mode_type == APP_OLD_COMM_STEP_MODE_MIX2) {
                if (!app_old_comm_check_custom_policy_capacity(step_index, 2, APP_OLD_COMM_CID_USER_DEFINE, cab)) {
                    return;
                }
                /// 混合2短吸
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_MIX_SHORT;
                pump_steps_info.data[step_index].level = level;
                pump_steps_info.data[step_index].freq = speed;
                pump_steps_info.data[step_index].loop_num = 2; 
                pump_steps_info.data[step_index].duration = time_policy_duration;
                pump_steps_info.data[step_index].policy_type = MOTOR_TIME_POLICY;
                /// 混合2长吸
                pump_steps_info.data[step_index + 1].mode_type = SUCTION_MODE_MIX_LONG;
                pump_steps_info.data[step_index + 1].level = level;
                pump_steps_info.data[step_index + 1].freq = speed;
                pump_steps_info.data[step_index + 1].loop_num = 1;
                pump_steps_info.data[step_index + 1].duration = 0;
                pump_steps_info.data[step_index + 1].policy_type = MOTOR_TIME_SUB_POLICY;
                step_index += 2;
            } else {
                if (!app_old_comm_check_custom_policy_capacity(step_index, 1, APP_OLD_COMM_CID_USER_DEFINE, cab)) {
                    return;
                }
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_IDLE;
                pump_steps_info.data[step_index].level = 0;
                pump_steps_info.data[step_index].freq = 0;
                pump_steps_info.data[step_index].loop_num = 1;
                pump_steps_info.data[step_index].duration = time_policy_duration;
                pump_steps_info.data[step_index].policy_type = MOTOR_TIME_POLICY;
                step_index++;
            }
        pump_steps_info.total_time += time_policy_duration;
		LOG_SRV("step %d mode=%d, level=%d, freq=%d, duration=%d", i, mode_type, level, speed, time_policy_duration);
	}
    pump_steps_info.policy_num = step_index;
    LOG_SRV("total_time=%d", pump_steps_info.total_time);
    LOG_SRV("policy_num=%d", pump_steps_info.policy_num);
    n += 10;
    LOG_SRV("data n = [%d] data len = [%d]", n, cal);

    pump_custom_mode_update_by_app(pump_steps_info,  mode_name); /// 将自定义模式参数存入
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_PUMP)) {
        app_pump_custom_mode_start_handler();
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_USER_DEFINE, 4, cab);
    } else if (wm->is_current_window(wm, ID_WINDOW_DUMMY) || wm->is_current_window(wm, ID_WINDOW_HOME)) {
        LOG_SRV("charge or dummy or home mode, dispatch pump window.");
        //multi_func_msg_data_t msg = {0};
        //msg.union_body.extra.window_id = ID_WINDOW_PUMP;
        //wm->dispatch_new_window_args(wm, &msg);
        POST_TASK_HANDLER_DELAYED_ARG1(app_pump_user_define_delay_task_handler, PUMP_CTRL_TASK_DELAY_TIME, NULL,
             "app_pump_user_define_delay_task");
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_USER_DEFINE, 4, cab);
    } else {
        LOG_SRV("not in pump window");
    }
}

///0xc5  
void app_old_comm_handle_set_trail_user_define_req(uint8_t cal, uint8_t *cab)
{
    uint8_t section_num = 0, speed = 0, mode_type = 0, level = 0;
    uint16_t time_policy_duration = 0;
	pump_mode_steps_info_t pump_steps_info = {0};
    char mode_name[PUMP_MODE_NAME_LEN];   /// 模式名称
    EXCEPTION_CHECK_VOID(cab);
    //memcpy(pump_steps_info.id.dat, cab, 4);
    pump_steps_info.id.dat[0] = cab[0];
    pump_steps_info.id.dat[1] = cab[1];
    pump_steps_info.id.dat[2] = cab[2];
    pump_steps_info.id.dat[3] = cab[3];
	LOG_SRV("cmd ud-mode id = %d", pump_steps_info.id.dat32);
	if (cab[4] <= ROOT_BREAST_PUMP_STEP_MAX) {
		section_num = cab[4];
		pump_steps_info.time_policy_num = section_num;
        LOG_SRV("time_policy_num = %d", pump_steps_info.time_policy_num);
	}
	else { //超出范围，发送NACK
		LOG_SRV("section_num: %d, out of range", section_num);
		app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NACK, APP_OLD_COMM_CID_PARA_DOWNLOAD, 4, cab);
		return;
	}

    ///名称标号
    if ( cab[5] < 10) {  
        snprintf(mode_name, sizeof(mode_name), "0%d", cab[5]);
    } else {
        snprintf(mode_name, sizeof(mode_name), "%d", cab[5]);
    }
    LOG_SRV("mode_name %s", mode_name);

	pump_steps_info.total_time = 0;
    uint8_t step_index = 0;
    uint16_t n = 0;
	for (uint8_t i = 0; i < section_num; i++) {
		n = 6 + 10 * i;
        if (!app_old_comm_check_custom_payload_len(cal, n, APP_OLD_COMM_CID_PARA_DOWNLOAD, cab)) {
            return;
        }
        if (cab[n] > ROOT_BREAST_PUMP_STEP_MAX) {
            LOG_SRV("section index out of range:%d", cab[n]);
            app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NACK, APP_OLD_COMM_CID_PARA_DOWNLOAD, 4, cab);
            return;
        }
        mode_type = cab[n + 1];
        level = cab[n + 2] >= BREAST_PUMP_LEVEL_MAX ? BREAST_PUMP_LEVEL_15 : cab[n + 2];
        speed = (cab[n + 3] - 1) >= BREAST_PUMP_SPEED_MAX ? BREAST_PUMP_SPEED_HIGH : (cab[n + 3] - 1);
        time_policy_duration = cab[n + 4] << 8 | cab[n + 5];
        if (mode_type == APP_OLD_COMM_STEP_MODE_SHORT) {
                if (!app_old_comm_check_custom_policy_capacity(step_index, 1, APP_OLD_COMM_CID_PARA_DOWNLOAD, cab)) {
                    return;
                }
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_SHORT;
                pump_steps_info.data[step_index].level = level;
                pump_steps_info.data[step_index].freq = speed;
                pump_steps_info.data[step_index].loop_num = 1;
                pump_steps_info.data[step_index].duration = time_policy_duration;
                pump_steps_info.data[step_index].policy_type = MOTOR_TIME_POLICY;
                step_index++;
            } else if (mode_type == APP_OLD_COMM_STEP_MODE_LONG) {
                if (!app_old_comm_check_custom_policy_capacity(step_index, 1, APP_OLD_COMM_CID_PARA_DOWNLOAD, cab)) {
                    return;
                }
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_LONG;
                pump_steps_info.data[step_index].level = level;
                pump_steps_info.data[step_index].freq = speed;
                pump_steps_info.data[step_index].loop_num = 1;
                pump_steps_info.data[step_index].duration = time_policy_duration;
                pump_steps_info.data[step_index].policy_type = MOTOR_TIME_POLICY;
                step_index++;
            } else if (mode_type == APP_OLD_COMM_STEP_MODE_MIX1) {
                if (!app_old_comm_check_custom_policy_capacity(step_index, 2, APP_OLD_COMM_CID_PARA_DOWNLOAD, cab)) {
                    return;
                }
                /// 混合1短吸
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_MIX_SHORT;
                pump_steps_info.data[step_index].level = level;
                pump_steps_info.data[step_index].freq = speed;
                pump_steps_info.data[step_index].loop_num = 5; 
                pump_steps_info.data[step_index].duration = time_policy_duration;
                pump_steps_info.data[step_index].policy_type = MOTOR_TIME_POLICY;
                /// 混合1长吸
                pump_steps_info.data[step_index + 1].mode_type = SUCTION_MODE_MIX_LONG;
                pump_steps_info.data[step_index + 1].level = level;
                pump_steps_info.data[step_index + 1].freq = speed;
                pump_steps_info.data[step_index + 1].loop_num = 1;
                pump_steps_info.data[step_index + 1].duration = 0;
                pump_steps_info.data[step_index + 1].policy_type = MOTOR_TIME_SUB_POLICY;
                step_index += 2;
            } else if (mode_type == APP_OLD_COMM_STEP_MODE_MIX2) {
                if (!app_old_comm_check_custom_policy_capacity(step_index, 2, APP_OLD_COMM_CID_PARA_DOWNLOAD, cab)) {
                    return;
                }
                /// 混合2短吸
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_MIX_SHORT;
                pump_steps_info.data[step_index].level = level;
                pump_steps_info.data[step_index].freq = speed;
                pump_steps_info.data[step_index].loop_num = 2; 
                pump_steps_info.data[step_index].duration = time_policy_duration;
                pump_steps_info.data[step_index].policy_type = MOTOR_TIME_POLICY;
                /// 混合2长吸
                pump_steps_info.data[step_index + 1].mode_type = SUCTION_MODE_MIX_LONG;
                pump_steps_info.data[step_index + 1].level = level;
                pump_steps_info.data[step_index + 1].freq = speed;
                pump_steps_info.data[step_index + 1].loop_num = 1;
                pump_steps_info.data[step_index + 1].duration = 0;
                pump_steps_info.data[step_index + 1].policy_type = MOTOR_TIME_SUB_POLICY;
                step_index += 2;
            } else {
                if (!app_old_comm_check_custom_policy_capacity(step_index, 1, APP_OLD_COMM_CID_PARA_DOWNLOAD, cab)) {
                    return;
                }
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_IDLE;
                pump_steps_info.data[step_index].level = 0;
                pump_steps_info.data[step_index].freq = 0;
                pump_steps_info.data[step_index].loop_num = 1;
                pump_steps_info.data[step_index].duration = time_policy_duration;
                pump_steps_info.data[step_index].policy_type = MOTOR_TIME_POLICY;
                step_index++;
            }
        pump_steps_info.total_time += time_policy_duration;
		LOG_SRV("step %d mode=%d, level=%d, freq=%d, duration=%d", i, mode_type, level, speed, time_policy_duration);
	}
    pump_steps_info.policy_num = step_index;
    LOG_SRV("total_time=%d", pump_steps_info.total_time);
    LOG_SRV("policy_num=%d", pump_steps_info.policy_num);
    n += 10;
    LOG_SRV("data n = [%d] data len = [%d]", n, cal);
    pump_custom_trial_mode_update_by_app(pump_steps_info,  mode_name); /// 将自定义模式参数存入
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_PARA_DOWNLOAD, 4, cab);
    /*window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_VOID(wm);
   if (wm->is_current_window(wm, ID_WINDOW_PUMP)) {
        app_pump_custom_trial_mode_start_handler();
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_PARA_DOWNLOAD, 4, cab);
    } else if (wm->is_current_window(wm, ID_WINDOW_CHARGE) || wm->is_current_window(wm, ID_WINDOW_DUMMY)
        ||wm->is_current_window(wm, ID_WINDOW_HOME)) {
        LOG_SRV("charge or dummy or home mode, dispatch pump window.");
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_PUMP;
        wm->dispatch_new_window_args(wm, &msg);
        POST_TASK_HANDLER_DELAYED_ARG1(app_pump_trial_user_define_delay_task_handler, PUMP_CTRL_TASK_DELAY_TIME, NULL,
             "app_pump_user_define_delay_task");
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_PARA_DOWNLOAD, 4, cab);
    } else {
        LOG_SRV("not in pump window");
    }*/
}

//0xf0
void app_old_comm_handle_get_dev_info_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling get device info request");
    if (cal != 0x04) {
        LOG_SRV(" Invalid data length for GET_DEV_INFO (expected 4, got %d)", cal);
        return;
    }
    storage_manager_context_t *ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    factory_dev_info_t factory_dev_info;

    EXCEPTION_CHECK_VOID(ctx);
    EXCEPTION_CHECK_VOID(cab);
    uint8_t ack_data[64] = {0};
    uint16_t data_len = 0;
    app_old_protocol_handler_context_t *handler_ctx = app_old_protocol_handler_ctx_get();

    ack_data[data_len++] = ctx->priv.ver_info.sys_product_id;
    ack_data[data_len++] = ctx->priv.ver_info.soc_version;
    ack_data[data_len++] = ctx->priv.ver_info.hw_version;
    ack_data[data_len++] = ctx->priv.ver_info.sw_version / 100;
    ack_data[data_len++] = ctx->priv.sys_info.mode_info.steps_info[BREAST_PUMP_CUSTOM_MODE].id.dat[0];//custom_mode_id;
    ack_data[data_len++] = ctx->priv.sys_info.mode_info.steps_info[BREAST_PUMP_CUSTOM_MODE].id.dat[1];//custom_mode_id;
    ack_data[data_len++] = ctx->priv.sys_info.mode_info.steps_info[BREAST_PUMP_CUSTOM_MODE].id.dat[2];//custom_mode_id;
    ack_data[data_len++] = ctx->priv.sys_info.mode_info.steps_info[BREAST_PUMP_CUSTOM_MODE].id.dat[3];//custom_mode_id;
    ack_data[data_len++] = 0x00;//milk_full_ver;
    ack_data[data_len++] = 0x00;//milk_full_switch;
    
    ctx->get_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &factory_dev_info);
    ///如果为空 填充0x00
    if (factory_dev_info.sn_len > 0) {
        memcpy(ack_data + data_len, factory_dev_info.sn, factory_dev_info.sn_len);
        data_len += factory_dev_info.sn_len;
    } else {
        memset(ack_data + data_len, 0x00, 35);
        data_len += 35;
    }
#if 1
    ack_data[data_len++] = handler_ctx->encrypt_status;//encrypt_status;
    ack_data[data_len++] = handler_ctx->encrypt_key;//encrypt_key;
#endif
    ack_data[data_len++] = ctx->priv.ver_info.sw_version % 100;
    ack_data[data_len++] = 0x00;//预留
    ack_data[data_len++] = 0x00;//预留
    ack_data[data_len++] = 0x00;//预留
    ack_data[data_len++] = ctx->priv.sys_info.mode_info.steps_info[BREAST_PUMP_CUSTOM_MODE].id.dat[0];
    ack_data[data_len++] = ctx->priv.sys_info.mode_info.steps_info[BREAST_PUMP_CUSTOM_MODE].id.dat[1];
    ack_data[data_len++] = ctx->priv.sys_info.mode_info.steps_info[BREAST_PUMP_CUSTOM_MODE].id.dat[2];
    ack_data[data_len++] = ctx->priv.sys_info.mode_info.steps_info[BREAST_PUMP_CUSTOM_MODE].id.dat[3];

    LOG_SRV(" Sending device info ACK (data len: %d)", data_len);
   app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_DEV_INFO, data_len, ack_data);
}

//0xcf 重置吸奶器状态
void app_old_comm_handle_ctrl_reset_dev_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling device reset request\n");

    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_VOID(pump_ctx);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_RESET_STATUS, 0, NULL);
    /// 先重置pump，再将pump状态设置为空闲
    WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_RESET, 1);
    app_broadcast_pump_status_to_window(BREAST_PUMP_RESET,OLD_BROADCAST_ALL_CT);
    /// 延时发送APP强制完成广播
    POST_TASK_HANDLER_DELAYED_ARG0(app_pump_broadcast_app_completed_delay_task_handler, PUMP_CTRL_TASK_DELAY_TIME,
                                    "app_pump_broadcast_app_completed_delay_task");
}
 
void app_old_comm_handle_get_record_num_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling get record number request");
    
    storage_manager_context_t *ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(ctx);
    
    breast_pump_data_head_t pump_data_head = {0};
    int ret = ctx->get_store_ram_val(STORE_ID_BREAST_PUMP_DATA_HEAD, 0, 0, &pump_data_head);
    if (ret != 0) {
        LOGE("Failed to get pump data head, ret: %d", ret);
        pump_data_head.num = 0;  /// 失败时返回0
    }
    
    /// 将记录数量转换为2字节（低字节在前，高字节在后）
    uint8_t ack_data[2] = {0};
    uint16_t record_num = (uint16_t)pump_data_head.num;
    ack_data[0] = (uint8_t)(record_num & 0xFF);        /// 低字节
    ack_data[1] = (uint8_t)((record_num >> 8) & 0xFF); /// 高字节
    
    LOG_SRV(" Record number: %d (0x%02X 0x%02X)", record_num, ack_data[0], ack_data[1]);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_RECORD_NUM, 2, ack_data);
}

void app_old_comm_handle_get_record_data_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling get record data request");
    
    EXCEPTION_CHECK_VOID(cab);
    if (cal < 2) {
        LOG_SRV(" Invalid data length for GET_RECORD_DATA (expected >= 2, got %d)", cal);
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_RECORD_DATA, 0, NULL);
        return;
    }
    
    /// 解析记录索引（2字节，低字节在前）
    uint16_t num = 0;
    num |= cab[0];
    num |= (uint16_t)(cab[1] << 8);
    
    LOG_SRV(" Request record index: %d", num);
    
    storage_manager_context_t *ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(ctx);
    
    /// 获取泵奶数据头
    breast_pump_data_head_t pump_data_head = {0};
    int ret = ctx->get_store_ram_val(STORE_ID_BREAST_PUMP_DATA_HEAD, 0, 0, &pump_data_head);
    if (ret != 0) {
        LOGE("Failed to get pump data head, ret: %d", ret);
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_RECORD_DATA, 0, NULL);
        return;
    }
    
    /// 验证索引范围（num从1开始，1表示最新的记录）
    if (num == 0 || num > pump_data_head.num) {
        LOG_SRV(" Record index out of range: %d (total: %d)", num, pump_data_head.num);
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_RECORD_DATA, 0, NULL);
        return;
    }
    
    /// 计算循环缓冲区中的实际索引（从tail往前倒推，num=1表示最新的记录）
    /// 统一使用循环缓冲区公式，避免未满情况下的负数问题
    /// num=1 -> tail (最新), num=2 -> tail-1, ..., num=num -> head (最旧)
    uint8_t body_index = (pump_data_head.tail + ROOT_BREAST_PUMP_DATA_BODY_NUM - (num - 1)) % ROOT_BREAST_PUMP_DATA_BODY_NUM;
    
    /// 读取泵奶数据体
    pump_result_info_t pump_data_body = {0};
    ret = ctx->get_store_ram_val(STORE_ID_BREAST_PUMP_DATA_BODY, 0, body_index, &pump_data_body);
    if (ret != 0) {
        LOGE("Failed to read pump data body, ret: %d, index: %d", ret, body_index);
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_RECORD_DATA, 0, NULL);
        return;
    }
    
    /// 转换为BLE记录格式
    app_old_comm_pump_ble_record_t record = {0};
    utc_time_t utc_time = {0};
    timestamp_to_utc_time(pump_data_body.finish_timestamp, &utc_time);
    
    /// 填充日期信息
    record.date.year = utc_time.year % 100;
    record.date.month = utc_time.month;
    record.date.day = utc_time.day;
    
    /// 填充时钟时间
    record.work.clock.hour = utc_time.hour;
    record.work.clock.minute = utc_time.minute;
    record.work.clock.second = utc_time.second;
    
    /// 填充工作记录信息
    record.work.mod = pump_data_body.mode;
    record.work.lvl = 0;  /// TODO: 需要从pump_data_body中获取级别信息
    record.work.time = (uint16_t)(pump_data_body.work_valid_time / 1000);  /// 转换为秒
    record.work.left = 0;   /// TODO: 需要从pump_data_body中获取左侧奶量
    record.work.right = 0;  /// TODO: 需要从pump_data_body中获取右侧奶量
    
    LOG_SRV(" Record data: date=%04d-%02d-%02d, time=%02d:%02d:%02d, mode=%d, duration=%d s", 
            utc_time.year + 2000, utc_time.month, utc_time.day,
            utc_time.hour, utc_time.minute, utc_time.second,
            record.work.mod, record.work.time);
    
    /// 发送响应（11字节：3字节日期 + 8字节工作记录）
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_RECORD_DATA, 
                                                sizeof(app_old_comm_pump_ble_record_t), (uint8_t *)&record);
}

/// 0xE9 清除自定义模式设置参数
void app_old_comm_handle_clear_trail_custom_mode_req(uint8_t cal, uint8_t *cab)
{
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_VOID(storage_ctx);
    EXCEPTION_CHECK_VOID(pump_ctx);
    EXCEPTION_CHECK_VOID(cab);

    // 检查参数长度
    if (cal != 4) {
        LOG_SRV("Invalid parameter length: %d, expected 4", cal);
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NACK, APP_OLD_COMM_CID_CLEAR_CUSTOM_MODE, 4, cab);
        return;
    }

    // 读取自定义模式ID（与现有代码保持一致的方式）
    pump_mode_steps_info_t clear_steps_info = {0};
    clear_steps_info.id.dat[0] = cab[0];
    clear_steps_info.id.dat[1] = cab[1];
    clear_steps_info.id.dat[2] = cab[2];
    clear_steps_info.id.dat[3] = cab[3];
    uint32_t clear_mode_id = clear_steps_info.id.dat32;
    LOG_SRV("Clear custom mode request, mode_id=0x%08X (%u)", clear_mode_id, clear_mode_id);

    // 获取当前自定义模式ID
    pump_mode_steps_info_t current_steps_info = {0};
    storage_ctx->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_TRIAL_MODE, 0, &current_steps_info);
    uint32_t current_mode_id = current_steps_info.id.dat32;

    // 不做限制，直接清除
    bool should_clear = true;
    LOG_SRV("Mode ID : request=0x%08X, current=0x%08X", clear_mode_id, current_mode_id);

    if (should_clear) {
        // 清除自定义模式步骤信息
        pump_mode_steps_info_t empty_steps_info = {0};
        storage_ctx->put_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_TRIAL_MODE, 0, &empty_steps_info);
        
        // 清除自定义模式名称
        char empty_name[PUMP_MODE_NAME_LEN] = {0};
        storage_ctx->put_store_ram_val(STORE_ID_PUMP_MODE_NAME_INFO, BREAST_PUMP_CUSTOM_TRIAL_MODE, 0, empty_name);
        
        // 清除自定义模式设置信息
        pump_setting_t empty_setting = {0};
        for (uint8_t i = 0; i < ROOT_BREAST_PUMP_STEP_MAX; i++) {
            storage_ctx->put_store_ram_val(STORE_ID_PUMP_SETTING_INFO, BREAST_PUMP_CUSTOM_TRIAL_MODE, i, &empty_setting);
        }
        
        // 清除试用模式激活标识
        WRITE_PUMP_MODE_ACTIVATE(pump_ctx, BREAST_PUMP_CUSTOM_MODE, 0);
        
        // 清除更新标志
        storage_ctx->put_store_ram_val(STORE_ID_MODE_NEW_STEP_INFO_FLAG, BREAST_PUMP_CUSTOM_TRIAL_MODE, false, 0);
        
        LOG_SRV("Custom mode parameters cleared successfully");
    }

    // 发送ACK响应，返回清除的自定义模式ID（4字节）
    uint8_t ack_data[4] = {0};
    ack_data[0] = cab[0];
    ack_data[1] = cab[1];
    ack_data[2] = cab[2];
    ack_data[3] = cab[3];
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_CLEAR_CUSTOM_MODE, 4, ack_data);
}

//0xEA
void app_old_comm_handle_set_bilateral_pump_param_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV("Handling set bilateral pump param request");
    EXCEPTION_CHECK_VOID(cab);
    
    // 检查参数长度
    if (cal != 0x04) {
        LOG_SRV("Invalid data length for SET_BILATERAL_PUMP_PARAM (expected 0x04, got 0x%02X)", cal);
        uint8_t nack_data = 0x00; // 错误状态
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NACK, APP_OLD_COMM_CID_SET_BILATERAL_PUMP_PARAM, 1, &nack_data);
        return;
    }
    
    // 解析参数
    uint8_t left_level = cab[0];   // 左边档位 0x00~0x0F
    uint8_t left_speed = cab[1];   // 左边频次 1-低, 2-中, 3-高
    uint8_t right_level = cab[2];  // 右边档位 0x00~0x0F
    uint8_t right_speed = cab[3];  // 右边频次 1-低, 2-中, 3-高
    
    // 参数有效性检查
    if (left_level > BREAST_PUMP_LEVEL_15 || right_level > BREAST_PUMP_LEVEL_15) {
        LOG_SRV("Invalid pump level: left=%d, right=%d (max=%d)", left_level, right_level, BREAST_PUMP_LEVEL_15);
        uint8_t nack_data = 0x00; // 错误状态
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NACK, APP_OLD_COMM_CID_SET_BILATERAL_PUMP_PARAM, 1, &nack_data);
        return;
    }
    
    if (left_speed < 1 || left_speed > 3 || right_speed < 1 || right_speed > 3) {
        LOG_SRV("Invalid pump speed: left=%d, right=%d (valid: 1-3)", left_speed, right_speed);
        uint8_t nack_data = 0x00; // 错误状态
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NACK, APP_OLD_COMM_CID_SET_BILATERAL_PUMP_PARAM, 1, &nack_data);
        return;
    }

    // 先发送ACK响应，返回设置状态 0x01（成功）
    uint8_t ack_data = 0x01; // 设置成功
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_SET_BILATERAL_PUMP_PARAM, 1, &ack_data);
    
    // 转换频次：协议中 1-低, 2-中, 3-高 -> 内部 0-低, 1-中, 2-高
    uint8_t left_speed_internal = left_speed - 1;
    uint8_t right_speed_internal = right_speed - 1;
    
    // 获取泵奶管理器
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_VOID(pump_ctx);
    
    // 读取当前设置
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(pump_ctx, time_index);
    pump_setting_t setting;
    READ_PUMP_SETTING_INFO(pump_ctx, time_index, setting);

    int control_type = BREAST_PUMP_CONTROL_TYPE_BILATERAL;
    WRITE_PUMP_LEVEL_CONTROL_TYPE(pump_ctx, control_type, 0);
   
     /*led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    if (led && led->hook && led->hook->set_key_backlight) {
        led->hook->set_key_backlight(led, 100);
    }*/
    ///设置不变的参数 直接return
    if (setting.level[0] == left_level && setting.level[1] == right_level && setting.speed == left_speed_internal) {
        LOG_SRV("Bilateral pump param no change, left_level=%d, right_level=%d, left_speed=%d", left_level, right_level, left_speed_internal);
        return;
    }
    // 设置左右两边的档位和频次
    setting.level[0] = left_level;
    setting.level[1] = right_level;
    // 注意：setting.speed 是全局的，但根据协议，这里应该分别设置左右两边的频次
    // 由于当前结构体只有一个speed字段，我们使用左边频次作为全局speed
    // 如果需要分别控制，可能需要修改数据结构或使用其他方式
    setting.speed = left_speed_internal;
    
    LOG_SRV("Set bilateral pump param: left_level=%d, left_speed=%d(%d), right_level=%d, right_speed=%d(%d)", 
            left_level, left_speed, left_speed_internal, right_level, right_speed, right_speed_internal);

    // 写入设置
    WRITE_PUMP_SETTING_INFO(pump_ctx, time_index, true, setting);
    
    // 广播更新
    app_broadcast_pump_level_to_window(left_level, OLD_BROADCAST_LEFT_CT);
    app_broadcast_pump_level_to_window(right_level, OLD_BROADCAST_RIGHT_CT);
    app_broadcast_pump_speed_to_window(left_speed_internal, OLD_BROADCAST_ALL_CT);
    
}