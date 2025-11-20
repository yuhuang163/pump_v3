/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "scan_key_base.h"
#include "scan_key.h"
#include "root_common_api.h"// IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define DEBOUNCE_TIMER_SLOT_MS                       (10)
#define KEY_IDLE_COUNT_MAX                           (5)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint8_t long_press_flag = 0;
static uint8_t s_key_code_size = 0;
static uint16_t key_idle_count = 0;
static uint32_t timer_flag = 0;
static uint32_t *long_press_count = NULL;
static scan_key_ctrl_data_t *scan_key_ctrl = NULL;
static scan_key_data_t *scan_key_data = NULL;

static rt_device_node_t *s_key_dev = NULL;
static os_timer_id g_input_key_timer_id = NULL;
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void bsp_post_key_event(uint16_t keycode, uint16_t keytype)
{
    rt_device_node_t *node = s_key_dev;

    if (node && node->write) {
        // LOG_KEY("write keyevent: %d --- %d", keycode, keytype);
        int buf[] = { (1 << (SCAN_KEY_CODE_BEGIN + keycode)), (1 << (KEY_TYPE_BEGIN + keytype)) };
        node->write(node, buf, sizeof(buf));
    } else {
        LOG_KEY("node or write is NULL: %d --- %d", keycode, keytype);
    }
}

static void scan_common_key_cb(scan_key_data_t* scan_key, uint8_t code_index)
{
    const uint16_t keycode = code_index;
    if (keycode >= s_key_code_size) {
        LOGE("keycode is out of range: %d", keycode);
        return;
    }

    if (scan_key->event == DE_SK_EVENT_DOWN_YES && scan_key->status == DE_SK_STATUS_DOWN) {
        bsp_post_key_event(keycode, long_press_flag == 1 ? KEY_TYPE_LONG_AFTER_DOWN : KEY_TYPE_DOWN);
    } else if (scan_key->event == DE_SK_EVENT_UP_YES && scan_key->status == DE_SK_STATUS_UP) {
        if (scan_key->up_status == 1) {
            bsp_post_key_event(keycode, long_press_flag == 1 ? KEY_TYPE_LONG_AFTER_CLICK : KEY_TYPE_CLICK);
        } else {
            LOG_KEY("key %d is not up, up_status: %d", keycode, scan_key->up_status);
        }
    } else if (scan_key->event == DE_SK_EVENT_DOWN_LONG_YES && scan_key->status == DE_SK_STATUS_DOWN_LONG) {
        long_press_count[keycode] = 0;
        bsp_post_key_event(keycode, KEY_TYPE_LONG_2S);
        long_press_flag = 1; // 长按标志位置1
    } else if (scan_key->event == DE_SK_EVENT_DOWN_CONTINUE_YES && scan_key->status == DE_SK_STATUS_DOWN_CONTINUE) {
        long_press_count[keycode]++;
        bsp_post_key_event(keycode, KEY_TYPE_LONG_2S + long_press_count[keycode]);
    } else if ((scan_key->event == DE_SK_EVENT_DOUBLE_NO || scan_key->event == DE_SK_EVENT_UP_LONG_YES || 
                scan_key->event == DE_SK_EVENT_UP_CONTINUE_YES) && scan_key->status == DE_SK_STATUS_START) {
        bsp_post_key_event(keycode, KEY_TYPE_UP);
    }
    
    if ((scan_key->event != 0 && scan_key->status == 0)) {
        long_press_count[keycode] = 0;
        CLR_BIT(timer_flag, keycode);
    }
}

static void scan_key_read(scan_key_ctrl_data_t *key_ctrl, scan_key_data_t *key_data, uint8_t code_size)
{
    uint8_t key_change = 0;
    uint32_t key_value = platform_all_key_value(); 
    /// 按键空闲计数
    if (key_value == 0) {
        long_press_flag = 0; // 释放所有按键后,长按标志位清零
        key_idle_count ++;
    } else {
        key_idle_count = 0;
    }
    for (uint8_t i = 0; i < code_size; i++) {
        if (key_value == key_data[i].code_value) {
            key_data[i].level = 1; ///  用于记录激活的按键数据,按键按下表示激活,对应位置1
        } else {
            key_data[i].level = 0;
        }
        key_data[i].up_status = (key_value & key_data[i].code_value) == 0 ? 1 : 0; ///  记录按键抬起状态, 限制组合按键触发时上报单键释放事件
        key_change         = key_data[i].level ^ key_ctrl[i].hold; ///  记录当前和上次不同状态的按键数据,对应位为1表示此按键电平对比上次有变动
        key_ctrl[i].hold   = key_data[i].level; ///  保存本次激活的位
        key_ctrl[i].triger = key_data[i].level & key_change; ///  当前激活的位是否包含,当前激活和上次激活的不同的位;也就是上次未激活,当前激活的位
        key_ctrl[i].reset  = key_change & (~key_data[i].level); ///  记录由按下到抬起的触发
    }
}

static void scan_key_timer_cb(const void *args)
{
    if (g_input_key_timer_id == NULL) {
        LOGD("system wakeup too late");
        return;
    }
    scan_key_read(scan_key_ctrl, scan_key_data, s_key_code_size);
    v_scan_key(scan_key_ctrl, scan_key_data, s_key_code_size);
    /// 如果定时器标志位为0或按键空闲计数大于最大值，则停止定时器
    if (timer_flag == 0 || key_idle_count > KEY_IDLE_COUNT_MAX) {
        timer_flag = 0;
        STOP_TIMER(g_input_key_timer_id);
    }
}

static void scan_key_init(const uint32_t *code_value, uint8_t key_code_size)
{
    EXCEPTION_CHECK_VOID(code_value);
    memset(scan_key_ctrl, 0, sizeof(scan_key_ctrl_data_t) * s_key_code_size);
    memset(scan_key_data, 0, sizeof(scan_key_data_t) * s_key_code_size);
    for (uint8_t i = 0; i < s_key_code_size; i++) {
        scan_key_data[i].code_value = code_value[i];
        scan_key_data[i].cb = (u_scan_key_cb)scan_common_key_cb;
    }

    CREATE_TIMER(&g_input_key_timer_id, "input_key_timer", NULL, OS_TIMER_PERIODIC, scan_key_timer_cb);
    if (g_input_key_timer_id == NULL) {
        LOGE("key timer create fail");
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void scan_key_timer_start(uint8_t code_index)
{
    uint8_t key_code = 0;
    if (g_input_key_timer_id == NULL) {
        LOGE("g_input_key_timer_id is NULL");
        return;
    }
    key_idle_count = 0;
    START_TIMER(g_input_key_timer_id, DEBOUNCE_TIMER_SLOT_MS);
    if (code_index < SCAN_KEY_CODE_MAX && code_index >= SCAN_KEY_CODE_BEGIN) {
        SET_BIT(timer_flag, code_index);
        return;
    }

    for (uint8_t i = 0; i < s_key_code_size; i++) {
        SET_BIT(timer_flag, i);

    }
    // LOG_KEY("scan_key_timer_start: %x", timer_flag);
}

int scan_key_input_init(struct rt_device_node *key_dev, const uint32_t *code_value, uint8_t key_code_size)
{
    s_key_dev = key_dev;
    s_key_code_size = key_code_size > SCAN_KEY_CODE_MAX ? SCAN_KEY_CODE_MAX : key_code_size;
    long_press_count = u_malloz(sizeof(uint32_t) * s_key_code_size);
    EXCEPTION_CHECK_INT(long_press_count);
    scan_key_ctrl = u_malloz(sizeof(scan_key_ctrl_data_t) * s_key_code_size);
    EXCEPTION_CHECK_INT(scan_key_ctrl);
    scan_key_data = u_malloz(sizeof(scan_key_data_t) * s_key_code_size);
    EXCEPTION_CHECK_INT(scan_key_data);
    scan_key_init(code_value, s_key_code_size);
    scan_key_timer_start(SCAN_KEY_ID1);
    return 0;
}

__attribute__((weak)) int platform_read_key_value(int type, int pin)
{
    return 0;
}

__attribute__((weak)) int platform_all_key_value(void)
{
    return 0;
}
