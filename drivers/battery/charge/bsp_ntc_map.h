#ifndef __BSP_NTC_MAP_H__
#define __BSP_NTC_MAP_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "stdint.h" // IWYU pragma: keep

typedef struct {
    int16_t temp;      /// Temperature in ℃
    float r_min;       /// Minimum resistance in kΩ
    float r_max;       /// Maximum resistance in kΩ
} ntc_temp_resistance_t;

/// NTC temperature to resistance lookup table
/// Resistance values are in kΩ units (non-overlapping ranges based on center values)
static const ntc_temp_resistance_t ntc_temp_table[] = {
    {-19, 67.550f, 70.961f},        // 65.634 center
    {-18, 64.324f, 67.549f},        // 62.529 center
    {-17, 61.270f, 64.323f},        // 59.589 center
    {-16, 58.378f, 61.269f},        // 56.804 center
    {-15, 55.641f, 58.377f},        // 54.166 center
    {-14, 53.047f, 55.640f},        // 51.665 center
    {-13, 50.589f, 53.046f},        // 49.294 center
    {-12, 48.259f, 50.588f},        // 47.046 center
    {-11, 46.050f, 48.258f},        // 44.913 center
    {-10, 43.954f, 46.049f},        // 42.889 center
    {-9, 41.965f, 43.953f},         // 40.967 center
    {-8, 40.078f, 41.964f},         // 39.142 center
    {-7, 38.285f, 40.077f},         // 37.408 center
    {-6, 36.583f, 38.284f},         // 35.761 center
    {-5, 34.966f, 36.582f},         // 34.196 center
    {-4, 33.429f, 34.965f},         // 32.707 center
    {-3, 31.967f, 33.428f},         // 31.291 center
    {-2, 30.579f, 31.966f},         // 29.945 center
    {-1, 29.258f, 30.578f},         // 28.664 center
    {0, 28.002f, 29.257f},          // 27.445 center
    {1, 26.805f, 28.001f},          // 26.283 center
    {2, 25.666f, 26.804f},          // 25.177 center
    {3, 24.582f, 25.665f},          // 24.124 center
    {4, 23.550f, 24.581f},          // 23.121 center
    {5, 22.567f, 23.549f},          // 22.165 center
    {6, 21.629f, 22.566f},          // 21.253 center
    {7, 20.736f, 21.628f},          // 20.384 center
    {8, 19.884f, 20.735f},          // 19.555 center
    {9, 19.072f, 19.883f},          // 18.764 center
    {10, 18.298f, 19.071f},         // 18.010 center
    {11, 17.560f, 18.297f},         // 17.290 center
    {12, 16.854f, 17.559f},         // 16.602 center
    {13, 16.182f, 16.853f},         // 15.946 center
    {14, 15.539f, 16.181f},         // 15.319 center
    {15, 14.926f, 15.538f},         // 14.720 center
    {16, 14.340f, 14.925f},         // 14.148 center
    {17, 13.780f, 14.339f},         // 13.601 center
    {18, 13.245f, 13.779f},         // 13.078 center
    {19, 12.734f, 13.244f},         // 12.578 center
    {20, 12.244f, 12.733f},         // 12.099 center
    {21, 11.777f, 12.243f},         // 11.642 center
    {22, 11.330f, 11.776f},         // 11.204 center
    {23, 10.902f, 11.329f},         // 10.785 center
    {24, 10.493f, 10.901f},         // 10.384 center
    {25, 10.101f, 10.492f},         // 10.000 center
    {26, 9.733f, 10.100f},          // 9.632 center
    {27, 9.381f, 9.732f},           // 9.280 center
    {28, 9.043f, 9.380f},           // 8.943 center
    {29, 8.719f, 9.042f},           // 8.619 center
    {30, 8.409f, 8.718f},           // 8.309 center
    {31, 8.111f, 8.408f},           // 8.012 center
    {32, 7.825f, 8.110f},           // 7.727 center
    {33, 7.551f, 7.824f},           // 7.453 center
    {34, 7.288f, 7.550f},           // 7.191 center
    {35, 7.035f, 7.287f},           // 6.939 center
    {36, 6.793f, 7.034f},           // 6.698 center
    {37, 6.560f, 6.792f},           // 6.466 center
    {38, 6.336f, 6.559f},           // 6.243 center
    {39, 6.121f, 6.335f},           // 6.029 center
    {40, 5.915f, 6.120f},           // 5.824 center
    {41, 5.717f, 5.914f},           // 5.627 center
    {42, 5.526f, 5.716f},           // 5.437 center
    {43, 5.343f, 5.525f},           // 5.255 center
    {44, 5.166f, 5.342f},           // 5.080 center
    {45, 4.997f, 5.165f},           // 4.911 center
    {46, 4.833f, 4.996f},           // 4.749 center
    {47, 4.676f, 4.832f},           // 4.593 center
    {48, 4.525f, 4.675f},           // 4.443 center
    {49, 4.380f, 4.524f},           // 4.299 center
    {50, 4.240f, 4.379f},           // 4.160 center
    {51, 4.105f, 4.239f},           // 4.027 center
    {52, 3.975f, 4.104f},           // 3.898 center
    {53, 3.850f, 3.974f},           // 3.774 center
    {54, 3.729f, 3.849f},           // 3.654 center
    {55, 3.613f, 3.728f},           // 3.539 center
    {56, 3.501f, 3.612f},           // 3.429 center
    {57, 3.393f, 3.500f},           // 3.322 center
    {58, 3.289f, 3.392f},           // 3.219 center
    {59, 3.189f, 3.288f},           // 3.119 center
};

// Get the table size
#define NTC_TEMP_TABLE_SIZE (sizeof(ntc_temp_table) / sizeof(ntc_temp_table[0]))


#ifdef __cplusplus
}
#endif

#endif 
