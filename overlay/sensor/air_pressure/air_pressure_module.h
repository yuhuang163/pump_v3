#ifndef __AIR_PRESSURE_MODULE_H__
#define __AIR_PRESSURE_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

#define AIR_PRESSURE_POLLING_PERIOD              (20)

struct rt_device_node;
/// 气压传感器上下文
typedef struct air_pressure_context {
    /// private
    struct __air_pressure_private {
        struct rt_device_node *dev;
        uint32_t polling_cnt;
        uint16_t sampling_interval;
    } priv;
    /// public
    uint16_t (*get_sampling_interval)(void);
    void (*set_sampling_interval)(uint16_t interval);
} air_pressure_context_t;

#ifdef __cplusplus
}
#endif

#endif 







