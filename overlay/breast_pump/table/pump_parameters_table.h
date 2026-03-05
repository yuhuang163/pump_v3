#ifndef __PUMP_TABLE_H__
#define __PUMP_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "motor_policy.h"
#include "persist_storage.h"

#define ONE_MIN_TIME_MS                    60000  /// 1分钟时间，单位ms
#define SHORT_SUCTION_HOLD_TIME            0  /// 短吸保持时间，单位ms
#define LONG_SUCTION_HOLD_TIME             0 /// 长吸保持时间，单位ms
#define SHORT_SUCTION_TIME_0               ((ONE_MIN_TIME_MS / 90 / 10)   * 10)  /// 1次吸奶时间，单位ms
#define SHORT_SUCTION_TIME_1               ((ONE_MIN_TIME_MS / 100 / 10)  * 10)  /// 1次吸奶时间，单位ms
#define SHORT_SUCTION_TIME_2               ((ONE_MIN_TIME_MS / 110 / 10)  * 10)  /// 1次吸奶时间，单位ms
#define PUMP_SLEEP_TIME_MAX                2000  /// 最大休眠时间，单位ms
#define PUMP_SLEEP_TIME_MIN                20  /// 最小休眠时间，单位ms

/// 短吸吸奶时间表
static const uint16_t short_suction_time_table[SUCTION_SPEED_MAX] = {
    SHORT_SUCTION_TIME_0, 
    SHORT_SUCTION_TIME_1, 
    SHORT_SUCTION_TIME_2, 
};

/// 吸力参数表（单位kpa）//泌乳力表
static const suction_parameters_t long_suction_table[SUCTION_LEVEL_MAX] = {
    {0,  60, 100}, /// 0KPA   
    {20, 370, 80}, /// 6KPA     
    {50, 80, 100}, /// 7KPA      
    {50, 80, 100}, /// 8KPA   
    {55, 90, 100}, /// 9KPA     
    {28, 400, 130}, /// 10KPA   
    {30, 393, 87}, /// 11KPA      
    {60, 130, 100}, /// 12KPA   
    {33, 420, 140}, /// 13KPA   
    {60, 160, 100}, /// 14KPA      
    {38, 440, 140}, /// 15KPA   
    {60, 200, 120}, /// 16KPA   
    {70, 200, 120}, /// 17KPA       
    {42, 468, 140}, /// 18KPA   
    {70, 240, 120}, /// 19KPA   
    {46, 485, 160}, /// 20KPA      
    {52, 500, 125}, /// 21KPA   
    {50, 500, 170}, /// 22KPA   
    {75, 320, 120}, /// 23KPA   
    {51, 540, 180}, /// 24KPA   
    {75, 360, 120}, /// 25KPA   
    {56, 560, 200}, /// 26KPA      
    {75, 410, 150}, /// 27KPA   
    {58, 580, 220}, /// 28KPA   
    {80, 470, 160}, /// 29KPA   
    {64, 600, 230}, /// 30KPA  
    {65, 630, 230}, /// 31KPA   
    {72, 659, 165}, /// 32KPA   
    {67, 660, 240}, /// 33KPA   
    {75, 686, 171}, /// 34KPA   
    {67, 715, 300}, /// 35KPA   
    {70, 720, 300}, /// 36KPA   
    {90, 720, 200}, /// 37KPA   
    {80, 783, 196}, /// 38KPA   
    {83, 818, 205}, /// 39KPA
    {83, 857, 214}, /// 40KPA             
};

/// 吸力参数表（单位kpa）//按摩力表
static const suction_parameters_t short_suction_table[SUCTION_LEVEL_MAX] = {
    {0,  60, 100}, /// 0KPA   
    {26, 270, 110}, /// 6KPA   
    {30, 275, 105}, /// 7KPA      
    {50, 80, 100}, /// 8KPA   
    {34, 280, 100}, /// 9KPA     
    {39, 275, 105}, /// 10KPA   
    {60, 110, 100}, /// 11KPA      
    {44, 263, 100}, /// 12KPA   
    {48, 275, 135}, /// 13KPA   
    {47, 315, 120}, /// 14KPA      
    {52, 300, 130}, /// 15KPA   
    {60, 200, 120}, /// 16KPA   
    {68, 280, 140}, /// 17KPA      
    {65, 311, 130}, /// 18KPA   
    {74, 285, 150}, /// 19KPA   
    {71, 328, 130}, /// 20KPA      
    {75, 328, 130}, /// 21KPA   
    {74, 345, 120}, /// 22KPA   
    {75, 350, 140}, /// 23KPA   
    {75, 340, 120}, /// 24KPA   
    {75, 360, 120}, /// 25KPA   
    {75, 390, 150}, /// 26KPA      
    {75, 410, 150}, /// 27KPA   
    {80, 440, 150}, /// 28KPA   
    {80, 470, 160}, /// 29KPA   
    {80, 500, 160}, /// 30KPA  
    {80, 530, 160}, /// 31KPA   
    {80, 550, 160}, /// 32KPA   
    {85, 580, 200}, /// 33KPA   
    {85, 620, 200}, /// 34KPA   
    {85, 660, 200}, /// 35KPA   
    {90, 680, 200}, /// 36KPA   
    {90, 720, 200}, /// 37KPA   
    {90, 760, 220}, /// 38KPA   
    {90, 810, 220}, /// 39KPA         
    {90, 850, 220}, /// 40KPA         
};

/// 长吸泵频次表 (泵频次 1分/次)
static const uint16_t long_pump_freq_table[BREAST_PUMP_LEVEL_MAX][SUCTION_SPEED_MAX] = {
    /// 低，中，高     
    {90, 100, 110}, /// level 0      
    {72, 76,  80},  /// level 1      
    {68, 72,  76 }, /// level 2  
    {65, 69,  73 }, /// level 3
    {62, 66,  70 }, /// level 4    
    {60, 64,  68 }, /// level 5 
    {58, 62,  66 }, /// level 6    
    {56, 60,  64 }, /// level 7     
    {54, 58,  62 }, /// level 8   
    {52, 56,  60 }, /// level 9  
    {50, 54,  58 }, /// level 10   
    {48, 52,  56 }, /// level 11  
    {46, 50,  54 }, /// level 12     
    {44, 48,  52 }, /// level 13 
    {42, 46,  50 }, /// level 14
    {40, 44,  48 }, /// level 15 
};

/// 短吸力度表 //按摩力表
static const uint16_t short_level_table[BREAST_PUMP_LEVEL_MAX] = {
    SUCTION_LEVEL_0KPA, /// 0KPA
    SUCTION_LEVEL_6KPA, /// 6KPA
    SUCTION_LEVEL_7KPA, /// 7.4KPA
    SUCTION_LEVEL_9KPA, /// 8.8KPA
    SUCTION_LEVEL_10KPA, /// 10.2KPA
    SUCTION_LEVEL_12KPA, /// 11.5KPA
    SUCTION_LEVEL_13KPA, /// 12.8KPA
    SUCTION_LEVEL_14KPA, /// 14.1KPA
    SUCTION_LEVEL_15KPA, /// 15.3KPA
    SUCTION_LEVEL_17KPA, /// 16.5KPA
    SUCTION_LEVEL_18KPA, /// 17.7KPA
    SUCTION_LEVEL_19KPA, /// 18.8KPA
    SUCTION_LEVEL_20KPA, /// 19.9KPA
    SUCTION_LEVEL_21KPA, /// 21KPA
    SUCTION_LEVEL_22KPA, /// 22KPA
    SUCTION_LEVEL_23KPA, /// 23KPA
};

/// 长吸力度表 //泌乳力表
static const uint16_t long_level_table[BREAST_PUMP_LEVEL_MAX] = {
    SUCTION_LEVEL_0KPA,  /// 0KPA
    SUCTION_LEVEL_6KPA, /// 6KPA
    SUCTION_LEVEL_10KPA, /// 9.5KPA
    SUCTION_LEVEL_13KPA, /// 12.5KPA
    SUCTION_LEVEL_15KPA, /// 15KPA
    SUCTION_LEVEL_18KPA, /// 17.5KPA
    SUCTION_LEVEL_20KPA, /// 19.5KPA
    SUCTION_LEVEL_22KPA, /// 21.5KPA
    SUCTION_LEVEL_24KPA, /// 23.5KPA
    SUCTION_LEVEL_26KPA, /// 25.5KPA
    SUCTION_LEVEL_28KPA, /// 27.5KPA
    SUCTION_LEVEL_30KPA, /// 29.5KPA
    SUCTION_LEVEL_31KPA, /// 31.3KPA
    SUCTION_LEVEL_33KPA, /// 32.9KPA
    SUCTION_LEVEL_35KPA, /// 34.5KPA
    SUCTION_LEVEL_36KPA, /// 36KPA
};

#ifdef __cplusplus
}
#endif

#endif 