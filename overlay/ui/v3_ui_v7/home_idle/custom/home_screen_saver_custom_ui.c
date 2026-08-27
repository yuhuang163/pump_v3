/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_ROOT_HOME_SCREEN_SAVER_COMP_BG                (0)

#define HOME_SCREEN_SAVER_ICON_X                                 (0)
#define HOME_SCREEN_SAVER_ICON_Y                                 (108 + 2)

typedef struct {
    lv_obj_t * root;
    lv_obj_t * backgroud;
    lv_obj_t * saver_icon;
    uint8_t force_update;
    uint8_t rsv[15];
} home_screen_saver_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_ROOT_HOME_SCREEN_SAVER_COMP_BG
static PSRAM_DATA_LOC uint8_t home_screen_saver_bg_data[] = {
    #include "comparison_png2txt/home_idle_360x360.txt"
};
#endif
/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*  
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_screen_saver_custom_ui_prepare(ui_manager_context_t *thiz)
{

}

void home_screen_saver_custom_ui_remove(ui_manager_context_t *thiz)
{

}

int home_screen_saver_custom_ui_process(ui_manager_context_t *thiz)
{
    return 0;
}
