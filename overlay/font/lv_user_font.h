#ifndef __LV_USER_FONT_H__
#define __LV_USER_FONT_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (ROOT_FONT_VERSION == ROOT_FONT_V1)
#include "lv_font_oppo_sans_12.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_14.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_16.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_20.h" // IWYU pragma: keep
// #include "lv_font_oppo_sans_22.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_24.h" // IWYU pragma: keep
// #include "lv_font_oppo_sans_32.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_72.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_96.h" // IWYU pragma: keep
#include "lv_font_quicksand_64.h" // IWYU pragma: keep
#include "lv_font_quicksand_96.h" // IWYU pragma: keep
#else
#include "lv_font_aeonik_soft_16.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_18.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_24.h" // IWYU pragma: keep
#include "lv_font_aeonik_soft_26.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_12.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_14.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_16.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_20.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_24.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_72.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_96.h" // IWYU pragma: keep
#include "lv_font_quicksand_64.h" // IWYU pragma: keep
#include "lv_font_quicksand_72.h" // IWYU pragma: keep
#include "lv_font_quicksand_96.h" // IWYU pragma: keep
#endif

#define LV_FONT_PACK_MAX_PACKAGE_SIZE     (102400u)
#define LV_FONT_PACK_MAGIC                (0x5046564Cu)

#define FONT_ID_OPPO_SANS_12_MEDIUM       (0x28A004A3u)
#define FONT_NAME_OPPO_SANS_12_MEDIUM     "lv_font_oppo_sans_12_medium"
#define FONT_ID_OPPO_SANS_14_BOLD         (0x265170BBu)
#define FONT_NAME_OPPO_SANS_14_BOLD       "lv_font_oppo_sans_14_Bold"
#define FONT_ID_OPPO_SANS_14_MEDIUM       (0x24F86124u)
#define FONT_NAME_OPPO_SANS_14_MEDIUM     "lv_font_oppo_sans_14_medium"
#define FONT_ID_OPPO_SANS_16_MEDIUM       (0x273A9659u)
#define FONT_NAME_OPPO_SANS_16_MEDIUM     "lv_font_oppo_sans_16_medium"
#define FONT_ID_OPPO_SANS_20_MEDIUM       (0x24D8A91Bu)
#define FONT_NAME_OPPO_SANS_20_MEDIUM     "lv_font_oppo_sans_20_medium"
#define FONT_ID_OPPO_SANS_24_MEDIUM       (0x236325E1u)
#define FONT_NAME_OPPO_SANS_24_MEDIUM     "lv_font_oppo_sans_24_medium"
#define FONT_ID_OPPO_SANS_72_BOLD         (0x2361A69Cu)
#define FONT_NAME_OPPO_SANS_72_BOLD       "lv_font_oppo_sans_72_Bold"
#define FONT_ID_OPPO_SANS_96_BOLD         (0x27499ED7u)
#define FONT_NAME_OPPO_SANS_96_BOLD       "lv_font_oppo_sans_96_Bold"
#define FONT_ID_QUICKSAND_64_BOLD         (0x2510D4FAu)
#define FONT_NAME_QUICKSAND_64_BOLD       "lv_font_quicksand_64_bold"
#define FONT_ID_QUICKSAND_96_BOLD         (0x23C63A18u)
#define FONT_NAME_QUICKSAND_96_BOLD       "lv_font_quicksand_96_bold"

/* 字体包头 */
typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t package_size;
    uint32_t package_crc32;
} font_pack_header_t;

#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

