/*
* INCLUDE FILES
*****************************************************************************************
*/
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
#define IS_SUPPORT_MODE_COMP_BG                  (0)
#define MODE_MAX_COUNT                           (3)
#define BREASTFEEDING_MODE_MAX_COUNT             (4)

#define PUMP_MODE_SEL_START_COLOR                0xB3686D
#define PUMP_MODE_SEL_END_COLOR                  0xF1D9D6
#define PUMP_MODE_UNSEL_COLOR                    0xF6F6F6
#define PUMP_MODE_TEXT_SEL_COLOR                 0xFFFFFF 
#define PUMP_MODE_TEXT_UNSEL_COLOR               0x000000   

#define STIMULATE_MODE_LABEL_X                   (69 + 4)
#define STIMULATE_MODE_LABEL_Y                   (90)

#define STIMULATE_MODE_ICON_X                    (109 + 4)
#define STIMULATE_MODE_ICON_Y                    (116 + 1)
#define STIMULATE_MODE_LABEL_TEXT_X              (143 + 3)
#define STIMULATE_MODE_LABEL_TEXT_Y              (110 + 4)

#define LACTATION_MODE_LABEL_X                   (69 + 4)
#define LACTATION_MODE_LABEL_Y                   (166)

#define LACTATION_MODE_ICON_X                    (110 + 4)
#define LACTATION_MODE_ICON_Y                    (189 + 1)
#define LACTATION_MODE_LABEL_TEXT_X              (144 + 3)
#define LACTATION_MODE_LABEL_TEXT_Y              (186 + 4)

#define MIX_MODE_LABEL_X                         (69 + 4)
#define MIX_MODE_LABEL_Y                         (242)

#define MIX_MODE_ICON_X                          (105 + 4)
#define MIX_MODE_ICON_Y                          (263 + 1)
#define MIX_MODE_LABEL_TEXT_X                    (176 + 3)
#define MIX_MODE_LABEL_TEXT_Y                    (263 + 4)

typedef struct {
    const char *name;
    uint16_t label_x;
    uint16_t label_y;
    uint16_t icon_x;
    uint16_t icon_y;
    uint16_t label_text_x;
    uint16_t label_text_y;
} mode_label_info_t;

typedef struct {
#if IS_SUPPORT_MODE_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * root;
    lv_obj_t * root_text;
    lv_obj_t * label[MODE_MAX_COUNT];
    lv_obj_t * label_texts[MODE_MAX_COUNT];
    lv_obj_t * label_icon[MODE_MAX_COUNT];
    res_img_info_t label_icon_infos[MODE_MAX_COUNT];
    uint8_t force_update;
    uint8_t mode;
} pump_mode_display_info_t;
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_MODE_COMP_BG
static PSRAM_DATA_LOC uint8_t mode_bg_data[] = {
    #include "comparison_png2txt/mode_comp_1_360x360.txt"
    // #include "comparison_png2txt/mode_comp_2_360x360.txt"
    // #include "comparison_png2txt/mode_comp_3_360x360.txt"
};
#endif

const static uint16_t mode_icon_selected[MODE_MAX_COUNT] = {
    RESOURCE_ID_MODE_STIMULATE_ICON_W,
    RESOURCE_ID_MODE_LACTATION_ICON_W,
    RESOURCE_ID_MODE_MIX_ICON_W,
};

const static uint16_t mode_icon_unselected[MODE_MAX_COUNT] = {
    RESOURCE_ID_MODE_STIMULATE_ICON_P,
    RESOURCE_ID_MODE_LACTATION_ICON_P,
    RESOURCE_ID_MODE_MIX_ICON_P,
};

const static mode_label_info_t mode_label_infos[MODE_MAX_COUNT] = {
    { "Stimulate", 
        STIMULATE_MODE_LABEL_X,      STIMULATE_MODE_LABEL_Y, 
        STIMULATE_MODE_ICON_X,       STIMULATE_MODE_ICON_Y, 
        STIMULATE_MODE_LABEL_TEXT_X, STIMULATE_MODE_LABEL_TEXT_Y
    },
    { "Lactation", 
        LACTATION_MODE_LABEL_X,      LACTATION_MODE_LABEL_Y,
        LACTATION_MODE_ICON_X,       LACTATION_MODE_ICON_Y,
        LACTATION_MODE_LABEL_TEXT_X, LACTATION_MODE_LABEL_TEXT_Y
    },
    { "Mix",       
        MIX_MODE_LABEL_X,            MIX_MODE_LABEL_Y,
        MIX_MODE_ICON_X,             MIX_MODE_ICON_Y, 
        MIX_MODE_LABEL_TEXT_X,       MIX_MODE_LABEL_TEXT_Y
    },
};

static pump_mode_display_info_t *mode_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_mode_root_init(void)
{
    if (mode_info->root == NULL || !lv_obj_is_valid(mode_info->root)) {
        mode_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(mode_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(mode_info->root, lv_color_white(), 0);     /// 设置背景颜色为白色
    lv_obj_set_style_bg_opa(mode_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(mode_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(mode_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(mode_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(mode_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(mode_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

#if IS_SUPPORT_MODE_COMP_BG
static void pump_home_bg_update(void)
{
    static uint8_t status = 0;
    if (mode_info->background == NULL || !lv_obj_is_valid(mode_info->root)) {
        mode_info->background = lv_image_create(mode_info->root);
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
#endif

static void pump_mode_root_text_update(ui_manager_context_t *thiz)
{
    if (mode_info->force_update == false) {
        return;
    }

    if (mode_info->root_text == NULL || !lv_obj_is_valid(mode_info->root_text)) {
        mode_info->root_text = lv_label_create(mode_info->root);
        EXCEPTION_CHECK_VOID(mode_info->root_text);
    }
    lv_label_set_text(mode_info->root_text, "Mode");  /// 设置 label 文字内容和样式
    lv_obj_set_style_text_color(mode_info->root_text, lv_color_hex(PUMP_MODE_TEXT_UNSEL_COLOR), 0);
    lv_obj_set_style_text_font(mode_info->root_text, &lv_font_roboto_24_medium, 0);  /// 设置字体
    lv_obj_align(mode_info->root_text, LV_ALIGN_TOP_LEFT, 150 + 3, 39 + 4); /// 设置文字位置
}

static int pump_mode_lable_update(ui_manager_context_t *thiz, uint8_t mode, uint8_t mode_index)
{
    if (mode_info->label[mode_index] == NULL || !lv_obj_is_valid(mode_info->label[mode_index])) {
        mode_info->label[mode_index] = lv_obj_create(mode_info->root);
        EXCEPTION_CHECK_INT(mode_info->label[mode_index]);
    }
    
    bool is_selected = (mode == (BREAST_PUMP_STIMULATION_MODE + mode_index)) ? true : false;

    lv_obj_clear_flag(mode_info->label[mode_index], LV_OBJ_FLAG_SCROLLABLE);    /// 不要滚动条
    lv_obj_set_size(mode_info->label[mode_index], 222, 68); /// 设置容器尺寸
    lv_obj_set_style_border_width(mode_info->label[mode_index], 0, 0);          /// 无边框
    if (is_selected) {
        lv_obj_set_style_bg_opa(mode_info->label[mode_index], LV_OPA_COVER, 0); /// 背景不透明 LV_OPA_COVER
        lv_obj_set_style_bg_color(mode_info->label[mode_index], 
            lv_color_hex(PUMP_MODE_SEL_START_COLOR), 0);      // start
        lv_obj_set_style_bg_grad_color(mode_info->label[mode_index],
            lv_color_hex(PUMP_MODE_SEL_END_COLOR), 0);  // end
        lv_obj_set_style_bg_grad_dir(mode_info->label[mode_index], LV_GRAD_DIR_HOR, 0); /// 横向渐变
    } else {
        lv_obj_set_style_bg_opa(mode_info->label[mode_index], LV_OPA_COVER, 0);    /// 背景不透明
        lv_obj_set_style_bg_color(mode_info->label[mode_index], 
            lv_color_hex(PUMP_MODE_UNSEL_COLOR), 0);
        lv_obj_set_style_bg_grad_color(mode_info->label[mode_index],
            lv_color_hex(PUMP_MODE_UNSEL_COLOR), 0); /// 渐变颜色
        lv_obj_set_style_bg_grad_dir(mode_info->label[mode_index], LV_GRAD_DIR_NONE, 0); /// 关闭渐变
    }
    lv_obj_set_style_radius(mode_info->label[mode_index], 34, 0);   /// 高度 68 → 圆角设 34（或更大）
    lv_obj_align(mode_info->label[mode_index], LV_ALIGN_TOP_LEFT,
         mode_label_infos[mode_index].label_x, mode_label_infos[mode_index].label_y);    /// 设置label位置

    if (mode_info->label_texts[mode_index] == NULL ||
         !lv_obj_is_valid(mode_info->label_texts[mode_index])) {
        mode_info->label_texts[mode_index] = lv_label_create(mode_info->root);
        EXCEPTION_CHECK_INT(mode_info->label_texts[mode_index]);
    }
    lv_label_set_text(mode_info->label_texts[mode_index], mode_label_infos[mode_index].name);  /// 设置 label 文字内容和样式
    lv_obj_set_style_text_color(mode_info->label_texts[mode_index], 
        lv_color_hex(is_selected ? PUMP_MODE_TEXT_SEL_COLOR : PUMP_MODE_TEXT_UNSEL_COLOR), 0);
    lv_obj_set_style_text_font(mode_info->label_texts[mode_index], &lv_font_roboto_25_medium, 0);  /// 设置字体
    lv_obj_align(mode_info->label_texts[mode_index], LV_ALIGN_TOP_LEFT, 
        mode_label_infos[mode_index].label_text_x, mode_label_infos[mode_index].label_text_y);    /// 设置label文字位置

    if (mode_info->label_icon[mode_index] == NULL ||
         !lv_obj_is_valid(mode_info->label_icon[mode_index])) {
        mode_info->label_icon[mode_index] = lv_image_create(mode_info->root);
        EXCEPTION_CHECK_INT(mode_info->label_icon[mode_index]);
    }

    lv_fs_path_ex_t mode_icon_img_path;
    if (is_selected) {
        common_ui_img_resource_init(thiz, mode_icon_selected[mode_index],
            &mode_info->label_icon_infos[mode_index]);
        EXCEPTION_CHECK_INT(mode_info->label_icon_infos[mode_index].data);
        lv_fs_make_path_from_buffer(&mode_icon_img_path, LV_FS_MEMFS_LETTER,
            mode_info->label_icon_infos[mode_index].data,
            mode_info->label_icon_infos[mode_index].size, "baj");
    } else {
        common_ui_img_resource_init(thiz, mode_icon_unselected[mode_index],
            &mode_info->label_icon_infos[mode_index]);
        EXCEPTION_CHECK_INT(mode_info->label_icon_infos[mode_index].data);
        lv_fs_make_path_from_buffer(&mode_icon_img_path, LV_FS_MEMFS_LETTER, 
            mode_info->label_icon_infos[mode_index].data,
            mode_info->label_icon_infos[mode_index].size, "baj");
    }
    lv_image_set_src(mode_info->label_icon[mode_index], (const void *)&mode_icon_img_path);
    lv_obj_align(mode_info->label_icon[mode_index], LV_ALIGN_TOP_LEFT,
        mode_label_infos[mode_index].icon_x, mode_label_infos[mode_index].icon_y);    /// 设置icon位置
    return 0;
}

static void pump_mode_ui_update(ui_manager_context_t *thiz, ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    uint8_t mode = proc->ui_normal_mode_index; /// 当前UI模式索引.
    if (mode_info->mode == mode && mode_info->force_update == false) {
        return;
    }

    for (uint8_t i = 0; i < MODE_MAX_COUNT; i++) {
        if (pump_mode_lable_update(thiz, mode, i) != 0) {
            return;
        }
    }
    mode_info->mode = mode;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_mode_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (mode_info == NULL) {
        mode_info = (pump_mode_display_info_t *)u_malloz(sizeof(pump_mode_display_info_t));
        EXCEPTION_CHECK_VOID(mode_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    pump_mode_root_init();
    mode_info->force_update = true;
}

void ui_display_pump_mode_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (mode_info == NULL) {
        return;
    }
    for (uint8_t i = 0; i < MODE_MAX_COUNT; i++) {
        common_ui_img_resource_deinit(thiz, &mode_info->label_icon_infos[i]);
    }
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
#if IS_SUPPORT_MODE_COMP_BG
    pump_home_bg_update();
#endif
    pump_mode_root_text_update(thiz);
    pump_mode_ui_update(thiz, proc);
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump mode fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
