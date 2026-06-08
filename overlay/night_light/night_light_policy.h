#ifndef __NIGHT_LIGHT_POLICY_H__
#define __NIGHT_LIGHT_POLICY_H__

#if IS_SUPPORT_NIGHT_LIGHT

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "adaptive_dimming.h" // IWYU pragma: keep

/// 手动/App 档位 → pwm_table[] 索引（与 ALS ADAP_DIM_NL_PWM_IDX_* 一致）
#define NIGHT_LIGHT_LEVEL_OFF                ADAP_DIM_NL_PWM_IDX_OFF
#define NIGHT_LIGHT_LEVEL_MAX                100u
#define NIGHT_LIGHT_LEVEL_GEAR_L1            ADAP_DIM_NL_PWM_IDX_L1
#define NIGHT_LIGHT_LEVEL_GEAR_L2            ADAP_DIM_NL_PWM_IDX_L2
#define NIGHT_LIGHT_LEVEL_GEAR_L3            ADAP_DIM_NL_PWM_IDX_L3

/// App / 协议档位（L0 关，L1~L3 亮）；ALS 曲线不在此文件
typedef enum {
    NIGHT_LIGHT_GEAR_L0 = 0,
    NIGHT_LIGHT_GEAR_L1,
    NIGHT_LIGHT_GEAR_L2,
    NIGHT_LIGHT_GEAR_L3,
    NIGHT_LIGHT_GEAR_MAX,
} night_light_gear_e;

uint8_t night_light_policy_default_gear(void);
uint8_t night_light_policy_safety_gear(void);

/// 当前亮度 → 档位标签（仅手动/App 同步用，ALS 线程不调用）
uint8_t night_light_policy_level_to_gear(uint8_t level);
uint8_t night_light_policy_gear_to_level(uint8_t gear);

#ifdef __cplusplus
}
#endif

#endif /* IS_SUPPORT_NIGHT_LIGHT */

#endif
