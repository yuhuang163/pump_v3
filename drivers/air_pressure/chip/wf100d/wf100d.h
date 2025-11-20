#ifndef __WF100D_H__
#define __WF100D_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_common.h" // IWYU pragma: keep

/**@brief Firmware information define*/
#define WF100D_CONVERSION_MODE_REG            0x30
#define WF100D_PRESSURE_MSB_REG               0x06
#define WF100D_PRESSURE_CSB_REG               0x07
#define WF100D_PRESSURE_LSB_REG               0x08

#define WF100D_TEMP_MSB_REG                   0x09
#define WF100D_TEMP_LSB_REG                   0x0A

#define WF100D_STATUS_REG                     0x02

/// 转换方式
typedef enum {
    WF100D_SINGLE_CONVERSION = 0x0A,  /// 单次转换
    WF100D_CONTINUE_CONVERSION = 0x0B,  /// 持续转换
    WF100D_62P5MS_PERIOD_CONVERSION = 0x1B, /// 62.5ms周期转换
    WF100D_125MS_PERIOD_CONVERSION = 0x2B,  /// 125ms周期转换
    WF100D_1S_PERIOD_CONVERSION = 0xFB,   /// 1s周期转换
} wf100d_conversion_mode_e;

/// 转换状态
#define WF100D_CONVERSION_COMPLETION     1  /// 转换完成



/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 
