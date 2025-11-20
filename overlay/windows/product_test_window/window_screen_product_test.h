#ifndef __WINDOW_SCREEN_PRODUCT_TEST_H__
#define __WINDOW_SCREEN_PRODUCT_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h" // IWYU pragma: keep
#include "windows_map_define.h" // IWYU pragma: keep
#include "input_key_code.h" // IWYU pragma: keep


typedef enum {
    PRODUCT_TEST_START = 100,
    PRODUCT_TEST_E10   = 101,
    PRODUCT_TEST_E11   = 102,
} product_test_ui_action_e;

typedef struct ui_proc_product_test {    
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    // ui_page_handle_t bridge;
    broadcast_obj_t receiver;
    uint32_t margic;
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
} ui_proc_product_test_t;

#define PRODUCT_TESTS_ANIMATION_DISPLAY_TIME      (2000) /// 2s

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void show_product_tests_ui(ui_proc_product_test_t *proc);
void stop_product_tests_ui_animation(ui_proc_product_test_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


