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
#define IS_SUPPORT_AIRPLANE_MODE_COMP_BG                          (0)
#define AIRPLANE_MODE_TEXT_COUNT                                  (2)
#define AIRPLANE_MODE_ICON_COUNT                                  (1)
#define AIRPLANE_MODE_UNLOCK_BUTTON_ICON_INDEX                    (0)
#define AIRPLANE_MODE_TITLE_COLOR                                 0xB25962
#define AIRPLANE_MODE_TEXT_COLOR                                  0xC5787E

typedef struct {
    const char *name;
    uint16_t text_x;
    uint16_t text_y;
    const lv_font_t *font;
} airplane_mode_text_info_t;

typedef struct {
    lv_obj_t * root;
    #if IS_SUPPORT_AIRPLANE_MODE_COMP_BG
    lv_obj_t * backgroud;
    #endif
    lv_obj_t * unlock_button_icon;
    lv_obj_t * texts[AIRPLANE_MODE_TEXT_COUNT];
    res_img_info_t unlock_button_icon_info;
    uint8_t force_update;
    uint8_t popup_ui_index; /// 当前弹窗索引.
} airplane_mode_display_info_t;

typedef struct {
    const char *name;
    uint16_t img_id;
    uint16_t icon_x;
    uint16_t icon_y;
} airplane_mode_icon_info_t;
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_AIRPLANE_MODE_COMP_BG
static PSRAM_DATA_LOC uint8_t complete_tip_bg_data[] = {
    #include "comparison_png2txt/airplane_mode_360x360.txt"
};
#endif

static airplane_mode_display_info_t *airplane_mode_info;

const static airplane_mode_text_info_t airplane_mode_text_infos[AIRPLANE_MODE_TEXT_COUNT] = {
    { "Device  locked", (116), (118 - 2), &lv_font_oppo_sans_20_medium},
    { "Press and hold Start/Pause  +  Mode\nfor 5 seconds to unlock.", (48), (146 + 4), &lv_font_oppo_sans_16_medium},
};

const static airplane_mode_icon_info_t airplane_mode_icon_infos[AIRPLANE_MODE_ICON_COUNT] = {
    { "unlock_button_icon", RESOURCE_ID_AIRPLANE_UNLOCK_BUTTON_ICON, (106), (188 + 16)},
};
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void airplane_mode_root_init(void)
{
    if (airplane_mode_info->root == NULL || 
        !lv_obj_is_valid(airplane_mode_info->root)) {
        airplane_mode_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(airplane_mode_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(airplane_mode_info->root, lv_color_white(), 0);     /// 设置背景颜色为白色
    lv_obj_set_style_bg_opa(airplane_mode_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(airplane_mode_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(airplane_mode_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(airplane_mode_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(airplane_mode_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(airplane_mode_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
}

#if IS_SUPPORT_AIRPLANE_MODE_COMP_BG
static void airplane_mode_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (airplane_mode_info->backgroud == NULL || !lv_obj_is_valid(airplane_mode_info->backgroud)) {
        airplane_mode_info->backgroud = lv_image_create(airplane_mode_info->root);
        EXCEPTION_CHECK_VOID(airplane_mode_info->backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(airplane_mode_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(airplane_mode_info->backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, complete_tip_bg_data,
            sizeof(complete_tip_bg_data), "png");
        lv_image_set_src(airplane_mode_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(airplane_mode_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void airplane_mode_bg_update(ui_manager_context_t *thiz)
{
    common_ui_bg_update(thiz, airplane_mode_info->root);
}
#endif

static void airplane_mode_button_icon_update(ui_manager_context_t *thiz)
{
    if (airplane_mode_info->force_update == false) {
        return;
    }
    if (airplane_mode_info->unlock_button_icon == NULL ||
        !lv_obj_is_valid(airplane_mode_info->unlock_button_icon)) {
        airplane_mode_info->unlock_button_icon = lv_image_create(airplane_mode_info->root);
    }
    common_ui_img_resource_init(thiz, RESOURCE_ID_AIRPLANE_UNLOCK_BUTTON_ICON, &airplane_mode_info->unlock_button_icon_info);
    EXCEPTION_CHECK_VOID(airplane_mode_info->unlock_button_icon_info.data);
    lv_fs_path_ex_t unlock_button_img_path;
    lv_fs_make_path_from_buffer(&unlock_button_img_path, LV_FS_MEMFS_LETTER,
        airplane_mode_info->unlock_button_icon_info.data, airplane_mode_info->unlock_button_icon_info.size, "baj");
    lv_image_set_src(airplane_mode_info->unlock_button_icon, unlock_button_img_path.path);
    lv_obj_align(airplane_mode_info->unlock_button_icon, LV_ALIGN_TOP_LEFT,
        airplane_mode_icon_infos[AIRPLANE_MODE_UNLOCK_BUTTON_ICON_INDEX].icon_x,
        airplane_mode_icon_infos[AIRPLANE_MODE_UNLOCK_BUTTON_ICON_INDEX].icon_y);
}

static void airplane_mode_texts_update(ui_manager_context_t *thiz, uint8_t index)
{
    if (airplane_mode_info->force_update == false) {
        return;
    }
    if (airplane_mode_info->texts[index] == NULL || !lv_obj_is_valid(airplane_mode_info->texts[index])) {
        airplane_mode_info->texts[index] = lv_label_create(airplane_mode_info->root);
    }
    lv_label_set_text(airplane_mode_info->texts[index], airplane_mode_text_infos[index].name);
    lv_obj_set_style_text_color(airplane_mode_info->texts[index], 
        index == 0 ? lv_color_hex(AIRPLANE_MODE_TITLE_COLOR) : lv_color_hex(AIRPLANE_MODE_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(airplane_mode_info->texts[index], airplane_mode_text_infos[index].font, 0);  /// 设置字体
    if (index == 1) {
        lv_obj_set_style_text_line_space(airplane_mode_info->texts[index], 4, 0);  /// 设置行间距（像素）
    }
    lv_obj_set_style_text_align(airplane_mode_info->texts[index], LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
    lv_obj_align(airplane_mode_info->texts[index], LV_ALIGN_TOP_LEFT,
        airplane_mode_text_infos[index].text_x, airplane_mode_text_infos[index].text_y);
}

static void airplane_mode_popup_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(airplane_mode_info);
    ui_display_airplane_mode_exit_remove(thiz);
}

static void airplane_mode_popup_ui_update(ui_manager_context_t *thiz, ui_proc_airplane_mode_t *proc)
{
    switch (proc->popup_ui_type) {
        case AIRPLANE_MODE_POPUP_UI_NONE:
            airplane_mode_popup_ui_remove(thiz);
            break;
        case AIRPLANE_MODE_POPUP_UI_EXIT:
            if (airplane_mode_info->popup_ui_index != AIRPLANE_MODE_POPUP_UI_EXIT) {
                airplane_mode_popup_ui_remove(thiz);
                ui_display_airplane_mode_exit_prepare(thiz, airplane_mode_info->root);
                airplane_mode_info->popup_ui_index = AIRPLANE_MODE_POPUP_UI_EXIT;
            } else {
                ui_display_airplane_mode_exit_process(thiz);
            }
            break;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_airplane_mode_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (airplane_mode_info == NULL) {
        airplane_mode_info = (airplane_mode_display_info_t *)u_malloz(sizeof(airplane_mode_display_info_t));
        EXCEPTION_CHECK_VOID(airplane_mode_info);
    }
    airplane_mode_info->force_update = true;
    ble_ui_init(thiz);
    airplane_mode_root_init();
    common_ui_bg_init(thiz);
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_airplane_mode_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz); 
    EXCEPTION_CHECK_VOID(airplane_mode_info);
    EXCEPTION_CHECK_VOID(airplane_mode_info->root);
    ble_ui_deinit(thiz);
    common_ui_bg_deinit(thiz);
    common_ui_img_resource_deinit(thiz, &airplane_mode_info->unlock_button_icon_info);
    ui_display_airplane_mode_exit_remove(thiz);
    LVGL_OBJ_DELETE(airplane_mode_info->root); 
    u_free(airplane_mode_info);
    airplane_mode_info = NULL;
}

int ui_display_airplane_mode_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(airplane_mode_info);
    EXCEPTION_CHECK_INT(airplane_mode_info->root);
    ui_proc_airplane_mode_t *proc = (ui_proc_airplane_mode_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->ble);
    airplane_mode_bg_update(thiz);
    airplane_mode_button_icon_update(thiz);
    ble_ui_update(thiz, airplane_mode_info->root, proc->ble, 168, 22);
    for (uint8_t i = 0; i < AIRPLANE_MODE_TEXT_COUNT; i++) {
        airplane_mode_texts_update(thiz, i);
    }
    airplane_mode_popup_ui_update(thiz, proc);
    airplane_mode_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        LOG_UI("===> airplane mode fps=%d", thiz->priv.fps_count);
        thiz->priv.fps_start_time = get_timestamp();
        thiz->priv.fps_count = 0;
    }
    return thiz->priv.fps_count;
}