// /*****************************************************************************************************************
// 版权所有：Shenzhen Root Inonvation
// 项目名称：V3pro
// 项目编号：
// 文 件 名：
// 生成日期：2026.03.26
// 作    者：sahar.hu
// 功能说明：V3pro pressure compensation functions
// 其他说明：
// 修改记录：
// 修改日期：
// 修 改 者：
// 修改内容：
// ******************************************************************************************************************/

// #pragma once
// #ifndef V3_PRESSURE_COMPENSATION_H
// #define V3_PRESSURE_COMPENSATION_H

// #include "v3_Defined.h"
// #include "v3_target_curve_table.h"

// // functions
// void V3ProModeReset(algo_pump_state_t pump_state, unsigned char channel_idx);
// void V3ProPressureDataPreprocess(algo_air_pressure_data_t sPressureData, unsigned char uchannelIdx);
// void PressureCompensationAnalysis(float fDataBuff, SPUMPFEATURE_INFO*tPumpFeatureInfoTemp, SDEVICE_INFO *tDeviceInfoTemp);
// // interface
// void V3ProAlgoInit(algo_air_pressure_data_t sPressureData);
// void V3ProAlgoMain(algo_air_pressure_data_t sPressureData);
// void GetCompensationResult(unsigned char* compensation_status, int *compensation_delta_pwm);
// /* get debug info */
// void GetPidDebugInfo(unsigned char channel_idx, SPID_DEBUG_INFO* ptDebugInfo);
// #endif
/*****************************************************************************************************************
版权所有：Shenzhen Root Inonvation
项目名称：V3pro
项目编号：
文 件 名：
生成日期：2026.03.26
作    者：sahar.hu
功能说明：V3pro pressure compensation functions
其他说明：
修改记录：
修改日期：
修 改 者：
修改内容：
******************************************************************************************************************/
#ifndef V3_PRESSURE_COMPENSATION_H
#define V3_PRESSURE_COMPENSATION_H
#include "v3_Defined.h"

/* functions*/
void V3ProModeReset(algo_pump_state_t pump_state, unsigned char channel_idx);
void V3ProPressureDataPreprocess(algo_air_pressure_data_t sPressureData, unsigned char uchannelIdx);
void PressureCompensationAnalysis(float fDataBuff, SPUMPFEATURE_INFO* tPumpFeatureInfoTemp, SDEVICE_INFO* tDeviceInfoTemp);

/* interface*/
void V3ProAlgoInit(algo_air_pressure_data_t sPressureData);
void V3ProAlgoMain(algo_air_pressure_data_t sPressureData);
void GetPumpCompensationResult(unsigned char* uCompsStatus, int* iDeltaPWM);

#endif