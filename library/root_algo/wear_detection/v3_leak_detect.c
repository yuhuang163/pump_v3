/*****************************************************************************************************************
文 件 名：v3_leak_detect.c
功能说明：泵周期 raw 段最小负压漏气分级（Python v2 规则移植）
流程概要：
  WearDetectionDetermination 在段 min 提交后调用 LeakDetectUpdate
  → 左移写入 6 槽谷值
  → 未佩戴：状态置 0（缓冲不清，仍累计谷值）
  → 混合模式：已戴固定为 1，不分级
  → 吸乳/刺激：LeakSpec* 填本档门限 → LeakClassify 先大漏后小漏
  → 瞬时 0/1/2 映射为 1/2/3，只升不降
编码：0 未戴；1 已戴无漏；2 小漏；3 大漏。不修改 uWearStatus。
tgt 为档位表 EXPRESSION_THRESHOLD / STIMULATION_THRESHOLD（负压）。
更浅 = 数值更大（接近 0）；更深 = 数值更小。
******************************************************************************************************************/
#include "v3_leak_detect.h"

/* --------------------------------------------------------------------------
 * LEAK_HAS_*：本档启用哪些门限（按位或进 SLEAK_SPEC.uHas）
 * 无 Python 的 None，未置位则不使用对应 T_*。
 * -------------------------------------------------------------------------- */
#define LEAK_HAS_LARGE          (1u)     /* T_large：最近 3 点大漏 */
#define LEAK_HAS_LARGE_SHALLOW  (2u)     /* T_large_shallow：L6+ 过浅大漏 */
#define LEAK_HAS_MIN6           (4u)     /* T_min6：6 点最小过深 → 大漏 C */
#define LEAK_HAS_CMEAN          (8u)     /* T_c_mean：大漏 C 的均值约束 */
#define LEAK_HAS_T6             (16u)    /* T_6：6 点全部过深 → 大漏 D */
#define LEAK_HAS_SMALL          (32u)    /* T_small：最近 3 点小漏（偏浅） */
#define LEAK_HAS_SMEAN          (64u)    /* T_small_mean：6 点均值小漏 */
#define LEAK_HAS_SMIN6          (128u)   /* T_small_min6：与均值一起的 min6 小漏 */
#define LEAK_HAS_LMEAN          (256u)   /* T_large_mean：刺激 L4–L5 大漏均值 */
#define LEAK_HAS_LMAX           (512u)   /* T_large_max：刺激 L4–L5 大漏 max6 */
#define LEAK_HAS_BAND           (1024u)  /* T_band_lo/hi：刺激 L1 近目标小漏带 */

/* T_large 三点比较方向：浅档三点须都 > 门限；深档 T_large 三点须都 < 门限 */
#define LEAK_FAM_SHALLOW  0
#define LEAK_FAM_DEEP     1

/* --------------------------------------------------------------------------
 * 吸乳：相对 tgt 的 kPa 裕度。T = tgt ± 下列常数。
 * -------------------------------------------------------------------------- */
#define MEAN6_DEEP_MARGIN          0.5f   /* 大漏 D：mean6 还须 < T_6 - 0.5 */
#define SHALLOW_LARGE_ABOVE_TARGET 3.2f   /* L2–L5 大漏：T_large = tgt+3.2 */
#define SHALLOW_SMALL_ABOVE_TARGET 2.4f   /* L2–L5 小漏：T_small = tgt+2.4 */
#define SHALLOW_L1_SMALL_ABOVE     1.40f  /* 仅 L1 小漏，贴 3# 密封 */
#define SHALLOW_L1_LARGE_ABOVE     2.20f  /* 仅 L1 大漏 */
#define DEEP_LAST3_BELOW_TARGET    2.5f   /* L6–L9 过深大漏：T_large = tgt-2.5；L10–15 用 -4 */
#define DEEP_MIN6_BELOW_TARGET     3.5f   /* L6–L12：T_min6 = tgt-3.5 */
#define SHALLOW_MEAN_ABOVE_TARGET  3.0f   /* L6–L10 小漏均值：tgt+3.0 */
#define SHALLOW_MIN6_ABOVE_TARGET  2.6f   /* L6–L10 小漏 min6：tgt+2.6 */
#define SHALLOW_LAST3_ABOVE_TARGET 2.6f   /* 仅 L8 三点小漏：tgt+2.6 */
#define DEEP_T6_MID_BELOW_TARGET   0.8f   /* L10–L12：T_6 = tgt-0.8 */
#define DEEP_T6_HIGH_BELOW_TARGET  2.0f   /* L13–L15：T_6 = tgt-2.0 */
#define NEAR_TGT_BAND_LO           0.25f  /* 刺激 L1 近目标带下限 tgt+0.25 */
#define DEEP_C_MEAN_ABOVE_MIN6     1.0f   /* T_c_mean = T_min6+1，压单周期尖谷 */
#define SHALLOW_LARGE_FRAC         0.30f  /* 过浅大漏：tgt + min(0.3*|tgt|, CAP) */
#define SHALLOW_LARGE_CAP_KPA      10.0f  /* 过浅大漏偏移上限 10 kPa */

/* --------------------------------------------------------------------------
 * 刺激：漏气一律偏浅。L1–L2 无大漏三点；L3 起有大漏；L4–L5 另加均值/max。
 * -------------------------------------------------------------------------- */
#define STIM_SMALL_LAST3_ABOVE     2.5f   /* L1–L2 三点小漏 */
#define STIM_SMALL_MEAN_ABOVE      2.5f   /* L1–L2 均值小漏 */
#define STIM_SMALL_MIN6_BELOW      0.0f   /* L1–L2：T_small_min6 = tgt - 0 */
#define STIM_L3_LARGE_ABOVE        3.2f   /* L3 大漏 */
#define STIM_L3_SMALL_LAST3_ABOVE  2.0f   /* L3 小漏三点 */
#define STIM_L3_SMALL_MEAN_ABOVE   2.0f   /* L3 小漏均值 */
#define STIM_L3_SMALL_MIN6_ABOVE   1.5f   /* L3 小漏 min6 */
#define STIM_L45_LARGE_ABOVE       2.5f   /* L4–L5 三点大漏 */
#define STIM_L45_LARGE_MEAN_ABOVE  1.8f   /* L4–L5 大漏均值 */
#define STIM_L45_LARGE_MAX_ABOVE   3.2f   /* L4–L5 大漏 max6 */
#define STIM_L45_SMALL_MEAN_ABOVE  2.2f   /* L4–L5 小漏均值 */
#define STIM_L45_SMALL_MIN6_ABOVE  1.8f   /* L4–L5 小漏 min6 */

typedef struct _LEAK_SPEC
{
	unsigned int uHas;           /* LEAK_HAS_* 组合 */
	unsigned char uFamily;       /* T_large 用浅/深比较 */
	float T_large;
	float T_large_shallow;
	float T_min6;
	float T_c_mean;
	float T_6;
	float T_small;
	float T_small_mean;
	float T_small_min6;
	float T_large_mean;
	float T_large_max;
	float T_band_lo;
	float T_band_hi;
} SLEAK_SPEC;

static float LeakMinF(float a, float b)
{
	return (a < b) ? a : b;
}

static float LeakAbsF(float x)
{
	return (x < 0.0f) ? -x : x;
}

/* 换档/Init：清空 6 槽、计数和漏气状态 */
void LeakDetectReset(SFEATURE_INFO* tFeatureInfoTemp)
{
	int i;
	if (tFeatureInfoTemp == 0)
	{
		return;
	}
	for (i = 0; i < 6; i++)
	{
		tFeatureInfoTemp->fLeakValleyBuff[i] = 0.0f;
	}
	tFeatureInfoTemp->uLeakValleyCount = 0;
	tFeatureInfoTemp->uLeakStatus = 0;
	tFeatureInfoTemp->uPrevLeakStatus = 0;
	for (unsigned i = 0; i < 10; i++)
	{
		tFeatureInfoTemp->uDataBaselineCnt[0] = 0;
		tFeatureInfoTemp->uDataBaselineCnt[1] = 0;
	}
}

/* 与 Python 相同：整体左移，新谷写 [5]；n 从 0 增到 6。n=3 时 last3 即 buff[3..5] */
static void LeakPushValley(SFEATURE_INFO* t, float valley)
{
	int i;
	for (i = 0; i < 5; i++)
	{
		t->fLeakValleyBuff[i] = t->fLeakValleyBuff[i + 1];
	}
	t->fLeakValleyBuff[5] = valley;
	if (t->uLeakValleyCount < 6)
	{
		t->uLeakValleyCount++;
	}
}

/* 吸乳档位门限。uLevel0 为设备 0 起算档（L1→0）。成功返回 1 */
static int LeakSpecExpression(unsigned int uLevel0, SLEAK_SPEC* s)
{
	unsigned int lv = uLevel0 + 1; /* 1–15 档 */
	float tgt;
	if (lv < 1 || lv > 15)
	{
		return 0;
	}
	tgt = EXPRESSION_THRESHOLD[uLevel0];
	s->uHas = 0;
	s->uFamily = LEAK_FAM_DEEP;
	if (lv <= 5)
	{
		/* 浅档：漏气=过浅 */
		s->uFamily = LEAK_FAM_SHALLOW;
		s->T_small = tgt + ((lv == 1) ? SHALLOW_L1_SMALL_ABOVE : SHALLOW_SMALL_ABOVE_TARGET);
		s->T_large = tgt + ((lv == 1) ? SHALLOW_L1_LARGE_ABOVE : SHALLOW_LARGE_ABOVE_TARGET);
		s->uHas = LEAK_HAS_SMALL | LEAK_HAS_LARGE;
		return 1;
	}
	/* 深档：过深大漏 + 过浅大漏兜底 */
	s->T_large = tgt - ((lv <= 9) ? DEEP_LAST3_BELOW_TARGET : 4.0f);
	s->T_large_shallow = tgt + LeakMinF(SHALLOW_LARGE_FRAC * LeakAbsF(tgt), SHALLOW_LARGE_CAP_KPA);
	s->uHas = LEAK_HAS_LARGE | LEAK_HAS_LARGE_SHALLOW;
	if (lv <= 12)
	{
		s->T_min6 = tgt - DEEP_MIN6_BELOW_TARGET;
		s->T_c_mean = s->T_min6 + DEEP_C_MEAN_ABOVE_MIN6;
		s->uHas |= LEAK_HAS_MIN6 | LEAK_HAS_CMEAN;
	}
	if (lv >= 10 && lv <= 12)
	{
		s->T_6 = tgt - DEEP_T6_MID_BELOW_TARGET;
		s->uHas |= LEAK_HAS_T6;
	}
	if (lv >= 13)
	{
		s->T_6 = tgt - DEEP_T6_HIGH_BELOW_TARGET;
		s->uHas |= LEAK_HAS_T6;
	}
	if (lv <= 10)
	{
		s->T_small_mean = tgt + SHALLOW_MEAN_ABOVE_TARGET;
		s->T_small_min6 = tgt + SHALLOW_MIN6_ABOVE_TARGET;
		s->uHas |= LEAK_HAS_SMEAN | LEAK_HAS_SMIN6;
	}
	if (lv == 8)
	{
		s->T_small = tgt + SHALLOW_LAST3_ABOVE_TARGET;
		s->uHas |= LEAK_HAS_SMALL;
	}
	return 1;
}

/* 刺激 1–5 档门限。uLevel0：L1→0 */
static int LeakSpecStimulation(unsigned int uLevel0, SLEAK_SPEC* s)
{
	unsigned int lv = uLevel0 + 1;
	float tgt;
	if (lv < 1 || lv > 5)
	{
		return 0;
	}
	tgt = STIMULATION_THRESHOLD[uLevel0];
	s->uHas = 0;
	s->uFamily = LEAK_FAM_SHALLOW;
	if (lv == 1)
	{
		s->T_small = tgt + STIM_SMALL_LAST3_ABOVE;
		s->T_small_mean = tgt + STIM_SMALL_MEAN_ABOVE;
		s->T_small_min6 = tgt - STIM_SMALL_MIN6_BELOW;
		s->T_band_lo = tgt + NEAR_TGT_BAND_LO;
		s->T_band_hi = tgt + 1.00f; /* 近目标带上限 tgt+1.0 */
		s->uHas = LEAK_HAS_SMALL | LEAK_HAS_SMEAN | LEAK_HAS_SMIN6 | LEAK_HAS_BAND;
		return 1;
	}
	if (lv == 2)
	{
		s->T_small = tgt + STIM_SMALL_LAST3_ABOVE;
		s->T_small_mean = tgt + STIM_SMALL_MEAN_ABOVE;
		s->T_small_min6 = tgt - STIM_SMALL_MIN6_BELOW;
		s->uHas = LEAK_HAS_SMALL | LEAK_HAS_SMEAN | LEAK_HAS_SMIN6;
		return 1;
	}
	if (lv == 3)
	{
		s->T_large = tgt + STIM_L3_LARGE_ABOVE;
		s->T_small = tgt + STIM_L3_SMALL_LAST3_ABOVE;
		s->T_small_mean = tgt + STIM_L3_SMALL_MEAN_ABOVE;
		s->T_small_min6 = tgt + STIM_L3_SMALL_MIN6_ABOVE;
		s->uHas = LEAK_HAS_LARGE | LEAK_HAS_SMALL | LEAK_HAS_SMEAN | LEAK_HAS_SMIN6;
		return 1;
	}
	/* L4–L5 */
	s->T_large = tgt + STIM_L45_LARGE_ABOVE;
	s->T_large_mean = tgt + STIM_L45_LARGE_MEAN_ABOVE;
	s->T_large_max = tgt + STIM_L45_LARGE_MAX_ABOVE;
	s->T_small_mean = tgt + STIM_L45_SMALL_MEAN_ABOVE;
	s->T_small_min6 = tgt + STIM_L45_SMALL_MIN6_ABOVE;
	s->uHas = LEAK_HAS_LARGE | LEAK_HAS_LMEAN | LEAK_HAS_LMAX | LEAK_HAS_SMEAN | LEAK_HAS_SMIN6;
	return 1;
}

/* 浅：三点都 > thr；深：三点都 < thr */
static int LeakThreePass(const float last3[3], float thr, unsigned char uFamily)
{
	if (uFamily == LEAK_FAM_SHALLOW)
	{
		return (last3[0] > thr && last3[1] > thr && last3[2] > thr);
	}
	return (last3[0] < thr && last3[1] < thr && last3[2] < thr);
}

/* 瞬时等级：0 无漏 / 1 小漏 / 2 大漏。先大漏条件，再小漏。n<3 不判 */
static unsigned char LeakClassify(const float* buff, unsigned char n, const SLEAK_SPEC* spec)
{
	float last3[3];
	float mean6;
	float min6;
	float max6;
	int i;
	int all_lt_t6;

	if (n < 3 || spec == 0 || spec->uHas == 0)
	{
		return 0;
	}
	/* 左移缓冲：有效 last3 始终在 [3][4][5] */
	last3[0] = buff[3];
	last3[1] = buff[4];
	last3[2] = buff[5];
	mean6 = 0.0f;
	min6 = buff[0];
	max6 = buff[0];
	if (n == 6)
	{
		for (i = 0; i < 6; i++)
		{
			mean6 += buff[i];
			if (buff[i] < min6)
			{
				min6 = buff[i];
			}
			if (buff[i] > max6)
			{
				max6 = buff[i];
			}
		}
		mean6 /= 6.0f;
	}

	/* --- 大漏 --- */
	if ((spec->uHas & LEAK_HAS_LARGE) && LeakThreePass(last3, spec->T_large, spec->uFamily))
	{
		return 2;
	}
	if (n == 6 && (spec->uHas & LEAK_HAS_LMEAN) && (spec->uHas & LEAK_HAS_LMAX) &&
		mean6 > spec->T_large_mean && max6 > spec->T_large_max)
	{
		return 2;
	}
	if ((spec->uHas & LEAK_HAS_LARGE_SHALLOW) && LeakThreePass(last3, spec->T_large_shallow, LEAK_FAM_SHALLOW))
	{
		return 2;
	}
	if (n == 6 && (spec->uHas & LEAK_HAS_MIN6) && min6 < spec->T_min6 &&
		(!(spec->uHas & LEAK_HAS_CMEAN) || mean6 < spec->T_c_mean))
	{
		return 2;
	}
	if (n == 6 && (spec->uHas & LEAK_HAS_T6))
	{
		all_lt_t6 = 1;
		for (i = 0; i < 6; i++)
		{
			if (!(buff[i] < spec->T_6))
			{
				all_lt_t6 = 0;
				break;
			}
		}
		if (all_lt_t6 && mean6 < (spec->T_6 - MEAN6_DEEP_MARGIN))
		{
			return 2;
		}
	}
	/* --- 小漏（偏浅）--- */
	if ((spec->uHas & LEAK_HAS_SMALL) && LeakThreePass(last3, spec->T_small, LEAK_FAM_SHALLOW))
	{
		return 1;
	}
	if (n == 6 && (spec->uHas & LEAK_HAS_SMEAN) && (spec->uHas & LEAK_HAS_SMIN6) &&
		mean6 > spec->T_small_mean && min6 > spec->T_small_min6)
	{
		return 1;
	}
	if (n == 6 && (spec->uHas & LEAK_HAS_BAND) &&
		mean6 > spec->T_band_lo && mean6 < spec->T_band_hi)
	{
		return 1;
	}
	return 0;
}

static unsigned char LeakDetectExpression(const float* buff, unsigned char n, unsigned int uLevel0)
{
	SLEAK_SPEC spec;
	if (!LeakSpecExpression(uLevel0, &spec))
	{
		return 0;
	}
	return LeakClassify(buff, n, &spec);
}

static unsigned char LeakDetectStimulation(const float* buff, unsigned char n, unsigned int uLevel0)
{
	SLEAK_SPEC spec;
	if (!LeakSpecStimulation(uLevel0, &spec))
	{
		return 0;
	}
	return LeakClassify(buff, n, &spec);
}

/* 每个泵周期提交调用一次。inst 0/1/2 → 对外 1/2/3 */
void LeakDetectUpdate(SFEATURE_INFO* tFeatureInfoTemp, SDEVICE_INFO* tDeviceInfoTemp)
{
	unsigned char inst;
	unsigned char mapped;
	unsigned int uMode;
	unsigned int uLevel;

	if (tFeatureInfoTemp == 0 || tDeviceInfoTemp == 0)
	{
		return;
	}

	/* 1. 无论是否佩戴，先写入本周期 raw 段最小 */
	LeakPushValley(tFeatureInfoTemp, tFeatureInfoTemp->fRawSegMinPressure);

	/* 2. 未佩戴：对外 0，缓冲保留 */
	if (tFeatureInfoTemp->uWearStatus == 0 && tFeatureInfoTemp->uPrevLeakStatus != 2)
	{
		tFeatureInfoTemp->uLeakStatus = 0;
		return;
	}

	uMode = tDeviceInfoTemp->uMode;
	uLevel = tDeviceInfoTemp->uLevel;

	/* 3. 混合：已戴固定 1，不分级 */
	if (uMode == MODE_MIXED)
	{
		tFeatureInfoTemp->uLeakStatus = 1;
		return;
	}

	/* 4. 已戴至少为 1；已到大漏 3 则不再判 */
	if (tFeatureInfoTemp->uLeakStatus < 1)
	{
		tFeatureInfoTemp->uLeakStatus = 1;
	}
	if (tFeatureInfoTemp->uLeakStatus >= 3)
	{
		return;
	}

	/* 5. 吸乳 / 刺激分级 */
	inst = 0;
	if (uMode == MODE_EXPRESSION)
	{
		inst = LeakDetectExpression(tFeatureInfoTemp->fLeakValleyBuff,
			tFeatureInfoTemp->uLeakValleyCount, uLevel);
	}
	else if (uMode == MODE_STIMULATION)
	{
		inst = LeakDetectStimulation(tFeatureInfoTemp->fLeakValleyBuff,
			tFeatureInfoTemp->uLeakValleyCount, uLevel);
	}
	mapped = (unsigned char)(inst + 1); /* 0→1, 1→2, 2→3 */
	if (mapped > tFeatureInfoTemp->uLeakStatus)
	{
		tFeatureInfoTemp->uLeakStatus = mapped;
	}
}
