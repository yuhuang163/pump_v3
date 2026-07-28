/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "home_base_ui_internal.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_PAGE_SCALE_FULL                            (256)
#define HOME_PAGE_PROGRESS_FULL                         (256)
#define HOME_PAGE_INDEX_INVALID                         (0xFF)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static bool home_page_rhythm3_enable(const home_base_ui_info_t *info)
{
    return info != NULL && info->mode.custom_mode_active != 0;
}

static bool home_page_is_available(const home_base_ui_info_t *info, uint8_t page_index)
{
    if (page_index >= HOME_PAGE_MAX) {
        return false;
    }
    if (page_index == HOME_MODE_PAGE_INDEX_CUSTOMIZE_SELECT) {
        return home_page_rhythm3_enable(info);
    }
    return true;
}

static void home_page_transform_apply(lv_obj_t *page, int16_t offset_x, int32_t scale, lv_opa_t opa)
{
    if (page == NULL || !lv_obj_is_valid(page)) {
        return;
    }

    lv_obj_align(page, LV_ALIGN_CENTER, offset_x, 0);
    lv_obj_set_style_transform_pivot_x(page, lv_obj_get_width(page) / 2, 0);
    lv_obj_set_style_transform_pivot_y(page, lv_obj_get_height(page) / 2, 0);
    lv_obj_set_style_transform_scale(page, scale, 0);
    lv_obj_set_style_opa(page, opa, 0);
}

static void home_page_transform_reset(lv_obj_t *page)
{
    home_page_transform_apply(page, 0, HOME_PAGE_SCALE_FULL, LV_OPA_COVER);
}

static bool home_page_is_mode_like(uint8_t page_index)
{
    return page_index == HOME_PAGE_MODE_0 ||
           page_index == HOME_PAGE_MODE_1 ||
           page_index == HOME_PAGE_RHYTHM_0 ||
           page_index == HOME_PAGE_RHYTHM_1 ||
           page_index == HOME_PAGE_RHYTHM_2;
}

/// 对指定页施加 hero 缩放 + 圆背景显隐；整页位移由 page 容器完成
static void home_page_hero_transition_apply(home_base_ui_info_t *info, uint8_t page_index,
    lv_obj_t *page, int32_t hero_scale, int32_t bg_scale, bool transitioning)
{
    if (page == NULL || !lv_obj_is_valid(page)) {
        return;
    }

    if (page_index == HOME_PAGE_MAIN) {
        /* MAIN 控件挂在 info->main；仅当仍属于该 page 时才施加（避免 peer 覆盖后误改） */
        if (info->main.mode_title != NULL && lv_obj_is_valid(info->main.mode_title) &&
            lv_obj_get_parent(info->main.mode_title) == page) {
            home_main_page_transition_apply(info, hero_scale, bg_scale, transitioning);
        }
        return;
    }

    if (home_page_is_mode_like(page_index)) {
        home_mode_page_transition_apply(page, hero_scale, transitioning);
    }
}

/** 过渡色块直径：page0 全尺寸不缩放；模式页为圆径 */
static int32_t home_page_blob_diameter(uint8_t page_index)
{
    if (page_index == HOME_PAGE_MAIN) {
        return HOME_BASE_PAGE_TRANSITION_MAIN_BG_FULL_SIZE;
    }
    return HOME_BASE_PAGE_TRANSITION_CIRCLE_SIZE;
}

static lv_obj_t *home_page_container_create(home_base_ui_info_t *info)
{
    lv_obj_t *page = lv_obj_create(info->root);

    if (page == NULL) {
        return NULL;
    }
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_CLICKABLE);
    home_page_transform_reset(page);
    return page;
}

static void home_page_content_build(home_base_ui_info_t *info, uint8_t page_index)
{
    switch (page_index) {
        case HOME_PAGE_MAIN:
            home_main_page_ui_build(info);
            break;
        case HOME_PAGE_MODE_0:
        case HOME_PAGE_MODE_1:
        case HOME_PAGE_RHYTHM_0:
        case HOME_PAGE_RHYTHM_1:
        case HOME_PAGE_RHYTHM_2:
            home_mode_page_ui_build(info, page_index);
            break;
        default:
            break;
    }
}

static void home_page_content_remove(home_base_ui_info_t *info, uint8_t page_index)
{
    switch (page_index) {
        case HOME_PAGE_MAIN:
            home_main_page_ui_remove(info);
            home_base_animation_remove(info);
            break;
        case HOME_PAGE_MODE_0:
        case HOME_PAGE_MODE_1:
        case HOME_PAGE_RHYTHM_0:
        case HOME_PAGE_RHYTHM_1:
        case HOME_PAGE_RHYTHM_2:
            home_mode_page_ui_remove(info);
            break;
        default:
            break;
    }
}

static void home_page_content_update(home_base_ui_info_t *info, ui_proc_home_t *proc)
{
    switch (info->page.shown_page_index) {
        case HOME_PAGE_MAIN:
            home_main_page_ui_update(info, proc);
            break;
        case HOME_PAGE_MODE_0:
        case HOME_PAGE_MODE_1:
        case HOME_PAGE_RHYTHM_0:
        case HOME_PAGE_RHYTHM_1:
        case HOME_PAGE_RHYTHM_2:
            home_mode_page_ui_update(info, proc);
            break;
        default:
            break;
    }
}

static void home_page_switch_instant(home_base_ui_info_t *info, uint8_t page_index)
{
    if (info->page.page != NULL && lv_obj_is_valid(info->page.page)) {
        home_page_content_remove(info, info->page.shown_page_index);
        LVGL_OBJ_DELETE(info->page.page);
        info->page.page = NULL;
    }
    if (info->page.peer_page != NULL && lv_obj_is_valid(info->page.peer_page)) {
        LVGL_OBJ_DELETE(info->page.peer_page);
        info->page.peer_page = NULL;
    }

    info->page.page_index = page_index;
    info->page.shown_page_index = page_index;
    info->page.transitioning = 0;
    info->page.slide_dir = HOME_PAGE_SLIDE_NONE;
    info->page.progress = 0;
    info->force_update = true;

    info->page.page = home_page_container_create(info);
    EXCEPTION_CHECK_VOID(info->page.page);
    home_page_content_build(info, page_index);
}

static void home_page_transition_lerp(home_base_ui_info_t *info)
{
    const int32_t p = info->page.progress;
    const int32_t slide_end = HOME_BASE_PAGE_TRANSITION_SLIDE_PROGRESS;
    const int32_t hero_scale_min = HOME_BASE_PAGE_TRANSITION_HERO_SCALE_MIN;
    const int32_t opa_min = HOME_BASE_PAGE_TRANSITION_OPA_MIN;
    const int32_t out_d = home_page_blob_diameter(info->page.shown_page_index);
    const int32_t in_d = home_page_blob_diameter(info->page.page_index);
    /* page0 全尺寸 + page1 圆径 + 固定边缘间距 */
    const int32_t dist = (out_d / 2) + (in_d / 2) + HOME_BASE_PAGE_TRANSITION_CIRCLE_GAP;
    /* 入场为模式页才需要落地放大；回 page0 只做位移 */
    const bool need_scale_up = home_page_is_mode_like(info->page.page_index);
    int32_t slide_p;
    int32_t scale_p;
    int32_t inv_slide;
    int16_t out_x;
    int16_t in_x;
    int32_t out_hero;
    int32_t in_hero;
    int32_t out_opa_i;
    int32_t in_opa_i;

    /*
     * 两段式（入场为模式页）：
     * 1) [0, slide_end)  仅位移；page0 全尺寸，模式页保持缩小
     * 2) [slide_end, FULL] 模式页落地后放大
     * 回 page0：全程仅位移
     */
    if (!need_scale_up || slide_end <= 0) {
        slide_p = p;
        scale_p = HOME_PAGE_PROGRESS_FULL;
    } else if (p < slide_end) {
        slide_p = (p * HOME_PAGE_PROGRESS_FULL) / slide_end;
        scale_p = 0;
    } else if (p >= HOME_PAGE_PROGRESS_FULL) {
        slide_p = HOME_PAGE_PROGRESS_FULL;
        scale_p = HOME_PAGE_PROGRESS_FULL;
    } else {
        slide_p = HOME_PAGE_PROGRESS_FULL;
        scale_p = ((p - slide_end) * HOME_PAGE_PROGRESS_FULL) /
                  (HOME_PAGE_PROGRESS_FULL - slide_end);
    }
    inv_slide = HOME_PAGE_PROGRESS_FULL - slide_p;

    if (info->page.slide_dir == HOME_PAGE_SLIDE_LEFT) {
        out_x = (int16_t)((-dist * slide_p) / HOME_PAGE_PROGRESS_FULL);
        in_x = (int16_t)((dist * inv_slide) / HOME_PAGE_PROGRESS_FULL);
    } else {
        out_x = (int16_t)((dist * slide_p) / HOME_PAGE_PROGRESS_FULL);
        in_x = (int16_t)((-dist * inv_slide) / HOME_PAGE_PROGRESS_FULL);
    }

    /* page0 始终 100%；模式页离场保持缩小，入场第二阶段放大 */
    if (info->page.shown_page_index == HOME_PAGE_MAIN) {
        out_hero = HOME_PAGE_SCALE_FULL;
    } else {
        out_hero = hero_scale_min;
    }
    if (info->page.page_index == HOME_PAGE_MAIN) {
        in_hero = HOME_PAGE_SCALE_FULL;
    } else {
        in_hero = hero_scale_min +
            ((HOME_PAGE_SCALE_FULL - hero_scale_min) * scale_p) / HOME_PAGE_PROGRESS_FULL;
    }

    out_opa_i = LV_OPA_COVER -
        ((LV_OPA_COVER - opa_min) * slide_p) / HOME_PAGE_PROGRESS_FULL;
    in_opa_i = opa_min +
        ((LV_OPA_COVER - opa_min) * slide_p) / HOME_PAGE_PROGRESS_FULL;

    home_page_transform_apply(info->page.page, out_x, HOME_PAGE_SCALE_FULL, (lv_opa_t)out_opa_i);
    home_page_transform_apply(info->page.peer_page, in_x, HOME_PAGE_SCALE_FULL, (lv_opa_t)in_opa_i);

    home_page_hero_transition_apply(info, info->page.shown_page_index, info->page.page,
        out_hero, HOME_PAGE_SCALE_FULL, true);
    home_page_hero_transition_apply(info, info->page.page_index, info->page.peer_page,
        in_hero, HOME_PAGE_SCALE_FULL, true);
}

static void home_page_transition_finish(home_base_ui_info_t *info)
{
    const uint8_t new_index = info->page.page_index;

    home_page_content_remove(info, info->page.shown_page_index);
    LVGL_OBJ_DELETE(info->page.page);
    info->page.page = info->page.peer_page;
    info->page.peer_page = NULL;
    info->page.shown_page_index = new_index;
    info->page.transitioning = 0;
    info->page.slide_dir = HOME_PAGE_SLIDE_NONE;
    info->page.progress = 0;
    home_page_transform_reset(info->page.page);
    home_page_hero_transition_apply(info, new_index, info->page.page,
        HOME_PAGE_SCALE_FULL, HOME_PAGE_SCALE_FULL, false);
    home_base_animation_resume(info);
}

static void home_page_transition_tick(home_base_ui_info_t *info)
{
    uint32_t elapsed;
    int32_t progress;

    if (!info->page.transitioning) {
        return;
    }

    elapsed = get_timestamp() - info->page.start_ms;
    if (elapsed >= HOME_BASE_PAGE_TRANSITION_MS) {
        progress = HOME_PAGE_PROGRESS_FULL;
    } else {
        progress = (int32_t)((elapsed * HOME_PAGE_PROGRESS_FULL) / HOME_BASE_PAGE_TRANSITION_MS);
    }
    info->page.progress = (int16_t)progress;
    home_page_transition_lerp(info);

    if (progress >= HOME_PAGE_PROGRESS_FULL) {
        home_page_transition_finish(info);
    }
}

static void home_page_ui_goto(home_base_ui_info_t *info, uint8_t page_index, ui_proc_home_t *proc)
{
    home_page_slide_dir_e dir;

    EXCEPTION_CHECK_VOID(info);
    if (info->page.transitioning) {
        return;
    }
    if (info->page.page == NULL || !lv_obj_is_valid(info->page.page)) {
        return;
    }
    if (page_index == info->page.shown_page_index) {
        info->page.page_index = page_index;
        return;
    }
    if (!home_page_is_available(info, page_index)) {
        return;
    }

    dir = (page_index > info->page.shown_page_index) ?
            HOME_PAGE_SLIDE_LEFT : HOME_PAGE_SLIDE_RIGHT;

    /// 过渡期间停掉主页动画，降低重绘压力
    home_base_animation_pause(info);

    info->page.peer_page = home_page_container_create(info);
    EXCEPTION_CHECK_VOID(info->page.peer_page);

    /// 临时把 page 指到目标页以复用 build/update 里的 parent_get
    {
        lv_obj_t *cur = info->page.page;
        info->page.page = info->page.peer_page;
        home_page_content_build(info, page_index);
        /* 回 page0：用真实 pump 数据刷新速度等，动画过程中控件齐全 */
        if (page_index == HOME_PAGE_MAIN && proc != NULL) {
            home_main_page_ui_update(info, proc);
        }
        info->page.page = cur;
    }

    info->page.page_index = page_index;
    info->page.slide_dir = (int8_t)dir;
    info->page.transitioning = 1;
    info->page.progress = 0;
    info->page.start_ms = get_timestamp();
    home_page_transition_lerp(info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_page_ui_prepare(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    memset(&info->page, 0, sizeof(info->page));
    info->page.shown_page_index = HOME_PAGE_INDEX_INVALID;
    info->page.page_index = HOME_PAGE_MAIN;
    home_mode_chrome_prepare(info);
    home_page_switch_instant(info, HOME_PAGE_MAIN);
}

void home_page_ui_remove(home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    if (info->page.page != NULL && lv_obj_is_valid(info->page.page)) {
        home_page_content_remove(info, info->page.shown_page_index);
        LVGL_OBJ_DELETE(info->page.page);
    }
    if (info->page.peer_page != NULL && lv_obj_is_valid(info->page.peer_page)) {
        LVGL_OBJ_DELETE(info->page.peer_page);
    }
    home_mode_chrome_remove(info);
    memset(&info->page, 0, sizeof(info->page));
    info->page.shown_page_index = HOME_PAGE_INDEX_INVALID;
}

void home_page_ui_update(home_base_ui_info_t *info, ui_proc_home_t *proc)
{
    uint8_t target_index;

    EXCEPTION_CHECK_VOID(info && proc);

    target_index = (uint8_t)proc->ui_mode_page_index;
    if (target_index >= HOME_PAGE_MAX) {
        target_index = HOME_PAGE_MAIN;
    }
    /// 个性化失活时，不允许停在第三韵律页（page_index 由 window 侧 clamp）
    if (target_index == HOME_MODE_PAGE_INDEX_CUSTOMIZE_SELECT &&
        !home_page_rhythm3_enable(info)) {
        target_index = HOME_MODE_PAGE_INDEX_POWER_PUMPING_SELECT;
    }

    if (info->page.shown_page_index == HOME_PAGE_INDEX_INVALID) {
        home_page_switch_instant(info, target_index);
    } else if (!info->page.transitioning &&
               info->page.shown_page_index != target_index) {
        home_page_ui_goto(info, target_index, proc);
    }

    home_page_transition_tick(info);
    /// 过渡中仍可能被 update 重建控件；结束后再 lerp 一次，保证圆背景/缩放不被冲掉
    if (info->page.transitioning) {
        home_page_content_update(info, proc);
        home_page_transition_lerp(info);
    } else {
        home_page_content_update(info, proc);
    }
    home_mode_chrome_update(info);
}

bool home_page_ui_is_transitioning(const home_base_ui_info_t *info)
{
    return info != NULL && info->page.transitioning != 0;
}
