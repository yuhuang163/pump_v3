#ifndef __FORCE_SENSOR_MODULE_H__
#define __FORCE_SENSOR_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

#define F_SENSOR_POLLING_PERIOD              (160)

struct rt_device_node;
typedef struct _force_sensor_module_data {
    struct rt_device_node *dev;
    uint32_t polling_cnt;
} force_sensor_module_data_t;

#ifdef __cplusplus
}
#endif

#endif 







