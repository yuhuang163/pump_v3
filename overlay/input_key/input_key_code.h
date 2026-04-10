#ifndef __INPUT_KEY_CODE_H__
#define __INPUT_KEY_CODE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/// 按键索引
typedef enum {
    KEY_INDEX_POWER = 0,   
    KEY_INDEX_START,
    KEY_INDEX_MODE,
    KEY_INDEX_FREQUENCY,
    KEY_INDEX_BREASTFEEDING,
    KEY_INDEX_LEFT_CONTROL,
    KEY_INDEX_RIGHT_CONTROL,
    KEY_INDEX_FACTORY_RESET,
    KEY_INDEX_TRAVEL_LOCK,
    KEY_INDEX_KNOB_LEFT,
    KEY_INDEX_KNOB_RIGHT,
    KEY_INDEX_MAX,
} key_index_e;

/// 按键编码
typedef enum {
    KEY_CODE_POWER = (1 << KEY_INDEX_POWER),   /// 支持唤醒
    KEY_CODE_START = (1 << KEY_INDEX_START),   /// 启动键
    KEY_CODE_MODE = (1 << KEY_INDEX_MODE),   /// 模式键
    KEY_CODE_FREQUENCY = (1 << KEY_INDEX_FREQUENCY),   /// 频率键
    KEY_CODE_BREASTFEEDING = (1 << KEY_INDEX_BREASTFEEDING),   /// 追奶键
    KEY_CODE_LEFT_CONTROL = (1 << KEY_INDEX_LEFT_CONTROL),   /// 左控制键
    KEY_CODE_RIGHT_CONTROL = (1 << KEY_INDEX_RIGHT_CONTROL),   /// 右控制键
    KEY_CODE_FACTORY_RESET = (1 << KEY_INDEX_FACTORY_RESET),   /// 工厂重置键
    KEY_CODE_TRAVEL_LOCK = (1 << KEY_INDEX_TRAVEL_LOCK),   /// 旅行锁键
    KEY_CODE_KNOB_LEFT = (1 << KEY_INDEX_KNOB_LEFT),   /// 旋钮逆时针
    KEY_CODE_KNOB_RIGHT = (1 << KEY_INDEX_KNOB_RIGHT),   /// 旋钮顺时针
} input_key_code_e;

/// 包含组合按键
#define ALL_KEY_CODE_WITH_COMBINE (KEY_CODE_POWER | KEY_CODE_START | KEY_CODE_MODE | KEY_CODE_FREQUENCY | KEY_CODE_BREASTFEEDING | \
    KEY_CODE_LEFT_CONTROL | KEY_CODE_RIGHT_CONTROL | KEY_CODE_FACTORY_RESET | KEY_CODE_TRAVEL_LOCK | KEY_CODE_KNOB_LEFT | KEY_CODE_KNOB_RIGHT)  

/// 不包含组合按键
#define ALL_KEY_CODE_NO_COMBINE (KEY_CODE_POWER | KEY_CODE_START | KEY_CODE_MODE | KEY_CODE_FREQUENCY | KEY_CODE_BREASTFEEDING | \
        KEY_CODE_LEFT_CONTROL | KEY_CODE_RIGHT_CONTROL | KEY_CODE_KNOB_LEFT | KEY_CODE_KNOB_RIGHT) 


const char *get_input_key_code_name(uint16_t keycode);

#ifdef __cplusplus
}
#endif

#endif 


