#ifndef __HAL_FLASH_H__
#define __HAL_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/**
 * @brief 描述FLASH操作码
*/
typedef enum {
    OP_CODE_FLASH_ERASE = 1,
    OP_CODE_FLASH_ERASE_ALL = 2,
} flash_dev_op_code_e;

/**
 * @brief 描述FLASH设备信息
*/
typedef struct flash_device_info {
    uint32_t id;
    uint32_t size;
} flash_device_info_t;

/**
 * @brief 描述FLASH裸数据信息
*/
typedef struct flash_raw_data_info {
    const char *partition;  
    uint32_t addr;
    uint32_t size;
    uint8_t *buf;
} flash_raw_data_info_t;

#ifdef __cplusplus
}
#endif

#endif 
