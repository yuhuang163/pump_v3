/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "breast_pump_manager.h"
#include "storage_manager_api.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_INFO_LOG_INTERVAL                             (10)            

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
    pump_result_info_t *result_info = ctx->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
    EXCEPTION_CHECK_INT(result_info);
    pump_result_info_t *history_info  = ctx->get_breast_pump_result(BREAST_PUMP_RESULT_HISTORY);
    EXCEPTION_CHECK_INT(history_info);
    if (result_info->work_valid_time < ROOT_BREAST_PUMP_TIME_VALID) {
        LOG_PMP("no need to save, work_valid_time: %lu", result_info->work_valid_time);
        need_save = 0;
    } else {
        memcpy(history_info, result_info, sizeof(pump_result_info_t)); /// 保存上次有效泵奶记录
    }

    int user_bind_info = 0;
    ctx->priv.st->get_store_ram_val(STORE_ID_USER_BIND_INFO, 0, 0, &user_bind_info);
    if (!user_bind_info || result_info->report_flag) {
        LOG_PMP("no need to save, report_flag: %d", result_info->report_flag);
        need_save = 0;
    }

    return need_save;
}

static int local_breast_pump_process(breast_pump_manager_context_t *ctx, pump_result_info_t *result_info)
{
    static uint8_t shake_state = 0, log_timer = 0;
    int ret = RESULT_PUMP_NORMAL;

    log_timer++;
    if (result_info->work_total_time >= result_info->target_total_time || result_info->work_valid_time >= result_info->target_valid_time) {
        LOG_PMP("finish, total: work_time=%d, target_time=%d; valid: work_time=%d, target_time=%d, mode_time=%d", 
            result_info->work_total_time, result_info->target_total_time, result_info->work_valid_time, result_info->target_valid_time,
            result_info->current_mode_work_time);   
        log_timer = 0;
        return RESULT_PUMP_FINISH;
    }
    /// 更新自然时间.
    result_info->work_total_time += PUMP_WORK_STEP_TIME_MS;
    result_info->current_mode_work_time += PUMP_WORK_STEP_TIME_MS;
    /// 更新模式和力度.
    result_info->mode = ctx->priv.m_info.mode;
    ctx->get_breast_pump_setting(result_info->setting, ROOT_BREAST_PUMP_STEP_MAX);
    /// 有效时间累加，更新各区域有效时间.
    result_info->work_valid_time += PUMP_WORK_STEP_TIME_MS;
    if (log_timer > PUMP_INFO_LOG_INTERVAL) {
        LOG_PMP("normal, total: work_time=%d, target_time=%d; valid: work_time=%d, target_time=%d, mode_time=%d", 
            result_info->work_total_time, result_info->target_total_time, result_info->work_valid_time, result_info->target_valid_time,
            result_info->current_mode_work_time);
        log_timer = 0;
    }
    return ret;
}

static int make_progress_route(breast_pump_manager_context_t *ctx, pump_result_info_t *result_info)
{
    int notify_id = -1;

    EXCEPTION_CHECK_INT(result_info);
    result_info->mode = ctx->priv.m_info.mode;
    ctx->get_breast_pump_setting(result_info->setting, ROOT_BREAST_PUMP_STEP_MAX);
    notify_id = local_breast_pump_process(ctx, result_info);

    return notify_id;
}

static int breast_pump_result_info_reset(breast_pump_manager_context_t *ctx, pump_result_info_t *result_info)
{
    result_info->work_valid_time  = 0;
    result_info->target_valid_time = result_info->setting_time;
    result_info->target_total_time = result_info->setting_time + result_info->work_total_time;
    result_info->work_total_time = 0;
    result_info->timestamp = platform_get_utc_time();
    dump_timestamp(result_info->timestamp);
    LOG_PMP("target_total_time: %lu, target_valid_time: %lu, setting_time: %lu",
                result_info->target_total_time, result_info->target_valid_time, result_info->setting_time);

    return 0;
}

static int breast_pump_result_info_app_reset(breast_pump_manager_context_t *ctx, pump_result_info_t *result_info)
{
    result_info->work_valid_time  = 0;
    result_info->target_valid_time = result_info->setting_time;
    result_info->target_total_time = 0;
    result_info->work_total_time = 0;
    result_info->timestamp = platform_get_utc_time();
    dump_timestamp(result_info->timestamp);
    LOG_PMP("target_total_time: %lu, target_valid_time: %lu, setting_time: %lu",
                result_info->target_total_time, result_info->target_valid_time, result_info->setting_time);

    return 0;
}

static int breast_pump_result_info_update(breast_pump_manager_context_t *ctx, pump_result_info_t *result_info)
{
    if (result_info->target_valid_time == result_info->work_valid_time) { 
        result_info->target_valid_time += result_info->setting_time;
        if (result_info->target_valid_time >= SINGLE_PUMP_TIME_MAX) {
            result_info->target_valid_time = SINGLE_PUMP_TIME_MAX;
        }
        result_info->target_total_time = result_info->work_total_time + result_info->setting_time + PUMP_WORK_TIME_INC;
        LOG_PMP("1-total: target_time=%d, work_time=%d; valid: target_time=%d, work_time=%d，setting_time=%d",
            result_info->target_total_time, result_info->work_total_time, result_info->target_valid_time,
            result_info->work_valid_time, result_info->setting_time);
        return 0;
    } else if (result_info->target_total_time == result_info->work_total_time) {
        result_info->target_total_time = result_info->target_total_time + result_info->setting_time + PUMP_WORK_TIME_INC;
        LOG_PMP("2-total: target_time=%d, work_time=%d; valid: target_time=%d, work_time=%d，setting_time=%d",
            result_info->target_total_time, result_info->work_total_time, result_info->target_valid_time,
            result_info->work_valid_time, result_info->setting_time);
        return 0;
    } else {
        LOG_PMP("3-total: target_time=%d, work_time=%d; valid: target_time=%d, work_time=%d，setting_time=%d",
            result_info->target_total_time, result_info->work_total_time, result_info->target_valid_time, 
            result_info->work_valid_time, result_info->setting_time);
        return -1;
    }

}

static int start_breast_pump_work_route(breast_pump_manager_context_t *ctx, int status)
{    
    struct storage_manager_context *st = ctx->priv.st;
    EXCEPTION_CHECK_INT(st);
    pump_result_info_t *result_info = ctx->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
    EXCEPTION_CHECK_INT(result_info);
    struct sys_persist_info *persist = ctx->priv.st->get_persist_sys_info();

    int work_time = 0;
    ctx->priv.st->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, ctx->priv.m_info.mode, 0, &work_time);
    if ((work_time * 1000) > SETTING_PUMP_TIME_MAX) {
        work_time = SETTING_PUMP_TIME_MAX / 1000;
    }
    result_info->setting_time = work_time * 1000;
    /// 模式或状态变化时，重置当前模式运行时间.
    if (result_info->mode != ctx->priv.m_info.mode || status == BREAST_PUMP_START) {
        result_info->current_mode_work_time = 0;
    }
    result_info->mode = ctx->priv.m_info.mode;
    ctx->get_breast_pump_setting(result_info->setting, ROOT_BREAST_PUMP_STEP_MAX);
    LOG_PMP("start pump work, mode: %d, work_time: %d, setting_time: %d, mode_time: %d, status: %d", 
        result_info->mode, work_time, result_info->setting_time, result_info->current_mode_work_time, status);
    /// 初始化泵奶数据.
    if (status == BREAST_PUMP_START) {
        breast_pump_result_info_reset(ctx, result_info);  
        // ctx->m_result.notify_ui.expected_valid_time_sec = algo_breast_pump->target_valid_time / 1000;
        // ctx->m_result.notify_ui.expected_elapse_time_sec = algo_breast_pump->target_total_time / 1000;
    } 
    /// 无限续刷逻辑处理
    if (status == BREAST_PUMP_CONTINUE) { 
        if (breast_pump_result_info_update(ctx, result_info) == 0) {
            // ctx->m_result.notify_ui.expected_valid_time_sec = algo_breast_pump->target_valid_time / 1000;
            // ctx->m_result.notify_ui.expected_elapse_time_sec = algo_breast_pump->target_total_time / 1000;
        }
    }
    /// 获取当前模式.
    result_info->mode = ctx->get_breast_pump_mode();
    /// 从ram中获取当前模式下的设置信息.
    st->get_store_ram_val(STORE_ID_PUMP_SETTING_INFO, result_info->mode, ROOT_BREAST_PUMP_STEP_MAX, &result_info->setting);
    ctx->set_breast_pump_setting(result_info->setting, ROOT_BREAST_PUMP_STEP_MAX);    

    return 0;
}

static int stop_breast_pump_work_route(breast_pump_manager_context_t *ctx, int status)
{
    pump_result_info_t *result_info = ctx->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
    EXCEPTION_CHECK_INT(result_info);
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

    if (sys_info->pump_info.is_remind_pump_finish > 0) {
        LOG_PMP("remind breast pump finish");
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
    ctx->hook->breast_pump_result_info_app_reset = breast_pump_result_info_app_reset;
}

 