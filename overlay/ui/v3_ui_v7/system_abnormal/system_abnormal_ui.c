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
#define SYS_ABNORMAL_TIP_TEXT_COUNT                              (2)
#define IS_SUPPORT_SYS_ABNORMAL_COMP_BG                          (0) /// 是否启用对比背景图

/// 坐标定义（方便调整，按异常类型独立配置）
/// 系统错误（存储故障 / 通信丢失）
#define SYS_ABNORMAL_ICON_Y_SYS_ERROR                            (96)
#define SYS_ABNORMAL_TITLE_X_SYS_ERROR                           (0)
#define SYS_ABNORMAL_TITLE_Y_SYS_ERROR                           (184)
#define SYS_ABNORMAL_CONTENT_X_SYS_ERROR                         (0)
#define SYS_ABNORMAL_CONTENT_Y_SYS_ERROR                         (219)
/// 电池温度过高
#define SYS_ABNORMAL_ICON_Y_TEMP_ERROR                           (108)
#define SYS_ABNORMAL_TITLE_X_TEMP_ERROR                          (0)
#define SYS_ABNORMAL_TITLE_Y_TEMP_ERROR                          (194)
#define SYS_ABNORMAL_CONTENT_X_TEMP_ERROR                        (0)
#define SYS_ABNORMAL_CONTENT_Y_TEMP_ERROR                        (232)
/// 公共颜色
#define SYS_ABNORMAL_TITLE_COLOR                                 (0x240F1B)
#define SYS_ABNORMAL_CONTENT_COLOR                               (0x7C6971)

typedef struct {
    const char *name;
    uint16_t text_x;
    uint16_t text_y;
    const lv_font_t *font;
    uint32_t color;
} sys_abnormal_text_info_t;

typedef struct {
    lv_obj_t * root;
    lv_obj_t * background;
    lv_obj_t * icon;
    lv_obj_t * texts[SYS_ABNORMAL_TIP_TEXT_COUNT];
    res_img_info_t icon_info;
    uint8_t force_update;
    uint8_t rsv[15];
} sys_abnormal_disp_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static sys_abnormal_disp_info_t *s_abnormal_info;

/// 系统错误文本 — 存储故障 / 通信丢失
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
const static sys_abnormal_text_info_t sys_error_texts[SYS_ABNORMAL_TIP_TEXT_COUNT] = {
    { "System error", SYS_ABNORMAL_TITLE_X_SYS_ERROR, SYS_ABNORMAL_TITLE_Y_SYS_ERROR,
        &lv_font_aeonik_soft_19_medium, SYS_ABNORMAL_TITLE_COLOR},
    { "Press and hold knob 10s\nto restart", SYS_ABNORMAL_CONTENT_X_SYS_ERROR, SYS_ABNORMAL_CONTENT_Y_SYS_ERROR,
        &lv_font_aeonik_soft_19_medium, SYS_ABNORMAL_CONTENT_COLOR},
};
#else
const static sys_abnormal_text_info_t sys_error_texts[SYS_ABNORMAL_TIP_TEXT_COUNT] = {
    { "System error", SYS_ABNORMAL_TITLE_X_SYS_ERROR, SYS_ABNORMAL_TITLE_Y_SYS_ERROR,
        &lv_font_aeonik_soft_19_medium, SYS_ABNORMAL_TITLE_COLOR},
    { "Unplug and replug to restart", SYS_ABNORMAL_CONTENT_X_SYS_ERROR, SYS_ABNORMAL_CONTENT_Y_SYS_ERROR,
        &lv_font_aeonik_soft_19_medium, SYS_ABNORMAL_CONTENT_COLOR},
};
#endif

/// 电池温度过高文本
const static sys_abnormal_text_info_t temp_error_texts[SYS_ABNORMAL_TIP_TEXT_COUNT] = {
    { "High battery temperature", SYS_ABNORMAL_TITLE_X_TEMP_ERROR, SYS_ABNORMAL_TITLE_Y_TEMP_ERROR,
        &lv_font_aeonik_soft_19_medium, SYS_ABNORMAL_TITLE_COLOR},
    { "Device suspended", SYS_ABNORMAL_CONTENT_X_TEMP_ERROR, SYS_ABNORMAL_CONTENT_Y_TEMP_ERROR,
        &lv_font_aeonik_soft_19_medium, SYS_ABNORMAL_CONTENT_COLOR},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_SYS_ABNORMAL_COMP_BG
static PSRAM_DATA_LOC uint8_t sys_abnormal_bg_data[] = {
    #include "comparison_png2txt/sys_error_win_360x360.txt"
    //#include "comparison_png2txt/temp_error_win_360x360.txt"
};
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void system_abnormal_ui_root_init(void)
{
    if (s_abnormal_info->root == NULL || !lv_obj_is_valid(s_abnormal_info->root)) {
        s_abnormal_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(s_abnormal_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    // lv_obj_set_style_bg_color(s_abnormal_info->root, lv_color_hex(0xEDE3E5), 0);     /// 设置背景颜色
    lv_obj_set_style_bg_color(s_abnormal_info->root, lv_color_black(), 0);     /// 设置背景颜色
    lv_obj_set_style_bg_opa(s_abnormal_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(s_abnormal_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(s_abnormal_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(s_abnormal_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(s_abnormal_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(s_abnormal_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

static void system_abnormal_ui_icon_update(ui_manager_context_t *thiz, uint16_t y_offset)
{
    if (s_abnormal_info->icon == NULL || !lv_obj_is_valid(s_abnormal_info->icon)) {
        s_abnormal_info->icon = lv_image_create(s_abnormal_info->root);
    }
    ui_theme_img_resource_init(thiz, 0, RES_THEME_TYPE_BASE,
        &s_abnormal_info->icon_info);
    EXCEPTION_CHECK_VOID(s_abnormal_info->icon_info.data);
    lv_fs_path_ex_t icon_img_path;
    lv_fs_make_path_from_buffer(&icon_img_path, LV_FS_MEMFS_LETTER,
        s_abnormal_info->icon_info.data, s_abnormal_info->icon_info.size, "baj");
    lv_image_set_src(s_abnormal_info->icon, icon_img_path.path);
    lv_obj_align(s_abnormal_info->icon, LV_ALIGN_TOP_MID, 0, y_offset);
}

static void system_abnormal_ui_texts_update(ui_manager_context_t *thiz, uint8_t index,
    const sys_abnormal_text_info_t *text_info)
{
    if (s_abnormal_info->texts[index] == NULL || !lv_obj_is_valid(s_abnormal_info->texts[index])) {
        s_abnormal_info->texts[index] = lv_label_create(s_abnormal_info->root);
    }
    lv_label_set_text(s_abnormal_info->texts[index], text_info[index].name);
    lv_obj_set_style_text_color(s_abnormal_info->texts[index], lv_color_hex(text_info[index].color), 0);
    lv_obj_set_style_text_font(s_abnormal_info->texts[index], text_info[index].font, 0);  /// 设置字体
    lv_obj_set_style_text_line_space(s_abnormal_info->texts[index], 4, 0);  /// 设置行间距（像素）
    lv_obj_set_style_text_align(s_abnormal_info->texts[index], LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
    lv_obj_align(s_abnormal_info->texts[index], LV_ALIGN_TOP_MID,
        text_info[index].text_x, text_info[index].text_y);
}

/// 根据当前异常类型选择对应图标Y坐标
static uint16_t system_abnormal_ui_get_icon_y(ui_manager_context_t *thiz)
{
    ui_proc_system_abnormal_t *proc =
        (ui_proc_system_abnormal_t *)thiz->priv.play_info.user_data;
    if (proc == NULL) {
        return SYS_ABNORMAL_ICON_Y_SYS_ERROR;
    }

    switch (proc->current_ui_type) {
        case SYSTEM_ABNORMAL_UI_TYPE_BATTERY_TEMP_HIGH:
            return SYS_ABNORMAL_ICON_Y_TEMP_ERROR;
        case SYSTEM_ABNORMAL_UI_TYPE_STORAGE_FAILURE:
        case SYSTEM_ABNORMAL_UI_TYPE_COMMUNICATION_ERROR:
        default:
            return SYS_ABNORMAL_ICON_Y_SYS_ERROR;
    }
}

/// 根据当前异常类型选择对应文本
static const sys_abnormal_text_info_t *system_abnormal_ui_get_text_info(ui_manager_context_t *thiz)
{
    ui_proc_system_abnormal_t *proc =
        (ui_proc_system_abnormal_t *)thiz->priv.play_info.user_data;
    if (proc == NULL) {
        return sys_error_texts;
    }

    switch (proc->current_ui_type) {
        case SYSTEM_ABNORMAL_UI_TYPE_BATTERY_TEMP_HIGH:
            return temp_error_texts;
        case SYSTEM_ABNORMAL_UI_TYPE_STORAGE_FAILURE:
        case SYSTEM_ABNORMAL_UI_TYPE_COMMUNICATION_ERROR:
        default:
            return sys_error_texts;
    }
}

#if IS_SUPPORT_SYS_ABNORMAL_COMP_BG
static void system_abnormal_ui_bg_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && s_abnormal_info && s_abnormal_info->root);
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (s_abnormal_info->background == NULL || !lv_obj_is_valid(s_abnormal_info->background)) {
        s_abnormal_info->background = lv_image_create(s_abnormal_info->root);
        EXCEPTION_CHECK_VOID(s_abnormal_info->background);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(s_abnormal_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(s_abnormal_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, sys_abnormal_bg_data,
            sizeof(sys_abnormal_bg_data), "png");
        lv_image_set_src(s_abnormal_info->background, (const void *)&bg_img_path);
        lv_obj_align(s_abnormal_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void system_abnormal_ui_bg_update(ui_manager_context_t *thiz)
{
    #if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_COMMON_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
    #endif
    if (s_abnormal_info->background == NULL || !lv_obj_is_valid(s_abnormal_info->background)) {
        s_abnormal_info->background = lv_image_create(s_abnormal_info->root);
        EXCEPTION_CHECK_VOID(s_abnormal_info->background);
        lv_obj_align(s_abnormal_info->background, LV_ALIGN_CENTER, 0, 0);
        #if LV_USE_SNAPSHOT
        lv_image_set_src(s_abnormal_info->background, background_snapshot);
        #else
        lv_obj_add_flag(s_abnormal_info->background, LV_OBJ_FLAG_HIDDEN);
        #endif
    }
}
#endif

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
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    system_abnormal_ui_root_init();
    s_abnormal_info->force_update = true;
}

void ui_display_system_abnormal_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && s_abnormal_info && s_abnormal_info->root);
    ui_img_resource_deinit(thiz, &s_abnormal_info->icon_info);
    LVGL_OBJ_DELETE(s_abnormal_info->root);
    u_free(s_abnormal_info);
    s_abnormal_info = NULL;
}

int ui_display_system_abnormal_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && s_abnormal_info && s_abnormal_info->root);
    system_abnormal_ui_bg_update(thiz);
    if (s_abnormal_info->force_update == true) {
        system_abnormal_ui_icon_update(thiz, system_abnormal_ui_get_icon_y(thiz));
        const sys_abnormal_text_info_t *text_info = system_abnormal_ui_get_text_info(thiz);
        for (uint8_t i = 0; i < SYS_ABNORMAL_TIP_TEXT_COUNT; i++) {
            system_abnormal_ui_texts_update(thiz, i, text_info);
        }
        s_abnormal_info->force_update = false;
    }
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> system abnormal fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
