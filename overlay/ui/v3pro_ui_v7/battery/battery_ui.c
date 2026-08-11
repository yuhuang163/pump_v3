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
#define BATTERY_FRAME_W                                    (30)
#define BATTERY_FRAME_H                                    (20)
#define BATTERY_CAP_MAX_W                                  (18)
#define BATTERY_CAP_H                                      (12)
#define BATTERY_CAP_X_OFFSET                               (4)
#define BATTERY_CAP_Y_OFFSET                               (4)
#define BATTERY_CAP_COLOR_NORMAL                           (0xFFFFFF)
#define BATTERY_CAP_COLOR_CHARGING                         (0x3DAD77)
#define BATTERY_CAP_COLOR_WARNING                          (0xEF4C4E)
#define BATTERY_CAP_RADIUS                                 (2)
#define BATTERY_CAP_WIDTH_BY_SEGMENT                       (0)
#define BATTERY_CAP_SEGMENT_COUNT                          (5)

typedef struct {
    lv_obj_t *root;
    lv_obj_t *container;
    lv_obj_t *cap_fill;
    lv_obj_t *frame_img;
    lv_obj_t *charge_img;
    lv_obj_t *eco_img;
    uint16_t frame_res_id;
    uint16_t battery_x;
    uint16_t battery_y;
    uint16_t eco_x;
    uint16_t ble_x;
    uint16_t ble_y;
    uint8_t percent;
    uint8_t is_low_power;
    uint8_t is_eco_layout;
    uint8_t is_charging;
    uint8_t is_ble_connected;
    uint8_t charge_icon_loaded;
    uint8_t force_update;
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
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, x, y);
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

static uint16_t battery_ui_frame_res_id_get(uint8_t is_charging, uint8_t is_low_power)
{
    if (is_charging) {
        return BASE_RES_ID_BATTERY_BOX_GREEN_ICON;
    }
    if (is_low_power) {
        return BASE_RES_ID_BATTERY_BOX_RED_ICON;
    }
    return BASE_RES_ID_BATTERY_BOX_WHITE_ICON;
}

static void battery_ui_layout_calc(uint8_t is_ble_connected, uint8_t is_eco_layout,
    uint16_t *battery_x, uint16_t *battery_y, uint16_t *eco_x, uint16_t *ble_x, uint16_t *ble_y)
{
    const uint8_t show_ble = (is_ble_connected == 0) ? 1 : 0;
    lv_coord_t total_w = BATTERY_FRAME_W;

    if (show_ble) {
        total_w += BATTERY_UI_BLE_ICON_W + BATTERY_UI_ICON_GAP;
    }
    if (is_eco_layout) {
        total_w += BATTERY_UI_ICON_GAP + BATTERY_UI_ECO_ICON_W;
    }

    lv_coord_t start_x = (BATTERY_UI_SCREEN_W - total_w) / 2;
    if (start_x < 0) {
        start_x = 0;
    }

    uint16_t cur_x = (uint16_t)start_x;
    *ble_y = BATTERY_UI_BLE_Y;
    *ble_x = cur_x;
    if (show_ble) {
        cur_x = (uint16_t)(cur_x + BATTERY_UI_BLE_ICON_W + BATTERY_UI_ICON_GAP);
    }

    *battery_x = cur_x;
    *battery_y = (show_ble || is_eco_layout) ? BATTERY_UI_Y_WITH_PEER : BATTERY_UI_Y_ALONE;
    *eco_x = (uint16_t)(cur_x + BATTERY_FRAME_W + BATTERY_UI_ICON_GAP);
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

static bool battery_ui_need_update(uint8_t percent, uint8_t is_low_power, uint8_t is_eco_layout,
    uint8_t is_charging, uint8_t is_ble_connected, uint16_t frame_res_id, lv_obj_t *root)
{
    if (battery_info->force_update) {
        return true;
    }
    if (battery_info->root != root) {
        return true;
    }
    if (battery_info->is_charging != is_charging) {
        return true;
    }
    if (battery_info->is_ble_connected != is_ble_connected) {
        return true;
    }
    if (battery_info->percent != percent) {
        return true;
    }
    if (battery_info->is_eco_layout != is_eco_layout) {
        return true;
    }
    if (battery_info->is_low_power != is_low_power) {
        return true;
    }
    return battery_info->frame_res_id != frame_res_id;
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
    battery_info->charge_icon_loaded = 0;
    battery_info->container = lv_obj_create(root);
    EXCEPTION_CHECK_VOID(battery_info->container);
    lv_obj_set_size(battery_info->container, BATTERY_FRAME_W, BATTERY_FRAME_H);
    lv_obj_set_style_bg_opa(battery_info->container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(battery_info->container, 0, 0);
    lv_obj_set_style_pad_all(battery_info->container, 0, 0);
    lv_obj_clear_flag(battery_info->container, LV_OBJ_FLAG_SCROLLABLE);
    battery_ui_align_set(battery_info->container, x, y);
}

static void battery_ui_cap_fill_update(uint8_t percent, uint8_t is_low_power, uint8_t is_charging)
{
    const lv_coord_t fill_width = battery_ui_cap_width_get(percent);
    uint32_t fill_color = BATTERY_CAP_COLOR_NORMAL;

    if (is_charging) {
        fill_color = BATTERY_CAP_COLOR_CHARGING;
    } else if (is_low_power) {
        fill_color = BATTERY_CAP_COLOR_WARNING;
    }

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

static void battery_ui_frame_update(ui_manager_context_t *thiz, uint16_t frame_res_id)
{
    if (battery_info->frame_img == NULL || !lv_obj_is_valid(battery_info->frame_img)) {
        battery_info->frame_img = lv_image_create(battery_info->container);
        EXCEPTION_CHECK_VOID(battery_info->frame_img);
    }

    if (battery_info->force_update || battery_info->frame_res_id != frame_res_id) {
        ui_img_baj_theme_set_src(thiz, battery_info->frame_img, frame_res_id,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        battery_info->frame_res_id = frame_res_id;
    }
    lv_obj_clear_flag(battery_info->frame_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(battery_info->frame_img, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_move_foreground(battery_info->frame_img);
}

static void battery_ui_charge_icon_update(ui_manager_context_t *thiz, uint8_t is_charging)
{
    if (!is_charging) {
        if (battery_info->charge_img != NULL && lv_obj_is_valid(battery_info->charge_img)) {
            lv_obj_add_flag(battery_info->charge_img, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    if (battery_info->charge_img == NULL || !lv_obj_is_valid(battery_info->charge_img)) {
        battery_info->charge_img = lv_image_create(battery_info->container);
        EXCEPTION_CHECK_VOID(battery_info->charge_img);
        battery_info->charge_icon_loaded = 0;
    }

    if (battery_info->force_update || !battery_info->charge_icon_loaded) {
        ui_img_baj_theme_set_src(thiz, battery_info->charge_img, BASE_RES_ID_CHARGING_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        battery_info->charge_icon_loaded = 1;
    }
    lv_obj_clear_flag(battery_info->charge_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(battery_info->charge_img, LV_ALIGN_CENTER, -3, 0);
    lv_obj_move_foreground(battery_info->charge_img);
}

static void battery_ui_eco_update(ui_manager_context_t *thiz, uint8_t is_eco_layout,
    uint16_t eco_x, uint16_t eco_y)
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
    battery_ui_align_set(battery_info->eco_img, eco_x, eco_y);
    lv_obj_move_foreground(battery_info->eco_img);
}

static void battery_ui_content_update(ui_manager_context_t *thiz, uint8_t percent,
    uint8_t is_low_power, uint8_t is_charging, uint16_t frame_res_id)
{
    battery_ui_cap_fill_update(percent, is_low_power, is_charging);
    battery_ui_frame_update(thiz, frame_res_id);
    battery_ui_charge_icon_update(thiz, is_charging);
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
    battery_info->frame_res_id = 0;
    battery_info->is_ble_connected = 0xFF;
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
    if (battery_info->charge_img != NULL && lv_obj_is_valid(battery_info->charge_img)) {
        ui_img_baj_release_decoded_buf(battery_info->charge_img);
    }
    if (battery_info->eco_img != NULL && lv_obj_is_valid(battery_info->eco_img)) {
        ui_img_baj_release_decoded_buf(battery_info->eco_img);
    }
    LVGL_OBJ_DELETE(battery_info->eco_img);
    LVGL_OBJ_DELETE(battery_info->container);
    u_free(battery_info);
    battery_info = NULL;
}

int battery_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, battery_manager_context_t *battery,
    uint8_t is_display_state, uint8_t is_ble_connected)
{
    EXCEPTION_CHECK_INT(thiz && root && battery);

    if (is_display_state == 0) {
        if (battery_info != NULL && battery_info->container != NULL &&
            lv_obj_is_valid(battery_info->container)) {
            lv_obj_add_flag(battery_info->container, LV_OBJ_FLAG_HIDDEN);
            battery_info->force_update = true;
        }
        if (battery_info != NULL && battery_info->eco_img != NULL &&
            lv_obj_is_valid(battery_info->eco_img)) {
            lv_obj_add_flag(battery_info->eco_img, LV_OBJ_FLAG_HIDDEN);
        }
        return 0;
    }

    const uint8_t percent = battery->get_battery_percent();
    const uint8_t charge_status = battery->get_charge_status();
    const uint8_t is_charging = battery_ui_is_charging_detect(battery, charge_status);
    const uint8_t is_low_power = (!is_charging && percent <= LOW_POWER_RUN_PERCENTAGE) ? 1 : 0;
    const uint8_t is_eco_layout = is_low_power;
    const uint16_t frame_res_id = battery_ui_frame_res_id_get(is_charging, is_low_power);

    uint16_t battery_x = 0;
    uint16_t battery_y = 0;
    uint16_t eco_x = 0;
    uint16_t ble_x = 0;
    uint16_t ble_y = 0;
    battery_ui_layout_calc(is_ble_connected, is_eco_layout,
        &battery_x, &battery_y, &eco_x, &ble_x, &ble_y);

    if (!battery_ui_need_update(percent, is_low_power, is_eco_layout, is_charging,
        is_ble_connected, frame_res_id, root)) {
        if (battery_info->container != NULL && lv_obj_is_valid(battery_info->container)) {
            lv_obj_clear_flag(battery_info->container, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(battery_info->container);
        }
        if (is_eco_layout && battery_info->eco_img != NULL && lv_obj_is_valid(battery_info->eco_img)) {
            lv_obj_clear_flag(battery_info->eco_img, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(battery_info->eco_img);
        }
        return 0;
    }

    battery_ui_container_ensure(root, battery_x, battery_y);
    EXCEPTION_CHECK_INT(battery_info->container);

    battery_ui_content_update(thiz, percent, is_low_power, is_charging, frame_res_id);
    battery_ui_eco_update(thiz, is_eco_layout, eco_x, BATTERY_UI_ECO_Y);
    lv_obj_clear_flag(battery_info->container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(battery_info->container);

    battery_info->percent = percent;
    battery_info->is_low_power = is_low_power;
    battery_info->is_eco_layout = is_eco_layout;
    battery_info->is_charging = is_charging;
    battery_info->is_ble_connected = is_ble_connected;
    battery_info->battery_x = battery_x;
    battery_info->battery_y = battery_y;
    battery_info->eco_x = eco_x;
    battery_info->ble_x = ble_x;
    battery_info->ble_y = ble_y;
    battery_info->force_update = false;
    return 0;
}

uint16_t battery_ui_ble_x_get(uint16_t default_x)
{
    if (battery_info != NULL) {
        return battery_info->ble_x;
    }
    return default_x;
}

uint16_t battery_ui_ble_y_get(uint16_t default_y)
{
    if (battery_info != NULL) {
        return battery_info->ble_y;
    }
    return default_y;
}
