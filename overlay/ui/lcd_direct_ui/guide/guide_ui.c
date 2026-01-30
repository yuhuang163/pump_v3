/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep
#include "guide_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_GUIDE_COMP_BG                                0
#define GUIDE_CONTENT_COUNT                                     4
#define GUIDE_CONTENT_COLOR                                     0x3C3D40
#define GUIDE_QRCODE_BG_COLOR                                   0xFFFFFF
#define GUIDE_QRCODE_FG_COLOR                                   0x000000
#define GUIDE_QRCODE_BORDER_COLOR                               0xA6A6A6

typedef struct {
    uint16_t img_id;
    uint16_t icon_x;
    uint16_t icon_y;
    uint16_t text_x;
    uint16_t text_y;
    const char *name;
} guide_content_info_t;

typedef struct {  
    lv_obj_t * root;
#if IS_SUPPORT_GUIDE_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * qrcode;
    lv_obj_t * qrcode_container;
    lv_obj_t * icon;
    lv_obj_t * text;
    res_img_info_t icon_info;
    uint8_t force_update;
    uint8_t page_index;  
} guide_display_info_t;
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_GUIDE_COMP_BG
static PSRAM_DATA_LOC uint8_t guide_bg_data[] = {
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

const static guide_content_info_t guide_content_infos[GUIDE_CONTENT_COUNT] = {
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

static guide_display_info_t *guide_info = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_ui_root_init(void)
{
    if (guide_info->root == NULL || !lv_obj_is_valid(guide_info->root)) {
        guide_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(guide_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(guide_info->root, lv_color_white(), 0);     /// 设置背景颜色为白色
    lv_obj_set_style_bg_opa(guide_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(guide_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(guide_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(guide_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(guide_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(guide_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

#if IS_SUPPORT_GUIDE_COMP_BG
static void guide_bg_update(ui_manager_context_t *thiz)
{
    static uint8_t status = 0;
    static uint32_t start_time = 0;
    if (guide_info->background == NULL || !lv_obj_is_valid(guide_info->root)) {
        guide_info->background = lv_image_create(guide_info->root);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(guide_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(guide_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, guide_bg_data,
            sizeof(guide_bg_data), "png");
        lv_image_set_src(guide_info->background, (const void *)&bg_img_path);
        lv_obj_align(guide_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#endif

static int guide_icon_image_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    if (guide_info->icon == NULL || !lv_obj_is_valid(guide_info->root)) {
        guide_info->icon = lv_image_create(guide_info->root);
        EXCEPTION_CHECK_INT(guide_info->icon);
    }

    if (page_index == GUIDE_PAGE_GUIDE_QRCODE) {
        lv_obj_add_flag(guide_info->icon, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(guide_info->icon, LV_OBJ_FLAG_HIDDEN);
        common_ui_img_resource_init(thiz, guide_content_infos[page_index].img_id, &guide_info->icon_info);
        EXCEPTION_CHECK_INT(guide_info->icon_info.data);
        lv_fs_path_ex_t complete_tip_img_path;
        lv_fs_make_path_from_buffer(&complete_tip_img_path, LV_FS_MEMFS_LETTER,
            guide_info->icon_info.data, guide_info->icon_info.size, "baj");
        lv_image_set_src(guide_info->icon, complete_tip_img_path.path);
        lv_obj_align(guide_info->icon, LV_ALIGN_TOP_LEFT, 
            guide_content_infos[page_index].icon_x, guide_content_infos[page_index].icon_y);
    }
    return 0;
}

static int guide_text_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    if (guide_info->text == NULL || !lv_obj_is_valid(guide_info->text)) {
        guide_info->text = lv_label_create(guide_info->root);
        EXCEPTION_CHECK_INT(guide_info->text);
    }

    if (page_index >= GUIDE_CONTENT_COUNT) {
        lv_label_set_text(guide_info->text, guide_content_infos[0].name);
        lv_obj_set_style_text_color(guide_info->text, lv_color_hex(GUIDE_CONTENT_COLOR), 0);
        lv_obj_set_style_text_font(guide_info->text, &lv_font_roboto_17_regular, 0);  /// 设置字体
        lv_obj_set_style_text_line_space(guide_info->text, 4, 0);  /// 设置行间距（像素）
        // lv_obj_set_width(guide_info->text, 360 - guide_content_infos[0].text_x);  /// 设置label宽度以便文本在区域内居中
        lv_obj_set_style_text_align(guide_info->text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_align(guide_info->text, LV_ALIGN_TOP_LEFT, 
            guide_content_infos[0].text_x, guide_content_infos[0].text_y);
        LOG_UI("===> guide text: %s", guide_content_infos[0].name);
    } else {
        lv_label_set_text(guide_info->text, guide_content_infos[page_index].name);
        lv_obj_set_style_text_color(guide_info->text, lv_color_hex(GUIDE_CONTENT_COLOR), 0);
        lv_obj_set_style_text_font(guide_info->text, &lv_font_roboto_17_regular, 0);  /// 设置字体
        lv_obj_set_style_text_line_space(guide_info->text, 4, 0);  /// 设置行间距（像素）
        // lv_obj_set_width(guide_info->text, LV_PCT(100));  /// 设置label宽度为屏幕宽度以便文本居中
        lv_obj_set_style_text_align(guide_info->text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_align(guide_info->text, LV_ALIGN_TOP_LEFT, 
            guide_content_infos[page_index].text_x, guide_content_infos[page_index].text_y);
    }
    return 0;
}

static int guide_qrcode_update(ui_manager_context_t *thiz, uint8_t page_index)
{  
    ///  创建容器包裹二维码（用于添加边框和内边距效果）
    if (guide_info->qrcode_container == NULL || !lv_obj_is_valid(guide_info->qrcode_container)) {
        guide_info->qrcode_container = lv_obj_create(guide_info->root);
        EXCEPTION_CHECK_INT(guide_info->qrcode_container);
        ///  设置容器尺寸
        lv_obj_set_size(guide_info->qrcode_container, 126, 126);
        ///  设置容器背景颜色为白色
        lv_obj_set_style_bg_color(guide_info->qrcode_container, lv_color_hex(GUIDE_QRCODE_BG_COLOR), 0);
        lv_obj_set_style_bg_opa(guide_info->qrcode_container, LV_OPA_COVER, 0);
        ///  设置容器边框样式（灰色边框，宽度2像素）
        lv_obj_set_style_border_color(guide_info->qrcode_container, lv_color_hex(GUIDE_QRCODE_BORDER_COLOR), 0);
        lv_obj_set_style_border_width(guide_info->qrcode_container, 2, 0);
        ///  设置内边距为4像素，让边框和二维码内容之间有白色间距
        lv_obj_set_style_pad_all(guide_info->qrcode_container, 10, 0);
        ///  禁用容器的滚动功能
        lv_obj_clear_flag(guide_info->qrcode_container, LV_OBJ_FLAG_SCROLLABLE);
    }
    
    if (guide_info->qrcode == NULL || !lv_obj_is_valid(guide_info->qrcode)) {
        guide_info->qrcode = lv_qrcode_create(guide_info->qrcode_container);
        EXCEPTION_CHECK_INT(guide_info->qrcode);
    }
    
    if (page_index != GUIDE_PAGE_GUIDE_QRCODE) {
        lv_obj_add_flag(guide_info->qrcode_container, LV_OBJ_FLAG_HIDDEN); /// 隐藏容器
    } else {
        lv_obj_clear_flag(guide_info->qrcode_container, LV_OBJ_FLAG_HIDDEN); /// 显示容器
        lv_qrcode_set_size(guide_info->qrcode, 102);
        lv_qrcode_set_dark_color(guide_info->qrcode, lv_color_hex(GUIDE_QRCODE_FG_COLOR));
        lv_qrcode_set_light_color(guide_info->qrcode, lv_color_hex(GUIDE_QRCODE_BG_COLOR));
        lv_qrcode_update(guide_info->qrcode, bind_qr_str[0], strlen(bind_qr_str[0]));
        lv_obj_center(guide_info->qrcode);
        lv_obj_align(guide_info->qrcode_container, LV_ALIGN_TOP_LEFT, 117 + 4, 85);
    }
    return 0;
}

static void guide_ui_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    if (guide_info->force_update == false && guide_info->page_index == page_index) {
        return;
    }
    int ret = guide_qrcode_update(thiz, page_index);
    if (ret != 0) {
        LOG_UI("===> guide qrcode update failed");
        return;
    }
    ret = guide_icon_image_update(thiz, page_index);
    if (ret != 0) {
        LOG_UI("===> guide icon image update failed");
        return;
    }
    ret = guide_text_update(thiz, page_index);
    if (ret != 0) {
        LOG_UI("===> guide text update failed");
        return;
    }
    guide_info->page_index = page_index;
    // LOG_UI("===> guide update success, page_index: %d", page_index);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_guide_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (guide_info == NULL) {
        guide_info = (guide_display_info_t *)u_malloz(sizeof(guide_display_info_t));
        EXCEPTION_CHECK_VOID(guide_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    guide_ui_root_init();
    battery_icon_init(thiz);
    ble_ui_init(thiz);
    guide_info->force_update = true;
}

void ui_display_guide_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(guide_info);
    battery_icon_deinit(thiz);
    ble_ui_deinit(thiz);
    common_ui_img_resource_deinit(thiz, &guide_info->icon_info);
    LVGL_OBJ_DELETE(guide_info->root);  /// 删除 root 时会自动删除所有子对象（包括 qrcode_container） 
    u_free(guide_info);
    guide_info = NULL;
}

int ui_display_guide_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(guide_info);
    EXCEPTION_CHECK_INT(guide_info->root);
    ui_proc_guide_t *proc = (ui_proc_guide_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
#if IS_SUPPORT_GUIDE_COMP_BG
    guide_bg_update(thiz);
#endif
    ble_ui_update(thiz, guide_info->root, proc->ble);
    battery_icon_update(thiz, guide_info->root, proc->battery);
    guide_ui_update(thiz, proc->page_index);
    thiz->priv.fps_count++;
    guide_info->force_update = false;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> guide fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
