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
static user_def_pump_steps_data_t user_define_data_temp = {0};
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

uint8_t app_old_comm_get_adaptive_dimming_switch(void)
{
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_UINT(led && led->hook && led->hook->get_night_light_adaptive_switch);
    return led->hook->get_night_light_adaptive_switch(led);
}

uint8_t app_old_comm_get_night_light_level(void)
{
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_UINT(led && led->hook && led->hook->get_night_light_level);
    return led->hook->get_night_light_level(led);
}

uint8_t app_old_comm_get_night_light_onoff(void)
{
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_UINT(led && led->hook && led->hook->get_night_light_onoff);
    return led->hook->get_night_light_onoff(led);
}

uint8_t app_old_comm_get_sys_status(void)
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

uint8_t app_old_comm_get_pump_running_status(void)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    LOG_SRV("pump_ctx->is_on_working =%d", pump_ctx->is_on_working());
    return pump_ctx->is_on_working();
}

uint8_t app_old_comm_get_battery_percent(void)
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

bool app_old_comm_check_if_low_battery(void)
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

uint8_t app_old_comm_get_current_pump_mode(void)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    int pump_mode = pump_ctx->get_breast_pump_mode();
    LOG_SRV("pump mode: %s", pump_ctx->get_breast_pump_mode_name(pump_mode));
    return (uint8_t)pump_mode;
}

uint8_t app_old_comm_get_current_custom_pump_step_mode(servers_old_protocol_publish_data_id_e id)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);

    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    //当前不是运行的不是自定义模式步骤，默认上报第一步参数,启动自定义模式从第一步开始
    if (pump_ctx->get_breast_pump_mode() != BREAST_PUMP_CUSTOM_MODE) {
        time_index = 0;
    }
    //获取自定义模式参数
    int pump_mode;
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    user_def_pump_steps_data_t custom_steps_parameter ={0};
    storage_ctx->get_store_ram_val(STORE_ID_PUMP_CUSTOM_STEPS_PARAMETER_INFO, 0, 0, &custom_steps_parameter);
    pump_mode = custom_steps_parameter.data[time_index].mode;
    LOG_SRV("custom_steps_parameter.data[%d].mode=%d", time_index, pump_mode);
    return (uint8_t)pump_mode;
}

uint8_t app_old_comm_get_current_mode_pump_level(app_old_commd_l_r_ctrl_e lr_ct, servers_old_protocol_publish_data_id_e id)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);

    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    ///当前系统在处于自定义模式步骤，应该上报什么，就默认上报吸乳第一步参数
    pump_setting_t setting;
    if (pump_ctx->get_breast_pump_mode() == BREAST_PUMP_CUSTOM_MODE) {
        //app bug,在自定义模式下，常规level需要上报自定义模式运行的level
        /*
        storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        EXCEPTION_CHECK_UINT(storage_ctx);
        store_pump_setting_value_t store_pump_setting_value = {0};
        store_pump_setting_value.is_custom_trial_mode = pump_ctx->priv.m_info.is_custom_trial_mode;
        store_pump_setting_value.setting = &setting;
        storage_ctx->get_store_ram_val(STORE_ID_PUMP_SETTING_INFO, BREAST_PUMP_LACTATION_MODE, 0, &store_pump_setting_value);*/
        pump_ctx->get_breast_pump_setting(&setting, time_index);
        LOG_SRV("setting.level [%d] =%d", lr_ct, setting.level[lr_ct]);
        return (uint8_t)setting.level[lr_ct];
    } else {
        //
        pump_ctx->get_breast_pump_setting(&setting, time_index);
        LOG_SRV("setting.level [%d] =%d", lr_ct, setting.level[lr_ct]);
        return (uint8_t)setting.level[lr_ct];
    }
}

uint8_t app_old_comm_get_current_custom_pump_level(app_old_commd_l_r_ctrl_e lr_ct, servers_old_protocol_publish_data_id_e id)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);

    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }

    pump_setting_t setting;
    //当前系统在处于自定义模式步骤
    if (pump_ctx->get_breast_pump_mode() == BREAST_PUMP_CUSTOM_MODE) {
        pump_ctx->get_breast_pump_setting(&setting, time_index);
        LOG_SRV("setting.level [%d] =%d", lr_ct, setting.level[lr_ct]);
        return (uint8_t)setting.level[lr_ct];
    } else { //默认上报flash中自定义模式第一步参数,启动自定义模式从第一步开始
        storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        EXCEPTION_CHECK_UINT(storage_ctx);
        store_pump_setting_value_t store_pump_setting_value = {0};
        store_pump_setting_value.is_custom_trial_mode = pump_ctx->priv.m_info.is_custom_trial_mode;
        store_pump_setting_value.setting = &setting;
        storage_ctx->get_store_ram_val(STORE_ID_PUMP_SETTING_INFO, BREAST_PUMP_CUSTOM_MODE, 0, &store_pump_setting_value);
        LOG_SRV("setting.level [%d] =%d", lr_ct, setting.level[lr_ct]);
        return (uint8_t)setting.level[lr_ct];
    }
}

//左右需要一致，不区分
uint8_t app_old_comm_get_current_mode_pump_speed(servers_old_protocol_publish_data_id_e id)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);

    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }

    ///当前系统在处于自定义模式步骤，应该上报什么，就默认上报吸乳第一步参数
    pump_setting_t setting;
    
    if (pump_ctx->get_breast_pump_mode() == BREAST_PUMP_CUSTOM_MODE) {
        storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        EXCEPTION_CHECK_UINT(storage_ctx);
        store_pump_setting_value_t store_pump_setting_value = {0};
        store_pump_setting_value.is_custom_trial_mode = pump_ctx->priv.m_info.is_custom_trial_mode;
        store_pump_setting_value.setting = &setting;
        storage_ctx->get_store_ram_val(STORE_ID_PUMP_SETTING_INFO, BREAST_PUMP_LACTATION_MODE, 0, &store_pump_setting_value);
        LOG_SRV("setting.speed =%d", setting.speed);
        return (uint8_t)setting.speed;
    } else {
        pump_ctx->get_breast_pump_setting(&setting, time_index);
        LOG_SRV("setting.speed =%d", setting.speed);
        return (uint8_t)setting.speed;
    }
}

uint8_t app_old_comm_get_current_custom_pump_speed(servers_old_protocol_publish_data_id_e id)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    //当前不是运行的不是自定义模式步骤，默认上报第一步参数,启动自定义模式从第一步开始
    if (pump_ctx->get_breast_pump_mode() != BREAST_PUMP_CUSTOM_MODE) {
        time_index = 0;
    }
    //不允许修改自定义模式频率，只能修改等级，所以从app设置缓存中获取
    int pump_speed;
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    user_def_pump_steps_data_t custom_steps_parameter = {0};
    storage_ctx->get_store_ram_val(STORE_ID_PUMP_CUSTOM_STEPS_PARAMETER_INFO, 0, 0, &custom_steps_parameter);
    pump_speed = custom_steps_parameter.data[time_index].freq;
    LOG_SRV("custom_steps_parameter.data[%d].pump_speed=%d", time_index, pump_speed);
    return (uint8_t)pump_speed;
}

uint32_t app_old_comm_get_current_pump_mode_run_time(void)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    pump_result_info_t *pumping_info = pump_ctx->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
    LOG_SRV("pumping_info->current_mode_work_time = %d", pumping_info->current_mode_work_time);
    return (uint32_t)pumping_info->current_mode_work_time/1000;
}

uint32_t app_old_comm_get_pump_mode_run_total_time(void)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    pump_result_info_t *pumping_info = pump_ctx->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
    LOG_SRV("pumping_info->work_total_time = %d", pumping_info->work_total_time);
    return (uint32_t)pumping_info->work_total_time/1000;
}

uint32_t app_old_comm_get_current_custom_pump_mode_total_time(void)
{
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    user_def_pump_steps_data_t custom_steps_parameter = {0};
    storage_ctx->get_store_ram_val(STORE_ID_PUMP_CUSTOM_STEPS_PARAMETER_INFO, 0, 0, &custom_steps_parameter);
    uint32_t custom_total_time = custom_steps_parameter.custom_total_time;
    LOG_SRV("custom_steps_parameter.custom_total_time= %d", custom_total_time);
    return (uint32_t)custom_total_time;
}

uint8_t app_old_comm_get_system_is_airplane_mode(void)
{
    airplane_mode_context_t *ap = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    EXCEPTION_CHECK_UINT(ap);
    int air_play_mode = ap->is_on_airplane_mode();
    return (uint8_t)air_play_mode;
}

uint8_t app_old_comm_set_custom_pump_step_parameter_infor(user_def_pump_steps_data_t *pump_steps_data)
{
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    storage_ctx->put_store_ram_val(STORE_ID_PUMP_CUSTOM_STEPS_PARAMETER_INFO, 0, 0, pump_steps_data);
    LOG_SRV("put custom_steps_parameter to ram");
    return true;
}

uint8_t app_old_comm_set_nommal_pump_level(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);

    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }

    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);

    pump_setting_t setting; /// 当前泵奶设置信息.

    pump_ctx->get_breast_pump_setting(&setting, time_index);

    if (app_old_comm_pump_ctrl_date->pump_mode_lr_or_all == APP_OLD_COMM_LEFT_CT) {
        if (app_old_comm_pump_ctrl_date->normal_pump_level >= 0) {
            setting.level[0] = app_old_comm_pump_ctrl_date->normal_pump_level;
            LOG_SRV("APP_OLD_COMM_LEFT_CT pump level %d",setting.level[0]);
            app_broadcast_pump_level_to_window(setting.level[0] ,OLD_BROADCAST_LEFT_CT);
        }
    } else if (app_old_comm_pump_ctrl_date->pump_mode_lr_or_all == APP_OLD_COMM_RIGHT_CT) {
        if (app_old_comm_pump_ctrl_date->normal_pump_level <= BREAST_PUMP_LEVEL_15) {
            setting.level[1] = app_old_comm_pump_ctrl_date->normal_pump_level;
            LOG_SRV("APP_OLD_COMM_RIGHT_CT pump level %d",setting.level[1]);
            app_broadcast_pump_level_to_window(setting.level[1] ,OLD_BROADCAST_RIGHT_CT);
        }
    } else {
        if (app_old_comm_pump_ctrl_date->normal_pump_level >= 0
            && app_old_comm_pump_ctrl_date->normal_pump_level <= BREAST_PUMP_LEVEL_15) {
            setting.level[1] = app_old_comm_pump_ctrl_date->normal_pump_level;
            setting.level[0] = app_old_comm_pump_ctrl_date->normal_pump_level;
            LOG_SRV("APP_OLD_COMM_ALL_CT pump level %d",setting.level[0]);
            LOG_SRV("APP_OLD_COMM_ALL_CT pump level %d",setting.level[1]);
            app_broadcast_pump_level_to_window(setting.level[0] ,OLD_BROADCAST_LEFT_CT);
            app_broadcast_pump_level_to_window(setting.level[1] ,OLD_BROADCAST_RIGHT_CT);
        }
    }

    pump_ctx->set_breast_pump_setting(&setting, time_index, true);
    return true;
}

uint8_t app_old_comm_set_nommal_pump_speed(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);

    pump_setting_t setting; /// 当前泵奶设置信息.

    pump_ctx->get_breast_pump_setting(&setting, time_index);
    //只允许常规模式设置 speed，并且左右同步控制
    setting.speed = app_old_comm_pump_ctrl_date->normal_pump_speed;
    LOG_SRV("pump speed  %d", setting.speed);
    pump_ctx->set_breast_pump_setting(&setting, time_index, true);
    app_broadcast_pump_speed_to_window(setting.speed ,OLD_BROADCAST_ALL_CT);
    return true;
}

uint8_t app_old_comm_set_pump_control_type(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
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

uint8_t app_old_comm_get_pump_control_type(void)
{
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump);
    uint8_t control_type = pump->get_breast_pump_control_type();
    LOG_SRV("get pump control type %d", control_type);
    return control_type;
}

uint8_t app_old_comm_set_custom_pump_level(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);

    pump_setting_t setting; /// 当前泵奶设置信息.

    pump_ctx->get_breast_pump_setting(&setting, time_index);

    if (app_old_comm_pump_ctrl_date->pump_mode_lr_or_all == APP_OLD_COMM_LEFT_CT) {
        if (app_old_comm_pump_ctrl_date->user_define_pump_level >= 0) {
            setting.level[0] = app_old_comm_pump_ctrl_date->user_define_pump_level;
            LOG_SRV("APP_OLD_COMM_LEFT_CT pump level %d",setting.level[0]);
            app_broadcast_pump_level_to_window(setting.level[0] ,OLD_BROADCAST_LEFT_CT);
        }
    } else if (app_old_comm_pump_ctrl_date->pump_mode_lr_or_all == APP_OLD_COMM_RIGHT_CT) {
        if (app_old_comm_pump_ctrl_date->user_define_pump_level <= BREAST_PUMP_LEVEL_15) {
            setting.level[1] = app_old_comm_pump_ctrl_date->user_define_pump_level;
            LOG_SRV("APP_OLD_COMM_RIGHT_CT pump level %d",setting.level[1]);
            app_broadcast_pump_level_to_window(setting.level[1] ,OLD_BROADCAST_RIGHT_CT);
        }
    } else {
        if (app_old_comm_pump_ctrl_date->user_define_pump_level >= 0
            && app_old_comm_pump_ctrl_date->user_define_pump_level <= BREAST_PUMP_LEVEL_15) {
            setting.level[1] = app_old_comm_pump_ctrl_date->user_define_pump_level;
            setting.level[0] = app_old_comm_pump_ctrl_date->user_define_pump_level;
            LOG_SRV("APP_OLD_COMM_ALL_CT pump level %d",setting.level[0]);
            LOG_SRV("APP_OLD_COMM_ALL_CT pump level %d",setting.level[1]);
            app_broadcast_pump_level_to_window(setting.level[0] ,OLD_BROADCAST_LEFT_CT);
            app_broadcast_pump_level_to_window(setting.level[1] ,OLD_BROADCAST_RIGHT_CT);
        }
    }

    pump_ctx->set_breast_pump_setting(&setting, time_index, true);
    return true;
}

uint8_t app_old_comm_pump_control_handler(app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date)
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
                LOG_SRV("app set normal pump speed");
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
    } else //如果模式改变，则改变模式,忽略参数
    {
        pump_ctx->set_breast_pump_mode(app_old_comm_pump_ctrl_date->pump_mode, 1);
        app_broadcast_pump_mode_to_window(app_old_comm_pump_ctrl_date->pump_mode,OLD_BROADCAST_ALL_CT);
        //prd 模式记忆
        LOG_SRV("app change pump_mode %d",app_old_comm_pump_ctrl_date->pump_mode);
    }
    return true;
}

uint8_t app_old_comm_pump_run_status_ctrl(uint8_t status)
{
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    LOG_SRV("run_status_ctrl %d", status);
    int pump_status = pump_ctx->get_breast_pump_status();

    if (pump_status == BREAST_PUMP_FINISH || pump_status == BREAST_PUMP_IDLE)
    {
        if (status == APP_OLD_COMM_SYS_RUN)
        {
            pump_ctx->set_breast_pump_status(BREAST_PUMP_START);
            app_broadcast_pump_status_to_window(BREAST_PUMP_START,OLD_BROADCAST_ALL_CT);
        }
    }
    else if (pump_status == BREAST_PUMP_START || pump_status == BREAST_PUMP_CONTINUE)
    {
        if (status == APP_OLD_COMM_SYS_STOP)
        {
            pump_ctx->set_breast_pump_status(BREAST_PUMP_PAUSE);
            app_broadcast_pump_status_to_window(BREAST_PUMP_PAUSE,OLD_BROADCAST_ALL_CT);
        }
    }
    else if (pump_status == BREAST_PUMP_PAUSE)
    {
        if (status == APP_OLD_COMM_SYS_RUN)
        {
            pump_ctx->set_breast_pump_status(BREAST_PUMP_CONTINUE);
            app_broadcast_pump_status_to_window(BREAST_PUMP_CONTINUE,OLD_BROADCAST_ALL_CT);
        }
    }
    return true;
}

uint8_t app_old_comm_pump_custom_step_para_to_local_motor_policy(user_def_pump_steps_data_t *pump_steps_data)
{
    motor_policy_cfg_t *custom_mode_policy_cfg = custom_mode_policy_cfg_get();
    //memset(&custom_mode_policy_cfg, 0, sizeof(motor_policy_cfg_t)*CUSTOM_MODE_MOTOR_POLICY_NUMBER);
    uint8_t cfg_num = 0;
    for (uint8_t i = 0; i < pump_steps_data->num; i++)
    {
        if (cfg_num >= CUSTOM_MODE_MOTOR_POLICY_NUMBER) {
            LOG_SRV("custom pump step will over max");
            return false;
        }
        LOG_SRV("custom pump step mode %d", pump_steps_data->data[i].mode);
        if (pump_steps_data->data[i].mode == APP_OLD_COMM_MODE_SHORT) {
            motor_policy_cfg_t stimulation_default_temp[STIMULATION_MODE_LEN] = {0};
            memcpy(stimulation_default_temp, &stimulation_default, sizeof(stimulation_default));
            stimulation_default_temp[0].duration = pump_steps_data->data[i].time;
            stimulation_default_temp[0].core.un.mapping.level = pump_steps_data->data[i].level;
            stimulation_default_temp[0].core.un.mapping.speed = pump_steps_data->data[i].freq;
            memcpy(custom_mode_policy_cfg + cfg_num, &stimulation_default_temp,
                   sizeof(stimulation_default_temp));
            cfg_num += 1;
        } else if (pump_steps_data->data[i].mode == APP_OLD_COMM_MODE_LONG) {
            motor_policy_cfg_t pump_default_temp[PUMP_MODE_LEN] = {0};
            memcpy(pump_default_temp, &pump_default, sizeof(pump_default));
            pump_default_temp[0].duration = pump_steps_data->data[i].time;
            pump_default_temp[0].core.un.mapping.level = pump_steps_data->data[i].level;
            pump_default_temp[0].core.un.mapping.speed = pump_steps_data->data[i].freq;
            memcpy(custom_mode_policy_cfg + cfg_num, &pump_default_temp,
                   sizeof(pump_default_temp));
            cfg_num += 1;
        } else if (pump_steps_data->data[i].mode == APP_OLD_COMM_MODE_MIX1) {
            motor_policy_cfg_t mix_default_temp[MIX_MODE_LEN] = {0};
            memcpy(mix_default_temp, &mix_default, sizeof(mix_default));
            for (uint8_t j = 0; j < MIX_MODE_LEN; j++) {
                mix_default_temp[j].duration = pump_steps_data->data[i].time;
                mix_default_temp[j].core.un.mapping.level = pump_steps_data->data[i].level;
                mix_default_temp[j].core.un.mapping.speed = pump_steps_data->data[i].freq;
            }
            memcpy(custom_mode_policy_cfg + cfg_num, &mix_default_temp,
                   sizeof(mix_default_temp));
            cfg_num += 2;
        } else if (pump_steps_data->data[i].mode == APP_OLD_COMM_MODE_MIX2) {
            motor_policy_cfg_t mix_default2_temp[MIX2_MODE_LEN] = {0};
            memcpy(mix_default2_temp, &mix_default2, sizeof(mix_default2));
            for (uint8_t j = 0; j < MIX2_MODE_LEN; j++) {
                mix_default2_temp[j].duration = pump_steps_data->data[i].time;
                mix_default2_temp[j].core.un.mapping.level = pump_steps_data->data[i].level;
                mix_default2_temp[j].core.un.mapping.speed = pump_steps_data->data[i].freq;
            }
            memcpy(custom_mode_policy_cfg + cfg_num, &mix_default2_temp,
                   sizeof(mix_default2_temp));
            cfg_num += 2;
        } 
        else if (pump_steps_data->data[i].mode == APP_OLD_COMM_MODE_RELAX) {
            motor_policy_cfg_t relax_default_temp[RELAX_MODE_LEN] = {0};
            memcpy(relax_default_temp, &relax_default, sizeof(relax_default));
            relax_default_temp[0].duration = pump_steps_data->data[i].time;
            relax_default_temp[0].core.un.mapping.level = pump_steps_data->data[i].level;
            relax_default_temp[0].core.un.mapping.speed = pump_steps_data->data[i].freq;
            memcpy(custom_mode_policy_cfg + cfg_num, &relax_default_temp,
                   sizeof(relax_default_temp));
            cfg_num += 1;
        } else {
            LOG_SRV("unknow custom pump step mode");
        }
    }
    print_pump_custom_cfg(custom_mode_policy_cfg, cfg_num);
    motor_manager_context_t *motor_ctx = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    EXCEPTION_CHECK_UINT(motor_ctx);
    EXCEPTION_CHECK_UINT(motor_ctx->hook);
    EXCEPTION_CHECK_UINT(motor_ctx->hook->motor_debug_with_user_para);
    motor_ctx->hook->generate_motor_para(motor_ctx, BREAST_PUMP_CUSTOM_MODE, custom_mode_policy_cfg, cfg_num*sizeof(motor_policy_cfg_t));
    //创建自定义模式策略后，如果当前是自定义模式，需要暂停pump，再启动pump   
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    ///如果当前是自定义模式，并且pump正在工作，则暂停pump，再启动pump,将参数更新到最新
    if (app_old_comm_get_current_pump_mode() == BREAST_PUMP_CUSTOM_MODE && pump_ctx->is_on_working()) {
        pump_ctx->set_breast_pump_status(BREAST_PUMP_PAUSE);
        app_broadcast_pump_status_to_window(BREAST_PUMP_PAUSE,OLD_BROADCAST_ALL_CT);
        pump_ctx->set_breast_pump_status(BREAST_PUMP_START);
        app_broadcast_pump_status_to_window(BREAST_PUMP_START,OLD_BROADCAST_ALL_CT);
    }
    //custom_mode_parameter_update(custom_mode_policy_cfg, cfg_num);
    return true;
}

uint8_t app_old_comm_update_storage_custom_setting(user_def_pump_steps_data_t *pump_user_defined_step_data_temp)
{
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump_ctx);
    if (pump_user_defined_step_data_temp->num > ROOT_BREAST_PUMP_STEP_MAX)
    {
       LOG_SRV(" over max step");
       return false;
    }
    for (uint8_t i = 0; i < pump_user_defined_step_data_temp->num; i++) {
        pump_setting_t setting;
        setting.speed = pump_user_defined_step_data_temp->data->freq;
        for (uint8_t j = 0; j < BREAST_PUMP_NUM; j++) {
            setting.level[j] = pump_user_defined_step_data_temp->data->level;
        }
        store_pump_setting_value_t store_pump_setting_value = {0};
        store_pump_setting_value.is_custom_trial_mode = pump_ctx->priv.m_info.is_custom_trial_mode;
        store_pump_setting_value.setting = &setting;
        storage_ctx->put_store_ram_val(STORE_ID_PUMP_SETTING_INFO, BREAST_PUMP_CUSTOM_MODE, i, &store_pump_setting_value);
    }
    return true;
}

void app_pump_ctrl_delay_task_handler(hdr_post_args_t *args)
{
    int status = (int)args->arg1;
    app_old_comm_pump_ctrl *app_old_comm_pump_ctrl_date = (app_old_comm_pump_ctrl *)args->user_data;
    EXCEPTION_CHECK_VOID(app_old_comm_pump_ctrl_date);
    app_old_comm_pump_control_handler(app_old_comm_pump_ctrl_date);
    app_old_comm_pump_run_status_ctrl(status);
    LOG_SRV("app_pump_ctrl_delay_task_handler, status: %d", status);
}

void app_old_comm_set_user_define_handler(user_def_pump_steps_data_t *pump_steps_data)
{
    ///将自定义模式参数存入flash
    app_old_comm_set_custom_pump_step_parameter_infor(pump_steps_data);
    //先暂停pump
    //breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    //EXCEPTION_CHECK_VOID(pump_ctx);
    //if (app_old_comm_get_current_pump_mode() == BREAST_PUMP_CUSTOM_MODE)
    //    pump_ctx->set_breast_pump_status(BREAST_PUMP_PAUSE);
    //将storage的custom 目标pump time更新
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(storage_ctx);
    store_pump_time_value_t store_pump_time_value = {0};
    store_pump_time_value.is_custom_trial_mode = 0;
    store_pump_time_value.pump_time = pump_steps_data->custom_total_time;
    storage_ctx->put_store_ram_val(STORE_ID_BREAST_PUMP_TIME, BREAST_PUMP_CUSTOM_MODE,
        0, &store_pump_time_value);
    LOG_SRV("pump_user_defined_step_data_temp.custom_total_time= 0x%x",
        pump_steps_data->custom_total_time);
    //将storage的custom 的setting更新
    app_old_comm_update_storage_custom_setting(pump_steps_data);
    //将默认的custom 策略更新
    //int custom_mode_parameter_update(motor_policy_cfg_t *cfg, uint8_t cfg_num)
    app_old_comm_pump_custom_step_para_to_local_motor_policy(pump_steps_data);
    //重新启动pump
    //if (app_old_comm_get_current_pump_mode() == BREAST_PUMP_CUSTOM_MODE)
    //    pump_ctx->set_breast_pump_status(BREAST_PUMP_START);
}

void app_pump_user_define_delay_task_handler(hdr_post_args_t *args)
{
    int status = (int)args->arg1;
    user_def_pump_steps_data_t *pump_steps_data = (user_def_pump_steps_data_t *)args->user_data;
    EXCEPTION_CHECK_VOID(pump_steps_data);
    app_old_comm_set_user_define_handler(pump_steps_data);
    LOG_SRV("enter");
}

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
    app_old_comm_pump_ctrl_date.pump_mode_lr_or_all = cab[6];
    app_old_comm_pump_ctrl_date.normal_pump_speed = cab[7]; //频次（0-低 1-中 2-高）（1字节）
    ///查看窗口，是否需要唤醒pump窗口
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_PUMP)) {
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_CTRL_DEV, 1, &cab[6]);
        app_old_comm_pump_control_handler(&app_old_comm_pump_ctrl_date);
        app_old_comm_pump_run_status_ctrl(cab[0]);
    } else if (wm->is_current_window(wm, ID_WINDOW_CHARGE) || wm->is_current_window(wm, ID_WINDOW_DUMMY)) {
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
	user_def_pump_steps_data_t pump_user_defined_step_data_temp = {0};

    uint8_t section_num = 0;
    uint8_t i = 0, n = 0;

    EXCEPTION_CHECK_VOID(cab);
    memset(&pump_user_defined_step_data_temp, 0, sizeof(pump_user_defined_step_data_temp));// 清除掉临时变量

	pump_user_defined_step_data_temp.id.dat[0] = cab[0]; // data[3];
	pump_user_defined_step_data_temp.id.dat[1] = cab[1]; // data[2];
	pump_user_defined_step_data_temp.id.dat[2] = cab[2]; // data[1];
	pump_user_defined_step_data_temp.id.dat[3] = cab[3]; // data[0];
	LOG_SRV("cmd ud-mode id = 0X%x", pump_user_defined_step_data_temp.id.dat32);

	if (cab[4] <= ROOT_BREAST_PUMP_STEP_MAX)
	{
		section_num = cab[4];
		pump_user_defined_step_data_temp.num = section_num;
        LOG_SRV("pump_user_defined_step_data_temp.num = %d", pump_user_defined_step_data_temp.num);
	}

	pump_user_defined_step_data_temp.custom_total_time = 0;

	for (i = 0; i < section_num; i++)
	{
		n = 5 + 10 * i;
		if (cab[n] <= ROOT_BREAST_PUMP_STEP_MAX)
		{
			pump_user_defined_step_data_temp.data[i].mode = cab[n + 1];
			pump_user_defined_step_data_temp.data[i].level = cab[n + 2];
			if (pump_user_defined_step_data_temp.data[i].mode >= (BREAST_PUMP_MODE_MAX + 1))
			{
                pump_user_defined_step_data_temp.data[i].mode = BREAST_PUMP_NORMAL_MODE_DEFAULT + 1;
            }
			if (pump_user_defined_step_data_temp.data[i].level >= BREAST_PUMP_LEVEL_MAX)
			{
				pump_user_defined_step_data_temp.data[i].level = BREAST_PUMP_LEVEL_15;
			}

			pump_user_defined_step_data_temp.data[i].freq = cab[n + 3] - 1;
			pump_user_defined_step_data_temp.data[i].time = cab[n + 4] << 8 | cab[n + 5];
			if (pump_user_defined_step_data_temp.data[i].freq >= SUCTION_SPEED_MAX)
			{
				pump_user_defined_step_data_temp.data[i].freq = SUCTION_SPEED_HIGH;
			}
			//PumpUserUdTemp[i].heat_switch = data[n + 6];
			//PumpUserUdTemp[i].heat_level = data[n + 7];
			//PumpUserUdTemp[i].shake_switch = data[n + 8];
			//PumpUserUdTemp[i].shake_level = data[n + 9];
			pump_user_defined_step_data_temp.custom_total_time += pump_user_defined_step_data_temp.data[i].time;
		}

		LOG_SRV("ud-mode[%d]=0x%02x", i, pump_user_defined_step_data_temp.data[i].mode);
		LOG_SRV("ud-level[%d]=0x%02x", i, pump_user_defined_step_data_temp.data[i].level);
		LOG_SRV("ud-time[%d]=0x%04x", i, pump_user_defined_step_data_temp.data[i].time);
        LOG_SRV("ud-freq[%d]=0x%02x", i, pump_user_defined_step_data_temp.data[i].freq);
	}
    //pump_user_defined_step_data_temp.custom_total_time = 1000*pump_user_defined_step_data_temp.custom_total_time; //转换为ms
    n = 5 + 10 * i;
    LOG_SRV("data n = [%d] data len = [%d]", n, cal);
    ///查看窗口，是否需要唤醒pump窗口
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_PUMP)) {
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_USER_DEFINE, 0, NULL);
        app_old_comm_set_user_define_handler(&pump_user_defined_step_data_temp);
    } else if (wm->is_current_window(wm, ID_WINDOW_CHARGE) || wm->is_current_window(wm, ID_WINDOW_DUMMY)) {
        LOG_SRV("charge or dummy, dispatch pump window.");
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_PUMP;
        wm->dispatch_new_window_args(wm, &msg);

        ///等待窗口弹出延时投递处理
        // 注意：POST_TASK_HANDLER_DELAYED_ARG2 会先删除同名旧任务，然后投递新任务
        // 使用静态变量避免内存泄漏，但如果删除失败，旧任务可能使用新数据
        memcpy(&user_define_data_temp, &pump_user_defined_step_data_temp, sizeof(user_def_pump_steps_data_t));
        POST_TASK_HANDLER_DELAYED_ARG2(app_pump_user_define_delay_task_handler, PUMP_CTRL_TASK_DELAY_TIME, 0, &user_define_data_temp,
                                       "app_pump_user_define_delay_task");
        app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_USER_DEFINE, 0, NULL);
    } else {
        LOG_SRV("not in pump window");
    }
}

//0xc5
void app_old_comm_handle_set_user_define_wr_flash_req(uint8_t cal, uint8_t *cab)
{
    //不需要做啥c4 已经写入
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
    ack_data[data_len++] = ctx->priv.sys_info.pump_info.custom_steps_parameter.id.dat[0];//custom_mode_id;
    ack_data[data_len++] = ctx->priv.sys_info.pump_info.custom_steps_parameter.id.dat[1];//custom_mode_id;
    ack_data[data_len++] = ctx->priv.sys_info.pump_info.custom_steps_parameter.id.dat[2];//custom_mode_id;
    ack_data[data_len++] = ctx->priv.sys_info.pump_info.custom_steps_parameter.id.dat[3];//custom_mode_id;
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