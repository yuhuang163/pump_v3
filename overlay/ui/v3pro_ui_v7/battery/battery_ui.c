/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "battery_ui.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ui_img_baj_decode.h" // IWYU pragma: keep
#include "ui_img_resource.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define BATTERY_FRAME_W                                    (27)
#define BATTERY_FRAME_H                                    (14)
#define BATTERY_CAP_MAX_W                                  (19)
#define BATTERY_CAP_H                                      (9)
#define BATTERY_CAP_X_OFFSET                               (3)
#define BATTERY_CAP_Y_OFFSET                               (3)
#define BATTERY_CAP_COLOR_NORMAL                           (0xFFFFFF)
#define BATTERY_CAP_COLOR_WARNING                          (0xE30509)
#define BATTERY_CAP_RADIUS                                 (2)
#define BATTERY_CHARGE_ICON_COUNT                          (6)
#define BATTERY_CHARGE_ANIM_INTERVAL_MS                    (1000)
#define BATTERY_CHARGE_FULL_PERCENT                        (98)
#define BATTERY_CAP_WIDTH_BY_SEGMENT                       (0)
#define BATTERY_CAP_SEGMENT_COUNT                          (5)
#define BATTERY_ECO_LAYOUT_PERCENTAGE                      (LOW_POWER_RUN_PERCENTAGE)

typedef struct {
    lv_obj_t *root;
    lv_obj_t *container;
    lv_obj_t *cap_fill;
    lv_obj_t *frame_img;
    lv_obj_t *charge_img;
    lv_obj_t *eco_img;
    lv_draw_buf_t *charge_icon_bufs[BATTERY_CHARGE_ICON_COUNT];
    uint16_t charge_res_id;
    uint32_t charge_anim_start_time;
    uint8_t percent;
    uint8_t is_low_power;
    uint8_t is_eco_layout;
    uint8_t is_charging;
    uint8_t charge_anim_index;
    uint8_t charge_anim_start_idx;
    uint8_t charge_icon_preloaded;
    uint8_t force_update;
    res_theme_type_e charge_res_theme_type;
} battery_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static battery_ui_info_t *battery_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void battery_ui_align_set(lv_obj_t *obj, uint16_t x, uint16_t y)
{
    lv_obj_align(obj, LV_ALIGN_TOP_MID, x, y);
}

static lv_coord_t battery_ui_cap_width_get(uint8_t percent)
{
#if BATTERY_CAP_WIDTH_BY_SEGMENT
    uint8_t segments = 0;

    if (percent >= 81) {
        segments = 5;
    } else if (percent >= 61) {
        segments = 4;
    } else if (percent >= 41) {
        segments = 3;
    } else if (percent >= 21) {
        segments = 2;
    } else if (percent >= 5) {
        segments = 1;
    }

    if (segments == 0) {
        return 0;
    }

    return (lv_coord_t)(BATTERY_CAP_MAX_W * segments) / BATTERY_CAP_SEGMENT_COUNT;
#else
    if (percent == 0) {
        return 0;
    }

    lv_coord_t width = ((lv_coord_t)BATTERY_CAP_MAX_W * percent + 50) / 100;
    if (width < 1) {
        width = 1;
    }
    if (width > BATTERY_CAP_MAX_W) {
        width = BATTERY_CAP_MAX_W;
    }
    return width;
#endif
}

static lv_coord_t battery_ui_cap_radius_get(lv_coord_t width)
{
    lv_coord_t radius = BATTERY_CAP_RADIUS;
    lv_coord_t max_radius = width / 2;
    if (max_radius < radius) {
        radius = max_radius;
    }
    if (radius < 1) {
        radius = 1;
    }
    return radius;
}

static res_theme_type_e battery_ui_theme_type_get(ui_manager_context_t *thiz)
{
    if (thiz == NULL || thiz->priv.res == NULL || thiz->priv.res->get_sys_theme_type == NULL) {
        return RES_THEME_TYPE_BASE;
    }

    res_theme_type_e theme = (res_theme_type_e)thiz->priv.res->get_sys_theme_type();
    if (theme == RES_THEME_TYPE_NONE || theme >= RES_THEME_TYPE_MAX) {
        return RES_THEME_TYPE_BASE;
    }
    return theme;
}

static uint16_t battery_ui_charge_res_id_by_index(res_theme_type_e theme_type, uint8_t index)
{
    if (index >= BATTERY_CHARGE_ICON_COUNT) {
        index = 0;
    }

    if (theme_type == RES_THEME_TYPE_CUSTOM) {
        return (uint16_t)(CUSTOM_RES_ID_CHARGE_BATT_0_ICON + index);
    }
    return (uint16_t)(BASE_RES_ID_CHARGE_BATT_0_ICON + index);
}

static void battery_ui_charge_anim_range_get(uint8_t percent, uint8_t *start_idx, bool *is_fixed)
{
    *is_fixed = false;

    if (percent > BATTERY_CHARGE_FULL_PERCENT) {
        *start_idx = 5;
        *is_fixed = true;
        return;
    }

    if (percent < 10) {
        *start_idx = 0;
    } else if (percent < 30) {
        *start_idx = 1;
    } else if (percent < 50) {
        *start_idx = 2;
    } else if (percent < 70) {
        *start_idx = 3;
    } else {
        *start_idx = 4;
    }
}

static uint8_t battery_ui_charge_icon_idx_get(uint8_t percent, uint8_t anim_index)
{
    uint8_t start_idx = 0;
    bool is_fixed = false;

    battery_ui_charge_anim_range_get(percent, &start_idx, &is_fixed);
    if (is_fixed) {
        return 5;
    }

    const uint8_t count = BATTERY_CHARGE_ICON_COUNT - start_idx;
    return start_idx + (anim_index % count);
}

static uint16_t battery_ui_charge_res_id_get(res_theme_type_e theme_type, uint8_t percent, uint8_t anim_index)
{
    return battery_ui_charge_res_id_by_index(theme_type,
        battery_ui_charge_icon_idx_get(percent, anim_index));
}

static lv_draw_buf_t *battery_ui_charge_icon_decode(ui_manager_context_t *thiz, uint16_t res_id,
    res_theme_type_e theme_type)
{
    res_img_info_t img_info = {0};

    ui_theme_img_resource_init(thiz, res_id, theme_type, &img_info);
    if (img_info.data == NULL) {
        return NULL;
    }

    lv_obj_t *tmp = lv_image_create(lv_screen_active());
    if (tmp == NULL) {
        ui_img_resource_deinit(thiz, &img_info);
        return NULL;
    }
    lv_obj_add_flag(tmp, LV_OBJ_FLAG_HIDDEN);

    const int ret = ui_img_baj_set_src(tmp, img_info.data, img_info.size, UI_IMG_BAJ_DECODE_RGBA8888);
    ui_img_resource_deinit(thiz, &img_info);
    if (ret != 0) {
        lv_obj_delete(tmp);
        return NULL;
    }

    const void *src = lv_image_get_src(tmp);
    lv_draw_buf_t *draw_buf = NULL;
    if (src != NULL && lv_image_src_get_type(src) == LV_IMAGE_SRC_VARIABLE) {
        draw_buf = (lv_draw_buf_t *)src;
        lv_image_set_src(tmp, NULL);
    }
    lv_obj_delete(tmp);
    return draw_buf;
}

static void battery_ui_charge_icons_release(void)
{
    for (uint8_t i = 0; i < BATTERY_CHARGE_ICON_COUNT; i++) {
        if (battery_info->charge_icon_bufs[i] != NULL) {
            lv_draw_buf_destroy(battery_info->charge_icon_bufs[i]);
            battery_info->charge_icon_bufs[i] = NULL;
        }
    }
    battery_info->charge_icon_preloaded = 0;
    battery_info->charge_res_theme_type = RES_THEME_TYPE_NONE;
}

static void battery_ui_charge_icons_preload(ui_manager_context_t *thiz)
{
    res_theme_type_e theme_type = battery_ui_theme_type_get(thiz);

    if (battery_info->charge_icon_preloaded && battery_info->charge_res_theme_type == theme_type) {
        return;
    }

    battery_ui_charge_icons_release();
    battery_info->charge_res_theme_type = theme_type;

    for (uint8_t i = 0; i < BATTERY_CHARGE_ICON_COUNT; i++) {
        battery_info->charge_icon_bufs[i] = battery_ui_charge_icon_decode(thiz,
            battery_ui_charge_res_id_by_index(theme_type, i), theme_type);
    }
    battery_info->charge_icon_preloaded = 1;
}

static uint8_t battery_ui_is_charging_detect(battery_manager_context_t *battery, uint8_t charge_status)
{
    if (BATTERY_IS_CHARGING(charge_status)) {
        return 1;
    }
    if (charge_status == CHARGE_STATE_ERROR || charge_status == CHARGE_STATE_NO_BATT) {
        return 0;
    }
    if (battery != NULL && battery->is_vbus_plug_in != NULL && battery->is_vbus_plug_in() > 0) {
        return 1;
    }
    return 0;
}

static void battery_ui_charge_state_reset(uint8_t percent)
{
    uint8_t start_idx = 0;
    bool is_fixed = false;

    battery_ui_charge_anim_range_get(percent, &start_idx, &is_fixed);
    battery_info->charge_anim_index = 0;
    battery_info->charge_anim_start_idx = start_idx;
    battery_info->charge_anim_start_time = get_timestamp();
    battery_info->charge_res_id = 0;
}

static bool battery_ui_need_update(ui_manager_context_t *thiz, uint8_t percent, uint8_t is_low_power,
    uint8_t is_eco_layout, uint8_t is_charging, lv_obj_t *root)
{
    res_theme_type_e theme_type = battery_ui_theme_type_get(thiz);
    if (battery_info->force_update) {
        return true;
    }
    if (battery_info->root != root) {
        return true;
    }
    if (battery_info->charge_res_theme_type != theme_type) {
        return true;
    }
    if (battery_info->is_charging != is_charging) {
        return true;
    }

    if (is_charging) {
        if (battery_info->percent != percent) {
            return true;
        }
        if (get_timestamp_ms(battery_info->charge_anim_start_time) >= BATTERY_CHARGE_ANIM_INTERVAL_MS) {
            return true;
        }
        return battery_info->charge_res_id != battery_ui_charge_res_id_get(theme_type, percent,
            battery_info->charge_anim_index);
    }

    if (battery_info->percent != percent) {
        return true;
    }
    if (battery_info->is_eco_layout != is_eco_layout) {
        return true;
    }
    return battery_info->is_low_power != is_low_power;
}

static void battery_ui_container_ensure(lv_obj_t *root, uint16_t x, uint16_t y)
{
    if (battery_info->container != NULL && lv_obj_is_valid(battery_info->container) &&
        battery_info->root == root) {
        battery_ui_align_set(battery_info->container, x, y);
        return;
    }

    battery_info->root = root;
    battery_info->frame_img = NULL;
    battery_info->cap_fill = NULL;
    battery_info->charge_img = NULL;
    battery_info->eco_img = NULL;
    battery_info->container = lv_obj_create(root);
    EXCEPTION_CHECK_VOID(battery_info->container);
    lv_obj_set_size(battery_info->container, BATTERY_FRAME_W, BATTERY_FRAME_H);
    lv_obj_set_style_bg_opa(battery_info->container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(battery_info->container, 0, 0);
    lv_obj_set_style_pad_all(battery_info->container, 0, 0);
    lv_obj_clear_flag(battery_info->container, LV_OBJ_FLAG_SCROLLABLE);
    battery_ui_align_set(battery_info->container, x, y);
}

static void battery_ui_cap_fill_update(uint8_t percent, uint8_t is_low_power)
{
    const lv_coord_t fill_width = battery_ui_cap_width_get(percent);
    const uint32_t fill_color = is_low_power ? BATTERY_CAP_COLOR_WARNING : BATTERY_CAP_COLOR_NORMAL;

    if (battery_info->cap_fill == NULL || !lv_obj_is_valid(battery_info->cap_fill)) {
        battery_info->cap_fill = lv_obj_create(battery_info->container);
        EXCEPTION_CHECK_VOID(battery_info->cap_fill);
        lv_obj_set_style_border_width(battery_info->cap_fill, 0, 0);
        lv_obj_set_style_pad_all(battery_info->cap_fill, 0, 0);
        lv_obj_set_style_bg_opa(battery_info->cap_fill, LV_OPA_COVER, 0);
        lv_obj_clear_flag(battery_info->cap_fill, LV_OBJ_FLAG_SCROLLABLE);
    }

    if (fill_width == 0) {
        lv_obj_add_flag(battery_info->cap_fill, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(battery_info->cap_fill, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(battery_info->cap_fill, fill_width, BATTERY_CAP_H);
    lv_obj_set_style_radius(battery_info->cap_fill, battery_ui_cap_radius_get(fill_width), 0);
    lv_obj_set_style_bg_color(battery_info->cap_fill, lv_color_hex(fill_color), 0);
    lv_obj_align(battery_info->cap_fill, LV_ALIGN_TOP_LEFT,
        BATTERY_CAP_X_OFFSET, BATTERY_CAP_Y_OFFSET);
}

static void battery_ui_frame_update(ui_manager_context_t *thiz, uint8_t is_low_power)
{
    const uint16_t frame_res_id = is_low_power ?
        BASE_RES_ID_BATTERY_WARNING_ICON : BASE_RES_ID_BATTERY_ICON;

    if (battery_info->frame_img == NULL || !lv_obj_is_valid(battery_info->frame_img)) {
        battery_info->frame_img = lv_image_create(battery_info->container);
        EXCEPTION_CHECK_VOID(battery_info->frame_img);
    }

    ui_img_baj_theme_set_src(thiz, battery_info->frame_img, frame_res_id,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_clear_flag(battery_info->frame_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(battery_info->frame_img, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_move_foreground(battery_info->frame_img);
}

static void battery_ui_eco_update(ui_manager_context_t *thiz, uint8_t is_eco_layout)
{
    if (!is_eco_layout) {
        if (battery_info->eco_img != NULL && lv_obj_is_valid(battery_info->eco_img)) {
            lv_obj_add_flag(battery_info->eco_img, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    if (battery_info->eco_img == NULL || !lv_obj_is_valid(battery_info->eco_img)) {
        battery_info->eco_img = lv_image_create(battery_info->root);
        EXCEPTION_CHECK_VOID(battery_info->eco_img);
    }

    if (battery_info->force_update || battery_info->is_eco_layout != is_eco_layout) {
        ui_img_baj_theme_set_src(thiz, battery_info->eco_img, BASE_RES_ID_POWER_ECO_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_clear_flag(battery_info->eco_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(battery_info->eco_img, LV_ALIGN_TOP_LEFT,
        BASE_PUMP_HOME_POWER_ECO_UI_X, BASE_PUMP_HOME_POWER_ECO_UI_Y);
    lv_obj_move_foreground(battery_info->eco_img);
}

static void battery_ui_charge_frame_update(ui_manager_context_t *thiz, uint8_t percent)
{
    uint8_t start_idx = 0;
    bool is_fixed = false;

    battery_ui_charge_anim_range_get(percent, &start_idx, &is_fixed);
    if (battery_info->charge_anim_start_idx != start_idx) {
        battery_info->charge_anim_index = 0;
        battery_info->charge_anim_start_idx = start_idx;
        battery_info->charge_anim_start_time = get_timestamp();
    }

    if (!is_fixed &&
        get_timestamp_ms(battery_info->charge_anim_start_time) >= BATTERY_CHARGE_ANIM_INTERVAL_MS) {
        const uint8_t count = BATTERY_CHARGE_ICON_COUNT - start_idx;
        battery_info->charge_anim_index = (battery_info->charge_anim_index + 1) % count;
        battery_info->charge_anim_start_time = get_timestamp();
    }

    const res_theme_type_e theme_type = battery_info->charge_res_theme_type;
    const uint8_t icon_idx = battery_ui_charge_icon_idx_get(percent, battery_info->charge_anim_index);
    const uint16_t res_id = battery_ui_charge_res_id_by_index(theme_type, icon_idx);

    if (battery_info->charge_img == NULL || !lv_obj_is_valid(battery_info->charge_img)) {
        battery_info->charge_img = lv_image_create(battery_info->container);
        EXCEPTION_CHECK_VOID(battery_info->charge_img);
    }

    if (battery_info->cap_fill != NULL && lv_obj_is_valid(battery_info->cap_fill)) {
        lv_obj_add_flag(battery_info->cap_fill, LV_OBJ_FLAG_HIDDEN);
    }
    if (battery_info->frame_img != NULL && lv_obj_is_valid(battery_info->frame_img)) {
        lv_obj_add_flag(battery_info->frame_img, LV_OBJ_FLAG_HIDDEN);
    }

    if (battery_info->charge_icon_bufs[icon_idx] != NULL) {
        lv_image_set_src(battery_info->charge_img, battery_info->charge_icon_bufs[icon_idx]);
    } else {
        ui_img_baj_theme_set_src(thiz, battery_info->charge_img, res_id, theme_type,
            UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_clear_flag(battery_info->charge_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(battery_info->charge_img, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_move_foreground(battery_info->charge_img);
    battery_info->charge_res_id = res_id;
}

static void battery_ui_normal_update(ui_manager_context_t *thiz, uint8_t percent, uint8_t is_low_power)
{
    if (battery_info->charge_img != NULL && lv_obj_is_valid(battery_info->charge_img)) {
        lv_image_set_src(battery_info->charge_img, NULL);
        lv_obj_add_flag(battery_info->charge_img, LV_OBJ_FLAG_HIDDEN);
    }

    battery_ui_cap_fill_update(percent, is_low_power);
    if (battery_info->force_update || battery_info->is_low_power != is_low_power) {
        battery_ui_frame_update(thiz, is_low_power);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void battery_ui_init(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (battery_info == NULL) {
        battery_info = (battery_ui_info_t *)u_malloz(sizeof(battery_ui_info_t));
        EXCEPTION_CHECK_VOID(battery_info);
    }
    battery_info->force_update = true;
    battery_info->charge_anim_start_idx = 0xFF;
    battery_ui_charge_icons_preload(thiz);
}

void battery_ui_deinit(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (battery_info == NULL) {
        return;
    }

    if (battery_info->frame_img != NULL && lv_obj_is_valid(battery_info->frame_img)) {
        ui_img_baj_release_decoded_buf(battery_info->frame_img);
    }
    if (battery_info->eco_img != NULL && lv_obj_is_valid(battery_info->eco_img)) {
        ui_img_baj_release_decoded_buf(battery_info->eco_img);
    }
    if (battery_info->charge_img != NULL && lv_obj_is_valid(battery_info->charge_img)) {
        lv_image_set_src(battery_info->charge_img, NULL);
    }
    battery_ui_charge_icons_release();
    LVGL_OBJ_DELETE(battery_info->eco_img);
    LVGL_OBJ_DELETE(battery_info->container);
    u_free(battery_info);
    battery_info = NULL;
}

int battery_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, battery_manager_context_t *battery,
    uint8_t is_display_state)
{
    EXCEPTION_CHECK_INT(thiz && root && battery);
    (void)is_display_state;

    const uint8_t percent = battery->get_battery_percent();
    const uint8_t charge_status = battery->get_charge_status();
    const uint8_t is_charging = battery_ui_is_charging_detect(battery, charge_status);
    const uint8_t is_low_power = (percent < LOW_POWER_RUN_PERCENTAGE) ? 1 : 0;
    const uint8_t is_eco_layout = (!is_charging && percent < BATTERY_ECO_LAYOUT_PERCENTAGE) ? 1 : 0;

    if (!battery_ui_need_update(thiz, percent, is_low_power, is_eco_layout, is_charging, root)) {
        return 0;
    }

    battery_ui_charge_icons_preload(thiz);

    if (is_charging && !battery_info->is_charging) {
        battery_ui_charge_state_reset(percent);
    } else if (!is_charging && battery_info->is_charging) {
        battery_info->charge_res_id = 0;
        battery_info->force_update = true;
    }

    const uint16_t battery_x = is_eco_layout ?
        BASE_PUMP_HOME_BATTERY_UI_ECO_X : BASE_PUMP_HOME_BATTERY_UI_X;
    battery_ui_container_ensure(root, battery_x, BASE_PUMP_HOME_BATTERY_UI_Y);
    EXCEPTION_CHECK_INT(battery_info->container);

    if (is_charging) {
        battery_ui_charge_frame_update(thiz, percent);
    } else {
        battery_ui_normal_update(thiz, percent, is_low_power);
    }
    battery_ui_eco_update(thiz, is_eco_layout);

    battery_info->percent = percent;
    battery_info->is_low_power = is_low_power;
    battery_info->is_eco_layout = is_eco_layout;
    battery_info->is_charging = is_charging;
    battery_info->force_update = false;
    return 0;
}

uint16_t battery_ui_ble_x_get(uint16_t default_x)
{
    if (battery_info != NULL && battery_info->is_eco_layout) {
        if (default_x >= BASE_PUMP_HOME_BLE_UI_ECO_SHIFT_X) {
            return default_x - BASE_PUMP_HOME_BLE_UI_ECO_SHIFT_X;
        }
        return 0;
    }
    return default_x;
}
