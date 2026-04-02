/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define BLE_LINE_COLOR                            0x1C1C20

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static lv_obj_t * ble_img;
static res_img_info_t ble_info;
static uint8_t s_is_connected = 0;
static uint8_t s_force_update = 0;
static uint32_t s_fps_start_time = 0;
static uint8_t s_is_show_ble = 0;

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
    s_is_connected = 0;
    s_is_show_ble = 0;
    s_force_update = true;
}

int ble_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, ble_manager_context_t *ble, uint16_t x, uint16_t y)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(root);
    EXCEPTION_CHECK_INT(ble);
    uint8_t is_connected = ble->ble_is_connected();
    uint32_t time_ms = get_timestamp();
    if (s_is_connected == is_connected && s_force_update == false) {        
        EXCEPTION_CHECK_INT(ble_img);
        if (time_ms >= 30000 || is_connected > 0) {
            lv_obj_clear_flag(ble_img, LV_OBJ_FLAG_HIDDEN); /// 显示
            return 0;
        }

        if ((time_ms - s_fps_start_time) < 100) {
            return 0;
        }
        s_fps_start_time = get_timestamp(); /// 更新开始时间

        if (s_is_show_ble) {
            s_is_show_ble = 0;
            lv_obj_clear_flag(ble_img, LV_OBJ_FLAG_HIDDEN); /// 显示
        } else {
            s_is_show_ble = 1;
            lv_obj_add_flag(ble_img, LV_OBJ_FLAG_HIDDEN); /// 隐藏
        }
        return 0;
    }

    common_ui_img_resource_init(thiz, is_connected > 0 ? RESOURCE_ID_BLE_CONNECT : RESOURCE_ID_BLE_DISCONNECT, &ble_info);
    EXCEPTION_CHECK_INT(ble_info.data);
    if (ble_img == NULL || !lv_obj_is_valid(ble_img)) {
        ble_img = lv_image_create(root);
        EXCEPTION_CHECK_INT(ble_img);
    }
    lv_fs_path_ex_t ble_img_path;
    lv_fs_make_path_from_buffer(&ble_img_path, LV_FS_MEMFS_LETTER, ble_info.data,
        ble_info.size, "baj");
    lv_image_set_src(ble_img, (const void *)&ble_img_path);
    // lv_obj_align(ble_img, LV_ALIGN_TOP_LEFT, 168 , 22);
    lv_obj_align(ble_img, LV_ALIGN_TOP_LEFT, x, y);
    s_is_connected = is_connected;
    s_force_update = false;
    s_fps_start_time = get_timestamp();
    s_is_show_ble = 1;
    return 0;
}

void ble_ui_deinit(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    ble_img = NULL;
    s_is_connected = 0;
    s_force_update = false;
    common_ui_img_resource_deinit(thiz, &ble_info);
}
