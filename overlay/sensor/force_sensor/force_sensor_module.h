#ifndef __FORCE_SENSOR_MODULE_H__
#define __FORCE_SENSOR_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

#define F_SENSOR_POLLING_PERIOD              (160) 

struct rt_device_node;
/// 压力传感器上下文
typedef struct force_sensor_context {
    /// private
    struct __force_sensor_private {
        struct rt_device_node *dev;
        uint32_t polling_cnt;
    } priv;
} force_sensor_context_t;

#ifdef __cplusplus
}
#endif

#endif 







