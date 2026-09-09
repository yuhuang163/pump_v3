/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_font_img_tip.h" // IWYU pragma: keep

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static PSRAM_NC_DATA_LOC uint8_t s_mode_select_tip_png[] = {
    #include "font_img/txt/mode_select_tip_220x63.txt"
};

static PSRAM_NC_DATA_LOC uint8_t s_level_adjust_tip_png[] = {
#if (PRODUCT_LANGUAGE_USED == PRODUCT_LANGUAGE_EN)
    #include "font_img/txt/level_adjust_tip_232x63.txt"
#else
    #include "font_img/txt/level_adjust_tip_220x63.txt"
#endif
};

static PSRAM_NC_DATA_LOC uint8_t s_pump_break_tip_png[] = {
    #include "font_img/txt/pump_break_tip_220x63.txt"
};

static PSRAM_NC_DATA_LOC uint8_t s_guide_skip_tip_png[] = {
    #include "font_img/txt/guide_skip_tip_220x63.txt"
};

static const ui_img_data_t s_mode_select_tip = {
    .data = s_mode_select_tip_png,
    .size = sizeof(s_mode_select_tip_png),
};

static const ui_img_data_t s_level_adjust_tip = {
    .data = s_level_adjust_tip_png,
    .size = sizeof(s_level_adjust_tip_png),
};

static const ui_img_data_t s_pump_break_tip = {
    .data = s_pump_break_tip_png,
    .size = sizeof(s_pump_break_tip_png),
};

static const ui_img_data_t s_guide_skip_tip = {
    .data = s_guide_skip_tip_png,
    .size = sizeof(s_guide_skip_tip_png),
};

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
const ui_img_data_t *ui_font_img_mode_select_tip_get(void)
{
    return &s_mode_select_tip;
}

const ui_img_data_t *ui_font_img_level_adjust_tip_get(void)
{
    return &s_level_adjust_tip;
}

const ui_img_data_t *ui_font_img_pump_break_tip_get(void)
{
    return &s_pump_break_tip;
}

const ui_img_data_t *ui_font_img_guide_skip_tip_get(void)
{
    return &s_guide_skip_tip;
}
