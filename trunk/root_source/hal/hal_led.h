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
    OP_CODE_NIGHT_LIGHT_STATUS_SET  = 3,
    OP_CODE_NIGHT_LIGHT_STATUS_GET  = 4,
} led_dev_op_code_e;

/**
 * @brief 描述RGB LED ID
*/
typedef enum {
    RGB_LED_ID_0 = 0,
    RGB_LED_ID_1,
    RGB_LED_ID_2,
    RGB_LED_ID_3,
    RGB_LED_ID_4,

    RGB_ALL_LED_ID = 0xFF,
} rgb_led_id_e;

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
    PUMP_BILATERRAL_CONTROL_BL,    /// 双边控制背光灯
    PUMP_LED_INDEX_MAX,
    PUMP_LED_ALL_ID = 0xFF,
} pump_led_index_e;

/**
 * @brief 描述夜灯LED索引
*/
typedef enum {
    NIGHT_LIGHT_LED_ID_0 = 0,
    NIGHT_LIGHT_INDEX_MAX,
    NIGHT_LIGHT_LED_ALL_ID = 0xFF,
} night_light_led_index_e;

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
    uint8_t  first_status;  /// 闪烁初始状态
    uint16_t period;        /// 周期，单位ms
    uint32_t continue_time; /// 闪烁连续时间，0：一直闪烁，单位ms
    led_blink_finish_cb cb; /// 闪烁回调
    void *user_data;        /// 上下文
    /// 模块内部使用
    uint8_t  status;        /// 灯状态 1亮，0灭
    uint16_t count;         /// 周期
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

typedef struct {
    uint32_t status;    /// 状态 1亮，0灭
} led_onoff_t;

/// 最大亮度值
#define LED_LEVEL_MAX      10

typedef struct {
    uint8_t  level;    /// 亮度 0-100
} led_luminous_t;

typedef struct {
    uint8_t  mode;          /// led模式详见led_mode_e
    union {
        led_blink_t blink;
        led_breath_t breath;
        led_onoff_t onoff;
        led_luminous_t luminous;
    } un;
} led_status_t;

#ifdef __cplusplus
}
#endif

#endif 
