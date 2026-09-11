#ifndef __LV_USER_FONT_H__
#define __LV_USER_FONT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * font_v3 目录：
 *   common/                 数字 56/69 + Latin Aeonik 19/22/26/29/33
 *   lang/$(PRODUCT_LANGUAGE) 当前语言正文（中文 Noto）+ font_img
 * 页面通过 UI_FONT_* 角色宏引用。
 */

#include "lv_font_aeonik_soft_56.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_69.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_19.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_22.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_26.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_29.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_33.h" // IWYU pragma: keep
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
