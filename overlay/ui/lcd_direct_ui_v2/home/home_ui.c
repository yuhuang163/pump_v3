/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep
#include "ble_ui.h" // IWYU pragma: keep
/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_ROOT_HOME_COMP_BG                (0)

#define HOME_ICON_MAX_COUNT                         (3)
#define HOME_TEXT_LABEL_INDEX                       (0)
#define HOME_TEXT_AREA_ICON_INDEX                   (1)
#define HOME_BUTTON_ICON_INDEX                      (2)
#define HOME_MOM_PICTURE_ICON_INDEX                 (3)

#define IDLE_TEXT_AREA_ICON_X                       (44)
#define IDLE_TEXT_AREA_ICON_Y                       (82)
#define IDLE_BUTTON_ICON_X                          (80)
#define IDLE_BUTTON_ICON_Y                          (112+7)
#define MOM_PICTURE_ICON_X                          (56)
#define MOM_PICTURE_ICON_Y                          (160)

#define IDLE_TEXT_LABEL_TEXT_X                      (80 + 28)
#define IDLE_TEXT_LABEL_TEXT_Y                      (118 - 4)
#define IDLE_TEXT_COLOR                             (0xB25962)


typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_ROOT_HOME_COMP_BG
    lv_obj_t * backgroud;
#endif
    lv_obj_t * text;
    lv_obj_t * idle_text_area_icon;
    res_img_info_t idle_text_area_info;
    lv_obj_t * idle_button_icon;
    res_img_info_t idle_button_info;
    lv_obj_t * animation;
    res_img_info_t animation_info;
    uint16_t animation_index;
    uint8_t force_update;
} home_display_info_t;

typedef struct {
    const char *name;
    uint16_t img_id;
    uint16_t icon_x;
    uint16_t icon_y;
} home_icon_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static home_display_info_t *home_info;
const static home_icon_info_t home_icon_infos[HOME_ICON_MAX_COUNT] = {
    { "idle_text_label", 0, 
        IDLE_TEXT_LABEL_TEXT_X,       IDLE_TEXT_LABEL_TEXT_Y, 
      },
    { "idle_text_area_icon", RESOURCE_ID_IDLE_TEXT_AREA_ICON, 
      IDLE_TEXT_AREA_ICON_X,       IDLE_TEXT_AREA_ICON_Y, 
    },
    { "idle_button_icon", RESOURCE_ID_IDLE_BUTTOM_ICON,     
      IDLE_BUTTON_ICON_X,              IDLE_BUTTON_ICON_Y, 
    },
};

#if IS_SUPPORT_ROOT_HOME_COMP_BG
static PSRAM_DATA_LOC uint8_t home_bg_data[] = {
    // #include "comparison_png2txt/bg_360x360.txt"
    #include "png2txt/home_com_bg_360x360.txt"
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
static void home_root_init(void)
{
    if (home_info->root == NULL || !lv_obj_is_valid(home_info->root)) {
        home_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(home_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(home_info->root, lv_color_white(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(home_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(home_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(home_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(home_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(home_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(home_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

#if IS_SUPPORT_ROOT_HOME_COMP_BG
static void home_root_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (home_info->backgroud == NULL || !lv_obj_is_valid(home_info->backgroud)) {
        home_info->backgroud = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(home_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(home_info->backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, home_bg_data,
            sizeof(home_bg_data), "png");
        lv_image_set_src(home_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(home_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void home_root_bg_update(ui_manager_context_t *thiz)
{
    common_ui_bg_update(thiz, home_info->root);
}
#endif
static void home_root_text_update(ui_manager_context_t *thiz)
{
    if (home_info->force_update == false) {
        return;
    }
    /// 设置文本区域icon
    if (home_info->idle_text_area_icon == NULL || !lv_obj_is_valid(home_info->idle_text_area_icon)) {
        home_info->idle_text_area_icon = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->idle_text_area_icon);
    }
    
    common_ui_img_resource_init(thiz, home_icon_infos[HOME_TEXT_AREA_ICON_INDEX].img_id, &home_info->idle_text_area_info);
    EXCEPTION_CHECK_VOID(home_info->idle_text_area_info.data);
    lv_fs_path_ex_t idle_text_area_img_path;
    lv_fs_make_path_from_buffer(&idle_text_area_img_path, LV_FS_MEMFS_LETTER, 
        home_info->idle_text_area_info.data, home_info->idle_text_area_info.size, "baj");
    lv_image_set_src(home_info->idle_text_area_icon, (const void *)&idle_text_area_img_path);
    lv_obj_align(home_info->idle_text_area_icon, LV_ALIGN_TOP_LEFT, 
        home_icon_infos[HOME_TEXT_AREA_ICON_INDEX].icon_x, home_icon_infos[HOME_TEXT_AREA_ICON_INDEX].icon_y); /// 设置icon位置
    /// 设置按钮icon
    if (home_info->idle_button_icon == NULL ||
        !lv_obj_is_valid(home_info->idle_button_icon)) {
      home_info->idle_button_icon = lv_image_create(home_info->root);
      EXCEPTION_CHECK_VOID(home_info->idle_button_icon);
    }

    common_ui_img_resource_init(thiz, home_icon_infos[HOME_BUTTON_ICON_INDEX].img_id,
        &home_info->idle_button_info);
    EXCEPTION_CHECK_VOID(home_info->idle_button_info.data);
    lv_fs_path_ex_t idle_button_img_path;
    lv_fs_make_path_from_buffer(&idle_button_img_path, LV_FS_MEMFS_LETTER,
                                home_info->idle_button_info.data,
                                home_info->idle_button_info.size, "baj");
    lv_image_set_src(home_info->idle_button_icon, (const void *)&idle_button_img_path);
    lv_obj_align(home_info->idle_button_icon, LV_ALIGN_TOP_LEFT, 
        home_icon_infos[HOME_BUTTON_ICON_INDEX].icon_x, home_icon_infos[HOME_BUTTON_ICON_INDEX].icon_y); /// 设置icon位置

    /// 设置文本label
    if (home_info->text == NULL || !lv_obj_is_valid(home_info->text)) {
        home_info->text = lv_label_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->text);
    }
    lv_label_set_text(home_info->text, "Select a mode to start\npumping.");  /// 设置 label 文字内容和样式
    lv_obj_set_style_text_color(home_info->text, lv_color_hex(IDLE_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(home_info->text, &lv_font_oppo_sans_14_medium, 0);  /// 设置字体
    lv_obj_align(home_info->text, LV_ALIGN_TOP_LEFT, home_icon_infos[HOME_TEXT_LABEL_INDEX].icon_x, home_icon_infos[HOME_TEXT_LABEL_INDEX].icon_y); /// 设置文字位置
    lv_obj_set_style_text_line_space(home_info->text, 3, 0); /// 设置行间距
}

static void home_root_animation_update(ui_manager_context_t *thiz)
{  
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(home_info);
    if (home_info->animation == NULL || !lv_obj_is_valid(home_info->animation)) {
        home_info->animation = lv_image_create(home_info->root);
        EXCEPTION_CHECK_VOID(home_info->animation);
    }  
    
    uint16_t animation_size = RESOURCE_ID_HOME_DEFAULT_ANIMATION_END - RESOURCE_ID_HOME_DEFAULT_ANIMATION_START + 1;
    if (home_info->animation_index >= animation_size) {
        home_info->animation_index = 0;
    }

    lv_obj_align(home_info->animation, LV_ALIGN_TOP_LEFT, 86, 162); 
    if (home_info->animation_info.data != NULL) {
        common_ui_img_resource_deinit(thiz, &home_info->animation_info);
        memset(&home_info->animation_info, 0, sizeof(home_info->animation_info));
    }
    common_ui_img_resource_init(thiz, 
        RESOURCE_ID_HOME_DEFAULT_ANIMATION_START + home_info->animation_index, &home_info->animation_info);
    EXCEPTION_CHECK_VOID(home_info->animation_info.data);
    lv_fs_path_ex_t animation_img_path;
    lv_fs_make_path_from_buffer(&animation_img_path, LV_FS_MEMFS_LETTER, 
        home_info->animation_info.data, home_info->animation_info.size, "baj");
    lv_image_set_src(home_info->animation, (const void *)&animation_img_path);
    home_info->animation_index++;
}
/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_home_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (home_info == NULL) {
        home_info = (home_display_info_t *)u_malloz(sizeof(home_display_info_t));
        EXCEPTION_CHECK_VOID(home_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    home_root_init();
    ble_ui_init(thiz);
    common_ui_bg_init(thiz);
    home_info->force_update = true;
    home_info->animation_index = 0;
}

void ui_display_home_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(home_info);
    common_ui_img_resource_deinit(thiz, &home_info->idle_text_area_info);
    common_ui_img_resource_deinit(thiz, &home_info->idle_button_info);
    common_ui_img_resource_deinit(thiz, &home_info->animation_info);
    ble_ui_deinit(thiz);
    common_ui_bg_deinit(thiz);
    LVGL_OBJ_DELETE(home_info->root);
    u_free(home_info);
    home_info = NULL;
}

int ui_display_home_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(home_info);
    EXCEPTION_CHECK_INT(home_info->root);
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->ble);
    home_root_bg_update(thiz);
    home_root_text_update(thiz);
    home_root_animation_update(thiz);
    ble_ui_update(thiz, home_info->root, proc->ble, 168, 22);
    home_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
