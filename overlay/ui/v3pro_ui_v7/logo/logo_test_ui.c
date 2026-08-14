/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_logo.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#if IS_SUPPORT_LOGO_UI_TEST

#define IS_SUPPORT_LOGO_COMP_BG                                (0)
#define IS_SUPPORT_LOGO_IMG_TEST                               (0)
#define IS_SUPPORT_LOGO_TEST_PAGE                              (0)
#define IS_SUPPORT_LOGO_WAVE_TEST                              (0)
#define IS_SUPPORT_LOGO_COLOR_TEST                             (1)

#define LOGO_WAVE_X                                            (0)
#define LOGO_WAVE_Y                                            (118)
#define LOGO_WAVE_COLOR                                        (0xFFFFFF)
#define LOGO_WAVE_SHADOW_COLOR                                 (0xFFE5E7)
#define LOGO_WAVE_SHADOW_Y_OFFSET                              (3)

typedef struct {
    lv_obj_t * root;
    lv_obj_t * test_page;
    res_img_info_t test_img_info;
    uint8_t test_page_index;
    uint16_t animation_index;
    uint8_t is_test_anim;
    lv_obj_t * logo_icon;
#if IS_SUPPORT_LOGO_WAVE_TEST
    ui_wave_t *wave_ui;
    uint8_t wave_type;
#endif
#if IS_SUPPORT_LOGO_COLOR_TEST
    uint8_t color_index;
#endif
    uint32_t start_time;
    uint8_t force_update;
    uint8_t rsv[15];
} logo_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static logo_display_info_t *logo_info = NULL;
#if IS_SUPPORT_LOGO_IMG_TEST
static PSRAM_DATA_LOC uint8_t logo_img_data1[] = {
    // #include "v3_res_v7/base/jpg2txt/bg_simulation_mode_352x352.txt"
    #include "v3_res_v7/base/png2txt/bg_simulation_mode_360x360.txt"
};    

static PSRAM_DATA_LOC uint8_t logo_img_data2[] = {
    // #include "v3_res_v7/base/jpg2txt/bg_mixed_mode_352x352.txt"
    #include "v3_res_v7/base/png2txt/bg_mixed_mode_360x360.txt"
};    

static PSRAM_DATA_LOC uint8_t logo_img_data3[] = {
    // #include "v3_res_v7/base/jpg2txt/bg_expression_mode_352x352.txt"
    #include "v3_res_v7/base/png2txt/bg_expression_mode_360x360.txt"
};    
    
static PSRAM_DATA_LOC uint8_t logo_img_data4[] = {
    // #include "v3_res_v7/base/jpg2txt/bg_program_mode_352x352.txt"
    #include "v3_res_v7/base/png2txt/bg_program_mode_360x360.txt"
};   

#define LOGO_IMG_COUNT 4
static ui_img_data_t logo_img[LOGO_IMG_COUNT] = {
    {logo_img_data1, sizeof(logo_img_data1)},
    {logo_img_data2, sizeof(logo_img_data2)},
    {logo_img_data3, sizeof(logo_img_data3)},
    {logo_img_data4, sizeof(logo_img_data4)},
};
#endif

#if IS_SUPPORT_LOGO_COLOR_TEST
const uint32_t logo_test_color_list[] = {
    0,
    0xFF0000,
    0x00FF00,
    0x0000FF,
    0xFFFFFF,
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
static void ui_logo_root_init(void)
{
    if (logo_info->root == NULL || !lv_obj_is_valid(logo_info->root)) {
        logo_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(logo_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(logo_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    // lv_obj_set_style_bg_color(logo_info->root, lv_color_hex(0xF44B77), 0);     /// 设置背景颜色为粉色
    lv_obj_set_style_bg_opa(logo_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(logo_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(logo_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(logo_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(logo_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(logo_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

static void ui_logo_bg_update(ui_manager_context_t *thiz)
{

}

#if IS_SUPPORT_LOGO_WAVE_TEST
static ui_wave_type_e logo_wave_type_from_page(uint8_t page_id)
{
    switch (page_id % 3U) {
    case 0:
        return UI_WAVE_TYPE_MIX;
    case 1:
        return UI_WAVE_TYPE_STIMULATION;
    case 2:
    default:
        return UI_WAVE_TYPE_LACTATION;
    }
}

static void logo_wave_ui_prepare(void)
{
    if (logo_info->wave_ui != NULL) {
        return;
    }

    ui_wave_cfg_t cfg;
    ui_wave_cfg_init_default(&cfg);
    cfg.color = LOGO_WAVE_COLOR;
    cfg.shadow_enable = 0U;
    cfg.shadow_color = LOGO_WAVE_SHADOW_COLOR;
    cfg.shadow_y_offset = LOGO_WAVE_SHADOW_Y_OFFSET;
    cfg.type = UI_WAVE_TYPE_MIX;
    cfg.anim_mode = UI_WAVE_ANIM_SCROLL;

    logo_info->wave_ui = ui_wave_create(logo_info->root, &cfg);
    EXCEPTION_CHECK_VOID(logo_info->wave_ui);
    if (ui_wave_get_shadow_obj(logo_info->wave_ui) != NULL) {
        lv_obj_align(ui_wave_get_shadow_obj(logo_info->wave_ui),
            LV_ALIGN_TOP_MID, LOGO_WAVE_X, LOGO_WAVE_Y);
    }
    lv_obj_align(ui_wave_get_obj(logo_info->wave_ui),
        LV_ALIGN_TOP_MID, LOGO_WAVE_X, LOGO_WAVE_Y);
    logo_info->wave_type = 0xFF;
}

static void logo_wave_ui_update(uint8_t page_id)
{
    logo_wave_ui_prepare();
    EXCEPTION_CHECK_VOID(logo_info->wave_ui);

    const ui_wave_type_e wave_type = logo_wave_type_from_page(page_id);
    if (logo_info->wave_type != (uint8_t)wave_type) {
        ui_wave_set_type(logo_info->wave_ui, wave_type);
        logo_info->wave_type = (uint8_t)wave_type;
        LOG_UI("logo wave type: %d", wave_type);
    }
}

static void logo_wave_ui_remove(void)
{
    if (logo_info->wave_ui != NULL) {
        ui_wave_delete(logo_info->wave_ui);
        logo_info->wave_ui = NULL;
        logo_info->wave_type = 0xFF;
    }
}
#endif

#if IS_SUPPORT_LOGO_IMG_TEST
static void sys_logo_label_update(ui_manager_context_t *thiz, uint8_t index)
{
    if (logo_info->logo_icon == NULL || !lv_obj_is_valid(logo_info->logo_icon)) {
        logo_info->logo_icon = lv_image_create(logo_info->root);
        EXCEPTION_CHECK_VOID(logo_info->logo_icon);
    }

    uint8_t page_id = index % LOGO_IMG_COUNT;
    if (logo_info->test_page_index == page_id && logo_info->force_update == false) {
        return;
    }
    LOG_UI("logo_img id: %d/%d", page_id, index);
    lv_fs_path_ex_t logo_img_path;
    lv_fs_make_path_from_buffer(&logo_img_path, LV_FS_MEMFS_LETTER, logo_img[page_id].data,
        logo_img[page_id].size, "png");
    lv_image_set_src(logo_info->logo_icon, (const void *)&logo_img_path);
    lv_obj_align(logo_info->logo_icon, LV_ALIGN_CENTER, 0, 0);    /// 设置logo图标位置  
    logo_info->force_update = false;
    logo_info->test_page_index = page_id;
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void logo_test_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (logo_info == NULL) {
        logo_info = (logo_display_info_t *)u_malloz(sizeof(logo_display_info_t));
        EXCEPTION_CHECK_VOID(logo_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_logo_root_init(); /// 初始化 root 容器
    logo_info->test_page_index = 0;
    logo_info->force_update = true;
#if IS_SUPPORT_LOGO_WAVE_TEST
    logo_wave_ui_prepare();
#endif
#if IS_SUPPORT_LOGO_COLOR_TEST
    logo_info->color_index = 0;
#endif
}

void logo_test_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && logo_info && logo_info->root);
#if IS_SUPPORT_LOGO_WAVE_TEST
    logo_wave_ui_remove();
#endif
    ui_img_resource_deinit(thiz, &logo_info->test_img_info);
    LVGL_OBJ_DELETE(logo_info->root);
    u_free(logo_info);
    logo_info = NULL;
}

int logo_test_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && logo_info && logo_info->root); 
#if (IS_SUPPORT_LOGO_IMG_TEST == 1 && IS_SUPPORT_LOGO_COMP_BG == 0)
    ui_proc_logo_t *proc = (ui_proc_logo_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    sys_logo_label_update(thiz, proc->test_page_count);
#elif IS_SUPPORT_LOGO_WAVE_TEST
    ui_proc_logo_t *proc = (ui_proc_logo_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    logo_wave_ui_update(proc->test_page_count);
#elif IS_SUPPORT_LOGO_COLOR_TEST
    lv_obj_set_style_bg_color(logo_info->root, 
        lv_color_hex(logo_test_color_list[logo_info->color_index]), 0);    
#else
    ui_proc_logo_t *proc = (ui_proc_logo_t *)thiz->priv.play_info.user_data;
    uint8_t is_white_color = (proc->test_page_count % 2) ? false : true;
    if (is_white_color) {
        lv_obj_set_style_bg_color(logo_info->root, lv_color_white(), 0);    
    } else {
        lv_obj_set_style_bg_color(logo_info->root, lv_color_black(), 0);    
    }
#endif
    logo_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> logo fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
        #if IS_SUPPORT_LOGO_COLOR_TEST
        logo_info->color_index++;
        if (logo_info->color_index >= sizeof(logo_test_color_list) / sizeof(logo_test_color_list[0])) {
            logo_info->color_index = 0;                
            key_input_context_t *key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
            if (key) {
                key->dispatch_client_input_key(KEY_CODE_LACTATION_MODE, KEY_EVENT_CLICK);
            }
        }
        #endif
    }
    return 0;
}
#endif