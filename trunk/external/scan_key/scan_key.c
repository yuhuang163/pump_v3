/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "cmsis_os_api.h"
#include "scan_key_base.h"
#include "scan_key.h"
#include "root_common_api.h"// IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define DEBOUNCE_TIMER_SLOT_MS                       (10)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint8_t timer_flag = 0;
static os_timer_id g_input_key_timer_id = NULL;

static int long_press_count[SCAN_KEY_CODE_MAX];
static scan_key_ctrl_data_t scan_key_ctrl[SCAN_KEY_CODE_MAX];
static scan_key_data_t scan_key_data[SCAN_KEY_CODE_MAX];
static rt_device_node_t *s_key_dev = NULL;
static uint8_t s_key_code_size = SCAN_KEY_CODE_MAX;

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
    }
}

static int8_t scan_key_pin_to_code(uint16_t pin)
{
    if (pin < SCAN_KEY_ID_BEGIN || pin >= SCAN_KEY_ID_MAX) {
        return -1;
    }
    return pin - SCAN_KEY_ID_BEGIN;
}

static void scan_common_key_cb(scan_key_data_t* scan_key)
{
    const uint16_t keycode = scan_key_pin_to_code(scan_key->pin);

    if (scan_key->event == DE_SK_EVENT_DOWN_YES && scan_key->status == DE_SK_STATUS_DOWN) {
        bsp_post_key_event(keycode, KEY_TYPE_DOWN);
    } else if (scan_key->event == DE_SK_EVENT_UP_YES && scan_key->status == DE_SK_STATUS_UP) {
        bsp_post_key_event(keycode, KEY_TYPE_CLICK);
    } else if (scan_key->event == DE_SK_EVENT_DOWN_LONG_YES && scan_key->status == DE_SK_STATUS_DOWN_LONG) {
        long_press_count[keycode] = 0;
        bsp_post_key_event(keycode, KEY_TYPE_LONG_2S);
    } else if (scan_key->event == DE_SK_EVENT_DOWN_CONTINUE_YES && scan_key->status == DE_SK_STATUS_DOWN_CONTINUE) {
        long_press_count[keycode]++;
        bsp_post_key_event(keycode, KEY_TYPE_LONG_2S + long_press_count[keycode]);
    } else if ((scan_key->event == DE_SK_EVENT_DOUBLE_NO || scan_key->event == DE_SK_EVENT_UP_LONG_YES || 
                scan_key->event == DE_SK_EVENT_UP_CONTINUE_YES) && scan_key->status == DE_SK_STATUS_START) {
        bsp_post_key_event(keycode, KEY_TYPE_UP);
    }
    
    if((scan_key->event != 0 && scan_key->status == 0)) {
        long_press_count[keycode] = 0;
        CLR_BIT(timer_flag, keycode);
    }
}

static void scan_key_timer_cb(const void *args)
{
    if (g_input_key_timer_id == NULL) {
        LOGD("system wakeup too late");
        return;
    }
    
    for (uint8_t i = 0; i < s_key_code_size; i++) {
        v_scan_key(&scan_key_ctrl[i], &scan_key_data[i]);
    }

    if (timer_flag == 0) {
        STOP_TIMER(g_input_key_timer_id);
    }
}

static void scan_key_init(void)
{
    memset(scan_key_ctrl, 0, sizeof(scan_key_ctrl));
    memset(scan_key_data, 0, sizeof(scan_key_data));

    for (uint8_t i = 0; i < SCAN_KEY_CODE_MAX; i++) {
        scan_key_data[i].pin = i;
        scan_key_data[i].type = 0;
        scan_key_data[i].cb = (u_scan_key_cb)scan_common_key_cb;
    }
    CREATE_TIMER(&g_input_key_timer_id, "input_key_timer", 
        NULL, OS_TIMER_PERIODIC, scan_key_timer_cb);
    if (g_input_key_timer_id == NULL) {
        LOGE("key timer create fail");
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int scan_key_input_init(struct rt_device_node *key_dev, uint8_t key_code_size)
{
    s_key_dev = key_dev;
    s_key_code_size = key_code_size > SCAN_KEY_CODE_MAX ? SCAN_KEY_CODE_MAX : key_code_size;
    scan_key_init();
    return 0;
}

void scan_key_timer_start(int key)
{
    uint8_t key_code = 0;
    if (g_input_key_timer_id == NULL) {
        LOGE("g_input_key_timer_id is NULL");
        return;
    }
    
    START_TIMER(g_input_key_timer_id, DEBOUNCE_TIMER_SLOT_MS);
    if (key < SCAN_KEY_ID_MAX && key >= SCAN_KEY_ID_BEGIN) {
        SET_BIT(timer_flag, scan_key_pin_to_code(key));
        return;
    }

    for (uint8_t i = 0; i < s_key_code_size; i++) {
        if ((key & (1 << i)) == 0) {
            continue;
        }
        SET_BIT(timer_flag, scan_key_pin_to_code(i));
    }
}

__attribute__((weak)) int platform_read_key_value(int type, int pin)
{
    return 0;
}
