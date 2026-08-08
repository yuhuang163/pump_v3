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


/**
 * 获取预解码背景 draw_buf（`LV_COLOR_FORMAT_NATIVE`）。
 * 支持：`BASE_RES_ID_SIMULATION_MODE_BG` / `BASE_RES_ID_EXPRESSION_MODE_BG` /
 * `BASE_RES_ID_PROGRAM_MODE_BG`。
 * 返回的缓冲由 snapshot 模块长期持有，页面侧勿 destroy。
 */
lv_draw_buf_t *get_predecoded_bg(uint16_t res_id);

/**
 * 获取预解码模式 ICON draw_buf（`LV_COLOR_FORMAT_ARGB8888`）。
 * 支持：`BASE_RES_ID_SIMULATION_ICON` / `BASE_RES_ID_EXPRESSION_ICON` /
 * `BASE_RES_ID_PROGRAM_ICON`。
 * 返回的缓冲由 snapshot 模块长期持有，页面侧勿 destroy。
 */
lv_draw_buf_t *get_predecoded_icon(uint16_t res_id);

/**
 * 获取预解码文案扫光蒙层 draw_buf（`LV_COLOR_FORMAT_ARGB8888`）。
 * 资源：`BASE_RES_ID_TEXT_SHINE_MASK`。
 * 返回的缓冲由 snapshot 模块长期持有，页面侧勿 destroy。
 */
lv_draw_buf_t *get_predecoded_text_shine_mask(void);

void ui_snap_shot_perpare(ui_manager_context_t *thiz);

/**
 * 分步预解码（供 logo 亮屏后异步推进，避免阻塞开机关键路径）。
 * @return true 表示还有后续步骤，调用方应继续调度；false 表示已完成。
 */
bool ui_snap_shot_prepare_step(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __UI_SNAPSHOT_H__
