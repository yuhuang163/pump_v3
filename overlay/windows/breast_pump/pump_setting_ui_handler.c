/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "ui_img_define.h"
#include "window_screen_pump.h"
#include "breast_pump_manager.h" 

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_LEVEL_DISPLAY_TASK_NAME       "pump_level_display_cmp"
#define PUMP_SPEED_DISPLAY_TASK_NAME       "pump_speed_display_cmp"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
/// 泵奶力度UI动画资源ID.
static const uint16_t s_pump_left_level_res_array[1] = {
    UI_IMG_ID_LEFT_LEVEL_ADJUST_DISPLAY,
};

static const uint16_t s_pump_right_level_res_array[1] = {
    UI_IMG_ID_RIGHT_LEVEL_ADJUST_DISPLAY,
};

static const uint16_t s_pump_bilateral_level_res_array[1] = {
    UI_IMG_ID_HOME_SCREEN,
};

/// 泵奶速度UI动画资源ID.
static const uint16_t s_pump_speed_res_array[1] = {
    UI_IMG_ID_PUMP_SPEED,
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_level_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_PUMP) == 0) {
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_LEFT_LEVEL || proc->current_ui_type == PUMP_UI_TYPE_RIGHT_LEVEL) {
        show_pump_home_ui(proc);
    }
}

static void pump_level_ui_policy(ui_manager_context_t *ui)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMP_SUB_UI_DISPLAY_TIME) {
        POST_TASK_HANDLER_DELAYED_ARG1(pump_level_display_completed_cb, 10, proc,
            PUMP_LEVEL_DISPLAY_TASK_NAME);
    } else {
        if (proc->display_progress != proc->ui_progress) {
            proc->display_progress++;
            if (ui->priv.play_info.reset_play_count_cb) {
                ui->priv.play_info.reset_play_count_cb(&ui->priv.play_info);
            }
        }
    }
}

static void pump_level_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            pump_level_ui_policy(ui);
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            break;
        default:
            break;
    }
}

static void show_pump_level_ui(ui_proc_pump_t *proc, pump_ui_type_e ui_type)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = "pump_level_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .res_count = 1,
        .progress_cb = pump_level_animation_progress_cb,
        .user_data = proc,
    };

    /// 根据UI类型设置资源数组.
    if (ui_type == PUMP_UI_TYPE_LEFT_LEVEL) {
        play_info.res_array = (uint16_t *)s_pump_left_level_res_array;
    } else if (ui_type == PUMP_UI_TYPE_RIGHT_LEVEL) {
        play_info.res_array = (uint16_t *)s_pump_right_level_res_array;
    } else {
        play_info.res_array = (uint16_t *)s_pump_bilateral_level_res_array;
    }
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = ui_type;
}

static void pump_speed_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_PUMP) == 0) {
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_SPEED) {
        show_pump_home_ui(proc);
    }
}

static void pump_speed_ui_policy(ui_manager_context_t *ui)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMP_SUB_UI_DISPLAY_TIME) {
        POST_TASK_HANDLER_DELAYED_ARG1(pump_speed_display_completed_cb, 10, proc,
            PUMP_SPEED_DISPLAY_TASK_NAME);
    } 
}

static void pump_speed_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_animation_play_t *ani = &ui->priv.play_info;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            pump_speed_ui_policy(ui);
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            break;
        default:
            break;
    }
}

static void show_pump_speed_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = "pump_speed_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .res_array = (uint16_t *)s_pump_speed_res_array,
        .res_count = LENGTH_OF_ARRAY(s_pump_speed_res_array),
        .progress_cb = pump_speed_animation_progress_cb,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = PUMP_UI_TYPE_SPEED;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int switch_pump_control_type_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump, uint8_t control_type)
{
    if (control_type >= BREAST_PUMP_CONTROL_TYPE_MAX) {
        LOGE("switch pump control type: %d, out of range", control_type);
        return -1;
    }

    struct storage_manager_context *st = pump->priv.st;
    EXCEPTION_CHECK_INT(st);
    if (proc->info.control_type != control_type) {
        proc->info.control_type = control_type;
        st->put_store_ram_val(STORE_ID_PUMP_CONTROL_TYPE, 0, 0, &proc->info.control_type);
    }

    peripheral_bus_context_t *pbus = pump->priv.pbus;
    EXCEPTION_CHECK_INT(pbus);
    rt_device_node_t *led_dev = pbus->find_device(ID_DEVICE_LED);
    EXCEPTION_CHECK_INT(led_dev);
    led_status_t led_status = {0};
    int args[2] = {0, (int)&led_status};

    if (control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) {
        args[0] = PUMP_RIGHT_CONTROL_BL;
        led_status.mode = LED_MODE_LEVEL;
        led_status.un.luminous.level = LED_LEVEL_MAX / 2;
        led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)args);
        args[0] = PUMP_LEFT_CONTROL_BL;
        led_status.mode = LED_MODE_BLINK;
        led_status.un.blink.period = 500;
        led_status.un.blink.continue_time = 3000;
        led_status.un.blink.first_status = LED_STATUS_ON;
        led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)args);
    } else if (control_type == BREAST_PUMP_CONTROL_TYPE_RIGHT) {
        args[0] = PUMP_LEFT_CONTROL_BL;
        led_status.mode = LED_MODE_LEVEL;
        led_status.un.luminous.level = LED_LEVEL_MAX / 2;
        led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)args);
        args[0] = PUMP_RIGHT_CONTROL_BL;
        led_status.mode = LED_MODE_BLINK;
        led_status.un.blink.period = 500;
        led_status.un.blink.continue_time = 3000;
        led_status.un.blink.first_status = LED_STATUS_ON;
        led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)args);
    } else {
        led_status.mode = LED_MODE_LEVEL;
        led_status.un.luminous.level = LED_LEVEL_MAX;
        args[0] = PUMP_RIGHT_CONTROL_BL;
        led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)args);
        args[0] = PUMP_LEFT_CONTROL_BL;
        led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)args);
    }
    
    if (control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) {
        show_pump_level_ui(proc, PUMP_UI_TYPE_LEFT_LEVEL);
    } else if (control_type == BREAST_PUMP_CONTROL_TYPE_RIGHT) {
        show_pump_level_ui(proc, PUMP_UI_TYPE_RIGHT_LEVEL);
    }
    LOG_UI("switch pump control type: %d, led status: %d", control_type, led_status.un.luminous.level);
    return 0;
}

int switch_pump_frequency_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump,
                        breast_pump_info_t *pump_info)
{
    struct storage_manager_context *st = pump->priv.st;
    EXCEPTION_CHECK_INT(st);

    if (!BREAST_PUMP_IS_COMMON_MODE(pump_info->mode)) {
        LOG_UI("pump is not common mode, ignore.");
        return 0;
    }
    pump->get_breast_pump_setting(&proc->setting);
    if (proc->setting.speed < BREAST_PUMP_SPEED_HIGH) {
        proc->setting.speed++;
    } else {
        proc->setting.speed = BREAST_PUMP_SPEED_LOW;
    }
    pump->set_breast_pump_setting(&proc->setting);
    st->put_store_ram_val(STORE_ID_PUMP_SETTING_INFO, pump_info->mode, 0, &proc->setting);
    show_pump_speed_ui(proc);
    LOG_UI("switch pump frequency: %d, mode: %s", proc->setting.speed,
         pump->get_breast_pump_mode_name(pump_info->mode));

    return 0;
}

int switch_pump_level_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump,
                    breast_pump_info_t *pump_info, uint8_t is_left)
{
    storage_manager_context_t *st = pump->priv.st;
    EXCEPTION_CHECK_INT(st);
    uint32_t ui_type = 0;
    pump->get_breast_pump_setting(&proc->setting);
    if (proc->info.control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) {
        if (is_left) {
            if (proc->setting.level[0] > 0) {
                proc->setting.level[0]--;
            }
        } else {
            if (proc->setting.level[0] < BREAST_PUMP_LEVEL_15) {
                proc->setting.level[0]++;
            }
        }
        ui_type = PUMP_UI_TYPE_LEFT_LEVEL;
    } else if (proc->info.control_type == BREAST_PUMP_CONTROL_TYPE_RIGHT) {
        if (is_left) {
            if (proc->setting.level[1] > 0) {
                proc->setting.level[1]--;
            }
        } else {
            if (proc->setting.level[1] < BREAST_PUMP_LEVEL_15) {
                proc->setting.level[1]++;
            }
        }
        ui_type = PUMP_UI_TYPE_RIGHT_LEVEL;
    } else if (proc->info.control_type == BREAST_PUMP_CONTROL_TYPE_BILATERAL) {
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
            if (is_left) {
                if (proc->setting.level[i] > 0) {
                    proc->setting.level[i]--;
                }
            } else {
                if (proc->setting.level[i] < BREAST_PUMP_LEVEL_15) {
                    proc->setting.level[i]++;
                }
            }
        }
        ui_type = PUMP_UI_TYPE_BILATERAL_LEVEL;
    }
    pump->set_breast_pump_setting(&proc->setting);
    st->put_store_ram_val(STORE_ID_PUMP_SETTING_INFO, pump_info->mode, 0, &proc->setting);
    if (proc->current_ui_type != ui_type) {
        show_pump_level_ui(proc, ui_type);
        proc->ui_progress = 0;
        proc->display_progress = 0;
    } else {
        proc->ui_progress++;
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("pump level ui progress: %d", proc->ui_progress);
    }

    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        LOG_UI("switch pump %d level: %d, mode: %s", i, proc->setting.level[i],
             pump->get_breast_pump_mode_name(pump_info->mode));
    } 
    
    return 0;
}   

void pump_setting_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_LEVEL_DISPLAY_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_SPEED_DISPLAY_TASK_NAME);
}