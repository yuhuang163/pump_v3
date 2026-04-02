/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "include/algo_fusion_lib.h" // IWYU pragma: keep
#include "wear_detecttion/v3_wear_detection.h" // IWYU pragma: keep
#include "wear_detecttion/v3_pressure_compensation_algo.h" // IWYU pragma: keep
/*
* DEFINES
*****************************************************************************************
*/
#define ALGO_FUSION_QUEUE_SIZE         10
#define ALGO_FUSION_QUEUE_ITEM_SIZE    sizeof(uint8_t)
#define ALGO_FUSION_LIB_VERSION        100

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static algo_fusion_result_t s_result = {0};
static algo_os_thread_id algo_thread_id = NULL;
static algo_os_message_id algo_msg_queue_id = NULL;
static algo_fusion_task_list_e s_task_flag = 0;
static algo_fusion_sensor_attr_type_e s_sensor_flag = 0;
static aglo_air_pressure_data_t s_air_pressure_data = {0};
/// 回调函数
static algo_result_callback s_result_cb = NULL;
static algo_sensor_attr_cfg_callback s_set_sensor_attr_cb = NULL;
static algo_sensor_attr_cfg_callback s_get_sensor_attr_cb = NULL;
static algo_write_nv_data_callback s_write_nv_data_cb = NULL;
static algo_read_nv_data_callback s_read_nv_data_cb = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void wear_detection_process_data(void)
{
    if (s_task_flag & (1 << WEAR_DETECTION_TASK)) {
        /// TODO: 实现佩戴检测算法
        /*
        V3WearDetectionMain(s_air_pressure_data.air_pressure, s_air_pressure_data.pump_state[0].level, 
            s_air_pressure_data.pump_state[1].level,
            s_air_pressure_data.pump_state[0].mode, 
            s_air_pressure_data.pump_state[1].mode);
        //ALGO_LOG_INFO("wear_detection_process_data: %f, %f", s_air_pressure_data.air_pressure[0], s_air_pressure_data.air_pressure[1]);
        /// TODO: 修改算法结果s_result
        GetWearDetectionResult(s_result.wear_status);
        */

        // TODO: 实现气压补偿算法
        V3ProAlgoMain(s_air_pressure_data);
        GetCompensationResult(s_result.compensation_status, s_result.compensation_delta_pwm);
        #if IS_SUPPORT_ALGO_LIB_LOG_OUTPUT
        ALGO_LOG_INFO("time_stamp: %d, air pressure: p1: %f, p2: %f", s_air_pressure_data.timestamp, s_air_pressure_data.air_pressure[0], s_air_pressure_data.air_pressure[1]);
        ALGO_LOG_INFO("pump state: %d, %d", s_air_pressure_data.pump_state[0].state, s_air_pressure_data.pump_state[1].state);
        ALGO_LOG_INFO("status: %d/%d, delta_pwm: %d, %d", s_result.compensation_status[0], s_result.compensation_status[1], s_result.compensation_delta_pwm[0], s_result.compensation_delta_pwm[1]);
        #endif
    }
}


static void algo_fusion_work_thread(void *args)
{ 
    uint8_t data_type = 0;
    ALGO_QUEUE_CREATE(&algo_msg_queue_id, "algo_fusion_queue", ALGO_FUSION_QUEUE_SIZE, ALGO_FUSION_QUEUE_ITEM_SIZE);
    for (;;) {
        ALGO_QUEUE_RECV(algo_msg_queue_id, &data_type, ALGO_OS_WAIT_FOREVER);
        #if IS_SUPPORT_ALGO_LIB_LOG_OUTPUT
        ALGO_LOG_INFO("algo_fusion_work_thread: %d", data_type);
        #endif
        switch (data_type) {
            case ALGO_SENSOR_ATTR_AIR_PRESSURE: {
                wear_detection_process_data();
                break;
            }
            default:
                break;
        }
        s_result_cb(&s_result); /// 回调结果
    } 
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int algo_fusion_depend_api_register(const algo_fusion_depend_api_t *depend_api)
{
    if (depend_api == NULL) {
        return ALGO_DEPEND_STRUCT_NULL;
    }
    s_result_cb = depend_api->result_cb;
    s_set_sensor_attr_cb = depend_api->set_sensor_attr_cb;
    s_get_sensor_attr_cb = depend_api->get_sensor_attr_cb;
    s_write_nv_data_cb = depend_api->write_nv_data_cb;
    s_read_nv_data_cb = depend_api->read_nv_data_cb;

    return ALGO_DEPEND_REGISTER_SUCCESS;
}

void algo_fusion_lib_input(algo_fusion_input_data_t *input)
{
    uint8_t data_type;
    data_type = input->data_type;
    if (input == NULL || algo_msg_queue_id == NULL) {
        return;
    }
    /// TODO: 是否需要缓存数据，比如未泵奶的情况
    /// TODO: 缓存数据到本地，避免阻塞
    /// TODO: 监听缓存是否满，满了发消息给算法线程
    #if IS_SUPPORT_ALGO_LIB_LOG_OUTPUT
    ALGO_LOG_INFO("algo_fusion_lib_input: %d", data_type);
    #endif
    // data type
    switch (data_type){
        case ALGO_SENSOR_ATTR_AIR_PRESSURE:
        {
            s_air_pressure_data.air_pressure[0] = ((aglo_air_pressure_data_t*)input->sensor_data)->air_pressure[1];     // 气压数据反了
            s_air_pressure_data.air_pressure[1] = ((aglo_air_pressure_data_t*)input->sensor_data)->air_pressure[0];     // 气压数据反了
            s_air_pressure_data.pump_state[0].level = ((aglo_air_pressure_data_t*)input->sensor_data)->pump_state[0].level;
            s_air_pressure_data.pump_state[1].level = ((aglo_air_pressure_data_t*)input->sensor_data)->pump_state[1].level;
            s_air_pressure_data.pump_state[0].mode = ((aglo_air_pressure_data_t*)input->sensor_data)->pump_state[0].mode;
            s_air_pressure_data.pump_state[1].mode = ((aglo_air_pressure_data_t*)input->sensor_data)->pump_state[1].mode;
            s_air_pressure_data.pump_state[0].state = ((aglo_air_pressure_data_t*)input->sensor_data)->pump_state[0].state;
            s_air_pressure_data.pump_state[1].state = ((aglo_air_pressure_data_t*)input->sensor_data)->pump_state[1].state;
            s_air_pressure_data.pump_state[0].pump_duty = ((aglo_air_pressure_data_t*)input->sensor_data)->pump_state[0].pump_duty;
            s_air_pressure_data.pump_state[1].pump_duty = ((aglo_air_pressure_data_t*)input->sensor_data)->pump_state[1].pump_duty;
            s_air_pressure_data.pump_state[0].valve_duty = ((aglo_air_pressure_data_t*)input->sensor_data)->pump_state[0].valve_duty;
            s_air_pressure_data.pump_state[1].valve_duty = ((aglo_air_pressure_data_t*)input->sensor_data)->pump_state[1].valve_duty;
            s_air_pressure_data.pump_state[0].target_pressure = ((aglo_air_pressure_data_t*)input->sensor_data)->pump_state[0].target_pressure;
            s_air_pressure_data.pump_state[1].target_pressure = ((aglo_air_pressure_data_t*)input->sensor_data)->pump_state[1].target_pressure;
            s_air_pressure_data.timestamp = ((aglo_air_pressure_data_t*)input->sensor_data)->timestamp;
            break;
        }
        default:
            break;
    }
    ALGO_QUEUE_SEND(algo_msg_queue_id, &data_type, 0); /// 将缓存已满的数据类型发给算法线程
}

int algo_fusion_lib_init(algo_fusion_sensor_attr_type_e sensor_flag, algo_fusion_task_list_e task_flag)
{
    s_sensor_flag = sensor_flag;
    s_task_flag = task_flag;
    if (algo_thread_id == NULL) {
        algo_thread_id = ALGO_THREAD_CREATE("algo_fusion_thread", algo_fusion_work_thread, RT_PRIO_ALGO_DISPATCH_THREAD, 2048);
        if (algo_thread_id == NULL) {
            return ALGO_FUSION_INIT_RUN_OUT_OF_MEMORY;
        }
        // just for initialization - not the final parameters
        // V3WearDetectionInit(0, 0, 0, 0);
        V3ProAlgoInit(s_air_pressure_data);
    }
    return ALGO_FUSION_INIT_SUCCESS;
}

void algo_fusion_lib_deinit(void)
{
    /// 不释放内存，避免碎片无法创建
}

void algo_fusion_reset(void)
{
    /// TODO: 重置算法内部状态
    ALGO_LOG_INFO("algo_fusion_reset");
}

int get_algo_fusion_lib_version(void)
{
    return ALGO_FUSION_LIB_VERSION;
}
