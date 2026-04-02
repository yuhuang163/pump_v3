#ifndef __LV_USER_FONT_H__
#define __LV_USER_FONT_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (V3_UI_VERSION == V3_UI_V1)
#include "lv_font_roboto_14.h" // IWYU pragma: keep
#include "lv_font_roboto_16.h" // IWYU pragma: keep
#include "lv_font_roboto_17.h" // IWYU pragma: keep
#include "lv_font_roboto_18.h" // IWYU pragma: keep
#include "lv_font_roboto_19.h" // IWYU pragma: keep
#include "lv_font_roboto_20.h" // IWYU pragma: keep
#include "lv_font_roboto_21.h" // IWYU pragma: keep
#include "lv_font_roboto_24.h" // IWYU pragma: keep
#include "lv_font_roboto_25.h" // IWYU pragma: keep
#include "lv_font_roboto_27.h" // IWYU pragma: keep
#include "lv_font_roboto_32.h" // IWYU pragma: keep
#include "lv_font_roboto_68.h" // IWYU pragma: keep
#include "lv_font_roboto_79.h" // IWYU pragma: keep
#include "lv_font_roboto_81.h" // IWYU pragma: keep
#elif (V3_UI_VERSION == V3_UI_V2 || V3_UI_VERSION == V3PRO_UI_V2)
#include "lv_font_oppo_sans_12.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_14.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_16.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_20.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_22.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_24.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_32.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_72.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_96.h" // IWYU pragma: keep
#include "lv_font_oppo_sans_14.h" // IWYU pragma: keep
#include "lv_font_quicksand_96.h" // IWYU pragma: keep
#else
#error "V3_UI_VERSION is not defined"
#endif

#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

