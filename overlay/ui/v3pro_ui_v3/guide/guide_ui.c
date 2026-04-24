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
#if 0
#define GUIDE_QRCODE_SIZE                                       (124 - 4)
#define GUIDE_QRCODE_ALIGN_Y                                    (68)
#else
#define GUIDE_QRCODE_SIZE                                       (124)
#define GUIDE_QRCODE_ALIGN_Y                                    (68)
#endif
#define GUIDE_TEXT_NUM                                          (4)
#define GUIDE_ARROW_TEXT_X_OFFSET                               (0)
#define GUIDE_ARROW_TEXT_Y_OFFSET                               (316)
#define GUIDE_TEXT_COLOR                                        (0xB25962)
#define GUIDE_TITLE_TEXT_COLOR                                  (0xB25962)
#define GUIDE_DESC_TEXT_COLOR                                   (0xC5757C)
#define GUIDE_EXIT_TEXT_COLOR                                   (0xDCA7A5)
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
    lv_draw_buf_t *page_decoded_buf;
    uint8_t force_update;
    uint8_t page_index;
    char qr_str[120];
} guide_display_info_t;

typedef struct {
    const char *text;
    const lv_font_t *font;
    uint16_t text_x;
    uint16_t text_y;
    uint8_t line_space; /// 行间距
    uint32_t color;
} guide_text_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static guide_display_info_t *guide_info;

const static char *bind_qr_str[] = {
    "https://qr.momcozy.com/apps/?v=1&type=device&model=V3&sn=",
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
    {"Operation Guide", 
        &lv_font_oppo_sans_20_medium, 0, 51, 4, GUIDE_TITLE_TEXT_COLOR},
    {"Turn right to next page", 
        &lv_font_oppo_sans_16_medium, 0, 81, 4, GUIDE_DESC_TEXT_COLOR},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, 4, GUIDE_TEXT_COLOR},
    {"Press down to exit", &lv_font_oppo_sans_14_medium, 0, 
        GUIDE_ARROW_TEXT_Y_OFFSET, 4, GUIDE_EXIT_TEXT_COLOR}
};

/// 开始泵奶
const static guide_text_info_t start_pump_text_infos[GUIDE_TEXT_NUM] = {
    {"Press to enter/exit\nthe function", 
        &lv_font_oppo_sans_20_medium, 0, 51, 4, GUIDE_TITLE_TEXT_COLOR},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, 4, GUIDE_DESC_TEXT_COLOR},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, 4, GUIDE_TEXT_COLOR},
    {"Press down to exit", &lv_font_oppo_sans_14_medium, 0, 
        GUIDE_ARROW_TEXT_Y_OFFSET, 4, GUIDE_EXIT_TEXT_COLOR}
};

/// 设置泵奶
const static guide_text_info_t setting_pump_text_infos[GUIDE_TEXT_NUM] = {
    {"Turn to adjust the options\nPress down to confirm", 
        &lv_font_oppo_sans_20_medium, 0, 51, 4, GUIDE_TITLE_TEXT_COLOR},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, 4, GUIDE_DESC_TEXT_COLOR},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, 4, GUIDE_TEXT_COLOR},
    {"Press down to exit", &lv_font_oppo_sans_14_medium, 0,
        GUIDE_ARROW_TEXT_Y_OFFSET, 4, GUIDE_EXIT_TEXT_COLOR}
};

/// 停止泵奶
const static guide_text_info_t stop_pump_text_infos[GUIDE_TEXT_NUM] = {
    {"Press", &lv_font_oppo_sans_20_medium, 78, 51, 4, GUIDE_TITLE_TEXT_COLOR},
    {"quick start", &lv_font_oppo_sans_20_medium, 180, 51, 4, GUIDE_TITLE_TEXT_COLOR},
    {"default program", &lv_font_oppo_sans_20_medium, 0, 75, 4, GUIDE_TITLE_TEXT_COLOR},
    {"Press down to exit", &lv_font_oppo_sans_14_medium, 0, 
        GUIDE_ARROW_TEXT_Y_OFFSET, 4, GUIDE_EXIT_TEXT_COLOR}
};

/// 泵奶模式选择
const static guide_text_info_t pump_mode_select_text_infos[GUIDE_TEXT_NUM] = {
    {"Press and hold for 3 seconds\nto stop pumping", 
        &lv_font_oppo_sans_20_medium, 0, 125, 4, 0xFFFFFF},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, 4, GUIDE_DESC_TEXT_COLOR},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, 4, GUIDE_TEXT_COLOR},
    {"Press down to exit", &lv_font_oppo_sans_14_medium, 0, 
        GUIDE_ARROW_TEXT_Y_OFFSET, 4, 0x938383}
};

/// 绑定引导
const static guide_text_info_t bind_guide_text_infos[GUIDE_TEXT_NUM] = {
    {"Scan to connect for tracking, remote\ncontrol, and custom modes",
    &lv_font_oppo_sans_16_medium, 0, 212, 4, GUIDE_BIND_TEXT_COLOR},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, 4, GUIDE_BIND_TEXT_COLOR},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, 4, GUIDE_BIND_TEXT_COLOR},
    {"Press down to exit", &lv_font_oppo_sans_14_medium, 0, 
        GUIDE_ARROW_TEXT_Y_OFFSET, 4, GUIDE_TITLE_TEXT_COLOR}
};

#if IS_SUPPORT_GUIDE_COMP_BG
static PSRAM_DATA_LOC uint8_t guide_bg_data[] = {
    // #include "comparison_png2txt/guide_home_360x360.txt"
    // #include "comparison_png2txt/guide_start_pump_360x360.txt"
    // #include "comparison_png2txt/guide_setting_page_360x360.txt"
    // #include "comparison_png2txt/guide_stop_pump_360x360.txt"
    #include "comparison_png2txt/guide_mode_select_360x360.txt"
    // #include "comparison_png2txt/guide_bind_360x360.txt"
};
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_ui_page_img_set_from_sw_decode(lv_obj_t *obj, lv_draw_buf_t **page_decoded_buf,
                                                 const uint8_t *png_data, size_t png_size)
{
    common_ui_png_decode_result_t dec;
    int err = common_ui_png_decode(png_data, png_size, COMMON_UI_PNG_DECODE_RGBA8888, &dec);
    if (err != 0) {
        LOG_UI("guide png sw decode failed: %d", err);
        if (*page_decoded_buf != NULL) {
            lv_image_set_src(obj, NULL);
            common_ui_png_decode_release(*page_decoded_buf);
            *page_decoded_buf = NULL;
        }
        lv_fs_path_ex_t page_img_path;
        lv_fs_make_path_from_buffer(&page_img_path, LV_FS_MEMFS_LETTER, png_data, png_size, "png");
        lv_image_set_src(obj, (const void *)&page_img_path);
        return;
    }
    lv_draw_buf_t *old_buf = *page_decoded_buf;
    *page_decoded_buf = dec.draw_buf;
    lv_image_set_src(obj, dec.draw_buf);
    if (old_buf != NULL) {
        common_ui_png_decode_release(old_buf);
    }
}

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
    lv_qrcode_set_size(guide_info->qrcode, GUIDE_QRCODE_SIZE);
    lv_qrcode_set_dark_color(guide_info->qrcode, lv_color_hex(GUIDE_QRCODE_FG_COLOR));
    lv_qrcode_set_light_color(guide_info->qrcode, lv_color_hex(GUIDE_QRCODE_BG_COLOR));
    lv_qrcode_update(guide_info->qrcode, guide_info->qr_str, strlen(guide_info->qr_str));
    lv_obj_align(guide_info->qrcode, LV_ALIGN_TOP_MID, 0, GUIDE_QRCODE_ALIGN_Y);
}

static void guide_ui_page_img_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    if (page_index >= GUIDE_PAGE_IMG_NUM) {
        LOG_UI("===> guide page index out of range: %d", page_index);
        return;
    }

    if (guide_info->page_img == NULL || !lv_obj_is_valid(guide_info->page_img)) {
        guide_info->page_img = lv_image_create(guide_info->root);
        EXCEPTION_CHECK_VOID(guide_info->page_img);
        lv_obj_align(guide_info->page_img, LV_ALIGN_CENTER, 0, 0);
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
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
#else
    common_ui_img_resource_init(thiz, guide_page_img_ids[page_index], &guide_info->img_info);
    EXCEPTION_CHECK_VOID(guide_info->img_info.data);
    guide_ui_page_img_set_from_sw_decode(guide_info->page_img, &guide_info->page_decoded_buf,
                                         guide_info->img_info.data, guide_info->img_info.size);
#endif
}

static void guide_ui_text_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    const guide_text_info_t *guide_text_infos = NULL;
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
    }

    for (int i = 0; i < GUIDE_TEXT_NUM; i++) {
        if (guide_info->text[i] == NULL || !lv_obj_is_valid(guide_info->text[i])) {
            guide_info->text[i] = lv_label_create(guide_info->root);
            EXCEPTION_CHECK_VOID(guide_info->text[i]);
        }
        lv_label_set_text(guide_info->text[i], guide_text_infos[i].text);
        lv_obj_set_style_text_color(guide_info->text[i], lv_color_hex(guide_text_infos[i].color), 0);
        lv_obj_set_style_text_font(guide_info->text[i], guide_text_infos[i].font, 0);  /// 设置字体
        lv_obj_set_style_text_line_space(guide_info->text[i], 4, 0);  /// 设置行间距（像素）
        lv_obj_set_style_text_align(guide_info->text[i], LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_set_style_text_opa(guide_info->text[i], LV_OPA_COVER, 0);  /// 设置文本透明度
        if (guide_text_infos[i].text_x == 0) {
            lv_obj_align(guide_info->text[i], LV_ALIGN_TOP_MID, 0, guide_text_infos[i].text_y);
        } else {
            lv_obj_align(guide_info->text[i], LV_ALIGN_TOP_LEFT, guide_text_infos[i].text_x, guide_text_infos[i].text_y);
        }
    }
}

static void guide_ui_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    // page_index =  3; /// 测试用
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
    storage_manager_context_t *storage = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (storage) {
        factory_data_store_t *factory_data = storage->get_factory_data();
        factory_dev_info_t *dev_info = &factory_data->dev_info;
        unsigned int sn_n = dev_info->sn_len;
        if (sn_n > DEVICE_UNIQUE_SN_LEN) {
            sn_n = DEVICE_UNIQUE_SN_LEN;
        }
        snprintf(guide_info->qr_str, sizeof(guide_info->qr_str), "%s%.*s", bind_qr_str[0],
                 (int)sn_n, (const char *)dev_info->sn);
        LOG_UI("%s", guide_info->qr_str);
    } else {
        snprintf(guide_info->qr_str, sizeof(guide_info->qr_str), "%s", bind_qr_str[0]);
        LOG_UI("%s", guide_info->qr_str);
    }
}

void ui_display_guide_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && guide_info && guide_info->root);
#if !IS_SUPPORT_GUIDE_COMP_BG
    if (guide_info->page_decoded_buf != NULL) {
        if (guide_info->page_img != NULL && lv_obj_is_valid(guide_info->page_img)) {
            lv_image_set_src(guide_info->page_img, NULL);
        }
        common_ui_png_decode_release(guide_info->page_decoded_buf);
        guide_info->page_decoded_buf = NULL;
    }
#endif
    common_ui_img_resource_deinit(thiz, &guide_info->img_info);
    LVGL_OBJ_DELETE(guide_info->root);
    u_free(guide_info);
    guide_info = NULL;
}

int ui_display_guide_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && guide_info && guide_info->root);
    ui_proc_guide_t *proc = (ui_proc_guide_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    guide_ui_update(thiz, proc->page_index);
    guide_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> guide fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
