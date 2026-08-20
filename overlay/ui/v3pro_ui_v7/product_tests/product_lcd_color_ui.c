/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "product_tests_ui.h" // IWYU pragma: keep
#include "window_screen_product_test.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define LCD_COLOR_GRAY_BAR_NUM                            (16)

typedef struct {
    lv_obj_t *root;
    lv_obj_t *gray_bars[LCD_COLOR_GRAY_BAR_NUM];
    lv_color_t s_color;
    uint8_t force_update;
    uint8_t last_color_id;
    uint8_t rsv[14];
} product_lcd_color_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static product_lcd_color_display_info_t *product_info = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void lcd_color_test_ui_init(void)
{
    if (product_info->root == NULL || !lv_obj_is_valid(product_info->root)) {
        product_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(product_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(product_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(product_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(product_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(product_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(product_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(product_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(product_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

static void lcd_color_test_hide_gray_bars(void)
{
    for (uint8_t i = 0; i < LCD_COLOR_GRAY_BAR_NUM; i++) {
        if (product_info->gray_bars[i] && lv_obj_is_valid(product_info->gray_bars[i])) {
            lv_obj_add_flag(product_info->gray_bars[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void lcd_color_test_show_gray_bars(void)
{
    int32_t w = lv_obj_get_width(product_info->root);
    int32_t h = lv_obj_get_height(product_info->root);
    if (w <= 0 || h <= 0) {
        return;
    }

    /// 参照 16 灰阶图：16 条等分水平条，上白(255) → 下黑(0)
    int32_t bar_h = h / LCD_COLOR_GRAY_BAR_NUM;
    for (uint8_t i = 0; i < LCD_COLOR_GRAY_BAR_NUM; i++) {
        if (product_info->gray_bars[i] == NULL || !lv_obj_is_valid(product_info->gray_bars[i])) {
            product_info->gray_bars[i] = lv_obj_create(product_info->root);
            lv_obj_set_style_border_width(product_info->gray_bars[i], 0, 0);
            lv_obj_set_style_pad_all(product_info->gray_bars[i], 0, 0);
            lv_obj_set_style_radius(product_info->gray_bars[i], 0, 0);
            lv_obj_set_style_bg_opa(product_info->gray_bars[i], LV_OPA_COVER, 0);
            lv_obj_clear_flag(product_info->gray_bars[i], LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
        }
        lv_obj_clear_flag(product_info->gray_bars[i], LV_OBJ_FLAG_HIDDEN);
        int32_t this_h = (i == LCD_COLOR_GRAY_BAR_NUM - 1) ? (h - bar_h * i) : bar_h;
        lv_obj_set_size(product_info->gray_bars[i], w, this_h);
        lv_obj_set_pos(product_info->gray_bars[i], 0, bar_h * i);
        uint8_t gray = (uint8_t)(255 - (255 * i) / (LCD_COLOR_GRAY_BAR_NUM - 1));
        lv_obj_set_style_bg_color(product_info->gray_bars[i], lv_color_make(gray, gray, gray), 0);
    }
}

static void lcd_color_test_update(ui_manager_context_t *thiz)
{
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (product_info->root == NULL || !lv_obj_is_valid(product_info->root)) {
        return;
    }
    lv_color_t color = lv_color_hex(proc->lcd_color);
    if (product_info->last_color_id == proc->lcd_color_id &&
        lv_color_eq(color, product_info->s_color) && !product_info->force_update) {
        return;
    }
    product_info->s_color = color;
    product_info->last_color_id = proc->lcd_color_id;
    LOG_UI("color_id: %d, red: %x, green: %x, blue: %x, color: %x",
        proc->lcd_color_id, color.red, color.green, color.blue, proc->lcd_color);
    if (proc->lcd_color_id == 0x06) {
        lcd_color_test_show_gray_bars();
    } else {
        lcd_color_test_hide_gray_bars();
        lv_obj_set_style_bg_color(product_info->root, color, 0);
    }
    product_info->force_update = false;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_lcd_color_test_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (product_info == NULL) {
        product_info = (product_lcd_color_display_info_t *)u_malloz(sizeof(product_lcd_color_display_info_t));
        EXCEPTION_CHECK_VOID(product_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    lcd_color_test_ui_init();
    product_info->force_update = true;
}

void ui_display_lcd_color_test_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(product_info);
    LVGL_OBJ_DELETE(product_info->root);
    product_info->root = NULL;
    u_free(product_info);
    product_info = NULL;
}

int ui_display_lcd_color_test_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    thiz->priv.fps_count++;
    product_info->force_update = true;
    lcd_color_test_update(thiz);
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> lcd_color fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
