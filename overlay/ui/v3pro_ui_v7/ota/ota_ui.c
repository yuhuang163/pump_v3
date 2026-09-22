/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ui_progress_arc_component.h" // IWYU pragma: keep
#include "window_screen_ota.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_OTA_COMP_BG                      (0)
#if IS_SUPPORT_OTA_COMP_BG
#define IS_SUPPORT_OTA_TEST                         (1)
#else
#define IS_SUPPORT_OTA_TEST                         (0)
#endif
#define OTA_DESC_TEXT_NUM                           (2)
#define OTA_ROOT_BG_COLOR                           (0x000000)
#define OTA_TITLE_TEXT_COLOR                        (0xFFFFFF)
#define OTA_PERCENT_TEXT_COLOR                      (0xFFFFFF)
#define OTA_SIZE_TEXT_COLOR                         (0x999999)
#define OTA_PROGRESS_DONE_COLOR                     (0xFFFFFF)
#define OTA_PROGRESS_REMAIN_COLOR                   (0x333232)
/** 进度弧匀速追赶目标的角速度（度/秒），适配硬件 OTA 稀疏大步进上报 */
#define OTA_PROGRESS_ARC_SPEED_DEG_PER_SEC          (90)
#define OTA_PROGRESS_ARC_ELAPSED_CLAMP_MS           (40)

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_OTA_COMP_BG
    lv_obj_t * background;
#endif
    ui_progress_arc_info_t progress_arc;
    lv_obj_t * ota_desc[2];
    lv_obj_t * ota_percent;
    lv_obj_t * ota_size;
    lv_obj_t * icon;
    lv_draw_buf_t *icon_buf;
    res_theme_type_e res_theme_type;
    uint16_t shown_icon_id;
    uint16_t anim_to_sweep;
    uint16_t anim_visual_sweep;
    uint32_t anim_last_tick;
    uint32_t start_time;
    uint32_t display_time;
    uint32_t total_size;
    uint32_t recv_size;
    uint32_t last_recv_size;
    uint8_t force_update;
    uint8_t status;
    uint8_t progress;
    uint8_t shown_percent;
    uint8_t object;
    uint8_t rsv[6];
} ota_display_info_t;

typedef struct {
    const char *name;
    const lv_font_t *font;
    uint32_t color;
    uint16_t y_offset;
    int16_t line_space;
} ota_text_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ota_display_info_t *ota_info = NULL;
#if IS_SUPPORT_OTA_COMP_BG
static PSRAM_DATA_LOC uint8_t factory_reset_confirm_bg_data[] = {
    // #include "comparison_png2txt/ota_hw_360x360.txt"
    // #include "comparison_png2txt/ota_fw_360x360.txt"
    // #include "comparison_png2txt/ota_them_360x360.txt"
    // #include "comparison_png2txt/ota_them_fail_360x360.txt"
    #include "comparison_png2txt/ota_them_ok_360x360.txt"
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
static void ui_ota_root_init(void)
{
    if (ota_info->root == NULL || !lv_obj_is_valid(ota_info->root)) {
        ota_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(ota_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(ota_info->root, lv_color_hex(OTA_ROOT_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(ota_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(ota_info->root, 0, 0);
    lv_obj_set_style_pad_all(ota_info->root, 0, 0);
    lv_obj_align(ota_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(ota_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(ota_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static uint16_t ui_ota_progress_arc_target_sweep_get(uint32_t recv_size, uint32_t total_size,
    uint8_t progress)
{
    if (total_size > 0) {
        return ui_progress_arc_sweep_from_progress(recv_size, total_size);
    }
    if (progress > 100) {
        progress = 100;
    }
    return ui_progress_arc_sweep_from_progress(progress, 100);
}

static uint8_t ui_ota_progress_percent_from_sweep(uint16_t done_sweep)
{
    uint16_t split_track = UI_PROGRESS_ARC_TRACK_DEG - UI_PROGRESS_ARC_SEG_GAP_DEG;

    if (done_sweep >= UI_PROGRESS_ARC_TRACK_DEG || split_track == 0) {
        return 100;
    }
    return (uint8_t)(((uint32_t)done_sweep * 100U) / split_track);
}

static void ui_ota_progress_arc_anim_sync(uint32_t recv_size, uint32_t total_size,
    uint8_t progress, bool snap)
{
    uint16_t target_sweep = ui_ota_progress_arc_target_sweep_get(recv_size, total_size, progress);

    ota_info->anim_to_sweep = target_sweep;
    if (snap || ota_info->force_update || ota_info->anim_visual_sweep == 0xFFFF) {
        ota_info->anim_visual_sweep = target_sweep;
        ota_info->anim_last_tick = get_timestamp();
    }
}

static uint16_t ui_ota_progress_arc_visual_sweep_advance(void)
{
    uint16_t visual = ota_info->anim_visual_sweep;
    uint16_t target = ota_info->anim_to_sweep;
    uint32_t elapsed;
    uint16_t step;
    uint16_t remain;

    if (visual == 0xFFFF) {
        ota_info->anim_last_tick = get_timestamp();
        return target;
    }
    if (visual == target) {
        ota_info->anim_last_tick = get_timestamp();
        return target;
    }

    elapsed = get_timestamp_ms(ota_info->anim_last_tick);
    ota_info->anim_last_tick = get_timestamp();
    if (elapsed == 0) {
        return visual;
    }
    if (elapsed > OTA_PROGRESS_ARC_ELAPSED_CLAMP_MS) {
        elapsed = OTA_PROGRESS_ARC_ELAPSED_CLAMP_MS;
    }

    step = (uint16_t)((elapsed * OTA_PROGRESS_ARC_SPEED_DEG_PER_SEC) / 1000U);
    if (step < 1) {
        step = 1;
    }

    if (target > visual) {
        remain = (uint16_t)(target - visual);
        return (uint16_t)(visual + ((step > remain) ? remain : step));
    }
    remain = (uint16_t)(visual - target);
    return (uint16_t)(visual - ((step > remain) ? remain : step));
}

static void ui_ota_progress_arc_update(ui_manager_context_t *thiz, uint8_t progress,
    uint32_t recv_size, uint32_t total_size, bool is_show)
{
    ui_progress_arc_style_t style;
    uint16_t done_sweep;
    uint8_t show_percent;
    bool need_draw;

    (void)thiz;
    if (progress > 100) {
        progress = 100;
    }

    if (!is_show) {
        ui_progress_arc_hide(&ota_info->progress_arc);
        if (ota_info->ota_percent != NULL && lv_obj_is_valid(ota_info->ota_percent)) {
            lv_obj_add_flag(ota_info->ota_percent, LV_OBJ_FLAG_HIDDEN);
        }
        ota_info->anim_to_sweep = 0;
        ota_info->anim_visual_sweep = 0xFFFF;
        ota_info->shown_percent = 0xFF;
        return;
    }

    if (ota_info->ota_percent == NULL || !lv_obj_is_valid(ota_info->ota_percent)) {
        ota_info->ota_percent = lv_label_create(ota_info->root);
        EXCEPTION_CHECK_VOID(ota_info->ota_percent);
    }
    lv_obj_set_style_text_font(ota_info->ota_percent, UI_FONT_AEONIK, 0);
    lv_obj_set_style_text_color(ota_info->ota_percent, lv_color_hex(OTA_PERCENT_TEXT_COLOR), 0);
    lv_obj_set_style_text_align(ota_info->ota_percent, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_clear_flag(ota_info->ota_percent, LV_OBJ_FLAG_HIDDEN);

    /* 换包(total 变化)或进度回退(recv 变小)时立刻跳到目标，避免从旧断点圆弧慢慢退回 0% */
    bool snap_arc = (ota_info->total_size != 0 && total_size != 0 &&
        total_size != ota_info->total_size) ||
        (recv_size < ota_info->recv_size);
    ui_ota_progress_arc_anim_sync(recv_size, total_size, progress, snap_arc);
    done_sweep = ui_ota_progress_arc_visual_sweep_advance();
    need_draw = (ota_info->force_update || ota_info->anim_visual_sweep != done_sweep);

    if (need_draw) {
        ui_progress_arc_style_init(&style, OTA_PROGRESS_DONE_COLOR);
        style.remain_color = OTA_PROGRESS_REMAIN_COLOR;
        style.show_countdown = false;
        ui_progress_arc_update(&ota_info->progress_arc, ota_info->root, &style,
            done_sweep, ota_info->force_update);
        ota_info->anim_visual_sweep = done_sweep;
    }

    show_percent = ui_ota_progress_percent_from_sweep(done_sweep);
    if (ota_info->force_update || ota_info->shown_percent != show_percent) {
        char ota_percent[8] = {0};
        snprintf(ota_percent, sizeof(ota_percent), "%d%%", show_percent);
        lv_label_set_text(ota_info->ota_percent, ota_percent);
        lv_obj_align(ota_info->ota_percent, LV_ALIGN_TOP_MID, 0, OTA_DOWNLOAD_PERCENT_Y);
        ota_info->shown_percent = show_percent;
    }
    ota_info->progress = progress;
}

static void ui_ota_icon_buf_release(void)
{
    if (ota_info->icon_buf != NULL) {
        if (ota_info->icon != NULL && lv_obj_is_valid(ota_info->icon)) {
            lv_image_set_src(ota_info->icon, NULL);
        }
        lv_draw_buf_destroy(ota_info->icon_buf);
        ota_info->icon_buf = NULL;
        return;
    }
    ui_img_baj_release_decoded_buf(ota_info->icon);
}

static void ui_ota_icon_update(ui_manager_context_t *thiz, uint16_t img_id, uint16_t y_offset,
    res_theme_type_e theme_type, bool predecode, bool is_show)
{
    if (ota_info->force_update == false) {
        return;
    }

    if (ota_info->icon == NULL || !lv_obj_is_valid(ota_info->icon)) {
        ota_info->icon = lv_image_create(ota_info->root);
        EXCEPTION_CHECK_VOID(ota_info->icon);
        ota_info->shown_icon_id = 0;
    }

    if (!is_show) {
        lv_obj_add_flag(ota_info->icon, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(ota_info->icon, LV_OBJ_FLAG_HIDDEN);
    if (ota_info->shown_icon_id != img_id) {
        ui_ota_icon_buf_release();
        if (predecode) {
            ota_info->icon_buf = ui_img_baj_theme_decode_draw_buf(thiz, img_id,
                theme_type, UI_IMG_BAJ_DECODE_RGBA8888);
            if (ota_info->icon_buf != NULL) {
                lv_image_set_src(ota_info->icon, ota_info->icon_buf);
            }
        } else {
            ui_img_baj_theme_set_src(thiz, ota_info->icon, img_id,
                theme_type, UI_IMG_BAJ_DECODE_RGBA8888);
        }
        ota_info->shown_icon_id = img_id;
    }
    lv_obj_align(ota_info->icon, LV_ALIGN_TOP_MID, 0, y_offset);
}

static void ui_ota_desc_update(ui_manager_context_t *thiz, const ota_text_info_t *text_info)
{
    EXCEPTION_CHECK_VOID(text_info);
    if (ota_info->force_update == false) {
        return;
    }

    (void)thiz;
    for (uint8_t i = 0; i < OTA_DESC_TEXT_NUM; i++) {
        if (ota_info->ota_desc[i] == NULL || !lv_obj_is_valid(ota_info->ota_desc[i])) {
            ota_info->ota_desc[i] = lv_label_create(ota_info->root);
            EXCEPTION_CHECK_VOID(ota_info->ota_desc[i]);
        }
        if (text_info[i].name == NULL || text_info[i].name[0] == '\0') {
            lv_label_set_text(ota_info->ota_desc[i], "");
            lv_obj_add_flag(ota_info->ota_desc[i], LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        lv_obj_clear_flag(ota_info->ota_desc[i], LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ota_info->ota_desc[i], text_info[i].name);
        lv_obj_set_style_text_color(ota_info->ota_desc[i], lv_color_hex(text_info[i].color), 0);
        lv_obj_set_style_text_font(ota_info->ota_desc[i], text_info[i].font, 0);
        lv_obj_set_style_text_line_space(ota_info->ota_desc[i], text_info[i].line_space, 0);
        lv_obj_set_style_text_align(ota_info->ota_desc[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(ota_info->ota_desc[i], LV_ALIGN_TOP_MID, 0, text_info[i].y_offset);
    }
}

static void ui_ota_size_update(ui_manager_context_t *thiz, uint32_t total_size, uint32_t recv_size,
    uint8_t is_show)
{
    (void)thiz;
    if (ota_info->force_update == false && ota_info->recv_size == recv_size &&
        ota_info->total_size == total_size && is_show) {
        return;
    }

    if (ota_info->ota_size == NULL || !lv_obj_is_valid(ota_info->ota_size)) {
        ota_info->ota_size = lv_label_create(ota_info->root);
        EXCEPTION_CHECK_VOID(ota_info->ota_size);
    }
    lv_obj_set_style_text_font(ota_info->ota_size, UI_FONT_AEONIK_19, 0);
    lv_obj_set_style_text_color(ota_info->ota_size, lv_color_hex(OTA_SIZE_TEXT_COLOR), 0);
    lv_obj_set_style_text_align(ota_info->ota_size, LV_TEXT_ALIGN_CENTER, 0);

    if (!is_show) {
        lv_obj_add_flag(ota_info->ota_size, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_clear_flag(ota_info->ota_size, LV_OBJ_FLAG_HIDDEN);

    char ota_size_text[32] = {0};
    if (total_size >= 1048576) {
        float total_f = total_size / 1048576.0f;
        if (recv_size >= 1048576) {
            float recv_f = recv_size / 1048576.0f;
            snprintf(ota_size_text, sizeof(ota_size_text), "%0.3fMB / %0.3fMB", recv_f, total_f);
        } else {
            float recv_f = recv_size / 1024.0f;
            snprintf(ota_size_text, sizeof(ota_size_text), "%0.1fKB / %0.1fMB", recv_f, total_f);
        }
    } else {
        float recv_f = recv_size / 1024.0f;
        float total_f = total_size / 1024.0f;
        snprintf(ota_size_text, sizeof(ota_size_text), "%0.1fKB / %0.1fKB", recv_f, total_f);
    }
    LOG_UI("ota progress %s, %d,%d", ota_size_text, recv_size, total_size);
    lv_label_set_text(ota_info->ota_size, ota_size_text);
    lv_obj_align(ota_info->ota_size, LV_ALIGN_TOP_MID, 0, OTA_DOWNLOAD_SIZE_Y);
    ota_info->recv_size = recv_size;
    ota_info->total_size = total_size;
}

static void ui_ota_status_update(ui_manager_context_t *thiz, ota_info_t info, uint8_t ota_status)
{
    const ota_text_info_t ota_res_downloading_text_infos[OTA_DESC_TEXT_NUM] = {
        {OTA_RES_UPGRADING_TEXT,
            UI_FONT_TITLE, OTA_TITLE_TEXT_COLOR, OTA_DOWNLOAD_TITLE_Y, 0},
        {"",
            UI_FONT_TITLE, 0, 0, 0}
    };
    const ota_text_info_t ota_app_downloading_text_infos[OTA_DESC_TEXT_NUM] = {
        {OTA_FW_UPGRADING_TEXT,
            UI_FONT_TITLE, OTA_TITLE_TEXT_COLOR, OTA_DOWNLOAD_TITLE_Y, 0},
        {"",
            UI_FONT_TITLE, 0, 0, 0}
    };
    const ota_text_info_t ota_hw_downloading_text_infos[OTA_DESC_TEXT_NUM] = {
        {OTA_HW_UPGRADING_TEXT,
            UI_FONT_TITLE, OTA_TITLE_TEXT_COLOR, OTA_DOWNLOAD_TITLE_Y, 0},
        {"",
            UI_FONT_TITLE, 0, 0, 0}
    };
    const ota_text_info_t ota_error_text_infos[OTA_DESC_TEXT_NUM] = {
        {OTA_UPGRADE_FAILED_TEXT,
            UI_FONT_TITLE_LG, OTA_TITLE_TEXT_COLOR, OTA_ERROR_TITLE_Y, OTA_ERROR_LINE_SPACE},
        {"",
            UI_FONT_TITLE_LG, 0, 0, 0}
    };
    const ota_text_info_t ota_suceess_text_infos[OTA_DESC_TEXT_NUM] = {
        {OTA_UPGRADE_COMPLETED_TEXT,
            UI_FONT_TITLE_LG, OTA_TITLE_TEXT_COLOR, OTA_SUCCESS_TITLE_Y, OTA_SUCCESS_LINE_SPACE},
        {"",
            UI_FONT_TITLE_LG, 0, 0, 0}
    };
#if IS_SUPPORT_THEME_GUIDE_WINDOW
    const ota_text_info_t ota_custom_error_text_infos[OTA_DESC_TEXT_NUM] = {
        {OTA_UPGRADE_FAILED_TEXT,
            UI_FONT_AEONIK_29, OTA_TITLE_TEXT_COLOR, OTA_CUSTOM_RESULT_TITLE_Y, 0},
        {"",
            UI_FONT_AEONIK_29, 0, 0, 0}
    };
    const ota_text_info_t ota_custom_success_text_infos[OTA_DESC_TEXT_NUM] = {
        {OTA_UPGRADE_COMPLETED_TEXT,
            UI_FONT_AEONIK_29, OTA_TITLE_TEXT_COLOR, OTA_CUSTOM_RESULT_TITLE_Y, 0},
        {"",
            UI_FONT_AEONIK_29, 0, 0, 0}
    };
    bool use_custom_result = (ota_info->res_theme_type == RES_THEME_TYPE_CUSTOM);
#endif

    if (ota_info->status != ota_status || ota_info->object != info.object) {
        ota_info->status = ota_status;
        ota_info->object = info.object;
        ota_info->force_update = true;
    }

    switch (ota_status) {
        default:
        case OTA_UI_STATUS_DOWNLOADING:
            ui_ota_icon_update(thiz, BASE_RES_ID_DOWNLOAD_ICON, OTA_DOWNLOAD_ICON_Y,
                RES_THEME_TYPE_BASE, false, true);
            if (info.object == OTA_OBJECT_MAIN_FIRMWARE) {
                ui_ota_desc_update(thiz, ota_app_downloading_text_infos);
            } else if (info.object == OTA_OBJECT_IMAGE_RESOURCE ||
                       info.object == OTA_OBJECT_LANGUAGE_PACK) {
                ui_ota_desc_update(thiz, ota_res_downloading_text_infos);
            } else if (info.object == OTA_OBJECT_TOUCH_KEY_FIRMWARE) {
                ui_ota_desc_update(thiz, ota_hw_downloading_text_infos);
            }
            ui_ota_progress_arc_update(thiz, info.progress, info.recv_size, info.total_size, true);
            ui_ota_size_update(thiz, info.total_size, info.recv_size, true);
            break;
        case OTA_UI_STATUS_ERROR:
#if IS_SUPPORT_THEME_GUIDE_WINDOW
            if (use_custom_result) {
                ui_ota_icon_update(thiz, CUSTOM_RES_ID_WARNING_IP_ICON, OTA_CUSTOM_RESULT_ICON_Y,
                    RES_THEME_TYPE_CUSTOM, true, true);
                ui_ota_desc_update(thiz, ota_custom_error_text_infos);
                ui_ota_size_update(thiz, info.total_size, info.recv_size, false);
                ui_ota_progress_arc_update(thiz, info.progress, info.recv_size, info.total_size, false);
                break;
            }
#endif
            ui_ota_icon_update(thiz, BASE_RES_ID_ERROR_ICON, OTA_ERROR_ICON_Y,
                RES_THEME_TYPE_BASE, false, true);
            ui_ota_desc_update(thiz, ota_error_text_infos);
            ui_ota_size_update(thiz, info.total_size, info.recv_size, false);
            ui_ota_progress_arc_update(thiz, info.progress, info.recv_size, info.total_size, false);
            break;
        case OTA_UI_STATUS_COMPLETE:
#if IS_SUPPORT_THEME_GUIDE_WINDOW
            if (use_custom_result) {
                ui_ota_icon_update(thiz, CUSTOM_RES_ID_SUCCESS_IP_ICON, OTA_CUSTOM_RESULT_ICON_Y,
                    RES_THEME_TYPE_CUSTOM, true, true);
                ui_ota_desc_update(thiz, ota_custom_success_text_infos);
                ui_ota_size_update(thiz, info.total_size, info.recv_size, false);
                ui_ota_progress_arc_update(thiz, info.progress, info.recv_size, info.total_size, false);
                break;
            }
#endif
            ui_ota_icon_update(thiz, BASE_RES_ID_SUCCESS_ICON, OTA_SUCCESS_ICON_Y,
                RES_THEME_TYPE_BASE, false, true);
            ui_ota_desc_update(thiz, ota_suceess_text_infos);
            ui_ota_size_update(thiz, info.total_size, info.recv_size, false);
            ui_ota_progress_arc_update(thiz, info.progress, info.recv_size, info.total_size, false);
            break;
    }
}

#if IS_SUPPORT_OTA_COMP_BG
static void ui_ota_background_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (ota_info->background == NULL || !lv_obj_is_valid(ota_info->background)) {
        ota_info->background = lv_image_create(ota_info->root);
        EXCEPTION_CHECK_VOID(ota_info->background);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(ota_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(ota_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, factory_reset_confirm_bg_data,
            sizeof(factory_reset_confirm_bg_data), "png");
        lv_image_set_src(ota_info->background, (const void *)&bg_img_path);
        lv_obj_align(ota_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_ota_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (ota_info == NULL) {
        ota_info = (ota_display_info_t *)u_malloz(sizeof(ota_display_info_t));
        EXCEPTION_CHECK_VOID(ota_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_ota_root_init();
    ui_progress_arc_reset(&ota_info->progress_arc);
    ota_info->force_update = true;
    ota_info->progress = 0;
    ota_info->anim_to_sweep = 0;
    ota_info->anim_visual_sweep = 0xFFFF;
    ota_info->shown_percent = 0xFF;
    ota_info->anim_last_tick = get_timestamp();
    ota_info->shown_icon_id = 0;
    ota_info->icon_buf = NULL;
#if IS_SUPPORT_OTA_TEST
    ota_info->res_theme_type = RES_THEME_TYPE_BASE;
#else
    ui_proc_ota_t *proc = (ui_proc_ota_t *)thiz->priv.play_info.user_data;
    if (proc) {
        ota_info_t ota_dev_info = {0};
        proc->ota->get_ota_info(&ota_dev_info);
        ota_info->last_recv_size = ota_dev_info.recv_size;
    }
    ota_info->res_theme_type = RES_THEME_TYPE_BASE;
#if IS_SUPPORT_THEME_GUIDE_WINDOW
    if (thiz && thiz->priv.res) {
        ota_info->res_theme_type = thiz->priv.res->get_sys_theme_type();
    }
#endif
#endif
}

void ui_display_ota_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && ota_info && ota_info->root);
    ui_ota_icon_buf_release();
    LVGL_OBJ_DELETE(ota_info->root);
    u_free(ota_info);
    ota_info = NULL;
}

int ui_display_ota_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && ota_info && ota_info->root);
#if IS_SUPPORT_OTA_TEST
    static ota_info_t ota_dev_info = {0};
    static uint8_t ota_status = OTA_UI_STATUS_COMPLETE;
    ota_dev_info.object = OTA_OBJECT_IMAGE_RESOURCE;
    ota_dev_info.total_size = 2 * 1024 * 1024;
#else
    ui_proc_ota_t *proc = (ui_proc_ota_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    ota_info_t ota_dev_info = {0};
    proc->ota->get_ota_info(&ota_dev_info);
    uint8_t ota_status = proc->ota_status;
#endif

#if IS_SUPPORT_OTA_COMP_BG
    ui_ota_background_update(thiz);
#endif
    ui_ota_status_update(thiz, ota_dev_info, ota_status);
    ota_info->force_update = false;
    thiz->priv.fps_count++;
    if (ota_status == OTA_UI_STATUS_DOWNLOADING) {
        ota_info->display_time = get_timestamp();
    } else {
        if (get_timestamp_ms(ota_info->display_time) > OTA_RESULT_DISPLAY_TIME) {
            ui_animation_play_t *ani = &thiz->priv.play_info;
            EXCEPTION_CHECK_INT(ani);
            LOG_UI("ota result display timeout");
            ani->progress_cb(thiz, 0, ANI_TIMER_DRAW_FINISH);
        }
    }

    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        float speed = ota_dev_info.recv_size > ota_info->last_recv_size ?
            (ota_dev_info.recv_size - ota_info->last_recv_size) / 1024.0f : 0;
        ota_info->last_recv_size = ota_dev_info.recv_size;
        LOG_UI("===> ota fps: %d, %0.2fKB/s", thiz->priv.fps_count, speed);
        thiz->priv.fps_count = 0;
        #if IS_SUPPORT_OTA_TEST
        ota_dev_info.recv_size += 10 * 1024;
        ota_dev_info.progress = ota_dev_info.recv_size * 100 / ota_dev_info.total_size;
        if (ota_dev_info.progress >= 100) {
            ota_status = OTA_UI_STATUS_COMPLETE;
        }
        #endif
    }

    return 0;
}
