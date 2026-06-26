/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep
#include "home_base_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IDLE_LABEL_X                                (49)
#define IDLE_LABEL_Y                                (150)
#define IDLE_TEXT_X                                 (0)
#define IDLE_TEXT_Y                                 (202)
#define IDLE_BUTTON_ICON_X                          (96)
#define IDLE_BUTTON_ICON_Y                          (24)
#define IDLE_TEXT_COLOR                             (0xA0183B)

#define STIMULATION_LABEL_X                         (49)
#define STIMULATION_LABEL_Y                         (156)
#define STIMULATION_TEXT_X                          (0)
#define STIMULATION_TEXT_Y                          (208)

#define EXPRESSION_LABEL_X                          (49)
#define EXPRESSION_LABEL_Y                          (150)
#define EXPRESSION_TEXT_X                           (0)
#define EXPRESSION_TEXT_Y                           (200)

#define MIXED_LABEL_X                               (49)
#define MIXED_LABEL_Y                               (150)
#define MIXED_TEXT_X                                (0)
#define MIXED_TEXT_Y                                (200)

#define CUSTOMIZE_LABEL_X                           (49)
#define CUSTOMIZE_LABEL_Y                           (116)
#define CUSTOMIZE_TEXT_X                            (0)
#define CUSTOMIZE_TEXT_Y                            (193)
#define CUSTOMIZE_TIME_TEXT_X                       (0)
#define CUSTOMIZE_TIME_TEXT_Y                       (CUSTOMIZE_TEXT_Y - 23)

#define MILK_BOOST_LABEL_X                          (49)
#define MILK_BOOST_LABEL_Y                          (150)
#define MILK_BOOST_TEXT_X                           (0)
#define MILK_BOOST_TEXT_Y                           (227)
#define MILK_BOOST_TIME_TEXT_X                      (0)
#define MILK_BOOST_TIME_TEXT_Y                      (MILK_BOOST_TEXT_Y - 23)

#define POWER_PUMPING_LABEL_X                       (49)
#define POWER_PUMPING_LABEL_Y                       (150)
#define POWER_PUMPING_TEXT_X                        (0)
#define POWER_PUMPING_TEXT_Y                        (227)
#define POWER_PUMPING_TIME_TEXT_X                   (0)
#define POWER_PUMPING_TIME_TEXT_Y                   (POWER_PUMPING_TEXT_Y - 23)

#define IDLE_RECORD_LABEL_X                         (0)
#define IDLE_RECORD_LABEL_Y                         (106)
#define IDLE_RECORD_TEXT_X                          (0)
#define IDLE_RECORD_TEXT_Y                          (30)
#define IDLE_RECORD_TIME_TEXT_X                     (0)
#define IDLE_RECORD_TIME_TEXT_Y                     (IDLE_RECORD_LABEL_Y + 10)

typedef enum {
    HOME_BASE_TEXT_IDLE = 0,          /// 默认文本
    HOME_BASE_TEXT_RECORD,            /// 显示记录文本
    HOME_BASE_TEXT_SIMULATE_MODE_DESC, /// 显示刺激模式描述文本
    HOME_BASE_TEXT_EXPRESSION_MODE_DESC, /// 显示表达模式描述文本
    HOME_BASE_TEXT_MIXED_MODE_DESC, /// 显示混合模式描述文本
    HOME_BASE_TEXT_MILK_BOOST_DESC, /// 显示2+8奶量提升模式描述文本
    HOME_BASE_TEXT_POWER_PUMPING_DESC, /// 显示PP追奶模式描述文本
    HOME_BASE_TEXT_CUSTOMIZE_DESC, /// 显示个性化模式描述文本
    HOME_BASE_TEXT_MAX
} home_base_text_id_e;

typedef struct {
    const char *name;
    uint16_t x_offset;
    uint16_t y_offset;
    bool align_center;
} home_base_text_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

const static home_base_text_info_t home_base_label_infos[HOME_BASE_TEXT_MAX] = {
    {"Press Mode/Program\nto switch the\npumping method",
        IDLE_TEXT_X, IDLE_TEXT_Y, true},
    {"since last pumping session", 
        0, IDLE_RECORD_TEXT_Y, false},
    {"Fast, gentle suction to\ntrigger let-down",
        0, STIMULATION_TEXT_Y, true},
    {"Slow, deep suction for\nefficient milk\nexpression.", 
        0, EXPRESSION_TEXT_Y, true},
    {"Efficiently promote let-\ndown and better\nemptying.", 
        0, MIXED_TEXT_Y, true},
    {"Most suitable for daily use\nafter successful lactation",
        0, MILK_BOOST_TEXT_Y, true},
    {"Increase milk supply, no\nmore than once a day",
        0, POWER_PUMPING_TEXT_Y, true},
    {"Edit & sync via\nMomcozy App", 
        0, CUSTOMIZE_TEXT_Y, true},
};

static void home_label_bg_update(home_base_ui_info_t *info, uint16_t img_id,
    uint16_t x_offset, uint16_t y_offset)
{
    if (info->label.label == NULL || !lv_obj_is_valid(info->label.label)) {
        info->label.label = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->label.label);
        lv_obj_add_flag(info->label.label, LV_OBJ_FLAG_HIDDEN);
    }

    if (img_id == 0) {
        lv_obj_add_flag(info->label.label, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_clear_flag(info->label.label, LV_OBJ_FLAG_HIDDEN);
    ui_img_baj_theme_set_src(info->ui, info->label.label, img_id,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(info->label.label, x_offset == 0 ? LV_ALIGN_TOP_MID : LV_ALIGN_TOP_LEFT,
        x_offset, y_offset);
}

static void home_label_text_update(home_base_ui_info_t *info, const home_base_text_info_t *text_info)
{
    if (info->label.text == NULL || !lv_obj_is_valid(info->label.text)) {
        info->label.text = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->label.text);
        lv_obj_add_flag(info->label.text, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_text_line_space(info->label.text, 4, 0);
    }

    if (text_info == NULL) {
        lv_obj_add_flag(info->label.text, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(info->label.text, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(info->label.text, text_info->name);
    lv_obj_set_style_text_color(info->label.text, lv_color_hex(IDLE_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(info->label.text, &lv_font_aeonik_soft_18_medium, 0);
    if (text_info->x_offset == 0) {
        lv_obj_align(info->label.text, LV_ALIGN_TOP_MID, 0, text_info->y_offset);
    } else {
        lv_obj_align(info->label.text, LV_ALIGN_TOP_LEFT, text_info->x_offset, text_info->y_offset);
    }
    lv_obj_set_style_text_align(info->label.text,
        text_info->align_center ? LV_TEXT_ALIGN_CENTER : LV_TEXT_ALIGN_LEFT, 0);
}

static void home_label_icon_update(home_base_ui_info_t *info, uint16_t img_id,
    uint16_t x_offset, uint16_t y_offset)
{
    if (info->label.icon == NULL || !lv_obj_is_valid(info->label.icon)) {
        info->label.icon = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->label.icon);
        lv_obj_add_flag(info->label.icon, LV_OBJ_FLAG_HIDDEN);
    }

    if (img_id == 0) {
        lv_obj_add_flag(info->label.icon, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(info->label.icon, LV_OBJ_FLAG_HIDDEN);
    ui_img_baj_theme_set_src(info->ui, info->label.icon, img_id,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    if (x_offset == 0) {
        lv_obj_align(info->label.icon, LV_ALIGN_TOP_MID, 0, y_offset);
    } else {
        lv_obj_align(info->label.icon, LV_ALIGN_TOP_LEFT, x_offset, y_offset);
    }
}

static void home_label_time_update(home_base_ui_info_t *info, const char *time_text,
    uint16_t x_offset, uint16_t y_offset)
{
    if (info->label.time == NULL || !lv_obj_is_valid(info->label.time)) {
        info->label.time = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->label.time);
        lv_obj_add_flag(info->label.time, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_text_color(info->label.time, lv_color_hex(IDLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(info->label.time, &lv_font_aeonik_soft_20_medium, 0);
    }

    if (time_text == NULL) {
        lv_obj_add_flag(info->label.time, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_clear_flag(info->label.time, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(info->label.time, time_text);
    if (x_offset == 0) {
        lv_obj_align(info->label.time, LV_ALIGN_TOP_MID, 0, y_offset);
    } else {
        lv_obj_align(info->label.time, LV_ALIGN_TOP_LEFT, x_offset, y_offset);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_pump_label_ui_remove(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_baj_release_decoded_buf(info->label.label);
    ui_img_baj_release_decoded_buf(info->label.icon);
}

void home_pump_label_ui_update(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
    ui_proc_home_t *proc = (ui_proc_home_t *)info->ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (info->label.type != proc->home_display_type) {
        info->label.type = proc->home_display_type;
        info->label.force_update = true;
    }

    // info->label.type = HOME_UI_DISPLAY_EXPRESSION_MODE_DESC;
    uint32_t rtc_timestamp = platform_get_utc_time();
    switch (info->label.type) {
        default:
        case HOME_UI_DISPLAY_DEFAULT:
            home_label_bg_update(info, BASE_RES_ID_HOME_WIDTH_LABLE,
                IDLE_LABEL_X, IDLE_LABEL_Y);
            home_label_text_update(info, &home_base_label_infos[HOME_BASE_TEXT_IDLE]);
            home_label_icon_update(info, 0, 0, 0);
            home_label_time_update(info, NULL, 0, 0);
            break;
        case HOME_UI_DISPLAY_SIMULATE_MODE_DESC:{
            home_label_bg_update(info, BASE_RES_ID_HOME_NARROW_LABLE,
                STIMULATION_LABEL_X, STIMULATION_LABEL_Y);
            home_label_text_update(info, &home_base_label_infos[HOME_BASE_TEXT_SIMULATE_MODE_DESC]);
            home_label_icon_update(info, 0, 0, 0);
            home_label_time_update(info, NULL, 0, 0);
            break;
        }
        case HOME_UI_DISPLAY_EXPRESSION_MODE_DESC:{
            home_label_bg_update(info, BASE_RES_ID_HOME_WIDTH_LABLE,
                EXPRESSION_LABEL_X, EXPRESSION_LABEL_Y);
            home_label_text_update(info, &home_base_label_infos[HOME_BASE_TEXT_EXPRESSION_MODE_DESC]);
            home_label_icon_update(info, 0, 0, 0);
            home_label_time_update(info, NULL, 0, 0);
            break;
        }
        case HOME_UI_DISPLAY_MIXED_MODE_DESC:{
            home_label_bg_update(info, BASE_RES_ID_HOME_WIDTH_LABLE, MIXED_LABEL_X, MIXED_LABEL_Y);
            home_label_text_update(info, &home_base_label_infos[HOME_BASE_TEXT_MIXED_MODE_DESC]);
            home_label_icon_update(info, 0, 0, 0);
            home_label_time_update(info, NULL, 0, 0);
            break;
        }
        case HOME_UI_DISPLAY_MILK_BOOST_DESC:{
            int pump_time = 0;
            storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
            if (st) {
                st->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, BREAST_PUMP_MILK_BOOST_MODE, 0, &pump_time);
            }
            char time_text[16] = {0};
            snprintf(time_text, sizeof(time_text), "≈%d min", pump_time / 60);
            home_label_bg_update(info, BASE_RES_ID_HOME_WIDTH_LABLE, MILK_BOOST_LABEL_X, MILK_BOOST_LABEL_Y);
            home_label_text_update(info, &home_base_label_infos[HOME_BASE_TEXT_MILK_BOOST_DESC]);
            home_label_icon_update(info, 0, 0, 0);
            home_label_time_update(info, time_text, MILK_BOOST_TIME_TEXT_X, MILK_BOOST_TIME_TEXT_Y);
            break;
        }
        case HOME_UI_DISPLAY_POWER_PUMPING_DESC:{
            int pump_time = 0;
            storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
            if (st) {
                st->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, BREAST_PUMP_MILK_BOOST_MODE, 0, &pump_time);
            }
            char time_text[16] = {0};
            snprintf(time_text, sizeof(time_text), "≈%d min", pump_time / 60);
            home_label_bg_update(info, BASE_RES_ID_HOME_WIDTH_LABLE, POWER_PUMPING_LABEL_X, POWER_PUMPING_LABEL_Y);
            home_label_text_update(info, &home_base_label_infos[HOME_BASE_TEXT_POWER_PUMPING_DESC]);
            home_label_icon_update(info, 0, 0, 0);
            home_label_time_update(info, time_text, POWER_PUMPING_TIME_TEXT_X, POWER_PUMPING_TIME_TEXT_Y);
            break;
        }   
        case HOME_UI_DISPLAY_CUSTOMIZE_DESC:{
            int pump_time = 0;
            storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
            if (st) {
                st->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, BREAST_PUMP_MILK_BOOST_MODE, 0, &pump_time);
            }
            char time_text[16] = {0};
            snprintf(time_text, sizeof(time_text), "≈%d min", pump_time / 60);
            home_label_bg_update(info, BASE_RES_ID_HOME_WIDTH_LABLE, CUSTOMIZE_LABEL_X, CUSTOMIZE_LABEL_Y);
            home_label_text_update(info, &home_base_label_infos[HOME_BASE_TEXT_CUSTOMIZE_DESC]);
            home_label_icon_update(info, 0, 0, 0);
            home_label_time_update(info, time_text, CUSTOMIZE_TIME_TEXT_X, CUSTOMIZE_TIME_TEXT_Y);
            break;
        }
    }
    info->label.force_update = false;
}
