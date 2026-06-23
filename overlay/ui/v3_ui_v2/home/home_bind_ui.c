/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_home.h"   // IWYU pragma: keep
#include "home_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_HOME_BIND_COMP_BG                         (0)
#define HOME_BIND_BG_COLOR                                   0xFFFFFF
#define HOME_BIND_FG_COLOR                                   0x000000
#define HOME_BIND_BORDER_COLOR                               (0xFFEAE6)  // 0xA6A6A6
#define HOME_BIND_BORDER_RADIUS                              (4)
#define HOME_BIND_TEXT_COLOR                                 (0x6A5A5B)
#define HOME_BIND_X_OFFSET                                   (111)
#define HOME_BIND_Y_OFFSET                                   (69)
#define HOME_BIND_TEXT_Y_OFFSET                              (223)
#define HOME_BIND_IP_X_OFFSET                                (84)
#define HOME_BIND_IP_Y_OFFSET                                (95)
#define HOME_BIND_LABEL_X_OFFSET                             (12)
#define HOME_BIND_LABEL_Y_OFFSET                             (111)

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_HOME_BIND_COMP_BG
    lv_obj_t * backgroud;
#endif
    lv_obj_t * qrcode_container;
    lv_obj_t * qrcode;
    lv_obj_t * text[2];
    lv_obj_t * label_icon;
    res_img_info_t label_info;
    lv_obj_t * ip_icon;
    res_img_info_t ip_info;
    uint8_t force_update;
    uint8_t popup_ui_index;
    uint8_t bind_state;
    uint8_t rsv[13];
    char qr_str[120];
} home_bind_display_info_t;

typedef struct {
    const char *text;
    const lv_font_t *font;
    uint16_t text_x;
    uint16_t text_y;
    uint32_t text_color;
} home_bind_text_info_t;

typedef enum {
    HOME_BIND_AND_DISCONNECT = 0, /// 绑定并断开状态
    HOME_BIND_AND_CONNECTED, /// 绑定并连接状态
    HOME_BIND_AND_CONNECTING, /// 绑定并连接中状态
    HOME_UNBIND_STATE, /// 未绑定状态
    HOME_BIND_STATE_MAX, /// 绑定状态最大值
} home_device_bind_state_e;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static home_bind_display_info_t *home_bind_info;
const static char *bind_qr_str[] = {
    "https://qr.momcozy.com/apps/?v=1&type=device&model=V3&sn=",
};

#if IS_SUPPORT_HOME_BIND_COMP_BG
static PSRAM_DATA_LOC uint8_t home_bg_data[] = {
    // #include "comparison_png2txt/home_ble_cmp_360x360.txt"
    // #include "comparison_png2txt/home_ble_connected_360x360.txt"
    #include "comparison_png2txt/home_ble_connecting_360x360.txt"
};
#endif

const static home_bind_text_info_t bind_text_infos[2] = {
    {"Scan to connect to the app",
        &lv_font_oppo_sans_20_semibold, 0, HOME_BIND_TEXT_Y_OFFSET, 0x423032},
    {"Smart pumping, milk tracking, easy\nremote control", 
        &lv_font_oppo_sans_14_medium, 0, 256, HOME_BIND_TEXT_COLOR}
};

const static home_bind_text_info_t bind_disconnect_infos[2] = {
    {"Disconnect", &lv_font_oppo_sans_14_medium, 80, 143, 0xB25962},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, HOME_BIND_TEXT_COLOR}
};

const static home_bind_text_info_t bind_connecting_infos[2] = {
    {"Connecting", &lv_font_oppo_sans_14_medium, 80, 143, 0xB25962},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, HOME_BIND_TEXT_COLOR}
};

const static home_bind_text_info_t bind_connected_infos[2] = {
    {"Device connected", &lv_font_oppo_sans_14_medium, 60, 143, 0xB25962},
    {"", &lv_font_oppo_sans_14_medium, 0, 0, HOME_BIND_TEXT_COLOR}
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_home_bind_root_init(void)
{
    if (home_bind_info->root == NULL || !lv_obj_is_valid(home_bind_info->root)) {
        home_bind_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(home_bind_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(home_bind_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    // lv_obj_set_style_bg_color(home_bind_info->root, lv_color_white(), 0);     /// 设置背景颜色为白色
    lv_obj_set_style_bg_opa(home_bind_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(home_bind_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(home_bind_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(home_bind_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(home_bind_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(home_bind_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

#if IS_SUPPORT_HOME_BIND_COMP_BG
static void ui_home_bind_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (home_bind_info->backgroud == NULL || !lv_obj_is_valid(home_bind_info->backgroud)) {
        home_bind_info->backgroud = lv_image_create(home_bind_info->root);
        EXCEPTION_CHECK_VOID(home_bind_info->backgroud);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(home_bind_info->backgroud, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(home_bind_info->backgroud, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, home_bg_data,
            sizeof(home_bg_data), "png");
        lv_image_set_src(home_bind_info->backgroud, (const void *)&bg_img_path);
        lv_obj_align(home_bind_info->backgroud, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void ui_home_bind_bg_update(ui_manager_context_t *thiz)
{
    common_ui_bg_update(thiz, home_bind_info->root);
}
#endif

static void home_bind_qrcode_update(ui_manager_context_t *thiz, uint8_t is_show)
{  
    ///  创建容器包裹二维码（用于添加边框和内边距效果）
    if (home_bind_info->qrcode_container == NULL || !lv_obj_is_valid(home_bind_info->qrcode_container)) {
        home_bind_info->qrcode_container = lv_obj_create(home_bind_info->root);
        EXCEPTION_CHECK_VOID(home_bind_info->qrcode_container);
        ///  设置容器尺寸
        lv_obj_set_size(home_bind_info->qrcode_container, 138, 138);
        ///  设置容器背景颜色为白色
        lv_obj_set_style_bg_color(home_bind_info->qrcode_container, lv_color_hex(HOME_BIND_BG_COLOR), 0);
        lv_obj_set_style_bg_opa(home_bind_info->qrcode_container, LV_OPA_0, 0); /// 设置容器背景透明
        ///  设置容器边框样式（灰色边框，宽度2像素）
        lv_obj_set_style_border_color(home_bind_info->qrcode_container, lv_color_hex(HOME_BIND_BORDER_COLOR), 0);
        lv_obj_set_style_border_width(home_bind_info->qrcode_container, 2, 0);
        lv_obj_set_style_radius(home_bind_info->qrcode_container, HOME_BIND_BORDER_RADIUS, 0);
        ///  设置内边距为4像素，让边框和二维码内容之间有白色间距
        lv_obj_set_style_pad_all(home_bind_info->qrcode_container, 10, 0);
        ///  禁用容器的滚动功能
        lv_obj_clear_flag(home_bind_info->qrcode_container, LV_OBJ_FLAG_SCROLLABLE);
    } 
    
    if (home_bind_info->qrcode == NULL || !lv_obj_is_valid(home_bind_info->qrcode)) {
        home_bind_info->qrcode = lv_qrcode_create(home_bind_info->qrcode_container);
        EXCEPTION_CHECK_VOID(home_bind_info->qrcode);
    } 
    
    if (is_show) {
        lv_obj_clear_flag(home_bind_info->qrcode_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(home_bind_info->qrcode, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(home_bind_info->qrcode_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(home_bind_info->qrcode, LV_OBJ_FLAG_HIDDEN);
        return;
    }   

    lv_qrcode_set_size(home_bind_info->qrcode, 124);
    lv_qrcode_set_dark_color(home_bind_info->qrcode, lv_color_hex(HOME_BIND_FG_COLOR));
    lv_qrcode_set_light_color(home_bind_info->qrcode, lv_color_hex(HOME_BIND_BG_COLOR));
    lv_qrcode_update(home_bind_info->qrcode, home_bind_info->qr_str, strlen(home_bind_info->qr_str));
    lv_obj_center(home_bind_info->qrcode);
    lv_obj_align(home_bind_info->qrcode_container, LV_ALIGN_TOP_LEFT, 
        HOME_BIND_X_OFFSET, HOME_BIND_Y_OFFSET);
}

static void home_bind_text_update(ui_manager_context_t *thiz, const home_bind_text_info_t *text_info)
{
    EXCEPTION_CHECK_VOID(thiz && text_info);
    for (int i = 0; i < 2; i++) {
        if (home_bind_info->text[i] == NULL || !lv_obj_is_valid(home_bind_info->text[i])) {
            home_bind_info->text[i] = lv_label_create(home_bind_info->root);
            EXCEPTION_CHECK_VOID(home_bind_info->text[i]);
        }
        lv_label_set_text(home_bind_info->text[i], text_info[i].text);
        lv_obj_set_style_text_color(home_bind_info->text[i], lv_color_hex(text_info[i].text_color), 0);
        lv_obj_set_style_text_font(home_bind_info->text[i], text_info[i].font, 0);
        lv_obj_set_style_text_align(home_bind_info->text[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_line_space(home_bind_info->text[i], 4, 0);
        lv_obj_set_style_text_opa(home_bind_info->text[i], LV_OPA_COVER, 0);
        if (text_info[i].text_x == 0) {
            lv_obj_align(home_bind_info->text[i], LV_ALIGN_TOP_MID, 0, text_info[i].text_y);
        } else {
            lv_obj_align(home_bind_info->text[i], LV_ALIGN_TOP_LEFT, text_info[i].text_x, text_info[i].text_y);
        }
    }
}

static void home_bind_img_update(ui_manager_context_t *thiz, uint8_t is_show)
{
    if (home_bind_info->ip_icon == NULL || !lv_obj_is_valid(home_bind_info->ip_icon)) {
        home_bind_info->ip_icon = lv_image_create(home_bind_info->root);
        EXCEPTION_CHECK_VOID(home_bind_info->ip_icon);
        lv_obj_align(home_bind_info->ip_icon, LV_ALIGN_TOP_LEFT, HOME_BIND_IP_X_OFFSET, HOME_BIND_IP_Y_OFFSET);
    }

    if (home_bind_info->label_icon == NULL || !lv_obj_is_valid(home_bind_info->label_icon)) {
        home_bind_info->label_icon = lv_image_create(home_bind_info->root);
        EXCEPTION_CHECK_VOID(home_bind_info->label_icon);
        lv_obj_align(home_bind_info->label_icon, LV_ALIGN_TOP_LEFT, HOME_BIND_LABEL_X_OFFSET, HOME_BIND_LABEL_Y_OFFSET);
    }

    if (is_show) {
        lv_obj_clear_flag(home_bind_info->ip_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(home_bind_info->label_icon, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(home_bind_info->ip_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(home_bind_info->label_icon, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    common_ui_img_resource_init_with_theme(thiz, BASE_RES_ID_HOME_BLE_IP, &home_bind_info->ip_info);
    EXCEPTION_CHECK_VOID(home_bind_info->ip_info.data);
    lv_fs_path_ex_t ip_img_path;
    lv_fs_make_path_from_buffer(&ip_img_path, LV_FS_MEMFS_LETTER, home_bind_info->ip_info.data,
        home_bind_info->ip_info.size, "baj");
    lv_image_set_src(home_bind_info->ip_icon, (const void *)&ip_img_path);
    common_ui_img_resource_init(thiz, BASE_RES_ID_HOME_BLE_LABEL, &home_bind_info->label_info);
    EXCEPTION_CHECK_VOID(home_bind_info->label_info.data);
    lv_fs_path_ex_t label_img_path;
    lv_fs_make_path_from_buffer(&label_img_path, LV_FS_MEMFS_LETTER, home_bind_info->label_info.data,
        home_bind_info->label_info.size, "baj");
    lv_image_set_src(home_bind_info->label_icon, (const void *)&label_img_path);
}

static void home_bind_status_ui_update(ui_manager_context_t *thiz, ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(thiz && proc && proc->ble);
#if 0
    int bind_state = proc->ble->ble_bind_state_get();
    int ble_state = proc->ble->ble_is_connected();
    uint8_t device_bind_state = HOME_UNBIND_STATE;
    if (bind_state == BLE_BIND_STATE_COMPLETE) {
        device_bind_state = ble_state > 0 ? HOME_BIND_AND_CONNECTED : HOME_BIND_AND_DISCONNECT;
    } else if (bind_state == BLE_BIND_STATE_KEY_GENERATE) {
        device_bind_state = HOME_BIND_AND_CONNECTING;
    } else {
        device_bind_state = HOME_UNBIND_STATE;
    }
#else
    uint8_t device_bind_state = HOME_UNBIND_STATE;
    if (proc->ble->ble_is_connected()) {
        device_bind_state = HOME_BIND_AND_CONNECTED;
    } else {
        device_bind_state = HOME_UNBIND_STATE;
    }
#endif
    if (home_bind_info->bind_state == device_bind_state &&
        home_bind_info->force_update == false) {
        return;
    }
    home_bind_img_update(thiz, device_bind_state != HOME_UNBIND_STATE ? true : false);
    home_bind_qrcode_update(thiz, device_bind_state == HOME_UNBIND_STATE ? true : false);
    if (device_bind_state == HOME_BIND_AND_CONNECTED) {
        home_bind_text_update(thiz, bind_connected_infos);
    } else if (device_bind_state == HOME_BIND_AND_DISCONNECT) {
        home_bind_text_update(thiz, bind_disconnect_infos);
    } else if (device_bind_state == HOME_BIND_AND_CONNECTING) {
        home_bind_text_update(thiz, bind_connecting_infos);
    } else {
        home_bind_text_update(thiz, bind_text_infos);
    }
    home_bind_info->bind_state = device_bind_state;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_home_bind_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (home_bind_info == NULL) {
        home_bind_info = (home_bind_display_info_t *)u_malloz(sizeof(home_bind_display_info_t));
        EXCEPTION_CHECK_VOID(home_bind_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_home_bind_root_init(); /// 初始化 root 容器
    common_ui_bg_init(thiz);
    home_bind_info->force_update = true;
    home_bind_info->popup_ui_index = ID_POPUP_UI_NONE;
    storage_manager_context_t *storage = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (storage) {
        factory_data_store_t *factory_data = storage->get_factory_data();
        factory_dev_info_t *dev_info = &factory_data->dev_info;
        unsigned int sn_n = dev_info->sn_len;
        if (sn_n > DEVICE_UNIQUE_SN_LEN) {
            sn_n = DEVICE_UNIQUE_SN_LEN;
        }
        snprintf(home_bind_info->qr_str, sizeof(home_bind_info->qr_str), "%s%.*s", bind_qr_str[0],
                 (int)sn_n, (const char *)dev_info->sn);
        LOG_UI("%s", home_bind_info->qr_str);
    } else {
        snprintf(home_bind_info->qr_str, sizeof(home_bind_info->qr_str), "%s", bind_qr_str[0]);
        LOG_UI("%s", home_bind_info->qr_str);
    }
}

void ui_display_home_bind_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && home_bind_info && home_bind_info->root);
    common_ui_bg_deinit(thiz);
    common_ui_img_resource_deinit(thiz, &home_bind_info->label_info);
    common_ui_img_resource_deinit(thiz, &home_bind_info->ip_info);
    ui_common_operation_tip_remove(thiz);
    LVGL_OBJ_DELETE(home_bind_info->root);
    u_free(home_bind_info);
    home_bind_info = NULL;
}

int ui_display_home_bind_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && home_bind_info && home_bind_info->root);
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    ui_home_bind_bg_update(thiz);
    home_bind_status_ui_update(thiz, proc);
    ui_common_operation_tip_update(thiz, proc->popup_ui_type, home_bind_info->root,
        &home_bind_info->popup_ui_index);
    home_bind_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home bind fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    
    return 0;
}