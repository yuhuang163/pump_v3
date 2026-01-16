#ifndef __LIGHT_SENSOR_MODULE_H__
#define __LIGHT_SENSOR_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

#define LIGHT_SENSOR_POLLING_PERIOD                   (100 - 1)

struct rt_device_node;
typedef struct _light_sensor_module_data {
    struct rt_device_node *dev;
    uint32_t polling_cnt;
} light_sensor_module_data_t;

#ifdef __cplusplus
}
#endif

#endif 







