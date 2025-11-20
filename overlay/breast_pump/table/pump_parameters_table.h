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
#define SHORT_SUCTION_HOLD_TIME            60  /// 短吸保持时间，单位ms
#define LONG_SUCTION_HOLD_TIME             100 /// 长吸保持时间，单位ms
#define SHORT_SUCTION_TIME_0               ((ONE_MIN_TIME_MS / 90 / 10)   * 10)  /// 1次吸奶时间，单位ms
#define SHORT_SUCTION_TIME_1               ((ONE_MIN_TIME_MS / 100 / 10)  * 10)  /// 1次吸奶时间，单位ms
#define SHORT_SUCTION_TIME_2               ((ONE_MIN_TIME_MS / 110 / 10)  * 10)  /// 1次吸奶时间，单位ms
#define PUMP_SLEEP_TIME_MAX                1000  /// 最大休眠时间，单位ms
#define PUMP_SLEEP_TIME_MIN                20  /// 最小休眠时间，单位ms

/// 短吸吸奶时间表
static const uint16_t short_suction_time_table[SUCTION_SPEED_MAX] = {
    SHORT_SUCTION_TIME_0, 
    SHORT_SUCTION_TIME_1, 
    SHORT_SUCTION_TIME_2, 
};

/// 吸力参数表（单位kpa）
static const suction_parameters_t suction_table[SUCTION_LEVEL_MAX] = {
    {0,  60, 100}, /// 0KPA   
    {40, 60, 100}, /// 6KPA   
    {50, 80, 100}, /// 7KPA      
    {50, 80, 100}, /// 8KPA   
    {55, 90, 100}, /// 9KPA     
    {60, 100, 100}, /// 10KPA   
    {60, 110, 100}, /// 11KPA      
    {60, 130, 100}, /// 12KPA   
    {60, 150, 100}, /// 13KPA   
    {60, 160, 100}, /// 14KPA      
    {60, 180, 120}, /// 15KPA   
    {60, 200, 120}, /// 16KPA   
    {70, 200, 120}, /// 17KPA      
    {70, 220, 120}, /// 18KPA   
    {70, 240, 120}, /// 19KPA   
    {75, 260, 120}, /// 20KPA      
    {75, 280, 120}, /// 21KPA   
    {75, 300, 120}, /// 22KPA   
    {75, 320, 120}, /// 23KPA   
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
};

/// 长吸泵频次表 (泵频次 1分/次)
static const uint16_t long_pump_freq_table[BREAST_PUMP_LEVEL_MAX][SUCTION_SPEED_MAX] = {
    /// 低，中，高     
    {90, 100, 110}, /// level 0      
    {78, 83,  88},  /// level 1      
    {75, 80,  85 }, /// level 2  
    {72, 77,  82 }, /// level 3
    {69, 74,  79 }, /// level 4    
    {66, 71,  76 }, /// level 5 
    {63, 68,  73 }, /// level 6    
    {60, 65,  70 }, /// level 7     
    {57, 62,  67 }, /// level 8   
    {54, 59,  64 }, /// level 9  
    {51, 56,  61 }, /// level 10   
    {48, 53,  58 }, /// level 11  
    {45, 50,  55 }, /// level 12     
    {40, 45,  50 }, /// level 13 
    {35, 40,  45 }, /// level 14
    {30, 35,  40 }, /// level 15 
};

/// 短吸力度表 
static const uint16_t short_level_table[BREAST_PUMP_LEVEL_MAX] = {
    SUCTION_LEVEL_0KPA, /// 0KPA
    SUCTION_LEVEL_6KPA, /// 6.1KPA
    SUCTION_LEVEL_7KPA, /// 7.5KPA
    SUCTION_LEVEL_9KPA, /// 9KPA
    SUCTION_LEVEL_10KPA, /// 10.5KPA
    SUCTION_LEVEL_12KPA, /// 12KPA
    SUCTION_LEVEL_13KPA, /// 13.5KPA
    SUCTION_LEVEL_14KPA, /// 14.7KPA
    SUCTION_LEVEL_15KPA, /// 15.5KPA
    SUCTION_LEVEL_16KPA, /// 16.4KPA
    SUCTION_LEVEL_18KPA, /// 17.8KPA
    SUCTION_LEVEL_19KPA, /// 19.1KPA
    SUCTION_LEVEL_20KPA, /// 19.9KPA
    SUCTION_LEVEL_21KPA, /// 20.9KPA
    SUCTION_LEVEL_22KPA, /// 22KPA
    SUCTION_LEVEL_23KPA, /// 23KPA
};

/// 长吸力度表 
static const uint16_t long_level_table[BREAST_PUMP_LEVEL_MAX] = {
    SUCTION_LEVEL_0KPA,  /// 0KPA
    SUCTION_LEVEL_12KPA, /// 12.1KPA
    SUCTION_LEVEL_14KPA, /// 14.3KPA
    SUCTION_LEVEL_15KPA, /// 15.1KPA
    SUCTION_LEVEL_16KPA, /// 16.1KPA
    SUCTION_LEVEL_17KPA, /// 17.5KPA
    SUCTION_LEVEL_19KPA, /// 19.1KPA
    SUCTION_LEVEL_21KPA, /// 21.3KPA
    SUCTION_LEVEL_24KPA, /// 23.7KPA
    SUCTION_LEVEL_26KPA, /// 26KPA
    SUCTION_LEVEL_28KPA, /// 27.7KPA
    SUCTION_LEVEL_30KPA, /// 30.1KPA
    SUCTION_LEVEL_32KPA, /// 31.7KPA
    SUCTION_LEVEL_34KPA, /// 33.9KPA
    SUCTION_LEVEL_36KPA, /// 35.8KPA
    SUCTION_LEVEL_38KPA, /// 38.1KPA
};

#ifdef __cplusplus
}
#endif

#endif 