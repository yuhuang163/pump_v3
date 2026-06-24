/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "home_base_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_MODE_TITLE_COLOR                      (0xFFFFFF)//(0xBD4060)
#define HOME_MODE_NAME_COLOR                       (0xD36682)
#define HOME_MODE_WAVE_Y_OFFSET                    (118)
#define HOME_MODE_TITLE_Y_OFFSET                   (69)
#define HOME_MODE_NAME_Y_OFFSET                    (158)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static void home_pump_mode_idle_wave_update(home_base_ui_info_t *info, uint16_t resource_id)
{
    if (info->mode.wave == NULL || !lv_obj_is_valid(info->mode.wave)) {
        info->mode.wave = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.wave);
        lv_obj_align(info->mode.wave, LV_ALIGN_TOP_MID, 0, HOME_MODE_WAVE_Y_OFFSET); /// 设置wave位置
    }
    ui_img_baj_theme_set_src(info->ui, info->mode.wave, resource_id,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
}

static void home_pump_mode_title_update(home_base_ui_info_t *info, const char *title, uint16_t x_offset)
{
    if (info->mode.title == NULL || !lv_obj_is_valid(info->mode.title)) {
        info->mode.title = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.title);
        lv_obj_set_style_text_color(info->mode.title, lv_color_hex(HOME_MODE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(info->mode.title, &lv_font_aeonik_soft_24_bold, 0);  /// 设置字体
    }

    if (title == NULL) {
        lv_obj_add_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(info->mode.title, title);
    if (x_offset == 0) {
        lv_obj_align(info->mode.title, LV_ALIGN_TOP_MID, 0, HOME_MODE_TITLE_Y_OFFSET); /// 设置label文字位置
    } else {
        lv_obj_align(info->mode.title, LV_ALIGN_TOP_LEFT, x_offset, HOME_MODE_TITLE_Y_OFFSET); /// 设置label文字位置
    }
}

static void home_pump_mode_name_update(home_base_ui_info_t *info, const char *name)
{
    if (info->mode.name == NULL || !lv_obj_is_valid(info->mode.name)) {
        info->mode.name = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.name);
        lv_obj_set_style_text_color(info->mode.name, lv_color_hex(HOME_MODE_NAME_COLOR), 0);
        lv_obj_set_style_text_font(info->mode.name, &lv_font_aeonik_soft_18_medium, 0);  /// 设置字体
    } 

    if (name == NULL) {
        lv_obj_add_flag(info->mode.name, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_clear_flag(info->mode.name, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(info->mode.name, name);
    lv_obj_align(info->mode.name, LV_ALIGN_TOP_MID, 0, 158); /// 设置label文字位置
}

static void home_pump_mode_icon_update(home_base_ui_info_t *info, uint16_t resource_id)
{
    if (info->mode.icon == NULL || !lv_obj_is_valid(info->mode.icon)) {
        info->mode.icon = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.icon);
        lv_obj_align(info->mode.icon, LV_ALIGN_LEFT_MID, 81, 72); /// 设置icon位置
    }

    if (resource_id == 0) {
        ui_img_baj_release_decoded_buf(info->mode.icon);
        lv_obj_add_flag(info->mode.icon, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_clear_flag(info->mode.icon, LV_OBJ_FLAG_HIDDEN);
    ui_img_baj_theme_set_src(info->ui, info->mode.icon, resource_id,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
}

static void home_pump_mode_time_update(home_base_ui_info_t *info, 
    const char *time, uint16_t x_offset, uint16_t y_offset)
{
    if (info->mode.time == NULL || !lv_obj_is_valid(info->mode.time)) {
        info->mode.time = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.time);
    } 
    if (time == NULL) {
        lv_obj_add_flag(info->mode.time, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_clear_flag(info->mode.time, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(info->mode.time, time);
    if (x_offset == 0) {
        lv_obj_align(info->mode.time, LV_ALIGN_TOP_MID, 0, y_offset); /// 设置label文字位置
    } else {
        lv_obj_align(info->mode.time, LV_ALIGN_TOP_LEFT, x_offset, y_offset); /// 设置label文字位置
    }
}

static void home_pump_mode_wave_animation_update(home_base_ui_info_t *info, 
    uint16_t start_index, uint16_t animation_num)
{



}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_pump_mode_ui_deinit(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_baj_release_decoded_buf(info->mode.wave);
    ui_img_baj_release_decoded_buf(info->mode.icon);
}

void home_pump_mode_working_info_update(home_base_ui_info_t *info)
{
    home_pump_mode_wave_animation_update(info, 0, 0);

    if (info->mode.index == info->mode.new_index && info->force_update == false) {
        return;
    }

    switch (info->mode.index) {
        default:
        case BREAST_PUMP_STIMULATION_MODE:

            break;
        case BREAST_PUMP_LACTATION_MODE:

            break;
        case BREAST_PUMP_MIX_MODE:

            break;
        case BREAST_PUMP_CUSTOM_MODE:

            break;
        case BREAST_PUMP_MILK_BOOST_MODE:

            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:

            break;
    }
}

void home_pump_mode_idle_info_update(home_base_ui_info_t *info)
{
    if (info->mode.index == info->mode.new_index && info->force_update == false) {
        return;
    }

    switch (info->mode.new_index) {
        default:
        case BREAST_PUMP_STIMULATION_MODE: {
            home_pump_mode_idle_wave_update(info, BASE_RES_ID_STIMULATION_WAVE);
            home_pump_mode_title_update(info, "STIMULATION", 0);
            home_pump_mode_name_update(info, NULL);
            home_pump_mode_time_update(info, NULL, 0, 0);
            break;
        }
        case BREAST_PUMP_LACTATION_MODE: {
            home_pump_mode_idle_wave_update(info, BASE_RES_ID_LACTATION_WAVE);
            home_pump_mode_title_update(info, "EXPRESSION", 0);
            home_pump_mode_name_update(info, NULL);
            home_pump_mode_time_update(info, NULL, 0, 0);
            break;
        }
        case BREAST_PUMP_MIX_MODE: {
            home_pump_mode_idle_wave_update(info, BASE_RES_ID_MIX_WAVE);
            home_pump_mode_title_update(info, "MIXED", 0);
            home_pump_mode_name_update(info, NULL);
            home_pump_mode_time_update(info, NULL, 0, 0);
            break;
        }
        case BREAST_PUMP_CUSTOM_MODE: {
            home_pump_mode_idle_wave_update(info, BASE_RES_ID_CUSTOM_WAVE);   
            char custom_str[24] = {0};
            sprintf(custom_str, "Customized %02d", info->mode.custom_mode_id);
            home_pump_mode_title_update(info, custom_str, 139);
            home_pump_mode_name_update(info, NULL);
            int custom_mode_time = 0;
            info->st->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, BREAST_PUMP_CUSTOM_MODE, 0, &custom_mode_time);
            memset(custom_str, 0, sizeof(custom_str));
            sprintf(custom_str, "≈%dmin", custom_mode_time / 60);
            home_pump_mode_time_update(info, custom_str, 0, 121);
            break;
        }
        case BREAST_PUMP_MILK_BOOST_MODE:
            home_pump_mode_idle_wave_update(info, BASE_RES_ID_MILK_BOOST_WAVE);
            home_pump_mode_title_update(info, "Milk Boost", 139);
            home_pump_mode_name_update(info, NULL);
            home_pump_mode_time_update(info, NULL, 0, 0);
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            home_pump_mode_idle_wave_update(info, BASE_RES_ID_POWER_PUMPING_WAVE);
            home_pump_mode_title_update(info, "Power Pumping", 139);
            home_pump_mode_name_update(info, NULL);
            home_pump_mode_time_update(info, NULL, 0, 0);
            break;
    }
    info->mode.index = info->mode.new_index;
}
