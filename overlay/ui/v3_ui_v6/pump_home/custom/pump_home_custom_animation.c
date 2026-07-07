/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ui_img_resource.h" // IWYU pragma: keep
#include "pump_home_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_ANIM_TRANSITION                     (0) /// 动画过渡支持.
#define PUMP_HOME_IP_ANIM_Y                            (206)
#define PUMP_HOME_ANIM_FRAME_INTERVAL_MS               (1000 / PUMP_ANIMATION_FREQ)
#define PUMP_HOME_ANIM_FRAME_RES_INVALID               (0xFFFF)

#if IS_SUPPORT_ANIM_TRANSITION
#define PUMP_HOME_ANIM_TRANSITION_MS                   (500)
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
static void pump_home_animation_state_reset(pump_home_custom_ui_info_t *info)
{
    pump_home_custom_animation_info_t *anim = &info->animation;

    anim->wave_res_type = PUMP_HOME_CUSTOM_ANIM_MAX;
    anim->anim_mode_type = PUMP_MODE_UNKNOWN;
    anim->current_anim_start = 0;
    anim->pending_anim_start = 0;
    anim->pending_anim_end = 0;
    anim->shown_frame_res_id = PUMP_HOME_ANIM_FRAME_RES_INVALID;
    anim->last_frame_tick_ms = 0;
    anim->anim_phase = PUMP_HOME_ANIM_PHASE_PLAY;
    anim->transition_start_ms = 0;
    anim->anim_play_state = 0;
}

static void pump_home_animation_wave_opa_set(pump_home_custom_ui_info_t *info, lv_opa_t opa)
{
    if (info->animation.wave == NULL || !lv_obj_is_valid(info->animation.wave)) {
        return;
    }
    lv_obj_set_style_opa(info->animation.wave, opa, 0);
}

static pump_home_custom_anim_res_type_e pump_home_animation_res_type_get(uint16_t start_id)
{
    switch (start_id) {
        case CUSTOM_RES_ID_SHAKE_BOTTLE_ANIM_IP_START:
            return PUMP_HOME_CUSTOM_ANIM_SHAKE_BOTTLE;
        case CUSTOM_RES_ID_YAWN_ANIM_IP_START:
            return PUMP_HOME_CUSTOM_ANIM_YAWN;
        case CUSTOM_RES_ID_SUCTION_ANIM_IP_START:
            return PUMP_HOME_CUSTOM_ANIM_SUCTION;
        default:
            return PUMP_HOME_CUSTOM_ANIM_MAX;
    }
}

static void pump_home_animation_mode_entry_get(uint8_t mode_type, uint16_t *start_id, uint16_t *end_id)
{
    if (mode_type == PUMP_MODE_STIMULATION || mode_type == PUMP_MODE_MIX) {
        *start_id = CUSTOM_RES_ID_SHAKE_BOTTLE_ANIM_IP_START;
        *end_id = CUSTOM_RES_ID_SHAKE_BOTTLE_ANIM_IP_END;
    } else {
        *start_id = CUSTOM_RES_ID_YAWN_ANIM_IP_START;
        *end_id = CUSTOM_RES_ID_YAWN_ANIM_IP_END;
    }
}

static bool pump_home_animation_next_res_select(pump_home_custom_animation_info_t *anim, uint8_t mode_type,
    uint16_t *start_id, uint16_t *end_id, pump_home_custom_anim_res_type_e *res_type)
{
    if (anim->frame_res_id <= anim->frame_end_res_id &&
        anim->anim_mode_type != PUMP_MODE_UNKNOWN) {
        return false;
    }

    if (mode_type == PUMP_MODE_STIMULATION) {
        if (anim->anim_mode_type != mode_type) {
            anim->anim_mode_type = mode_type;
            *start_id = CUSTOM_RES_ID_SHAKE_BOTTLE_ANIM_IP_START;
            *end_id = CUSTOM_RES_ID_SHAKE_BOTTLE_ANIM_IP_END;
            *res_type = PUMP_HOME_CUSTOM_ANIM_SHAKE_BOTTLE;
        } else {
            *start_id = CUSTOM_RES_ID_SUCTION_ANIM_IP_START;
            *end_id = CUSTOM_RES_ID_SUCTION_ANIM_IP_END;
            *res_type = PUMP_HOME_CUSTOM_ANIM_SUCTION;
        }
    } else if (mode_type == PUMP_MODE_LACTATION) {
        if (anim->anim_mode_type != mode_type) {
            anim->anim_mode_type = mode_type;
            *start_id = CUSTOM_RES_ID_YAWN_ANIM_IP_START;
            *end_id = CUSTOM_RES_ID_YAWN_ANIM_IP_END;
            *res_type = PUMP_HOME_CUSTOM_ANIM_YAWN;
        } else {
            *start_id = CUSTOM_RES_ID_SUCTION_ANIM_IP_START;
            *end_id = CUSTOM_RES_ID_SUCTION_ANIM_IP_END;
            *res_type = PUMP_HOME_CUSTOM_ANIM_SUCTION;
        }
    } else if (mode_type == PUMP_MODE_MIX) {
        anim->anim_mode_type = mode_type;
        *start_id = CUSTOM_RES_ID_SHAKE_BOTTLE_ANIM_IP_START;
        *end_id = CUSTOM_RES_ID_SHAKE_BOTTLE_ANIM_IP_END;
        *res_type = PUMP_HOME_CUSTOM_ANIM_SHAKE_BOTTLE;
    } else {
        anim->anim_mode_type = mode_type;
        *start_id = CUSTOM_RES_ID_YAWN_ANIM_IP_START;
        *end_id = CUSTOM_RES_ID_YAWN_ANIM_IP_END;
        *res_type = PUMP_HOME_CUSTOM_ANIM_YAWN;
    }
    return true;
}

static void pump_home_animation_wave_align(pump_home_custom_ui_info_t *info)
{
    if (info->animation.wave == NULL || !lv_obj_is_valid(info->animation.wave)) {
        return;
    }
    lv_obj_align(info->animation.wave, LV_ALIGN_TOP_MID, 0, PUMP_HOME_IP_ANIM_Y);
}

static void pump_home_animation_wave_hide(pump_home_custom_ui_info_t *info)
{
    if (info->animation.wave == NULL || !lv_obj_is_valid(info->animation.wave)) {
        return;
    }
    if (info->animation.wave_info.data != NULL) {
        ui_img_resource_deinit(info->ui, &info->animation.wave_info);
        memset(&info->animation.wave_info, 0, sizeof(info->animation.wave_info));
    }
    info->animation.shown_frame_res_id = PUMP_HOME_ANIM_FRAME_RES_INVALID;
    lv_obj_add_flag(info->animation.wave, LV_OBJ_FLAG_HIDDEN);
}

static void pump_home_animation_wave_prepare(pump_home_custom_ui_info_t *info)
{
    if (info->animation.wave == NULL || !lv_obj_is_valid(info->animation.wave)) {
        info->animation.wave = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->animation.wave);
        lv_obj_set_style_opa(info->animation.wave, LV_OPA_COVER, 0);
        pump_home_animation_wave_align(info);
    }
}

static void pump_home_animation_frame_show(pump_home_custom_ui_info_t *info, uint16_t frame_id)
{
    pump_home_custom_animation_info_t *anim = &info->animation;
    ui_manager_context_t *thiz = info->ui;

    if (anim->shown_frame_res_id == frame_id) {
        return;
    }

    if (anim->wave_info.data != NULL) {
        ui_img_resource_deinit(thiz, &anim->wave_info);
        memset(&anim->wave_info, 0, sizeof(anim->wave_info));
    }
    ui_theme_img_resource_init(thiz, frame_id, RES_THEME_TYPE_CUSTOM, &anim->wave_info);
    if (anim->wave_info.data == NULL) {
        return;
    }

    lv_fs_path_ex_t animation_img_path;
    lv_fs_make_path_from_buffer(&animation_img_path, LV_FS_MEMFS_LETTER,
        anim->wave_info.data, anim->wave_info.size, "baj");
    lv_image_set_src(anim->wave, (const void *)&animation_img_path);
    anim->shown_frame_res_id = frame_id;
}

static void pump_home_animation_apply(pump_home_custom_ui_info_t *info, uint16_t start_id, uint16_t end_id,
    pump_home_custom_anim_res_type_e res_type)
{
    pump_home_custom_animation_info_t *anim = &info->animation;

    pump_home_animation_wave_prepare(info);
    pump_home_animation_wave_align(info);
    anim->wave_res_type = res_type;
    anim->current_anim_start = start_id;
    anim->frame_res_id = start_id;
    anim->frame_end_res_id = end_id;
    anim->shown_frame_res_id = PUMP_HOME_ANIM_FRAME_RES_INVALID;
    anim->last_frame_tick_ms = 0;
    pump_home_animation_frame_show(info, anim->frame_res_id);
    lv_obj_clear_flag(anim->wave, LV_OBJ_FLAG_HIDDEN);
}

#if IS_SUPPORT_ANIM_TRANSITION
static void pump_home_animation_transition_begin(pump_home_custom_ui_info_t *info, uint16_t start_id,
    uint16_t end_id)
{
    pump_home_custom_animation_info_t *anim = &info->animation;

    anim->pending_anim_start = start_id;
    anim->pending_anim_end = end_id;
    anim->anim_phase = PUMP_HOME_ANIM_PHASE_FADE_OUT;
    anim->transition_start_ms = get_timestamp();
    pump_home_animation_wave_prepare(info);
    lv_obj_clear_flag(anim->wave, LV_OBJ_FLAG_HIDDEN);
}

static bool pump_home_animation_transition_update(pump_home_custom_ui_info_t *info)
{
    pump_home_custom_animation_info_t *anim = &info->animation;
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

static void pump_home_animation_frame_update(pump_home_custom_ui_info_t *info, uint8_t mode_type)
{
    pump_home_custom_animation_info_t *anim = &info->animation;
    const uint32_t now = get_timestamp();
    uint16_t start_id = 0;
    uint16_t end_id = 0;
    pump_home_custom_anim_res_type_e res_type = PUMP_HOME_CUSTOM_ANIM_MAX;

    if (anim->last_frame_tick_ms != 0 &&
        (now - anim->last_frame_tick_ms) < PUMP_HOME_ANIM_FRAME_INTERVAL_MS) {
        return;
    }
    anim->last_frame_tick_ms = now;

    if (pump_home_animation_next_res_select(anim, mode_type, &start_id, &end_id, &res_type)) {
        pump_home_animation_apply(info, start_id, end_id, res_type);
        return;
    }

    pump_home_animation_frame_show(info, anim->frame_res_id);
    anim->frame_res_id++;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_home_custom_animation_ui_remove(pump_home_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
    if (info->animation.wave_info.data != NULL) {
        ui_img_resource_deinit(info->ui, &info->animation.wave_info);
    }
    pump_home_animation_state_reset(info);
}

void pump_home_custom_animation_ui_update(pump_home_custom_ui_info_t *info)
{
    pump_home_custom_animation_info_t *anim = &info->animation;
    const uint8_t mode_type = info->mode.new_mode_type;
    uint16_t start_id = 0;
    uint16_t end_id = 0;
    pump_home_custom_anim_res_type_e res_type = PUMP_HOME_CUSTOM_ANIM_MAX;

    EXCEPTION_CHECK_VOID(info);

    if (info->force_update || anim->anim_mode_type == PUMP_MODE_UNKNOWN) {
        anim->frame_res_id = anim->frame_end_res_id + 1;
        if (pump_home_animation_next_res_select(anim, mode_type, &start_id, &end_id, &res_type)) {
            pump_home_animation_apply(info, start_id, end_id, res_type);
            pump_home_animation_wave_opa_set(info, LV_OPA_COVER);
            anim->anim_phase = PUMP_HOME_ANIM_PHASE_PLAY;
        }
    } else if (anim->anim_mode_type != mode_type && anim->wave_res_type < PUMP_HOME_CUSTOM_ANIM_MAX) {
        pump_home_animation_mode_entry_get(mode_type, &start_id, &end_id);
        anim->anim_mode_type = mode_type;
#if IS_SUPPORT_ANIM_TRANSITION
        if (anim->anim_phase == PUMP_HOME_ANIM_PHASE_PLAY) {
            pump_home_animation_transition_begin(info, start_id, end_id);
        } else {
            anim->pending_anim_start = start_id;
            anim->pending_anim_end = end_id;
        }
#else
        pump_home_animation_apply(info, start_id, end_id,
            pump_home_animation_res_type_get(start_id));
        pump_home_animation_wave_opa_set(info, LV_OPA_COVER);
        anim->anim_phase = PUMP_HOME_ANIM_PHASE_PLAY;
#endif
    }

    if (anim->wave_res_type >= PUMP_HOME_CUSTOM_ANIM_MAX) {
        return;
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

    pump_home_animation_frame_update(info, mode_type);
}
