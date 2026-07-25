#ifndef __UI_POPUP_H__
#define __UI_POPUP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

struct _lv_obj_t;
typedef struct _lv_obj_t lv_obj_t;

void ui_popup_update(ui_manager_context_t *thiz,
    popup_ui_id_e type, lv_obj_t *parent, uint8_t *ui_index);
void ui_popup_remove(ui_manager_context_t *thiz);
/// 启动长按圆弧倒计时（用于手动结束确认等需二次长按的弹窗）
void ui_popup_hold_countdown_start(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __UI_POPUP_H__
