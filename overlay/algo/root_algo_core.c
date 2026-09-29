/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "algorithm_manager.h"  // IWYU pragma: keep
#include "algo_fusion_lib.h" // IWYU pragma: keep
#include "root_algo_core.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define ALGO_LOG_LINE_BUF_SIZE                  512
#define IS_SUPPORT_ALGO_LOG_OUTPUT              0

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static algo_manager_context_t *s_algo_ctx = {0};
#if IS_SUPPORT_ALGO_LOG_OUTPUT
static char s_log_encode_buf[ALGO_LOG_LINE_BUF_SIZE] = {0};
#endif

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_DEVICE_AIR_PRESSURE
static void on_sensor_data_algo_cb(void *user_data, uint8_t *data, int data_len)
{
    algo_manager_context_t *ctx = user_data;
    EXCEPTION_CHECK_VOID(ctx);
    if (ctx->priv.is_sampling) {
        algo_sampling_data_cb(ctx, data, data_len);
    } else {
        algo_fusion_data_cb(ctx, data, data_len);
    }
}

static void algo_out_result_cb(algo_fusion_result_t *algo_result)
{  
    static uint16_t log_count = 0;
    uint8_t old_leakage[AIR_PRESSURE_NUM];
    EXCEPTION_CHECK_VOID(algo_result);
    algo_manager_context_t *ctx = s_algo_ctx;
    EXCEPTION_CHECK_VOID(ctx);
    /// 缓存原始算法结果，供算法结果上报（0x0008 notify）。
    /// 注意：必须在下方 is_algo_enabled 判断之前缓存——算法关闭时下方会覆写 algo_result 为 INVALID，
    /// 结果上报要求独立于算法开关（set_algo_enabled），故此处须先存原始值，勿挪进 if 分支。
    memcpy(&ctx->priv.latest_result, algo_result, sizeof(algo_fusion_result_t));
    memcpy(old_leakage, ctx->priv.algo_data.wear_detection.result, sizeof(old_leakage));
    if (!ctx->priv.is_algo_enabled) {
        /// 算法未启用，结果上报默认值
        algo_result->leakage_status[0] = ALGO_WEAR_DETECTION_RESULT_INVALID;
        algo_result->leakage_status[1] = ALGO_WEAR_DETECTION_RESULT_INVALID;
        algo_result->compensation_status[0] = 0;
        algo_result->compensation_status[1] = 0;
        algo_result->compensation_delta_pwm[0] = 0;
        algo_result->compensation_delta_pwm[1] = 0;
        algo_manager_dispatch_to_clients(ctx);
        return;
    }
    /// 仅当任一侧「新进入」微小漏气(2)/强漏气(3)时才上报异常：变化侧必须自身进入漏气态，
    /// 避免另一侧在非漏气态之间爬坡(0→1)时被误判为“状态变化”而重复上报。
    if ((algo_result->leakage_status[0] >= ALGO_WEAR_DETECTION_RESULT_SMALL_LEAK &&
         algo_result->leakage_status[0] != old_leakage[0]) ||
        (algo_result->leakage_status[1] >= ALGO_WEAR_DETECTION_RESULT_SMALL_LEAK &&
         algo_result->leakage_status[1] != old_leakage[1])) {
        ///增加算法输出
        LOG_ALGO("bury leakage:%d/%d",algo_result->leakage_status[0], algo_result->leakage_status[1]); 
        /// 漏气异常上报 EVT_011：error_code=AIR_LEAKAGE(0x24)，error_data 低8位左/次8位右
        DATA_BURY_EVENT_TRIGGER_EXCEPTION(DATA_BURY_ERROR_AIR_LEAKAGE,
            DATA_BURY_PACK_WEAR_DETECTION_DATA(algo_result->leakage_status[0],
                                               algo_result->leakage_status[1]));
    }
    ctx->priv.algo_data.wear_detection.result[0] = algo_result->leakage_status[0];
    ctx->priv.algo_data.wear_detection.result[1] = algo_result->leakage_status[1];
    ctx->priv.algo_data.wear_detection.wear_result[0] = algo_result->weared_status[0];
    ctx->priv.algo_data.wear_detection.wear_result[1] = algo_result->weared_status[1];
    ctx->priv.algo_data.wear_detection.compensation_status[0] = algo_result->compensation_status[0];
    ctx->priv.algo_data.wear_detection.compensation_status[1] = algo_result->compensation_status[1];
    ctx->priv.algo_data.wear_detection.compensation_delta_pwm[0] = algo_result->compensation_delta_pwm[0];
    ctx->priv.algo_data.wear_detection.compensation_delta_pwm[1] = algo_result->compensation_delta_pwm[1];

    /// 佩戴检测算法数据埋点：按 device_side 分左右，uGetResultFlag==1 时各带单通道 SRESULT_INFO 上报
    for (uint8_t side = 0; side < AIR_PRESSURE_NUM; side++) {
        if (algo_result->sDebugInfo[side].uGetResultFlag == 1) {
            data_bury_trigger_wear_detection_t wear = {0};
            wear.device_side = (side == 0) ? DATA_BURY_DEVICE_SIDE_LEFT : DATA_BURY_DEVICE_SIDE_RIGHT;
            wear.detection_type = DATA_BURY_WEAR_DETECTION_TYPE_PRESSURE;
            wear.detection_data = algo_result->weared_status[side];
            memcpy(wear.algorithm_data, &algo_result->sDebugInfo[side], sizeof(SRESULT_INFO));
            DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_WEAR_DETECTION, 0, 0, &wear);
        }
    }
    algo_manager_dispatch_to_clients(ctx);
    log_count++;
    if (log_count > 100) {
        log_count = 0;
        LOG_ALGO("leakage:%d/%d; weared:%d/%d; compensation:%d/%d; delta_pwm:%d/%d",
             algo_result->leakage_status[0], algo_result->leakage_status[1],
             algo_result->weared_status[0], algo_result->weared_status[1],
             algo_result->compensation_status[0], algo_result->compensation_status[1],
             algo_result->compensation_delta_pwm[0], algo_result->compensation_delta_pwm[1]);
    }
}

static void set_sensor_attr_cb(algo_fusion_sensor_attr_cfg_t *attr)
{
    EXCEPTION_CHECK_VOID(attr);
}

static void get_sensor_attr_cb(algo_fusion_sensor_attr_cfg_t *attr)
{
    EXCEPTION_CHECK_VOID(attr);
}

static void algo_fusion_nv_write_cb(algo_fusion_nv_data_t *para)
{
    EXCEPTION_CHECK_VOID(para);
}

static void algo_fusion_nv_read_cb(algo_fusion_nv_data_t *para)
{
    EXCEPTION_CHECK_VOID(para);
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void root_algo_init(algo_manager_context_t *ctx)
{
#if IS_SUPPORT_DEVICE_AIR_PRESSURE
    LOG_ALGO("enter");
    s_algo_ctx = ctx;
    const algo_fusion_depend_api_t api = {
        .result_cb          = algo_out_result_cb,
        .set_sensor_attr_cb = set_sensor_attr_cb,
        .get_sensor_attr_cb = get_sensor_attr_cb,
        .write_nv_data_cb   = algo_fusion_nv_write_cb,
        .read_nv_data_cb    = algo_fusion_nv_read_cb,
    };

    if (algo_fusion_depend_api_register(&api) != 0) {
        LOGE("register failure!");
    }

    algo_fusion_sensor_attr_type_e sensor_flag = 0; 
    algo_fusion_task_list_e task_flag = 0;
    task_flag = (1 << WEAR_DETECTION_TASK);
    sensor_flag = (1 << ALGO_SENSOR_ATTR_AIR_PRESSURE);
    int ret = algo_fusion_lib_init(sensor_flag, task_flag);
    LOG_ALGO("sensor_flag: 0x%x, task_flag: 0x%x, ret: %d", sensor_flag, task_flag, ret);
#endif
}

void root_algo_deinit(algo_manager_context_t *ctx)
{
#if IS_SUPPORT_DEVICE_AIR_PRESSURE
    algo_fusion_lib_deinit();
    LOG_ALGO("exit");
#endif
}

void root_algo_reset(algo_manager_context_t *ctx)
{
#if IS_SUPPORT_DEVICE_AIR_PRESSURE
    algo_fusion_reset();
    LOG_ALGO("RESET");
#endif
}

int root_algo_version_get(void)
{
    return get_algo_fusion_lib_version();
}

void root_algo_sensor_cb_register(algo_manager_context_t *ctx)
{
#if IS_SUPPORT_DEVICE_AIR_PRESSURE
    EXCEPTION_CHECK_VOID(ctx);
    sensor_manager_context_t *sensor = ctx->priv.sensor;
    EXCEPTION_CHECK_VOID(sensor);
    sensor->add_sensor_evt_lisenser(ctx, AIR_PRESSURE_EVT_POLLING, 1, on_sensor_data_algo_cb);
#endif
}
