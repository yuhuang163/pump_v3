/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_system_abnormal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define SYS_ABNORMAL_TIP_TEXT_COLOR                              0xE6DFDF
#define SYS_ABNORMAL_TIP_TEXT_COUNT                              2
#define SYS_ABNORMAL_STATUS_TEXT_COLOR                           0xFFFFFF
#define SYS_ABNORMAL_WARNING_ICON_Y                              118

typedef struct {
    lv_obj_t * root;
    lv_obj_t * icon;
    lv_obj_t * text[SYS_ABNORMAL_TIP_TEXT_COUNT];
    lv_obj_t * mask_layer;            /// 遮罩层对象
    res_img_info_t img_info;          /// 图片信息
    uint8_t force_update;
    uint8_t rsv[15];
} sys_abnormal_disp_info_t;

typedef struct {
    const char *name;
    uint16_t text_y;
    const lv_font_t *font;
    uint16_t text_opa; /// 文本不透明度
    uint32_t color;
} sys_abnormal_text_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static sys_abnormal_disp_info_t *s_abnormal_info;

/// 系统错误文本 — 存储故障 / 通信丢失
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
const static sys_abnormal_text_info_t sys_error_infos[SYS_ABNORMAL_TIP_TEXT_COUNT] = {
    { "System error", 200, &lv_font_oppo_sans_20_medium, LV_OPA_100, SYS_ABNORMAL_STATUS_TEXT_COLOR},
    { "Press and hold knob 10s\nto restart.", 240, &lv_font_oppo_sans_14_medium, LV_OPA_100, SYS_ABNORMAL_TIP_TEXT_COLOR},
};
#else
const static sys_abnormal_text_info_t sys_error_infos[SYS_ABNORMAL_TIP_TEXT_COUNT] = {
    { "System error", 200, &lv_font_oppo_sans_20_medium, LV_OPA_100, SYS_ABNORMAL_STATUS_TEXT_COLOR},
    { "Unplug and replug to restart", 240, &lv_font_oppo_sans_14_medium, LV_OPA_100, SYS_ABNORMAL_TIP_TEXT_COLOR},
};
#endif

/// 电池温度过高文本
const static sys_abnormal_text_info_t temp_error_infos[SYS_ABNORMAL_TIP_TEXT_COUNT] = {
    { "High battery temperature", 200, &lv_font_oppo_sans_20_medium, LV_OPA_100, SYS_ABNORMAL_STATUS_TEXT_COLOR},
    { "Device suspended.", 240, &lv_font_oppo_sans_14_medium, LV_OPA_100, SYS_ABNORMAL_TIP_TEXT_COLOR},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void system_abnormal_ui_root_init(ui_manager_context_t *thiz)
{
    if (s_abnormal_info->root == NULL || !lv_obj_is_valid(s_abnormal_info->root)) {
        s_abnormal_info->root = lv_obj_create(lv_layer_top());
        EXCEPTION_CHECK_VOID(s_abnormal_info->root);
        lv_obj_set_size(s_abnormal_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%
        lv_obj_set_style_bg_color(s_abnormal_info->root, lv_color_black(), 0);     /// 黑色背景
        lv_obj_set_style_bg_opa(s_abnormal_info->root, LV_OPA_COVER, 0);           /// 背景不透明
        lv_obj_set_style_border_width(s_abnormal_info->root, 0, 0);                /// 无边框
        lv_obj_set_style_pad_all(s_abnormal_info->root, 0, 0);                     /// 无内边距
        lv_obj_align(s_abnormal_info->root, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(s_abnormal_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
    }
}

static void system_abnormal_ui_text_update(ui_manager_context_t *thiz, const sys_abnormal_text_info_t *text_info)
{
    EXCEPTION_CHECK_VOID(text_info);

    if (s_abnormal_info->force_update == false) {
        return;
    }

    for (uint8_t i = 0; i < SYS_ABNORMAL_TIP_TEXT_COUNT; i++) {
        if (s_abnormal_info->text[i] == NULL || !lv_obj_is_valid(s_abnormal_info->text[i])) {
            s_abnormal_info->text[i] = lv_label_create(s_abnormal_info->root);
            EXCEPTION_CHECK_VOID(s_abnormal_info->text[i]);
        }
        lv_label_set_text(s_abnormal_info->text[i], text_info[i].name);
        lv_obj_set_style_text_color(s_abnormal_info->text[i], lv_color_hex(text_info[i].color), 0);
        lv_obj_set_style_text_font(s_abnormal_info->text[i], text_info[i].font, 0);
        lv_obj_set_style_text_opa(s_abnormal_info->text[i], text_info[i].text_opa, 0);
        lv_obj_align(s_abnormal_info->text[i], LV_ALIGN_TOP_MID, 0, text_info[i].text_y);
    }
}

static void system_abnormal_ui_icon_update(ui_manager_context_t *thiz, uint16_t img_id, uint16_t y_offset)
{
    if (s_abnormal_info->force_update == false) {
        return;
    }

    if (s_abnormal_info->icon == NULL || !lv_obj_is_valid(s_abnormal_info->icon)) {
        s_abnormal_info->icon = lv_image_create(s_abnormal_info->root);
        EXCEPTION_CHECK_VOID(s_abnormal_info->icon);
    }

    common_ui_img_resource_init(thiz, img_id, &s_abnormal_info->img_info);
    EXCEPTION_CHECK_VOID(s_abnormal_info->img_info.data);
    lv_fs_path_ex_t invalid_popup_icon_img_path;
    lv_fs_make_path_from_buffer(&invalid_popup_icon_img_path, LV_FS_MEMFS_LETTER,
        s_abnormal_info->img_info.data, s_abnormal_info->img_info.size, "baj");
    lv_image_set_src(s_abnormal_info->icon, invalid_popup_icon_img_path.path);
    lv_obj_align(s_abnormal_info->icon, LV_ALIGN_TOP_MID, 0, y_offset);
}

/// 根据当前异常类型选择对应文本
static const sys_abnormal_text_info_t *system_abnormal_ui_get_text_info(ui_manager_context_t *thiz)
{
    ui_proc_system_abnormal_t *proc =
        (ui_proc_system_abnormal_t *)thiz->priv.play_info.user_data;
    if (proc == NULL) {
        return sys_error_infos;
    }

    switch (proc->current_ui_type) {
        case SYSTEM_ABNORMAL_UI_TYPE_BATTERY_TEMP_HIGH:
            return temp_error_infos;
        case SYSTEM_ABNORMAL_UI_TYPE_STORAGE_FAILURE:
        case SYSTEM_ABNORMAL_UI_TYPE_COMMUNICATION_ERROR:
        default:
            return sys_error_infos;
    }
}

static void system_abnormal_ui_update(ui_manager_context_t *thiz)
{
    const sys_abnormal_text_info_t *text_info = system_abnormal_ui_get_text_info(thiz);
    system_abnormal_ui_text_update(thiz, text_info);
    system_abnormal_ui_icon_update(thiz, BASE_RES_ID_WARNING_ICON, SYS_ABNORMAL_WARNING_ICON_Y);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_system_abnormal_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (s_abnormal_info == NULL) {
        s_abnormal_info = (sys_abnormal_disp_info_t *)u_malloz(sizeof(sys_abnormal_disp_info_t));
        EXCEPTION_CHECK_VOID(s_abnormal_info);
    }
    system_abnormal_ui_root_init(thiz);
    s_abnormal_info->force_update = true;
    LOG_UI("system abnormal ui prepare");
}

void ui_display_system_abnormal_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (s_abnormal_info == NULL) {
        return;
    }
    common_ui_img_resource_deinit(thiz, &s_abnormal_info->img_info);
    LVGL_OBJ_DELETE(s_abnormal_info->root);
    u_free(s_abnormal_info);
    s_abnormal_info = NULL;
}

int ui_display_system_abnormal_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && s_abnormal_info && s_abnormal_info->root);
    system_abnormal_ui_update(thiz);
    s_abnormal_info->force_update = false;
    return 0;
}
