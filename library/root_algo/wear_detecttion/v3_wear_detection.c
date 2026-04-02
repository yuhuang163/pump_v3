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
#if defined(ALGO_TYPE_V3_WEAR_DETECTION)
#include "v3_wear_detection.h"


// Thresholds for each mode and level
float EXPRESSION_THRESHOLD[] = {-12.06, -12.465, -13.655, -15.77, -16.815, -18.39, -22.165, -26.885, -32.46, -36.585, -39.84, -42.305, -44.645, -46.74, -48.83};
float STIMULATION_THRESHOLD[] = { -3, -4, -4, -8, -10, -5, -5, -5, -6, -10, -15, -16, -18, -20, -23 };
float MIXED_THRESHOLD[] = {-12.01, -12.45, -13.785, -15.75, -17.04, -18.375, -22.52, -27.19, -32.69,-36.795, -40.145, -42.725, -45.32, -47.04, -48.93};


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
	// Device info
	tDeviceInfo[CHANNEL_0].uMode = uModeLeft;
	tDeviceInfo[CHANNEL_0].uLevel = uLevelLeft;
	tDeviceInfo[CHANNEL_1].uMode = uModeRight;
	tDeviceInfo[CHANNEL_1].uLevel = ulevelRight;

	for (int j = 0; j < CHANNEL_NUM; j++)
	{
		// pv info
		for (int i = 0; i < 100; i++)
		{
			// PVInfo initiation
			tPVInfo[j].uPos[i] = 0;
			tPVInfo[j].fValue[i] = 0;
			tPVInfo[j].uType[i] = 0;
			// WaveInfo initiation
			tWaveInfo[j].fLeftAmp[i] = 0;
			tWaveInfo[j].fRightAmp[i] = 0;
			tWaveInfo[j].fValue[i] = 0;
			tWaveInfo[j].uPos[i] = 0;
			tWaveInfo[j].uType[i] = 0;
			tWaveInfo[j].uwidth[i] = 0;

		}
		// data buffer
		for (int i = 0; i < WIN_SIZE; i++)
		{
			tDataBuffer[j].fDataBuff[i] = 0;
			tDataBuffer[j].uCurrPos = 0;
		}
		tDataBuffer[j].fDataMax = -1;
		tDataBuffer[j].fDataMin = 0;
		tDataBuffer[j].uFirstTime = 1;

		// features
		tFeatureInfo[j].uUpdateFlag = 0;
		tFeatureInfo[j].fMinData = 0;
		tFeatureInfo[j].uNoiseLevel = 0;
		tFeatureInfo[j].uWaveSpikes = 0;
		tFeatureInfo[j].uPVIndex = 0;
		tFeatureInfo[j].uWearStatus = 0;
		tFeatureInfo[j].uNormalWaveform = 0;

		switch (tDeviceInfo[j].uMode)
		{
		case MODE_EXPRESSION:
			tFeatureInfo[j].fThreshold = EXPRESSION_THRESHOLD[tDeviceInfo[j].uLevel - 1];
		case MODE_MIXED:
			tFeatureInfo[j].fThreshold = MIXED_THRESHOLD[tDeviceInfo[j].uLevel - 1];
		case MODE_STIMULATION:
			tFeatureInfo[j].fThreshold = STIMULATION_THRESHOLD[tDeviceInfo[j].uLevel - 1];
		}
		// filter info
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
		// PVInfo initiation
		tPVInfo[uChannelIdx].uPos[i] = 0;
		tPVInfo[uChannelIdx].fValue[i] = 0;
		tPVInfo[uChannelIdx].uType[i] = 0;
		// WaveInfo initiation
		tWaveInfo[uChannelIdx].fLeftAmp[i] = 0;
		tWaveInfo[uChannelIdx].fRightAmp[i] = 0;
		tWaveInfo[uChannelIdx].fValue[i] = 0;
		tWaveInfo[uChannelIdx].uPos[i] = 0;
		tWaveInfo[uChannelIdx].uType[i] = 0;
		tWaveInfo[uChannelIdx].uwidth[i] = 0;

	}
	// data buffer
	for (int i = 0; i < WIN_SIZE; i++)
	{
		tDataBuffer[uChannelIdx].fDataBuff[i] = 0;
	}
	tDataBuffer[uChannelIdx].uCurrPos = 0;
	tDataBuffer[uChannelIdx].fDataMax = -1;
	tDataBuffer[uChannelIdx].fDataMin = 0;
	tDataBuffer[uChannelIdx].uFirstTime = 1;

	// features
	tFeatureInfo[uChannelIdx].uUpdateFlag = 0;
	tFeatureInfo[uChannelIdx].fMinData = 0;
	tFeatureInfo[uChannelIdx].uNoiseLevel = 0;
	tFeatureInfo[uChannelIdx].uWaveSpikes = 0;
	tFeatureInfo[uChannelIdx].uPVIndex = 0;
	tFeatureInfo[uChannelIdx].uWearStatus = 0;
	tFeatureInfo[uChannelIdx].uNormalWaveform = 0;
	// Device info
	tDeviceInfo[uChannelIdx].uMode = uMode;
	tDeviceInfo[uChannelIdx].uLevel = uLevel;
	switch (uMode)
	{
	case MODE_EXPRESSION:
		tFeatureInfo[uChannelIdx].fThreshold = EXPRESSION_THRESHOLD[uLevel - 1];
	case MODE_MIXED:
		tFeatureInfo[uChannelIdx].fThreshold = MIXED_THRESHOLD[uLevel - 1];
	case MODE_STIMULATION:
		tFeatureInfo[uChannelIdx].fThreshold = STIMULATION_THRESHOLD[uLevel - 1];
	}
	// filter info
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
unsigned int BufferCircleRelativeIndexrCircle(unsigned int uPos, unsigned int uIdx, unsigned int uBufferLen, unsigned int uFirstTime)
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
	filtered_data = FilterPreprocess(fPumpData, tFilterInfoTemp);

	tDataBufferTemp->fDataBuff[tDataBufferTemp->uCurrPos] = filtered_data;
	if (tFeatureInfoTemp->uDataIdx == SAMPLE_RATE - 1)
	{
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
		tFeatureInfoTemp->uDataIdx ++;
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
			unsigned int uTempCurrPos = BufferCircleRelativeIndexrCircle(tDataBufferTemp->uCurrPos, i, WIN_SIZE, tDataBufferTemp->uFirstTime);
			unsigned int uTempNextPos = BufferCircleRelativeIndexrCircle(tDataBufferTemp->uCurrPos, i + 1, WIN_SIZE, tDataBufferTemp->uFirstTime);
			unsigned int uTempPrevPos = BufferCircleRelativeIndexrCircle(tDataBufferTemp->uCurrPos, i - 1, WIN_SIZE, tDataBufferTemp->uFirstTime);

			//printf("i: %d, currPos: %d, prev: %d, curr: %d, next: %d\n", i, tDataBuffer.uCurrPos, uTempPrevPos, uTempCurrPos, uTempNextPos);
			//printf("i: %d: %f %d\n", i, tDataBufferTemp->fDataBuff[uTempCurrPos], tDataBufferTemp->uCurrPos);
		
			// peak
			if ((tDataBufferTemp->fDataBuff[uTempPrevPos] <= tDataBufferTemp->fDataBuff[uTempCurrPos] && tDataBufferTemp->fDataBuff[uTempCurrPos] > tDataBufferTemp->fDataBuff[uTempNextPos]) ||
			(tDataBufferTemp->fDataBuff[uTempPrevPos] < tDataBufferTemp->fDataBuff[uTempCurrPos] && tDataBufferTemp->fDataBuff[uTempCurrPos] >= tDataBufferTemp->fDataBuff[uTempNextPos]))
			{
				// combine with the previous PV if it's flat
				if (tFeatureInfoTemp->uPVIndex >= 1)
				{
					if (tPVInfoTemp->fValue[tFeatureInfoTemp->uPVIndex - 1] == tDataBufferTemp->fDataBuff[uTempCurrPos] && tPVInfoTemp->uType[tFeatureInfoTemp->uPVIndex - 1] == 1)
					{
						tFeatureInfoTemp->uPVIndex -= 1;
					}
				}
				// update PVInfo
				tPVInfoTemp->uPos[tFeatureInfoTemp->uPVIndex] = i;
				tPVInfoTemp->fValue[tFeatureInfoTemp->uPVIndex] = tDataBufferTemp->fDataBuff[uTempCurrPos];
				tPVInfoTemp->uType[tFeatureInfoTemp->uPVIndex] = 1;
				tFeatureInfoTemp->uPVIndex ++;
			}

			// valley
			if ((tDataBufferTemp->fDataBuff[uTempPrevPos] >= tDataBufferTemp->fDataBuff[uTempCurrPos] && tDataBufferTemp->fDataBuff[uTempCurrPos] < tDataBufferTemp->fDataBuff[uTempNextPos]) ||
				(tDataBufferTemp->fDataBuff[uTempPrevPos] > tDataBufferTemp->fDataBuff[uTempCurrPos] && tDataBufferTemp->fDataBuff[uTempCurrPos] <= tDataBufferTemp->fDataBuff[uTempNextPos]))
			{
				// combine with the previous PV if it's flat
				if (tFeatureInfoTemp->uPVIndex >= 1)
				{
					if (tPVInfoTemp->fValue[tFeatureInfoTemp->uPVIndex - 1] == tDataBufferTemp->fDataBuff[uTempCurrPos] && tPVInfoTemp->uType[tFeatureInfoTemp->uPVIndex - 1] == 0)
					{
						tFeatureInfoTemp->uPVIndex -= 1;
					}
				}
				// update PVInfo
				tPVInfoTemp->uPos[tFeatureInfoTemp->uPVIndex] = i;
				tPVInfoTemp->fValue[tFeatureInfoTemp->uPVIndex] = tDataBufferTemp->fDataBuff[uTempCurrPos];
				tPVInfoTemp->uType[tFeatureInfoTemp->uPVIndex] = 0;
				tFeatureInfoTemp->uPVIndex++;
			}
		}
		// update the buffer status
		if (tDataBufferTemp->uFirstTime == 1 && tDataBufferTemp->uCurrPos == 89)
		{
			tDataBufferTemp->uFirstTime = 0;
		}
		// update the max and min data
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
	// peak and valley
	PeakValleyAnalysis(tPVInfoTemp, tWaveInfoTemp, tDataBufferTemp, tFeatureInfoTemp);
	// wave analysis
	unsigned int uWaveIndex = 0;
	if (tFeatureInfoTemp->uUpdateFlag == 1)
	{
		if (tFeatureInfoTemp->uPVIndex >= 2)
		{
			// check the peak/valley starting at the first PV
			unsigned int i = 0;
			if (tPVInfoTemp->uType[i] == 0 && tPVInfoTemp->uType[i + 1] == 1)
			{
				tWaveInfoTemp->uPos[uWaveIndex] = tPVInfoTemp->uPos[i];
				tWaveInfoTemp->fValue[uWaveIndex] = tPVInfoTemp->fValue[i];
				tWaveInfoTemp->uwidth[uWaveIndex] = tPVInfoTemp->uPos[i+1];		// it starts at "0"
				tWaveInfoTemp->uType[uWaveIndex] = 0;
				tWaveInfoTemp->fRightAmp[uWaveIndex] = tPVInfoTemp->fValue[i + 1] - tPVInfoTemp->fValue[i];
				tWaveInfoTemp->fLeftAmp[uWaveIndex] = fabs(tPVInfoTemp->fValue[i]);
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
				tWaveInfoTemp->uwidth[uWaveIndex] = tPVInfoTemp->uPos[i + 1];		// it starts at "0"
				tWaveInfoTemp->uType[uWaveIndex] = 1;
				tWaveInfoTemp->fRightAmp[uWaveIndex] = tPVInfoTemp->fValue[i] - tPVInfoTemp->fValue[i + 1];
				tWaveInfoTemp->fLeftAmp[uWaveIndex] = fabs(tPVInfoTemp->fValue[i]);
				uWaveIndex += 1;
			}
			if (tFeatureInfoTemp->uPVIndex >= 3)
			{
				for (i = 1; i < tFeatureInfoTemp->uPVIndex - 1; i++)
				{
					// check a valley wave
					if (tPVInfoTemp->uType[i - 1] == 1 && tPVInfoTemp->uType[i] == 0 && tPVInfoTemp->uType[i + 1] == 1)
					{
						tWaveInfoTemp->uPos[uWaveIndex] = tPVInfoTemp->uPos[i];
						tWaveInfoTemp->fValue[uWaveIndex] = tPVInfoTemp->fValue[i];
						tWaveInfoTemp->uwidth[uWaveIndex] = tPVInfoTemp->uPos[i + 1] - tPVInfoTemp->uPos[i - 1];
						tWaveInfoTemp->uType[uWaveIndex] = 0;
						tWaveInfoTemp->fRightAmp[uWaveIndex] = tPVInfoTemp->fValue[i + 1] - tPVInfoTemp->fValue[i];
						tWaveInfoTemp->fLeftAmp[uWaveIndex] = tPVInfoTemp->fValue[i - 1] - tPVInfoTemp->fValue[i];
						if (tFeatureInfoTemp->fMinData > tWaveInfoTemp->fValue[uWaveIndex])
						{
							tFeatureInfoTemp->fMinData = tWaveInfoTemp->fValue[uWaveIndex];
						}
						uWaveIndex += 1;
					}
					// check a peak wave
					if (tPVInfoTemp->uType[i - 1] == 0 && tPVInfoTemp->uType[i] == 1 && tPVInfoTemp->uType[i + 1] == 0)
					{
						tWaveInfoTemp->uPos[uWaveIndex] = tPVInfoTemp->uPos[i];
						tWaveInfoTemp->fValue[uWaveIndex] = tPVInfoTemp->fValue[i];
						tWaveInfoTemp->uwidth[uWaveIndex] = tPVInfoTemp->uPos[i+1] - tPVInfoTemp->uPos[i - 1];
						tWaveInfoTemp->uType[uWaveIndex] = 1;
						tWaveInfoTemp->fRightAmp[uWaveIndex] = tPVInfoTemp->fValue[i] - tPVInfoTemp->fValue[i + 1];
						tWaveInfoTemp->fLeftAmp[uWaveIndex] = tPVInfoTemp->fValue[i] - tPVInfoTemp->fValue[i - 1];
						uWaveIndex += 1;
					}
				}
			}
		}
		// check wave spikes
		for (unsigned int i = 0; i < uWaveIndex; i++)
		{
			if (tWaveInfoTemp->uType[i] == 1 && fabs(tWaveInfoTemp->fLeftAmp[i] - tWaveInfoTemp->fRightAmp[i]) > 2 && fabs(tWaveInfoTemp->fLeftAmp[i]) > .1 && fabs(tWaveInfoTemp->fRightAmp[i]) > .1 &&
			( (2 <= tWaveInfoTemp->uwidth[i] && tWaveInfoTemp->uwidth[i] <= 15 && tFeatureInfoTemp->fThreshold < tWaveInfoTemp->fValue[i] && tWaveInfoTemp->fValue[i] < -.1) ||
			(15 <= tWaveInfoTemp->uwidth[i] && tWaveInfoTemp->uwidth[i] <= 50 && tWaveInfoTemp->fValue[i] < -.1)))
			{
				tFeatureInfoTemp->uWaveSpikes ++;
			}
			else if (tWaveInfoTemp->uType[i] == 0 &&
			(tWaveInfoTemp->fValue[i] < tFeatureInfoTemp->fThreshold && tDeviceInfoTemp->uMode == MODE_STIMULATION || (tDeviceInfoTemp->uMode == MODE_EXPRESSION && tDeviceInfoTemp->uLevel <= 7)) ||
				(tWaveInfoTemp->fValue[i] < -1 && abs(tFeatureInfoTemp->fThreshold - tWaveInfoTemp->fValue[i]) < 10 && (tDeviceInfoTemp->uMode == MODE_EXPRESSION && tDeviceInfoTemp->uLevel > 7 && tDeviceInfoTemp->uLevel < 10)) ||
				(tWaveInfoTemp->fValue[i] < -1 && abs(tFeatureInfoTemp->fThreshold - tWaveInfoTemp->fValue[i]) > 2 && (tDeviceInfoTemp->uMode == MODE_EXPRESSION && tDeviceInfoTemp->uLevel >=11))
				)
			{
				tFeatureInfoTemp->uNormalWaveform ++;
			}

		}
	}
	//printf("%d \n", tFeatureInfo.uWaveSpikes);
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
	
	if (tFeatureInfoTemp->uWearStatus == 0)
	{
		//
		if (tFeatureInfoTemp->uWaveSpikes <= 1 && tFeatureInfoTemp->uNoiseLevel == 0 && tFeatureInfoTemp->uNormalWaveform >=1)
		{
			tFeatureInfoTemp->uWearStatus = 1;
		}
	}
	else if (tFeatureInfoTemp->uWearStatus == 1)
	{
		if (((tFeatureInfoTemp->fThreshold > tFeatureInfoTemp->fMinData && tDeviceInfoTemp->uLevel > LEVEL_12 && tDeviceInfoTemp->uMode == MODE_EXPRESSION)
			|| (tFeatureInfoTemp->fThreshold < tFeatureInfoTemp->fMinData && tFeatureInfoTemp->fMinData < -.1 && tDeviceInfoTemp->uMode == MODE_STIMULATION)
			|| (STIMULATION_THRESHOLD[tDeviceInfoTemp->uLevel] < tFeatureInfoTemp->fMinData && tFeatureInfoTemp->fMinData < -.1 && tDeviceInfoTemp->uMode == MODE_MIXED && tDeviceInfoTemp->uLevel <= 7)) ||
			(tFeatureInfoTemp->fThreshold > tFeatureInfoTemp->fMinData && tDeviceInfoTemp->uLevel > 12 && tDeviceInfoTemp->uMode == MODE_EXPRESSION))
		{
			tFeatureInfoTemp->uWearStatus = 0;		// off
		}
		else if (tFeatureInfoTemp->fMinData > -.1 || \
			(tFeatureInfoTemp->fThreshold > tFeatureInfoTemp->fMinData && tDeviceInfoTemp->uLevel > 12 && tDeviceInfoTemp->uMode == MODE_EXPRESSION || tDeviceInfoTemp->uMode == MODE_MIXED))
		{
			tFeatureInfoTemp->uWearStatus = tFeatureInfoTemp->uPrevWearStatus;
		}
	}
	tFeatureInfoTemp->uPrevWearStatus = tFeatureInfoTemp->uWearStatus;
	
	
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
void V3WearDetectionMain(float *fPressureData, unsigned char uLevelLeft, unsigned char ulevelRight, unsigned char uModeLeft, unsigned char uModeRight)
{
	// reset if mode or level changed
	if (uLevelLeft != tDeviceInfo[CHANNEL_0].uLevel || uModeLeft != tDeviceInfo[CHANNEL_0].uMode)
	{
		V3WearDetectionModeReset(uLevelLeft, uModeLeft, CHANNEL_0);
	}
	if (ulevelRight != tDeviceInfo[CHANNEL_1].uLevel || uModeRight != tDeviceInfo[CHANNEL_1].uMode)
	{
		V3WearDetectionModeReset(ulevelRight, uModeRight, CHANNEL_1);
	}

	for (unsigned int i = 0; i < CHANNEL_NUM; i++)
	{
		// processing left and right data
		PressureDataPreprocess(fPressureData[i], &tFilterInfo[i], &tDataBuffer[i], &tFeatureInfo[i]);
		PressureDataFeature(&tPVInfo[i], &tWaveInfo[i], &tDataBuffer[i], &tFeatureInfo[i], &tDeviceInfo[i]);
		WearDetectionDetermination(&tFeatureInfo[i], &tDeviceInfo[i]);
		// update current index 
		tDataBuffer[i].uCurrPos = BufferCircleNextIndex(tDataBuffer[i].uCurrPos, 1, WIN_SIZE);
	}
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
#endif
