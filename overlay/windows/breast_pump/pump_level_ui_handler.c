/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_pump.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_LEVEL_DISPLAY_TASK_NAME       "pump_level_display_cmp"
#define PUMP_LEVEL_SYNC_TASK_NAME          "pump_level_sync_cmp"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

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
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL) {
        show_pump_home_ui(proc);
        /// 档位变化值归零
        proc->level_change = 0;
        /// 清零档位变化方向
        proc->level_change_direction = 0xff;/// 无效值
    }
}

static void pump_ui_level_status_syn(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL) {
        EXCEPTION_CHECK_VOID(proc->motor);
        int time_index = 0;
        READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
        WRITE_PUMP_SETTING_INFO(proc->pump, time_index, true, proc->setting);
    }
}

static void pump_level_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void pump_level_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    ui_display_pump_level_process(ui);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= PUMP_SUB_UI_DISPLAY_TIME) {
        POST_TASK_HANDLER_NO_DELAY(pump_level_display_completed_cb, proc, PUMP_LEVEL_DISPLAY_TASK_NAME);
    } else if (proc->level_sync_flag && time_diff >= PUMP_LEVEL_SYNC_TIMEOUT) {
        proc->level_sync_flag = false;
        POST_TASK_HANDLER_NO_DELAY(pump_ui_level_status_syn, proc, PUMP_LEVEL_SYNC_TASK_NAME);
    }
}

static void pump_level_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            pump_level_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            pump_level_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void pump_level_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_level_prepare(ui);
}

static void pump_level_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_pump_level_remove(ui);
}

static void show_pump_level_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == PUMP_UI_TYPE_LEVEL) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    } else {
        pump_ui_change_hdr_remove(proc);
    }
    ui_play_info_t play_info = {
        .tag = "pump_level_screen",
        .fps = PUMP_LEVEL_ANIMATION_FREQ,
        .progress_cb = pump_level_animation_progress_cb,
        .display_prepare_cb = pump_level_ui_display_prepare,
        .display_remove_cb = pump_level_ui_display_remove,
        .user_data = proc,
    };
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = PUMP_UI_TYPE_LEVEL;
    ui->hook->ui_animation_play_init(ui, play_info);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void switch_pump_control_type_ui(ui_proc_pump_t *proc,uint8_t control_type)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    int is_pump_working = 0; 
    READ_PUMP_IS_WORKING(proc->pump, is_pump_working);
    if (is_pump_working == 0) {
        // LOG_UI("pump is not working, ignore.");
        return;
    }

    if (control_type >= BREAST_PUMP_CONTROL_TYPE_MAX) {
        LOGE("switch pump control type: %d, out of range", control_type);
        return;
    }

    if (proc->current_ui_type != PUMP_UI_TYPE_LEVEL) {
        int time_index = 0;
        READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
        READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    } 

    if (control_type == BREAST_PUMP_CONTROL_TYPE_RIGHT) {
        proc->led->set_led_blink(PUMP_RIGHT_CONTROL_BL, PUMP_BLINK_PERIOD,
            PUMP_LEVEL_BLINK_TIMES, LED_STATUS_ON);
        // proc->led->set_led_level(PUMP_LEFT_CONTROL_BL, LED_LEVEL_MAX / 2);
    } else if (control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) {
        proc->led->set_led_blink(PUMP_LEFT_CONTROL_BL, PUMP_BLINK_PERIOD,
            PUMP_LEVEL_BLINK_TIMES, LED_STATUS_ON);
        // proc->led->set_led_level(PUMP_RIGHT_CONTROL_BL, LED_LEVEL_MAX / 2);
    } else {
        proc->led->set_led_level(PUMP_RIGHT_CONTROL_BL, LED_LEVEL_MAX);
        proc->led->set_led_level(PUMP_LEFT_CONTROL_BL, LED_LEVEL_MAX);
    }

    if (proc->info.control_type != control_type) {
        proc->info.control_type = control_type;
        WRITE_PUMP_LEVEL_CONTROL_TYPE(proc->pump, control_type);
    }
    
    show_pump_level_ui(proc);
    LOG_UI("switch pump control type: %d", control_type);
}

/// 通过旋钮同时切双边换档位和控制类型为双边模式
void switch_pump_bilateral_control_type_ui_by_knob(ui_proc_pump_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    int is_pump_working = 0; 
    READ_PUMP_IS_WORKING(proc->pump, is_pump_working);
    if (is_pump_working == 0) {
        return;
    }

    if (proc->current_ui_type != PUMP_UI_TYPE_LEVEL) {
        int time_index = 0;
        READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
        READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    }
    /// 设置双边模式时，两侧的灯亮着
    proc->led->set_led_level(PUMP_RIGHT_CONTROL_BL, LED_LEVEL_MAX);
    proc->led->set_led_level(PUMP_LEFT_CONTROL_BL, LED_LEVEL_MAX);

    /// 设置控制类型为双边模式
    proc->info.control_type = BREAST_PUMP_CONTROL_TYPE_BILATERAL;
    WRITE_PUMP_LEVEL_CONTROL_TYPE(proc->pump, BREAST_PUMP_CONTROL_TYPE_BILATERAL);
    if (proc->info.control_type == BREAST_PUMP_CONTROL_TYPE_BILATERAL) {
        /// 双侧模式：level_change 表示档位变化值，范围 -15 到 15
        /// is_increase=true: 增加强度（level_change++, level[i]++）
        /// is_increase=false: 减小强度（level_change--, level[i]--）

        /// 如果上次档位变化方向与is_increase相反，则清零档位变化值
        if (proc->level_change_direction != is_increase) {
            proc->level_change = 0;
        }
        /// 更新档位变化方向
        proc->level_change_direction = is_increase;
       
        /// 更新档位变化值
        if (is_increase) {
            if (!(proc->setting.level[0] == BREAST_PUMP_LEVEL_0 && proc->setting.level[1] == BREAST_PUMP_LEVEL_0)) {
                if (proc->level_change > -15) {
                    proc->level_change--;
                }
                LOG_UI("switch pump level --: %d", proc->level_change);
            }
        } else {
            if (!(proc->setting.level[0] == BREAST_PUMP_LEVEL_15 && proc->setting.level[1] == BREAST_PUMP_LEVEL_15)) {
                if (proc->level_change < 15) {
                    proc->level_change++;
                    LOG_UI("switch pump level ++: %d", proc->level_change);
                }
            }
        }
        // 同步更新两侧的档位值（档位值越小，强度越大）
        for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
            if (is_increase) {
                if (proc->setting.level[i] > 0) {
                    proc->setting.level[i]--;
                }
            } else {
                if (proc->setting.level[i] < BREAST_PUMP_LEVEL_15) {
                    proc->setting.level[i]++;
                }
            }
        }
        char *mode_name = "unknow";
        READ_PUMP_WORK_MODE_NAME(proc->pump, &mode_name);
        LOG_UI("switch pump level change: %d, mode: %s", proc->level_change, mode_name);
    }
    // 延后同步
    proc->level_sync_flag = true;
    /// 显示UI
    show_pump_level_ui(proc);
}

void switch_pump_level_ui(ui_proc_pump_t *proc, bool is_increase)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    EXCEPTION_CHECK_VOID(proc->motor);
    int is_pump_working = 0; 
    READ_PUMP_IS_WORKING(proc->pump, is_pump_working);
    if (is_pump_working == 0) {
        LOG_UI("pump is not working, ignore.");
        return;
    }
    char *mode_name = "unknow";
    READ_PUMP_WORK_MODE_NAME(proc->pump, &mode_name);;
    if (proc->current_ui_type != PUMP_UI_TYPE_LEVEL) {
        int time_index = 0;
        READ_PUMP_MODE_STEP_INDEX(proc->pump, time_index);
        int control_type = 0;
        READ_PUMP_LEVEL_CONTROL_TYPE(proc->pump, control_type);
        proc->info.control_type = control_type;
        READ_PUMP_SETTING_INFO(proc->pump, time_index, proc->setting);
    } else {
        /// 如果当前是PUMP_UI_TYPE_LEVEL，则根据档位变化方向更新档位变化值
        if (proc->info.control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) {
            if (is_increase) {
                if (proc->setting.level[0] > 0) {
                    proc->setting.level[0]--;
                }
            } else {
                if (proc->setting.level[0] < BREAST_PUMP_LEVEL_15) {
                    proc->setting.level[0]++;
                }
            }
        } else if (proc->info.control_type == BREAST_PUMP_CONTROL_TYPE_RIGHT) {
            if (is_increase) {
                if (proc->setting.level[1] > 0) {
                    proc->setting.level[1]--;
                }
            } else {
                if (proc->setting.level[1] < BREAST_PUMP_LEVEL_15) {
                    proc->setting.level[1]++;
                }
            }
        } else if (proc->info.control_type == BREAST_PUMP_CONTROL_TYPE_BILATERAL) {
            /// 双侧模式：level_change 表示档位变化值，范围 -15 到 15
            /// is_increase=true: 增加强度（level_change++, level[i]++）
            /// is_increase=false: 减小强度（level_change--, level[i]--）
    
            /// 如果上次档位变化方向与is_increase相反，则清零档位变化值
            if (proc->level_change_direction != is_increase) {
                proc->level_change = 0;
            }
            /// 更新档位变化方向
            proc->level_change_direction = is_increase;
            /// 更新档位变化值
            if (is_increase) {
                if (!(proc->setting.level[0] == BREAST_PUMP_LEVEL_0 && proc->setting.level[1] == BREAST_PUMP_LEVEL_0)) {
                    if (proc->level_change > -15) {
                        proc->level_change--;
                    }
                }
            } else {
                if (!(proc->setting.level[0] == BREAST_PUMP_LEVEL_15 && proc->setting.level[1] == BREAST_PUMP_LEVEL_15)) {
                    if (proc->level_change < 15) {
                        proc->level_change++;
                    }
                }
            }
            // 同步更新两侧的档位值（档位值越小，强度越大）
            for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
                if (is_increase) {
                    if (proc->setting.level[i] > 0) {
                        proc->setting.level[i]--;
                    }
                } else {
                    if (proc->setting.level[i] < BREAST_PUMP_LEVEL_15) {
                        proc->setting.level[i]++;
                    }
                }
            }
            LOG_UI("switch pump level change: %d, mode: %s", proc->level_change, mode_name);
        }
        /// 如果 UI 已经是 LEVEL 类型，设置同步标志，延后同步；否则立即同步
        proc->level_sync_flag = true;
    
    }
    
    show_pump_level_ui(proc);
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        LOG_UI("switch pump %d level: %d, mode: %s", i, proc->setting.level[i], mode_name);
    } 
}   

void pump_level_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_LEVEL_DISPLAY_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_LEVEL_SYNC_TASK_NAME);
}