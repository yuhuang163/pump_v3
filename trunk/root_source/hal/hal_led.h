#ifndef __HAL_LED_H__
#define __HAL_LED_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/**
 * @brief 描述LED操作码
*/
typedef enum {
    OP_CODE_LED_STATUS_SET  = 1,
    OP_CODE_LED_STATUS_GET = 2,
    OP_CODE_LED_LEVEL_MAX_SET = 3,  /// 设置亮度系数上限
    OP_CODE_LED_IS_READY = 5,
} led_dev_op_code_e;

/**
 * @brief 描述吸奶器LED索引
*/  
typedef enum {
    PUMP_START_BL,            /// 启动背光灯
    PUMP_MODE_BL,             /// 模式背光灯
    PUMP_FREQ_SET_BL,         /// 频率设置背光灯
    PUMP_BREASTFEEDING_BL,    /// 追奶背光灯
    PUMP_LEFT_CONTROL_BL,     /// 左控制背光灯
    PUMP_RIGHT_CONTROL_BL,    /// 右控制背光灯
    PUMP_LED_INDEX_MAX,
    PUMP_LED_ALL_ID = 0x10,
    NIGHT_LIGHT_LED = 0x11,
} pump_led_index_e;

/**
 * @brief 描述LED状态
*/     
typedef enum {
    LED_STATUS_OFF,           
    LED_STATUS_ON,    
} led_status_e;

/**
 * @brief 描述LED模式
*/
typedef enum {
    LED_MODE_INVALID,       /// led 无效
    LED_MODE_ONOFF,         /// led 开关
    LED_MODE_BLINK,         /// led 闪烁
    LED_MODE_BREATH,        /// led 循环呼吸
    LED_MODE_LEVEL,         /// led 亮度调节
    LED_MODEL_MAX,
} led_mode_e;

typedef void (*led_blink_finish_cb)(void *user_data, uint32_t args);
typedef struct {
    uint16_t period;        /// 周期，单位ms
    uint16_t times;         /// 闪烁次数，0：一直闪烁，单位次
    uint8_t  finish_status; /// 闪烁完成状态
    /// 模块内部使用
    uint8_t  status;        /// 灯状态 1亮，0灭
    uint8_t  level;         /// 亮度 0-100
    uint8_t  times_count;   /// 闪烁次数计数
    uint32_t start_time;    /// 开始时间
} led_blink_t;

typedef void (*led_breath_finish_cb)(void);
typedef struct {
    uint8_t  polarity;            /// 极性
    uint8_t  direction;           /// 呼吸方向 1呼吸+，2呼吸-
    uint16_t step;                /// 脉宽调节幅度
    uint16_t pluse_max;           /// 脉宽最大值
    uint16_t period;              /// 周期，单位ms
    uint32_t start_time;          /// 开始时间
    uint32_t continue_time;       /// 呼吸连续时间，0：一直闪烁，单位ms
    led_breath_finish_cb cb;      /// 呼吸回调
} led_breath_t;

/// 最大亮度值
#define LED_LEVEL_MAX      (100)

typedef struct {
    uint8_t status;    /// 状态 1亮，0灭
    uint8_t  level;    /// 亮度 0-100
} led_onoff_t;

typedef struct {
    uint8_t status;    /// 状态 1亮，0灭
    uint8_t  level;    /// 亮度 0-100
} led_luminous_t;

typedef struct {
    uint8_t  mode;          /// led模式详见led_mode_e
    uint8_t  level_max;     /// 亮度系数上限 0-100
    union {
        led_blink_t    blink;
        led_breath_t   breath;
        led_onoff_t    onoff;
        led_luminous_t luminous;
    } un;
} led_status_t;

typedef struct {
    uint8_t led_index;
    led_status_t led_status;
} led_status_msg_t;

typedef struct {
    uint8_t led_index;
    uint8_t level_max;
} led_level_msg_t;

#ifdef __cplusplus
}
#endif

#endif 
