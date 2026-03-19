/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "persist_storage.h"
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
#define IS_SUPPORT_SPEED_COMP_BG                (0)
#define IS_SUPPORT_SPEED_BG_IMG                 (0)
#define SPEED_COUNT                             (3)
#define SPEED_CIRCLE_SLOT_COUNT                 (3)   /// 仅保留左/中/右 3 个圆槽位
#define SPEED_VIEW_W                            (360)
#define SPEED_VIEW_H                            (360)
#define SPEED_CIRCLE_W                          (220)
#define SPEED_CIRCLE_H                          (220)
#define SPEED_CIRCLE_CENTER_X_GAP               (SPEED_CIRCLE_W + 20)  /// 220 + 20
#define SPEED_SIDE_CIRCLE_Y_OFFSET              (24 + 5)
#define SPEED_CHILD_CENTER_OPA                  (LV_OPA_30)
#define SPEED_CHILD_SIDE_OPA                    (LV_OPA_20)
#define SPEED_DOT_COLOR                         (0xB25962)
#define SPEED_TEXT_COLOR                        (0xB25962)
#define SPEED_CENTER_CIRCLE_GRAD_TOP_COLOR      (0xEB7581)
#define SPEED_CENTER_CIRCLE_GRAD_BOTTOM_COLOR   (0xF0868A)
#define SPEED_SIDE_CIRCLE_GRAD_TOP_COLOR        (0xF7A1A8)
#define SPEED_SIDE_CIRCLE_GRAD_BOTTOM_COLOR     (0xF7A1A8)
#define SPEED_TEXT_CENTER_COLOR                 (0xFFFFFF)
#define SPEED_TITLE_TEXT_COLOR                  (0xFFFFFF)
#define SPEED_TEXT_SIDE_COLOR                   (0xE7D4D9)
#define SPEED_DESC_TEXT_COLOR                   (0xFFDDDF)
#define SPEED_CENTER_DESC_TEXT_COLOR            (0xFFEAEA)
#define SPEED_TEXT_CENTER_OPA                   (LV_OPA_COVER)
#define SPEED_TEXT_SIDE_OPA                     (100)
#define SPEED_SWITCH_ANIM_TIME_MS               (260)
#define SPEED_DESC_BASE_Y_OFFSET                (33)
#define SPEED_DESC_BASE_LINE_COUNT              (3)
#define SPEED_CIRCLE_BG_EXT                     ("baj")
#define SPEED_COMPONENT_EXT                     ("baj")

typedef enum {
    SPEED_SLOT_LEFT = 0,
    SPEED_SLOT_CENTER,
    SPEED_SLOT_RIGHT,
    SPEED_SLOT_MAX,
} speed_circle_slot_e;

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_SPEED_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t *speed_type_text;
    lv_obj_t *circle_obj[SPEED_CIRCLE_SLOT_COUNT]; /// 圆圈对象
    lv_obj_t *dots[SPEED_COUNT]; /// 模式序号点
    lv_obj_t *speed_component_img[SPEED_CIRCLE_SLOT_COUNT]; /// 模式组件图片
    lv_obj_t *speed_title_label[SPEED_CIRCLE_SLOT_COUNT]; /// 模式标题标签
    lv_obj_t *speed_desc_label[SPEED_CIRCLE_SLOT_COUNT]; /// 模式描述标签
    res_img_info_t speed_component_info[SPEED_CIRCLE_SLOT_COUNT];
    lv_fs_path_ex_t speed_component_path[SPEED_CIRCLE_SLOT_COUNT];
    uint16_t slot_res_id[SPEED_CIRCLE_SLOT_COUNT];
    int16_t slot_speed_idx[SPEED_CIRCLE_SLOT_COUNT];
    uint8_t cur_speed;
    uint8_t force_update;
    uint8_t speed_obj_init_done;
} speed_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static speed_display_info_t *speed_info;

static const uint16_t speed_component_ids[SPEED_COUNT] = {
    RESOURCE_ID_FREQ_PAGE_BOOST_ICON, /// 快速
    RESOURCE_ID_FREQ_PAGE_COMFORT_ICON, /// 舒适
    RESOURCE_ID_FREQ_PAGE_REGULAR_ICON, /// 常规
};

static const char *speed_name[SPEED_COUNT] = {
    "Fast", /// 快速
    "Slow", /// 慢速
    "Regular", /// 常规
};

static const char *speed_desc[SPEED_COUNT] = {
    "Stronger yet safe for\nfaster pumping.",
    "Efficient yet gentle.",
    "Suitable for most moms.",
};

#if IS_SUPPORT_SPEED_COMP_BG
static PSRAM_DATA_LOC uint8_t speed_bg_data[] = {
    #include "comparison_png2txt/pump_speed_1_360x360.txt"
};
#endif

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int32_t slot_target_x(speed_circle_slot_e slot)
{
    int32_t center_x = (SPEED_VIEW_W - SPEED_CIRCLE_W) / 2;
    int32_t delta = (int32_t)slot - (int32_t)SPEED_SLOT_CENTER;
    return center_x + delta * SPEED_CIRCLE_CENTER_X_GAP;
}

static int32_t slot_target_y(speed_circle_slot_e slot)
{
    int32_t center_y = (SPEED_VIEW_H - SPEED_CIRCLE_H) / 2 + 16;
    if (slot == SPEED_SLOT_CENTER) {
        return center_y;
    }
    /// 左右两个圆都比中间圆高 24
    return center_y - SPEED_SIDE_CIRCLE_Y_OFFSET;
}

static void apply_slot_children_opa(uint8_t slot)
{
#if 0
    lv_obj_t *circle = speed_info->circle_obj[slot];
    if (circle == NULL || !lv_obj_is_valid(circle)) {
        return;
    }
    lv_opa_t opa = (slot == SPEED_SLOT_CENTER) ? LV_OPA_COVER : 5;
    lv_obj_set_style_image_opa(circle, opa, 0);
    lv_obj_invalidate(circle); /// 刷新

    if (speed_info->speed_component_img[slot] != NULL && lv_obj_is_valid(speed_info->speed_component_img[slot])) {
        opa =(slot == SPEED_SLOT_CENTER) ? LV_OPA_COVER : LV_OPA_80;
        lv_obj_set_style_image_opa(speed_info->speed_component_img[slot], opa, 0);;
        lv_obj_invalidate(speed_info->speed_component_img[slot]); /// 刷新
    }

    if (speed_info->speed_title_label[slot] != NULL && lv_obj_is_valid(speed_info->speed_title_label[slot])) {
        opa =(slot == SPEED_SLOT_CENTER) ? LV_OPA_COVER : LV_OPA_80;
        lv_obj_set_style_text_opa(speed_info->speed_title_label[slot], opa, 0);
        lv_obj_invalidate(speed_info->speed_title_label[slot]); /// 刷新
    }
    
    if (speed_info->speed_desc_label[slot] != NULL && lv_obj_is_valid(speed_info->speed_desc_label[slot])) {
        opa =(slot == SPEED_SLOT_CENTER) ? 240 : 210;
        lv_obj_set_style_text_opa(speed_info->speed_desc_label[slot], opa, 0);
        lv_obj_invalidate(speed_info->speed_desc_label[slot]); /// 刷新
    }
#endif
}

static void apply_all_children_opa(void)
{
#if 0
    for (uint8_t i = 0; i < SPEED_CIRCLE_SLOT_COUNT; i++) {
        apply_slot_children_opa(i);
    }
#endif
}

static void update_dots(uint8_t cur)
{
    for (uint8_t i = 0; i < SPEED_COUNT; i++) {
        if (speed_info->dots[i] == NULL || !lv_obj_is_valid(speed_info->dots[i])) {
            continue;
        }
        lv_obj_set_style_bg_opa(speed_info->dots[i], (i == cur) ? LV_OPA_COVER : LV_OPA_30, 0);
    }
}

static void item_anim_exec_x(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_obj_set_x(obj, (lv_coord_t)v);
    apply_all_children_opa();
    lv_obj_invalidate(speed_info->root);
}

static void item_anim_exec_y(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_obj_set_y(obj, (lv_coord_t)v);
    lv_obj_invalidate(speed_info->root);
}

static int16_t slot_speed_for_center(uint8_t center_speed, speed_circle_slot_e slot)
{
    int16_t speed_idx = (int16_t)center_speed + (int16_t)slot - (int16_t)SPEED_SLOT_CENTER;
    while (speed_idx < 0) {
        speed_idx += SPEED_COUNT;
    }
    return (int16_t)(speed_idx % SPEED_COUNT);
}

static void update_slot_content(ui_manager_context_t *thiz, speed_circle_slot_e slot, int16_t speed_idx)
{
    lv_obj_t *item = speed_info->circle_obj[slot];
    lv_obj_t *img = speed_info->speed_component_img[slot];
    lv_obj_t *title = speed_info->speed_title_label[slot];
    lv_obj_t *desc = speed_info->speed_desc_label[slot];
    if (item == NULL || img == NULL || title == NULL || desc == NULL) {
        return;
    }

    lv_obj_clear_flag(item, LV_OBJ_FLAG_HIDDEN);
    if (speed_info->slot_speed_idx[slot] == speed_idx) {
        return;
    }

    uint16_t res_id = speed_component_ids[speed_idx];
    if (speed_info->slot_res_id[slot] != res_id) {
        if (speed_info->slot_res_id[slot] != 0) {
            common_ui_img_resource_deinit(thiz, &speed_info->speed_component_info[slot]);
            memset(&speed_info->speed_component_info[slot], 0, sizeof(speed_info->speed_component_info[slot]));
        }
        common_ui_img_resource_init(thiz, res_id, &speed_info->speed_component_info[slot]);
        if (speed_info->speed_component_info[slot].data != NULL) {
            lv_fs_make_path_from_buffer(&speed_info->speed_component_path[slot],
                                        LV_FS_MEMFS_LETTER,
                                        speed_info->speed_component_info[slot].data,
                                        speed_info->speed_component_info[slot].size,
                                        SPEED_COMPONENT_EXT);
            lv_image_set_src(img, (const void *)&speed_info->speed_component_path[slot]);
            speed_info->slot_res_id[slot] = res_id;
        }
    }

    lv_label_set_text(title, speed_name[speed_idx]);
    lv_label_set_text(desc, speed_desc[speed_idx]);

    uint8_t line_cnt = 1;
    const char *desc_text = speed_desc[speed_idx];
    for (const char *p = desc_text; *p != '\0'; p++) {
        if (*p == '\n') {
            line_cnt++;
        }
    }
    lv_coord_t desc_y = SPEED_DESC_BASE_Y_OFFSET;
    if (line_cnt > SPEED_DESC_BASE_LINE_COUNT) {
        const lv_font_t *font = lv_obj_get_style_text_font(desc, 0);
        lv_coord_t line_h = font != NULL ? lv_font_get_line_height(font) : 14;
        lv_coord_t line_space = lv_obj_get_style_text_line_space(desc, 0);
        lv_coord_t extra_h = (lv_coord_t)((line_h + line_space) * (line_cnt - SPEED_DESC_BASE_LINE_COUNT));
        desc_y = (lv_coord_t)(desc_y + (extra_h / 2));
    }
    lv_obj_align(desc, LV_ALIGN_CENTER, 0, desc_y);
    speed_info->slot_speed_idx[slot] = speed_idx;
}

static void circle_layout_apply(int8_t direction, bool anim)
{
    for (uint8_t i = 0; i < SPEED_CIRCLE_SLOT_COUNT; i++) {
        lv_obj_t *item = speed_info->circle_obj[i];
        if (item == NULL || !lv_obj_is_valid(item)) {
            continue;
        }

        int32_t to_x = slot_target_x((speed_circle_slot_e)i);
        int32_t to_y = slot_target_y((speed_circle_slot_e)i);
        int32_t from_x = to_x;
        int32_t from_y = to_y;

        if (anim && direction != 0) {
            if (direction > 0) { /// 向左移动
                if (i == SPEED_SLOT_LEFT) {
                    from_x = slot_target_x(SPEED_SLOT_CENTER);
                    from_y = slot_target_y(SPEED_SLOT_CENTER);
                } else if (i == SPEED_SLOT_CENTER) {
                    from_x = slot_target_x(SPEED_SLOT_RIGHT);
                    from_y = slot_target_y(SPEED_SLOT_RIGHT);
                } else {
                    from_x = slot_target_x(SPEED_SLOT_RIGHT) + SPEED_CIRCLE_CENTER_X_GAP;
                    from_y = slot_target_y(SPEED_SLOT_RIGHT);
                }
            } else { /// 向右移动
                if (i == SPEED_SLOT_RIGHT) {
                    from_x = slot_target_x(SPEED_SLOT_CENTER);
                    from_y = slot_target_y(SPEED_SLOT_CENTER);
                } else if (i == SPEED_SLOT_CENTER) {
                    from_x = slot_target_x(SPEED_SLOT_LEFT);
                    from_y = slot_target_y(SPEED_SLOT_LEFT);
                } else {
                    from_x = slot_target_x(SPEED_SLOT_LEFT) - SPEED_CIRCLE_CENTER_X_GAP;
                    from_y = slot_target_y(SPEED_SLOT_LEFT);
                }
            }
        }

        if (!anim || direction == 0) {
            lv_obj_set_pos(item, (lv_coord_t)to_x, (lv_coord_t)to_y);
            apply_slot_children_opa(i);
            continue;
        }

        lv_obj_set_pos(item, (lv_coord_t)from_x, (lv_coord_t)from_y);
        lv_anim_del(item, item_anim_exec_x);
        lv_anim_del(item, item_anim_exec_y);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, item);
        lv_anim_set_time(&a, SPEED_SWITCH_ANIM_TIME_MS);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);

        lv_anim_set_exec_cb(&a, item_anim_exec_x);
        lv_anim_set_values(&a, from_x, to_x);
        lv_anim_start(&a);

        lv_anim_set_exec_cb(&a, item_anim_exec_y);
        lv_anim_set_values(&a, from_y, to_y);
        lv_anim_start(&a);
    }
    apply_all_children_opa();
}

static void circle_carousel_set_speed(ui_manager_context_t *thiz, uint8_t idx, bool anim)
{
    if (idx >= SPEED_COUNT) {
        idx = SPEED_COUNT - 1;
    }

    int8_t direction = 0;
    uint8_t from = speed_info->cur_speed;
    uint8_t cw_steps = (uint8_t)((idx + SPEED_COUNT - from) % SPEED_COUNT);
    uint8_t ccw_steps = (uint8_t)((from + SPEED_COUNT - idx) % SPEED_COUNT);
    if (cw_steps == 1) {
        direction = 1;
    } else if (ccw_steps == 1) {
        direction = -1;
    } else if (cw_steps == 0) {
        direction = 0;
    } else {
        direction = 0; /// 非相邻目标直接落位，避免连续补间
        anim = false;
    }

    update_slot_content(thiz, SPEED_SLOT_LEFT, slot_speed_for_center(idx, SPEED_SLOT_LEFT));
    update_slot_content(thiz, SPEED_SLOT_CENTER, slot_speed_for_center(idx, SPEED_SLOT_CENTER));
    update_slot_content(thiz, SPEED_SLOT_RIGHT, slot_speed_for_center(idx, SPEED_SLOT_RIGHT));
    circle_layout_apply(direction, anim);

    speed_info->cur_speed = idx;
    update_dots(idx);
}

#if IS_SUPPORT_SPEED_BG_IMG
static void create_speed_bg_by_lvgl_snapshot(ui_manager_context_t *thiz, lv_obj_t *parent, speed_circle_slot_e slot)
{
#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = thiz->get_lvgl_snap_shot(slot == SPEED_SLOT_CENTER ? \
         UI_SNAP_SHOT_PUMP_MODE_CENTER_BG : UI_SNAP_SHOT_PUMP_MODE_SIDE_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
#endif
    if (speed_info->circle_obj[slot] == NULL || !lv_obj_is_valid(speed_info->circle_obj[slot])) {
        speed_info->circle_obj[slot] = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(speed_info->circle_obj[slot]);
    }
#if LV_USE_SNAPSHOT
    lv_image_set_src(speed_info->circle_obj[slot], background_snapshot);
#else
    lv_obj_add_flag(speed_info->circle_obj[slot], LV_OBJ_FLAG_HIDDEN); /// 隐藏
#endif
    lv_obj_set_pos(speed_info->circle_obj[slot], (lv_coord_t)slot_target_x(slot), (lv_coord_t)slot_target_y(slot));
    lv_obj_set_style_image_opa(speed_info->circle_obj[slot], LV_OPA_COVER, 0);
}
#else
static void create_speed_bg_by_lvgl_draw(ui_manager_context_t *thiz, lv_obj_t *parent, speed_circle_slot_e slot)
{
    if (speed_info->circle_obj[slot] == NULL || !lv_obj_is_valid(speed_info->circle_obj[slot])) {
        speed_info->circle_obj[slot] = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(speed_info->circle_obj[slot]);
    }
    lv_obj_remove_style_all(speed_info->circle_obj[slot]);
    lv_obj_set_size(speed_info->circle_obj[slot], SPEED_CIRCLE_W, SPEED_CIRCLE_H); /// 设置白点大小
    lv_obj_set_style_radius(speed_info->circle_obj[slot], LV_RADIUS_CIRCLE, 0); /// 圆角
    lv_obj_set_style_bg_color(speed_info->circle_obj[slot], 
        lv_color_hex(slot == SPEED_SLOT_CENTER ? SPEED_CENTER_CIRCLE_GRAD_TOP_COLOR : SPEED_SIDE_CIRCLE_GRAD_TOP_COLOR), 0);
    lv_obj_set_style_bg_grad_color(speed_info->circle_obj[slot], 
        lv_color_hex(slot == SPEED_SLOT_CENTER ? SPEED_CENTER_CIRCLE_GRAD_BOTTOM_COLOR : SPEED_SIDE_CIRCLE_GRAD_BOTTOM_COLOR), 0);
    lv_obj_set_style_bg_grad_dir(speed_info->circle_obj[slot], LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_main_stop(speed_info->circle_obj[slot], 0, 0); /// 主颜色停止位置
    lv_obj_set_style_bg_grad_stop(speed_info->circle_obj[slot], 255, 0); /// 渐变颜色停止位置
    lv_obj_set_style_bg_opa(speed_info->circle_obj[slot], LV_OPA_COVER, 0);
}
#endif

static void create_speed_item(ui_manager_context_t *thiz, lv_obj_t *parent, speed_circle_slot_e slot)
{
#if IS_SUPPORT_SPEED_BG_IMG
    create_speed_bg_by_lvgl_snapshot(thiz, parent, slot);
#else
    create_speed_bg_by_lvgl_draw(thiz, parent, slot);
#endif

    if (speed_info->speed_component_img[slot] == NULL || !lv_obj_is_valid(speed_info->speed_component_img[slot])) {
        speed_info->speed_component_img[slot] = lv_image_create(speed_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(speed_info->speed_component_img[slot]);
        lv_obj_align(speed_info->speed_component_img[slot], LV_ALIGN_CENTER, 0, -58);
    }

    if (speed_info->speed_title_label[slot] == NULL || !lv_obj_is_valid(speed_info->speed_title_label[slot])) {
        speed_info->speed_title_label[slot] = lv_label_create(speed_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(speed_info->speed_title_label[slot]);
        lv_obj_set_style_text_color(speed_info->speed_title_label[slot], lv_color_hex(SPEED_TITLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(speed_info->speed_title_label[slot], &lv_font_oppo_sans_16_medium, 0);
        lv_obj_set_style_text_align(speed_info->speed_title_label[slot], LV_TEXT_ALIGN_CENTER, 0);  /// 居中对齐
        lv_obj_align(speed_info->speed_title_label[slot], LV_ALIGN_CENTER, 0, -15);
        lv_obj_set_style_text_opa(speed_info->speed_title_label[slot], LV_OPA_COVER, 0);
    }

    if (speed_info->speed_desc_label[slot] == NULL || !lv_obj_is_valid(speed_info->speed_desc_label[slot])) {
        speed_info->speed_desc_label[slot] = lv_label_create(speed_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(speed_info->speed_desc_label[slot]);
        lv_obj_set_style_text_color(speed_info->speed_desc_label[slot], 
            lv_color_hex(slot == SPEED_SLOT_CENTER ? SPEED_CENTER_DESC_TEXT_COLOR : SPEED_DESC_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(speed_info->speed_desc_label[slot], &lv_font_oppo_sans_14_medium, 0);
        lv_obj_set_style_text_align(speed_info->speed_desc_label[slot], LV_TEXT_ALIGN_CENTER, 0);  /// 居中对齐
        lv_obj_set_style_text_line_space(speed_info->speed_desc_label[slot], 4, 0); /// 设置行间距（像素）
        lv_obj_set_style_text_opa(speed_info->speed_desc_label[slot], LV_OPA_COVER, 0);
        lv_obj_align(speed_info->speed_desc_label[slot], LV_ALIGN_CENTER, 0, SPEED_DESC_BASE_Y_OFFSET);
    }
}

static void circle_carousel_create(ui_manager_context_t *thiz, lv_obj_t *parent, uint8_t speed_idx)
{
    for (uint8_t i = 0; i < SPEED_CIRCLE_SLOT_COUNT; i++) {
        speed_info->slot_speed_idx[i] = -1;
        create_speed_item(thiz, parent, (speed_circle_slot_e)i); /// 创建圆圈
        EXCEPTION_CHECK_VOID(speed_info->circle_obj[i]);
        EXCEPTION_CHECK_VOID(speed_info->speed_component_img[i]);
        EXCEPTION_CHECK_VOID(speed_info->speed_title_label[i]);
        EXCEPTION_CHECK_VOID(speed_info->speed_desc_label[i]);
    }
    circle_carousel_set_speed(thiz, speed_idx, false); /// 设置模式
}

static void dotbar_carousel_create(ui_manager_context_t *thiz, lv_obj_t *parent, uint8_t speed_idx)
{
    lv_obj_t *dotbar = lv_obj_create(parent);
    EXCEPTION_CHECK_VOID(dotbar);
    lv_obj_set_size(dotbar, 128, 16); /// 92x16
    lv_obj_align_to(dotbar, speed_info->root, LV_ALIGN_OUT_BOTTOM_MID, 0, -36); /// 底部居中对齐
    lv_obj_clear_flag(dotbar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(dotbar, LV_OPA_TRANSP, 0); /// 透明
    lv_obj_set_style_border_opa(dotbar, LV_OPA_TRANSP, 0); /// 透明
    lv_obj_set_style_pad_all(dotbar, 0, 0); /// 内边距为0
    lv_obj_set_style_pad_gap(dotbar, 8, 0); /// 8px 间距
    lv_obj_set_flex_flow(dotbar, LV_FLEX_FLOW_ROW); /// 行布局
    lv_obj_set_flex_align(dotbar, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); /// 居中对齐
    for (uint8_t i = 0; i < SPEED_COUNT; i++) {
        speed_info->dots[i] = lv_obj_create(dotbar);
        lv_obj_set_size(speed_info->dots[i], 8, 8);
        lv_obj_set_style_radius(speed_info->dots[i], LV_RADIUS_CIRCLE, 0); /// 圆形
        lv_obj_set_style_bg_color(speed_info->dots[i], lv_color_hex(SPEED_DOT_COLOR), 0);
        lv_obj_set_style_border_opa(speed_info->dots[i], LV_OPA_TRANSP, 0); /// 透明
    }
    update_dots(speed_idx);
}

static void speed_root_init(ui_manager_context_t *thiz)
{
    if (speed_info->root == NULL || !lv_obj_is_valid(speed_info->root)) {
        speed_info->root = lv_obj_create(lv_screen_active());
        EXCEPTION_CHECK_VOID(speed_info->root);
        lv_obj_set_size(speed_info->root, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(speed_info->root, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(speed_info->root, LV_OPA_COVER, 0);
        // lv_obj_set_style_radius(speed_info->root, LV_RADIUS_CIRCLE, 0); /// 圆角
        // lv_obj_set_style_clip_corner(speed_info->root, true, 0); /// 启用圆角裁剪
        lv_obj_set_style_clip_corner(speed_info->root, false, 0); /// 关闭圆角裁剪
        lv_obj_set_style_border_width(speed_info->root, 0, 0);
        lv_obj_set_style_pad_all(speed_info->root, 0, 0);
        lv_obj_clear_flag(speed_info->root, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(speed_info->root, LV_OBJ_FLAG_CLICKABLE);
    }
}

#if IS_SUPPORT_SPEED_COMP_BG
static void speed_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (speed_info->backgroud == NULL || !lv_obj_is_valid(speed_info->backgroud)) {
        speed_info->backgroud = lv_image_create(speed_info->root);
        EXCEPTION_CHECK_VOID(speed_info->backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(speed_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(speed_info->backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, speed_bg_data,
            sizeof(speed_bg_data), "png");
        lv_image_set_src(speed_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(speed_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void speed_bg_update(ui_manager_context_t *thiz)
{
    common_ui_bg_update(thiz, speed_info->root);
}
#endif

static void speed_ui_update(ui_manager_context_t *thiz, uint8_t speed_idx)
{
    if (speed_info->speed_obj_init_done == false) {
        circle_carousel_create(thiz, speed_info->root, speed_idx);
        dotbar_carousel_create(thiz, speed_info->root, speed_idx);
        speed_info->cur_speed = speed_idx;
        speed_info->speed_obj_init_done = true;
        return;
    }

    if (speed_info->force_update || speed_idx != speed_info->cur_speed) {
        circle_carousel_set_speed(thiz, speed_idx, !speed_info->force_update);
    }
}

static void speed_text_update(ui_manager_context_t *thiz)
{
    if (speed_info->force_update == false) {
        return;
    }

    if (speed_info->speed_type_text == NULL || !lv_obj_is_valid(speed_info->speed_type_text)) {
        speed_info->speed_type_text = lv_label_create(speed_info->root);
        EXCEPTION_CHECK_VOID(speed_info->speed_type_text);
    }
    lv_label_set_text(speed_info->speed_type_text, "Speed");
    lv_obj_set_style_text_color(speed_info->speed_type_text, lv_color_hex(SPEED_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(speed_info->speed_type_text, &lv_font_oppo_sans_20_semibold, 0);
    lv_obj_set_style_text_align(speed_info->speed_type_text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
    lv_obj_set_style_text_opa(speed_info->speed_type_text, LV_OPA_COVER, 0);
    lv_obj_align(speed_info->speed_type_text, LV_ALIGN_TOP_MID, 0, 40); /// 顶部居中对齐，偏移34px
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_speed_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (speed_info == NULL) {
        speed_info = (speed_display_info_t *)u_malloz(sizeof(speed_display_info_t));
        EXCEPTION_CHECK_VOID(speed_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    speed_root_init(thiz);
    common_ui_bg_init(thiz);
    speed_info->force_update = true;
    speed_info->speed_obj_init_done = false;
}

void ui_display_pump_speed_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(speed_info);
    for (uint8_t i = 0; i < SPEED_CIRCLE_SLOT_COUNT; i++) {
        common_ui_img_resource_deinit(thiz, &speed_info->speed_component_info[i]);
    }
    common_ui_bg_deinit(thiz);
    LVGL_OBJ_DELETE(speed_info->root);
    u_free(speed_info);
    speed_info = NULL;
}

int ui_display_pump_speed_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_UINT(thiz);
    EXCEPTION_CHECK_UINT(speed_info);
    EXCEPTION_CHECK_INT(speed_info->root);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    speed_bg_update(thiz);
    speed_ui_update(thiz, pump_speed_convert_to_ui_speed_idx(proc->setting.speed));
    speed_text_update(thiz);
    speed_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump speed fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}