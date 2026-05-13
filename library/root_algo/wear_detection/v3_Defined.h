#ifndef __V3_DEFINED_H__
#define __V3_DEFINED_H__
#include "include/algo_fusion_lib.h"

#ifndef abs
#define abs(x)			((x) < 0 ? -(x) : (x))
#endif // !abs

#ifndef fabs
#define fabs(x)			((x) < 0.0f ? -(x) : (x))
#endif // !abs

#ifndef CLAMP
#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))
#endif

/* define pressure algorithm version */
#define V3_PRESSURE_ALGO_VERSION_STR    "1.0.0.1"

// hardware parameters
#define SAMPLE_RATE			(50)
#define WIN_SIZE			(50*3)  //(1 second)
#define CHANNEL_NUM			(2)		// 2 channels - left and right
#define CHANNEL_0			(0)		// 0: left
#define CHANNEL_1			(1)		// 1: right

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

#define INVALID_TARGET_PRESSURE_REF	(-255.0f) /* 无效的目标压力参考值 */
#define RAW_PUMP_POST_TAIL_POINTS	3 /* 每段结束后多采的原始压力点数（含 active->inactive 当前点） */
#define RAW_PUMP_ACTIVE_CAP			128 /* 原始泵压力段最大缓存长度 */
#define RAW_PUMP_SEG_MAX			(RAW_PUMP_ACTIVE_CAP + RAW_PUMP_POST_TAIL_POINTS)

// target pressure for each level - v2.13
static float EXPRESSION_THRESHOLD[] = { -6, -9.5, -12.5, -15, -17.5,
											-19.5, -21.5, -23.5, -25.5, -27.5,
											-29.5, -31.3, -32.9, -34.5, -36 };
static float STIMULATION_THRESHOLD[] = { -6, -7.4,	-8.8, -10.2, -11.5,
											-12.8, -14.1, -15.3, -16.5, -17.7,
											-18.8, -19.9, -21, -22, -23 };
static float MIXED_THRESHOLD[] = { -4, -5, -6, -7.5, -11, -11, -16, -20,
									   -25, -30, -35, -35, -40, -40, -40 };

static float EXPRESSION_WORK_TIME[] = { 370, 404, 425, 440, 468,
										  475, 490, 540, 548, 560,
										  600, 620, 650, 685, 710 };

static float STIMULATION_WORK_TIME[] = { 270, 245, 270, 270, 260,
										  303, 290, 302, 285, 305,
										  285, 305, 313, 340, 322 };

// unloaded threshold for wear detection 
static float UNLOADED_EXPRESSION_THRESHOLD[] = { -4, -5, -6, -7.5, -11, -11, -16, -18,
									   -18, -12, -35, -35, -40, -40, -40 };
static float UNLOADED_STIMULATION_THRESHOLD[] = { -3, -3, -4, -5, -5.5, -5.5, -7, -8,
									   -8, -9, -11, -12, -14, -15, -19 };
static float UNLOADED_MIXED_THRESHOLD[] = { -4, -5, -6, -7.5, -11, -11, -16, -18,
									   -18, -12, -35, -35, -40, -40, -40 };

static float expression_K_slope[] = { 0.9614, 0.3633, 0.5492, 0.6025, 0.0771,
									0.0463, 0.1959, 0.2455, 0.0770, -0.0935,
									 -0.0129, 0.0370, 0.0958, -0.0150, -0.0257 };

static float expression_intercep[] = { 0.8553, -4.6521, -4.7871, -5.0214, -17.6085,
										-20.6986, -18.9113, -18.6675, -26.4728, -35.2280,
										-35.4251, -35.3878, -34.7144, -41.3431, -42.9398 };

///////////////////////////////////////////////////////from software
/// 吸力状态
typedef enum {
	SUCTION_STATUS_IDLE = 0,
	SUCTION_STATUS_GENERATE,    /// 生成	泵开始工作
	SUCTION_STATUS_HOLD,        /// 保持	泵保持工作
	SUCTION_STATUS_RELEASE,     /// 解除	泵结束工作
	SUCTION_STATUS_SLEEP,       /// 休眠
} suction_status_e;

///////////////////////////////////////////////////////


/////////////////////////////////////////////////////// for algorithm
// pump_pressure_compensation

typedef struct _GLOBAL_INFO
{
	unsigned char	do_work;			// 是否正在泵奶
	unsigned int	currtimestamp;		// 当前时间戳
	unsigned int	prevtimestamp;		// 上一时间戳
}SGLOBAL_INFO;

typedef struct _DEVICE_INFO
{
	unsigned short	uMode;
	unsigned short	uLevel;
	unsigned short	run_time;			// 运行时间，单位ms
	unsigned char	uTargetPressure;	// 正值
	unsigned short  uTargetPumpTime;	/* 吸力保持时间，单位ms */
}SDEVICE_INFO;

/* 目标压力曲线项 */
typedef struct _TARGET_CURVE_ITEM
{
    unsigned char target_pressure;
    unsigned short target_pump_time;
	float target_pump_pressure;
	float target_load_pressure;
    unsigned short curve_len;
    const float *p1_curve;
} STARGET_CURVE_ITEM;

typedef struct _DATA_BUFF_
{
	float			fDataBuff[WIN_SIZE];
	unsigned int	uCurrPos;		// current pos
}SDATA_BUFF;

/* pid control info */
typedef struct _PID_CTRL_INFO
{
    unsigned char uPrevPumpState;   /* 上一次泵状态 */
    unsigned char uCurveValid;      /* 当前是否找到目标曲线 */
	unsigned char uCycleSynced;   /* 是否已经同步到完整周期边界 */

    float fTargetPressureRef;       /* 当前时刻目标压力 */
    float fCurrError;               /* 当前误差 */
    float fPrevError;               /* 上一次误差 */
    float fIntegral;                /* 积分项 */
    float fPidOutput;               /* PID原始输出 */
} SPID_CTRL_INFO;

/* pid debug info */
typedef struct _PID_DEBUG_INFO
{
    unsigned short pumping_time;
    unsigned char curve_valid;
    float target_ref;
    float error;
    float integral;
    float pid_output;
    int delta_pwm;
    unsigned char comps_status;
} SPID_DEBUG_INFO;

typedef struct _PUMP_FEATURE_INFO
{
	unsigned char	uState;				// 吸力状态
	unsigned short	pumping_time;		// 泵开始工作后，开始计时
	unsigned short	remain_time;		// 距离结束泵奶时间
	unsigned char	uCompsStatus;		// 输出的补偿结果
	float			fSlope;
	float			fIntercept;
	int				iDelta_pwm;			// 返回delta_pwm	-10000~10000
	SPID_CTRL_INFO tPidCtrl;			/* pid控制信息 */
}SPUMPFEATURE_INFO;


// tightness_detection
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
	float			fLeftValue[100];
	float			fRightValue[100];
	unsigned int	uLeftWidth[100];
	unsigned int	uRightWidth[100];
	unsigned int	uwidth[100];
}SWAVE_INFO;


typedef struct _FEATURE_INFO_
{
	unsigned int	uPVIndex;
	unsigned int	uWaveSpikes;
	float			fMinData;
	float			fMinDataBuff[6];
	float			fLoadedThresholdBuff[6];
	float			fMinDataBuffMin;
	unsigned int	uNoiseLevel;
	unsigned int	uDataIdx;		// data index
	unsigned short	uUpdateFlag;
	float			fThreshold;
	float			fLoadedThreshold;
	unsigned char	uPumpFlag;
	unsigned char	pumping_time;
	unsigned char	uPumpState;
	unsigned char	uWearStatus;	// result
	unsigned char	uCompsStatus;
	int 			iDelta_pwm;			// 返回delta_pwm	-10000~10000
	int 			iPwmOffset;   	/* 累计补偿量 */
	float 			fCompErrSum;      /* PI积分项 */
	float 			fCompPrevErr;     /* 上一周期误差，后续需要D时可直接用 */
	unsigned char	uWearOnCount;
	unsigned char	uWearOffCount;
	unsigned char	uPrevWearStatus;
	unsigned int	uNormalWaveform;

	float			fRawSegMinPressure;
	float			fRawSegThresholdAtMinPressure;
	unsigned char	uPrevPumpState;
	unsigned char	uPrevActive;
	unsigned char	uCurrActive;
	unsigned char	uRawTailRemain; /* 新段时置 POST_TAIL；段末起每写入一个尾随点减 1，到 0 表示本段采集结束 */
	unsigned short	uRawSegLen;
	float			fRawPumpSeg[RAW_PUMP_SEG_MAX];
}SFEATURE_INFO;

typedef struct _DATA_BUFFER_
{
	float			fDataBuff[WIN_SIZE];
	float fLoadedThresholdData[WIN_SIZE];
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


/////////////////////////////////////////////////////////////////////
#endif