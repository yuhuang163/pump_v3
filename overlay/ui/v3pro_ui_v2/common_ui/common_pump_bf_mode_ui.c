/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_MODE_COMP_BG              (0)
#define IS_SUPPORT_MODE_BG_IMG               (0)
#define BF_MODE_COUNT                        (6)
#define BF_CIRCLE_SLOT_COUNT                 (3)   /// 仅保留左/中/右 3 个圆槽位
#define BF_VIEW_W                            (360)
#define BF_VIEW_H                            (360)
#define BF_CIRCLE_W                          (250)
#define BF_CIRCLE_H                          (250)
#define BF_CIRCLE_CENTER_X_GAP               (BF_CIRCLE_W + 20)  /// 圆圈中心间距
#define BF_SIDE_CIRCLE_Y_OFFSET              (24)  /// 左右两个圆都比中间圆高 24
#define BF_TEXT_COLOR                        (0xB25962)
#define BF_CENTER_CIRCLE_GRAD_TOP_COLOR      (0xEB7581)
#define BF_CENTER_CIRCLE_GRAD_BOTTOM_COLOR   (0xF0868A)
#define BF_SIDE_CIRCLE_GRAD_TOP_COLOR        (0xF7A1A8)
#define BF_SIDE_CIRCLE_GRAD_BOTTOM_COLOR     (0xF7A1A8)
#define BF_TITLE_TEXT_COLOR                  (0xFFFFFF)
#define BF_DESC_TEXT_COLOR                   (0xFFDDDF)
#define BF_CENTER_DESC_TEXT_COLOR            (0xFFEAEA)

#define BF_PAGE_TITLE_Y_OFFSET               (34)
#define BF_COMPONENT_CENTER_Y_OFFSET         (-70)
#define BF_LAST_USAGE_ICON_Y_OFFSET          (BF_COMPONENT_CENTER_Y_OFFSET - 14)
#define BF_UNCONFIRMED_ICON_Y_OFFSET         (BF_COMPONENT_CENTER_Y_OFFSET - 18)
#define BF_TITLE_BASE_Y_OFFSET               (-26)
#define BF_DESC_BASE_Y_OFFSET                (6)

#define BF_SWITCH_ANIM_TIME_MS               (260)  /// 切换动画时间，单位：毫秒

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
    lv_obj_t *dots_img; /// 模式序号点图片
    lv_obj_t *last_usage_icon[BF_CIRCLE_SLOT_COUNT]; /// 模式图标对象
    lv_obj_t *unconfirmed_icon[BF_CIRCLE_SLOT_COUNT]; /// 新模式未确认图标对象
    lv_obj_t *mode_component_img[BF_CIRCLE_SLOT_COUNT]; /// 模式组件图片
    lv_obj_t *mode_title_label[BF_CIRCLE_SLOT_COUNT]; /// 模式标题标签
    lv_obj_t *mode_desc_label[BF_CIRCLE_SLOT_COUNT]; /// 模式描述标签
    res_img_info_t dots_img_info;
    res_img_info_t last_usage_icon_info;
    res_img_info_t unconfirmed_icon_info;
    res_img_info_t mode_component_info[BF_CIRCLE_SLOT_COUNT];
    breast_pump_manager_context_t *pump_ctx;
    uint32_t custom_mode_id; /// 个性化模式ID
    char custom_name[20]; /// 个性化模式名称
    uint16_t slot_res_id[BF_CIRCLE_SLOT_COUNT];
    int16_t slot_mode_idx[BF_CIRCLE_SLOT_COUNT];
    uint8_t cur_mode;
    uint8_t force_update;
    uint8_t mode_obj_init_done;
    uint8_t mode_num;
    uint8_t popup_ui_index;
    uint8_t new_step_info_flag; /// 新模式新步数信息标志 true-有新步数信息 false-无新步数信息, 使用后重置
    uint8_t rsv[2];
} bf_mode_display_info_t;


/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static bf_mode_display_info_t *bf_mode_info;

static const uint16_t bf_mode_component_ids[BF_MODE_COUNT] = {
    BASE_RES_ID_MODE_EXPERT_ICON,  
    BASE_RES_ID_MODE_EXPERT_ICON,  
    BASE_RES_ID_MODE_EXPERT_ICON,  
    BASE_RES_ID_MODE_EXPERT_ICON, 
    BASE_RES_ID_MODE_EXPERT_ICON,  
    BASE_RES_ID_MODE_CUSTOM_ICON, 
};

static const uint16_t dots_5_img_ids[BF_MODE_COUNT - 1] = {
    BASE_RES_ID_SELECTED_LOCATE_5_START,
    BASE_RES_ID_SELECTED_LOCATE_5_START + 1,
    BASE_RES_ID_SELECTED_LOCATE_5_START + 2,
    BASE_RES_ID_SELECTED_LOCATE_5_START + 3,
    BASE_RES_ID_SELECTED_LOCATE_5_END,
};

static const uint16_t dots_6_img_ids[BF_MODE_COUNT] = {
    BASE_RES_ID_SELECTED_LOCATE_6_START,
    BASE_RES_ID_SELECTED_LOCATE_6_START + 1,
    BASE_RES_ID_SELECTED_LOCATE_6_START + 2,
    BASE_RES_ID_SELECTED_LOCATE_6_START + 3,
    BASE_RES_ID_SELECTED_LOCATE_6_START + 4,
    BASE_RES_ID_SELECTED_LOCATE_6_END,
};

static const char *bf_mode_name[BF_MODE_COUNT] = {
    "Cozy Flow", /// 自然韵律模式
    "Milk Boost", /// 2+8奶量提升模式
    "Power Pumping", /// PP追奶模式
    "Lactation Starter", /// 开奶模式
    "Deep Emptying", /// 追加排空模式
    "Customize", /// 已使用过的个性化模式
};

static const char *bf_mode_desc[BF_MODE_COUNT] = {
    "Moves from quick, gentle\nsuction to slower, deeper\nsuction to follow a baby's\nnatural nursing rhythm.",
    "Smooth, gradual suction to\nsupport milk production\nduring low-supply\nmoments.",
    "Frequent, gentle\nstimulation to encourage\nmultiple let-downs and\nsupport milk supply.",
    "Initiate, build, and maintain\na consistent milk supply.",
    "Helps encourage more\ncomplete milk removal at\nthe end of a session.",
    "For mothers who have a\nhigh demand for milk\nproduction.",
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

static uint16_t get_dots_img_id(uint8_t mode_idx)
{
    if (bf_mode_info->mode_num == BF_MODE_COUNT - 1) {
        if (mode_idx >= BF_MODE_COUNT - 1) {
            mode_idx = 0;
        }
        return dots_5_img_ids[mode_idx];
    }

    if (mode_idx >= BF_MODE_COUNT) {
        mode_idx = 0;
    }
    return dots_6_img_ids[mode_idx];
}

static void update_dots(ui_manager_context_t *thiz, uint8_t mode_idx)
{
    if (bf_mode_info->dots_img == NULL || !lv_obj_is_valid(bf_mode_info->dots_img)) {
        return;
    }
    common_ui_img_resource_init(thiz, get_dots_img_id(mode_idx), &bf_mode_info->dots_img_info);
    EXCEPTION_CHECK_VOID(bf_mode_info->dots_img_info.data);
    lv_fs_path_ex_t dots_img_path;
    lv_fs_make_path_from_buffer(&dots_img_path, LV_FS_MEMFS_LETTER, bf_mode_info->dots_img_info.data,
        bf_mode_info->dots_img_info.size, "baj");
    lv_image_set_src(bf_mode_info->dots_img, (const void *)&dots_img_path);
}

static void item_anim_exec_x(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_obj_set_x(obj, (lv_coord_t)v);
    lv_obj_invalidate(bf_mode_info->root);
}

static void item_anim_exec_y(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_obj_set_y(obj, (lv_coord_t)v);
    lv_obj_invalidate(bf_mode_info->root);
}

static uint8_t slot_mode_for_center(uint8_t center_mode, bf_circle_slot_e slot)
{
    uint8_t mode = center_mode;
    if (slot == BF_SLOT_LEFT) {
        mode = center_mode > 0 ? (center_mode - 1) : (bf_mode_info->mode_num - 1);
    } else if (slot == BF_SLOT_RIGHT) {
        mode = center_mode < (bf_mode_info->mode_num - 1) ? (center_mode + 1) : 0;
    }
    return mode;
}

static void update_slot_content(ui_manager_context_t *thiz, bf_circle_slot_e slot, 
    uint8_t mode_idx, uint8_t last_mode_idx, bool new_step_info_flag)
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
            lv_fs_path_ex_t mode_component_path;
            lv_fs_make_path_from_buffer(&mode_component_path, LV_FS_MEMFS_LETTER,
                bf_mode_info->mode_component_info[slot].data, bf_mode_info->mode_component_info[slot].size, "baj");
            lv_image_set_src(img, (const void *)&mode_component_path);
            bf_mode_info->slot_res_id[slot] = res_id;
        }
    }
    LOG_UI("slot:%d, last_mode_idx:%d, mode_idx:%d, name:%s", slot, last_mode_idx, 
        mode_idx, bf_mode_name[mode_idx]);

    if (mode_idx == BF_MODE_COUNT - 1 && bf_mode_info->new_step_info_flag) {
        lv_obj_clear_flag(bf_mode_info->unconfirmed_icon[slot], LV_OBJ_FLAG_HIDDEN); /// 显示新模式未确认图标
        lv_obj_add_flag(bf_mode_info->last_usage_icon[slot], LV_OBJ_FLAG_HIDDEN); /// 隐藏上一次使用模式图标
    } else {
        lv_obj_add_flag(bf_mode_info->unconfirmed_icon[slot], LV_OBJ_FLAG_HIDDEN); /// 隐藏新模式未确认图标
        if (last_mode_idx == mode_idx) {
            lv_obj_clear_flag(bf_mode_info->last_usage_icon[slot], LV_OBJ_FLAG_HIDDEN); /// 显示上一次使用模式图标
        } else {
            lv_obj_add_flag(bf_mode_info->last_usage_icon[slot], LV_OBJ_FLAG_HIDDEN); /// 隐藏上一次使用模式图标
        }
    }

    if (mode_idx == BF_MODE_COUNT - 1) {
        char title_text[32] = {0};
        snprintf(title_text, sizeof(title_text), "Customize %s", bf_mode_info->custom_name);
        lv_label_set_text(title, title_text);
    } else {
        lv_label_set_text(title, bf_mode_name[mode_idx]);
    }
    lv_label_set_text(desc, bf_mode_desc[mode_idx]);

    lv_coord_t desc_y = BF_DESC_BASE_Y_OFFSET;
    uint8_t line_cnt = 1;
    const char *desc_text = bf_mode_desc[mode_idx];
    for (const char *p = desc_text; *p != '\0'; p++) {
        if (*p == '\n') {
            line_cnt++;
        }
    }

    if (line_cnt > 1) {
        const lv_font_t *font = lv_obj_get_style_text_font(desc, 0);
        lv_coord_t line_h = font != NULL ? lv_font_get_line_height(font) : 14;
        lv_coord_t line_space = lv_obj_get_style_text_line_space(desc, 0);
        lv_coord_t extra_h = (lv_coord_t)((line_h + line_space) * (line_cnt - 1));
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
}

static void circle_carousel_set_mode(ui_manager_context_t *thiz, uint8_t idx, bool anim)
{
    if (idx >= bf_mode_info->mode_num) {
        idx = bf_mode_info->mode_num - 1;
    }

    int8_t direction = 0;
    uint8_t from = bf_mode_info->cur_mode;
    uint8_t cw_steps = (uint8_t)((idx + bf_mode_info->mode_num - from) % bf_mode_info->mode_num);
    uint8_t ccw_steps = (uint8_t)((from + bf_mode_info->mode_num - idx) % bf_mode_info->mode_num);
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
     
    bool new_step_info_flag = (idx == BF_MODE_COUNT - 1) ? bf_mode_info->new_step_info_flag : false;
    EXCEPTION_CHECK_VOID(bf_mode_info->pump_ctx);
    int last_mode_idx = 0;
    READ_PUMP_EXPERT_MODE(bf_mode_info->pump_ctx, true, last_mode_idx);
    last_mode_idx = last_mode_idx - BREAST_PUMP_EXPERT_MODE_LOWER;
    update_slot_content(thiz, BF_SLOT_LEFT, slot_mode_for_center(idx, BF_SLOT_LEFT) , last_mode_idx, false);
    update_slot_content(thiz, BF_SLOT_CENTER, slot_mode_for_center(idx, BF_SLOT_CENTER) , last_mode_idx, new_step_info_flag);
    update_slot_content(thiz, BF_SLOT_RIGHT, slot_mode_for_center(idx, BF_SLOT_RIGHT) , last_mode_idx, false);
    circle_layout_apply(direction, anim);
    bf_mode_info->cur_mode = idx;
    update_dots(thiz, slot_mode_for_center(idx, BF_SLOT_CENTER));
}

static void create_mode_bg_by_lvgl_draw(ui_manager_context_t *thiz, lv_obj_t *parent, bf_circle_slot_e slot)
{
    if (bf_mode_info->circle_obj[slot] == NULL || !lv_obj_is_valid(bf_mode_info->circle_obj[slot])) {
        bf_mode_info->circle_obj[slot] = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(bf_mode_info->circle_obj[slot]);
    }
    lv_obj_remove_style_all(bf_mode_info->circle_obj[slot]);
    lv_obj_set_size(bf_mode_info->circle_obj[slot], BF_CIRCLE_W, BF_CIRCLE_H); /// 设置圆圈大小
    lv_obj_set_style_radius(bf_mode_info->circle_obj[slot], LV_RADIUS_CIRCLE, 0); /// 圆角
    lv_obj_set_style_bg_color(bf_mode_info->circle_obj[slot], 
        lv_color_hex(slot == BF_SLOT_CENTER ? BF_CENTER_CIRCLE_GRAD_TOP_COLOR : BF_SIDE_CIRCLE_GRAD_TOP_COLOR), 0);
    lv_obj_set_style_bg_grad_color(bf_mode_info->circle_obj[slot], 
        lv_color_hex(slot == BF_SLOT_CENTER ? BF_CENTER_CIRCLE_GRAD_BOTTOM_COLOR : BF_SIDE_CIRCLE_GRAD_BOTTOM_COLOR), 0);
    lv_obj_set_style_bg_grad_dir(bf_mode_info->circle_obj[slot], LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_main_stop(bf_mode_info->circle_obj[slot], 0, 0); /// 主颜色停止位置
    lv_obj_set_style_bg_grad_stop(bf_mode_info->circle_obj[slot], 255, 0); /// 渐变颜色停止位置
    lv_obj_set_style_bg_opa(bf_mode_info->circle_obj[slot], LV_OPA_COVER, 0);
}

static void create_mode_item(ui_manager_context_t *thiz, lv_obj_t *parent, bf_circle_slot_e slot)
{
    create_mode_bg_by_lvgl_draw(thiz, parent, slot);

    /// 模式组件图片对象
    if (bf_mode_info->mode_component_img[slot] == NULL || !lv_obj_is_valid(bf_mode_info->mode_component_img[slot])) {
        bf_mode_info->mode_component_img[slot] = lv_image_create(bf_mode_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(bf_mode_info->mode_component_img[slot]);
        lv_obj_align(bf_mode_info->mode_component_img[slot], LV_ALIGN_CENTER, 0, BF_COMPONENT_CENTER_Y_OFFSET);
    }

    /// 上一次使用模式图标资源  
    if (bf_mode_info->last_usage_icon_info.data == NULL) {
        common_ui_img_resource_init(thiz, BASE_RES_ID_LAST_USED_MODE_ICON, &bf_mode_info->last_usage_icon_info);
        EXCEPTION_CHECK_VOID(bf_mode_info->last_usage_icon_info.data);
    }

    /// 上一次使用模式图标对象
    if (bf_mode_info->last_usage_icon[slot] == NULL || !lv_obj_is_valid(bf_mode_info->last_usage_icon[slot])) {
        bf_mode_info->last_usage_icon[slot] = lv_image_create(bf_mode_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(bf_mode_info->last_usage_icon[slot]);
        lv_fs_path_ex_t last_usage_icon_path;
        lv_fs_make_path_from_buffer(&last_usage_icon_path, LV_FS_MEMFS_LETTER,
                                    bf_mode_info->last_usage_icon_info.data, bf_mode_info->last_usage_icon_info.size, "baj");
        lv_image_set_src(bf_mode_info->last_usage_icon[slot], (const void *)&last_usage_icon_path);
        lv_obj_align(bf_mode_info->last_usage_icon[slot], LV_ALIGN_CENTER, 22, BF_LAST_USAGE_ICON_Y_OFFSET);
        lv_obj_set_style_image_opa(bf_mode_info->last_usage_icon[slot], LV_OPA_COVER, 0);
        lv_obj_add_flag(bf_mode_info->last_usage_icon[slot], LV_OBJ_FLAG_HIDDEN);
    }

    /// 新模式未确认图标资源
    if (bf_mode_info->unconfirmed_icon_info.data == NULL) {
        common_ui_img_resource_init(thiz, BASE_RES_ID_NEW_MODE_STEPS_ICON, &bf_mode_info->unconfirmed_icon_info);
        EXCEPTION_CHECK_VOID(bf_mode_info->unconfirmed_icon_info.data);
    }
    
    /// 新模式未确认图标
    if (bf_mode_info->unconfirmed_icon[slot] == NULL || !lv_obj_is_valid(bf_mode_info->unconfirmed_icon[slot])) {
        bf_mode_info->unconfirmed_icon[slot] = lv_image_create(bf_mode_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(bf_mode_info->unconfirmed_icon[slot]);
        lv_fs_path_ex_t unconfirmed_icon_path;
        lv_fs_make_path_from_buffer(&unconfirmed_icon_path, LV_FS_MEMFS_LETTER,
                                    bf_mode_info->unconfirmed_icon_info.data, bf_mode_info->unconfirmed_icon_info.size, "baj");
        lv_image_set_src(bf_mode_info->unconfirmed_icon[slot], (const void *)&unconfirmed_icon_path);
        lv_obj_align(bf_mode_info->unconfirmed_icon[slot], LV_ALIGN_CENTER, 16, BF_UNCONFIRMED_ICON_Y_OFFSET);
        lv_obj_set_style_image_opa(bf_mode_info->unconfirmed_icon[slot], LV_OPA_COVER, 0);
        lv_obj_add_flag(bf_mode_info->unconfirmed_icon[slot], LV_OBJ_FLAG_HIDDEN);
    }

    /// 模式标题标签
    if (bf_mode_info->mode_title_label[slot] == NULL || !lv_obj_is_valid(bf_mode_info->mode_title_label[slot])) {
        bf_mode_info->mode_title_label[slot] = lv_label_create(bf_mode_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(bf_mode_info->mode_title_label[slot]);
        lv_obj_set_style_text_color(bf_mode_info->mode_title_label[slot], lv_color_hex(BF_TITLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(bf_mode_info->mode_title_label[slot], &lv_font_oppo_sans_16_medium, 0);
        lv_obj_set_style_text_align(bf_mode_info->mode_title_label[slot], LV_TEXT_ALIGN_CENTER, 0);  /// 居中对齐
        lv_obj_align(bf_mode_info->mode_title_label[slot], LV_ALIGN_CENTER, 0, BF_TITLE_BASE_Y_OFFSET);
        lv_obj_set_style_text_opa(bf_mode_info->mode_title_label[slot], LV_OPA_COVER, 0);
    }

    /// 模式描述标签
    if (bf_mode_info->mode_desc_label[slot] == NULL || !lv_obj_is_valid(bf_mode_info->mode_desc_label[slot])) {
        bf_mode_info->mode_desc_label[slot] = lv_label_create(bf_mode_info->circle_obj[slot]);
        EXCEPTION_CHECK_VOID(bf_mode_info->mode_desc_label[slot]);
        lv_obj_set_style_text_color(bf_mode_info->mode_desc_label[slot], 
            lv_color_hex(slot == BF_SLOT_CENTER ? BF_CENTER_DESC_TEXT_COLOR : BF_DESC_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(bf_mode_info->mode_desc_label[slot], &lv_font_oppo_sans_16_medium, 0);
        lv_obj_set_style_text_align(bf_mode_info->mode_desc_label[slot], LV_TEXT_ALIGN_CENTER, 0);  /// 居中对齐
        lv_obj_set_style_text_line_space(bf_mode_info->mode_desc_label[slot], 4, 0); /// 设置行间距（像素）
        lv_obj_set_style_text_opa(bf_mode_info->mode_desc_label[slot], LV_OPA_COVER, 0);
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
    if (bf_mode_info->dots_img == NULL || !lv_obj_is_valid(bf_mode_info->dots_img)) {
        bf_mode_info->dots_img = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(bf_mode_info->dots_img);
        lv_obj_set_style_image_opa(bf_mode_info->dots_img, LV_OPA_COVER, 0);
        lv_obj_align(bf_mode_info->dots_img, LV_ALIGN_BOTTOM_MID, 0, -18);
    }
    update_dots(thiz, mode_idx);
}

static void bf_mode_root_init(ui_manager_context_t *thiz)
{
    if (bf_mode_info->root == NULL || !lv_obj_is_valid(bf_mode_info->root)) {
        bf_mode_info->root = lv_obj_create(lv_screen_active());
        EXCEPTION_CHECK_VOID(bf_mode_info->root);
        lv_obj_set_size(bf_mode_info->root, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(bf_mode_info->root, lv_color_black(), 0);
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
    uint8_t target_idx = mode_idx - BREAST_PUMP_EXPERT_MODE_LOWER;
    if (target_idx >= bf_mode_info->mode_num || target_idx >= BF_MODE_COUNT) {
        target_idx = 0;
    }

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
    lv_obj_align(bf_mode_info->mode_type_text, LV_ALIGN_TOP_MID, 0, BF_PAGE_TITLE_Y_OFFSET); /// 顶部居中对齐，偏移34px
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void common_pump_bf_mode_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (bf_mode_info == NULL) {
        bf_mode_info = (bf_mode_display_info_t *)u_malloz(sizeof(bf_mode_display_info_t));
        EXCEPTION_CHECK_VOID(bf_mode_info);
    }
    bf_mode_info->pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    if (bf_mode_info->pump_ctx == NULL) {
        u_free(bf_mode_info);
        bf_mode_info = NULL;
        return;
    }
    bf_mode_root_init(thiz);
    common_ui_bg_init(thiz);
    bf_mode_info->force_update = true;
    bf_mode_info->mode_num = BF_MODE_COUNT - 1;
    bf_mode_info->popup_ui_index = ID_POPUP_UI_NONE;
    bf_mode_info->custom_mode_id = 0;
    memset(bf_mode_info->custom_name, 0, sizeof(bf_mode_info->custom_name));
    int activate_status = 0;
    READ_PUMP_MODE_ACTIVATE(bf_mode_info->pump_ctx, BREAST_PUMP_CUSTOM_MODE, activate_status);
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st && activate_status) {
        bf_mode_info->mode_num = BF_MODE_COUNT;    
        uint32_t custom_mode_id = 0;
        st->get_store_ram_val(STORE_ID_PUMP_MODE_NAME_INFO, BREAST_PUMP_CUSTOM_MODE,
            sizeof(bf_mode_info->custom_name), bf_mode_info->custom_name);
        LOG_UI("custom_name: %s", bf_mode_info->custom_name);

        st->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO_ID, BREAST_PUMP_CUSTOM_MODE, 0, &custom_mode_id);
        bf_mode_info->custom_mode_id = custom_mode_id;
        int new_step_info_flag = false; 
        st->get_store_ram_val(STORE_ID_MODE_NEW_STEP_INFO_FLAG, BREAST_PUMP_CUSTOM_MODE, 0, &new_step_info_flag);
        bf_mode_info->new_step_info_flag = new_step_info_flag > 0 ? true : false;
        LOG_UI("customization activated, id: %d, flag: %d", bf_mode_info->custom_mode_id, bf_mode_info->new_step_info_flag);
    } else {
        bf_mode_info->new_step_info_flag = false;
    }
}

void common_pump_bf_mode_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && bf_mode_info);
    common_ui_img_resource_deinit(thiz, &bf_mode_info->last_usage_icon_info);
    common_ui_img_resource_deinit(thiz, &bf_mode_info->unconfirmed_icon_info);
    common_ui_img_resource_deinit(thiz, &bf_mode_info->dots_img_info);
    for (uint8_t i = 0; i < BF_CIRCLE_SLOT_COUNT; i++) {
        common_ui_img_resource_deinit(thiz, &bf_mode_info->mode_component_info[i]);
    }
    common_ui_bg_deinit(thiz);
    ui_common_operation_tip_remove(thiz);
    LVGL_OBJ_DELETE(bf_mode_info->root);
    u_free(bf_mode_info);
    bf_mode_info = NULL;
}

int common_pump_bf_mode_ui_process(ui_manager_context_t *thiz, uint8_t mode_idx, uint16_t popup_ui_type)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(bf_mode_info && bf_mode_info->root);
    if (bf_mode_info->mode_num > BF_MODE_COUNT || bf_mode_info->mode_num == 0) {
        int activate_status = 0;
        READ_PUMP_MODE_ACTIVATE(bf_mode_info->pump_ctx, BREAST_PUMP_CUSTOM_MODE, activate_status);
        if (activate_status) {
            bf_mode_info->mode_num = BF_MODE_COUNT;
        } else {
            bf_mode_info->mode_num = BF_MODE_COUNT - 1;
        }
    }
    bf_mode_bg_update(thiz);
    bf_mode_ui_update(thiz, mode_idx);
    bf_mode_text_update(thiz);
    ui_common_operation_tip_update(thiz, popup_ui_type, bf_mode_info->root, &bf_mode_info->popup_ui_index);
    bf_mode_info->force_update = false;
    
    return 0;
}