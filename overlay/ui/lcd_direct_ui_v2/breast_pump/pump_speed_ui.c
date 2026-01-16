/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "breast_pump_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_SPEED_COMP_BG                        (0)
#define SPEED_MAX_COUNT                                 3

#define PUMP_SPEED_TEXT_SEL_COLOR                       0xFFFFFF
#define PUMP_SPEED_TEXT_UNSEL_COLOR                     0x000000
#define PUMP_SPEED_SEL_START_COLOR                      0xB3686D
#define PUMP_SPEED_SEL_END_COLOR                        0xF1D9D6
#define PUMP_SPEED_UNSEL_COLOR                          0xF6F6F6

#define COMFORT_LABEL_X                                 (69 + 4)
#define COMFORT_LABEL_Y                                 (90)
#define COMFORT_LABEL_TEXT_X                            (133 + 3)
#define COMFORT_LABEL_TEXT_Y                            (110 + 4)

#define REGULAR_LABEL_X                                 (69 + 4)
#define REGULAR_LABEL_Y                                 (166)
#define REGULAR_LABEL_TEXT_X                            (137 + 3)
#define REGULAR_LABEL_TEXT_Y                            (186 + 4)

#define BOOST_LABEL_X                                   (69 + 4)
#define BOOST_LABEL_Y                                   (242)
#define BOOST_LABEL_TEXT_X                              (147 + 3)
#define BOOST_LABEL_TEXT_Y                              (263 + 4)

typedef struct ui_display_speed_info {
    lv_obj_t * root;
#if IS_SUPPORT_SPEED_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * root_text;
    lv_obj_t * label[SPEED_MAX_COUNT];
    lv_obj_t * label_texts[SPEED_MAX_COUNT];
} speed_display_info_t;

typedef struct {
    const char *name;
    uint16_t label_x;
    uint16_t label_y;
    uint16_t label_text_x;
    uint16_t label_text_y;
} speed_label_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static const speed_label_info_t speed_label_infos[SPEED_MAX_COUNT] = {
    {"Comfort", 
     COMFORT_LABEL_X, COMFORT_LABEL_Y, 
     COMFORT_LABEL_TEXT_X, COMFORT_LABEL_TEXT_Y
    },
    {"Regular", 
     REGULAR_LABEL_X, REGULAR_LABEL_Y, 
     REGULAR_LABEL_TEXT_X, REGULAR_LABEL_TEXT_Y
    },
    {"Boost", 
     BOOST_LABEL_X, BOOST_LABEL_Y, 
     BOOST_LABEL_TEXT_X, BOOST_LABEL_TEXT_Y
    },
};

static speed_display_info_t *speed_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_speed_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (speed_info == NULL) {
        speed_info = (speed_display_info_t *)u_malloz(sizeof(speed_display_info_t));
        EXCEPTION_CHECK_VOID(speed_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    // pump_speed_root_init();
}

void ui_display_pump_speed_remove(ui_manager_context_t *thiz)
{
    LVGL_OBJ_DELETE(speed_info->root);
    u_free(speed_info);
    speed_info = NULL;
}

int ui_display_pump_speed(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_UINT(thiz);
    EXCEPTION_CHECK_UINT(speed_info);
    EXCEPTION_CHECK_INT(speed_info->root);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->pump);
    EXCEPTION_CHECK_INT(proc->motor);
    pump_setting_t setting;
    uint8_t time_index = proc->motor->get_runtime_policy_index(MOTOR_TIME_POLICY);
    proc->pump->get_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index);
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump speed fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}