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
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
#define IS_SUPPORT_BATTERY_PERCENT                                   1
#else
#define IS_SUPPORT_BATTERY_PERCENT                                   0
#endif

#define AGING_TEST_TEXT_BLUE_COLOR                            (0x01D65A)    // 文本颜色蓝色
#define AGING_TEST_TEXT_RED_COLOR                             (0xFF0000)    // 文本颜色红色


#define AGING_TEST_MAC_NAME_X                                (80)
#define AGING_TEST_MAC_NAME_Y                                (80)
#define AGING_TEST_AGING_INFO_X                              (AGING_TEST_MAC_NAME_X)
#define AGING_TEST_AGING_INFO_Y                              (AGING_TEST_MAC_NAME_Y + 36)
#define AGING_TEST_AGING_STATUS_X                            (AGING_TEST_MAC_NAME_X)
#define AGING_TEST_AGING_STATUS_Y                            (AGING_TEST_AGING_INFO_Y + 48)
#define AGING_TEST_BATTERY_PERCENT_X                         (AGING_TEST_MAC_NAME_X )
#define AGING_TEST_BATTERY_PERCENT_Y                         (AGING_TEST_AGING_STATUS_Y + 52)

typedef struct {
    const char *name;
    uint16_t text_x;
    uint16_t text_y;
} aging_tests_content_info_t;

typedef struct {  
    lv_obj_t * root;
    lv_obj_t * mac_name;
#if IS_SUPPORT_BATTERY_PERCENT
    lv_obj_t * battery_percent;
#endif
    lv_obj_t * aging_info;    // 老化时间
    lv_obj_t * aging_status;  // aging-running / aging-complete
    uint8_t force_update;
    uint8_t mac_addr[6];
    bool all_color_change;
    uint8_t rsv[16];
} aging_test_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/


static aging_test_display_info_t *tests_info = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void aging_test_ui_root_init(void)
{
    if (tests_info->root == NULL || !lv_obj_is_valid(tests_info->root)) {
        tests_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(tests_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(tests_info->root, lv_color_hex(0xFF9999), 0);     /// 设置背景颜色为淡粉色
    lv_obj_set_style_bg_opa(tests_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(tests_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(tests_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(tests_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(tests_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(tests_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

#if IS_SUPPORT_BATTERY_PERCENT
static void aging_test_ui_battery_percent_update(ui_manager_context_t *thiz,  uint16_t battery_percent, uint8_t is_red)
{
    if (tests_info->battery_percent == NULL || !lv_obj_is_valid(tests_info->battery_percent)) {
        tests_info->battery_percent = lv_label_create(tests_info->root);
    }
    ////
    char battery_percent_str[16] = {0};
    snprintf(battery_percent_str, sizeof(battery_percent_str), PRODUCT_TESTS_BATTERY_FMT, battery_percent);
    lv_label_set_text(tests_info->battery_percent, battery_percent_str);
    lv_obj_set_style_text_color(tests_info->battery_percent, lv_color_hex(is_red ? AGING_TEST_TEXT_RED_COLOR : AGING_TEST_TEXT_BLUE_COLOR), 0);
    lv_obj_set_style_text_font(tests_info->battery_percent, UI_FONT_HINT, 0);  /// 设置字体
    lv_obj_align(tests_info->battery_percent, LV_ALIGN_TOP_LEFT, 
        AGING_TEST_BATTERY_PERCENT_X, AGING_TEST_BATTERY_PERCENT_Y);
}
#endif

/// 设置背景颜色变化
static void aging_test_ui_background_color_update(ui_manager_context_t *thiz, uint8_t black_bg)
{
    EXCEPTION_CHECK_VOID(thiz);
    ///蓝色和粉色交替变化
    lv_obj_set_style_bg_color(tests_info->root, lv_color_hex(black_bg ? 0x0000FF : 0xFF9999), 0);
    lv_obj_set_style_bg_opa(tests_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(tests_info->root, 0, 0);
    lv_obj_set_style_pad_all(tests_info->root, 0, 0);
    lv_obj_align(tests_info->root, LV_ALIGN_CENTER, 0, 0);
}


static void aging_test_ui_mac_name_update(ui_manager_context_t *thiz, uint8_t *mac_addr, uint8_t is_red)
{
    if (tests_info->mac_name == NULL || !lv_obj_is_valid(tests_info->mac_name)) {
        tests_info->mac_name = lv_label_create(tests_info->root);
    }
    
    if (tests_info->force_update) {
        char mac_name[16] = {0};
        ///根据ROOT_DEVICE_NAME显示不同的name
        snprintf(mac_name, sizeof(mac_name), "%s-%02X%02X", ROOT_DEVICE_NAME, mac_addr[0], mac_addr[1]);
        lv_label_set_text(tests_info->mac_name, mac_name);
    }
    lv_obj_set_style_text_color(tests_info->mac_name, lv_color_hex(is_red ? AGING_TEST_TEXT_BLUE_COLOR : AGING_TEST_TEXT_RED_COLOR), 0);
    lv_obj_set_style_text_font(tests_info->mac_name, UI_FONT_HINT, 0);  /// 设置字体
    // lv_obj_set_style_text_line_space(tests_info->mac_name, 4, 0);  /// 设置行间距（像素）
    // lv_obj_set_style_text_align(tests_info->mac_name, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
    lv_obj_align(tests_info->mac_name, LV_ALIGN_TOP_LEFT, 
        AGING_TEST_MAC_NAME_X, AGING_TEST_MAC_NAME_Y);
}

static void aging_test_ui_aging_info_update(ui_manager_context_t *thiz, ui_proc_product_test_t *proc, uint8_t is_red)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(tests_info);
    EXCEPTION_CHECK_VOID(tests_info->root);
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->tests);

    if (tests_info->aging_info == NULL || !lv_obj_is_valid(tests_info->aging_info)) {
        tests_info->aging_info = lv_label_create(tests_info->root);
        EXCEPTION_CHECK_VOID(tests_info->aging_info);
    }
    if (tests_info->aging_status == NULL || !lv_obj_is_valid(tests_info->aging_status)) {
        tests_info->aging_status = lv_label_create(tests_info->root);
        EXCEPTION_CHECK_VOID(tests_info->aging_status);
    }

    /*
     * 老化页本身即表示处于老化 UI，状态必须常显。
     * 勿再依赖 current_mode：产线竞态下 mode 可能短暂偏离 TST_AGING_TEST_TYPE，
     * 会导致标签被 HIDDEN，而背景色/MAC 仍在刷新。
     * 时间与状态分两个 label：时间在上，状态在下（约 2 倍字号）。
     * 当前字体无中文 glyph：aging-complete / aging-running。
     */
    uint32_t age_time = 0;
    int ret = proc->tests->get_factory_tests_event_process(FAC_TST_AGING_TEST_GET_CURRENT_AGE_TIME_EVENT, 0, 0, &age_time);
    if (ret != 0) {
        age_time = 0;
    }

    uint32_t target_time = proc->tests->priv.factory_ctx.aging_target_info.total_target_time;
    bool aging_done = (proc->tests->priv.factory_ctx.aging_result_info.aging_status
                       == AGING_TEST_STATUS_SUCCESS);
    if (!aging_done && target_time > 0 && age_time >= target_time) {
        aging_done = true;
    }
    /* 完成后冻结时间，避免 current_cycle 继续累加导致“完成了还在跑” */
    if (aging_done && target_time > 0 && age_time > target_time) {
        age_time = target_time;
    }

    uint32_t hours = age_time / 3600U;
    uint32_t minutes = (age_time % 3600U) / 60U;
    uint32_t seconds = age_time % 60U;
    char aging_info_str[64] = {0};
    snprintf(aging_info_str, sizeof(aging_info_str),
        PRODUCT_TESTS_AGING_TIME_FMT,
        (unsigned long)hours, (unsigned long)minutes, (unsigned long)seconds);
    lv_label_set_text(tests_info->aging_info, aging_info_str);
    lv_obj_set_style_text_color(tests_info->aging_info,
        lv_color_hex(is_red ? AGING_TEST_TEXT_RED_COLOR : AGING_TEST_TEXT_BLUE_COLOR), 0);
    lv_obj_set_style_text_font(tests_info->aging_info, UI_FONT_HINT, 0);
    lv_obj_set_style_text_align(tests_info->aging_info, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_bg_opa(tests_info->aging_info, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(tests_info->aging_info, LV_OPA_TRANSP, 0);
    lv_obj_align(tests_info->aging_info, LV_ALIGN_TOP_LEFT,
        AGING_TEST_AGING_INFO_X, AGING_TEST_AGING_INFO_Y);
    lv_obj_clear_flag(tests_info->aging_info, LV_OBJ_FLAG_HIDDEN);

    lv_label_set_text(tests_info->aging_status, aging_done ? PRODUCT_TESTS_AGING_COMPLETE_TEXT : PRODUCT_TESTS_AGING_RUNNING_TEXT);
    lv_obj_set_style_text_color(tests_info->aging_status,
        lv_color_hex(is_red ? AGING_TEST_TEXT_RED_COLOR : AGING_TEST_TEXT_BLUE_COLOR), 0);
    lv_obj_set_style_text_font(tests_info->aging_status, UI_FONT_TITLE_LG, 0);
    lv_obj_set_style_text_align(tests_info->aging_status, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_bg_opa(tests_info->aging_status, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(tests_info->aging_status, LV_OPA_TRANSP, 0);
    lv_obj_align(tests_info->aging_status, LV_ALIGN_TOP_LEFT,
        AGING_TEST_AGING_STATUS_X, AGING_TEST_AGING_STATUS_Y);
    lv_obj_clear_flag(tests_info->aging_status, LV_OBJ_FLAG_HIDDEN);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_aging_test_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (tests_info == NULL) {
        tests_info = (aging_test_display_info_t *)u_malloz(sizeof(aging_test_display_info_t));
        EXCEPTION_CHECK_VOID(tests_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    tests_info->all_color_change = false;
    aging_test_ui_root_init();
    tests_info->force_update = true;
}

void ui_display_aging_test_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(tests_info);
    LVGL_OBJ_DELETE(tests_info->root);
    u_free(tests_info);
    tests_info = NULL;
}

int ui_display_aging_test_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(tests_info);
    EXCEPTION_CHECK_INT(tests_info->root);
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc->ble);
    EXCEPTION_CHECK_INT(proc->battery);
    uint8_t mac_addr[6] = {0};
    proc->ble->get_mac_addr(mac_addr);
    
    aging_test_ui_aging_info_update(thiz, proc, tests_info->all_color_change);
    aging_test_ui_mac_name_update(thiz, mac_addr, tests_info->all_color_change);
    aging_test_ui_background_color_update(thiz, tests_info->all_color_change);
#if IS_SUPPORT_BATTERY_PERCENT
    aging_test_ui_battery_percent_update(thiz, proc->battery->get_battery_percent(), tests_info->all_color_change);
#endif
    tests_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 10000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> product_tests home fps: %d", thiz->priv.fps_count / 10);
        thiz->priv.fps_count = 0;
        tests_info->all_color_change = !tests_info->all_color_change;
    }
    return 0;
}
