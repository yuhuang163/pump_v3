/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PNG_TEST                0
#define IS_SUPPORT_BAJ_TEST                0
#define IS_SUPPORT_BG_TEST                 0

#if IS_SUPPORT_BAJ_TEST
#define IS_SUPPORT_BAJ_TEST_BG             1
#else
#define IS_SUPPORT_BAJ_TEST_BG             0
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_PNG_TEST
#define PNG_IMG_COUNT  6
static PSRAM_DATA_LOC ALIGNED(64) uint8_t png_data0[] = {
    #include "comparison_png2txt/ver_img_360x360.txt"

};
static PSRAM_DATA_LOC ALIGNED(64) uint8_t png_data1[] = {
    #include "comparison_png2txt/ver_img_360x360.txt"
};

static ui_img_data_t png_images[PNG_IMG_COUNT] = {
    { png_data0, sizeof(png_data0) },
    { png_data1, sizeof(png_data1) },
    { png_data2, sizeof(png_data2) },
    { png_data0, sizeof(png_data0) },
    { png_data0, sizeof(png_data0) },
    { png_data0, sizeof(png_data0) },
};
#endif

#if IS_SUPPORT_BAJ_TEST
#define IS_SUPPORT_BAJ_RES           0
#define BAJ_IMG_COUNT                150

static const uint8_t baj_data0[] = {
#include "baj2txt/IP_00000_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data1[] = {
#include "baj2txt/IP_00001_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data2[] = {
#include "baj2txt/IP_00002_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data3[] = {
#include "baj2txt/IP_00003_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data4[] = {
#include "baj2txt/IP_00004_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data5[] = {
#include "baj2txt/IP_00005_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data6[] = {
#include "baj2txt/IP_00006_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data7[] = {
#include "baj2txt/IP_00007_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data8[] = {
#include "baj2txt/IP_00008_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data9[] = {
#include "baj2txt/IP_00009_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data10[] = {
#include "baj2txt/IP_00010_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data11[] = {
#include "baj2txt/IP_00011_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data12[] = {
#include "baj2txt/IP_00012_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data13[] = {
#include "baj2txt/IP_00013_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data14[] = {
#include "baj2txt/IP_00014_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data15[] = {
#include "baj2txt/IP_00015_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data16[] = {
#include "baj2txt/IP_00016_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data17[] = {
#include "baj2txt/IP_00017_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data18[] = {
#include "baj2txt/IP_00018_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data19[] = {
#include "baj2txt/IP_00019_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data20[] = {
#include "baj2txt/IP_00020_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data21[] = {
#include "baj2txt/IP_00021_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data22[] = {
#include "baj2txt/IP_00022_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data23[] = {
#include "baj2txt/IP_00023_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data24[] = {
#include "baj2txt/IP_00024_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data25[] = {
#include "baj2txt/IP_00025_200x200_a4_q99_422.txt"
};
static const uint8_t baj_data26[] = {
#include "baj2txt/IP_00026_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data27[] = {
#include "baj2txt/IP_00027_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data28[] = {
#include "baj2txt/IP_00028_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data29[] = {
#include "baj2txt/IP_00029_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data30[] = {
#include "baj2txt/IP_00030_200x200_a4_q99_422.txt"
};
 
static const uint8_t baj_data31[] = {
#include "baj2txt/IP_00031_200x200_a4_q99_422.txt"
};
    
static const uint8_t baj_data32[] = {
#include "baj2txt/IP_00032_200x200_a4_q99_422.txt"
};
    
static const uint8_t baj_data33[] = {
#include "baj2txt/IP_00033_200x200_a4_q99_422.txt"
};
    
static const uint8_t baj_data34[] = {
#include "baj2txt/IP_00034_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data35[] = {
#include "baj2txt/IP_00035_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data36[] = {
#include "baj2txt/IP_00036_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data37[] = {
#include "baj2txt/IP_00037_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data38[] = {
#include "baj2txt/IP_00038_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data39[] = {
#include "baj2txt/IP_00039_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data40[] = {
#include "baj2txt/IP_00040_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data41[] = {
#include "baj2txt/IP_00041_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data42[] = {
#include "baj2txt/IP_00042_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data43[] = {
#include "baj2txt/IP_00043_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data44[] = {
#include "baj2txt/IP_00044_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data45[] = {
#include "baj2txt/IP_00045_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data46[] = {
#include "baj2txt/IP_00046_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data47[] = {
#include "baj2txt/IP_00047_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data48[] = {
#include "baj2txt/IP_00048_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data49[] = {
#include "baj2txt/IP_00049_200x200_a4_q99_422.txt"
};
#if 1
static const uint8_t baj_data50[] = {
#include "baj2txt/IP_00050_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data51[] = {
#include "baj2txt/IP_00051_200x200_a4_q99_422.txt"
};  

static const uint8_t baj_data52[] = {
#include "baj2txt/IP_00052_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data53[] = {
#include "baj2txt/IP_00053_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data54[] = {
#include "baj2txt/IP_00054_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data55[] = {
#include "baj2txt/IP_00055_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data56[] = {
#include "baj2txt/IP_00056_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data57[] = {
#include "baj2txt/IP_00057_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data58[] = {
#include "baj2txt/IP_00058_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data59[] = {
#include "baj2txt/IP_00059_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data60[] = {
#include "baj2txt/IP_00060_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data61[] = {
#include "baj2txt/IP_00061_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data62[] = {
#include "baj2txt/IP_00062_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data63[] = {
#include "baj2txt/IP_00063_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data64[] = {
#include "baj2txt/IP_00064_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data65[] = {
#include "baj2txt/IP_00065_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data66[] = {
#include "baj2txt/IP_00066_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data67[] = {
#include "baj2txt/IP_00067_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data68[] = {
#include "baj2txt/IP_00068_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data69[] = {
#include "baj2txt/IP_00069_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data70[] = {
#include "baj2txt/IP_00070_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data71[] = {
#include "baj2txt/IP_00071_200x200_a4_q99_422.txt"
};
static const uint8_t baj_data72[] = {
#include "baj2txt/IP_00072_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data73[] = {
#include "baj2txt/IP_00073_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data74[] = {
#include "baj2txt/IP_00074_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data75[] = {
#include "baj2txt/IP_00075_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data76[] = {
#include "baj2txt/IP_00076_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data77[] = {
#include "baj2txt/IP_00077_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data78[] = {
#include "baj2txt/IP_00078_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data79[] = {
#include "baj2txt/IP_00079_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data80[] = {
#include "baj2txt/IP_00080_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data81[] = {
#include "baj2txt/IP_00081_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data82[] = {
#include "baj2txt/IP_00082_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data83[] = {
#include "baj2txt/IP_00083_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data84[] = {
#include "baj2txt/IP_00084_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data85[] = {
#include "baj2txt/IP_00085_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data86[] = {
#include "baj2txt/IP_00086_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data87[] = {
#include "baj2txt/IP_00087_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data88[] = {
#include "baj2txt/IP_00088_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data89[] = {  
#include "baj2txt/IP_00089_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data90[] = {
#include "baj2txt/IP_00090_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data91[] = {
#include "baj2txt/IP_00091_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data92[] = {
#include "baj2txt/IP_00092_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data93[] = {
#include "baj2txt/IP_00093_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data94[] = {
#include "baj2txt/IP_00094_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data95[] = {
#include "baj2txt/IP_00095_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data96[] = {
#include "baj2txt/IP_00096_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data97[] = {
#include "baj2txt/IP_00097_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data98[] = {
#include "baj2txt/IP_00098_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data99[] = {
#include "baj2txt/IP_00099_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data100[] = {
#include "baj2txt/IP_00100_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data101[] = {
#include "baj2txt/IP_00101_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data102[] = {
#include "baj2txt/IP_00102_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data103[] = {
#include "baj2txt/IP_00103_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data104[] = {
#include "baj2txt/IP_00104_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data105[] = {
#include "baj2txt/IP_00105_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data106[] = {
#include "baj2txt/IP_00106_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data107[] = {
#include "baj2txt/IP_00107_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data108[] = {
#include "baj2txt/IP_00108_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data109[] = {
#include "baj2txt/IP_00109_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data110[] = {
#include "baj2txt/IP_00110_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data111[] = {
#include "baj2txt/IP_00111_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data112[] = {
#include "baj2txt/IP_00112_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data113[] = {
#include "baj2txt/IP_00113_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data114[] = {
#include "baj2txt/IP_00114_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data115[] = {
#include "baj2txt/IP_00115_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data116[] = {
#include "baj2txt/IP_00116_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data117[] = {
#include "baj2txt/IP_00117_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data118[] = {
#include "baj2txt/IP_00118_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data119[] = {
#include "baj2txt/IP_00119_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data120[] = {
#include "baj2txt/IP_00120_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data121[] = {
#include "baj2txt/IP_00121_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data122[] = {
#include "baj2txt/IP_00122_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data123[] = {
#include "baj2txt/IP_00123_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data124[] = {
#include "baj2txt/IP_00124_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data125[] = {
#include "baj2txt/IP_00125_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data126[] = {
#include "baj2txt/IP_00126_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data127[] = {
#include "baj2txt/IP_00127_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data128[] = {
#include "baj2txt/IP_00128_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data129[] = {
#include "baj2txt/IP_00129_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data130[] = {
#include "baj2txt/IP_00130_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data131[] = {
#include "baj2txt/IP_00131_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data132[] = {
#include "baj2txt/IP_00132_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data133[] = {
#include "baj2txt/IP_00133_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data134[] = {
#include "baj2txt/IP_00134_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data135[] = {
#include "baj2txt/IP_00135_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data136[] = {
#include "baj2txt/IP_00136_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data137[] = {
#include "baj2txt/IP_00137_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data138[] = {
#include "baj2txt/IP_00138_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data139[] = {
#include "baj2txt/IP_00139_200x200_a4_q99_422.txt"
};

static const uint8_t baj_data140[] = {
#include "baj2txt/IP_00140_200x200_a4_q99_422.txt"
};
static const uint8_t baj_data141   [] = {
#include "baj2txt/IP_00141_200x200_a4_q99_422.txt"
};
static const uint8_t baj_data142[] = {
#include "baj2txt/IP_00142_200x200_a4_q99_422.txt"
};
static const uint8_t baj_data143[] = {
#include "baj2txt/IP_00143_200x200_a4_q99_422.txt"
};
static const uint8_t baj_data144[] = {
#include "baj2txt/IP_00144_200x200_a4_q99_422.txt"
};
static const uint8_t baj_data145[] = {
#include "baj2txt/IP_00145_200x200_a4_q99_422.txt"
};
static const uint8_t baj_data146[] = {
#include "baj2txt/IP_00146_200x200_a4_q99_422.txt"
};
static const uint8_t baj_data147[] = {
#include "baj2txt/IP_00147_200x200_a4_q99_422.txt"
};
static const uint8_t baj_data148[] = {
#include "baj2txt/IP_00148_200x200_a4_q99_422.txt"
};
static const uint8_t baj_data149[] = {
#include "baj2txt/IP_00149_200x200_a4_q99_422.txt"
};
#endif
#if IS_SUPPORT_BAJ_TEST_BG
static const uint8_t baj_data_bg[] = {
// #include "overlay/resources/img_baj_bin/bg_360x360_a4_q99_422.txt"
#include "overlay/resources/img_png_bin/home_bg_v2_360x360.txt"
};
#endif
static ui_img_data_t baj_images[BAJ_IMG_COUNT] = {
    { baj_data0, sizeof(baj_data0) },
    { baj_data1, sizeof(baj_data1) },
    { baj_data2, sizeof(baj_data2) },
    { baj_data3, sizeof(baj_data3) },
    { baj_data4, sizeof(baj_data4) },
    { baj_data5, sizeof(baj_data5) },
    { baj_data6, sizeof(baj_data6) },
    { baj_data7, sizeof(baj_data7) },
    { baj_data8, sizeof(baj_data8) },
    { baj_data9, sizeof(baj_data9) },
    { baj_data10, sizeof(baj_data10) },
    { baj_data11, sizeof(baj_data11) },
    { baj_data12, sizeof(baj_data12) },
    { baj_data13, sizeof(baj_data13) },
    { baj_data14, sizeof(baj_data14) },
    { baj_data15, sizeof(baj_data15) },
    { baj_data16, sizeof(baj_data16) },
    { baj_data17, sizeof(baj_data17) },
    { baj_data18, sizeof(baj_data18) },
    { baj_data19, sizeof(baj_data19) },
    { baj_data20, sizeof(baj_data20) },
    { baj_data21, sizeof(baj_data21) },
    { baj_data22, sizeof(baj_data22) },
    { baj_data23, sizeof(baj_data23) },
    { baj_data24, sizeof(baj_data24) },
    { baj_data25, sizeof(baj_data25) },
    { baj_data26, sizeof(baj_data26) },
    { baj_data27, sizeof(baj_data27) },
    { baj_data28, sizeof(baj_data28) },
    { baj_data29, sizeof(baj_data29) },
    { baj_data30, sizeof(baj_data30) },
    { baj_data31, sizeof(baj_data31) },
    { baj_data32, sizeof(baj_data32) },
    { baj_data33, sizeof(baj_data33) },
    { baj_data34, sizeof(baj_data34) },
    { baj_data35, sizeof(baj_data35) },
    { baj_data36, sizeof(baj_data36) },
    { baj_data37, sizeof(baj_data37) },
    { baj_data38, sizeof(baj_data38) },
    { baj_data39, sizeof(baj_data39) },
    { baj_data40, sizeof(baj_data40) },
    { baj_data41, sizeof(baj_data41) },
    { baj_data42, sizeof(baj_data42) },
    { baj_data43, sizeof(baj_data43) },
    { baj_data44, sizeof(baj_data44) },
    { baj_data45, sizeof(baj_data45) },
    { baj_data46, sizeof(baj_data46) },
    { baj_data47, sizeof(baj_data47) },
    { baj_data48, sizeof(baj_data48) },
    { baj_data49, sizeof(baj_data49) },
#if 1
    { baj_data50, sizeof(baj_data50) },
    { baj_data51, sizeof(baj_data51) },
    { baj_data52, sizeof(baj_data52) },
    { baj_data53, sizeof(baj_data53) },
    { baj_data54, sizeof(baj_data54) },
    { baj_data55, sizeof(baj_data55) },
    { baj_data56, sizeof(baj_data56) },
    { baj_data57, sizeof(baj_data57) },
    { baj_data58, sizeof(baj_data58) },
    { baj_data59, sizeof(baj_data59) },
    { baj_data60, sizeof(baj_data60) },
    { baj_data61, sizeof(baj_data61) },
    { baj_data62, sizeof(baj_data62) },
    { baj_data63, sizeof(baj_data63) },
    { baj_data64, sizeof(baj_data64) },
    { baj_data65, sizeof(baj_data65) },
    { baj_data66, sizeof(baj_data66) },
    { baj_data67, sizeof(baj_data67) },
    { baj_data68, sizeof(baj_data68) },
    { baj_data69, sizeof(baj_data69) },
    { baj_data70, sizeof(baj_data70) },
    { baj_data71, sizeof(baj_data71) },
    { baj_data72, sizeof(baj_data72) },
    { baj_data73, sizeof(baj_data73) },
    { baj_data74, sizeof(baj_data74) },
    { baj_data75, sizeof(baj_data75) },
    { baj_data76, sizeof(baj_data76) },
    { baj_data77, sizeof(baj_data77) },
    { baj_data78, sizeof(baj_data78) },
    { baj_data79, sizeof(baj_data79) },
    { baj_data80, sizeof(baj_data80) },
    { baj_data81, sizeof(baj_data81) },
    { baj_data82, sizeof(baj_data82) },
    { baj_data83, sizeof(baj_data83) },
    { baj_data84, sizeof(baj_data84) },
    { baj_data85, sizeof(baj_data85) },
    { baj_data86, sizeof(baj_data86) },
    { baj_data87, sizeof(baj_data87) },

    { baj_data88, sizeof(baj_data88) },
    { baj_data89, sizeof(baj_data89) },
    { baj_data90, sizeof(baj_data90) },
    { baj_data91, sizeof(baj_data91) },
    { baj_data92, sizeof(baj_data92) },
    { baj_data93, sizeof(baj_data93) },
    { baj_data94, sizeof(baj_data94) },
    { baj_data95, sizeof(baj_data95) },
    { baj_data96, sizeof(baj_data96) },
    { baj_data97, sizeof(baj_data97) },
    { baj_data98, sizeof(baj_data98) },
    { baj_data99, sizeof(baj_data99) },
    { baj_data100, sizeof(baj_data100) },
    { baj_data101, sizeof(baj_data101) },
    { baj_data102, sizeof(baj_data102) },
    { baj_data103, sizeof(baj_data103) },   
    { baj_data104, sizeof(baj_data104) },
    { baj_data105, sizeof(baj_data105) },
    { baj_data106, sizeof(baj_data106) },
    { baj_data107, sizeof(baj_data107) },
    { baj_data108, sizeof(baj_data108) },
    { baj_data109, sizeof(baj_data109) },
    { baj_data110, sizeof(baj_data110) },
    { baj_data111, sizeof(baj_data111) },
    { baj_data112, sizeof(baj_data112) },
    { baj_data113, sizeof(baj_data113) },
    { baj_data114, sizeof(baj_data114) },
    { baj_data115, sizeof(baj_data115) },
    { baj_data116, sizeof(baj_data116) },
    { baj_data117, sizeof(baj_data117) },
    { baj_data118, sizeof(baj_data118) },
    { baj_data119, sizeof(baj_data119) },
    { baj_data120, sizeof(baj_data120) },
    { baj_data121, sizeof(baj_data121) },
    { baj_data122, sizeof(baj_data122) },
    { baj_data123, sizeof(baj_data123) },
    { baj_data124, sizeof(baj_data124) },
    { baj_data125, sizeof(baj_data125) },
    { baj_data126, sizeof(baj_data126) },
    { baj_data127, sizeof(baj_data127) },
    { baj_data128, sizeof(baj_data128) },
    { baj_data129, sizeof(baj_data129) },
    { baj_data130, sizeof(baj_data130) },
    { baj_data131, sizeof(baj_data131) },
    { baj_data132, sizeof(baj_data132) },
    { baj_data133, sizeof(baj_data133) },
    { baj_data134, sizeof(baj_data134) },
    { baj_data135, sizeof(baj_data135) },
    { baj_data136, sizeof(baj_data136) },
    { baj_data137, sizeof(baj_data137) },
    { baj_data138, sizeof(baj_data138) },
    { baj_data139, sizeof(baj_data139) },
    { baj_data140, sizeof(baj_data140) },
    { baj_data141, sizeof(baj_data141) },
    { baj_data142, sizeof(baj_data142) },
    { baj_data143, sizeof(baj_data143) },
    { baj_data144, sizeof(baj_data144) },
    { baj_data145, sizeof(baj_data145) },
    { baj_data146, sizeof(baj_data146) },
    { baj_data147, sizeof(baj_data147) },
    { baj_data148, sizeof(baj_data148) },
    { baj_data149, sizeof(baj_data149) },
 #endif
};

const static uint16_t baj_images_ids[BAJ_IMG_COUNT] = {
    RESOURCE_ID_PERSONALIZATION_MODE,
    RESOURCE_ID_PP_MODE,
    RESOURCE_ID_MILK_MODE,
    RESOURCE_ID_INITIATING_MODE,
    RESOURCE_ID_PERSONALIZATION_MODE,
    RESOURCE_ID_PP_MODE,
};
res_img_info_t baj_infos;

#if IS_SUPPORT_BAJ_TEST_BG
static lv_obj_t *logo_bg;
static lv_draw_buf_t *logo_bg_snapshot = NULL;  /// 背景快照，避免重复解码
#endif

#endif

static uint8_t logo_force_update = false;
static lv_obj_t *logo_root;
static res_img_info_t logo_img_info;
static uint32_t start_time = 0;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_PNG_TEST
static int system_test_png_init(void)
{
    if (logo_root == NULL || !lv_obj_is_valid(logo_root)) {
        logo_root = lv_image_create(lv_screen_active());
    }
    lv_fs_path_ex_t png_path;
    lv_fs_make_path_from_buffer(&png_path, LV_FS_MEMFS_LETTER, png_data0, sizeof(png_data0), "png");
    lv_image_set_src(logo_root, (const void *)&png_path);
    lv_obj_align(logo_root, LV_ALIGN_CENTER, 0, 0);
    return 0;
}

static int system_test_png_process(void)
{
    static uint32_t index = 0;
    lv_fs_path_ex_t png_path;
    const uint8_t *png_data = png_images[index % PNG_IMG_COUNT].data;
    uint32_t png_size = png_images[index % PNG_IMG_COUNT].size;
    lv_fs_make_path_from_buffer(&png_path, LV_FS_MEMFS_LETTER, png_data, png_size, "png");
    lv_image_set_src(logo_root, (const void *)&png_path);
#if 1
    LOG_UI("===> test 7 png: %d", index);
    if (get_timestamp() - start_time > 5000) {
        start_time = get_timestamp();
        index++;
        if (index > 2) {
            index = 0;
        }
    }
#else
    index++;
#endif
    return 0;
}
#endif

#if IS_SUPPORT_BAJ_TEST
static int system_test_baj_init(ui_manager_context_t *thiz)
{
    lv_lock();  /// 保护所有 LVGL 函数调用，防止多线程竞争导致堆损坏
#if IS_SUPPORT_BAJ_TEST_BG
    if (logo_bg == NULL || !lv_obj_is_valid(logo_bg)) {
        logo_bg = lv_image_create(lv_screen_active());
    }
    
    if (logo_bg_snapshot != NULL) {
        lv_draw_buf_destroy(logo_bg_snapshot);
        logo_bg_snapshot = NULL;
    }

    lv_fs_path_ex_t bg_path;
    // lv_fs_make_path_from_buffer(&bg_path, LV_FS_MEMFS_LETTER, baj_data_bg, sizeof(baj_data_bg), "baj");
    lv_fs_make_path_from_buffer(&bg_path, LV_FS_MEMFS_LETTER, baj_data_bg, sizeof(baj_data_bg), "png");
    lv_image_set_src(logo_bg, (const void *)&bg_path);
    lv_obj_align(logo_bg, LV_ALIGN_CENTER, 0, 0);
    lv_refr_now(NULL); /// 等待渲染完成后再创建快照
#if LV_USE_SNAPSHOT
    logo_bg_snapshot = lv_snapshot_take(logo_bg, LV_COLOR_FORMAT_NATIVE_WITH_ALPHA);
#else
    logo_bg_snapshot = NULL;
    LOGE("===> LV_USE_SNAPSHOT is not enabled");
#endif
    if (logo_bg_snapshot != NULL) {
        lv_image_set_src(logo_bg, logo_bg_snapshot);
    } else {
        LOGE("===> logo_bg snapshot creation failed");
    }

    if (logo_root == NULL || !lv_obj_is_valid(logo_root)) {
        logo_root = lv_image_create(logo_bg);
    }
#else
    if (logo_root == NULL || !lv_obj_is_valid(logo_root)) {
        logo_root = lv_image_create(lv_screen_active());
    }
    lv_obj_set_style_bg_color(logo_root, lv_color_white(), 0);     /// 设置背景颜色为白色
#endif

    lv_fs_path_ex_t baj_path;
#if IS_SUPPORT_BAJ_RES
    common_ui_img_resource_init(thiz, baj_images_ids[0], &baj_infos);
    lv_fs_make_path_from_buffer(&baj_path, LV_FS_MEMFS_LETTER, baj_infos.data, baj_infos.size, "baj");
#else
    lv_fs_make_path_from_buffer(&baj_path, LV_FS_MEMFS_LETTER, baj_data0, sizeof(baj_data0), "baj");
#endif
    lv_image_set_src(logo_root, (const void *)&baj_path);
    lv_obj_align(logo_root, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_unlock();  /// 释放锁

    return 0;
}

static int system_test_baj_process(ui_manager_context_t *thiz)
{
    static uint32_t index = 0;

    lv_lock();  /// 保护所有 LVGL 函数调用，防止多线程竞争导致堆损坏
#if IS_SUPPORT_BAJ_TEST_BG
    if (logo_bg_snapshot != NULL) {
        lv_image_set_src(logo_bg, logo_bg_snapshot);
    }
#endif

    lv_fs_path_ex_t baj_path;
#if IS_SUPPORT_BAJ_RES
    common_ui_img_resource_init(thiz, baj_images_ids[index % BAJ_IMG_COUNT], &baj_infos);
    lv_fs_make_path_from_buffer(&baj_path, LV_FS_MEMFS_LETTER, baj_infos.data, baj_infos.size, "baj");
#else
    const uint8_t *baj_data = baj_images[index % BAJ_IMG_COUNT].data;
    uint32_t baj_size = baj_images[index % BAJ_IMG_COUNT].size;
    lv_fs_make_path_from_buffer(&baj_path, LV_FS_MEMFS_LETTER, baj_data, baj_size, "baj");
#endif
    lv_image_set_src(logo_root, (const void *)&baj_path);
    lv_unlock();  /// 释放锁
    index++;

    return 0;
}
#endif


#if IS_SUPPORT_BG_TEST
static uint32_t rgb_color_change_time = 0;
static int system_lcd_test_init(void)
{
    /// 设置初始背景色为红色
    lv_obj_t *scr = lv_screen_active();
    if (scr != NULL && lv_obj_is_valid(scr)) {
        lv_lock();
        lv_obj_set_style_bg_color(scr, lv_color_hex(0xFF0000), LV_PART_MAIN);
        lv_unlock();
    }
    rgb_color_change_time = get_timestamp();
    return 0;
}

static int system_lcd_test_process(void)
{
    /// 五色交替：每 1 秒切换一次颜色（红、绿、蓝、黑、白）
    static uint8_t color_index = 0;  /// 0=红色, 1=绿色, 2=蓝色, 3=黑色, 4=白色
    uint32_t color_colors[5] = {0xFF0000, 0x00FF00, 0x0000FF, 0x000000, 0xFFFFFF};  /// 红、绿、蓝、黑、白
    
    if (get_timestamp() - rgb_color_change_time > 1000) {
        rgb_color_change_time = get_timestamp();
        color_index = (color_index + 1) % 5;
        
        /// 添加对象有效性检查和线程安全保护
        /// 注意：在 VGLite 渲染线程正在工作时修改对象可能导致崩溃
        /// 使用 lv_refr_now() 确保重绘完成，避免与 VGLite 渲染线程冲突
        lv_obj_t *scr = lv_screen_active();
        if (scr != NULL && lv_obj_is_valid(scr)) {
            lv_lock();
            lv_obj_set_style_bg_color(scr, lv_color_hex(color_colors[color_index]), LV_PART_MAIN);
            /// 立即触发重绘并等待完成，确保 VGLite 渲染线程有时间完成当前任务
            /// 这样可以避免在 GPU 等待中断期间修改对象导致的崩溃
            lv_refr_now(NULL);
            lv_unlock();
            
            LOG_UI("===> lcd test color: %d", color_index);
        } else {
            LOGE("===> lcd test: invalid screen object");
        }
    }
    return 0;
}
#endif

static void sys_logo_label_update(ui_manager_context_t *thiz)
{
    common_ui_img_resource_init(thiz, RESOURCE_ID_START_LOGO_ICON, &logo_img_info);
    EXCEPTION_CHECK_VOID(logo_img_info.data);
    if (logo_root == NULL || !lv_obj_is_valid(logo_root)) {
        logo_root = lv_image_create(lv_screen_active());
    }
    lv_obj_set_size(logo_root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(logo_root, lv_color_white(), 0);     /// 设置背景颜色为白色
    lv_obj_set_style_bg_opa(logo_root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(logo_root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(logo_root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_fs_path_ex_t logo_img_path;
    lv_fs_make_path_from_buffer(&logo_img_path, LV_FS_MEMFS_LETTER, logo_img_info.data,
        logo_img_info.size, "baj");
    lv_image_set_src(logo_root, (const void *)&logo_img_path);
    lv_obj_align(logo_root, LV_ALIGN_CENTER, 0, 0);    /// 设置模式标签位置
    lv_obj_clear_flag(logo_root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(logo_root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_logo_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
#if IS_SUPPORT_PNG_TEST
    system_test_png_init();
#elif IS_SUPPORT_BG_TEST
    system_lcd_test_init();
#elif IS_SUPPORT_BAJ_TEST
    system_test_baj_init(thiz);
#endif
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    logo_force_update = true;
}

void ui_display_logo_remove(ui_manager_context_t *thiz)
{
    LVGL_OBJ_DELETE(logo_root);
#if IS_SUPPORT_BAJ_TEST_BG
    if (logo_bg_snapshot != NULL) {
        lv_draw_buf_destroy(logo_bg_snapshot);
        logo_bg_snapshot = NULL;
    }
    LVGL_OBJ_DELETE(logo_bg);
#endif
#if IS_SUPPORT_PNG_TEST || IS_SUPPORT_BG_TEST || IS_SUPPORT_BAJ_TEST
#else
    common_ui_img_resource_deinit(thiz, &logo_img_info);
#endif

}

int ui_display_logo_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
#if IS_SUPPORT_PNG_TEST
    system_test_png_process();
#elif IS_SUPPORT_BG_TEST
    system_lcd_test_process();
#elif IS_SUPPORT_BAJ_TEST
    system_test_baj_process(thiz);
#else
    if (logo_force_update == true) {
        logo_force_update = false;
        sys_logo_label_update(thiz);
    }   
#endif
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> logo fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
