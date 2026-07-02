/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static guide_base_ui_info_t *guide_info;

const static char *bind_qr_str[] = {
    "https://qr.momcozy.com/apps/?v=1&type=device&model=V3&sn=",
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_base_root_init(void)
{
    if (guide_info->root == NULL || !lv_obj_is_valid(guide_info->root)) {
        guide_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(guide_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(guide_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(guide_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(guide_info->root, 0, 0);
    lv_obj_set_style_pad_all(guide_info->root, 0, 0);
    lv_obj_align(guide_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(guide_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(guide_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void guide_base_qr_str_init(void)
{
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

static void guide_base_text_num_init(void)
{
    resource_manager_context_t *res = (resource_manager_context_t *)find_module(ID_MODULE_RES);

    guide_info->text_num = GUIDE_TEXT_NUM;
    if (res) {
        uint16_t img_res_version = res->get_img_res_version(RES_THEME_TYPE_BASE);
        LOG_UI("===> guide img res version: %u", (unsigned)img_res_version);
        if (img_res_version == 102) {
            guide_info->text_num = 4;
        }
    }
}

static void guide_base_ui_update(ui_manager_context_t *thiz, uint8_t page_index)
{
    if (page_index >= GUIDE_PAGE_IMG_NUM) {
        LOG_UI("===> guide page index out of range: %d", page_index);
        return;
    }

    if (guide_info->force_update == false && guide_info->page_index == page_index) {
#if IS_SUPPORT_GUIDE_COMP_BG
        guide_base_page_ui_update(thiz, page_index);
#endif
        return;
    }

    guide_base_page_ui_update(thiz, page_index);
    guide_base_qrcode_ui_update(thiz, page_index);
    guide_base_text_ui_update(thiz, page_index);
    guide_info->page_index = page_index;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
guide_base_ui_info_t *guide_base_info_get(void)
{
    return guide_info;
}

void guide_base_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (guide_info == NULL) {
        guide_info = (guide_base_ui_info_t *)u_malloz(sizeof(guide_base_ui_info_t));
        EXCEPTION_CHECK_VOID(guide_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    guide_base_root_init();
    guide_info->force_update = true;
    guide_base_qr_str_init();
    guide_base_text_num_init();
}

void guide_base_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && guide_info && guide_info->root);
#if !IS_SUPPORT_GUIDE_COMP_BG
    ui_img_png_release_decoded_buf(guide_info->page_img);
#endif
    ui_img_resource_deinit(thiz, &guide_info->img_info);
    LVGL_OBJ_DELETE(guide_info->root);
    u_free(guide_info);
    guide_info = NULL;
}

int guide_base_ui_process(ui_manager_context_t *thiz)
{
    ui_proc_guide_t *proc = (ui_proc_guide_t *)thiz->priv.play_info.user_data;

    EXCEPTION_CHECK_INT(thiz && guide_info && guide_info->root);
    EXCEPTION_CHECK_INT(proc);
    guide_base_ui_update(thiz, proc->page_index);
    guide_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> guide fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
