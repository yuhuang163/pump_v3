/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "airplane_mode_ui.h" // IWYU pragma: keep
#include "window_screen_airplane_mode.h" // IWYU pragma: keep
/*
* DEFINES
*****************************************************************************************
*/
// 新增：圆弧进度条配置
#define COUNTDOWN_ARC_SIZE            360    // 圆弧尺寸（匹配图片视觉）
#define COUNTDOWN_ARC_WIDTH           12      // 圆弧粗细
#define COUNTDOWN_ARC_COLOR           0xB25962 // 圆弧颜色（匹配吸奶器粉色）
#define COUNTDOWN_MAX_VALUE           5      // 倒计时最大值

/*
* TYPEDEFS
*****************************************************************************************
*/
static lv_obj_t * sub_root;
static lv_obj_t * countdown_num;
static lv_obj_t * countdown_arc;            // 新增：环形进度条对象
static lv_obj_t * backgroud;
static uint16_t countdown_value;
static uint8_t force_update;
static uint32_t start_time;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_AIRPLANE_MODE_COMP_BG
static PSRAM_DATA_LOC uint8_t complete_tip_bg_data[] = {
    #include "png2txt/airplay_exit_com_bg_360x360.txt"
};
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void airplane_mode_exit_sub_root_init(lv_obj_t *root)
{

    if (sub_root == NULL || 
        !lv_obj_is_valid(sub_root)) {
        sub_root = lv_obj_create(root);
        EXCEPTION_CHECK_VOID(sub_root);
        lv_obj_set_size(sub_root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%
        lv_obj_set_style_bg_color(sub_root, lv_color_black(), 0);     /// 黑色背景
        lv_obj_set_style_bg_opa(sub_root, LV_OPA_50, 0);           /// 背景透明度50%
        lv_obj_set_style_border_width(sub_root, 0, 0);                /// 无边框
        lv_obj_set_style_pad_all(sub_root, 0, 0);                     /// 无内边距
        lv_obj_align(sub_root, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(sub_root, LV_OBJ_FLAG_CLICKABLE);           /// 不拦截点击事件
        lv_obj_clear_flag(sub_root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
    }

}

// 环形进度条更新函数（参照 pump_level_arc_progress_update）
static void airplane_mode_exit_arc_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(sub_root);

    /* 1. 创建 / 校验圆弧对象 */
    if (countdown_arc == NULL ||
        !lv_obj_is_valid(countdown_arc)) {

        lv_obj_t *arc = lv_arc_create(sub_root);
        EXCEPTION_CHECK_VOID(arc);
        countdown_arc = arc;

        /* 基础配置：范围、背景角度、尺寸和位置 */
        lv_arc_set_range(arc, 0, COUNTDOWN_MAX_VALUE);      // 0 ~ 5
        lv_arc_set_bg_angles(arc, 0, 360);                  // 背景整圆

        lv_obj_set_size(arc, COUNTDOWN_ARC_SIZE, COUNTDOWN_ARC_SIZE);
        lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);

        /* 样式：隐藏背景弧和 knob，只显示指示弧 */
        lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);          // 背景弧透明
        lv_obj_set_style_opa(arc, LV_OPA_TRANSP, LV_PART_KNOB);              // knob 透明

        lv_obj_set_style_arc_color(arc, lv_color_hex(COUNTDOWN_ARC_COLOR),
                                   LV_PART_INDICATOR);                       // 指示弧颜色
        lv_obj_set_style_arc_width(arc, COUNTDOWN_ARC_WIDTH,
                                   LV_PART_INDICATOR);                       // 指示弧宽度
    }

    // progress: 0..COUNTDOWN_MAX_VALUE
    int16_t progress = (int16_t)COUNTDOWN_MAX_VALUE -
                       (int16_t)countdown_value;
    if (progress < 0)
      progress = 0;
    if (progress > (int16_t)COUNTDOWN_MAX_VALUE)
      progress = (int16_t)COUNTDOWN_MAX_VALUE;

    // 0~5 的值同步给 arc
    lv_arc_set_value(countdown_arc, progress);

    int16_t start, end;
    if (progress >= COUNTDOWN_MAX_VALUE) {
      // 倒计时=0 → 满圆
      start = 0;
      end = 359; // 用 359° 而不是 360°，避免 wrap
    } else {
      // 按比例从 270° 开始画
      start = 270;
      int16_t sweep =
          (int16_t)((360 * progress) / COUNTDOWN_MAX_VALUE); // 0..288
      end = start + sweep;
    }

    lv_arc_set_angles(countdown_arc, start, end);
}

#if IS_SUPPORT_AIRPLANE_MODE_COMP_BG
static void airplane_mode_exit_bg_update(ui_manager_context_t *thiz)
{
    if (backgroud == NULL || !lv_obj_is_valid(backgroud)) {
        backgroud = lv_image_create(sub_root);
        EXCEPTION_CHECK_VOID(backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, complete_tip_bg_data,
            sizeof(complete_tip_bg_data), "png");
        lv_image_set_src(backgroud, (const void *)&bg_img_path);
        lv_obj_align(backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#endif

static int airplane_mode_exit_countdown_num_update(ui_manager_context_t *thiz, lv_obj_t *root, uint8_t show_countdown_num)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(root);
    /* 1. 创建 / 校验子根对象 */
    if (show_countdown_num == 1) {
        airplane_mode_exit_sub_root_init(root);
    }

    if (get_timestamp() - start_time < 700) {
        return 0;
    }
    // 如果不需要显示倒计时数字，则直接返回
    if (show_countdown_num == 0) {
        if (sub_root) {
            lv_obj_add_flag(sub_root, LV_OBJ_FLAG_HIDDEN);
        }
        countdown_value = COUNTDOWN_MAX_VALUE;
        return 0;
    }
    lv_obj_clear_flag(sub_root, LV_OBJ_FLAG_HIDDEN);
    if (countdown_num == NULL ||
        !lv_obj_is_valid(countdown_num)) {
        countdown_num = lv_label_create(sub_root);
    }

    // 把数字转成字符串
    char num_buf[8];
    snprintf(num_buf, sizeof(num_buf), "%d", countdown_value);

    lv_label_set_text(countdown_num, num_buf);
    // 大号白色字体，居中
    lv_obj_set_style_text_color(countdown_num, lv_color_white(), 0);
    lv_obj_set_style_text_font(countdown_num, &lv_font_quicksand_96_bold, 0);
    lv_obj_set_style_text_align(countdown_num, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(countdown_num, LV_ALIGN_CENTER, 0, 0);

    // 新增：更新圆弧进度条
    airplane_mode_exit_arc_update(thiz);

    if (countdown_value > 0) {
        countdown_value--;
    }
    start_time = get_timestamp();
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_airplane_mode_exit_init(ui_manager_context_t *thiz, lv_obj_t *root)
{
    EXCEPTION_CHECK_VOID(thiz);
    force_update = true;
    countdown_value = 5;
    start_time = get_timestamp(); // 初始化倒计时起始时间
}

void ui_display_airplane_mode_exit_deinit(ui_manager_context_t *thiz, lv_obj_t *root)
{
    EXCEPTION_CHECK_VOID(thiz);
    countdown_num = NULL;
    countdown_arc = NULL;
    backgroud = NULL;
    sub_root = NULL;
    force_update = false;
    countdown_value = 0;
    start_time = 0;
}

void ui_display_airplane_mode_exit_process(ui_manager_context_t *thiz, lv_obj_t *root, uint8_t need_show_countdown_time)
{
    EXCEPTION_CHECK_VOID(thiz);

    //是否显示倒计时数字
    uint8_t show_countdown_num = need_show_countdown_time;
    // 数字更新时自动同步圆弧进度
    airplane_mode_exit_countdown_num_update(thiz, root, show_countdown_num);
    force_update = false;
}