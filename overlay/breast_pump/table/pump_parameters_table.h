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
#define IS_USE_NEW_SUCTION                 0 /// 是否使用新的吸力参数表 //时光的吸力参数表

/// 短吸吸奶时间表
static const uint16_t short_suction_time_table[SUCTION_SPEED_MAX] = {
    SHORT_SUCTION_TIME_0, 
    SHORT_SUCTION_TIME_1, 
    SHORT_SUCTION_TIME_2, 
};

#if IS_USE_NEW_SUCTION
/// 吸力参数表（单位kpa）//泌乳力表
static const suction_parameters_t long_suction_table[SUCTION_LEVEL_MAX] = {
    {0,  60, 100}, /// 0KPA   
    {29, 340, 100}, /// 6KPA     
    {50, 80, 100}, /// 7KPA      
    {50, 80, 100}, /// 8KPA   
    {55, 90, 100}, /// 9KPA     
    {34, 406, 100}, /// 10KPA   
    {30, 393, 87}, /// 11KPA      
    {60, 130, 100}, /// 12KPA   
    {38, 452, 120}, /// 13KPA   
    {60, 160, 100}, /// 14KPA      
    {43, 472, 120}, /// 15KPA   
    {60, 200, 120}, /// 16KPA   
    {70, 200, 120}, /// 17KPA       
    {48, 492, 140}, /// 18KPA   
    {70, 240, 120}, /// 19KPA   
    {50, 528, 140}, /// 20KPA      
    {52, 500, 125}, /// 21KPA   
    {53, 560, 180}, /// 22KPA   
    {75, 320, 120}, /// 23KPA   
    {54, 622, 200}, /// 24KPA   
    {75, 360, 120}, /// 25KPA   
    {58, 632, 220}, /// 26KPA      
    {75, 410, 150}, /// 27KPA   
    {60, 660, 220}, /// 28KPA   
    {80, 470, 160}, /// 29KPA   
    {65, 647, 250}, /// 30KPA  
    {66, 705, 260}, /// 31KPA   
    {72, 659, 165}, /// 32KPA   
    {71, 684, 280}, /// 33KPA   
    {75, 686, 171}, /// 34KPA   
    {72, 718, 320}, /// 35KPA   
    {76, 708, 330}, /// 36KPA   
    {90, 720, 200}, /// 37KPA   
    {80, 783, 196}, /// 38KPA   
    {83, 818, 205}, /// 39KPA
    {83, 857, 214}, /// 40KPA             
};

/// 吸力参数表（单位kpa）//按摩力表
static const suction_parameters_t short_suction_table[SUCTION_LEVEL_MAX] = {
    {0,  60, 100}, /// 0KPA   
    {34, 220, 130}, /// 6KPA   
    {38, 230, 130}, /// 7KPA      
    {50, 80, 100}, /// 8KPA   
    {45, 230, 145}, /// 9KPA     
    {50, 240, 140}, /// 10KPA   
    {60, 110, 100}, /// 11KPA      
    {55, 245, 150}, /// 12KPA   
    {56, 270, 150}, /// 13KPA   
    {63, 264, 150}, /// 14KPA      
    {64, 287, 170}, /// 15KPA   
    {60, 200, 120}, /// 16KPA   
    {72, 272, 155}, /// 17KPA      
    {80, 291, 160}, /// 18KPA   
    {76, 300, 145}, /// 19KPA   
    {89, 300, 165}, /// 20KPA      
    {85, 314, 160}, /// 21KPA   
    {90, 330, 165}, /// 22KPA   
    {89, 328, 160}, /// 23KPA   
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
#else
/// 吸力参数表（单位kpa）//泌乳力表
static const suction_parameters_t long_suction_table[SUCTION_LEVEL_MAX] = {
    {0,  60, 100}, /// 0KPA   
    {18, 370, 80}, /// 6KPA     
    {50, 80, 100}, /// 7KPA      
    {50, 80, 100}, /// 8KPA   
    {55, 90, 100}, /// 9KPA     
    {23, 404, 126}, /// 10KPA   
    {30, 393, 87}, /// 11KPA      
    {60, 130, 100}, /// 12KPA   
    {29, 425, 140}, /// 13KPA   
    {60, 160, 100}, /// 14KPA      
    {34, 440, 140}, /// 15KPA   
    {60, 200, 120}, /// 16KPA   
    {70, 200, 120}, /// 17KPA       
    {37, 468, 140}, /// 18KPA   
    {70, 240, 120}, /// 19KPA   
    {41, 475, 170}, /// 20KPA      
    {52, 500, 125}, /// 21KPA   
    {45, 490, 180}, /// 22KPA   
    {75, 320, 120}, /// 23KPA   
    {45, 540, 180}, /// 24KPA   
    {75, 360, 120}, /// 25KPA   
    {51, 548, 230}, /// 26KPA      
    {75, 410, 150}, /// 27KPA   
    {55, 560, 250}, /// 28KPA   
    {80, 470, 160}, /// 29KPA   
    {57, 600, 230}, /// 30KPA  
    {61, 620, 240}, /// 31KPA   
    {72, 659, 165}, /// 32KPA   
    {61, 650, 250}, /// 33KPA   
    {75, 686, 171}, /// 34KPA   
    {63, 685, 330}, /// 35KPA   
    {64, 710, 320}, /// 36KPA   
    {90, 720, 200}, /// 37KPA   
    {80, 783, 196}, /// 38KPA   
    {83, 818, 205}, /// 39KPA
    {83, 857, 214}, /// 40KPA             
};

/// 吸力参数表（单位kpa）//按摩力表
static const suction_parameters_t short_suction_table[SUCTION_LEVEL_MAX] = {
    {0,  60, 100}, /// 0KPA   
    {22, 270, 110}, /// 6KPA   
    {27, 245, 130}, /// 7KPA      
    {50, 80, 100}, /// 8KPA   
    {31, 270, 110}, /// 9KPA     
    {33, 270, 110}, /// 10KPA   
    {60, 110, 100}, /// 11KPA      
    {41, 260, 105}, /// 12KPA   
    {38, 303, 140}, /// 13KPA   
    {46, 290, 140}, /// 14KPA      
    {47, 302, 130}, /// 15KPA   
    {60, 200, 120}, /// 16KPA   
    {56, 285, 140}, /// 17KPA      
    {55, 305, 140}, /// 18KPA   
    {66, 285, 150}, /// 19KPA   
    {68, 305, 140}, /// 20KPA      
    {69, 313, 140}, /// 21KPA   
    {68, 340, 140}, /// 22KPA   
    {75, 322, 140}, /// 23KPA   
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
#endif

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