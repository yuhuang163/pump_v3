#ifndef __LIMS_H__
#define __LIMS_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_common.h" // IWYU pragma: keep

/**@brief Firmware information define*/
#define LIMS_CONVERSION_MODE_REG            0x5B
#define LIMS_PRESSURE_MSB_REG               0x55
#define LIMS_PRESSURE_CSB_REG               0x56
#define LIMS_PRESSURE_LSB_REG               0x57

#define LIMS_TEMP_MSB_REG                   0x58
#define LIMS_TEMP_LSB_REG                   0x59

#define LIMS_STATUS_REG                     0x54
#define LIMS_RESET_REG                      0x50

#define LIMS_PRESSURE_CONVERSION_TIME       7 /// 转换时间，单位ms
#define LIMS_TEMP_CONVERSION_TIME           3 /// 转换时间，单位ms

/// 压力转换方式
typedef enum {
    LIMS_128X_PRESSURE_CONVERSION  = 0xE0,    /// 128X压力转换
    LIMS_256X_PRESSURE_CONVERSION  = 0xE1,    /// 256X压力转换
    LIMS_512X_PRESSURE_CONVERSION  = 0xE2,    /// 512X压力转换
    LIMS_1024X_PRESSURE_CONVERSION = 0xE3,   /// 1024X压力转换
    LIMS_2048X_PRESSURE_CONVERSION = 0xE4,   /// 2048X压力转换
    LIMS_4096X_PRESSURE_CONVERSION = 0xE5,   /// 4096X压力转换
    LIMS_8192X_PRESSURE_CONVERSION = 0xE6,   /// 8192X压力转换
} lims_pressure_conversion_mode_e;

/// 温度转换方式
typedef enum {
    LIMS_128X_TEMP_CONVERSION  = 0xA0,    /// 128X温度转换
    LIMS_256X_TEMP_CONVERSION  = 0xA1,    /// 256X温度转换
    LIMS_512X_TEMP_CONVERSION  = 0xA2,    /// 512X温度转换
    LIMS_1024X_TEMP_CONVERSION = 0xA3,   /// 1024X温度转换
    LIMS_2048X_TEMP_CONVERSION = 0xA4,   /// 2048X温度转换
    LIMS_4096X_TEMP_CONVERSION = 0xA5,   /// 4096X温度转换
    LIMS_8192X_TEMP_CONVERSION = 0xA6,   /// 8192X温度转换
} lims_temp_conversion_mode_e;

/// 数据类型
typedef enum {
    LIMS_PRESSURE_DATA = 0,    /// 压力数据
    LIMS_TEMP_DATA = 1,        /// 温度数据
} lims_conversion_data_type_e;

/// 转换状态
#define LIMS_CONVERSION_COMPLETION          1  /// 转换完成

/// 复位
#define LIMS_RESET_VALUE                    (1 << 5)



/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 
