/*
* INCLUDE FILES
*****************************************************************************************
*/
#include <stdio.h> // IWYU pragma: keep
#include <stdint.h>
#include <string.h> // IWYU pragma: keep
#include <ctype.h> // IWYU pragma: keep

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

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
unsigned short crc16_compute(unsigned char const *p_data, unsigned int size, unsigned short const * p_crc)
{
    unsigned short crc = (p_crc == NULL) ? 0xFFFF : *p_crc;
    for (uint32_t i = 0; i < size; i++) {
        crc  = (unsigned char)(crc >> 8) | (crc << 8);
        crc ^= p_data[i];
        crc ^= (unsigned char)(crc & 0xFF) >> 4;
        crc ^= (crc << 8) << 4;
        crc ^= ((crc & 0xFF) << 4) << 1;
    }

    return crc;
}

void hex_to_ascii(const unsigned char *hex, int len, char *out)
{
    for (int i = 0; i < len; i++) {
        sprintf(out + i * 2, "%02X", hex[i]);  // 每个字节转成两个ASCII字符
    }
}