#ifndef __LV_USER_FONT_LANG_ZH_H__
#define __LV_USER_FONT_LANG_ZH_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 与英文同一字库类型 Aeonik Soft，中文子集投放：
 *   font_aeonik_soft_zh_19/lv_font_aeonik_soft_zh_19_medium.c → lv_font_aeonik_soft_zh_19_medium
 *   font_aeonik_soft_zh_22/lv_font_aeonik_soft_zh_22_medium.c → lv_font_aeonik_soft_zh_22_medium
 *   font_aeonik_soft_zh_26/lv_font_aeonik_soft_zh_26_medium.c → lv_font_aeonik_soft_zh_26_medium
 *   font_aeonik_soft_zh_29/lv_font_aeonik_soft_zh_29_medium.c → lv_font_aeonik_soft_zh_29_medium
 *   font_aeonik_soft_zh_33/lv_font_aeonik_soft_zh_33_medium.c → lv_font_aeonik_soft_zh_33_medium
 *
 * 汉字/标点/未翻译 ASCII 均在 charset_19.txt 等文件（空格在汉字与数字之间）。
 * 生成时整份丢进 --symbols，不要再用 -r 0x20-0x7E。
 * 例：
 *   lv_font_conv --bpp 4 --size 26 --no-compress --stride 1 --align 1 \
 *     --font OPPO_Sans_Medium.ttf --symbols "$(cat charset_26.txt)" \
 *     --format lvgl -o lv_font_aeonik_soft_zh_26_medium.c
 */

#include "lv_font_aeonik_soft_zh_19.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_zh_22.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_zh_26.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_zh_29.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_zh_33.h" // IWYU pragma: keep

#ifdef __cplusplus
}
#endif
#endif // __LV_USER_FONT_LANG_ZH_H__
