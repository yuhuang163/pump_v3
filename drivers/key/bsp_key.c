/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "scan_key.h" // IWYU pragma: keep
#include "bsp_key_internal.h"

/*
* DEFINES
*****************************************************************************************
*/
#define KEY_CODE_SIZE (6 + 1 + 2) // 6个按键 + 1个电源键 + 2个组合按键

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static rt_device_node_t *node = NULL;
static key_input_evt_cb_fptr key_cb = NULL;
static const uint32_t key_code_value[SCAN_KEY_CODE_MAX] = {
    SCAN_KEY_CODE_1_VALUE,
    SCAN_KEY_CODE_2_VALUE,
    SCAN_KEY_CODE_3_VALUE,
    SCAN_KEY_CODE_4_VALUE,
    SCAN_KEY_CODE_5_VALUE,
    SCAN_KEY_CODE_6_VALUE,
    SCAN_KEY_CODE_7_VALUE,
    SCAN_KEY_CODE_56_VALUE,
    SCAN_KEY_CODE_23_VALUE,
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int scan_key_module_init(void)
{
    int ret = 0;
    ret = bsp_key_init();
    if (ret) {
        LOGE("key init fail:%d", ret);
        return ret;
    }
    ret = scan_key_input_init(node, key_code_value, KEY_CODE_SIZE);
    if (node) {
        node->status = DEVICE_BUS_INIT;
    }
    return ret;
}

static int key_dev_open(struct rt_device_node *dev)
{
    scan_key_module_init();
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
    int *tmp = (int *)buf;
    int keycode = 0;
    int keytype = 0;

    if (tmp) {
        keycode = tmp[0];
        keytype = tmp[1];
    }

    // LOG_PBUS("keycode: %d, keytype: %d, key_cb: %p", keycode, keytype, key_cb);
    if (keycode && key_cb) {
        key_cb(dev, keycode, keytype);
    }
    return 0;
}

static int key_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    uint32_t *tmp = (uint32_t *)args;

    switch (op_code) {
        case OP_CODE_INPUT_ADD_CALLBACK:
            key_cb = (key_input_evt_cb_fptr)args;
            return 0;
        case OP_CODE_INPUT_IS_TRIGGERED:
            return platform_read_key_value(0, tmp[0]);
        case OP_CODE_INPUT_SET_KEY_SENSITIVITY:
            return bsp_key_sensitivity_set(tmp[0], tmp[1]);
        default:
            return -1;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int platform_read_key_value(int type, int index)
{
    return bsp_read_key_value(index);
}

int platform_all_key_value(void)
{
    return bsp_read_all_key_value();
}

void key_irq_event_callback(int index)
{
    scan_key_timer_start(index);
}

void knob_button_event_handler(int arg1, int arg2, void *param)
{
    LOGI("knob button event: arg1=%d, arg2=%d", arg1, arg2);
    int keycode = arg1 == 0 ? KEY_CODE_10 : KEY_CODE_11;
    key_cb(node, keycode, KEY_EVENT_CLICK);
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
