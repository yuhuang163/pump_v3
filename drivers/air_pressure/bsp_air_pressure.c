/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_i2c.h" // IWYU pragma: keep
#include "bsp_config.h"
#include "bsp_internal.h"
#include "bsp_air_pressure_internal.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static rt_device_node_t *node = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int air_pressure_dev_open(struct rt_device_node *dev)
{
    bsp_i2c_init(AIR_PRESSURE_I2C_ID1);
    bsp_i2c_init(AIR_PRESSURE_I2C_ID2);
    msleep(10);
    return bsp_air_pressure_init();
}

static int air_pressure_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    uint32_t *tmp = (uint32_t *)args;

    switch (op_code) {
        case OP_CODE_AIR_PRESSURE_GET_STATUS:
            return bsp_read_air_pressure_status(args);
        case OP_CODE_AIR_PRESSURE_IS_READY:
            return bsp_air_pressure_is_ready(args);
        case OP_CODE_AIR_PRESSURE_IS_SUPPORT_AIR_PRESSURE:
            return true;
        default:
            return -1;
    }
}

static int air_pressure_dev_suspend(struct rt_device_node *dev)
{
    return bsp_air_pressure_deinit();
}

static int air_pressure_dev_resume(struct rt_device_node *dev)
{
    return bsp_air_pressure_init();
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_device_for_air_pressure(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "air_pressure";
    node->id = ID_DEVICE_AIR_PRESSURE;
    node->open = air_pressure_dev_open;
    node->suspend = air_pressure_dev_suspend;
    node->resume = air_pressure_dev_resume;
    node->ioctl = air_pressure_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
