/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "bsp_common.h"
#include "bsp_led_internal.h"

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
static int led_dev_open(struct rt_device_node *dev)
{
    return 0;
}

static int led_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int led_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    return 0;
}

static int led_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    return 0;
}

static int led_dev_suspend(struct rt_device_node *dev)
{
    return 0;
}

static int led_dev_resume(struct rt_device_node *dev)
{
    return 0;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
void *register_device_for_led(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "led";
    node->id = ID_DEVICE_LED;
    node->open = led_dev_open;
    node->close = led_dev_close;
    node->suspend = led_dev_suspend;
    node->resume = led_dev_resume;
    node->write = led_dev_write;
    node->ioctl = led_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
