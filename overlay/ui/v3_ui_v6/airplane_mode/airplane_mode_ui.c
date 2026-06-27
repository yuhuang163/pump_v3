/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_airplane_mode.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_AIRPLANE_MODE_COMP_BG                          (0)
#define AIRPLANE_MODE_TEXT_COUNT                                  (2)
#define AIRPLANE_MODE_ICON_COUNT                                  (1)
#define AIRPLANE_MODE_UNLOCK_BUTTON_ICON_INDEX                    (0)
#define AIRPLANE_MODE_TITLE_COLOR                                 (0x240F1B)
#define AIRPLANE_MODE_TEXT_COLOR                                  (0x7C6971)
#define AIRPLANE_MODE_ICON_Y                                      (220)

typedef struct {
    const char *name;
    uint16_t text_y;
    const lv_font_t *font;
    uint32_t color;
    int16_t line_space;
} airplane_mode_text_info_t;

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_AIRPLANE_MODE_COMP_BG
    lv_obj_t * backgroud;
#endif
    lv_obj_t * icon;
    lv_obj_t * texts[AIRPLANE_MODE_TEXT_COUNT];
    uint8_t force_update;
    uint8_t popup_ui_index;
    uint8_t rsv[14];
} airplane_mode_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_AIRPLANE_MODE_COMP_BG
static PSRAM_DATA_LOC uint8_t complete_tip_bg_data[] = {
    #include "comparison_png2txt/airplane_mode_360x360.txt"
};
#endif

static airplane_mode_display_info_t *airplane_info;

static const airplane_mode_text_info_t airplane_mode_text_infos[AIRPLANE_MODE_TEXT_COUNT] = {
    { "Device locked", 108, 
        &lv_font_aeonik_soft_28_semibold, AIRPLANE_MODE_TITLE_COLOR, 0},
    { "Press & hold for 5\nseconds to unlock.", 146, 
        &lv_font_aeonik_soft_24_medium, AIRPLANE_MODE_TEXT_COLOR, 4},
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void airplane_mode_root_init(void)
{
    if (airplane_info->root == NULL || !lv_obj_is_valid(airplane_info->root)) {
        airplane_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(airplane_info->root, LV_PCT(100), LV_PCT(100));
    // lv_obj_set_style_bg_color(airplane_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_color(airplane_info->root, lv_color_hex(0xEDE3E5), 0);
    lv_obj_set_style_bg_opa(airplane_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(airplane_info->root, 0, 0);
    lv_obj_set_style_pad_all(airplane_info->root, 0, 0);
    lv_obj_align(airplane_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(airplane_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(airplane_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_AIRPLANE_MODE_COMP_BG
static void airplane_mode_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    if (airplane_info->backgroud == NULL || !lv_obj_is_valid(airplane_info->backgroud)) {
        airplane_info->backgroud = lv_image_create(airplane_info->root);
        EXCEPTION_CHECK_VOID(airplane_info->backgroud);
    }

    if (status == 0) {
        lv_obj_add_flag(airplane_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_fs_path_ex_t bg_img_path;
        lv_obj_clear_flag(airplane_info->backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, complete_tip_bg_data,
            sizeof(complete_tip_bg_data), "png");
        lv_image_set_src(airplane_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(airplane_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void airplane_mode_bg_update(ui_manager_context_t *thiz)
{
    ui_bg_update(thiz, airplane_info->root);
}
#endif

static void airplane_mode_button_icon_update(ui_manager_context_t *thiz)
{
    if (airplane_info->force_update == false) {
        return;
    }

    if (airplane_info->icon == NULL ||
        !lv_obj_is_valid(airplane_info->icon)) {
        airplane_info->icon = lv_image_create(airplane_info->root);
        EXCEPTION_CHECK_VOID(airplane_info->icon);
    }
    ui_img_baj_theme_set_src(thiz, airplane_info->icon,
        BASE_RES_ID_AIRPLANE_UNLOCK_BUTTON_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(airplane_info->icon, LV_ALIGN_TOP_MID, 0, AIRPLANE_MODE_ICON_Y);
}

static void airplane_mode_text_label_update(lv_obj_t **label, const airplane_mode_text_info_t *cfg)
{
    if (*label == NULL || !lv_obj_is_valid(*label)) {
        *label = lv_label_create(airplane_info->root);
        EXCEPTION_CHECK_VOID(*label);
    }
    lv_label_set_text(*label, cfg->name);
    lv_obj_set_style_text_color(*label, lv_color_hex(cfg->color), 0);
    lv_obj_set_style_text_font(*label, cfg->font, 0);
    lv_obj_set_style_text_line_space(*label, cfg->line_space, 0);
    lv_obj_set_style_text_align(*label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(*label, LV_ALIGN_TOP_MID, 0, cfg->text_y);
}

static void airplane_mode_texts_update(void)
{
    if (airplane_info->force_update == false) {
        return;
    }

    for (uint8_t i = 0; i < AIRPLANE_MODE_TEXT_COUNT; i++) {
        airplane_mode_text_label_update(&airplane_info->texts[i], &airplane_mode_text_infos[i]);
    }
}

static void airplane_mode_popup_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    ui_popup_update(thiz, popup_type, airplane_info->root, &airplane_info->popup_ui_index);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_airplane_mode_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (airplane_info == NULL) {
        airplane_info = (airplane_mode_display_info_t *)u_malloz(sizeof(airplane_mode_display_info_t));
        EXCEPTION_CHECK_VOID(airplane_info);
    }
    airplane_info->force_update = true;
    ble_ui_init(thiz);
    airplane_mode_root_init();
    ui_bg_init(thiz);
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_airplane_mode_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && airplane_info && airplane_info->root);
    ble_ui_deinit(thiz);
    ui_bg_deinit(thiz);
    ui_img_baj_release_decoded_buf(airplane_info->icon);
    ui_popup_remove(thiz);
    LVGL_OBJ_DELETE(airplane_info->root);
    u_free(airplane_info);
    airplane_info = NULL;
}

int ui_display_airplane_mode_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && airplane_info && airplane_info->root);
    ui_proc_airplane_mode_t *proc = (ui_proc_airplane_mode_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->ble);
    airplane_mode_bg_update(thiz);
    airplane_mode_button_icon_update(thiz);
    ble_ui_update(thiz, airplane_info->root, proc->ble, COMMON_BLE_UI_X, COMMON_BLE_UI_Y, NULL);
    airplane_mode_texts_update();
    airplane_mode_popup_update(thiz, proc->popup_ui_type);
    airplane_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        LOG_UI("===> airplane mode fps=%d, %d, %d",
            thiz->priv.fps_count, proc->popup_ui_type, airplane_info->popup_ui_index);
        thiz->priv.fps_start_time = get_timestamp();
        thiz->priv.fps_count = 0;
    }
    return thiz->priv.fps_count;
}
