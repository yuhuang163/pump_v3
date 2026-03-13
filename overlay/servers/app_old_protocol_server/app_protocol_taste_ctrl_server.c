#include "root_common_api.h" // IWYU pragma: keep
#include "app_protocol_handler.h"
#include "app_protocol_taste_ctrl_server.h"
#include "pump_mode_default_config.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

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

uint8_t app_old_comm_handle_ctrl_taste_dev_ctrl(app_old_comm_pump_taste_ctrl *taste_ctrl_date)
{
    LOG_SRV(" Handling device taste control request");
    EXCEPTION_CHECK_UINT(taste_ctrl_date);
    motor_manager_context_t *motor = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    EXCEPTION_CHECK_UINT(motor);
    EXCEPTION_CHECK_UINT(motor->hook);
    EXCEPTION_CHECK_UINT(motor->hook->motor_debug_with_user_para);
    motor_multiple_policy_cfg_t para = {0};
    app_old_protocol_handler_context_t *handler_ctx = app_old_protocol_handler_ctx_get();
    if (handler_ctx->taste_ctrl.taste_status_ctrl != taste_ctrl_date->taste_status_ctrl) {
        handler_ctx->taste_ctrl.taste_status_ctrl = taste_ctrl_date->taste_status_ctrl;
        if (taste_ctrl_date->taste_status_ctrl == APP_OLD_COMM_TASTE_STATUS_EXIT) {
            motor->hook->motor_debug_with_user_para(motor, MOTOR_USER_TASTE_POLICY_EXIT, NULL);
            breast_pump_manager_context_t *pump_ctx =
                (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
            EXCEPTION_CHECK_UINT(pump_ctx);
            /// 退出体验模式后，继续运行正常pump  //CONTINUE 存在问题 timestep和 time都被体验策略改了
            WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_START);
            //操作暂停退出体验模式 不用设置参数直接返回
            return true;
        } else if (taste_ctrl_date->taste_status_ctrl == APP_OLD_COMM_TASTE_STATUS_STOP) {
            motor->hook->motor_debug_with_user_para(motor, MOTOR_USER_TASTE_POLICY_STOP, NULL);
            //操作暂停退出体验模式 不用设置参数直接返回
            return true;
        } else if (taste_ctrl_date->taste_status_ctrl == APP_OLD_COMM_TASTE_STATUS_RUN) {
            ///如果正常pump正在运行，先退出正常pump，再启动体验模式
            //先暂停正常pump运行
            breast_pump_manager_context_t *pump_ctx =
                (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
            EXCEPTION_CHECK_UINT(pump_ctx);
            WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_PAUSE);
        }
    }
    //暂停后在控制启动，或在启动状态模式变化，才需要暂停motor后，重新加载策略启动
    if ((handler_ctx->taste_ctrl.taste_status_ctrl != taste_ctrl_date->taste_status_ctrl)
        || (handler_ctx->taste_ctrl.taste_pump_mode != taste_ctrl_date->taste_pump_mode)) {
        handler_ctx->taste_ctrl.taste_pump_mode = taste_ctrl_date->taste_pump_mode;
        handler_ctx->taste_ctrl.taste_status_ctrl = taste_ctrl_date->taste_status_ctrl;
        handler_ctx->taste_ctrl.taste_pump_level = taste_ctrl_date->taste_pump_level;
        handler_ctx->taste_ctrl.taste_pump_speed = taste_ctrl_date->taste_pump_speed;

        /// 设置用户区体验模式参数，用于启动体验模式时使用
        taste_setting_t taste_setting;
        taste_setting.speed = taste_ctrl_date->taste_pump_speed - 1; //频次（1-低 2-中 3-高 需要减1）
        if (taste_setting.speed > BREAST_PUMP_SPEED_HIGH) {
            taste_setting.speed = BREAST_PUMP_SPEED_HIGH;
        }
        for (uint8_t j = 0; j < BREAST_PUMP_NUM; j++) {
            taste_setting.level[j] = taste_ctrl_date->taste_pump_level;
            if (taste_setting.level[j] > BREAST_PUMP_LEVEL_15) {
                taste_setting.level[j] = BREAST_PUMP_LEVEL_15;
            }
        }
        motor->hook->motor_debug_with_taste_setting(motor, &taste_setting);
        /// 设置体验模式默认策略
        if (taste_ctrl_date->taste_pump_mode == APP_OLD_COMM_TASTE_MODE_SUCTION
            && taste_ctrl_date->taste_status_ctrl == APP_OLD_COMM_TASTE_STATUS_RUN) {
            LOG_SRV(" Taste control suction");
            motor_policy_cfg_t pump_default_temp[PUMP_MODE_LEN] = {0};
            memcpy(pump_default_temp, &pump_default, sizeof(pump_default));
            pump_default_temp[0].core.un.mapping.level = taste_ctrl_date->taste_pump_level;
            pump_default_temp[0].core.un.mapping.speed =
                taste_ctrl_date->taste_pump_speed - 1; //频次（1-低 2-中 3-高 需要减1）
            para.policy_cfg_list = pump_default_temp;
            para.policy_cfg_count = PUMP_MODE_LEN;
            //会重新启动motor
            motor->hook->motor_debug_with_user_para(motor, MOTOR_USER_TASTE_PUMP_POLICY, &para);
            return true;
        }
        if (taste_ctrl_date->taste_pump_mode == APP_OLD_COMM_TASTE_MODE_MIX1
            && taste_ctrl_date->taste_status_ctrl == APP_OLD_COMM_TASTE_STATUS_RUN) {
            LOG_SRV(" Taste control mix1");
            motor_policy_cfg_t mix_default_temp[MIX_MODE_LEN] = {0};
            memcpy(mix_default_temp, &mix_default, sizeof(mix_default));
            for (uint8_t j = 0; j < MIX_MODE_LEN; j++) {
                mix_default_temp[j].core.un.mapping.level = taste_ctrl_date->taste_pump_level;
                mix_default_temp[j].core.un.mapping.speed =
                    taste_ctrl_date->taste_pump_speed - 1; //频次（1-低 2-中 3-高 需要减1）
            }
            para.policy_cfg_list = mix_default_temp;
            para.policy_cfg_count = MIX_MODE_LEN;
            motor->hook->motor_debug_with_user_para(motor, MOTOR_USER_TASTE_MIX1_POLICY, &para);
            return true;
        }
        if (taste_ctrl_date->taste_pump_mode == APP_OLD_COMM_TASTE_MODE_MIX2
            && taste_ctrl_date->taste_status_ctrl == APP_OLD_COMM_TASTE_STATUS_RUN) {
            LOG_SRV(" Taste control mix2");
            motor_policy_cfg_t mix_default2_temp[MIX2_MODE_LEN] = {0};
            memcpy(mix_default2_temp, &mix_default2, sizeof(mix_default2));
            for (uint8_t j = 0; j < MIX2_MODE_LEN; j++) {
                mix_default2_temp[j].core.un.mapping.level = taste_ctrl_date->taste_pump_level;
                mix_default2_temp[j].core.un.mapping.speed =
                    taste_ctrl_date->taste_pump_speed - 1; //频次（1-低 2-中 3-高 需要减1）
            }
            para.policy_cfg_list = mix_default2_temp;
            para.policy_cfg_count = MIX2_MODE_LEN;
            motor->hook->motor_debug_with_user_para(motor, MOTOR_USER_TASTE_MIX2_POLICY, &para);
            return true;
        }
    }
    else { //模式和状态都没有变化，说明只需要直接设置taste用户参数
        taste_setting_t taste_setting;
        taste_setting.speed = taste_ctrl_date->taste_pump_speed - 1; //频次（1-低 2-中 3-高 需要减1）
        for (uint8_t j = 0; j < BREAST_PUMP_NUM; j++) {
            taste_setting.level[j] = taste_ctrl_date->taste_pump_level;
        }
        motor->hook->motor_debug_with_taste_setting(motor, &taste_setting);
        return true;
    }
    LOGE(" Invalid taste control parameters: mode=%d, status=%d", 
         taste_ctrl_date->taste_pump_mode, taste_ctrl_date->taste_status_ctrl);
    return false;
}

//0xc8 体验模式设置
void app_old_comm_handle_ctrl_taste_dev_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling device taste control request");
    EXCEPTION_CHECK_VOID(cab);
    if (cal < 0x04) {
        LOG_SRV(" Invalid data length for TASTE_CTRL (expected >=0X04, got %d)", cal);
        return;
    }
    // TODO: Send parameters to pump valve component
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_TASTE_CTRL, 0, NULL);

    app_old_comm_pump_taste_ctrl taste_ctrl = {0};
    taste_ctrl.taste_status_ctrl = cab[0];
    taste_ctrl.taste_pump_mode = cab[1];
    taste_ctrl.taste_pump_level = cab[2];
    taste_ctrl.taste_pump_speed = cab[3];
    LOG_SRV("taste_ctrl.taste_status_ctrl = %d", taste_ctrl.taste_status_ctrl);
    LOG_SRV("taste_ctrl.taste_pump_mode = %d", taste_ctrl.taste_pump_mode);
    LOG_SRV("taste_ctrl.taste_pump_level = %d", taste_ctrl.taste_pump_level);
    LOG_SRV("taste_ctrl.taste_pump_speed = %d", taste_ctrl.taste_pump_speed);

    app_old_comm_handle_ctrl_taste_dev_ctrl(&taste_ctrl);
    app_old_comm_handle_report_taste_dev_status(OLD_PUBLISH_DATA_TASTE_POLL_REPORT, NULL);
}

//0xB8 体验模式获取状态
void app_old_comm_handle_get_taste_dev_status_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling get device taste status request");
    uint8_t ack_data[10] = {0};
    uint16_t data_len = 0;
    app_old_protocol_handler_context_t *handler_ctx = app_old_protocol_handler_ctx_get();
    ack_data[data_len++] = handler_ctx->taste_ctrl.taste_status_ctrl;
    ack_data[data_len++] = handler_ctx->taste_ctrl.taste_pump_mode;
    ack_data[data_len++] = handler_ctx->taste_ctrl.taste_pump_level;
    ack_data[data_len++] = handler_ctx->taste_ctrl.taste_pump_speed;
    // TODO: Send parameters to pump valve component
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_TASTE_MODE_STATE, data_len, ack_data);
}

//0xd8 体验模式状态上报
void app_old_comm_handle_report_taste_dev_status(servers_old_protocol_publish_data_id_e id, void *val)
{
    LOG_SRV(" Handling get device taste status request id = %d", id);
    uint8_t ack_data[10] = {0};
    uint16_t data_len = 0;
    app_old_protocol_handler_context_t *handler_ctx = app_old_protocol_handler_ctx_get();
    ack_data[data_len++] = handler_ctx->taste_ctrl.taste_status_ctrl;
    ack_data[data_len++] = handler_ctx->taste_ctrl.taste_pump_mode;
    ack_data[data_len++] = handler_ctx->taste_ctrl.taste_pump_level;
    ack_data[data_len++] = handler_ctx->taste_ctrl.taste_pump_speed;
    switch (id) {
        case OLD_PUBLISH_DATA_TASTE_START_REPORT:
            ack_data[0] = APP_OLD_COMM_TASTE_STATUS_RUN;
            handler_ctx->taste_ctrl.taste_status_ctrl = APP_OLD_COMM_TASTE_STATUS_RUN;
            break;
        case OLD_PUBLISH_DATA_TASTE_STOP_REPORT:
            ack_data[0] = APP_OLD_COMM_TASTE_STATUS_STOP;
            handler_ctx->taste_ctrl.taste_status_ctrl = APP_OLD_COMM_TASTE_STATUS_STOP;
            break;
        case OLD_PUBLISH_DATA_TASTE_EXIT_REPORT:
            ack_data[0] = APP_OLD_COMM_TASTE_STATUS_EXIT;
            handler_ctx->taste_ctrl.taste_status_ctrl = APP_OLD_COMM_TASTE_STATUS_EXIT;
            break;
        case OLD_PUBLISH_DATA_TASTE_MODE_CHANGE:
            break;
        case OLD_PUBLISH_DATA_TASTE_LEVEL_CHANGE:
            break;
        case OLD_PUBLISH_DATA_TASTE_SPEED_CHANGE:
            break;
        default:
            break;
    }
    // TODO: Send parameters to pump valve component
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_NOTIFY, APP_OLD_COMM_CID_TASTE_REPORT, data_len, ack_data);
}


