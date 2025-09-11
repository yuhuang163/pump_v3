#ifndef __RESOURCE_MANAGER_API_H__
#define __RESOURCE_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

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
/// private:
    struct __resource_manager_private {
        int install_result;
        int install_status;
        int boot_index;
        void *install_user_data;
        on_install_result_cb_fptr install_result_cb;
        os_mutex_id m_fs_rw_lock;
        os_semaphore_id sem_init;
        bool res_init_completed;
        struct storage_context_struct *storage;
        struct power_manager_context *pm;
    } priv;
} resource_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


