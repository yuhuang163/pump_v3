/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"
#include "bsp_internal.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static rt_device_node_t *node = NULL;
static thread_context_t interrupt_thd;
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void interrupt_msg_thread(const void *args)
{
    uint8_t io_num = INT_NUM_NONE;
    while (1) {
        MSG_QUEUE_RECV(interrupt_thd.msg_id, (void *)&io_num, OS_WAIT_FOREVER);
        switch (io_num) {
            case INT_NUM_KEY_1:
            case INT_NUM_KEY_2:
                key_irq_event_callback(io_num);
                break;
            case INT_NUM_TOUCH_KEY:
                key_irq_event_callback(-1);
                break;
        #if IS_SUPPORT_DEVICE_KNOB
            case INT_NUM_KNOB_LEFT:
                knob_button_event_handler(0, 0, NULL);
                // BSP_LOG_PASS("left knob %d/%d\r\n", bsp_knob_get_count(0), bsp_knob_get_count(1));
                break;
            case INT_NUM_KNOB_RIGHT:
                knob_button_event_handler(1, 0, NULL);
                // BSP_LOG_PASS("right knob %d/%d\r\n", bsp_knob_get_count(0), bsp_knob_get_count(1));
                break;
        #endif
        #if IS_SUPPORT_PD_CHARGE
            case INT_NUM_CHARGE_PD:
                charge_pd_event_handler();
                break;
        #endif
            default: 
                LOG_PBUS("unknown int:%ld", io_num);
                break;
        }
    }
}

static int interrupt_dev_open(struct rt_device_node *dev)
{
    CREATE_MSG_TYPE_QUEUE(&interrupt_thd.msg_id, INTERRUPT_MESQ_NUM, 1);
    RUN_NEW_THREAD(&interrupt_thd, "interrupt", interrupt_msg_thread, 
        RT_PRIO_INTERRUPT_THREAD, (INTERRUPT_THREAD_STACKSIZE));
    if (node) {
        node->status = DEVICE_BUS_INIT;
    }
    return 0;
}

static int interrupt_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int interrupt_dev_suspend(struct rt_device_node *dev)
{
    if (node) {
        node->status = DEVICE_BUS_SUSPEND;
    }
    return 0;
}

static int interrupt_dev_resume(struct rt_device_node *dev)
{
    if (node) {
        node->status = DEVICE_BUS_RESUME;
    }
    return 0;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
void bsp_irq_msg_send(uint8_t index)
{
    uint8_t io_num = index;
    if (interrupt_thd.msg_id == NULL) {
        LOGE("interrupt message queue not initialized");
        return;
    }
    MSG_QUEUE_SEND(interrupt_thd.msg_id, &io_num, 0);
}

void *register_device_for_irq(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "irq";
    node->id = ID_DEVICE_IRQ;
    node->open = interrupt_dev_open;
    node->close = interrupt_dev_close;
    node->suspend = interrupt_dev_suspend;
    node->resume = interrupt_dev_resume;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
