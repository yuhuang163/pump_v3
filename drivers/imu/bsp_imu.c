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
// static void (*s_imu_irq_callback)(void *param);

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int imu_dev_open(struct rt_device_node *dev)
{
    return 0;
}

static int imu_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int imu_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    return 0;
}

static int imu_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    return 0;
}

static int imu_dev_suspend(struct rt_device_node *dev)
{
    return 0;
}

static int imu_dev_resume(struct rt_device_node *dev)
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
    node->name = "imu";
    node->id = ID_DEVICE_IMU;
    node->open = imu_dev_open;
    node->close = imu_dev_close;
    node->suspend = imu_dev_suspend;
    node->resume = imu_dev_resume;
    node->read = imu_dev_read;
    node->ioctl = imu_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
