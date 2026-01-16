/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "breast_pump_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define BREASTFEEDING_MODE_MAX_COUNT             (4)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
typedef struct {
    lv_obj_t * root;
    res_img_info_t icon_infos;
} bf_mode_display_info_t;

static bf_mode_display_info_t *bf_mode_info;

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
void ui_display_breastfeeding_mode_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (bf_mode_info == NULL) {
        bf_mode_info = (bf_mode_display_info_t *)u_malloz(sizeof(bf_mode_display_info_t));
        EXCEPTION_CHECK_VOID(bf_mode_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_breastfeeding_mode_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (bf_mode_info == NULL) {
        return;
    }
    // common_ui_img_resource_deinit(thiz, &bf_mode_info->icon_infos);
    LVGL_OBJ_DELETE(bf_mode_info->root);
    u_free(bf_mode_info);
    bf_mode_info = NULL;
}

int ui_display_breastfeeding_mode(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc->pump);
    uint8_t mode = proc->pump->get_breast_pump_mode();
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> breastfeeding mode fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
