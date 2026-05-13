#include "v3_target_curve_table.h"

/*
 * 目标曲线占位表：
 * 1. 先保留一个无效项，这样即使曲线还没补全，工程也能先编译。
 * 2. 你后续把真实曲线数组和表项继续往下面加即可。
 *
 * 示例：
 * static const float g_expr_60_370_p1[] = { ... };
 *
 * {
 *     60u,
 *     370u,
 *     (unsigned short)(sizeof(g_expr_60_370_p1) / sizeof(g_expr_60_370_p1[0])),
 *     g_expr_60_370_p1
 * },
 */

//刺激L0
static const float g_stim_0_60_p1[] =
{0.0f};

 //刺激L1
static const float g_stim_60_270_p1[] =
{
    -0.2277645f,
    -0.758020081f,
    -1.412004571f,
    -1.764302474f,
    -1.817206431f,
    -2.385209442f,
    -3.734647542f,
    -4.591112508f,
    -4.969895725f,
    -4.994055326f,
    -5.683785264f,
    -6.444892206f,
    -7.369425167f
};

//刺激L2
static const float g_stim_70_245_p1[] =
{0.0f};

//刺激L3
static const float g_stim_90_270_p1[] =
{0.0f};

//刺激L4
static const float g_stim_100_270_p1[] =
{0.0f};

 //刺激L5
static const float g_stim_120_260_p1[] =
{
    -0.369623889f,
    -0.404244229f,
    -0.864033649f,
    -2.180534265f,
    -4.030406083f,
    -4.128340757f,
    -5.584267722f,
    -7.488378174f,
    -7.37653967f,
    -9.002562032f,
    -10.63375677f,
    -10.48452863f,
    -11.85199111f,
    -13.5564585
};

//刺激L6
static const float g_stim_130_303_p1[] =
{0.0f};

//刺激L7
static const float g_stim_140_290_p1[] =
{0.0f};

//刺激L8
static const float g_stim_150_302_p1[] =
{0.0f};

//刺激L9
static const float g_stim_170_285_p1[] =
{0.0f};

//刺激L10
static const float g_stim_180_305_p1[] =
{
    -0.482956833f,
    -0.496291538f,
    -1.044188513f,
    -1.536093215f,
    -1.769866497f,
    -5.57040481f,
    -5.209376143f,
    -9.194042516f,
    -9.018939863f,
    -12.71345887f,
    -12.33160493f,
    -15.08207845f,
    -14.41060757f,
    -14.62020679f,
    -16.76234368f,
    -17.36662974f,
    -19.598294f,
};

//刺激L11
static const float g_stim_190_285_p1[] =
{0.0f};

//刺激L12
static const float g_stim_200_305_p1[] =
{0.0f};

//刺激L13
static const float g_stim_210_313_p1[] =
{0.0f};

//刺激L14
static const float g_stim_220_340_p1[] =
{0.0f};

//刺激L15
static const float g_stim_230_322_p1[] =
{
    -0.617718857f,
    -1.261753564f,
    -2.836990742f,
    -3.610224531f,
    -7.973250692f,
    -8.101080283f,
    -11.51628773f,
    -9.079001874f,
    -11.38350567f,
    -19.37700006f,
    -16.87799352f,
    -18.39216124f,
    -18.26157325f,
    -20.53389674f,
    -26.75173413f,
    -23.8551285f,
    -25.30834357f
};

//吸乳L0
static const float g_expr_0_60_p1[] =
{0.0f};

//吸乳L1
static const float g_expr_60_370_p1[] =
{
    -0.263379667f,
    -0.361036434f,
    -1.203922234f,
    -1.794293231f,
    -2.108822163f,
    -2.147593775f,
    -2.085389309f,
    -2.735232485f,
    -3.600229582f,
    -4.458681837f,
    -4.973239333f,
    -5.086823293f,
    -5.020634075f,
    -4.93138889f,
    -5.18088927f,
    -6.091753951f,
    -6.911443905f,
    -7.416795333f
};

//吸乳L2
static const float g_expr_100_404_p1[] =
{0.0f};

//吸乳L3
static const float g_expr_130_425_p1[] =
{0.0f};

//吸乳L4
static const float g_expr_150_440_p1[] =
{0.0f};

//吸乳L5
static const float g_expr_180_468_p1[] =
{
    -0.558732625f,
    -0.65209177f,
    -1.150416956f,
    -2.68112012f,
    -3.273930495f,
    -4.363153532f,
    -5.847837072f,
    -6.70751928f,
    -7.108192127f,
    -8.107820889f,
    -9.522265013f,
    -10.09934608f,
    -10.93368029f,
    -12.46308204f,
    -12.65695578f,
    -13.07921601f,
    -14.07256937f,
    -15.25762428f,
    -15.57343561f,
    -16.33442296f,
    -17.81057929f,
    -17.84439881f,
    -18.19152314f,
    -18.84878556f,
    -19.66725263f
};

//吸乳L6
static const float g_expr_200_475_p1[] =
{0.0f};

//吸乳L7
static const float g_expr_220_490_p1[] =
{0.0f};

//吸乳L8
static const float g_expr_240_540_p1[] =
{0.0f};

//吸乳L9
static const float g_expr_260_548_p1[] =
{0.0f};

//吸乳L10
static const float g_expr_280_560_p1[] =
{
    -0.50224175f,
    -1.07545793f,
    -2.414437839f,
    -3.091550349f,
    -4.353235933f,
    -5.609771445f,
    -7.437839103f,
    -9.522791086f,
    -9.942584794f,
    -12.40991366f,
    -12.9601487f,
    -16.48115124f,
    -15.41706143f,
    -15.99557416f,
    -17.86853932f,
    -18.84428124f,
    -20.12334259f,
    -20.63381713f,
    -23.86752235f,
    -22.65451193f,
    -23.34496532f,
    -24.91828457f,
    -25.19187332f,
    -26.50752047f,
    -27.06682755f,
    -28.83660944f,
    -28.66245812f,
    -29.42034975f,
    -30.16553317f
};

//吸乳L11
static const float g_expr_300_600_p1[] =
{0.0f};

//吸乳L12
static const float g_expr_310_620_p1[] =
{0.0f};

//吸乳L13
static const float g_expr_330_650_p1[] =
{0.0f};

//吸乳L14
static const float g_expr_350_685_p1[] =
{0.0f};

//吸乳L15
static const float g_expr_360_710_p1[] =
{
    -0.4584392f,
    -0.448486582f,
    -0.621818505f,
    -4.334847111f,
    -6.281517339f,
    -10.51092201f,
    -9.794703241f,
    -12.4614522f,
    -12.0157424f,
    -14.27536127f,
    -14.44423229f,
    -16.5537526f,
    -17.25157279f,
    -18.99285093f,
    -19.98897748f,
    -21.4041172f,
    -22.48667067f,
    -23.68326978f,
    -24.76471886f,
    -25.84821275f,
    -26.89991896f,
    -27.88146869f,
    -28.91104804f,
    -29.82801791f,
    -30.8798997f,
    -31.60640375f,
    -32.64896094f,
    -33.31417979f,
    -34.45458623f,
    -34.99516856f,
    -36.02493698f,
    -36.53571524f,
    -37.57817813f,
    -38.0368867f,
    -39.06188686f,
    -39.4620522f
};

const STARGET_CURVE_ITEM g_target_curve_table[] =
{
     //刺激0
    {
        0u,
        60u,
        0.0f,
        0.0f,
        (unsigned short)(sizeof(g_stim_0_60_p1) / sizeof(g_stim_0_60_p1[0])),
        g_stim_0_60_p1
    },
    //刺激1
    {
        6u,
        270u,
        -3.5f,
        -7.2f,
        (unsigned short)(sizeof(g_stim_60_270_p1) / sizeof(g_stim_60_270_p1[0])),
        g_stim_60_270_p1
    },
    //刺激2
    {
        7u,
        245u,
        -4.0f,
        -8.6f,
        (unsigned short)(sizeof(g_stim_70_245_p1) / sizeof(g_stim_70_245_p1[0])),
        g_stim_70_245_p1
    },
    //刺激3
    {
        9u,
        270u,
        -4.5f,
        -9.8f,
        (unsigned short)(sizeof(g_stim_90_270_p1) / sizeof(g_stim_90_270_p1[0])),
        g_stim_90_270_p1
    },
    //刺激4
    {
        10u,
        270u,
        -5.0f,
        -11.7f,
        (unsigned short)(sizeof(g_stim_100_270_p1) / sizeof(g_stim_100_270_p1[0])),
        g_stim_100_270_p1
    },
    //刺激5
    {
        12u,
        260u,
        -5.5f,
        -13.3f,
        (unsigned short)(sizeof(g_stim_120_260_p1) / sizeof(g_stim_120_260_p1[0])),
        g_stim_120_260_p1
    },
    //刺激6
    {
        13u,
        303u,
        -6.0f,
        -13.7f,
        (unsigned short)(sizeof(g_stim_130_303_p1) / sizeof(g_stim_130_303_p1[0])),
        g_stim_130_303_p1
    },
    //刺激7
    {
        14u,
        290u,
        -7.0f,
        -15.6f,
        (unsigned short)(sizeof(g_stim_140_290_p1) / sizeof(g_stim_140_290_p1[0])),
        g_stim_140_290_p1
    },
    //刺激8
    {
        15u,
        302u,
        -8.0f,
        -16.5f,
        (unsigned short)(sizeof(g_stim_150_302_p1) / sizeof(g_stim_150_302_p1[0])),
        g_stim_150_302_p1
    },
    //刺激9
    {
        17u,
        285u,
        -9.0f,
        -18.0f,
        (unsigned short)(sizeof(g_stim_170_285_p1) / sizeof(g_stim_170_285_p1[0])),
        g_stim_170_285_p1
    },
    //刺激10
    {
        18u,
        305u,
        -10.0f,
        -19.1f,
        (unsigned short)(sizeof(g_stim_180_305_p1) / sizeof(g_stim_180_305_p1[0])),
        g_stim_180_305_p1
    },
    //刺激11
    {
        19u,
        285u,
        -18.0f,
        -20.2f,
        (unsigned short)(sizeof(g_stim_190_285_p1) / sizeof(g_stim_190_285_p1[0])),
        g_stim_190_285_p1
    },
    //刺激12
    {
        20u,
        305u,
        -19.1f,
        -21.1f,
        (unsigned short)(sizeof(g_stim_200_305_p1) / sizeof(g_stim_200_305_p1[0])),
        g_stim_200_305_p1
    },
    //刺激13
    {
        21u,
        313u,
        -21.0f,
        -22.1f,
        (unsigned short)(sizeof(g_stim_210_313_p1) / sizeof(g_stim_210_313_p1[0])),
        g_stim_210_313_p1
    },
    //刺激14
    {
        22u,
        340u,
        -22.0f,
        -23.1f,
        (unsigned short)(sizeof(g_stim_220_340_p1) / sizeof(g_stim_220_340_p1[0])),
        g_stim_220_340_p1
    },
    //刺激15
    {
        23u,
        322u,
        -23.0f,
        -24.5f,
        (unsigned short)(sizeof(g_stim_230_322_p1) / sizeof(g_stim_230_322_p1[0])),
        g_stim_230_322_p1
    },
    //吸乳L0
    {
        0u,
        60u,
        0.0f,
        0.0f,
        (unsigned short)(sizeof(g_expr_0_60_p1) / sizeof(g_expr_0_60_p1[0])),
        g_expr_0_60_p1
    },
    //吸乳L1
    {
        6u,
        370u,
        -4.5f,
        -7.4f,
        (unsigned short)(sizeof(g_expr_60_370_p1) / sizeof(g_expr_60_370_p1[0])),
        g_expr_60_370_p1
    },
    //吸乳L2
    {
        10u,
        404u,
        -5.0f,
        -10.5f,
        (unsigned short)(sizeof(g_expr_100_404_p1) / sizeof(g_expr_100_404_p1[0])),
        g_expr_100_404_p1
    },
    //吸乳L3
    {
        13u,
        425u,
        -6.0f,
        -14.2f,
        (unsigned short)(sizeof(g_expr_130_425_p1) / sizeof(g_expr_130_425_p1[0])),
        g_expr_130_425_p1
    }
    ,
    //吸乳L4
    {
        15u,
        440u,
        -7.5f,
        -17.1f,
        (unsigned short)(sizeof(g_expr_150_440_p1) / sizeof(g_expr_150_440_p1[0])),
        g_expr_150_440_p1
    },
    //吸乳L5
    {
        18u,
        468u,
        -11.0f,
        -19.4f,
        (unsigned short)(sizeof(g_expr_180_468_p1) / sizeof(g_expr_180_468_p1[0])),
        g_expr_180_468_p1
    }
    ,
    //吸乳L6
    {
        20u,
        475u,
        -11.0f,
        -22.3f,
        (unsigned short)(sizeof(g_expr_200_475_p1) / sizeof(g_expr_200_475_p1[0])),
        g_expr_200_475_p1
    },
    //吸乳L7
    {
        22u,
        490u,
        -16.0f,
        -24.0f,
        (unsigned short)(sizeof(g_expr_220_490_p1) / sizeof(g_expr_220_490_p1[0])),
        g_expr_220_490_p1
    },
    //吸乳L8
    {
        24u,
        540u,
        -18.0f,
        -25.4f,
        (unsigned short)(sizeof(g_expr_240_540_p1) / sizeof(g_expr_240_540_p1[0])),
        g_expr_240_540_p1
    },
    //吸乳L9
    {
        26u,
        548u,
        -18.0f,
        -28.3f,
        (unsigned short)(sizeof(g_expr_260_548_p1) / sizeof(g_expr_260_548_p1[0])),
        g_expr_260_548_p1
    },
    //吸乳L10
    {
        28u,
        560u,
        -21.0f,
        //-35.0f,
        -30.4f,
        (unsigned short)(sizeof(g_expr_280_560_p1) / sizeof(g_expr_280_560_p1[0])),
        g_expr_280_560_p1
    },
    //吸乳L11
    {
        30u,
        600u,
        -29.0f,
        //-35.0f,
        -32.5f,
        (unsigned short)(sizeof(g_expr_300_600_p1) / sizeof(g_expr_300_600_p1[0])),
        g_expr_300_600_p1
    },
    //吸乳L12
    {
        31u,
        620u,
        -30.0f,
        //-40.0f,
        -34.8f,
        (unsigned short)(sizeof(g_expr_310_620_p1) / sizeof(g_expr_310_620_p1[0])),
        g_expr_310_620_p1
    },
    //吸乳L13
    {
        33u,
        650u,
        -32.5f,
        //-40.0f,
        -36.0f,
        (unsigned short)(sizeof(g_expr_330_650_p1) / sizeof(g_expr_330_650_p1[0])),
        g_expr_330_650_p1
    },
    //吸乳L14
    {
        35u,
        685u,
        -34.7f,
        //-40.0f,
        -37.8f,
        (unsigned short)(sizeof(g_expr_350_685_p1) / sizeof(g_expr_350_685_p1[0])),
        g_expr_350_685_p1
    },
    //吸乳L15
    {
        36u,
        710u,
        -35.5f,
        //-40.0f,
        -39.0f,
        (unsigned short)(sizeof(g_expr_360_710_p1) / sizeof(g_expr_360_710_p1[0])),
        g_expr_360_710_p1
    },
    { 0u, 0u, 0.0f, 0.0f, 0u, 0 }
};

const unsigned int g_target_curve_table_count =
    (unsigned int)(sizeof(g_target_curve_table) / sizeof(g_target_curve_table[0]));

/* 按双键查找目标曲线：
 * - target_pressure
 * - target_pump_time
 */
const STARGET_CURVE_ITEM* FindTargetCurve(unsigned char target_pressure,
                                          unsigned short target_pump_time)
{
    unsigned int i;

    for (i = 0; i < g_target_curve_table_count; i++)
    {
        const STARGET_CURVE_ITEM* item = &g_target_curve_table[i];

        /* 跳过尚未填写完成的无效表项 */
        if (item->target_pressure == 0u ||
            item->target_pump_time == 0u ||
            item->curve_len == 0u ||
            item->p1_curve == 0)
        {
            continue;
        }

        /* 双键都匹配时，说明找到了对应目标曲线 */
        if (item->target_pressure == target_pressure &&
            item->target_pump_time == target_pump_time)
        {
            return item;
        }
    }

    return 0;
}

/* 在已找到的目标曲线上取当前目标点：
 * - 只取 P1 曲线
 * - 再把当前 pumping_time 按比例映射到曲线下标
 */
float GetTargetPressureFromCurve(const STARGET_CURVE_ITEM* curve,
                                 unsigned short pumping_time,
                                 unsigned char* is_valid)
{
    unsigned short idx;

    if (is_valid != 0)
    {
        *is_valid = 0u;
    }

    if (curve == 0 || curve->curve_len == 0u || curve->target_pump_time == 0u || curve->p1_curve == 0)
    {
        return INVALID_TARGET_PRESSURE_REF;
    }

    /* 当前 pumping_time 超过目标 pumping_time 时，直接钳到最后一个点 */
    if (pumping_time >= curve->curve_len)
    {
        idx = (curve->curve_len - 1u);
    }
    else
    {
        /* 把当前 pumping_time 等比例映射到目标曲线长度 */
        idx = pumping_time;
    }

    /* 再做一次保护，避免下标越界 */
    if (idx >= curve->curve_len)
    {
        idx = (unsigned int)(curve->curve_len - 1u);
    }

    if (is_valid != 0)
    {
        *is_valid = 1u;
    }

    return curve->p1_curve[idx];
}

const STARGET_CURVE_ITEM* GetCurrentTargetCurveItem(
    const SDEVICE_INFO* ptDeviceInfo)
{
    unsigned int i = 0;

    if (ptDeviceInfo == 0)
    {
        return 0;
    }

    for (i = 0; g_target_curve_table[i].target_pump_time != 0u; i++)
    {
        if ((g_target_curve_table[i].target_pressure == ptDeviceInfo->uTargetPressure) &&
            (g_target_curve_table[i].target_pump_time == ptDeviceInfo->uTargetPumpTime))
        {
            return &g_target_curve_table[i];
        }
    }

    return 0;
}