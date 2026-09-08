/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "ui_img_resource.h" // IWYU pragma: keep
#include "ui_img_baj_decode.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "pump_seal_leak_ui.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_SEAL_LEAK_SCREEN_W                         (360)
#define PUMP_SEAL_LEAK_SCALE_FULL                       (256)
#define PUMP_SEAL_LEAK_PROGRESS_FULL                    (256)
#define PUMP_SEAL_LEAK_PAGE_INDEX_INVALID               (0xFF)
#define PUMP_SEAL_LEAK_ANIM_FRAME_INVALID               (0xFFFF)
#define PUMP_SEAL_LEAK_ANIM_ALIGN_MID                   (INT16_MIN)

#ifndef IS_SUPPORT_PUMP_SEAL_LEAK_COMPLETE_ANIM_PREDECODE
#define IS_SUPPORT_PUMP_SEAL_LEAK_COMPLETE_ANIM_PREDECODE (1)
#endif

#define PUMP_SEAL_LEAK_COMPLETE_TIP_H                   (28)
#define PUMP_SEAL_LEAK_COMPLETE_TIP_Y \
    (PANLE_DISPLAY_H - PUMP_SEAL_LEAK_COMPLETE_TIP_H - 8)
#define PUMP_SEAL_LEAK_COMPLETE_TIP_BASE_OPA            (LV_OPA_70)

#define PUMP_SEAL_LEAK_CHECK_BG_COLOR                   (0x000000)
#define PUMP_SEAL_LEAK_TITLE_COLOR                      (0xFFFFFF)
#define PUMP_SEAL_LEAK_DESC_COLOR                       (0x999999)

#define PUMP_SEAL_LEAK_FIRST_ICON_Y                     (120)
#define PUMP_SEAL_LEAK_DIR_LEFT_X                       (122)
#define PUMP_SEAL_LEAK_DIR_RIGHT_X                      (186)

#define PUMP_SEAL_LEAK_CUP_ANIM_X                       (67 + 23)
#define PUMP_SEAL_LEAK_CUP_ANIM_Y                       (87)
#define PUMP_SEAL_LEAK_TUBING_LEFT_ANIM_X               (200)
#define PUMP_SEAL_LEAK_TUBING_LEFT_ANIM_Y               (263)
#define PUMP_SEAL_LEAK_TUBING_RIGHT_ANIM_X              (219)
#define PUMP_SEAL_LEAK_TUBING_RIGHT_ANIM_Y              (260)
#define PUMP_SEAL_LEAK_TUBING_BOTH_ANIM_X               (199)
#define PUMP_SEAL_LEAK_TUBING_BOTH_ANIM_Y               (259)
#define PUMP_SEAL_LEAK_FIT_ANIM_X                       (54)
#define PUMP_SEAL_LEAK_FIT_ANIM_Y                       (64)
#define PUMP_SEAL_LEAK_COMPLETE_ANIM_X                  (68)
#define PUMP_SEAL_LEAK_COMPLETE_ANIM_Y                  (108)

#define PUMP_SEAL_LEAK_ANIM_REST_MS_CUP_FIT            (2000)
#define PUMP_SEAL_LEAK_ANIM_REST_MS_COMPLETE             (1000)
#define PUMP_SEAL_LEAK_ANIM_FRAME_INTERVAL_MS           (1000 / PUMP_ANIMATION_FREQ)

#define PUMP_SEAL_LEAK_DOT_DIAMETER                     (11)
#define PUMP_SEAL_LEAK_DOT_COLOR_ACTIVE                 (0xFFFFFF)
#define PUMP_SEAL_LEAK_DOT_COLOR_INACTIVE               (0x4D4D4D)
#define PUMP_SEAL_LEAK_DOT_COUNT                        (PUMP_SEAL_LEAK_PAGE_MAX)

#ifndef PUMP_SEAL_LEAK_PAGE_TRANSITION_MS
#define PUMP_SEAL_LEAK_PAGE_TRANSITION_MS               (400)
#endif
#ifndef PUMP_SEAL_LEAK_PAGE_TRANSITION_OPA_MIN
#define PUMP_SEAL_LEAK_PAGE_TRANSITION_OPA_MIN          (102)
#endif

typedef struct {
    lv_obj_t *title;
    lv_obj_t *icon;
    lv_obj_t *desc;
    lv_obj_t *anim;
    lv_obj_t *tip_img;
    lv_obj_t *dir_icon[AIR_PRESSURE_NUM];
    lv_draw_buf_t *icon_buf;
    lv_draw_buf_t *dir_buf[AIR_PRESSURE_NUM];
    lv_draw_buf_t *anim_buf;
    lv_draw_buf_t *tip_buf;
    ui_shine_info_t tip_shine;
    res_img_info_t anim_info;
    lv_fs_path_ex_t anim_path;
    uint8_t page_index;
    uint8_t anim_resting;
    uint16_t anim_start;
    uint16_t anim_end;
    uint16_t frame_id;
    uint16_t shown_frame_id;
    uint32_t last_frame_tick_ms;
    uint32_t anim_rest_start_ms;
} pump_seal_leak_page_ctx_t;

typedef struct {
    lv_obj_t *root;
    lv_obj_t *page;
    lv_obj_t *peer_page;
    lv_obj_t *dots[PUMP_SEAL_LEAK_DOT_COUNT];
    uint8_t page_index;
    uint8_t shown_page_index;
    uint8_t transitioning;
    uint8_t enter_instant;
    int8_t slide_dir;
    int16_t progress;
    uint32_t start_ms;
    uint8_t selected_dot;
    uint8_t force_update;
    uint8_t leak_channel[AIR_PRESSURE_NUM];
    ui_manager_context_t *ui;
#if IS_SUPPORT_PUMP_SEAL_RESULT_TIP
    lv_obj_t *wear_result_text;
    uint8_t wear_result[AIR_PRESSURE_NUM];
    uint8_t seal_result;
#endif
} pump_seal_leak_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static pump_seal_leak_ui_info_t *s_leak_info;

/** 4 点沿顶部圆弧排布（参考模式选择 5/6 点内圈） */
static const lv_point_t s_leak_dot_pos[PUMP_SEAL_LEAK_DOT_COUNT] = {
    {141, 8},
    {163, 5},
    {185, 5},
    {207, 8},
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static const char *pump_seal_leak_title_get(uint8_t page_index)
{
    switch (page_index) {
        case PUMP_SEAL_LEAK_PAGE_CHECK_TUBING:
            return PUMP_SEAL_LEAK_TITLE_AIR_SEAL;
        case PUMP_SEAL_LEAK_PAGE_CHECK_CUP:
            return PUMP_SEAL_LEAK_TITLE_CUP;
        case PUMP_SEAL_LEAK_PAGE_CHECK_FIT:
            return PUMP_SEAL_LEAK_TITLE_FIT;
        case PUMP_SEAL_LEAK_PAGE_CHECK_COMPLETE:
            return PUMP_SEAL_LEAK_TITLE_COMPLETE;
        default:
            return "";
    }
}

static const char *pump_seal_leak_desc_get(uint8_t page_index)
{
    switch (page_index) {
        case PUMP_SEAL_LEAK_PAGE_CHECK_TUBING:
            return PUMP_SEAL_LEAK_DESC_TUBING;
        case PUMP_SEAL_LEAK_PAGE_CHECK_CUP:
            return PUMP_SEAL_LEAK_DESC_CUP;
        case PUMP_SEAL_LEAK_PAGE_CHECK_FIT:
            return PUMP_SEAL_LEAK_DESC_FLANGE;
        case PUMP_SEAL_LEAK_PAGE_CHECK_COMPLETE:
            return PUMP_SEAL_LEAK_DESC_COMPLETE;
        default:
            return "";
    }
}

static lv_coord_t pump_seal_leak_title_y_get(uint8_t page_index)
{
    return (page_index == PUMP_SEAL_LEAK_PAGE_CHECK_TUBING) ?
        PUMP_SEAL_LEAK_FIRST_TITLE_Y : PUMP_SEAL_LEAK_TITLE_Y;
}

static lv_coord_t pump_seal_leak_desc_y_get(uint8_t page_index)
{
    switch (page_index) {
        case PUMP_SEAL_LEAK_PAGE_CHECK_TUBING:
            return PUMP_SEAL_LEAK_FIRST_DESC_Y;
        case PUMP_SEAL_LEAK_PAGE_CHECK_COMPLETE:
            return PUMP_SEAL_LEAK_COMPLETE_DESC_Y;
        default:
            return PUMP_SEAL_LEAK_DESC_Y;
    }
}

static int32_t pump_seal_leak_desc_line_space_get(uint8_t page_index)
{
    if (page_index == PUMP_SEAL_LEAK_PAGE_CHECK_CUP ||
        page_index == PUMP_SEAL_LEAK_PAGE_CHECK_FIT) {
        return PUMP_SEAL_LEAK_DESC_LINE_SPACE;
    }
    return 0;
}

static int32_t pump_seal_leak_title_line_space_get(uint8_t page_index)
{
    if (page_index == PUMP_SEAL_LEAK_PAGE_CHECK_COMPLETE) {
        return PUMP_SEAL_LEAK_COMPLETE_TITLE_LINE_SPACE;
    }
    return 0;
}

static uint32_t pump_seal_leak_anim_rest_ms_get(uint8_t page_index)
{
    switch (page_index) {
        case PUMP_SEAL_LEAK_PAGE_CHECK_CUP:
        case PUMP_SEAL_LEAK_PAGE_CHECK_FIT:
            return PUMP_SEAL_LEAK_ANIM_REST_MS_CUP_FIT;
        case PUMP_SEAL_LEAK_PAGE_CHECK_COMPLETE:
            return PUMP_SEAL_LEAK_ANIM_REST_MS_COMPLETE;
        default:
            return 0;
    }
}

static bool pump_seal_leak_anim_cfg_get(const pump_seal_leak_ui_info_t *info,
    uint8_t page_index, uint16_t *start_id, uint16_t *end_id, lv_coord_t *x, lv_coord_t *y)
{
    switch (page_index) {
        case PUMP_SEAL_LEAK_PAGE_CHECK_TUBING: {
            uint8_t left_leak = (info != NULL) ? info->leak_channel[0] : 1;
            uint8_t right_leak = (info != NULL) ? info->leak_channel[1] : 1;

            if (left_leak && !right_leak) {
                *start_id = BASE_RES_ID_CHECK_TUBING_LEFT_ANIM_START;
                *end_id = BASE_RES_ID_CHECK_TUBING_LEFT_ANIM_END;
                *x = PUMP_SEAL_LEAK_TUBING_LEFT_ANIM_X;
                *y = PUMP_SEAL_LEAK_TUBING_LEFT_ANIM_Y;
            } else if (!left_leak && right_leak) {
                *start_id = BASE_RES_ID_CHECK_TUBING_RIGHT_ANIM_START;
                *end_id = BASE_RES_ID_CHECK_TUBING_RIGHT_ANIM_END;
                *x = PUMP_SEAL_LEAK_TUBING_RIGHT_ANIM_X;
                *y = PUMP_SEAL_LEAK_TUBING_RIGHT_ANIM_Y;
            } else {
                *start_id = BASE_RES_ID_CHECK_TUBING_BOTH_ANIM_START;
                *end_id = BASE_RES_ID_CHECK_TUBING_BOTH_ANIM_END;
                *x = PUMP_SEAL_LEAK_TUBING_BOTH_ANIM_X;
                *y = PUMP_SEAL_LEAK_TUBING_BOTH_ANIM_Y;
            }
            return true;
        }
        case PUMP_SEAL_LEAK_PAGE_CHECK_CUP:
            *start_id = BASE_RES_ID_CHECK_CUP_ANIM_START;
            *end_id = BASE_RES_ID_CHECK_CUP_ANIM_END;
            *x = PUMP_SEAL_LEAK_CUP_ANIM_X;
            *y = PUMP_SEAL_LEAK_CUP_ANIM_Y;
            return true;
        case PUMP_SEAL_LEAK_PAGE_CHECK_FIT:
            *start_id = BASE_RES_ID_CHECK_FIT_ANIM_START;
            *end_id = BASE_RES_ID_CHECK_FIT_ANIM_END;
            *x = PUMP_SEAL_LEAK_FIT_ANIM_X;
            *y = PUMP_SEAL_LEAK_FIT_ANIM_Y;
            return true;
        case PUMP_SEAL_LEAK_PAGE_CHECK_COMPLETE:
            *start_id = BASE_RES_ID_CHECK_COMPLETE_ANIM_START;
            *end_id = BASE_RES_ID_CHECK_COMPLETE_ANIM_END;
            *x = PUMP_SEAL_LEAK_COMPLETE_ANIM_X;
            *y = PUMP_SEAL_LEAK_COMPLETE_ANIM_Y;
            return true;
        default:
            return false;
    }
}

static void pump_seal_leak_obj_flag_set(lv_obj_t *obj, bool visible)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    if (visible) {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

static lv_obj_t *pump_seal_leak_label_create(lv_obj_t *parent, const lv_font_t *font,
    uint32_t color, lv_coord_t y, int32_t line_space)
{
    lv_obj_t *label = lv_label_create(parent);

    if (label == NULL) {
        return NULL;
    }
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_letter_space(label, 0, 0);
    if (line_space != 0) {
        lv_obj_set_style_text_line_space(label, line_space, 0);
    }
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, y);
    return label;
}

static lv_draw_buf_t *pump_seal_leak_png_bind(ui_manager_context_t *ui, lv_obj_t *obj,
    uint16_t res_id)
{
    lv_draw_buf_t *buf = ui_img_png_theme_decode_draw_buf(ui, res_id, RES_THEME_TYPE_BASE,
        UI_IMG_PNG_DECODE_RGBA8888);

    if (buf != NULL) {
        lv_image_set_src(obj, buf);
        return buf;
    }
    ui_img_png_theme_set_src(ui, obj, res_id, RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    return NULL;
}

static void pump_seal_leak_png_unbind(lv_obj_t *obj, lv_draw_buf_t **buf)
{
    if (buf != NULL && *buf != NULL) {
        if (obj != NULL && lv_obj_is_valid(obj)) {
            lv_image_set_src(obj, NULL);
        }
        lv_draw_buf_destroy(*buf);
        *buf = NULL;
        return;
    }
    ui_img_png_release_decoded_buf(obj);
}

static lv_draw_buf_t *pump_seal_leak_baj_bind(ui_manager_context_t *ui, lv_obj_t *obj,
    uint16_t res_id)
{
    lv_draw_buf_t *buf = ui_img_baj_theme_decode_draw_buf(ui, res_id, RES_THEME_TYPE_BASE,
        UI_IMG_BAJ_DECODE_RGBA8888);

    if (buf != NULL) {
        lv_image_set_src(obj, buf);
        return buf;
    }
    ui_img_baj_theme_set_src(ui, obj, res_id, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    return NULL;
}

static void pump_seal_leak_baj_unbind(lv_obj_t *obj, lv_draw_buf_t **buf)
{
    if (buf != NULL && *buf != NULL) {
        if (obj != NULL && lv_obj_is_valid(obj)) {
            lv_image_set_src(obj, NULL);
        }
        lv_draw_buf_destroy(*buf);
        *buf = NULL;
        return;
    }
    ui_img_baj_release_decoded_buf(obj);
}

static pump_seal_leak_page_ctx_t *pump_seal_leak_page_ctx_get(lv_obj_t *page)
{
    if (page == NULL || !lv_obj_is_valid(page)) {
        return NULL;
    }
    return (pump_seal_leak_page_ctx_t *)lv_obj_get_user_data(page);
}

static void pump_seal_leak_anim_deinit(pump_seal_leak_ui_info_t *info, pump_seal_leak_page_ctx_t *ctx)
{
    if (ctx == NULL) {
        return;
    }
#if IS_SUPPORT_PUMP_SEAL_LEAK_COMPLETE_ANIM_PREDECODE
    if (ctx->page_index == PUMP_SEAL_LEAK_PAGE_CHECK_COMPLETE) {
        pump_seal_leak_baj_unbind(ctx->anim, &ctx->anim_buf);
        ctx->shown_frame_id = PUMP_SEAL_LEAK_ANIM_FRAME_INVALID;
        return;
    }
#endif
    if (ctx->anim_info.data != NULL && info != NULL && info->ui != NULL) {
        ui_img_resource_deinit(info->ui, &ctx->anim_info);
        memset(&ctx->anim_info, 0, sizeof(ctx->anim_info));
    }
    if (ctx->anim != NULL && lv_obj_is_valid(ctx->anim)) {
        lv_image_set_src(ctx->anim, NULL);
    }
    ctx->shown_frame_id = PUMP_SEAL_LEAK_ANIM_FRAME_INVALID;
}

static void pump_seal_leak_page_ctx_release(pump_seal_leak_ui_info_t *info,
    pump_seal_leak_page_ctx_t *ctx)
{
    uint8_t i;

    if (ctx == NULL) {
        return;
    }
    pump_seal_leak_anim_deinit(info, ctx);
    ui_shine_remove(&ctx->tip_shine);
    pump_seal_leak_png_unbind(ctx->tip_img, &ctx->tip_buf);
    pump_seal_leak_png_unbind(ctx->icon, &ctx->icon_buf);
    for (i = 0; i < AIR_PRESSURE_NUM; i++) {
        pump_seal_leak_baj_unbind(ctx->dir_icon[i], &ctx->dir_buf[i]);
    }
    u_free(ctx);
}

static void pump_seal_leak_page_content_remove(pump_seal_leak_ui_info_t *info, lv_obj_t *page)
{
    pump_seal_leak_page_ctx_t *ctx = pump_seal_leak_page_ctx_get(page);

    if (page == NULL || !lv_obj_is_valid(page)) {
        return;
    }
    lv_obj_set_user_data(page, NULL);
    pump_seal_leak_page_ctx_release(info, ctx);
}

static lv_obj_t *pump_seal_leak_page_container_create(pump_seal_leak_ui_info_t *info)
{
    lv_obj_t *page = lv_obj_create(info->root);

    if (page == NULL) {
        return NULL;
    }
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(page, lv_color_hex(PUMP_SEAL_LEAK_CHECK_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_align(page, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_transform_pivot_x(page, PUMP_SEAL_LEAK_SCREEN_W / 2, 0);
    lv_obj_set_style_transform_pivot_y(page, PUMP_SEAL_LEAK_SCREEN_W / 2, 0);
    lv_obj_set_style_transform_scale(page, PUMP_SEAL_LEAK_SCALE_FULL, 0);
    lv_obj_set_style_opa(page, LV_OPA_COVER, 0);
    return page;
}

static uint16_t pump_seal_leak_tubing_icon_id_get(const pump_seal_leak_ui_info_t *info)
{
    uint8_t left_leak = (info != NULL) ? info->leak_channel[0] : 1;
    uint8_t right_leak = (info != NULL) ? info->leak_channel[1] : 1;

    if (left_leak && !right_leak) {
        return BASE_RES_ID_CHECK_TUBING_LEFT_ICON;
    }
    if (!left_leak && right_leak) {
        return BASE_RES_ID_CHECK_TUBING_RIGHT_ICON;
    }
    return BASE_RES_ID_CHECK_TUBING_BOTH_ICON;
}

static void pump_seal_leak_first_dir_build(pump_seal_leak_ui_info_t *info,
    lv_obj_t *page, pump_seal_leak_page_ctx_t *ctx)
{
    uint16_t left_id;
    uint16_t right_id;

    EXCEPTION_CHECK_VOID(info && page && ctx);
    left_id = info->leak_channel[0] ? BASE_RES_ID_CHECK_SEAL_LEFT_ICON :
        BASE_RES_ID_CHECK_SEAL_LEFT_UNSEL_ICON;
    right_id = info->leak_channel[1] ? BASE_RES_ID_CHECK_SEAL_RIGHT_ICON :
        BASE_RES_ID_CHECK_SEAL_RIGHT_UNSEL_ICON;

    ctx->dir_icon[0] = lv_image_create(page);
    EXCEPTION_CHECK_VOID(ctx->dir_icon[0]);
    ctx->dir_buf[0] = pump_seal_leak_baj_bind(info->ui, ctx->dir_icon[0], left_id);
    lv_obj_align(ctx->dir_icon[0], LV_ALIGN_TOP_LEFT, PUMP_SEAL_LEAK_DIR_LEFT_X,
        PUMP_SEAL_LEAK_DIR_ICON_Y);

    ctx->dir_icon[1] = lv_image_create(page);
    EXCEPTION_CHECK_VOID(ctx->dir_icon[1]);
    ctx->dir_buf[1] = pump_seal_leak_baj_bind(info->ui, ctx->dir_icon[1], right_id);
    lv_obj_align(ctx->dir_icon[1], LV_ALIGN_TOP_LEFT, PUMP_SEAL_LEAK_DIR_RIGHT_X,
        PUMP_SEAL_LEAK_DIR_ICON_Y);
}

static void pump_seal_leak_first_title_build(pump_seal_leak_ui_info_t *info,
    lv_obj_t *page, pump_seal_leak_page_ctx_t *ctx)
{
    EXCEPTION_CHECK_VOID(info && page && ctx);
    ctx->title = pump_seal_leak_label_create(page, UI_FONT_TITLE,
        PUMP_SEAL_LEAK_TITLE_COLOR, PUMP_SEAL_LEAK_FIRST_TITLE_Y, 0);
    EXCEPTION_CHECK_VOID(ctx->title);
    lv_label_set_text(ctx->title, pump_seal_leak_title_get(ctx->page_index));
}

static void pump_seal_leak_anim_frame_show(pump_seal_leak_ui_info_t *info,
    pump_seal_leak_page_ctx_t *ctx, uint16_t frame_id)
{
    EXCEPTION_CHECK_VOID(info && ctx && ctx->anim);
    if (ctx->shown_frame_id == frame_id) {
        return;
    }
#if IS_SUPPORT_PUMP_SEAL_LEAK_COMPLETE_ANIM_PREDECODE
    if (ctx->page_index == PUMP_SEAL_LEAK_PAGE_CHECK_COMPLETE) {
        pump_seal_leak_baj_unbind(ctx->anim, &ctx->anim_buf);
        ctx->anim_buf = pump_seal_leak_baj_bind(info->ui, ctx->anim, frame_id);
        ctx->shown_frame_id = frame_id;
        return;
    }
#endif
    if (ctx->anim_info.data != NULL) {
        ui_img_resource_deinit(info->ui, &ctx->anim_info);
        memset(&ctx->anim_info, 0, sizeof(ctx->anim_info));
    }
    ui_theme_img_resource_init(info->ui, frame_id, RES_THEME_TYPE_BASE, &ctx->anim_info);
    EXCEPTION_CHECK_VOID(ctx->anim_info.data);
    lv_fs_make_path_from_buffer(&ctx->anim_path, LV_FS_MEMFS_LETTER,
        ctx->anim_info.data, ctx->anim_info.size, "baj");
    lv_image_set_src(ctx->anim, (const void *)&ctx->anim_path);
    ctx->shown_frame_id = frame_id;
}

static void pump_seal_leak_anim_build(pump_seal_leak_ui_info_t *info, lv_obj_t *page,
    pump_seal_leak_page_ctx_t *ctx)
{
    uint16_t start_id = 0;
    uint16_t end_id = 0;
    lv_coord_t x = 0;
    lv_coord_t y = 0;

    EXCEPTION_CHECK_VOID(info && page && ctx);
    if (!pump_seal_leak_anim_cfg_get(info, ctx->page_index, &start_id, &end_id, &x, &y)) {
        return;
    }
    ctx->anim = lv_image_create(page);
    EXCEPTION_CHECK_VOID(ctx->anim);
    ctx->anim_start = start_id;
    ctx->anim_end = end_id;
    ctx->frame_id = start_id;
    ctx->shown_frame_id = PUMP_SEAL_LEAK_ANIM_FRAME_INVALID;
    ctx->anim_resting = 0;
    ctx->last_frame_tick_ms = get_timestamp();
    pump_seal_leak_anim_frame_show(info, ctx, ctx->frame_id);
    if (x == PUMP_SEAL_LEAK_ANIM_ALIGN_MID) {
        lv_obj_align(ctx->anim, LV_ALIGN_TOP_MID, 0, y);
    } else {
        lv_obj_align(ctx->anim, LV_ALIGN_TOP_LEFT, x, y);
    }
}

static void pump_seal_leak_complete_tip_style_get(ui_shine_style_t *style)
{
    ui_shine_style_init(style);
    style->mask_y = PUMP_SEAL_LEAK_COMPLETE_TIP_Y;
    style->screen_w = PUMP_SEAL_LEAK_SCREEN_W;
}

static lv_draw_buf_t *pump_seal_leak_complete_tip_predecode(pump_seal_leak_page_ctx_t *ctx)
{
    const ui_img_data_t *tip;

    if (ctx == NULL) {
        return NULL;
    }
    if (ctx->tip_buf != NULL) {
        return ctx->tip_buf;
    }
    tip = ui_font_img_pump_resume_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    ctx->tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return ctx->tip_buf;
}

static void pump_seal_leak_complete_tip_build(lv_obj_t *page, pump_seal_leak_page_ctx_t *ctx)
{
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    EXCEPTION_CHECK_VOID(page && ctx);
    predecoded = pump_seal_leak_complete_tip_predecode(ctx);
    ctx->tip_img = lv_image_create(page);
    EXCEPTION_CHECK_VOID(ctx->tip_img);
    lv_obj_clear_flag(ctx->tip_img, LV_OBJ_FLAG_CLICKABLE);
    if (predecoded != NULL) {
        lv_image_set_src(ctx->tip_img, predecoded);
    }
    lv_obj_set_style_image_opa(ctx->tip_img, PUMP_SEAL_LEAK_COMPLETE_TIP_BASE_OPA, 0);
    lv_obj_align(ctx->tip_img, LV_ALIGN_TOP_MID, 0, PUMP_SEAL_LEAK_COMPLETE_TIP_Y);

    pump_seal_leak_complete_tip_style_get(&style);
    ui_shine_prepare_from_tip(&ctx->tip_shine, &style, predecoded);
    ui_shine_update(&ctx->tip_shine, page, &style, ctx->tip_img, 1);
    lv_obj_move_foreground(ctx->tip_img);
    ui_shine_show(&ctx->tip_shine);
}

static void pump_seal_leak_complete_tip_tick(lv_obj_t *page)
{
    pump_seal_leak_page_ctx_t *ctx = pump_seal_leak_page_ctx_get(page);
    ui_shine_style_t style;

    if (ctx == NULL || ctx->tip_img == NULL || !lv_obj_is_valid(ctx->tip_img)) {
        return;
    }
    pump_seal_leak_complete_tip_style_get(&style);
    ui_shine_anim_update(&ctx->tip_shine, &style, ctx->tip_img, 0);
}

static void pump_seal_leak_check_page_build(pump_seal_leak_ui_info_t *info, lv_obj_t *page,
    uint8_t page_index)
{
    pump_seal_leak_page_ctx_t *ctx = (pump_seal_leak_page_ctx_t *)lv_obj_get_user_data(page);

    EXCEPTION_CHECK_VOID(ctx);
    lv_obj_set_style_bg_color(page, lv_color_hex(PUMP_SEAL_LEAK_CHECK_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);

    if (page_index == PUMP_SEAL_LEAK_PAGE_CHECK_TUBING) {
        ctx->icon = lv_image_create(page);
        EXCEPTION_CHECK_VOID(ctx->icon);
        ctx->icon_buf = pump_seal_leak_png_bind(info->ui, ctx->icon,
            pump_seal_leak_tubing_icon_id_get(info));
        lv_obj_align(ctx->icon, LV_ALIGN_TOP_MID, 0, PUMP_SEAL_LEAK_FIRST_ICON_Y);
    }

    pump_seal_leak_anim_build(info, page, ctx);

    if (page_index == PUMP_SEAL_LEAK_PAGE_CHECK_COMPLETE) {
        pump_seal_leak_complete_tip_build(page, ctx);
    } else {
        ctx->desc = pump_seal_leak_label_create(page, UI_FONT_HINT,
            PUMP_SEAL_LEAK_DESC_COLOR, pump_seal_leak_desc_y_get(page_index),
            pump_seal_leak_desc_line_space_get(page_index));
        EXCEPTION_CHECK_VOID(ctx->desc);
        lv_label_set_text(ctx->desc, pump_seal_leak_desc_get(page_index));
    }

    if (page_index == PUMP_SEAL_LEAK_PAGE_CHECK_TUBING) {
        pump_seal_leak_first_dir_build(info, page, ctx);
        pump_seal_leak_first_title_build(info, page, ctx);
    } else {
        ctx->title = pump_seal_leak_label_create(page, UI_FONT_TITLE,
            PUMP_SEAL_LEAK_TITLE_COLOR, pump_seal_leak_title_y_get(page_index),
            pump_seal_leak_title_line_space_get(page_index));
        EXCEPTION_CHECK_VOID(ctx->title);
        lv_label_set_text(ctx->title, pump_seal_leak_title_get(page_index));
    }
}

static void pump_seal_leak_page_content_build(pump_seal_leak_ui_info_t *info, lv_obj_t *page,
    uint8_t page_index)
{
    pump_seal_leak_page_ctx_t *ctx;

    EXCEPTION_CHECK_VOID(info && page);
    ctx = (pump_seal_leak_page_ctx_t *)u_malloz(sizeof(pump_seal_leak_page_ctx_t));
    EXCEPTION_CHECK_VOID(ctx);
    ctx->page_index = page_index;
    lv_obj_set_user_data(page, ctx);
    pump_seal_leak_check_page_build(info, page, page_index);
}

static void pump_seal_leak_anim_tick(pump_seal_leak_ui_info_t *info, lv_obj_t *page)
{
    pump_seal_leak_page_ctx_t *ctx = pump_seal_leak_page_ctx_get(page);
    uint32_t now;

    if (info == NULL || ctx == NULL || ctx->anim == NULL || !lv_obj_is_valid(ctx->anim)) {
        return;
    }
    now = get_timestamp();

    /// 尾帧停留：第一页不停顿，二/三页 2s，第四页 1s，再从首帧重播
    if (ctx->anim_resting) {
        if ((now - ctx->anim_rest_start_ms) < pump_seal_leak_anim_rest_ms_get(ctx->page_index)) {
            return;
        }
        ctx->anim_resting = 0;
        ctx->frame_id = ctx->anim_start;
        ctx->last_frame_tick_ms = now;
        pump_seal_leak_anim_frame_show(info, ctx, ctx->frame_id);
        if (ctx->frame_id < ctx->anim_end) {
            ctx->frame_id++;
        }
        return;
    }

    if (ctx->last_frame_tick_ms != 0 &&
        (now - ctx->last_frame_tick_ms) < PUMP_SEAL_LEAK_ANIM_FRAME_INTERVAL_MS) {
        return;
    }
    ctx->last_frame_tick_ms = now;
    pump_seal_leak_anim_frame_show(info, ctx, ctx->frame_id);
    if (ctx->frame_id >= ctx->anim_end) {
        if (pump_seal_leak_anim_rest_ms_get(ctx->page_index) == 0) {
            ctx->frame_id = ctx->anim_start;
            return;
        }
        ctx->anim_resting = 1;
        ctx->anim_rest_start_ms = now;
        return;
    }
    ctx->frame_id++;
}

static void pump_seal_leak_page_transform_apply(lv_obj_t *page, int16_t offset_x, lv_opa_t opa)
{
    if (page == NULL || !lv_obj_is_valid(page)) {
        return;
    }
    lv_obj_align(page, LV_ALIGN_CENTER, offset_x, 0);
    lv_obj_set_style_opa(page, opa, 0);
}

static void pump_seal_leak_dots_update(pump_seal_leak_ui_info_t *info, uint8_t selected)
{
    uint8_t i;

    EXCEPTION_CHECK_VOID(info && info->root);
    info->selected_dot = selected;
    for (i = 0; i < PUMP_SEAL_LEAK_DOT_COUNT; i++) {
        if (info->dots[i] == NULL || !lv_obj_is_valid(info->dots[i])) {
            info->dots[i] = lv_obj_create(info->root);
            EXCEPTION_CHECK_VOID(info->dots[i]);
            lv_obj_remove_style_all(info->dots[i]);
            lv_obj_set_size(info->dots[i], PUMP_SEAL_LEAK_DOT_DIAMETER, PUMP_SEAL_LEAK_DOT_DIAMETER);
            lv_obj_set_style_radius(info->dots[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_opa(info->dots[i], LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(info->dots[i], 0, 0);
            lv_obj_clear_flag(info->dots[i], LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(info->dots[i], LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_pos(info->dots[i], s_leak_dot_pos[i].x, s_leak_dot_pos[i].y);
        }
        lv_obj_set_style_bg_color(info->dots[i],
            lv_color_hex(i == selected ? PUMP_SEAL_LEAK_DOT_COLOR_ACTIVE :
                PUMP_SEAL_LEAK_DOT_COLOR_INACTIVE), 0);
        /* 只在 leak root 内置顶，不触及 lv_layer_top 顶层弹窗 */
        lv_obj_move_foreground(info->dots[i]);
        pump_seal_leak_obj_flag_set(info->dots[i], true);
    }
}

static void pump_seal_leak_page_transition_lerp(pump_seal_leak_ui_info_t *info)
{
    const int32_t slide_span = PUMP_SEAL_LEAK_SCREEN_W;
    const int32_t opa_min = PUMP_SEAL_LEAK_PAGE_TRANSITION_OPA_MIN;
    int32_t slide_p = info->progress;
    int16_t out_x;
    int16_t in_x;
    int32_t out_opa_i;
    int32_t in_opa_i;
    int8_t dir = info->slide_dir;

    if (slide_p < 0) {
        slide_p = 0;
    } else if (slide_p > PUMP_SEAL_LEAK_PROGRESS_FULL) {
        slide_p = PUMP_SEAL_LEAK_PROGRESS_FULL;
    }

    if (dir == (int8_t)PUMP_HOME_PAGE_SLIDE_LEFT) {
        out_x = (int16_t)((-slide_span * slide_p) / PUMP_SEAL_LEAK_PROGRESS_FULL);
        in_x = (int16_t)(slide_span + out_x);
    } else {
        out_x = (int16_t)((slide_span * slide_p) / PUMP_SEAL_LEAK_PROGRESS_FULL);
        in_x = (int16_t)(-slide_span + out_x);
    }

    out_opa_i = LV_OPA_COVER -
        ((LV_OPA_COVER - opa_min) * slide_p) / PUMP_SEAL_LEAK_PROGRESS_FULL;
    in_opa_i = opa_min +
        ((LV_OPA_COVER - opa_min) * slide_p) / PUMP_SEAL_LEAK_PROGRESS_FULL;

    pump_seal_leak_page_transform_apply(info->page, out_x, (lv_opa_t)out_opa_i);
    pump_seal_leak_page_transform_apply(info->peer_page, in_x, (lv_opa_t)in_opa_i);
}

static void pump_seal_leak_page_switch_instant(pump_seal_leak_ui_info_t *info, uint8_t page_index)
{
    EXCEPTION_CHECK_VOID(info);
    if (info->page != NULL && lv_obj_is_valid(info->page)) {
        pump_seal_leak_page_content_remove(info, info->page);
        LVGL_OBJ_DELETE(info->page);
    }
    if (info->peer_page != NULL && lv_obj_is_valid(info->peer_page)) {
        pump_seal_leak_page_content_remove(info, info->peer_page);
        LVGL_OBJ_DELETE(info->peer_page);
    }
    info->page = pump_seal_leak_page_container_create(info);
    EXCEPTION_CHECK_VOID(info->page);
    pump_seal_leak_page_content_build(info, info->page, page_index);
    info->page_index = page_index;
    info->shown_page_index = page_index;
    info->transitioning = 0;
    info->slide_dir = PUMP_HOME_PAGE_SLIDE_NONE;
    info->progress = 0;
    pump_seal_leak_dots_update(info, page_index);
}

static void pump_seal_leak_page_transition_finish(pump_seal_leak_ui_info_t *info)
{
    const uint8_t new_index = info->page_index;

    pump_seal_leak_page_content_remove(info, info->page);
    LVGL_OBJ_DELETE(info->page);
    info->page = info->peer_page;
    info->peer_page = NULL;
    info->shown_page_index = new_index;
    info->transitioning = 0;
    info->slide_dir = PUMP_HOME_PAGE_SLIDE_NONE;
    info->progress = 0;
    pump_seal_leak_page_transform_apply(info->page, 0, LV_OPA_COVER);
    pump_seal_leak_dots_update(info, new_index);
}

static void pump_seal_leak_page_transition_tick(pump_seal_leak_ui_info_t *info)
{
    uint32_t elapsed;
    int32_t progress;

    if (!info->transitioning) {
        return;
    }
    elapsed = get_timestamp() - info->start_ms;
    if (elapsed >= PUMP_SEAL_LEAK_PAGE_TRANSITION_MS) {
        progress = PUMP_SEAL_LEAK_PROGRESS_FULL;
    } else {
        progress = (int32_t)((elapsed * PUMP_SEAL_LEAK_PROGRESS_FULL) / PUMP_SEAL_LEAK_PAGE_TRANSITION_MS);
    }
    info->progress = (int16_t)progress;
    pump_seal_leak_page_transition_lerp(info);
    if (progress >= PUMP_SEAL_LEAK_PROGRESS_FULL) {
        pump_seal_leak_page_transition_finish(info);
    }
}

static void pump_seal_leak_page_goto(pump_seal_leak_ui_info_t *info, uint8_t page_index)
{
    pump_home_page_slide_dir_e dir;

    EXCEPTION_CHECK_VOID(info);
    if (info->transitioning) {
        return;
    }
    if (info->page == NULL || !lv_obj_is_valid(info->page)) {
        return;
    }
    if (page_index == info->shown_page_index || page_index >= PUMP_SEAL_LEAK_PAGE_MAX) {
        info->page_index = page_index;
        return;
    }

    dir = (page_index > info->shown_page_index) ?
            PUMP_HOME_PAGE_SLIDE_LEFT : PUMP_HOME_PAGE_SLIDE_RIGHT;
    info->peer_page = pump_seal_leak_page_container_create(info);
    EXCEPTION_CHECK_VOID(info->peer_page);
    pump_seal_leak_page_content_build(info, info->peer_page, page_index);
    info->page_index = page_index;
    info->slide_dir = (int8_t)dir;
    info->transitioning = 1;
    info->progress = 0;
    info->start_ms = get_timestamp();
    pump_seal_leak_page_transition_lerp(info);
    pump_seal_leak_dots_update(info, page_index);
}

static void pump_seal_leak_root_init(void)
{
    if (s_leak_info->root == NULL || !lv_obj_is_valid(s_leak_info->root)) {
        s_leak_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(s_leak_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(s_leak_info->root, lv_color_hex(PUMP_SEAL_LEAK_CHECK_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(s_leak_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_leak_info->root, 0, 0);
    lv_obj_set_style_pad_all(s_leak_info->root, 0, 0);
    lv_obj_align(s_leak_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(s_leak_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(s_leak_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void pump_seal_leak_runtime_sync(pump_seal_leak_ui_info_t *info, ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(info && proc);
    memcpy(info->leak_channel, proc->leak_channel, sizeof(info->leak_channel));
}

#if IS_SUPPORT_PUMP_SEAL_RESULT_TIP
static void pump_seal_leak_wear_result_update(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->wear_result[0] == s_leak_info->wear_result[0] &&
        proc->wear_result[1] == s_leak_info->wear_result[1] &&
        proc->seal_result    == s_leak_info->seal_result) {
        return;
    }

    if (s_leak_info->wear_result_text == NULL || !lv_obj_is_valid(s_leak_info->wear_result_text)) {
        s_leak_info->wear_result_text = lv_label_create(s_leak_info->root);
        lv_obj_set_style_text_color(s_leak_info->wear_result_text, lv_color_white(), 0);
        lv_obj_set_style_text_font(s_leak_info->wear_result_text, UI_FONT_TITLE, 0);
        lv_obj_set_style_text_line_space(s_leak_info->wear_result_text, 2, 0);
        lv_obj_set_style_text_align(s_leak_info->wear_result_text, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(s_leak_info->wear_result_text, LV_ALIGN_TOP_MID, 0, 325);
    }
    char wear_result_text[32];
    sprintf(wear_result_text, "%d-%d-%d", proc->seal_result, proc->wear_result[0], proc->wear_result[1]);
    lv_label_set_text(s_leak_info->wear_result_text, wear_result_text);
    memcpy(s_leak_info->wear_result, proc->wear_result, sizeof(s_leak_info->wear_result));
    s_leak_info->seal_result = proc->seal_result;
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_seal_leak_prepare(ui_manager_context_t *thiz)
{
    ui_proc_pump_t *proc;

    EXCEPTION_CHECK_VOID(thiz);
    if (s_leak_info == NULL) {
        s_leak_info = (pump_seal_leak_ui_info_t *)u_malloz(sizeof(pump_seal_leak_ui_info_t));
        EXCEPTION_CHECK_VOID(s_leak_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    pump_seal_leak_root_init();
    s_leak_info->ui = thiz;
    s_leak_info->force_update = true;
    s_leak_info->enter_instant = 1;
    s_leak_info->shown_page_index = PUMP_SEAL_LEAK_PAGE_INDEX_INVALID;
    proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    pump_seal_leak_runtime_sync(s_leak_info, proc);
    #if IS_SUPPORT_PUMP_SEAL_RESULT_TIP
    s_leak_info->wear_result[0] = 0xFF;
    s_leak_info->wear_result[1] = 0xFF;
    s_leak_info->seal_result = 0xFF;
    #endif
}

void ui_display_pump_seal_leak_remove(ui_manager_context_t *thiz)
{
    uint8_t i;

    EXCEPTION_CHECK_VOID(thiz && s_leak_info && s_leak_info->root);
    if (s_leak_info->page != NULL && lv_obj_is_valid(s_leak_info->page)) {
        pump_seal_leak_page_content_remove(s_leak_info, s_leak_info->page);
        LVGL_OBJ_DELETE(s_leak_info->page);
    }
    if (s_leak_info->peer_page != NULL && lv_obj_is_valid(s_leak_info->peer_page)) {
        pump_seal_leak_page_content_remove(s_leak_info, s_leak_info->peer_page);
        LVGL_OBJ_DELETE(s_leak_info->peer_page);
    }
    for (i = 0; i < PUMP_SEAL_LEAK_DOT_COUNT; i++) {
        LVGL_OBJ_DELETE(s_leak_info->dots[i]);
    }
    LVGL_OBJ_DELETE(s_leak_info->root);
    u_free(s_leak_info);
    s_leak_info = NULL;
}

int ui_display_pump_seal_leak_process(ui_manager_context_t *thiz)
{
    ui_proc_pump_t *proc;
    uint8_t target_index;

    EXCEPTION_CHECK_INT(thiz && s_leak_info && s_leak_info->root);
    proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    pump_seal_leak_runtime_sync(s_leak_info, proc);

    target_index = (uint8_t)proc->seal_leak_page_index;
    if (target_index >= PUMP_SEAL_LEAK_PAGE_MAX) {
        target_index = PUMP_SEAL_LEAK_PAGE_CHECK_TUBING;
    }

    if (s_leak_info->shown_page_index == PUMP_SEAL_LEAK_PAGE_INDEX_INVALID ||
        s_leak_info->enter_instant) {
        if (s_leak_info->shown_page_index != target_index) {
            pump_seal_leak_page_switch_instant(s_leak_info, target_index);
        }
        s_leak_info->enter_instant = 0;
    } else if (!s_leak_info->transitioning &&
               s_leak_info->shown_page_index != target_index) {
        pump_seal_leak_page_goto(s_leak_info, target_index);
    }

    pump_seal_leak_page_transition_tick(s_leak_info);
    pump_seal_leak_anim_tick(s_leak_info, s_leak_info->page);
    pump_seal_leak_anim_tick(s_leak_info, s_leak_info->peer_page);
    pump_seal_leak_complete_tip_tick(s_leak_info->page);
    pump_seal_leak_complete_tip_tick(s_leak_info->peer_page);
    pump_seal_leak_dots_update(s_leak_info, s_leak_info->page_index);
    #if IS_SUPPORT_PUMP_SEAL_RESULT_TIP
    pump_seal_leak_wear_result_update(proc);
    #endif
    s_leak_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump seal leak fps: %d, page:%d", thiz->priv.fps_count,
            proc->seal_leak_page_index);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
