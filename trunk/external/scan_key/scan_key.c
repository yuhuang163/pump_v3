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
 
 static int long_press_count[SCAN_KEY_MAX];
 static scan_key_ctrl_data_t scan_key_ctrl[SCAN_KEY_MAX];
 static scan_key_data_t scan_key_data[SCAN_KEY_MAX];
 static rt_device_node_t *s_key_dev = NULL;
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
         int buf[] = { (1 << (SCAN_KEY_BEGIN + keycode)), (1 << (KEY_TYPE_BEGIN + keytype)) };
         node->write(node, buf, sizeof(buf));
     }
 }
 
 static void scan_power_key_cb(scan_key_data_t* scan_key)
 {
     const uint16_t keycode = SCAN_KEY_POWER;
 
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
 
 static void scan_mode_key_cb(scan_key_data_t* scan_key)
 {
     const uint16_t keycode = SCAN_KEY_MODE;
 
     // LOGW("Mode #event: %d, status:%d", scan_key->event, scan_key->status);
     if (scan_key->event == DE_SK_EVENT_DOWN_YES && scan_key->status == DE_SK_STATUS_DOWN) {
         bsp_post_key_event(keycode, KEY_TYPE_DOWN);
     } else if (scan_key->event == DE_SK_EVENT_UP_YES && scan_key->status == DE_SK_STATUS_UP) {
         bsp_post_key_event(keycode, KEY_TYPE_CLICK);
     } else if (scan_key->event == DE_SK_EVENT_UP_AGAIN && scan_key->status == DE_SK_STATUS_DOWN_AGAIN) {
         bsp_post_key_event(keycode, KEY_TYPE_DOUBLE);
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
 
     if(scan_key->event != 0 && scan_key->status == 0) {
         long_press_count[keycode] = 0;
         CLR_BIT(timer_flag, keycode);
         PRINT_LOG("timer_flag:%x\r\n", timer_flag);
     }
 }
 
 static void scan_mmi_key_cb(scan_key_data_t* scan_key)
 {
     if (scan_key->event != 0 && scan_key->status == 0) {
         CLR_BIT(timer_flag, SCAN_KEY_MMI);
     }
 }
 
 static void scan_key_timer_cb(const void *args)
 {
     if (g_input_key_timer_id == NULL) {
         LOGD("system wakeup too late");
         return;
     }
     
     for (int i = 0; i < SCAN_KEY_MAX; i++) {
         v_scan_key(&scan_key_ctrl[i], &scan_key_data[i]);
     }
 
     if (timer_flag == 0) {
         // LOGE("scan key time stop");
         STOP_TIMER(g_input_key_timer_id);
     }
 }
 
 static void scan_key_init(void)
 {
     memset(scan_key_ctrl, 0, sizeof(scan_key_ctrl));
     memset(scan_key_data, 0, sizeof(scan_key_data));
     
     scan_key_data[SCAN_KEY_POWER].pin = POWER_KEY_ID;
     scan_key_data[SCAN_KEY_POWER].type = KEY_S1_TYPE;
     scan_key_data[SCAN_KEY_POWER].cb = (u_scan_key_cb)scan_power_key_cb;
 
     scan_key_data[SCAN_KEY_MODE].pin = MODE_KEY_ID;
     scan_key_data[SCAN_KEY_MODE].type = MODE_KEY_TYPE;
     scan_key_data[SCAN_KEY_MODE].cb = (u_scan_key_cb)scan_mode_key_cb;
 
     scan_key_data[SCAN_KEY_MMI].pin = FAST_MMI_ID;
     scan_key_data[SCAN_KEY_MMI].type = FAST_MMI_TYPE;
     scan_key_data[SCAN_KEY_MMI].cb = (u_scan_key_cb)scan_mmi_key_cb;
 
     CREATE_TIMER(&g_input_key_timer_id, "input_key_timer", NULL, OS_TIMER_PERIODIC, scan_key_timer_cb);
 
     if (g_input_key_timer_id == NULL) {
         LOGE("key timer create fail");
     }
 };
 
 /*
  * GLOBAL FUNCTION DEFINITIONS
  *****************************************************************************************
  */
 int scan_key_input_init(struct rt_device_node *key_dev)
 {
     s_key_dev = key_dev;
     scan_key_init();
     return 0;
 }
 
 void scan_key_timer_start(int key)
 {
     if (g_input_key_timer_id == NULL) {
         LOGE("g_input_key_timer_id is NULL");
         return;
     }
 
     START_TIMER(g_input_key_timer_id, DEBOUNCE_TIMER_SLOT_MS);
 
     switch (key) {
         case POWER_KEY_ID:
         SET_BIT(timer_flag, SCAN_KEY_POWER);
         break;
 
         case MODE_KEY_ID:
         SET_BIT(timer_flag, SCAN_KEY_MODE);
         break;
 
         default:
         SET_BIT(timer_flag, SCAN_KEY_MMI);
         break;
     }
     // printf("timer_flag:%x\r\n", timer_flag);
 }
 
 __attribute__((weak)) int platform_read_key_value(int type, int pin)
 {
     return 0;
 }
 