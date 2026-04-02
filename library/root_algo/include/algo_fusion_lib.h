#ifndef __ALGO_FUSION_LIB_H__
#define __ALGO_FUSION_LIB_H__

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "algo_fusion_os.h" // IWYU pragma: keep

#ifndef ALGO_AIR_PRESSURE_NUM
#define ALGO_AIR_PRESSURE_NUM 2
#endif

#define IS_SUPPORT_ALGO_LIB_LOG_OUTPUT     0

typedef struct {
    uint8_t wear_status[2];   /// 0: 未佩戴 1: 正常佩戴
    uint8_t compensation_status[2];     // 0: 不补偿, 1: 减小气压; 2:加大气压
    int compensation_delta_pwm[2];               // 0: 不补偿, -10000~-1:PWM减小, 1~10000: PWM增加
} algo_fusion_result_t;

typedef enum {
    ALGO_SENSOR_ATTR_ACC = 0,
    ALGO_SENSOR_ATTR_GYRO,
    ALGO_SENSOR_ATTR_MAG,
    ALGO_SENSOR_ATTR_AIR_PRESSURE,
    ALGO_SENSOR_ATTR_CAMERA,
    ALGO_SENSOR_ATTR_MAX
} algo_fusion_sensor_attr_type_e;

typedef struct {
    int16_t sensor_type;
    void *sensor_attr;
} algo_fusion_sensor_attr_cfg_t;

typedef struct {
    uint8_t data_type;
    float *data;
    uint32_t data_size;
} algo_fusion_nv_data_t;

typedef void (*algo_result_callback)(algo_fusion_result_t *);
typedef void (*algo_sensor_attr_cfg_callback)(algo_fusion_sensor_attr_cfg_t *);
typedef void (*algo_write_nv_data_callback)(algo_fusion_nv_data_t *);
typedef void (*algo_read_nv_data_callback)(algo_fusion_nv_data_t *);

typedef struct {
    algo_result_callback result_cb;
    algo_sensor_attr_cfg_callback set_sensor_attr_cb;
    algo_sensor_attr_cfg_callback get_sensor_attr_cb;
    algo_write_nv_data_callback write_nv_data_cb;
    algo_read_nv_data_callback read_nv_data_cb;
} algo_fusion_depend_api_t;

typedef enum {
    ALGO_DEPEND_REGISTER_SUCCESS = 0,               /// 依赖API注册成功
    ALGO_DEPEND_STRUCT_NULL,                        /// 依赖API结构体为空
    ALGO_DEPEND_RESULT_CB_NULL,                     /// 结果回调函数注册失败
    ALGO_DEPEND_LOG_CB_NULL,                        /// 日志回调函数注册失败
    ALGO_DEPEND_SENSOR_CFG_CB_NULL,                 /// 传感器配置回调函数注册失败
    ALGO_DEPEND_GET_SENSOR_ATTR_CB_NULL,            /// 获取传感器属性回调函数注册失败
    ALGO_DEPEND_WRITE_NV_DATA_CB_NULL,              /// 写NV数据回调函数注册失败
    ALGO_DEPEND_READ_NV_DATA_CB_NULL,               /// 读NV数据回调函数注册失败
} algo_fusion_depend_api_result_e;

typedef enum {
    ALGO_FUSION_INIT_SUCCESS = 0,                    /// 初始化成功
    ALGO_FUSION_INIT_DEPEND_API_NULL,                /// 依赖API注册失败
    ALGO_FUSION_INIT_RUN_OUT_OF_MEMORY,              /// 运行内存不足       
    ALGO_FUSION_INIT_DEINIT,                        
} algo_fusion_initialize_result_e;

/// 泵状态数据
typedef struct {
    uint8_t level;       /// 0-15
    uint8_t mode;        /// 0:刺激模式 1:吸乳模式 2:混合模式 3:个性化模式 4:PP追奶模式 5:2+8奶量提升模式 6:开奶模式
    uint8_t state;       /// 吸力状态 suction_status_e
    uint8_t pump_duty;        /// 泵奶力度
    uint8_t valve_duty;       /// 吸乳器电磁阀门力度
    uint8_t target_pressure;  /// 目标压力
    uint16_t run_time;        /// 运行时间，单位ms
} algo_pump_state_t;

/// 气压数据
typedef struct {
    uint8_t do_work;     /// 是否正在泵奶
    uint32_t timestamp;
    float air_pressure[ALGO_AIR_PRESSURE_NUM];
    algo_pump_state_t pump_state[ALGO_AIR_PRESSURE_NUM];
} aglo_air_pressure_data_t;

typedef struct {
    algo_fusion_sensor_attr_type_e data_type;        /// 数据类型
    void *sensor_data;                               /// 传感器数据
} algo_fusion_input_data_t;

typedef enum {
    WEAR_DETECTION_TASK = 0,  /// 佩戴检测任务
    ALGO_FUSION_TASK_LIST_MAX /// 任务列表最大值
} algo_fusion_task_list_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int algo_fusion_depend_api_register(const algo_fusion_depend_api_t *depend_api);
void algo_fusion_lib_input(algo_fusion_input_data_t *input);
int algo_fusion_lib_init(algo_fusion_sensor_attr_type_e sensor_flag, 
    algo_fusion_task_list_e task_flag);
void algo_fusion_lib_deinit(void);
void algo_fusion_reset(void);
int get_algo_fusion_lib_version(void);

#ifdef __cplusplus
}
#endif

#endif 
