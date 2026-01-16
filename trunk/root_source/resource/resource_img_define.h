#ifndef __RESOURCE_IMG_DEFINE_H__
#define __RESOURCE_IMG_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdlib.h> // IWYU pragma: keep
#include <stdint.h> // IWYU pragma: keep
#include <stdbool.h> // IWYU pragma: keep

/// 图片资源
#define IMG_RES_MAGIC (0x52534547) /// 资源包魔数 "GESR"

/// MD5哈希大小
#define IMG_RES_MD5_SIZE (16)

/// 图片资源类型
typedef enum {
    IMG_RES_TYPE_BASE,  /// 基础资源
    IMG_RES_TYPE_UI,    /// UI资源
    IMG_RES_TYPE_ALGO,  /// 算法资源
    IMG_RES_TYPE_MAX
} img_res_type_e;

/// 图片资源语言类型
typedef enum {
    IMG_RES_LANG_TYPE_EN,  /// 英文
    IMG_RES_LANG_TYPE_ZH,  /// 中文
    IMG_RES_LANG_TYPE_MAX
} img_res_lang_type_e;

/// 描述资源ID和数据包信息映射关系.
typedef struct img_res_obj_info {
    uint16_t img_id;          /// 图片资源ID.(1~n)
    uint16_t crc16;           /// 单个资源校验和(CRC16)
    uint16_t width;           /// 图片宽度(像素)
    uint16_t height;          /// 图片高度(像素)
    uint16_t x_offset;        /// X坐标偏移(用于UI布局)
    uint16_t y_offset;        /// Y坐标偏移(用于UI布局)
    uint32_t offset_addr;     /// 记录每个图片数据包的偏移地址
    uint32_t data_size;       /// 单个资源数据大小(字节)
} __attribute__((packed)) img_res_obj_info_t;

/// 图片资源包头描述
typedef struct img_res_head_desc {
    uint32_t magic;           /// 资源包魔数(固定值 IMG_RES_MAGIC)
    uint16_t crc16;           /// 资源包头部CRC16校验值
    uint16_t flags;           /// 标志位(bit0:是否压缩, bit1-15:保留)
    uint16_t type;            /// 资源包类型(img_res_type_e)
    uint16_t lang_id;         /// 语言ID(img_res_lang_type_e)
    uint16_t version;         /// 资源版本（推荐: major*100 + minor，与 ROOT_RES_VERSION 一致）
    uint16_t res_count;       /// 资源个数(最多支持 (32KB-64)/20 = 1636个)
    uint32_t total_size;      /// 整个资源包的总大小(字节)
    uint32_t reserved[4];     /// 预留字段(16字节)
} __attribute__((packed)) img_res_head_desc_t;

/// 图片资源包整体描述.
typedef struct img_res_head_wrapper {
    img_res_head_desc_t head;            /// 资源包头描述
    uint8_t md5_head[IMG_RES_MD5_SIZE];  /// 资源包头部MD5哈希(计算范围: head字段)
    uint8_t md5_body[IMG_RES_MD5_SIZE];  /// 资源包数据体MD5哈希(计算范围: obj_info+res_body)
    uint8_t data[];                      /// 可变长度数据(obj_info数组 + res_body数据)
} __attribute__((packed)) img_res_head_wrapper_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 


