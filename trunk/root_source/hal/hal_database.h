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
} db_dev_op_code_e;

typedef enum {
    DB_STORAGE_INTERNAL = 0,
    DB_STORAGE_EXTERNAL = 1,
} db_storage_type_e;

#ifdef __cplusplus
}
#endif

#endif 
