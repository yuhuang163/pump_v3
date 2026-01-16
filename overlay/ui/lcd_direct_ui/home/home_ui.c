/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep
#include "home_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_HOME_COMP_BG                                0
#define HOME_CONTENT_COUNT                                     4
#define HOME_CONTENT_COLOR                                     0x3C3D40
#define HOME_QRCODE_BG_COLOR                                   0xFFFFFF
#define HOME_QRCODE_FG_COLOR                                   0x000000
#define HOME_QRCODE_BORDER_COLOR                               0xA6A6A6

typedef struct {
    uint16_t img_id;
    uint16_t icon_x;
    uint16_t icon_y;
    uint16_t text_x;
    uint16_t text_y;
    const char *name;
} home_content_info_t;

typedef struct {  
    lv_obj_t * root;
#if IS_SUPPORT_HOME_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * qrcode;
    lv_obj_t * qrcode_container;
    lv_obj_t * icon;
    lv_obj_t * text;
    res_img_info_t icon_info;
    uint8_t force_update;
    uint8_t is_connected;
    uint8_t percent;
    uint8_t is_charging;
    uint8_t page_index;  
} home_display_info_t;
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_HOME_COMP_BG
static PSRAM_DATA_LOC uint8_t home_bg_data[] = {
    // #include "comparison_png2txt/guide_phone_cmp_360x360.txt"
    // #include "comparison_png2txt/guide_chasing_milk_cmp_360x360.txt"
    // #include "comparison_png2txt/guide_adjust_comp_360x360.txt"
    // #include "comparison_png2txt/guide_milk_quantity_cmp_360x360.txt"
    #include "comparison_png2txt/guide_qrcode_cmp_360x360.txt"
};
#endif

const static char *bind_qr_str[] = {
    "https://lvgl.io",
};

const static home_content_info_t home_content_infos[HOME_CONTENT_COUNT] = {
    { RESOURCE_ID_GUIDE_PHONE, 
      (146 + 4), (85), 
      (63 + 6), (234 + 1), 
      "Scan and download the app to\nenjoy the best breast milk\nextraction experience."},
    { RESOURCE_ID_GUIDE_CHASING_MILK,
      (46 + 4), (80),
      (88 + 11), (251 + 2), 
      "Choose your favorite\nsmart pumping mode!"},
    { RESOURCE_ID_GUIDE_ADJUST_LEVEL,
      (89 + 4), (86),
      (46 + 22), (251 + 2), 
      "Twist the knob to adjust the\nsuction strength on the left and\nright sides."},
    { RESOURCE_ID_GUIDE_MILK_QUANTITY,
      (69 + 4), (95),
      (60 + 8), (272 + 2), 
      "Control it via the app and track\nyour milk volume."
    },
};  

static home_display_info_t *home_info = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_ui_root_init(void)
{
    if (home_info->root == NULL || !lv_obj_is_valid(home_info->root)) {
        home_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(home_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(home_info->root, lv_color_white(), 0);     /// 设置背景颜色为白色
    lv_obj_set_style_bg_opa(home_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(home_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(home_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(home_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(home_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(home_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

#if IS_SUPPORT_HOME_COMP_BG
static void home_bg_update(ui_manager_context_t *thiz)
{
    static uint8_t status = 0;
    static uint32_t start_time = 0;
    if (home_info->background == NULL || !lv_obj_is_valid(home_info->root)) {
        home_info->background = lv_image_create(home_info->root);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(home_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(home_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, home_bg_data,
            sizeof(home_bg_data), "png");
        lv_image_set_src(home_info->background, (const void *)&bg_img_path);
        lv_obj_align(home_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#endif

static void home_ble_update(ui_manager_context_t *thiz, ble_manager_context_t *ble)
{
    EXCEPTION_CHECK_VOID(ble);
    uint8_t is_connected = ble->ble_is_connected();
    if (home_info->is_connected == is_connected && home_info->force_update == false) {
        return;
    }

    if (ble_ui_update(thiz, home_info->root, is_connected) != 0) {
        return;
    }
    home_info->is_connected = is_connected;
}

static void home_battery_update(ui_manager_context_t *thiz, battery_manager_context_t *battery)
{
    EXCEPTION_CHECK_VOID(battery);
    uint8_t percent = battery->get_battery_percent();
    uint8_t is_charging = battery->is_charge_plug_in();
    if (home_info->percent == percent && 
        home_info->is_charging == is_charging &&
        home_info->force_update == false) {
        return;
    }

    if (battery_icon_update(thiz, home_info->root, percent, is_charging) != 0) {
        return;
    }
    
    home_info->percent = percent;
    home_info->is_charging = is_charging;
}

static int home_icon_image_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    if (home_info->icon == NULL || !lv_obj_is_valid(home_info->root)) {
        home_info->icon = lv_image_create(home_info->root);
        EXCEPTION_CHECK_INT(home_info->icon);
    }

    if (page_index == HOME_PAGE_GUIDE_QRCODE) {
        lv_obj_add_flag(home_info->icon, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(home_info->icon, LV_OBJ_FLAG_HIDDEN);
        common_ui_img_resource_init(thiz, home_content_infos[page_index].img_id, &home_info->icon_info);
        EXCEPTION_CHECK_INT(home_info->icon_info.data);
        lv_fs_path_ex_t complete_tip_img_path;
        lv_fs_make_path_from_buffer(&complete_tip_img_path, LV_FS_MEMFS_LETTER,
            home_info->icon_info.data, home_info->icon_info.size, "baj");
        lv_image_set_src(home_info->icon, complete_tip_img_path.path);
        lv_obj_align(home_info->icon, LV_ALIGN_TOP_LEFT, 
            home_content_infos[page_index].icon_x, home_content_infos[page_index].icon_y);
    }
    return 0;
}

static int home_text_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    if (home_info->text == NULL || !lv_obj_is_valid(home_info->text)) {
        home_info->text = lv_label_create(home_info->root);
        EXCEPTION_CHECK_INT(home_info->text);
    }

    if (page_index >= HOME_CONTENT_COUNT) {
        lv_label_set_text(home_info->text, home_content_infos[0].name);
        lv_obj_set_style_text_color(home_info->text, lv_color_hex(HOME_CONTENT_COLOR), 0);
        lv_obj_set_style_text_font(home_info->text, &lv_font_roboto_17_regular, 0);  /// 设置字体
        lv_obj_set_style_text_line_space(home_info->text, 4, 0);  /// 设置行间距（像素）
        // lv_obj_set_width(home_info->text, 360 - home_content_infos[0].text_x);  /// 设置label宽度以便文本在区域内居中
        lv_obj_set_style_text_align(home_info->text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_align(home_info->text, LV_ALIGN_TOP_LEFT, 
            home_content_infos[0].text_x, home_content_infos[0].text_y);
        LOG_UI("===> home text: %s", home_content_infos[0].name);
    } else {
        lv_label_set_text(home_info->text, home_content_infos[page_index].name);
        lv_obj_set_style_text_color(home_info->text, lv_color_hex(HOME_CONTENT_COLOR), 0);
        lv_obj_set_style_text_font(home_info->text, &lv_font_roboto_17_regular, 0);  /// 设置字体
        lv_obj_set_style_text_line_space(home_info->text, 4, 0);  /// 设置行间距（像素）
        // lv_obj_set_width(home_info->text, LV_PCT(100));  /// 设置label宽度为屏幕宽度以便文本居中
        lv_obj_set_style_text_align(home_info->text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_align(home_info->text, LV_ALIGN_TOP_LEFT, 
            home_content_infos[page_index].text_x, home_content_infos[page_index].text_y);
    }
    return 0;
}

static int home_qrcode_update(ui_manager_context_t *thiz, uint8_t page_index)
{  
    ///  创建容器包裹二维码（用于添加边框和内边距效果）
    if (home_info->qrcode_container == NULL || !lv_obj_is_valid(home_info->qrcode_container)) {
        home_info->qrcode_container = lv_obj_create(home_info->root);
        EXCEPTION_CHECK_INT(home_info->qrcode_container);
        ///  设置容器尺寸
        lv_obj_set_size(home_info->qrcode_container, 126, 126);
        ///  设置容器背景颜色为白色
        lv_obj_set_style_bg_color(home_info->qrcode_container, lv_color_hex(HOME_QRCODE_BG_COLOR), 0);
        lv_obj_set_style_bg_opa(home_info->qrcode_container, LV_OPA_COVER, 0);
        ///  设置容器边框样式（灰色边框，宽度2像素）
        lv_obj_set_style_border_color(home_info->qrcode_container, lv_color_hex(HOME_QRCODE_BORDER_COLOR), 0);
        lv_obj_set_style_border_width(home_info->qrcode_container, 2, 0);
        ///  设置内边距为4像素，让边框和二维码内容之间有白色间距
        lv_obj_set_style_pad_all(home_info->qrcode_container, 10, 0);
        ///  禁用容器的滚动功能
        lv_obj_clear_flag(home_info->qrcode_container, LV_OBJ_FLAG_SCROLLABLE);
    }
    
    if (home_info->qrcode == NULL || !lv_obj_is_valid(home_info->qrcode)) {
        home_info->qrcode = lv_qrcode_create(home_info->qrcode_container);
        EXCEPTION_CHECK_INT(home_info->qrcode);
    }
    
    if (page_index != HOME_PAGE_GUIDE_QRCODE) {
        lv_obj_add_flag(home_info->qrcode_container, LV_OBJ_FLAG_HIDDEN); /// 隐藏容器
    } else {
        lv_obj_clear_flag(home_info->qrcode_container, LV_OBJ_FLAG_HIDDEN); /// 显示容器
        lv_qrcode_set_size(home_info->qrcode, 102);
        lv_qrcode_set_dark_color(home_info->qrcode, lv_color_hex(HOME_QRCODE_FG_COLOR));
        lv_qrcode_set_light_color(home_info->qrcode, lv_color_hex(HOME_QRCODE_BG_COLOR));
        lv_qrcode_update(home_info->qrcode, bind_qr_str[0], strlen(bind_qr_str[0]));
        lv_obj_center(home_info->qrcode);
        lv_obj_align(home_info->qrcode_container, LV_ALIGN_TOP_LEFT, 117 + 4, 85);
    }
    return 0;
}

static void home_ui_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    if (home_info->force_update == false && home_info->page_index == page_index) {
        return;
    }
    int ret = home_qrcode_update(thiz, page_index);
    if (ret != 0) {
        LOG_UI("===> home qrcode update failed");
        return;
    }
    ret = home_icon_image_update(thiz, page_index);
    if (ret != 0) {
        LOG_UI("===> home icon image update failed");
        return;
    }
    ret = home_text_update(thiz, page_index);
    if (ret != 0) {
        LOG_UI("===> home text update failed");
        return;
    }
    home_info->page_index = page_index;
    // LOG_UI("===> home update success, page_index: %d", page_index);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_home_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (home_info == NULL) {
        home_info = (home_display_info_t *)u_malloz(sizeof(home_display_info_t));
        EXCEPTION_CHECK_VOID(home_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    home_ui_root_init();
    home_info->force_update = true;
}

void ui_display_home_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(home_info);
    common_ui_img_resource_deinit(thiz, &home_info->icon_info);
    LVGL_OBJ_DELETE(home_info->root);  /// 删除 root 时会自动删除所有子对象（包括 qrcode_container） 
    u_free(home_info);
    home_info = NULL;
}

int ui_display_home_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(home_info);
    EXCEPTION_CHECK_INT(home_info->root);
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
#if IS_SUPPORT_HOME_COMP_BG
    home_bg_update(thiz);
#endif
    home_ble_update(thiz, proc->ble);
    home_battery_update(thiz, proc->battery);
    home_ui_update(thiz, proc->page_index);
    thiz->priv.fps_count++;
    home_info->force_update = false;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
