#ifndef V3_TARGET_CURVE_TABLE_H
#define V3_TARGET_CURVE_TABLE_H

#include "v3_Defined.h"

/* 目标曲线总表，后续由你手动补充真实曲线项 */
extern const STARGET_CURVE_ITEM g_target_curve_table[];
/* 目标曲线总表项数 */
extern const unsigned int g_target_curve_table_count;

/* 根据 target_pressure_x10 和 target_pumping_time 查找对应的目标曲线 */
const STARGET_CURVE_ITEM* FindTargetCurve(unsigned char target_pressure,
                                          unsigned short target_pumping_time);

/* 在已找到的目标曲线上，按当前 pumping_time 取 P1 目标压力值
 * is_valid 非空时，用于返回本次取值是否成功
 */
float GetTargetPressureFromCurve(const STARGET_CURVE_ITEM* curve,
                                 unsigned short pumping_time,
                                 unsigned char* is_valid);
const STARGET_CURVE_ITEM* GetCurrentTargetCurveItem(const SDEVICE_INFO* ptDeviceInfo);

#endif
