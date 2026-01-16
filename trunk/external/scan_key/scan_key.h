#ifndef __SCAN_KEY_H__
#define __SCAN_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/// 按键索引
typedef enum scan_key_id_e {
    SCAN_KEY_ID_BEGIN = 0,
    SCAN_KEY_ID1 = SCAN_KEY_ID_BEGIN,
    SCAN_KEY_ID2,
    SCAN_KEY_ID3,
    SCAN_KEY_ID4,
    SCAN_KEY_ID5,
    SCAN_KEY_ID6,
    SCAN_KEY_ID7,
    SCAN_KEY_ID_MAX,
} scan_key_id_e;

/// 按键编码
typedef enum {
    SCAN_KEY_CODE_BEGIN = 0,
    SCAN_KEY_CODE_1 = SCAN_KEY_CODE_BEGIN,
    SCAN_KEY_CODE_2,
    SCAN_KEY_CODE_3,
    SCAN_KEY_CODE_4,
    SCAN_KEY_CODE_5,
    SCAN_KEY_CODE_6,
    SCAN_KEY_CODE_7,
    SCAN_KEY_CODE_12,
    SCAN_KEY_CODE_23,
    SCAN_KEY_CODE_MAX,
} scan_key_code_e;

/// 按键值
typedef enum {
    SCAN_KEY_CODE_1_VALUE  = (1 << SCAN_KEY_CODE_1),
    SCAN_KEY_CODE_2_VALUE  = (1 << SCAN_KEY_CODE_2),
    SCAN_KEY_CODE_3_VALUE  = (1 << SCAN_KEY_CODE_3),
    SCAN_KEY_CODE_4_VALUE  = (1 << SCAN_KEY_CODE_4),
    SCAN_KEY_CODE_5_VALUE  = (1 << SCAN_KEY_CODE_5),
    SCAN_KEY_CODE_6_VALUE  = (1 << SCAN_KEY_CODE_6),
    SCAN_KEY_CODE_7_VALUE  = (1 << SCAN_KEY_CODE_7),

    SCAN_KEY_CODE_13_VALUE = (SCAN_KEY_CODE_1_VALUE | SCAN_KEY_CODE_3_VALUE),
    SCAN_KEY_CODE_23_VALUE = (SCAN_KEY_CODE_2_VALUE | SCAN_KEY_CODE_3_VALUE),
    SCAN_KEY_CODE_56_VALUE = (SCAN_KEY_CODE_5_VALUE | SCAN_KEY_CODE_6_VALUE),
    SCAN_KEY_CODE_67_VALUE = (SCAN_KEY_CODE_6_VALUE | SCAN_KEY_CODE_7_VALUE),
} scan_key_code_value_e;

/// 按键类型
typedef enum {
    KEY_TYPE_BEGIN = 0,
    KEY_TYPE_DOWN = KEY_TYPE_BEGIN,     /// 按下. 
    KEY_TYPE_UP,                        /// 抬起
    KEY_TYPE_CLICK,                     /// 单击
    KEY_TYPE_DOUBLE,                    /// 双击
    KEY_TYPE_LONG_2S,                   /// 长按2s
    KEY_TYPE_LONG_3S,
    KEY_TYPE_LONG_4S,
    KEY_TYPE_LONG_5S,
    KEY_TYPE_LONG_6S,
    KEY_TYPE_LONG_7S,
    KEY_TYPE_LONG_8S,
    KEY_TYPE_LONG_9S,
    KEY_TYPE_LONG_10S,
    KEY_TYPE_LONG_AFTER_DOWN,         /// 长按后接续按下
    KEY_TYPE_LONG_AFTER_CLICK,        /// 长按后接续单击
    KEY_TYPE_MAX,
} scan_key_type_e;

/// 设备节点
struct rt_device_node;

/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */

/// 按键IRQ调用此接口,通知启动扫描定时器.
void scan_key_timer_start(uint8_t code_index);
/// 具体平台读取io状态接口
int platform_read_key_value(int type, int pin);
/// 具体平台读取所有io状态接口
int platform_all_key_value(void);
/// 初始化scankey
int scan_key_input_init(struct rt_device_node *key_dev, const uint32_t *code_value, uint8_t key_code_size);

#ifdef __cplusplus
}
#endif

#endif 


