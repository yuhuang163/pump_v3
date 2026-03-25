/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "breast_pump_manager.h" // IWYU pragma: keep
#include "storage_manager_api.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_INFO_LOG_INTERVAL                             (10)            
#define PUMP_WILL_FINISH_TIME_MS                           (3000)
#define PUMP_TIME_TO_SECOND(time)                          (time / 1000)

// 比较两数相等，允许一定范围的误差
#define IS_UINT_EQUAL_IN_ERROR(num_scr, num_dest, diff)           (((num_dest) <= (num_scr)) && ((num_scr) < ((num_dest) + (diff))))

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
static int breast_pump_completed_route(breast_pump_manager_context_t *ctx) 
{
    int need_save = 1;
    pump_result_info_t *result_info   = &ctx->priv.result_info[BREAST_PUMP_RESULT_PUMPING];
    pump_result_info_t *history_info  = &ctx->priv.result_info[BREAST_PUMP_RESULT_HISTORY];
    if (result_info->work_total_time < ROOT_BREAST_PUMP_TIME_VALID) {
        LOG_PMP("no need to save, work_total_time: %lu", result_info->work_total_time);
        need_save = 0;
    } else {
        memcpy(history_info, result_info, sizeof(pump_result_info_t)); /// 保存上次有效泵奶记录
        history_info->timestamp_finish = platform_get_utc_time();
    }

    int user_bind_info = 0;
    ctx->priv.st->get_store_ram_val(STORE_ID_USER_BIND_INFO, 0, 0, &user_bind_info);
    if (!user_bind_info || result_info->report_flag) {
        // LOG_PMP("no need to save, report_flag: %d", result_info->report_flag);
        need_save = 0;
    }

    return need_save;
}

static int local_breast_pump_process(breast_pump_manager_context_t *ctx, pump_result_info_t *result_info)
{
    static uint8_t log_timer = 0;
    int ret = RESULT_PUMP_NORMAL;
    log_timer++;
    if ((result_info->work_total_time >= result_info->target_total_time && result_info->target_total_time != 0)) {
        LOG_PMP("finish, work_time=%d/%d, mode_time=%d", 
            PUMP_TIME_TO_SECOND(result_info->work_total_time), PUMP_TIME_TO_SECOND(result_info->target_total_time),
            result_info->current_mode_work_time);   
        log_timer = 0;
        return RESULT_PUMP_FINISH;
    }
#if MOTOR_DRIVER_VERSION == MOTOR_DRIVER_V2
    uint32_t poll_time = get_timestamp_ms(result_info->start_time);
    result_info->work_total_time = ctx->priv.motor->get_policy_running_time();
    result_info->current_mode_work_time += poll_time;
    result_info->start_time = get_timestamp();
#else   
    uint32_t poll_time = get_timestamp_ms(result_info->start_time);
    result_info->start_time = get_timestamp();
    result_info->work_total_time += poll_time;
    result_info->current_mode_work_time += poll_time;       
#endif
    /// 判断是否即将结束
    if ((result_info->work_total_time + PUMP_WILL_FINISH_TIME_MS) >= result_info->target_total_time) {
        ret = RESULT_PUMP_WILL_FINISH;
    }
    /// 更新模式和力度.
    result_info->mode = ctx->priv.m_info.mode;
    memcpy(result_info->setting, ctx->priv.setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
    if (log_timer > PUMP_INFO_LOG_INTERVAL) {
        int time_index = 0;
        pump_setting_t setting;
        READ_PUMP_MODE_STEP_INDEX(ctx, time_index);
        READ_PUMP_SETTING_INFO(ctx, time_index, setting);
        uint32_t time_policy_remaining_time = ctx->priv.motor->get_time_policy_remaining_time();
        LOG_PMP("%s, work_time=%d/%d, mode_time=%d, policy_rtime=%d, setting=%d/%d/%d", 
            ret == RESULT_PUMP_WILL_FINISH ? "will finish" : "normal",
            PUMP_TIME_TO_SECOND(result_info->work_total_time), PUMP_TIME_TO_SECOND(result_info->target_total_time),
            PUMP_TIME_TO_SECOND(result_info->current_mode_work_time), time_policy_remaining_time,
            setting.level[0], setting.level[1], setting.speed);
        log_timer = 0;
    }
    return ret;
}

static int make_progress_route(breast_pump_manager_context_t *ctx, pump_result_info_t *result_info)
{
    int notify_id = -1;

    EXCEPTION_CHECK_INT(result_info);
    notify_id = local_breast_pump_process(ctx, result_info);

    return notify_id;
}

static int breast_pump_result_info_reset(breast_pump_manager_context_t *ctx, pump_result_info_t *result_info)
{
    result_info->current_mode_work_time = 0;
    result_info->target_total_time = result_info->setting_time;
    result_info->work_total_time = 0;
    result_info->timestamp = platform_get_utc_time();
    dump_timestamp(result_info->timestamp);
    LOG_PMP("target_total_time: %d", PUMP_TIME_TO_SECOND(result_info->target_total_time));
    return 0;
}

static int breast_pump_result_info_update(breast_pump_manager_context_t *ctx, pump_result_info_t *result_info)
{

    result_info->target_total_time = result_info->setting_time;
    result_info->timestamp = platform_get_utc_time();
    dump_timestamp(result_info->timestamp);
    LOG_PMP("target_total_time: %d", PUMP_TIME_TO_SECOND(result_info->target_total_time));
    return 0;
}

static int start_breast_pump_work_route(breast_pump_manager_context_t *ctx, int status)
{    
    struct storage_manager_context *st = ctx->priv.st;
    EXCEPTION_CHECK_INT(st);
    pump_result_info_t *result_info = &ctx->priv.result_info[BREAST_PUMP_RESULT_PUMPING];
    struct sys_persist_info *persist = ctx->priv.st->get_persist_sys_info();
    int work_time = 0;
    uint16_t pump_time = 0;
    ctx->priv.st->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, ctx->priv.m_info.mode, 0, &pump_time);
    work_time = pump_time;
    if ((work_time * 1000) > SETTING_PUMP_TIME_MAX) {
        work_time = SETTING_PUMP_TIME_MAX / 1000;
    }
    result_info->setting_time = work_time * 1000;
    if (status == BREAST_PUMP_START) {
        breast_pump_result_info_reset(ctx, result_info);  
    } else {
        breast_pump_result_info_update(ctx, result_info);
        if (result_info->mode != ctx->priv.m_info.mode) {
            result_info->current_mode_work_time = 0;
        }
    }
    result_info->start_time = get_timestamp();
    result_info->mode = ctx->priv.m_info.mode;
    memcpy(result_info->setting, ctx->priv.setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
    LOG_PMP("mode: %d, work_time: %d, setting_time: %d, mode_time: %d, status: %d", 
        result_info->mode, work_time, 
        PUMP_TIME_TO_SECOND(result_info->setting_time), 
        result_info->current_mode_work_time, status);
    return 0;
}

static int stop_breast_pump_work_route(breast_pump_manager_context_t *ctx, int status)
{
    pump_result_info_t *result_info = &ctx->priv.result_info[BREAST_PUMP_RESULT_PUMPING];
    sys_persist_info_t *sys_info = ctx->priv.st->get_persist_sys_info();
    EXCEPTION_CHECK_INT(sys_info);
    /// 矫正时间同步后的泵奶开始时间
    if (result_info->timestamp < PRODUCT_BUILD_TIMESTAMP) { 
        uint32_t timestamp = platform_get_utc_time();
        if (timestamp > PRODUCT_BUILD_TIMESTAMP) {
            result_info->timestamp = timestamp;
            LOG_PMP("breast pump timestamp calib: %lu", result_info->timestamp);
        }
    }
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_breast_pump_hook(breast_pump_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __breast_pump_hook));
    ctx->hook->breast_pump_completed_route = breast_pump_completed_route;
    ctx->hook->make_progress_route = make_progress_route;
    ctx->hook->start_breast_pump_work_route = start_breast_pump_work_route;
    ctx->hook->stop_breast_pump_work_route = stop_breast_pump_work_route;
}

 