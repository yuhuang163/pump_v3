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
#define IS_SUPPORT_LABEL_TEXT_CMP                   (0) /// 支持label文本对比

#if IS_SUPPORT_HOME_BASE_MODE_CMP
#define HOME_LABEL_CMP_INDEX                        HOME_UI_DISPLAY_MILK_BOOST_DESC
#endif

#define IDLE_LABEL_X                                (49)
#define IDLE_LABEL_Y                                (147)
#define IDLE_TEXT_X                                 (0)
#define IDLE_TEXT_Y                                 (200)
#define IDLE_BUTTON_ICON_X                          (137)
#define IDLE_BUTTON_ICON_Y                          (195)
#define IDLE_BUTTON2_ICON_X                         (167)
#define IDLE_TEXT_COLOR                             (0xA0183B)
#define IDLE_TIME_TEXT_COLOR                        (0xD36682)

#define STIMULATION_LABEL_X                         (49)
#define STIMULATION_LABEL_Y                         (155)
#define STIMULATION_TEXT_X                          (0)
#define STIMULATION_TEXT_Y                          (207)

#define EXPRESSION_LABEL_X                          (49)
#define EXPRESSION_LABEL_Y                          (141)
#define EXPRESSION_TEXT_X                           (0)
#define EXPRESSION_TEXT_Y                           (193)

#define MIXED_LABEL_X                               (49)
#define MIXED_LABEL_Y                               (141)
#define MIXED_TEXT_X                                (0)
#define MIXED_TEXT_Y                                (193)

#define CUSTOMIZE_LABEL_X                           (49)
#define CUSTOMIZE_LABEL_Y                           (137)
#define CUSTOMIZE_TEXT_X                            (0)
#define CUSTOMIZE_TEXT_Y                            (186)
#define CUSTOMIZE_TIME_TEXT_X                       (0)
#define CUSTOMIZE_TIME_TEXT_Y                       (104)

#define MILK_BOOST_LABEL_X                          (49)
#define MILK_BOOST_LABEL_Y                          (163 - 14) /// 164
#define MILK_BOOST_TEXT_X                           (0)
#define MILK_BOOST_TEXT_Y                           (215 - 14) /// 198
#define MILK_BOOST_TIME_TEXT_X                      (0)
#define MILK_BOOST_TIME_TEXT_Y                      (152 - 48)

#define POWER_PUMPING_LABEL_X                       (49)
#define POWER_PUMPING_LABEL_Y                       (163 - 14)
#define POWER_PUMPING_TEXT_X                        (0)
#define POWER_PUMPING_TEXT_Y                        (215 - 14)
#define POWER_PUMPING_TIME_TEXT_X                   (0)
#define POWER_PUMPING_TIME_TEXT_Y                   (152 - 48)

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
    uint16_t line_space;
} home_base_text_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

const static home_base_text_info_t home_base_label_infos[HOME_BASE_TEXT_MAX] = {
    {"Press                  to switch\nthe pumping method",
        IDLE_TEXT_X, IDLE_TEXT_Y, true, 6},
    {"since last pumping session", 
        0, IDLE_RECORD_TEXT_Y, false, 4},
    {"Fast, gentle suction to\ntrigger let-down",
        0, STIMULATION_TEXT_Y, true, 4},
    {"Slow, deep suction\nfor efficient milk\nexpression", 
        0, EXPRESSION_TEXT_Y, true, 4},
    {"Efficiently promote\nlet-down and better\nemptying", 
        0, MIXED_TEXT_Y, true, 4},
    {"Suitable for daily use\nafter successful\nlactation",
        0, MILK_BOOST_TEXT_Y, true, 4},
    {"Increase milk supply\nno more than\nonce a day",
        0, POWER_PUMPING_TEXT_Y, true, 4},
    {"Edited & sync via\nMomcozy App", 
        0, CUSTOMIZE_TEXT_Y, true, 4},
};

static bool home_label_need_update(const home_base_ui_info_t *info)
{
    return (info->label.force_update != 0 || info->force_update != 0);
}

static void home_label_bg_update(home_base_ui_info_t *info, uint16_t img_id,
    uint16_t x_offset, uint16_t y_offset)
{
    if (info->label.label == NULL || !lv_obj_is_valid(info->label.label)) {
        info->label.label = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->label.label);
        lv_obj_add_flag(info->label.label, LV_OBJ_FLAG_HIDDEN);
    }

    if (img_id == 0) {
        if (info->label.label_img_id != 0) {
            ui_img_baj_release_decoded_buf(info->label.label);
            info->label.label_img_id = 0;
        }
        lv_obj_add_flag(info->label.label, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    if (!home_label_need_update(info) && info->label.label_img_id == img_id &&
        info->label.label_bg_x == x_offset && info->label.label_bg_y == y_offset) {
        lv_obj_clear_flag(info->label.label, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(info->label.label, LV_OBJ_FLAG_HIDDEN);
    ui_img_baj_theme_set_src(info->ui, info->label.label, img_id,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    #if IS_SUPPORT_LABEL_TEXT_CMP
    lv_obj_align(info->label.label, x_offset == 0 ? LV_ALIGN_TOP_MID : LV_ALIGN_TOP_LEFT,
        x_offset, 360);
    #else
    lv_obj_align(info->label.label, x_offset == 0 ? LV_ALIGN_TOP_MID : LV_ALIGN_TOP_LEFT,
        x_offset, y_offset + info->mode.select_label_y_offset);
    #endif
    info->label.label_img_id = img_id;
    info->label.label_bg_x = x_offset;
    info->label.label_bg_y = y_offset;
}

static void home_label_text_update(home_base_ui_info_t *info, home_base_text_id_e text_id)
{
    if (info->label.text == NULL || !lv_obj_is_valid(info->label.text)) {
        info->label.text = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->label.text);
        lv_obj_add_flag(info->label.text, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_text_line_space(info->label.text, 4, 0);
    }

    if (text_id >= HOME_BASE_TEXT_MAX) {
        if (info->label.text_id < HOME_BASE_TEXT_MAX) {
            lv_obj_add_flag(info->label.text, LV_OBJ_FLAG_HIDDEN);
            info->label.text_id = HOME_BASE_TEXT_MAX;
        }
        return;
    }

    if (!home_label_need_update(info) && info->label.text_id == text_id) {
        return;
    }

    const home_base_text_info_t *text_info = &home_base_label_infos[text_id];
    lv_obj_clear_flag(info->label.text, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(info->label.text, text_info->name);
    lv_obj_set_style_text_color(info->label.text, lv_color_hex(IDLE_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(info->label.text, &lv_font_aeonik_soft_18_regular, 0);
    lv_obj_set_style_text_line_space(info->label.text, text_info->line_space, 0);
    if (text_info->x_offset == 0) {
        lv_obj_align(info->label.text, LV_ALIGN_TOP_MID, 0,
            text_info->y_offset + info->mode.select_label_y_offset);
    } else {
        lv_obj_align(info->label.text, LV_ALIGN_TOP_LEFT, text_info->x_offset,
            text_info->y_offset + info->mode.select_label_y_offset);
    }
    lv_obj_set_style_text_align(info->label.text,
        text_info->align_center ? LV_TEXT_ALIGN_CENTER : LV_TEXT_ALIGN_LEFT, 0);
    info->label.text_id = text_id;
}

static void home_label_button_update(home_base_ui_info_t *info, bool show)
{
    // static const uint16_t button_res_ids[2] = {
    //     BASE_RES_ID_MODE_BUTTON_ICON,
    //     BASE_RES_ID_PROGRAM_BUTTON_ICON,
    // };
    static const uint16_t button_res_ids[2] = {0, 0};
    for (uint8_t i = 0; i < 2; i++) {
        if (info->label.button[i] == NULL || !lv_obj_is_valid(info->label.button[i])) {
            info->label.button[i] = lv_image_create(info->root);
            EXCEPTION_CHECK_VOID(info->label.button[i]);
            lv_obj_add_flag(info->label.button[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (!show || info->label.label == NULL || !lv_obj_is_valid(info->label.label) ||
        lv_obj_has_flag(info->label.label, LV_OBJ_FLAG_HIDDEN)) {
        for (uint8_t i = 0; i < 2; i++) {
            if (info->label.button[i] == NULL || !lv_obj_is_valid(info->label.button[i])) {
                continue;
            }
            if (info->label.button_img_id[i] != 0) {
                ui_img_baj_release_decoded_buf(info->label.button[i]);
                info->label.button_img_id[i] = 0;
            }
            lv_obj_add_flag(info->label.button[i], LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    for (uint8_t i = 0; i < 2; i++) {
        if (info->label.button[i] == NULL || !lv_obj_is_valid(info->label.button[i])) {
            continue;
        }

        uint16_t img_id = button_res_ids[i];
        if (!home_label_need_update(info) && info->label.button_img_id[i] == img_id) {
            lv_obj_clear_flag(info->label.button[i], LV_OBJ_FLAG_HIDDEN);
            continue;
        }

        lv_obj_clear_flag(info->label.button[i], LV_OBJ_FLAG_HIDDEN);
        ui_img_baj_theme_set_src(info->ui, info->label.button[i], img_id,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
        if (i == 0) {
            lv_obj_align(info->label.button[i], LV_ALIGN_TOP_LEFT, IDLE_BUTTON_ICON_X, IDLE_BUTTON_ICON_Y);
        } else {
            lv_obj_align(info->label.button[i], LV_ALIGN_TOP_LEFT, IDLE_BUTTON2_ICON_X, IDLE_BUTTON_ICON_Y);
        }
        info->label.button_img_id[i] = img_id;
    }
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
        if (info->label.icon_img_id != 0) {
            ui_img_baj_release_decoded_buf(info->label.icon);
            info->label.icon_img_id = 0;
        }
        lv_obj_add_flag(info->label.icon, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    if (!home_label_need_update(info) && info->label.icon_img_id == img_id) {
        lv_obj_clear_flag(info->label.icon, LV_OBJ_FLAG_HIDDEN);
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
    info->label.icon_img_id = img_id;
}

static void home_label_time_update(home_base_ui_info_t *info, const char *time_text,
    uint16_t x_offset, uint16_t y_offset)
{
    if (info->label.time == NULL || !lv_obj_is_valid(info->label.time)) {
        info->label.time = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->label.time);
        lv_obj_add_flag(info->label.time, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_text_color(info->label.time, lv_color_hex(IDLE_TIME_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(info->label.time, &lv_font_aeonik_soft_20_medium, 0);
    }

    if (time_text == NULL) {
        if (info->label.time_text[0] != '\0') {
            lv_obj_add_flag(info->label.time, LV_OBJ_FLAG_HIDDEN);
            info->label.time_text[0] = '\0';
        }
        return;
    }

    if (!home_label_need_update(info) && strcmp(info->label.time_text, time_text) == 0) {
        return;
    }

    lv_obj_clear_flag(info->label.time, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(info->label.time, time_text);
    info->label.label_time_x = x_offset;
    info->label.label_time_y = y_offset;
    if (x_offset == 0) {
        lv_obj_align(info->label.time, LV_ALIGN_TOP_MID, 0,
            y_offset + info->mode.select_time_y_offset);
    } else {
        lv_obj_align(info->label.time, LV_ALIGN_TOP_LEFT, x_offset,
            y_offset + info->mode.select_time_y_offset);
    }
    strncpy(info->label.time_text, time_text, sizeof(info->label.time_text) - 1);
    info->label.time_text[sizeof(info->label.time_text) - 1] = '\0';
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_pump_label_apply_select_offset(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);

    if (info->label.label != NULL && lv_obj_is_valid(info->label.label) &&
        !lv_obj_has_flag(info->label.label, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_align(info->label.label,
            info->label.label_bg_x == 0 ? LV_ALIGN_TOP_MID : LV_ALIGN_TOP_LEFT,
            info->label.label_bg_x,
            info->label.label_bg_y + info->mode.select_label_y_offset);
    }

    if (info->label.text != NULL && lv_obj_is_valid(info->label.text) &&
        !lv_obj_has_flag(info->label.text, LV_OBJ_FLAG_HIDDEN) &&
        info->label.text_id < HOME_BASE_TEXT_MAX) {
        const home_base_text_info_t *text_info = &home_base_label_infos[info->label.text_id];
        if (text_info->x_offset == 0) {
            lv_obj_align(info->label.text, LV_ALIGN_TOP_MID, 0,
                text_info->y_offset + info->mode.select_label_y_offset);
        } else {
            lv_obj_align(info->label.text, LV_ALIGN_TOP_LEFT, text_info->x_offset,
                text_info->y_offset + info->mode.select_label_y_offset);
        }
    }

    if (info->label.time != NULL && lv_obj_is_valid(info->label.time) &&
        !lv_obj_has_flag(info->label.time, LV_OBJ_FLAG_HIDDEN) &&
        info->label.time_text[0] != '\0') {
        if (info->label.label_time_x == 0) {
            lv_obj_align(info->label.time, LV_ALIGN_TOP_MID, 0,
                info->label.label_time_y + info->mode.select_time_y_offset);
        } else {
            lv_obj_align(info->label.time, LV_ALIGN_TOP_LEFT, info->label.label_time_x,
                info->label.label_time_y + info->mode.select_time_y_offset);
        }
    }
}

void home_pump_label_ui_remove(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_baj_release_decoded_buf(info->label.label);
    ui_img_baj_release_decoded_buf(info->label.icon);
    for (uint8_t i = 0; i < 2; i++) {
        ui_img_baj_release_decoded_buf(info->label.button[i]);
        info->label.button_img_id[i] = 0;
    }
    info->label.label_img_id = 0;
    info->label.icon_img_id = 0;
    info->label.text_id = HOME_BASE_TEXT_MAX;
    info->label.time_text[0] = '\0';
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

#if IS_SUPPORT_HOME_BASE_MODE_CMP
    info->label.type = HOME_LABEL_CMP_INDEX;
#endif

    switch (info->label.type) {
        default:
        case HOME_UI_DISPLAY_DEFAULT:
            home_label_bg_update(info, UI_IMG_RES_ID_NONE,
                IDLE_LABEL_X, IDLE_LABEL_Y);
            home_label_text_update(info, HOME_BASE_TEXT_IDLE);
            home_label_button_update(info, true);
            home_label_icon_update(info, 0, 0, 0);
            home_label_time_update(info, NULL, 0, 0);
            break;
        case HOME_UI_DISPLAY_SIMULATE_MODE_DESC:{
            home_label_bg_update(info, UI_IMG_RES_ID_NONE,
                STIMULATION_LABEL_X, STIMULATION_LABEL_Y);
            home_label_text_update(info, HOME_BASE_TEXT_SIMULATE_MODE_DESC);
            home_label_button_update(info, false);
            home_label_icon_update(info, 0, 0, 0);
            home_label_time_update(info, NULL, 0, 0);
            break;
        }
        case HOME_UI_DISPLAY_EXPRESSION_MODE_DESC:{
            home_label_bg_update(info, // BASE_RES_ID_HOME_WIDTH_LABLE,
                0, EXPRESSION_LABEL_X, EXPRESSION_LABEL_Y);
            home_label_text_update(info, HOME_BASE_TEXT_EXPRESSION_MODE_DESC);
            home_label_button_update(info, false);
            home_label_icon_update(info, 0, 0, 0);
            home_label_time_update(info, NULL, 0, 0);
            break;
        }
        case HOME_UI_DISPLAY_MIXED_MODE_DESC:{
            home_label_bg_update(info, // BASE_RES_ID_HOME_WIDTH_LABLE,
                0, MIXED_LABEL_X, MIXED_LABEL_Y);
            home_label_text_update(info, HOME_BASE_TEXT_MIXED_MODE_DESC);
            home_label_button_update(info, false);
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
            home_label_bg_update(info, // BASE_RES_ID_HOME_WIDTH_LABLE,
                0, MILK_BOOST_LABEL_X, MILK_BOOST_LABEL_Y);
            home_label_text_update(info, HOME_BASE_TEXT_MILK_BOOST_DESC);
            home_label_button_update(info, false);
            home_label_icon_update(info, 0, 0, 0);
            home_label_time_update(info, time_text, MILK_BOOST_TIME_TEXT_X, MILK_BOOST_TIME_TEXT_Y);
            break;
        }
        case HOME_UI_DISPLAY_POWER_PUMPING_DESC:{
            int pump_time = 0;
            storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
            if (st) {
                st->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, BREAST_PUMP_PP_MILK_TRACK_MODE, 0, &pump_time);
            }
            char time_text[16] = {0};
            snprintf(time_text, sizeof(time_text), "≈%d min", pump_time / 60);
            home_label_bg_update(info, // BASE_RES_ID_HOME_WIDTH_LABLE,
                0, POWER_PUMPING_LABEL_X, POWER_PUMPING_LABEL_Y);
            home_label_text_update(info, HOME_BASE_TEXT_POWER_PUMPING_DESC);
            home_label_button_update(info, false);
            home_label_icon_update(info, 0, 0, 0);
            home_label_time_update(info, time_text, POWER_PUMPING_TIME_TEXT_X, POWER_PUMPING_TIME_TEXT_Y);
            break;
        }
        case HOME_UI_DISPLAY_CUSTOMIZE_DESC:{
            int pump_time = 0;
            storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
            if (st) {
                st->get_store_ram_val(STORE_ID_BREAST_PUMP_TIME, BREAST_PUMP_CUSTOM_MODE, 0, &pump_time);
            }
            char time_text[16] = {0};
            snprintf(time_text, sizeof(time_text), "≈%d min", pump_time / 60);
            home_label_bg_update(info, // UI_IMG_RES_ID_NONE,
                0, CUSTOMIZE_LABEL_X, CUSTOMIZE_LABEL_Y);
            home_label_text_update(info, HOME_BASE_TEXT_CUSTOMIZE_DESC);
            home_label_button_update(info, false);
            home_label_icon_update(info, 0, 0, 0);
            home_label_time_update(info, time_text, CUSTOMIZE_TIME_TEXT_X, CUSTOMIZE_TIME_TEXT_Y);
            break;
        }
    }
    info->label.force_update = false;
}
