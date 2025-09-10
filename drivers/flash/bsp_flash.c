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
static int flash_dev_open(struct rt_device_node *dev)
{
    return 0;
}

static int flash_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int flash_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    return 0;
}

static int flash_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    return 0;
}


static int flash_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    return 0;
}


static int flash_dev_resume(struct rt_device_node *dev)
{
    return 0;
}

static int flash_dev_suspend(struct rt_device_node *dev)
{
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_device_for_flash(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "flash";
    node->id = ID_DEVICE_FLASH;
    node->open = flash_dev_open;
    node->close = flash_dev_close;
    node->read = flash_dev_read;
    node->write = flash_dev_write;
    node->ioctl = flash_dev_ioctl;
    node->suspend = flash_dev_suspend;
    node->resume = flash_dev_resume;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
