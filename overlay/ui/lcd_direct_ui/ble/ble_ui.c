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

int ble_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, ble_manager_context_t *ble)
{
    EXCEPTION_CHECK_INT(ble);
    uint8_t is_connected = ble->ble_is_connected();
    if (s_is_connected == is_connected && s_force_update == false) {
        return 0;
    }

    common_ui_img_resource_init(thiz, is_connected > 0 ? RESOURCE_ID_BLE_CONNECT_ICON : RESOURCE_ID_BLE_DISCONNECT_ICON, &ble_info);
    EXCEPTION_CHECK_INT(ble_info.data);
    if (ble_img == NULL || !lv_obj_is_valid(ble_img)) {
        ble_img = lv_image_create(root);
        EXCEPTION_CHECK_INT(ble_img);
    }
    lv_fs_path_ex_t ble_img_path;
    lv_fs_make_path_from_buffer(&ble_img_path, LV_FS_MEMFS_LETTER, ble_info.data,
        ble_info.size, "baj");
    lv_image_set_src(ble_img, (const void *)&ble_img_path);
    lv_obj_align(ble_img, LV_ALIGN_TOP_LEFT, 138 + 6 - 1, 40);

    if (ble_line_icon == NULL || !lv_obj_is_valid(ble_line_icon)) {
        ble_line_icon = lv_line_create(root);
        EXCEPTION_CHECK_INT(ble_line_icon);
    }
    static lv_point_precise_t s_ble_line_pts[] = { {0, 0}, {0, 15} };   /// 高度 12px 的竖线
    lv_line_set_points(ble_line_icon, s_ble_line_pts, 2); 
    lv_obj_align(ble_line_icon, LV_ALIGN_TOP_LEFT, 168 + 5, 42 + 2); 
    lv_obj_set_style_line_width(ble_line_icon, 1, 0); /// 线宽
    lv_obj_set_style_line_color(ble_line_icon, lv_color_hex(BLE_LINE_COLOR), 0); /// 颜色
    lv_obj_set_style_line_rounded(ble_line_icon, true, 0); /// 圆角端点(可选)

    return 0;
}

void ble_ui_deinit(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    ble_img = NULL;
    ble_line_icon = NULL;
    common_ui_img_resource_deinit(thiz, &ble_info);
}
