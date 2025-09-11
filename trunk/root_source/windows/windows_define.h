#ifndef __WINDOWS_DEFINE_H__
#define __WINDOWS_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

#ifndef MAX_UI_DRAW_PROP
#define MAX_UI_DRAW_PROP (8)
#endif

#define CONCAT_MACRO(a, b) a##b

#define CONCAT_MACRO_3(a, b, c) a##b##c

#define UI_STRING_IFY(name) (#name)

#define PAGE_CONTEXT_NAME(name) CONCAT_MACRO(page_context_, name)

#define PAGE_HANDLE_NAME(name) CONCAT_MACRO_3(enter_new_window_, name, _screen)

#define PAGE_CONTEXT_NAME_STR(name) UI_STRING_IFY(name)

#ifndef LVGL_TIME_TICK_PERIOD
#define LVGL_TIME_TICK_PERIOD (10)
#endif

#define LVGL_HDR_TASK_PERIOD (16)

#define DEBUG_LV_FPS (1)

#define INIT_WINDOW_CONTEXT()  static void *__s_window_refs = NULL
#define GET_WINDOW_CONTEXT() ( __s_window_refs)

#define SET_WINDOW_CONTEXT(ctx, _log) \
    do { \
        __s_window_refs = ctx; \
        _log("set window ctx:%p", __s_window_refs); \
    } while (0)

#define IS_WINDOW_CONTEXT_PRESENT(closure, _log) \
    do { \
        if (__s_window_refs != NULL) { \
            { closure } \
        } else { \
            _log("Oops! NOT WINDOW_PRESENT"); \
        } \
    } while (0)
#ifdef __cplusplus
}
#endif

#endif 


