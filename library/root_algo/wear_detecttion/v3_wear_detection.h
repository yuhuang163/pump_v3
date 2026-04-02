/*****************************************************************************************************************
版权所有：Shenzhen Root Inonvation
项目名称：Air2
项目编号：
文 件 名：
生成日期：2025.12.19
作    者：sahar.hu
功能说明：Air2 wear detection functions
其他说明：
修改记录：
修改日期：
修 改 者：
修改内容：
******************************************************************************************************************/
#ifndef V3_WEAR_DETECTION_H
#define V3_WEAR_DETECTION_H

#ifdef __cplusplus
extern "C" {
#endif
#if defined(ALGO_TYPE_V3_WEAR_DETECTION)
// define modes
#define MODE_STIMULATION	(0)
#define MODE_EXPRESSION		(1)
#define MODE_MIXED			(2)

//define levels
#define LEVEL_1				(1)
#define LEVEL_2				(2)
#define LEVEL_3				(3)
#define LEVEL_4				(4)
#define LEVEL_5				(5)
#define LEVEL_6				(6)
#define LEVEL_7				(7)
#define LEVEL_8				(8)
#define LEVEL_9				(9)
#define LEVEL_10			(10)
#define LEVEL_11			(11)
#define LEVEL_12			(12)
#define LEVEL_13			(13)
#define LEVEL_14			(14)
#define LEVEL_15			(15)

// hardware parameters
#define SAMPLE_RATE			(50)
#define WIN_SIZE			(50*3)  //(in second)
#define CHANNEL_NUM			(2)		// 2 channels - left and right
#define CHANNEL_0			(0)		// 0: left
#define CHANNEL_1			(1)		// 1: right
// structures

#ifndef abs
#define abs(x)                              ((x) < 0 ? -(x) : (x))
#endif

#ifndef fabs
#define fabs(x)                              ((x) < 0.0f ? -(x) : (x))
#endif

typedef struct _DEVICE_INFO
{
	unsigned short	uMode;
	unsigned short	uLevel;
}SDEVICE_INFO;

typedef struct _PV_INFO
{
	unsigned short	uType[100];
	unsigned int	uPos[100];
	float			fValue[100];
}SPV_INFO;

typedef struct _WAVE_INFO
{
	unsigned short	uType[100];
	unsigned int	uPos[100];
	float			fValue[100];
	float			fLeftAmp[100];
	float			fRightAmp[100];
	unsigned int	uwidth[100];
}SWAVE_INFO;


typedef struct _FEATURE_INFO_
{
	unsigned int	uPVIndex;
	unsigned int	uWaveSpikes;
	float			fMinData;
	unsigned int	uNoiseLevel;
	unsigned int	uDataIdx;		// data index
	unsigned short	uUpdateFlag;
	float			fThreshold;
	unsigned char	uWearStatus;	// result
	unsigned char	uPrevWearStatus;
	unsigned int	uNormalWaveform;
}SFEATURE_INFO;

typedef struct _DATA_BUFFER_
{
	float			fDataBuff[WIN_SIZE];
	float			fDataMax;
	float			fDataMin;
	unsigned int	uCurrPos;		// current pos
	unsigned short  uFirstTime;
}SDATA_BUFFER;


typedef struct _FILTER_INFO_
{
	float			fFc;
	float			fDT;
	float			fRC;
	float			fAlpha;
	float			fYpre;
	unsigned short	uInitState;
}SFILTER_INFO;

// functions
unsigned int BufferCircleNextIndex(unsigned int uCurrPos, unsigned int uIndent, unsigned int uBufferLen);
unsigned int BufferCircleRelativeIndex(int uCurrPos, unsigned int uIdx, unsigned int uBufferLen, unsigned short uFirstTime);
void PressureDataPreprocess(float fPumpData, SFILTER_INFO* tFilterInfoTemp, SDATA_BUFFER* tDataBufferTemp, SFEATURE_INFO* tFeatureInfoTemp);
void PeakValleyAnalysis(SPV_INFO* tPVInfoTemp, SWAVE_INFO* tWaveInfoTemp, SDATA_BUFFER* tDataBufferTemp, SFEATURE_INFO* tFeatureInfoTemp);
void PressureDataFeature(SPV_INFO* tPVInfoTemp, SWAVE_INFO* tWaveInfoTemp, SDATA_BUFFER* tDataBufferTemp, SFEATURE_INFO* tFeatureInfoTemp, SDEVICE_INFO* tDeviceInfoTemp);
void WearDetectionDetermination(SFEATURE_INFO* tFeatureInfoTemp, SDEVICE_INFO* tDeviceInfoTemp);

// interface
void V3WearDetectionInit(unsigned char uLevelLeft, unsigned char ulevelRight, unsigned char uModeLeft, unsigned char uModeRight);
void V3WearDetectionModeReset(unsigned char uLevel, unsigned char uMode, unsigned int uChannelIdx);
void V3WearDetectionMain(float* fPressureData, unsigned char uLevelLeft, unsigned char ulevelRight, unsigned char uModeLeft, unsigned char uModeRight);
void GetWearDetectionResult(unsigned char* uWearStatus);

#endif
#ifdef __cplusplus
}
#endif

#endif