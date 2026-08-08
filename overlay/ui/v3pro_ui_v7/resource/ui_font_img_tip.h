#ifndef __UI_FONT_IMG_TIP_H__
#define __UI_FONT_IMG_TIP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "ui_img_define.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
/** 「Press the knob to confirm」提示图，全工程只保留一份 PNG 数据 */
const ui_img_data_t *ui_font_img_mode_select_tip_get(void);
/** 单边力度调节提示图，全工程只保留一份 PNG 数据 */
const ui_img_data_t *ui_font_img_level_adjust_tip_get(void);
/** 泵奶休息轻提醒页底部提示图，全工程只保留一份 PNG 数据 */
const ui_img_data_t *ui_font_img_pump_break_tip_get(void);
/** 引导页「Press the knob to skip」提示图，全工程只保留一份 PNG 数据 */
const ui_img_data_t *ui_font_img_guide_skip_tip_get(void);

#ifdef __cplusplus
}
#endif
#endif // __UI_FONT_IMG_TIP_H__
