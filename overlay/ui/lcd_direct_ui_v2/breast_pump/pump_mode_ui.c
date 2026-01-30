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
#define IS_SUPPORT_MODE_COMP_BG                  (0)
#define MODE_MAX_COUNT                           (3)
#define BREASTFEEDING_MODE_MAX_COUNT             (4)

#define PUMP_MODE_SEL_START_COLOR                0xB3686D
#define PUMP_MODE_SEL_END_COLOR                  0xF1D9D6
#define PUMP_MODE_UNSEL_COLOR                    0xF6F6F6
#define PUMP_MODE_TEXT_SEL_COLOR                 0xFFFFFF 
#define PUMP_MODE_TEXT_UNSEL_COLOR               0x000000   

#define STIMULATE_MODE_LABEL_X                   (69 + 4)
#define STIMULATE_MODE_LABEL_Y                   (90)

#define STIMULATE_MODE_ICON_X                    (109 + 4)
#define STIMULATE_MODE_ICON_Y                    (116 + 1)
#define STIMULATE_MODE_LABEL_TEXT_X              (143 + 3)
#define STIMULATE_MODE_LABEL_TEXT_Y              (110 + 4)

#define LACTATION_MODE_LABEL_X                   (69 + 4)
#define LACTATION_MODE_LABEL_Y                   (166)

#define LACTATION_MODE_ICON_X                    (110 + 4)
#define LACTATION_MODE_ICON_Y                    (189 + 1)
#define LACTATION_MODE_LABEL_TEXT_X              (144 + 3)
#define LACTATION_MODE_LABEL_TEXT_Y              (186 + 4)

#define MIX_MODE_LABEL_X                         (69 + 4)
#define MIX_MODE_LABEL_Y                         (242)

#define MIX_MODE_ICON_X                          (105 + 4)
#define MIX_MODE_ICON_Y                          (263 + 1)
#define MIX_MODE_LABEL_TEXT_X                    (176 + 3)
#define MIX_MODE_LABEL_TEXT_Y                    (263 + 4)

typedef struct {
    const char *name;
    uint16_t label_x;
    uint16_t label_y;
    uint16_t icon_x;
    uint16_t icon_y;
    uint16_t label_text_x;
    uint16_t label_text_y;
} mode_label_info_t;

typedef struct {
#if IS_SUPPORT_MODE_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * root;
    lv_obj_t * root_text;
    lv_obj_t * label[MODE_MAX_COUNT];
    lv_obj_t * label_texts[MODE_MAX_COUNT];
    lv_obj_t * label_icon[MODE_MAX_COUNT];
    res_img_info_t label_icon_infos[MODE_MAX_COUNT];
} pump_mode_display_info_t;
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_MODE_COMP_BG
static PSRAM_DATA_LOC uint8_t mode_bg_data[] = {
    #include "comparison_png2txt/mode_comp_1_360x360.txt"
    // #include "comparison_png2txt/mode_comp_2_360x360.txt"
    // #include "comparison_png2txt/mode_comp_3_360x360.txt"
};
#endif

const static mode_label_info_t mode_label_infos[MODE_MAX_COUNT] = {
    { "Stimulate", 
        STIMULATE_MODE_LABEL_X,      STIMULATE_MODE_LABEL_Y, 
        STIMULATE_MODE_ICON_X,       STIMULATE_MODE_ICON_Y, 
        STIMULATE_MODE_LABEL_TEXT_X, STIMULATE_MODE_LABEL_TEXT_Y
    },
    { "Lactation", 
        LACTATION_MODE_LABEL_X,      LACTATION_MODE_LABEL_Y,
        LACTATION_MODE_ICON_X,       LACTATION_MODE_ICON_Y,
        LACTATION_MODE_LABEL_TEXT_X, LACTATION_MODE_LABEL_TEXT_Y
    },
    { "Mix",       
        MIX_MODE_LABEL_X,            MIX_MODE_LABEL_Y,
        MIX_MODE_ICON_X,             MIX_MODE_ICON_Y, 
        MIX_MODE_LABEL_TEXT_X,       MIX_MODE_LABEL_TEXT_Y
    },
};

static pump_mode_display_info_t *mode_info;

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
void ui_display_pump_mode_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (mode_info == NULL) {
        mode_info = (pump_mode_display_info_t *)u_malloz(sizeof(pump_mode_display_info_t));
        EXCEPTION_CHECK_VOID(mode_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    // pump_mode_root_init();
}

void ui_display_pump_mode_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (mode_info == NULL) {
        return;
    }
    LVGL_OBJ_DELETE(mode_info->root);
    u_free(mode_info);
    mode_info = NULL;
}

int ui_display_pump_mode_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(mode_info);
    // EXCEPTION_CHECK_INT(mode_info->root);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->pump);
    uint8_t mode = proc->pump->get_breast_pump_mode();
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump mode fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
