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
#define IS_SUPPORT_SPEED_COMP_BG                        (0)
#define SPEED_MAX_COUNT                                 3

#define PUMP_SPEED_TEXT_SEL_COLOR                       0xFFFFFF
#define PUMP_SPEED_TEXT_UNSEL_COLOR                     0x000000
#define PUMP_SPEED_SEL_START_COLOR                      0xB3686D
#define PUMP_SPEED_SEL_END_COLOR                        0xF1D9D6
#define PUMP_SPEED_UNSEL_COLOR                          0xF6F6F6

#define COMFORT_LABEL_X                                 (69 + 4)
#define COMFORT_LABEL_Y                                 (90)
#define COMFORT_LABEL_TEXT_X                            (133 + 3)
#define COMFORT_LABEL_TEXT_Y                            (110 + 4)

#define REGULAR_LABEL_X                                 (69 + 4)
#define REGULAR_LABEL_Y                                 (166)
#define REGULAR_LABEL_TEXT_X                            (137 + 3)
#define REGULAR_LABEL_TEXT_Y                            (186 + 4)

#define BOOST_LABEL_X                                   (69 + 4)
#define BOOST_LABEL_Y                                   (242)
#define BOOST_LABEL_TEXT_X                              (147 + 3)
#define BOOST_LABEL_TEXT_Y                              (263 + 4)

typedef struct ui_display_speed_info {
    lv_obj_t * root;
#if IS_SUPPORT_SPEED_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * root_text;
    lv_obj_t * label[SPEED_MAX_COUNT];
    lv_obj_t * label_texts[SPEED_MAX_COUNT];
    uint8_t force_update;
    uint8_t speed;
} speed_display_info_t;

typedef struct {
    const char *name;
    uint16_t label_x;
    uint16_t label_y;
    uint16_t label_text_x;
    uint16_t label_text_y;
} speed_label_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static const speed_label_info_t speed_label_infos[SPEED_MAX_COUNT] = {
    {"Comfort", 
     COMFORT_LABEL_X, COMFORT_LABEL_Y, 
     COMFORT_LABEL_TEXT_X, COMFORT_LABEL_TEXT_Y
    },
    {"Regular", 
     REGULAR_LABEL_X, REGULAR_LABEL_Y, 
     REGULAR_LABEL_TEXT_X, REGULAR_LABEL_TEXT_Y
    },
    {"Boost", 
     BOOST_LABEL_X, BOOST_LABEL_Y, 
     BOOST_LABEL_TEXT_X, BOOST_LABEL_TEXT_Y
    },
};

static speed_display_info_t *speed_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_speed_root_init(void)
{
    if (speed_info->root == NULL || !lv_obj_is_valid(speed_info->root)) {
        speed_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(speed_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(speed_info->root, lv_color_white(), 0);     /// 设置背景颜色为白色
    lv_obj_set_style_bg_opa(speed_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(speed_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(speed_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(speed_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(speed_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(speed_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

static void pump_speed_root_text_update(ui_manager_context_t *thiz)
{
    if (speed_info->force_update == false) {
        return;
    }

    if (speed_info->root_text == NULL || !lv_obj_is_valid(speed_info->root_text)) {
        speed_info->root_text = lv_label_create(speed_info->root);
        EXCEPTION_CHECK_VOID(speed_info->root_text);
    }
    lv_label_set_text(speed_info->root_text, "Frequency");  /// 设置 label 文字内容和样式
    lv_obj_set_style_text_color(speed_info->root_text, lv_color_hex(PUMP_SPEED_TEXT_UNSEL_COLOR), 0);
    lv_obj_set_style_text_font(speed_info->root_text, &lv_font_roboto_24_medium, 0);  /// 设置字体
    lv_obj_align(speed_info->root_text, LV_ALIGN_TOP_LEFT, 125 + 3, 39 + 4); /// 设置文字位置
}

#if IS_SUPPORT_SPEED_COMP_BG
static void pump_home_bg_update(void)
{
    static uint8_t status = 0;
    if (speed_info->background == NULL || !lv_obj_is_valid(speed_info->root)) {
        speed_info->background = lv_image_create(speed_info->root);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(speed_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(speed_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, speed_bg_data,
            sizeof(speed_bg_data), "png");
        lv_image_set_src(speed_info->background, (const void *)&bg_img_path);
        lv_obj_align(speed_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#endif

static int pump_speed_lable_update(ui_manager_context_t *thiz, uint8_t speed, uint8_t speed_index)
{
    if (speed_info->label[speed_index] == NULL || !lv_obj_is_valid(speed_info->label[speed_index])) {
        speed_info->label[speed_index] = lv_obj_create(speed_info->root);
        EXCEPTION_CHECK_INT(speed_info->label[speed_index]);
    }

    bool is_selected = (speed == (BREAST_PUMP_SPEED_LOW + speed_index)) ? true : false;
    lv_obj_clear_flag(speed_info->label[speed_index], LV_OBJ_FLAG_SCROLLABLE);    /// 不要滚动条
    lv_obj_set_size(speed_info->label[speed_index], 222, 68); /// 设置容器尺寸
    lv_obj_set_style_border_width(speed_info->label[speed_index], 0, 0);          /// 无边框
    if (is_selected) {
        lv_obj_set_style_bg_opa(speed_info->label[speed_index], LV_OPA_COVER, 0); /// 背景不透明 LV_OPA_COVER
        lv_obj_set_style_bg_color(speed_info->label[speed_index], 
            lv_color_hex(PUMP_SPEED_SEL_START_COLOR), 0);      // start
        lv_obj_set_style_bg_grad_color(speed_info->label[speed_index],
            lv_color_hex(PUMP_SPEED_SEL_END_COLOR), 0);  // end
        lv_obj_set_style_bg_grad_dir(speed_info->label[speed_index], LV_GRAD_DIR_HOR, 0); /// 横向渐变
    } else {
        lv_obj_set_style_bg_opa(speed_info->label[speed_index], LV_OPA_COVER, 0);    /// 背景不透明
        lv_obj_set_style_bg_color(speed_info->label[speed_index], 
            lv_color_hex(PUMP_SPEED_UNSEL_COLOR), 0);
        lv_obj_set_style_bg_grad_color(speed_info->label[speed_index],
            lv_color_hex(PUMP_SPEED_UNSEL_COLOR), 0); /// 渐变颜色
        lv_obj_set_style_bg_grad_dir(speed_info->label[speed_index], LV_GRAD_DIR_NONE, 0); /// 关闭渐变
    }
    lv_obj_set_style_radius(speed_info->label[speed_index], 34, 0);   /// 高度 68 → 圆角设 34（或更大）
    lv_obj_align(speed_info->label[speed_index], LV_ALIGN_TOP_LEFT,
         speed_label_infos[speed_index].label_x, speed_label_infos[speed_index].label_y);    /// 设置label位置
    if (speed_info->label_texts[speed_index] == NULL ||
         !lv_obj_is_valid(speed_info->label_texts[speed_index])) {
        speed_info->label_texts[speed_index] = lv_label_create(speed_info->root);
        EXCEPTION_CHECK_INT(speed_info->label_texts[speed_index]);
    }
    lv_label_set_text(speed_info->label_texts[speed_index], speed_label_infos[speed_index].name);  /// 设置 label 文字内容和样式
    lv_obj_set_style_text_color(speed_info->label_texts[speed_index], 
        lv_color_hex(is_selected ? PUMP_SPEED_TEXT_SEL_COLOR : PUMP_SPEED_TEXT_UNSEL_COLOR), 0);
    lv_obj_set_style_text_font(speed_info->label_texts[speed_index], &lv_font_roboto_25_medium, 0);  /// 设置字体
    lv_obj_align(speed_info->label_texts[speed_index], LV_ALIGN_TOP_LEFT, 
        speed_label_infos[speed_index].label_text_x, speed_label_infos[speed_index].label_text_y);    /// 设置label文字位置
    
    return 0;
}

static void pump_speed_ui_update(ui_manager_context_t *thiz, ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    EXCEPTION_CHECK_VOID(proc->motor);
    pump_setting_t setting;
    uint8_t time_index = proc->motor->get_runtime_policy_index(MOTOR_TIME_POLICY);
    proc->pump->get_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index);
    if (speed_info->speed == setting.speed && speed_info->force_update == false) {
        return;
    }

    for (uint8_t i = 0; i < SPEED_MAX_COUNT; i++) {
        if (pump_speed_lable_update(thiz, setting.speed, i) != 0) {
            return;
        }
    }
    speed_info->speed = setting.speed;
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
    pump_speed_root_init();
    speed_info->force_update = true;
}

void ui_display_pump_speed_remove(ui_manager_context_t *thiz)
{
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
#if IS_SUPPORT_SPEED_COMP_BG
    pump_home_bg_update();
#endif
    pump_speed_root_text_update(thiz);
    pump_speed_ui_update(thiz, proc);
    thiz->priv.fps_count++;
    speed_info->force_update = false;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump speed fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}