/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_factory_uart.h" // IWYU pragma: keep

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
static int uart_dev_open(struct rt_device_node *dev)
{
    return bsp_factory_uart_init();
}

static int uart_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int uart_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    return bsp_factory_uart_send_data((uint8_t *)buf, n_bytes);
}

static int uart_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    switch (op_code) {
        case OP_CODE_UART_ADD_EVENT_CB:
            bsp_factory_uart_register_event_cb((uart_recv_event_cb_func)args, dev);
            break;

        default:
            break;
    }
    return 0;
}

static int uart_dev_suspend(struct rt_device_node *dev)
{
    return 0;
}

static int uart_dev_resume(struct rt_device_node *dev)
{
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_device_for_uart(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));

    node->name = "uart";
    node->id = ID_DEVICE_UART;
    node->open = uart_dev_open;
    node->close = uart_dev_close;
    node->suspend = uart_dev_suspend;
    node->resume = uart_dev_resume;
    node->write = uart_dev_write;
    node->ioctl = uart_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
