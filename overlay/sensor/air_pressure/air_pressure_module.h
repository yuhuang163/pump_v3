#ifndef __AIR_PRESSURE_MODULE_H__
#define __AIR_PRESSURE_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

#define AIR_PRESSURE_POLLING_PERIOD              (10)

struct rt_device_node;
typedef struct _air_pressure_module_data {
    struct rt_device_node *dev;
    uint32_t polling_cnt;
} air_pressure_module_data_t;

#ifdef __cplusplus
}
#endif

#endif 







