/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_BASE_GUIDE_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t guide_bg_data[] = {
    #include "comparison_png2txt/guide_base_p62_360x360.txt"
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
void guide_base_ui_prepare(ui_manager_context_t *thiz)
{
   
}

void guide_base_ui_remove(ui_manager_context_t *thiz)
{

}

int guide_base_ui_process(ui_manager_context_t *thiz)
{
    return 0;
}
