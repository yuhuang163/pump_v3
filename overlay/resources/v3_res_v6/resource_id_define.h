#ifndef __RESOURCE_ID_DEFINE_H__
#define __RESOURCE_ID_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "base/resource_base_id_define.h"  // IWYU pragma: keep
#include "custom/resource_custom_id_define.h"  // IWYU pragma: keep

/// 全局唯一资源 ID 类型（数值跨主题唯一）
typedef uint16_t resource_id_type_e;

#ifdef __cplusplus
}
#endif

#endif
