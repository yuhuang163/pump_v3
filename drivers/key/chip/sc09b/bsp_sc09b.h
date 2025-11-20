#ifndef __BSP_SC09B_H__
#define __BSP_SC09B_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h> // IWYU pragma: keep


/**@brief Firmware information define*/
#define SC09B_SEN_REG0             0x00  /// SC12B为对应CIN3, SC09B SC05B为对应硬件CIN0, 通道灵敏度的设置地址 																					//SC09B SC05B为对应硬件CIN0 通道灵敏度的设置地址
#define SC09B_SEN_REG_COM          0x01  /// 其他通道灵敏度的设置地址
#define SC09B_CTRL0_REG            0x02  /// CTRL0 控制寄存器设置地址
#define SC09B_CTRL1_REG 	       0x03  /// CTRL1 控制寄存器设置地址
#define SC09B_OUTPUT1_REG 		   0x08  /// 触摸状态寄存器输出地址
#define SC09B_OUTPUT2_REG 		   0x09  /// 触摸状态寄存器输出地址
#define SC09B_SAMP_H_REG 	       0x0A  /// 采样值寄存器高8位
#define SC09B_SAMP_L_REG 		   0x0B  /// 采样值寄存器低8位 SAMP[11:0] 采样值  SAMP[15:12] 通道号

/// 采样通道号
#define SC09B_SAMP_CHN_POS 12
#define SC09B_SAMP_CHN_MASK 0xFFF

typedef enum {
    SC09B_SAMPLE_CHANNEL_0 = 4 << SC09B_SAMP_CHN_POS,
    SC09B_SAMPLE_CHANNEL_1 = 5 << SC09B_SAMP_CHN_POS,
    SC09B_SAMPLE_CHANNEL_2 = 6 << SC09B_SAMP_CHN_POS,
    SC09B_SAMPLE_CHANNEL_3 = 7 << SC09B_SAMP_CHN_POS,
    SC09B_SAMPLE_CHANNEL_4 = 8 << SC09B_SAMP_CHN_POS,
    SC09B_SAMPLE_CHANNEL_5 = 9 << SC09B_SAMP_CHN_POS,
    SC09B_SAMPLE_CHANNEL_6 = 10 << SC09B_SAMP_CHN_POS,
    SC09B_SAMPLE_CHANNEL_7 = 11 << SC09B_SAMP_CHN_POS,
    SC09B_SAMPLE_CHANNEL_8 = 12 << SC09B_SAMP_CHN_POS,
} sc09b_sample_channel_e;

/// 灵敏度
typedef enum {
    SC09B_SEN_0 = 0x04,
    SC09B_SEN_1 = 0x15,
    SC09B_SEN_2 = 0x25,
    SC09B_SEN_3 = 0x36,
    SC09B_SEN_4 = 0x47,
    SC09B_SEN_5 = 0x58,
    SC09B_SEN_6 = 0x68,
    SC09B_SEN_7 = 0x79,
    SC09B_SEN_8 = 0x8A,
    SC09B_SEN_9 = 0x9B,
    SC09B_SEN_10 = 0xAC,
    SC09B_SEN_11 = 0xBC,
    SC09B_SEN_12 = 0xCD,
    SC09B_SEN_13 = 0xDE,
    SC09B_SEN_14 = 0xEF,
    SC09B_SEN_15 = 0xFF,
    SC09B_SEN_DEFAULT = SC09B_SEN_7,
} sc09b_sen_e;

/// 休眠后采样间隔
#define SC09B_SLPCYC_POS   5
typedef enum {
    SC09B_SLPCYC_LGT  = 0x00 << SC09B_SLPCYC_POS,
    SC09B_SLPCYC_0R5T = 0x01 << SC09B_SLPCYC_POS,
    SC09B_SLPCYC_1R5T = 0x02 << SC09B_SLPCYC_POS,
    SC09B_SLPCYC_2R5T = 0x03 << SC09B_SLPCYC_POS,
    SC09B_SLPCYC_3R5T = 0x04 << SC09B_SLPCYC_POS,
    SC09B_SLPCYC_4R5T = 0x05 << SC09B_SLPCYC_POS,
    SC09B_SLPCYC_5R5T = 0x06 << SC09B_SLPCYC_POS,
    SC09B_SLPCYC_6R5T = 0x07 << SC09B_SLPCYC_POS,
    SC09B_SLPCYC_DEFAULT = SC09B_SLPCYC_3R5T,
} sc09b_slpcyc_e;

/// 空闲休眠延时
#define SC09B_IDLE_SLEEP_DELAY_POS   4
typedef enum {
    SC09B_IDLE_SLEEP_DELAY_75S = 0x00 << SC09B_IDLE_SLEEP_DELAY_POS,
    SC09B_IDLE_SLEEP_NO_DELAY = 0x01 << SC09B_IDLE_SLEEP_DELAY_POS,
    SC09B_IDLE_SLEEP_DELAY_DEFAULT = SC09B_IDLE_SLEEP_DELAY_75S,
} sc09b_idle_sleep_delay_e;

/// 校准触发方式
#define SC09B_CALIBRATION_TRIGGER_METHOD_POS   4
typedef enum {
    SC09B_PRESS_50S_ENTER_CALIBRATION = 0x00 << SC09B_CALIBRATION_TRIGGER_METHOD_POS,  /// 按键50s后进去校准
    SC09B_ON_RELEASE_ENTER_CALIBRATION = 0x01 << SC09B_CALIBRATION_TRIGGER_METHOD_POS, /// 按键停止后进入校准
    SC09B_ENTER_CALIBRATION_DEFAULT = SC09B_PRESS_50S_ENTER_CALIBRATION,
} sc09b_calibration_trigger_method_e;

/// 校准方式
#define SC09B_CALIBRATION_METHOD_POS   3
typedef enum {
    SC09B_NOMAL_CALIBRATION = 0x00 << SC09B_CALIBRATION_METHOD_POS, /// 正常校准
    SC09B_STOP_CALIBRATION_REFERENCE_VALUE = 0x01 << SC09B_CALIBRATION_METHOD_POS,  /// 停止基准值校准
    SC09B_CALIBRATION_DEFAULT_METHOD = SC09B_NOMAL_CALIBRATION,
} sc09b_calibration_method_e;

/// 按键反应速度
typedef enum {
    SC09B_RTM0 = 0x00, /// 按键按下3个周期有效，释放1个周期有效
    SC09B_RTM1, /// 按键按下4个周期有效，释放2个周期有效
    SC09B_RTM2, /// 按键按下5个周期有效，释放3个周期有效
    SC09B_RTM3, /// 按键按下6个周期有效，释放4个周期有效
    SC09B_RTM_DEFALUT = SC09B_RTM3, 
} sc09b_trigger_time_e;

/// 基准通道电容选择
typedef enum {
    SC09B_CAP_4PF = 0x04, 
    SC09B_CAP_5PF, 
    SC09B_CAP_6PF,
    SC09B_CAP_7PF,
    SC09B_CAP_8PF,
    SC09B_CAP_9PF,
    SC09B_CAP_10PF,
    SC09B_CAP_11PF,
    SC09B_CAP_12PF,
    SC09B_CAP_13PF,
    SC09B_CAP_14PF,
    SC09B_CAP_15PF,
    SC09B_CAP_DEFALUT = SC09B_CAP_8PF, 
} sc09b_reference_capacitance_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/


#ifdef __cplusplus
}
#endif

#endif 
