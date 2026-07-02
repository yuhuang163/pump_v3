/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "battery_ui.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define BATTERY_FRAME_W                                    (27)
#define BATTERY_FRAME_H                                    (14)
#define BATTERY_CAP_MAX_W                                  (19)
#define BATTERY_CAP_H                                      (10)
#define BATTERY_CAP_X_OFFSET                               (3)
#define BATTERY_CAP_Y_OFFSET                               (2)
#define BATTERY_CAP_COLOR_NORMAL                           (0xFFFFFF)
#define BATTERY_CAP_COLOR_WARNING                          (0xE30509)
#define BATTERY_CAP_RADIUS                                 (2)

typedef struct {
    lv_obj_t *root;
    lv_obj_t *container;
    lv_obj_t *cap_fill;
    lv_obj_t *frame_img;
    uint8_t percent;
    uint8_t is_low_power;
    uint8_t force_update;
    uint8_t rsv;
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

static void battery_ui_container_ensure(lv_obj_t *root, uint16_t x, uint16_t y)
{
    if (battery_info->container != NULL && lv_obj_is_valid(battery_info->container) &&
        battery_info->root == root) {
        battery_ui_align_set(battery_info->container, x, y);
        return;
    }

    battery_info->root = root;
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
    lv_obj_align(battery_info->frame_img, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_move_foreground(battery_info->frame_img);
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
    const uint8_t is_low_power = (percent < LOW_POWER_RUN_PERCENTAGE) ? 1 : 0;

    if (battery_info->force_update == false && battery_info->percent == percent &&
        battery_info->is_low_power == is_low_power && battery_info->root == root) {
        return 0;
    }

    battery_ui_container_ensure(root, BASE_PUMP_HOME_BATTERY_UI_X, BASE_PUMP_HOME_BATTERY_UI_Y);
    EXCEPTION_CHECK_INT(battery_info->container);
    battery_ui_cap_fill_update(percent, is_low_power);
    if (battery_info->force_update || battery_info->is_low_power != is_low_power) {
        battery_ui_frame_update(thiz, is_low_power);
    }

    battery_info->percent = percent;
    battery_info->is_low_power = is_low_power;
    battery_info->force_update = false;
    return 0;
}
