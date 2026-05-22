#include "v3_target_curve_table.h"
#define LONGYI_OR_SHIGUANG  0 //1: 龙一, 0: 时光
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

#if LONGYI_OR_SHIGUANG == 1
//刺激L0
static const float g_stim_0_60_p1[] =
{0.0f};

 //刺激L1
static const float g_stim_60_270_p1[] =
{0.0f};

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
{0.0f};

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
{0.0f};

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
{0.0f};

//吸乳L0
static const float g_expr_0_60_p1[] =
{0.0f};

//吸乳L1
static const float g_expr_60_370_p1[] =
{0.0f};

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
{0.0f};

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
{0.0f};

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
{0.0f};

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
        -28.0f,
        //-35.0f,
        -32.5f,
        (unsigned short)(sizeof(g_expr_300_600_p1) / sizeof(g_expr_300_600_p1[0])),
        g_expr_300_600_p1
    },
    //吸乳L12
    {
        31u,
        620u,
        -29.0f,
        //-40.0f,
        -34.8f,
        (unsigned short)(sizeof(g_expr_310_620_p1) / sizeof(g_expr_310_620_p1[0])),
        g_expr_310_620_p1
    },
    //吸乳L13
    {
        33u,
        650u,
        -31.0f,
        //-40.0f,
        -36.0f,
        (unsigned short)(sizeof(g_expr_330_650_p1) / sizeof(g_expr_330_650_p1[0])),
        g_expr_330_650_p1
    },
    //吸乳L14
    {
        35u,
        685u,
        -33.0f,
        //-40.0f,
        -37.8f,
        (unsigned short)(sizeof(g_expr_350_685_p1) / sizeof(g_expr_350_685_p1[0])),
        g_expr_350_685_p1
    },
    //吸乳L15
    {
        36u,
        710u,
        -40.0f,
        //-40.0f,
        -39.0f,
        (unsigned short)(sizeof(g_expr_360_710_p1) / sizeof(g_expr_360_710_p1[0])),
        g_expr_360_710_p1
    },
    { 0u, 0u, 0.0f, 0.0f, 0u, 0 }
};
#else
//刺激L0
static const float g_stim_0_60_p1[] =
{0.0f};

 //刺激L1
static const float g_stim_60_220_p1[] =
{0.0f};

//刺激L2
static const float g_stim_70_230_p1[] =
{0.0f};

//刺激L3
static const float g_stim_90_230_p1[] =
{0.0f};

//刺激L4
static const float g_stim_100_240_p1[] =
{0.0f};

 //刺激L5
static const float g_stim_120_245_p1[] =
{0.0f};

//刺激L6
static const float g_stim_130_270_p1[] =
{0.0f};

//刺激L7
static const float g_stim_140_264_p1[] =
{0.0f};

//刺激L8
static const float g_stim_150_287_p1[] =
{0.0f};

//刺激L9
static const float g_stim_170_272_p1[] =
{0.0f};

//刺激L10
static const float g_stim_180_291_p1[] =
{0.0f};

//刺激L11
static const float g_stim_190_300_p1[] =
{0.0f};

//刺激L12
static const float g_stim_200_300_p1[] =
{0.0f};

//刺激L13
static const float g_stim_210_314_p1[] =
{0.0f};

//刺激L14
static const float g_stim_220_330_p1[] =
{0.0f};

//刺激L15
static const float g_stim_230_328_p1[] =
{0.0f};

//吸乳L0
static const float g_expr_0_100_p1[] =
{0.0f};

//吸乳L1
static const float g_expr_60_340_p1[] =
{0.0f};

//吸乳L2
static const float g_expr_100_406_p1[] =
{0.0f};

//吸乳L3
static const float g_expr_130_452_p1[] =
{0.0f};

//吸乳L4
static const float g_expr_150_472p1[] =
{0.0f};

//吸乳L5
static const float g_expr_180_492_p1[] =
{0.0f};

//吸乳L6
static const float g_expr_200_528_p1[] =
{0.0f};

//吸乳L7
static const float g_expr_220_560_p1[] =
{0.0f};

//吸乳L8
static const float g_expr_240_622_p1[] =
{0.0f};

//吸乳L9
static const float g_expr_260_632_p1[] =
{0.0f};

//吸乳L10
static const float g_expr_280_660_p1[] =
{0.0f};

//吸乳L11
static const float g_expr_300_647_p1[] =
{0.0f};

//吸乳L12
static const float g_expr_310_705_p1[] =
{0.0f};

//吸乳L13
static const float g_expr_330_684_p1[] =
{0.0f};

//吸乳L14
static const float g_expr_350_718_p1[] =
{0.0f};

//吸乳L15
static const float g_expr_360_708_p1[] =
{0.0f};

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
        220u,
        -5.0f,
        -7.2f,
        (unsigned short)(sizeof(g_stim_60_220_p1) / sizeof(g_stim_60_220_p1[0])),
        g_stim_60_220_p1
    },
    //刺激2
    {
        7u,
        230u,
        -6.0f,
        -8.6f,
        (unsigned short)(sizeof(g_stim_70_230_p1) / sizeof(g_stim_70_230_p1[0])),
        g_stim_70_230_p1
    },
    //刺激3
    {
        9u,
        230u,
        -8.0f,
        -9.8f,
        (unsigned short)(sizeof(g_stim_90_230_p1) / sizeof(g_stim_90_230_p1[0])),
        g_stim_90_230_p1
    },
    //刺激4
    {
        10u,
        240u,
        -10.0f,
        -11.7f,
        (unsigned short)(sizeof(g_stim_100_240_p1) / sizeof(g_stim_100_240_p1[0])),
        g_stim_100_240_p1
    },
    //刺激5
    {
        12u,
        245u,
        -11.0f,
        -13.3f,
        (unsigned short)(sizeof(g_stim_120_245_p1) / sizeof(g_stim_120_245_p1[0])),
        g_stim_120_245_p1
    },
    //刺激6
    {
        13u,
        270u,
        -12.0f,
        -13.7f,
        (unsigned short)(sizeof(g_stim_130_270_p1) / sizeof(g_stim_130_270_p1[0])),
        g_stim_130_270_p1
    },
    //刺激7
    {
        14u,
        264u,
        -7.0f,
        -15.6f,
        (unsigned short)(sizeof(g_stim_140_264_p1) / sizeof(g_stim_140_264_p1[0])),
        g_stim_140_264_p1
    },
    //刺激8
    {
        15u,
        287u,
        -12.0f,
        -16.5f,
        (unsigned short)(sizeof(g_stim_150_287_p1) / sizeof(g_stim_150_287_p1[0])),
        g_stim_150_287_p1
    },
    //刺激9
    {
        17u,
        272u,
        -15.0f,
        -18.0f,
        (unsigned short)(sizeof(g_stim_170_272_p1) / sizeof(g_stim_170_272_p1[0])),
        g_stim_170_272_p1
    },
    //刺激10
    {
        18u,
        291u,
        -10.0f,
        -19.1f,
        (unsigned short)(sizeof(g_stim_180_291_p1) / sizeof(g_stim_180_291_p1[0])),
        g_stim_180_291_p1
    },
    //刺激11
    {
        19u,
        300u,
        -17.0f,
        -20.2f,
        (unsigned short)(sizeof(g_stim_190_300_p1) / sizeof(g_stim_190_300_p1[0])),
        g_stim_190_300_p1
    },
    //刺激12
    {
        20u,
        300u,
        -19.1f,
        -21.1f,
        (unsigned short)(sizeof(g_stim_200_300_p1) / sizeof(g_stim_200_300_p1[0])),
        g_stim_200_300_p1
    },
    //刺激13
    {
        21u,
        314u,
        -21.0f,
        -22.1f,
        (unsigned short)(sizeof(g_stim_210_314_p1) / sizeof(g_stim_210_314_p1[0])),
        g_stim_210_314_p1
    },
    //刺激14
    {
        22u,
        330u,
        -27.5f,
        -23.1f,
        (unsigned short)(sizeof(g_stim_220_330_p1) / sizeof(g_stim_220_330_p1[0])),
        g_stim_220_330_p1
    },
    //刺激15
    {
        23u,
        328u,
        -28.0f,
        -24.5f,
        (unsigned short)(sizeof(g_stim_230_328_p1) / sizeof(g_stim_230_328_p1[0])),
        g_stim_230_328_p1
    },
    //吸乳L0
    {
        0u,
        100u,
        0.0f,
        0.0f,
        (unsigned short)(sizeof(g_expr_0_100_p1) / sizeof(g_expr_0_100_p1[0])),
        g_expr_0_100_p1
    },
    //吸乳L1
    {
        6u,
        340u,
        -5.5f,
        -7.4f,
        (unsigned short)(sizeof(g_expr_60_340_p1) / sizeof(g_expr_60_340_p1[0])),
        g_expr_60_340_p1
    },
    //吸乳L2
    {
        10u,
        406u,
        -8.0f,
        -10.5f,
        (unsigned short)(sizeof(g_expr_100_406_p1) / sizeof(g_expr_100_406_p1[0])),
        g_expr_100_406_p1
    },
    //吸乳L3
    {
        13u,
        452u,
        -11.0f,
        -14.2f,
        (unsigned short)(sizeof(g_expr_130_452_p1) / sizeof(g_expr_130_452_p1[0])),
        g_expr_130_452_p1
    }
    ,
    //吸乳L4
    {
        15u,
        472u,
        -17.5f,
        -17.1f,
        (unsigned short)(sizeof(g_expr_150_472p1) / sizeof(g_expr_150_472p1[0])),
        g_expr_150_472p1
    },
    //吸乳L5
    {
        18u,
        492u,
        -19.0f,
        -19.4f,
        (unsigned short)(sizeof(g_expr_180_492_p1) / sizeof(g_expr_180_492_p1[0])),
        g_expr_180_492_p1
    }
    ,
    //吸乳L6
    {
        20u,
        528u,
        -23.0f,
        -22.3f,
        (unsigned short)(sizeof(g_expr_200_528_p1) / sizeof(g_expr_200_528_p1[0])),
        g_expr_200_528_p1
    },
    //吸乳L7
    {
        22u,
        560u,
        -24.0f,
        -24.0f,
        (unsigned short)(sizeof(g_expr_220_560_p1) / sizeof(g_expr_220_560_p1[0])),
        g_expr_220_560_p1
    },
    //吸乳L8
    {
        24u,
        622u,
        -28.0f,
        -25.4f,
        (unsigned short)(sizeof(g_expr_240_622_p1) / sizeof(g_expr_240_622_p1[0])),
        g_expr_240_622_p1
    },
    //吸乳L9
    {
        26u,
        632u,
        -29.0f,
        -28.3f,
        (unsigned short)(sizeof(g_expr_260_632_p1) / sizeof(g_expr_260_632_p1[0])),
        g_expr_260_632_p1
    },
    //吸乳L10
    {
        28u,
        660u,
        -33.0f,
        //-35.0f,
        -30.4f,
        (unsigned short)(sizeof(g_expr_280_660_p1) / sizeof(g_expr_280_660_p1[0])),
        g_expr_280_660_p1
    },
    //吸乳L11
    {
        30u,
        647u,
        -37.0f,
        //-35.0f,
        -32.5f,
        (unsigned short)(sizeof(g_expr_300_647_p1) / sizeof(g_expr_300_647_p1[0])),
        g_expr_300_647_p1
    },
    //吸乳L12
    {
        31u,
        705u,
        -39.5f,
        //-40.0f,
        -34.8f,
        (unsigned short)(sizeof(g_expr_310_705_p1) / sizeof(g_expr_310_705_p1[0])),
        g_expr_310_705_p1
    },
    //吸乳L13
    {
        33u,
        684u,
        -41.5f,
        //-40.0f,
        -36.0f,
        (unsigned short)(sizeof(g_expr_330_684_p1) / sizeof(g_expr_330_684_p1[0])),
        g_expr_330_684_p1
    },
    //吸乳L14
    {
        35u,
        718u,
        -42.5f,
        //-40.0f,
        -37.8f,
        (unsigned short)(sizeof(g_expr_350_718_p1) / sizeof(g_expr_350_718_p1[0])),
        g_expr_350_718_p1
    },
    //吸乳L15
    {
        36u,
        708u,
        -45.0f,
        //-40.0f,
        -39.0f,
        (unsigned short)(sizeof(g_expr_360_708_p1) / sizeof(g_expr_360_708_p1[0])),
        g_expr_360_708_p1
    },
    { 0u, 0u, 0.0f, 0.0f, 0u, 0 }
};
#endif

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