#include "plat_types.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "app_protocol_handler.h"
#include "app_protocol_pump_ctrl_server.h"
#include "breast_pump_manager_api.h"
#include "pump_mode_default_config.h"
#include "breast_pump_mode.h"
#include "pump_mode_default_config.h"
#include "app_protocol_broadcast_to_window.h"
#include "breast_pump_persist_data.h"
/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_CTRL_TASK_DELAY_TIME 1000
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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

static uint8_t app_old_comm_get_adaptive_dimming_switch(void)
{
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_UINT(led && led->hook && led->hook->get_night_light_adaptive_switch);
    return led->hook->get_night_light_adaptive_switch(led);
}

static uint8_t app_old_comm_get_night_light_level(void)
{
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_UINT(led && led->hook && led->hook->get_night_light_level);
    return led->hook->get_night_light_level(led);
}

static uint8_t app_old_comm_get_night_light_onoff(void)
{
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_UINT(led && led->hook && led->hook->get_night_light_onoff);
    return led->hook->get_night_light_onoff(led);
}

static uint8_t app_old_comm_get_sys_status(void)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_UINT(pbus);
    struct rt_device_node *battery = pbus->find_device(ID_DEVICE_BATTERY);
    uint8_t chage_state = 0;
    if (battery) {
        chage_state = DEVICE_IOCTL(battery, OP_CODE_BAT_GET_CHARGE_STATUS);
    } else {
        chage_state = CHARGE_STATE_UNCHARGE;
    }
    int pump_status = pump_ctx->get_breast_pump_status();
    int pump_mode = pump_ctx->get_breast_pump_mode();
    uint8_t sys_status = APP_OLD_COMM_SYS_STOP;

    if (BATTERY_IS_CHARGING(chage_state)) {
        LOG_SRV("APP_OLD_COMM_SYS_CHANRGE");
        sys_status = APP_OLD_COMM_SYS_CHANRGE;
    }

    if (BREAST_PUMP_START == pump_status || BREAST_PUMP_CONTINUE == pump_status) {
        LOG_SRV("APP_OLD_COMM_SYS_RUN");
        sys_status = APP_OLD_COMM_SYS_RUN;
    }

    if ((BREAST_PUMP_START == pump_status || BREAST_PUMP_CONTINUE == pump_status)
        && BATTERY_IS_CHARGING(chage_state)) {
        LOG_SRV("APP_OLD_COMM_PUMP_RUN_IN_CHARGE");
        sys_status = APP_OLD_COMM_PUMP_RUN_IN_CHARGE;
    }

    if (BREAST_PUMP_IDLE == pump_status || BREAST_PUMP_PAUSE == pump_status
        || BREAST_PUMP_FINISH == pump_status) {
        LOG_SRV("APP_OLD_COMM_SYS_STOP");
        sys_status = APP_OLD_COMM_SYS_STOP;
    }

    if ((BREAST_PUMP_IDLE == pump_status || BREAST_PUMP_PAUSE == pump_status || BREAST_PUMP_FINISH == pump_status)
        && BATTERY_IS_CHARGING(chage_state)) {
        LOG_SRV("APP_OLD_COMM_PUMP_STOP_IN_CHARGE");
        sys_status = APP_OLD_COMM_PUMP_STOP_IN_CHARGE;
    }

    if (BREAST_PUMP_RESET == pump_status) {
        LOG_SRV("APP_OLD_COMM_SYS_RESET");
        sys_status = APP_OLD_COMM_SYS_RESET;
    }

    return sys_status;
}

POSSIBLY_UNUSED static uint8_t app_old_comm_get_pump_running_status(void)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    LOG_SRV("pump_ctx->is_on_working =%d", pump_ctx->is_on_working());
    return pump_ctx->is_on_working();
}

static uint8_t app_old_comm_get_battery_percent(void)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_UINT(pbus);
    struct rt_device_node *battery = pbus->find_device(ID_DEVICE_BATTERY);
    uint8_t percent = 0;
    if (battery) {
        DEVICE_READ_ARGS(battery, &percent, sizeof(percent));
    } else {
        percent = 100;
    }
    LOG_SRV("percent=%d", percent);
    return percent;
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
    int pump_mode = pump_ctx->get_breast_pump_mode();
    LOG_SRV("pump mode: %s", pump_ctx->get_breast_pump_mode_name(pump_mode));
    return (uint8_t)pump_mode;
}

static uint8_t app_old_comm_get_current_custom_pump_step_mode(servers_old_protocol_publish_data_id_e id)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);

    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    /// 当前不是运行的不是自定义模式步骤，默认上报第一步参数,启动自定义模式从第一步开始
    if (pump_ctx->get_breast_pump_mode() != BREAST_PUMP_CUSTOM_MODE) {
        time_index = 0;
    }
    /// 获取自定义模式参数
    uint8_t time_step_mode = APP_OLD_COMM_MODE_SHORT;
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    pump_mode_steps_info_t steps_info ={0};
    storage_ctx->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_MODE, 0, &steps_info);
    uint8_t time_step_index = 0;    
    for (uint8_t i = 0; i < steps_info.policy_num; i++) {
        if (steps_info.data[i].policy_type == MOTOR_TIME_POLICY) {
            if (time_step_index == time_index) {
                if (steps_info.data[i].mode_type == SUCTION_MODE_SHORT) {
                    time_step_mode = APP_OLD_COMM_MODE_SHORT;
                } else if (steps_info.data[i].mode_type == SUCTION_MODE_LONG) {
                    time_step_mode = APP_OLD_COMM_MODE_LONG;
                } else if (steps_info.data[i].mode_type == SUCTION_MODE_MIX_SHORT || 
                    steps_info.data[i].mode_type == SUCTION_MODE_MIX_LONG) {
                    time_step_mode = steps_info.data[i].loop_num == 5 ? APP_OLD_COMM_MODE_MIX1 : APP_OLD_COMM_MODE_MIX2;
                } else {
                    time_step_mode = APP_OLD_COMM_MODE_RELAX;
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
    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    ///当前系统在处于自定义模式步骤，应该上报什么，就默认上报吸乳第一步参数
    pump_setting_t setting;
    if (pump_ctx->get_breast_pump_mode() == BREAST_PUMP_CUSTOM_MODE) {
        //app bug,在自定义模式下，常规level需要上报自定义模式运行的level
        /*
        storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        EXCEPTION_CHECK_UINT(storage_ctx);
        */
        pump_ctx->get_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index);
        LOG_SRV("setting.level [%d] =%d", lr_ct, setting.level[lr_ct]);
        return (uint8_t)setting.level[lr_ct];
    } else {
        //
        pump_ctx->get_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index);
        LOG_SRV("setting.level [%d] =%d", lr_ct, setting.level[lr_ct]);
        return (uint8_t)setting.level[lr_ct];
    }
}

static uint8_t app_old_comm_get_current_custom_pump_level(app_old_commd_l_r_ctrl_e lr_ct, servers_old_protocol_publish_data_id_e id)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    pump_setting_t setting;
    /// 当前系统在处于自定义模式步骤
    if (pump_ctx->get_breast_pump_mode() == BREAST_PUMP_CUSTOM_MODE) {
        pump_ctx->get_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index);
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
    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    /// 当前系统在处于自定义模式步骤，应该上报什么，就默认上报吸乳第一步参数
    pump_setting_t setting;
    if (pump_ctx->get_breast_pump_mode() == BREAST_PUMP_CUSTOM_MODE) {
        storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        EXCEPTION_CHECK_UINT(storage_ctx);
        storage_ctx->get_store_ram_val(STORE_ID_PUMP_SETTING_INFO, BREAST_PUMP_LACTATION_MODE, 0, &setting);
        LOG_SRV("setting.speed =%d", setting.speed);
        return (uint8_t)setting.speed;
    } else {
        pump_ctx->get_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index);
        LOG_SRV("setting.speed =%d", setting.speed);
        return (uint8_t)setting.speed;
    }
}

static uint8_t app_old_comm_get_current_custom_pump_speed(servers_old_protocol_publish_data_id_e id)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    /// 当前不是运行的不是自定义模式步骤，默认上报第一步参数,启动自定义模式从第一步开始
    if (pump_ctx->get_breast_pump_mode() != BREAST_PUMP_CUSTOM_MODE) {
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
    pump_result_info_t *pumping_info = pump_ctx->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
    LOG_SRV("run time = %d", pumping_info->current_mode_work_time);
    return (uint32_t)pumping_info->current_mode_work_time/1000;
}

static uint32_t app_old_comm_get_pump_mode_run_total_time(void)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    pump_result_info_t *pumping_info = pump_ctx->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
    LOG_SRV("total time = %d", pumping_info->work_total_time);
    return (uint32_t)pumping_info->work_total_time/1000;
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
}

static uint8_t pump_custom_trial_mode_update_by_app(pump_mode_steps_info_t pump_steps_info)
{
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    motor_manager_context_t *motor_ctx = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    EXCEPTION_CHECK_UINT(storage_ctx);
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
    return true;
}

static uint8_t app_old_comm_set_nommal_pump_level(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    pump_setting_t setting; /// 当前泵奶设置信息.
    pump_ctx->get_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index);
    if (app_old_comm_pump_ctrl_date->pump_mode_lr_or_all == APP_OLD_COMM_LEFT_CT) {
        if (app_old_comm_pump_ctrl_date->normal_pump_level >= 0) {
            setting.level[0] = app_old_comm_pump_ctrl_date->normal_pump_level;
            LOG_SRV("left pump level %d",setting.level[0]);
            app_broadcast_pump_level_to_window(setting.level[0], OLD_BROADCAST_LEFT_CT);
        }
    } else if (app_old_comm_pump_ctrl_date->pump_mode_lr_or_all == APP_OLD_COMM_RIGHT_CT) {
        if (app_old_comm_pump_ctrl_date->normal_pump_level <= BREAST_PUMP_LEVEL_15) {
            setting.level[1] = app_old_comm_pump_ctrl_date->normal_pump_level;
            LOG_SRV("right pump level %d",setting.level[1]);
            app_broadcast_pump_level_to_window(setting.level[1], OLD_BROADCAST_RIGHT_CT);
        }
    } else {
        if (app_old_comm_pump_ctrl_date->normal_pump_level >= 0
            && app_old_comm_pump_ctrl_date->normal_pump_level <= BREAST_PUMP_LEVEL_15) {
            setting.level[1] = app_old_comm_pump_ctrl_date->normal_pump_level;
            setting.level[0] = app_old_comm_pump_ctrl_date->normal_pump_level;
            LOG_SRV("all pump level %d, %d", setting.level[0], setting.level[1]);
            app_broadcast_pump_level_to_window(setting.level[0], OLD_BROADCAST_LEFT_CT);
            app_broadcast_pump_level_to_window(setting.level[1], OLD_BROADCAST_RIGHT_CT);
        }
    }
    pump_ctx->set_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index, true);
    return true;
}

static uint8_t app_old_comm_set_nommal_pump_speed(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    pump_setting_t setting; /// 当前泵奶设置信息.
    pump_ctx->get_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index);
    //只允许常规模式设置 speed，并且左右同步控制
    setting.speed = app_old_comm_pump_ctrl_date->normal_pump_speed;
    LOG_SRV("pump speed  %d", setting.speed);
    pump_ctx->set_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index, true);
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
    pump->set_breast_pump_control_type(control_type);
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    if (led && led->hook && led->hook->set_key_backlight) {
        led->hook->set_key_backlight(led, 100);
    }
    LOG_SRV("set pump control type %d", control_type);
    return true;
}

static uint8_t app_old_comm_get_pump_control_type(void)
{
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump);
    uint8_t control_type = pump->get_breast_pump_control_type();
    LOG_SRV("get pump control type %d", control_type);
    return control_type;
}

static uint8_t app_old_comm_set_custom_pump_level(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    pump_setting_t setting; /// 当前泵奶设置信息.
    pump_ctx->get_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index);
    if (app_old_comm_pump_ctrl_date->pump_mode_lr_or_all == APP_OLD_COMM_LEFT_CT) {
        if (app_old_comm_pump_ctrl_date->user_define_pump_level >= 0) {
            setting.level[0] = app_old_comm_pump_ctrl_date->user_define_pump_level;
            LOG_SRV("left pump level %d",setting.level[0]);
            app_broadcast_pump_level_to_window(setting.level[0] ,OLD_BROADCAST_LEFT_CT);
        }
    } else if (app_old_comm_pump_ctrl_date->pump_mode_lr_or_all == APP_OLD_COMM_RIGHT_CT) {
        if (app_old_comm_pump_ctrl_date->user_define_pump_level <= BREAST_PUMP_LEVEL_15) {
            setting.level[1] = app_old_comm_pump_ctrl_date->user_define_pump_level;
            LOG_SRV("right pump level %d",setting.level[1]);
            app_broadcast_pump_level_to_window(setting.level[1] ,OLD_BROADCAST_RIGHT_CT);
        }
    } else {
        if (app_old_comm_pump_ctrl_date->user_define_pump_level >= 0
            && app_old_comm_pump_ctrl_date->user_define_pump_level <= BREAST_PUMP_LEVEL_15) {
            setting.level[1] = app_old_comm_pump_ctrl_date->user_define_pump_level;
            setting.level[0] = app_old_comm_pump_ctrl_date->user_define_pump_level;
            LOG_SRV("all pump level %d, %d", setting.level[0], setting.level[1]);
            app_broadcast_pump_level_to_window(setting.level[0] ,OLD_BROADCAST_LEFT_CT);
            app_broadcast_pump_level_to_window(setting.level[1] ,OLD_BROADCAST_RIGHT_CT);
        }
    }

    pump_ctx->set_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index, true);
    return true;
}

static uint8_t app_old_comm_pump_control_handler(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
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
            case BREAST_PUMP_MIX_MODE: {       /// 常规的吸乳模式允许调频率
                app_old_comm_set_nommal_pump_speed(app_old_comm_pump_ctrl_date);
            }
            case BREAST_PUMP_REGULAR_PUMPING_MODE:
            case BREAST_PUMP_EMPTYING_BOOST_MODE:
            case BREAST_PUMP_PP_MILK_TRACK_MODE:
            case BREAST_PUMP_MILK_BOOST_MODE:
            case BREAST_PUMP_LACTATION_START_MODE: {
                app_old_comm_set_nommal_pump_level(app_old_comm_pump_ctrl_date);
                break;
            }
            case BREAST_PUMP_CUSTOM_MODE: {
                app_old_comm_set_custom_pump_level(app_old_comm_pump_ctrl_date); //设置运行中的参数
                //todo                                                          //将缓存的cust para也更新下
                break;
            }
            default:
                break;
        }
    } else {
        /// 如果模式改变，则改变模式,忽略参数
        pump_ctx->set_breast_pump_mode(app_old_comm_pump_ctrl_date->pump_mode, 1);
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
    int pump_status = pump_ctx->get_breast_pump_status();
    if (pump_status == BREAST_PUMP_FINISH || pump_status == BREAST_PUMP_IDLE) {
        if (status == APP_OLD_COMM_SYS_RUN) {
            pump_ctx->set_breast_pump_status(BREAST_PUMP_START);
            app_broadcast_pump_status_to_window(BREAST_PUMP_START, OLD_BROADCAST_ALL_CT);
        }
    } else if (pump_status == BREAST_PUMP_START || pump_status == BREAST_PUMP_CONTINUE) {
        if (status == APP_OLD_COMM_SYS_STOP) {
            pump_ctx->set_breast_pump_status(BREAST_PUMP_PAUSE);
            app_broadcast_pump_status_to_window(BREAST_PUMP_PAUSE,OLD_BROADCAST_ALL_CT);
        }
    } else if (pump_status == BREAST_PUMP_PAUSE) {
        if (status == APP_OLD_COMM_SYS_RUN) {
            pump_ctx->set_breast_pump_status(BREAST_PUMP_CONTINUE);
            app_broadcast_pump_status_to_window(BREAST_PUMP_CONTINUE,OLD_BROADCAST_ALL_CT);
        }
    }
    return true;
}

static void app_pump_ctrl_delay_task_handler(hdr_post_args_t *args)
{
    int status = (int)args->arg1;
    app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date = (app_old_comm_pump_ctrl *)args->user_data;
    EXCEPTION_CHECK_VOID(app_old_comm_pump_ctrl_date);
    app_old_comm_pump_control_handler(app_old_comm_pump_ctrl_date);
    app_old_comm_pump_run_status_ctrl(status);
    LOG_SRV("status: %d", status);
}

static void app_pump_custom_trial_mode_start_handler(void)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_VOID(pump_ctx);
    if (app_old_comm_get_current_pump_mode() == BREAST_PUMP_CUSTOM_TRIAL_MODE && pump_ctx->is_on_working()) {
        pump_ctx->set_breast_pump_status(BREAST_PUMP_PAUSE);
        app_broadcast_pump_status_to_window(BREAST_PUMP_PAUSE, OLD_BROADCAST_ALL_CT);
        pump_ctx->set_breast_pump_status(BREAST_PUMP_START);
        app_broadcast_pump_status_to_window(BREAST_PUMP_START, OLD_BROADCAST_ALL_CT);
    }
}

static void app_pump_user_define_delay_task_handler(hdr_post_args_t *args)
{
    app_pump_custom_trial_mode_start_handler();
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

    report_data[data_len++] = app_old_comm_get_battery_percent();
    report_data[data_len++] = app_old_comm_get_sys_status();
    report_data[data_len++] = lr_ct;
    report_data[data_len++] = app_old_comm_get_current_pump_mode() + 1; //0x01~0x05（按摩-吸乳-混合-自定义-智能吸乳）
    report_data[data_len++] = app_old_comm_get_current_mode_pump_level(lr_ct, id); //0x01~0x0F（1档~15档）
    report_data[data_len++] = app_old_comm_get_current_custom_pump_step_mode(id);
    report_data[data_len++] = app_old_comm_get_current_custom_pump_level(lr_ct, id); //0x01~0x0F（1档~15档）
    report_data[data_len++] = app_old_comm_get_current_custom_pump_speed(id) + 1;     //频次（1-低 2-中 3-高）（1字节）
    report_data[data_len++] = (app_old_comm_get_pump_mode_run_total_time() >> 0) & 0xFF; //单位s
    report_data[data_len++] = (app_old_comm_get_pump_mode_run_total_time() >> 8) & 0xFF; //单位s
    report_data[data_len++] = (app_old_comm_get_current_pump_mode_run_time() >> 0) & 0xFF;//单位s
    report_data[data_len++] = (app_old_comm_get_current_pump_mode_run_time() >> 8) & 0xFF;//单位s
    report_data[data_len++] = 0; //v3无储奶检测
    report_data[data_len++] = app_old_comm_check_if_low_battery();
    report_data[data_len++] = (app_old_comm_get_current_custom_pump_mode_total_time() >> 0) & 0xFF;
    report_data[data_len++] = (app_old_comm_get_current_custom_pump_mode_total_time() >> 8) & 0xFF;
    report_data[data_len++] = app_old_comm_get_system_is_airplane_mode();
    report_data[data_len++] = app_old_comm_get_current_mode_pump_speed(id); //normal speed //频次（0-低 1-中 2-高）（1字节）
    report_data[data_len++] = 0x00; //预留
    report_data[data_len++] = 0x00; //预留
    report_data[data_len++] = app_old_comm_get_night_light_onoff();
    report_data[data_len++] = app_old_comm_get_night_light_level();
    report_data[data_len++] = app_old_comm_get_adaptive_dimming_switch();
    report_data[data_len++] = app_old_comm_get_pump_control_type();
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NOTIFY, APP_OLD_COMM_CID_NOTIFY_STATUS, data_len, report_data);
}

//0xb0
void app_old_comm_handle_get_status_req(app_old_commd_l_r_ctrl_e lr_ct)
{
    LOG_SRV(" Handling get device status request");
    uint8_t ack_data[45] = {0};
    uint16_t data_len = 0;

    ack_data[data_len++] = app_old_comm_get_battery_percent();
    ack_data[data_len++] = app_old_comm_get_sys_status();
    ack_data[data_len++] = lr_ct;
    ack_data[data_len++] = app_old_comm_get_current_pump_mode() + 1;
    ack_data[data_len++] = app_old_comm_get_current_mode_pump_level(lr_ct, OLD_PUBLISH_DATA_POLL_REPORT);
    ack_data[data_len++] = app_old_comm_get_current_custom_pump_step_mode(OLD_PUBLISH_DATA_POLL_REPORT);
    ack_data[data_len++] = app_old_comm_get_current_custom_pump_level(lr_ct, OLD_PUBLISH_DATA_POLL_REPORT);
    ack_data[data_len++] = app_old_comm_get_current_custom_pump_speed(OLD_PUBLISH_DATA_POLL_REPORT) + 1;
    ack_data[data_len++] = (app_old_comm_get_pump_mode_run_total_time() >> 0) & 0xFF;
    ack_data[data_len++] = (app_old_comm_get_pump_mode_run_total_time() >> 8) & 0xFF;
    ack_data[data_len++] = (app_old_comm_get_current_pump_mode_run_time() >> 0) & 0xFF;
    ack_data[data_len++] = (app_old_comm_get_current_pump_mode_run_time() >> 8) & 0xFF;
    ack_data[data_len++] = 0; //v3无储奶检测
    ack_data[data_len++] = app_old_comm_check_if_low_battery();
    ack_data[data_len++] = (app_old_comm_get_current_custom_pump_mode_total_time() >> 0) & 0xFF;
    ack_data[data_len++] = (app_old_comm_get_current_custom_pump_mode_total_time() >> 8) & 0xFF;
    ack_data[data_len++] = app_old_comm_get_system_is_airplane_mode();
    ack_data[data_len++] = app_old_comm_get_current_mode_pump_speed(OLD_PUBLISH_DATA_POLL_REPORT); //normal speed
    ack_data[data_len++] = 0x00; //预留
    ack_data[data_len++] = 0x00; //预留
    ack_data[data_len++] = app_old_comm_get_night_light_onoff();
    ack_data[data_len++] = app_old_comm_get_night_light_level();
    ack_data[data_len++] = app_old_comm_get_adaptive_dimming_switch();
    ack_data[data_len++] = app_old_comm_get_pump_control_type();
    LOG_SRV(" Sending device status ACK (data len: %d)", data_len);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_STATUS, data_len, ack_data);
}

//0xc0
void app_old_comm_handle_ctrl_dev_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling device control request");
    EXCEPTION_CHECK_VOID(cab);
    if (cal < 0x0A) {
        LOG_SRV(" Invalid data length for CTRL_DEV (expected >=0X0A, got %d)", cal);
    }
    // TODO: Send parameters to pump valve component
    app_old_comm_pump_ctrl app_old_comm_pump_ctrl_date;
    app_old_comm_pump_ctrl_date.pump_mode = cab[1] - 1; //0x01~0x05（按摩-吸乳-混合-自定义-智能吸乳）
    app_old_comm_pump_ctrl_date.normal_pump_level = cab[2];
    app_old_comm_pump_ctrl_date.user_define_pump_step_mode = cab[3];
    app_old_comm_pump_ctrl_date.user_define_pump_level = cab[4];
    app_old_comm_pump_ctrl_date.user_define_pump_speed = cab[5] - 1; //频次（1-低 2-中 3-高）（1字节）
    app_old_comm_pump_ctrl_date.pump_mode_lr_or_all = cab[6];//0:左 1:右 2:双
    app_old_comm_pump_ctrl_date.normal_pump_speed = cab[7]; //频次（0-低 1-中 2-高）（1字节）
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
    EXCEPTION_CHECK_VOID(cab);
    memcpy(pump_steps_info.id.dat, cab, 4);
	LOG_SRV("cmd ud-mode id = %d", pump_steps_info.id.dat32);
	if (cab[4] <= ROOT_BREAST_PUMP_STEP_MAX) {
		section_num = cab[4];
		pump_steps_info.time_policy_num = section_num;
        LOG_SRV("time_policy_num = %d", pump_steps_info.time_policy_num);
	}
	else { //超出范围，发送NACK
		LOG_SRV("section_num: %d, out of range", section_num);
		app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NACK, APP_OLD_COMM_CID_USER_DEFINE, 0, NULL);
		return;
	}
	pump_steps_info.total_time = 0;
    uint8_t step_index = 0, n = 0;
	for (uint8_t i = 0; i < section_num; i++) {
		n = 5 + 10 * i;
		if (cab[n] <= ROOT_BREAST_PUMP_STEP_MAX) {
            mode_type = cab[n + 1];
            level = cab[n + 2] >= BREAST_PUMP_LEVEL_MAX ? BREAST_PUMP_LEVEL_15 : cab[n + 2];
            speed = (cab[n + 3] - 1) >= BREAST_PUMP_SPEED_MAX ? BREAST_PUMP_SPEED_HIGH : (cab[n + 3] - 1);
            time_policy_duration = cab[n + 4] << 8 | cab[n + 5];
		    if (mode_type == APP_OLD_COMM_MODE_SHORT) {
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_SHORT;
                pump_steps_info.data[step_index].level = level;
                pump_steps_info.data[step_index].freq = speed;
                pump_steps_info.data[step_index].loop_num = 1;
                pump_steps_info.data[step_index].duration = time_policy_duration;
                step_index++;
            } else if (mode_type == APP_OLD_COMM_MODE_LONG) {
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_LONG;
                pump_steps_info.data[step_index].level = level;
                pump_steps_info.data[step_index].freq = speed;
                pump_steps_info.data[step_index].loop_num = 1;
                pump_steps_info.data[step_index].duration = time_policy_duration;
                step_index++;
            } else if (mode_type == APP_OLD_COMM_MODE_MIX1) {
                /// 混合1短吸
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_MIX_SHORT;
                pump_steps_info.data[step_index].level = level;
                pump_steps_info.data[step_index].freq = speed;
                pump_steps_info.data[step_index].loop_num = 5; 
                pump_steps_info.data[step_index].duration = time_policy_duration;
                /// 混合1长吸
                pump_steps_info.data[step_index + 1].mode_type = SUCTION_MODE_MIX_LONG;
                pump_steps_info.data[step_index + 1].level = level;
                pump_steps_info.data[step_index + 1].freq = speed;
                pump_steps_info.data[step_index + 1].loop_num = 1;
                pump_steps_info.data[step_index + 1].duration = 0;
                step_index += 2;
            } else if (mode_type == APP_OLD_COMM_MODE_MIX2) {
                /// 混合2短吸
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_MIX_SHORT;
                pump_steps_info.data[step_index].level = level;
                pump_steps_info.data[step_index].freq = speed;
                pump_steps_info.data[step_index].loop_num = 2; 
                pump_steps_info.data[step_index].duration = time_policy_duration;
                /// 混合2长吸
                pump_steps_info.data[step_index + 1].mode_type = SUCTION_MODE_MIX_LONG;
                pump_steps_info.data[step_index + 1].level = level;
                pump_steps_info.data[step_index + 1].freq = speed;
                pump_steps_info.data[step_index + 1].loop_num = 1;
                pump_steps_info.data[step_index + 1].duration = 0;
                step_index += 2;
            } else {
                pump_steps_info.data[step_index].mode_type = SUCTION_MODE_IDLE;
                pump_steps_info.data[step_index].level = 0;
                pump_steps_info.data[step_index].freq = 0;
                pump_steps_info.data[step_index].loop_num = 1;
                pump_steps_info.data[step_index].duration = time_policy_duration;
                step_index++;
            }
			pump_steps_info.total_time += time_policy_duration;
		}
		LOG_SRV("step %d mode=%d, level=%d, freq=%d, duration=%d", i, mode_type, level, speed, time_policy_duration);
	}
    LOG_SRV("total_time=%d", pump_steps_info.total_time);
    n += 10;
    LOG_SRV("data n = [%d] data len = [%d]", n, cal);
    pump_custom_trial_mode_update_by_app(pump_steps_info); /// 将自定义模式参数存入
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_PUMP)) {
        app_pump_custom_trial_mode_start_handler();
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_USER_DEFINE, 0, NULL);
    } else if (wm->is_current_window(wm, ID_WINDOW_CHARGE) || wm->is_current_window(wm, ID_WINDOW_DUMMY)
        ||wm->is_current_window(wm, ID_WINDOW_HOME)) {
        LOG_SRV("charge or dummy or home mode, dispatch pump window.");
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_PUMP;
        wm->dispatch_new_window_args(wm, &msg);
        POST_TASK_HANDLER_DELAYED_ARG1(app_pump_user_define_delay_task_handler, PUMP_CTRL_TASK_DELAY_TIME, NULL,
             "app_pump_user_define_delay_task");
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_USER_DEFINE, 0, NULL);
    } else {
        LOG_SRV("not in pump window");
    }
}

//0xc5
void app_old_comm_handle_set_user_define_wr_flash_req(uint8_t cal, uint8_t *cab)
{
    // 复制BREAST_PUMP_CUSTOM_TRIAL_MODE的数据到BREAST_PUMP_CUSTOM_MODE
    pump_copy_trial_mode_to_custom_mode();

    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(storage_ctx);

    storage_ctx->put_store_ram_val(STORE_ID_MODE_UPDATE_FLAG, BREAST_PUMP_CUSTOM_MODE, 0, 0);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_PARA_DOWNLOAD, 0, NULL);
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

    ack_data[data_len++] = ctx->priv.ver_info.sys_local_protocol_id;
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
    memcpy(ack_data + data_len, factory_dev_info.sn, factory_dev_info.sn_len);
    data_len += factory_dev_info.sn_len;
#if BLE_PAYLOAD_CRYPTION
    ack_data[data_len++] = handler_ctx->encrypt_status;//encrypt_status;
    ack_data[data_len++] = handler_ctx->encrypt_key;//encrypt_key;
#endif
    ack_data[data_len++] = ctx->priv.ver_info.sw_version % 100;
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
    pump_ctx->set_breast_pump_status(BREAST_PUMP_RESET);
    app_broadcast_pump_status_to_window(BREAST_PUMP_RESET,OLD_BROADCAST_ALL_CT);
    pump_ctx->set_breast_pump_status(BREAST_PUMP_IDLE);
    app_broadcast_pump_status_to_window(BREAST_PUMP_IDLE,OLD_BROADCAST_ALL_CT);
    /// 发送APP强制完成广播
    SEND_BROADCAST_ARGS1(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_APP_COMPLETED);
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
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = body_index,
        .arg2 = 0,
        .val = &pump_data_body,
    };
    ret = read_pump_data_body_from_flash(args);
    if (ret != 0) {
        LOGE("Failed to read pump data body, ret: %d, index: %d", ret, body_index);
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_RECORD_DATA, 0, NULL);
        return;
    }
    
    /// 转换为BLE记录格式
    app_old_comm_pump_ble_record_t record = {0};
    utc_time_t utc_time = {0};
    timestamp_to_utc_time(pump_data_body.timestamp, &utc_time);
    
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
    record.work.time = (uint16_t)(pump_data_body.work_total_time / 1000);  /// 转换为秒
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