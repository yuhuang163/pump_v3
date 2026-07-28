#ifndef __HOME_UI_H__
#define __HOME_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
typedef enum {
    HOME_PAGE_MAIN = 0,                                     /// 主页
    HOME_PAGE_MODE_0,                                       /// 模式选择页 0
    HOME_PAGE_MODE_1,                                       /// 模式选择页 1
    HOME_PAGE_RHYTHM_0,                                     /// 韵律选择页 0
    HOME_PAGE_RHYTHM_1,                                     /// 韵律选择页 1
    HOME_PAGE_RHYTHM_2,                                     /// 韵律选择页 2（个性化，可动态禁用）
    HOME_PAGE_MAX
} home_page_id_e;

typedef enum {
    HOME_PAGE_SLIDE_NONE = 0,
    HOME_PAGE_SLIDE_LEFT,                                   /// 内容往左移（旋钮右/下一页）
    HOME_PAGE_SLIDE_RIGHT,                                  /// 内容往右移（旋钮左/上一页）
} home_page_slide_dir_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ui_display_home_prepare(ui_manager_context_t *thiz);
void ui_display_home_remove(ui_manager_context_t *thiz);
int ui_display_home_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif// __HOME_UI_H__
