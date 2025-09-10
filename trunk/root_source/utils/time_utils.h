#ifndef __TIME_UTILS_H__
#define __TIME_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h> // IWYU pragma: keep
#include <string.h> // IWYU pragma: keep
#include <ctype.h> // IWYU pragma: keep
#include "cmsis_os_api.h" // IWYU pragma: keep

void timestamp_to_utc_time(uint32_t seconds, utc_time_t *utc_time);
uint32_t utc_time_to_timestamp(utc_time_t *utc_time);

#ifdef __cplusplus
}
#endif

#endif
