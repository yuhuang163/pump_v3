#ifndef __SCAN_KEY_H__
#define __SCAN_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

typedef enum scan_key_id_e {
    SCAN_KEY_ID_BEGIN = 0,
    SCAN_KEY_ID1 = SCAN_KEY_ID_BEGIN,
    SCAN_KEY_ID2,
    SCAN_KEY_ID3,
    SCAN_KEY_ID4,
    SCAN_KEY_ID5,
    SCAN_KEY_ID6,
    SCAN_KEY_ID_MAX,
} scan_key_id_e;

typedef enum {
    SCAN_KEY_CODE_BEGIN = 0,
    SCAN_KEY_CODE_1 = SCAN_KEY_CODE_BEGIN,
    SCAN_KEY_CODE_2,
    SCAN_KEY_CODE_3,
    SCAN_KEY_CODE_4,
    SCAN_KEY_CODE_5,
    SCAN_KEY_CODE_6,
    SCAN_KEY_CODE_MAX,
} scan_key_code_e;

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
  
    KEY_TYPE_MAX,
} scan_key_type_e;

struct rt_device_node;

/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */

/// 按键IRQ调用此接口,通知启动扫描定时器.
void scan_key_timer_start(int key);
/// 具体平台读取io状态接口
int platform_read_key_value(int type, int pin);
/// 初始化scankey
int scan_key_input_init(struct rt_device_node *key_dev, uint8_t key_code_size);

#ifdef __cplusplus
}
#endif

#endif 


