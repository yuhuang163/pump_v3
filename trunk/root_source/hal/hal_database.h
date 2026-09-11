#ifndef __HAL_DATABASE_H__
#define __HAL_DATABASE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/**
 * @brief 描述数据库操作码
*/
typedef enum {
    OP_CODE_DB_DELETE_ALL = 1,
    OP_CODE_DB_DELETE = 2,
    OP_CODE_DB_EXT_DELETE = 3,
    OP_CODE_DB_EXT_WRITE_AT = 4,
    OP_CODE_DB_EXT_READ_AT = 5,
} db_dev_op_code_e;

typedef struct {
    uint32_t key;
    uint32_t offset;
    void *buffer;
    uint32_t size;
} db_ext_rw_args_t;

typedef enum {
    DB_STORAGE_INTERNAL = 0,
    DB_STORAGE_EXTERNAL = 1,
} db_storage_type_e;

#ifdef __cplusplus
}
#endif

#endif 
