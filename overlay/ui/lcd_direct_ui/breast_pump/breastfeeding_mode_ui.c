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
    uint8_t force_update;
    uint8_t mode;
} bf_mode_display_info_t;

static bf_mode_display_info_t *bf_mode_info;

const static uint16_t breastfeeding_icon_ids[BREASTFEEDING_MODE_MAX_COUNT] = {
    RESOURCE_ID_PERSONALIZATION_MODE,
    RESOURCE_ID_PP_MODE,
    RESOURCE_ID_MILK_MODE,
    RESOURCE_ID_INITIATING_MODE,
};
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

static int breastfeeding_mode_img_update(ui_manager_context_t *thiz, uint8_t mode)
{
    if (bf_mode_info->root == NULL || !lv_obj_is_valid(bf_mode_info->root)) {
        bf_mode_info->root = lv_image_create(lv_screen_active());
        EXCEPTION_CHECK_INT(bf_mode_info->root);
    }
    
    uint8_t mode_index = 0;
    if (mode < BREAST_PUMP_CUSTOM_MODE || mode > BREAST_PUMP_LACTATION_START_MODE) {
        mode_index = 0;
        LOGE("mode is invalid: %d", mode);
    } else {
        mode_index = mode - BREAST_PUMP_CUSTOM_MODE;
    }
    common_ui_img_resource_init(thiz, breastfeeding_icon_ids[mode_index], 
        &bf_mode_info->icon_infos);
    EXCEPTION_CHECK_INT(bf_mode_info->icon_infos.data);
    lv_fs_path_ex_t breastfeeding_img_path;
    lv_fs_make_path_from_buffer(&breastfeeding_img_path, LV_FS_MEMFS_LETTER, 
        bf_mode_info->icon_infos.data, bf_mode_info->icon_infos.size, "baj");
    lv_image_set_src(bf_mode_info->root, breastfeeding_img_path.path);
    lv_obj_align(bf_mode_info->root, LV_ALIGN_CENTER, 0, 0);
    return 0;
}

static void breastfeeding_mode_ui_update(ui_manager_context_t *thiz, ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    uint8_t mode = proc->pump->get_breast_pump_mode();
    if (bf_mode_info->mode == mode && bf_mode_info->force_update == false) {
        return;
    }

    if (breastfeeding_mode_img_update(thiz, mode) != 0) {
        return;
    }
    bf_mode_info->mode = mode;
}
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
    bf_mode_info->force_update = true;
}

void ui_display_breastfeeding_mode_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (bf_mode_info == NULL) {
        return;
    }
    common_ui_img_resource_deinit(thiz, &bf_mode_info->icon_infos);
    LVGL_OBJ_DELETE(bf_mode_info->root);
    u_free(bf_mode_info);
    bf_mode_info = NULL;
}

int ui_display_breastfeeding_mode(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    breastfeeding_mode_ui_update(thiz, proc);
    bf_mode_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> breastfeeding mode fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
