/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"
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
    return bsp_air_pressure_init();
}

static int air_pressure_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    uint32_t *tmp = (uint32_t *)args;

    switch (op_code) {
        case OP_CODE_AIR_PRESSURE_GET_STATUS:
            return bsp_read_air_pressure_status(args);
        default:
            return -1;
    }
}

static int air_pressure_dev_suspend(struct rt_device_node *dev)
{
    return 0;
}

static int air_pressure_dev_resume(struct rt_device_node *dev)
{
    return 0;
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
