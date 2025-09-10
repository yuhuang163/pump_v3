/*
* INCLUDE FILES
*****************************************************************************************
*/
#include <stdio.h> // IWYU pragma: keep
#include <stdint.h>
#include <string.h> // IWYU pragma: keep
#include "cmsis_os_api.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
uint16_t g_log_level_set = US_LOG_LEVEL_VERBOSE;
uint32_t g_log_filter = (uint32_t)FILTER_FULL;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void set_log_ouput_level(uint16_t level)
{
    if (g_log_level_set != level) {
        g_log_level_set = level;
        PRINT_LOG("%s() new g_log_level_set: %d", __func__, level);
    }
}

uint16_t get_log_output_level(void)
{
    return g_log_level_set;
}
 
void reset_log_filter(uint32_t filter) 
{
    PRINT_LOG("New log filter: 0x%x\n", filter);
    g_log_filter = filter;
    if (filter == 0) {
        g_log_level_set = US_LOG_LEVEL_ERROR;
    } else {
        g_log_level_set = US_LOG_LEVEL_DEBUG;
    }
}
 
uint32_t get_log_filter(void)
{
    return g_log_filter;
}
 
void init_user_log_filter(void) 
{
    uint32_t filter = 0;
    PRINT_LOG("PERSIST_KEY_LOG_FILTER: 0x%x\n", filter);
    if (filter) {
        reset_log_filter(filter);
    }
}
