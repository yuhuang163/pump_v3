#ifndef __STRING_UTILS_H__
#define __STRING_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h> // IWYU pragma: keep
#include <string.h> // IWYU pragma: keep
#include <ctype.h> // IWYU pragma: keep

#define SEC_STRNCPY(_dest, _src, n) \
do { \
    sec_strncpy((char *)_dest, (const char *)_src, n); \
} while (0)

char *sec_strncpy(char *dest, const char *src, size_t n);

#ifdef __cplusplus
}
#endif

#endif 
