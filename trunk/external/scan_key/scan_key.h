#ifndef __SCAN_KEY_H__
#define __SCAN_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h>

#ifndef KEY_S1_ID
#define KEY_S1_ID                        0
#define KEY_S1_TYPE                      0
#endif

#ifndef POWER_KEY_ID
#define POWER_KEY_ID                     0
#define POWER_KEY_TYPE                   0
#endif

#ifndef MODE_KEY_ID
#define MODE_KEY_ID                      1
#define MODE_KEY_TYPE                    0
#endif

#ifndef FAST_MMI_ID
#define FAST_MMI_ID                      2
#define FAST_MMI_TYPE                    0
#endif

typedef enum {
    SCAN_KEY_BEGIN = 0,
    SCAN_KEY_POWER = SCAN_KEY_BEGIN,
    SCAN_KEY_MODE,
    SCAN_KEY_MMI,
    SCAN_KEY_MAX,
} scan_key_id_e;

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
int scan_key_input_init(struct rt_device_node *key_dev);

#ifdef __cplusplus
}
#endif

#endif 


