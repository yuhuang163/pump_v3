#ifndef __UI_CUSTOM_MODE_NAME_H__
#define __UI_CUSTOM_MODE_NAME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "ui_lang.h"

/**
 * 英文前缀 -> 当前语言名。
 * 匹配规则：完全等于前缀，或「前缀 + 空格 + 后缀」。
 * 多条同时命中时取最长前缀（避免 "Cozy" 抢掉 "Cozy Flow"）。
 *
 * 新增示例：
 *   { "Cozy Flow", UI_STR_ID_MODE_COZY_FLOW },
 */
typedef struct {
    const char *en_prefix;
    ui_str_id_e local_id;
} ui_custom_mode_name_map_t;

/**
 * 将 App 下发的自定义模式名转为当前展示语言。
 * 有外部语言包时跟包的 lang_id；否则跟编译期 PRODUCT_LANGUAGE。
 * 英文：原样输出。非英文：按英文前缀表替换（可带后缀）；未匹配则回退原文。
 */
static inline int ui_custom_mode_name_need_localize(void)
{
    if (ui_lang_pack_is_active()) {
        return ui_lang_pack_lang_id() != (uint16_t)PRODUCT_LANGUAGE_EN;
    }
    return PRODUCT_LANGUAGE_USED != PRODUCT_LANGUAGE_EN;
}

static inline void ui_custom_mode_name_localize(const char *src, char *dst, size_t dst_len)
{
    static const ui_custom_mode_name_map_t k_map[] = {
        { "Customize", UI_STR_ID_MODE_CUSTOM },
        { "Cozy Flow", UI_STR_ID_MODE_COZY_FLOW },
        { "Power Pumping", UI_STR_ID_MODE_DENSE_MILK },
        { "Milk Boost", UI_STR_ID_MODE_HIGH_EFFICIENCY },
        { "Lactation Starter", UI_STR_ID_MODE_LACTATION_STARTER },
        { "Deep Emptying", UI_STR_ID_MODE_DEEP_EMPTYING },
        { "Let-down Response", UI_STR_ID_MODE_LET_DOWN_RESPONSE },
        { "Before Bed", UI_STR_ID_MODE_BEFORE_BED },
        { "Quick Refill Relief", UI_STR_ID_MODE_QUICK_REFILL },
        { "Post-Nursing", UI_STR_ID_MODE_POST_NURSING },
        { NULL, (ui_str_id_e)0 },
    };
    size_t i;
    size_t best_len = 0;
    const char *best_local = NULL;
    const char *rest = NULL;

    if (dst == NULL || dst_len == 0U) {
        return;
    }
    dst[0] = '\0';
    if (src == NULL) {
        return;
    }
    if (!ui_custom_mode_name_need_localize()) {
        snprintf(dst, dst_len, "%s", src);
        return;
    }

    for (i = 0; k_map[i].en_prefix != NULL; i++) {
        size_t prefix_len = strlen(k_map[i].en_prefix);

        if (prefix_len == 0U || prefix_len <= best_len) {
            continue;
        }
        if (strncmp(src, k_map[i].en_prefix, prefix_len) != 0) {
            continue;
        }
        if (src[prefix_len] != '\0' && src[prefix_len] != ' ') {
            continue;
        }
        best_len = prefix_len;
        best_local = ui_str(k_map[i].local_id);
    }

    if (best_local != NULL) {
        rest = src + best_len;
        while (*rest == ' ') {
            rest++;
        }
        if (*rest != '\0') {
            snprintf(dst, dst_len, "%s %s", best_local, rest);
        } else {
            snprintf(dst, dst_len, "%s", best_local);
        }
        return;
    }

    snprintf(dst, dst_len, "%s", src);
}

#ifdef __cplusplus
}
#endif
#endif /* __UI_CUSTOM_MODE_NAME_H__ */
