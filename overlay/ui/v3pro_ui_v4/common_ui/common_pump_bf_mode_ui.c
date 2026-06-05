/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_MODE_COMP_BG                     (0)
#define MODE_NUM                                    (3)
#define LABEL_NUM                                   (3)   /// 仅保留左/中/右 3 个圆槽位
#define LABEL_GROUP_NUM                             (2)

#define PAGE_TITLE_TEXT_COLOR                       (0xB25962)
#define PAGE_TITLE_Y_OFFSET                         (34)

#define UP_LABEL_BG_COLOR                           (0xFFEBE8)
#define UP_LABEL_TITLE_TEXT_COLOR                   (0xB25962)
#define MID_LABEL_GRAD_TOP_COLOR                    (0xF27D85)
#define MID_LABEL_GRAD_BOTTOM_COLOR                 (0xE25D66)
#define MID_LABEL_TITLE_TEXT_COLOR                  (0xFFFFFF)
#define MID_LABEL_DESC_TEXT_COLOR                   (0xFFE6E7)

#define MODE_SWITCH_ANIM_TIME_MS                    (400)

#define MODE_MID_LABEL_MIN_OPA_MIN_EXIT             (0)
#define MODE_UP_LABEL_MIN_OPA_MIN_EXIT              (0)
#define MODE_DOWN_LABEL_MIN_OPA_MIN_EXIT            (0)

#define MODE_MID_LABEL_MIN_OPA_ENTER                (180) 
#define MODE_UP_LABEL_MIN_OPA_ENTER                 (0)
#define MODE_DOWN_LABEL_MIN_OPA_ENTER               (0)

#define MODE_MID_LABEL_HIDDEN_OPA                   (MODE_MID_LABEL_MIN_OPA_MIN_EXIT + 70) 

#define MODE_SLOT_Y_UP_EXIT                         (34)   /// 上胶囊出场，滚出顶部
#define MODE_SLOT_Y_UP                              (88)
#define MODE_SLOT_Y_UP_ENTER                        (320)  /// 入场上胶囊从中间槽位升起
#define MODE_SLOT_Y_MID_EXIT                        (50)   /// 中胶囊出场，滚出顶部
#define MODE_SLOT_Y_MID                             (150)
#define MODE_SLOT_Y_MID_ENTER                       (350)  /// 入场中胶囊从下方进入
#define MODE_SLOT_Y_DOWN_EXIT                       (160)  /// 下胶囊出场，滚出顶部
#define MODE_SLOT_Y_DOWN                            (262)
#define MODE_SLOT_Y_DOWN_ENTER                      (400)  /// 入场下胶囊从更下方进入

typedef struct label_info {
    lv_obj_t *obj[LABEL_NUM]; /// 标签对象
    lv_obj_t *title_text[LABEL_NUM]; /// 模式标题标签
    lv_obj_t *icon_img;  /// 模式组件图片
    lv_obj_t *desc_text; /// 模式描述标签
    lv_obj_t *status_img; /// 状态图标
    res_img_info_t icon_img_info;
    uint8_t idx[LABEL_NUM];
} label_info_t;

typedef struct ui_display_mode_info {
    lv_obj_t * root;
#if IS_SUPPORT_MODE_COMP_BG
    lv_obj_t * background;
#endif
    label_info_t label_info[LABEL_GROUP_NUM];
    res_img_info_t used_icon_info;
    res_img_info_t new_icon_info;
    lv_obj_t *page_title_text;
    lv_obj_t *slider_img; /// 滑动条图片
    res_img_info_t slider_img_info;
    char custom_name[24]; /// 个性化模式名称
    breast_pump_manager_context_t *pump;
    storage_manager_context_t *st;
    uint32_t custom_mode_id; /// 个性化模式ID
    uint8_t mode_idx;
    uint8_t force_update;
    uint8_t label_obj_init_done;
    uint8_t active_label_group;
    uint8_t popup_ui_index;
    uint8_t mode_num;
    int8_t last_used_mode_idx; /// 上一次使用模式索引
    int8_t unconfirmed_mode_idx; /// 新模式状态
    uint8_t rsv[8];
} mode_display_info_t;

typedef struct mode_label_anim_param {
    int16_t x_ofs;
    int16_t y;
    lv_opa_t opa;
} mode_label_anim_param_t;

typedef enum {
    MODE_SLOT_UP = 0,
    MODE_SLOT_MID,
    MODE_SLOT_DOWN,
} mode_label_slot_e;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static mode_display_info_t *mode_info;

static const mode_label_anim_param_t mode_label_visible_param[LABEL_NUM] = {
    [MODE_SLOT_UP]   = {0, MODE_SLOT_Y_UP, LV_OPA_COVER},
    [MODE_SLOT_MID]  = {0, MODE_SLOT_Y_MID, LV_OPA_COVER},
    [MODE_SLOT_DOWN] = {0, MODE_SLOT_Y_DOWN, LV_OPA_COVER},
};

/// [group: 0=out, 1=in][slot][from/to]
/// 出场/入场各槽位 Y 独立，间距随动画变化（上/下胶囊 50px，中胶囊约 112px）
static const mode_label_anim_param_t mode_switch_anim_param[2][LABEL_NUM][2] = {
    {
        {{0, MODE_SLOT_Y_UP, LV_OPA_COVER},   {0, MODE_SLOT_Y_UP_EXIT, MODE_UP_LABEL_MIN_OPA_MIN_EXIT}},
        {{0, MODE_SLOT_Y_MID, LV_OPA_COVER},  {0, MODE_SLOT_Y_MID_EXIT, MODE_MID_LABEL_MIN_OPA_MIN_EXIT}},
        {{0, MODE_SLOT_Y_DOWN, LV_OPA_COVER},  {0, MODE_SLOT_Y_DOWN_EXIT, MODE_DOWN_LABEL_MIN_OPA_MIN_EXIT}},
    },
    {
        {{0, MODE_SLOT_Y_UP_ENTER, MODE_UP_LABEL_MIN_OPA_ENTER},   {0, MODE_SLOT_Y_UP, LV_OPA_COVER}},
        {{0, MODE_SLOT_Y_MID_ENTER, MODE_MID_LABEL_MIN_OPA_ENTER},  {0, MODE_SLOT_Y_MID, LV_OPA_COVER}},
        {{0, MODE_SLOT_Y_DOWN_ENTER, MODE_DOWN_LABEL_MIN_OPA_ENTER}, {0, MODE_SLOT_Y_DOWN, LV_OPA_COVER}},
    },
};

static const uint16_t mode_icon_ids[MODE_NUM] = {
    RESOURCE_ID_MODE_EXPERT_ICON,  
    RESOURCE_ID_MODE_EXPERT_ICON,  
    RESOURCE_ID_MODE_CUSTOM_ICON, 
};

#ifdef UI_SNAP_SHOT_VERTICAL_SLIDER_1
static const ui_snap_shot_id_e slider_snap_ids[MODE_NUM] = {
    UI_SNAP_SHOT_VERTICAL_SLIDER_1,
    UI_SNAP_SHOT_VERTICAL_SLIDER_2,
    UI_SNAP_SHOT_VERTICAL_SLIDER_3,
};
#else
static const uint16_t slider_res_ids[MODE_NUM] = {
    RESOURCE_ID_VERTICAL_SLIDER_1,
    RESOURCE_ID_VERTICAL_SLIDER_2,
    RESOURCE_ID_VERTICAL_SLIDER_3,
};
#endif

static const char *mode_name[MODE_NUM] = {
    "Milk Boost", /// 2+8奶量提升模式
    "Power Pumping", /// PP追奶模式
    "Customize", /// 已使用过的个性化模式
};

static const char *mode_desc[MODE_NUM] = {
    "Most suitable for daily use\nafter successful lactation",
    "Increase milk supply, no\nmore than once a day.",
    "Edit and sync anytime via the\nMomcozy App",
};

#if IS_SUPPORT_MODE_COMP_BG
static PSRAM_DATA_LOC uint8_t mode_bg_data[] = {
    #include "comparison_png2txt/stimulation_mode_info_360x360.txt"
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
static void mode_root_init(ui_manager_context_t *thiz)
{
    if (mode_info->root == NULL || !lv_obj_is_valid(mode_info->root)) {
        mode_info->root = lv_obj_create(lv_screen_active());
        EXCEPTION_CHECK_VOID(mode_info->root);
        lv_obj_set_size(mode_info->root, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(mode_info->root, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(mode_info->root, LV_OPA_COVER, 0);
        lv_obj_set_style_clip_corner(mode_info->root, false, 0); /// 关闭圆角裁剪
        lv_obj_set_style_border_width(mode_info->root, 0, 0);
        lv_obj_set_style_pad_all(mode_info->root, 0, 0);
        lv_obj_clear_flag(mode_info->root, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(mode_info->root, LV_OBJ_FLAG_CLICKABLE);
    }
}

#if IS_SUPPORT_MODE_COMP_BG
static void ui_background_init(ui_manager_context_t *thiz)
{

}

static void ui_background_deinit(ui_manager_context_t *thiz)
{

}
static void ui_background_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (mode_info->background == NULL || !lv_obj_is_valid(mode_info->background)) {
        mode_info->background = lv_image_create(mode_info->root);
        EXCEPTION_CHECK_VOID(mode_info->background);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(mode_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(mode_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, mode_bg_data,
            sizeof(mode_bg_data), "png");
        lv_image_set_src(mode_info->background, (const void *)&bg_img_path);
        lv_obj_align(mode_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void ui_background_init(ui_manager_context_t *thiz)
{
    common_ui_bg_init(thiz);
}

static void ui_background_deinit(ui_manager_context_t *thiz)
{
    common_ui_bg_deinit(thiz);
}

static void ui_background_update(ui_manager_context_t *thiz)
{
    common_ui_bg_update(thiz, mode_info->root);
}
#endif

static void item_anim_exec_x(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_obj_set_x(obj, (lv_coord_t)v);
    lv_obj_invalidate(mode_info->root);
}

static void item_anim_exec_y(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_obj_set_y(obj, (lv_coord_t)v);
    lv_obj_invalidate(mode_info->root);
}

static void item_anim_exec_opa(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    if (v < MODE_MID_LABEL_HIDDEN_OPA) {
        if (obj == mode_info->label_info[0].obj[MODE_SLOT_MID] ||
            obj == mode_info->label_info[1].obj[MODE_SLOT_MID]) {
            // LOG_UI("obj: %p, v: %d", obj, v);
            v = LV_OPA_TRANSP;
        }
    } 
    lv_obj_set_style_opa(obj, (lv_opa_t)v, 0);
    lv_obj_invalidate(mode_info->root);
}

static void label_group_set_hidden(label_info_t *label_info, bool hidden)
{
    for (uint8_t i = 0; i < LABEL_NUM; i++) {
        if (label_info->obj[i] == NULL || !lv_obj_is_valid(label_info->obj[i])) {
            continue;
        }

        if (hidden) {
            lv_obj_add_flag(label_info->obj[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(label_info->obj[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void label_group_hide_inactive_completed(lv_anim_t *a)
{
    (void)a;

    if (mode_info == NULL) {
        return;
    }

    for (uint8_t i = 0; i < LABEL_GROUP_NUM; i++) {
        if (i == mode_info->active_label_group) {
            continue;
        }

        label_group_set_hidden(&mode_info->label_info[i], true);
    }
}

static void label_item_apply_param(lv_obj_t *item, const mode_label_anim_param_t *param)
{
    lv_obj_align(item, LV_ALIGN_TOP_MID, param->x_ofs, param->y);
    lv_obj_update_layout(item);
    lv_obj_set_style_opa(item, param->opa, 0);
}

static void label_item_start_anim(lv_obj_t *item,
    const mode_label_anim_param_t *from, const mode_label_anim_param_t *to, bool completed_cb)
{
    label_item_apply_param(item, from);
    int32_t from_x = from->x_ofs;
    int32_t from_y = from->y;
    lv_opa_t from_opa = from->opa;
    int32_t to_x = to->x_ofs;
    int32_t to_y = to->y;
    lv_opa_t to_opa = to->opa;

    lv_obj_align(item, LV_ALIGN_TOP_MID, (lv_coord_t)from_x, (lv_coord_t)from_y);
    lv_obj_set_style_opa(item, from_opa, 0);
    lv_anim_del(item, item_anim_exec_x);
    lv_anim_del(item, item_anim_exec_y);
    lv_anim_del(item, item_anim_exec_opa);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, item);
    lv_anim_set_time(&anim, MODE_SWITCH_ANIM_TIME_MS);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);

    lv_anim_set_exec_cb(&anim, item_anim_exec_x);
    lv_anim_set_values(&anim, from_x, to_x);
    lv_anim_start(&anim);

    lv_anim_set_exec_cb(&anim, item_anim_exec_y);
    lv_anim_set_values(&anim, from_y, to_y);
    lv_anim_start(&anim);

    lv_anim_set_exec_cb(&anim, item_anim_exec_opa);
    lv_anim_set_values(&anim, from_opa, to_opa);
    lv_anim_set_completed_cb(&anim, completed_cb ? label_group_hide_inactive_completed : NULL);
    lv_anim_start(&anim);
}

static void label_group_apply_visible_param(label_info_t *label_info, bool visible)
{
    for (uint8_t i = 0; i < LABEL_NUM; i++) {
        lv_obj_t *item = label_info->obj[i];
        if (item == NULL || !lv_obj_is_valid(item)) {
            continue;
        }

        label_item_apply_param(item, &mode_label_visible_param[i]);
        lv_obj_set_style_opa(item, visible ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    }
    label_group_set_hidden(label_info, !visible);
}

static void label_group_start_switch_anim(label_info_t *out_group, label_info_t *in_group)
{
    label_group_set_hidden(in_group, false);
    label_group_set_hidden(out_group, false);

    for (uint8_t i = 0; i < LABEL_NUM; i++) {
        if (out_group->obj[i] != NULL && lv_obj_is_valid(out_group->obj[i])) {
            label_item_start_anim(out_group->obj[i],
                &mode_switch_anim_param[0][i][0],
                &mode_switch_anim_param[0][i][1],
                i == MODE_SLOT_DOWN);
        }

        if (in_group->obj[i] != NULL && lv_obj_is_valid(in_group->obj[i])) {
            label_item_start_anim(in_group->obj[i],
                &mode_switch_anim_param[1][i][0],
                &mode_switch_anim_param[1][i][1],
                false);
        }
    }
}

static void create_up_and_down_label_item(ui_manager_context_t *thiz, label_info_t *label_info, uint8_t slot)
{
    if (label_info->obj[slot] == NULL || !lv_obj_is_valid(label_info->obj[slot])) {
        label_info->obj[slot] = lv_obj_create(mode_info->root);
        EXCEPTION_CHECK_VOID(label_info->obj[slot]);
    }

    lv_obj_remove_style_all(label_info->obj[slot]);
    lv_obj_set_size(label_info->obj[slot], 200, 50); /// 设置圆圈大小
    lv_obj_set_style_radius(label_info->obj[slot], LV_RADIUS_CIRCLE, 0); /// 圆角
    lv_obj_set_style_bg_color(label_info->obj[slot], lv_color_hex(UP_LABEL_BG_COLOR), 0); /// 背景颜色
    lv_obj_set_style_bg_grad_color(label_info->obj[slot], lv_color_hex(UP_LABEL_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(label_info->obj[slot], LV_OPA_COVER, 0);
    lv_obj_align(label_info->obj[slot], LV_ALIGN_TOP_MID, 0, slot == MODE_SLOT_UP ? 88 : 262); /// 居中对齐
    
    if (label_info->title_text[slot] == NULL || !lv_obj_is_valid(label_info->title_text[slot])) {
        label_info->title_text[slot] = lv_label_create(label_info->obj[slot]);
        EXCEPTION_CHECK_VOID(label_info->title_text[slot]);
        lv_obj_set_style_text_color(label_info->title_text[slot], lv_color_hex(UP_LABEL_TITLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(label_info->title_text[slot], &lv_font_oppo_sans_20_medium, 0);
        lv_obj_set_style_text_align(label_info->title_text[slot], LV_TEXT_ALIGN_CENTER, 0);  /// 居中对齐
        lv_obj_align(label_info->title_text[slot], LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_opa(label_info->title_text[slot], LV_OPA_COVER, 0);
    }
}

static void create_mid_label_item(ui_manager_context_t *thiz, label_info_t *label_info)
{
    lv_draw_buf_t *background_snapshot = NULL;

    #if LV_USE_SNAPSHOT
    background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_SELECT_LABEL_ICON);
    #endif

    if (label_info->obj[MODE_SLOT_MID] == NULL || !lv_obj_is_valid(label_info->obj[MODE_SLOT_MID])) {
        label_info->obj[MODE_SLOT_MID] = lv_image_create(mode_info->root);
        EXCEPTION_CHECK_VOID(label_info->obj[MODE_SLOT_MID]);
    }

    if (background_snapshot != NULL) {
        lv_image_set_src(label_info->obj[MODE_SLOT_MID], background_snapshot);
    } else {
        lv_obj_add_flag(label_info->obj[MODE_SLOT_MID], LV_OBJ_FLAG_HIDDEN); /// 隐藏
    }
    lv_obj_align(label_info->obj[MODE_SLOT_MID], LV_ALIGN_TOP_MID, 0, 150); /// 居中对齐

    if (label_info->icon_img == NULL || !lv_obj_is_valid(label_info->icon_img)) {
        label_info->icon_img = lv_image_create(label_info->obj[MODE_SLOT_MID]);
        EXCEPTION_CHECK_VOID(label_info->icon_img);
        lv_obj_align(label_info->icon_img, LV_ALIGN_LEFT_MID, 24, 0);
    }

    if (label_info->title_text[MODE_SLOT_MID] == NULL || !lv_obj_is_valid(label_info->title_text[MODE_SLOT_MID])) {
        label_info->title_text[MODE_SLOT_MID] = lv_label_create(label_info->obj[MODE_SLOT_MID]);
        EXCEPTION_CHECK_VOID(label_info->title_text[MODE_SLOT_MID]);
        lv_obj_set_style_text_color(label_info->title_text[MODE_SLOT_MID], lv_color_hex(MID_LABEL_TITLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(label_info->title_text[MODE_SLOT_MID], &lv_font_oppo_sans_20_medium, 0);
        lv_obj_set_style_text_align(label_info->title_text[MODE_SLOT_MID], LV_TEXT_ALIGN_CENTER, 0);  /// 居中对齐
        lv_obj_set_style_text_opa(label_info->title_text[MODE_SLOT_MID], LV_OPA_COVER, 0);
        lv_obj_align(label_info->title_text[MODE_SLOT_MID], LV_ALIGN_TOP_LEFT, 80, 16);
    }

    if (label_info->desc_text == NULL || !lv_obj_is_valid(label_info->desc_text)) {
        label_info->desc_text = lv_label_create(label_info->obj[MODE_SLOT_MID]);
        EXCEPTION_CHECK_VOID(label_info->desc_text);
        lv_obj_set_style_text_color(label_info->desc_text, lv_color_hex(MID_LABEL_DESC_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(label_info->desc_text, &lv_font_oppo_sans_16_medium, 0);
        lv_obj_set_style_text_align(label_info->desc_text, LV_TEXT_ALIGN_LEFT, 0);  /// 居中对齐
        lv_obj_set_style_text_line_space(label_info->desc_text, 4, 0); /// 设置行间距（像素）
        lv_obj_set_style_text_opa(label_info->desc_text, LV_OPA_COVER, 0);
        lv_obj_align(label_info->desc_text, LV_ALIGN_TOP_LEFT, 80, 44);
    }

    if (label_info->status_img == NULL || !lv_obj_is_valid(label_info->status_img)) {
        label_info->status_img = lv_image_create(label_info->obj[MODE_SLOT_MID]);
        EXCEPTION_CHECK_VOID(label_info->status_img);
        lv_obj_align(label_info->status_img, LV_ALIGN_TOP_LEFT, 54, 28);
        lv_obj_set_style_image_opa(label_info->status_img, LV_OPA_COVER, 0);
    }
}

static const char *get_mode_display_name(uint8_t idx)
{
    uint8_t custom_idx = BREAST_PUMP_CUSTOM_MODE - BREAST_PUMP_EXPERT_MODE_LOWER;

    if (idx == custom_idx && mode_info->custom_name[0] != '\0') {
        return mode_info->custom_name;
    }
    return mode_name[idx];
}

static void invalidate_label_cache(void)
{
    for (uint8_t i = 0; i < LABEL_GROUP_NUM; i++) {
        for (uint8_t j = 0; j < LABEL_NUM; j++) {
            mode_info->label_info[i].idx[j] = BREAST_PUMP_MODE_INVALID;
        }
    }
}

static void update_up_and_down_label_content(ui_manager_context_t *thiz, label_info_t *label_info, 
    mode_label_slot_e slot, uint8_t idx)
{
    if (label_info->obj[slot] == NULL || label_info->title_text[slot] == NULL) {
        return;
    }

    if (label_info->idx[slot] == idx) {
        return;
    }

    lv_label_set_text(label_info->title_text[slot], get_mode_display_name(idx));
    label_info->idx[slot] = idx;
}

static void update_mid_label_content(ui_manager_context_t *thiz, label_info_t *label_info, uint8_t idx)
{
    if (label_info->obj[MODE_SLOT_MID] == NULL || label_info->title_text[MODE_SLOT_MID] == NULL ||
        label_info->icon_img == NULL || label_info->desc_text == NULL) {
        return;
    }

    if (label_info->idx[MODE_SLOT_MID] == idx) {
        LOG_UI("idx: %d, same as before", idx);
        return;
    }

    uint16_t res_id = mode_icon_ids[idx];
    common_ui_img_resource_init(thiz, res_id, &label_info->icon_img_info);
    if (label_info->icon_img_info.data != NULL) {
        lv_fs_path_ex_t mode_component_path;
        lv_fs_make_path_from_buffer(&mode_component_path, LV_FS_MEMFS_LETTER,
            label_info->icon_img_info.data, label_info->icon_img_info.size, "baj");
        lv_image_set_src(label_info->icon_img, (const void *)&mode_component_path);
    }
    lv_label_set_text(label_info->title_text[MODE_SLOT_MID], get_mode_display_name(idx));
    lv_label_set_text(label_info->desc_text, mode_desc[idx]);
    lv_fs_path_ex_t status_img_path;
    if (mode_info->unconfirmed_mode_idx == idx) {
        if (mode_info->new_icon_info.data != NULL) {
            lv_obj_clear_flag(label_info->status_img, LV_OBJ_FLAG_HIDDEN);
            lv_fs_make_path_from_buffer(&status_img_path, LV_FS_MEMFS_LETTER,
                mode_info->new_icon_info.data, mode_info->new_icon_info.size, "baj");
            lv_image_set_src(label_info->status_img, (const void *)&status_img_path);
            lv_obj_align(label_info->status_img, LV_ALIGN_TOP_LEFT, 54, 28);
        } else {
            lv_obj_add_flag(label_info->status_img, LV_OBJ_FLAG_HIDDEN); /// 隐藏
        }
    } else if (mode_info->last_used_mode_idx == idx ) {
        if (mode_info->used_icon_info.data != NULL) {
            lv_obj_clear_flag(label_info->status_img, LV_OBJ_FLAG_HIDDEN);
            lv_fs_make_path_from_buffer(&status_img_path, LV_FS_MEMFS_LETTER,
                mode_info->used_icon_info.data, mode_info->used_icon_info.size, "baj");
            lv_image_set_src(label_info->status_img, (const void *)&status_img_path);
            lv_obj_align(label_info->status_img, LV_ALIGN_TOP_LEFT, 52, 24);
        } else {
            lv_obj_add_flag(label_info->status_img, LV_OBJ_FLAG_HIDDEN); /// 隐藏
        }
    } else {
        lv_obj_add_flag(label_info->status_img, LV_OBJ_FLAG_HIDDEN); /// 隐藏
    }
    label_info->idx[MODE_SLOT_MID] = idx;
    // LOG_UI("idx: %d, used:%d, unconfirmed:%d", idx, mode_info->last_used_mode_idx, mode_info->unconfirmed_mode_idx);
}

static void circle_carousel_set_mode(ui_manager_context_t *thiz, label_info_t *label_info, uint8_t idx, bool anim)
{
    (void)anim;

    uint8_t up_idx = 0;
    uint8_t down_idx = 0;
    uint8_t mid_idx = 0;
    if (idx == 0) {
        mid_idx  = idx;
        up_idx   = mode_info->mode_num - 1;
        down_idx = 1;
    } else if (idx >= mode_info->mode_num - 1) {
        mid_idx  = mode_info->mode_num - 1;
        up_idx   = mid_idx - 1;
        down_idx = 0;
    } else {
        mid_idx  = idx;
        up_idx   = mid_idx - 1;
        down_idx = mid_idx + 1;
    }
    update_mid_label_content(thiz, label_info, mid_idx);
    update_up_and_down_label_content(thiz, label_info, MODE_SLOT_UP, up_idx);
    update_up_and_down_label_content(thiz, label_info, MODE_SLOT_DOWN, down_idx);
    // LOG_UI("idx: %d, mid_idx: %d, up_idx: %d, down_idx: %d", idx, mid_idx, up_idx, down_idx);
}

static void circle_carousel_create(ui_manager_context_t *thiz, uint8_t idx)
{ 
    /// 创建上胶囊，层级小于中层胶囊
    for (uint8_t i = 0; i < LABEL_GROUP_NUM; i++) {
        mode_info->label_info[i].idx[MODE_SLOT_UP] = BREAST_PUMP_MODE_INVALID;
        create_up_and_down_label_item(thiz, &mode_info->label_info[i], MODE_SLOT_UP); 
        EXCEPTION_CHECK_VOID(mode_info->label_info[i].obj[MODE_SLOT_UP]);
        EXCEPTION_CHECK_VOID(mode_info->label_info[i].title_text[MODE_SLOT_UP]);
    }
    
    /// 创建中间胶囊，层级最高
    for (uint8_t i = 0; i < LABEL_GROUP_NUM; i++) {
        mode_info->label_info[i].idx[MODE_SLOT_MID] = BREAST_PUMP_MODE_INVALID;
        create_mid_label_item(thiz, &mode_info->label_info[i]); 
        EXCEPTION_CHECK_VOID(mode_info->label_info[i].obj[MODE_SLOT_MID]);
        EXCEPTION_CHECK_VOID(mode_info->label_info[i].title_text[MODE_SLOT_MID]);
        EXCEPTION_CHECK_VOID(mode_info->label_info[i].icon_img);
        EXCEPTION_CHECK_VOID(mode_info->label_info[i].desc_text);
    }

    /// 创建下胶囊，层级小于上层胶囊
    for (uint8_t i = 0; i < LABEL_GROUP_NUM; i++) {
        mode_info->label_info[i].idx[MODE_SLOT_DOWN] = BREAST_PUMP_MODE_INVALID;
        create_up_and_down_label_item(thiz, &mode_info->label_info[i], MODE_SLOT_DOWN); 
        EXCEPTION_CHECK_VOID(mode_info->label_info[i].obj[MODE_SLOT_DOWN]);
        EXCEPTION_CHECK_VOID(mode_info->label_info[i].title_text[MODE_SLOT_DOWN]);
    }
}

static void update_slider(ui_manager_context_t *thiz, uint8_t idx)
{
#ifdef UI_SNAP_SHOT_VERTICAL_SLIDER_1
    lv_draw_buf_t *background_snapshot = NULL;
    #if LV_USE_SNAPSHOT
    background_snapshot = get_lvgl_snap_shot(slider_snap_ids[idx]);
    #endif
    if (background_snapshot != NULL) {
        lv_image_set_src(mode_info->slider_img, background_snapshot);
    } else {
        lv_obj_add_flag(mode_info->slider_img, LV_OBJ_FLAG_HIDDEN); /// 隐藏
    }
#else
    common_ui_img_resource_init(thiz, slider_res_ids[idx], &mode_info->slider_img_info);
    if (mode_info->slider_img_info.data != NULL) {
        lv_fs_path_ex_t slider_path;
        lv_fs_make_path_from_buffer(&slider_path, LV_FS_MEMFS_LETTER, mode_info->slider_img_info.data,
            mode_info->slider_img_info.size, "baj");
        lv_image_set_src(mode_info->slider_img, (const void *)&slider_path);
    }
#endif
}

static void circle_carousel_update(ui_manager_context_t *thiz, uint8_t idx)
{
    if (mode_info->slider_img == NULL || !lv_obj_is_valid(mode_info->slider_img)) {
        mode_info->slider_img = lv_image_create(mode_info->root);
        EXCEPTION_CHECK_VOID(mode_info->slider_img);
        lv_obj_set_style_image_opa(mode_info->slider_img, LV_OPA_COVER, 0);
        lv_obj_align(mode_info->slider_img, LV_ALIGN_RIGHT_MID, -8, 0);
    }
    update_slider(thiz, idx);
}

static void label_ui_update(ui_manager_context_t *thiz, uint8_t mode)
{
    uint8_t idx = mode - BREAST_PUMP_EXPERT_MODE_LOWER;
    if (idx >= mode_info->mode_num) {
        idx = 0;
    }

    if (mode_info->label_obj_init_done == false) {
        circle_carousel_create(thiz, idx);
        circle_carousel_set_mode(thiz, &mode_info->label_info[0], idx, false);
        label_group_apply_visible_param(&mode_info->label_info[0], true);
        label_group_apply_visible_param(&mode_info->label_info[1], false);
        circle_carousel_update(thiz, idx);
        mode_info->mode_idx = idx;
        mode_info->active_label_group = 0;
        mode_info->label_obj_init_done = true;
        return;
    }

    if (mode_info->force_update || idx != mode_info->mode_idx) {
        uint8_t old_group = mode_info->active_label_group;
        uint8_t new_group = (uint8_t)(old_group ^ 1);
        circle_carousel_set_mode(thiz, &mode_info->label_info[new_group], idx, false);
        if (mode_info->force_update) {
            label_group_apply_visible_param(&mode_info->label_info[old_group], false);
            label_group_apply_visible_param(&mode_info->label_info[new_group], true);
        } else {
            label_group_start_switch_anim(&mode_info->label_info[old_group],
                &mode_info->label_info[new_group]);
        }
        circle_carousel_update(thiz, idx);
        mode_info->active_label_group = new_group;
        mode_info->mode_idx = idx;
    }
}

static void ui_page_title_text_update(ui_manager_context_t *thiz)
{
    if (mode_info->force_update == false) {
        return;
    }

    if (mode_info->page_title_text == NULL || !lv_obj_is_valid(mode_info->page_title_text)) {
        mode_info->page_title_text = lv_label_create(mode_info->root);
        EXCEPTION_CHECK_VOID(mode_info->page_title_text);
        lv_label_set_text(mode_info->page_title_text, "Program");
        lv_obj_set_style_text_color(mode_info->page_title_text, lv_color_hex(PAGE_TITLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(mode_info->page_title_text, &lv_font_oppo_sans_24_semibold, 0);
        lv_obj_set_style_text_align(mode_info->page_title_text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_set_style_text_opa(mode_info->page_title_text, LV_OPA_COVER, 0);
        lv_obj_align(mode_info->page_title_text, LV_ALIGN_TOP_MID, 0, PAGE_TITLE_Y_OFFSET); /// 顶部居中对齐，偏移32px
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void common_pump_bf_mode_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (mode_info == NULL) {
        mode_info = (mode_display_info_t *)u_malloz(sizeof(mode_display_info_t));
        EXCEPTION_CHECK_VOID(mode_info);
    }
    mode_root_init(thiz);
    ui_background_init(thiz);
    mode_info->force_update = true;
    mode_info->label_obj_init_done = false;
    mode_info->active_label_group = 0;
    mode_info->popup_ui_index = ID_POPUP_UI_NONE;
    memset(mode_info->custom_name, 0, sizeof(mode_info->custom_name));
    mode_info->last_used_mode_idx = -1;
    mode_info->unconfirmed_mode_idx = -1;
    mode_info->mode_num = MODE_NUM - 1;
    mode_info->custom_mode_id = 0;
    mode_info->pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    mode_info->st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    common_ui_img_resource_init(thiz, RESOURCE_ID_LAST_USED_MODE_ICON, &mode_info->used_icon_info);
    common_ui_img_resource_init(thiz, RESOURCE_ID_NEW_MODE_STEPS_ICON, &mode_info->new_icon_info);
    int last_mode_idx = 0;
    if (mode_info->pump) {
        READ_PUMP_EXPERT_MODE(mode_info->pump, true, last_mode_idx);
        mode_info->last_used_mode_idx = last_mode_idx - BREAST_PUMP_EXPERT_MODE_LOWER;
    }
}

void common_pump_bf_mode_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && mode_info);
    for (uint8_t i = 0; i < LABEL_GROUP_NUM; i++) {
        common_ui_img_resource_deinit(thiz, &mode_info->label_info[i].icon_img_info);
    }
    common_ui_img_resource_deinit(thiz, &mode_info->used_icon_info);
    common_ui_img_resource_deinit(thiz, &mode_info->new_icon_info);
    common_ui_img_resource_deinit(thiz, &mode_info->slider_img_info);
    ui_background_deinit(thiz);
    ui_common_operation_tip_remove(thiz);
    LVGL_OBJ_DELETE(mode_info->root);
    u_free(mode_info);
    mode_info = NULL;
}

int common_pump_bf_mode_ui_process(ui_manager_context_t *thiz, uint8_t mode, uint16_t popup_ui_type)
{
    EXCEPTION_CHECK_INT(thiz && mode_info && mode_info->root);
    int new_step_info_flag = 0, activate_status = 0;
   
    if (mode_info->pump) {
        READ_PUMP_MODE_ACTIVATE(mode_info->pump, BREAST_PUMP_CUSTOM_MODE, activate_status);
    }

    if (activate_status && mode_info->st) {
        uint32_t custom_mode_id = 0;
        mode_info->st->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO_ID, BREAST_PUMP_CUSTOM_MODE,
            0, &custom_mode_id);
        mode_info->st->get_store_ram_val(STORE_ID_MODE_NEW_STEP_INFO_FLAG, BREAST_PUMP_CUSTOM_MODE,
            0, &new_step_info_flag);
        if (new_step_info_flag) {
            mode_info->unconfirmed_mode_idx = BREAST_PUMP_CUSTOM_MODE - BREAST_PUMP_EXPERT_MODE_LOWER;
            if (mode_info->unconfirmed_mode_idx == mode_info->last_used_mode_idx) {
                mode_info->last_used_mode_idx = -1;
            }   mode_info->force_update = true;
        }

        if (custom_mode_id != mode_info->custom_mode_id) {
            mode_info->custom_mode_id = custom_mode_id;
            mode_info->st->get_store_ram_val(STORE_ID_PUMP_MODE_NAME_INFO, BREAST_PUMP_CUSTOM_MODE,
                sizeof(mode_info->custom_name), mode_info->custom_name);
            invalidate_label_cache();
            mode_info->force_update = true;
        }
        mode_info->mode_num = MODE_NUM;
    } else {
        mode_info->custom_mode_id = 0;
        mode_info->mode_num = MODE_NUM - 1;
        mode_info->unconfirmed_mode_idx = -1;
        memset(mode_info->custom_name, 0, sizeof(mode_info->custom_name));
    }
    ui_background_update(thiz);
    label_ui_update(thiz, mode);
    ui_page_title_text_update(thiz);
    ui_common_operation_tip_update(thiz, popup_ui_type, mode_info->root, &mode_info->popup_ui_index);
    mode_info->force_update = false;
    static uint16_t log_time = 0;
    log_time++;
    if (log_time % 10 == 0) {
        LOG_UI("used:%d, unconfirmed:%d, mode:%d, activate:%d, last_mode_idx:%d, flag:%d, name:%s", 
            mode_info->last_used_mode_idx, mode_info->unconfirmed_mode_idx, mode,
            activate_status, mode_info->last_used_mode_idx, new_step_info_flag, mode_info->custom_name);
    }
    return 0;
}
