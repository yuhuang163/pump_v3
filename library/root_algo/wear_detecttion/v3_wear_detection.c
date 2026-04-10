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
#define DEBUG		0

// global data
//SGLOBAL_INFO		tGlobalInfo;
SDATA_BUFFER		tDataBuffer[2];
SPV_INFO			tPVInfo[2];
SWAVE_INFO			tWaveInfo[2];
SFEATURE_INFO		tFeatureInfo[2];
SDEVICE_INFO		tDeviceInfo[2];
SFILTER_INFO		tFilterInfo[2];

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
			tDataBuffer[j].uCurrPos = 0;
		}
		tDataBuffer[j].fDataMax = -1;
		tDataBuffer[j].fDataMin = 0;
		tDataBuffer[j].uFirstTime = 1;

		/* features */
		tFeatureInfo[j].uUpdateFlag = 0;
		tFeatureInfo[j].fMinData = 0;
		tFeatureInfo[j].uNoiseLevel = 0;
		tFeatureInfo[j].uWaveSpikes = 0;
		tFeatureInfo[j].uPVIndex = 0;
		tFeatureInfo[j].uPumpFlag = 0;
		tFeatureInfo[j].uWearStatus = 0;
		tFeatureInfo[j].uNormalWaveform = 0;
		tFeatureInfo[j].fMinDataBuffMin = 0;
		tFeatureInfo[j].uCompsStatus = 0;
		tFeatureInfo[j].iDelta_pwm = 0;
		tFeatureInfo[j].uPrevWearStatus = 0;
		for (int i = 0; i < 5; i++)
		{
			tFeatureInfo[j].fMinDataBuff[i] = 0;
		}

		switch (tDeviceInfo[j].uMode)
		{
		case MODE_EXPRESSION:
			tDeviceInfo[j].run_time = EXPRESSION_WORK_TIME[tDeviceInfo[j].uLevel];
			tFeatureInfo[j].fThreshold = UNLOADED_EXPRESSION_THRESHOLD[tDeviceInfo[j].uLevel];
			tFeatureInfo[j].fLoadedThreshold = EXPRESSION_THRESHOLD[tDeviceInfo[j].uLevel];
			break;
		case MODE_MIXED:
			tFeatureInfo[j].fThreshold = UNLOADED_MIXED_THRESHOLD[tDeviceInfo[j].uLevel];
			tFeatureInfo[j].fLoadedThreshold = MIXED_THRESHOLD[tDeviceInfo[j].uLevel];
			break;
		case MODE_STIMULATION:
			tDeviceInfo[j].run_time = STIMULATION_WORK_TIME[tDeviceInfo[j].uLevel];
			tFeatureInfo[j].fThreshold = UNLOADED_STIMULATION_THRESHOLD[tDeviceInfo[j].uLevel];
			tFeatureInfo[j].fLoadedThreshold = STIMULATION_THRESHOLD[tDeviceInfo[j].uLevel];
			break;
		}
		/* filter info */
		tFilterInfo[j].fFc = 3.0;
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
	for (int i = 0; i < 6; i++)
	{
		tFeatureInfo[uChannelIdx].fMinDataBuff[i] = 0;
	}
	/* Device info */
	tDeviceInfo[uChannelIdx].uMode = uMode;
	tDeviceInfo[uChannelIdx].uLevel = uLevel - 1;
	switch (uMode)
	{
	case MODE_EXPRESSION:
		tFeatureInfo[uChannelIdx].fThreshold = UNLOADED_EXPRESSION_THRESHOLD[tDeviceInfo[uChannelIdx].uLevel];
		tFeatureInfo[uChannelIdx].fLoadedThreshold = EXPRESSION_THRESHOLD[tDeviceInfo[uChannelIdx].uLevel];
		break;
	case MODE_MIXED:
		tFeatureInfo[uChannelIdx].fThreshold = UNLOADED_MIXED_THRESHOLD[tDeviceInfo[uChannelIdx].uLevel];
		tFeatureInfo[uChannelIdx].fLoadedThreshold = MIXED_THRESHOLD[tDeviceInfo[uChannelIdx].uLevel];
		break;
	case MODE_STIMULATION:
		tFeatureInfo[uChannelIdx].fThreshold = UNLOADED_STIMULATION_THRESHOLD[tDeviceInfo[uChannelIdx].uLevel];
		tFeatureInfo[uChannelIdx].fLoadedThreshold = STIMULATION_THRESHOLD[tDeviceInfo[uChannelIdx].uLevel];
		break;
	}
	/* filter info */
	tFilterInfo[uChannelIdx].fFc = 3.0;
	tFilterInfo[uChannelIdx].fDT = 1.0 / SAMPLE_RATE;
	tFilterInfo[uChannelIdx].fRC = 1.0 / (2.0 * 3.1415926 * tFilterInfo[uChannelIdx].fFc);
	tFilterInfo[uChannelIdx].fAlpha = tFilterInfo[uChannelIdx].fDT / (tFilterInfo[uChannelIdx].fRC + tFilterInfo[uChannelIdx].fDT);
	tFilterInfo[uChannelIdx].fYpre = 0.0;
	tFilterInfo[uChannelIdx].uInitState = 0;
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
float FilterPreprocess(float fInputData, SFILTER_INFO *tFilterInfoTemp)
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
void PressureDataPreprocess(float fPumpData, SFILTER_INFO *tFilterInfoTemp, SDATA_BUFFER *tDataBufferTemp, SFEATURE_INFO *tFeatureInfoTemp)
{
	float filtered_data = 0.0;
	if (fPumpData < -100)
		fPumpData = 0;
	filtered_data = FilterPreprocess(fPumpData, tFilterInfoTemp);
	
	tDataBufferTemp->fDataBuff[tDataBufferTemp->uCurrPos] = filtered_data;
	if (tFeatureInfoTemp->uDataIdx == SAMPLE_RATE - 1)
	{
		float fminDataBuffMin = 0, fminDataBuffMax = -100;
		for (int i = 0; i < 5; i++)
		{
			if (tFeatureInfoTemp->fMinDataBuff[i] < fminDataBuffMin)
				fminDataBuffMin = tFeatureInfoTemp->fMinDataBuff[i];
			if (tFeatureInfoTemp->fMinDataBuff[i] > fminDataBuffMax)
				fminDataBuffMax = tFeatureInfoTemp->fMinDataBuff[i];
			tFeatureInfoTemp->fMinDataBuff[i] = 0;
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
void PressureDataFeature(SPV_INFO *tPVInfoTemp, SWAVE_INFO * tWaveInfoTemp, SDATA_BUFFER* tDataBufferTemp, SFEATURE_INFO* tFeatureInfoTemp, SDEVICE_INFO *tDeviceInfoTemp)
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
		/* check wave spikes */
		for (unsigned int i = 0; i < uWaveIndex; i++)
		{
			if (tWaveInfoTemp->uType[i] == 0 && tWaveInfoTemp->fValue[i] < -1 && tWaveInfoTemp->uwidth[i] > 10)
			{
				float fBuffMinTemp = 0;
				for (unsigned int k = 0; k < 4; k++)
				{
					tFeatureInfoTemp->fMinDataBuff[k] = tFeatureInfoTemp->fMinDataBuff[k + 1];
					if (tFeatureInfoTemp->fMinDataBuff[k] > fBuffMinTemp)
						fBuffMinTemp = tFeatureInfoTemp->fMinDataBuff[k];
					tFeatureInfoTemp->fMinDataBuffMin = fBuffMinTemp;
				}
				tFeatureInfoTemp->fMinDataBuff[4] = tWaveInfoTemp->fValue[i];
				if (tFeatureInfoTemp->fMinDataBuff[4] > fBuffMinTemp)
					fBuffMinTemp = tFeatureInfoTemp->fMinDataBuff[4];
				tFeatureInfoTemp->fMinDataBuffMin = fBuffMinTemp;
			}
			/*printf("\n%f %f %f %f %f \n", (float)tFeatureInfoTemp->fMinDataBuff[0],
				tFeatureInfoTemp->fMinDataBuff[1],
				tFeatureInfoTemp->fMinDataBuff[2],
				tFeatureInfoTemp->fMinDataBuff[3], tFeatureInfoTemp->fMinDataBuff[4]);*/
			if (tWaveInfoTemp->uType[i] == 0 &&

				(tWaveInfoTemp->fValue[i] < tFeatureInfoTemp->fThreshold &&
					(tDeviceInfoTemp->uMode == MODE_STIMULATION || (tDeviceInfoTemp->uMode == MODE_EXPRESSION && tDeviceInfoTemp->uLevel <= 12)))

				|| (tWaveInfoTemp->fValue[i] < -1 && fabs(tFeatureInfoTemp->fThreshold - tWaveInfoTemp->fValue[i]) < 10 &&
					(tDeviceInfoTemp->uMode == MODE_EXPRESSION && 6 <= tDeviceInfoTemp->uLevel && tDeviceInfoTemp->uLevel <= 10))

				|| (tWaveInfoTemp->fValue[i] < -1 && fabs(tFeatureInfoTemp->fThreshold - tWaveInfoTemp->fValue[i]) >= 2 &&
					(tDeviceInfoTemp->uMode == MODE_EXPRESSION && 11 <= tDeviceInfoTemp->uLevel))

				|| (tWaveInfoTemp->fValue[i] < -1 &&
					(STIMULATION_THRESHOLD[tDeviceInfoTemp->uLevel] > tWaveInfoTemp->fValue[i] ||
						EXPRESSION_THRESHOLD[tDeviceInfoTemp->uLevel] > tWaveInfoTemp->fValue[i])))
			{
				tFeatureInfoTemp->uNormalWaveform++;
			}

		}
	}
	/* update current index */
	tDataBufferTemp->uCurrPos = BufferCircleNextIndex(tDataBufferTemp->uCurrPos, 1, WIN_SIZE);
}


/*************************************************************************************************************************************************
* 函数名称：WearDetectionDetermination
* 作    者：sahar.hu
* 创建日期：2025.12.19
* 功能说明：佩戴检测
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void WearDetectionDetermination(SFEATURE_INFO* tFeatureInfoTemp, SDEVICE_INFO* tDeviceInfoTemp)
{
	//tFeatureInfoTemp->iDelta_pwm = 0;
	if (tFeatureInfoTemp->uWearStatus == 0)
	{
		if ((tFeatureInfoTemp->uWaveSpikes <= 1 && tFeatureInfoTemp->uNoiseLevel == 0 && tFeatureInfoTemp->uNormalWaveform >= 1) &&
			((tDeviceInfoTemp->uLevel <= 12 && (tDeviceInfoTemp->uMode == MODE_EXPRESSION || tDeviceInfoTemp->uMode == MODE_MIXED))
				|| (tDeviceInfoTemp->uMode == MODE_STIMULATION && tDeviceInfoTemp->uLevel < 15)))
			tFeatureInfoTemp->uWearStatus = 1;
		else if ((tFeatureInfoTemp->fThreshold < tFeatureInfoTemp->fMinData && tFeatureInfoTemp->fMinData < -1) &&
			((tFeatureInfoTemp->uWaveSpikes <= 1 && tDeviceInfoTemp->uLevel >= 12 && (tDeviceInfoTemp->uMode == MODE_EXPRESSION)) ||
				(tFeatureInfoTemp->uWaveSpikes < 5 && tDeviceInfoTemp->uMode == MODE_STIMULATION && tDeviceInfoTemp->uLevel >= 12)))
			tFeatureInfoTemp->uWearStatus = 1;
		else if ((tFeatureInfoTemp->fThreshold < tFeatureInfoTemp->fMinDataBuff[5] && tFeatureInfoTemp->fMinDataBuff[5] < -1) &&
			(tDeviceInfoTemp->uLevel >= 9 && tDeviceInfoTemp->uMode == MODE_MIXED) && tFeatureInfoTemp->uWaveSpikes < 5)
			tFeatureInfoTemp->uWearStatus = 1;
	}
	else if (tFeatureInfoTemp->uWearStatus == 1)
	{
		if (tFeatureInfoTemp->fMinData > -1)
		{
			tFeatureInfoTemp->uWearStatus = 0;
		}
		
		if ((tFeatureInfoTemp->fThreshold < tFeatureInfoTemp->fMinData && tFeatureInfoTemp->fMinData < -.1 && tDeviceInfoTemp->uLevel <= 6 && (tDeviceInfoTemp->uMode == MODE_EXPRESSION))
			|| (tFeatureInfoTemp->fThreshold > tFeatureInfoTemp->fMinData && tDeviceInfoTemp->uLevel >= 7 && (tDeviceInfoTemp->uMode == MODE_EXPRESSION))
			|| (tFeatureInfoTemp->fThreshold < tFeatureInfoTemp->fMinData && tFeatureInfoTemp->fMinData < -.1 && tDeviceInfoTemp->uMode == MODE_STIMULATION && tDeviceInfoTemp->uLevel <= 13)
			/* || (wave_spikes >= 3 && tDeviceInfoTemp->uMode == 'stimulation' && 14 > level >= 10)*/
			|| (STIMULATION_THRESHOLD[tDeviceInfoTemp->uLevel] < tFeatureInfoTemp->fMinData && tFeatureInfoTemp->fMinData < -.1 && tDeviceInfoTemp->uLevel <= 3 && tDeviceInfoTemp->uMode == MODE_MIXED))
			tFeatureInfoTemp->uWearStatus = 0;
		else if (tFeatureInfoTemp->uWaveSpikes > 2 && (7 <= tDeviceInfoTemp->uLevel && (tDeviceInfoTemp->uMode == MODE_EXPRESSION || tDeviceInfoTemp->uMode == MODE_MIXED)))
			tFeatureInfoTemp->uWearStatus = 0;
		else if ((STIMULATION_THRESHOLD[tDeviceInfoTemp->uLevel] < tFeatureInfoTemp->fMinDataBuffMin) && tFeatureInfoTemp->uWaveSpikes >= 15 &&
			(4 <= tDeviceInfoTemp->uLevel && tDeviceInfoTemp->uLevel <= 6 && tDeviceInfoTemp->uMode == MODE_MIXED))
			tFeatureInfoTemp->uWearStatus = 0;
		else if ((tFeatureInfoTemp->fThreshold > tFeatureInfoTemp->fMinData) && (8 <= tDeviceInfoTemp->uLevel && tDeviceInfoTemp->uMode == MODE_MIXED))
			tFeatureInfoTemp->uWearStatus = 0;
		else if (tFeatureInfoTemp->uWaveSpikes > 10 && (tDeviceInfoTemp->uLevel == 14 && tDeviceInfoTemp->uMode == MODE_STIMULATION))
			tFeatureInfoTemp->uWearStatus = 0;
		else if (tFeatureInfoTemp->fMinData > -.1
			|| (tFeatureInfoTemp->fThreshold > tFeatureInfoTemp->fMinData && tDeviceInfoTemp->uLevel >= 6 && (tDeviceInfoTemp->uMode == MODE_EXPRESSION || tDeviceInfoTemp->uMode == MODE_MIXED)))
			/* noise detection*/
			tFeatureInfoTemp->uWearStatus = tFeatureInfoTemp->uPrevWearStatus;
	}
	tFeatureInfoTemp->uPrevWearStatus = tFeatureInfoTemp->uWearStatus;
	/* compensation status */
	if (tFeatureInfoTemp->uWearStatus == 1 && tFeatureInfoTemp->pumping_time == 1)
	{
		unsigned int uCount_Min = 0;
		unsigned int uCount_Max = 0;

		for (unsigned int i = 0; i < 6; i++)
		{
			// fMinDataBuff是滤波后的数据，会比实际负压偏小，所以这里用fMinDataBuff[i] - 2 来做比较（实际需要缓存原始负压数据）
			if ((tFeatureInfoTemp->fMinDataBuff[i] - 2) > tFeatureInfoTemp->fLoadedThreshold + 2 && (tFeatureInfoTemp->fMinDataBuff[i]-2) < -5)
			{
				/* flag */
				uCount_Min += 1;
			}
			else if ((tFeatureInfoTemp->fMinDataBuff[i] - 2) < tFeatureInfoTemp->fLoadedThreshold - 2 && (tFeatureInfoTemp->fMinDataBuff[i]-2) < -5)
			{
				/* flag */
				uCount_Max += 1;
			}
		}
		if (uCount_Min >= 2)
		{
			tFeatureInfoTemp->uCompsStatus = 1;
			if (fabs((float)(tFeatureInfoTemp->fLoadedThreshold - (tFeatureInfoTemp->fMinData-2)) > 10))
				tFeatureInfoTemp->iDelta_pwm = 0;
			else
				/*2% delta_pwm 对应1kPa变化*/
				tFeatureInfoTemp->iDelta_pwm = 200 * fabs((float)(tFeatureInfoTemp->fLoadedThreshold - (tFeatureInfoTemp->fMinData-2)));
		}

		else if (uCount_Max >= 2)
		{
			tFeatureInfoTemp->uCompsStatus = 2;
			if (fabs((float)(tFeatureInfoTemp->fLoadedThreshold - (tFeatureInfoTemp->fMinData-2)) > 10))
				tFeatureInfoTemp->iDelta_pwm = 0;
			else
				tFeatureInfoTemp->iDelta_pwm = -200 * fabs((float)(tFeatureInfoTemp->fLoadedThreshold - (tFeatureInfoTemp->fMinData-2)));
		}
		else
		{
			tFeatureInfoTemp->uCompsStatus = 0;
			tFeatureInfoTemp->iDelta_pwm = 0;
		}
	}
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
void V3WearDetectionMain(float *fPressureData, algo_air_pressure_data_t sPressureData, unsigned char uLevelLeft, unsigned char ulevelRight, unsigned char uModeLeft, unsigned char uModeRight)
{
	/* reset if mode or level changed */
	if (uLevelLeft - 1 != tDeviceInfo[CHANNEL_0].uLevel || uModeLeft != tDeviceInfo[CHANNEL_0].uMode)
	{
		V3WearDetectionModeReset(uLevelLeft, uModeLeft, CHANNEL_0);
	}
	if (ulevelRight - 1 != tDeviceInfo[CHANNEL_1].uLevel || uModeRight != tDeviceInfo[CHANNEL_1].uMode)
	{
		V3WearDetectionModeReset(ulevelRight, uModeRight, CHANNEL_1);
	}
#if DEBUG == 1
	/* processing left and right data */
	/* for debug, process one channel */
	unsigned int i = 0;
	PressureDataPreprocess(fPressureData[i], &tFilterInfo[i], &tDataBuffer[i], &tFeatureInfo[i]);
	PressureDataFeature(&tPVInfo[i], &tWaveInfo[i], &tDataBuffer[i], &tFeatureInfo[i], &tDeviceInfo[i]);
	WearDetectionDetermination(&tFeatureInfo[i], &tDeviceInfo[i]);
#else

	for (unsigned int i = 0; i < CHANNEL_NUM; i++)
	{
		if (((sPressureData.pump_state[i].state == SUCTION_STATUS_GENERATE) || (sPressureData.pump_state[i].state == SUCTION_STATUS_HOLD)))
		{
			tFeatureInfo[i].pumping_time += 1;
		}
		else /* if (sPressureData.pump_state[i].state != SUCTION_STATUS_IDLE)*/
		{
			// 如果不在泵奶期间，则不进行算法处理
			tFeatureInfo[i].uCompsStatus = 0;
			tFeatureInfo[i].iDelta_pwm = 0;
			tFeatureInfo[i].pumping_time = 0;
		}
		/* processing left and right data */
		PressureDataPreprocess(fPressureData[i], &tFilterInfo[i], &tDataBuffer[i], &tFeatureInfo[i]);
		PressureDataFeature(&tPVInfo[i], &tWaveInfo[i], &tDataBuffer[i], &tFeatureInfo[i], &tDeviceInfo[i]);
		WearDetectionDetermination(&tFeatureInfo[i], &tDeviceInfo[i]);
		/* update current index */
		/*tDataBuffer[i].uCurrPos = BufferCircleNextIndex(tDataBuffer[i].uCurrPos, 1, WIN_SIZE);*/
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
void GetWearDetectionResult(unsigned char *uWearStatus)
{
	uWearStatus[0] = tFeatureInfo[0].uWearStatus;
	uWearStatus[1] = tFeatureInfo[1].uWearStatus;
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
	iDelta_pwm[0] = tFeatureInfo[0].iDelta_pwm;
	iDelta_pwm[1] = tFeatureInfo[1].iDelta_pwm;
}
//#endif
/*
void GetDebugInfo(SRESULT_INFO *result_info)
{
	for (unsigned i = 0; i < CHANNEL_NUM; i++)
	{
		result_info[i].uWearStatus = tFeatureInfo[i].uWearStatus;
		result_info[i].uCompsStatus = tFeatureInfo[i].uCompsStatus;
		result_info[i].iDelta_pwm = tFeatureInfo[i].iDelta_pwm;
		result_info[i].fMinData = tFeatureInfo[i].fMinData;
		result_info[i].fThreshold = tFeatureInfo[i].fThreshold;
		result_info[i].fLoadedThreshold = tFeatureInfo[i].fLoadedThreshold;
		result_info[i].fMinDataBuffMin = tFeatureInfo[i].fMinDataBuffMin;
		result_info[i].uNoiseLevel = tFeatureInfo[i].uNoiseLevel;
		result_info[i].uWaveSpikes = tFeatureInfo[i].uWaveSpikes;
		result_info[i].uNormalWaveform = tFeatureInfo[i].uNormalWaveform;
		result_info[i].fMinDataBuff[0] = tFeatureInfo[i].fMinDataBuff[0];
		result_info[i].fMinDataBuff[1] = tFeatureInfo[i].fMinDataBuff[1];
		result_info[i].fMinDataBuff[2] = tFeatureInfo[i].fMinDataBuff[2];
		result_info[i].fMinDataBuff[3] = tFeatureInfo[i].fMinDataBuff[3];
		result_info[i].fMinDataBuff[4] = tFeatureInfo[i].fMinDataBuff[4];
		result_info[i].fMinDataBuff[5] = tFeatureInfo[i].fMinDataBuff[5];
	}
}*/