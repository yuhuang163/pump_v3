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
#define HOME_MODE_TITLE_COLOR                      (0xBD4060)
#define HOME_MODE_NAME_COLOR                       (0xD36682)
#define HOME_MODE_WAVE_Y                           (112)
#define HOME_MODE_TITLE_Y                          (70)
#define HOME_MODE_NAME_Y                           (158)

#define HOME_MODE_ICON_WIDTH                       (32)
#define HOME_MODE_ICON_TITLE_GAP                   (6)
#define HOME_MODE_ICON_Y                           (66)


/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static void home_pump_mode_idle_wave_update(home_base_ui_info_t *info, uint16_t resource_id)
{
    if (info->mode.wave == NULL || !lv_obj_is_valid(info->mode.wave)) {
        info->mode.wave = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.wave);
        lv_obj_align(info->mode.wave, LV_ALIGN_TOP_MID, 0, HOME_MODE_WAVE_Y); /// 设置wave位置
    }
    if (resource_id == 0) {
        ui_img_baj_release_decoded_buf(info->mode.wave);
        lv_obj_add_flag(info->mode.wave, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_clear_flag(info->mode.wave, LV_OBJ_FLAG_HIDDEN);
    ui_img_baj_theme_set_src(info->ui, info->mode.wave, resource_id,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
}

static void home_pump_mode_title_update(home_base_ui_info_t *info, const char *title,
    uint16_t icon_resource_id)
{
    if (info->mode.title == NULL || !lv_obj_is_valid(info->mode.title)) {
        info->mode.title = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.title);
        lv_obj_set_style_text_color(info->mode.title, lv_color_hex(HOME_MODE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(info->mode.title, &lv_font_aeonik_soft_24_bold, 0);
    }

    if (title == NULL) {
        lv_obj_add_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
        if (info->mode.icon != NULL && lv_obj_is_valid(info->mode.icon)) {
            ui_img_baj_release_decoded_buf(info->mode.icon);
            lv_obj_add_flag(info->mode.icon, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    lv_obj_clear_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(info->mode.title, title);

    if (icon_resource_id == 0) {
        if (info->mode.icon != NULL && lv_obj_is_valid(info->mode.icon)) {
            ui_img_baj_release_decoded_buf(info->mode.icon);
            lv_obj_add_flag(info->mode.icon, LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_align(info->mode.title, LV_ALIGN_TOP_MID, 0, HOME_MODE_TITLE_Y);
        return;
    }

    if (info->mode.icon == NULL || !lv_obj_is_valid(info->mode.icon)) {
        info->mode.icon = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.icon);
    }
    lv_obj_clear_flag(info->mode.icon, LV_OBJ_FLAG_HIDDEN);
    ui_img_baj_theme_set_src(info->ui, info->mode.icon, icon_resource_id,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);

    lv_obj_update_layout(info->mode.title);
    lv_coord_t title_width = lv_obj_get_width(info->mode.title);
    lv_coord_t screen_width = lv_obj_get_width(info->root);
    lv_coord_t total_width = HOME_MODE_ICON_WIDTH + HOME_MODE_ICON_TITLE_GAP + title_width;
    lv_coord_t icon_x = (screen_width - total_width) / 2;
    lv_coord_t title_x = icon_x + HOME_MODE_ICON_WIDTH + HOME_MODE_ICON_TITLE_GAP;

    lv_obj_align(info->mode.icon, LV_ALIGN_TOP_LEFT, icon_x, HOME_MODE_ICON_Y);
    lv_obj_align(info->mode.title, LV_ALIGN_TOP_LEFT, title_x, HOME_MODE_TITLE_Y);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_pump_mode_ui_remove(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_baj_release_decoded_buf(info->mode.wave);
    ui_img_baj_release_decoded_buf(info->mode.icon);
}

void home_pump_mode_ui_update(home_base_ui_info_t *info)
{
    if (info->mode.index == info->mode.new_index && info->force_update == false) {
        return;
    }
    // info->mode.new_index = BREAST_PUMP_LACTATION_MODE;
    switch (info->mode.new_index) {
        default:
        case BREAST_PUMP_STIMULATION_MODE: {
            home_pump_mode_idle_wave_update(info, BASE_RES_ID_STIMULATION_WAVE);
            home_pump_mode_title_update(info, "STIMULATION", 0);
            break;
        }
        case BREAST_PUMP_LACTATION_MODE: {
            home_pump_mode_idle_wave_update(info, BASE_RES_ID_LACTATION_WAVE);
            home_pump_mode_title_update(info, "EXPRESSION", 0);
            break;
        }
        case BREAST_PUMP_MIX_MODE: {
            home_pump_mode_idle_wave_update(info, BASE_RES_ID_MIX_WAVE);
            home_pump_mode_title_update(info, "MIXED", 0);
            break;
        }
        case BREAST_PUMP_CUSTOM_MODE: {
            home_pump_mode_idle_wave_update(info, 0);
            char custom_str[24] = {0};
            sprintf(custom_str, "Customized %02d", info->mode.custom_mode_id);
            home_pump_mode_title_update(info, custom_str, BASE_RES_ID_PUMP_RHYTHM_ICON);
            break;
        }
        case BREAST_PUMP_MILK_BOOST_MODE:
            home_pump_mode_idle_wave_update(info, BASE_RES_ID_MILK_BOOST_WAVE);
            home_pump_mode_title_update(info, "Milk Boost", BASE_RES_ID_PUMP_RHYTHM_ICON);
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            home_pump_mode_idle_wave_update(info, BASE_RES_ID_POWER_PUMPING_WAVE);
            home_pump_mode_title_update(info, "Power Pumping", BASE_RES_ID_PUMP_RHYTHM_ICON);
            break;
    }
    info->mode.index = info->mode.new_index;
}
