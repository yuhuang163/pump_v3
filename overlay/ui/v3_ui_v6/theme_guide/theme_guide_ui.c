/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define THEME_GUIDE_OPTION_NUM                          (2)
#define THEME_GUIDE_SELECT_SIMPLE                       (0)
#define THEME_GUIDE_SELECT_IP                           (1)

#define THEME_GUIDE_TITLE_COLOR                         (0xB25962)
#define THEME_GUIDE_DESC_COLOR                          (0xC5757C)
#define THEME_GUIDE_LABEL_SELECTED_COLOR                (0xB25962)
#define THEME_GUIDE_LABEL_UNSELECTED_COLOR              (0xC5757C)

#define THEME_GUIDE_TITLE_Y                             (51)
#define THEME_GUIDE_DESC_Y                              (81)
#define THEME_GUIDE_PREVIEW_Y                           (98)
#define THEME_GUIDE_PREVIEW_LEFT_X                      (100)
#define THEME_GUIDE_PREVIEW_RIGHT_X                   (260)
#define THEME_GUIDE_RADIO_Y                             (248)
#define THEME_GUIDE_RADIO_LEFT_X                        (72)
#define THEME_GUIDE_RADIO_RIGHT_X                       (232)
#define THEME_GUIDE_LABEL_LEFT_X                        (100)
#define THEME_GUIDE_LABEL_RIGHT_X                       (260)
#define THEME_GUIDE_LABEL_Y                             (252)

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
    lv_obj_t *preview[THEME_GUIDE_OPTION_NUM];
    lv_obj_t *radio[THEME_GUIDE_OPTION_NUM];
    lv_obj_t *label[THEME_GUIDE_OPTION_NUM];
    uint8_t selected_index;
    uint8_t force_update;
    uint8_t rsv[14];
} theme_guide_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static theme_guide_ui_info_t *theme_info = NULL;

static const theme_guide_text_cfg_t theme_guide_texts[] = {
    {
        .text = "System settings",
        .font = &lv_font_aeonik_soft_20_medium,
        .color = THEME_GUIDE_TITLE_COLOR,
        .y = THEME_GUIDE_TITLE_Y,
        .line_space = 0,
    },
    {
        .text = "Turn the knob to select style",
        .font = &lv_font_aeonik_soft_16_medium,
        .color = THEME_GUIDE_DESC_COLOR,
        .y = THEME_GUIDE_DESC_Y,
        .line_space = 0,
    },
};

static const char *theme_guide_option_labels[THEME_GUIDE_OPTION_NUM] = {
    "Simple",
    "IP",
};

static const uint16_t theme_guide_preview_res_ids[THEME_GUIDE_OPTION_NUM] = {
    BASE_RES_ID_SYS_THEME_IMG,
    BASE_RES_ID_IP_THEME_IMG,
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

static void theme_guide_text_update(void)
{
    if (theme_info->force_update == false) {
        return;
    }

    theme_guide_label_update(&theme_info->title, &theme_guide_texts[0]);
    theme_guide_label_update(&theme_info->desc, &theme_guide_texts[1]);
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
        ui_img_baj_theme_set_src(thiz, theme_info->preview[i], theme_guide_preview_res_ids[i],
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        lv_obj_align(theme_info->preview[i], LV_ALIGN_TOP_MID, preview_x[i] - 180, THEME_GUIDE_PREVIEW_Y);
    }
}

static void theme_guide_option_label_update(uint8_t index)
{
    bool selected = (theme_info->selected_index == index);

    if (theme_info->label[index] == NULL || !lv_obj_is_valid(theme_info->label[index])) {
        theme_info->label[index] = lv_label_create(theme_info->root);
        EXCEPTION_CHECK_VOID(theme_info->label[index]);
        lv_label_set_text(theme_info->label[index], theme_guide_option_labels[index]);
        lv_obj_set_style_text_font(theme_info->label[index], &lv_font_aeonik_soft_16_medium, 0);
        lv_obj_set_style_text_align(theme_info->label[index], LV_TEXT_ALIGN_LEFT, 0);
    }

    lv_obj_set_style_text_color(theme_info->label[index],
        lv_color_hex(selected ? THEME_GUIDE_LABEL_SELECTED_COLOR : THEME_GUIDE_LABEL_UNSELECTED_COLOR), 0);
    lv_obj_set_style_text_opa(theme_info->label[index], selected ? LV_OPA_COVER : LV_OPA_70, 0);
    lv_obj_align(theme_info->label[index], LV_ALIGN_TOP_MID,
        (index == 0) ? (THEME_GUIDE_LABEL_LEFT_X - 180) : (THEME_GUIDE_LABEL_RIGHT_X - 180),
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
    lv_obj_align(theme_info->radio[index], LV_ALIGN_TOP_MID, radio_x[index] - 180, THEME_GUIDE_RADIO_Y);
    theme_guide_option_label_update(index);
}

static void theme_guide_selection_update(ui_manager_context_t *thiz)
{
    static uint8_t last_selected_index = 0xFF;

    if (theme_info->force_update == false && last_selected_index == theme_info->selected_index) {
        return;
    }
    last_selected_index = theme_info->selected_index;

    for (uint8_t i = 0; i < THEME_GUIDE_OPTION_NUM; i++) {
        theme_guide_radio_update(thiz, i);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void theme_guide_ui_set_selected_index(uint8_t index)
{
    if (theme_info == NULL || index >= THEME_GUIDE_OPTION_NUM) {
        return;
    }
    theme_info->selected_index = index;
}

uint8_t theme_guide_ui_get_selected_index(void)
{
    if (theme_info == NULL) {
        return THEME_GUIDE_SELECT_SIMPLE;
    }
    return theme_info->selected_index;
}

res_theme_type_e theme_guide_ui_get_selected_theme_type(void)
{
    return (theme_guide_ui_get_selected_index() == THEME_GUIDE_SELECT_IP) ?
        RES_THEME_TYPE_CUSTOM : RES_THEME_TYPE_BASE;
}

void theme_guide_ui_knob_event(bool is_left)
{
    if (theme_info == NULL) {
        return;
    }

    if (is_left) {
        if (theme_info->selected_index > THEME_GUIDE_SELECT_SIMPLE) {
            theme_info->selected_index--;
        }
    } else if (theme_info->selected_index + 1 < THEME_GUIDE_OPTION_NUM) {
        theme_info->selected_index++;
    }
}

void ui_display_theme_guide_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (theme_info == NULL) {
        theme_info = (theme_guide_ui_info_t *)u_malloz(sizeof(theme_guide_ui_info_t));
        EXCEPTION_CHECK_VOID(theme_info);
    }
    theme_info->selected_index = THEME_GUIDE_SELECT_SIMPLE;
    theme_info->force_update = true;
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    theme_guide_root_init();
    ui_bg_init(thiz);
}

void ui_display_theme_guide_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && theme_info && theme_info->root);
    ui_bg_deinit(thiz);

    for (uint8_t i = 0; i < THEME_GUIDE_OPTION_NUM; i++) {
        ui_img_baj_release_decoded_buf(theme_info->preview[i]);
        ui_img_baj_release_decoded_buf(theme_info->radio[i]);
    }

    LVGL_OBJ_DELETE(theme_info->root);
    u_free(theme_info);
    theme_info = NULL;
}

int ui_display_theme_guide_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && theme_info && theme_info->root);
    theme_guide_bg_update(thiz);
    theme_guide_text_update();
    theme_guide_preview_update(thiz);
    theme_guide_selection_update(thiz);
    theme_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> theme_guide fps: %d, selected: %u",
            thiz->priv.fps_count, theme_info->selected_index);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
