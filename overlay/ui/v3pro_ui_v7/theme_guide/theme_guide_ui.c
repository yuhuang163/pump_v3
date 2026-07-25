/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "window_screen_theme_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_THEME_GUIDE_COMP_BG                  (0)
#if IS_SUPPORT_THEME_GUIDE_COMP_BG
#define IS_SUPPORT_THEME_GUIDE_UI_TEST                  (1)
#else
#define IS_SUPPORT_THEME_GUIDE_UI_TEST                  (0)
#endif

#define THEME_GUIDE_OPTION_NUM                          (2)

#define THEME_GUIDE_TITLE_COLOR                         (0xBD4060)
#define THEME_GUIDE_DESC_COLOR                          (0xCD6F87)
#define THEME_GUIDE_LABEL_SELECTED_COLOR                (0xBD4060)
#define THEME_GUIDE_LABEL_UNSELECTED_COLOR              (0xB99BA3)
#define THEME_GUIDE_RING_COLOR                          (0xBD4060)

#define THEME_GUIDE_TITLE_Y                             (51)
#define THEME_GUIDE_DESC_Y                              (87)

#define THEME_GUIDE_PREVIEW_SIZE                        (140)
#define THEME_GUIDE_PREVIEW_Y                           (126)
#define THEME_GUIDE_PREVIEW_LEFT_X                      (32)
#define THEME_GUIDE_PREVIEW_RIGHT_X                     (188)

#define THEME_GUIDE_RING_SIZE                           (148)
#define THEME_GUIDE_RING_LINE_WIDTH                     (2)
#define THEME_GUIDE_RING_OFFSET                         ((THEME_GUIDE_RING_SIZE - THEME_GUIDE_PREVIEW_SIZE) / 2)
#define THEME_GUIDE_RING_Y                              (THEME_GUIDE_PREVIEW_Y - THEME_GUIDE_RING_OFFSET)
#define THEME_GUIDE_RING_LEFT_X                         (THEME_GUIDE_PREVIEW_LEFT_X - THEME_GUIDE_RING_OFFSET)
#define THEME_GUIDE_RING_RIGHT_X                        (THEME_GUIDE_PREVIEW_RIGHT_X - THEME_GUIDE_RING_OFFSET)

#define THEME_GUIDE_CONFIRM_ICON_X                      (144)
#define THEME_GUIDE_CONFIRM_ICON_Y                      (80)

#define THEME_GUIDE_RADIO_Y                             (282)
#define THEME_GUIDE_RADIO_LEFT_X                        (57)
#define THEME_GUIDE_RADIO_RIGHT_X                       (233)

#define THEME_GUIDE_LABEL_LEFT_X                        (91)
#define THEME_GUIDE_LABEL_RIGHT_X                       (267)
#define THEME_GUIDE_LABEL_Y                             (289)

#define THEME_GUIDE_DESC_SELECT_TEXT                    "Turn the knob to select style"
#define THEME_GUIDE_DESC_CONFIRM_TEXT                   "Press          to confirm"

typedef struct {
    const char *text;
    const lv_font_t *font;
    uint32_t color;
    int16_t y;
    int16_t line_space;
} theme_guide_text_cfg_t;

typedef struct {
    lv_obj_t *root;
    lv_obj_t *background;
    lv_obj_t *title;
    lv_obj_t *desc;
    lv_obj_t *confirm_icon;
    lv_obj_t *preview[THEME_GUIDE_OPTION_NUM];
    lv_obj_t *select_ring;
    lv_obj_t *radio[THEME_GUIDE_OPTION_NUM];
    lv_obj_t *label[THEME_GUIDE_OPTION_NUM];
    uint8_t selected_index;
    uint8_t is_operated;
    uint8_t force_update;
    uint8_t popup_ui_index;
    uint8_t rsv[11];
} theme_guide_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_THEME_GUIDE_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t theme_guide_bg_data[] = {
    #include "comparison_png2txt/them_select_360x360.txt"
};
#endif

static theme_guide_ui_info_t *theme_info = NULL;

static const theme_guide_text_cfg_t theme_guide_title_cfg = {
    .text = "System settings",
    .font = &lv_font_aeonik_soft_24_semibold,
    .color = THEME_GUIDE_TITLE_COLOR,
    .y = THEME_GUIDE_TITLE_Y,
    .line_space = 0,
};

static const char *theme_guide_option_labels[THEME_GUIDE_OPTION_NUM] = {
    "Minimal",
    "IP",
};

static const uint16_t theme_guide_preview_res_ids[THEME_GUIDE_OPTION_NUM] = {
    BASE_RES_ID_SYS_THEME_IMG,
    CUSTOM_RES_ID_IP_THEME_IMG,
};

static const res_theme_type_e theme_guide_preview_theme_types[THEME_GUIDE_OPTION_NUM] = {
    RES_THEME_TYPE_BASE,
    RES_THEME_TYPE_CUSTOM,
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void theme_guide_root_init(void)
{
    if (theme_info->root == NULL || !lv_obj_is_valid(theme_info->root)) {
        theme_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(theme_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(theme_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(theme_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(theme_info->root, 0, 0);
    lv_obj_set_style_pad_all(theme_info->root, 0, 0);
    lv_obj_align(theme_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(theme_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(theme_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void theme_guide_bg_update(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_THEME_GUIDE_COMP_BG
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    if (theme_info->background == NULL || !lv_obj_is_valid(theme_info->background)) {
        theme_info->background = lv_image_create(theme_info->root);
        EXCEPTION_CHECK_VOID(theme_info->background);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(theme_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(theme_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, theme_guide_bg_data,
            sizeof(theme_guide_bg_data), "png");
        lv_image_set_src(theme_info->background, (const void *)&bg_img_path);
        lv_obj_align(theme_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
#else
#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_COMMON_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
#endif
    if (theme_info->background == NULL || !lv_obj_is_valid(theme_info->background)) {
        theme_info->background = lv_image_create(theme_info->root);
        EXCEPTION_CHECK_VOID(theme_info->background);
        lv_obj_align(theme_info->background, LV_ALIGN_CENTER, 0, 0);
#if LV_USE_SNAPSHOT
        lv_image_set_src(theme_info->background, background_snapshot);
#else
        lv_obj_add_flag(theme_info->background, LV_OBJ_FLAG_HIDDEN);
#endif
    }
#endif
    (void)thiz;
}

static void theme_guide_label_update(lv_obj_t **label, const theme_guide_text_cfg_t *cfg)
{
    if (*label == NULL || !lv_obj_is_valid(*label)) {
        *label = lv_label_create(theme_info->root);
        EXCEPTION_CHECK_VOID(*label);
    }
    lv_label_set_text(*label, cfg->text);
    lv_obj_set_style_text_align(*label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(*label, cfg->line_space, 0);
    lv_obj_set_style_text_color(*label, lv_color_hex(cfg->color), 0);
    lv_obj_set_style_text_font(*label, cfg->font, 0);
    lv_obj_align(*label, LV_ALIGN_TOP_MID, 0, cfg->y);
}

static void theme_guide_title_update(void)
{
    if (theme_info->force_update == false) {
        return;
    }
    theme_guide_label_update(&theme_info->title, &theme_guide_title_cfg);
}

static void theme_guide_confirm_icon_update(ui_manager_context_t *thiz)
{
    if (theme_info->confirm_icon == NULL || !lv_obj_is_valid(theme_info->confirm_icon)) {
        theme_info->confirm_icon = lv_image_create(theme_info->root);
        EXCEPTION_CHECK_VOID(theme_info->confirm_icon);
        ui_img_baj_theme_set_src(thiz, theme_info->confirm_icon, BASE_RES_ID_GUIDE_START_BUTTON_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        lv_obj_align(theme_info->confirm_icon, LV_ALIGN_TOP_LEFT,
            THEME_GUIDE_CONFIRM_ICON_X, THEME_GUIDE_CONFIRM_ICON_Y);
    }

    if (theme_info->is_operated) {
        lv_obj_clear_flag(theme_info->confirm_icon, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(theme_info->confirm_icon, LV_OBJ_FLAG_HIDDEN);
    }
}

static void theme_guide_desc_update(ui_manager_context_t *thiz)
{
    static uint8_t last_is_operated = 0xFF;
    const char *desc_text = theme_info->is_operated ?
        THEME_GUIDE_DESC_CONFIRM_TEXT : THEME_GUIDE_DESC_SELECT_TEXT;
    theme_guide_text_cfg_t desc_cfg = {
        .text = desc_text,
        .font = &lv_font_aeonik_soft_20_medium,
        .color = THEME_GUIDE_DESC_COLOR,
        .y = THEME_GUIDE_DESC_Y,
        .line_space = 0,
    };

    if (theme_info->force_update == false && last_is_operated == theme_info->is_operated) {
        return;
    }
    last_is_operated = theme_info->is_operated;
    theme_guide_label_update(&theme_info->desc, &desc_cfg);
    theme_guide_confirm_icon_update(thiz);
}

static void theme_guide_select_ring_update(void)
{
    const int16_t ring_x[THEME_GUIDE_OPTION_NUM] = {
        THEME_GUIDE_RING_LEFT_X,
        THEME_GUIDE_RING_RIGHT_X,
    };

    if (theme_info->select_ring == NULL || !lv_obj_is_valid(theme_info->select_ring)) {
        theme_info->select_ring = lv_obj_create(theme_info->root);
        EXCEPTION_CHECK_VOID(theme_info->select_ring);
        lv_obj_remove_style_all(theme_info->select_ring);
        lv_obj_set_size(theme_info->select_ring, THEME_GUIDE_RING_SIZE, THEME_GUIDE_RING_SIZE);
        lv_obj_set_style_radius(theme_info->select_ring, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(theme_info->select_ring, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_color(theme_info->select_ring, lv_color_hex(THEME_GUIDE_RING_COLOR), 0);
        lv_obj_set_style_border_width(theme_info->select_ring, THEME_GUIDE_RING_LINE_WIDTH, 0);
        lv_obj_set_style_border_opa(theme_info->select_ring, LV_OPA_COVER, 0);
        lv_obj_clear_flag(theme_info->select_ring, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    }

    lv_obj_align(theme_info->select_ring, LV_ALIGN_TOP_LEFT,
        ring_x[theme_info->selected_index], THEME_GUIDE_RING_Y);
}

static void theme_guide_preview_update(ui_manager_context_t *thiz)
{
    if (theme_info->force_update == false) {
        return;
    }

    const int16_t preview_x[THEME_GUIDE_OPTION_NUM] = {
        THEME_GUIDE_PREVIEW_LEFT_X,
        THEME_GUIDE_PREVIEW_RIGHT_X,
    };

    for (uint8_t i = 0; i < THEME_GUIDE_OPTION_NUM; i++) {
        if (theme_info->preview[i] == NULL || !lv_obj_is_valid(theme_info->preview[i])) {
            theme_info->preview[i] = lv_image_create(theme_info->root);
            EXCEPTION_CHECK_VOID(theme_info->preview[i]);
        }

        ui_img_png_theme_set_src(thiz, theme_info->preview[i], theme_guide_preview_res_ids[i],
            theme_guide_preview_theme_types[i], UI_IMG_PNG_DECODE_RGBA8888);
        lv_obj_align(theme_info->preview[i], LV_ALIGN_TOP_LEFT, preview_x[i], THEME_GUIDE_PREVIEW_Y);
    }
}

static void theme_guide_option_label_update(uint8_t index)
{
    bool selected = (theme_info->selected_index == index);

    if (theme_info->label[index] == NULL || !lv_obj_is_valid(theme_info->label[index])) {
        theme_info->label[index] = lv_label_create(theme_info->root);
        EXCEPTION_CHECK_VOID(theme_info->label[index]);
        lv_label_set_text(theme_info->label[index], theme_guide_option_labels[index]);
        lv_obj_set_style_text_font(theme_info->label[index], &lv_font_aeonik_soft_18_medium, 0);
        lv_obj_set_style_text_align(theme_info->label[index], LV_TEXT_ALIGN_LEFT, 0);
    }

    lv_obj_set_style_text_color(theme_info->label[index],
        lv_color_hex(selected ? THEME_GUIDE_LABEL_SELECTED_COLOR : THEME_GUIDE_LABEL_UNSELECTED_COLOR), 0);
    lv_obj_set_style_text_opa(theme_info->label[index], selected ? LV_OPA_COVER : LV_OPA_70, 0);
    lv_obj_align(theme_info->label[index], LV_ALIGN_TOP_LEFT,
        (index == 0) ? THEME_GUIDE_LABEL_LEFT_X : THEME_GUIDE_LABEL_RIGHT_X,
        THEME_GUIDE_LABEL_Y);
}

static void theme_guide_radio_update(ui_manager_context_t *thiz, uint8_t index)
{
    uint16_t res_id = (theme_info->selected_index == index) ?
        BASE_RES_ID_THEME_SELECT_ICON : BASE_RES_ID_THEME_UNSELECT_ICON;
    const int16_t radio_x[THEME_GUIDE_OPTION_NUM] = {
        THEME_GUIDE_RADIO_LEFT_X,
        THEME_GUIDE_RADIO_RIGHT_X,
    };

    if (theme_info->radio[index] == NULL || !lv_obj_is_valid(theme_info->radio[index])) {
        theme_info->radio[index] = lv_image_create(theme_info->root);
        EXCEPTION_CHECK_VOID(theme_info->radio[index]);
    }

    ui_img_baj_theme_set_src(thiz, theme_info->radio[index], res_id,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(theme_info->radio[index], LV_ALIGN_TOP_LEFT, radio_x[index], THEME_GUIDE_RADIO_Y);
    theme_guide_option_label_update(index);
}

static void theme_guide_selection_update(ui_manager_context_t *thiz)
{
    static uint8_t last_selected_index = 0xFF;

    if (theme_info->force_update == false && last_selected_index == theme_info->selected_index) {
        return;
    }
    last_selected_index = theme_info->selected_index;

    theme_guide_select_ring_update();
    for (uint8_t i = 0; i < THEME_GUIDE_OPTION_NUM; i++) {
        theme_guide_radio_update(thiz, i);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_theme_guide_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (theme_info == NULL) {
        theme_info = (theme_guide_ui_info_t *)u_malloz(sizeof(theme_guide_ui_info_t));
        EXCEPTION_CHECK_VOID(theme_info);
    }
    theme_info->force_update = true;
    theme_info->popup_ui_index = ID_POPUP_UI_NONE;
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    theme_guide_root_init();
}

void ui_display_theme_guide_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && theme_info && theme_info->root);
    for (uint8_t i = 0; i < THEME_GUIDE_OPTION_NUM; i++) {
        ui_img_png_release_decoded_buf(theme_info->preview[i]);
        ui_img_baj_release_decoded_buf(theme_info->radio[i]);
    }
    ui_img_baj_release_decoded_buf(theme_info->confirm_icon);
    ui_popup_remove(thiz);
    LVGL_OBJ_DELETE(theme_info->root);
    u_free(theme_info);
    theme_info = NULL;
}

int ui_display_theme_guide_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && theme_info && theme_info->root);
    popup_ui_id_e popup_type = ID_POPUP_UI_NONE;

#if IS_SUPPORT_THEME_GUIDE_UI_TEST
    static uint8_t test_selected_index = 0;
    static uint8_t test_is_operated = 0;
    theme_info->selected_index = test_selected_index;
    theme_info->is_operated = test_is_operated;
#else
    ui_proc_theme_guide_t *proc = (ui_proc_theme_guide_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    theme_info->selected_index = proc->selected_index;
    theme_info->is_operated = proc->is_operated;
    popup_type = proc->popup_ui_type;
#endif

    theme_guide_bg_update(thiz);
    theme_guide_title_update();
    theme_guide_desc_update(thiz);
    theme_guide_preview_update(thiz);
    theme_guide_selection_update(thiz);
    theme_info->force_update = false;
    ui_popup_update(thiz, popup_type, theme_info->root, &theme_info->popup_ui_index);
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> theme_guide fps: %d, selected: %u, operated: %u",
            thiz->priv.fps_count, theme_info->selected_index, theme_info->is_operated);
        thiz->priv.fps_count = 0;
        #if IS_SUPPORT_THEME_GUIDE_UI_TEST
        test_selected_index = test_selected_index > 0  ? 0 : 1;
        test_is_operated = (test_is_operated == 0) ? 1 : 0;
        #endif
    }
    return 0;
}
