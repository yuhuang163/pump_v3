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
    if (page_index == HOME_PAGE_RHYTHM_2) {
        return home_page_rhythm3_enable(info);
    }
    return true;
}

static uint8_t home_page_neighbor_get(home_base_ui_info_t *info, uint8_t page_index,
    home_page_slide_dir_e dir)
{
    uint8_t next = page_index;
    uint8_t guard = 0;

    if (dir == HOME_PAGE_SLIDE_NONE) {
        return page_index;
    }

    do {
        if (dir == HOME_PAGE_SLIDE_LEFT) {
            next = (uint8_t)((next + 1) % HOME_PAGE_MAX);
        } else {
            next = (uint8_t)((next + HOME_PAGE_MAX - 1) % HOME_PAGE_MAX);
        }
        guard++;
    } while (!home_page_is_available(info, next) && guard < HOME_PAGE_MAX);

    return home_page_is_available(info, next) ? next : page_index;
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
            /// TODO: 模式选择页，等参数后再画
            break;
        case HOME_PAGE_RHYTHM_0:
        case HOME_PAGE_RHYTHM_1:
        case HOME_PAGE_RHYTHM_2:
            /// TODO: 韵律选择页，等参数后再画
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
    const int32_t inv = HOME_PAGE_PROGRESS_FULL - p;
    const int16_t side = HOME_PAGE_TRANSITION_SIDE_OFFSET_X;
    const int32_t scale_min = HOME_PAGE_TRANSITION_SCALE_MIN;
    const int32_t opa_min = HOME_PAGE_TRANSITION_OPA_MIN;
    int16_t out_x;
    int16_t in_x;

    if (info->page.slide_dir == HOME_PAGE_SLIDE_LEFT) {
        out_x = (int16_t)((-side * p) / HOME_PAGE_PROGRESS_FULL);
        in_x = (int16_t)((side * inv) / HOME_PAGE_PROGRESS_FULL);
    } else {
        out_x = (int16_t)((side * p) / HOME_PAGE_PROGRESS_FULL);
        in_x = (int16_t)((-side * inv) / HOME_PAGE_PROGRESS_FULL);
    }

    const int32_t out_scale = HOME_PAGE_SCALE_FULL -
        ((HOME_PAGE_SCALE_FULL - scale_min) * p) / HOME_PAGE_PROGRESS_FULL;
    const int32_t in_scale = scale_min +
        ((HOME_PAGE_SCALE_FULL - scale_min) * p) / HOME_PAGE_PROGRESS_FULL;
    const int32_t out_opa_i = LV_OPA_COVER -
        ((LV_OPA_COVER - opa_min) * p) / HOME_PAGE_PROGRESS_FULL;
    const int32_t in_opa_i = opa_min +
        ((LV_OPA_COVER - opa_min) * p) / HOME_PAGE_PROGRESS_FULL;

    home_page_transform_apply(info->page.page, out_x, out_scale, (lv_opa_t)out_opa_i);
    home_page_transform_apply(info->page.peer_page, in_x, in_scale, (lv_opa_t)in_opa_i);
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
    if (elapsed >= HOME_PAGE_TRANSITION_MS) {
        progress = HOME_PAGE_PROGRESS_FULL;
    } else {
        progress = (int32_t)((elapsed * HOME_PAGE_PROGRESS_FULL) / HOME_PAGE_TRANSITION_MS);
    }
    info->page.progress = (int16_t)progress;
    home_page_transition_lerp(info);

    if (progress >= HOME_PAGE_PROGRESS_FULL) {
        home_page_transition_finish(info);
    }
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
    memset(&info->page, 0, sizeof(info->page));
    info->page.shown_page_index = HOME_PAGE_INDEX_INVALID;
}

void home_page_ui_update(home_base_ui_info_t *info, ui_proc_home_t *proc)
{
    EXCEPTION_CHECK_VOID(info && proc);

    if (info->page.shown_page_index == HOME_PAGE_INDEX_INVALID) {
        home_page_switch_instant(info, info->page.page_index);
    }

    /// 个性化失活时，若停在第三韵律页则退回上一可用页
    if (!info->page.transitioning &&
        info->page.shown_page_index == HOME_PAGE_RHYTHM_2 &&
        !home_page_rhythm3_enable(info)) {
        home_page_switch_instant(info, HOME_PAGE_RHYTHM_1);
    }

    home_page_transition_tick(info);
    home_page_content_update(info, proc);
}

bool home_page_ui_is_transitioning(const home_base_ui_info_t *info)
{
    return info != NULL && info->page.transitioning != 0;
}

void home_page_ui_request_switch(home_base_ui_info_t *info, home_page_slide_dir_e dir)
{
    uint8_t next_index;

    EXCEPTION_CHECK_VOID(info);
    if (dir == HOME_PAGE_SLIDE_NONE || info->page.transitioning) {
        return;
    }
    if (info->page.page == NULL || !lv_obj_is_valid(info->page.page)) {
        return;
    }

    next_index = home_page_neighbor_get(info, info->page.shown_page_index, dir);
    if (next_index == info->page.shown_page_index) {
        return;
    }

    /// 过渡期间停掉主页动画，降低重绘压力
    home_base_animation_pause(info);

    info->page.peer_page = home_page_container_create(info);
    EXCEPTION_CHECK_VOID(info->page.peer_page);

    /// 临时把 page 指到邻页以复用 build 里的 parent_get
    {
        lv_obj_t *cur = info->page.page;
        info->page.page = info->page.peer_page;
        home_page_content_build(info, next_index);
        info->page.page = cur;
    }

    info->page.page_index = next_index;
    info->page.slide_dir = (int8_t)dir;
    info->page.transitioning = 1;
    info->page.progress = 0;
    info->page.start_ms = get_timestamp();
    home_page_transition_lerp(info);
}
