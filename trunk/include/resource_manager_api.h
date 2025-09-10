#ifndef __RESOURCE_MANAGER_API_H__
#define __RESOURCE_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h"

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
    int (*init)(void);
    // int (*read_sound_stream_buffer_offset)(int res_id, uint32_t offset, void *buf, uint32_t buf_size);
    // void (*release_sound_stream_buffer_if_need)(void *user_data);
    // int (*get_themes_ani_res_info)(themes_res_type_e type_id, uint16_t *start_res_id, uint16_t *res_count);
    // int (*get_themes_sound_res_info)(themes_res_type_e type_id, uint16_t *sound_res_id);
    // res_gift_head_desc_t *(*get_gift_ani_res_info)(themes_res_type_e type_id, uint16_t *start_res_id, uint16_t *res_count);
    // int (*get_gift_sound_res_info)(themes_res_type_e type_id, uint16_t *sound_res_id);
    // res_obj_desc_t *(*read_img_res_buffer_from_id)(int res_id, uint8_t *buffer, uint32_t buffer_size);
    // int (*is_need_show_themes_pkg)(void);
    // int (*is_need_show_gift_pkg)(res_gift_play_type_e res_type);
    // int (*get_res_version_info)(char *buf, int size_of_buf);

    /// 计算文件指定偏移长度md5值.
    // const uint8_t *(*caculate_file_md5)(const char *filepath, uint32_t offset, uint32_t length);

    /// 安装主题资源包.
    // int (*install_resource)(struct ota_file_info *file_info, on_install_result_cb_fptr cb);

    /// 卸载安装主题资源包.
    // int (*uninstall_resource)(int which);
    // int (*write_to_flash)(void *fp, const uint8_t *data, uint32_t data_len, uint32_t block_size);
    // bool (*is_recovery_mode)(void);    
    // bool (*head_sound_exist)(uint8_t sound_theme);
    // int (*wait_init_completed)(void *res);
    /// 外部钩子函数.
    struct __resource_manager_hook {
        int (*get_custom_map_sound_res_id_route)(struct peripheral_bus_context *context, int sound_enum_id);
    } *hook;


// private:
    // int verify_status;
    int install_result;
    int install_status;
    int boot_index;
    // res_obj_desc_t *img_obj_buf;
    void *install_user_data;
    int use_def_sound_replace;
    on_install_result_cb_fptr install_result_cb;
    os_mutex_id m_fs_rw_lock;
    os_semaphore_id sem_init;
    bool res_init_completed;
    struct storage_context_struct *storage;
    struct power_manager_context *pm;
    // res_download_head_wrapper_t *dl_head;                           // ota下载包(组合包)头.
    // res_head_wrapper_struct_t *head_main;                           // 基础图片资源
    // res_ota_head_wrapper_struct_t *head_ota;                        // OTA图片资源.
    // res_sound_head_wrapper_struct_t *head_sound[SOUND_THEMES_MAX];  // 官方语音包指针, 指向具体的语音包（男声女声童声自定义)
    // res_themes_head_wrapper_t *head_themes[THEMES_TYPE_MAX];        // 主题定制(包含语音和动画资源)，长期有效. 
    // res_gift_head_wrapper_t *head_gifts;                            // 推送礼物定制(包含语音和动画资源)，单次有效.
    // int need_reload_res;
    // res_sound_obj_desc_t *play_sound_obj;
} resource_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


