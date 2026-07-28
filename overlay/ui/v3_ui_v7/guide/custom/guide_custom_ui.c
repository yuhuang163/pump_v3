/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_custom_ui_internal.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_CUSTOM_GUIDE_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t guide_bg_data[] = {
    #include "comparison_png2txt/guide_custom_p66_360x360.txt"
    // #include "comparison_png2txt/home_custom_34_360x360.txt"
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
void guide_custom_ui_prepare(ui_manager_context_t *thiz)
{

}

void guide_custom_ui_remove(ui_manager_context_t *thiz)
{

}

int guide_custom_ui_process(ui_manager_context_t *thiz)
{
    return 0;
}
