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

#define HOME_SCREEN_SAVER_MID_GRAY_RING_SIZE                     (248)
#define HOME_SCREEN_SAVER_INNER_GRAY_RING_SIZE                   (160)
#define HOME_SCREEN_SAVER_MID_GRAY_RING_COLOR                    (0x131212)
#define HOME_SCREEN_SAVER_INNER_GRAY_RING_COLOR                  (0x1D1C1C)
typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_ROOT_HOME_SCREEN_SAVER_COMP_BG
    lv_obj_t * backgroud;
#endif
    lv_obj_t * black_ring;
    lv_obj_t * mid_gray_ring;
    lv_obj_t * inner_gray_ring;
    lv_obj_t * saver_icon;
    uint8_t force_update;
    uint8_t rsv[15];
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
static lv_obj_t *home_screen_saver_circle_create(lv_obj_t *parent, lv_coord_t size, uint32_t color)
{
    lv_obj_t *circle = lv_obj_create(parent);

    if (circle == NULL) {
        return NULL;
    }
    lv_obj_remove_style_all(circle);
    lv_obj_set_size(circle, size, size);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(circle, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(circle, 0, 0);
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(circle, LV_ALIGN_CENTER, 0, 0);
    return circle;
}

static void home_screen_saver_root_init(void)
{
    if (saver_info->root == NULL || !lv_obj_is_valid(saver_info->root)) {
        saver_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(saver_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(saver_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(saver_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(saver_info->root, 0, 0);
    lv_obj_set_style_pad_all(saver_info->root, 0, 0);
    lv_obj_align(saver_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(saver_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(saver_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void home_screen_saver_rings_init(void)
{
    if (saver_info->mid_gray_ring == NULL || !lv_obj_is_valid(saver_info->mid_gray_ring)) {
        saver_info->mid_gray_ring = home_screen_saver_circle_create(saver_info->root,
            HOME_SCREEN_SAVER_MID_GRAY_RING_SIZE, HOME_SCREEN_SAVER_MID_GRAY_RING_COLOR);
        EXCEPTION_CHECK_VOID(saver_info->mid_gray_ring);
    }
    if (saver_info->inner_gray_ring == NULL || !lv_obj_is_valid(saver_info->inner_gray_ring)) {
        saver_info->inner_gray_ring = home_screen_saver_circle_create(saver_info->root,
            HOME_SCREEN_SAVER_INNER_GRAY_RING_SIZE, HOME_SCREEN_SAVER_INNER_GRAY_RING_COLOR);
        EXCEPTION_CHECK_VOID(saver_info->inner_gray_ring);
    }
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
    (void)thiz;
}
#endif

static void home_screen_saver_icon_prepare(ui_manager_context_t *thiz)
{
    if (saver_info->saver_icon == NULL || !lv_obj_is_valid(saver_info->saver_icon)) {
        saver_info->saver_icon = lv_image_create(saver_info->root);
        EXCEPTION_CHECK_VOID(saver_info->saver_icon);
    }
    ui_img_baj_theme_set_src(thiz, saver_info->saver_icon, BASE_RES_ID_HOME_IDLE_ICON,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(saver_info->saver_icon, LV_ALIGN_CENTER, 0, 0);
}

static void home_screen_saver_ui_update(ui_manager_context_t *thiz, ui_proc_home_t *proc)
{
    (void)proc;
    if (saver_info->force_update == false) {
        return;
    }
    home_screen_saver_icon_prepare(thiz);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_screen_saver_base_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (saver_info == NULL) {
        saver_info = (home_screen_saver_display_info_t *)u_malloz(sizeof(home_screen_saver_display_info_t));
        EXCEPTION_CHECK_VOID(saver_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    home_screen_saver_root_init();
    home_screen_saver_rings_init();
    ble_ui_init(thiz);
    battery_ui_init(thiz);
    saver_info->force_update = true;
    home_screen_saver_icon_prepare(thiz);
    saver_info->force_update = false;
}

void home_screen_saver_base_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && saver_info);
    ui_img_baj_release_decoded_buf(saver_info->saver_icon);
    ble_ui_deinit(thiz);
    battery_ui_deinit(thiz);
    LVGL_OBJ_DELETE(saver_info->root);
    u_free(saver_info);
    saver_info = NULL;
}

int home_screen_saver_base_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && saver_info && saver_info->root);
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->ble);
    home_screen_saver_root_bg_update(thiz);
    battery_ui_update(thiz, saver_info->root, proc->battery, true);
    ble_ui_update(thiz, saver_info->root, proc->ble,
        BASE_PUMP_HOME_BLE_UI_X, BASE_PUMP_HOME_BLE_UI_Y, NULL);
    home_screen_saver_ui_update(thiz, proc);
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home screen saver fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
