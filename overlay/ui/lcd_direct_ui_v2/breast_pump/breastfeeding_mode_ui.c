/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "src/misc/lv_color.h"
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "breast_pump_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_MODE_COMP_BG              (0)
#define BF_MODE_COUNT                        (6)
#define BF_CIRCLE_SLOT_COUNT                 (3)   /// 仅保留左/中/右 3 个圆槽位
#define BF_VIEW_W                            (360)
#define BF_VIEW_H                            (360)
#define BF_CIRCLE_W                          (220)
#define BF_CIRCLE_H                          (220)
#define BF_CIRCLE_CENTER_X_GAP               (BF_CIRCLE_W + 20)  /// 220 + 20
#define BF_SIDE_CIRCLE_Y_OFFSET              (24 + 5)
#define BF_CHILD_CENTER_OPA                  (LV_OPA_30)
#define BF_CHILD_SIDE_OPA                    (LV_OPA_20)
#define BF_DOT_COLOR                         (0xB25962)
#define BF_TEXT_COLOR                        (0xB25962)
#define BF_TEXT_CENTER_COLOR                 (0xFFFFFF)
#define BF_TITLE_TEXT_COLOR                  (0xFFFFFF)
#define BF_DESC_TEXT_COLOR                   (0xFFFFFF)
#define BF_TEXT_SIDE_COLOR                   (0xE7D4D9)
#define BF_TEXT_CENTER_OPA                   (LV_OPA_COVER)
#define BF_TEXT_SIDE_OPA                     (100)
#define BF_SWITCH_ANIM_TIME_MS               (260)
#define BF_DESC_BASE_Y_OFFSET                (33)
#define BF_DESC_BASE_LINE_COUNT              (3)
#define BF_CIRCLE_BG_EXT                     ("baj")
#define BF_MODE_COMPONENT_EXT                ("baj")

typedef enum {
    BF_SLOT_LEFT = 0,
    BF_SLOT_CENTER,
    BF_SLOT_RIGHT,
    BF_SLOT_MAX,
} bf_circle_slot_e;

typedef struct {
    lv_obj_t *root;
#if IS_SUPPORT_MODE_COMP_BG
    lv_obj_t *backgroud;
#endif
    lv_obj_t *mode_type_text;
    lv_obj_t *circle_obj[BF_CIRCLE_SLOT_COUNT]; /// 圆圈对象
    lv_obj_t *dots[BF_MODE_COUNT]; /// 模式序号点
    lv_obj_t *last_usage_icon[BF_CIRCLE_SLOT_COUNT]; /// 模式图标对象
    lv_obj_t *unconfirmed_icon[BF_CIRCLE_SLOT_COUNT]; /// 新模式未确认图标对象
    lv_obj_t *mode_component_img[BF_CIRCLE_SLOT_COUNT]; /// 模式组件图片
    lv_obj_t *mode_title_label[BF_CIRCLE_SLOT_COUNT]; /// 模式标题标签
    lv_obj_t *mode_desc_label[BF_CIRCLE_SLOT_COUNT]; /// 模式描述标签
    res_img_info_t circle_bg_info;
    lv_fs_path_ex_t circle_bg_path;
    res_img_info_t last_usage_icon_info;
    lv_fs_path_ex_t last_usage_icon_path;
    res_img_info_t unconfirmed_icon_info;
    lv_fs_path_ex_t unconfirmed_icon_path;
    res_img_info_t mode_component_info[BF_CIRCLE_SLOT_COUNT];
    lv_fs_path_ex_t mode_component_path[BF_CIRCLE_SLOT_COUNT];
    uint16_t slot_res_id[BF_CIRCLE_SLOT_COUNT];
    int16_t slot_mode_idx[BF_CIRCLE_SLOT_COUNT];
    uint8_t cur_mode;
    uint8_t force_update;
    uint8_t mode_obj_init_done;
} bf_mode_display_info_t;


/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static bf_mode_display_info_t *bf_mode_info;

static const uint16_t bf_mode_component_ids[BF_MODE_COUNT] = {
    RESOURCE_ID_MODE_CUSTOM_RDE_ICON,  
    RESOURCE_ID_MODE_CUSTOM_URD_ICON,  
    RESOURCE_ID_MODE_CUSTOM_URD_ICON,  
    RESOURCE_ID_MODE_CUSTOM_URD_ICON, 
    RESOURCE_ID_MODE_CUSTOM_URD_ICON,  
    RESOURCE_ID_MODE_CUSTOM_URD_ICON, 
};

static const char *bf_mode_name[BF_MODE_COUNT] = {
    "Custom", /// 已使用过的个性化模式
    "Regular Pumping", /// 常规韵律模式
    "Milk Boost", /// 2+8奶量提升模式
    "Power Pumping", /// PP追奶模式
    "Lactation Kickstart", /// 开奶模式
    "Emptying Boost" /// 追加排空模式
};

static const char *bf_mode_desc[BF_MODE_COUNT] = {
    "For mothers who have a\nhigh demand for milk\nproduction.",
    "Automatically transitions\nfrom fast, gentle suction\nto slow, deep suction to\nmimic a baby's natural\nnursing.",
    "Smooth, gradual suction\nto help improve low milk\nsupply.",
    "Gentle, frequent\nstimulation to trigger\nmultiple let-downs.",
    "Initiates, builds, and\nmaintains a healthy milk\nsupply.",
    "Effectively empties\nhindmilk\nto move on with ease.",
};

#if IS_SUPPORT_MODE_COMP_BG
static PSRAM_DATA_LOC uint8_t bf_mode_bg_data[] = {
    #include "comparison_png2txt/Custom3_360x360.txt"
};
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int32_t slot_target_x(bf_circle_slot_e slot)
{
    int32_t center_x = (BF_VIEW_W - BF_CIRCLE_W) / 2;
    int32_t delta = (int32_t)slot - (int32_t)BF_SLOT_CENTER;
    return center_x + delta * BF_CIRCLE_CENTER_X_GAP;
}

static int32_t slot_target_y(bf_circle_slot_e slot)
{
    int32_t center_y = (BF_VIEW_H - BF_CIRCLE_H) / 2 + 16;
    if (slot == BF_SLOT_CENTER) {
        return center_y;
    }
    /// 左右两个圆都比中间圆高 24
    return center_y - BF_SIDE_CIRCLE_Y_OFFSET;
}

static void apply_slot_children_opa(uint8_t slot)
{
    lv_obj_t *circle = bf_mode_info->circle_obj[slot];
    if (circle == NULL || !lv_obj_is_valid(circle)) {
        return;
    }
    lv_opa_t opa = (slot == BF_SLOT_CENTER) ? LV_OPA_COVER : LV_OPA_20;
    #if IS_SUPPORT_MODE_COMP_BG
        lv_obj_set_style_image_opa(circle, LV_OPA_TRANSP, 0);
    #else
        lv_obj_set_style_image_opa(circle, opa, 0);
    #endif
    // lv_obj_invalidate(circle); /// 刷新

    if (bf_mode_info->mode_component_img[slot] != NULL && lv_obj_is_valid(bf_mode_info->mode_component_img[slot])) {
        opa =(slot == BF_SLOT_CENTER) ? LV_OPA_COVER : LV_OPA_80;
        lv_obj_set_style_image_opa(bf_mode_info->mode_component_img[slot], opa, 0);;
        // lv_obj_invalidate(bf_mode_info->mode_component_img[slot]);
    }

    if (bf_mode_info->mode_title_label[slot] != NULL && lv_obj_is_valid(bf_mode_info->mode_title_label[slot])) {
        opa =(slot == BF_SLOT_CENTER) ? LV_OPA_COVER : LV_OPA_80;
        lv_obj_set_style_text_opa(bf_mode_info->mode_title_label[slot], opa, 0);
    }
    
    if (bf_mode_info->mode_desc_label[slot] != NULL && lv_obj_is_valid(bf_mode_info->mode_desc_label[slot])) {
        opa =(slot == BF_SLOT_CENTER) ? 240 : 210;
        lv_obj_set_style_text_opa(bf_mode_info->mode_desc_label[slot], opa, 0);
        // lv_obj_invalidate(bf_mode_info->mode_desc_label[slot]);
    }
}

static void apply_all_children_opa(void)
{
    for (uint8_t i = 0; i < BF_CIRCLE_SLOT_COUNT; i++) {
        apply_slot_children_opa(i);
    }
}

static void update_dots(uint8_t cur)
{
    for (uint8_t i = 0; i < BF_MODE_COUNT; i++) {
        if (bf_mode_info->dots[i] == NULL || !lv_obj_is_valid(bf_mode_info->dots[i])) {
            continue;
        }
        lv_obj_set_style_bg_opa(bf_mode_info->dots[i], (i == cur) ? LV_OPA_COVER : LV_OPA_30, 0);
    }
}

static void item_anim_exec_x(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_obj_set_x(obj, (lv_coord_t)v);
    apply_all_children_opa();
    lv_obj_invalidate(bf_mode_info->root);
}

static void item_anim_exec_y(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_obj_set_y(obj, (lv_coord_t)v);
    lv_obj_invalidate(bf_mode_info->root);
}

static int16_t slot_mode_for_center(uint8_t center_mode, bf_circle_slot_e slot)
{
    int16_t mode_idx = (int16_t)center_mode + (int16_t)slot - (int16_t)BF_SLOT_CENTER;
    while (mode_idx < 0) {
        mode_idx += BF_MODE_COUNT;
    }
    return (int16_t)(mode_idx % BF_MODE_COUNT);
}

static void update_slot_content(ui_manager_context_t *thiz, bf_circle_slot_e slot, int16_t mode_idx)
{
    lv_obj_t *item = bf_mode_info->circle_obj[slot];
    lv_obj_t *img = bf_mode_info->mode_component_img[slot];
    lv_obj_t *title = bf_mode_info->mode_title_label[slot];
    lv_obj_t *desc = bf_mode_info->mode_desc_label[slot];
    if (item == NULL || img == NULL || title == NULL || desc == NULL) {
        return;
    }

    lv_obj_clear_flag(item, LV_OBJ_FLAG_HIDDEN);
    if (bf_mode_info->slot_mode_idx[slot] == mode_idx) {
        return;
    }

    uint16_t res_id = bf_mode_component_ids[mode_idx];
    if (bf_mode_info->slot_res_id[slot] != res_id) {
        if (bf_mode_info->slot_res_id[slot] != 0) {
            common_ui_img_resource_deinit(thiz, &bf_mode_info->mode_component_info[slot]);
            memset(&bf_mode_info->mode_component_info[slot], 0, sizeof(bf_mode_info->mode_component_info[slot]));
        }
        common_ui_img_resource_init(thiz, res_id, &bf_mode_info->mode_component_info[slot]);
        if (bf_mode_info->mode_component_info[slot].data != NULL) {
            lv_fs_make_path_from_buffer(&bf_mode_info->mode_component_path[slot],
                                        LV_FS_MEMFS_LETTER,
                                        bf_mode_info->mode_component_info[slot].data,
                                        bf_mode_info->mode_component_info[slot].size,
                                        BF_MODE_COMPONENT_EXT);
            lv_image_set_src(img, (const void *)&bf_mode_info->mode_component_path[slot]);
            bf_mode_info->slot_res_id[slot] = res_id;
        }
    }

    lv_label_set_text(title, bf_mode_name[mode_idx]);
    lv_label_set_text(desc, bf_mode_desc[mode_idx]);
    /* Keep multi-line description top visually stable across modes */
    uint8_t line_cnt = 1;
    const char *desc_text = bf_mode_desc[mode_idx];
    for (const char *p = desc_text; *p != '\0'; p++) {
        if (*p == '\n') {
            line_cnt++;
        }
    }
    lv_coord_t desc_y = BF_DESC_BASE_Y_OFFSET;
    if (line_cnt > BF_DESC_BASE_LINE_COUNT) {
        const lv_font_t *font = lv_obj_get_style_text_font(desc, 0);
        lv_coord_t line_h = font != NULL ? lv_font_get_line_height(font) : 14;
        lv_coord_t line_space = lv_obj_get_style_text_line_space(desc, 0);
        lv_coord_t extra_h = (lv_coord_t)((line_h + line_space) * (line_cnt - BF_DESC_BASE_LINE_COUNT));
        desc_y = (lv_coord_t)(desc_y + (extra_h / 2));
    }
    lv_obj_align(desc, LV_ALIGN_CENTER, 0, desc_y);
    bf_mode_info->slot_mode_idx[slot] = mode_idx;
}

static void circle_layout_apply(int8_t direction, bool anim)
{
    for (uint8_t i = 0; i < BF_CIRCLE_SLOT_COUNT; i++) {
        lv_obj_t *item = bf_mode_info->circle_obj[i];
        if (item == NULL || !lv_obj_is_valid(item)) {
            continue;
        }

        int32_t to_x = slot_target_x((bf_circle_slot_e)i);
        int32_t to_y = slot_target_y((bf_circle_slot_e)i);
        int32_t from_x = to_x;
        int32_t from_y = to_y;

        if (anim && direction != 0) {
            if (direction > 0) { /// 切到下一个模式，视觉上整体向左移动
                if (i == BF_SLOT_LEFT) {
                    from_x = slot_target_x(BF_SLOT_CENTER);
                    from_y = slot_target_y(BF_SLOT_CENTER);
                } else if (i == BF_SLOT_CENTER) {
                    from_x = slot_target_x(BF_SLOT_RIGHT);
                    from_y = slot_target_y(BF_SLOT_RIGHT);
                } else {
                    from_x = slot_target_x(BF_SLOT_RIGHT) + BF_CIRCLE_CENTER_X_GAP;
                    from_y = slot_target_y(BF_SLOT_RIGHT);
                }
            } else { /// 切到上一个模式，视觉上整体向右移动
                if (i == BF_SLOT_RIGHT) {
                    from_x = slot_target_x(BF_SLOT_CENTER);
                    from_y = slot_target_y(BF_SLOT_CENTER);
                } else if (i == BF_SLOT_CENTER) {
                    from_x = slot_target_x(BF_SLOT_LEFT);
                    from_y = slot_target_y(BF_SLOT_LEFT);
                } else {
                    from_x = slot_target_x(BF_SLOT_LEFT) - BF_CIRCLE_CENTER_X_GAP;
                    from_y = slot_target_y(BF_SLOT_LEFT);
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
        lv_anim_set_time(&a, BF_SWITCH_ANIM_TIME_MS);
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
    if (idx >= BF_MODE_COUNT) {
        idx = BF_MODE_COUNT - 1;
    }

    int8_t direction = 0;
    uint8_t from = bf_mode_info->cur_mode;
    uint8_t cw_steps = (uint8_t)((idx + BF_MODE_COUNT - from) % BF_MODE_COUNT);
    uint8_t ccw_steps = (uint8_t)((from + BF_MODE_COUNT - idx) % BF_MODE_COUNT);
    if (cw_steps == 1) {
        direction = 1;
    } else if (ccw_steps == 1) {
        direction = -1;
    } else if (cw_steps == 0) {
        direction = 0;
    } else {
        /// 非相邻目标直接落位，避免连续补间
        direction = 0;
        anim = false;
    }

    update_slot_content(thiz, BF_SLOT_LEFT, slot_mode_for_center(idx, BF_SLOT_LEFT));
    update_slot_content(thiz, BF_SLOT_CENTER, slot_mode_for_center(idx, BF_SLOT_CENTER));
    update_slot_content(thiz, BF_SLOT_RIGHT, slot_mode_for_center(idx, BF_SLOT_RIGHT));
    circle_layout_apply(direction, anim);

    bf_mode_info->cur_mode = idx;
    update_dots(idx);
}

static void create_mode_item(ui_manager_context_t *thiz, lv_obj_t *parent, bf_circle_slot_e slot)
{
    /// 模式背景图标资源
    if (bf_mode_info->circle_bg_info.data == NULL) {
        common_ui_img_resource_init(thiz, RESOURCE_ID_MODE_BG_ICON, &bf_mode_info->circle_bg_info);
        EXCEPTION_CHECK_VOID(bf_mode_info->circle_bg_info.data);
        lv_fs_make_path_from_buffer(&bf_mode_info->circle_bg_path,
                                    LV_FS_MEMFS_LETTER,
                                    bf_mode_info->circle_bg_info.data,
                                    bf_mode_info->circle_bg_info.size,
                                    BF_CIRCLE_BG_EXT);
    }
    /// 模式背景图标对象
    if (bf_mode_info->circle_obj[slot] == NULL || !lv_obj_is_valid(bf_mode_info->circle_obj[slot])) {
        bf_mode_info->circle_obj[slot] = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(bf_mode_info->circle_obj[slot]);
        lv_image_set_src(bf_mode_info->circle_obj[slot], (const void *)&bf_mode_info->circle_bg_path);
        lv_obj_set_pos(bf_mode_info->circle_obj[slot], (lv_coord_t)slot_target_x(slot), (lv_coord_t)slot_target_y(slot));
    }
    /// 上一次使用模式图标资源  
    if (bf_mode_info->last_usage_icon_info.data == NULL) {
        common_ui_img_resource_init(thiz, RESOURCE_ID_LAST_USED_MODE_ICON, &bf_mode_info->last_usage_icon_info);
        EXCEPTION_CHECK_VOID(bf_mode_info->last_usage_icon_info.data);
        lv_fs_make_path_from_buffer(&bf_mode_info->last_usage_icon_path,
                                    LV_FS_MEMFS_LETTER,
                                    bf_mode_info->last_usage_icon_info.data,
                                    bf_mode_info->last_usage_icon_info.size,
                                    "baj");
    }
    /// 上一次使用模式图标对象
    if (bf_mode_info->last_usage_icon[slot] == NULL || !lv_obj_is_valid(bf_mode_info->last_usage_icon[slot])) {
        bf_mode_info->last_usage_icon[slot] = lv_image_create(bf_mode_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(bf_mode_info->last_usage_icon[slot]);
        lv_image_set_src(bf_mode_info->last_usage_icon[slot], (const void *)&bf_mode_info->last_usage_icon_path);
        lv_obj_align(bf_mode_info->last_usage_icon[slot], LV_ALIGN_CENTER, 22, -72);
        lv_obj_set_style_image_opa(bf_mode_info->last_usage_icon[slot], LV_OPA_COVER, 0);
        lv_obj_add_flag(bf_mode_info->last_usage_icon[slot], LV_OBJ_FLAG_HIDDEN);
    }

    /// 新模式未确认图标资源
    if (bf_mode_info->unconfirmed_icon_info.data == NULL) {
        common_ui_img_resource_init(thiz, RESOURCE_ID_NEW_MODE_UNCONFIRMED_ICON, &bf_mode_info->unconfirmed_icon_info);
        EXCEPTION_CHECK_VOID(bf_mode_info->unconfirmed_icon_info.data);
        lv_fs_make_path_from_buffer(&bf_mode_info->unconfirmed_icon_path,
                                    LV_FS_MEMFS_LETTER,
                                    bf_mode_info->unconfirmed_icon_info.data,
                                    bf_mode_info->unconfirmed_icon_info.size,
                                    "baj");
    }
    /// 新模式未确认图标
    if (bf_mode_info->unconfirmed_icon[slot] == NULL || !lv_obj_is_valid(bf_mode_info->unconfirmed_icon[slot])) {
        bf_mode_info->unconfirmed_icon[slot] = lv_image_create(bf_mode_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(bf_mode_info->unconfirmed_icon[slot]);
        lv_image_set_src(bf_mode_info->unconfirmed_icon[slot], (const void *)&bf_mode_info->unconfirmed_icon_path);
        lv_obj_align(bf_mode_info->unconfirmed_icon[slot], LV_ALIGN_CENTER, 16, -76);
        lv_obj_set_style_image_opa(bf_mode_info->unconfirmed_icon[slot], LV_OPA_COVER, 0);
        lv_obj_add_flag(bf_mode_info->unconfirmed_icon[slot], LV_OBJ_FLAG_HIDDEN);
    }
    /// 模式组件图片对象
    if (bf_mode_info->mode_component_img[slot] == NULL || !lv_obj_is_valid(bf_mode_info->mode_component_img[slot])) {
        bf_mode_info->mode_component_img[slot] = lv_image_create(bf_mode_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(bf_mode_info->mode_component_img[slot]);
        lv_obj_align(bf_mode_info->mode_component_img[slot], LV_ALIGN_CENTER, 0, -58);
    }
    /// 模式标题标签
    if (bf_mode_info->mode_title_label[slot] == NULL || !lv_obj_is_valid(bf_mode_info->mode_title_label[slot])) {
        bf_mode_info->mode_title_label[slot] = lv_label_create(bf_mode_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(bf_mode_info->mode_title_label[slot]);
        lv_obj_set_style_text_color(bf_mode_info->mode_title_label[slot], lv_color_hex(BF_TITLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(bf_mode_info->mode_title_label[slot], &lv_font_oppo_sans_16_medium, 0);
        lv_obj_set_style_text_align(bf_mode_info->mode_title_label[slot], LV_TEXT_ALIGN_CENTER, 0);  /// 居中对齐
        lv_obj_align(bf_mode_info->mode_title_label[slot], LV_ALIGN_CENTER, 0, -15);
    }
    /// 模式描述标签
    if (bf_mode_info->mode_desc_label[slot] == NULL || !lv_obj_is_valid(bf_mode_info->mode_desc_label[slot])) {
        bf_mode_info->mode_desc_label[slot] = lv_label_create(bf_mode_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(bf_mode_info->mode_desc_label[slot]);
        lv_obj_set_style_text_color(bf_mode_info->mode_desc_label[slot], lv_color_hex(BF_DESC_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(bf_mode_info->mode_desc_label[slot], &lv_font_oppo_sans_14_medium, 0);
        lv_obj_set_style_text_align(bf_mode_info->mode_desc_label[slot], LV_TEXT_ALIGN_CENTER, 0);  /// 居中对齐
        lv_obj_set_style_text_line_space(bf_mode_info->mode_desc_label[slot], 4, 0); /// 设置行间距（像素）
        lv_obj_set_style_text_opa(bf_mode_info->mode_desc_label[slot], 240, 0); /// 94% 透明度
        lv_obj_align(bf_mode_info->mode_desc_label[slot], LV_ALIGN_CENTER, 0, BF_DESC_BASE_Y_OFFSET);
    }
}

static void circle_carousel_create(ui_manager_context_t *thiz, lv_obj_t *parent, uint8_t mode_idx)
{
    for (uint8_t i = 0; i < BF_CIRCLE_SLOT_COUNT; i++) {
        bf_mode_info->slot_mode_idx[i] = -1;
        create_mode_item(thiz, parent, (bf_circle_slot_e)i); /// 创建圆圈
        EXCEPTION_CHECK_VOID(bf_mode_info->circle_obj[i]);
        EXCEPTION_CHECK_VOID(bf_mode_info->mode_component_img[i]);
        EXCEPTION_CHECK_VOID(bf_mode_info->mode_title_label[i]);
        EXCEPTION_CHECK_VOID(bf_mode_info->mode_desc_label[i]);
    }
    circle_carousel_set_mode(thiz, mode_idx, false); /// 设置模式
}

static void dotbar_carousel_create(ui_manager_context_t *thiz, lv_obj_t *parent, uint8_t mode_idx)
{
    lv_obj_t *dotbar = lv_obj_create(parent);
    EXCEPTION_CHECK_VOID(dotbar);
    lv_obj_set_size(dotbar, 128, 16); /// 92x16
    lv_obj_align_to(dotbar, bf_mode_info->root, LV_ALIGN_OUT_BOTTOM_MID, 0, -36); /// 底部居中对齐
    lv_obj_clear_flag(dotbar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(dotbar, LV_OPA_TRANSP, 0); /// 透明
    lv_obj_set_style_border_opa(dotbar, LV_OPA_TRANSP, 0); /// 透明
    lv_obj_set_style_pad_all(dotbar, 0, 0); /// 内边距为0
    lv_obj_set_style_pad_gap(dotbar, 8, 0); /// 8px 间距
    lv_obj_set_flex_flow(dotbar, LV_FLEX_FLOW_ROW); /// 行布局
    lv_obj_set_flex_align(dotbar, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); /// 居中对齐
    for (uint8_t i = 0; i < BF_MODE_COUNT; i++) {
        bf_mode_info->dots[i] = lv_obj_create(dotbar);
        lv_obj_set_size(bf_mode_info->dots[i], 8, 8);
        lv_obj_set_style_radius(bf_mode_info->dots[i], LV_RADIUS_CIRCLE, 0); /// 圆形
        lv_obj_set_style_bg_color(bf_mode_info->dots[i], lv_color_hex(BF_DOT_COLOR), 0);
        lv_obj_set_style_border_opa(bf_mode_info->dots[i], LV_OPA_TRANSP, 0); /// 透明
    }
    update_dots(mode_idx);
}

static void bf_mode_root_init(ui_manager_context_t *thiz)
{
    if (bf_mode_info->root == NULL || !lv_obj_is_valid(bf_mode_info->root)) {
        bf_mode_info->root = lv_obj_create(lv_screen_active());
        EXCEPTION_CHECK_VOID(bf_mode_info->root);
        lv_obj_set_size(bf_mode_info->root, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(bf_mode_info->root, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(bf_mode_info->root, LV_OPA_COVER, 0);
        // lv_obj_set_style_radius(bf_mode_info->root, LV_RADIUS_CIRCLE, 0); /// 圆角
        // lv_obj_set_style_clip_corner(bf_mode_info->root, true, 0); /// 启用圆角裁剪
        lv_obj_set_style_clip_corner(bf_mode_info->root, false, 0); /// 关闭圆角裁剪
        lv_obj_set_style_border_width(bf_mode_info->root, 0, 0);
        lv_obj_set_style_pad_all(bf_mode_info->root, 0, 0);
        lv_obj_clear_flag(bf_mode_info->root, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(bf_mode_info->root, LV_OBJ_FLAG_CLICKABLE);
    }
}

#if IS_SUPPORT_MODE_COMP_BG
static void bf_mode_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (bf_mode_info->backgroud == NULL || !lv_obj_is_valid(bf_mode_info->backgroud)) {
        bf_mode_info->backgroud = lv_image_create(bf_mode_info->root);
        EXCEPTION_CHECK_VOID(bf_mode_info->backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(bf_mode_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(bf_mode_info->backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, bf_mode_bg_data,
            sizeof(bf_mode_bg_data), "png");
        lv_image_set_src(bf_mode_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(bf_mode_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void bf_mode_bg_update(ui_manager_context_t *thiz)
{
    common_ui_bg_update(thiz, bf_mode_info->root);
}
#endif

static void bf_mode_ui_update(ui_manager_context_t *thiz, uint8_t mode_idx)
{
    if (PUMP_IS_BREASTFEEDING_MODE(mode_idx) == false) {
        mode_idx = BREAST_PUMP_PP_MILK_TRACK_MODE;
    }
    uint8_t target_idx = mode_idx - BREAST_PUMP_CUSTOM_MODE;
    if (bf_mode_info->mode_obj_init_done == false) {
        circle_carousel_create(thiz, bf_mode_info->root, target_idx);
        dotbar_carousel_create(thiz, bf_mode_info->root, target_idx);
        bf_mode_info->cur_mode = target_idx;
        bf_mode_info->mode_obj_init_done = true;
        return;
    }

    if (bf_mode_info->force_update || target_idx != bf_mode_info->cur_mode) {
        circle_carousel_set_mode(thiz, target_idx, !bf_mode_info->force_update);
    }
}

static void bf_mode_text_update(ui_manager_context_t *thiz)
{
    if (bf_mode_info->force_update == false) {
        return;
    }

    if (bf_mode_info->mode_type_text == NULL || !lv_obj_is_valid(bf_mode_info->mode_type_text)) {
        bf_mode_info->mode_type_text = lv_label_create(bf_mode_info->root);
        EXCEPTION_CHECK_VOID(bf_mode_info->mode_type_text);
    }
    lv_label_set_text(bf_mode_info->mode_type_text, "Program");
    lv_obj_set_style_text_color(bf_mode_info->mode_type_text, lv_color_hex(BF_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(bf_mode_info->mode_type_text, &lv_font_oppo_sans_20_semibold, 0);
    lv_obj_set_style_text_align(bf_mode_info->mode_type_text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
    lv_obj_set_style_text_opa(bf_mode_info->mode_type_text, LV_OPA_COVER, 0);
    lv_obj_align(bf_mode_info->mode_type_text, LV_ALIGN_TOP_MID, 0, 40); /// 顶部居中对齐，偏移34px
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
    bf_mode_root_init(thiz);
    common_ui_bg_init(thiz);
    bf_mode_info->force_update = true;
    bf_mode_info->mode_obj_init_done = false;
}

void ui_display_breastfeeding_mode_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(bf_mode_info);
    common_ui_img_resource_deinit(thiz, &bf_mode_info->circle_bg_info);
    common_ui_img_resource_deinit(thiz, &bf_mode_info->last_usage_icon_info);
    common_ui_img_resource_deinit(thiz, &bf_mode_info->unconfirmed_icon_info);
    for (uint8_t i = 0; i < BF_CIRCLE_SLOT_COUNT; i++) {
        common_ui_img_resource_deinit(thiz, &bf_mode_info->mode_component_info[i]);
    }
    common_ui_bg_deinit(thiz);
    LVGL_OBJ_DELETE(bf_mode_info->root);
    u_free(bf_mode_info);
    bf_mode_info = NULL;
}

int ui_display_breastfeeding_mode_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(bf_mode_info);
    EXCEPTION_CHECK_INT(bf_mode_info->root);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    uint8_t mode_idx = proc->ui_breastfeeding_mode_index;
    bf_mode_bg_update(thiz);
    bf_mode_ui_update(thiz, mode_idx);
    bf_mode_text_update(thiz);
    bf_mode_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> bf mode fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
