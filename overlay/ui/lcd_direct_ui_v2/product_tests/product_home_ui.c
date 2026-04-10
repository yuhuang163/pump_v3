/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "product_tests_ui.h" // IWYU pragma: keep
#include "window_screen_product_test.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_SCAN_QRCODE_TIP                                   0
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
#define IS_SUPPORT_BATTERY_PERCENT                                   1
#else
#define IS_SUPPORT_BATTERY_PERCENT                                   0
#endif
#define PRODUCT_TESTS_HOME_CONTENT_COUNT                             1
#define PRODUCT_TESTS_HOME_CONTENT_COLOR                             0x3C3D40
#define PRODUCT_TESTS_HOME_QRCODE_BG_COLOR                           0xFFFFFF
#define PRODUCT_TESTS_HOME_QRCODE_FG_COLOR                           0x000000
#define PRODUCT_TESTS_HOME_QRCODE_BORDER_COLOR                       0xA6A6A6
#define PRODUCT_TESTS_HOME_MAC_NAME_COLOR                            0x01D65A // 0x3C3D40
#define PRODUCT_TESTS_HOME_BATTERY_PERCENT_COLOR                      0x01D65A // 0x3C3D40
#define PRODUCT_TESTS_HOME_AGING_INFO_COLOR                           0x01D65A // 老化信息颜色
#if IS_SUPPORT_SCAN_QRCODE_TIP
#define PRODUCT_TESTS_HOME_MAC_NAME_X                                (105 + 4)
#define PRODUCT_TESTS_HOME_MAC_NAME_Y                                (80)
#define PRODUCT_TESTS_HOME_QRCODE_X                                  (PRODUCT_TESTS_HOME_MAC_NAME_X)
#define PRODUCT_TESTS_HOME_QRCODE_Y                                  (PRODUCT_TESTS_HOME_MAC_NAME_Y + 20 + 5)
#define PRODUCT_TESTS_HOME_QRCODE_SIZE                               (150)
#else
#define PRODUCT_TESTS_HOME_MAC_NAME_X                                (90 + 4)
#define PRODUCT_TESTS_HOME_MAC_NAME_Y                                (80)
#define PRODUCT_TESTS_HOME_QRCODE_X                                  (PRODUCT_TESTS_HOME_MAC_NAME_X)
#define PRODUCT_TESTS_HOME_QRCODE_Y                                  (PRODUCT_TESTS_HOME_MAC_NAME_Y + 20 + 10)
#define PRODUCT_TESTS_HOME_QRCODE_SIZE                               (180)
#define PRODUCT_TESTS_HOME_BATTERY_PERCENT_X                         (PRODUCT_TESTS_HOME_MAC_NAME_X+70)
#define PRODUCT_TESTS_HOME_BATTERY_PERCENT_Y                         (PRODUCT_TESTS_HOME_MAC_NAME_Y)
#define PRODUCT_TESTS_HOME_AGING_INFO_X                              (PRODUCT_TESTS_HOME_MAC_NAME_X)
#define PRODUCT_TESTS_HOME_AGING_INFO_Y                              (PRODUCT_TESTS_HOME_BATTERY_PERCENT_Y + PRODUCT_TESTS_HOME_QRCODE_SIZE+36)
#endif

typedef struct {
    const char *name;
    uint16_t text_x;
    uint16_t text_y;
} product_tests_content_info_t;

typedef struct {  
    lv_obj_t * root;
    lv_obj_t * qrcode;
    lv_obj_t * mac_name;
#if IS_SUPPORT_SCAN_QRCODE_TIP
    lv_obj_t * text;
#endif
#if IS_SUPPORT_BATTERY_PERCENT
    lv_obj_t * battery_percent;
#endif
    lv_obj_t * aging_info;  // 老化信息显示
    uint8_t force_update;
    uint8_t mac_addr[6];
    uint8_t popup_ui_index;
    uint8_t rsv[16];
} product_tests_home_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_SCAN_QRCODE_TIP
const static product_tests_content_info_t product_tests_content_infos[PRODUCT_TESTS_HOME_CONTENT_COUNT] = {
    { "Please scan the QR code\nto quickly connect the device", (63 + 6), (250 + 20)},
};  
#endif

static product_tests_home_display_info_t *tests_info = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void product_tests_ui_root_init(void)
{
    if (tests_info->root == NULL || !lv_obj_is_valid(tests_info->root)) {
        tests_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(tests_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(tests_info->root, lv_color_white(), 0);     /// 设置背景颜色为白色
    lv_obj_set_style_bg_opa(tests_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(tests_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(tests_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(tests_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(tests_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(tests_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

static void product_tests_mac_name_update(ui_manager_context_t *thiz, uint8_t *mac_addr)
{
    if (tests_info->force_update == false) {
        return;
    }

    if (tests_info->mac_name == NULL || !lv_obj_is_valid(tests_info->mac_name)) {
        tests_info->mac_name = lv_label_create(tests_info->root);
    }
    
    char mac_name[16] = {0};
    ///根据ROOT_DEVICE_NAME显示不同的name
    snprintf(mac_name, sizeof(mac_name), "%s-%02X%02X", ROOT_DEVICE_NAME, mac_addr[0], mac_addr[1]);
    lv_label_set_text(tests_info->mac_name, mac_name);
    lv_obj_set_style_text_color(tests_info->mac_name, lv_color_hex(PRODUCT_TESTS_HOME_MAC_NAME_COLOR), 0);
    lv_obj_set_style_text_font(tests_info->mac_name, &lv_font_oppo_sans_14_medium, 0);  /// 设置字体
    // lv_obj_set_style_text_line_space(tests_info->mac_name, 4, 0);  /// 设置行间距（像素）
    // lv_obj_set_style_text_align(tests_info->mac_name, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
    lv_obj_align(tests_info->mac_name, LV_ALIGN_TOP_LEFT, 
        PRODUCT_TESTS_HOME_MAC_NAME_X, PRODUCT_TESTS_HOME_MAC_NAME_Y);
}

static void product_tests_qrcode_update(ui_manager_context_t *thiz, uint8_t *mac_addr)
{      
    lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 5);
    lv_color_t fg_color = lv_palette_darken(LV_PALETTE_BLUE, 4);
    if (tests_info->force_update == false || memcmp(tests_info->mac_addr, mac_addr, 6) == 0) {
        return;
    }

    if (tests_info->qrcode == NULL || !lv_obj_is_valid(tests_info->qrcode)) {
        tests_info->qrcode = lv_qrcode_create(tests_info->root);
        EXCEPTION_CHECK_VOID(tests_info->qrcode);
    }
    
    lv_qrcode_set_size(tests_info->qrcode, PRODUCT_TESTS_HOME_QRCODE_SIZE);
    lv_qrcode_set_dark_color(tests_info->qrcode, fg_color);
    lv_qrcode_set_light_color(tests_info->qrcode, bg_color);
    lv_obj_set_style_border_color(tests_info->qrcode, bg_color, 0);
    char qr_str[32] = {0};
    snprintf(qr_str, sizeof(qr_str), "%02X:%02X:%02X:%02X:%02X:%02X",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    lv_qrcode_update(tests_info->qrcode, qr_str, strlen(qr_str));
    lv_obj_center(tests_info->qrcode);
    lv_obj_align(tests_info->qrcode, LV_ALIGN_TOP_LEFT, PRODUCT_TESTS_HOME_QRCODE_X, PRODUCT_TESTS_HOME_QRCODE_Y);
}

static void product_tests_aging_info_update(ui_manager_context_t *thiz, ui_proc_product_test_t *proc)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(tests_info);
    EXCEPTION_CHECK_VOID(tests_info->root);
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->tests);
    EXCEPTION_CHECK_VOID(proc->pump);
    
    // 创建或获取label对象
    if (tests_info->aging_info == NULL || !lv_obj_is_valid(tests_info->aging_info)) {
        tests_info->aging_info = lv_label_create(tests_info->root);
        EXCEPTION_CHECK_VOID(tests_info->aging_info);
    }
    
    // 计算老化时间（单位：秒）
    uint32_t age_time = 0;
    int ret = proc->tests->get_factory_tests_event_process(FAC_TST_AGING_TEST_GET_CURRENT_AGE_TIME_EVENT, 0, 0, &age_time);
    if (ret != 0) {
        // 如果获取失败，使用默认值0
        age_time = 0;
    }
    
    //如果正处在老化测试中，则加上pump的运行时间
	if (proc->tests->tests_manager_get_current_mode() == TST_AGING_TEST_TYPE) {
		int is_working = 0;
		READ_PUMP_IS_WORKING(proc->pump, is_working);
		if (is_working) {
            const pump_result_info_t *result_info = NULL;
            READ_PUMP_RESULT_INFO(proc->pump, BREAST_PUMP_RESULT_PUMPING, &result_info);
            if (result_info != NULL) {
                // current_mode_work_time 单位是毫秒，需要转换为秒
                age_time += result_info->current_mode_work_time / 1000;
           }
        }
        //显示老化时间标识
        lv_obj_clear_flag(tests_info->aging_info, LV_OBJ_FLAG_HIDDEN);  /// 确保可见
	}
	else {
        //老化时间标识隐藏
        lv_obj_add_flag(tests_info->aging_info, LV_OBJ_FLAG_HIDDEN);  /// 确保可见
        return;
    }
    
    //转换为时分秒格式
    uint8_t hours = age_time / 3600;
    uint8_t minutes = (age_time % 3600) / 60;
    uint8_t seconds = age_time % 60;
    char aging_info_str[64] = {0};
    snprintf(aging_info_str, sizeof(aging_info_str), 
        "aging-time-%02dh-%02dm-%02ds", hours, minutes, seconds);
    lv_label_set_text(tests_info->aging_info, aging_info_str);
    lv_obj_set_style_text_color(tests_info->aging_info, lv_color_hex(PRODUCT_TESTS_HOME_AGING_INFO_COLOR), 0);
    lv_obj_set_style_text_font(tests_info->aging_info, &lv_font_oppo_sans_14_medium, 0);
    lv_obj_set_style_text_align(tests_info->aging_info, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_bg_opa(tests_info->aging_info, LV_OPA_TRANSP, 0);  /// 背景透明
    lv_obj_set_style_border_opa(tests_info->aging_info, LV_OPA_TRANSP, 0);  /// 边框透明
    lv_obj_align(tests_info->aging_info, LV_ALIGN_TOP_LEFT, 
        PRODUCT_TESTS_HOME_AGING_INFO_X, PRODUCT_TESTS_HOME_AGING_INFO_Y);
    lv_obj_clear_flag(tests_info->aging_info, LV_OBJ_FLAG_HIDDEN);  /// 确保可见
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_product_tests_home_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (tests_info == NULL) {
        tests_info = (product_tests_home_display_info_t *)u_malloz(sizeof(product_tests_home_display_info_t));
        EXCEPTION_CHECK_VOID(tests_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    product_tests_ui_root_init();
    ble_ui_init(thiz);
    tests_info->force_update = true;
    tests_info->popup_ui_index = PRODUCT_TEST_POPUP_UI_NONE;
}

void ui_display_product_tests_home_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(tests_info);
    ble_ui_deinit(thiz);
    product_tip_popup_ui_remove(thiz);
    LVGL_OBJ_DELETE(tests_info->root);
    u_free(tests_info);
    tests_info = NULL;
}

int ui_display_product_tests_home_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(tests_info);
    EXCEPTION_CHECK_INT(tests_info->root);
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc->ble);
    EXCEPTION_CHECK_INT(proc->battery);
    ble_ui_update(thiz, tests_info->root, proc->ble, COMMON_BLE_UI_X, COMMON_BLE_UI_Y);
    uint8_t mac_addr[6] = {0};
    proc->ble->get_mac_addr(mac_addr);
    product_tests_qrcode_update(thiz, mac_addr);
    product_tests_aging_info_update(thiz, proc);
    product_tests_mac_name_update(thiz, mac_addr);
    product_tip_popup_ui_update(thiz, proc, tests_info->root, &tests_info->popup_ui_index);
    tests_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> product_tests home fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
