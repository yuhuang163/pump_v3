/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_GUIDE_COMP_BG                                (0)
#define GUIDE_PAGE_IMG_NUM                                      (6)
#define GUIDE_QRCODE_BG_COLOR                                   0xFFFFFF
#define GUIDE_QRCODE_FG_COLOR                                   0x000000
#define GUIDE_QRCODE_BORDER_COLOR                               (0xFFEAE6)  // 0xA6A6A6
#define GUIDE_TEXT_NUM                                          (4)
#define GUIDE_ARROW_TEXT_X_OFFSET                               (0)
#define GUIDE_ARROW_TEXT_Y_OFFSET                               (328)
#define GUIDE_TEXT_COLOR                                        (0xB25962)
#define GUIDE_BIND_TEXT_COLOR                                   (0x6A5A5B)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
typedef struct {
    lv_obj_t * root;
    lv_obj_t * qrcode;
    lv_obj_t * text[GUIDE_TEXT_NUM];
    lv_obj_t * page_img;
    res_img_info_t img_info;
    uint8_t force_update;
    uint8_t page_index;
    uint8_t popup_ui_index;
    uint8_t rsv[13];
} guide_display_info_t;

typedef struct {
    const char *text;
    const lv_font_t *font;
    uint16_t text_x;
    uint16_t text_y;
    lv_opa_t text_opa;
} guide_text_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static guide_display_info_t *guide_info;

const static char *bind_qr_str[] = {
    "https://lvgl.io",
};

const static uint16_t guide_page_img_ids[GUIDE_PAGE_IMG_NUM] = {
    RESOURCE_ID_GUIDE_PAGE1_IMG,
    RESOURCE_ID_GUIDE_PAGE2_IMG,
    RESOURCE_ID_GUIDE_PAGE3_IMG,
    RESOURCE_ID_GUIDE_PAGE4_IMG,
    RESOURCE_ID_GUIDE_PAGE5_IMG,
    RESOURCE_ID_GUIDE_PAGE6_IMG,
};

/// 首页
const static guide_text_info_t home_text_infos[GUIDE_TEXT_NUM] = {
    {"Pumping  Trip", &lv_font_oppo_sans_20_medium, 0, 51, LV_OPA_COVER},
    {"Turn right to start", &lv_font_oppo_sans_14_medium, 0, 84, LV_OPA_70},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, LV_OPA_70},
    {"Press down to exit", &lv_font_oppo_sans_14_medium, 0, GUIDE_ARROW_TEXT_Y_OFFSET, LV_OPA_70}
};

/// 开始泵奶
const static guide_text_info_t start_pump_text_infos[GUIDE_TEXT_NUM] = {
    {"Regular  Pumping", &lv_font_oppo_sans_20_medium, 0, 51, LV_OPA_COVER},
    {"2 min Stimulation  +  20 min Expression for\n most moms.                               ", 
    &lv_font_oppo_sans_14_medium, 0, 84, LV_OPA_70},
    {"Press to start", &lv_font_oppo_sans_14_bold, 176, 103, LV_OPA_70},
    {"Press down to exit", &lv_font_oppo_sans_14_medium, 0, GUIDE_ARROW_TEXT_Y_OFFSET, LV_OPA_70}
};

/// 设置泵奶
const static guide_text_info_t setting_pump_text_infos[GUIDE_TEXT_NUM] = {
    {"Adjust suction and speed per side by\n pressing buttons or knobs, or switch modes",
    &lv_font_oppo_sans_14_medium, 0, 208, LV_OPA_70},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, LV_OPA_COVER},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, LV_OPA_COVER},
    {"Press down to exit", &lv_font_oppo_sans_14_medium, 0, GUIDE_ARROW_TEXT_Y_OFFSET, LV_OPA_70}
};

/// 停止泵奶
const static guide_text_info_t stop_pump_text_infos[GUIDE_TEXT_NUM] = {
    {"Press and hold for 3  seconds to stop\n pumping", 
    &lv_font_oppo_sans_14_medium, 0, 208, LV_OPA_70},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, LV_OPA_COVER},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, LV_OPA_COVER},
    {"Press down to exit", &lv_font_oppo_sans_14_medium, 0, GUIDE_ARROW_TEXT_Y_OFFSET, LV_OPA_70}
};

/// 泵奶模式选择
const static guide_text_info_t pump_mode_select_text_infos[GUIDE_TEXT_NUM] = {
    {"More pumping options:3 modes, 6\nprograms.                                             ", 
    &lv_font_oppo_sans_14_medium, 0, 208, LV_OPA_70},
    {"Turn right to switch", &lv_font_oppo_sans_14_bold, 148, 226, LV_OPA_70},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, LV_OPA_COVER},
    {"Press down to exit", &lv_font_oppo_sans_14_medium, 0, GUIDE_ARROW_TEXT_Y_OFFSET, LV_OPA_70}
};

/// 绑定引导
const static guide_text_info_t bind_guide_text_infos[GUIDE_TEXT_NUM] = {
    {"Scan to connect for tracking,  remote\ncontrol, custom modes... ",
    &lv_font_oppo_sans_14_medium, 0, 228, LV_OPA_COVER},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, LV_OPA_COVER},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, LV_OPA_COVER},
    {"End  Trip", &lv_font_oppo_sans_14_medium, 0, GUIDE_ARROW_TEXT_Y_OFFSET, LV_OPA_COVER}
};

#if IS_SUPPORT_GUIDE_COMP_BG
static PSRAM_DATA_LOC uint8_t guide_bg_data[] = {
    // #include "comparison_png2txt/guide_home_360x360.txt"
    // #include "comparison_png2txt/guide_start_pump_360x360.txt"
    // #include "comparison_png2txt/guide_setting_page_360x360.txt"
    // #include "comparison_png2txt/guide_stop_pump_360x360.txt"
    // #include "comparison_png2txt/guide_mode_select_360x360.txt"
    #include "comparison_png2txt/guide_bind_360x360.txt"
};
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_guide_root_init(void)
{
    if (guide_info->root == NULL || !lv_obj_is_valid(guide_info->root)) {
        guide_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(guide_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(guide_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(guide_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(guide_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(guide_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(guide_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(guide_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(guide_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

static void guide_qrcode_update(ui_manager_context_t *thiz, uint8_t page_index)
{  
    if (guide_info->qrcode == NULL || !lv_obj_is_valid(guide_info->qrcode)) {
        guide_info->qrcode = lv_qrcode_create(guide_info->root);
        EXCEPTION_CHECK_VOID(guide_info->qrcode);
    } 
    
    if (page_index != GUIDE_PAGE_BIND_GUIDE) {
        lv_obj_add_flag(guide_info->qrcode, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(guide_info->qrcode, LV_OBJ_FLAG_HIDDEN);
    lv_qrcode_set_size(guide_info->qrcode, 124);
    lv_qrcode_set_dark_color(guide_info->qrcode, lv_color_hex(GUIDE_QRCODE_FG_COLOR));
    lv_qrcode_set_light_color(guide_info->qrcode, lv_color_hex(GUIDE_QRCODE_BG_COLOR));
    lv_qrcode_update(guide_info->qrcode, bind_qr_str[0], strlen(bind_qr_str[0]));
    lv_obj_center(guide_info->qrcode);
    lv_obj_align(guide_info->qrcode, LV_ALIGN_TOP_LEFT, 111 + 7, 71 + 7);
}

static void guide_ui_page_img_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    if (guide_info->page_img == NULL || !lv_obj_is_valid(guide_info->page_img)) {
        guide_info->page_img = lv_image_create(guide_info->root);
        EXCEPTION_CHECK_VOID(guide_info->page_img);
    } 
#if IS_SUPPORT_GUIDE_COMP_BG
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(guide_info->page_img, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(guide_info->page_img, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, guide_bg_data,
            sizeof(guide_bg_data), "png");
        lv_image_set_src(guide_info->page_img, (const void *)&bg_img_path);
        lv_obj_align(guide_info->page_img, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
#else
    common_ui_img_resource_init(thiz, guide_page_img_ids[page_index], &guide_info->img_info);
    EXCEPTION_CHECK_VOID(guide_info->img_info.data);
    lv_fs_path_ex_t page_img_path;
    lv_fs_make_path_from_buffer(&page_img_path, LV_FS_MEMFS_LETTER, guide_info->img_info.data,
        guide_info->img_info.size, "png");
    lv_image_set_src(guide_info->page_img, (const void *)&page_img_path);
    lv_obj_align(guide_info->page_img, LV_ALIGN_CENTER, 0, 0);
#endif
}

static void guide_ui_text_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    const guide_text_info_t *guide_text_infos = NULL;
    lv_color_t text_color = lv_color_hex(GUIDE_TEXT_COLOR);
    if (page_index == GUIDE_PAGE_HOME) {
        guide_text_infos = home_text_infos;
    } else if (page_index == GUIDE_PAGE_START_PUMP) {
        guide_text_infos = start_pump_text_infos;
    } else if (page_index == GUIDE_PAGE_SETTING_PUMP) {
        guide_text_infos = setting_pump_text_infos;
    } else if (page_index == GUIDE_PAGE_STOP_PUMP) {
        guide_text_infos = stop_pump_text_infos;
    } else if (page_index == GUIDE_PAGE_PUMP_MODE_SELECT) {
        guide_text_infos = pump_mode_select_text_infos;
    } else {
        guide_text_infos = bind_guide_text_infos;
        text_color = lv_color_hex(GUIDE_BIND_TEXT_COLOR);
    }

    for (int i = 0; i < GUIDE_TEXT_NUM; i++) {
        if (guide_info->text[i] == NULL || !lv_obj_is_valid(guide_info->text[i])) {
            guide_info->text[i] = lv_label_create(guide_info->root);
            EXCEPTION_CHECK_VOID(guide_info->text[i]);
        }
        lv_label_set_text(guide_info->text[i], guide_text_infos[i].text);
        lv_obj_set_style_text_color(guide_info->text[i], text_color, 0);
        lv_obj_set_style_text_font(guide_info->text[i], guide_text_infos[i].font, 0);  /// 设置字体
        lv_obj_set_style_text_line_space(guide_info->text[i], 4, 0);  /// 设置行间距（像素）
        lv_obj_set_style_text_align(guide_info->text[i], LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_set_style_text_opa(guide_info->text[i], guide_text_infos[i].text_opa, 0);  /// 设置文本透明度
        if (guide_text_infos[i].text_x == 0) {
            lv_obj_align(guide_info->text[i], LV_ALIGN_TOP_MID, 0, guide_text_infos[i].text_y);
        } else {
            lv_obj_align(guide_info->text[i], LV_ALIGN_TOP_LEFT, guide_text_infos[i].text_x, guide_text_infos[i].text_y);
        }
    }
}

static void guide_ui_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    if (page_index >= GUIDE_PAGE_IMG_NUM) {
        LOG_UI("===> guide page index out of range: %d", page_index);
        return;
    }

    if (guide_info->force_update == false && guide_info->page_index == page_index) {
#if IS_SUPPORT_GUIDE_COMP_BG
        guide_ui_page_img_update(thiz, page_index);
#endif
        return;
    }
    
    guide_ui_page_img_update(thiz, page_index);
    guide_qrcode_update(thiz, page_index);
    guide_ui_text_update(thiz, page_index);
    guide_info->page_index = page_index;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_guide_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (guide_info == NULL) {
        guide_info = (guide_display_info_t *)u_malloz(sizeof(guide_display_info_t));
        EXCEPTION_CHECK_VOID(guide_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_guide_root_init(); /// 初始化 root 容器
    guide_info->force_update = true;
    guide_info->popup_ui_index = GUIDE_POPUP_UI_NONE;
}

void ui_display_guide_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(guide_info);
    EXCEPTION_CHECK_VOID(guide_info->root);
    common_ui_img_resource_deinit(thiz, &guide_info->img_info);
    guide_tip_popup_ui_remove(thiz);
    LVGL_OBJ_DELETE(guide_info->root);
    u_free(guide_info);
    guide_info = NULL;
}

int ui_display_guide_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(guide_info);
    EXCEPTION_CHECK_INT(guide_info->root);
    ui_proc_guide_t *proc = (ui_proc_guide_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    guide_ui_update(thiz, proc->page_index);
    guide_tip_popup_ui_update(thiz, proc, guide_info->root, &guide_info->popup_ui_index);
    guide_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> guide fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
