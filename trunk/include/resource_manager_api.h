#ifndef __RESOURCE_MANAGER_API_H__
#define __RESOURCE_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

#define RES_PSRAM_BSS_LOC            0
#define RES_PSRAM_NC_BSS_LOC         1
#define RES_PSRAM_TYPE               0  /// 0: PSRAM, 1: PSRAM_NC

#define FS_LOCK_LOG(...) do {} while (0)

#define MUTEX_LOCK_FS(__lock, tag) \
do { \
    MUTEX_LOCK(__lock); \
    FS_LOCK_LOG("m_fs_rw_lock=>Take:%s", tag); \
} while (0)

#define MUTEX_UNLOCK_FS(__lock, tag) \
do { \
    FS_LOCK_LOG("m_fs_rw_lock=>Give:%s", tag); \
    MUTEX_UNLOCK(__lock); \
} while (0)

/// 描述资源存储信息.
typedef struct img_res_storage_info {
    uint16_t img_id;          /// 存储ID(0~n)
    uint16_t width;           /// 图片宽度(像素)
    uint16_t height;          /// 图片高度(像素)
    uint16_t x_offset;        /// X坐标偏移(用于UI布局)
    uint16_t y_offset;        /// Y坐标偏移(用于UI布局)
    uint32_t storage_addr;    /// 记录每个图片的存储地址
    uint32_t data_size;       /// 单个资源数据大小(字节)
} __attribute__((packed)) img_res_storage_info_t;

/// UI 图片数据缓存信息（用于 LVGL MEMFS 等场景）
typedef struct {
    uint16_t id;
    uint8_t *data;
    uint32_t size;
} res_img_info_t;

typedef void (*on_install_result_cb_fptr)(void *user_data, int args0, int args1);

/**
 * 描述资源管理上下文, 负责管理所有显示资源
*/
typedef struct resource_manager_context {
/// public:
    /// 计算文件指定偏移长度md5值.
    const uint8_t *(*caculate_file_md5)(const char *filepath, uint32_t offset, uint32_t length);
    /// 安装主题资源包.
    // int (*install_resource)(struct ota_file_info *file_info, on_install_result_cb_fptr cb);
    /// 卸载安装主题资源包.
    int (*uninstall_resource)(int which);
    int (*wait_init_completed)(void *res);
    /// 根据 img_id 加载 UI 图片数据到 res_img_info_t 中（带缓存复用）
    int (*res_ui_img_data_load)(uint16_t img_id, res_img_info_t *out);
    /// 释放 res_img_info_t 中的缓存并清零
    void (*res_ui_img_data_unload)(res_img_info_t *img);
/// private:
    struct __resource_manager_private {
        int install_result;
        int install_status;
        int boot_index;
        void *install_user_data;
        on_install_result_cb_fptr install_result_cb;
        os_mutex_id m_fs_rw_lock;
        bool res_init_completed;
        struct storage_manager_context *storage;
        struct power_manager_context *pm;
    } priv;
} resource_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


