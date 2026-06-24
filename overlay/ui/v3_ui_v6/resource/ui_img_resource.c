/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_resource.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

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
void ui_img_resource_init(ui_manager_context_t *thiz, uint16_t id, res_img_info_t *info)
{
    EXCEPTION_CHECK_VOID(thiz && thiz->priv.res && info);
    if (info->data != NULL && info->id != id) {
        lv_draw_wait_for_finish();
        ui_img_resource_deinit(thiz, info);
    }

    int ret = thiz->priv.res->base_res_data_load(id, info);
    if (ret != 0) {
        LOGE("===> load base img data failed, id=%u ret=%d", (unsigned)id, ret);
    }
}

void ui_img_resource_init_with_theme(ui_manager_context_t *thiz, uint16_t id, res_img_info_t *info)
{
    EXCEPTION_CHECK_VOID(thiz && thiz->priv.res && info);
    if (info->data != NULL && info->id != id) {
        lv_draw_wait_for_finish();
        ui_img_resource_deinit(thiz, info);
    }

    int ret = thiz->priv.res->theme_res_data_load(id, info);
    if (ret != 0) {
        LOGE("===> load theme img data failed, id=%u ret=%d", (unsigned)id, ret);
    }
}

void ui_theme_img_resource_init(ui_manager_context_t *thiz, uint16_t id,
    res_theme_type_e theme_type, res_img_info_t *info)
{
    EXCEPTION_CHECK_VOID(thiz && thiz->priv.res && info);
    if (info->data != NULL && info->id != id) {
        lv_draw_wait_for_finish();
        ui_img_resource_deinit(thiz, info);
    }

    int ret = 0;
    if (theme_type >= RES_THEME_TYPE_MAX) {
        LOGE("===> invalid theme type, theme=%d id=%u", (int)theme_type, (unsigned)id);
        return;
    }
    
    if (theme_type == RES_THEME_TYPE_BASE) {
        ret = thiz->priv.res->base_res_data_load(id, info);
    } else {
        ret = thiz->priv.res->theme_res_data_load(id, info);
    }

    if (ret != 0) {
        LOGE("===> load img data failed, theme=%d id=%u ret=%d",
            (int)theme_type, (unsigned)id, ret);
    }
}

void ui_img_resource_deinit(ui_manager_context_t *thiz, res_img_info_t *info)
{
    EXCEPTION_CHECK_VOID(thiz && thiz->priv.res && info);
    if (info->data != NULL) {
        lv_draw_wait_for_finish();
    }
    thiz->priv.res->res_data_unload(info);
}

void ui_text_place_in_circle(lv_obj_t * lab, uint16_t angle, uint16_t ring_r, uint8_t is_reverse)
{
    EXCEPTION_CHECK_VOID(lab);
    lv_obj_update_layout(lab);
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

void ui_place_dot_on_arc(lv_obj_t * arc, lv_obj_t * dot, uint16_t angle_deg,
    uint16_t arc_w, uint16_t dot_d, float extra_inset, bool is_reverse)
{
    EXCEPTION_CHECK_VOID(arc);
    EXCEPTION_CHECK_VOID(dot);

    lv_obj_update_layout(arc);

    lv_area_t arc_area;
    lv_obj_get_coords(arc, &arc_area);

    int32_t left_bg = lv_obj_get_style_pad_left(arc, LV_PART_MAIN);
    int32_t right_bg = lv_obj_get_style_pad_right(arc, LV_PART_MAIN);
    int32_t top_bg = lv_obj_get_style_pad_top(arc, LV_PART_MAIN);
    int32_t bottom_bg = lv_obj_get_style_pad_bottom(arc, LV_PART_MAIN);

    int32_t w = lv_area_get_width(&arc_area) - left_bg - right_bg;
    int32_t h = lv_area_get_height(&arc_area) - top_bg - bottom_bg;
    int32_t r = (w < h ? w : h) / 2;

    lv_point_t c = {
        .x = arc_area.x1 + r + left_bg,
        .y = arc_area.y1 + r + top_bg
    };

    // 弧线中心线半径
    float r_centerline = (float)r - (float)arc_w / 2.0f;

    // 让白点中心落在弧线中心线；extra_inset>0 时再向内微调
    float r_dot = r_centerline - extra_inset;

    // 保持角度与 lv_arc 一致（单位：度）
    int16_t angle = (int16_t)angle_deg;
    if (is_reverse) {
        angle = 360 - angle;
    }
    while (angle < 0) {
        angle += 360;
    }
    while (angle >= 360) {
        angle -= 360;
    }

    int32_t x = c.x + (int32_t)((r_dot * lv_trigo_sin(angle + 90)) / (float)(1 << LV_TRIGO_SHIFT));
    int32_t y = c.y + (int32_t)((r_dot * lv_trigo_sin(angle)) / (float)(1 << LV_TRIGO_SHIFT));

    // dot 是 arc 的子对象：转成局部坐标
    lv_coord_t local_x = x - arc_area.x1;
    lv_coord_t local_y = y - arc_area.y1;

    lv_obj_clear_flag(dot, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(dot, local_x - dot_d / 2, local_y - dot_d / 2);
}