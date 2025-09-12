/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static rt_device_node_t *node = NULL;
// static void (*s_air_pressure_irq_callback)(void *param);

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
    return 0;
}

static int air_pressure_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int air_pressure_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    return 0;
}

static int air_pressure_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    return 0;
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
void *register_device_for_imu(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "air_pressure";
    node->id = ID_DEVICE_AIR_PRESSURE;
    node->open = air_pressure_dev_open;
    node->close = air_pressure_dev_close;
    node->suspend = air_pressure_dev_suspend;
    node->resume = air_pressure_dev_resume;
    node->read = air_pressure_dev_read;
    node->ioctl = air_pressure_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
