#ifndef __UI_SNAPSHOT_H__
#define __UI_SNAPSHOT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep

struct _lv_draw_buf_t;
typedef struct _lv_draw_buf_t lv_draw_buf_t;

struct ui_manager_context;
typedef struct ui_manager_context ui_manager_context_t;

typedef enum {
    UI_SNAP_SHOT_COMMON_BG = 0,
    UI_SNAP_SHOT_ID_MAX
} ui_snap_shot_id_e;

typedef struct {
    uint16_t img_id;
    uint8_t is_need_alpha;
} ui_snap_shot_info_t;

lv_draw_buf_t *get_lvgl_snap_shot(ui_snap_shot_id_e snapshot_id);
void ui_snap_shot_perpare(ui_manager_context_t *thiz);
bool ui_snap_shot_prepare_step(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __UI_SNAPSHOT_H__
