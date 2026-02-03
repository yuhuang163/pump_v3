#ifndef __LIGHT_SENSOR_MODULE_H__
#define __LIGHT_SENSOR_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

#define LIGHT_SENSOR_POLLING_PERIOD                   (100)

struct rt_device_node;
/// 光传感器上下文
typedef struct light_sensor_context {
    /// private
    struct __light_sensor_private {
        struct rt_device_node *dev;
        uint32_t polling_cnt;
        uint16_t sampling_interval;
    } priv;
    /// public
    uint16_t (*get_sampling_interval)(void);
    void (*set_sampling_interval)(uint16_t interval);
} light_sensor_context_t;

#ifdef __cplusplus
}
#endif

#endif 







