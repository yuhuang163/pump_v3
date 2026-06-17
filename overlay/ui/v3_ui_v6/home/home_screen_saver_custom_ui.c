/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_ROOT_HOME_SCREEN_SAVER_COMP_BG                (0)

#define HOME_SCREEN_SAVER_ICON_MAX_COUNT                         (1)
#define HOME_SCREEN_SAVER_ICON_INDEX                             (0)

#define HOME_SCREEN_SAVER_ICON_X                                 (0)
#define HOME_SCREEN_SAVER_ICON_Y                                 (108 + 2)
#define IDLE_TEXT_COLOR                                          (0xB25962)
#define HOME_CLOCK_HOUR_X                                        (44 + 2)
#define HOME_CLOCK_MINUTE_X                                      (192 + 2)
#define HOME_CLOCK_TIME_Y                                        (141 - 26)
#define HOME_CLOCK_COLON_X                                       (174 )
#define HOME_CLOCK_COLON_Y                                       (141 - 14)

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_ROOT_HOME_SCREEN_SAVER_COMP_BG
    lv_obj_t * backgroud;
#endif
    lv_obj_t * saver_icon;
    res_img_info_t saver_img_info;
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
    lv_obj_t * clock_hour;
    lv_obj_t * clock_minute;
    lv_obj_t * clock_colon;
    res_img_info_t clock_colon_info;
    uint8_t home_screen_saver_display_type;
    uint8_t hour;
    uint8_t minute;
    uint8_t force_update;
    uint8_t rsv[13];
#else
    uint8_t force_update;
    uint8_t rsv[15];
#endif
} home_screen_saver_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static home_screen_saver_display_info_t *saver_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_ROOT_HOME_SCREEN_SAVER_COMP_BG
static PSRAM_DATA_LOC uint8_t home_screen_saver_bg_data[] = {
    #include "comparison_png2txt/home_idle_360x360.txt"
};
#endif
/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_screen_saver_root_init(void)
{
    if (saver_info->root == NULL || !lv_obj_is_valid(saver_info->root)) {
        saver_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(saver_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(saver_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(saver_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(saver_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(saver_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(saver_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(saver_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(saver_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

#if IS_SUPPORT_ROOT_HOME_SCREEN_SAVER_COMP_BG
static void home_screen_saver_root_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (saver_info->backgroud == NULL || !lv_obj_is_valid(saver_info->backgroud)) {
        saver_info->backgroud = lv_image_create(saver_info->root);
        EXCEPTION_CHECK_VOID(saver_info->backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(saver_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(saver_info->backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, home_screen_saver_bg_data,
            sizeof(home_screen_saver_bg_data), "png");
        lv_image_set_src(saver_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(saver_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void home_screen_saver_root_bg_update(ui_manager_context_t *thiz)
{
    common_ui_bg_update(thiz, saver_info->root);
}
#endif

static void home_screen_saver_dummy_icon_update(ui_manager_context_t *thiz, int16_t y_offset)
{
    if (saver_info->force_update == false) {
        return;
    }
    
    if (saver_info->saver_icon == NULL || !lv_obj_is_valid(saver_info->saver_icon)) {
        saver_info->saver_icon = lv_image_create(saver_info->root);
        EXCEPTION_CHECK_VOID(saver_info->saver_icon);
    }
    
    common_ui_theme_img_resource_init(thiz, RESOURCE_ID_HOME_IDLE_IP, RES_THEME_TYPE_CUSTOM,
        &saver_info->saver_img_info);
    EXCEPTION_CHECK_VOID(saver_info->saver_img_info.data);
    lv_fs_path_ex_t saver_icon_img_path;
    lv_fs_make_path_from_buffer(&saver_icon_img_path, LV_FS_MEMFS_LETTER, 
        saver_info->saver_img_info.data, saver_info->saver_img_info.size, "baj");
    lv_image_set_src(saver_info->saver_icon, (const void *)&saver_icon_img_path);
    lv_obj_align(saver_info->saver_icon, LV_ALIGN_TOP_MID, 
        HOME_SCREEN_SAVER_ICON_X, HOME_SCREEN_SAVER_ICON_Y + y_offset); /// 设置icon位置
}

#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
static void home_screen_saver_clock_ui_update(ui_manager_context_t *thiz, bool is_show)
{
    /// 更新时钟小时
    if (saver_info->clock_hour == NULL || !lv_obj_is_valid(saver_info->clock_hour)) {
        saver_info->clock_hour = lv_label_create(saver_info->root);
        EXCEPTION_CHECK_VOID(saver_info->clock_hour);
        lv_obj_set_width(saver_info->clock_hour, 120);
        lv_label_set_long_mode(saver_info->clock_hour, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_align(saver_info->clock_hour, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_style_text_color(saver_info->clock_hour, lv_color_hex(IDLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(saver_info->clock_hour, &lv_font_quicksand_96_bold, 0);
        lv_obj_add_flag(saver_info->clock_hour, LV_OBJ_FLAG_HIDDEN);
    }
    
    /// 更新时钟分钟
    if (saver_info->clock_minute == NULL || !lv_obj_is_valid(saver_info->clock_minute)) {
        saver_info->clock_minute = lv_label_create(saver_info->root);
        EXCEPTION_CHECK_VOID(saver_info->clock_minute);
        lv_obj_set_width(saver_info->clock_minute, 120); 
        lv_label_set_long_mode(saver_info->clock_minute, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_align(saver_info->clock_minute, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_style_text_color(saver_info->clock_minute, lv_color_hex(IDLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(saver_info->clock_minute, &lv_font_quicksand_96_bold, 0);
        lv_obj_add_flag(saver_info->clock_minute, LV_OBJ_FLAG_HIDDEN);
    }

    /// 更新时钟冒号
    if (saver_info->clock_colon == NULL || !lv_obj_is_valid(saver_info->clock_colon)) {
        saver_info->clock_colon = lv_image_create(saver_info->root);
        EXCEPTION_CHECK_VOID(saver_info->clock_colon);
        common_ui_theme_img_resource_init(thiz, RESOURCE_ID_CLOCK_COLON_ICON, RES_THEME_TYPE_CUSTOM,
            &saver_info->clock_colon_info);
        EXCEPTION_CHECK_VOID(saver_info->clock_colon_info.data);
        lv_fs_path_ex_t clock_colon_img_path;
        lv_fs_make_path_from_buffer(&clock_colon_img_path, LV_FS_MEMFS_LETTER, 
            saver_info->clock_colon_info.data, saver_info->clock_colon_info.size, "baj");
        LOG_UI("clock_colon_img_path: %s", clock_colon_img_path.path);
        lv_image_set_src(saver_info->clock_colon, (const void *)&clock_colon_img_path);
        lv_obj_add_flag(saver_info->clock_colon, LV_OBJ_FLAG_HIDDEN);
    }

    if (is_show) {
        utc_time_t utc_time = {0};
        uint32_t timestamp = platform_get_utc_time();
        timestamp_to_utc_time(timestamp, &utc_time);
        if (saver_info->hour == utc_time.hour && saver_info->minute == utc_time.minute
            && saver_info->force_update == false) {
            return;
        }
        char time_text[16] = {0};
        lv_obj_clear_flag(saver_info->clock_hour, LV_OBJ_FLAG_HIDDEN);
        snprintf(time_text, sizeof(time_text), "%02d", utc_time.hour);
        lv_label_set_text(saver_info->clock_hour, time_text);
        lv_obj_align(saver_info->clock_hour, LV_ALIGN_TOP_LEFT, 
            HOME_CLOCK_HOUR_X, HOME_CLOCK_TIME_Y);
        lv_obj_clear_flag(saver_info->clock_minute, LV_OBJ_FLAG_HIDDEN);
        snprintf(time_text, sizeof(time_text), "%02d", utc_time.minute);
        lv_label_set_text(saver_info->clock_minute, time_text);
        lv_obj_align(saver_info->clock_minute, LV_ALIGN_TOP_LEFT, 
            HOME_CLOCK_MINUTE_X, HOME_CLOCK_TIME_Y);
        lv_obj_clear_flag(saver_info->clock_colon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(saver_info->clock_colon, LV_ALIGN_TOP_LEFT, 
            HOME_CLOCK_COLON_X, HOME_CLOCK_COLON_Y);
        saver_info->hour = utc_time.hour;
        saver_info->minute = utc_time.minute;
    } else {
        lv_obj_add_flag(saver_info->clock_hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(saver_info->clock_minute, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(saver_info->clock_colon, LV_OBJ_FLAG_HIDDEN);
    }

}

static void home_screen_saver_ui_update(ui_manager_context_t *thiz, ui_proc_home_t *proc)
{
    home_page_display_type_e home_ui_type = proc->home_display_type;
    if (home_ui_type != saver_info->home_screen_saver_display_type) {
        saver_info->home_screen_saver_display_type = home_ui_type;
        saver_info->force_update = true;
    }

    if (home_ui_type == HOME_UI_DISPLAY_CLOCK || 
        home_ui_type == HOME_UI_DISPLAY_CLOCK_AND_RECORD ||
        home_ui_type == HOME_UI_DISPLAY_CLOCK_LOW_POWER ||
        home_ui_type == HOME_UI_DISPLAY_CLOCK_LIGHT_TIP) {
        home_screen_saver_clock_ui_update(thiz, true);
        home_screen_saver_dummy_icon_update(thiz, 43);
    } else {
        home_screen_saver_clock_ui_update(thiz, false);
        home_screen_saver_dummy_icon_update(thiz, 0);
    }
}
#else
static void home_screen_saver_ui_update(ui_manager_context_t *thiz, ui_proc_home_t *proc)
{
    home_screen_saver_dummy_icon_update(thiz, 0);   
}
#endif

/*  
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_screen_saver_custom_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (saver_info == NULL) {
        saver_info = (home_screen_saver_display_info_t *)u_malloz(sizeof(home_screen_saver_display_info_t));
        EXCEPTION_CHECK_VOID(saver_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    home_screen_saver_root_init();
    common_ui_bg_init(thiz);
    ble_ui_init(thiz);
    saver_info->force_update = true;
}

void home_screen_saver_custom_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && saver_info);
    common_ui_img_resource_deinit(thiz, &saver_info->saver_img_info);
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
    common_ui_img_resource_deinit(thiz, &saver_info->clock_colon_info);
#endif
    ble_ui_deinit(thiz);
    common_ui_bg_deinit(thiz);
    LVGL_OBJ_DELETE(saver_info->root);
    u_free(saver_info);
    saver_info = NULL;
}

int home_screen_saver_custom_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && saver_info && saver_info->root);
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->ble);
    home_screen_saver_root_bg_update(thiz);
    ble_ui_update(thiz, saver_info->root, proc->ble, COMMON_BLE_UI_X, COMMON_BLE_UI_Y);
    home_screen_saver_ui_update(thiz, proc);
    saver_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home screen saver fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
