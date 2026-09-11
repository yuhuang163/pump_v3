#ifndef __UI_LANG_ZH_FONTS_H__
#define __UI_LANG_ZH_FONTS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_lang_runtime.h"

/*******************************************************************************
 * *_FW：固件默认字库；UI_FONT_* 运行时走外部语言包（缺项回退 *_FW）
 ******************************************************************************/
#define UI_FONT_HINT_FW                                 (&lv_font_noto_sans_zh_19_medium)
#define UI_FONT_BODY_SM_FW                              (&lv_font_noto_sans_zh_22_medium)
#define UI_FONT_TITLE_FW                                (&lv_font_noto_sans_zh_26_medium)
#define UI_FONT_TITLE_MD_FW                             (&lv_font_noto_sans_zh_29_medium)
#define UI_FONT_TITLE_LG_FW                             (&lv_font_noto_sans_zh_33_medium)
#define UI_FONT_AEONIK_FW                               (&lv_font_aeonik_soft_26_medium)
#define UI_FONT_AEONIK_19_FW                            (&lv_font_aeonik_soft_19_medium)
#define UI_FONT_AEONIK_29_FW                            (&lv_font_aeonik_soft_29_medium)
#define UI_FONT_NUM_MD_FW                               (&lv_font_aeonik_soft_56_semibold)
#define UI_FONT_NUM_LG_FW                               (&lv_font_aeonik_soft_69_semibold)

#define UI_FONT_HINT                                    ui_font(UI_FONT_ROLE_HINT)
#define UI_FONT_BODY_SM                                 ui_font(UI_FONT_ROLE_BODY_SM)
#define UI_FONT_TITLE                                   ui_font(UI_FONT_ROLE_TITLE)
#define UI_FONT_TITLE_MD                                ui_font(UI_FONT_ROLE_TITLE_MD)
#define UI_FONT_TITLE_LG                                ui_font(UI_FONT_ROLE_TITLE_LG)
#define UI_FONT_AEONIK                                  ui_font(UI_FONT_ROLE_LATIN_26)
#define UI_FONT_AEONIK_19                               ui_font(UI_FONT_ROLE_LATIN_19)
#define UI_FONT_AEONIK_29                               ui_font(UI_FONT_ROLE_LATIN_29)
#define UI_FONT_NUM_MD                                  ui_font(UI_FONT_ROLE_NUM_MD)
#define UI_FONT_NUM_LG                                  ui_font(UI_FONT_ROLE_NUM_LG)

#ifdef __cplusplus
}
#endif
#endif // __UI_LANG_ZH_FONTS_H__
