#ifndef __RESOURCE_INFO_H__
#define __RESOURCE_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "product_info.h" // IWYU pragma: keep

/// 图片资源包最大数量
#define IMG_RES_MAX_COUNT                         (1000)

/// 资源头部固定大小（包括wrapper+obj_info数组）
#define IMG_RES_HEADER_FIXED_SIZE                 (32 * 1024)  /// 固定32KB

/// 资源包文件名前缀（用于生成资源包文件名）
#define IMG_RES_NAME_PREFIX                       ROOT_DEVICE_NAME

/// 当前资源包版本 (格式: MMmm, MM=主版本, mm=次版本)
#define IMG_RES_VERSION_CURRENT                   ROOT_RES_VERSION

#ifdef __cplusplus
}
#endif

#endif 


