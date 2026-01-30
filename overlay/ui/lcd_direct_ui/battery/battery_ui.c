/*
* INCLUDE FILES
*****************************************************************************************
*/
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
#define HOME_CHARGE_BATTERY_REFRESH_TIME           (1000)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static uint16_t home_battery_ids[HOME_BATTERY_MAX_COUNT] = {
    RESOURCE_ID_HOME_EMPTY_POWER,
    RESOURCE_ID_HOME_LOW_POWER,
    RESOURCE_ID_HOME_UNCHARGE_BATTERY_25,
    RESOURCE_ID_HOME_UNCHARGE_BATTERY_50,
    RESOURCE_ID_HOME_UNCHARGE_BATTERY_75,
    RESOURCE_ID_HOME_UNCHARGE_BATTERY_100,
};

const static uint16_t home_charge_battery_ids[HOME_CHARGE_BATTERY_MAX_COUNT] = {
    RESOURCE_ID_HOME_LOW_POWER,
    RESOURCE_ID_HOME_CHARGE_25,
    RESOURCE_ID_HOME_CHARGE_50,
    RESOURCE_ID_HOME_CHARGE_75,
    RESOURCE_ID_HOME_CHARGE_100,
};

static lv_obj_t * battery_icon_img;
static res_img_info_t battery_icon_info;
static uint32_t animation_refresh_time = 0;
static uint8_t page_index = 0;
static uint8_t s_is_charging = 0;
static uint8_t s_battery_percent = 0;
static uint8_t s_force_update = 0;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint8_t get_battery_img_id(uint8_t battery_percent)
{
    if (battery_percent >= 75) {
        return RESOURCE_ID_HOME_UNCHARGE_BATTERY_100;
    } else if (battery_percent >= 50) {
        return RESOURCE_ID_HOME_UNCHARGE_BATTERY_75;
    } else if (battery_percent >= 25) {
        return RESOURCE_ID_HOME_UNCHARGE_BATTERY_50;
    } else if (battery_percent >= 10) {
        return RESOURCE_ID_HOME_UNCHARGE_BATTERY_25;
    } else if (battery_percent >= 2) {
        return RESOURCE_ID_HOME_LOW_POWER;
    }
    return RESOURCE_ID_HOME_EMPTY_POWER;
}

static uint8_t get_charge_battery_img_id(uint8_t battery_percent)
{
    uint8_t result = RESOURCE_ID_HOME_EMPTY_POWER;
    if (battery_percent >= 100) {
        result = RESOURCE_ID_HOME_CHARGE_100;
    } else if (battery_percent >= 75) {
        if (page_index == 0) {
            result = RESOURCE_ID_HOME_CHARGE_75;
            page_index = 1;
        } else {
            result = RESOURCE_ID_HOME_CHARGE_100;
            page_index = 0;
        }
    } else if (battery_percent >= 50) {
        if (page_index == 0) {
            result = RESOURCE_ID_HOME_CHARGE_50;
            page_index = 1;
        } else if (page_index == 1) {
            result = RESOURCE_ID_HOME_CHARGE_75;
            page_index = 2;
        } else {
            result = RESOURCE_ID_HOME_CHARGE_100;
            page_index = 0;
        }
    } else if (battery_percent >= 25) {
        if (page_index == 0) {
            result = RESOURCE_ID_HOME_CHARGE_25;
            page_index = 1;
        } else if (page_index == 1) {
            result = RESOURCE_ID_HOME_CHARGE_50;
            page_index = 2;
        } else if (page_index == 2) {
            result = RESOURCE_ID_HOME_CHARGE_75;
            page_index = 3;
        } else {
            result = RESOURCE_ID_HOME_CHARGE_100;
            page_index = 0;
        }
    } else {
        if (page_index == 0) {
            result = RESOURCE_ID_HOME_EMPTY_POWER;
            page_index = 1;
        } else if (page_index == 1) {
            result = RESOURCE_ID_HOME_CHARGE_25;
            page_index = 2;
        } else if (page_index == 2) {
            result = RESOURCE_ID_HOME_CHARGE_50;
            page_index = 3;
        } else if (page_index == 3) {
            result = RESOURCE_ID_HOME_CHARGE_75;
            page_index = 4;
        } else {
            result = RESOURCE_ID_HOME_CHARGE_100;
            page_index = 0;
        }
    }
    return result;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void battery_icon_init(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    animation_refresh_time = 0;
    page_index = 0;
    s_is_charging = 0;
    s_battery_percent = 255;
    s_force_update = true;
}

int battery_icon_update(ui_manager_context_t *thiz, lv_obj_t *root, battery_manager_context_t *battery)
{
    EXCEPTION_CHECK_INT(battery);
    uint8_t battery_percent = battery->get_battery_percent();
    uint8_t is_charging = BATTERY_IS_CHARGING(battery->get_charge_status());
    uint8_t battery_img_id = RESOURCE_ID_HOME_UNCHARGE_BATTERY_50;
    if (is_charging == 0) {
        if (battery_percent < 10) {
            if (get_timestamp() - animation_refresh_time >= HOME_CHARGE_BATTERY_REFRESH_TIME || s_is_charging != is_charging) {
                battery_img_id = page_index == 0 ? RESOURCE_ID_HOME_EMPTY_POWER : RESOURCE_ID_HOME_LOW_POWER;
                page_index = page_index == 0 ? 1 : 0;
                common_ui_img_resource_init(thiz, battery_img_id, &battery_icon_info);
            } else {
                return 0;
            }
        } else {
            if (s_battery_percent != battery_percent || s_is_charging != is_charging || s_force_update == true) {
                battery_img_id = get_battery_img_id(battery_percent);
                common_ui_img_resource_init(thiz, battery_img_id, &battery_icon_info);
            } else {
                return 0;
            }
        }
    } else {
        if (get_timestamp() - animation_refresh_time >= HOME_CHARGE_BATTERY_REFRESH_TIME || s_is_charging != is_charging) {
            battery_img_id = get_charge_battery_img_id(battery_percent);
            common_ui_img_resource_init(thiz, battery_img_id, &battery_icon_info);
        } else {
            return 0;
        }
    }
    EXCEPTION_CHECK_INT(battery_icon_info.data);
    if (battery_icon_img == NULL || !lv_obj_is_valid(battery_icon_img)) {
        battery_icon_img = lv_image_create(root);
        EXCEPTION_CHECK_INT(battery_icon_img);
    }
    lv_fs_path_ex_t battery_icon_img_path;
    lv_fs_make_path_from_buffer(&battery_icon_img_path, LV_FS_MEMFS_LETTER, battery_icon_info.data,
        battery_icon_info.size, "baj");
    lv_image_set_src(battery_icon_img, (const void *)&battery_icon_img_path);
    lv_obj_align(battery_icon_img, LV_ALIGN_TOP_LEFT, 185 + 5, 40);
    s_battery_percent = battery_percent;
    s_is_charging = is_charging;
    animation_refresh_time = get_timestamp();
    s_force_update = false;
    return 0;
}

void battery_icon_deinit(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    battery_icon_img = NULL;
    animation_refresh_time = 0;
    page_index = 0;
    s_is_charging = 0;
    s_battery_percent = 0;
    s_force_update = 0;
    common_ui_img_resource_deinit(thiz, &battery_icon_info);
}
