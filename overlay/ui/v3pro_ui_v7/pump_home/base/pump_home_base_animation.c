/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ui_img_resource.h" // IWYU pragma: keep
#include "pump_home_base_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_HOME_WAVE_Y                               (190)
#define PUMP_HOME_WAVE_PAUSE_OFFSET_Y                  (46)
#define PUMP_HOME_SEAL_ANIM_Y                          (115)
#define PUMP_HOME_ANIM_FRAME_INTERVAL_MS               (1000 / PUMP_ANIMATION_FREQ)
#define PUMP_HOME_ANIM_FRAME_RES_INVALID               (0xFFFF)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_home_animation_state_reset(pump_home_base_ui_info_t *info)
{
    pump_home_base_animation_info_t *anim = &info->animation;

    anim->wave_res_type = PUMP_HOME_BASE_ANIM_MAX;
    anim->current_anim_start = 0;
    anim->shown_frame_res_id = PUMP_HOME_ANIM_FRAME_RES_INVALID;
    anim->last_frame_tick_ms = 0;
    anim->frame_res_id = 0;
    anim->frame_end_res_id = 0;
    anim->anim_loop = 1;
}

static bool pump_home_animation_res_get(uint8_t mode_index, uint8_t mode_type, uint16_t *start_id, uint16_t *end_id,
    pump_home_base_anim_res_type_e *res_type)
{
    switch (mode_type) {
        case PUMP_MODE_STIMULATION:
            if (PUMP_IS_COMMON_MODE(mode_index)) {
                *start_id = BASE_RES_ID_STIMULATION_ANIM_START;
                *end_id = BASE_RES_ID_STIMULATION_ANIM_END;
                *res_type = PUMP_HOME_BASE_ANIM_STIMULATION_WAVE;
            } else {
                *start_id = BASE_RES_ID_PROGRAM_STIMULATION_ANIM_START;
                *end_id = BASE_RES_ID_PROGRAM_STIMULATION_ANIM_END;
                *res_type = PUMP_HOME_BASE_ANIM_PROGRAM_STIMULATION_WAVE;
            }
            return true;
        case PUMP_MODE_LACTATION:
            if (PUMP_IS_COMMON_MODE(mode_index)) {
                *start_id = BASE_RES_ID_EXPRESSION_ANIM_START;
                *end_id = BASE_RES_ID_EXPRESSION_ANIM_END;
                *res_type = PUMP_HOME_BASE_ANIM_EXPRESSION_WAVE;
            } else {
                *start_id = BASE_RES_ID_PROGRAM_EXPRESSION_ANIM_START;
                *end_id = BASE_RES_ID_PROGRAM_EXPRESSION_ANIM_END;
                *res_type = PUMP_HOME_BASE_ANIM_PROGRAM_EXPRESSION_WAVE;
            }
            return true;
        case PUMP_MODE_MIX:
            *start_id = BASE_RES_ID_STIMULATION_ANIM_START;
            *end_id = BASE_RES_ID_STIMULATION_ANIM_END;
            *res_type = PUMP_HOME_BASE_ANIM_MIX_WAVE;
            return true;
        case PUMP_MODE_IDLE:
            *start_id = BASE_RES_ID_PROGRAM_RELAX_ANIM_START;
            *end_id = BASE_RES_ID_PROGRAM_RELAX_ANIM_END;
            *res_type = PUMP_HOME_BASE_ANIM_PROGRAM_RELAX_WAVE;
            return true;
        default:
            return false;
    }
}

static bool pump_home_animation_is_seal(const pump_home_base_animation_info_t *anim)
{
    return anim->seal_tip == PUMP_SEAL_TIP_CHECKING ||
        anim->seal_tip == PUMP_SEAL_TIP_RESULT ||
        anim->seal_tip == PUMP_SEAL_TIP_RESULT_FAIL;
}

static bool pump_home_animation_should_hide(const pump_home_base_animation_info_t *anim)
{
    return anim->hidden_by_mode_switch && !pump_home_animation_is_seal(anim);
}

static bool pump_home_animation_target_get(const pump_home_base_ui_info_t *info,
    uint16_t *start_id, uint16_t *end_id, pump_home_base_anim_res_type_e *res_type, uint8_t *loop)
{
    *loop = 1;
    if (info->animation.seal_tip == PUMP_SEAL_TIP_CHECKING) {
        *start_id = BASE_RES_ID_CHECKING_SEAL_ANIM_START;
        *end_id = BASE_RES_ID_CHECKING_SEAL_ANIM_END;
        *res_type = PUMP_HOME_BASE_ANIM_CHECKING_SEAL;
        *loop = 1;
        return true;
    }
    if (info->animation.seal_tip == PUMP_SEAL_TIP_RESULT) {
        *start_id = BASE_RES_ID_CHECK_PASSED_ANIM_START;
        *end_id = BASE_RES_ID_CHECK_PASSED_ANIM_END;
        *res_type = PUMP_HOME_BASE_ANIM_CHECK_PASSED;
        *loop = 0;
        return true;
    }
    if (info->animation.seal_tip == PUMP_SEAL_TIP_RESULT_FAIL) {
        *start_id = BASE_RES_ID_CHECK_FAILED_ANIM_START;
        *end_id = BASE_RES_ID_CHECK_FAILED_ANIM_END;
        *res_type = PUMP_HOME_BASE_ANIM_CHECK_FAILED;
        *loop = 0;
        return true;
    }
    return pump_home_animation_res_get(info->mode.new_index, info->mode.new_mode_type,
        start_id, end_id, res_type);
}

static void pump_home_animation_wave_align(pump_home_base_ui_info_t *info)
{
    lv_coord_t y = PUMP_HOME_WAVE_Y;
    lv_opa_t opa = LV_OPA_COVER;

    if (info->animation.wave == NULL || !lv_obj_is_valid(info->animation.wave)) {
        return;
    }
    if (pump_home_animation_is_seal(&info->animation)) {
        y = PUMP_HOME_SEAL_ANIM_Y;
    } else if (info->animation.anim_play_state != 0) {
        y = (lv_coord_t)(PUMP_HOME_WAVE_Y - PUMP_HOME_WAVE_PAUSE_OFFSET_Y);
        opa = LV_OPA_60;
    }
    lv_obj_align(info->animation.wave, LV_ALIGN_TOP_MID, 0, y);
    lv_obj_set_style_opa(info->animation.wave, opa, 0);
}

static void pump_home_animation_wave_hide(pump_home_base_ui_info_t *info)
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

static void pump_home_animation_wave_prepare(pump_home_base_ui_info_t *info)
{
    if (info->animation.wave == NULL || !lv_obj_is_valid(info->animation.wave)) {
        info->animation.wave = lv_image_create(pump_home_page_parent_get(info));
        EXCEPTION_CHECK_VOID(info->animation.wave);
        lv_obj_set_style_opa(info->animation.wave, LV_OPA_COVER, 0);
    }
}

static void pump_home_animation_frame_show(pump_home_base_ui_info_t *info, uint16_t frame_id)
{
    pump_home_base_animation_info_t *anim = &info->animation;

    if (anim->shown_frame_res_id == frame_id) {
        return;
    }

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

static void pump_home_animation_apply(pump_home_base_ui_info_t *info, uint16_t start_id,
    uint16_t end_id, pump_home_base_anim_res_type_e res_type, uint8_t loop)
{
    pump_home_base_animation_info_t *anim = &info->animation;

    pump_home_animation_wave_prepare(info);
    pump_home_animation_wave_align(info);
    anim->wave_res_type = res_type;
    anim->current_anim_start = start_id;
    anim->frame_res_id = start_id;
    anim->frame_end_res_id = end_id;
    anim->shown_frame_res_id = PUMP_HOME_ANIM_FRAME_RES_INVALID;
    anim->last_frame_tick_ms = 0;
    anim->anim_loop = loop;
    pump_home_animation_frame_show(info, anim->frame_res_id);
    if (pump_home_animation_should_hide(anim)) {
        lv_obj_add_flag(anim->wave, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(anim->wave, LV_OBJ_FLAG_HIDDEN);
    }
}

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
        if (anim->anim_loop == 0) {
            return;
        }
        anim->frame_res_id = anim->current_anim_start;
    }

    pump_home_animation_frame_show(info, anim->frame_res_id);
    anim->frame_res_id++;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_home_base_animation_ui_remove(pump_home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
    if (info->animation.wave_info.data != NULL) {
        ui_img_resource_deinit(info->ui, &info->animation.wave_info);
    }
    pump_home_animation_state_reset(info);
    info->animation.wave = NULL;
    info->animation.anim_play_state = 0;
    info->animation.paused_by_transition = 0;
    info->animation.hidden_by_mode_switch = 0;
    info->animation.seal_tip = PUMP_SEAL_TIP_NONE;
    info->animation.anim_loop = 1;
}

void pump_home_base_animation_pause(pump_home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    info->animation.paused_by_transition = 1;
}

void pump_home_base_animation_resume(pump_home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    info->animation.paused_by_transition = 0;
    info->animation.last_frame_tick_ms = get_timestamp();
}

void pump_home_base_animation_ui_update(pump_home_base_ui_info_t *info)
{
    pump_home_base_animation_info_t *anim = &info->animation;
    pump_home_base_anim_res_type_e res_type = PUMP_HOME_BASE_ANIM_MAX;
    uint16_t start_id = 0;
    uint16_t end_id = 0;
    uint8_t loop = 1;

    EXCEPTION_CHECK_VOID(info);
    if (pump_home_animation_target_get(info, &start_id, &end_id, &res_type, &loop) == false) {
        pump_home_animation_wave_hide(info);
        pump_home_animation_state_reset(info);
        return;
    }

    if (info->force_update || anim->wave_res_type >= PUMP_HOME_BASE_ANIM_MAX) {
        pump_home_animation_apply(info, start_id, end_id, res_type, loop);
    } else if (res_type != anim->wave_res_type) {
        pump_home_animation_apply(info, start_id, end_id, res_type, loop);
    } else {
        pump_home_animation_wave_align(info);
    }

    if (pump_home_animation_should_hide(anim)) {
        if (anim->wave != NULL && lv_obj_is_valid(anim->wave)) {
            lv_obj_add_flag(anim->wave, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    if (anim->wave != NULL && lv_obj_is_valid(anim->wave)) {
        lv_obj_clear_flag(anim->wave, LV_OBJ_FLAG_HIDDEN);
    }

    if (anim->anim_play_state != 0 || anim->paused_by_transition != 0) {
        return;
    }

    pump_home_animation_frame_update(info);
}
