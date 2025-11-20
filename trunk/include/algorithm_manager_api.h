#ifndef __ALGO_MANAGER_API_H__
#define __ALGO_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "broadcast_manager_api.h"
#include "root_algo.h" // IWYU pragma: keep

struct breast_pump_context;
struct input_dispatcher_context;
struct storage_manager_context;

/// 吸奶数据ID
typedef enum{
    ALGO_BREAST_PUMP_DATA_PUMPING,  /// 吸奶中的信息 
    ALGO_BREAST_PUMP_DATA_HISTORY,   /// 上次有效的泵奶信息 >30秒
    ALGO_BREAST_PUMP_DATA_MAX,
} algo_breast_pump_data_id_e;

/// 算法接口.
typedef struct algo_manager_context {
/// private
    struct __algo_private {
        broadcast_obj_t m_bcd;
        int algo_inited;
        uint8_t is_sampling;          /// 是否正在采集
    } priv;

/// public
    int (*get_algo_version)(void);
    struct __algo_manager_hook {
        int (*on_post_init_route)(struct algo_manager_context *ctx);
        int (*on_algo_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
    #if IS_SUPPORT_ALGO_SAMPLING
        void (*algo_sampling_start)(struct algo_manager_context *ctx);
        void (*algo_sampling_stop)(struct algo_manager_context *ctx);
    #endif
    #if IS_SUPPORT_DEVICE_IMU
        int (*add_static_calib_cmpl_cb)(void (*cb)(void));
        void (*imu_static_calib_start)(void);
        void (*imu_static_calib_stop)(void);
    #endif
    } *hook;

} algo_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


