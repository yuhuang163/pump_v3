/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "ui_font_img_tip.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define GUIDE_FINISH_QRCODE_SIZE                        (120)
#define GUIDE_FINISH_QRCODE_PAD                         (8)     /// 二维码四周留白
#define GUIDE_FINISH_QRCODE_INNER_SIZE                  (GUIDE_FINISH_QRCODE_SIZE - GUIDE_FINISH_QRCODE_PAD * 2)
#define GUIDE_FINISH_QRCODE_RADIUS                      (11)
#define GUIDE_FINISH_QRCODE_ALIGN_Y                     (56)
#define GUIDE_FINISH_QRCODE_BG_COLOR                    (0xFFFFFF)
#define GUIDE_FINISH_QRCODE_FG_COLOR                    (0x000000)
#define GUIDE_FINISH_QRCODE_TEXT                        "https://support.momcozy.com/product/bpv-3"

#define GUIDE_FINISH_DESC_Y                             (198)
#define GUIDE_FINISH_DESC_COLOR                         (0xFFFFFF)
#define GUIDE_FINISH_DESC_LINE_SPACE                    (8)
#define GUIDE_FINISH_DESC_TEXT \
    "Tutorial complete. Scan the\nQR code anytime to view it\nagain."

#define GUIDE_FINISH_TIP_H                              (63)
#define GUIDE_FINISH_TIP_Y                              (PANLE_DISPLAY_H - GUIDE_FINISH_TIP_H - 8)
#define GUIDE_FINISH_TIP_BASE_OPA                       (LV_OPA_70)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint16_t guide_finish_bg_res_id_get(uint8_t mode)
{
    switch (mode) {
        case BREAST_PUMP_LACTATION_MODE:
            return BASE_RES_ID_EXPRESSION_MODE_BG;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return BASE_RES_ID_PROGRAM_MODE_BG;
        case BREAST_PUMP_STIMULATION_MODE:
        default:
            return BASE_RES_ID_SIMULATION_MODE_BG;
    }
}

static uint8_t guide_finish_mode_from_info(const guide_base_ui_info_t *info)
{
    uint8_t slot = GUIDE_MODE_SWITCH_SLOT_STIMULATION;

    if (info != NULL && info->ui != NULL) {
        ui_proc_guide_t *proc = (ui_proc_guide_t *)info->ui->priv.play_info.user_data;
        if (proc != NULL && proc->ui_mode_index < GUIDE_MODE_SWITCH_SLOT_COUNT) {
            slot = (uint8_t)proc->ui_mode_index;
        }
    }
    return guide_mode_switch_mode_from_slot(slot);
}

static void guide_finish_bg_detach(guide_base_ui_info_t *info)
{
    const void *src;

    if (info->finish.bg == NULL || !lv_obj_is_valid(info->finish.bg)) {
        return;
    }
    src = lv_image_get_src(info->finish.bg);
    if (src == NULL) {
        return;
    }
    if (get_predecoded_bg(info->finish.shown_bg_id) == src) {
        lv_image_set_src(info->finish.bg, NULL);
        return;
    }
    ui_img_png_release_decoded_buf(info->finish.bg);
}

static void guide_finish_bg_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    const uint16_t res_id = guide_finish_bg_res_id_get(guide_finish_mode_from_info(info));
    lv_draw_buf_t *predecoded;

    EXCEPTION_CHECK_VOID(parent);
    if (info->finish.bg == NULL || !lv_obj_is_valid(info->finish.bg)) {
        info->finish.bg = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->finish.bg);
        lv_obj_move_background(info->finish.bg);
    }

    guide_finish_bg_detach(info);
    predecoded = get_predecoded_bg(res_id);
    if (predecoded != NULL) {
        lv_image_set_src(info->finish.bg, predecoded);
    } else if (info->ui != NULL) {
        ui_img_png_theme_set_src(info->ui, info->finish.bg, res_id,
            RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    }
    lv_obj_align(info->finish.bg, LV_ALIGN_CENTER, 0, 0);
    info->finish.shown_bg_id = res_id;
}

static void guide_finish_qrcode_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    guide_base_finish_info_t *page = &info->finish;

    EXCEPTION_CHECK_VOID(parent);
    snprintf(page->qr_str, sizeof(page->qr_str), "%s", GUIDE_FINISH_QRCODE_TEXT);

    if (page->qr_cont == NULL || !lv_obj_is_valid(page->qr_cont)) {
        page->qr_cont = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(page->qr_cont);
        lv_obj_remove_style_all(page->qr_cont);
        lv_obj_set_size(page->qr_cont, GUIDE_FINISH_QRCODE_SIZE, GUIDE_FINISH_QRCODE_SIZE);
        lv_obj_set_style_bg_color(page->qr_cont, lv_color_hex(GUIDE_FINISH_QRCODE_BG_COLOR), 0);
        lv_obj_set_style_bg_opa(page->qr_cont, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(page->qr_cont, GUIDE_FINISH_QRCODE_RADIUS, 0);
        lv_obj_set_style_clip_corner(page->qr_cont, true, 0);
        lv_obj_set_style_border_width(page->qr_cont, 0, 0);
        lv_obj_set_style_pad_all(page->qr_cont, 0, 0);
        lv_obj_clear_flag(page->qr_cont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(page->qr_cont, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_align(page->qr_cont, LV_ALIGN_TOP_MID, 0, GUIDE_FINISH_QRCODE_ALIGN_Y);
    }

    if (page->qrcode == NULL || !lv_obj_is_valid(page->qrcode)) {
        page->qrcode = lv_qrcode_create(page->qr_cont);
        EXCEPTION_CHECK_VOID(page->qrcode);
    }
    lv_qrcode_set_size(page->qrcode, GUIDE_FINISH_QRCODE_INNER_SIZE);
    lv_qrcode_set_dark_color(page->qrcode, lv_color_hex(GUIDE_FINISH_QRCODE_FG_COLOR));
    lv_qrcode_set_light_color(page->qrcode, lv_color_hex(GUIDE_FINISH_QRCODE_BG_COLOR));
    lv_qrcode_update(page->qrcode, page->qr_str, strlen(page->qr_str));
    lv_obj_center(page->qrcode);
}

static void guide_finish_desc_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    guide_base_finish_info_t *page = &info->finish;

    EXCEPTION_CHECK_VOID(parent);
    if (page->desc != NULL && lv_obj_is_valid(page->desc)) {
        return;
    }

    page->desc = lv_label_create(parent);
    EXCEPTION_CHECK_VOID(page->desc);
    lv_label_set_text(page->desc, GUIDE_FINISH_DESC_TEXT);
    lv_obj_set_style_text_color(page->desc, lv_color_hex(GUIDE_FINISH_DESC_COLOR), 0);
    lv_obj_set_style_text_font(page->desc, &lv_font_aeonik_soft_22_medium, 0);
    lv_obj_set_style_text_line_space(page->desc, GUIDE_FINISH_DESC_LINE_SPACE, 0);
    lv_obj_set_style_text_align(page->desc, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(page->desc, LV_OPA_COVER, 0);
    lv_obj_align(page->desc, LV_ALIGN_TOP_MID, 0, GUIDE_FINISH_DESC_Y);
}

static lv_draw_buf_t *guide_finish_tip_predecode(guide_base_ui_info_t *info)
{
    const ui_img_data_t *tip;

    if (info->finish.tip_buf != NULL) {
        return info->finish.tip_buf;
    }
    tip = ui_font_img_mode_select_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    info->finish.tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return info->finish.tip_buf;
}

static void guide_finish_tip_rebuild(guide_base_ui_info_t *info)
{
    guide_base_finish_info_t *page = &info->finish;
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(parent);
    if (page->tip_img != NULL && lv_obj_is_valid(page->tip_img) &&
        page->tip_shine.mask != NULL && lv_obj_is_valid(page->tip_shine.mask) &&
        page->tip_shine.shine != NULL && lv_obj_is_valid(page->tip_shine.shine)) {
        return;
    }

    predecoded = guide_finish_tip_predecode(info);
    if (page->tip_img == NULL || !lv_obj_is_valid(page->tip_img)) {
        page->tip_img = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(page->tip_img);
        lv_obj_clear_flag(page->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(page->tip_img, predecoded);
            page->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(page->tip_img, GUIDE_FINISH_TIP_BASE_OPA, 0);
        lv_obj_align(page->tip_img, LV_ALIGN_TOP_MID, 0, GUIDE_FINISH_TIP_Y);
    }

    ui_shine_style_init(&style);
    style.mask_y = GUIDE_FINISH_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_prepare_from_tip(&page->tip_shine, &style, predecoded);
    ui_shine_update(&page->tip_shine, parent, &style, page->tip_img, 1);
    lv_obj_move_foreground(page->tip_img);
    ui_shine_show(&page->tip_shine);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_finish_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_finish_bg_detach(info);
    ui_shine_remove(&info->finish.tip_shine);
    if (info->finish.tip_img != NULL && lv_obj_is_valid(info->finish.tip_img)) {
        lv_image_set_src(info->finish.tip_img, NULL);
    }
    if (info->finish.tip_buf != NULL) {
        lv_draw_buf_destroy(info->finish.tip_buf);
        info->finish.tip_buf = NULL;
    }
    memset(&info->finish, 0, sizeof(info->finish));
}

void guide_finish_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_finish_bg_create(info);
    guide_finish_qrcode_create(info);
    guide_finish_desc_create(info);
    guide_finish_tip_rebuild(info);
}

void guide_finish_ui_update(guide_base_ui_info_t *info)
{
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(info);
    if (info->force_update) {
        guide_finish_tip_rebuild(info);
    }
    ui_shine_style_init(&style);
    style.mask_y = GUIDE_FINISH_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_anim_update(&info->finish.tip_shine, &style, info->finish.tip_img, 0);
}
