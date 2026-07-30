/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "home_base_ui_internal.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_MODE_PAGE_DOT_DIAMETER                 (11)
#define HOME_MODE_PAGE_DOT_COLOR_ACTIVE             (0xFFFFFF)
#define HOME_MODE_PAGE_DOT_COLOR_INACTIVE           (0x4D4D4D)

#define HOME_MODE_PAGE_ICON_Y                       (100)
#define HOME_MODE_PAGE_TITLE_Y                      (214)
#define HOME_MODE_PAGE_TITLE_COLOR                  (0xFFFFFF)
#define HOME_MODE_PAGE_DURATION_Y                   (246)
#define HOME_MODE_PAGE_DURATION_COLOR               (0x999999)

#define HOME_MODE_PAGE_TIP_W                        (212)
#define HOME_MODE_PAGE_TIP_H                        (50)
/** 底部提示图 Y：贴底留边 */
#define HOME_MODE_PAGE_TIP_Y                        (PANLE_DISPLAY_H - HOME_MODE_PAGE_TIP_H - 8)
#if HOME_BASE_MODE_TIP_SHINE_IS_HARD
/** 提示硬扫光：硬边裁切窗 */
#define HOME_MODE_PAGE_TIP_MASK_W                   (HOME_BASE_MODE_TIP_SHINE_W)
#define HOME_MODE_PAGE_TIP_MASK_H                   (HOME_MODE_PAGE_TIP_H)
#define HOME_MODE_PAGE_TIP_MASK_Y                   (HOME_MODE_PAGE_TIP_Y)
#define HOME_MODE_PAGE_TIP_SHINE_RECOLOR_OPA        (LV_OPA_COVER)
#define HOME_MODE_PAGE_TIP_BASE_OPA                 (LV_OPA_COVER)
#else
/** SOFT / OVERLAY：宽由 HOME_BASE_MODE_TIP_SHINE_W 控制，高贴 tip */
#define HOME_MODE_PAGE_TIP_MASK_W                   (HOME_BASE_MODE_TIP_SHINE_W)
#define HOME_MODE_PAGE_TIP_MASK_H                   (HOME_MODE_PAGE_TIP_H)
#define HOME_MODE_PAGE_TIP_MASK_Y                   (HOME_MODE_PAGE_TIP_Y)
#define HOME_MODE_PAGE_TIP_SHINE_RECOLOR_OPA        (LV_OPA_80)
#define HOME_MODE_PAGE_TIP_BASE_OPA                 (LV_OPA_70)
#endif
#define HOME_MODE_PAGE_TIP_MASK_X_MIN               (-HOME_MODE_PAGE_TIP_MASK_W)
#define HOME_MODE_PAGE_TIP_MASK_X_MAX               (PANLE_DISPLAY_W)
#define HOME_MODE_PAGE_TIP_MASK_PERIOD_MS           (3200)
#define HOME_MODE_PAGE_TIP_SHINE_COLOR              (0xFFFFFF)

#define HOME_MODE_PAGE_SCALE_FULL                   (256)
/** 5 点布局第 3 点 X 居中：(屏宽 - 直径) / 2 */
#define HOME_MODE_PAGE_DOT_X_CENTER \
    ((PANLE_DISPLAY_W - HOME_MODE_PAGE_DOT_DIAMETER) / 2)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
/** 提示文案预解码缓存（本模块持有，页面侧勿 destroy） */
static lv_draw_buf_t *s_mode_select_tip_predecoded = NULL;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_mode_page_obj_scale_set(lv_obj_t *obj, int32_t scale)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    lv_obj_update_layout(obj);
    lv_obj_set_style_transform_pivot_x(obj, lv_obj_get_width(obj) / 2, 0);
    lv_obj_set_style_transform_pivot_y(obj, lv_obj_get_height(obj) / 2, 0);
    lv_obj_set_style_transform_scale(obj, scale, 0);
}

static lv_obj_t *home_mode_page_circle_bg_create(lv_obj_t *parent)
{
    lv_obj_t *circle = lv_obj_create(parent);

    EXCEPTION_CHECK_POINT(circle);
    lv_obj_remove_style_all(circle);
    lv_obj_set_size(circle, HOME_BASE_PAGE_TRANSITION_CIRCLE_SIZE, HOME_BASE_PAGE_TRANSITION_CIRCLE_SIZE);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(circle, lv_color_hex(HOME_BASE_PAGE_TRANSITION_CIRCLE_COLOR), 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(circle, 0, 0);
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(circle, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(circle, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_background(circle);
    return circle;
}

static uint8_t home_mode_page_mode_from_page(uint8_t page_index)
{
    switch (page_index) {
        case HOME_MODE_PAGE_INDEX_STIMULATION_SELECT:
            return BREAST_PUMP_STIMULATION_MODE;
        case HOME_MODE_PAGE_INDEX_EXPRESSION_SELECT:
            return BREAST_PUMP_LACTATION_MODE;
        case HOME_MODE_PAGE_INDEX_MILK_BOOST_SELECT:
            return BREAST_PUMP_MILK_BOOST_MODE;
        case HOME_MODE_PAGE_INDEX_POWER_PUMPING_SELECT:
            return BREAST_PUMP_PP_MILK_TRACK_MODE;
        case HOME_MODE_PAGE_INDEX_CUSTOMIZE_SELECT:
            return BREAST_PUMP_CUSTOM_MODE;
        default:
            return BREAST_PUMP_STIMULATION_MODE;
    }
}

static uint16_t home_mode_page_icon_res_id_get(uint8_t mode)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return BASE_RES_ID_SIMULATION_ICON;
        case BREAST_PUMP_LACTATION_MODE:
            return BASE_RES_ID_EXPRESSION_ICON;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
            return BASE_RES_ID_PROGRAM_ICON;
        default:
            return BASE_RES_ID_SIMULATION_ICON;
    }
}

static void home_mode_page_title_get(home_base_ui_info_t *info, uint8_t mode, char *buf, size_t buf_len)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            snprintf(buf, buf_len, "Stimulation");
            break;
        case BREAST_PUMP_LACTATION_MODE:
            snprintf(buf, buf_len, "Expression");
            break;
        case BREAST_PUMP_MILK_BOOST_MODE:
            snprintf(buf, buf_len, "Milk Boost");
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            snprintf(buf, buf_len, "Power Pumping");
            break;
        case BREAST_PUMP_CUSTOM_MODE:
            snprintf(buf, buf_len, "Customized %s", info->mode.custom_id_str);
            break;
        default:
            snprintf(buf, buf_len, "");
            break;
    }
}

static bool home_mode_page_is_available(const home_base_ui_info_t *info, uint8_t page_index)
{
    if (page_index >= HOME_PAGE_MAX) {
        return false;
    }
    if (page_index == HOME_MODE_PAGE_INDEX_CUSTOMIZE_SELECT) {
        return info != NULL && info->mode.custom_mode_active != 0;
    }
    return true;
}

static uint8_t home_mode_page_dot_count_get(const home_base_ui_info_t *info)
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < HOME_PAGE_MAX; i++) {
        if (home_mode_page_is_available(info, i)) {
            count++;
        }
    }
    return count;
}

static uint8_t home_mode_page_dot_index_get(const home_base_ui_info_t *info, uint8_t page_index)
{
    uint8_t index = 0;

    for (uint8_t i = 0; i < HOME_PAGE_MAX; i++) {
        if (!home_mode_page_is_available(info, i)) {
            continue;
        }
        if (i == page_index) {
            return index;
        }
        index++;
    }
    return 0;
}

/** custom 未激活：5 点绝对坐标（左上角） */
static const lv_point_t home_mode_dot_pos_5[5] = {
    {130, 11},
    {152, 7},
    {HOME_MODE_PAGE_DOT_X_CENTER, 5},
    {198, 7},
    {220, 11},
};

/** custom 激活：6 点绝对坐标（左上角） */
static const lv_point_t home_mode_dot_pos_6[6] = {
    {119, 14},
    {141, 8},
    {163, 5},
    {186, 5},
    {208, 8},
    {230, 14},
};

static void home_mode_page_dot_place(lv_obj_t *dot, uint8_t dot_index, uint8_t dot_count)
{
    const lv_point_t *table;
    uint8_t table_n;

    EXCEPTION_CHECK_VOID(dot);
    if (dot_count >= 6) {
        table = home_mode_dot_pos_6;
        table_n = 6;
    } else {
        table = home_mode_dot_pos_5;
        table_n = 5;
    }
    if (dot_index >= table_n) {
        return;
    }
    lv_obj_set_pos(dot, table[dot_index].x, table[dot_index].y);
}

static home_base_mode_page_ctx_t *home_mode_page_ctx_get(home_base_ui_info_t *info)
{
    lv_obj_t *page = home_page_parent_get(info);

    if (page == NULL || !lv_obj_is_valid(page)) {
        return NULL;
    }
    return (home_base_mode_page_ctx_t *)lv_obj_get_user_data(page);
}

static bool home_mode_page_is_mode_like(uint8_t page_index)
{
    return page_index == HOME_PAGE_MODE_0 ||
           page_index == HOME_PAGE_MODE_1 ||
           page_index == HOME_PAGE_RHYTHM_0 ||
           page_index == HOME_PAGE_RHYTHM_1 ||
           page_index == HOME_PAGE_RHYTHM_2;
}

static void home_mode_chrome_dots_rebuild(home_base_ui_info_t *info, uint8_t selected_page)
{
    home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    lv_obj_t *root = info->root;
    const uint8_t dot_count = home_mode_page_dot_count_get(info);
    const uint8_t selected = home_mode_page_dot_index_get(info, selected_page);
    uint8_t i;

    EXCEPTION_CHECK_VOID(root);
    chrome->dot_count = dot_count;
    chrome->selected_dot = selected;

    for (i = 0; i < HOME_BASE_MODE_PAGE_DOT_MAX; i++) {
        if (i >= dot_count) {
            if (chrome->dots[i] != NULL && lv_obj_is_valid(chrome->dots[i])) {
                lv_obj_add_flag(chrome->dots[i], LV_OBJ_FLAG_HIDDEN);
            }
            continue;
        }

        if (chrome->dots[i] == NULL || !lv_obj_is_valid(chrome->dots[i])) {
            chrome->dots[i] = lv_obj_create(root);
            EXCEPTION_CHECK_VOID(chrome->dots[i]);
            lv_obj_remove_style_all(chrome->dots[i]);
            lv_obj_set_size(chrome->dots[i], HOME_MODE_PAGE_DOT_DIAMETER, HOME_MODE_PAGE_DOT_DIAMETER);
            lv_obj_set_style_radius(chrome->dots[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_opa(chrome->dots[i], LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(chrome->dots[i], 0, 0);
            lv_obj_clear_flag(chrome->dots[i], LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(chrome->dots[i], LV_OBJ_FLAG_CLICKABLE);
        }
        lv_obj_set_style_bg_color(chrome->dots[i],
            lv_color_hex(i == selected ? HOME_MODE_PAGE_DOT_COLOR_ACTIVE :
                HOME_MODE_PAGE_DOT_COLOR_INACTIVE), 0);
        home_mode_page_dot_place(chrome->dots[i], i, dot_count);
        lv_obj_move_foreground(chrome->dots[i]);
    }
}

static lv_draw_buf_t *home_mode_select_tip_predecode(ui_manager_context_t *ui)
{
    if (s_mode_select_tip_predecoded != NULL) {
        return s_mode_select_tip_predecoded;
    }
#if HOME_BASE_MODE_SELECT_TIP_FROM_CODE
    (void)ui;
    {
        const ui_img_data_t *tip = ui_font_img_mode_select_tip_get();
        if (tip == NULL || tip->data == NULL || tip->size == 0U) {
            return NULL;
        }
        s_mode_select_tip_predecoded = ui_img_png_decode_draw_buf(tip->data, tip->size,
            UI_IMG_PNG_DECODE_RGBA8888);
    }
#else
    if (ui == NULL) {
        return NULL;
    }
    s_mode_select_tip_predecoded = ui_img_baj_theme_decode_draw_buf(ui,
        BASE_RES_ID_MODE_SELECT_TIP, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
#endif
    return s_mode_select_tip_predecoded;
}

static void home_mode_chrome_tip_shine_sync(home_base_ui_info_t *info)
{
    home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    lv_coord_t tip_x;
    lv_coord_t tip_y;

    if (chrome->tip_img == NULL || !lv_obj_is_valid(chrome->tip_img) ||
        chrome->tip_shine == NULL || !lv_obj_is_valid(chrome->tip_shine) ||
        chrome->tip_mask == NULL || !lv_obj_is_valid(chrome->tip_mask)) {
        return;
    }

    lv_obj_update_layout(chrome->tip_img);
    tip_x = lv_obj_get_x(chrome->tip_img);
    tip_y = lv_obj_get_y(chrome->tip_img);
    lv_obj_set_pos(chrome->tip_shine,
        tip_x - chrome->tip_mask_x, tip_y - HOME_MODE_PAGE_TIP_MASK_Y);
}

#if HOME_BASE_MODE_TIP_SHINE_USE_IMG_MASK
/** 从预解码 TEXT_SHINE_MASK 生成 A8；高度不足时垂直居中裁切 */
static lv_draw_buf_t *home_mode_tip_mask_a8_from_rgba(const lv_draw_buf_t *rgba,
    uint32_t dst_w, uint32_t dst_h)
{
    lv_draw_buf_t *a8;
    uint32_t src_w;
    uint32_t src_h;
    uint32_t y;
    uint32_t x;
    uint32_t y0;
    uint32_t x0;

    if (rgba == NULL || rgba->data == NULL || dst_w == 0U || dst_h == 0U) {
        return NULL;
    }
    if (rgba->header.cf != LV_COLOR_FORMAT_ARGB8888 &&
        rgba->header.cf != LV_COLOR_FORMAT_ARGB8888_PREMULTIPLIED) {
        return NULL;
    }

    src_w = rgba->header.w;
    src_h = rgba->header.h;
    if (src_w == 0U || src_h == 0U) {
        return NULL;
    }

    a8 = lv_draw_buf_create(dst_w, dst_h, LV_COLOR_FORMAT_A8, LV_STRIDE_AUTO);
    if (a8 == NULL || a8->data == NULL) {
        if (a8 != NULL) {
            lv_draw_buf_destroy(a8);
        }
        return NULL;
    }

    x0 = (src_w > dst_w) ? ((src_w - dst_w) / 2U) : 0U;
    y0 = (src_h > dst_h) ? ((src_h - dst_h) / 2U) : 0U;

    for (y = 0; y < dst_h; y++) {
        uint32_t sy = (y0 + y < src_h) ? (y0 + y) : (src_h - 1U);
        const lv_color32_t *src =
            (const lv_color32_t *)(rgba->data + sy * rgba->header.stride);
        uint8_t *dst = a8->data + y * a8->header.stride;
        for (x = 0; x < dst_w; x++) {
            uint32_t sx = (x0 + x < src_w) ? (x0 + x) : (src_w - 1U);
            uint32_t a = src[sx].alpha;
#if HOME_BASE_MODE_TIP_SHINE_IS_SOFT
            /* SOFT：再叠加抛物线，两端更透 */
            uint32_t edge = (x * (dst_w - 1U - x) * 4U * 255U) / (dst_w * dst_w);
            if (edge > 255U) {
                edge = 255U;
            }
            a = (a * edge) / 255U;
            a = (a * a) / 255U;
#endif
            /* OVERLAY：直接使用 TEXT_SHINE_MASK 的 alpha */
            dst[x] = (uint8_t)a;
        }
    }
    return a8;
}

static void home_mode_chrome_tip_mask_a8_release(home_base_ui_info_t *info)
{
    home_base_mode_chrome_info_t *chrome;

    if (info == NULL) {
        return;
    }
    chrome = &info->mode_chrome;
    if (chrome->tip_mask_a8 == NULL) {
        return;
    }
    if (chrome->tip_mask != NULL && lv_obj_is_valid(chrome->tip_mask)) {
        lv_obj_set_style_bitmap_mask_src(chrome->tip_mask, NULL, 0);
    }
    lv_draw_buf_destroy(chrome->tip_mask_a8);
    chrome->tip_mask_a8 = NULL;
}

static void home_mode_chrome_tip_mask_a8_prepare(home_base_ui_info_t *info)
{
    home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    const lv_draw_buf_t *rgba;

    if (chrome->tip_mask_a8 != NULL) {
        if (chrome->tip_mask != NULL && lv_obj_is_valid(chrome->tip_mask)) {
            lv_obj_set_style_bitmap_mask_src(chrome->tip_mask, chrome->tip_mask_a8, 0);
        }
        return;
    }

    rgba = get_predecoded_text_shine_mask();
    chrome->tip_mask_a8 = home_mode_tip_mask_a8_from_rgba(rgba,
        HOME_MODE_PAGE_TIP_MASK_W, HOME_MODE_PAGE_TIP_MASK_H);
    if (chrome->tip_mask_a8 != NULL && chrome->tip_mask != NULL &&
        lv_obj_is_valid(chrome->tip_mask)) {
        lv_obj_set_style_bitmap_mask_src(chrome->tip_mask, chrome->tip_mask_a8, 0);
    }
}
#endif /* HOME_BASE_MODE_TIP_SHINE_USE_IMG_MASK */

static void home_mode_chrome_tip_rebuild(home_base_ui_info_t *info)
{
    home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    lv_obj_t *root = info->root;
    lv_draw_buf_t *predecoded;

    EXCEPTION_CHECK_VOID(root);
    if (chrome->tip_img != NULL && lv_obj_is_valid(chrome->tip_img) &&
        chrome->tip_mask != NULL && lv_obj_is_valid(chrome->tip_mask) &&
        chrome->tip_shine != NULL && lv_obj_is_valid(chrome->tip_shine)) {
        home_mode_chrome_tip_shine_sync(info);
        return;
    }

    predecoded = home_mode_select_tip_predecode(info->ui);

    if (chrome->tip_img == NULL || !lv_obj_is_valid(chrome->tip_img)) {
        chrome->tip_img = lv_image_create(root);
        EXCEPTION_CHECK_VOID(chrome->tip_img);
        lv_obj_clear_flag(chrome->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(chrome->tip_img, predecoded);
            chrome->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(chrome->tip_img, HOME_MODE_PAGE_TIP_BASE_OPA, 0);
        lv_obj_align(chrome->tip_img, LV_ALIGN_TOP_MID, 0, HOME_MODE_PAGE_TIP_Y);
    }

    if (chrome->tip_mask == NULL || !lv_obj_is_valid(chrome->tip_mask)) {
        chrome->tip_mask = lv_obj_create(root);
        EXCEPTION_CHECK_VOID(chrome->tip_mask);
        lv_obj_remove_style_all(chrome->tip_mask);
        lv_obj_set_size(chrome->tip_mask, HOME_MODE_PAGE_TIP_MASK_W, HOME_MODE_PAGE_TIP_MASK_H);
        lv_obj_set_style_bg_opa(chrome->tip_mask, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(chrome->tip_mask, 0, 0);
        lv_obj_set_style_pad_all(chrome->tip_mask, 0, 0);
        lv_obj_clear_flag(chrome->tip_mask, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(chrome->tip_mask, LV_OBJ_FLAG_CLICKABLE);
        chrome->tip_mask_x = HOME_MODE_PAGE_TIP_MASK_X_MIN;
        chrome->tip_mask_tick_ms = get_timestamp();
    }

    if (chrome->tip_shine == NULL || !lv_obj_is_valid(chrome->tip_shine)) {
        chrome->tip_shine = lv_image_create(chrome->tip_mask);
        EXCEPTION_CHECK_VOID(chrome->tip_shine);
        lv_obj_clear_flag(chrome->tip_shine, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(chrome->tip_shine, predecoded);
        }
        lv_obj_set_style_image_recolor(chrome->tip_shine,
            lv_color_hex(HOME_MODE_PAGE_TIP_SHINE_COLOR), 0);
        lv_obj_set_style_image_recolor_opa(chrome->tip_shine,
            HOME_MODE_PAGE_TIP_SHINE_RECOLOR_OPA, 0);
        lv_obj_set_style_blend_mode(chrome->tip_shine, LV_BLEND_MODE_NORMAL, 0);
    }

#if HOME_BASE_MODE_TIP_SHINE_USE_IMG_MASK
    home_mode_chrome_tip_mask_a8_prepare(info);
#endif
    lv_obj_align(chrome->tip_mask, LV_ALIGN_TOP_LEFT, chrome->tip_mask_x, HOME_MODE_PAGE_TIP_MASK_Y);
    lv_obj_move_foreground(chrome->tip_img);
    lv_obj_move_foreground(chrome->tip_mask);
    home_mode_chrome_tip_shine_sync(info);
}

static void home_mode_chrome_tip_mask_anim_update(home_base_ui_info_t *info)
{
    home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    uint32_t elapsed;
    int32_t x;

    if (chrome->tip_mask == NULL || !lv_obj_is_valid(chrome->tip_mask)) {
        return;
    }
    if (lv_obj_has_flag(chrome->tip_mask, LV_OBJ_FLAG_HIDDEN)) {
        return;
    }

    elapsed = get_timestamp() - chrome->tip_mask_tick_ms;
    if (elapsed >= HOME_MODE_PAGE_TIP_MASK_PERIOD_MS) {
        chrome->tip_mask_tick_ms = get_timestamp();
        elapsed = 0;
    }
    x = HOME_MODE_PAGE_TIP_MASK_X_MIN +
        (int32_t)((HOME_MODE_PAGE_TIP_MASK_X_MAX - HOME_MODE_PAGE_TIP_MASK_X_MIN) * elapsed) /
        HOME_MODE_PAGE_TIP_MASK_PERIOD_MS;
    if (x != chrome->tip_mask_x) {
        chrome->tip_mask_x = (int16_t)x;
        lv_obj_set_x(chrome->tip_mask, chrome->tip_mask_x);
        home_mode_chrome_tip_shine_sync(info);
    }
}

static void home_mode_chrome_visibility_apply(home_base_ui_info_t *info)
{
    home_base_mode_chrome_info_t *chrome = &info->mode_chrome;
    const uint8_t shown = info->page.shown_page_index;
    const uint8_t target = info->page.page_index;
    const bool transitioning = info->page.transitioning != 0;
    /* page0 静止：隐藏；离 page0 动画一开始即显示定位点 */
    const bool dots_show = !(shown == HOME_PAGE_MAIN && !transitioning);
    /* page0 相关（在主页或进出主页过渡）：隐藏提示文案 */
    const bool page0_related =
        (shown == HOME_PAGE_MAIN) ||
        (transitioning && target == HOME_PAGE_MAIN);
    const bool tip_show = !page0_related && home_mode_page_is_mode_like(shown);
    uint8_t i;

    for (i = 0; i < HOME_BASE_MODE_PAGE_DOT_MAX; i++) {
        if (chrome->dots[i] == NULL || !lv_obj_is_valid(chrome->dots[i])) {
            continue;
        }
        if (dots_show && i < chrome->dot_count) {
            lv_obj_clear_flag(chrome->dots[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(chrome->dots[i]);
        } else {
            lv_obj_add_flag(chrome->dots[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (chrome->tip_img != NULL && lv_obj_is_valid(chrome->tip_img)) {
        if (tip_show) {
            lv_obj_clear_flag(chrome->tip_img, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(chrome->tip_img);
        } else {
            lv_obj_add_flag(chrome->tip_img, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (chrome->tip_mask != NULL && lv_obj_is_valid(chrome->tip_mask)) {
        if (tip_show) {
            lv_obj_clear_flag(chrome->tip_mask, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(chrome->tip_mask);
            home_mode_chrome_tip_shine_sync(info);
        } else {
            lv_obj_add_flag(chrome->tip_mask, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void home_mode_page_icon_detach(home_base_mode_page_ctx_t *ctx)
{
    if (ctx == NULL || ctx->icon == NULL || !lv_obj_is_valid(ctx->icon)) {
        return;
    }

    const void *src = lv_image_get_src(ctx->icon);
    if (src == NULL) {
        return;
    }

    /* 预解码 ICON 由 snapshot 持有，页面只解绑，不 destroy */
    if (ctx->icon_predecoded || get_predecoded_icon(ctx->icon_id) == src) {
        lv_image_set_src(ctx->icon, NULL);
        ctx->icon_predecoded = 0;
        return;
    }
    ui_img_png_release_decoded_buf(ctx->icon);
}

static void home_mode_page_icon_build(home_base_ui_info_t *info, home_base_mode_page_ctx_t *ctx)
{
    lv_obj_t *parent = home_page_parent_get(info);
    const uint16_t res_id = home_mode_page_icon_res_id_get(ctx->mode);
    lv_draw_buf_t *predecoded;

    if (ctx->icon == NULL || !lv_obj_is_valid(ctx->icon)) {
        ctx->icon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(ctx->icon);
    }

    home_mode_page_icon_detach(ctx);
    predecoded = get_predecoded_icon(res_id);
    if (predecoded != NULL) {
        lv_image_set_src(ctx->icon, predecoded);
        ctx->icon_predecoded = 1;
    } else {
        ui_img_png_theme_set_src(info->ui, ctx->icon, res_id,
            RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
        ctx->icon_predecoded = 0;
    }
    lv_obj_align(ctx->icon, LV_ALIGN_TOP_MID, 0, HOME_MODE_PAGE_ICON_Y);
    ctx->icon_id = res_id;
}

static void home_mode_page_title_build(home_base_ui_info_t *info, home_base_mode_page_ctx_t *ctx)
{
    char title[24] = {0};
    lv_obj_t *parent = home_page_parent_get(info);

    home_mode_page_title_get(info, ctx->mode, title, sizeof(title));

    if (ctx->title == NULL || !lv_obj_is_valid(ctx->title)) {
        ctx->title = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(ctx->title);
        lv_obj_set_style_text_color(ctx->title, lv_color_hex(HOME_MODE_PAGE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(ctx->title, &lv_font_aeonik_soft_33_medium, 0);
        lv_obj_set_style_text_align(ctx->title, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(ctx->title, 0, 0);
    }
    lv_label_set_text(ctx->title, title);
    lv_obj_align(ctx->title, LV_ALIGN_TOP_MID, 0, HOME_MODE_PAGE_TITLE_Y);
    strncpy(ctx->shown_title, title, sizeof(ctx->shown_title) - 1);
    ctx->shown_title[sizeof(ctx->shown_title) - 1] = '\0';
}

static bool home_mode_page_has_duration(uint8_t mode)
{
    return mode == BREAST_PUMP_MILK_BOOST_MODE ||
           mode == BREAST_PUMP_PP_MILK_TRACK_MODE ||
           mode == BREAST_PUMP_CUSTOM_MODE;
}

static void home_mode_page_duration_text_get(home_base_ui_info_t *info, uint8_t mode,
    char *buf, size_t buf_len)
{
    int pump_time = 0;

    if (buf == NULL || buf_len == 0) {
        return;
    }
    buf[0] = '\0';
    if (!home_mode_page_has_duration(mode)) {
        return;
    }
    if (info != NULL && info->st != NULL) {
        info->st->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, mode, 0, &pump_time);
    }
    snprintf(buf, buf_len, "≈ %dmin", pump_time / 60);
}

static void home_mode_page_duration_build(home_base_ui_info_t *info, home_base_mode_page_ctx_t *ctx)
{
    char duration[16] = {0};
    lv_obj_t *parent = home_page_parent_get(info);

    if (!home_mode_page_has_duration(ctx->mode)) {
        if (ctx->duration != NULL && lv_obj_is_valid(ctx->duration)) {
            lv_obj_add_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN);
        }
        ctx->shown_duration[0] = '\0';
        return;
    }

    home_mode_page_duration_text_get(info, ctx->mode, duration, sizeof(duration));

    if (ctx->duration == NULL || !lv_obj_is_valid(ctx->duration)) {
        ctx->duration = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(ctx->duration);
        lv_obj_set_style_text_color(ctx->duration, lv_color_hex(HOME_MODE_PAGE_DURATION_COLOR), 0);
        lv_obj_set_style_text_font(ctx->duration, &lv_font_aeonik_soft_26_medium, 0);
        lv_obj_set_style_text_align(ctx->duration, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(ctx->duration, 0, 0);
    }
    lv_label_set_text(ctx->duration, duration);
    lv_obj_align(ctx->duration, LV_ALIGN_TOP_MID, 0, HOME_MODE_PAGE_DURATION_Y);
    lv_obj_clear_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN);
    strncpy(ctx->shown_duration, duration, sizeof(ctx->shown_duration) - 1);
    ctx->shown_duration[sizeof(ctx->shown_duration) - 1] = '\0';
}

static void home_mode_page_duration_content_update(home_base_ui_info_t *info,
    home_base_mode_page_ctx_t *ctx)
{
    char duration[16] = {0};

    if (!home_mode_page_has_duration(ctx->mode)) {
        return;
    }
    home_mode_page_duration_text_get(info, ctx->mode, duration, sizeof(duration));
    if (strncmp(ctx->shown_duration, duration, sizeof(ctx->shown_duration)) == 0 &&
        !info->force_update) {
        return;
    }
    if (ctx->duration == NULL || !lv_obj_is_valid(ctx->duration)) {
        home_mode_page_duration_build(info, ctx);
        return;
    }
    lv_label_set_text(ctx->duration, duration);
    lv_obj_align(ctx->duration, LV_ALIGN_TOP_MID, 0, HOME_MODE_PAGE_DURATION_Y);
    strncpy(ctx->shown_duration, duration, sizeof(ctx->shown_duration) - 1);
    ctx->shown_duration[sizeof(ctx->shown_duration) - 1] = '\0';
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_mode_chrome_prepare(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->root);
    if (info->mode_chrome.ready) {
        return;
    }
    memset(&info->mode_chrome, 0, sizeof(info->mode_chrome));
    home_mode_chrome_tip_rebuild(info);
    home_mode_chrome_dots_rebuild(info, HOME_PAGE_MAIN);
    info->mode_chrome.ready = 1;
    home_mode_chrome_visibility_apply(info);
}

void home_mode_chrome_remove(home_base_ui_info_t *info)
{
    uint8_t i;

    EXCEPTION_CHECK_VOID(info);
    for (i = 0; i < HOME_BASE_MODE_PAGE_DOT_MAX; i++) {
        LVGL_OBJ_DELETE(info->mode_chrome.dots[i]);
        info->mode_chrome.dots[i] = NULL;
    }
    /* 预解码 tip 由本模块静态持有，仅删控件、不 destroy draw_buf */
    if (info->mode_chrome.tip_shine != NULL && lv_obj_is_valid(info->mode_chrome.tip_shine)) {
        lv_image_set_src(info->mode_chrome.tip_shine, NULL);
        LVGL_OBJ_DELETE(info->mode_chrome.tip_shine);
        info->mode_chrome.tip_shine = NULL;
    }
#if HOME_BASE_MODE_TIP_SHINE_USE_IMG_MASK
    home_mode_chrome_tip_mask_a8_release(info);
#endif
    if (info->mode_chrome.tip_mask != NULL && lv_obj_is_valid(info->mode_chrome.tip_mask)) {
        LVGL_OBJ_DELETE(info->mode_chrome.tip_mask);
        info->mode_chrome.tip_mask = NULL;
    }
    if (info->mode_chrome.tip_img != NULL && lv_obj_is_valid(info->mode_chrome.tip_img)) {
        if (info->mode_chrome.tip_predecoded) {
            lv_image_set_src(info->mode_chrome.tip_img, NULL);
        } else {
            ui_img_baj_release_decoded_buf(info->mode_chrome.tip_img);
        }
        LVGL_OBJ_DELETE(info->mode_chrome.tip_img);
        info->mode_chrome.tip_img = NULL;
    }
    memset(&info->mode_chrome, 0, sizeof(info->mode_chrome));
}

void home_mode_chrome_update(home_base_ui_info_t *info)
{
    uint8_t selected_page;

    EXCEPTION_CHECK_VOID(info);
    if (!info->mode_chrome.ready) {
        home_mode_chrome_prepare(info);
    }

    selected_page = info->page.transitioning ?
        info->page.page_index : info->page.shown_page_index;
    if (selected_page >= HOME_PAGE_MAX) {
        selected_page = HOME_PAGE_MAIN;
    }

    {
        const uint8_t dot_count = home_mode_page_dot_count_get(info);
        const uint8_t selected = home_mode_page_dot_index_get(info, selected_page);
        if (dot_count != info->mode_chrome.dot_count ||
            selected != info->mode_chrome.selected_dot ||
            info->force_update) {
            home_mode_chrome_dots_rebuild(info, selected_page);
        }
    }
    home_mode_chrome_visibility_apply(info);
    home_mode_chrome_tip_mask_anim_update(info);
}

void home_mode_page_ui_build(home_base_ui_info_t *info, uint8_t page_index)
{
    home_base_mode_page_ctx_t *ctx;
    lv_obj_t *page;

    EXCEPTION_CHECK_VOID(info);
    page = home_page_parent_get(info);
    EXCEPTION_CHECK_VOID(page);

    ctx = (home_base_mode_page_ctx_t *)u_malloz(sizeof(home_base_mode_page_ctx_t));
    EXCEPTION_CHECK_VOID(ctx);
    lv_obj_set_user_data(page, ctx);

    ctx->page_index = page_index;
    ctx->mode = home_mode_page_mode_from_page(page_index);

    home_mode_chrome_prepare(info);
    ctx->circle_bg = home_mode_page_circle_bg_create(page);
    home_mode_page_icon_build(info, ctx);
    home_mode_page_title_build(info, ctx);
    home_mode_page_duration_build(info, ctx);
    if (ctx->circle_bg != NULL && lv_obj_is_valid(ctx->circle_bg)) {
        lv_obj_move_background(ctx->circle_bg);
    }
}

void home_mode_page_ui_update(home_base_ui_info_t *info, ui_proc_home_t *proc)
{
    home_base_mode_page_ctx_t *ctx;
    char title[24] = {0};

    EXCEPTION_CHECK_VOID(info && proc);
    ctx = home_mode_page_ctx_get(info);
    if (ctx == NULL) {
        return;
    }

    /// 个性化 ID 变化时刷新 Customized 文案
    if (ctx->mode == BREAST_PUMP_CUSTOM_MODE || info->force_update) {
        home_mode_page_title_get(info, ctx->mode, title, sizeof(title));
        if (strncmp(ctx->shown_title, title, sizeof(ctx->shown_title)) != 0 ||
            info->force_update) {
            if (ctx->title != NULL && lv_obj_is_valid(ctx->title)) {
                lv_label_set_text(ctx->title, title);
                lv_obj_align(ctx->title, LV_ALIGN_TOP_MID, 0, HOME_MODE_PAGE_TITLE_Y);
            }
            strncpy(ctx->shown_title, title, sizeof(ctx->shown_title) - 1);
            ctx->shown_title[sizeof(ctx->shown_title) - 1] = '\0';
        }
    }
    home_mode_page_duration_content_update(info, ctx);

    if (!home_page_ui_is_transitioning(info)) {
        home_mode_page_transition_apply(home_page_parent_get(info),
            HOME_MODE_PAGE_SCALE_FULL, false);
    }
}

void home_mode_page_ui_remove(home_base_ui_info_t *info)
{
    home_base_mode_page_ctx_t *ctx;
    lv_obj_t *page;

    EXCEPTION_CHECK_VOID(info);
    page = home_page_parent_get(info);
    if (page == NULL || !lv_obj_is_valid(page)) {
        return;
    }

    ctx = (home_base_mode_page_ctx_t *)lv_obj_get_user_data(page);
    if (ctx == NULL) {
        return;
    }

    home_mode_page_icon_detach(ctx);
    lv_obj_set_user_data(page, NULL);
    u_free(ctx);
}

void home_mode_page_transition_apply(lv_obj_t *page, int32_t hero_scale, bool transitioning)
{
    home_base_mode_page_ctx_t *ctx;
    /* 放大未完成前保持圆+小字号；放大完成（或静止）再隐藏圆并切 36 */
    const bool small = transitioning && (hero_scale < HOME_MODE_PAGE_SCALE_FULL);

    if (page == NULL || !lv_obj_is_valid(page)) {
        return;
    }
    ctx = (home_base_mode_page_ctx_t *)lv_obj_get_user_data(page);
    if (ctx == NULL) {
        return;
    }

    if (ctx->circle_bg != NULL && lv_obj_is_valid(ctx->circle_bg)) {
        if (small) {
            lv_obj_clear_flag(ctx->circle_bg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_background(ctx->circle_bg);
        } else {
            lv_obj_add_flag(ctx->circle_bg, LV_OBJ_FLAG_HIDDEN);
        }
        /* 圆背景不参与缩放 */
        home_mode_page_obj_scale_set(ctx->circle_bg, HOME_MODE_PAGE_SCALE_FULL);
    }

    /* ICON 跟 hero_scale；标题不缩放，放大完成后再 29→36 */
    home_mode_page_obj_scale_set(ctx->icon, hero_scale);
    if (ctx->title != NULL && lv_obj_is_valid(ctx->title)) {
        const lv_font_t *font = small ?
            &lv_font_aeonik_soft_29_medium : &lv_font_aeonik_soft_33_medium;

        home_mode_page_obj_scale_set(ctx->title, HOME_MODE_PAGE_SCALE_FULL);
        lv_obj_set_style_text_font(ctx->title, font, 0);
        lv_obj_align(ctx->title, LV_ALIGN_TOP_MID, 0, HOME_MODE_PAGE_TITLE_Y);
    }

    /* 总时间：圆背景消失后展示，圆背景出现时隐藏 */
    if (ctx->duration != NULL && lv_obj_is_valid(ctx->duration)) {
        if (!small && home_mode_page_has_duration(ctx->mode)) {
            lv_obj_clear_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN);
            lv_obj_align(ctx->duration, LV_ALIGN_TOP_MID, 0, HOME_MODE_PAGE_DURATION_Y);
        } else {
            lv_obj_add_flag(ctx->duration, LV_OBJ_FLAG_HIDDEN);
        }
    }
}
