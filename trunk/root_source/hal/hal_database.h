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
} db_dev_op_code_e;

#ifdef __cplusplus
}
#endif

#endif 
