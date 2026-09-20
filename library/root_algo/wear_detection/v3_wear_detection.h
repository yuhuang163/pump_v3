/*****************************************************************************************************************
版权所有：Shenzhen Root Inonvation
项目名称：V3
项目编号：
文 件 名：
生成日期：2025.12.19
作    者：sahar.hu
功能说明：V3 wear detection functions
其他说明：
修改记录：
修改日期：
修 改 者：
修改内容：
******************************************************************************************************************/



#ifndef V3_WEAR_DETECTION_H
#define V3_WEAR_DETECTION_H
#include "v3_Defined.h"
#include "v3_target_curve_table.h"

#ifdef FOR_PC_DISPLAY
#include "MultiParamExter.h"

typedef struct _deubg_info
{
	int       mNoiseLevel[2];
	int       mWearStatus[2];
	int       mWaveDetect[2];
	int       mFilteredPress[2];
	int       mBaseMinPress[2];
	int       mRawMinEndPress[2];
	int       mRawMinPress[2];
	int       mSpikes[2];
	int       mWearOnCount[2];
	int       mWearOffCount[2];
	int       mNormalWaveform[2];
	int       mSwitchTimes[2];
}DEBUGINFO;
#endif

/* interface */
// 初始化
void V3WearDetectionInit(unsigned char uLevelLeft, unsigned char ulevelRight, unsigned char uModeLeft, unsigned char uModeRight);

// 逐点计算主函数
void V3WearDetectionMain(algo_air_pressure_data_t sPressureData);

// 佩戴、漏气、未佩戴超时状态获取
void GetWearDetectionResult(unsigned char* uWearStatus, unsigned char* uLeakStatus, unsigned char* DetectOverTime);

//补偿状态获取
void GetCompensationStatus(unsigned char* uCompsStatus, int* iDelta_pwm);

// 版本号获取
const char* v3_pressure_algo_get_version(void);

void GetWearDetectionDebugPackage(SRESULT_INFO *result_info);

// 用于上位机调试，可忽略
#ifdef FOR_PC_DISPLAY
void GetDebugInfo(DEBUGINFO* DebugInfo);
#endif

#endif
