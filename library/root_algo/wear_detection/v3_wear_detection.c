/*****************************************************************************************************************
版权所有：Shenzhen Root Inonvation
项目名称：V3
项目编号：
文 件 名：
生成日期：2026.01.07
作    者：sahar.hu
功能说明：V3 wear detection functions
其他说明：this version is only for stimulation-level 4. 
修改记录：
修改日期：
修 改 者：
修改内容：
******************************************************************************************************************/
//#if defined(ALGO_TYPE_V3_WEAR_DETECTION)
#include "v3_wear_detection.h"
#include "v3_leak_detect.h"
#define DEBUG		0

// global data
SDATA_BUFFER		tDataBuffer[2];
SPV_INFO			tPVInfo[2];
SWAVE_INFO			tWaveInfo[2];
SFEATURE_INFO		tFeatureInfo[2];
SDEVICE_INFO		tDeviceInfo[2];
SFILTER_INFO		tFilterInfo[2];


/* get pressure algorithm version */
const char* v3_pressure_algo_get_version(void)
{
    return V3_PRESSURE_ALGO_VERSION_STR;
}


/*************************************************************************************************************************************************
* 函数名称：V3WearDetectionInit
* 作    者：sahar.hu
* 创建日期：2025.12.19
* 功能说明：初始化
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void V3WearDetectionInit(unsigned char uLevelLeft, unsigned char ulevelRight, unsigned char uModeLeft, unsigned char uModeRight)
{
	/* Device info */
	tDeviceInfo[CHANNEL_0].uMode = uModeLeft;
	tDeviceInfo[CHANNEL_0].uLevel = uLevelLeft - 1;
	tDeviceInfo[CHANNEL_1].uMode = uModeRight;
	tDeviceInfo[CHANNEL_1].uLevel = ulevelRight - 1;

	for (int j = 0; j < CHANNEL_NUM; j++)
	{
		/* pv info */
		for (int i = 0; i < 100; i++)
		{
			/* PVInfo initiation */
			tPVInfo[j].uPos[i] = 0;
			tPVInfo[j].fValue[i] = 0;
			tPVInfo[j].uType[i] = 0;
			/* WaveInfo initiation */
			tWaveInfo[j].fLeftAmp[i] = 0;
			tWaveInfo[j].fRightAmp[i] = 0;
			tWaveInfo[j].fValue[i] = 0;
			tWaveInfo[j].uPos[i] = 0;
			tWaveInfo[j].uType[i] = 0;
			tWaveInfo[j].uwidth[i] = 0;
			tWaveInfo[j].fLeftValue[i] = 0;
			tWaveInfo[j].fRightValue[i] = 0;
			tWaveInfo[j].uLeftWidth[i] = 0;
			tWaveInfo[j].uRightWidth[i] = 0;

		}
		/* data buffer */
		for (int i = 0; i < WIN_SIZE; i++)
		{
			tDataBuffer[j].fDataBuff[i] = 0;
			tDataBuffer[j].fLoadedThresholdData[i] = 0;
		}
		tDataBuffer[j].uCurrPos = 0;
		tDataBuffer[j].fDataMax = -1;
		tDataBuffer[j].fDataMin = 0;
		tDataBuffer[j].uFirstTime = 1;

		/* features */
		tFeatureInfo[j].uUpdateFlag = 0;
		tFeatureInfo[j].fMinData = 0;
		tFeatureInfo[j].uNoiseLevel = 0;
		tFeatureInfo[j].uWaveSpikes = 0;
		tFeatureInfo[j].uPVIndex = 0;
		tFeatureInfo[j].uWearStatus = 0;
		tFeatureInfo[j].uNormalWaveform = 0;
		tFeatureInfo[j].pumping_time = 0;
		tFeatureInfo[j].fMinDataBuffMin = 0;
		tFeatureInfo[j].uCompsStatus = 0;
		tFeatureInfo[j].uWearOnCount = 0;
		tFeatureInfo[j].uWearOffCount = 0;
		tFeatureInfo[j].iDelta_pwm = 0;
		tFeatureInfo[j].iPwmOffset = 0;
		tFeatureInfo[j].fCompErrSum = 0;
		tFeatureInfo[j].fCompPrevErr = 0;
		tFeatureInfo[j].uPrevWearStatus = 0;
		tFeatureInfo[j].uPumpState = SUCTION_STATUS_IDLE;
		tFeatureInfo[j].fRawSegMinPressure = 0;
		tFeatureInfo[j].fRawSegThresholdAtMinPressure = 0;
		tFeatureInfo[j].fRawSegRunningMin = 0;
		tFeatureInfo[j].fFilteredSegMinPressure = 0;
		tFeatureInfo[j].fFilteredSegRunningMin = 0;
		tFeatureInfo[j].uPrevPumpState = SUCTION_STATUS_IDLE;
		tFeatureInfo[j].uPrevActive = 0;
		tFeatureInfo[j].uCurrActive = 0;
		tFeatureInfo[j].uRawTailRemain = 0;
		tFeatureInfo[j].uRawSegLen = 0;
		tFeatureInfo[j].uWearDetSegMinCommitted = 0;
		tFeatureInfo[j].uLeakDetectTime = 0;
		tFeatureInfo[j].uWearDetectTime = 0;
		
		LeakDetectReset(&tFeatureInfo[j]);

		for (int i = 0; i < 6; i++)
		{
			tFeatureInfo[j].fMinDataBuff[i] = 0;
			tFeatureInfo[j].fLoadedThresholdBuff[i] = 0;
		}

		switch (tDeviceInfo[j].uMode)
		{
		case MODE_EXPRESSION:
			tFilterInfo[j].fFc = 4.0;
			tDeviceInfo[j].run_time = EXPRESSION_WORK_TIME[tDeviceInfo[j].uLevel];
			tFeatureInfo[j].fThreshold = UNLOADED_EXPRESSION_THRESHOLD[tDeviceInfo[j].uLevel];
			tFeatureInfo[j].fLoadedThreshold = EXPRESSION_THRESHOLD[tDeviceInfo[j].uLevel];
			break;
		case MODE_MIXED:
			tFilterInfo[j].fFc = 3.0;
			tFeatureInfo[j].fThreshold = UNLOADED_MIXED_THRESHOLD[tDeviceInfo[j].uLevel];
			tFeatureInfo[j].fLoadedThreshold = MIXED_THRESHOLD[tDeviceInfo[j].uLevel];
			break;
		case MODE_STIMULATION:
			tFilterInfo[j].fFc = 3.0;
			tDeviceInfo[j].run_time = STIMULATION_WORK_TIME[tDeviceInfo[j].uLevel];
			tFeatureInfo[j].fThreshold = UNLOADED_STIMULATION_THRESHOLD[tDeviceInfo[j].uLevel];
			tFeatureInfo[j].fLoadedThreshold = STIMULATION_THRESHOLD[tDeviceInfo[j].uLevel];
			break;
		}
		/* filter info */
		tFilterInfo[j].fDT = 1.0 / SAMPLE_RATE;
		tFilterInfo[j].fRC = 1.0 / (2.0 * 3.1415926 * tFilterInfo[j].fFc);
		tFilterInfo[j].fAlpha = tFilterInfo[j].fDT / (tFilterInfo[j].fRC + tFilterInfo[j].fDT);
		tFilterInfo[j].fYpre = 0.0;
		tFilterInfo[j].uInitState = 0;
	}
}


/*************************************************************************************************************************************************
* 函数名称：V3WearDetectionModeReset
* 作    者：sahar.hu
* 创建日期：2025.01.19
* 功能说明：初始化
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void V3WearDetectionModeReset(unsigned char uLevel, unsigned char uMode, unsigned int uChannelIdx)
{
	for (int i = 0; i < 100; i++)
	{
		/* PVInfo initiation */
		tPVInfo[uChannelIdx].uPos[i] = 0;
		tPVInfo[uChannelIdx].fValue[i] = 0;
		tPVInfo[uChannelIdx].uType[i] = 0;
		/* WaveInfo initiation */
		tWaveInfo[uChannelIdx].fLeftAmp[i] = 0;
		tWaveInfo[uChannelIdx].fRightAmp[i] = 0;
		tWaveInfo[uChannelIdx].fValue[i] = 0;
		tWaveInfo[uChannelIdx].uPos[i] = 0;
		tWaveInfo[uChannelIdx].uType[i] = 0;
		tWaveInfo[uChannelIdx].uwidth[i] = 0;
		tWaveInfo[uChannelIdx].fLeftValue[i] = 0;
		tWaveInfo[uChannelIdx].fRightValue[i] = 0;
		tWaveInfo[uChannelIdx].uLeftWidth[i] = 0;
		tWaveInfo[uChannelIdx].uRightWidth[i] = 0;

	}
	/* data buffer */
	for (int i = 0; i < WIN_SIZE; i++)
	{
		tDataBuffer[uChannelIdx].fDataBuff[i] = 0;
		tDataBuffer[uChannelIdx].fLoadedThresholdData[i] = 0;
	}
	tDataBuffer[uChannelIdx].uCurrPos = 0;
	tDataBuffer[uChannelIdx].fDataMax = -1;
	tDataBuffer[uChannelIdx].fDataMin = 0;
	tDataBuffer[uChannelIdx].uFirstTime = 1;

	/* features */
	tFeatureInfo[uChannelIdx].uUpdateFlag = 0;
	tFeatureInfo[uChannelIdx].fMinData = 0;
	tFeatureInfo[uChannelIdx].uNoiseLevel = 0;
	tFeatureInfo[uChannelIdx].uWaveSpikes = 0;

	tFeatureInfo[uChannelIdx].uPVIndex = 0;
	tFeatureInfo[uChannelIdx].uWearStatus = 0;
	tFeatureInfo[uChannelIdx].uNormalWaveform = 0;
	tFeatureInfo[uChannelIdx].fMinDataBuffMin = 0;
	tFeatureInfo[uChannelIdx].uCompsStatus = 0;
	tFeatureInfo[uChannelIdx].iDelta_pwm = 0;
	tFeatureInfo[uChannelIdx].iPwmOffset = 0;
	tFeatureInfo[uChannelIdx].fCompErrSum = 0;
	tFeatureInfo[uChannelIdx].fCompPrevErr = 0;
	tFeatureInfo[uChannelIdx].uWearOnCount = 0;
	tFeatureInfo[uChannelIdx].uWearOffCount = 0;
	tFeatureInfo[uChannelIdx].uPumpState = SUCTION_STATUS_IDLE;
	tFeatureInfo[uChannelIdx].fRawSegMinPressure = 0;
	tFeatureInfo[uChannelIdx].fRawSegThresholdAtMinPressure = 0;
	tFeatureInfo[uChannelIdx].fRawSegRunningMin = 0;
	tFeatureInfo[uChannelIdx].fFilteredSegMinPressure = 0;
	tFeatureInfo[uChannelIdx].fFilteredSegRunningMin = 0;
	tFeatureInfo[uChannelIdx].uPrevPumpState = SUCTION_STATUS_IDLE;
	tFeatureInfo[uChannelIdx].uPrevActive = 0;
	tFeatureInfo[uChannelIdx].uCurrActive = 0;
	tFeatureInfo[uChannelIdx].uRawTailRemain = 0;
	tFeatureInfo[uChannelIdx].uRawSegLen = 0;
	tFeatureInfo[uChannelIdx].uWearDetSegMinCommitted = 0;
	tFeatureInfo[uChannelIdx].uLeakDetectTime = 0;
	tFeatureInfo[uChannelIdx].uWearDetectTime = 0;
	LeakDetectReset(&tFeatureInfo[uChannelIdx]);
	for (int i = 0; i < 6; i++)
	{
		tFeatureInfo[uChannelIdx].fMinDataBuff[i] = 0;
		tFeatureInfo[uChannelIdx].fLoadedThresholdBuff[i] = 0;
	}
	/* Device info */
	tDeviceInfo[uChannelIdx].uMode = uMode;
	tDeviceInfo[uChannelIdx].uLevel = uLevel - 1;
	switch (uMode)
	{
	case MODE_EXPRESSION:
		tFilterInfo[uChannelIdx].fFc = 4.0;
		tFeatureInfo[uChannelIdx].fThreshold = UNLOADED_EXPRESSION_THRESHOLD[tDeviceInfo[uChannelIdx].uLevel];
		tFeatureInfo[uChannelIdx].fLoadedThreshold = EXPRESSION_THRESHOLD[tDeviceInfo[uChannelIdx].uLevel];
		break;
	case MODE_MIXED:
		tFilterInfo[uChannelIdx].fFc = 3.0;
		tFeatureInfo[uChannelIdx].fThreshold = UNLOADED_MIXED_THRESHOLD[tDeviceInfo[uChannelIdx].uLevel];
		tFeatureInfo[uChannelIdx].fLoadedThreshold = MIXED_THRESHOLD[tDeviceInfo[uChannelIdx].uLevel];
		break;
	case MODE_STIMULATION:
		tFilterInfo[uChannelIdx].fFc = 3.0;
		tFeatureInfo[uChannelIdx].fThreshold = UNLOADED_STIMULATION_THRESHOLD[tDeviceInfo[uChannelIdx].uLevel];
		tFeatureInfo[uChannelIdx].fLoadedThreshold = STIMULATION_THRESHOLD[tDeviceInfo[uChannelIdx].uLevel];
		break;
	}
	/* filter info */
	tFilterInfo[uChannelIdx].fDT = 1.0 / SAMPLE_RATE;
	tFilterInfo[uChannelIdx].fRC = 1.0 / (2.0 * 3.1415926 * tFilterInfo[uChannelIdx].fFc);
	tFilterInfo[uChannelIdx].fAlpha = tFilterInfo[uChannelIdx].fDT / (tFilterInfo[uChannelIdx].fRC + tFilterInfo[uChannelIdx].fDT);
	tFilterInfo[uChannelIdx].fYpre = 0.0;
	tFilterInfo[uChannelIdx].uInitState = 0;
}

/**
 * @brief 判断泵泵状态是否为活动状态
 * 
 * @param state 泵状态
 * @return unsigned char 1：活动状态，0：非活动状态
 */
static unsigned char IsPumpActiveState(unsigned char state)
{
	if ((state == SUCTION_STATUS_GENERATE) ||
		(state == SUCTION_STATUS_HOLD))
	{
		return 1;
	}

	if ((state == SUCTION_STATUS_RELEASE) ||
		(state == SUCTION_STATUS_SLEEP))
	{
		return 0;
	}
	return 0;
}


/**
 * @brief 查找原始数据中的主要下降范围
 * 
 * @param fData 原始数据指针
 * @param uLen 原始数据长度
 * @param uStartIdx 主要下降范围起始索引指针
 * @param uEndIdx 主要下降范围结束索引指针
 * @return unsigned char 1：找到主要下降范围，0：未找到主要下降范围
 */
static unsigned char FindMainFallingRange(const float* fData,
	unsigned short uLen,
	unsigned short* uStartIdx,
	unsigned short* uEndIdx)
{
	unsigned short i = 0;
	unsigned short uMaxIdx = 0;
	unsigned short uMinIdx = 0;

	if ((fData == 0) || (uStartIdx == 0) || (uEndIdx == 0))
	{
		return 0;
	}

	if (uLen < 6)
	{
		return 0;
	}

	for (i = 1; i < uLen; i++)
	{
		if (fData[i] > fData[uMaxIdx])
		{
			uMaxIdx = i;
		}
	}

	uMinIdx = uMaxIdx;
	for (i = (unsigned short)(uMaxIdx + 1); i < uLen; i++)
	{
		if (fData[i] < fData[uMinIdx])
		{
			uMinIdx = i;
		}
	}

	if (uMinIdx <= (unsigned short)(uMaxIdx + 2))
	{
		return 0;
	}

	*uStartIdx = uMaxIdx;
	*uEndIdx = uMinIdx;

	return 1;
}

/* 段内 raw / 滤波最小：running min，与原先 uRawSegLen 入段规则一致；raw 与 filtered 同拍成对 */
static void RawSegTryAcceptSample(SFEATURE_INFO* t, float raw_pressure, float filtered_pressure, unsigned int uMaxLen)
{
	if (t->uRawSegLen >= uMaxLen)
		return;
	if (t->uRawSegLen == 0)
	{
		t->fRawSegRunningMin = raw_pressure;
		t->fFilteredSegRunningMin = filtered_pressure;
	}
	else
	{
		if (raw_pressure < t->fRawSegRunningMin)
			t->fRawSegRunningMin = raw_pressure;
		if (filtered_pressure < t->fFilteredSegRunningMin)
			t->fFilteredSegRunningMin = filtered_pressure;
	}
	t->uRawSegLen++;
}

static void RawSegCommitMinIfAny(SFEATURE_INFO* t)
{
	if (t->uRawSegLen > 0)
	{
		t->fRawSegMinPressure = t->fRawSegRunningMin;
		t->fFilteredSegMinPressure = t->fFilteredSegRunningMin;
		t->uWearDetSegMinCommitted = 1;
	}
}


/**
 * @brief 更新泵泵原始数据段（与当前拍滤波对齐）
 * 
 * @param raw_pressure 原始压力
 * @param filtered_pressure 与 raw 同拍的滤波值（须先 PressureDataPreprocess 再传入）
 * @param curr_state 当前泵泵状态
 * @param tFeatureInfoTemp 特征信息指针
 */
static void UpdateRawPumpSegment(float raw_pressure,
	float filtered_pressure,
	float fThreshold,
	unsigned char curr_state,
	SFEATURE_INFO* tFeatureInfoTemp)
{
	unsigned char uCurrStateFixed = 0;
	tFeatureInfoTemp->uCurrActive = 0;
	tFeatureInfoTemp->uPrevActive = 0;

	if (tFeatureInfoTemp == 0)
	{
		return;
	}

	uCurrStateFixed = curr_state;

	/* 如果当前状态偶发为 0，且上一拍是活动状态，则保持上一拍状态 */
	if ((uCurrStateFixed == SUCTION_STATUS_IDLE) &&
		(IsPumpActiveState(tFeatureInfoTemp->uPrevPumpState)))
	{
		uCurrStateFixed = tFeatureInfoTemp->uPrevPumpState;
	}

	tFeatureInfoTemp->uCurrActive = IsPumpActiveState(uCurrStateFixed);
	tFeatureInfoTemp->uPrevActive = IsPumpActiveState(tFeatureInfoTemp->uPrevPumpState);

	/* inactive -> active：开启新段，并把当前点作为首点写入 */
	if ((!tFeatureInfoTemp->uPrevActive) && tFeatureInfoTemp->uCurrActive)
	{
		if ((tFeatureInfoTemp->uRawSegLen > 0) && (tFeatureInfoTemp->uRawTailRemain > 0))
		{
			RawSegCommitMinIfAny(tFeatureInfoTemp);
		}
		tFeatureInfoTemp->uRawSegLen = 0;
		tFeatureInfoTemp->uRawTailRemain = (unsigned char)RAW_PUMP_POST_TAIL_POINTS;

		RawSegTryAcceptSample(tFeatureInfoTemp, raw_pressure, filtered_pressure, RAW_PUMP_ACTIVE_CAP);
		tFeatureInfoTemp->fRawSegThresholdAtMinPressure = fThreshold;
	}
	/* active -> active：持续累积（活动期长度上限 RAW_PUMP_ACTIVE_CAP） */
	else if (tFeatureInfoTemp->uPrevActive && tFeatureInfoTemp->uCurrActive)
	{
		RawSegTryAcceptSample(tFeatureInfoTemp, raw_pressure, filtered_pressure, RAW_PUMP_ACTIVE_CAP);
	}
	/* active -> inactive：写入结束边沿点，并开始尾点倒计时 */
	else if (tFeatureInfoTemp->uPrevActive && (!tFeatureInfoTemp->uCurrActive))
	{
		RawSegTryAcceptSample(tFeatureInfoTemp, raw_pressure, filtered_pressure, RAW_PUMP_SEG_MAX);
		if (tFeatureInfoTemp->uRawTailRemain > 0)
		{
			tFeatureInfoTemp->uRawTailRemain--;
		}
		if ((tFeatureInfoTemp->uRawTailRemain == 0) ||
			(tFeatureInfoTemp->uRawSegLen >= RAW_PUMP_SEG_MAX))
		{
			RawSegCommitMinIfAny(tFeatureInfoTemp);
			tFeatureInfoTemp->uRawSegLen = 0;
			tFeatureInfoTemp->uRawTailRemain = 0;
		}
	}
	/* inactive -> inactive：继续写尾随点，每写一点 uRawTailRemain 减 1，到 0 表示本段采集结束 */
	else if ((!tFeatureInfoTemp->uPrevActive) && (!tFeatureInfoTemp->uCurrActive) &&
		(tFeatureInfoTemp->uRawTailRemain > 0))
	{
		RawSegTryAcceptSample(tFeatureInfoTemp, raw_pressure, filtered_pressure, RAW_PUMP_SEG_MAX);
		tFeatureInfoTemp->uRawTailRemain--;
		if ((tFeatureInfoTemp->uRawTailRemain == 0) ||
			(tFeatureInfoTemp->uRawSegLen >= RAW_PUMP_SEG_MAX))
		{
			RawSegCommitMinIfAny(tFeatureInfoTemp);
			tFeatureInfoTemp->uRawSegLen = 0;
			tFeatureInfoTemp->uRawTailRemain = 0;
		}
	}

	tFeatureInfoTemp->uPrevPumpState = uCurrStateFixed;
}


/*************************************************************************************************************************************************
* 函数名称：CircleIndex
* 作    者：sahar.hu
* 创建日期：2025.12.19
* 功能说明：数据当前定位
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
unsigned int BufferCircleNextIndex(unsigned int uPos, unsigned int uIndent, unsigned int uBufferLen)
{
	unsigned int uTempPos = 0;
	if (uPos == uBufferLen - uIndent)
	{
		uTempPos = 0;
	}
	else if (uPos + uIndent >= uBufferLen)
		uTempPos = uPos + uIndent - uBufferLen;
	else
		uTempPos = uPos + uIndent;
	return uTempPos;
}


/*************************************************************************************************************************************************
* 函数名称：BufferCircleRelativeIndex
* 作    者：sahar.hu
* 创建日期：2025.12.19
* 功能说明：数据当前定位
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
unsigned int BufferCircleRelativeIndex(unsigned int uPos, unsigned int uIdx, unsigned int uBufferLen, unsigned int uFirstTime)
{
	unsigned int uTempPos = 0;
	if (uFirstTime == 1)
	{
		unsigned int uStartPos = 0;
		uTempPos = uIdx;
	}
	else
	{
		unsigned int uStartPos = uPos + 1;
		uTempPos = BufferCircleNextIndex(uStartPos, uIdx, uBufferLen);
	}

	return uTempPos;
}


/*************************************************************************************************************************************************
* 函数名称：V3WearDetectionMain
* 作    者：sahar.hu
* 创建日期：2025.12.19
* 功能说明：佩戴检测主函数
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
float FilterPreprocess(float fInputData, SFILTER_INFO* tFilterInfoTemp)
{
	// low pass filter
	float filtered_data = 0.0;
	if (tFilterInfoTemp->uInitState == 0)
	{
		tFilterInfoTemp->fYpre = fInputData;
		tFilterInfoTemp->uInitState = 1;
		return fInputData;
	}
	filtered_data = tFilterInfoTemp->fYpre + tFilterInfoTemp->fAlpha * (fInputData - tFilterInfoTemp->fYpre);
	tFilterInfoTemp->fYpre = filtered_data;
	return filtered_data;
}


/*************************************************************************************************************************************************
* 函数名称：PressureDataPreprocess
* 作    者：sahar.hu
* 创建日期：2025.12.19
* 功能说明：数据缓存处理, 滤波
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void PressureDataPreprocess(float fPumpData, SFILTER_INFO* tFilterInfoTemp, SDATA_BUFFER* tDataBufferTemp, SFEATURE_INFO* tFeatureInfoTemp)
{
	float filtered_data = 0.0;
	if (fPumpData < -100)
		fPumpData = 0;
	filtered_data = FilterPreprocess(fPumpData, tFilterInfoTemp);

	tDataBufferTemp->fDataBuff[tDataBufferTemp->uCurrPos] = filtered_data;
	tDataBufferTemp->fLoadedThresholdData[tDataBufferTemp->uCurrPos] = tFeatureInfoTemp->fLoadedThreshold;
	if (tFeatureInfoTemp->uDataIdx == SAMPLE_RATE - 1)
	{
		float fminDataBuffMin = 0, fminDataBuffMax = -100;
		for (int i = 0; i < 5; i++)
		{
			if (tFeatureInfoTemp->fMinDataBuff[i] < fminDataBuffMin)
				fminDataBuffMin = tFeatureInfoTemp->fMinDataBuff[i];
			if (tFeatureInfoTemp->fMinDataBuff[i] > fminDataBuffMax)
				fminDataBuffMax = tFeatureInfoTemp->fMinDataBuff[i];
			// tFeatureInfoTemp->fMinDataBuff[i] = 0;
			// tFeatureInfoTemp->fLoadedThresholdBuff[i] = 0;
		}
		if (tFeatureInfoTemp->fMinDataBuff[5] < fminDataBuffMin)
			fminDataBuffMin = tFeatureInfoTemp->fMinDataBuff[5];
		if (tFeatureInfoTemp->fMinDataBuff[5] > fminDataBuffMax)
			fminDataBuffMax = tFeatureInfoTemp->fMinDataBuff[5];
		tFeatureInfoTemp->fMinDataBuffMin = fminDataBuffMin;
		if (fminDataBuffMax - fminDataBuffMin > 8)
			tFeatureInfoTemp->fMinDataBuff[5] = fminDataBuffMin;
		tFeatureInfoTemp->fMinDataBuff[5] = fminDataBuffMin;
		tDataBufferTemp->fDataMax = -1;
		tDataBufferTemp->fDataMin = 0;

		tFeatureInfoTemp->fMinData = 0;
		tFeatureInfoTemp->uNoiseLevel = 0;
		tFeatureInfoTemp->uWaveSpikes = 0;
		tFeatureInfoTemp->uDataIdx = 0;
		tFeatureInfoTemp->uUpdateFlag = 1;
		tFeatureInfoTemp->uNormalWaveform = 0;

	}
	else
	{
		tFeatureInfoTemp->uDataIdx++;
		tFeatureInfoTemp->uUpdateFlag = 0;
	}

}


/*************************************************************************************************************************************************
* 函数名称：PeakValleyAnalysis
* 作    者：sahar.hu
* 创建日期：2025.12.19
* 功能说明：波峰、波谷检测
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void PeakValleyAnalysis(SPV_INFO* tPVInfoTemp, SWAVE_INFO* tWaveInfoTemp, SDATA_BUFFER* tDataBufferTemp, SFEATURE_INFO* tFeatureInfoTemp)
{
	if (tFeatureInfoTemp->uUpdateFlag == 1)
	{
		for (int j = 0; j < 100; j++)
		{
			tPVInfoTemp->fValue[j] = 0;
			tPVInfoTemp->uPos[j] = 0;
			tPVInfoTemp->uType[j] = 0;
			tWaveInfoTemp->fLeftAmp[j] = 0;
			tWaveInfoTemp->fRightAmp[j] = 0;
			tWaveInfoTemp->fValue[j] = 0;
			tWaveInfoTemp->uPos[j] = 0;
			tWaveInfoTemp->uType[j] = 0;
			tWaveInfoTemp->uwidth[j] = 0;
			tWaveInfoTemp->fLeftValue[j] = 0;
			tWaveInfoTemp->fRightValue[j] = 0;
			tWaveInfoTemp->uLeftWidth[j] = 0;
			tWaveInfoTemp->uRightWidth[j] = 0;

		}
		tFeatureInfoTemp->uPVIndex = 0;
		if (tDataBufferTemp->fDataMax < tDataBufferTemp->fDataBuff[0])
		{
			tDataBufferTemp->fDataMax = tDataBufferTemp->fDataBuff[0];
		}
		if (tDataBufferTemp->fDataMin > tDataBufferTemp->fDataBuff[0])
		{
			tDataBufferTemp->fDataMin = tDataBufferTemp->fDataBuff[0];
		}
		for (int i = 1; i < WIN_SIZE - 1; i++)
		{
			if (tDataBufferTemp->fDataMax < tDataBufferTemp->fDataBuff[i])
			{
				tDataBufferTemp->fDataMax = tDataBufferTemp->fDataBuff[i];
			}
			if (tDataBufferTemp->fDataMin > tDataBufferTemp->fDataBuff[i])
			{
				tDataBufferTemp->fDataMin = tDataBufferTemp->fDataBuff[i];
			}
			unsigned int uTempCurrPos = BufferCircleRelativeIndex(tDataBufferTemp->uCurrPos, i, WIN_SIZE, tDataBufferTemp->uFirstTime);
			unsigned int uTempNextPos = BufferCircleRelativeIndex(tDataBufferTemp->uCurrPos, i + 1, WIN_SIZE, tDataBufferTemp->uFirstTime);
			unsigned int uTempPrevPos = BufferCircleRelativeIndex(tDataBufferTemp->uCurrPos, i - 1, WIN_SIZE, tDataBufferTemp->uFirstTime);

			/*printf("i: %d, currPos: %d, prev: %d, curr: %d, next: %d\n", i, tDataBuffer.uCurrPos, uTempPrevPos, uTempCurrPos, uTempNextPos);*/
			/*printf("i: %d: %f %d\n", i, tDataBufferTemp->fDataBuff[uTempCurrPos], tDataBufferTemp->uCurrPos);*/

			/* peak */
			if ((tDataBufferTemp->fDataBuff[uTempPrevPos] <= tDataBufferTemp->fDataBuff[uTempCurrPos] && tDataBufferTemp->fDataBuff[uTempCurrPos] > tDataBufferTemp->fDataBuff[uTempNextPos]) ||
				(tDataBufferTemp->fDataBuff[uTempPrevPos] < tDataBufferTemp->fDataBuff[uTempCurrPos] && tDataBufferTemp->fDataBuff[uTempCurrPos] >= tDataBufferTemp->fDataBuff[uTempNextPos]))
			{
				tFeatureInfoTemp->uNoiseLevel++;
				/* combine with the previous PV if it's flat */
				if (tFeatureInfoTemp->uPVIndex >= 1)
				{
					if (tPVInfoTemp->fValue[tFeatureInfoTemp->uPVIndex - 1] == tDataBufferTemp->fDataBuff[uTempCurrPos] && tPVInfoTemp->uType[tFeatureInfoTemp->uPVIndex - 1] == 1)
					{
						tFeatureInfoTemp->uPVIndex -= 1;
					}
				}
				/* update PVInfo */
				tPVInfoTemp->uPos[tFeatureInfoTemp->uPVIndex] = i;
				tPVInfoTemp->fValue[tFeatureInfoTemp->uPVIndex] = tDataBufferTemp->fDataBuff[uTempCurrPos];
				tPVInfoTemp->uType[tFeatureInfoTemp->uPVIndex] = 1;
				tFeatureInfoTemp->uPVIndex++;
			}

			/* valley */
			if ((tDataBufferTemp->fDataBuff[uTempPrevPos] >= tDataBufferTemp->fDataBuff[uTempCurrPos] && tDataBufferTemp->fDataBuff[uTempCurrPos] < tDataBufferTemp->fDataBuff[uTempNextPos]) ||
				(tDataBufferTemp->fDataBuff[uTempPrevPos] > tDataBufferTemp->fDataBuff[uTempCurrPos] && tDataBufferTemp->fDataBuff[uTempCurrPos] <= tDataBufferTemp->fDataBuff[uTempNextPos]))
			{
				/* combine with the previous PV if it's flat */
				if (tFeatureInfoTemp->uPVIndex >= 1)
				{
					if (tPVInfoTemp->fValue[tFeatureInfoTemp->uPVIndex - 1] == tDataBufferTemp->fDataBuff[uTempCurrPos] && tPVInfoTemp->uType[tFeatureInfoTemp->uPVIndex - 1] == 0)
					{
						tFeatureInfoTemp->uPVIndex -= 1;
					}
				}
				/* update PVInfo */
				tPVInfoTemp->uPos[tFeatureInfoTemp->uPVIndex] = i;
				tPVInfoTemp->fValue[tFeatureInfoTemp->uPVIndex] = tDataBufferTemp->fDataBuff[uTempCurrPos];
				tPVInfoTemp->uType[tFeatureInfoTemp->uPVIndex] = 0;
				tFeatureInfoTemp->uPVIndex++;
			}
		}
		/* update the buffer status */
		if (tDataBufferTemp->uFirstTime == 1 && tDataBufferTemp->uCurrPos == 89)
		{
			tDataBufferTemp->uFirstTime = 0;
		}
		/* update the max and min data */
		if ((tDataBufferTemp->fDataMax - tDataBufferTemp->fDataMin) < 5)
			tFeatureInfoTemp->uNoiseLevel += 1;

	}
}

/*************************************************************************************************************************************************
* 函数名称：PressureDataFeature
* 作    者：sahar.hu
* 创建日期：2025.12.19
* 功能说明：波形特征分析
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void PressureDataFeature(SPV_INFO* tPVInfoTemp, SWAVE_INFO* tWaveInfoTemp, SDATA_BUFFER* tDataBufferTemp, SFEATURE_INFO* tFeatureInfoTemp, SDEVICE_INFO* tDeviceInfoTemp)
{
	/* peak and valley */
	PeakValleyAnalysis(tPVInfoTemp, tWaveInfoTemp, tDataBufferTemp, tFeatureInfoTemp);

	/* wave analysis */
	unsigned int uWaveIndex = 0;
	if (tFeatureInfoTemp->uUpdateFlag == 1)
	{
		if (tFeatureInfoTemp->uPVIndex >= 2)
		{
			/* check the peak/valley starting at the first PV */
			unsigned int i = 0;
			if (tPVInfoTemp->uType[i] == 0 && tPVInfoTemp->uType[i + 1] == 1)
			{
				tWaveInfoTemp->uPos[uWaveIndex] = tPVInfoTemp->uPos[i];
				tWaveInfoTemp->fValue[uWaveIndex] = tPVInfoTemp->fValue[i];

				tWaveInfoTemp->uType[uWaveIndex] = 0;
				tWaveInfoTemp->fRightAmp[uWaveIndex] = tPVInfoTemp->fValue[i + 1] - tPVInfoTemp->fValue[i];
				tWaveInfoTemp->fLeftAmp[uWaveIndex] = fabs(tPVInfoTemp->fValue[i]);
				tWaveInfoTemp->fLeftValue[uWaveIndex] = 0;
				tWaveInfoTemp->fRightValue[uWaveIndex] = tPVInfoTemp->fValue[i + 1];
				tWaveInfoTemp->uLeftWidth[uWaveIndex] = tPVInfoTemp->uPos[i];
				tWaveInfoTemp->uRightWidth[uWaveIndex] = tPVInfoTemp->uPos[i + 1] - tPVInfoTemp->uPos[i];

				tWaveInfoTemp->uwidth[uWaveIndex] = tWaveInfoTemp->uRightWidth[uWaveIndex] + tWaveInfoTemp->uLeftWidth[uWaveIndex];		/* it starts at "0" */
				if (tFeatureInfoTemp->fMinData > tWaveInfoTemp->fValue[uWaveIndex])
				{
					tFeatureInfoTemp->fMinData = tWaveInfoTemp->fValue[uWaveIndex];
				}
				uWaveIndex += 1;
			}
			else if (tPVInfoTemp->uType[i] == 1 && tPVInfoTemp->uType[i + 1] == 0)
			{
				tWaveInfoTemp->uPos[uWaveIndex] = tPVInfoTemp->uPos[i];
				tWaveInfoTemp->fValue[uWaveIndex] = tPVInfoTemp->fValue[i];
				tWaveInfoTemp->uType[uWaveIndex] = 1;
				tWaveInfoTemp->fRightAmp[uWaveIndex] = tPVInfoTemp->fValue[i] - tPVInfoTemp->fValue[i + 1];
				tWaveInfoTemp->fLeftAmp[uWaveIndex] = fabs(tPVInfoTemp->fValue[i]);

				tWaveInfoTemp->fLeftValue[uWaveIndex] = 0;
				tWaveInfoTemp->fRightValue[uWaveIndex] = tPVInfoTemp->fValue[i + 1];
				tWaveInfoTemp->uLeftWidth[uWaveIndex] = tPVInfoTemp->uPos[i];
				tWaveInfoTemp->uRightWidth[uWaveIndex] = tPVInfoTemp->uPos[i + 1] - tPVInfoTemp->uPos[i];
				tWaveInfoTemp->uwidth[uWaveIndex] = tWaveInfoTemp->uRightWidth[uWaveIndex] + tWaveInfoTemp->uLeftWidth[uWaveIndex];		/* it starts at "0" */
				uWaveIndex += 1;
			}
			if (tFeatureInfoTemp->uPVIndex >= 3)
			{
				for (i = 1; i < tFeatureInfoTemp->uPVIndex - 1; i++)
				{
					/* check a valley wave */
					if (tPVInfoTemp->uType[i - 1] == 1 && tPVInfoTemp->uType[i] == 0 && tPVInfoTemp->uType[i + 1] == 1)
					{
						tWaveInfoTemp->uPos[uWaveIndex] = tPVInfoTemp->uPos[i];
						tWaveInfoTemp->fValue[uWaveIndex] = tPVInfoTemp->fValue[i];
						tWaveInfoTemp->uwidth[uWaveIndex] = tPVInfoTemp->uPos[i + 1] - tPVInfoTemp->uPos[i - 1];
						tWaveInfoTemp->uType[uWaveIndex] = 0;
						tWaveInfoTemp->fRightAmp[uWaveIndex] = tPVInfoTemp->fValue[i + 1] - tPVInfoTemp->fValue[i];
						tWaveInfoTemp->fLeftAmp[uWaveIndex] = tPVInfoTemp->fValue[i - 1] - tPVInfoTemp->fValue[i];

						tWaveInfoTemp->fLeftValue[uWaveIndex] = tPVInfoTemp->fValue[i - 1];
						tWaveInfoTemp->fRightValue[uWaveIndex] = tPVInfoTemp->fValue[i + 1];
						tWaveInfoTemp->uLeftWidth[uWaveIndex] = tPVInfoTemp->uPos[i] - tPVInfoTemp->uPos[i - 1];
						tWaveInfoTemp->uRightWidth[uWaveIndex] = tPVInfoTemp->uPos[i + 1] - tPVInfoTemp->uPos[i];
						tWaveInfoTemp->uwidth[uWaveIndex] = tWaveInfoTemp->uRightWidth[uWaveIndex] + tWaveInfoTemp->uLeftWidth[uWaveIndex];		/* it starts at "0" */
						if (tFeatureInfoTemp->fMinData > tWaveInfoTemp->fValue[uWaveIndex])
						{
							tFeatureInfoTemp->fMinData = tWaveInfoTemp->fValue[uWaveIndex];
						}
						uWaveIndex += 1;
						if (uWaveIndex >= 100 || uWaveIndex < 0)
							uWaveIndex = 0;
					}
					/* check a peak wave */
					if (tPVInfoTemp->uType[i - 1] == 0 && tPVInfoTemp->uType[i] == 1 && tPVInfoTemp->uType[i + 1] == 0)
					{
						tWaveInfoTemp->uPos[uWaveIndex] = tPVInfoTemp->uPos[i];
						tWaveInfoTemp->fValue[uWaveIndex] = tPVInfoTemp->fValue[i];
						tWaveInfoTemp->uwidth[uWaveIndex] = tPVInfoTemp->uPos[i + 1] - tPVInfoTemp->uPos[i - 1];
						tWaveInfoTemp->uType[uWaveIndex] = 1;
						tWaveInfoTemp->fRightAmp[uWaveIndex] = tPVInfoTemp->fValue[i] - tPVInfoTemp->fValue[i + 1];
						tWaveInfoTemp->fLeftAmp[uWaveIndex] = tPVInfoTemp->fValue[i] - tPVInfoTemp->fValue[i - 1];

						tWaveInfoTemp->fLeftValue[uWaveIndex] = tPVInfoTemp->fValue[i - 1];
						tWaveInfoTemp->fRightValue[uWaveIndex] = tPVInfoTemp->fValue[i + 1];
						tWaveInfoTemp->uLeftWidth[uWaveIndex] = tPVInfoTemp->uPos[i] - tPVInfoTemp->uPos[i - 1];
						tWaveInfoTemp->uRightWidth[uWaveIndex] = tPVInfoTemp->uPos[i + 1] - tPVInfoTemp->uPos[i];
						tWaveInfoTemp->uwidth[uWaveIndex] = tWaveInfoTemp->uRightWidth[uWaveIndex] + tWaveInfoTemp->uLeftWidth[uWaveIndex];		/* it starts at "0" */

						/* optimize */
						if ((2 <= tWaveInfoTemp->uwidth[uWaveIndex] && tWaveInfoTemp->uwidth[uWaveIndex] <= 10 && tWaveInfoTemp->fValue[uWaveIndex] < -.1)
							&& ((.8 < tWaveInfoTemp->fLeftAmp[uWaveIndex] && tWaveInfoTemp->fLeftAmp[uWaveIndex] < 5) ||
								(.5 < tWaveInfoTemp->fRightAmp[uWaveIndex] && tWaveInfoTemp->fRightAmp[uWaveIndex] < 5) && tDeviceInfoTemp->uLevel <= 6))
						{
							tFeatureInfoTemp->uWaveSpikes++;
						}
						else if ((tWaveInfoTemp->fValue[uWaveIndex] < -.1)
							&& ((.8 < tWaveInfoTemp->fLeftAmp[uWaveIndex] && tWaveInfoTemp->fLeftAmp[uWaveIndex] < 5) ||
								(.8 < tWaveInfoTemp->fRightAmp[uWaveIndex] && tWaveInfoTemp->fRightAmp[uWaveIndex] < 5) && tDeviceInfoTemp->uLevel >= 7))
						{
							tFeatureInfoTemp->uWaveSpikes++;
						}
						uWaveIndex += 1;
					}
				}
			}
		}
		/*printf("\n%f %f %f %f %f \n", (float)tFeatureInfoTemp->fMinDataBuff[0],
			tFeatureInfoTemp->fMinDataBuff[1],
			tFeatureInfoTemp->fMinDataBuff[2],
			tFeatureInfoTemp->fMinDataBuff[3], tFeatureInfoTemp->fMinData);*/
		unsigned char uHasBestValley = 0;
		float fBestValleyValue = 0;
		float fBestValleyThreshold = 0;
		/* check wave spikes */
		for (unsigned int i = 0; i < uWaveIndex; i++)
		{
			if (tWaveInfoTemp->uType[i] == 0 &&
				tWaveInfoTemp->fValue[i] < -1 &&
				tWaveInfoTemp->uwidth[i] > 10 &&
				tWaveInfoTemp->fLeftAmp[i] > 1 &&
				tWaveInfoTemp->fRightAmp[i] > 1)
			{
				if (uHasBestValley == 0 || tWaveInfoTemp->fValue[i] < fBestValleyValue)
				{
					uHasBestValley = 1;
					fBestValleyValue = tWaveInfoTemp->fValue[i];
					fBestValleyThreshold = tDataBufferTemp->fLoadedThresholdData[tWaveInfoTemp->uPos[i]];
				}
			}

			if (tWaveInfoTemp->uType[i] == 0 &&

				(tWaveInfoTemp->fValue[i] < tFeatureInfoTemp->fThreshold &&
					(tDeviceInfoTemp->uMode == MODE_STIMULATION || (tDeviceInfoTemp->uMode == MODE_EXPRESSION && tDeviceInfoTemp->uLevel <= 12)))

				|| (tWaveInfoTemp->fValue[i] < -1 && fabs(tFeatureInfoTemp->fThreshold - tWaveInfoTemp->fValue[i]) < 10 &&
					(tDeviceInfoTemp->uMode == MODE_EXPRESSION && 6 <= tDeviceInfoTemp->uLevel && tDeviceInfoTemp->uLevel <= 10))

				|| (tWaveInfoTemp->fValue[i] < -1 && fabs(tFeatureInfoTemp->fThreshold - tWaveInfoTemp->fValue[i]) >= 2 &&
					(tDeviceInfoTemp->uMode == MODE_EXPRESSION && 11 <= tDeviceInfoTemp->uLevel))

				|| (tWaveInfoTemp->fValue[i] < -10))
			{
				tFeatureInfoTemp->uNormalWaveform++;
			}
		}
			if (uHasBestValley == 1)
			{
				float fBuffMinTemp = 0;

				for (unsigned int k = 0; k < 4; k++)
				{
					tFeatureInfoTemp->fMinDataBuff[k] = tFeatureInfoTemp->fMinDataBuff[k + 1];
					tFeatureInfoTemp->fLoadedThresholdBuff[k] = tFeatureInfoTemp->fLoadedThresholdBuff[k + 1];

					if (tFeatureInfoTemp->fMinDataBuff[k] > fBuffMinTemp)
						fBuffMinTemp = tFeatureInfoTemp->fMinDataBuff[k];

					tFeatureInfoTemp->fMinDataBuffMin = fBuffMinTemp;
				}

				tFeatureInfoTemp->fMinDataBuff[4] = fBestValleyValue;
				tFeatureInfoTemp->fLoadedThresholdBuff[4] = fBestValleyThreshold;

				if (tFeatureInfoTemp->fMinDataBuff[4] > fBuffMinTemp)
					fBuffMinTemp = tFeatureInfoTemp->fMinDataBuff[4];

				tFeatureInfoTemp->fMinDataBuffMin = fBuffMinTemp;
			}
	}
	/* update current index */
	tDataBufferTemp->uCurrPos = BufferCircleNextIndex(tDataBufferTemp->uCurrPos, 1, WIN_SIZE);
}


#if LONGYI_OR_SHIGUANG == 2
void WearDetectionDetermination(SFEATURE_INFO* tFeatureInfoTemp, SDEVICE_INFO* tDeviceInfoTemp)
{
	//tFeatureInfoTemp->iDelta_pwm = 0;

	/* 本轮先计算一个“候选穿戴状态”，
	 * 不直接修改 uWearStatus，最后再结合抗抖计数决定是否切换
	 */
	unsigned char uCandidateWearStatus = tFeatureInfoTemp->uWearStatus;
	unsigned int uMode = tDeviceInfoTemp->uMode;
	unsigned int uLevel = tDeviceInfoTemp->uLevel;

	// 漏气状态
	if (tFeatureInfoTemp->uWearDetSegMinCommitted)
	{
		/* 平台期 负压回不到0基线 ( < -1kPa ) 则认为是大漏 */
		if (tFeatureInfoTemp->uDataBaselineCnt[0] > 10)
		{
			tFeatureInfoTemp->uLeakStatus = 2;
			tFeatureInfoTemp->uDataBaselineCnt[1] = 0;
		}
		else if (tFeatureInfoTemp->uDataBaselineCnt[1] > 10)
		{
			tFeatureInfoTemp->uLeakStatus = 0;
			tFeatureInfoTemp->uDataBaselineCnt[0] = 0;
		}
		LeakDetectUpdate(tFeatureInfoTemp, tDeviceInfoTemp);
	}

	/* 初始 -> 佩戴检测(包括漏气) 时间 */
	if (tFeatureInfoTemp->uLeakStatus == 0)
	{
		tFeatureInfoTemp->uWearDetectTime ++;
		tFeatureInfoTemp->uLeakDetectTime = 0;
	}

	/* 佩戴(包括漏气) -> 非密封检测时间 */
	if (tFeatureInfoTemp->uLeakStatus != 0)
	{
		tFeatureInfoTemp->uLeakDetectTime ++;
		tFeatureInfoTemp->uWearDetectTime = 0;
	}

	/* =========================================================
	 * 1. 当前处于“未佩戴”状态时，判断是否进入“已佩戴”状态
	 * ========================================================= */
	if (tFeatureInfoTemp->uWearStatus == 0)
	{
		/* -------------------------
		 * 1.1 吸乳模式
		 * ------------------------- */
		if (uMode == MODE_EXPRESSION)
		{
			if (tFeatureInfo->uDataBaselineCnt[1] > 10 && tFeatureInfoTemp->fRawSegMinPressure < -10)
			{
				tFeatureInfo->uLeakStatus = 0;
				uCandidateWearStatus = 1;
			}

			/* L1 ~ L15 的基础进入条件 */
			if (uLevel >= 5 && (tFeatureInfoTemp->fThreshold - 0.25) < tFeatureInfoTemp->fRawSegMinPressure && tFeatureInfoTemp->uWaveSpikes <= 2)
			{
				uCandidateWearStatus = 1;
			}
			else if ((uLevel < 3) && ((tFeatureInfoTemp->fThreshold - 0.25) > tFeatureInfoTemp->fRawSegMinPressure))
			{
				uCandidateWearStatus = 1;
			} 
			else if (((uLevel >= 3 && uLevel <= 4) || uLevel == 5) && tFeatureInfoTemp->uNoiseLevel < 10)
			{
				uCandidateWearStatus = 1;
			}
			else
			{
				if (tFeatureInfoTemp->uWaveSpikes <= 1 &&
					tFeatureInfoTemp->uNoiseLevel == 0 &&
					tFeatureInfoTemp->uNormalWaveform >= 1 &&
					tFeatureInfoTemp->uNormalWaveform <= 10)
				{
					uCandidateWearStatus = 1;
				}
				else if (tFeatureInfoTemp->uWaveSpikes <= 2 && uLevel >= 8)
				{
					/* 漏气情况，高档位可能存在高频噪声 */
					uCandidateWearStatus = 1;
				}
			}
		}

		/* -------------------------
		 * 1.2 刺激模式
		 * ------------------------- */
		else if (uMode == MODE_STIMULATION)
		{

			if (uLevel <= 3 && (tFeatureInfoTemp->fThreshold + 0.25) > tFeatureInfoTemp->fFilteredSegMinPressure)
			{
				uCandidateWearStatus = 1;
			}
			else if (uLevel > 3 && (tFeatureInfoTemp->fThreshold + 0.25) > (tFeatureInfoTemp->fFilteredSegMinPressure + ((float)tFeatureInfoTemp->uWaveSpikes - 1.5) * 3))
			{
				uCandidateWearStatus = 1;
			}
			
			else if (tFeatureInfoTemp->uWaveSpikes == 0 && tFeatureInfoTemp->fRawSegMinPressure < -3.0)
			{
				uCandidateWearStatus = 1;
			}
		}

	}

	/* =========================================================
	 * 2. 当前处于“已佩戴”状态时，判断是否退出为“未佩戴”状态
	 * ========================================================= */
	else if (tFeatureInfoTemp->uWearStatus == 1)
	{
		/* 条件1：
		 * 当前最小负压已经不够深，直接判定退出佩戴
		 */
		if (tFeatureInfoTemp->fMinData > -1)
		{
			uCandidateWearStatus = 0;
		}

		/* 只有在前面没有已经直接判成 0 的情况下，再继续按模式和档位判定 */
		if (uCandidateWearStatus == 1)
		{
			/* -------------------------
			 * 2.1 吸乳模式
			 * ------------------------- */
			if (uMode == MODE_EXPRESSION)
			{
				if (uLevel >= 7 && (tFeatureInfoTemp->fThreshold - 0.25) < tFeatureInfoTemp->fRawSegMinPressure && tFeatureInfoTemp->uWaveSpikes > 4)
				{
					uCandidateWearStatus = 0;		//? 
				}
				else if (uLevel <= 1)
				{
					if (tFeatureInfoTemp->uNormalWaveform < 4 &&
						(tFeatureInfoTemp->fThreshold + 0.25) < tFeatureInfoTemp->fRawSegMinPressure)
					{
						uCandidateWearStatus = 0;
					}
				}
				else
				{
					if (tFeatureInfoTemp->uNormalWaveform > 10)
					{
						uCandidateWearStatus = 0;
					}
					else if (tFeatureInfoTemp->uWaveSpikes > 4)
					{
						uCandidateWearStatus = 0;
					}
					else if (tFeatureInfoTemp->fMinData > -.1
						|| (tFeatureInfoTemp->fThreshold > tFeatureInfoTemp->fMinData && uLevel >= 6))
					{
						uCandidateWearStatus = tFeatureInfoTemp->uPrevWearStatus;
					}
				}
			}

			/* -------------------------
			 * 2.2 刺激模式
			 * ------------------------- */
			else if (uMode == MODE_STIMULATION)
			{
				if (uLevel <= 3 && (tFeatureInfoTemp->fThreshold + 0.25) < tFeatureInfoTemp->fFilteredSegMinPressure && tFeatureInfoTemp->uWaveSpikes > 4)
				{
					uCandidateWearStatus = 0;
				}
				else if (uLevel > 3 && (tFeatureInfoTemp->fThreshold + 0.25) < (tFeatureInfoTemp->fFilteredSegMinPressure + ((float)tFeatureInfoTemp->uWaveSpikes - 1.5) * 3) && (float)tFeatureInfoTemp->uWaveSpikes > 1)
				{
					uCandidateWearStatus = 0;
				}
			}
		}
	}
	/* =========================================================
	 * 3. 根据候选状态和当前状态的差异，做抗抖切换
	 * ========================================================= */
	if (uCandidateWearStatus != tFeatureInfoTemp->uWearStatus)
	{
		/* 目标是从 0 -> 1 */
		if (uCandidateWearStatus == 1)
		{
			/* 进入佩戴计数加一，退出计数清零 */
			tFeatureInfoTemp->uWearOnCount++;
			tFeatureInfoTemp->uWearOffCount = 0;

			/* 连续 n 次满足进入条件，才真正切换为佩戴 */
			if (tFeatureInfoTemp->uWearOnCount >= 6)
			{
				tFeatureInfoTemp->uWearStatus = 1;
				tFeatureInfoTemp->uWearOnCount = 0;
			}
		}
		else
		{
			/* 目标是从 1 -> 0
			 * 退出佩戴计数加一，进入计数清零
			 */
			tFeatureInfoTemp->uWearOffCount++;
			tFeatureInfoTemp->uWearOnCount = 0;

			/* 连续 n 次满足退出条件，才真正切换为未佩戴 */
			if (tFeatureInfoTemp->uWearOffCount >= 3)
			{
				tFeatureInfoTemp->uWearStatus = 0;
				tFeatureInfoTemp->uWearOffCount = 0;
			}
		}
	}
	else
	{
		/* 候选状态和当前状态一致，说明本轮没有切换趋势，清空抗抖计数 */
		tFeatureInfoTemp->uWearOnCount = 0;
		tFeatureInfoTemp->uWearOffCount = 0;
	}

	/* 保存最终状态，供下一轮“保持上一状态”逻辑使用 */
	tFeatureInfoTemp->uPrevWearStatus = tFeatureInfoTemp->uWearStatus;

	/* =========================================================
	 * 4. 只有在“已佩戴”时，才进行补偿状态计算
	 * ========================================================= */
	if (tFeatureInfoTemp->uWearStatus == 1)
	{
		unsigned int uCount_Min = 0;
		unsigned int uCount_Max = 0;

		/* 遍历最近缓存的 6 个最小负压点，判断当前是否偏离 loaded 阈值 */
		for (unsigned int i = 0; i < 5; i++)
		{

			if ((tFeatureInfoTemp->fMinDataBuff[i]) > tFeatureInfoTemp->fLoadedThresholdBuff[i] + 0.5f &&
				(tFeatureInfoTemp->fMinDataBuff[i]) < -3.4)
			{
				/* 负压偏小，记录一次“需要增强补偿”的计数 */
				uCount_Min += 1;
			}
			else if ((tFeatureInfoTemp->fMinDataBuff[i]) < tFeatureInfoTemp->fLoadedThresholdBuff[i] - 0.5f &&
				(tFeatureInfoTemp->fMinDataBuff[i]) < -3.4)
			{
				/* 负压偏大，记录一次“需要减弱补偿”的计数 */
				uCount_Max += 1;
			}

			

		}

		float error = 0.0f;
		float err_sum_candidate = 0.0f;
		int newPwmOffset = 0;

		float kp = 18.0f;
		float ki = 3.0f;
		float deadband = 0.1f;
		float i_limit = 30.0f;
		int step_limit = 100;

		/* 先用最近3次 valley 计算一个统一误差，注意方向要和旧逻辑一致：
		* 偏浅 -> error > 0 -> 输出正补偿
		* 偏深 -> error < 0 -> 输出负补偿
		*/
		float err0 = tFeatureInfoTemp->fMinDataBuff[0] - tFeatureInfoTemp->fLoadedThresholdBuff[0];
		float err1 = tFeatureInfoTemp->fMinDataBuff[1] - tFeatureInfoTemp->fLoadedThresholdBuff[1];
		float err2 = tFeatureInfoTemp->fMinDataBuff[2] - tFeatureInfoTemp->fLoadedThresholdBuff[2];
		float err3 = tFeatureInfoTemp->fMinDataBuff[3] - tFeatureInfoTemp->fLoadedThresholdBuff[3];
		float err4 = tFeatureInfoTemp->fMinDataBuff[4] - tFeatureInfoTemp->fLoadedThresholdBuff[4];
		error = 0.2f * err2 + 0.3f * err3 + 0.5f * err4;

		/* 至少有 4 个点提示负压偏小，进入补偿状态 1 */
		if (uCount_Min >= 4)
		{
			tFeatureInfoTemp->uCompsStatus = 1;

			if (error < 0.0f)
			{
				tFeatureInfoTemp->uCompsStatus = 0;
				error = 0.0f;
			}

			/* 如果偏差过大，则不输出 PWM 补偿 */
			if (fabs(error) > 10.0f)
			{
				tFeatureInfoTemp->iDelta_pwm = 0;
				tFeatureInfoTemp->fCompErrSum = 0.0f;
				tFeatureInfoTemp->fCompPrevErr = 0.0f;
			}
			else
			{
				if (fabs(error) < deadband)
				{
					error = 0.0f;
				}

				/* PI 控制：偏浅时 error 应为正，输出正补偿 */
				err_sum_candidate = tFeatureInfoTemp->fCompErrSum + error;
				err_sum_candidate = CLAMP(err_sum_candidate, -i_limit, i_limit);

				tFeatureInfoTemp->iDelta_pwm =
					(int)(kp * error + ki * err_sum_candidate);
				tFeatureInfoTemp->iDelta_pwm = CLAMP(tFeatureInfoTemp->iDelta_pwm, 0, step_limit);

				newPwmOffset = tFeatureInfoTemp->iPwmOffset + tFeatureInfoTemp->iDelta_pwm;

				/* 输出正向饱和时冻结积分 */
				if (!(newPwmOffset >= 3000 && error > 0.0f))
				{
					tFeatureInfoTemp->fCompErrSum = err_sum_candidate;
				}

				tFeatureInfoTemp->iPwmOffset = CLAMP(newPwmOffset, -3000, 3000);
				tFeatureInfoTemp->fCompPrevErr = error;
			}
		}
		/* 至少有 4 个点提示负压偏大，进入补偿状态 2 */
		else if (uCount_Max >= 4)
		{
			tFeatureInfoTemp->uCompsStatus = 2;

			if (error > 0.0f)
			{
				error = 0.0f;
				tFeatureInfoTemp->uCompsStatus = 0;
			}

			/* 如果偏差过大，则不输出 PWM 补偿 */
			if (fabs(error) > 10.0f)
			{
				tFeatureInfoTemp->iDelta_pwm = 0;
				tFeatureInfoTemp->fCompErrSum = 0.0f;
				tFeatureInfoTemp->fCompPrevErr = 0.0f;
			}
			else
			{
				if (fabs(error) < deadband)
				{
					error = 0.0f;
				}

				/* PI 控制：偏深时 error 应为负，输出负补偿 */
				err_sum_candidate = tFeatureInfoTemp->fCompErrSum + error;
				err_sum_candidate = CLAMP(err_sum_candidate, -i_limit, i_limit);

				tFeatureInfoTemp->iDelta_pwm =
					(int)(kp * error + ki * err_sum_candidate);
				tFeatureInfoTemp->iDelta_pwm = CLAMP(tFeatureInfoTemp->iDelta_pwm, -step_limit, 0);

				newPwmOffset = tFeatureInfoTemp->iPwmOffset + tFeatureInfoTemp->iDelta_pwm;

				/* 输出负向饱和时冻结积分 */
				if (!(newPwmOffset <= -3000 && error < 0.0f))
				{
					tFeatureInfoTemp->fCompErrSum = err_sum_candidate;
				}

				tFeatureInfoTemp->iPwmOffset = CLAMP(newPwmOffset, -3000, 3000);
				tFeatureInfoTemp->fCompPrevErr = error;
			}
		}
		else
		{
			/* 当前不需要补偿，后续可继续补充更完整的泵侧补偿逻辑 */
			tFeatureInfoTemp->uCompsStatus = 0;
			tFeatureInfoTemp->iDelta_pwm = 0;

			/* 没触发补偿时，缓慢释放积分，避免积分残留过大 */
			tFeatureInfoTemp->fCompErrSum *= 0.5f;
			tFeatureInfoTemp->fCompPrevErr = 0.0f;
		}
	}
}

#elif LONGYI_OR_SHIGUANG == 1
void WearDetectionDetermination(SFEATURE_INFO* tFeatureInfoTemp, SDEVICE_INFO* tDeviceInfoTemp)
{
	//tFeatureInfoTemp->iDelta_pwm = 0;

	/* 本轮先计算一个“候选穿戴状态”，
	 * 不直接修改 uWearStatus，最后再结合抗抖计数决定是否切换
	 */
	unsigned char uCandidateWearStatus = tFeatureInfoTemp->uWearStatus;
	unsigned int uMode = tDeviceInfoTemp->uMode;
	unsigned int uLevel = tDeviceInfoTemp->uLevel;

	/* =========================================================
	 * 1. 当前处于“未佩戴”状态时，判断是否进入“已佩戴”状态
	 * ========================================================= */
	if (tFeatureInfoTemp->uWearStatus == 0)
	{
		/* -------------------------
		 * 1.1 吸乳模式
		 * ------------------------- */
		if (uMode == MODE_EXPRESSION)
		{
			/* L1 ~ L15 的基础进入条件 */
			if(uLevel >= 10)
			{
				if(uLevel == 14) // L15 可能需要特殊处理，因为 L15 的阈值漏气后负压变化不明显，通过WaveSpikes作为权重加入来判断是否佩戴
				{
					if((tFeatureInfoTemp->fThreshold - 0.25) < (tFeatureInfoTemp->fMinData - ((float)tFeatureInfoTemp->uWaveSpikes - 1.5) * 3)
						&& (tFeatureInfoTemp->fMinData < -33.25))
					{
						uCandidateWearStatus = 1;
					}
				}
				else 
				{
					if((((tFeatureInfoTemp->fThreshold - 0.25) > tFeatureInfoTemp->fRawSegMinPressure)) && (tFeatureInfoTemp->uWaveSpikes <= 1))
					{
						uCandidateWearStatus = 1;
					}
				}
			}
			else if(uLevel == 0)
			{
				if((tFeatureInfoTemp->fThreshold - 0.25) > tFeatureInfoTemp->fRawSegMinPressure)
				{
					uCandidateWearStatus = 1;
				}
			}
			else
			{
				if (tFeatureInfoTemp->uWaveSpikes <= 1 &&
					tFeatureInfoTemp->uNoiseLevel == 0 &&
					tFeatureInfoTemp->uNormalWaveform >= 1 &&
					tFeatureInfoTemp->uNormalWaveform <= 10)
				{
					uCandidateWearStatus = 1;
				}
			}
		}

		/* -------------------------
		 * 1.2 刺激模式
		 * ------------------------- */
		else if (uMode == MODE_STIMULATION)
		{
			if((tFeatureInfoTemp->fThreshold + 0.25) > (tFeatureInfoTemp->fFilteredSegMinPressure + ((float)tFeatureInfoTemp->uWaveSpikes - 1.5) * 3))
			{
				uCandidateWearStatus = 1;
			}
		}

		/* -------------------------
		 * 1.3 mixed 模式
		 * ------------------------- */
		else if (uMode == MODE_MIXED)
		{
			if(uLevel >= 10)
			{
				if((tFeatureInfoTemp->fRawSegThresholdAtMinPressure + 0.25) > tFeatureInfoTemp->fRawSegMinPressure && (tFeatureInfoTemp->uWaveSpikes <= 1))
				{
					uCandidateWearStatus = 1;
				}
			}
			else if(uLevel <= 6)
			{
				if((tFeatureInfoTemp->fRawSegThresholdAtMinPressure + 0.25) > tFeatureInfoTemp->fRawSegMinPressure)
				{
					uCandidateWearStatus = 1;
				}
			}
			else
			{
				/* mixed 模式基础进入条件 */
				if (uLevel <= 14)
				{
					if ((tFeatureInfoTemp->uWaveSpikes <= 1 &&
						tFeatureInfoTemp->uNoiseLevel == 0 &&
						tFeatureInfoTemp->uNormalWaveform >= 1) &&
						(tFeatureInfoTemp->uNormalWaveform <= 10))
					{
						uCandidateWearStatus = 1;
					}
				}

				/* mixed 高档位下，利用缓存历史最小值辅助进入 */
				if (uCandidateWearStatus == 0)
				{
					if ((tFeatureInfoTemp->fThreshold < tFeatureInfoTemp->fMinDataBuff[5] &&
						tFeatureInfoTemp->fMinDataBuff[5] < -1) &&
						(uLevel >= 9) &&
						tFeatureInfoTemp->uWaveSpikes < 5)
					{
						uCandidateWearStatus = 1;
					}
				}
			}
		}
	}

	/* =========================================================
	 * 2. 当前处于“已佩戴”状态时，判断是否退出为“未佩戴”状态
	 * ========================================================= */
	else if (tFeatureInfoTemp->uWearStatus == 1)
	{
		/* 条件1：
		 * 当前最小负压已经不够深，直接判定退出佩戴
		 */
		if (tFeatureInfoTemp->fMinData > -1)
		{
			uCandidateWearStatus = 0;
		}

		/* 只有在前面没有已经直接判成 0 的情况下，再继续按模式和档位判定 */
		if (uCandidateWearStatus == 1)
		{
			/* -------------------------
			 * 2.1 吸乳模式
			 * ------------------------- */
			if (uMode == MODE_EXPRESSION)
			{
				if(uLevel >= 10)
				{
					if(uLevel == 14) // L15 可能需要特殊处理，因为 L15 的阈值漏气后负压变化不明显，通过WaveSpikes作为权重加入来判断是否佩戴
					{

						if((tFeatureInfoTemp->fThreshold + 0.25) > (tFeatureInfoTemp->fMinData + ((float)tFeatureInfoTemp->uWaveSpikes - 1.5) * 3)
							|| (tFeatureInfoTemp->fMinData > -32.75))
						{
							uCandidateWearStatus = 0;
						}
					}
					else
					{
						if (((tFeatureInfoTemp->fThreshold + 0.25) < tFeatureInfoTemp->fRawSegMinPressure) || (tFeatureInfoTemp->uWaveSpikes >= 2))
						{
							uCandidateWearStatus = 0;
						}
					}
				}
				else if(uLevel == 0 || uLevel == 1)
				{
					if(tFeatureInfoTemp->uNormalWaveform < 4 &&
					(tFeatureInfoTemp->fThreshold + 0.25) < tFeatureInfoTemp->fRawSegMinPressure)
					{
						uCandidateWearStatus = 0;
					}
				}
				/* L13 ~ L15 */
				else
				{
					if (tFeatureInfoTemp->uNormalWaveform > 10)
					{
						uCandidateWearStatus = 0;
					}
					else if (tFeatureInfoTemp->uWaveSpikes > 2)
					{
						uCandidateWearStatus = 0;
					}
					else if (tFeatureInfoTemp->fMinData > -.1
						|| (tFeatureInfoTemp->fThreshold > tFeatureInfoTemp->fMinData && uLevel >= 6))
					{
						uCandidateWearStatus = tFeatureInfoTemp->uPrevWearStatus;
					}
				}
			}

			/* -------------------------
			 * 2.2 刺激模式
			 * ------------------------- */
			else if (uMode == MODE_STIMULATION)
			{
				if((tFeatureInfoTemp->fThreshold + 0.25) < (tFeatureInfoTemp->fFilteredSegMinPressure + ((float)tFeatureInfoTemp->uWaveSpikes - 1.5) * 3))
				{
					uCandidateWearStatus = 0;
				}
			}

			/* -------------------------
			 * 2.3 mixed 模式
			 * ------------------------- */
			else if (uMode == MODE_MIXED)
			{
				if(uLevel >= 10)
				{
					if((tFeatureInfoTemp->fRawSegThresholdAtMinPressure + 0.25) < tFeatureInfoTemp->fRawSegMinPressure || (tFeatureInfoTemp->uWaveSpikes >= 2))
					{
						uCandidateWearStatus = 0;
					}
				}
				else if(uLevel <= 6)
				{
					if((tFeatureInfoTemp->fRawSegThresholdAtMinPressure + 0.25) < tFeatureInfoTemp->fRawSegMinPressure)
					{
						uCandidateWearStatus = 0;
					}
				}
				else
				{				
					/* L4 ~ L6 */
					if (uLevel > 3 && uLevel <= 6)
					{
						if ((tFeatureInfoTemp->fThreshold < tFeatureInfoTemp->fMinDataBuffMin) &&
							tFeatureInfoTemp->uWaveSpikes >= 5)
						{
							uCandidateWearStatus = 0;
						}
					}
					/* L7 及以上 */
					 else if (uLevel >= 7)
					{
						if (tFeatureInfoTemp->uWaveSpikes > 5)
						{
							uCandidateWearStatus = 0;
						}
					}

					/* 噪声/边界保持逻辑 */
					if (uCandidateWearStatus == 1)
					{
						if (tFeatureInfoTemp->fMinData > -.1
							|| (tFeatureInfoTemp->fThreshold > tFeatureInfoTemp->fMinData && uLevel >= 6))
						{
							uCandidateWearStatus = tFeatureInfoTemp->uPrevWearStatus;
						}
					}
				}
			}
		}
	}

	/* =========================================================
	 * 3. 根据候选状态和当前状态的差异，做抗抖切换
	 * ========================================================= */
	if (uCandidateWearStatus != tFeatureInfoTemp->uWearStatus)
	{
		/* 目标是从 0 -> 1 */
		if (uCandidateWearStatus == 1)
		{
			/* 进入佩戴计数加一，退出计数清零 */
			tFeatureInfoTemp->uWearOnCount++;
			tFeatureInfoTemp->uWearOffCount = 0;

			/* 连续 n 次满足进入条件，才真正切换为佩戴 */
			if (tFeatureInfoTemp->uWearOnCount >= 6)
			{
				tFeatureInfoTemp->uWearStatus = 1;
				tFeatureInfoTemp->uWearOnCount = 0;
			}
		}
		else
		{
			/* 目标是从 1 -> 0
			 * 退出佩戴计数加一，进入计数清零
			 */
			tFeatureInfoTemp->uWearOffCount++;
			tFeatureInfoTemp->uWearOnCount = 0;

			/* 连续 n 次满足退出条件，才真正切换为未佩戴 */
			if (tFeatureInfoTemp->uWearOffCount >= 3)
			{
				tFeatureInfoTemp->uWearStatus = 0;
				tFeatureInfoTemp->uWearOffCount = 0;
			}
		}
	}
	else
	{
		/* 候选状态和当前状态一致，说明本轮没有切换趋势，清空抗抖计数 */
		tFeatureInfoTemp->uWearOnCount = 0;
		tFeatureInfoTemp->uWearOffCount = 0;
	}

	/* 保存最终状态，供下一轮“保持上一状态”逻辑使用 */
	tFeatureInfoTemp->uPrevWearStatus = tFeatureInfoTemp->uWearStatus;

	/* =========================================================
	 * 4. 只有在“已佩戴”时，才进行补偿状态计算
	 * ========================================================= */
	if ((tFeatureInfoTemp->uWearStatus == 1) && uLevel < 10) 
	{
		unsigned int uCount_Min = 0;
		unsigned int uCount_Max = 0;

		/* 遍历最近缓存的 6 个最小负压点，判断当前是否偏离 loaded 阈值 */
		for (unsigned int i = 0; i < 5; i++)
		{
			/* fMinDataBuff 是滤波后的数据，会比实际负压偏小，
			 * 所以这里用 fMinDataBuff[i] - 2 来做近似补偿
			 * （后续如果能缓存原始负压数据会更准确）
			 */
			if ((tFeatureInfoTemp->fMinDataBuff[i]) > tFeatureInfoTemp->fLoadedThresholdBuff[i] + 0.5f &&
				(tFeatureInfoTemp->fMinDataBuff[i]) < -5)
			{
				/* 负压偏小，记录一次“需要增强补偿”的计数 */
				uCount_Min += 1;
			}
			else if ((tFeatureInfoTemp->fMinDataBuff[i]) < tFeatureInfoTemp->fLoadedThresholdBuff[i] - 0.5f &&
					 (tFeatureInfoTemp->fMinDataBuff[i]) < -5)
			{
				/* 负压偏大，记录一次“需要减弱补偿”的计数 */
				uCount_Max += 1;
			}
		}

		float error = 0.0f;
		float err_sum_candidate = 0.0f;
		int newPwmOffset = 0;

		float kp = 18.0f;
		float ki = 3.0f;
		float deadband = 0.1f;
		float i_limit = 30.0f;
		int step_limit = 100;

		/* 先用最近3次 valley 计算一个统一误差，注意方向要和旧逻辑一致：
		* 偏浅 -> error > 0 -> 输出正补偿
		* 偏深 -> error < 0 -> 输出负补偿
		*/
		float err0 = tFeatureInfoTemp->fMinDataBuff[0] - tFeatureInfoTemp->fLoadedThresholdBuff[0];
		float err1 = tFeatureInfoTemp->fMinDataBuff[1] - tFeatureInfoTemp->fLoadedThresholdBuff[1];
		float err2 = tFeatureInfoTemp->fMinDataBuff[2] - tFeatureInfoTemp->fLoadedThresholdBuff[2];
		float err3 = tFeatureInfoTemp->fMinDataBuff[3] - tFeatureInfoTemp->fLoadedThresholdBuff[3];
		float err4 = tFeatureInfoTemp->fMinDataBuff[4] - tFeatureInfoTemp->fLoadedThresholdBuff[4];
		error = 0.2f * err2 + 0.3f * err3 + 0.5f * err4;

		/* 至少有 4 个点提示负压偏小，进入补偿状态 1 */
		if (uCount_Min >= 4)
		{
			tFeatureInfoTemp->uCompsStatus = 1;

			if (error < 0.0f)
			{
				tFeatureInfoTemp->uCompsStatus = 0;
				error = 0.0f;
			}			

			/* 如果偏差过大，则不输出 PWM 补偿 */
			if (fabs(error) > 10.0f)
			{
				tFeatureInfoTemp->iDelta_pwm = 0;
				tFeatureInfoTemp->fCompErrSum = 0.0f;
				tFeatureInfoTemp->fCompPrevErr = 0.0f;
			}
			else
			{
				if (fabs(error) < deadband)
				{
					error = 0.0f;
				}

				/* PI 控制：偏浅时 error 应为正，输出正补偿 */
				err_sum_candidate = tFeatureInfoTemp->fCompErrSum + error;
				err_sum_candidate = CLAMP(err_sum_candidate, -i_limit, i_limit);

				tFeatureInfoTemp->iDelta_pwm =
					(int)(kp * error + ki * err_sum_candidate);
				tFeatureInfoTemp->iDelta_pwm = CLAMP(tFeatureInfoTemp->iDelta_pwm, 0, step_limit);

				newPwmOffset = tFeatureInfoTemp->iPwmOffset + tFeatureInfoTemp->iDelta_pwm;

				/* 输出正向饱和时冻结积分 */
				if (!(newPwmOffset >= 3000 && error > 0.0f))
				{
					tFeatureInfoTemp->fCompErrSum = err_sum_candidate;
				}

				tFeatureInfoTemp->iPwmOffset = CLAMP(newPwmOffset, -3000, 3000);
				tFeatureInfoTemp->fCompPrevErr = error;
			}
		}
		/* 至少有 4 个点提示负压偏大，进入补偿状态 2 */
		else if (uCount_Max >= 4)
		{
			tFeatureInfoTemp->uCompsStatus = 2;

			if (error > 0.0f)
			{
				error = 0.0f;
				tFeatureInfoTemp->uCompsStatus = 0;
			}		

			/* 如果偏差过大，则不输出 PWM 补偿 */
			if (fabs(error) > 10.0f)
			{
				tFeatureInfoTemp->iDelta_pwm = 0;
				tFeatureInfoTemp->fCompErrSum = 0.0f;
				tFeatureInfoTemp->fCompPrevErr = 0.0f;
			}
			else
			{
				if (fabs(error) < deadband)
				{
					error = 0.0f;
				}

				/* PI 控制：偏深时 error 应为负，输出负补偿 */
				err_sum_candidate = tFeatureInfoTemp->fCompErrSum + error;
				err_sum_candidate = CLAMP(err_sum_candidate, -i_limit, i_limit);

				tFeatureInfoTemp->iDelta_pwm =
					(int)(kp * error + ki * err_sum_candidate);
				tFeatureInfoTemp->iDelta_pwm = CLAMP(tFeatureInfoTemp->iDelta_pwm, -step_limit, 0);

				newPwmOffset = tFeatureInfoTemp->iPwmOffset + tFeatureInfoTemp->iDelta_pwm;

				/* 输出负向饱和时冻结积分 */
				if (!(newPwmOffset <= -3000 && error < 0.0f))
				{
					tFeatureInfoTemp->fCompErrSum = err_sum_candidate;
				}

				tFeatureInfoTemp->iPwmOffset = CLAMP(newPwmOffset, -3000, 3000);
				tFeatureInfoTemp->fCompPrevErr = error;
			}
		}
		else
		{
			/* 当前不需要补偿，后续可继续补充更完整的泵侧补偿逻辑 */
			tFeatureInfoTemp->uCompsStatus = 0;
			tFeatureInfoTemp->iDelta_pwm = 0;

			/* 没触发补偿时，缓慢释放积分，避免积分残留过大 */
			tFeatureInfoTemp->fCompErrSum *= 0.5f;
			tFeatureInfoTemp->fCompPrevErr = 0.0f;
		}
	}
}
#else
void WearDetectionDetermination(SFEATURE_INFO* tFeatureInfoTemp, SDEVICE_INFO* tDeviceInfoTemp)
{
	//tFeatureInfoTemp->iDelta_pwm = 0;

	/* 本轮先计算一个“候选穿戴状态”，
	 * 不直接修改 uWearStatus，最后再结合抗抖计数决定是否切换
	 */
	unsigned char uCandidateWearStatus = tFeatureInfoTemp->uWearStatus;
	unsigned int uMode = tDeviceInfoTemp->uMode;
	unsigned int uLevel = tDeviceInfoTemp->uLevel;

	/* =========================================================
	 * 1. 当前处于“未佩戴”状态时，判断是否进入“已佩戴”状态
	 * ========================================================= */
	if (tFeatureInfoTemp->uWearStatus == 0)
	{
		/* -------------------------
		 * 1.1 吸乳模式
		 * ------------------------- */
		if (uMode == MODE_EXPRESSION)
		{
			if(tFeatureInfoTemp->uWaveSpikes <= 1)
			{
				uCandidateWearStatus = 1;
			}
		}
		/* -------------------------
		 * 1.2 刺激模式
		 * ------------------------- */
		else if (uMode == MODE_STIMULATION)
		{
			if(uLevel >=4)
			{
				if(tFeatureInfoTemp->uWaveSpikes == 0)
				{
					uCandidateWearStatus = 1;
				}
			}
			else
			{
				if((tFeatureInfoTemp->fThreshold + 0.25) > (tFeatureInfoTemp->fRawSegMinPressure + ((float)tFeatureInfoTemp->uWaveSpikes - 1.5) * 3))
				{
					uCandidateWearStatus = 1;
				}
			}
		}

		/* -------------------------
		 * 1.3 mixed 模式
		 * ------------------------- */
		else if (uMode == MODE_MIXED)
		{
			if(uLevel >=4)
			{
				if(tFeatureInfoTemp->uWaveSpikes == 0)
				{
					uCandidateWearStatus = 1;
				}
			}
			else
			{
				if((tFeatureInfoTemp->fThreshold + 0.25) > (tFeatureInfoTemp->fRawSegMinPressure + ((float)tFeatureInfoTemp->uWaveSpikes - 1.5) * 3))
				{
					uCandidateWearStatus = 1;
				}
			}
		}
	}

	/* =========================================================
	 * 2. 当前处于“已佩戴”状态时，判断是否退出为“未佩戴”状态
	 * ========================================================= */
	else if (tFeatureInfoTemp->uWearStatus == 1)
	{
		/* 条件1：
		 * 当前最小负压已经不够深，直接判定退出佩戴
		 */
		if (tFeatureInfoTemp->fMinData > -1)
		{
			uCandidateWearStatus = 0;
		}

		/* 只有在前面没有已经直接判成 0 的情况下，再继续按模式和档位判定 */
		if (uCandidateWearStatus == 1)
		{
			/* -------------------------
			 * 2.1 吸乳模式
			 * ------------------------- */
			if (uMode == MODE_EXPRESSION)
			{
				if(tFeatureInfoTemp->uWaveSpikes >= 2)
				{
					uCandidateWearStatus = 0;
				}
			}

			/* -------------------------
			 * 2.2 刺激模式
			 * ------------------------- */
			else if (uMode == MODE_STIMULATION)
			{
				if(uLevel >=4)
				{
					if(tFeatureInfoTemp->uWaveSpikes >= 1)
					{
						uCandidateWearStatus = 0;
					}
				}
				else
				{
					if((tFeatureInfoTemp->fThreshold + 0.25) < (tFeatureInfoTemp->fFilteredSegMinPressure + ((float)tFeatureInfoTemp->uWaveSpikes - 1.5) * 3))
					{
						uCandidateWearStatus = 0;
					}
				}
			}

			/* -------------------------
			 * 2.3 mixed 模式
			 * ------------------------- */
			else if (uMode == MODE_MIXED)
			{
				if(uLevel >=4)
				{
					if(tFeatureInfoTemp->uWaveSpikes >= 1)
					{
						uCandidateWearStatus = 0;
					}
				}
				else
				{
					if((tFeatureInfoTemp->fThreshold + 0.25) < (tFeatureInfoTemp->fFilteredSegMinPressure + ((float)tFeatureInfoTemp->uWaveSpikes - 1.5) * 3))
					{
						uCandidateWearStatus = 0;
					}
				}
			}
		}
	}

	/* =========================================================
	 * 3. 根据候选状态和当前状态的差异，做抗抖切换
	 * ========================================================= */
	if (uCandidateWearStatus != tFeatureInfoTemp->uWearStatus)
	{
		/* 目标是从 0 -> 1 */
		if (uCandidateWearStatus == 1)
		{
			/* 进入佩戴计数加一，退出计数清零 */
			tFeatureInfoTemp->uWearOnCount++;
			tFeatureInfoTemp->uWearOffCount = 0;

			/* 连续 n 次满足进入条件，才真正切换为佩戴 */
			if (tFeatureInfoTemp->uWearOnCount >= 6)
			{
				tFeatureInfoTemp->uWearStatus = 1;
				tFeatureInfoTemp->uWearOnCount = 0;
			}
		}
		else
		{
			/* 目标是从 1 -> 0
			 * 退出佩戴计数加一，进入计数清零
			 */
			tFeatureInfoTemp->uWearOffCount++;
			tFeatureInfoTemp->uWearOnCount = 0;

			/* 连续 n 次满足退出条件，才真正切换为未佩戴 */
			if (tFeatureInfoTemp->uWearOffCount >= 3)
			{
				tFeatureInfoTemp->uWearStatus = 0;
				tFeatureInfoTemp->uWearOffCount = 0;
			}
		}
	}
	else
	{
		/* 候选状态和当前状态一致，说明本轮没有切换趋势，清空抗抖计数 */
		tFeatureInfoTemp->uWearOnCount = 0;
		tFeatureInfoTemp->uWearOffCount = 0;
	}

	/* 保存最终状态，供下一轮“保持上一状态”逻辑使用 */
	tFeatureInfoTemp->uPrevWearStatus = tFeatureInfoTemp->uWearStatus;

	/* =========================================================
	 * 4. 只有在“已佩戴”时，才进行补偿状态计算
	 * ========================================================= */
	if (tFeatureInfoTemp->uWearStatus == 1) 
	{
		unsigned int uCount_Min = 0;
		unsigned int uCount_Max = 0;

		/* 遍历最近缓存的 6 个最小负压点，判断当前是否偏离 loaded 阈值 */
		for (unsigned int i = 0; i < 5; i++)
		{
			/* fMinDataBuff 是滤波后的数据，会比实际负压偏小，
			 * 所以这里用 fMinDataBuff[i] - 2 来做近似补偿
			 * （后续如果能缓存原始负压数据会更准确）
			 */
			if ((tFeatureInfoTemp->fMinDataBuff[i]) > tFeatureInfoTemp->fLoadedThresholdBuff[i] + 0.5f &&
				(tFeatureInfoTemp->fMinDataBuff[i]) < -5)
			{
				/* 负压偏小，记录一次“需要增强补偿”的计数 */
				uCount_Min += 1;
			}
			else if ((tFeatureInfoTemp->fMinDataBuff[i]) < tFeatureInfoTemp->fLoadedThresholdBuff[i] - 0.5f &&
					 (tFeatureInfoTemp->fMinDataBuff[i]) < -5)
			{
				/* 负压偏大，记录一次“需要减弱补偿”的计数 */
				uCount_Max += 1;
			}
		}

		float error = 0.0f;
		float err_sum_candidate = 0.0f;
		int newPwmOffset = 0;

		float kp = 18.0f;
		float ki = 3.0f;
		float deadband = 0.1f;
		float i_limit = 30.0f;
		int step_limit = 100;

		/* 先用最近3次 valley 计算一个统一误差，注意方向要和旧逻辑一致：
		* 偏浅 -> error > 0 -> 输出正补偿
		* 偏深 -> error < 0 -> 输出负补偿
		*/
		float err0 = tFeatureInfoTemp->fMinDataBuff[0] - tFeatureInfoTemp->fLoadedThresholdBuff[0];
		float err1 = tFeatureInfoTemp->fMinDataBuff[1] - tFeatureInfoTemp->fLoadedThresholdBuff[1];
		float err2 = tFeatureInfoTemp->fMinDataBuff[2] - tFeatureInfoTemp->fLoadedThresholdBuff[2];
		float err3 = tFeatureInfoTemp->fMinDataBuff[3] - tFeatureInfoTemp->fLoadedThresholdBuff[3];
		float err4 = tFeatureInfoTemp->fMinDataBuff[4] - tFeatureInfoTemp->fLoadedThresholdBuff[4];
		error = 0.2f * err2 + 0.3f * err3 + 0.5f * err4;

		/* 至少有 4 个点提示负压偏小，进入补偿状态 1 */
		if (uCount_Min >= 4)
		{
			tFeatureInfoTemp->uCompsStatus = 1;

			if (error < 0.0f)
			{
				tFeatureInfoTemp->uCompsStatus = 0;
				error = 0.0f;
			}			

			/* 如果偏差过大，则不输出 PWM 补偿 */
			if (fabs(error) > 10.0f)
			{
				tFeatureInfoTemp->iDelta_pwm = 0;
				tFeatureInfoTemp->fCompErrSum = 0.0f;
				tFeatureInfoTemp->fCompPrevErr = 0.0f;
			}
			else
			{
				if (fabs(error) < deadband)
				{
					error = 0.0f;
				}

				/* PI 控制：偏浅时 error 应为正，输出正补偿 */
				err_sum_candidate = tFeatureInfoTemp->fCompErrSum + error;
				err_sum_candidate = CLAMP(err_sum_candidate, -i_limit, i_limit);

				tFeatureInfoTemp->iDelta_pwm =
					(int)(kp * error + ki * err_sum_candidate);
				tFeatureInfoTemp->iDelta_pwm = CLAMP(tFeatureInfoTemp->iDelta_pwm, 0, step_limit);

				newPwmOffset = tFeatureInfoTemp->iPwmOffset + tFeatureInfoTemp->iDelta_pwm;

				/* 输出正向饱和时冻结积分 */
				if (!(newPwmOffset >= 3000 && error > 0.0f))
				{
					tFeatureInfoTemp->fCompErrSum = err_sum_candidate;
				}

				tFeatureInfoTemp->iPwmOffset = CLAMP(newPwmOffset, -3000, 3000);
				tFeatureInfoTemp->fCompPrevErr = error;
			}
		}
		/* 至少有 4 个点提示负压偏大，进入补偿状态 2 */
		else if (uCount_Max >= 4)
		{
			tFeatureInfoTemp->uCompsStatus = 2;

			if (error > 0.0f)
			{
				error = 0.0f;
				tFeatureInfoTemp->uCompsStatus = 0;
			}		

			/* 如果偏差过大，则不输出 PWM 补偿 */
			if (fabs(error) > 10.0f)
			{
				tFeatureInfoTemp->iDelta_pwm = 0;
				tFeatureInfoTemp->fCompErrSum = 0.0f;
				tFeatureInfoTemp->fCompPrevErr = 0.0f;
			}
			else
			{
				if (fabs(error) < deadband)
				{
					error = 0.0f;
				}

				/* PI 控制：偏深时 error 应为负，输出负补偿 */
				err_sum_candidate = tFeatureInfoTemp->fCompErrSum + error;
				err_sum_candidate = CLAMP(err_sum_candidate, -i_limit, i_limit);

				tFeatureInfoTemp->iDelta_pwm =
					(int)(kp * error + ki * err_sum_candidate);
				tFeatureInfoTemp->iDelta_pwm = CLAMP(tFeatureInfoTemp->iDelta_pwm, -step_limit, 0);

				newPwmOffset = tFeatureInfoTemp->iPwmOffset + tFeatureInfoTemp->iDelta_pwm;

				/* 输出负向饱和时冻结积分 */
				if (!(newPwmOffset <= -3000 && error < 0.0f))
				{
					tFeatureInfoTemp->fCompErrSum = err_sum_candidate;
				}

				tFeatureInfoTemp->iPwmOffset = CLAMP(newPwmOffset, -3000, 3000);
				tFeatureInfoTemp->fCompPrevErr = error;
			}
		}
		else
		{
			/* 当前不需要补偿，后续可继续补充更完整的泵侧补偿逻辑 */
			tFeatureInfoTemp->uCompsStatus = 0;
			tFeatureInfoTemp->iDelta_pwm = 0;

			/* 没触发补偿时，缓慢释放积分，避免积分残留过大 */
			tFeatureInfoTemp->fCompErrSum *= 0.5f;
			tFeatureInfoTemp->fCompPrevErr = 0.0f;
		}
	}
}
#endif

/*************************************************************************************************************************************************
* 函数名称：V3WearDetectionMain
* 作    者：sahar.hu
* 创建日期：2025.12.19
* 功能说明：佩戴检测主函数
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void V3WearDetectionMain(algo_air_pressure_data_t sPressureData)
{
	unsigned char uLevelLeft = sPressureData.pump_state[CHANNEL_0].level;
	unsigned char uLevelRight = sPressureData.pump_state[CHANNEL_1].level;
	unsigned char uModeLeft = sPressureData.pump_state[CHANNEL_0].mode;
	unsigned char uModeRight = sPressureData.pump_state[CHANNEL_1].mode;

	/* reset if mode or level changed */
	if ((uLevelLeft - 1 != tDeviceInfo[CHANNEL_0].uLevel) ||
		(uModeLeft != tDeviceInfo[CHANNEL_0].uMode))
	{
		V3WearDetectionModeReset(uLevelLeft, uModeLeft, CHANNEL_0);
	}

	if ((uLevelRight - 1 != tDeviceInfo[CHANNEL_1].uLevel) ||
		(uModeRight != tDeviceInfo[CHANNEL_1].uMode))
	{
		V3WearDetectionModeReset(uLevelRight, uModeRight, CHANNEL_1);
	}

	/* reset if do_work is 0 */
	if(sPressureData.do_work == 0)
	{
		V3WearDetectionModeReset(uLevelLeft, uModeLeft, CHANNEL_0);
		V3WearDetectionModeReset(uLevelRight, uModeRight, CHANNEL_1);
	}

	/* update device target parameters every cycle */
	for (unsigned int i = 0; i < CHANNEL_NUM; i++)
	{
		tDeviceInfo[i].uTargetPressure = sPressureData.pump_state[i].target_pressure;
		tDeviceInfo[i].uTargetPumpTime = sPressureData.pump_state[i].target_pump_time;
	}

#if DEBUG == 1
	{
		unsigned int i = 0;
		const STARGET_CURVE_ITEM* ptTargetCurve = NULL;

		if ((sPressureData.pump_state[i].state == SUCTION_STATUS_GENERATE) ||
			(sPressureData.pump_state[i].state == SUCTION_STATUS_HOLD))
		{
			tFeatureInfo[i].pumping_time += 1;
		}
		else
		{
			tFeatureInfo[i].uCompsStatus = 0;
			tFeatureInfo[i].iDelta_pwm = 0;
			tFeatureInfo[i].pumping_time = 0;
		}

		ptTargetCurve = GetCurrentTargetCurveItem(&tDeviceInfo[i]);
		if (ptTargetCurve != NULL)
		{
			tFeatureInfo[i].fThreshold = ptTargetCurve->target_pump_pressure;
			tFeatureInfo[i].fLoadedThreshold = ptTargetCurve->target_load_pressure;
		}

		PressureDataPreprocess(sPressureData.air_pressure[i],
			&tFilterInfo[i],
			&tDataBuffer[i],
			&tFeatureInfo[i]);
		PressureDataFeature(&tPVInfo[i],
			&tWaveInfo[i],
			&tDataBuffer[i],
			&tFeatureInfo[i],
			&tDeviceInfo[i]);
		WearDetectionDetermination(&tFeatureInfo[i], &tDeviceInfo[i]);
	}
#else
	for (unsigned int i = 0; i < CHANNEL_NUM; i++)
	{
		tFeatureInfo[i].uPrevLeakStatus = tFeatureInfo[i].uLeakStatus;
		if (tFeatureInfo[i].uWearDetSegMinCommitted == 1)
		{
			tFeatureInfo[i].uWearDetSegMinCommitted = 0;
		}
		const STARGET_CURVE_ITEM* ptTargetCurve = NULL;

		tFeatureInfo[i].uPumpState = sPressureData.pump_state[i].state;

		ptTargetCurve = GetCurrentTargetCurveItem(&tDeviceInfo[i]);
		if (ptTargetCurve != NULL)
		{
			tFeatureInfo[i].fThreshold = ptTargetCurve->target_pump_pressure;
			tFeatureInfo[i].fLoadedThreshold = ptTargetCurve->target_load_pressure;
		}
		PressureDataPreprocess(sPressureData.air_pressure[i],
			&tFilterInfo[i],
			&tDataBuffer[i],
			&tFeatureInfo[i]);
		{
			float fFilteredNow = tDataBuffer[i].fDataBuff[tDataBuffer[i].uCurrPos];
			UpdateRawPumpSegment(sPressureData.air_pressure[i],
				fFilteredNow,
				tFeatureInfo[i].fThreshold,
				sPressureData.pump_state[i].state,
				&tFeatureInfo[i]);
		}

		if ((sPressureData.pump_state[i].state == SUCTION_STATUS_GENERATE) ||
			(sPressureData.pump_state[i].state == SUCTION_STATUS_HOLD))
		{
			tFeatureInfo[i].pumping_time += 1;
		}
		else
		{
			/* 如果不在泵奶期间，则不进行算法处理 */
			tFeatureInfo[i].uCompsStatus = 0;
			tFeatureInfo[i].iDelta_pwm = 0;
			tFeatureInfo[i].pumping_time = 0;
			tFeatureInfo[i].fCompErrSum = 0;
			tFeatureInfo[i].fCompPrevErr = 0;
		}
		/* 休眠状态下 平台期是否回到基线0 */
		if (SUCTION_STATUS_SLEEP == sPressureData.pump_state[i].state || 
			(fabs(sPressureData.air_pressure[i] - tDataBuffer[i].fDataBuff[tDataBuffer[i].uCurrPos]) < .1 && sPressureData.pump_state[i].level > 1))
		{
			if (sPressureData.air_pressure[i] < -1)
			{
				tFeatureInfo[i].uDataBaselineCnt[0]++;
				//printf("1. %d, %d %f, %f\n", sPressureData.pump_state[i].state, tFeatureInfo[i].uDataBaselineCnt[0], sPressureData.air_pressure[i], tDataBuffer[i].fDataBuff[tDataBuffer[i].uCurrPos]);
			}
		}
		if (SUCTION_STATUS_SLEEP == sPressureData.pump_state[i].state && sPressureData.air_pressure[i] > -.6)
		{
			tFeatureInfo[i].uDataBaselineCnt[1]++;
		}

		PressureDataFeature(&tPVInfo[i],
			&tWaveInfo[i],
			&tDataBuffer[i],
			&tFeatureInfo[i],
			&tDeviceInfo[i]);
		if (tFeatureInfo[i].uWearDetSegMinCommitted)
		{
			WearDetectionDetermination(&tFeatureInfo[i], &tDeviceInfo[i]);
			tFeatureInfo[i].uDataBaselineCnt[1] = 0;
		}


	}
#endif /* DEBUG */
}


/*************************************************************************************************************************************************
* 函数名称：GetWearDetectionResult
* 作    者：sahar.hu
* 创建日期：2025.12.19
* 功能说明：佩戴检测取结果
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void GetWearDetectionResult(unsigned char* uWearStatus)
{
	// uWearStatus[0] = tFeatureInfo[0].uWearStatus;
	// uWearStatus[1] = tFeatureInfo[1].uWearStatus;
	uWearStatus[0] = tFeatureInfo[0].uLeakStatus;
	uWearStatus[1] = tFeatureInfo[1].uLeakStatus;
}


/*************************************************************************************************************************************************
* 函数名称：GetCompensationStatus
* 作    者：sahar.hu
* 创建日期：2025.12.19
* 功能说明：获取补偿状态
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void GetCompensationStatus(unsigned char* uCompsStatus, int* iDelta_pwm)
{
	uCompsStatus[0] = tFeatureInfo[0].uCompsStatus;
	uCompsStatus[1] = tFeatureInfo[1].uCompsStatus;

	if ((tFeatureInfo[0].uPumpState == SUCTION_STATUS_GENERATE) ||
		(tFeatureInfo[0].uPumpState == SUCTION_STATUS_HOLD))
		iDelta_pwm[0] = tFeatureInfo[0].iPwmOffset;
	else
		iDelta_pwm[0] = 0;

	if ((tFeatureInfo[1].uPumpState == SUCTION_STATUS_GENERATE) ||
		(tFeatureInfo[1].uPumpState == SUCTION_STATUS_HOLD))
		iDelta_pwm[1] = tFeatureInfo[1].iPwmOffset;
	else
		iDelta_pwm[1] = 0;
}

//#endif