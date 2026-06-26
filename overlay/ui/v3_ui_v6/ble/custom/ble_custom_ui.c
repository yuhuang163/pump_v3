/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ble_custom_ui_internal.h" // IWYU pragma: keep
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ble_custom_ui_info_t *ble_info;
static bool connect_prompt_flag = true;  /// 连接提示标志 true-显示 false-隐藏

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static bool ble_custom_ui_blink_visible_from_time(uint32_t blink_time)
{
    return ((get_timestamp() - blink_time) / BLE_UI_BLINK_PERIOD) % 2U == 0U;
}

static void ble_custom_ui_icon_visible_set(lv_obj_t *ble_img, bool visible)
{
    if (visible) {
        lv_obj_clear_flag(ble_img, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(ble_img, LV_OBJ_FLAG_HIDDEN);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ble_custom_ui_init(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (ble_info == NULL) {
        ble_info = (ble_custom_ui_info_t *)u_malloz(sizeof(ble_custom_ui_info_t));
        EXCEPTION_CHECK_VOID(ble_info);
    }
    ble_info->fps_start_time = 0;
    ble_info->is_show_ble = 0;
    ble_info->force_update = true;
}

int ble_custom_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, ble_manager_context_t *ble,
    uint16_t x, uint16_t y, const ble_ui_blink_sync_t *sync)
{
    EXCEPTION_CHECK_INT(thiz && root && ble);
    uint8_t is_connected = ble->ble_is_connected();
    uint32_t time_ms = get_timestamp();
    if (ble_info->is_connected == is_connected && ble_info->force_update == false) {
        EXCEPTION_CHECK_INT(ble_info->ble_img);
        if (time_ms >= 30000 || is_connected > 0 || connect_prompt_flag == false) {
            connect_prompt_flag = false;
            lv_obj_clear_flag(ble_info->ble_img, LV_OBJ_FLAG_HIDDEN); /// 显示
            return 0;
        }

        if (sync != NULL && sync->sync_enable != 0) {
            ble_custom_ui_icon_visible_set(ble_info->ble_img,
                ble_custom_ui_blink_visible_from_time(sync->blink_time));
            return 0;
        }

        if ((time_ms - ble_info->fps_start_time) < BLE_UI_BLINK_PERIOD) {
            return 0;
        }
        ble_info->fps_start_time = get_timestamp(); /// 更新开始时间
        if (ble_info->is_show_ble) {
            ble_info->is_show_ble = 0;
            lv_obj_clear_flag(ble_info->ble_img, LV_OBJ_FLAG_HIDDEN); /// 显示
        } else {
            ble_info->is_show_ble = 1;
            lv_obj_add_flag(ble_info->ble_img, LV_OBJ_FLAG_HIDDEN); /// 隐藏
        }
        return 0;
    }
    uint16_t resource_id = is_connected > 0 ? BASE_RES_ID_BLE_CONNECT_ICON : BASE_RES_ID_BLE_DISCONNECT_ICON;
    if (ble_info->ble_img == NULL || !lv_obj_is_valid(ble_info->ble_img)) {
        ble_info->ble_img = lv_image_create(root);
        EXCEPTION_CHECK_INT(ble_info->ble_img);
    }
    ui_img_baj_theme_set_src(thiz, ble_info->ble_img, resource_id,
        RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(ble_info->ble_img, LV_ALIGN_TOP_LEFT, x, y);
    ble_info->is_connected = is_connected;
    ble_info->force_update = false;
    ble_info->fps_start_time = get_timestamp();
    ble_info->is_show_ble = 1;
    return 0;
}

void ble_custom_ui_deinit(ui_manager_context_t *thiz)
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
