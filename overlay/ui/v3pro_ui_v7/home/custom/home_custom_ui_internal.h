#ifndef __HOME_CUSTOM_UI_INTERNAL_H__
#define __HOME_CUSTOM_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ble_ui.h" // IWYU pragma: keep
#include "home_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_HOME_CUSTOM_MODE_CMP         (0)

/// 页间过渡动画参数（custom 主题，与 base 独立配置）
#ifndef HOME_CUSTOM_PAGE_TRANSITION_MS
#define HOME_CUSTOM_PAGE_TRANSITION_MS                 (650)
#endif
#ifndef HOME_CUSTOM_PAGE_TRANSITION_HERO_MS
#define HOME_CUSTOM_PAGE_TRANSITION_HERO_MS            (180)   /// ICON 缩放 + 标题字号切换时长
#endif
#ifndef HOME_CUSTOM_PAGE_TRANSITION_HERO_SCALE_MIN
#define HOME_CUSTOM_PAGE_TRANSITION_HERO_SCALE_MIN     ((256 * 85) / 100)  /// ICON 最小缩放 ≈85%
#endif
#ifndef HOME_CUSTOM_PAGE_TRANSITION_SLIDE_PROGRESS
#if (HOME_CUSTOM_PAGE_TRANSITION_MS > HOME_CUSTOM_PAGE_TRANSITION_HERO_MS)
#define HOME_CUSTOM_PAGE_TRANSITION_SLIDE_PROGRESS \
    (((HOME_CUSTOM_PAGE_TRANSITION_MS - HOME_CUSTOM_PAGE_TRANSITION_HERO_MS) * 256) / \
     HOME_CUSTOM_PAGE_TRANSITION_MS)
#else
#define HOME_CUSTOM_PAGE_TRANSITION_SLIDE_PROGRESS     (0)
#endif
#endif
#ifndef HOME_CUSTOM_PAGE_TRANSITION_CIRCLE_SIZE
#define HOME_CUSTOM_PAGE_TRANSITION_CIRCLE_SIZE        (280)   /// 过渡圆直径
#endif
#ifndef HOME_CUSTOM_PAGE_TRANSITION_CIRCLE_COLOR
#define HOME_CUSTOM_PAGE_TRANSITION_CIRCLE_COLOR       (0x4D4D4D)
#endif
#ifndef HOME_CUSTOM_PAGE_TRANSITION_CIRCLE_GAP
#define HOME_CUSTOM_PAGE_TRANSITION_CIRCLE_GAP         (24)    /// 两圆边缘间距
#endif
#ifndef HOME_CUSTOM_PAGE_TRANSITION_OPA_MIN
#define HOME_CUSTOM_PAGE_TRANSITION_OPA_MIN            (102)   /// ~40%
#endif
#ifndef HOME_CUSTOM_PAGE_TRANSITION_MAIN_BG_FULL_SIZE
#define HOME_CUSTOM_PAGE_TRANSITION_MAIN_BG_FULL_SIZE  (360)   /// page0 全尺寸，过渡不缩放
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif // __HOME_CUSTOM_UI_INTERNAL_H__
