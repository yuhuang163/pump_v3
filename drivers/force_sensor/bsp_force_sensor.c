/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_force_sensor_internal.h"

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
static int force_sensor_dev_open(struct rt_device_node *dev)
{
    return 0;
}

static int force_sensor_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int force_sensor_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    return 0;
}

static int force_sensor_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    // EXCEPTION_CHECK_INT(buf);
    return 0;
}

static int force_sensor_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    return 0;
}

static int force_sensor_dev_suspend(struct rt_device_node *dev)
{
    return 0;
}

static int force_sensor_dev_resume(struct rt_device_node *dev)
{
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_device_for_force_sensor(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "fsensor";
    node->id = ID_DEVICE_FSENSOR;
    node->open = force_sensor_dev_open;
    node->close =  force_sensor_dev_close;
    node->suspend = force_sensor_dev_suspend;
    node->resume = force_sensor_dev_resume;
    node->read = force_sensor_dev_read;
    node->write = force_sensor_dev_write;
    node->ioctl = force_sensor_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
