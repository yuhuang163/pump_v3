/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_charge.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_CHARGE_ADAPTER_COMP_BG              (0)
#define CHARGE_ADAPTER_TEXT_COUNT                      (2)
#define CHARGE_ADAPTER_ICON_Y                          (104)
#define CHARGE_ADAPTER_TITLE_Y                         (193)
#define CHARGE_ADAPTER_SUBTITLE_Y                      (232)
#define CHARGE_ADAPTER_TITLE_COLOR                     (0x240F1B)
#define CHARGE_ADAPTER_SUBTITLE_COLOR                  (0x7C6971)
#define CHARGE_ADAPTER_ROOT_BG_COLOR                   (0xEDE3E5)

typedef struct {
    const char *name;
    uint16_t text_y;
    const lv_font_t *font;
    uint32_t color;
    int16_t line_space;
} charge_adapter_text_info_t;

typedef struct {
    lv_obj_t * root;
    lv_obj_t * background;
    lv_obj_t * charge_adapter_icon;
    lv_obj_t * texts[CHARGE_ADAPTER_TEXT_COUNT];
    uint8_t force_update;
    uint8_t rsv[15];
} charge_adapter_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_CHARGE_ADAPTER_COMP_BG
static PSRAM_DATA_LOC uint8_t charge_adapter_bg_data[] = {
    #include "comparison_png2txt/charge_error_360x360.txt"
};
#endif

static charge_adapter_display_info_t *adapter_info;

static const charge_adapter_text_info_t charge_adapter_text_infos[CHARGE_ADAPTER_TEXT_COUNT] = {
    { "Incompatible adapter", CHARGE_ADAPTER_TITLE_Y,
        &lv_font_aeonik_soft_19_medium, CHARGE_ADAPTER_TITLE_COLOR, 0 },
    { "Unable to charge", CHARGE_ADAPTER_SUBTITLE_Y,
        &lv_font_aeonik_soft_19_medium, CHARGE_ADAPTER_SUBTITLE_COLOR, 4 },
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void charge_adapter_ui_root_init(void)
{
    if (adapter_info->root == NULL || !lv_obj_is_valid(adapter_info->root)) {
        adapter_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(adapter_info->root, LV_PCT(100), LV_PCT(100));
    // lv_obj_set_style_bg_color(adapter_info->root, lv_color_hex(CHARGE_ADAPTER_ROOT_BG_COLOR), 0);
    lv_obj_set_style_bg_color(adapter_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(adapter_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(adapter_info->root, 0, 0);
    lv_obj_set_style_pad_all(adapter_info->root, 0, 0);
    lv_obj_align(adapter_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(adapter_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(adapter_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_CHARGE_ADAPTER_COMP_BG
static void charge_adapter_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    if (adapter_info->background == NULL || !lv_obj_is_valid(adapter_info->background)) {
        adapter_info->background = lv_image_create(adapter_info->root);
        EXCEPTION_CHECK_VOID(adapter_info->background);
    }
    if (status == 0) {
        lv_obj_add_flag(adapter_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_fs_path_ex_t bg_img_path;
        lv_obj_clear_flag(adapter_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, charge_adapter_bg_data,
            sizeof(charge_adapter_bg_data), "png");
        lv_image_set_src(adapter_info->background, (const void *)&bg_img_path);
        lv_obj_align(adapter_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void charge_adapter_bg_update(ui_manager_context_t *thiz)
{
    #if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_COMMON_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
    #endif
    if (adapter_info->background == NULL || !lv_obj_is_valid(adapter_info->background)) {
        adapter_info->background = lv_image_create(adapter_info->root);
        EXCEPTION_CHECK_VOID(adapter_info->background);
        lv_obj_align(adapter_info->background, LV_ALIGN_CENTER, 0, 0);
        #if LV_USE_SNAPSHOT
        lv_image_set_src(adapter_info->background, background_snapshot);
        #else
        lv_obj_add_flag(adapter_info->background, LV_OBJ_FLAG_HIDDEN);
        #endif
    }
}
#endif

static void charge_adapter_text_label_update(lv_obj_t **label, const charge_adapter_text_info_t *cfg)
{
    if (*label == NULL || !lv_obj_is_valid(*label)) {
        *label = lv_label_create(adapter_info->root);
        EXCEPTION_CHECK_VOID(*label);
    }
    lv_label_set_text(*label, cfg->name);
    lv_obj_set_style_text_color(*label, lv_color_hex(cfg->color), 0);
    lv_obj_set_style_text_font(*label, cfg->font, 0);
    lv_obj_set_style_text_line_space(*label, cfg->line_space, 0);
    lv_obj_set_style_text_align(*label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(*label, LV_ALIGN_TOP_MID, 0, cfg->text_y);
}

static void charge_adapter_icon_update(ui_manager_context_t *thiz)
{
    if (adapter_info->charge_adapter_icon == NULL ||
        !lv_obj_is_valid(adapter_info->charge_adapter_icon)) {
        adapter_info->charge_adapter_icon = lv_image_create(adapter_info->root);
        EXCEPTION_CHECK_VOID(adapter_info->charge_adapter_icon);
    }
    ui_img_baj_theme_set_src(thiz, adapter_info->charge_adapter_icon,
        UI_IMG_RES_ID_NONE, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(adapter_info->charge_adapter_icon, LV_ALIGN_TOP_MID, 0, CHARGE_ADAPTER_ICON_Y);
}

static void charge_adapter_texts_update(void)
{
    for (uint8_t i = 0; i < CHARGE_ADAPTER_TEXT_COUNT; i++) {
        charge_adapter_text_label_update(&adapter_info->texts[i], &charge_adapter_text_infos[i]);
    }
}

static void charge_adapter_object_update(ui_manager_context_t *thiz)
{
    if (adapter_info->force_update == false) {
        return;
    }
    charge_adapter_icon_update(thiz);
    charge_adapter_texts_update();
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void charge_adapter_base_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (adapter_info == NULL) {
        adapter_info = (charge_adapter_display_info_t *)u_malloz(sizeof(charge_adapter_display_info_t));
        EXCEPTION_CHECK_VOID(adapter_info);
    }
    charge_adapter_ui_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    adapter_info->force_update = true;
}

void charge_adapter_base_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && adapter_info);
    ui_img_baj_release_decoded_buf(adapter_info->charge_adapter_icon);
    LVGL_OBJ_DELETE(adapter_info->root);
    u_free(adapter_info);
    adapter_info = NULL;
}

int charge_adapter_base_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && adapter_info && adapter_info->root);
    charge_adapter_bg_update(thiz);
    charge_adapter_object_update(thiz);
    adapter_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> charge adapter fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
