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
#if 0
static int bsp_sys_power_init(void)
{
    app_io_init_t io_init = {
        .pull = APP_IO_PULLUP,
        .mode = APP_IO_MODE_INPUT,
        .pin  = PCBA_TEST_PIN,
        .mux  = BSP_COMMON_IO_MUX
    };
    return app_io_init(PCBA_TEST_IO_TYPE, &io_init);
}
#endif

static int sys_power_dev_open(struct rt_device_node *dev)
{
    return 0;
}

static int sys_power_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int sys_power_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    return 0;
}

static int sys_power_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    return 0;
}

static int sys_power_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    switch (op_code) {
        default:
            break;
    }
    return 0;
}

static int sys_power_dev_suspend(struct rt_device_node *dev)
{
    return 0;
}

static int sys_power_dev_resume(struct rt_device_node *dev)
{
    return 0;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
void *register_device_for_power(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "sys_power";
    node->id = ID_DEVICE_POWER;
    node->open = sys_power_dev_open;
    node->close = sys_power_dev_close;
    node->suspend = sys_power_dev_suspend;
    node->resume = sys_power_dev_resume;
    node->read = sys_power_dev_read;
    node->write = sys_power_dev_write;
    node->ioctl = sys_power_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
