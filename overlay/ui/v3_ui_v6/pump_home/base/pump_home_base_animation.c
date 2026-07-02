/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "pump_home_base_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_ANIM_TRANSITION                 (1)
#define PUMP_HOME_STIMULATION_WAVE_Y               (60)
#define PUMP_HOME_LACTATION_WAVE_Y                 (60)
#define PUMP_HOME_MIX_WAVE_Y                       (60)
#define PUMP_HOME_RELAX_WAVE_Y                     (60)

#define PUMP_HOME_ANIM_FRAME_INTERVAL_MS           (1000 / PUMP_ANIMATION_FREQ)
#define PUMP_HOME_ANIM_FRAME_RES_INVALID           (0xFFFF)
#define IS_SUPPORT_PUMP_HOME_ANIM_STATIC_WAVE      (0)

#if IS_SUPPORT_ANIM_TRANSITION
#define PUMP_HOME_ANIM_TRANSITION_MS               (1000)
#endif

#if IS_SUPPORT_PUMP_HOME_ANIM_STATIC_WAVE
#define PUMP_HOME_ANIM_STATIC_FRAME_ID_APPLY(res_type_val, frame_id_var) \
    do { \
        const uint16_t static_frame_id = pump_home_animation_static_frame_id_get(res_type_val); \
        if (static_frame_id != 0) { \
            (frame_id_var) = static_frame_id; \
        } \
    } while (0)
#else
#define PUMP_HOME_ANIM_STATIC_FRAME_ID_APPLY(res_type_val, frame_id_var) \
    do { \
        (void)(res_type_val); \
        (void)(frame_id_var); \
    } while (0)
#endif

typedef enum {
    PUMP_HOME_ANIM_PHASE_PLAY = 0,
    PUMP_HOME_ANIM_PHASE_FADE_OUT,
    PUMP_HOME_ANIM_PHASE_FADE_IN,
} pump_home_anim_phase_e;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_home_animation_state_reset(pump_home_base_ui_info_t *info)
{
    pump_home_base_animation_info_t *anim = &info->animation;

    anim->wave_res_type = PUMP_HOME_BASE_ANIM_MAX;
    anim->current_anim_start = 0;
    anim->pending_anim_start = 0;
    anim->pending_anim_end = 0;
    anim->shown_frame_res_id = PUMP_HOME_ANIM_FRAME_RES_INVALID;
    anim->last_frame_tick_ms = 0;
    anim->anim_phase = PUMP_HOME_ANIM_PHASE_PLAY;
    anim->transition_start_ms = 0;
    anim->anim_play_state = 0;
}

static void pump_home_animation_wave_opa_set(pump_home_base_ui_info_t *info, lv_opa_t opa)
{
    if (info->animation.wave == NULL || !lv_obj_is_valid(info->animation.wave)) {
        return;
    }
    lv_obj_set_style_opa(info->animation.wave, opa, 0);
}

static bool pump_home_animation_res_get(uint8_t mode_type, uint16_t *start_id, uint16_t *end_id,
    pump_home_base_anim_res_type_e *res_type)
{
    switch (mode_type) {
        case PUMP_MODE_STIMULATION:
            *start_id = BASE_RES_ID_SHORT_SUCTION_ANIM_START;
            *end_id = BASE_RES_ID_SHORT_SUCTION_ANIM_END;
            *res_type = PUMP_HOME_BASE_ANIM_STIMULATION_WAVE;
            return true;
        case PUMP_MODE_LACTATION:
            *start_id = BASE_RES_ID_LONG_SUCTION_ANIM_START;
            *end_id = BASE_RES_ID_LONG_SUCTION_ANIM_END;
            *res_type = PUMP_HOME_BASE_ANIM_LACTATION_WAVE;
            return true;
        case PUMP_MODE_MIX:
            *start_id = BASE_RES_ID_MIXED_SUCTION_ANIM_START;
            *end_id = BASE_RES_ID_MIXED_SUCTION_ANIM_END;
            *res_type = PUMP_HOME_BASE_ANIM_MIX_WAVE;
            return true;
        case PUMP_MODE_IDLE:
            *start_id = BASE_RES_ID_RELAX_SUCTION_ANIM_START;
            *end_id = BASE_RES_ID_RELAX_SUCTION_ANIM_END;
            *res_type = PUMP_HOME_BASE_ANIM_RELAX_WAVE;
            return true;
        default:
            return false;
    }
}

static pump_home_base_anim_res_type_e pump_home_animation_res_type_get(uint16_t start_id)
{
    switch (start_id) {
        case BASE_RES_ID_SHORT_SUCTION_ANIM_START:
            return PUMP_HOME_BASE_ANIM_STIMULATION_WAVE;
        case BASE_RES_ID_LONG_SUCTION_ANIM_START:
            return PUMP_HOME_BASE_ANIM_LACTATION_WAVE;
        case BASE_RES_ID_MIXED_SUCTION_ANIM_START:
            return PUMP_HOME_BASE_ANIM_MIX_WAVE;
        case BASE_RES_ID_RELAX_SUCTION_ANIM_START:
            return PUMP_HOME_BASE_ANIM_RELAX_WAVE;
        default:
            return PUMP_HOME_BASE_ANIM_MAX;
    }
}

static lv_coord_t pump_home_animation_wave_y_get(uint8_t mode_type)
{
    #if 0
    switch (mode_type) {
        case PUMP_MODE_STIMULATION:
            return PUMP_HOME_STIMULATION_WAVE_Y;
        case PUMP_MODE_LACTATION:
            return PUMP_HOME_LACTATION_WAVE_Y;
        case PUMP_MODE_MIX:
            return PUMP_HOME_MIX_WAVE_Y;
        case PUMP_MODE_IDLE:
            return PUMP_HOME_RELAX_WAVE_Y;
        default:
            return PUMP_HOME_STIMULATION_WAVE_Y;
    }
    #endif
    return PUMP_HOME_STIMULATION_WAVE_Y;
}

static uint8_t pump_home_animation_mode_type_get(pump_home_base_anim_res_type_e res_type)
{
    switch (res_type) {
        case PUMP_HOME_BASE_ANIM_STIMULATION_WAVE:
            return PUMP_MODE_STIMULATION;
        case PUMP_HOME_BASE_ANIM_LACTATION_WAVE:
            return PUMP_MODE_LACTATION;
        case PUMP_HOME_BASE_ANIM_MIX_WAVE:
            return PUMP_MODE_MIX;
        case PUMP_HOME_BASE_ANIM_RELAX_WAVE:
            return PUMP_MODE_IDLE;
        default:
            return PUMP_MODE_STIMULATION;
    }
}

static void pump_home_animation_wave_align(pump_home_base_ui_info_t *info, uint8_t mode_type)
{
    if (info->animation.wave == NULL || !lv_obj_is_valid(info->animation.wave)) {
        return;
    }
    lv_obj_align(info->animation.wave, LV_ALIGN_TOP_MID, 0,
        pump_home_animation_wave_y_get(mode_type));
}

static void pump_home_animation_wave_hide(pump_home_base_ui_info_t *info)
{
    if (info->animation.wave == NULL || !lv_obj_is_valid(info->animation.wave)) {
        return;
    }
    ui_img_baj_release_decoded_buf(info->animation.wave);
    ui_img_resource_deinit(info->ui, &info->animation.wave_info);
    memset(&info->animation.wave_info, 0, sizeof(info->animation.wave_info));
    info->animation.shown_frame_res_id = PUMP_HOME_ANIM_FRAME_RES_INVALID;
    lv_obj_add_flag(info->animation.wave, LV_OBJ_FLAG_HIDDEN);
}

static void pump_home_animation_wave_prepare(pump_home_base_ui_info_t *info)
{
    if (info->animation.wave == NULL || !lv_obj_is_valid(info->animation.wave)) {
        info->animation.wave = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->animation.wave);
        lv_obj_set_style_opa(info->animation.wave, LV_OPA_COVER, 0);
    }
}

#if IS_SUPPORT_PUMP_HOME_ANIM_STATIC_WAVE
static uint16_t pump_home_animation_static_frame_id_get(pump_home_base_anim_res_type_e res_type)
{
    switch (res_type) {
        case PUMP_HOME_BASE_ANIM_STIMULATION_WAVE:
            return BASE_RES_ID_STIMULATION_WAVE;
        case PUMP_HOME_BASE_ANIM_LACTATION_WAVE:
            return BASE_RES_ID_LACTATION_WAVE;
        case PUMP_HOME_BASE_ANIM_MIX_WAVE:
            return BASE_RES_ID_MIX_WAVE;
        case PUMP_HOME_BASE_ANIM_RELAX_WAVE:
            return BASE_RES_ID_RELAX_WAVE;
        default:
            return 0;
    }
}
#endif

static void pump_home_animation_frame_show(pump_home_base_ui_info_t *info, uint16_t frame_id)
{
    pump_home_base_animation_info_t *anim = &info->animation;

    if (anim->shown_frame_res_id == frame_id) {
        return;
    }

    ui_img_baj_release_decoded_buf(anim->wave);
    if (anim->wave_info.data != NULL) {
        ui_img_resource_deinit(info->ui, &anim->wave_info);
        memset(&anim->wave_info, 0, sizeof(anim->wave_info));
    }

    ui_theme_img_resource_init(info->ui, frame_id, RES_THEME_TYPE_BASE, &anim->wave_info);
    EXCEPTION_CHECK_VOID(anim->wave_info.data);
    lv_fs_path_ex_t img_path;
    lv_fs_make_path_from_buffer(&img_path, LV_FS_MEMFS_LETTER,
        anim->wave_info.data, anim->wave_info.size, "baj");
    lv_image_set_src(anim->wave, (const void *)&img_path);
    anim->shown_frame_res_id = frame_id;
}

static void pump_home_animation_apply(pump_home_base_ui_info_t *info, uint16_t start_id, uint16_t end_id,
    pump_home_base_anim_res_type_e res_type)
{
    pump_home_base_animation_info_t *anim = &info->animation;
    const uint8_t mode_type = pump_home_animation_mode_type_get(res_type);

    pump_home_animation_wave_prepare(info);
    pump_home_animation_wave_align(info, mode_type);
    anim->wave_res_type = res_type;
    anim->current_anim_start = start_id;
    anim->frame_res_id = start_id;
    anim->frame_end_res_id = end_id;
    anim->shown_frame_res_id = PUMP_HOME_ANIM_FRAME_RES_INVALID;
    anim->last_frame_tick_ms = 0;
    PUMP_HOME_ANIM_STATIC_FRAME_ID_APPLY(res_type, anim->frame_res_id);
    pump_home_animation_frame_show(info, anim->frame_res_id);
    lv_obj_clear_flag(anim->wave, LV_OBJ_FLAG_HIDDEN);
}

#if IS_SUPPORT_ANIM_TRANSITION
static void pump_home_animation_transition_begin(pump_home_base_ui_info_t *info, uint16_t start_id,
    uint16_t end_id)
{
    pump_home_base_animation_info_t *anim = &info->animation;

    anim->pending_anim_start = start_id;
    anim->pending_anim_end = end_id;
    anim->anim_phase = PUMP_HOME_ANIM_PHASE_FADE_OUT;
    anim->transition_start_ms = get_timestamp();
    pump_home_animation_wave_prepare(info);
    lv_obj_clear_flag(anim->wave, LV_OBJ_FLAG_HIDDEN);
}

static bool pump_home_animation_transition_update(pump_home_base_ui_info_t *info)
{
    pump_home_base_animation_info_t *anim = &info->animation;
    const uint32_t elapsed = get_timestamp() - anim->transition_start_ms;
    const uint32_t half_time = PUMP_HOME_ANIM_TRANSITION_MS / 2U;

    if (anim->anim_phase == PUMP_HOME_ANIM_PHASE_FADE_OUT) {
        if (elapsed < half_time) {
            const lv_opa_t opa = (lv_opa_t)(LV_OPA_COVER -
                (LV_OPA_COVER * elapsed / half_time));
            pump_home_animation_wave_opa_set(info, opa);
            return true;
        }
        pump_home_animation_apply(info, anim->pending_anim_start, anim->pending_anim_end,
            pump_home_animation_res_type_get(anim->pending_anim_start));
        anim->anim_phase = PUMP_HOME_ANIM_PHASE_FADE_IN;
        anim->transition_start_ms = get_timestamp();
        pump_home_animation_wave_opa_set(info, LV_OPA_TRANSP);
        return true;
    }

    if (elapsed < half_time) {
        const lv_opa_t opa = (lv_opa_t)(LV_OPA_COVER * elapsed / half_time);
        pump_home_animation_wave_opa_set(info, opa);
        return true;
    }

    pump_home_animation_wave_opa_set(info, LV_OPA_COVER);
    anim->anim_phase = PUMP_HOME_ANIM_PHASE_PLAY;
    return false;
}
#endif

static void pump_home_animation_frame_update(pump_home_base_ui_info_t *info)
{
    pump_home_base_animation_info_t *anim = &info->animation;
    const uint32_t now = get_timestamp();

    if (anim->last_frame_tick_ms != 0 &&
        (now - anim->last_frame_tick_ms) < PUMP_HOME_ANIM_FRAME_INTERVAL_MS) {
        return;
    }
    anim->last_frame_tick_ms = now;

    if (anim->frame_res_id > anim->frame_end_res_id) {
        anim->frame_res_id = anim->current_anim_start;
    }

    PUMP_HOME_ANIM_STATIC_FRAME_ID_APPLY((pump_home_base_anim_res_type_e)anim->wave_res_type,
        anim->frame_res_id);

    pump_home_animation_frame_show(info, anim->frame_res_id);
    anim->frame_res_id++;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_home_base_animation_ui_remove(pump_home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_img_baj_release_decoded_buf(info->animation.wave);
    ui_img_resource_deinit(info->ui, &info->animation.wave_info);
    pump_home_animation_state_reset(info);
}

void pump_home_base_animation_ui_update(pump_home_base_ui_info_t *info)
{
    pump_home_base_animation_info_t *anim = &info->animation;
    const uint8_t mode_type = info->mode.new_mode_type;
    pump_home_base_anim_res_type_e res_type = PUMP_HOME_BASE_ANIM_MAX;
    uint16_t start_id = 0;
    uint16_t end_id = 0;

    EXCEPTION_CHECK_VOID(info);
    if (pump_home_animation_res_get(mode_type, &start_id, &end_id, &res_type) == false) {
        pump_home_animation_wave_hide(info);
        pump_home_animation_state_reset(info);
        anim->frame_res_id = 0;
        anim->frame_end_res_id = 0;
        return;
    }

    if (info->force_update || anim->wave_res_type >= PUMP_HOME_BASE_ANIM_MAX) {
        pump_home_animation_apply(info, start_id, end_id, res_type);
        pump_home_animation_wave_opa_set(info, LV_OPA_COVER);
        anim->anim_phase = PUMP_HOME_ANIM_PHASE_PLAY;
    } else if (res_type != anim->wave_res_type) {
#if IS_SUPPORT_ANIM_TRANSITION
        if (anim->anim_phase == PUMP_HOME_ANIM_PHASE_PLAY) {
            pump_home_animation_transition_begin(info, start_id, end_id);
        } else {
            anim->pending_anim_start = start_id;
            anim->pending_anim_end = end_id;
        }
#else
        pump_home_animation_apply(info, start_id, end_id, res_type);
        pump_home_animation_wave_opa_set(info, LV_OPA_COVER);
        anim->anim_phase = PUMP_HOME_ANIM_PHASE_PLAY;
#endif
    }

#if IS_SUPPORT_ANIM_TRANSITION
    if (anim->anim_phase != PUMP_HOME_ANIM_PHASE_PLAY) {
        if (anim->anim_play_state != 0) {
            return;
        }
        (void)pump_home_animation_transition_update(info);
        return;
    }
#endif

    if (anim->anim_play_state != 0) {
        return;
    }

    pump_home_animation_frame_update(info);
}
