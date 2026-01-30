/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PNG_TEST                0
#define IS_SUPPORT_BAJ_TEST                0
#define IS_SUPPORT_BG_TEST                 0

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_PNG_TEST

#define PNG_IMG_COUNT  2

static const uint8_t png_data0[] = {
    #include "comparison_png2txt/ver_img_360x360.txt"

};
static const uint8_t png_data1[] = {
    #include "comparison_png2txt/ver_img_360x360.txt"
};

static ui_img_data_t png_images[PNG_IMG_COUNT] = {
    { png_data0, sizeof(png_data0) },
    { png_data1, sizeof(png_data1) },
};
static lv_obj_t *logo_root;

#elif IS_SUPPORT_BAJ_TEST

#define BAJ_IMG_COUNT                4

static const uint8_t baj_data0[] = {
    #include "baj2txt/IP_00000_200x200_a4_q99_422.txt"
    // #include "baj2txt/start_logo_icon_221x29_a4_q99_422.txt"
};

static const uint8_t baj_data1[] = {
    #include "baj2txt/IP_00149_200x200_a4_q99_422.txt"
    // #include "baj2txt/charge_page_icon_90x48_a4_q99_422.txt"
};

static const uint8_t baj_data2[] = {
    #include "baj2txt/charge_page_icon_90x48_a4_q99_422.txt"
};

static const uint8_t baj_data3[] = {
    #include "baj2txt/home_uncharge_battery_100_37x21_a4_q99_422.txt"
};


static const uint8_t baj_data_bg[] = {
    #include "comparison_png2txt/home_bg_v2_360x360.txt"
};

static ui_img_data_t baj_images[BAJ_IMG_COUNT] = {
    { baj_data0, sizeof(baj_data0) },
    { baj_data1, sizeof(baj_data1) },
    { baj_data2, sizeof(baj_data2) },
    { baj_data3, sizeof(baj_data3) },
};

static lv_obj_t *logo_root;
static lv_obj_t *logo_animation;
static lv_draw_buf_t *logo_bg_snapshot = NULL;  /// 背景快照，避免重复解码
#elif IS_SUPPORT_BG_TEST
static lv_obj_t *logo_root;
#else
static uint8_t logo_force_update = false;
static lv_obj_t *logo_root;
static res_img_info_t logo_img_info;
#if 0
static PSRAM_DATA_LOC ALIGNED(64) uint8_t baj_data_bg[] = {
    // #include "baj2txt/IP_00000_200x200_a4_q99_422.txt"
    #include "baj2txt/charge_page_icon_90x48_a4_q99_422.txt"
    // #include "baj2txt/start_logo_icon_221x29_a4_q99_422.txt"
    // #include "baj2txt/home_uncharge_battery_75_37x21_a4_q99_422.txt"
    // #include "baj2txt/home_uncharge_battery_100_37x21_a4_q99_422.txt"
    // #include "baj2txt/air_leakage_sign_24x24_a4_q99_422.txt"
};
#endif
#endif



/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_PNG_TEST
static int system_test_png_init(void)
{
    if (logo_root == NULL || !lv_obj_is_valid(logo_root)) {
        logo_root = lv_image_create(lv_screen_active());
    }
    lv_fs_path_ex_t png_path;
    lv_fs_make_path_from_buffer(&png_path, LV_FS_MEMFS_LETTER, png_data0, sizeof(png_data0), "png");
    lv_image_set_src(logo_root, (const void *)&png_path);
    lv_obj_align(logo_root, LV_ALIGN_CENTER, 0, 0);
    return 0;
}

static int system_test_png_process(void)
{
    static uint32_t index = 0;
    lv_fs_path_ex_t png_path;
    const uint8_t *png_data = png_images[index % PNG_IMG_COUNT].data;
    uint32_t png_size = png_images[index % PNG_IMG_COUNT].size;
    lv_fs_make_path_from_buffer(&png_path, LV_FS_MEMFS_LETTER, png_data, png_size, "png");
    lv_image_set_src(logo_root, (const void *)&png_path);
#if 1
    LOG_UI("===> test 7 png: %d", index);
    if (get_timestamp() - start_time > 5000) {
        start_time = get_timestamp();
        index++;
        if (index > 2) {
            index = 0;
        }
    }
#else
    index++;
#endif
    return 0;
}

#elif IS_SUPPORT_BAJ_TEST
static int system_test_baj_init(ui_manager_context_t *thiz)
{
    if (logo_root == NULL || !lv_obj_is_valid(logo_root)) {
        logo_root = lv_image_create(lv_screen_active());
    }
    lv_fs_path_ex_t bg_path;
    lv_fs_make_path_from_buffer(&bg_path, LV_FS_MEMFS_LETTER, baj_data_bg, sizeof(baj_data_bg), "png");
    lv_image_set_src(logo_root, (const void *)&bg_path);
    lv_obj_align(logo_root, LV_ALIGN_CENTER, 0, 0);
    lv_refr_now(NULL); /// 等待渲染完成后再创建快照
#if LV_USE_SNAPSHOT
    logo_bg_snapshot = lv_snapshot_take(logo_root, LV_COLOR_FORMAT_NATIVE_WITH_ALPHA);
#else
    logo_bg_snapshot = NULL;
    LOGE("===> LV_USE_SNAPSHOT is not enabled");
#endif
    if (logo_bg_snapshot != NULL) {
        lv_image_set_src(logo_root, logo_bg_snapshot);
    } else {
        LOGE("===> logo_bg snapshot creation failed");
    }
    return 0;
}

static int system_test_baj_process(ui_manager_context_t *thiz)
{
    static uint32_t index = 0;
    if (logo_animation == NULL || !lv_obj_is_valid(logo_animation)) {
        logo_animation = lv_image_create(lv_screen_active());
        lv_obj_align(logo_animation, LV_ALIGN_BOTTOM_MID, 0, 0);
    }
    lv_fs_path_ex_t baj_path;
    const uint8_t *baj_data = baj_images[index % BAJ_IMG_COUNT].data;
    uint32_t baj_size = baj_images[index % BAJ_IMG_COUNT].size;
    lv_fs_make_path_from_buffer(&baj_path, LV_FS_MEMFS_LETTER, baj_data, baj_size, "baj");
    lv_image_set_src(logo_animation, (const void *)&baj_path);
    index++;
    return 0 ;
}

#elif IS_SUPPORT_BG_TEST
static uint32_t rgb_color_change_time = 0;
static int system_lcd_test_init(void)
{
    /// 设置初始背景色为红色
    lv_obj_t *scr = lv_screen_active();
    if (scr != NULL && lv_obj_is_valid(scr)) {
        lv_obj_set_style_bg_color(scr, lv_color_hex(0xFF0000), LV_PART_MAIN);
    }
    rgb_color_change_time = get_timestamp();
    return 0;
}

static int system_lcd_test_process(void)
{
    /// 五色交替：每 1 秒切换一次颜色（红、绿、蓝、黑、白）
    static uint8_t color_index = 0;  /// 0=红色, 1=绿色, 2=蓝色, 3=黑色, 4=白色
    uint32_t color_colors[5] = {0xFF0000, 0x00FF00, 0x0000FF, 0x000000, 0xFFFFFF};  /// 红、绿、蓝、黑、白
    
    if (get_timestamp() - rgb_color_change_time > 1000) {
        rgb_color_change_time = get_timestamp();
        color_index = (color_index + 1) % 5;
        
        /// 添加对象有效性检查和线程安全保护
        /// 注意：在 VGLite 渲染线程正在工作时修改对象可能导致崩溃
        /// 使用 lv_refr_now() 确保重绘完成，避免与 VGLite 渲染线程冲突
        lv_obj_t *scr = lv_screen_active();
        if (scr != NULL && lv_obj_is_valid(scr)) {
            lv_obj_set_style_bg_color(scr, lv_color_hex(color_colors[color_index]), LV_PART_MAIN);
            /// 立即触发重绘并等待完成，确保 VGLite 渲染线程有时间完成当前任务
            /// 这样可以避免在 GPU 等待中断期间修改对象导致的崩溃
            lv_refr_now(NULL);
            
            LOG_UI("===> lcd test color: %d", color_index);
        } else {
            LOGE("===> lcd test: invalid screen object");
        }
    }
    return 0;
}
#else
static void sys_logo_label_update(ui_manager_context_t *thiz)
{
    common_ui_img_resource_init(thiz, RESOURCE_ID_START_LOGO_ICON, &logo_img_info);
    EXCEPTION_CHECK_VOID(logo_img_info.data);
    if (logo_root == NULL || !lv_obj_is_valid(logo_root)) {
        logo_root = lv_image_create(lv_screen_active());
    }
    lv_obj_set_size(logo_root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(logo_root, lv_color_white(), 0);     /// 设置背景颜色为白色
    lv_obj_set_style_bg_opa(logo_root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(logo_root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(logo_root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_fs_path_ex_t logo_img_path;
#if 0
    lv_fs_make_path_from_buffer(&logo_img_path, LV_FS_MEMFS_LETTER, baj_data_bg, sizeof(baj_data_bg), "baj");
#else
    lv_fs_make_path_from_buffer(&logo_img_path, LV_FS_MEMFS_LETTER, logo_img_info.data,
        logo_img_info.size, "baj");
#endif
    lv_image_set_src(logo_root, (const void *)&logo_img_path);
    lv_obj_align(logo_root, LV_ALIGN_CENTER, 0, 0);    /// 设置模式标签位置
    lv_obj_clear_flag(logo_root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(logo_root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_logo_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
#if IS_SUPPORT_PNG_TEST
    system_test_png_init();
#elif IS_SUPPORT_BG_TEST
    system_lcd_test_init();
#elif IS_SUPPORT_BAJ_TEST
    system_test_baj_init(thiz);
#else
    logo_force_update = true;
#endif
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_logo_remove(ui_manager_context_t *thiz)
{
#if IS_SUPPORT_PNG_TEST
    LVGL_OBJ_DELETE(logo_root);
#elif IS_SUPPORT_BAJ_TEST
    LVGL_OBJ_DELETE(logo_animation);
    LVGL_OBJ_DELETE(logo_root);
    if (logo_bg_snapshot != NULL) {
        lv_draw_buf_destroy(logo_bg_snapshot);
        logo_bg_snapshot = NULL;
    }
#elif IS_SUPPORT_BG_TEST    
    LVGL_OBJ_DELETE(logo_root);
#else
    LVGL_OBJ_DELETE(logo_root);
    common_ui_img_resource_deinit(thiz, &logo_img_info);
#endif

}

int ui_display_logo_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
#if IS_SUPPORT_PNG_TEST
    system_test_png_process();
#elif IS_SUPPORT_BG_TEST
    system_lcd_test_process();
#elif IS_SUPPORT_BAJ_TEST
    system_test_baj_process(thiz);
#else
    if (logo_force_update == true) {
        logo_force_update = false;
        sys_logo_label_update(thiz);
    }   
#endif
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> logo fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
