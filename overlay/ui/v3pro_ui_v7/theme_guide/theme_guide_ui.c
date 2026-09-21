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

#define THEME_GUIDE_TITLE_COLOR                         (0xFFFFFF)
#define THEME_GUIDE_DESC_COLOR                          (0x999999)
#define THEME_GUIDE_LABEL_COLOR                         (0xFFFFFF)
#define THEME_GUIDE_RING_SELECTED_COLOR                 (0xE2637E)
#define THEME_GUIDE_RING_UNSELECTED_COLOR               (0x252525)

#define THEME_GUIDE_RING_SIZE                           (120)
#define THEME_GUIDE_RING_LINE_WIDTH                     (3)

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
    lv_obj_t *ring[THEME_GUIDE_OPTION_NUM];
    lv_obj_t *radio[THEME_GUIDE_OPTION_NUM];
    lv_obj_t *label[THEME_GUIDE_OPTION_NUM];
    uint8_t selected_index;
    uint8_t is_operated;
    uint8_t force_update;
    uint8_t popup_ui_index;
    uint8_t rsv[12];
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

static const uint16_t theme_guide_preview_res_ids[THEME_GUIDE_OPTION_NUM] = {
    CUSTOM_RES_ID_SYS_THEME_ICON,
    CUSTOM_RES_ID_IP_THEME_ICON,
};

static const uint16_t theme_guide_radio_selected_res_id = CUSTOM_RES_ID_THEME_SELECTED_ICON;
static const uint16_t theme_guide_radio_unselected_res_id = CUSTOM_RES_ID_THEME_UNSELECTED_ICON;

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
    theme_guide_text_cfg_t title_cfg = {
        .text = THEME_GUIDE_TITLE_TEXT,
        .font = &lv_font_aeonik_soft_26_medium,
        .color = THEME_GUIDE_TITLE_COLOR,
        .y = THEME_GUIDE_TITLE_Y,
        .line_space = 0,
    };

    if (theme_info->force_update == false) {
        return;
    }
    theme_guide_label_update(&theme_info->title, &title_cfg);
}

static void theme_guide_desc_update(void)
{
    theme_guide_text_cfg_t desc_cfg = {
        .text = THEME_GUIDE_DESC_SELECT_TEXT,
        .font = &lv_font_aeonik_soft_19_medium,
        .color = THEME_GUIDE_DESC_COLOR,
        .y = THEME_GUIDE_DESC_Y,
        .line_space = THEME_GUIDE_DESC_LINE_SPACE,
    };

    if (theme_info->force_update == false) {
        return;
    }
    theme_guide_label_update(&theme_info->desc, &desc_cfg);
}

static void theme_guide_select_ring_update(uint8_t index)
{
    const int16_t ring_x[THEME_GUIDE_OPTION_NUM] = {
        THEME_GUIDE_RING_LEFT_X,
        THEME_GUIDE_RING_RIGHT_X,
    };
    const int16_t ring_y[THEME_GUIDE_OPTION_NUM] = {
        THEME_GUIDE_RING_LEFT_Y,
        THEME_GUIDE_RING_RIGHT_Y,
    };
    bool selected = (theme_info->selected_index == index);
    uint32_t ring_color = selected ? THEME_GUIDE_RING_SELECTED_COLOR : THEME_GUIDE_RING_UNSELECTED_COLOR;

    if (theme_info->ring[index] == NULL || !lv_obj_is_valid(theme_info->ring[index])) {
        theme_info->ring[index] = lv_obj_create(theme_info->root);
        EXCEPTION_CHECK_VOID(theme_info->ring[index]);
        lv_obj_remove_style_all(theme_info->ring[index]);
        lv_obj_set_size(theme_info->ring[index], THEME_GUIDE_RING_SIZE, THEME_GUIDE_RING_SIZE);
        lv_obj_set_style_radius(theme_info->ring[index], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(theme_info->ring[index], LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(theme_info->ring[index], THEME_GUIDE_RING_LINE_WIDTH, 0);
        lv_obj_set_style_border_opa(theme_info->ring[index], LV_OPA_COVER, 0);
        lv_obj_clear_flag(theme_info->ring[index], LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(theme_info->ring[index], LV_ALIGN_TOP_LEFT, ring_x[index], ring_y[index]);
    }

    lv_obj_set_style_border_color(theme_info->ring[index], lv_color_hex(ring_color), 0);
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
    const int16_t preview_y[THEME_GUIDE_OPTION_NUM] = {
        THEME_GUIDE_PREVIEW_LEFT_Y,
        THEME_GUIDE_PREVIEW_RIGHT_Y,
    };

    for (uint8_t i = 0; i < THEME_GUIDE_OPTION_NUM; i++) {
        if (theme_info->preview[i] == NULL || !lv_obj_is_valid(theme_info->preview[i])) {
            theme_info->preview[i] = lv_image_create(theme_info->root);
            EXCEPTION_CHECK_VOID(theme_info->preview[i]);
        }

        ui_img_png_theme_set_src(thiz, theme_info->preview[i], theme_guide_preview_res_ids[i],
            RES_THEME_TYPE_CUSTOM, UI_IMG_PNG_DECODE_RGBA8888);
        lv_obj_align(theme_info->preview[i], LV_ALIGN_TOP_LEFT, preview_x[i], preview_y[i]);
    }
}

static void theme_guide_option_label_update(uint8_t index)
{
    if (theme_info->label[index] == NULL || !lv_obj_is_valid(theme_info->label[index])) {
        theme_info->label[index] = lv_label_create(theme_info->root);
        EXCEPTION_CHECK_VOID(theme_info->label[index]);
        const char *option_labels[THEME_GUIDE_OPTION_NUM] = {
            THEME_GUIDE_OPTION_MINIMAL_TEXT,
            THEME_GUIDE_OPTION_IP_TEXT,
        };

        lv_label_set_text(theme_info->label[index], option_labels[index]);
        lv_obj_set_style_text_font(theme_info->label[index], &lv_font_aeonik_soft_19_medium, 0);
        lv_obj_set_style_text_align(theme_info->label[index], LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_style_text_color(theme_info->label[index], lv_color_hex(THEME_GUIDE_LABEL_COLOR), 0);
        lv_obj_align(theme_info->label[index], LV_ALIGN_TOP_LEFT,
            (index == 0) ? THEME_GUIDE_LABEL_LEFT_X : THEME_GUIDE_LABEL_RIGHT_X,
            THEME_GUIDE_LABEL_Y);
    }
}

static void theme_guide_radio_update(ui_manager_context_t *thiz, uint8_t index)
{
    uint16_t res_id = (theme_info->selected_index == index) ?
        theme_guide_radio_selected_res_id : theme_guide_radio_unselected_res_id;
    const int16_t radio_x[THEME_GUIDE_OPTION_NUM] = {
        THEME_GUIDE_RADIO_LEFT_X,
        THEME_GUIDE_RADIO_RIGHT_X,
    };
    const int16_t radio_y[THEME_GUIDE_OPTION_NUM] = {
        THEME_GUIDE_RADIO_LEFT_Y,
        THEME_GUIDE_RADIO_RIGHT_Y,
    };

    if (theme_info->radio[index] == NULL || !lv_obj_is_valid(theme_info->radio[index])) {
        theme_info->radio[index] = lv_image_create(theme_info->root);
        EXCEPTION_CHECK_VOID(theme_info->radio[index]);
    }

    ui_img_baj_theme_set_src(thiz, theme_info->radio[index], res_id,
        RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(theme_info->radio[index], LV_ALIGN_TOP_LEFT, radio_x[index], radio_y[index]);
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
        theme_guide_select_ring_update(i);
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
    theme_guide_desc_update();
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
