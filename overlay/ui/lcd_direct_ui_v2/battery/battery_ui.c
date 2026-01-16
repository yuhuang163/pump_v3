/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "resource_id_define.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_BATTERY_MAX_COUNT                     (6)
#define HOME_CHARGE_BATTERY_MAX_COUNT              (5)
#define HOME_BATTERY_TEXT_COLOR                    (0xB25962)
#define HOME_BATTERY_TEXT_RING_SIZE                (310 - 4)
#define HOME_BATTERY_TEXT_RING_R                   (HOME_BATTERY_TEXT_RING_SIZE / 2)
#define HOME_BATTERY_TEXT_ANGLE_DEG                (285)

typedef struct {    
    lv_obj_t * text;
    lv_obj_t * icon_img;
    res_img_info_t icon_info;
    lv_obj_t * charge_icon;
    res_img_info_t charge_icon_info;
    uint16_t text_angle_deg;
} battery_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static battery_display_info_t *battery_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int get_battery_img_id(uint8_t battery_percent)
{
    if (battery_percent >= 80) {
        return RESOURCE_ID_HOME_BATTERY_100;
    } else if (battery_percent >= 60) {
        return RESOURCE_ID_HOME_BATTERY_80;
    } else if (battery_percent >= 40) {
        return RESOURCE_ID_HOME_BATTERY_60;
    } else if (battery_percent >= 20) {
        return RESOURCE_ID_HOME_BATTERY_40;
    } else if (battery_percent >= 2) {
        return RESOURCE_ID_HOME_BATTERY_20;
    }
    return RESOURCE_ID_HOME_BATTERY_0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int battery_icon_update(ui_manager_context_t *thiz, lv_obj_t *root, uint8_t percent, uint8_t is_charging)
{
    EXCEPTION_CHECK_INT(battery_info);
    if (battery_info->icon_img == NULL || !lv_obj_is_valid(battery_info->icon_img)) {
        battery_info->icon_img = lv_image_create(root);
        EXCEPTION_CHECK_INT(battery_info->icon_img);
        lv_obj_align(battery_info->icon_img, LV_ALIGN_TOP_LEFT, 156 + 3, 22);
    }
    common_ui_img_resource_init(thiz, get_battery_img_id(percent), &battery_info->icon_info);
    EXCEPTION_CHECK_INT(battery_info->icon_info.data);
    lv_fs_path_ex_t battery_icon_img_path;
    lv_fs_make_path_from_buffer(&battery_icon_img_path, LV_FS_MEMFS_LETTER, battery_info->icon_info.data,
        battery_info->icon_info.size, "baj");
    lv_image_set_src(battery_info->icon_img, (const void *)&battery_icon_img_path);

    if (battery_info->text == NULL || !lv_obj_is_valid(battery_info->text)) {
        battery_info->text = lv_label_create(root);
        EXCEPTION_CHECK_INT(battery_info->text);
        lv_obj_set_style_text_color(battery_info->text, lv_color_hex(HOME_BATTERY_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(battery_info->text, &lv_font_oppo_sans_14_medium, 0);
    }
    
    uint16_t angle_deg = 0, ring_r = 0;
    if (percent < 10) {
        angle_deg = HOME_BATTERY_TEXT_ANGLE_DEG;
        ring_r = HOME_BATTERY_TEXT_RING_R;
    } else if (percent < 100) {
        angle_deg = HOME_BATTERY_TEXT_ANGLE_DEG + 3;
        ring_r = HOME_BATTERY_TEXT_RING_R + 1;
    } else {
        angle_deg = HOME_BATTERY_TEXT_ANGLE_DEG + 4;
        ring_r = HOME_BATTERY_TEXT_RING_R + 2;
    }

    if (angle_deg != battery_info->text_angle_deg) {
        battery_info->text_angle_deg = angle_deg;
        common_ui_text_place_in_circle(battery_info->text, 
            battery_info->text_angle_deg, ring_r, false);
    }
    
    char second_value[8] = {0};
    sprintf(second_value, "%d%%", percent);
    lv_label_set_text(battery_info->text, second_value);

    if (battery_info->charge_icon == NULL || !lv_obj_is_valid(battery_info->charge_icon)) {
        battery_info->charge_icon = lv_image_create(root);
        EXCEPTION_CHECK_INT(battery_info->charge_icon);
        lv_obj_align(battery_info->charge_icon, LV_ALIGN_TOP_LEFT, 140 + 3, 27);
    }
    common_ui_img_resource_init(thiz, 
        is_charging > 0 ? RESOURCE_ID_HOME_CHARGING : RESOURCE_ID_HOME_UNCHARGE, &battery_info->charge_icon_info);
    EXCEPTION_CHECK_INT(battery_info->charge_icon_info.data);
    lv_fs_path_ex_t charge_icon_img_path;
    lv_fs_make_path_from_buffer(&charge_icon_img_path, LV_FS_MEMFS_LETTER, battery_info->charge_icon_info.data,
        battery_info->charge_icon_info.size, "baj");
    lv_image_set_src(battery_info->charge_icon, (const void *)&charge_icon_img_path);

    return 0;
}

void battery_icon_init(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (battery_info == NULL) {
        battery_info = (battery_display_info_t *)u_malloz(sizeof(battery_display_info_t));
        EXCEPTION_CHECK_VOID(battery_info);
    }
}

void battery_icon_deinit(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    common_ui_img_resource_deinit(thiz, &battery_info->icon_info);
    common_ui_img_resource_deinit(thiz, &battery_info->charge_icon_info);
    u_free(battery_info);
    battery_info = NULL;
}

void ui_display_charge_screen_prepare(ui_manager_context_t *thiz)
{

}

void ui_display_charge_screen_remove(ui_manager_context_t *thiz)
{

}

int ui_display_charge_screen(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_UINT(thiz);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    return 0;
}