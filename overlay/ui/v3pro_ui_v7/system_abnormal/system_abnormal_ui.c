/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_system_abnormal.h" // IWYU pragma: keep
#if IS_SUPPORT_SYS_ABNORMAL_RESTART_HOLD
#include "ui_progress_arc_component.h" // IWYU pragma: keep
#endif

/*
* DEFINES
*****************************************************************************************
*/
#define SYS_ABNORMAL_TIP_TEXT_COUNT                              (2)
#define IS_SUPPORT_SYS_ABNORMAL_COMP_BG                          (0) /// 是否启用对比背景图

/// 公共颜色
#define SYS_ABNORMAL_ROOT_BG_COLOR                               (0x000000)
#define SYS_ABNORMAL_TITLE_COLOR                                 (0xFFFFFF)
#define SYS_ABNORMAL_CONTENT_COLOR                               (0x999999)
#if IS_SUPPORT_SYS_ABNORMAL_RESTART_HOLD
/// 长按重启进度（文案 10s，实际 8s 走完）
#define SYS_ABNORMAL_RESTART_HOLD_ARC_DONE_COLOR                 (0xEF4C4E)
#define SYS_ABNORMAL_RESTART_HOLD_DURATION_MS                    (8000)
#define SYS_ABNORMAL_RESTART_HOLD_COUNTDOWN_MAX                  (10)
#endif

typedef struct {
    const char *name;
    uint16_t text_y;
    const lv_font_t *font;
    uint32_t color;
    int16_t line_space;
} sys_abnormal_text_info_t;

typedef struct {
    lv_obj_t * root;
    lv_obj_t * background;
    lv_obj_t * icon;
    lv_obj_t * texts[SYS_ABNORMAL_TIP_TEXT_COUNT];
#if IS_SUPPORT_SYS_ABNORMAL_RESTART_HOLD
    ui_progress_arc_info_t arc;
    uint32_t start_time;
    uint8_t force_update;
    uint8_t holding;
    uint8_t completed;
    uint8_t rsv[13];
#else
    uint8_t force_update;
    uint8_t rsv[15];
#endif
} sys_abnormal_disp_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static sys_abnormal_disp_info_t *s_abnormal_info;

/// 系统错误文本 — 存储故障 / 通信丢失
const static sys_abnormal_text_info_t sys_error_texts[SYS_ABNORMAL_TIP_TEXT_COUNT] = {
    { SYS_ERROR_TITLE_TEXT, SYS_ABNORMAL_TITLE_Y_SYS_ERROR,
        UI_FONT_TITLE, SYS_ABNORMAL_TITLE_COLOR, 0 },
    { SYS_ERROR_REPLUG_TEXT, SYS_ABNORMAL_CONTENT_Y_SYS_ERROR,
        UI_FONT_TITLE, SYS_ABNORMAL_CONTENT_COLOR, 8},
};

/// 电池温度过高文本
const static sys_abnormal_text_info_t temp_error_texts[SYS_ABNORMAL_TIP_TEXT_COUNT] = {
    { TEMP_ERROR_TITLE_TEXT, SYS_ABNORMAL_TITLE_Y_TEMP_ERROR,
        UI_FONT_TITLE, SYS_ABNORMAL_TITLE_COLOR, 8 },
    { TEMP_ERROR_CONTENT_TEXT, SYS_ABNORMAL_CONTENT_Y_TEMP_ERROR,
        UI_FONT_TITLE, SYS_ABNORMAL_CONTENT_COLOR, 0 },
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
    lv_obj_set_size(s_abnormal_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(s_abnormal_info->root, lv_color_hex(SYS_ABNORMAL_ROOT_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(s_abnormal_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_abnormal_info->root, 0, 0);
    lv_obj_set_style_pad_all(s_abnormal_info->root, 0, 0);
    lv_obj_align(s_abnormal_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(s_abnormal_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(s_abnormal_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static uint16_t system_abnormal_ui_get_icon_y(uint16_t system_abnormal_type)
{
    switch (system_abnormal_type) {
        case SYS_ABNORMAL_BATTERY_TEMP_HIGH:
            return SYS_ABNORMAL_ICON_Y_TEMP_ERROR;
        case SYS_ABNORMAL_STORAGE_FAILURE:
        case SYS_ABNORMAL_COMMUNICATION_ERROR:
        default:
            return SYS_ABNORMAL_ICON_Y_SYS_ERROR;
    }
}

static const sys_abnormal_text_info_t *system_abnormal_ui_get_text_info(uint16_t system_abnormal_type)
{
    switch (system_abnormal_type) {
        case SYS_ABNORMAL_BATTERY_TEMP_HIGH:
            return temp_error_texts;
        case SYS_ABNORMAL_STORAGE_FAILURE:
        case SYS_ABNORMAL_COMMUNICATION_ERROR:
        default:
            return sys_error_texts;
    }
}

static void system_abnormal_ui_icon_update(ui_manager_context_t *thiz, uint16_t system_abnormal_type)
{
    if (s_abnormal_info->icon == NULL || !lv_obj_is_valid(s_abnormal_info->icon)) {
        s_abnormal_info->icon = lv_image_create(s_abnormal_info->root);
        EXCEPTION_CHECK_VOID(s_abnormal_info->icon);
    }
    ui_img_baj_theme_set_src(thiz, s_abnormal_info->icon,
        BASE_RES_ID_ERROR_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(s_abnormal_info->icon, LV_ALIGN_TOP_MID, 0,
        system_abnormal_ui_get_icon_y(system_abnormal_type));
}

static void system_abnormal_ui_texts_update(ui_manager_context_t *thiz, uint16_t system_abnormal_type)
{
    const sys_abnormal_text_info_t *text_info = system_abnormal_ui_get_text_info(system_abnormal_type);

    for (uint8_t i = 0; i < SYS_ABNORMAL_TIP_TEXT_COUNT; i++) {
        if (s_abnormal_info->texts[i] == NULL || !lv_obj_is_valid(s_abnormal_info->texts[i])) {
            s_abnormal_info->texts[i] = lv_label_create(s_abnormal_info->root);
            EXCEPTION_CHECK_VOID(s_abnormal_info->texts[i]);
        }
        lv_label_set_text(s_abnormal_info->texts[i], text_info[i].name);
        lv_obj_set_style_text_color(s_abnormal_info->texts[i], lv_color_hex(text_info[i].color), 0);
        lv_obj_set_style_text_font(s_abnormal_info->texts[i], text_info[i].font, 0);
        lv_obj_set_style_text_line_space(s_abnormal_info->texts[i], text_info[i].line_space, 0);
        lv_obj_set_style_text_align(s_abnormal_info->texts[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(s_abnormal_info->texts[i], LV_ALIGN_TOP_MID, 0, text_info[i].text_y);
    }
}

#if IS_SUPPORT_SYS_ABNORMAL_RESTART_HOLD
static void system_abnormal_ui_holding_sync(ui_manager_context_t *thiz)
{
    ui_proc_system_abnormal_t *proc =
        (ui_proc_system_abnormal_t *)thiz->priv.play_info.user_data;
    uint8_t holding = 0;

    if (proc != NULL) {
        holding = proc->restart_hold_holding;
    }
    if (holding && s_abnormal_info->holding == false) {
        s_abnormal_info->start_time = get_timestamp();
        s_abnormal_info->completed = false;
        ui_progress_arc_reset(&s_abnormal_info->arc);
    } else if (holding == false && s_abnormal_info->holding) {
        s_abnormal_info->completed = false;
        ui_progress_arc_reset(&s_abnormal_info->arc);
    }
    s_abnormal_info->holding = holding;
}

static void system_abnormal_ui_arc_update(void)
{
    ui_progress_arc_style_t style;

    if (s_abnormal_info->holding == false) {
        ui_progress_arc_hide(&s_abnormal_info->arc);
        ui_progress_arc_countdown_hide(&s_abnormal_info->arc);
        return;
    }

    ui_progress_arc_style_init(&style, SYS_ABNORMAL_RESTART_HOLD_ARC_DONE_COLOR);
    style.show_countdown = true;
    ui_progress_arc_time_update(&s_abnormal_info->arc, s_abnormal_info->root, &style,
        s_abnormal_info->start_time, SYS_ABNORMAL_RESTART_HOLD_DURATION_MS,
        SYS_ABNORMAL_RESTART_HOLD_COUNTDOWN_MAX, s_abnormal_info->force_update);
}
#endif

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
#if IS_SUPPORT_SYS_ABNORMAL_RESTART_HOLD
    s_abnormal_info->holding = false;
    s_abnormal_info->completed = false;
    ui_progress_arc_reset(&s_abnormal_info->arc);
#endif
}

void ui_display_system_abnormal_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && s_abnormal_info && s_abnormal_info->root);
    ui_img_baj_release_decoded_buf(s_abnormal_info->icon);
    LVGL_OBJ_DELETE(s_abnormal_info->root);
    u_free(s_abnormal_info);
    s_abnormal_info = NULL;
}

int ui_display_system_abnormal_process(ui_manager_context_t *thiz, uint16_t system_abnormal_type)
{
    EXCEPTION_CHECK_INT(thiz && s_abnormal_info && s_abnormal_info->root);
#if IS_SUPPORT_SYS_ABNORMAL_RESTART_HOLD
    system_abnormal_ui_holding_sync(thiz);
#endif
    system_abnormal_ui_bg_update(thiz);
    if (s_abnormal_info->force_update == true) {
        system_abnormal_ui_icon_update(thiz, system_abnormal_type);
        system_abnormal_ui_texts_update(thiz, system_abnormal_type);
#if !IS_SUPPORT_SYS_ABNORMAL_RESTART_HOLD
        s_abnormal_info->force_update = false;
#endif
    }
#if IS_SUPPORT_SYS_ABNORMAL_RESTART_HOLD
    system_abnormal_ui_arc_update();
    s_abnormal_info->force_update = false;

    if (s_abnormal_info->holding && s_abnormal_info->completed == false &&
        get_timestamp_ms(s_abnormal_info->start_time) >= SYS_ABNORMAL_RESTART_HOLD_DURATION_MS) {
        s_abnormal_info->completed = true;
        if (thiz->priv.play_info.progress_cb) {
            thiz->priv.play_info.progress_cb(thiz, 0, ANI_POPUP_DISPLAY_FINISH);
        }
    }
#endif

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> system abnormal fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
