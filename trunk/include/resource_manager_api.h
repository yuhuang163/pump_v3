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

/// 资源主题类型
typedef enum {
    RES_THEME_TYPE_NONE = 0,  /// 无主题资源
    RES_THEME_TYPE_BASE,      /// 基础主题资源
    RES_THEME_TYPE_CUSTOM,    /// 自定义主题资源
    RES_THEME_TYPE_MAX
} res_theme_type_e;

/// 描述资源存储信息.
typedef struct img_res_storage_info {
    uint16_t img_id;          /// 存储ID(0~n)
    uint16_t width;           /// 图片宽度(像素)
    uint16_t height;          /// 图片高度(像素)
    uint16_t x_offset;        /// X坐标偏移(用于UI布局)
    uint16_t y_offset;        /// Y坐标偏移(用于UI布局)
    uint16_t crc16;           /// 单个资源校验和(CRC16)
    uint32_t storage_addr;    /// 记录每个图片的存储地址
    uint32_t data_size;       /// 单个资源数据大小(字节)
} __attribute__((packed)) img_res_storage_info_t;

/// UI 图片数据缓存信息（用于 LVGL MEMFS 等场景）
typedef struct {
    uint16_t id;
    uint16_t crc16;
    uint8_t *data;
    uint32_t size;
} res_img_info_t;

/// 资源管理上下文
typedef struct resource_manager_context {
/// public:
    const uint8_t *(*caculate_file_md5)(const char *filepath, uint32_t offset, uint32_t length); /// 计算文件指定偏移长度md5值
    int (*install_resource)(int which); /// 安装主题资源包
    int (*uninstall_resource)(int which); /// 卸载资源
    int (*wait_init_completed)(void *res); /// 等待初始化完成
    int (*base_res_data_load)(uint16_t img_id, res_img_info_t *out); /// 基础资源加载
    int (*theme_res_data_load)(uint16_t img_id, res_img_info_t *out); /// 主题资源加载
    int (*res_data_load_by_theme)(res_theme_type_e theme_type, uint16_t img_id, res_img_info_t *out); /// 按指定主题分区加载
    void (*res_data_unload)(res_img_info_t *img); /// 释放资源
    int (*get_img_res_version)(res_theme_type_e theme_type); /// 获取基础资源版本
    int (*get_sys_theme_type)(void); /// 主题类型
    int (*set_sys_theme_type)(res_theme_type_e theme_type); /// 设置主题类型
    int (*get_lang_res_active)(void); /// 外部语言包是否已生效
/// private:
    struct __resource_manager_hook {
        int (*on_lang_res_init)(struct resource_manager_context *thiz); /// 语言包初始化（overlay）
    } *hook;
    struct __resource_manager_private {
        int install_result;
        int install_status;
        int boot_index;
        void *install_user_data;
        uint16_t img_base_res_version;
        uint16_t img_custom_res_version;
        os_mutex_id m_fs_rw_lock;
        bool res_init_completed;
        bool theme_res_init_completed;
        bool base_res_init_completed;
        bool lang_res_init_completed;
        res_theme_type_e theme_type;
        struct storage_manager_context *st;
        struct power_manager_context *pm;
    } priv;
} resource_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


