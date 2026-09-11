#ifndef __UI_LAYOUT_ID_H__
#define __UI_LAYOUT_ID_H__

/* 不挂在单个控件上的 layout 标量（_GAP / _SHIFT_Y 等） */
/* 本文件由 gen_ui_lang_tables.py 生成，不要手改。对照表见同目录 ui_lang_id_map.html */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UI_LAY_PUMP_PAUSE_TIMEOUT_TITLE_GAP = 0, /* PUMP */
    UI_LAY_PUMP_FINISH_TIMEOUT_TITLE_GAP, /* PUMP */
    UI_LAY_PUMP_WILL_FINISH_TITLE_GAP, /* PUMP */
    UI_LAY_POWER_ON_HOLD_LINE_GAP, /* POWER */
    UI_LAY_AIRPLANE_MODE_HOLDING_CONTENT_SHIFT_Y, /* POWER */
    UI_LAY_FACTORY_RESET_PROGRESS_TEXT_GAP, /* FACTORY */
    UI_LAY_SYS_ABNORMAL_ICON_Y_SYS_ERROR, /* COMMON */
    UI_LAY_SYS_ABNORMAL_TITLE_Y_SYS_ERROR, /* COMMON */
    UI_LAY_SYS_ABNORMAL_CONTENT_Y_SYS_ERROR, /* COMMON */
    UI_LAY_SYS_ABNORMAL_ICON_Y_TEMP_ERROR, /* COMMON */
    UI_LAY_SYS_ABNORMAL_TITLE_Y_TEMP_ERROR, /* COMMON */
    UI_LAY_SYS_ABNORMAL_CONTENT_Y_TEMP_ERROR, /* COMMON */
    UI_LAY_COUNT
} ui_layout_id_e;

#ifdef __cplusplus
}
#endif
#endif /* __UI_LAYOUT_ID_H__ */
