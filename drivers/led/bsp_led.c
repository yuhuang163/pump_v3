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
    bsp_led_init();
#if IS_SUPPORT_NIGHT_LIGHT
    bsp_night_light_init();
#endif
    return BSP_OK;
}

static int led_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int led_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    switch (op_code) {
        case OP_CODE_LED_STATUS_SET: {
            led_status_msg_t *msg = (led_status_msg_t *)args;
            EXCEPTION_CHECK_INT(msg);
            if (NIGHT_LIGHT_LED == msg->led_index) {
                #if IS_SUPPORT_NIGHT_LIGHT
                return bsp_night_light_status_set(args);
                #else
                return 0;
                #endif
            } else {
                return bsp_led_status_set(msg);
            }
        }
        case OP_CODE_LED_LEVEL_MAX_SET: {
            led_level_msg_t *msg = (led_level_msg_t *)args;
            if (NIGHT_LIGHT_LED == msg->led_index) {
                #if IS_SUPPORT_NIGHT_LIGHT
                return bsp_night_light_level_max_update(args);
                #else
                return 0;
                #endif
            } else {
                return bsp_led_level_max_update(msg);
            }
            return 0;
        }
        case OP_CODE_LED_IS_READY:
            return bsp_led_is_ready(args);
        default:
            LOGE("err op_code: %lx", op_code);
            return -1;
    }
}

static int led_dev_suspend(struct rt_device_node *dev)
{
    bsp_led_suspend();
#if IS_SUPPORT_NIGHT_LIGHT
    bsp_night_light_suspend();
#endif
    return 0;
}

static int led_dev_resume(struct rt_device_node *dev)
{
    bsp_led_resume();
#if IS_SUPPORT_NIGHT_LIGHT
    bsp_night_light_resume();
#endif
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
    node->ioctl = led_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
