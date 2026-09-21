/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "home_common_ui.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_base_animation_remove(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    memset(&info->animation, 0, sizeof(info->animation));
}

void home_base_animation_pause(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    info->animation.paused_by_transition = 1;
}

void home_base_animation_resume(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    if (info->animation.paused_by_transition) {
        info->animation.paused_by_transition = 0;
        info->main.hint_shine.mask_tick_ms = get_timestamp();
    }
}
