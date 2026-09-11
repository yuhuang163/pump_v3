#ifndef __LV_USER_FONT_LANG_ZH_H__
#define __LV_USER_FONT_LANG_ZH_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 中文 UI 正文：Noto Sans SC 子集。
 * Latin / 数字 / L-R 走 font_v3/common 的 Aeonik，不再维护 zh Aeonik 子集。
 *
 * 汉字/标点/数字/字母在 string_infos/charset_*.txt，按行拆开便于核对：
 *   1 当前已用汉字
 *   2 暂未使用仍保留汉字（可无此行）
 *   3 空格（U+0020，行首一个空格）+ 标点（33px 该行仅空格）
 *   4 数字
 *   5 A-Z
 *   6 a-z
 * 生成时整份丢进 --symbols，不要再用 -r 0x20-0x7E。标点行开头的空格不要删。
 */

#include "lv_font_noto_sans_zh_19.h" // IWYU pragma: keep
#include "lv_font_noto_sans_zh_22.h" // IWYU pragma: keep
#include "lv_font_noto_sans_zh_26.h" // IWYU pragma: keep
#include "lv_font_noto_sans_zh_29.h" // IWYU pragma: keep
#include "lv_font_noto_sans_zh_33.h" // IWYU pragma: keep

#ifdef __cplusplus
}
#endif
#endif // __LV_USER_FONT_LANG_ZH_H__
