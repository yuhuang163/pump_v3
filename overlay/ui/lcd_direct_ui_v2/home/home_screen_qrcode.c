/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_home.h"   // IWYU pragma: keep
#include "home_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_HOME_QRCODE_COMP_BG                         (0)
#define HOME_QRCODE_BG_COLOR                                   0xFFFFFF
#define HOME_QRCODE_FG_COLOR                                   0x000000
#define HOME_QRCODE_BORDER_COLOR                               (0xFFF2F3)  // 0xA6A6A6
#define HOME_BIND_TEXT_COLOR                                   (0x6A5A5B)
#define HOME_QRCODE_X_OFFSET                                   (111)
#define HOME_QRCODE_Y_OFFSET                                   (71 + 12)
#define HOME_BIND_TEXT_X_OFFSET                                (66)
#define HOME_BIND_TEXT_Y_OFFSET                                (228 + 13)

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_HOME_QRCODE_COMP_BG
    lv_obj_t * backgroud;
#endif
    lv_obj_t * qrcode_container;
    lv_obj_t * qrcode;
    lv_obj_t * arrow; /// 箭头
    lv_obj_t * arrow_text; /// 箭头文本框
    lv_obj_t * text;
    res_img_info_t arrow_info;
    uint8_t force_update;
} home_qrcode_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static home_qrcode_display_info_t *home_qrcode_info;
const static char *bind_qr_str[] = {
    "https://lvgl.io",
};

#if IS_SUPPORT_HOME_QRCODE_COMP_BG
static PSRAM_DATA_LOC uint8_t home_bg_data[] = {
    #include "comparison_png2txt/home_ble_cmp_360x360.txt"
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
static void ui_home_qrcode_root_init(void)
{
    if (home_qrcode_info->root == NULL || !lv_obj_is_valid(home_qrcode_info->root)) {
        home_qrcode_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(home_qrcode_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(home_qrcode_info->root, lv_color_white(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(home_qrcode_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(home_qrcode_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(home_qrcode_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(home_qrcode_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(home_qrcode_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(home_qrcode_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

#if IS_SUPPORT_HOME_QRCODE_COMP_BG
static void ui_home_qrcode_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (home_qrcode_info->backgroud == NULL || !lv_obj_is_valid(home_qrcode_info->backgroud)) {
        home_qrcode_info->backgroud = lv_image_create(home_qrcode_info->root);
        EXCEPTION_CHECK_VOID(home_qrcode_info->backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(home_qrcode_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(home_qrcode_info->backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, home_bg_data,
            sizeof(home_bg_data), "png");
        lv_image_set_src(home_qrcode_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(home_qrcode_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void ui_home_qrcode_bg_update(ui_manager_context_t *thiz)
{
    common_ui_bg_update(thiz, home_qrcode_info->root);
}
#endif

static void home_qrcode_update(ui_manager_context_t *thiz)
{  
    ///  创建容器包裹二维码（用于添加边框和内边距效果）
    if (home_qrcode_info->qrcode_container == NULL || !lv_obj_is_valid(home_qrcode_info->qrcode_container)) {
        home_qrcode_info->qrcode_container = lv_obj_create(home_qrcode_info->root);
        EXCEPTION_CHECK_VOID(home_qrcode_info->qrcode_container);
        ///  设置容器尺寸
        lv_obj_set_size(home_qrcode_info->qrcode_container, 138, 138);
        ///  设置容器背景颜色为白色
        lv_obj_set_style_bg_color(home_qrcode_info->qrcode_container, lv_color_hex(HOME_QRCODE_BG_COLOR), 0);
        lv_obj_set_style_bg_opa(home_qrcode_info->qrcode_container, LV_OPA_0, 0); /// 设置容器背景透明
        ///  设置容器边框样式（灰色边框，宽度2像素）
        lv_obj_set_style_border_color(home_qrcode_info->qrcode_container, lv_color_hex(HOME_QRCODE_BORDER_COLOR), 0);
        lv_obj_set_style_border_width(home_qrcode_info->qrcode_container, 2, 0);
        ///  设置内边距为4像素，让边框和二维码内容之间有白色间距
        lv_obj_set_style_pad_all(home_qrcode_info->qrcode_container, 10, 0);
        ///  禁用容器的滚动功能
        lv_obj_clear_flag(home_qrcode_info->qrcode_container, LV_OBJ_FLAG_SCROLLABLE);
    } else {
        lv_obj_clear_flag(home_qrcode_info->qrcode_container, LV_OBJ_FLAG_HIDDEN);
    }
    
    if (home_qrcode_info->qrcode == NULL || !lv_obj_is_valid(home_qrcode_info->qrcode)) {
        home_qrcode_info->qrcode = lv_qrcode_create(home_qrcode_info->qrcode_container);
        EXCEPTION_CHECK_VOID(home_qrcode_info->qrcode);
    } else {
        lv_obj_clear_flag(home_qrcode_info->qrcode, LV_OBJ_FLAG_HIDDEN);
    }
    lv_qrcode_set_size(home_qrcode_info->qrcode, 124);
    lv_qrcode_set_dark_color(home_qrcode_info->qrcode, lv_color_hex(HOME_QRCODE_FG_COLOR));
    lv_qrcode_set_light_color(home_qrcode_info->qrcode, lv_color_hex(HOME_QRCODE_BG_COLOR));
    lv_qrcode_update(home_qrcode_info->qrcode, bind_qr_str[0], strlen(bind_qr_str[0]));
    lv_obj_center(home_qrcode_info->qrcode);
    lv_obj_align(home_qrcode_info->qrcode_container, LV_ALIGN_TOP_LEFT, 
        HOME_QRCODE_X_OFFSET, HOME_QRCODE_Y_OFFSET);
}

static void home_qrcode_ui_bind_update(ui_manager_context_t *thiz)
{
    home_qrcode_update(thiz);
    if (home_qrcode_info->text == NULL || !lv_obj_is_valid(home_qrcode_info->text)) {
        home_qrcode_info->text = lv_label_create(home_qrcode_info->root);
        EXCEPTION_CHECK_VOID(home_qrcode_info->text);
    } else {
        lv_obj_clear_flag(home_qrcode_info->text, LV_OBJ_FLAG_HIDDEN);
    }   
    lv_label_set_text(home_qrcode_info->text, "Scan to connect for tracking,  remote\ncontrol, custom modes... ");
    lv_obj_set_style_text_color(home_qrcode_info->text, lv_color_hex(HOME_BIND_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(home_qrcode_info->text, &lv_font_oppo_sans_14_medium, 0);
    lv_obj_set_style_text_align(home_qrcode_info->text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
    lv_obj_set_style_text_line_space(home_qrcode_info->text, 6, 0); /// 设置行间距（像素）
    lv_obj_set_style_text_opa(home_qrcode_info->text, LV_OPA_COVER, 0);
    lv_obj_align(home_qrcode_info->text, LV_ALIGN_TOP_LEFT, HOME_BIND_TEXT_X_OFFSET, HOME_BIND_TEXT_Y_OFFSET);
}

static void home_qrcode_ui_update(ui_manager_context_t *thiz)
{
    if (home_qrcode_info->force_update == false) {
        return;
    }
    home_qrcode_ui_bind_update(thiz);
}


/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_home_qrcode_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (home_qrcode_info == NULL) {
        home_qrcode_info = (home_qrcode_display_info_t *)u_malloz(sizeof(home_qrcode_display_info_t));
        EXCEPTION_CHECK_VOID(home_qrcode_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_home_qrcode_root_init(); /// 初始化 root 容器
    common_ui_bg_init(thiz);
    home_qrcode_info->force_update = true;
}

void ui_display_home_qrcode_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(home_qrcode_info);
    EXCEPTION_CHECK_VOID(home_qrcode_info->root);
    common_ui_img_resource_deinit(thiz, &home_qrcode_info->arrow_info);
    common_ui_bg_deinit(thiz);
    LVGL_OBJ_DELETE(home_qrcode_info->root);
    u_free(home_qrcode_info);
    home_qrcode_info = NULL;
}

int ui_display_home_qrcode_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(home_qrcode_info);
    EXCEPTION_CHECK_INT(home_qrcode_info->root);
    ui_home_qrcode_bg_update(thiz);
    home_qrcode_ui_update(thiz);
    home_qrcode_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home qrcode fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}