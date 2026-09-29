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
#define V3_PRESSURE_ALGO_VERSION_STR    "3.0.2.7"

/* hardware parameters */
#define SAMPLE_RATE			(50)
#define WIN_SIZE			(50*3)  /* (1 second) */
#define CHANNEL_NUM			(2)		/* 2 channels - left and right */
#define CHANNEL_0			(0)		/* 0: left */
#define CHANNEL_1			(1)		/* 1: right */
#define TEMPLATEWAVE_NUM	(4)

/* define modes */
#define MODE_STIMULATION	(0)
#define MODE_EXPRESSION		(1)
#define MODE_MIXED			(2)

#define PUMP_WAVE_NUM       (6)
#define PUMP_WAVE_LEN       (40)

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

/* define levels */
typedef enum _Set_Levels {
	LEVEL_1 = 0,
	LEVEL_2,
	LEVEL_3,
	LEVEL_4,
	LEVEL_5,
	LEVEL_6,
	LEVEL_7,
	LEVEL_8,
	LEVEL_9,
	LEVEL_10,
	LEVEL_11,
	LEVEL_12,
	LEVEL_13,
	LEVEL_14,
	LEVEL_15,
	LEVEL_Max,
}SETLEVELS;


typedef enum {
	SUCTION_STATUS_IDLE = 0,
	SUCTION_STATUS_GENERATE,    /// 生成
	SUCTION_STATUS_HOLD,        /// 保持
	SUCTION_STATUS_RELEASE,     /// 解除
	SUCTION_STATUS_SLEEP,       /// 休眠
	SUCTION_STATUS_UPDATE_POLICY,      /// 更新策略
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


/* 佩戴状态 */
typedef enum {
	WEAR_STATE_0 = 0,
	WEAR_STATE_ON,
	WEAR_STATE_OFF,
	WEAR_STATE_OVERTIME,
} wear_status_e;



/* 漏气分级 */
typedef enum {
	LEAKAGE_0 = 0,
	LEAKAGE_PUMP,
	LEAKAGE_SMALL,
	LEAKAGE_LARGE,
} leakage_status_e;



typedef struct _GLOBAL_INFO
{
	unsigned char	do_work;			/* 是否正在泵奶 */
	unsigned char	uLogIndex;			/* 模式 */
	unsigned int	uCoolDownTime;		/* 冷却时间 */
	unsigned int	currtimestamp;		/* 当前时间戳 */
	unsigned int	prevtimestamp;		/* 泵初始时间戳 */
}SGLOBAL_INFO;


typedef struct _DEVICE_INFO
{
	unsigned short	uMode;
	unsigned short	uLevel;
	unsigned short  uPumpDuty;
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
	unsigned char	target_pressure;
	unsigned short	target_pump_time;
	float			target_pump_pressure;
	float			target_load_pressure;
	unsigned short	curve_len;
	const float*	p1_curve;
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
	unsigned int	uDataIdx;		/* data index */
	unsigned short	uUpdateFlag;
	unsigned int	uPVIndex;
	unsigned int	uWaveSpikes;
	unsigned int	uNoiseLevel;
	unsigned int	uNormalWaveform;
	float			fThreshold;
	float			fLoadedThreshold;

	float			fMinData;
	float			fMinDataBuff[6];
	float			fLoadedThresholdBuff[6];
	float			fMinDataBuffMin;

	unsigned short	uPump_flag;
	unsigned char	pumping_time;
	unsigned char	uPumpState;
	unsigned char	uPumpStateLast;
	unsigned char	uCompsStatus;

	unsigned char	uWearStatus;		/* result */
	unsigned char   uWearStatusOutput;	/* 对外输出 */
	unsigned char	uDetectOverTime;	/* 检测超时 */

	int				iDelta_pwm;
	int 			iPwmOffset;              	/* 累计补偿量 */
	float 			fCompErrSum;                /* PI积分项 */
	float 			fCompPrevErr;               /* 上一周期误差，后续需要D时可直接用 */
	unsigned char	uWearOnCount;
	unsigned char	uWearOffCount;
	unsigned char	uPrevWearStatus;
	
	unsigned char	uPrevPumpState;
	unsigned char	uPrevActive;
	unsigned char	uCurrActive;
	unsigned char	uRawTailRemain;				/* 新段时置 POST_TAIL；段末起每写入一个尾随点减 1，到 0 表示本段采集结束 */
	unsigned char	uWearDetSegMinCommitted;    /* RawSegCommitMinIfAny 提交段 min 后置 1，PressureDataFeature 后消费 */
	unsigned short	uRawSegLen;
	float			fRawSegMinPressure;
	float			fRawSegThresholdAtMinPressure;
	float			fRawSegRunningMin; /* 当前段内 raw 最小（仅用于求 fRawSegMinPressure） */
	float			fFilteredSegMinPressure; /* 与 fRawSegMinPressure 同泵段、同拍滤波的段内最小 */
	float			fFilteredSegRunningMin; /* 段内滤波最小（内部） */

	unsigned char   uPump1delay;
	unsigned short	uPump1LastTime;
	float           fRawValleyDownBase;
	float           fRawValleyUpBase;
	float           fRawWearValleyBase;
	float			fRawValleyBuff[11];         /* 泵周期 raw 段最小，左移 6 槽，不复用 fMinDataBuff */

	unsigned char   uPumpWaveLastIndex;
	unsigned char   uPumpWaveIndex;
	unsigned char   uPumpWaveNumber;
	unsigned char   uPumpWaveWidth;
	int 			iPwmPumWaveBase;              	/* 累计补偿量 */
	int 			iPwmPumWaveFix;              	/* 累计补偿量 */
	unsigned char   uPumpFiltWidth[PUMP_WAVE_NUM];

	unsigned char   uPumpFiltUpCount;
	unsigned char   uPumpFiltDownCount;
	float           fPumpFiltChange;
	float           fPumpFiltWave[PUMP_WAVE_NUM][PUMP_WAVE_LEN + 1];
	float           fPumpFiltWaveFeature[3][PUMP_WAVE_LEN + 1];

	unsigned char	uLeakValleyCount;           /* 0~6 */
	unsigned char	uLeakStatus;                /* 0 未佩戴；1 已佩戴无漏；2 小漏；3 大漏 */
	unsigned char	uPrevLeakStatus;            /* 0 未佩戴；1 已佩戴无漏；2 小漏；3 大漏 */
	unsigned short  uWearDetectTime;			/* 检测佩戴状态的时长 */
	
	unsigned short  uLeakSwitchTime;			/* 切换状态时长 */
	unsigned short  uLeakAdjustTime;			/* 切换状态时长 */
	unsigned short  uLeakSwitchDelayTime;	    /* 切换状态时长 */

	unsigned short	uDataBaselineCnt[3];		/* 休眠状态下, 基线平台>1200Pa (大漏), >700Pa (小漏), <100Pa (正常) 计数 */
	unsigned short  uMinPressureCnt;			/* 最小负压低于目标负压 */
	float			fPumpState4Pressure[11];	/* 分析泵休眠期趋势 其中：[0]为当前循环，[9]为前10个循环，[10]为当前循环的最小负压 */
	unsigned short	uNoiseLevelBuff[11] ;			/* 分析噪声水平突变 其中：[0]为当前循环，[9]为前10个循环，[10]为当前循环的最小负压 */
	unsigned short  uUnloadedINcmpstStatusCnt;	/* 持续出现大漏特征的计时 --- 在补偿阶段，可能会出现大漏/脱落的情况，这个时候需要判断 */
}SFEATURE_INFO;

typedef struct _DATA_BUFFER_
{
	float			fDataBuff[WIN_SIZE];
	float			fLoadedThresholdData[WIN_SIZE];
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

#endif
