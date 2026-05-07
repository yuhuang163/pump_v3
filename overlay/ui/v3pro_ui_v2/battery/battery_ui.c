/*
* INCLUDE FILES
*****************************************************************************************
*/
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

typedef enum {
    UI_BATTERY_STATE_NONE = 0,
    UI_BATTERY_STATE_CHARGING = 1,
    UI_BATTERY_STATE_LOW_POWER = 2,
} ui_battery_state_e;

typedef struct {    
    lv_obj_t * cap_icon;
    res_img_info_t cap_icon_info;
    lv_obj_t * state_icon;
    res_img_info_t stat_icon_info;
    uint16_t img_id;
    uint8_t status;
    uint8_t force_update;
    uint8_t animation_index;
    uint32_t start_time;
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
        return RESOURCE_ID_BATT_CAP_100_ICON;
    } else if (battery_percent >= 60) {
        return RESOURCE_ID_BATT_CAP_80_ICON;
    } else if (battery_percent >= 40) {
        return RESOURCE_ID_BATT_CAP_60_ICON;
    } else if (battery_percent >= 20) {
        return RESOURCE_ID_BATT_CAP_40_ICON;
    } else if (battery_percent >= 5) {
        return RESOURCE_ID_BATT_CAP_20_ICON;
    }
    return RESOURCE_ID_BATT_CAP_0_ICON;
}

static uint8_t get_charge_battery_img_id(uint8_t battery_percent)
{
    uint8_t result = RESOURCE_ID_BATT_CAP_0_ICON;
    if (battery_percent >= 100) {
        result = RESOURCE_ID_BATT_CAP_100_ICON;
    } else if (battery_percent >= 80) {
        if (battery_info->animation_index > 1) {
            battery_info->animation_index = 0;
        }
        result = RESOURCE_ID_BATT_CAP_80_ICON + battery_info->animation_index;
    } else if (battery_percent >= 60) {
        if (battery_info->animation_index > 2) {
            battery_info->animation_index = 0;
        }
        result = RESOURCE_ID_BATT_CAP_60_ICON + battery_info->animation_index;
    } else if (battery_percent >= 40) {
        if (battery_info->animation_index > 3) {
            battery_info->animation_index = 0;
        }
        result = RESOURCE_ID_BATT_CAP_40_ICON + battery_info->animation_index;
    } else if (battery_percent >= 20) {
        if (battery_info->animation_index > 4) {
            battery_info->animation_index = 0;
        }
        result = RESOURCE_ID_BATT_CAP_20_ICON + battery_info->animation_index;
    } else {
        if (battery_info->animation_index > 5) {
            battery_info->animation_index = 0;
        }
        result = RESOURCE_ID_BATT_CAP_0_ICON + battery_info->animation_index;
    }

    if (get_timestamp_ms(battery_info->start_time) > 1000) {
        battery_info->animation_index++;
        battery_info->start_time = get_timestamp(); 
    }
    return result;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int battery_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, battery_manager_context_t *battery, uint8_t is_display_state)
{
    EXCEPTION_CHECK_INT(battery);
    uint8_t percent = battery->get_battery_percent();
    uint8_t battery_status = UI_BATTERY_STATE_NONE;
    uint8_t charge_status = battery->get_charge_status();
    uint16_t battery_img_id = RESOURCE_ID_BATT_CAP_0_ICON;   
    if (BATTERY_IS_CHARGING(charge_status)) {
        battery_img_id = get_charge_battery_img_id(percent);
        battery_status = is_display_state > 0 ? UI_BATTERY_STATE_CHARGING : UI_BATTERY_STATE_NONE;
    } else {
        if (is_display_state > 0 && percent < LOW_POWER_RUN_PERCENTAGE) {
            battery_status = UI_BATTERY_STATE_LOW_POWER;
        }
        battery_img_id = get_battery_img_id(percent);
    }

    if (battery_info->force_update == false && battery_info->img_id == battery_img_id &&
        battery_info->status == battery_status) {
        return 0;
    }

    /// 显示电池电量
    EXCEPTION_CHECK_INT(battery_info);
    if (battery_info->cap_icon == NULL || !lv_obj_is_valid(battery_info->cap_icon)) {
        battery_info->cap_icon = lv_image_create(root);
        EXCEPTION_CHECK_INT(battery_info->cap_icon);
        lv_obj_align(battery_info->cap_icon, LV_ALIGN_TOP_LEFT, 156, 22);
    }

    common_ui_img_resource_init(thiz, battery_img_id, &battery_info->cap_icon_info);
    EXCEPTION_CHECK_INT(battery_info->cap_icon_info.data);
    lv_fs_path_ex_t battery_icon_img_path;
    lv_fs_make_path_from_buffer(&battery_icon_img_path, LV_FS_MEMFS_LETTER, battery_info->cap_icon_info.data,
        battery_info->cap_icon_info.size, "baj");
    lv_image_set_src(battery_info->cap_icon, (const void *)&battery_icon_img_path);

    /// 显示电池状态
    if (battery_info->state_icon ==  NULL || !lv_obj_is_valid(battery_info->state_icon)) {
        battery_info->state_icon = lv_image_create(root);
        EXCEPTION_CHECK_INT(battery_info->state_icon);
    }

    if (battery_status == UI_BATTERY_STATE_CHARGING) {
        lv_obj_clear_flag(battery_info->state_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(battery_info->state_icon, LV_ALIGN_TOP_LEFT, 206, 26);
        common_ui_img_resource_init(thiz, RESOURCE_ID_CHARGE_ICON, &battery_info->stat_icon_info);
    } else if (battery_status == UI_BATTERY_STATE_LOW_POWER) {
        lv_obj_clear_flag(battery_info->state_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(battery_info->state_icon, LV_ALIGN_TOP_LEFT, 206, 26);
        common_ui_img_resource_init(thiz, RESOURCE_ID_LOW_POWER_ICON, &battery_info->stat_icon_info);
    } else {
        lv_obj_add_flag(battery_info->state_icon, LV_OBJ_FLAG_HIDDEN);
        goto DISPLAY_EXIT;
    }

    EXCEPTION_CHECK_INT(battery_info->stat_icon_info.data);
    lv_fs_path_ex_t state_icon_img_path;
    lv_fs_make_path_from_buffer(&state_icon_img_path, LV_FS_MEMFS_LETTER, battery_info->stat_icon_info.data,
        battery_info->stat_icon_info.size, "baj");
    lv_image_set_src(battery_info->state_icon, (const void *)&state_icon_img_path);

DISPLAY_EXIT:    
    battery_info->force_update = false;
    battery_info->img_id = battery_img_id;
    battery_info->status = battery_status;

    return 0;
}

void battery_ui_init(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (battery_info == NULL) {
        battery_info = (battery_display_info_t *)u_malloz(sizeof(battery_display_info_t));
        EXCEPTION_CHECK_VOID(battery_info);
    }
    battery_info->img_id = RESOURCE_ID_BATT_CAP_0_ICON;
    battery_info->status = UI_BATTERY_STATE_NONE;
    battery_info->force_update = true;
}

void battery_ui_deinit(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    common_ui_img_resource_deinit(thiz, &battery_info->cap_icon_info);
    common_ui_img_resource_deinit(thiz, &battery_info->stat_icon_info);
    u_free(battery_info);
    battery_info = NULL;
}
