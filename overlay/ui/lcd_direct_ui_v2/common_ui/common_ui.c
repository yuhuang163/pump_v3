/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

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
void common_ui_img_resource_init(ui_manager_context_t *thiz, uint16_t id, res_img_info_t *info)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(thiz->priv.res);
    EXCEPTION_CHECK_VOID(info);
    int ret = thiz->priv.res->res_ui_img_data_load(id, info);
    if (ret != 0) {
        LOGE("===> load img data failed, id=%u ret=%d", (unsigned)id, ret);
    }
}

void common_ui_img_resource_deinit(ui_manager_context_t *thiz, res_img_info_t *info)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(thiz->priv.res);
    EXCEPTION_CHECK_VOID(info);
    thiz->priv.res->res_ui_img_data_unload(info);
}

void common_ui_text_place_in_circle(lv_obj_t * lab, uint16_t angle, uint16_t ring_r, uint8_t is_reverse)
{
    EXCEPTION_CHECK_VOID(lab);
    /// 父对象 360x360，圆心(180,180)
    lv_point_t c = { .x = PANLE_DISPLAY_W / 2, .y = PANLE_DISPLAY_H / 2 };
    /// 计算圆周点
    lv_coord_t px = c.x + (lv_trigo_cos(angle) * ring_r) / LV_TRIGO_SIN_MAX;
    lv_coord_t py = c.y + (lv_trigo_sin(angle) * ring_r) / LV_TRIGO_SIN_MAX;
    /// 先把label中心放到该点（固定位置）
    lv_obj_align(lab, LV_ALIGN_CENTER, px - c.x, py - c.y);
    /// 旋转中心设为label自身中心（必须在 set_text 后取宽高）
    lv_coord_t w = lv_obj_get_width(lab);
    lv_coord_t h = lv_obj_get_height(lab);
    lv_obj_set_style_transform_pivot_x(lab, w / 2, 0);
    lv_obj_set_style_transform_pivot_y(lab, h / 2, 0);
    /// 切线方向：angle + 90°（单位0.1°）
    int tangent = angle + 90 + (is_reverse ? 180 : 0);
    lv_obj_set_style_transform_angle(lab, tangent * 10, 0);
}