/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "scan_key.h" // IWYU pragma: keep
#include "bsp_key_internal.h"
#if IS_SUPPORT_DEVICE_KNOB
#include "bsp_knob.h" // IWYU pragma: keep
#endif

/*
* DEFINES
*****************************************************************************************
*/
#if V3_UX_VERSION == V3_UX_V6
#define KEY_CODE_SIZE (6 + 1 + 3) // 6个按键 + 1个电源键 + 3个组合按键
#else
#define KEY_CODE_SIZE (6 + 1 + 2) // 6个按键 + 1个电源键 + 2个组合按键
#endif

#if IS_SUPPORT_KNOB_POWER_KEY_DEBOUNCE
#define POWER_KEY_FORBIDDEN_TIME_MS      (100)
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static rt_device_node_t *node = NULL;
static key_input_evt_cb_fptr key_cb = NULL;
#if IS_SUPPORT_KNOB_POWER_KEY_DEBOUNCE
static uint32_t knob_button_trigger_time = 0;
static bool power_key_forbidden = false;
#endif

#if V3_UX_VERSION == V3_UX_V6
static const uint32_t key_code_value[SCAN_KEY_CODE_MAX] = {
    SCAN_KEY_CODE_1_VALUE, /// 电源键
    SCAN_KEY_CODE_2_VALUE, /// 开始键
    SCAN_KEY_CODE_3_VALUE, /// 模式键
    SCAN_KEY_CODE_4_VALUE, /// 频率键
    SCAN_KEY_CODE_5_VALUE, /// 吸吮键
    SCAN_KEY_CODE_6_VALUE, /// 左控制键
    SCAN_KEY_CODE_7_VALUE, /// 右控制键
    SCAN_KEY_CODE_24_VALUE, /// 恢复出厂设置键
    SCAN_KEY_CODE_37_VALUE, /// 旅行锁键
    SCAN_KEY_CODE_67_VALUE, /// 双边控制键
};
#else
static const uint32_t key_code_value[SCAN_KEY_CODE_MAX] = {
    SCAN_KEY_CODE_1_VALUE,
    SCAN_KEY_CODE_2_VALUE,
    SCAN_KEY_CODE_3_VALUE,
    SCAN_KEY_CODE_4_VALUE,
    SCAN_KEY_CODE_5_VALUE,
    SCAN_KEY_CODE_6_VALUE,
    SCAN_KEY_CODE_7_VALUE,
    SCAN_KEY_CODE_67_VALUE,
#if V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4
    SCAN_KEY_CODE_36_VALUE,
#else
    SCAN_KEY_CODE_23_VALUE,
#endif
    SCAN_KEY_CODE_13_VALUE 
};
#endif

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
#if IS_SUPPORT_DEVICE_KNOB
    bsp_knob_init();
#endif
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
        case OP_CODE_INPUT_IS_READY:
            return 0;
        case OP_CODE_INPUT_TOUCH_IS_READY:
            return bsp_touch_is_ready(args);
#if IS_SUPPORT_TOUCH_IC_OTA
        case OP_CODE_INPUT_ADD_KEY_OTA_EVENT_CB:
            return bsp_add_touch_key_ota_event_cb(args);
        case OP_CODE_INPUT_KEY_FW_VERSION_CHECK:
            return bsp_start_touch_key_update_fw();
        case OP_CODE_INPUT_GET_KEY_FW_VERSION:
            return bsp_get_touch_key_fw_version();
#endif
        case OP_CODE_INPUT_TOUCH_SIGNAL_READ:
            return bsp_touch_signal_read(args);
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
    int key_value = bsp_read_all_key_value();
#if IS_SUPPORT_KNOB_POWER_KEY_DEBOUNCE
    if (power_key_forbidden) {
        if (get_timestamp() - knob_button_trigger_time > POWER_KEY_FORBIDDEN_TIME_MS) {
            power_key_forbidden = false;
            BSP_LOG("power key", "forbidden reset");
        }
        key_value &= 0xFFFE;
    }
#endif
    return key_value;
}

void key_irq_event_callback(int index)
{
    scan_key_timer_start(index);
}

bool touch_key_is_upgrading(void)
{
#if IS_SUPPORT_TOUCH_IC_OTA
    return bsp_touch_key_is_upgrading();
#else
    return false;
#endif
}

void knob_button_event_handler(int arg1, int arg2, void *param)
{
    // LOG_PBUS("knob button event: arg1=%d, arg2=%d", arg1, arg2);
#if IS_SUPPORT_KNOB_POWER_KEY_DEBOUNCE
    knob_button_trigger_time = get_timestamp();
    power_key_forbidden = true;
#endif
    int keycode = arg1 == 0 ? KEY_CODE_11 : KEY_CODE_12;
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
