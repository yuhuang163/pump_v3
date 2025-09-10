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

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int key_dev_open(struct rt_device_node *dev)
{
    return 0;
}

static int key_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int key_dev_suspend(struct rt_device_node *dev)
{
    if (node) {
        node->status = DEVICE_BUS_SUSPEND;
    }
    return 0;
}

static int key_dev_resume(struct rt_device_node *dev)
{
    if (node) {
        node->status = DEVICE_BUS_RESUME;
    }
    return 0;
}

static int key_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    return 0;
}

static int key_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int platform_read_key_value(int type, int index)
{
    return 0;
}

void *register_device_for_input_key(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "input-key";
    node->id = ID_DEVICE_INPUT_KEY;
    node->open = key_dev_open;
    node->close = key_dev_close;
    node->suspend = key_dev_suspend;
    node->resume = key_dev_resume;
    node->write = key_dev_write;
    node->ioctl = key_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
