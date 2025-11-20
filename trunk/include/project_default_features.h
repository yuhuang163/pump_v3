#ifndef __PROJECT_DEFAULT_FEATURES_H__
#define __PROJECT_DEFAULT_FEATURES_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/


/**
 * 开机长按时间
*/
#ifndef BOOT_LONG_PRESS_TIME
#define BOOT_LONG_PRESS_TIME                                         (2000) /// 单位：毫秒
#endif

/**
 * 屏幕显示分辨率.
*/
#ifndef PANLE_DISPLAY_W
#define PANLE_DISPLAY_W                                              (80)
#endif

#ifndef PANLE_DISPLAY_H
#define PANLE_DISPLAY_H                                              (160)
#endif

/**
 * 输入按键任务投递到UI(WM)线程
 */
#ifndef IS_INPUT_KEY_TASK_POST_UI_THREAD
#define IS_INPUT_KEY_TASK_POST_UI_THREAD                             (1)
#endif

/**
 * 10秒长按软复位
*/
#ifndef IS_SUPPORT_KEY_SOFT_RESET
#define IS_SUPPORT_KEY_SOFT_RESET                                    (0)
#endif

/**
 * OTA尝试本地恢复.
*/
#ifndef IS_SUPPORT_OTA_LOCAL_RECOVERY
#define IS_SUPPORT_OTA_LOCAL_RECOVERY                                (0)
#endif

/**
 * 重置不解绑
*/
#ifndef IS_SUPPORT_RESET_UNBINDING
#define IS_SUPPORT_RESET_UNBINDING                                   (0)
#endif

/**
 * 模式记忆排序
*/
#ifndef USING_MODE_REMEMBER_SORT 
#define USING_MODE_REMEMBER_SORT                                     (0)
#endif

/**
 * 算法采测支持.
*/
#ifndef IS_SUPPORT_ALGO_TEST_TOOL
#define IS_SUPPORT_ALGO_TEST_TOOL                                    (0)
#endif

/**
 * 进入飞行模式弹窗时间，默认2000
*/
#ifndef POP_AIRPLANE_WINDOW_TIME
#define POP_AIRPLANE_WINDOW_TIME                                     (2000)
#endif

/**
 * 马达模式参数配置
*/
/// 刺激模式参数长度，短吸
#ifndef STIMULATION_MODE_LEN             
#define STIMULATION_MODE_LEN                                         (1)
#endif

/// 吸乳模式参数长度，长吸
#ifndef PUMP_MODE_LEN
#define PUMP_MODE_LEN                                                (1)
#endif

/// 混合模式参数长度，短吸 + 长吸
#ifndef MIX_MODE_LEN
#define MIX_MODE_LEN                                                 (1 * 2)
#endif

/// 个性化模式默认参数长度，按摩 + 吸乳
#ifndef CUSTOM_MODE_LEN 
#define CUSTOM_MODE_LEN                                              (1 + 1)
#endif

/// PP追奶模式参数长度 吸乳 + 停顿
#ifndef PP_MODE_LEN 
#define PP_MODE_LEN                                                  (5 * 1)
#endif

/// 2+8奶量提升模式参数长度 吸乳 + 停顿
#ifndef MILK_BOOST_MODE_LEN 
#define MILK_BOOST_MODE_LEN                                          (2 * 1)
#endif

/// 开奶模式 吸乳 + 停顿
#ifndef LACTATION_START_MODE_LEN 
#define LACTATION_START_MODE_LEN                                     (2 * 1)
#endif

/// 吸奶器泵数据
#ifndef BREAST_PUMP_NUM
#define BREAST_PUMP_NUM                                              (2)
#endif

/**
 * 陀螺仪在线标定支持.
*/
#ifndef IS_USE_GYRO_ONLINE_CALI
#define IS_USE_GYRO_ONLINE_CALI                                      (0)
#endif

/**
 * imu fifo支持.
*/
#ifndef IS_SUPPORT_IMU_FIFO
#define IS_SUPPORT_IMU_FIFO                                         (0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PROJECT_DEFAULT_FEATURES_H__ */ 


