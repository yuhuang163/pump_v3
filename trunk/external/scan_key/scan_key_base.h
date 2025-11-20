#ifndef __SCAN_KEY_BASE_H__
#define __SCAN_KEY_BASE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h>

/**@brief Firmware information define*/
/// 要扫描的按键数
#define DE_SK_COUNT                          (1)           
/**< 按键消抖需要的循环滴答次数(30ms,100ms) */
#define DE_SK_DEBOUNCE_TICK                  (3)  
/**< 判断双击动作,按键第一次抬起的最长滴答次数(200ms,400ms) */
#ifndef DE_SK_INTERVEL_TICK
#define DE_SK_INTERVEL_TICK                  (1) 
#endif
/**< 按键长按开始所经历的滴答次数(2500ms,5000ms) */
#define DE_SK_LONG_START_TICK                (200)
/**< 按键长按触发连击时所需的连击间隔滴答次数(4000ms,8000ms) */
#define DE_SK_CONTINUE_START_TICK            (400) 
/**< 按键长按触发连击时所需的连击间隔滴答次数(250ms,500ms) */
#define DE_SK_CONTINUE_TICK                  (100) 

#define DE_SK_STATUS_START                   (0) /**< 起始状态 */
#define DE_SK_STATUS_DOWN                    (1) /**< 按下状态 */
#define DE_SK_STATUS_UP                      (2) /**< 按下抬起状态 */
#define DE_SK_STATUS_DOWN_AGAIN              (3) /**< 再次按下状态 */
#define DE_SK_STATUS_DOWN_LONG               (4) /**< 长按状态 */
#define DE_SK_STATUS_DOWN_CONTINUE           (5) /**< 连按状态 */

#define DE_SK_EVENT_DOWN                     (0) /**< 按下事件 */
#define DE_SK_EVENT_DOWN_YES                 (1) /**< 按下通过抖动事件 */
#define DE_SK_EVENT_DOWN_NO                  (2) /**< 按下不通过抖动事件 */
#define DE_SK_EVENT_UP                       (3) /**< 抬起事件 */
#define DE_SK_EVENT_UP_YES                   (4) /**< 抬起通过抖动事件 */
#define DE_SK_EVENT_UP_NO                    (5) /**< 抬起不通过抖动事件 */
#define DE_SK_EVENT_DOWN_AGAIN               (6) /**< 再次按下事件 */
#define DE_SK_EVENT_DOWN_AGAIN_YES           (7) /**< 再次按下通过抖动事件 */
#define DE_SK_EVENT_DOWN_AGAIN_NO            (8) /**< 再次按下不通过抖动事件 */
#define DE_SK_EVENT_DOUBLE_NO                (9) /**< 非双击事件 */
#define DE_SK_EVENT_UP_AGAIN                 (10) /**< 再次抬起事件 */
#define DE_SK_EVENT_UP_AGAIN_YES             (11) /**< 再次抬起通过抖动事件 */
#define DE_SK_EVENT_UP_AGAIN_NO              (12) /**< 再次抬起不通过抖动事件 */
#define DE_SK_EVENT_DOWN_LONG_YES            (13) /**< 长按确认事件 */
#define DE_SK_EVENT_UP_LONG                  (14) /**< 长按抬起事件 */
#define DE_SK_EVENT_UP_LONG_YES              (15) /**< 长按抬起通过抖动事件 */
#define DE_SK_EVENT_UP_LONG_NO               (16) /**< 长按抬起不通过抖动事件 */
#define DE_SK_EVENT_DOWN_CONTINUE_YES        (17) /**< 连按确认开始事件 */
#define DE_SK_EVENT_UP_CONTINUE              (18) /**< 连按抬起事件 */
#define DE_SK_EVENT_UP_CONTINUE_YES          (19) /**< 连按抬起通过抖动事件 */
#define DE_SK_EVENT_UP_CONTINUE_NO           (20) /**< 连按抬起不通过抖动事件 */

struct _scan_key_data;
typedef void(*u_scan_key_cb)(struct _scan_key_data* scan_key, uint8_t code_index); /**< 定义扫描按键回调处理函数指针类型 */

typedef struct _scan_key_data {
    uint8_t up_status;                          /**< 按键抬起状态机，1表示按键完全抬起，0表示按键未完全抬起，用来适配组合按键 */
    uint8_t event;                              /**< 扫描按键的事件 */
    uint8_t status;                             /**< 扫描按键的状态机 */
    uint8_t debounce_tick;                      /**< 按键消抖经历的滴答时间 */
    uint8_t level;                              /**< 激活电平 */
    uint16_t double_start_tick;                 /**< 记录按键双击开始的滴答时刻和连击开始的滴答时刻 */
    uint16_t scan_count;                        /**< 按键经历的滴答时间消抖验证通过后为非0值 */
    uint32_t code_value;                        /**< 按键值 */
    u_scan_key_cb cb;                           /**< 按键事件回调函数 */
} scan_key_data_t;

typedef struct {
    uint8_t hold;                              /**< hold(继续、连续)按键按下保持的信号 */
    uint8_t triger;                            /**< triger(触发)对应按键按下的信号,记录当前扫描是否有新的按键按下,有的话对应的位置1 */
    uint8_t reset;                             /**< 记录由按下到抬起的触发 */
}scan_key_ctrl_data_t;

/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */
void v_scan_key(scan_key_ctrl_data_t *key_ctrl, scan_key_data_t *key_data, uint8_t code_size);

#ifdef __cplusplus
}
#endif

#endif 
