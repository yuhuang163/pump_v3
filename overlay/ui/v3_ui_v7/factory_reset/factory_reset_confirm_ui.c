/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_factory_reset.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_FACTORY_RESET_COMP_BG                  (0)
#define IS_SUPPORT_RETURN_BTN                             (0)

#define FACTORY_RESET_CONFIRM_FIRST_TEXT_COLOR            (0xA9A7A6)
#define FACTORY_RESET_CONFIRM_TEXT_COLOR                  (0xFFFFFF)

#if IS_SUPPORT_RETURN_BTN
#define FACTORY_RESET_CONFIRM_TITLE_Y                     (123)
#define FACTORY_RESET_CONFIRM_HINT_Y                      (160)
#define FACTORY_RESET_CONFIRM_RETURN_BTN_Y                (244)
#else
#define FACTORY_RESET_CONFIRM_TITLE_Y                     (132)
#define FACTORY_RESET_CONFIRM_HINT_Y                      (168)
#endif

typedef struct {
    const char *text;
    uint32_t color;
    const lv_font_t *font;
    int16_t y;
    int16_t line_space;
} factory_reset_confirm_text_cfg_t;

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_FACTORY_RESET_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * text[2];
    lv_obj_t * return_icon;
    uint8_t force_update;
    uint8_t popup_ui_index;
} ui_factory_reset_confirm_info_t;    

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_FACTORY_RESET_COMP_BG
static PSRAM_DATA_LOC uint8_t factory_reset_confirm_bg_data[] = {
    #include "comparison_png2txt/factory_reset_popup_360x360.txt"
};
#endif

static ui_factory_reset_confirm_info_t *ui_info;
static const factory_reset_confirm_text_cfg_t factory_reset_confirm_texts[] = {
    {
        .text = "Reset settings",
        .color = FACTORY_RESET_CONFIRM_FIRST_TEXT_COLOR,
        .font = &lv_font_aeonik_soft_19_medium,
        .y = FACTORY_RESET_CONFIRM_TITLE_Y,
        .line_space = 0,
    },
    {
        .text = "Press and hold\nthe knob for 3s",
        .color = FACTORY_RESET_CONFIRM_TEXT_COLOR,
        .font = &lv_font_aeonik_soft_19_medium,
        .y = FACTORY_RESET_CONFIRM_HINT_Y,
        .line_space = 6,
    },
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_factory_reset_confirm_root_init(void)
{
    if (ui_info->root == NULL || !lv_obj_is_valid(ui_info->root)) {
        ui_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(ui_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(ui_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(ui_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(ui_info->root, 0, 0);
    lv_obj_set_style_pad_all(ui_info->root, 0, 0);
    lv_obj_align(ui_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(ui_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(ui_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void ui_factory_reset_confirm_label_update(lv_obj_t **label,
    const factory_reset_confirm_text_cfg_t *cfg)
{
    if (*label == NULL || !lv_obj_is_valid(*label)) {
        *label = lv_label_create(ui_info->root);
        EXCEPTION_CHECK_VOID(*label);
    }
    lv_label_set_text(*label, cfg->text);
    lv_obj_set_style_text_align(*label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(*label, 4, 0);
    lv_obj_set_style_text_color(*label, lv_color_hex(cfg->color), 0);
    lv_obj_set_style_text_font(*label, cfg->font, 0);
    lv_obj_set_style_text_line_space(*label, cfg->line_space, 0);
    lv_obj_align(*label, LV_ALIGN_TOP_MID, 0, cfg->y);
}

static void ui_factory_reset_confirm_text_update(void)
{
    if (ui_info->force_update == false) {
        return;
    }

    for (uint8_t i = 0; i < sizeof(factory_reset_confirm_texts) / sizeof(factory_reset_confirm_texts[0]); i++) {
        ui_factory_reset_confirm_label_update(&ui_info->text[i], &factory_reset_confirm_texts[i]);
    }
}

#if IS_SUPPORT_RETURN_BTN
static void ui_factory_reset_confirm_return_btn_update(ui_manager_context_t *thiz)
{
    if (ui_info->force_update == false) {
        return;
    }

    if (ui_info->return_icon == NULL || !lv_obj_is_valid(ui_info->return_icon)) {
        ui_info->return_icon = lv_image_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->return_icon);
    }

    ui_img_baj_theme_set_src(thiz, ui_info->return_icon, UI_IMG_RES_ID_NONE,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(ui_info->return_icon, LV_ALIGN_TOP_MID, 0, FACTORY_RESET_CONFIRM_RETURN_BTN_Y);
}
#endif

#if 0
static void ui_factory_reset_confirm_return_btn_blink_apply(bool blink_visible)
{
    if (ui_info->return_icon == NULL || !lv_obj_is_valid(ui_info->return_icon)) {
        return;
    }
    if (blink_visible) {
        lv_obj_clear_flag(ui_info->return_icon, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(ui_info->return_icon, LV_OBJ_FLAG_HIDDEN);
    }
}
#endif

#if IS_SUPPORT_FACTORY_RESET_COMP_BG
static void ui_factory_reset_confirm_background_update(void)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    if (ui_info->background == NULL || !lv_obj_is_valid(ui_info->background)) {
        ui_info->background = lv_image_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->background);
    }

    if (status == 0) {
        lv_obj_add_flag(ui_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_fs_path_ex_t bg_img_path;
        lv_obj_clear_flag(ui_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, factory_reset_confirm_bg_data,
            sizeof(factory_reset_confirm_bg_data), "png");
        lv_image_set_src(ui_info->background, (const void *)&bg_img_path);
        lv_obj_align(ui_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void ui_factory_reset_confirm_background_update(void)
{
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_factory_reset_confirm_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (ui_info == NULL) {
        ui_info = (ui_factory_reset_confirm_info_t *)u_malloz(sizeof(ui_factory_reset_confirm_info_t));
        EXCEPTION_CHECK_VOID(ui_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_factory_reset_confirm_root_init();
    ui_info->force_update = true;
    ui_info->popup_ui_index = ID_POPUP_UI_NONE;
}

void ui_display_factory_reset_confirm_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && ui_info);
    ui_img_baj_release_decoded_buf(ui_info->return_icon);
    ui_popup_remove(thiz);
    LVGL_OBJ_DELETE(ui_info->root);
    u_free(ui_info);
    ui_info = NULL;
}

int ui_display_factory_reset_confirm_process(ui_manager_context_t *thiz, bool blink_visible)
{
    EXCEPTION_CHECK_INT(thiz && ui_info && ui_info->root);
    ui_proc_factory_reset_t *proc = (ui_proc_factory_reset_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    ui_factory_reset_confirm_background_update();
    ui_factory_reset_confirm_text_update();
#if IS_SUPPORT_RETURN_BTN
    ui_factory_reset_confirm_return_btn_update(thiz);
#endif
    // ui_factory_reset_confirm_return_btn_blink_apply(blink_visible);
    ui_popup_update(thiz, proc->popup_ui_type, ui_info->root, &ui_info->popup_ui_index);
    ui_info->force_update = false;
    thiz->priv.fps_count++;

    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> factory reset confirm fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
