/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_wave.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep
#include "lvgl.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define UI_WAVE_PHASE_OFFSET_DEG          270
#define UI_WAVE_SWITCH_LOG             0

#if UI_WAVE_SWITCH_LOG
#define WAVE_SWITCH_LOG(...)                  LOG_UI(__VA_ARGS__)
#else
#define WAVE_SWITCH_LOG(...)                  do {} while (0)
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
struct ui_wave {
    lv_obj_t *line;
    lv_obj_t *shadow_line;
    lv_point_precise_t *points;
    lv_point_precise_t *shadow_points;
    lv_timer_t *timer;
    ui_wave_cfg_t cfg;
    uint16_t width;
    uint16_t height;
    uint16_t narrow_period_px;
    uint16_t wide_period_px;
    uint16_t amplitude_px;
    uint8_t mix_short_count;
    int32_t scroll_sample_index;
    uint8_t points_bound;
    uint8_t shadow_points_bound;
    ui_wave_type_e pending_type;
    uint8_t has_pending_type;
    lv_coord_t *sample_lut;
    uint16_t sample_lut_len;
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void wave_redraw_full(ui_wave_t *wave);

static uint16_t wave_default_narrow_period(uint16_t width)
{
    uint16_t period = width / UI_WAVE_NARROW_COUNT;
    if (period < UI_WAVE_MIN_PERIOD) {
        period = UI_WAVE_MIN_PERIOD;
    }
    return period;
}

static void wave_cache_metrics(ui_wave_t *wave)
{
    int32_t full_range = wave->height - wave->cfg.line_width;
    if (full_range < 1) {
        full_range = 1;
    }

    if (wave->cfg.amplitude > 0 && wave->cfg.amplitude < full_range) {
        wave->amplitude_px = (uint16_t)wave->cfg.amplitude;
    } else {
        wave->amplitude_px = (uint16_t)full_range;
    }

    if (wave->cfg.period_px >= UI_WAVE_MIN_PERIOD) {
        wave->narrow_period_px = wave->cfg.period_px;
    } else {
        wave->narrow_period_px = wave_default_narrow_period(wave->width);
    }
    wave->wide_period_px = wave->narrow_period_px * UI_WAVE_WIDE_FACTOR_DEFAULT;
    wave->mix_short_count = UI_WAVE_MIX_SHORT_COUNT;
}

static int16_t wave_normalize_angle(int32_t angle)
{
    angle %= 360;
    if (angle < 0) {
        angle += 360;
    }
    return (int16_t)angle;
}

static int32_t wave_mod_pos(int32_t index, int32_t cycle)
{
    if (cycle <= 0) {
        return 0;
    }

    int32_t pos = index % cycle;
    if (pos < 0) {
        pos += cycle;
    }
    return pos;
}

static int16_t wave_uniform_angle(int32_t index, uint16_t period)
{
    const int32_t rounded = ((int32_t)index * 360) + ((int32_t)period / 2);
    return wave_normalize_angle(rounded / (int32_t)period + UI_WAVE_PHASE_OFFSET_DEG);
}

static int16_t wave_mix_angle(const ui_wave_t *wave, int32_t index)
{
    const int32_t narrow_period = wave->narrow_period_px;
    const int32_t wide_period = wave->wide_period_px;
    const int32_t short_block = (int32_t)wave->mix_short_count * narrow_period;
    const int32_t cycle = short_block + wide_period;
    const int32_t pos = wave_mod_pos(index, cycle);
    int32_t angle;

    if (pos < short_block) {
        angle = (pos * 360 + narrow_period / 2) / narrow_period + UI_WAVE_PHASE_OFFSET_DEG;
    } else {
        const int32_t wide_pos = pos - short_block;
        angle = (int32_t)wave->mix_short_count * 360 +
            (wide_pos * 360 + wide_period / 2) / wide_period + UI_WAVE_PHASE_OFFSET_DEG;
    }

    return wave_normalize_angle(angle);
}

static int32_t wave_y_from_angle(const ui_wave_t *wave, int16_t angle)
{
    const int32_t sin_val = lv_trigo_sin(angle);
    const int32_t num = (sin_val + LV_TRIGO_SIN_MAX) * wave->amplitude_px;
    const int32_t rise = (num + LV_TRIGO_SIN_MAX) / (2 * LV_TRIGO_SIN_MAX);

    return (wave->height - wave->cfg.line_width) - rise;
}

static int32_t wave_mix_cycle_len(const ui_wave_t *wave)
{
    return (int32_t)wave->mix_short_count * wave->narrow_period_px + wave->wide_period_px;
}

static int32_t wave_sample_y_calc(const ui_wave_t *wave, int32_t index, ui_wave_type_e type)
{
    int16_t angle;

    switch (type) {
    case UI_WAVE_TYPE_LACTATION:
        angle = wave_uniform_angle(index, wave->wide_period_px);
        break;
    case UI_WAVE_TYPE_MIX:
        angle = wave_mix_angle(wave, index);
        break;
    case UI_WAVE_TYPE_STIMULATION:
    default:
        angle = wave_uniform_angle(index, wave->narrow_period_px);
        break;
    }

    return wave_y_from_angle(wave, angle);
}

static uint16_t wave_cycle_len_for_type(const ui_wave_t *wave, ui_wave_type_e type)
{
    switch (type) {
    case UI_WAVE_TYPE_LACTATION:
        return wave->wide_period_px;
    case UI_WAVE_TYPE_MIX:
        return (uint16_t)wave_mix_cycle_len(wave);
    case UI_WAVE_TYPE_STIMULATION:
    default:
        return wave->narrow_period_px;
    }
}

static void wave_free_sample_lut(ui_wave_t *wave)
{
    if (wave == NULL) {
        return;
    }

    if (wave->sample_lut != NULL) {
        u_free(wave->sample_lut);
        wave->sample_lut = NULL;
    }
    wave->sample_lut_len = 0U;
}

static void wave_rebuild_sample_lut(ui_wave_t *wave)
{
    const uint16_t len = wave_cycle_len_for_type(wave, wave->cfg.type);

    if (wave == NULL || len == 0U) {
        wave_free_sample_lut(wave);
        return;
    }

    if (wave->sample_lut == NULL || wave->sample_lut_len != len) {
        wave_free_sample_lut(wave);
        wave->sample_lut = (lv_coord_t *)u_malloz(sizeof(lv_coord_t) * len);
        if (wave->sample_lut == NULL) {
            wave->sample_lut_len = 0U;
            return;
        }
        wave->sample_lut_len = len;
    }

    for (uint16_t i = 0; i < len; i++) {
        wave->sample_lut[i] = (lv_coord_t)wave_sample_y_calc(wave, (int32_t)i, wave->cfg.type);
    }
}

static int32_t wave_sample_y_at_index(const ui_wave_t *wave, int32_t index)
{
    if (wave->sample_lut != NULL && wave->sample_lut_len > 0U) {
        return wave->sample_lut[wave_mod_pos(index, wave->sample_lut_len)];
    }

    return wave_sample_y_calc(wave, index, wave->cfg.type);
}

static int32_t wave_mix_pos(const ui_wave_t *wave, int32_t index)
{
    return wave_mod_pos(index, wave_mix_cycle_len(wave));
}

static bool wave_is_narrow_cycle_start(int32_t index, uint16_t narrow_period)
{
    return wave_mod_pos(index, narrow_period) == 0;
}

static bool wave_is_mix_trough(const ui_wave_t *wave, int32_t index)
{
    const int32_t narrow_period = wave->narrow_period_px;
    const int32_t short_block = (int32_t)wave->mix_short_count * narrow_period;
    const int32_t pos = wave_mod_pos(index, wave_mix_cycle_len(wave));

    if (pos < short_block) {
        return wave_mod_pos(pos, narrow_period) == 0;
    }

    return pos == short_block;
}

static bool wave_is_mix_wide_start(const ui_wave_t *wave, int32_t index)
{
    const int32_t narrow_period = wave->narrow_period_px;
    const int32_t short_block = (int32_t)wave->mix_short_count * narrow_period;
    return wave_mod_pos(index, wave_mix_cycle_len(wave)) == short_block;
}

static bool wave_is_trough_at(const ui_wave_t *wave, int32_t index, ui_wave_type_e type)
{
    switch (type) {
    case UI_WAVE_TYPE_LACTATION:
        return wave_mod_pos(index, wave->wide_period_px) == 0;
    case UI_WAVE_TYPE_MIX:
        return wave_is_mix_trough(wave, index);
    case UI_WAVE_TYPE_STIMULATION:
    default:
        return wave_mod_pos(index, wave->narrow_period_px) == 0;
    }
}

static bool wave_can_switch_at(const ui_wave_t *wave, ui_wave_type_e from,
    ui_wave_type_e to, int32_t index)
{
    switch (from) {
    case UI_WAVE_TYPE_STIMULATION:
        if (to == UI_WAVE_TYPE_LACTATION || to == UI_WAVE_TYPE_MIX) {
            return wave_is_narrow_cycle_start(index, wave->narrow_period_px);
        }
        break;
    case UI_WAVE_TYPE_LACTATION:
        if (to == UI_WAVE_TYPE_MIX || to == UI_WAVE_TYPE_STIMULATION) {
            /* Switch at trough; do not wait for a full mix cycle. */
            return wave_is_trough_at(wave, index, UI_WAVE_TYPE_LACTATION);
        }
        break;
    case UI_WAVE_TYPE_MIX:
        if (to == UI_WAVE_TYPE_STIMULATION) {
            return wave_is_trough_at(wave, index, UI_WAVE_TYPE_MIX);
        }
        if (to == UI_WAVE_TYPE_LACTATION) {
            return wave_is_mix_wide_start(wave, index);
        }
        break;
    default:
        break;
    }

    return wave_is_trough_at(wave, index, from);
}

static int32_t wave_current_left_index(const ui_wave_t *wave)
{
    return wave->scroll_sample_index;
}

static bool wave_is_alignment_sample(const ui_wave_t *wave, int32_t index)
{
    if (wave_mod_pos(index, wave->narrow_period_px) == 0) {
        return true;
    }
    if (wave_mod_pos(index, wave->wide_period_px) == 0) {
        return true;
    }
    if (wave_mod_pos(index, wave_mix_cycle_len(wave)) == 0) {
        return true;
    }
    if (wave_is_mix_wide_start(wave, index)) {
        return true;
    }
    return false;
}

static void wave_log_switch_check(const ui_wave_t *wave, int32_t index, bool can_switch)
{
    const int32_t narrow_mod = wave_mod_pos(index, wave->narrow_period_px);
    const int32_t wide_mod = wave_mod_pos(index, wave->wide_period_px);
    const int32_t mix_pos = wave_mix_pos(wave, index);
    const int32_t short_block = (int32_t)wave->mix_short_count * wave->narrow_period_px;

    WAVE_SWITCH_LOG("wave sw chk cur=%d pend=%d idx=%d n%%=%d w%%=%d mix=%d/%d wide=%d can=%d",
        wave->cfg.type, wave->pending_type, (int)index, (int)narrow_mod, (int)wide_mod,
        (int)mix_pos, (int)short_block, (int)wave_is_mix_wide_start(wave, index), can_switch ? 1 : 0);
}

static void wave_realign_index_for_type(ui_wave_t *wave, ui_wave_type_e to, int32_t left_index)
{
    int32_t pos = 0;
    int32_t aligned = left_index;

    switch (to) {
    case UI_WAVE_TYPE_MIX:
        pos = wave_mix_pos(wave, left_index);
        aligned = left_index - pos;
        break;
    case UI_WAVE_TYPE_LACTATION:
        pos = wave_mod_pos(left_index, wave->wide_period_px);
        aligned = left_index - pos;
        break;
    case UI_WAVE_TYPE_STIMULATION:
    default:
        pos = wave_mod_pos(left_index, wave->narrow_period_px);
        aligned = left_index - pos;
        break;
    }

    if (aligned != wave->scroll_sample_index) {
        WAVE_SWITCH_LOG("wave realign to=%d idx %d -> %d phase=%d", to, (int)left_index,
            (int)aligned, (int)pos);
        wave->scroll_sample_index = aligned;
    }
}

static void wave_try_apply_pending_type(ui_wave_t *wave)
{
    if (wave == NULL || wave->has_pending_type == 0U) {
        return;
    }

    if (wave->pending_type == wave->cfg.type) {
        wave->has_pending_type = 0U;
        return;
    }

    const int32_t left_index = wave_current_left_index(wave);
    const bool can_switch = wave_can_switch_at(wave, wave->cfg.type, wave->pending_type, left_index);

#if UI_WAVE_SWITCH_LOG
    if (wave_is_alignment_sample(wave, left_index)) {
        wave_log_switch_check(wave, left_index, can_switch);
    }
#endif

    if (can_switch) {
        const ui_wave_type_e from_type = wave->cfg.type;
        const ui_wave_type_e to_type = wave->pending_type;

        WAVE_SWITCH_LOG("wave sw done %d->%d at idx=%d", from_type, to_type, (int)left_index);
        wave->cfg.type = to_type;
        wave_realign_index_for_type(wave, to_type, left_index);
        wave_rebuild_sample_lut(wave);
        wave->has_pending_type = 0U;
    }
}

static void wave_request_type(ui_wave_t *wave, ui_wave_type_e type, bool force_redraw)
{
    if (force_redraw) {
        wave->cfg.type = type;
        wave->pending_type = type;
        wave->has_pending_type = 0U;
        WAVE_SWITCH_LOG("wave req force type=%d scroll_idx=%d", type, (int)wave->scroll_sample_index);
        wave_redraw_full(wave);
        return;
    }

    if (wave->cfg.type == type) {
        wave->has_pending_type = 0U;
        return;
    }

    WAVE_SWITCH_LOG("wave req pending %d->%d scroll_idx=%d", wave->cfg.type, type,
        (int)wave->scroll_sample_index);
    wave->pending_type = type;
    wave->has_pending_type = 1U;
    wave_try_apply_pending_type(wave);
}

static void wave_apply_line_style(ui_wave_t *wave)
{
    lv_obj_set_style_line_width(wave->line, wave->cfg.line_width, 0);
    lv_obj_set_style_line_color(wave->line, lv_color_hex(wave->cfg.color), 0);
    lv_obj_set_style_line_opa(wave->line, LV_OPA_COVER, 0);
    lv_obj_set_style_line_rounded(wave->line, true, 0);
}

static void wave_apply_shadow_style(ui_wave_t *wave)
{
    if (wave->shadow_line == NULL) {
        return;
    }

    lv_obj_set_style_line_width(wave->shadow_line, wave->cfg.line_width, 0);
    lv_obj_set_style_line_color(wave->shadow_line, lv_color_hex(wave->cfg.shadow_color), 0);
    lv_obj_set_style_line_opa(wave->shadow_line, LV_OPA_COVER, 0);
    lv_obj_set_style_line_rounded(wave->shadow_line, true, 0);
}

static void wave_sync_shadow_points_full(ui_wave_t *wave)
{
    if (wave->shadow_line == NULL || wave->shadow_points == NULL || wave->points == NULL) {
        return;
    }

    const lv_coord_t max_y = (lv_coord_t)(wave->height - 1U);
    const lv_coord_t offset = (lv_coord_t)wave->cfg.shadow_y_offset;

    for (uint16_t x = 0; x < wave->width; x++) {
        wave->shadow_points[x].x = wave->points[x].x;
        lv_coord_t shadow_y = wave->points[x].y + offset;
        if (shadow_y > max_y) {
            shadow_y = max_y;
        }
        wave->shadow_points[x].y = shadow_y;
    }
}

static lv_coord_t wave_shadow_y_from_main(const ui_wave_t *wave, lv_coord_t main_y)
{
    const lv_coord_t max_y = (lv_coord_t)(wave->height - 1U);
    lv_coord_t shadow_y = main_y + (lv_coord_t)wave->cfg.shadow_y_offset;

    if (shadow_y > max_y) {
        shadow_y = max_y;
    }
    return shadow_y;
}

static void wave_scroll_shadow_step(ui_wave_t *wave)
{
    if (wave->shadow_line == NULL || wave->shadow_points == NULL || wave->points == NULL) {
        return;
    }

    const uint16_t last = wave->width - 1U;
    lv_point_precise_t *shadow_pts = wave->shadow_points;

    for (uint16_t x = last; x > 0U; x--) {
        shadow_pts[x].y = shadow_pts[x - 1].y;
    }
    shadow_pts[0].y = wave_shadow_y_from_main(wave, wave->points[0].y);
}

static void wave_bind_shadow_points(ui_wave_t *wave)
{
    if (wave->shadow_line == NULL || wave->shadow_points == NULL) {
        return;
    }

    if (wave->shadow_points_bound) {
        lv_obj_invalidate(wave->shadow_line);
        return;
    }

    lv_line_set_points_mutable(wave->shadow_line, wave->shadow_points, wave->width);
    wave->shadow_points_bound = 1U;
}

static void wave_bind_points(ui_wave_t *wave)
{
    if (wave->points_bound) {
        lv_obj_invalidate(wave->line);
        wave_sync_shadow_points_full(wave);
        wave_bind_shadow_points(wave);
        return;
    }

    lv_line_set_points_mutable(wave->line, wave->points, wave->width);
    wave->points_bound = 1U;
    wave_sync_shadow_points_full(wave);
    wave_bind_shadow_points(wave);
}

static void wave_init_points_x(ui_wave_t *wave)
{
    for (uint16_t x = 0; x < wave->width; x++) {
        wave->points[x].x = (lv_coord_t)x;
        if (wave->shadow_points != NULL) {
            wave->shadow_points[x].x = (lv_coord_t)x;
        }
    }
}

static bool wave_create_shadow_line(ui_wave_t *wave, lv_obj_t *parent)
{
    if (wave->cfg.shadow_enable == 0U) {
        return true;
    }

    if (wave->cfg.shadow_y_offset == 0) {
        wave->cfg.shadow_y_offset = UI_WAVE_SHADOW_Y_OFFSET_DEFAULT;
    }
    if (wave->cfg.shadow_color == 0U) {
        wave->cfg.shadow_color = UI_WAVE_SHADOW_COLOR_DEFAULT;
    }

    wave->shadow_points = (lv_point_precise_t *)u_malloz(sizeof(lv_point_precise_t) * wave->width);
    if (wave->shadow_points == NULL) {
        return false;
    }

    wave->shadow_line = lv_line_create(parent);
    if (wave->shadow_line == NULL) {
        u_free(wave->shadow_points);
        wave->shadow_points = NULL;
        return false;
    }

    lv_obj_set_size(wave->shadow_line, wave->width, wave->height);
    lv_obj_set_style_bg_opa(wave->shadow_line, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(wave->shadow_line, 0, 0);
    lv_obj_set_style_pad_all(wave->shadow_line, 0, 0);
    lv_obj_remove_flag(wave->shadow_line, LV_OBJ_FLAG_SCROLLABLE);
    wave_apply_shadow_style(wave);
    return true;
}

static void wave_setup_main_line(ui_wave_t *wave, lv_obj_t *parent)
{
    wave->line = lv_line_create(parent);
    lv_obj_set_size(wave->line, wave->width, wave->height);
    lv_obj_set_style_bg_opa(wave->line, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(wave->line, 0, 0);
    lv_obj_set_style_pad_all(wave->line, 0, 0);
    lv_obj_remove_flag(wave->line, LV_OBJ_FLAG_SCROLLABLE);
    wave_apply_line_style(wave);
}

static void wave_redraw_full(ui_wave_t *wave)
{
    if (wave == NULL || wave->line == NULL || wave->points == NULL || wave->width < 2U) {
        return;
    }

    wave_rebuild_sample_lut(wave);

    for (uint16_t x = 0; x < wave->width; x++) {
        wave->points[x].y = (lv_coord_t)wave_sample_y_at_index(wave, (int32_t)x);
    }

    wave->scroll_sample_index = (int32_t)wave->width;
    wave->has_pending_type = 0U;
    wave->pending_type = wave->cfg.type;
    wave_bind_points(wave);
}

static void wave_scroll_step(ui_wave_t *wave)
{
    if (wave == NULL || wave->line == NULL || wave->points == NULL || wave->width < 2U) {
        return;
    }

    wave_try_apply_pending_type(wave);

    const uint16_t last = wave->width - 1U;
    lv_point_precise_t *pts = wave->points;

    for (uint16_t x = last; x > 0U; x--) {
        pts[x].y = pts[x - 1].y;
    }
    pts[0].y = (lv_coord_t)wave_sample_y_at_index(wave, wave->scroll_sample_index);
    wave->scroll_sample_index++;
    wave_scroll_shadow_step(wave);
    lv_obj_invalidate(wave->line);
    if (wave->shadow_line != NULL) {
        lv_obj_invalidate(wave->shadow_line);
    }
}

static void wave_timer_cb(lv_timer_t *timer)
{
    ui_wave_t *wave = lv_timer_get_user_data(timer);
    if (wave == NULL) {
        return;
    }
    wave_scroll_step(wave);
}

static void wave_timer_start(ui_wave_t *wave)
{
    if (wave->cfg.anim_mode == UI_WAVE_ANIM_NONE) {
        return;
    }

    const uint32_t period = (wave->cfg.anim_period_ms == 0) ?
        UI_WAVE_DEFAULT_ANIM_PERIOD_MS : wave->cfg.anim_period_ms;

    if (wave->timer == NULL) {
        wave->timer = lv_timer_create(wave_timer_cb, period, wave);
    } else {
        lv_timer_set_period(wave->timer, period);
        lv_timer_resume(wave->timer);
    }
}

static void wave_timer_stop(ui_wave_t *wave)
{
    if (wave->timer != NULL) {
        lv_timer_pause(wave->timer);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_wave_cfg_init_default(ui_wave_cfg_t *cfg)
{
    if (cfg == NULL) {
        return;
    }

    lv_memzero(cfg, sizeof(*cfg));
    cfg->color = 0xFFFFFF;
    cfg->line_width = UI_WAVE_LINE_WIDTH_DEFAULT;
    cfg->type = UI_WAVE_TYPE_STIMULATION;
    cfg->anim_mode = UI_WAVE_ANIM_NONE;
    cfg->anim_period_ms = UI_WAVE_DEFAULT_ANIM_PERIOD_MS;
    cfg->shadow_enable = 0U;
    cfg->shadow_color = UI_WAVE_SHADOW_COLOR_DEFAULT;
    cfg->shadow_y_offset = UI_WAVE_SHADOW_Y_OFFSET_DEFAULT;
}

ui_wave_t *ui_wave_create(lv_obj_t *parent, const ui_wave_cfg_t *cfg)
{
    if (parent == NULL || cfg == NULL) {
        return NULL;
    }

    ui_wave_t *wave = (ui_wave_t *)u_malloz(sizeof(*wave));
    if (wave == NULL) {
        return NULL;
    }

    wave->cfg = *cfg;
    wave->width = UI_WAVE_WIDTH;
    wave->height = UI_WAVE_HEIGHT;
    wave->scroll_sample_index = 0;
    wave->points_bound = 0U;
    wave->shadow_points_bound = 0U;
    wave->has_pending_type = 0U;
    wave->pending_type = wave->cfg.type;
    if (wave->cfg.line_width == 0) {
        wave->cfg.line_width = UI_WAVE_LINE_WIDTH_DEFAULT;
    }
    wave_cache_metrics(wave);

    wave->points = (lv_point_precise_t *)u_malloz(sizeof(lv_point_precise_t) * wave->width);
    if (wave->points == NULL) {
        u_free(wave);
        return NULL;
    }

    if (!wave_create_shadow_line(wave, parent)) {
        u_free(wave->points);
        u_free(wave);
        return NULL;
    }

    wave_setup_main_line(wave, parent);
    if (wave->line == NULL) {
        if (wave->shadow_line != NULL) {
            lv_obj_delete(wave->shadow_line);
            wave->shadow_line = NULL;
        }
        u_free(wave->shadow_points);
        wave->shadow_points = NULL;
        u_free(wave->points);
        u_free(wave);
        return NULL;
    }

    wave_init_points_x(wave);
    wave_redraw_full(wave);
    wave_timer_start(wave);
    return wave;
}

void ui_wave_delete(ui_wave_t *wave)
{
    if (wave == NULL) {
        return;
    }

    if (wave->timer != NULL) {
        lv_timer_delete(wave->timer);
        wave->timer = NULL;
    }

    if (wave->line != NULL && lv_obj_is_valid(wave->line)) {
        lv_obj_delete(wave->line);
        wave->line = NULL;
    }

    if (wave->shadow_line != NULL && lv_obj_is_valid(wave->shadow_line)) {
        lv_obj_delete(wave->shadow_line);
        wave->shadow_line = NULL;
    }

    if (wave->points != NULL) {
        u_free(wave->points);
        wave->points = NULL;
    }

    if (wave->shadow_points != NULL) {
        u_free(wave->shadow_points);
        wave->shadow_points = NULL;
    }

    wave_free_sample_lut(wave);

    u_free(wave);
}

lv_obj_t *ui_wave_get_obj(const ui_wave_t *wave)
{
    return (wave != NULL) ? wave->line : NULL;
}

lv_obj_t *ui_wave_get_shadow_obj(const ui_wave_t *wave)
{
    return (wave != NULL) ? wave->shadow_line : NULL;
}

void ui_wave_set_type(ui_wave_t *wave, ui_wave_type_e type)
{
    if (wave == NULL) {
        return;
    }

    const bool force_redraw = (wave->cfg.anim_mode == UI_WAVE_ANIM_NONE);
    wave_request_type(wave, type, force_redraw);
}

void ui_wave_set_amplitude(ui_wave_t *wave, int16_t amplitude)
{
    if (wave == NULL) {
        return;
    }

    wave->cfg.amplitude = amplitude;
    wave_cache_metrics(wave);
    wave_rebuild_sample_lut(wave);
    wave_redraw_full(wave);
}

void ui_wave_set_narrow_period_px(ui_wave_t *wave, uint16_t period_x100)
{
    if (wave == NULL || period_x100 == 0) {
        return;
    }

    wave->cfg.period_px = (uint16_t)(period_x100 / 100U);
    if (wave->cfg.period_px < UI_WAVE_MIN_PERIOD) {
        wave->cfg.period_px = UI_WAVE_MIN_PERIOD;
    }
    wave_cache_metrics(wave);
    wave_rebuild_sample_lut(wave);
    wave_redraw_full(wave);
}

void ui_wave_set_color(ui_wave_t *wave, uint32_t color_hex)
{
    if (wave == NULL) {
        return;
    }

    wave->cfg.color = color_hex;
    wave_apply_line_style(wave);
    wave_redraw_full(wave);
}

void ui_wave_start_anim(ui_wave_t *wave, ui_wave_anim_mode_e mode)
{
    if (wave == NULL) {
        return;
    }

    wave->cfg.anim_mode = mode;
    wave_timer_start(wave);
}

void ui_wave_stop_anim(ui_wave_t *wave)
{
    if (wave == NULL) {
        return;
    }

    wave->cfg.anim_mode = UI_WAVE_ANIM_NONE;
    wave_timer_stop(wave);
}

void ui_wave_refresh(ui_wave_t *wave)
{
    if (wave != NULL) {
        wave_redraw_full(wave);
    }
}
