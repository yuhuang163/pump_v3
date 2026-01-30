#ifndef __PROJECT_PARTITIONS_H__
#define __PROJECT_PARTITIONS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern uint32_t __littlefs_start[];
extern uint32_t __littlefs_end[];
extern uint32_t __factory_data_start[];
extern uint32_t __factory_data_end[];
extern uint32_t __lute_picture_resources_start[];
extern uint32_t __lute_picture_resources_end[];


// FDS 4k Sector num
#define FDS_START_ADDR                        (uint32_t)__littlefs_start
#define FDS_END_ADDR                          (uint32_t)__littlefs_end
#define FDS_BLOCK_SIZE                        (FDS_END_ADDR - FDS_START_ADDR)
#define FDS_SECTOR_NUM                        (FDS_BLOCK_SIZE / 4096)

// factory data
#define FACTORY_DATA_START_ADDR               (uint32_t)__factory_data_start
#define FACTORY_DATA_END_ADDR                 (uint32_t)__factory_data_end
#define FACTORY_DATA_BLOCK_SIZE               (FACTORY_DATA_END_ADDR - FACTORY_DATA_START_ADDR)
#define FACTORY_DATA_SECTOR_NUM               (FACTORY_DATA_BLOCK_SIZE / 4096)

// picture data internal flash
#define IMG_INT_DATA_START_ADDR               (uint32_t)__lute_picture_resources_start
#define IMG_INT_DATA_END_ADDR                 (uint32_t)__lute_picture_resources_end
#define IMG_INT_DATA_BLOCK_SIZE               (IMG_INT_DATA_END_ADDR - IMG_INT_DATA_START_ADDR)
#define IMG_INT_DATA_SECTOR_NUM               (IMG_INT_DATA_BLOCK_SIZE / 4096)

// picture data external flash
#define EXTERNAL_FLASH_BASE_ADDR              (0)
#define EXTERNAL_FLASH_SECTOR_SIZE            (4096)
#define EXTERNAL_FLASH_SIZE                   (0x1000000)
#define IMG_EXT_DATA_START_ADDR               (EXTERNAL_FLASH_BASE_ADDR)
#define IMG_EXT_DATA_END_ADDR                 (IMG_EXT_DATA_START_ADDR + EXTERNAL_FLASH_SIZE)
#define IMG_EXT_DATA_BLOCK_SIZE               (IMG_EXT_DATA_END_ADDR - IMG_EXT_DATA_START_ADDR)
#define IMG_EXT_DATA_SECTOR_NUM               (IMG_EXT_DATA_BLOCK_SIZE / EXTERNAL_FLASH_SECTOR_SIZE)

#ifdef __cplusplus
}
#endif

#endif


