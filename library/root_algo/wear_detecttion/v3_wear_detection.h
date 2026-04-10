/*****************************************************************************************************************
版权所有：Shenzhen Root Inonvation
项目名称：Air2
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

#ifdef __cplusplus
extern "C" {
#endif


/*functions*/
unsigned int BufferCircleNextIndex(unsigned int uCurrPos, unsigned int uIndent, unsigned int uBufferLen);
unsigned int BufferCircleRelativeIndex(unsigned int uPos, unsigned int uIdx, unsigned int uBufferLen, unsigned int uFirstTime);
void PressureDataPreprocess(float fPumpData, SFILTER_INFO* tFilterInfoTemp, SDATA_BUFFER* tDataBufferTemp, SFEATURE_INFO* tFeatureInfoTemp);
void PeakValleyAnalysis(SPV_INFO* tPVInfoTemp, SWAVE_INFO* tWaveInfoTemp, SDATA_BUFFER* tDataBufferTemp, SFEATURE_INFO* tFeatureInfoTemp);
void PressureDataFeature(SPV_INFO* tPVInfoTemp, SWAVE_INFO* tWaveInfoTemp, SDATA_BUFFER* tDataBufferTemp, SFEATURE_INFO* tFeatureInfoTemp, SDEVICE_INFO* tDeviceInfoTemp);
void WearDetectionDetermination(SFEATURE_INFO* tFeatureInfoTemp, SDEVICE_INFO* tDeviceInfoTemp);
void V3WearDetectionModeReset(unsigned char uLevel, unsigned char uMode, unsigned int uChannelIdx);

/*interface*/
void V3WearDetectionInit(unsigned char uLevelLeft, unsigned char ulevelRight, unsigned char uModeLeft, unsigned char uModeRight);
void V3WearDetectionMain(float* fPressureData, algo_air_pressure_data_t sPressureData, unsigned char uLevelLeft, unsigned char ulevelRight, unsigned char uModeLeft, unsigned char uModeRight);
void GetWearDetectionResult(unsigned char* uWearStatus);
void GetCompensationStatus(unsigned char* uCompsStatus, int* iDelta_pwm);

/*void GetDebugInfo(SRESULT_INFO *result_info);*/
#endif
#ifdef __cplusplus
}
#endif

//#endif