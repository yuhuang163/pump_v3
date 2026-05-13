/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "led_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static led_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void set_led_blink(uint8_t led_index, uint16_t period, uint16_t blink_times, uint8_t finish_status)
{
    EXCEPTION_CHECK_VOID(thiz);
    led_status_msg_t msg = {
        .led_index = led_index,
        .led_status = {
            .mode = LED_MODE_BLINK,
            .un.blink.period = period,
            .un.blink.times = blink_times,
            .un.blink.finish_status = finish_status,
        },
    };
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void set_led_level(uint8_t led_index, uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    led_status_msg_t msg = {
        .led_index = led_index,
        .led_status = {
            .mode = LED_MODE_LEVEL,
            .un.luminous.level = level,
        },
    };
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void set_led_onoff(uint8_t led_index, uint8_t onoff)
{
    EXCEPTION_CHECK_VOID(thiz);
    led_status_msg_t msg = {
        .led_index = led_index,
        .led_status = {
            .mode = LED_MODE_ONOFF,
            .un.onoff.status = onoff,
        },
    };
    thiz->priv.led_dev->ioctl(thiz->priv.led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
}

static void on_led_broadcast_receiver(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    LOG_MISC("broadcast id: %d, arg1: %d, arg2: %d", id.code, arg1, arg2);
    if (thiz && thiz->hook && thiz->hook->on_broadcast_receiver_route) {
        thiz->hook->on_broadcast_receiver_route(id, context, arg1, arg2, pdata);
    }
}

static int on_post_led_manager_init(void *ptr)
{
    led_manager_context_t *led = (led_manager_context_t *)ptr;
    EXCEPTION_CHECK_INT(led);
    /// 广播监听PM状态改变.
    led->priv.m_bcd.context = led;
    led->priv.m_bcd.name = "led_manager";
    led->priv.m_bcd.id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    led->priv.m_bcd.id.code |= ID_BROADCAST_SYS_STATE_CHANGE;
    led->priv.m_bcd.receiver_cb = on_led_broadcast_receiver;
    register_broadcast_receiver(&led->priv.m_bcd);
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_INT(pbus);
    led->priv.led_dev = pbus->find_device(ID_DEVICE_LED);
    if (led->hook && led->hook->on_post_init_route) {
        led->hook->on_post_init_route(led);
    }
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_led_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, led_manager_context_t);
    ptr->set_led_blink = set_led_blink;
    ptr->set_led_level = set_led_level;
    ptr->set_led_onoff = set_led_onoff;
    thiz = ptr;
    return ADD_MODULE("led", ID_MODULE_LED, NULL, on_post_led_manager_init, 
        ptr, list_head, sizeof(led_manager_context_t));
}
