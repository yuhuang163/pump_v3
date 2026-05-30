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
    EXCEPTION_CHECK_VOID(algo_result);
    algo_manager_context_t *ctx = s_algo_ctx;
    EXCEPTION_CHECK_VOID(ctx);
    /*
    if (algo_result->wear_status[0] != ctx->priv.algo_data.wear_detection.result[0] || \
         algo_result->wear_status[1] != ctx->priv.algo_data.wear_detection.result[1]) {
        if (0 == algo_result->wear_status[0] || 0 == algo_result->wear_status[1]) {
            /// 未佩戴，通知app未佩戴
            servers_publish_data_args_t args_temp = {
                .arg1 = OLD_PUBLISH_ABNORMAL_AIR_LEAK,
                .arg2 = OLD_PUBLISH_ABNORMAL_STATE_ACTIVE,
                .val = NULL,
            };
            servers_manager_context_t *servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
            EXCEPTION_CHECK_VOID(servers_ctx);
            servers_ctx->hook->app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL, &args_temp);
        }
    }*/
    ctx->priv.algo_data.wear_detection.result[0] = algo_result->wear_status[0];
    ctx->priv.algo_data.wear_detection.result[1] = algo_result->wear_status[1];
    ctx->priv.algo_data.wear_detection.compensation_status[0] = algo_result->compensation_status[0];
    ctx->priv.algo_data.wear_detection.compensation_status[1] = algo_result->compensation_status[1];
    ctx->priv.algo_data.wear_detection.compensation_delta_pwm[0] = algo_result->compensation_delta_pwm[0];
    ctx->priv.algo_data.wear_detection.compensation_delta_pwm[1] = algo_result->compensation_delta_pwm[1];
    algo_manager_dispatch_to_clients(ctx);
    log_count++;
    if (log_count > 100) {
        log_count = 0;
        LOG_ALGO("wear status %d/%d", algo_result->wear_status[0], algo_result->wear_status[1]);
        LOG_ALGO("compensation status %d/%d %d/%d", algo_result->compensation_status[0], algo_result->compensation_status[1], algo_result->compensation_delta_pwm[0], \
                 algo_result->compensation_delta_pwm[1]);
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
