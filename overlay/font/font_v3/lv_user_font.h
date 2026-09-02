#ifndef __LV_USER_FONT_H__
#define __LV_USER_FONT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * font_v3 目录：
 *   common/              语言无关数字字库（56 / 69）
 *   lang/en/             英文本体 + font_img
 *   lang/zh/             中文本体（noto_sans_zh_*）+ Aeonik 子集 + font_img
 * Makefile 只把 common + lang/$(PRODUCT_LANGUAGE) 加入编译和 include。
 * 页面通过 UI_FONT_* 角色宏引用。
 */

#include "lv_font_aeonik_soft_56.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_69.h" // IWYU pragma: keep
#include "lv_user_font_lang.h" // IWYU pragma: keep

/* 字体包头 */
typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t package_size;
    uint32_t package_crc32;
} font_pack_header_t;

#ifdef __cplusplus
}
#endif
#endif // __LV_USER_FONT_H__
