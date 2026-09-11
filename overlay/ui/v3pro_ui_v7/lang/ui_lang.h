#ifndef __UI_LANG_H__
#define __UI_LANG_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * 语言入口：先展开编译期 strings/layout/fonts，再叠 bind，把宏改成运行时查表。
 * 改文案/坐标请改 lang/<zh|en>/，然后跑 gen_ui_lang_tables.py。
 * ID 对照：ui_lang_id_map.html；打包预览：overlay/font/packaging_tools/preview.sh
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

#include "ui_lang_runtime.h" // IWYU pragma: keep
#include "ui_lang_pack.h" // IWYU pragma: keep
#include "ui_lang_bind.h" // IWYU pragma: keep

#ifdef __cplusplus
}
#endif
#endif // __UI_LANG_H__
