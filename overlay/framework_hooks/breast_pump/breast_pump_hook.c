/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "breast_pump_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_INFO_LOG_INTERVAL                             (2000 / PUMP_WORK_STEP_TIME_MS) 
#define PUMP_WILL_FINISH_TIME_MS                           (2300)
#define PUMP_TIME_TO_SECOND(time)                          (time / 1000)
#if 1
#define PUMP_AUTO_PAUSE_TIME_MS                            (30 * 60 * 1000)
#else
#define PUMP_AUTO_PAUSE_TIME_MS                            (20 * 1000)
#endif

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
    
    memcpy(history_info, result_info, sizeof(pump_result_info_t)); /// 保存上次有效泵奶记录
    history_info->finish_timestamp = platform_get_utc_time();
#if 0
    int user_bind_info = 0;
    ctx->priv.st->get_store_ram_val(STORE_ID_USER_BIND_INFO, 0, 0, &user_bind_info);
    if (!user_bind_info || result_info->report_flag) {
        LOG_PMP("no need to save, report_flag: %d", result_info->report_flag);
        need_save = 0;
    }
#endif
    return need_save;
}

static void update_work_valid_time(breast_pump_manager_context_t *ctx, pump_result_info_t *result_info)
{
    uint32_t current_mode_work_time = ctx->priv.motor->get_policy_running_time(ctx->priv.m_info.mode);
    uint32_t offset_work_time = current_mode_work_time > result_info->current_mode_work_time ? 
        current_mode_work_time - result_info->current_mode_work_time : 0;
    if (offset_work_time < 100) {
        result_info->work_valid_time += offset_work_time;
    }
    result_info->current_mode_work_time = current_mode_work_time;
    ctx->priv.m_result.work_valid_time = result_info->work_valid_time;
}


#if IS_SUPPORT_MODE_WORK_TIME_ACCUMULATE
static void breast_pump_mode_update(breast_pump_manager_context_t *ctx, pump_result_info_t *result_info)
{
    int pump_time = 0;
    breast_pump_info_t *m_info = &ctx->priv.m_info;
    breast_pump_result_t *m_result = &ctx->priv.m_result;
    ctx->priv.st->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, m_info->mode, 0, &pump_time);
    result_info->setting_time = pump_time * 1000;
    if (result_info->setting_time > SETTING_PUMP_TIME_MAX) {
        result_info->setting_time = SETTING_PUMP_TIME_MAX;
    }
    result_info->target_valid_time = result_info->work_valid_time + result_info->setting_time;
    m_result->total_valid_time = result_info->target_valid_time;
    if (result_info->target_valid_time > SETTING_PUMP_TIME_MAX) {
        LOG_PMP("target_valid_time overflow: %d/%d/%d", 
            result_info->target_valid_time, result_info->work_valid_time, result_info->setting_time);
        result_info->target_valid_time = SETTING_PUMP_TIME_MAX;
    }

    if (PUMP_IS_COMMON_MODE(m_info->mode)) {
        m_info->common_mode = m_info->mode;
        ctx->priv.st->put_store_ram_val(STORE_ID_BREAST_PUMP_MODE_WITH_TYPE,
            BREAST_PUMP_MODE_TYPE_COMMON, m_info->mode, NULL);
    } else {
        m_info->breastfeeding_mode = m_info->mode;
        ctx->priv.st->put_store_ram_val(STORE_ID_BREAST_PUMP_MODE_WITH_TYPE,
            BREAST_PUMP_MODE_TYPE_BREASTFEEDING, m_info->mode, NULL);
    }
    /// 更新历史运行时间
    result_info->hostory_work_time += result_info->current_mode_work_time;
    /// 清除模式更新标志
    ctx->priv.st->put_store_ram_val(STORE_ID_MODE_NEW_STEP_INFO_FLAG, m_info->mode, false, NULL);
    if (result_info->work_valid_time < PUMP_AUTO_PAUSE_TIME_MS) {
        m_info->auto_pause_onoff = true;
    }
    LOG_PMP("mode update:%d/%d; common:%d expert:%d; auto pause:%d time:%d/%d/%d", 
        result_info->mode, m_info->mode, m_info->common_mode, 
        m_info->breastfeeding_mode, m_info->auto_pause_onoff,
        PUMP_TIME_TO_SECOND(result_info->current_mode_work_time), 
        PUMP_TIME_TO_SECOND(result_info->hostory_work_time),
        PUMP_TIME_TO_SECOND(result_info->target_valid_time));
    /// todo:更新数据到缓存    
    result_info->current_mode_work_time = 0;
    result_info->mode = m_info->mode;
    result_info->start_time = get_timestamp();
    memcpy(result_info->setting, ctx->priv.setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
}
#else
static void breast_pump_mode_update(breast_pump_manager_context_t *ctx, pump_result_info_t *result_info)
{
    int pump_time = 0;
    breast_pump_info_t *m_info = &ctx->priv.m_info;
    breast_pump_result_t *m_result = &ctx->priv.m_result;
    ctx->priv.st->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, m_info->mode, 0, &pump_time);
    result_info->setting_time = pump_time * 1000;
    if (result_info->setting_time > SETTING_PUMP_TIME_MAX) {
        result_info->setting_time = SETTING_PUMP_TIME_MAX;
    }
    result_info->target_valid_time = result_info->setting_time;
    m_result->total_valid_time = result_info->target_valid_time;
    result_info->hostory_work_time += result_info->current_mode_work_time;
    if (PUMP_IS_COMMON_MODE(m_info->mode)) {
        update_work_valid_time(ctx, result_info);
        m_info->common_mode = m_info->mode;
        ctx->priv.st->put_store_ram_val(STORE_ID_BREAST_PUMP_MODE_WITH_TYPE,
            BREAST_PUMP_MODE_TYPE_COMMON, m_info->mode, NULL);
    } else {
        result_info->work_valid_time = 0;
        ctx->priv.m_result.work_valid_time = result_info->work_valid_time;
        m_info->breastfeeding_mode = m_info->mode;
        ctx->priv.st->put_store_ram_val(STORE_ID_BREAST_PUMP_MODE_WITH_TYPE,
            BREAST_PUMP_MODE_TYPE_BREASTFEEDING, m_info->mode, NULL);
    }
    /// 清除模式更新标志
    ctx->priv.st->put_store_ram_val(STORE_ID_MODE_NEW_STEP_INFO_FLAG, m_info->mode, false, NULL);
    if (result_info->work_valid_time < PUMP_AUTO_PAUSE_TIME_MS) {
        m_info->auto_pause_onoff = true;
    }
    LOG_PMP("mode update:%d/%d; common:%d expert:%d; auto pause:%d time:(%d/%d)%d/%d", 
        result_info->mode, m_info->mode, m_info->common_mode,
        m_info->breastfeeding_mode, m_info->auto_pause_onoff,
        PUMP_TIME_TO_SECOND(result_info->current_mode_work_time), 
        PUMP_TIME_TO_SECOND(result_info->hostory_work_time),
        PUMP_TIME_TO_SECOND(result_info->work_valid_time),
        PUMP_TIME_TO_SECOND(result_info->target_valid_time));
    result_info->current_mode_work_time = 0;
    result_info->mode = m_info->mode;
    result_info->start_time = get_timestamp();
    memcpy(result_info->setting, ctx->priv.setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
}
#endif

static int local_breast_pump_process(breast_pump_manager_context_t *ctx, pump_result_info_t *result_info)
{
    static uint8_t log_timer = 0;
    int ret = RESULT_PUMP_NORMAL;
    log_timer++;
    /// 更新模式和力度.
    ctx->priv.m_result.working_mode = ctx->priv.m_info.mode;
    memcpy(result_info->setting, ctx->priv.setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
    /// 更新运行时间
    update_work_valid_time(ctx, result_info);
#if IS_SUPPORT_MODE_WORK_TIME_ACCUMULATE
    if ((result_info->work_valid_time >= result_info->target_valid_time  || 
        result_info->work_valid_time >= SINGLE_PUMP_TIME_MAX)  && 
        result_info->target_valid_time != 0) {
        LOG_PMP("finish, work_time=%d/%d, mode_time=%d", 
            PUMP_TIME_TO_SECOND(result_info->work_valid_time), PUMP_TIME_TO_SECOND(result_info->target_valid_time),
            result_info->current_mode_work_time);   
        log_timer = 0;
        return RESULT_PUMP_FINISH;
    }
#else
    if ((result_info->work_valid_time >= result_info->target_valid_time && result_info->target_valid_time != 0)) {
        LOG_PMP("finish, work_time=%d/%d, mode_time=%d", 
            PUMP_TIME_TO_SECOND(result_info->work_valid_time), PUMP_TIME_TO_SECOND(result_info->target_valid_time),
            result_info->current_mode_work_time);   
        log_timer = 0;
        return RESULT_PUMP_FINISH;
    }
#endif
    
    /// 判断是否需要自动暂停
    if (ctx->priv.m_info.auto_pause_onoff && result_info->work_valid_time >= PUMP_AUTO_PAUSE_TIME_MS) {
        ctx->priv.m_info.auto_pause_onoff = false;
        if (PUMP_IS_COMMON_MODE(ctx->priv.m_info.mode)) {
            ret = RESULT_PUMP_AUTO_PAUSE;
        }
    }
    
    /// 判断是否即将结束
    if ((result_info->work_valid_time + PUMP_WILL_FINISH_TIME_MS) >= result_info->target_valid_time) {
        ret = RESULT_PUMP_WILL_FINISH;
    }

    /// 状态输出
    if (log_timer > PUMP_INFO_LOG_INTERVAL) {
        int time_index = 0;
        pump_setting_t setting;
        READ_PUMP_MODE_STEP_INDEX(ctx, time_index);
        READ_PUMP_SETTING_INFO(ctx, time_index, setting);
        uint32_t time_policy_remaining_time = ctx->priv.motor->get_time_policy_remaining_time();
        LOG_PMP("%s, work_time=%d/%d/%d/%d, policy_rtime=%d, mode_time=%d, setting=%d/%d/%d", 
            ret == RESULT_PUMP_WILL_FINISH ? "will finish" : "normal",
            PUMP_TIME_TO_SECOND(result_info->work_valid_time), PUMP_TIME_TO_SECOND(result_info->hostory_work_time), 
            PUMP_TIME_TO_SECOND(result_info->target_valid_time), PUMP_TIME_TO_SECOND(result_info->setting_time), 
            time_policy_remaining_time, PUMP_TIME_TO_SECOND(result_info->current_mode_work_time), 
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

static void start_breast_pump_work_route(breast_pump_manager_context_t *ctx, int status)
{    
    struct storage_manager_context *st = ctx->priv.st;
    EXCEPTION_CHECK_VOID(st);
    pump_result_info_t *result_info = &ctx->priv.result_info[BREAST_PUMP_RESULT_PUMPING];
    struct sys_persist_info *persist = ctx->priv.st->get_persist_sys_info();
    if (status == BREAST_PUMP_START) {
        result_info->work_valid_time = 0;
        result_info->hostory_work_time = 0;
        result_info->current_mode_work_time = 0;
        result_info->start_time = get_timestamp();
        breast_pump_mode_update(ctx, result_info);      
    } else {
        result_info->start_time = get_timestamp();
        if (result_info->mode != ctx->priv.m_info.mode) {
            breast_pump_mode_update(ctx, result_info);
        }
        LOG_PMP("mode %d continue", result_info->mode);
    }
}

static void stop_breast_pump_work_route(breast_pump_manager_context_t *ctx, int status)
{
    pump_result_info_t *result_info = &ctx->priv.result_info[BREAST_PUMP_RESULT_PUMPING];
    LOG_PMP("stop, work_time=%d", PUMP_TIME_TO_SECOND(result_info->work_valid_time));
}

static void mode_change_route(breast_pump_manager_context_t *ctx, uint8_t mode)
{
    breast_pump_info_t *m_info = &ctx->priv.m_info;
    if (m_info->status == BREAST_PUMP_START || m_info->status == BREAST_PUMP_CONTINUE || m_info->status == BREAST_PUMP_PAUSE) {
        LOG_PMP("mode change: %d", mode);
        breast_pump_mode_update(ctx, &ctx->priv.result_info[BREAST_PUMP_RESULT_PUMPING]);
    } else {
        LOG_PMP("mode change: %d, ignore", mode);
    }
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
    ctx->hook->mode_change_route = mode_change_route;
}

 