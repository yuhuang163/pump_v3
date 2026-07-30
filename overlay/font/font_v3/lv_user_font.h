#ifndef __LV_USER_FONT_H__
#define __LV_USER_FONT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lv_font_aeonik_soft_13.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_16.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_18.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_19.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_20.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_22.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_24.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_26.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_28.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_29.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_32.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_33.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_36.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_48.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_56.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_69.h" // IWYU pragma: keep

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
