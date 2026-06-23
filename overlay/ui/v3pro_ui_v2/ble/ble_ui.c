/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define BLE_LINE_COLOR                            0x1C1C20

typedef struct {
    lv_obj_t * ble_img;
    res_img_info_t img;
    uint32_t fps_start_time;
    uint8_t is_show_ble;
    uint8_t is_connected;
    uint8_t force_update;
    uint8_t rsv[14];
} ble_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ble_display_info_t *ble_info;
static bool connect_prompt_flag = true;  /// 连接提示标志 true-显示 false-隐藏

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

void ble_ui_init(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (ble_info == NULL) {
        ble_info = (ble_display_info_t *)u_malloz(sizeof(ble_display_info_t));
        EXCEPTION_CHECK_VOID(ble_info);
    }
    ble_info->fps_start_time = 0;
    ble_info->is_show_ble = 0;
    ble_info->force_update = true;
}

int ble_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, ble_manager_context_t *ble, uint16_t x, uint16_t y)
{
    EXCEPTION_CHECK_INT(thiz && root && ble);
    uint8_t is_connected = ble->ble_is_connected();
    uint32_t time_ms = get_timestamp();
    if (ble_info->is_connected == is_connected && ble_info->force_update == false) {        
        EXCEPTION_CHECK_INT(ble_info->ble_img);
        if (time_ms >= 30000 || is_connected > 0 || connect_prompt_flag == false) {
            connect_prompt_flag = false; 
            lv_obj_clear_flag(ble_info->ble_img, LV_OBJ_FLAG_HIDDEN); /// 显示
            return 0;
        }

        if ((time_ms - ble_info->fps_start_time) < 500) {
            return 0;
        }
        ble_info->fps_start_time = get_timestamp(); /// 更新开始时间
        if (ble_info->is_show_ble) {
            ble_info->is_show_ble = 0;
            lv_obj_clear_flag(ble_info->ble_img, LV_OBJ_FLAG_HIDDEN); /// 显示
        } else {
            ble_info->is_show_ble = 1;
            lv_obj_add_flag(ble_info->ble_img, LV_OBJ_FLAG_HIDDEN); /// 隐藏
        }
        return 0;
    }
    common_ui_img_resource_init(thiz, 
        is_connected > 0 ? BASE_RES_ID_BLE_CONNECT_ICON : BASE_RES_ID_BLE_DISCONNECT_ICON, &ble_info->img);
    EXCEPTION_CHECK_INT(ble_info->img.data);
    if (ble_info->ble_img == NULL || !lv_obj_is_valid(ble_info->ble_img)) {
        ble_info->ble_img = lv_image_create(root);
        EXCEPTION_CHECK_INT(ble_info->ble_img);
    }
    lv_fs_path_ex_t ble_img_path;
    lv_fs_make_path_from_buffer(&ble_img_path, LV_FS_MEMFS_LETTER, ble_info->img.data,
        ble_info->img.size, "baj");
    lv_image_set_src(ble_info->ble_img, (const void *)&ble_img_path);
    lv_obj_align(ble_info->ble_img, LV_ALIGN_TOP_LEFT, x, y);
    ble_info->is_connected = is_connected;
    ble_info->force_update = false;
    ble_info->fps_start_time = get_timestamp();
    ble_info->is_show_ble = 1;
    return 0;
}

void ble_ui_deinit(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    ble_info->ble_img = NULL;
    ble_info->is_connected = 0;
    ble_info->force_update = false;
    common_ui_img_resource_deinit(thiz, &ble_info->img);
    u_free(ble_info);
    ble_info = NULL;
}
