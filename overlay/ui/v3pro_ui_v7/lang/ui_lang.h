#ifndef __UI_LANG_H__
#define __UI_LANG_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "product_info.h" // IWYU pragma: keep

#if (PRODUCT_LANGUAGE_USED == PRODUCT_LANGUAGE_EN)
#include "en/strings.h" // IWYU pragma: keep
#include "en/layout.h" // IWYU pragma: keep
#include "en/fonts.h" // IWYU pragma: keep
#elif (PRODUCT_LANGUAGE_USED == PRODUCT_LANGUAGE_ZH)
#include "zh/strings.h" // IWYU pragma: keep
#include "zh/layout.h" // IWYU pragma: keep
#include "zh/fonts.h" // IWYU pragma: keep
#else
#error "Unsupported PRODUCT_LANGUAGE_USED"
#endif

#ifdef __cplusplus
}
#endif
#endif // __UI_LANG_H__
