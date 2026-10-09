/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "home_custom_ui_internal.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_CUSTOM_HOME_COMP_BG              (0)
#if IS_SUPPORT_CUSTOM_HOME_COMP_BG
#define IS_SUPPORT_HOME_UI_TEST                     (0)
#else
#define IS_SUPPORT_HOME_UI_TEST                     (0)
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_CUSTOM_HOME_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t home_bg_data[] = {
    #include "comparison_png2txt/home_custom_31_360x360.txt"
};
#endif

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_custom_ui_prepare(ui_manager_context_t *thiz)
{

}

void home_custom_ui_remove(ui_manager_context_t *thiz)
{

}

int home_custom_ui_process(ui_manager_context_t *thiz)
{

}