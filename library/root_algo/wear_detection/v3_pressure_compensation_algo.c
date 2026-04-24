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
// #include "v3_pressure_compensation_algo.h"
// #define DEBUG		0


// SGLOBAL_INFO			tGlobalInfo;
// SDATA_BUFF				tPumpDataBuffer[2];
// SPUMPFEATURE_INFO		tPumpFeatureInfo[2];
// SDEVICE_INFO			tPumpDeviceInfo[2];

// /*************************************************************************************************************************************************
// * 函数名称：V3PressureCompensation Initialization
// * 作    者：sahar.hu
// * 创建日期：2026.03.26
// * 功能说明：初始化
// * 输入参数：无
// * 输出参数：无
// * 返 回 值：无
// *************************************************************************************************************************************************
// */

// void V3ProAlgoInit(algo_air_pressure_data_t sPressureData)
// {
// 	/* 把sPressureData的数据拆解出来 */
// 	tGlobalInfo.currtimestamp = 0;
// 	tGlobalInfo.prevtimestamp = 0;
// 	tGlobalInfo.do_work = 0;

// 	for (unsigned int j = 0; j < CHANNEL_NUM; j++)
// 	{
// 		tPumpDeviceInfo[j].uMode = sPressureData.pump_state[j].mode;
// 		tPumpDeviceInfo[j].uLevel = sPressureData.pump_state[j].level - 1;
// 		if (tPumpDeviceInfo[j].uMode == MODE_EXPRESSION)
// 			tPumpDeviceInfo[j].run_time = EXPRESSION_WORK_TIME[tPumpDeviceInfo[j].uLevel];
// 		else if (tPumpDeviceInfo[j].uMode == MODE_STIMULATION)
// 			tPumpDeviceInfo[j].run_time = STIMULATION_WORK_TIME[tPumpDeviceInfo[j].uLevel];
// 		tPumpDeviceInfo[j].uTargetPressure = sPressureData.pump_state[j].target_pressure;
// 		tPumpDeviceInfo[j].uTargetPumpTime = sPressureData.pump_state[j].target_pump_time;
// 		tPumpFeatureInfo[j].pumping_time = 0;
// 		tPumpFeatureInfo[j].remain_time = 0;
// 		tPumpFeatureInfo[j].uState = 0;
// 		tPumpFeatureInfo[j].uCompsStatus = 0;
// 		tPumpFeatureInfo[j].iDelta_pwm = 0;
// 		if (tPumpDeviceInfo[j].uMode == MODE_EXPRESSION)
// 		{
// 			tPumpFeatureInfo[j].fSlope = expression_K_slope[tPumpDeviceInfo[j].uLevel];
// 			tPumpFeatureInfo[j].fIntercept = expression_intercep[tPumpDeviceInfo[j].uLevel];
// 		}
// 		else if (tPumpDeviceInfo[j].uMode == MODE_STIMULATION)
// 		{
// 			tPumpFeatureInfo[j].fSlope = expression_K_slope[tPumpDeviceInfo[j].uLevel];
// 			tPumpFeatureInfo[j].fIntercept = expression_intercep[tPumpDeviceInfo[j].uLevel];
// 		}

// 		/* 初始化pid控制信息 */
// 		tPumpFeatureInfo[j].tPidCtrl.uPrevPumpState = 0;
// 		tPumpFeatureInfo[j].tPidCtrl.uCurveValid = 0;
// 		tPumpFeatureInfo[j].tPidCtrl.uCycleSynced = 0;
// 		tPumpFeatureInfo[j].tPidCtrl.fTargetPressureRef = INVALID_TARGET_PRESSURE_REF;
// 		tPumpFeatureInfo[j].tPidCtrl.fCurrError = 0.0f;
// 		tPumpFeatureInfo[j].tPidCtrl.fPrevError = 0.0f;
// 		tPumpFeatureInfo[j].tPidCtrl.fIntegral = 0.0f;
// 		tPumpFeatureInfo[j].tPidCtrl.fPidOutput = 0.0f;
// 	}
// }

// /*************************************************************************************************************************************************
// * 函数名称：V3PressureCompensation Mode reset
// * 作    者：sahar.hu
// * 创建日期：2026.03.26
// * 功能说明：重置
// * 输入参数：无
// * 输出参数：无
// * 返 回 值：无
// *************************************************************************************************************************************************
// */

// void V3ProModeReset(algo_pump_state_t pump_state, unsigned char channel_idx)
// {
// 	/* 把sPressureData的数据拆解出来 */
// 	tPumpDeviceInfo[channel_idx].uMode = pump_state.mode;
// 	tPumpDeviceInfo[channel_idx].uLevel = pump_state.level - 1;
// 	if (tPumpDeviceInfo[channel_idx].uMode == MODE_EXPRESSION)
// 		tPumpDeviceInfo[channel_idx].run_time = EXPRESSION_WORK_TIME[tPumpDeviceInfo[channel_idx].uLevel];
// 	else if (tPumpDeviceInfo[channel_idx].uMode == MODE_STIMULATION)
// 		tPumpDeviceInfo[channel_idx].run_time = STIMULATION_WORK_TIME[tPumpDeviceInfo[channel_idx].uLevel];
// 	tPumpDeviceInfo[channel_idx].uTargetPressure = pump_state.target_pressure;
// 	tPumpDeviceInfo[channel_idx].uTargetPumpTime = pump_state.target_pump_time;
// 	tPumpFeatureInfo[channel_idx].pumping_time = 0;
// 	tPumpFeatureInfo[channel_idx].remain_time = 0;
// 	tPumpFeatureInfo[channel_idx].uState = 0;
// 	tPumpFeatureInfo[channel_idx].uCompsStatus = 0;
// 	if (tPumpDeviceInfo[channel_idx].uMode == MODE_EXPRESSION)
// 	{
// 		tPumpFeatureInfo[channel_idx].fSlope = expression_K_slope[tPumpDeviceInfo[channel_idx].uLevel];
// 		tPumpFeatureInfo[channel_idx].fIntercept = expression_intercep[tPumpDeviceInfo[channel_idx].uLevel];
// 	}

// 	/* 初始化pid控制信息 */
// 	tPumpFeatureInfo[channel_idx].tPidCtrl.uPrevPumpState = 0;
// 	tPumpFeatureInfo[channel_idx].tPidCtrl.uCurveValid = 0;
// 	tPumpFeatureInfo[channel_idx].tPidCtrl.uCycleSynced = 0;
// 	tPumpFeatureInfo[channel_idx].tPidCtrl.fTargetPressureRef = INVALID_TARGET_PRESSURE_REF;
// 	tPumpFeatureInfo[channel_idx].tPidCtrl.fCurrError = 0.0f;
// 	tPumpFeatureInfo[channel_idx].tPidCtrl.fPrevError = 0.0f;
// 	tPumpFeatureInfo[channel_idx].tPidCtrl.fIntegral = 0.0f;
// 	tPumpFeatureInfo[channel_idx].tPidCtrl.fPidOutput = 0.0f;
// }


// static unsigned int V3ProBufferCircleNextIndex(unsigned int uPos, unsigned int uIndent, unsigned int uBufferLen)
// {
// 	unsigned int uTempPos = 0;
// 	if (uPos == uBufferLen - uIndent)
// 	{
// 		uTempPos = 0;
// 	}
// 	else if (uPos + uIndent >= uBufferLen)
// 		uTempPos = uPos + uIndent - uBufferLen;
// 	else
// 		uTempPos = uPos + uIndent;
// 	return uTempPos;
// }


// void V3ProPressureDataPreprocess(algo_air_pressure_data_t sPressureData, unsigned char uchannelIdx)
// {
// 	/* 更新时间戳--实际时间戳分辨度不够*/
// 	tGlobalInfo.currtimestamp = sPressureData.timestamp;

// 	/* 如果发生模式 / 档位变更，重置*/
// 	if (sPressureData.pump_state[uchannelIdx].level - 1 != tPumpDeviceInfo[uchannelIdx].uLevel ||
// 		sPressureData.pump_state[uchannelIdx].mode != tPumpDeviceInfo[uchannelIdx].uMode)
// 	{
// 		V3ProModeReset(sPressureData.pump_state[uchannelIdx], uchannelIdx);
// 	}
// 	tPumpDataBuffer[uchannelIdx].fDataBuff[tPumpDataBuffer[uchannelIdx].uCurrPos] = sPressureData.air_pressure[uchannelIdx];

// 	/*tPumpFeatureInfo->remain_time[uchannelIdx] = sPressureData.timestamp;*/

// }


// /*************************************************************************************************************************************************
// * 函数名称：GetCurrentTargetPressure
// * 作    者：merlin.xu
// * 创建日期：2026.04.07
// * 功能说明：根据目标压力和目标 pumping_time 查找目标曲线，并返回当前 pumping_time 对应的目标压力
// * 输入参数：tFeatureInfoTemp   当前通道特征信息
// *           tDeviceInfoTemp    当前通道设备信息
// * 输出参数：无
// * 返 回 值：当前时刻目标压力；若未找到曲线或取值失败，则返回 -255.0f
// *************************************************************************************************************************************************
// */
// static float GetCurrentTargetPressure(SPUMPFEATURE_INFO *tFeatureInfoTemp,
//                                       SDEVICE_INFO *tDeviceInfoTemp)
// {
// 	const STARGET_CURVE_ITEM *ptCurve = 0;
// 	unsigned char uValid = 0;
// 	float fTargetRef = 0.0f;


// 	/* 按“目标压力 + 目标 pumping_time”双键查找对应目标曲线 */
// 	ptCurve = FindTargetCurve(tDeviceInfoTemp->uTargetPressure,
// 							  tDeviceInfoTemp->uTargetPumpTime);
	
// 	/* 若未找到目标曲线，则标记当前曲线无效 */
// 	if (ptCurve == 0)
// 	{
// 		tFeatureInfoTemp->tPidCtrl.uCurveValid = 0;
// 		tFeatureInfoTemp->tPidCtrl.fTargetPressureRef = INVALID_TARGET_PRESSURE_REF;
// 		return INVALID_TARGET_PRESSURE_REF;
// 	}

// 	/* 根据当前 pumping_time，从目标曲线上取当前时刻的目标压力 */
// 	fTargetRef = GetTargetPressureFromCurve(ptCurve,
// 											tFeatureInfoTemp->pumping_time,
// 											&uValid);

											
// 	/* 若本次取值失败，则标记当前曲线无效 */
// 	if (uValid == 0)
// 	{
// 		tFeatureInfoTemp->tPidCtrl.uCurveValid = 0;
// 		tFeatureInfoTemp->tPidCtrl.fTargetPressureRef = INVALID_TARGET_PRESSURE_REF;
// 		return INVALID_TARGET_PRESSURE_REF;
// 	}

// 	/* 保存当前曲线有效标志和目标压力参考值 */
// 	tFeatureInfoTemp->tPidCtrl.uCurveValid = 1;
// 	tFeatureInfoTemp->tPidCtrl.fTargetPressureRef = fTargetRef;

// 	return fTargetRef;
// }



// /*************************************************************************************************************************************************
// * 函数名称：PressureCompensationAnalysis
// * 作    者：sahar.hu
// * 创建日期：2026.03.26
// * 功能说明：重置
// * 输入参数：无
// * 输出参数：无
// * 返 回 值：无
// *************************************************************************************************************************************************
// */

// void PressureCompensationAnalysis(float fDataBuff, SPUMPFEATURE_INFO*tFeatureInfoTemp, SDEVICE_INFO *tDeviceInfoTemp)
// {
// 	if (tFeatureInfoTemp->uCompsStatus != 0)
// 	{
// 		tFeatureInfoTemp->uCompsStatus = 0;
// 		tFeatureInfoTemp->iDelta_pwm = 0;
// 	}
// 	/* 判断当前是否接近结束泵工作ms级运算 */
// 	else if (tDeviceInfoTemp->run_time - (tGlobalInfo.currtimestamp - tGlobalInfo.prevtimestamp) <  100 && tGlobalInfo.currtimestamp - tGlobalInfo.prevtimestamp > 0)	
// 	{
// 		/*float fPredPressure = tFeatureInfoTemp->fSlope * fDataBuff + tFeatureInfoTemp->fIntercept;*/
// 		float fPredPressure = fDataBuff + 2;
// #if DEBUG == 1
// 		/*printf("%d, %f\n", tFeatureInfoTemp->pumping_time, fPredPressure);*/
// #endif
// 		/* 预测值有问题时，不进行处理 */
// 		/*float fPredPressure = fDataBuff + 2;*/
// 		/* 当前已超过目标负压最大值，则直接反馈补偿 */
// 		/*if ((fPredPressure + tDeviceInfoTemp->uTargetPressure) < (tDeviceInfoTemp->run_time - (tGlobalInfo.currtimestamp - tGlobalInfo.prevtimestamp))*.05)*/
// 		if (fDataBuff < -tDeviceInfoTemp->uTargetPressure)
// 		{
// 			tFeatureInfoTemp->uCompsStatus = 1;
// 			/* 需要反馈iDelta_pwm (超出目标负压，反馈减小pwm） */
// 			tFeatureInfoTemp->iDelta_pwm = -100;		/*  - (fPredPressure - tDeviceInfoTemp->uTargetPressure) * 10 */
// 		}	
// 		/*else if ((tDeviceInfoTemp->uTargetPressure + fPredPressure) > (tDeviceInfoTemp->run_time - (tGlobalInfo.currtimestamp - tGlobalInfo.prevtimestamp))*.1)*/
// 		else if (fDataBuff > -tDeviceInfoTemp->uTargetPressure)
// 		{
// 			tFeatureInfoTemp->uCompsStatus = 2;
// 			/* 需要反馈iDelta_pwm (低于目标负压，反馈减小pwm） */
// 			tFeatureInfoTemp->iDelta_pwm = 100;			/*  + (fPredPressure - tDeviceInfoTemp->uTargetPressure) * 10 */
// 		}
// 		else
// 		{
// 			tFeatureInfoTemp->uCompsStatus = 0;
// 			tFeatureInfoTemp->iDelta_pwm = 0;
// 		}
// 	}
// }

// /*************************************************************************************************************************************************
// * 函数名称：PressureCompensationCurveAnalysis
// * 作    者：merlin.xu
// * 创建日期：2026.04.07
// * 功能说明：基于目标压力曲线的补偿分析
// * 输入参数：fDataBuff          当前压力值
// *           tFeatureInfoTemp   当前通道特征信息
// *           tDeviceInfoTemp    当前通道设备信息
// * 输出参数：无
// * 返 回 值：无
// *************************************************************************************************************************************************
// */
// static void PressureCompensationCurveAnalysis(float fDataBuff,
// 	SPUMPFEATURE_INFO* tFeatureInfoTemp,
// 	SDEVICE_INFO* tDeviceInfoTemp)
// {
// 	const STARGET_CURVE_ITEM* ptTargetCurve = NULL;
// 	float fTargetRef = INVALID_TARGET_PRESSURE_REF;
// 	float fError = 0.0f;
// 	float fOutput = 0.0f;
// 	float fKp = 0.0f;
//     float fKi = 0.0f;

// 	ptTargetCurve = GetCurrentTargetCurveItem(tFeatureInfoTemp, tDeviceInfoTemp);

// 	if (ptTargetCurve == NULL)
//     {
//         tFeatureInfoTemp->uCompsStatus = 0;
//         tFeatureInfoTemp->iDelta_pwm = 0;
//         tFeatureInfoTemp->tPidCtrl.fTargetPressureRef = INVALID_TARGET_PRESSURE_REF;
//         tFeatureInfoTemp->tPidCtrl.fCurrError = 0.0f;
//         tFeatureInfoTemp->tPidCtrl.fPrevError = 0.0f;
//         tFeatureInfoTemp->tPidCtrl.fIntegral = 0.0f;
//         tFeatureInfoTemp->tPidCtrl.fPidOutput = 0.0f;
//         return;
//     }

//     fKp = ptTargetCurve->fKp;
//     fKi = ptTargetCurve->fKi;

// 	/* 死区：误差很小时不调，避免抖动 */
// 	const float fDeadBand = 0.5f;

// 	/* 积分限幅，避免积分发散 */
// 	const float fIntegralMax = 200.0f;
// 	const float fIntegralMin = -200.0f;

// 	/* 输出限幅，先沿用当前 ±100 的量级 */
// 	const float fOutputMax = 200.0f;
// 	const float fOutputMin = -200.0f;

// 	/* 先获取当前时刻目标压力 */
// 	fTargetRef = GetCurrentTargetPressure(tFeatureInfoTemp, tDeviceInfoTemp);

// 	/* 若当前曲线无效，则不进行补偿处理 */
// 	if (tFeatureInfoTemp->tPidCtrl.uCurveValid == 0)
// 	{
// 		tFeatureInfoTemp->uCompsStatus = 0;
// 		tFeatureInfoTemp->iDelta_pwm = 0;
// 		tFeatureInfoTemp->tPidCtrl.fTargetPressureRef = INVALID_TARGET_PRESSURE_REF;
// 		tFeatureInfoTemp->tPidCtrl.fCurrError = 0.0f;
// 		tFeatureInfoTemp->tPidCtrl.fPrevError = 0.0f;
// 		tFeatureInfoTemp->tPidCtrl.fIntegral = 0.0f;
// 		tFeatureInfoTemp->tPidCtrl.fPidOutput = 0.0f;
// 		return;
// 	}

// 	/* 误差方向：
// 	 * 当前压力比目标更大（不够负）时，error > 0，需要增大 pwm
// 	 * 当前压力比目标更小（过负）时，error < 0，需要减小 pwm
// 	 */
// 	fError = fDataBuff - fTargetRef;
// 	tFeatureInfoTemp->tPidCtrl.fCurrError = fError;

// 	ALGO_LOG_INFO("Current target pressure: %f\n, Current measured pressure: %f, Error: %f", fTargetRef, fDataBuff, fError);

// 	/* 死区内不调节，同时不累计积分 */
// 	if ((-fDeadBand < fError) && (fError < fDeadBand))
// 	{
// 		tFeatureInfoTemp->uCompsStatus = 0;
// 		tFeatureInfoTemp->iDelta_pwm = 0;
// 		tFeatureInfoTemp->tPidCtrl.fPidOutput = 0.0f;
// 		tFeatureInfoTemp->tPidCtrl.fPrevError = fError;
// 		return;
// 	}

// 	/* 先用当前积分值计算一次输出 */
// 	fOutput = fKp * fError + fKi * tFeatureInfoTemp->tPidCtrl.fIntegral;

// 	/* 只有在“没有继续往饱和方向推”时，才允许积分继续累加 */
// 	if (!((fOutput >= fOutputMax && fError > 0.0f) ||
// 		  (fOutput <= fOutputMin && fError < 0.0f)))
// 	{
// 		tFeatureInfoTemp->tPidCtrl.fIntegral += fError;

// 		if (tFeatureInfoTemp->tPidCtrl.fIntegral > fIntegralMax)
// 		{
// 			tFeatureInfoTemp->tPidCtrl.fIntegral = fIntegralMax;
// 		}
// 		else if (tFeatureInfoTemp->tPidCtrl.fIntegral < fIntegralMin)
// 		{
// 			tFeatureInfoTemp->tPidCtrl.fIntegral = fIntegralMin;
// 		}
// 	}

// 	/* 用更新后的积分值重新计算最终输出 */
// 	fOutput = fKp * fError + fKi * tFeatureInfoTemp->tPidCtrl.fIntegral;

// 	/* 输出限幅 */
// 	if (fOutput > fOutputMax)
// 	{
// 		fOutput = fOutputMax;
// 	}
// 	else if (fOutput < fOutputMin)
// 	{
// 		fOutput = fOutputMin;
// 	}


// 	tFeatureInfoTemp->tPidCtrl.fPidOutput = fOutput;
// 	tFeatureInfoTemp->iDelta_pwm = (int)fOutput;

// 	/* 根据输出方向更新补偿状态 */
// 	if (tFeatureInfoTemp->iDelta_pwm > 0)
// 	{
// 		tFeatureInfoTemp->uCompsStatus = 2;
// 	}
// 	else if (tFeatureInfoTemp->iDelta_pwm < 0)
// 	{
// 		tFeatureInfoTemp->uCompsStatus = 1;
// 	}
// 	else
// 	{
// 		tFeatureInfoTemp->uCompsStatus = 0;
// 	}

// 	tFeatureInfoTemp->tPidCtrl.fPrevError = fError;
// }


// /*************************************************************************************************************************************************
// * 函数名称：Pressure Compensation Main
// * 作    者：sahar.hu
// * 创建日期：2026.03.26
// * 功能说明：重置
// * 输入参数：无
// * 输出参数：无
// * 返 回 值：无
// *************************************************************************************************************************************************
// */

// void V3ProAlgoMain(algo_air_pressure_data_t sPressureData)
// {
// 	unsigned int end_idx = CHANNEL_NUM;
// #if DEBUG == 1
// 	end_idx = 1;
// #endif
// 	/* 处理所有通道 */
// 	for (unsigned int i = 0; i < end_idx; i++)
// 	{
// 		unsigned char curr_state = sPressureData.pump_state[i].state;
// 		unsigned char prev_state = tPumpFeatureInfo[i].tPidCtrl.uPrevPumpState;
// 		unsigned char curr_active = 0;
// 		unsigned char prev_active = 0;
// 		tPumpDeviceInfo[i].uTargetPressure = sPressureData.pump_state[i].target_pressure;
//     	tPumpDeviceInfo[i].uTargetPumpTime = sPressureData.pump_state[i].target_pump_time;
// 		curr_active = ((curr_state == SUCTION_STATUS_GENERATE) || (curr_state == SUCTION_STATUS_HOLD)) ? 1u : 0u;
// 		prev_active = ((prev_state == SUCTION_STATUS_GENERATE) || (prev_state == SUCTION_STATUS_HOLD)) ? 1u : 0u;
// 		/* 只有先见过一次非泵奶状态，后面才能认为已经同步到周期边界 */
// 		if (tPumpFeatureInfo[i].tPidCtrl.uCycleSynced == 0u)
// 		{
// 			if (curr_active == 0u)
// 			{
// 				tPumpFeatureInfo[i].tPidCtrl.uCycleSynced = 1u;
// 			}

// 			tPumpFeatureInfo[i].uCompsStatus = 0;
// 			tPumpFeatureInfo[i].iDelta_pwm = 0;
// 			tPumpFeatureInfo[i].pumping_time = 0;
// 			tPumpFeatureInfo[i].tPidCtrl.fIntegral = 0.0f;
// 			tPumpFeatureInfo[i].tPidCtrl.fPrevError = 0.0f;
// 			tPumpFeatureInfo[i].tPidCtrl.fCurrError = 0.0f;
// 			tPumpFeatureInfo[i].tPidCtrl.fPidOutput = 0.0f;
// 			tPumpFeatureInfo[i].tPidCtrl.uCurveValid = 0;
// 			tPumpFeatureInfo[i].tPidCtrl.fTargetPressureRef = INVALID_TARGET_PRESSURE_REF;
// 			tPumpFeatureInfo[i].tPidCtrl.uPrevPumpState = curr_state;
// 			continue;
// 		}
// 		/* 如果从非泵奶状态切换到泵奶状态，则重置pid控制信息 */
// 		if ((prev_active == 0u) && (curr_active == 1u))
// 		{
// 			tPumpFeatureInfo[i].pumping_time = 0;
// 			tPumpFeatureInfo[i].tPidCtrl.fIntegral = 0.0f;
// 			tPumpFeatureInfo[i].tPidCtrl.fPrevError = 0.0f;
// 			tPumpFeatureInfo[i].tPidCtrl.fCurrError = 0.0f;
// 			tPumpFeatureInfo[i].tPidCtrl.fPidOutput = 0.0f;
// 			tPumpFeatureInfo[i].tPidCtrl.uCurveValid = 0;
// 			tPumpFeatureInfo[i].tPidCtrl.fTargetPressureRef = INVALID_TARGET_PRESSURE_REF;
// 			tGlobalInfo.prevtimestamp = sPressureData.timestamp;
// 		}
// 		/* 如果在泵奶期间，则进行算法处理 */
// 		if (curr_active == 1u)
// 		{
// 			if (tPumpFeatureInfo[i].pumping_time == 0)
// 			{
// 				tGlobalInfo.prevtimestamp = sPressureData.timestamp;
// 			}
// 			V3ProPressureDataPreprocess(sPressureData, i);
// 			//PressureCompensationAnalysis(tPumpDataBuffer[i].fDataBuff[tPumpDataBuffer[i].uCurrPos], &tPumpFeatureInfo[i], &tPumpDeviceInfo[i]);
// 			PressureCompensationCurveAnalysis(tPumpDataBuffer[i].fDataBuff[tPumpDataBuffer[i].uCurrPos], &tPumpFeatureInfo[i], &tPumpDeviceInfo[i]);
// 			/* update current index */
// 			tPumpDataBuffer[i].uCurrPos = V3ProBufferCircleNextIndex(tPumpDataBuffer[i].uCurrPos, 1, WIN_SIZE);
// 			tPumpFeatureInfo[i].pumping_time += 1;
// 		}
// 		else
// 		{
// 			/* 如果不在泵奶期间，则不进行算法处理 */
// 			tPumpFeatureInfo[i].uCompsStatus = 0;
// 			tPumpFeatureInfo[i].iDelta_pwm = 0;
// 			tPumpFeatureInfo[i].pumping_time = 0;
// 			/* 重置pid控制信息 */
// 			tPumpFeatureInfo[i].tPidCtrl.fIntegral = 0.0f;
// 			tPumpFeatureInfo[i].tPidCtrl.fPrevError = 0.0f;
// 			tPumpFeatureInfo[i].tPidCtrl.fCurrError = 0.0f;
// 			tPumpFeatureInfo[i].tPidCtrl.fPidOutput = 0.0f;
// 			tPumpFeatureInfo[i].tPidCtrl.uCurveValid = 0;
// 			tPumpFeatureInfo[i].tPidCtrl.fTargetPressureRef = INVALID_TARGET_PRESSURE_REF;

// 		}
// 		tPumpFeatureInfo[i].tPidCtrl.uPrevPumpState = curr_state;
// 	}
// }

// /*************************************************************************************************************************************************
// * 函数名称：GetCompensationStatus
// * 作    者：sahar.hu
// * 创建日期：2026.03.26
// * 功能说明：重置
// * 输入参数：无
// * 输出参数：无
// * 返 回 值：无
// *************************************************************************************************************************************************
// */

// void GetCompensationResult(unsigned char* uCompsStatus, int* iDeltaPWM)
// {
// 	uCompsStatus[0] = tPumpFeatureInfo[0].uCompsStatus;
// 	uCompsStatus[1] = tPumpFeatureInfo[1].uCompsStatus;
// 	iDeltaPWM[0] = tPumpFeatureInfo[0].iDelta_pwm;
// 	iDeltaPWM[1] = tPumpFeatureInfo[1].iDelta_pwm;

// }

// /* get debug info */

// void GetPidDebugInfo(unsigned char channel_idx, SPID_DEBUG_INFO* ptDebugInfo)
// {
//     if (ptDebugInfo == 0 || channel_idx >= CHANNEL_NUM)
//     {
//         return;
//     }

//     ptDebugInfo->pumping_time = tPumpFeatureInfo[channel_idx].pumping_time;
//     ptDebugInfo->curve_valid = tPumpFeatureInfo[channel_idx].tPidCtrl.uCurveValid;
//     ptDebugInfo->target_ref = tPumpFeatureInfo[channel_idx].tPidCtrl.fTargetPressureRef;
//     ptDebugInfo->error = tPumpFeatureInfo[channel_idx].tPidCtrl.fCurrError;
//     ptDebugInfo->integral = tPumpFeatureInfo[channel_idx].tPidCtrl.fIntegral;
//     ptDebugInfo->pid_output = tPumpFeatureInfo[channel_idx].tPidCtrl.fPidOutput;
//     ptDebugInfo->delta_pwm = tPumpFeatureInfo[channel_idx].iDelta_pwm;
//     ptDebugInfo->comps_status = tPumpFeatureInfo[channel_idx].uCompsStatus;
// }
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


static SGLOBAL_INFO			tGlobalInfo;
static SDATA_BUFF			tDataBuffer[2];
static SPUMPFEATURE_INFO	tPumpFeatureInfo[2];
SDEVICE_INFO				tDeviceInfo[2];

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

void V3ProAlgoInit(algo_air_pressure_data_t sPressureData)
{
	/* 把sPressureData的数据拆解出来 */
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
		tPumpFeatureInfo[j].remain_time = 0;
		tPumpFeatureInfo[j].uState = 0;
		tPumpFeatureInfo[j].uCompsStatus = 0;
		if (tDeviceInfo[j].uMode == MODE_EXPRESSION)
		{
			tPumpFeatureInfo[j].fSlope = expression_K_slope[tDeviceInfo[j].uLevel];
			tPumpFeatureInfo[j].fIntercept = expression_intercep[tDeviceInfo[j].uLevel];
		}
		else if (tDeviceInfo[j].uMode == MODE_STIMULATION)
		{
			tPumpFeatureInfo[j].fSlope = expression_K_slope[tDeviceInfo[j].uLevel];
			tPumpFeatureInfo[j].fIntercept = expression_intercep[tDeviceInfo[j].uLevel];
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
	/* 把sPressureData的数据拆解出来 */
	tDeviceInfo[channel_idx].uMode = pump_state.mode;
	tDeviceInfo[channel_idx].uLevel = pump_state.level - 1;
	if (tDeviceInfo[channel_idx].uMode == MODE_EXPRESSION)
		tDeviceInfo[channel_idx].run_time = EXPRESSION_WORK_TIME[tDeviceInfo[channel_idx].uLevel];
	else if (tDeviceInfo[channel_idx].uMode == MODE_STIMULATION)
		tDeviceInfo[channel_idx].run_time = STIMULATION_WORK_TIME[tDeviceInfo[channel_idx].uLevel];
	tDeviceInfo[channel_idx].uTargetPressure = pump_state.target_pressure;
	tPumpFeatureInfo[channel_idx].pumping_time = 0;
	tPumpFeatureInfo[channel_idx].remain_time = 0;
	tPumpFeatureInfo[channel_idx].uState = 0;
	tPumpFeatureInfo[channel_idx].uCompsStatus = 0;
	if (tDeviceInfo[channel_idx].uMode == MODE_EXPRESSION)
	{
		tPumpFeatureInfo[channel_idx].fSlope = expression_K_slope[tDeviceInfo[channel_idx].uLevel];
		tPumpFeatureInfo[channel_idx].fIntercept = expression_intercep[tDeviceInfo[channel_idx].uLevel];
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

void V3ProPressureDataPreprocess(algo_air_pressure_data_t sPressureData, unsigned char uchannelIdx)
{
	/* 更新时间戳--实际时间戳分辨度不够*/
	tGlobalInfo.currtimestamp = sPressureData.timestamp;

	/* 如果发生模式 / 档位变更，重置*/
	if (sPressureData.pump_state[uchannelIdx].level - 1 != tDeviceInfo[uchannelIdx].uLevel ||
		sPressureData.pump_state[uchannelIdx].mode != tDeviceInfo[uchannelIdx].uMode)
	{
		V3ProModeReset(sPressureData.pump_state[uchannelIdx], uchannelIdx);
	}
	tDataBuffer[uchannelIdx].fDataBuff[tDataBuffer[uchannelIdx].uCurrPos] = sPressureData.air_pressure[uchannelIdx];

	/*tFeatureInfo->remain_time[uchannelIdx] = sPressureData.timestamp;*/

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

void PressureCompensationAnalysis(float fDataBuff, SPUMPFEATURE_INFO *tFeatureInfoTemp, SDEVICE_INFO *tDeviceInfoTemp)
{
	if (tFeatureInfoTemp->uCompsStatus != 0)
	{
		tFeatureInfoTemp->uCompsStatus == 0;
		tFeatureInfoTemp->iDelta_pwm = 0;
	}
	/* 判断当前是否接近结束泵工作ms级运算 */
	else if (tDeviceInfoTemp->run_time - (tGlobalInfo.currtimestamp - tGlobalInfo.prevtimestamp) <  100 && tGlobalInfo.currtimestamp - tGlobalInfo.prevtimestamp > 0)	
	{
		/*float fPredPressure = tFeatureInfoTemp->fSlope * fDataBuff + tFeatureInfoTemp->fIntercept;*/
		float fPredPressure = fDataBuff + 2;
#if DEBUG == 1
		/*printf("%d, %f\n", tFeatureInfoTemp->pumping_time, fPredPressure);*/
#endif
		/* 预测值有问题时，不进行处理 */
		/*float fPredPressure = fDataBuff + 2;*/
		/* 当前已超过目标负压最大值，则直接反馈补偿 */
		/*if ((fPredPressure + tDeviceInfoTemp->uTargetPressure) < (tDeviceInfoTemp->run_time - (tGlobalInfo.currtimestamp - tGlobalInfo.prevtimestamp))*.05)*/
		if (fDataBuff < -tDeviceInfoTemp->uTargetPressure)
		{
			tFeatureInfoTemp->uCompsStatus = 1;
			/* 需要反馈iDelta_pwm (超出目标负压，反馈减小pwm） */
			tFeatureInfoTemp->iDelta_pwm = -100;		/*  - (fPredPressure - tDeviceInfoTemp->uTargetPressure) * 10 */
		}	
		/*else if ((tDeviceInfoTemp->uTargetPressure + fPredPressure) > (tDeviceInfoTemp->run_time - (tGlobalInfo.currtimestamp - tGlobalInfo.prevtimestamp))*.1)*/
		else if (fDataBuff > -tDeviceInfoTemp->uTargetPressure)
		{
			tFeatureInfoTemp->uCompsStatus = 2;
			/* 需要反馈iDelta_pwm (低于目标负压，反馈减小pwm） */
			tFeatureInfoTemp->iDelta_pwm = 100;			/*  + (fPredPressure - tDeviceInfoTemp->uTargetPressure) * 10 */
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

void V3ProAlgoMain(algo_air_pressure_data_t sPressureData)
{
	
#if DEBUG == 1
	/* processing left and right data */
	/* for debug, process one channel */
	unsigned int i = 0;
	if ((sPressureData.pump_state[i].state == SUCTION_STATUS_GENERATE) || (sPressureData.pump_state[i].state == SUCTION_STATUS_HOLD))	/*  */
	{
		tPumpFeatureInfo[i].pumping_time += 1;
		if (tPumpFeatureInfo[i].pumping_time == 1)
		{
			tGlobalInfo.prevtimestamp = sPressureData.timestamp;
		}
		V3ProPressureDataPreprocess(sPressureData, i);
		PressureCompensationAnalysis(tDataBuffer[i].fDataBuff[tDataBuffer[i].uCurrPos], &tPumpFeatureInfo[i], &tDeviceInfo[i]);
		/* update current index */
		tDataBuffer[i].uCurrPos = V3ProBufferCircleNextIndex(tDataBuffer[i].uCurrPos, 1, WIN_SIZE);
	}
	else
	{
		/* 如果不在泵奶期间，则不进行算法处理 */
		tPumpFeatureInfo[i].uCompsStatus = 0;
		tPumpFeatureInfo[i].iDelta_pwm = 0;
		tPumpFeatureInfo[i].pumping_time = 0;
	}

#else
	
	for (unsigned int i = 0; i < CHANNEL_NUM; i++)
	{
		if (((sPressureData.pump_state[i].state == SUCTION_STATUS_GENERATE) || (sPressureData.pump_state[i].state == SUCTION_STATUS_HOLD)))
		{
			tPumpFeatureInfo[i].pumping_time += 1;
			if (tPumpFeatureInfo[i].pumping_time == 1)
			{
				tGlobalInfo.prevtimestamp = sPressureData.timestamp;
			}
			V3ProPressureDataPreprocess(sPressureData, i);
			PressureCompensationAnalysis(tDataBuffer[i].fDataBuff[tDataBuffer[i].uCurrPos], &tPumpFeatureInfo[i], &tDeviceInfo[i]);
			/* update current index */
			tDataBuffer[i].uCurrPos = V3ProBufferCircleNextIndex(tDataBuffer[i].uCurrPos, 1, WIN_SIZE);
		}
		else
		{
			/* 如果不在泵奶期间，则不进行算法处理 */
			tPumpFeatureInfo[i].uCompsStatus = 0;
			tPumpFeatureInfo[i].iDelta_pwm = 0;
			tPumpFeatureInfo[i].pumping_time = 0;
		}
	}
#endif /* DEBUG == 1 */

}

