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
 * @brief 描述FS操作码
*/
typedef enum {
    OP_CODE_FS_MOUNT = 1,
    OP_CODE_FS_UNMOUNT = 2,
    OP_CODE_FS_FORMAT = 3,
    OP_CODE_FS_IS_READY = 4,
    OP_CODE_FS_INTERFACE_GET = 5,
} fs_dev_op_code_e;

/*
 * 描述对接posix接口的file system操作接口.
 * 用于兼容带fs和不带fs的项目平台.
*/
typedef struct fs_interface {
    void *(*fopen)(const char *filename, const char *mode);
    int (*fclose)(void *fp);
    int (*fflush)(void *fp);
    long int (*ftell)(void *fp);
    uint32_t (*fread)(void *buffer, uint32_t size, uint32_t size_count, void *fp);
    uint32_t (*fwrite)(const void *buffer, uint32_t size, uint32_t size_count, void *fp);
    int (*fseek)(void *fp, uint32_t Offset, int origin);
    uint32_t (*get_size)(const char *filename);
    int (*remove)(const char *filename);
    int (*rename)(const char *ole_filename, const char *new_filename);
    int (*mkdir)(const char *dir_path);
    int (*fs_mount)(const char *root, const char *partition_label);
    int (*fs_unmount)(const char *root);
    int (*fs_format)(const char *root, const char *partition_label);
    int (*fs_get_size_info)(const char *root, uint32_t *_bytes_total, uint32_t *_bytes_free);
    int fs_is_ready;
} fs_interface_t;

#ifdef __cplusplus
}
#endif

#endif 

