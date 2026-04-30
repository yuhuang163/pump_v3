/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_charge.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define POWER_OFF_CHARGE_BG_W                     204
#define POWER_OFF_CHARGE_BG_H                     88
#define POWER_OFF_CHARGE_BG_X                     50
#define POWER_OFF_CHARGE_BG_Y                     20
#define CHARGE_TEXT_COUNT                         3

typedef struct {
    lv_obj_t * root;
    lv_obj_t * text[CHARGE_TEXT_COUNT];
    lv_obj_t * battery_bg;
    lv_obj_t * battery_fill;
    lv_obj_t * charge_icon; 
    res_img_info_t charge_icon_info;
    uint8_t force_update;
    uint8_t rsv[15];
} power_off_charge_display_info_t;

typedef struct {
    const char *str;
    uint16_t x_offset; 
} power_off_charge_text_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static power_off_charge_display_info_t *charge_info;
const static power_off_charge_text_info_t charge_text_info[CHARGE_TEXT_COUNT] = {
    {"0", 78},
    {"50", 167},
    {"100", 247},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void power_off_charge_ui_root_init(ui_manager_context_t *thiz)
{
    if (charge_info->root == NULL || !lv_obj_is_valid(charge_info->root)) {
        charge_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(charge_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(charge_info->root, lv_color_black(), 0);     /// 设置背景颜色为白色
    lv_obj_set_style_bg_opa(charge_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(charge_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(charge_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(charge_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(charge_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(charge_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

static void charge_anim_cb(void *obj, int32_t v)
{
    lv_obj_set_width((lv_obj_t *)obj, v);
    // LOG_PASS("w:%d\r\n", v);
}

void battery_charge_anim_start(lv_obj_t *battery_fill, uint8_t percent)
{
    if (percent > 100) {
        percent = 100;
    }
    
    int max_w = POWER_OFF_CHARGE_BG_W - 8;     /// padding 左右各 4
    int start_w = max_w * percent / 100;
    lv_obj_set_width(battery_fill, start_w);   /// 设置电池填充宽度
    lv_anim_del(battery_fill, charge_anim_cb); /// 删除动画

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, battery_fill);
    lv_anim_set_exec_cb(&a, charge_anim_cb);
    lv_anim_set_values(&a, start_w, max_w);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_repeat_count(&a, 0); /// 设置动画重复次数
    lv_anim_set_playback_time(&a, 0); /// 设置动画回放时间
    lv_anim_start(&a); /// 启动动画
}

static void power_off_charge_anim_create(ui_manager_context_t *thiz, uint8_t percent)
{
    if (charge_info->battery_bg == NULL || !lv_obj_is_valid(charge_info->battery_bg)) {
        charge_info->battery_bg = lv_obj_create(charge_info->root);
        EXCEPTION_CHECK_VOID(charge_info->battery_bg);
        lv_obj_set_size(charge_info->battery_bg, POWER_OFF_CHARGE_BG_W, POWER_OFF_CHARGE_BG_H); /// 设置电池外框尺寸
        lv_obj_set_style_radius(charge_info->battery_bg, 16, 0); /// 设置电池外框圆角
        lv_obj_set_style_bg_color(charge_info->battery_bg, lv_color_hex(0x2E2E2E), 0); /// 设置电池外框颜色
        lv_obj_set_style_bg_opa(charge_info->battery_bg, LV_OPA_COVER, 0); /// 设置电池外框不透明度
        lv_obj_set_style_border_width(charge_info->battery_bg, 1, 0); /// 设置电池外框边框宽度
        lv_obj_set_style_border_color(charge_info->battery_bg, lv_color_hex(0x4B4B4B), 0); /// 设置电池外框边框颜色
        lv_obj_set_style_pad_all(charge_info->battery_bg, 2, 0); /// 设置电池外框内边距
        lv_obj_clear_flag(charge_info->battery_bg, LV_OBJ_FLAG_SCROLLABLE); /// 禁止电池外框滚动
        lv_obj_align(charge_info->battery_bg, LV_ALIGN_TOP_MID, 0, 142); /// 设置电池外框居中

        // lv_obj_set_style_shadow_width(charge_info->battery_bg, 3, 0); /// 设置电池外框阴影宽度
        // lv_obj_set_style_shadow_color(charge_info->battery_bg, lv_color_hex(0x000000), 0); /// 设置电池外框阴影颜色
        // lv_obj_set_style_shadow_opa(charge_info->battery_bg, LV_OPA_30, 0); /// 设置电池外框阴影不透明度
    }

    if (charge_info->battery_fill == NULL || !lv_obj_is_valid(charge_info->battery_fill)) {
        charge_info->battery_fill = lv_obj_create(charge_info->battery_bg);
        EXCEPTION_CHECK_VOID(charge_info->battery_fill);
        lv_obj_set_size(charge_info->battery_fill, 0, LV_PCT(100)); /// 设置电池填充尺寸
        lv_obj_set_style_radius(charge_info->battery_fill, 16, 0); /// 设置电池填充圆角
        lv_obj_set_style_bg_color(charge_info->battery_fill, lv_color_hex(0x04B455), 0); /// 设置电池填充颜色
        lv_obj_set_style_border_width(charge_info->battery_fill, 0, 0); /// 设置电池填充边框宽度
        lv_obj_clear_flag(charge_info->battery_fill, LV_OBJ_FLAG_SCROLLABLE); /// 禁止电池填充滚动

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, charge_info->battery_fill); /// 设置动画变量为电池填充
        lv_anim_set_values(&a, 0, POWER_OFF_CHARGE_BG_W - 8); /// 减去 padding*2
        lv_anim_set_time(&a, 2000); /// 设置动画时间
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE); /// 设置动画重复次数
        // lv_anim_set_playback_time(&a, 300); /// 设置动画回放时间
        lv_anim_set_playback_time(&a, 0); /// 设置动画回放时间
        // lv_anim_set_repeat_count(&a, 0); /// 设置动画重复次数
        lv_anim_set_exec_cb(&a, charge_anim_cb); /// 设置动画执行回调函数
        lv_anim_start(&a); /// 启动动画
    }
}

static void power_off_charge_icon_update(ui_manager_context_t *thiz)
{
    if (charge_info->charge_icon == NULL || !lv_obj_is_valid(charge_info->charge_icon)) {
        charge_info->charge_icon = lv_image_create(charge_info->root);
        EXCEPTION_CHECK_VOID(charge_info->charge_icon);
        common_ui_img_resource_init(thiz, RESOURCE_ID_CHARGE_PAGE_ICON, &charge_info->charge_icon_info);
        EXCEPTION_CHECK_VOID(charge_info->charge_icon_info.data);
        lv_fs_path_ex_t charge_icon_img_path;
        lv_fs_make_path_from_buffer(&charge_icon_img_path, LV_FS_MEMFS_LETTER,
            charge_info->charge_icon_info.data, charge_info->charge_icon_info.size, "baj");
        lv_image_set_src(charge_info->charge_icon, charge_icon_img_path.path);
        lv_obj_center(charge_info->charge_icon); /// 设置充电图标居中
    }
}

static void power_off_charge_text_update(ui_manager_context_t *thiz)
{
    for (int i = 0; i < CHARGE_TEXT_COUNT; i++) {
        if (charge_info->text[i] == NULL || !lv_obj_is_valid(charge_info->text[i])) {
            charge_info->text[i] = lv_label_create(charge_info->root);
            EXCEPTION_CHECK_VOID(charge_info->text[i]);
            lv_label_set_text(charge_info->text[i], charge_text_info[i].str);
            lv_obj_set_style_text_color(charge_info->text[i], lv_color_hex(0x3C3C3C), 0);
            lv_obj_set_style_text_font(charge_info->text[i], &lv_font_oppo_sans_20_medium, 0);
            lv_obj_set_style_text_align(charge_info->text[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_align(charge_info->text[i], LV_ALIGN_TOP_LEFT, charge_text_info[i].x_offset, 108);
        }
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_power_off_charge_screen_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (charge_info == NULL) {
        charge_info = (power_off_charge_display_info_t *)u_malloz(sizeof(power_off_charge_display_info_t));
        EXCEPTION_CHECK_VOID(charge_info);
    }
    power_off_charge_ui_root_init(thiz);  
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    charge_info->force_update = true;
}

void ui_display_power_off_charge_screen_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && charge_info);
    common_ui_img_resource_deinit(thiz, &charge_info->charge_icon_info);
    common_ui_bg_deinit(thiz);
    LVGL_OBJ_DELETE(charge_info->root);
    u_free(charge_info);
    charge_info = NULL;
}

int ui_display_power_off_charge_screen_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && charge_info && charge_info->root);
    ui_proc_charge_t *proc = (ui_proc_charge_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    power_off_charge_anim_create(thiz, 100);
    power_off_charge_icon_update(thiz);
    power_off_charge_text_update(thiz);
    thiz->priv.fps_count++;
    charge_info->force_update = false; 
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> power off charge fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}