/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_MODE_COMP_BG                (0)
#define IS_SUPPORT_MODE_BG_IMG                 (0)
#define MODE_COUNT                             (3)
#define MODE_CIRCLE_SLOT_COUNT                 (3)   /// 仅保留左/中/右 3 个圆槽位
#define MODE_VIEW_W                            (360)
#define MODE_VIEW_H                            (360)
#define MODE_CIRCLE_W                          (220)
#define MODE_CIRCLE_H                          (220)
#define MODE_CIRCLE_CENTER_X_GAP               (MODE_CIRCLE_W + 20)  /// 220 + 20
#define MODE_SIDE_CIRCLE_Y_OFFSET              (24 + 5)
#define MODE_CHILD_CENTER_OPA                  (LV_OPA_30)
#define MODE_CHILD_SIDE_OPA                    (LV_OPA_20)
#define MODE_DOT_COLOR                         (0xB25962)
#define MODE_TEXT_COLOR                        (0xB25962)
#define MODE_CENTER_CIRCLE_GRAD_TOP_COLOR      (0xEB7581)
#define MODE_CENTER_CIRCLE_GRAD_BOTTOM_COLOR   (0xF0868A)
#define MODE_SIDE_CIRCLE_GRAD_TOP_COLOR        (0xF7A1A8)
#define MODE_SIDE_CIRCLE_GRAD_BOTTOM_COLOR     (0xF7A1A8)
#define MODE_TEXT_CENTER_COLOR                 (0xFFFFFF)
#define MODE_TITLE_TEXT_COLOR                  (0xFFFFFF)
#define MODE_TEXT_SIDE_COLOR                   (0xE7D4D9)
#define MODE_DESC_TEXT_COLOR                   (0xFFDDDF)
#define MODE_CENTER_DESC_TEXT_COLOR            (0xFFEAEA)
#define MODE_TEXT_CENTER_OPA                   (LV_OPA_COVER)
#define MODE_TEXT_SIDE_OPA                     (100)
#define MODE_SWITCH_ANIM_TIME_MS               (260)
#define MODE_DESC_BASE_Y_OFFSET                (33)
#define MODE_DESC_BASE_LINE_COUNT              (3)
#define MODE_CIRCLE_BG_EXT                     ("baj")
#define MODE_COMPONENT_EXT                     ("baj")

typedef enum {
    MODE_SLOT_LEFT = 0,
    MODE_SLOT_CENTER,
    MODE_SLOT_RIGHT,
    MODE_SLOT_MAX,
} mode_circle_slot_e;

typedef struct ui_display_mode_info {
    lv_obj_t * root;
#if IS_SUPPORT_MODE_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t *mode_type_text;
    lv_obj_t *circle_obj[MODE_CIRCLE_SLOT_COUNT]; /// 圆圈对象
    lv_obj_t *dots[MODE_COUNT]; /// 模式序号点
    lv_obj_t *mode_component_img[MODE_CIRCLE_SLOT_COUNT]; /// 模式组件图片
    lv_obj_t *mode_title_label[MODE_CIRCLE_SLOT_COUNT]; /// 模式标题标签
    lv_obj_t *mode_desc_label[MODE_CIRCLE_SLOT_COUNT]; /// 模式描述标签
    res_img_info_t mode_component_info[MODE_CIRCLE_SLOT_COUNT];
    lv_fs_path_ex_t mode_component_path[MODE_CIRCLE_SLOT_COUNT];
    uint16_t slot_res_id[MODE_CIRCLE_SLOT_COUNT];
    int16_t slot_mode_idx[MODE_CIRCLE_SLOT_COUNT];
    uint8_t cur_mode;
    uint8_t force_update;
    uint8_t mode_obj_init_done;
} mode_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static mode_display_info_t *mode_info;

static const uint16_t mode_component_ids[MODE_COUNT] = {
    RESOURCE_ID_MODE_STIMULATE_ICON, /// 舒适
    RESOURCE_ID_MODE_LACTATION_ICON, /// 常规
    RESOURCE_ID_MODE_MIX_ICON, /// 快速
};

static const char *mode_name[MODE_COUNT] = {
    "Stimulation", /// 慢速
    "Expression", /// 常规
    "Mixed", /// 快速
};

static const char *mode_desc[MODE_COUNT] = {
    "Fast, gentle suction to\ntrigger let-down.",
    "Slow, deep suction for\nefficient milk expression.",
    "Alternates 5 times of fast,\ngentle suction with 1 slow,\ndeep suction for let-\ndowns and better\nemptying.",
};

#if IS_SUPPORT_MODE_COMP_BG
static PSRAM_DATA_LOC uint8_t mode_bg_data[] = {
    #include "comparison_png2txt/pump_mode_1_360x360.txt"
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
static int32_t slot_target_x(mode_circle_slot_e slot)
{
    int32_t center_x = (MODE_VIEW_W - MODE_CIRCLE_W) / 2;
    int32_t delta = (int32_t)slot - (int32_t)MODE_SLOT_CENTER;
    return center_x + delta * MODE_CIRCLE_CENTER_X_GAP;
}

static int32_t slot_target_y(mode_circle_slot_e slot)
{
    int32_t center_y = (MODE_VIEW_H - MODE_CIRCLE_H) / 2 + 16;
    if (slot == MODE_SLOT_CENTER) {
        return center_y;
    }
    /// 左右两个圆都比中间圆高 24
    return center_y - MODE_SIDE_CIRCLE_Y_OFFSET;
}

static void apply_slot_children_opa(uint8_t slot)
{
#if 0
    lv_opa_t opa = (slot == MODE_SLOT_CENTER) ? LV_OPA_COVER : 20;
    if (mode_info->circle_obj[slot] != NULL && lv_obj_is_valid(mode_info->circle_obj[slot])) {
        lv_obj_set_style_image_opa(mode_info->circle_obj[slot], opa, 0);
        lv_obj_invalidate(mode_info->circle_obj[slot]); /// 刷新
    }

    if (mode_info->mode_title_label[slot] != NULL && lv_obj_is_valid(mode_info->mode_title_label[slot])) {
        opa =(slot == MODE_SLOT_CENTER) ? LV_OPA_COVER : LV_OPA_80;
        lv_obj_set_style_text_opa(mode_info->mode_title_label[slot], opa, 0);
        lv_obj_invalidate(mode_info->mode_title_label[slot]); /// 刷新
    }
    
    if (mode_info->mode_desc_label[slot] != NULL && lv_obj_is_valid(mode_info->mode_desc_label[slot])) {
        opa =(slot == MODE_SLOT_CENTER) ? 240 : 210;
        lv_obj_set_style_text_opa(mode_info->mode_desc_label[slot], opa, 0);
        lv_obj_invalidate(mode_info->mode_desc_label[slot]); /// 刷新
    }
#endif
}

static void apply_all_children_opa(void)
{
#if 0
    for (uint8_t i = 0; i < MODE_CIRCLE_SLOT_COUNT; i++) {
        apply_slot_children_opa(i);
    }
#endif
}

static void update_dots(uint8_t cur)
{
    for (uint8_t i = 0; i < MODE_COUNT; i++) {
        if (mode_info->dots[i] == NULL || !lv_obj_is_valid(mode_info->dots[i])) {
            continue;
        }
        lv_obj_set_style_bg_opa(mode_info->dots[i], (i == cur) ? LV_OPA_COVER : LV_OPA_30, 0);
    }
}

static void item_anim_exec_x(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_obj_set_x(obj, (lv_coord_t)v);
    apply_all_children_opa();
    lv_obj_invalidate(mode_info->root);
}

static void item_anim_exec_y(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_obj_set_y(obj, (lv_coord_t)v);
    lv_obj_invalidate(mode_info->root);
}

static int16_t slot_mode_for_center(uint8_t center_mode, mode_circle_slot_e slot)
{
    int16_t mode_idx = (int16_t)center_mode + (int16_t)slot - (int16_t)MODE_SLOT_CENTER;
    while (mode_idx < 0) {
        mode_idx += MODE_COUNT;
    }
    return (int16_t)(mode_idx % MODE_COUNT);
}

static void update_slot_content(ui_manager_context_t *thiz, mode_circle_slot_e slot, int16_t mode_idx)
{
    lv_obj_t *item = mode_info->circle_obj[slot];
    lv_obj_t *img = mode_info->mode_component_img[slot];
    lv_obj_t *title = mode_info->mode_title_label[slot];
    lv_obj_t *desc = mode_info->mode_desc_label[slot];
    if (item == NULL || img == NULL || title == NULL || desc == NULL) {
        return;
    }

    lv_obj_clear_flag(item, LV_OBJ_FLAG_HIDDEN);
    if (mode_info->slot_mode_idx[slot] == mode_idx) {
        return;
    }

    uint16_t res_id = mode_component_ids[mode_idx];
    if (mode_info->slot_res_id[slot] != res_id) {
        if (mode_info->slot_res_id[slot] != 0) {
            common_ui_img_resource_deinit(thiz, &mode_info->mode_component_info[slot]);
            memset(&mode_info->mode_component_info[slot], 0, sizeof(mode_info->mode_component_info[slot]));
        }
        common_ui_img_resource_init(thiz, res_id, &mode_info->mode_component_info[slot]);
        if (mode_info->mode_component_info[slot].data != NULL) {
            lv_fs_make_path_from_buffer(&mode_info->mode_component_path[slot],
                                        LV_FS_MEMFS_LETTER,
                                        mode_info->mode_component_info[slot].data,
                                        mode_info->mode_component_info[slot].size,
                                        MODE_COMPONENT_EXT);
            lv_image_set_src(img, (const void *)&mode_info->mode_component_path[slot]);
            mode_info->slot_res_id[slot] = res_id;
        }
    }

    lv_label_set_text(title, mode_name[mode_idx]);
    lv_label_set_text(desc, mode_desc[mode_idx]);

    uint8_t line_cnt = 1;
    const char *desc_text = mode_desc[mode_idx];
    for (const char *p = desc_text; *p != '\0'; p++) {
        if (*p == '\n') {
            line_cnt++;
        }
    }
    lv_coord_t desc_y = MODE_DESC_BASE_Y_OFFSET;
    if (line_cnt > MODE_DESC_BASE_LINE_COUNT) {
        const lv_font_t *font = lv_obj_get_style_text_font(desc, 0);
        lv_coord_t line_h = font != NULL ? lv_font_get_line_height(font) : 14;
        lv_coord_t line_space = lv_obj_get_style_text_line_space(desc, 0);
        lv_coord_t extra_h = (lv_coord_t)((line_h + line_space) * (line_cnt - MODE_DESC_BASE_LINE_COUNT));
        desc_y = (lv_coord_t)(desc_y + (extra_h / 2));
    }
    lv_obj_align(desc, LV_ALIGN_CENTER, 0, desc_y);
    mode_info->slot_mode_idx[slot] = mode_idx;
}

static void circle_layout_apply(int8_t direction, bool anim)
{
    for (uint8_t i = 0; i < MODE_CIRCLE_SLOT_COUNT; i++) {
        lv_obj_t *item = mode_info->circle_obj[i];
        if (item == NULL || !lv_obj_is_valid(item)) {
            continue;
        }

        int32_t to_x = slot_target_x((mode_circle_slot_e)i);
        int32_t to_y = slot_target_y((mode_circle_slot_e)i);
        int32_t from_x = to_x;
        int32_t from_y = to_y;

        if (anim && direction != 0) {
            if (direction > 0) { /// 向左移动
                if (i == MODE_SLOT_LEFT) {
                    from_x = slot_target_x(MODE_SLOT_CENTER);
                    from_y = slot_target_y(MODE_SLOT_CENTER);
                } else if (i == MODE_SLOT_CENTER) {
                    from_x = slot_target_x(MODE_SLOT_RIGHT);
                    from_y = slot_target_y(MODE_SLOT_RIGHT);
                } else {
                    from_x = slot_target_x(MODE_SLOT_RIGHT) + MODE_CIRCLE_CENTER_X_GAP;
                    from_y = slot_target_y(MODE_SLOT_RIGHT);
                }
            } else { /// 向右移动
                if (i == MODE_SLOT_RIGHT) {
                    from_x = slot_target_x(MODE_SLOT_CENTER);
                    from_y = slot_target_y(MODE_SLOT_CENTER);
                } else if (i == MODE_SLOT_CENTER) {
                    from_x = slot_target_x(MODE_SLOT_LEFT);
                    from_y = slot_target_y(MODE_SLOT_LEFT);
                } else {
                    from_x = slot_target_x(MODE_SLOT_LEFT) - MODE_CIRCLE_CENTER_X_GAP;
                    from_y = slot_target_y(MODE_SLOT_LEFT);
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
        lv_anim_set_time(&a, MODE_SWITCH_ANIM_TIME_MS);
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

static void circle_carousel_set_mode(ui_manager_context_t *thiz, uint8_t idx, bool anim)
{
    if (idx >= MODE_COUNT) {
        idx = MODE_COUNT - 1;
    }

    int8_t direction = 0;
    uint8_t from = mode_info->cur_mode;
    uint8_t cw_steps = (uint8_t)((idx + MODE_COUNT - from) % MODE_COUNT);
    uint8_t ccw_steps = (uint8_t)((from + MODE_COUNT - idx) % MODE_COUNT);
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

    update_slot_content(thiz, MODE_SLOT_LEFT, slot_mode_for_center(idx, MODE_SLOT_LEFT));
    update_slot_content(thiz, MODE_SLOT_CENTER, slot_mode_for_center(idx, MODE_SLOT_CENTER));
    update_slot_content(thiz, MODE_SLOT_RIGHT, slot_mode_for_center(idx, MODE_SLOT_RIGHT));
    circle_layout_apply(direction, anim);

    mode_info->cur_mode = idx;
    update_dots(idx);
}

#if IS_SUPPORT_MODE_BG_IMG
static void create_mode_bg_by_lvgl_snapshot(ui_manager_context_t *thiz, lv_obj_t *parent, mode_circle_slot_e slot)
{
#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = thiz->get_lvgl_snap_shot(slot == MODE_SLOT_CENTER ? \
         UI_SNAP_SHOT_PUMP_MODE_CENTER_BG : UI_SNAP_SHOT_PUMP_MODE_SIDE_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
#endif
    if (mode_info->circle_obj[slot] == NULL || !lv_obj_is_valid(mode_info->circle_obj[slot])) {
        mode_info->circle_obj[slot] = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(mode_info->circle_obj[slot]);
    }
#if LV_USE_SNAPSHOT
    lv_image_set_src(mode_info->circle_obj[slot], background_snapshot);
#else
    lv_obj_add_flag(bf_mode_info->circle_obj[slot], LV_OBJ_FLAG_HIDDEN); /// 隐藏
#endif
    lv_obj_set_pos(mode_info->circle_obj[slot], (lv_coord_t)slot_target_x(slot), (lv_coord_t)slot_target_y(slot));
    lv_obj_set_style_image_opa(mode_info->circle_obj[slot], LV_OPA_COVER, 0);
}
#else
static void create_mode_bg_by_lvgl_draw(ui_manager_context_t *thiz, lv_obj_t *parent, mode_circle_slot_e slot)
{
    if (mode_info->circle_obj[slot] == NULL || !lv_obj_is_valid(mode_info->circle_obj[slot])) {
        mode_info->circle_obj[slot] = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(mode_info->circle_obj[slot]);
    }
    lv_obj_remove_style_all(mode_info->circle_obj[slot]);
    lv_obj_set_size(mode_info->circle_obj[slot], MODE_CIRCLE_W, MODE_CIRCLE_H); /// 设置白点大小
    lv_obj_set_style_radius(mode_info->circle_obj[slot], LV_RADIUS_CIRCLE, 0); /// 圆角
    lv_obj_set_style_bg_color(mode_info->circle_obj[slot], 
        lv_color_hex(slot == MODE_SLOT_CENTER ? MODE_CENTER_CIRCLE_GRAD_TOP_COLOR : MODE_SIDE_CIRCLE_GRAD_TOP_COLOR), 0);
    lv_obj_set_style_bg_grad_color(mode_info->circle_obj[slot], 
        lv_color_hex(slot == MODE_SLOT_CENTER ? MODE_CENTER_CIRCLE_GRAD_BOTTOM_COLOR : MODE_SIDE_CIRCLE_GRAD_BOTTOM_COLOR), 0);
    lv_obj_set_style_bg_grad_dir(mode_info->circle_obj[slot], LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_main_stop(mode_info->circle_obj[slot], 0, 0); /// 主颜色停止位置
    lv_obj_set_style_bg_grad_stop(mode_info->circle_obj[slot], 255, 0); /// 渐变颜色停止位置
    lv_obj_set_style_bg_opa(mode_info->circle_obj[slot], LV_OPA_COVER, 0);
}
#endif

static void create_mode_item(ui_manager_context_t *thiz, lv_obj_t *parent, mode_circle_slot_e slot)
{
#if IS_SUPPORT_MODE_BG_IMG
    create_mode_bg_by_lvgl_snapshot(thiz, parent, slot);
#else
    create_mode_bg_by_lvgl_draw(thiz, parent, slot);
#endif
    if (mode_info->mode_component_img[slot] == NULL || !lv_obj_is_valid(mode_info->mode_component_img[slot])) {
        mode_info->mode_component_img[slot] = lv_image_create(mode_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(mode_info->mode_component_img[slot]);
        lv_obj_align(mode_info->mode_component_img[slot], LV_ALIGN_CENTER, 0, -58);
    }

    if (mode_info->mode_title_label[slot] == NULL || !lv_obj_is_valid(mode_info->mode_title_label[slot])) {
        mode_info->mode_title_label[slot] = lv_label_create(mode_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(mode_info->mode_title_label[slot]);
        lv_obj_set_style_text_color(mode_info->mode_title_label[slot], lv_color_hex(MODE_TITLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(mode_info->mode_title_label[slot], &lv_font_oppo_sans_16_medium, 0);
        lv_obj_set_style_text_align(mode_info->mode_title_label[slot], LV_TEXT_ALIGN_CENTER, 0);  /// 居中对齐
        lv_obj_align(mode_info->mode_title_label[slot], LV_ALIGN_CENTER, 0, -15);
        lv_obj_set_style_text_opa(mode_info->mode_title_label[slot], LV_OPA_COVER, 0);
    }

    if (mode_info->mode_desc_label[slot] == NULL || !lv_obj_is_valid(mode_info->mode_desc_label[slot])) {
        mode_info->mode_desc_label[slot] = lv_label_create(mode_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(mode_info->mode_desc_label[slot]);
        lv_obj_set_style_text_color(mode_info->mode_desc_label[slot], 
            lv_color_hex(slot == MODE_SLOT_CENTER ? MODE_CENTER_DESC_TEXT_COLOR : MODE_DESC_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(mode_info->mode_desc_label[slot], &lv_font_oppo_sans_14_medium, 0);
        lv_obj_set_style_text_align(mode_info->mode_desc_label[slot], LV_TEXT_ALIGN_CENTER, 0);  /// 居中对齐
        lv_obj_set_style_text_line_space(mode_info->mode_desc_label[slot], 4, 0); /// 设置行间距（像素）
        lv_obj_set_style_text_opa(mode_info->mode_desc_label[slot], LV_OPA_COVER, 0);
        lv_obj_align(mode_info->mode_desc_label[slot], LV_ALIGN_CENTER, 0, MODE_DESC_BASE_Y_OFFSET);
    }
}

static void circle_carousel_create(ui_manager_context_t *thiz, lv_obj_t *parent, uint8_t mode_idx)
{
    for (uint8_t i = 0; i < MODE_CIRCLE_SLOT_COUNT; i++) {
        mode_info->slot_mode_idx[i] = -1;
        create_mode_item(thiz, parent, (mode_circle_slot_e)i); /// 创建圆圈
        EXCEPTION_CHECK_VOID(mode_info->circle_obj[i]);
        EXCEPTION_CHECK_VOID(mode_info->mode_component_img[i]);
        EXCEPTION_CHECK_VOID(mode_info->mode_title_label[i]);
        EXCEPTION_CHECK_VOID(mode_info->mode_desc_label[i]);
    }
    circle_carousel_set_mode(thiz, mode_idx, false); /// 设置模式
}

static void dotbar_carousel_create(ui_manager_context_t *thiz, lv_obj_t *parent, uint8_t mode_idx)
{
    lv_obj_t *dotbar = lv_obj_create(parent);
    EXCEPTION_CHECK_VOID(dotbar);
    lv_obj_set_size(dotbar, 128, 16); /// 92x16
    lv_obj_align_to(dotbar, mode_info->root, LV_ALIGN_OUT_BOTTOM_MID, 0, -36); /// 底部居中对齐
    lv_obj_clear_flag(dotbar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(dotbar, LV_OPA_TRANSP, 0); /// 透明
    lv_obj_set_style_border_opa(dotbar, LV_OPA_TRANSP, 0); /// 透明
    lv_obj_set_style_pad_all(dotbar, 0, 0); /// 内边距为0
    lv_obj_set_style_pad_gap(dotbar, 8, 0); /// 8px 间距
    lv_obj_set_flex_flow(dotbar, LV_FLEX_FLOW_ROW); /// 行布局
    lv_obj_set_flex_align(dotbar, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); /// 居中对齐
    for (uint8_t i = 0; i < MODE_COUNT; i++) {
        mode_info->dots[i] = lv_obj_create(dotbar);
        lv_obj_set_size(mode_info->dots[i], 8, 8);
        lv_obj_set_style_radius(mode_info->dots[i], LV_RADIUS_CIRCLE, 0); /// 圆形
        lv_obj_set_style_bg_color(mode_info->dots[i], lv_color_hex(MODE_DOT_COLOR), 0);
        lv_obj_set_style_border_opa(mode_info->dots[i], LV_OPA_TRANSP, 0); /// 透明
    }
    update_dots(mode_idx);
}

static void mode_root_init(ui_manager_context_t *thiz)
{
    if (mode_info->root == NULL || !lv_obj_is_valid(mode_info->root)) {
        mode_info->root = lv_obj_create(lv_screen_active());
        EXCEPTION_CHECK_VOID(mode_info->root);
        lv_obj_set_size(mode_info->root, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(mode_info->root, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(mode_info->root, LV_OPA_COVER, 0);
        // lv_obj_set_style_radius(mode_info->root, LV_RADIUS_CIRCLE, 0); /// 圆角
        // lv_obj_set_style_clip_corner(mode_info->root, true, 0); /// 启用圆角裁剪
        lv_obj_set_style_clip_corner(mode_info->root, false, 0); /// 关闭圆角裁剪
        lv_obj_set_style_border_width(mode_info->root, 0, 0);
        lv_obj_set_style_pad_all(mode_info->root, 0, 0);
        lv_obj_clear_flag(mode_info->root, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(mode_info->root, LV_OBJ_FLAG_CLICKABLE);
    }
}

#if IS_SUPPORT_MODE_COMP_BG
static void mode_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (mode_info->backgroud == NULL || !lv_obj_is_valid(mode_info->backgroud)) {
        mode_info->backgroud = lv_image_create(mode_info->root);
        EXCEPTION_CHECK_VOID(mode_info->backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(mode_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(mode_info->backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, mode_bg_data,
            sizeof(mode_bg_data), "png");
        lv_image_set_src(mode_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(mode_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void mode_bg_update(ui_manager_context_t *thiz)
{
    common_ui_bg_update(thiz, mode_info->root);
}
#endif

static void mode_ui_update(ui_manager_context_t *thiz, uint8_t mode_idx)
{
    if (mode_idx > BREAST_PUMP_MIX_MODE) {
        mode_idx = BREAST_PUMP_STIMULATION_MODE;
    }

    if (mode_info->mode_obj_init_done == false) {
        circle_carousel_create(thiz, mode_info->root, mode_idx);
        dotbar_carousel_create(thiz, mode_info->root, mode_idx);
        mode_info->cur_mode = mode_idx;
        mode_info->mode_obj_init_done = true;
        return;
    }

    if (mode_info->force_update || mode_idx != mode_info->cur_mode) {
        circle_carousel_set_mode(thiz, mode_idx, !mode_info->force_update);
    }
}

static void mode_text_update(ui_manager_context_t *thiz)
{
    if (mode_info->force_update == false) {
        return;
    }

    if (mode_info->mode_type_text == NULL || !lv_obj_is_valid(mode_info->mode_type_text)) {
        mode_info->mode_type_text = lv_label_create(mode_info->root);
        EXCEPTION_CHECK_VOID(mode_info->mode_type_text);
    }
    lv_label_set_text(mode_info->mode_type_text, "Mode");
    lv_obj_set_style_text_color(mode_info->mode_type_text, lv_color_hex(MODE_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(mode_info->mode_type_text, &lv_font_oppo_sans_20_semibold, 0);
    lv_obj_set_style_text_align(mode_info->mode_type_text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
    lv_obj_set_style_text_opa(mode_info->mode_type_text, LV_OPA_COVER, 0);
    lv_obj_align(mode_info->mode_type_text, LV_ALIGN_TOP_MID, 0, 40); /// 顶部居中对齐，偏移34px
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_mode_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (mode_info == NULL) {
        mode_info = (mode_display_info_t *)u_malloz(sizeof(mode_display_info_t));
        EXCEPTION_CHECK_VOID(mode_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    mode_root_init(thiz);
    common_ui_bg_init(thiz);
    mode_info->force_update = true;
    mode_info->mode_obj_init_done = false;
}

void ui_display_pump_mode_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(mode_info);
    for (uint8_t i = 0; i < MODE_CIRCLE_SLOT_COUNT; i++) {
        common_ui_img_resource_deinit(thiz, &mode_info->mode_component_info[i]);
    }
    common_ui_bg_deinit(thiz);
    LVGL_OBJ_DELETE(mode_info->root);
    u_free(mode_info);
    mode_info = NULL;
}

int ui_display_pump_mode_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(mode_info);
    EXCEPTION_CHECK_INT(mode_info->root);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->pump);
    uint8_t mode = proc->ui_common_mode_index; /// 当前UI模式索引.
    mode_bg_update(thiz);
    mode_ui_update(thiz, mode);
    mode_text_update(thiz);
    mode_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump mode fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}