/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "home_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_HOME_MODE_TEXT_CMP              (0)

#if IS_SUPPORT_HOME_MODE_TEXT_CMP
#define HOME_MODE_TITLE_COLOR                      (0x00FF00)
#define HOME_MODE_NAME_COLOR                       (0x00FF00)
#define HOME_MODE_CMP_INDEX                        BREAST_PUMP_MIX_MODE
#else
#define HOME_MODE_TITLE_COLOR                      (0xBD4060)
#define HOME_MODE_NAME_COLOR                       (0xD36682)
#endif

#define HOME_MODE_TITLE_Y                          (72)
#define HOME_MODE_NAME_Y                           (158)

#define HOME_MODE_ICON_WIDTH                       (32)
#define HOME_MODE_ICON_TITLE_GAP                   (4)
#define HOME_MODE_ICON_Y                           (66)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static void home_pump_mode_title_update(home_custom_ui_info_t *info, const char *title,
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
        RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);

    lv_obj_update_layout(info->mode.title);
    lv_coord_t title_width = lv_obj_get_width(info->mode.title);
    lv_coord_t screen_width = lv_obj_get_width(info->root);
    lv_coord_t total_width = HOME_MODE_ICON_WIDTH + HOME_MODE_ICON_TITLE_GAP + title_width;
    lv_coord_t icon_x = (screen_width - total_width) / 2;
    lv_coord_t title_x = icon_x + HOME_MODE_ICON_WIDTH + HOME_MODE_ICON_TITLE_GAP;

    lv_obj_align(info->mode.icon, LV_ALIGN_TOP_LEFT, icon_x, HOME_MODE_ICON_Y);
    lv_obj_align(info->mode.title, LV_ALIGN_TOP_LEFT, title_x, HOME_MODE_TITLE_Y);
}

static void home_pump_mode_obj_visible_set(lv_obj_t *obj, bool visible)
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

static void home_pump_mode_title_blink_apply(home_custom_ui_info_t *info, bool blink_visible)
{
    const bool visible = (info->mode.mode_blink_state == 0) || blink_visible;
    home_pump_mode_obj_visible_set(info->mode.title, visible);
}

static void home_pump_mode_icon_blink_apply(home_custom_ui_info_t *info, bool blink_visible)
{
    if (info->mode.icon == NULL || !lv_obj_is_valid(info->mode.icon)) {
        return;
    }
    const bool visible = (info->mode.icon_blink_state == 0) || blink_visible;
    home_pump_mode_obj_visible_set(info->mode.icon, visible);
}

static bool home_pump_mode_need_update(home_custom_ui_info_t *info)
{
    if (info->force_update) {
        return true;
    }
    return info->mode.index != info->mode.new_index;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_custom_pump_mode_ui_remove(home_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_baj_release_decoded_buf(info->mode.icon);
}

void home_custom_pump_mode_ui_update(home_custom_ui_info_t *info, bool blink_visible)
{
    #if IS_SUPPORT_HOME_MODE_TEXT_CMP
    info->mode.new_index = HOME_MODE_CMP_INDEX; 
    #endif
    if (home_pump_mode_need_update(info)) {
        switch (info->mode.new_index) {
            default:
            case BREAST_PUMP_STIMULATION_MODE: {
                home_pump_mode_title_update(info, "Stimulation", 0);
                break;
            }
            case BREAST_PUMP_LACTATION_MODE: {
                home_pump_mode_title_update(info, "Expression", 0);
                break;
            }
            case BREAST_PUMP_MIX_MODE: {
                home_pump_mode_title_update(info, "Mixed", 0);
                break;
            }
            case BREAST_PUMP_CUSTOM_MODE: {
                char custom_str[24] = {0};
                if (strlen(info->mode.custom_id_str) > 0) {
                    sprintf(custom_str, "Customized %s", info->mode.custom_id_str);
                } else {
                    sprintf(custom_str, "Customized 01");
                }
                home_pump_mode_title_update(info, custom_str, CUSTOM_RES_ID_PUMP_RHYTHM_ICON);
                break;
            }
            case BREAST_PUMP_MILK_BOOST_MODE:
                home_pump_mode_title_update(info, "Milk Boost", CUSTOM_RES_ID_PUMP_RHYTHM_ICON);
                break;
            case BREAST_PUMP_PP_MILK_TRACK_MODE:
                home_pump_mode_title_update(info, "Power Pumping", CUSTOM_RES_ID_PUMP_RHYTHM_ICON);
                break;
        }
        info->mode.index = info->mode.new_index;
    }
    home_pump_mode_title_blink_apply(info, blink_visible);
    home_pump_mode_icon_blink_apply(info, blink_visible);
}
