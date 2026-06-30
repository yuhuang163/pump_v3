/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ble_ui.h" // IWYU pragma: keep
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
typedef struct {
    lv_obj_t *ble_img;
    uint32_t fps_start_time;
    uint8_t is_show_ble;
    uint8_t is_connected;
    uint8_t force_update;
    uint8_t rsv[14];
} ble_ui_info_t;

static ble_ui_info_t *ble_info;
static bool connect_prompt_flag = true;  /// 连接提示标志 true-显示 false-隐藏

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ble_ui_icon_visible_set(lv_obj_t *ble_img, bool visible)
{
    if (visible) {
        lv_obj_clear_flag(ble_img, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(ble_img, LV_OBJ_FLAG_HIDDEN);
    }
}

static bool ble_ui_connect_prompt_blinking(uint32_t time_ms, uint8_t is_connected)
{
    return (time_ms < BLE_UI_CONNECT_PROMPT_TIMEOUT_MS && is_connected == 0 && connect_prompt_flag);
}

static uint32_t ble_ui_blink_time_resolve(const ble_ui_blink_sync_t *sync)
{
    if (sync != NULL && sync->sync_enable != 0) {
        return sync->blink_time;
    }
    return ble_info->fps_start_time;
}

static int ble_ui_steady_update(uint32_t time_ms, uint8_t is_connected,
    const ble_ui_blink_sync_t *sync)
{
    EXCEPTION_CHECK_INT(ble_info->ble_img);
    if (!ble_ui_connect_prompt_blinking(time_ms, is_connected)) {
        connect_prompt_flag = false;
        ble_ui_icon_visible_set(ble_info->ble_img, true);
        return 0;
    }

    ble_ui_icon_visible_set(ble_info->ble_img,
        BLE_UI_BLINK_VISIBLE(ble_ui_blink_time_resolve(sync)));
    return 0;
}

static void ble_ui_resource_update(ui_manager_context_t *thiz, lv_obj_t *root,
    uint8_t is_connected, uint16_t x, uint16_t y, uint32_t time_ms)
{
    const uint16_t resource_id = is_connected > 0 ?
        BASE_RES_ID_BLE_CONNECT_ICON : BASE_RES_ID_BLE_DISCONNECT_ICON;

    if (ble_info->ble_img == NULL || !lv_obj_is_valid(ble_info->ble_img)) {
        ble_info->ble_img = lv_image_create(root);
    }
    ui_img_baj_theme_set_src(thiz, ble_info->ble_img, resource_id,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    if (x == 0) {
        lv_obj_align(ble_info->ble_img, LV_ALIGN_TOP_MID, 0, y);
    } else {
        lv_obj_align(ble_info->ble_img, LV_ALIGN_TOP_LEFT, x, y);
    }
    ble_info->is_connected = is_connected;
    ble_info->force_update = false;
    ble_info->fps_start_time = time_ms;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ble_ui_init(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (ble_info == NULL) {
        ble_info = (ble_ui_info_t *)u_malloz(sizeof(ble_ui_info_t));
        EXCEPTION_CHECK_VOID(ble_info);
    }
    ble_info->fps_start_time = 0;
    ble_info->force_update = true;
}

int ble_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, ble_manager_context_t *ble,
    uint16_t x, uint16_t y, const ble_ui_blink_sync_t *sync)
{
    EXCEPTION_CHECK_INT(thiz && root && ble);
    const uint8_t is_connected = ble->ble_is_connected();
    const uint32_t time_ms = get_timestamp();

    if (ble_info->is_connected == is_connected && ble_info->force_update == false) {
        return ble_ui_steady_update(time_ms, is_connected, sync);
    }

    ble_ui_resource_update(thiz, root, is_connected, x, y, time_ms);
    EXCEPTION_CHECK_INT(ble_info->ble_img);
    return 0;
}

void ble_ui_deinit(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (ble_info == NULL) {
        return;
    }
    ui_img_baj_release_decoded_buf(ble_info->ble_img);
    ble_info->ble_img = NULL;
    ble_info->is_connected = 0;
    ble_info->force_update = false;
    u_free(ble_info);
    ble_info = NULL;
}
