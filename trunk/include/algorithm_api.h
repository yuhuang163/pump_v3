#ifndef __ALGO_MANAGER_API_H__
#define __ALGO_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h"

struct breath_pump_context;
struct input_dispatcher_context;
struct storage_context_struct;

// 算法接口.
typedef struct algo_fusion_context {
// private
    struct breath_pump_context *bth;
    struct input_dispatcher_context *input;
    struct storage_context_struct *st;
    broadcast_obj_t mbrodadcast;
    algo_breath_pump_data_t algo_breath_pump[ALGO_BREATH_PUMP_DATA_MAX];  
    int algo_inited;
#if ALGO_LIB_API_NORMALIZATION_SUPPORT
    algo_result_fusion_t *out_result;
#endif

// public
    int (*get_version)(void);
    algo_breath_pump_data_t *(*get_algo_breath_pump_result)(void);
    algo_breath_pump_data_t *(*get_breath_pump_info)(int index);

    struct __algo_fusion_hook {
        int (*on_sensor_data_cb_route)(struct algo_fusion_context *ctx, uint8_t *data, int data_len);
        int (*add_static_calib_cmpl_cb)(void (*cb)(void));
        void (*imu_static_calib_start)(void);
        void (*imu_static_calib_stop)(void);
    } *hook;

} algo_fusion_context_t;


void on_sensor_data_algo_cb(void *user_data, uint8_t *data, int data_len);

#ifdef __cplusplus
}
#endif

#endif 


