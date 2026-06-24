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
#define PUMP_LEVEL_MAX                             (15)
#define PUMP_LEVEL_RING_SIZE                       (306)
#define PUMP_LEVEL_RING_R                          (PUMP_LEVEL_RING_SIZE / 2)
#define PUMP_LEFT_ARC_START_ANGLE                  (136 + 8)
#define PUMP_LEFT_ARC_END_ANGLE                    (224 + 12)
#define PUMP_RIGHT_ARC_START_ANGLE                 (316 - 12)
#define PUMP_RIGHT_ARC_END_ANGLE                   (404 - 8)
#define PUMP_LEVEL_ARC_WIDTH                       (6)
#define PUMP_LEVEL_CIRCLE_SIZE                     (9)
#define PUMP_LEVEL_ARC_COLOR                       (0xB3636D)
#define PUMP_LEVEL_CIRCLE_COLOR                    (0xFFFFFF) /// (0xFFEDEF)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static void home_left_level_indicator_update(home_base_ui_info_t *info)
{

}

static void home_left_level_progress_update(home_base_ui_info_t *info)
{

}

static void home_left_level_arc_update(home_base_ui_info_t *info)
{

}

static void home_right_level_indicator_update(home_base_ui_info_t *info)
{

}

static void home_right_level_progress_update(home_base_ui_info_t *info)
{

}

static void home_right_level_arc_update(home_base_ui_info_t *info)
{

}

static void home_level_separator_update(home_base_ui_info_t *info)
{
    if (info->level.separator == NULL || !lv_obj_is_valid(info->level.separator)) {
        info->level.separator = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->level.separator);
        lv_obj_align(info->level.separator, LV_ALIGN_BOTTOM_MID, 0, -18); /// 设置分隔符位置
        ui_img_baj_theme_set_src(info->ui, info->level.separator, BASE_RES_ID_PUMP_LEVEL_SEPARATOR,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_pump_level_ui_deinit(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_baj_release_decoded_buf(info->level.separator);
}

void home_pump_level_info_update(home_base_ui_info_t *info)
{
    home_level_separator_update(info);
}