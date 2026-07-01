/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "pump_home_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PUMP_HOME_MODE_TEXT_CMP          (0)

#if IS_SUPPORT_PUMP_HOME_MODE_TEXT_CMP
#define PUMP_HOME_MODE_TITLE_COLOR                 (0x00FF00)
#define PUMP_HOME_MODE_NAME_COLOR                  (0x00FF00)
#else
#define PUMP_HOME_MODE_TITLE_COLOR                 (0xBD4060)
#define PUMP_HOME_MODE_NAME_COLOR                  (0xD36682)
#endif

#define PUMP_HOME_MODE_TITLE_Y                     (90)
#define PUMP_HOME_RHYTHM_TITLE_Y                   (72)
#define PUMP_HOME_MODE_NAME_X                      (90)
#define PUMP_HOME_MODE_NAME_Y                      (124)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static const char *pump_home_mode_type_name_get(uint8_t mode_type)
{
    switch (mode_type) {
        default:
        case PUMP_MODE_STIMULATION:
            return "Stimulation";
        case PUMP_MODE_LACTATION:
            return "Expression";
        case PUMP_MODE_MIX:
            return "Mixed";
        case PUMP_MODE_IDLE:
            return "Relax";
    }
}

static void pump_home_mode_title_update(pump_home_custom_ui_info_t *info, const char *title, int16_t y_offset)
{
    if (info->mode.title == NULL || !lv_obj_is_valid(info->mode.title)) {
        info->mode.title = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.title);
        lv_obj_set_style_text_color(info->mode.title, lv_color_hex(PUMP_HOME_MODE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(info->mode.title, &lv_font_aeonik_soft_24_bold, 0);
    }

    if (title == NULL) {
        lv_obj_add_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(info->mode.title, title);
    lv_obj_align(info->mode.title, LV_ALIGN_TOP_MID, 0, y_offset);
}

static void pump_home_mode_name_update(pump_home_custom_ui_info_t *info, const char *name,
    uint16_t x_offset, uint16_t y_offset)
{
    if (info->mode.mode == NULL || !lv_obj_is_valid(info->mode.mode)) {
        info->mode.mode = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.mode);
        lv_obj_set_style_text_color(info->mode.mode, lv_color_hex(PUMP_HOME_MODE_NAME_COLOR), 0);
        lv_obj_set_style_text_font(info->mode.mode, &lv_font_aeonik_soft_18_medium, 0);
    }

    if (name == NULL) {
        lv_obj_add_flag(info->mode.mode, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(info->mode.mode, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(info->mode.mode, name);
    if (x_offset == 0) {
        lv_obj_align(info->mode.mode, LV_ALIGN_TOP_MID, 0, y_offset);
    } else {
        lv_obj_align(info->mode.mode, LV_ALIGN_TOP_LEFT, x_offset, y_offset);
    }
}

static void pump_home_mode_title_blink_apply(pump_home_custom_ui_info_t *info, bool blink_visible)
{
    if (info->mode.title == NULL || !lv_obj_is_valid(info->mode.title)) {
        return;
    }
    if (info->mode.mode_blink_state == 0) {
        lv_obj_clear_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    if (blink_visible) {
        lv_obj_clear_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
    }
}

static bool pump_home_mode_need_update(pump_home_custom_ui_info_t *info)
{
    if (info->force_update) {
        return true;
    }
    if (info->mode.index != info->mode.new_index) {
        return true;
    }
    if (info->mode.mode_type != info->mode.new_mode_type) {
        return true;
    }
    return false;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_home_custom_mode_ui_remove(pump_home_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
}

void pump_home_custom_mode_ui_update(pump_home_custom_ui_info_t *info, bool blink_visible)
{
    char rhythm_name[24] = {0};

    EXCEPTION_CHECK_VOID(info);
    if (pump_home_mode_need_update(info)) {
        switch (info->mode.new_index) {
            default:
            case BREAST_PUMP_STIMULATION_MODE:
                pump_home_mode_title_update(info, "Stimulation", PUMP_HOME_MODE_TITLE_Y);
                pump_home_mode_name_update(info, NULL, 0, 0);
                info->mode.is_rhythm_mode = false;
                break;
            case BREAST_PUMP_LACTATION_MODE:
                pump_home_mode_title_update(info, "Expression", PUMP_HOME_MODE_TITLE_Y);
                pump_home_mode_name_update(info, NULL, 0, 0);
                info->mode.is_rhythm_mode = false;
                break;
            case BREAST_PUMP_MIX_MODE:
                pump_home_mode_title_update(info, "Mixed", PUMP_HOME_MODE_TITLE_Y);
                pump_home_mode_name_update(info, NULL, 0, 0);
                info->mode.is_rhythm_mode = false;
                break;
            case BREAST_PUMP_MILK_BOOST_MODE:
                pump_home_mode_title_update(info,
                    pump_home_mode_type_name_get(info->mode.new_mode_type), PUMP_HOME_RHYTHM_TITLE_Y);
                pump_home_mode_name_update(info, "Milk Boost", PUMP_HOME_MODE_NAME_X, PUMP_HOME_MODE_NAME_Y);
                info->mode.is_rhythm_mode = true;
                break;
            case BREAST_PUMP_PP_MILK_TRACK_MODE:
                pump_home_mode_title_update(info,
                    pump_home_mode_type_name_get(info->mode.new_mode_type), PUMP_HOME_RHYTHM_TITLE_Y);
                pump_home_mode_name_update(info, "Power Pumping", PUMP_HOME_MODE_NAME_X, PUMP_HOME_MODE_NAME_Y);
                info->mode.is_rhythm_mode = true;
                break;
            case BREAST_PUMP_CUSTOM_MODE:
                pump_home_mode_title_update(info,
                    pump_home_mode_type_name_get(info->mode.new_mode_type), PUMP_HOME_RHYTHM_TITLE_Y);
                snprintf(rhythm_name, sizeof(rhythm_name), "Customize %s", info->mode.custom_id_str);
                pump_home_mode_name_update(info, rhythm_name, PUMP_HOME_MODE_NAME_X, PUMP_HOME_MODE_NAME_Y);
                info->mode.is_rhythm_mode = true;
                break;
        }
        info->mode.index = info->mode.new_index;
        info->mode.mode_type = info->mode.new_mode_type;
    }
    pump_home_mode_title_blink_apply(info, blink_visible);
}
