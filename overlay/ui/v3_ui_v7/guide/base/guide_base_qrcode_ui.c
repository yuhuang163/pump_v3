/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "storage_manager_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define GUIDE_QRCODE_INNER_SIZE                     (GUIDE_QRCODE_SIZE - GUIDE_QRCODE_PAD * 2)
#define GUIDE_QRCODE_RADIUS                         (12)
#define GUIDE_QRCODE_BG_COLOR                       (0xFFFFFF)
#define GUIDE_QRCODE_FG_COLOR                       (0x000000)

#define GUIDE_QRCODE_DESC_COLOR                     (0xFFFFFF)

/** 底部提示图 Y：贴底留边（与 home 模式选择页一致） */
#define GUIDE_QRCODE_TIP_Y                          (PANLE_DISPLAY_H - GUIDE_QRCODE_TIP_H - 8)
#define GUIDE_QRCODE_TIP_BASE_OPA                   (LV_OPA_70)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static char *bind_qr_str[] = {
    "https://qr.momcozy.com/apps/?v=1&type=device&model=V3&sn=",
};

const static char *unbind_qr_str[] = {
    "https://qr.momcozy.com/apps/?v=1&type=device&model=V3&sn=0123456789abcdefghijklmn",
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_qrcode_str_prepare(guide_base_ui_info_t *info)
{
    storage_manager_context_t *storage;

    EXCEPTION_CHECK_VOID(info);
    storage = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (storage != NULL && storage->get_factory_data != NULL) {
        factory_data_store_t *factory_data = storage->get_factory_data();
        if (factory_data != NULL) {
            factory_dev_info_t *dev_info = &factory_data->dev_info;
            unsigned int sn_n = dev_info->sn_len;

            if (sn_n > DEVICE_UNIQUE_SN_LEN) {
                sn_n = DEVICE_UNIQUE_SN_LEN;
            }
            snprintf(info->qrcode.qr_str, sizeof(info->qrcode.qr_str), "%s%.*s", bind_qr_str[0],
                     (int)sn_n, (const char *)dev_info->sn);
            LOG_UI("%s", info->qrcode.qr_str);
            return;         
        }
    }
    snprintf(info->qrcode.qr_str, sizeof(info->qrcode.qr_str), "%s", unbind_qr_str[0]);
    LOG_UI("%s", info->qrcode.qr_str);
}

static void guide_qrcode_widget_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    guide_base_qrcode_info_t *qr = &info->qrcode;

    EXCEPTION_CHECK_VOID(parent);

    if (qr->qr_cont == NULL || !lv_obj_is_valid(qr->qr_cont)) {
        qr->qr_cont = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(qr->qr_cont);
        lv_obj_remove_style_all(qr->qr_cont);
        lv_obj_set_size(qr->qr_cont, GUIDE_QRCODE_SIZE, GUIDE_QRCODE_SIZE);
        lv_obj_set_style_bg_color(qr->qr_cont, lv_color_hex(GUIDE_QRCODE_BG_COLOR), 0);
        lv_obj_set_style_bg_opa(qr->qr_cont, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(qr->qr_cont, GUIDE_QRCODE_RADIUS, 0);
        lv_obj_set_style_clip_corner(qr->qr_cont, true, 0);
        lv_obj_set_style_border_width(qr->qr_cont, 0, 0);
        lv_obj_set_style_pad_all(qr->qr_cont, 0, 0);
        lv_obj_clear_flag(qr->qr_cont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(qr->qr_cont, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_align(qr->qr_cont, LV_ALIGN_TOP_MID, 0, GUIDE_QRCODE_ALIGN_Y);
    }

    if (qr->qrcode == NULL || !lv_obj_is_valid(qr->qrcode)) {
        qr->qrcode = lv_qrcode_create(qr->qr_cont);
        EXCEPTION_CHECK_VOID(qr->qrcode);
    }

    lv_qrcode_set_size(qr->qrcode, GUIDE_QRCODE_INNER_SIZE);
    lv_qrcode_set_dark_color(qr->qrcode, lv_color_hex(GUIDE_QRCODE_FG_COLOR));
    lv_qrcode_set_light_color(qr->qrcode, lv_color_hex(GUIDE_QRCODE_BG_COLOR));
    lv_qrcode_update(qr->qrcode, qr->qr_str, strlen(qr->qr_str));
    lv_obj_center(qr->qrcode);
}

static void guide_qrcode_desc_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    guide_base_qrcode_info_t *qr = &info->qrcode;

    EXCEPTION_CHECK_VOID(parent);
    if (qr->desc != NULL && lv_obj_is_valid(qr->desc)) {
        return;
    }

    qr->desc = lv_label_create(parent);
    EXCEPTION_CHECK_VOID(qr->desc);
    lv_label_set_text(qr->desc, GUIDE_QRCODE_DESC_TEXT);
    lv_obj_set_style_text_color(qr->desc, lv_color_hex(GUIDE_QRCODE_DESC_COLOR), 0);
    lv_obj_set_style_text_font(qr->desc, UI_FONT_BODY_SM, 0);
    lv_obj_set_style_text_line_space(qr->desc, GUIDE_QRCODE_DESC_LINE_SPACE, 0);
    lv_obj_set_style_text_align(qr->desc, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(qr->desc, LV_OPA_COVER, 0);
    lv_obj_align(qr->desc, LV_ALIGN_TOP_MID, 0, GUIDE_QRCODE_DESC_Y);
}

static lv_draw_buf_t *guide_qrcode_tip_predecode(guide_base_ui_info_t *info)
{
    const ui_img_data_t *tip;

    if (info->qrcode.tip_buf != NULL) {
        return info->qrcode.tip_buf;
    }
    tip = ui_font_img_guide_skip_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    info->qrcode.tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return info->qrcode.tip_buf;
}

static void guide_qrcode_tip_rebuild(guide_base_ui_info_t *info)
{
    guide_base_qrcode_info_t *qr = &info->qrcode;
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(parent);
    if (qr->tip_img != NULL && lv_obj_is_valid(qr->tip_img) &&
        qr->tip_shine.mask != NULL && lv_obj_is_valid(qr->tip_shine.mask) &&
        qr->tip_shine.shine != NULL && lv_obj_is_valid(qr->tip_shine.shine)) {
        return;
    }

    predecoded = guide_qrcode_tip_predecode(info);

    if (qr->tip_img == NULL || !lv_obj_is_valid(qr->tip_img)) {
        qr->tip_img = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(qr->tip_img);
        lv_obj_clear_flag(qr->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(qr->tip_img, predecoded);
            qr->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(qr->tip_img, GUIDE_QRCODE_TIP_BASE_OPA, 0);
        lv_obj_align(qr->tip_img, LV_ALIGN_TOP_MID, 0, GUIDE_QRCODE_TIP_Y);
    }

    ui_shine_style_init(&style);
    style.mask_y = GUIDE_QRCODE_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;

    ui_shine_prepare_from_tip(&qr->tip_shine, &style, predecoded);
    ui_shine_update(&qr->tip_shine, parent, &style, qr->tip_img, 1);
    lv_obj_move_foreground(qr->tip_img);
    ui_shine_show(&qr->tip_shine);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_qrcode_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_shine_remove(&info->qrcode.tip_shine);
    if (info->qrcode.tip_img != NULL && lv_obj_is_valid(info->qrcode.tip_img)) {
        lv_image_set_src(info->qrcode.tip_img, NULL);
    }
    if (info->qrcode.tip_buf != NULL) {
        lv_draw_buf_destroy(info->qrcode.tip_buf);
        info->qrcode.tip_buf = NULL;
    }
    /* page 容器删除时一并释放 tip_img / qr_cont 等子对象，这里只清句柄 */
    memset(&info->qrcode, 0, sizeof(info->qrcode));
}

void guide_qrcode_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_qrcode_str_prepare(info);
    guide_qrcode_widget_create(info);
    guide_qrcode_desc_create(info);
    guide_qrcode_tip_rebuild(info);
}

void guide_qrcode_ui_update(guide_base_ui_info_t *info)
{
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(info);
    if (info->force_update) {
        guide_qrcode_tip_rebuild(info);
    }
    ui_shine_style_init(&style);
    style.mask_y = GUIDE_QRCODE_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_anim_update(&info->qrcode.tip_shine, &style, info->qrcode.tip_img, 0);
}
