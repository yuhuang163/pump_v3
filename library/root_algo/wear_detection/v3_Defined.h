#ifndef __V3_DEFINED_H__
#define __V3_DEFINED_H__
#include "algo_fusion_lib.h"

#ifndef abs
#define abs(x)			((x) < 0 ? -(x) : (x))
#endif /* !abs */

#ifndef fabs
#define fabs(x)			((x) < 0.0f ? -(x) : (x))
#endif /* !abs */

#ifndef CLAMP
#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))
#endif

/* define pressure algorithm version */
#define V3_PRESSURE_ALGO_VERSION_STR    "2.0.0.0"

/* hardware parameters */
#define SAMPLE_RATE			(50)
#define WIN_SIZE			(50*3)  /* (1 second) */
#define CHANNEL_NUM			(2)		/* 2 channels - left and right */
#define CHANNEL_0			(0)		/* 0: left */
#define CHANNEL_1			(1)		/* 1: right */

/* define modes */
#define MODE_STIMULATION	(0)
#define MODE_EXPRESSION		(1)
#define MODE_MIXED			(2)

/* define levels */
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

#define INVALID_TARGET_PRESSURE_REF    (-255.0f) /* 无效的目标压力参考值 */
#define RAW_PUMP_POST_TAIL_POINTS 3 /* 每段结束后多采的原始压力点数（含 active->inactive 当前点） */
#define RAW_PUMP_ACTIVE_CAP       128
#define RAW_PUMP_SEG_MAX          (RAW_PUMP_ACTIVE_CAP + RAW_PUMP_POST_TAIL_POINTS)
#define LONGYI_OR_SHIGUANG  2 //2: 龙一-spectra; 1: 龙一, 0: 时光

/* target pressure for each level - v2.13 */
/* TODO SPECTRA -- 20260801 - LONGYI*/
static float EXPRESSION_THRESHOLD[] = { -5, -10, -15, -18, -21,
											-24, -26, -28, -30, -32,
											-34, -36, -38, -39, -40 };
static float STIMULATION_THRESHOLD[] = { -6, -12,	-18, -23, -26};

  
/* mixed_threshold not used*/
static float MIXED_THRESHOLD[] = { -4, -5, -6, -7.5, -11, -11, -16, -20,
									   -25, -30, -35, -35, -40, -40, -40 };

static float EXPRESSION_WORK_TIME[] = { 300, 430, 520, 540, 559, 
										638, 639, 636, 648, 643, 
										641, 634, 644, 636, 638};

static float STIMULATION_WORK_TIME[] = { 202, 202, 270, 320, 360};

/* unloaded threshold for wear detection */
/* TODO */
static float UNLOADED_EXPRESSION_THRESHOLD[] = { -2.5, -3.0, -10, -14.0, -16.0, 
												-24.7, -29.2, -33.0, -36.0, -40.0, 
												-45.0, -47.5, -52.5, -52.5, -55.0};
static float UNLOADED_STIMULATION_THRESHOLD[] = {-3.5, -5.0, -12.0, -18.0, -24.2};

/* mixed_threshold not used*/
static float UNLOADED_MIXED_THRESHOLD[] = { -4, -5, -6, -7.5, -11, -11, -16, -18,
									   -18, -12, -35, -35, -40, -40, -40 };

/* realtime pwm adjustment --- not for this function*/
static float expression_K_slope[] = { 0.9614, 0.3633, 0.5492, 0.6025, 0.0771,
									0.0463, 0.1959, 0.2455, 0.0770, -0.0935,
									 -0.0129, 0.0370, 0.0958, -0.0150, -0.0257 };

static float expression_intercep[] = { 0.8553, -4.6521, -4.7871, -5.0214, -17.6085,
										-20.6986, -18.9113, -18.6675, -26.4728, -35.2280,
										-35.4251, -35.3878, -34.7144, -41.3431, -42.9398 };


/* from software, 吸力状态 */


typedef enum {
	SUCTION_STATUS_IDLE = 0,
	SUCTION_STATUS_GENERATE,    /// 生成	泵开始工作
	SUCTION_STATUS_HOLD,        /// 保持	泵保持工作
	SUCTION_STATUS_RELEASE,     /// 解除	泵结束工作
	SUCTION_STATUS_SLEEP,       /// 休眠
} suction_status_e;

//typedef struct _PUMP_STATE
//{
//	unsigned char		level;       /* 0-15 */
//	unsigned char		mode;        /* 0:刺激模式 1:吸乳模式 2:混合模式 3:个性化模式 4:PP追奶模式 5:2+8奶量提升模式 6:开奶模式 */
//	unsigned char		state;       /* 吸力状态 suction_status_e */
//	unsigned char		pump_duty;        /* 泵奶力度 */
//	unsigned char		valve_duty;       /* 吸乳器电磁阀门力度 */
//	unsigned char		target_pressure;  /* 目标压力 */
//	unsigned short 		target_pump_time; /// 泵工作目标时间，单位ms
//	unsigned short		run_time;         /* 运行时间，单位ms */
//}algo_pump_state_t;



/* 气压数据 */
//typedef struct _AIR_PRESSURE
//{
//	unsigned char		do_work;     /* 是否正在泵奶 */
//	unsigned int		timestamp;
//	float				air_pressure[CHANNEL_NUM];
//	algo_pump_state_t	pump_state[CHANNEL_NUM];
//}algo_air_pressure_data_t;
/* ---------------------------------------------------------------- */


/* for algorithm */
/* pump_pressure_compensation */


/* 漏气分级 */
typedef enum {
	LEAKAGE_0 = 0,
	LEAKAGE_PUMP,
	LEAKAGE_SMALL,
	LEAKAGE_MEDIUM,
	LEAKAGE_LARGE,
} leakage_status_e;



typedef struct _GLOBAL_INFO
{
	unsigned char	do_work;			/* 是否正在泵奶 */
	unsigned int	currtimestamp;		/* 当前时间戳 */
	unsigned int	prevtimestamp;		/* 泵初始时间戳 */
}SGLOBAL_INFO;


typedef struct _DEVICE_INFO
{
	unsigned short	uMode;
	unsigned short	uLevel;
	unsigned short	run_time;			// 运行时间，单位ms
	unsigned char	uTargetPressure;	// 正值
	unsigned short  uTargetPumpTime;	/* 吸力保持时间，单位ms */
}SDEVICE_INFO;


typedef struct _DATA_BUFF_
{
	float			fDataBuff[WIN_SIZE];
	unsigned int	uCurrPos;		/* current pos */
}SDATA_BUFF;

typedef struct _PUMP_FEATURE_INFO
{
	unsigned char	uState;				/* 吸力状态 */
	unsigned short	pumping_time;		/* 泵开始工作后，开始计时 */
	unsigned short	remain_time;		/* 距离结束泵奶时间 */
	unsigned char	uCompsStatus;		/* 输出的补偿结果 */
	float			fSlope;
	float			fIntercept;
	int				iDelta_pwm;			/* 返回delta_pwm -10000~10000 */
}SPUMPFEATURE_INFO;

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

/* tightness_detection */
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
	unsigned int	uDataIdx;		/* data index */
	unsigned short	uUpdateFlag;
	float			fThreshold;
	float			fLoadedThreshold;
	unsigned char	uWearStatus;	/* result */
	unsigned short	uPump_flag;
	unsigned char	pumping_time;
	unsigned char	uPumpState;
	unsigned char	uCompsStatus;
	int				iDelta_pwm;
	int 			iPwmOffset;   	/* 累计补偿量 */
	float 			fCompErrSum;      /* PI积分项 */
	float 			fCompPrevErr;     /* 上一周期误差，后续需要D时可直接用 */
	unsigned char	uWearOnCount;
	unsigned char	uWearOffCount;
	unsigned char	uPrevWearStatus;
	unsigned int	uNormalWaveform;

	float			fRawSegMinPressure;
	float			fRawSegThresholdAtMinPressure;
	float			fRawSegRunningMin; /* 当前段内 raw 最小（仅用于求 fRawSegMinPressure） */
	float			fFilteredSegMinPressure; /* 与 fRawSegMinPressure 同泵段、同拍滤波的段内最小 */
	float			fFilteredSegRunningMin; /* 段内滤波最小（内部） */
	unsigned char	uPrevPumpState;
	unsigned char	uPrevActive;
	unsigned char	uCurrActive;
	unsigned char	uRawTailRemain; /* 新段时置 POST_TAIL；段末起每写入一个尾随点减 1，到 0 表示本段采集结束 */
	unsigned short	uRawSegLen;
	unsigned char	uWearDetSegMinCommitted;    /* RawSegCommitMinIfAny 提交段 min 后置 1，PressureDataFeature 后消费 */
	float			fLeakValleyBuff[6];         /* 泵周期 raw 段最小，左移 6 槽，不复用 fMinDataBuff */
	unsigned char	uLeakValleyCount;           /* 0~6 */
	unsigned char	uLeakStatus;                /* 0 未佩戴；1 已佩戴无漏；2 小漏；3 大漏 */
	unsigned char	uPrevLeakStatus;                /* 0 未佩戴；1 已佩戴无漏；2 小漏；3 大漏 */
	unsigned short  uWearDetectTime;			/* 检测佩戴状态的时长 */
	unsigned short  uLeakDetectTime;			/* 检测完全漏气状态的时长 */
}SFEATURE_INFO;

typedef struct _DATA_BUFFER_
{
	float			fDataBuff[WIN_SIZE];
	float fLoadedThresholdData[WIN_SIZE];
	float			fDataMax;
	float			fDataMin;
	unsigned int	uCurrPos;		/* current pos */
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

/*

*/
#endif
