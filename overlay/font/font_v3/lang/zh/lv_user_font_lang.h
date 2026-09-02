#ifndef __LV_USER_FONT_LANG_ZH_H__
#define __LV_USER_FONT_LANG_ZH_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 与英文同一字库类型 Noto Sans，中文子集投放：
 *   font_noto_sans_zh_19/lv_font_noto_sans_zh_19_medium.c → lv_font_noto_sans_zh_19_medium
 *   font_noto_sans_zh_22/lv_font_noto_sans_zh_22_medium.c → lv_font_noto_sans_zh_22_medium
 *   font_noto_sans_zh_26/lv_font_noto_sans_zh_26_medium.c → lv_font_noto_sans_zh_26_medium
 *   font_noto_sans_zh_29/lv_font_noto_sans_zh_29_medium.c → lv_font_noto_sans_zh_29_medium
 *   font_noto_sans_zh_33/lv_font_noto_sans_zh_33_medium.c → lv_font_noto_sans_zh_33_medium
 *
 * 汉字/标点/未翻译 ASCII 均在 charset_19.txt 等文件（空格在汉字与数字之间）。
 * 生成时整份丢进 --symbols，不要再用 -r 0x20-0x7E。
 * 例：
 *   lv_font_conv --bpp 4 --size 26 --no-compress --stride 1 --align 1 \
 *     --font NotoSansSC-Medium.ttf --symbols "$(cat charset_26.txt)" \
 *     --format lvgl -o lv_font_noto_sans_zh_26_medium.c
 *
 * 速度数字 / L-R / 模式页时分秒 / OTA 百分比用 Aeonik 26 子集：
 *   font_aeonik_soft_zh_26（charset_aeonik_26.txt）
 * 引导页号总页、OTA 大小用 Aeonik 19 子集：
 *   font_aeonik_soft_zh_19（charset_aeonik_19.txt）
 * 引导页号当前页用 Aeonik 29 子集：
 *   font_aeonik_soft_zh_29（charset_aeonik_29.txt）
 */

#include "lv_font_noto_sans_zh_19.h" // IWYU pragma: keep
#include "lv_font_noto_sans_zh_22.h" // IWYU pragma: keep
#include "lv_font_noto_sans_zh_26.h" // IWYU pragma: keep
#include "lv_font_noto_sans_zh_29.h" // IWYU pragma: keep
#include "lv_font_noto_sans_zh_33.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_zh_19.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_zh_26.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_zh_29.h" // IWYU pragma: keep

#ifdef __cplusplus
}
#endif
#endif // __LV_USER_FONT_LANG_ZH_H__
