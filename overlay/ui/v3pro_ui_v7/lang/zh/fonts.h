#ifndef __UI_LANG_ZH_FONTS_H__
#define __UI_LANG_ZH_FONTS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * 与英文同一套 Noto Sans 角色；中文子集符号为 lv_font_noto_sans_zh_*。
 * 数字字库仍用 common 的 56/69。
 * L/R、速度数字、模式页时分秒、OTA 百分比用 Aeonik 26 子集。
 * 引导页号当前页 / 总页用 Aeonik 29 / 19 子集；OTA 大小用 Aeonik 19 子集。
 ******************************************************************************/
#define UI_FONT_HINT                                    (&lv_font_noto_sans_zh_19_medium)
#define UI_FONT_BODY_SM                                 (&lv_font_noto_sans_zh_22_medium)
#define UI_FONT_TITLE                                   (&lv_font_noto_sans_zh_26_medium)
#define UI_FONT_TITLE_MD                                (&lv_font_noto_sans_zh_29_medium)
#define UI_FONT_TITLE_LG                                (&lv_font_noto_sans_zh_33_medium)
#define UI_FONT_AEONIK                                  (&lv_font_aeonik_soft_zh_26_medium)
#define UI_FONT_AEONIK_19                               (&lv_font_aeonik_soft_zh_19_medium)
#define UI_FONT_AEONIK_29                               (&lv_font_aeonik_soft_zh_29_medium)
#define UI_FONT_NUM_MD                                  (&lv_font_aeonik_soft_56_semibold)
#define UI_FONT_NUM_LG                                  (&lv_font_aeonik_soft_69_semibold)

#ifdef __cplusplus
}
#endif
#endif // __UI_LANG_ZH_FONTS_H__
