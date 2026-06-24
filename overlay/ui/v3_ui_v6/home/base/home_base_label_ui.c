/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
// #include "home_base_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IDLE_LABEL_AREA_ICON_X                      (0)
#define IDLE_LABEL_AREA_ICON_Y                      (112)
#define IDLE_BUTTON_ICON_X                          (96)
#define IDLE_BUTTON_ICON_Y                          (24)

#define IDLE_LABEL_TEXT_X                           (0)
#define IDLE_LABEL_TEXT_Y                           (27)
#define IDLE_TEXT_COLOR                             (0xB25962)

#define MODE_DESC_LABEL_AREA_ICON_X                 (0)
#define MODE_DESC_LABEL_AREA_ICON_Y                 (106)

#define MODE_DESC_LABEL_TEXT_X                      (0)
#define MODE_DESC_LABEL_TEXT_Y                      (30)

#define IDLE_RECORD_AREA_LABEL_X                    (0)
#define IDLE_RECORD_AREA_LABEL_Y                    (106)
#define IDLE_RECORD_TIME_TEXT_X                     (0)
#define IDLE_RECORD_TIME_TEXT_Y                     (IDLE_RECORD_LABEL_Y + 10)
#define IDLE_RECORD_LABEL_TEXT_X                    (0)
#define IDLE_RECORD_LABEL_TEXT_Y                    (30)

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
    uint16_t img_id;
    uint16_t x_offset; 
    uint16_t y_offset; 
    bool align_center; 
    const lv_font_t *font;
} home_base_label_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static home_base_label_info_t home_base_label_infos[HOME_BASE_TEXT_MAX] = {
    {"Click         to start pumping", BASE_RES_ID_HOME_NARROW_LABLE, 
        IDLE_LABEL_TEXT_X, IDLE_LABEL_TEXT_Y, false, &lv_font_oppo_sans_16_medium},
    {"since last pumping session", BASE_RES_ID_HOME_WIDTH_LABLE, 
        0, IDLE_RECORD_LABEL_TEXT_Y, false, &lv_font_oppo_sans_16_medium},
    {"Fast, gentle suction to\ntrigger let-down", BASE_RES_ID_HOME_WIDTH_LABLE, 
        0, MODE_DESC_LABEL_TEXT_Y, true, &lv_font_oppo_sans_16_medium},
    {"Slow, deep suction for\nefficient milk expression", BASE_RES_ID_HOME_WIDTH_LABLE, 
        0, MODE_DESC_LABEL_TEXT_Y, true, &lv_font_oppo_sans_16_medium},
    {"Efficiently promote let-\ndown and better emptying", BASE_RES_ID_HOME_WIDTH_LABLE, 
        0, MODE_DESC_LABEL_TEXT_Y, true, &lv_font_oppo_sans_16_medium},
    {"Most suitable for daily use\nafter successful lactation", BASE_RES_ID_HOME_WIDTH_LABLE, 
        0, MODE_DESC_LABEL_TEXT_Y, true, &lv_font_oppo_sans_16_medium},
    {"Increase milk supply, no\nmore than once a day", BASE_RES_ID_HOME_WIDTH_LABLE, 
        0, MODE_DESC_LABEL_TEXT_Y, true, &lv_font_oppo_sans_16_medium},
    {"Edit and sync anytime via\nthe Momcozy App", BASE_RES_ID_HOME_WIDTH_LABLE, 
        0, MODE_DESC_LABEL_TEXT_Y, true, &lv_font_oppo_sans_16_medium},   
};

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
