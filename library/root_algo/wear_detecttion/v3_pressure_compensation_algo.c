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
#include "v3_pressure_compensation_algo.h"
#define DEBUG		0

// global data
SDEVICE_INFO		tDeviceInfo[2];
SDATA_BUFF			tDataBuffer[2];
SPUMPFEATURE_INFO	tFeatureInfo[2];
SGLOBAL_INFO		tGlobalInfo;
/*************************************************************************************************************************************************
* 函数名称：V3PressureCompensation Initialization
* 作    者：sahar.hu
* 创建日期：2026.03.26
* 功能说明：初始化
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void V3ProAlgoInit(aglo_air_pressure_data_t sPressureData)
{
	// 把sPressureData的数据拆解出来
	tGlobalInfo.currtimestamp = 0;
	tGlobalInfo.prevtimestamp = 0;
	tGlobalInfo.do_work = 0;

	for (unsigned int j = 0; j < CHANNEL_NUM; j++)
	{
		tDeviceInfo[j].uMode = sPressureData.pump_state[j].mode;
		tDeviceInfo[j].uLevel = sPressureData.pump_state[j].level - 1;
		if (tDeviceInfo[j].uMode == MODE_EXPRESSION)
			tDeviceInfo[j].run_time = EXPRESSION_WORK_TIME[tDeviceInfo[j].uLevel];
		else if (tDeviceInfo[j].uMode == MODE_STIMULATION)
			tDeviceInfo[j].run_time = STIMULATION_WORK_TIME[tDeviceInfo[j].uLevel];
		tDeviceInfo[j].uTargetPressure = sPressureData.pump_state[j].target_pressure;
		tFeatureInfo[j].remain_time = 0;
		tFeatureInfo[j].uState = 0;
		tFeatureInfo[j].uCompsStatus = 0;
		if (tDeviceInfo[j].uMode == MODE_EXPRESSION)
		{
			tFeatureInfo[j].fSlope = expression_K_slope[tDeviceInfo[j].uLevel];
			tFeatureInfo[j].fIntercept = expression_intercep[tDeviceInfo[j].uLevel];
		}
	}
}

/*************************************************************************************************************************************************
* 函数名称：V3PressureCompensation Mode reset
* 作    者：sahar.hu
* 创建日期：2026.03.26
* 功能说明：重置
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void V3ProModeReset(algo_pump_state_t pump_state, unsigned char channel_idx)
{
	// 把sPressureData的数据拆解出来
	tDeviceInfo[channel_idx].uMode = pump_state.mode;
	tDeviceInfo[channel_idx].uLevel = pump_state.level - 1;
	if (tDeviceInfo[channel_idx].uMode == MODE_EXPRESSION)
		tDeviceInfo[channel_idx].run_time = EXPRESSION_WORK_TIME[tDeviceInfo[channel_idx].uLevel];
	else if (tDeviceInfo[channel_idx].uMode == MODE_STIMULATION)
		tDeviceInfo[channel_idx].run_time = STIMULATION_WORK_TIME[tDeviceInfo[channel_idx].uLevel];
	tDeviceInfo[channel_idx].uTargetPressure = pump_state.target_pressure;
	tFeatureInfo[channel_idx].pumping_time = 0;
	tFeatureInfo[channel_idx].remain_time = 0;
	tFeatureInfo[channel_idx].uState = 0;
	tFeatureInfo[channel_idx].uCompsStatus = 0;
	if (tDeviceInfo[channel_idx].uMode == MODE_EXPRESSION)
	{
		tFeatureInfo[channel_idx].fSlope = expression_K_slope[tDeviceInfo[channel_idx].uLevel];
		tFeatureInfo[channel_idx].fIntercept = expression_intercep[tDeviceInfo[channel_idx].uLevel];
	}
}

/*************************************************************************************************************************************************
* 函数名称：V3ProBufferCircleNextIndex -- buffer index
* 作    者：sahar.hu
* 创建日期：2026.03.26
* 功能说明：下一个索引号
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
unsigned int V3ProBufferCircleNextIndex(unsigned int uPos, unsigned int uIndent, unsigned int uBufferLen)
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
* 函数名称：V3ProPressureDataPreprocess
* 作    者：sahar.hu
* 创建日期：2026.03.26
* 功能说明：to buffer
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void V3ProPressureDataPreprocess(aglo_air_pressure_data_t sPressureData, unsigned char uchannelIdx)
{
	// 更新时间戳
	tGlobalInfo.currtimestamp = sPressureData.timestamp;

	// 如果发生模式/档位变更，重置
	if (sPressureData.pump_state[uchannelIdx].level - 1 != tDeviceInfo[uchannelIdx].uLevel ||
		sPressureData.pump_state[uchannelIdx].mode != tDeviceInfo[uchannelIdx].uMode)
	{
		V3ProModeReset(sPressureData.pump_state[uchannelIdx], uchannelIdx);
	}
	tDataBuffer[uchannelIdx].fDataBuff[tDataBuffer[uchannelIdx].uCurrPos] = sPressureData.air_pressure[uchannelIdx];
	//tFeatureInfo->remain_time[uchannelIdx] = sPressureData.timestamp;
}

/*************************************************************************************************************************************************
* 函数名称：PressureCompensationAnalysis
* 作    者：sahar.hu
* 创建日期：2026.03.26
* 功能说明：重置
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void PressureCompensationAnalysis(float fDataBuff, SPUMPFEATURE_INFO*tFeatureInfoTemp, SDEVICE_INFO *tDeviceInfoTemp)
{
	ALGO_LOG_INFO("run_time: %d, prev: %d, curr: %d", tDeviceInfoTemp->run_time, tGlobalInfo.prevtimestamp, tGlobalInfo.currtimestamp);
	if (tFeatureInfoTemp->uCompsStatus != 0)
	{
		tFeatureInfoTemp->uCompsStatus == 0;
		tFeatureInfoTemp->iDelta_pwm = 0;
	}
	// 判断当前是否接近结束泵工作ms级运算
	else if (tDeviceInfoTemp->run_time - (tGlobalInfo.currtimestamp - tGlobalInfo.prevtimestamp) <  100 && tGlobalInfo.currtimestamp - tGlobalInfo.prevtimestamp > 0)	{
		//float fPredPressure = tFeatureInfoTemp->fSlope * fDataBuff + tFeatureInfoTemp->fIntercept;
		float fPredPressure = fDataBuff + 2;
#if DEBUG == 1
		//printf("%d, %f\n", tFeatureInfoTemp->pumping_time, fPredPressure);
#endif
		// 预测值有问题时，不进行处理
		//float fPredPressure = fDataBuff + 2;
		// 当前已超过目标负压最大值，则直接反馈补偿
		//if ((fPredPressure + tDeviceInfoTemp->uTargetPressure) < (tDeviceInfoTemp->run_time - (tGlobalInfo.currtimestamp - tGlobalInfo.prevtimestamp))*.05)
		if (fDataBuff < -tDeviceInfoTemp->uTargetPressure)
		{
			tFeatureInfoTemp->uCompsStatus = 1;
			// 需要反馈iDelta_pwm (超出目标负压，反馈减小pwm）
			tFeatureInfoTemp->iDelta_pwm = -100;		//  - (fPredPressure - tDeviceInfoTemp->uTargetPressure) * 10
		}	
		//else if ((tDeviceInfoTemp->uTargetPressure + fPredPressure) > (tDeviceInfoTemp->run_time - (tGlobalInfo.currtimestamp - tGlobalInfo.prevtimestamp))*.1)
		else if (fDataBuff > -tDeviceInfoTemp->uTargetPressure)
		{
			tFeatureInfoTemp->uCompsStatus = 2;
			// 需要反馈iDelta_pwm (低于目标负压，反馈减小pwm）
			tFeatureInfoTemp->iDelta_pwm = 100;			//  + (fPredPressure - tDeviceInfoTemp->uTargetPressure) * 10
		}
		else
		{
			tFeatureInfoTemp->uCompsStatus = 0;
			tFeatureInfoTemp->iDelta_pwm = 0;
		}
	}
}

/*************************************************************************************************************************************************
* 函数名称：Pressure Compensation Main
* 作    者：sahar.hu
* 创建日期：2026.03.26
* 功能说明：重置
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void V3ProAlgoMain(aglo_air_pressure_data_t sPressureData)
{
#if DEBUG == 1
	// processing left and right data
	// for debug, process one channel
	unsigned int i = 0;
	if ((sPressureData.pump_state[i].state == SUCTION_STATUS_GENERATE) || (sPressureData.pump_state[i].state == SUCTION_STATUS_HOLD))	//  
	{
		tFeatureInfo[i].pumping_time += 1;
		if (tFeatureInfo[i].pumping_time == 1)
		{
			tGlobalInfo.prevtimestamp = sPressureData.timestamp;
		}
		V3ProPressureDataPreprocess(sPressureData, i);
		PressureCompensationAnalysis(tDataBuffer[i].fDataBuff[tDataBuffer[i].uCurrPos], &tFeatureInfo[i], &tDeviceInfo[i]);
		// update current index
		tDataBuffer[i].uCurrPos = V3ProBufferCircleNextIndex(tDataBuffer[i].uCurrPos, 1, WIN_SIZE);
	}
	else
	{
		// 如果不在泵奶期间，则不进行算法处理
		tFeatureInfo[i].uCompsStatus = 0;
		tFeatureInfo[i].iDelta_pwm = 0;
		tFeatureInfo[i].pumping_time = 0;
	}

#else

	for (unsigned int i = 0; i < CHANNEL_NUM; i++)
	{
		if (((sPressureData.pump_state[i].state == SUCTION_STATUS_GENERATE) || (sPressureData.pump_state[i].state == SUCTION_STATUS_HOLD)))
		{
			tFeatureInfo[i].pumping_time += 1;
			if (tFeatureInfo[i].pumping_time == 1)
			{
				tGlobalInfo.prevtimestamp = sPressureData.timestamp;
			}
			V3ProPressureDataPreprocess(sPressureData, i);
			PressureCompensationAnalysis(tDataBuffer[i].fDataBuff[tDataBuffer[i].uCurrPos], &tFeatureInfo[i], &tDeviceInfo[i]);
			// update current index
			tDataBuffer[i].uCurrPos = V3ProBufferCircleNextIndex(tDataBuffer[i].uCurrPos, 1, WIN_SIZE);
		}
		else
		{
			// 如果不在泵奶期间，则不进行算法处理
			tFeatureInfo[i].uCompsStatus = 0;
			tFeatureInfo[i].iDelta_pwm = 0;
			tFeatureInfo[i].pumping_time = 0;
		}
	}
#endif /* DEBUG == 1 */
}

/*************************************************************************************************************************************************
* 函数名称：GetCompensationStatus
* 作    者：sahar.hu
* 创建日期：2026.03.26
* 功能说明：重置
* 输入参数：无
* 输出参数：无
* 返 回 值：无
*************************************************************************************************************************************************
*/
void GetCompensationResult(unsigned char* uCompsStatus, int* iDeltaPWM)
{
	uCompsStatus[0] = tFeatureInfo[0].uCompsStatus;
	uCompsStatus[1] = tFeatureInfo[1].uCompsStatus;
	iDeltaPWM[0] = tFeatureInfo[0].iDelta_pwm;
	iDeltaPWM[1] = tFeatureInfo[1].iDelta_pwm;

}