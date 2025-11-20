#ifndef __UI_DISPLAY_H__
#define __UI_DISPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "ui_manager_api.h" // IWYU pragma: keep
#include "ui_img_define.h" // IWYU pragma: keep

/// 描述图片数据处理接口.
typedef int (*ui_display_process_func)(ui_animation_play_t *ani, lcd_tlvs_t **tlvs);
/// 描述图片数据处理结构.
typedef struct {
    uint16_t img_id;                             /// 图片id   
    ui_display_process_func ui_display_process;   /// 图片数据处理接口.
} ui_display_process_handle_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int lvgl_display_img_data_perpare(ui_animation_play_t *ani, lcd_tlvs_t **tlvs);
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

