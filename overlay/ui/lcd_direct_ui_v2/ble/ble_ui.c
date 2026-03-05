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
static lv_obj_t * ble_line_icon;
static uint8_t s_is_connected = 0;
static uint8_t s_force_update = 0;

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
    s_force_update = true;
}

int ble_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, ble_manager_context_t *ble, uint16_t x, uint16_t y)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(root);
    EXCEPTION_CHECK_INT(ble);
    uint8_t is_connected = ble->ble_is_connected();
    if (s_is_connected == is_connected && s_force_update == false) {
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
    if (ble_line_icon == NULL || !lv_obj_is_valid(ble_line_icon)) {
        ble_line_icon = lv_line_create(root);
        EXCEPTION_CHECK_INT(ble_line_icon);
    }
    s_is_connected = is_connected;
    s_force_update = false;
    return 0;
}

void ble_ui_deinit(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    ble_img = NULL;
    ble_line_icon = NULL;
    s_is_connected = 0;
    s_force_update = false;
    common_ui_img_resource_deinit(thiz, &ble_info);
}
