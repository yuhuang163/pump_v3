#ifndef __UI_LANG_TYPES_H__
#define __UI_LANG_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 控件样式（语言包 PAGE_WGT 与固件 g_ui_widget_default 共用）。
 * 文案在 ui_str_id_e，坐标标量在 ui_layout_id_e，不要塞进本结构。 */

#define UI_WIDGET_KIND_TEXT                         (0u)
#define UI_WIDGET_KIND_ICON                         (1u)
#define UI_WIDGET_KIND_SCALAR                       (2u)

#define UI_STYLE_F_COLOR_INHERIT                    (1u << 0)
#define UI_STYLE_F_OPA_INHERIT                      (1u << 1)

typedef enum {
    UI_FONT_ROLE_HINT = 0,      /* 19 */
    UI_FONT_ROLE_BODY_SM,       /* 22 */
    UI_FONT_ROLE_TITLE,         /* 26 */
    UI_FONT_ROLE_TITLE_MD,      /* 29 */
    UI_FONT_ROLE_TITLE_LG,      /* 33 */
    UI_FONT_ROLE_LATIN_19,
    UI_FONT_ROLE_LATIN_26,
    UI_FONT_ROLE_LATIN_29,
    UI_FONT_ROLE_NUM_MD,        /* 56, firmware common */
    UI_FONT_ROLE_NUM_LG,        /* 69, firmware common */
    UI_FONT_ROLE_COUNT
} ui_font_role_e;

typedef enum {
    UI_FONT_IMG_MODE_SELECT_TIP = 0,
    UI_FONT_IMG_LEVEL_ADJUST_TIP,
    UI_FONT_IMG_PUMP_BREAK_TIP,
    UI_FONT_IMG_GUIDE_SKIP_TIP,
    UI_FONT_IMG_PUMP_RESUME_TIP,
    UI_FONT_IMG_COUNT
} ui_font_img_id_e;

typedef struct __attribute__((packed)) {
    uint8_t kind;
    uint8_t obj_align;
    int16_t x;
    int16_t y;
    uint8_t opa;
    uint8_t text_align;
    int8_t line_space;
    uint8_t font_role;
    uint16_t max_w;
    uint16_t color_rgb;
    uint8_t flags;
    uint8_t rsv;
} ui_widget_style_t;

#ifdef __cplusplus
}
#endif
#endif /* __UI_LANG_TYPES_H__ */
