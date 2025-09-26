/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_motor_internal.h"

/*
* DEFINES
*****************************************************************************************
*/
#define MOTOR_TIMER_PERIOD     20

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
static int motor_dev_open(struct rt_device_node *dev)
{
    return bsp_motor_init();
}

static int motor_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int motor_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    return 0;
}

static int motor_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    return 0;
}

static int motor_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    switch (op_code) {
        case OP_CODE_MOTOR_START_PLAY:
            return bsp_motor_start(args);

        case OP_CODE_MOTOR_STOP_PLAY:
            return bsp_motor_stop(args);

        case OP_CODE_MOTOR_SET_PARAMETER:
            return bsp_motor_parameter_set(args);
    
        case OP_CODE_MOTOR_GET_STATUS:
            return bsp_motor_is_running(args);

        default:
            LOGE("err op_code: %lx", op_code);
            return -1;
    }
}

static int motor_dev_suspend(struct rt_device_node *dev)
{
    return bsp_motor_deinit();
}

static int motor_dev_resume(struct rt_device_node *dev)
{
    return bsp_motor_init();
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_device_for_motor(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "motor";
    node->id = ID_DEVICE_MOTOR;
    node->open = motor_dev_open;
    node->close = motor_dev_close;
    node->suspend = motor_dev_suspend;
    node->resume = motor_dev_resume;
    node->read = motor_dev_read;
    node->write = motor_dev_write;
    node->ioctl = motor_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
