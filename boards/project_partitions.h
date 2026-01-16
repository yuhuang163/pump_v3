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
#define PICTURE_DATA_INT_START_ADDR            (uint32_t)__lute_picture_resources_start
#define PICTURE_DATA_INT_END_ADDR              (uint32_t)__lute_picture_resources_end
#define PICTURE_DATA_INT_BLOCK_SIZE            (PICTURE_DATA_INT_END_ADDR - PICTURE_DATA_INT_START_ADDR)
#define PICTURE_DATA_INT_SECTOR_NUM            (PICTURE_DATA_INT_BLOCK_SIZE / 4096)

// picture data external flash
#define PICTURE_DATA_EXT_START_ADDR            0
#define PICTURE_DATA_EXT_END_ADDR              0x2000000
#define PICTURE_DATA_EXT_BLOCK_SIZE            (PICTURE_DATA_EXT_END_ADDR - PICTURE_DATA_EXT_START_ADDR)
#define PICTURE_DATA_EXT_SECTOR_NUM            (PICTURE_DATA_EXT_BLOCK_SIZE / 4096)

#ifdef __cplusplus
}
#endif

#endif


