#ifndef __STORAGE_MANAGER_API_H__
#define __STORAGE_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "persist_storage.h"
#include "broadcast_manager_api.h"
#include "data_bury_persist_defs.h"

#define ADDR_FACTORY_DATA_OFFSET                          (0)

/*
 * 描述存储操作事务id
*/
typedef enum {
    STORE_ID_FACTORY_DEV_INFO = 100,        /// 工厂设备信息
    STORE_ID_FACTORY_DEV_DID,               /// 工厂设备did
    STORE_ID_FACTORY_DEV_SN,                /// 工厂设备sn
    STORE_ID_FACTORY_DEV_BODY_COLOR,        /// 工厂设备body color
    STORE_ID_FACTORY_DEV_REGION_ID,         /// 工厂设备区域id
    STORE_ID_FACTORY_DEV_PRODUCT_ID,        /// 工厂设备产品id
    STORE_ID_FACTORY_FLAG_INFO,             /// 工厂标志信息
    STORE_ID_FACTORY_DEPLOY_FLAG,           /// 工厂部署标志
    STORE_ID_FACTORY_AGE_FLAG,              /// 工厂老化标志
    STORE_ID_FACTORY_AGE_DISPLAY_FLAG,      /// 工厂老化展示标志
    STORE_ID_FACTORY_FSENSOR_CALI_FLAG,     /// 工厂fsensor校准标志
    STORE_ID_FACTORY_IMU_CALI_DATA,         /// 工厂陀螺仪校准数据
    STORE_ID_FACTORY_SUCTION_TABLE,         /// 工厂预设泵奶参数
    STORE_ID_FACTORY_MOTOR_TYPE,            /// 工厂预设泵类型
    STORE_ID_FACTORY_BATTERY_TYPE,          /// 工厂预设电池类型
#if IS_SUPPORT_DEVICE_LIGHT_SENSOR_CALIB
    STORE_ID_FACTORY_LIGHT_SENSOR_CALIB_DATA, /// 工厂光感校准数据
#endif
    STORE_ID_AUDIO_PACKET_INFO = 200,       /// 语音包信息
    STORE_ID_THEME_PACKET_INFO,             /// 主题包信息
    STORE_ID_BREAST_PUMP_INFO = 300,        /// 吸奶信息
    STORE_ID_BREAST_PUMP_TIME,              /// 吸奶时间
    STORE_ID_PUMP_SETTING_INFO,             /// 吸奶设置信息
    STORE_ID_PUMP_MODE_STEPS_INFO,          /// 吸奶模式多步设置信息
    STORE_ID_PUMP_MODE_NAME_INFO,           ///模式名称
    STORE_ID_PUMP_CONTROL_TYPE,             /// 吸奶控制类型
    STORE_ID_BREAST_PUMP_DATA_HEAD,         /// 吸奶器数据头
    STORE_ID_BREAST_PUMP_DATA_BODY,         /// 吸奶器数据体
    STORE_ID_BREAST_PUMP_BURY_INFO,         /// 吸奶器埋点信息
    STORE_ID_BREAST_PUMP_MODE,              /// 吸奶器模式
    STORE_ID_BREAST_PUMP_MODE_WITH_TYPE,    /// 吸奶器模式(追奶模式/常规模式)
    STORE_ID_MODE_NEW_STEP_INFO_FLAG,       /// 模式新步数信息标志
    STORE_ID_PUMP_MODE_ACTIVATE_STATUS,     /// 模式激活状态更新
    STORE_ID_PUMP_MODE_STEPS_INFO_ID,       /// 模式配置ID
    STORE_ID_DATA_BURY_FLASH_BANK,          /// Momcozy 埋点 Flash 槽位索引
    STORE_ID_DATA_BURY_RING_META,           /// Momcozy 埋点环 meta arg1=slot_index
    STORE_ID_DATA_BURY_RING_CHUNK,          /// Momcozy 埋点环数据块(2048B) arg1=0 arg2=slot_index
    STORE_ID_DATA_BURY_RING_APPEND,         /// 埋点环追加 val=data_bury_ring_append_args_t*
    STORE_ID_DATA_BURY_RING_PENDING_COUNT,  /// 埋点环待上报条数 val=uint16_t*
    STORE_ID_DATA_BURY_RING_POP_OLDEST,       /// 埋点环 pop 最旧一条
    STORE_ID_DATA_BURY_RING_PEEK_OLDEST,      /// 埋点环 peek 最旧一条 val=data_bury_ring_peek_args_t*
    STORE_ID_SYS_CONFIG_INFO = 400,         /// 系统配置信息
    STORE_ID_SYS_TIME_ZONE,                 /// 系统时区
    STORE_ID_SYS_AP_MODE_ONOFF,             /// 系统飞航模式开关
    STORE_ID_SYS_SILENT_UPGRADE,            /// 系统静默升级开关
    STORE_ID_SYS_UI_STYLE,                  /// 系统UI风格
    STORE_ID_SYS_LOG_SWITCH,                /// 系统UART日志开关
    STORE_ID_SYS_LANGUAGE,                  /// 系统语言类型
    STORE_ID_SYS_UTC_TIMESTAMP,             /// 系统时间戳
    STORE_ID_SYS_REBOOT_REASON,             /// 系统重启原因
    STORE_ID_SYS_BATTERY_PERCENT,           /// 电池电量
    STORE_ID_SYS_SEAL_LEAK_ENABLED,         /// 漏气检测功能开关
    STORE_ID_SYS_STATE_INFO = 500,          /// 系统状态信息
    STORE_ID_SYS_GUIDE_DEPLOYED,            /// 向导部署标志
    STORE_ID_SYS_BATTERY_HEALTH,            /// 电池健康
    STORE_ID_SYS_BREAST_PUMP_HEADER_LIFE,   /// 吸嘴寿命
    STORE_ID_SYS_NIGHT_LIGHT_ADAPTIVE_SWITCH, /// 夜灯自适应开关 @deprecated
    STORE_ID_SYS_AIRPLANE_MODE_STATE,       /// 飞行模式状态
    STORE_ID_SYS_SYNC_SYS_TIME,             /// 保存第一次同步系统时间时 本地经历的最终时间点 用于计算吸奶记录的差值.
    STORE_ID_SYS_LOG_FILTER,                /// 系统日志过滤
    STORE_ID_SYS_ADAPTIVE_SWITCH,           /// global_adaptive_switch（Flash sys_state）
    STORE_ID_SYS_NIGHT_LIGHT_GEAR,          /// 夜灯档位 L0~L3
    STORE_ID_SYS_BACK_LIGHT_LEVEL,          /// LCD+按键背光亮度
    STORE_ID_SYS_NIGHT_LIGHT_STARTUP_SWITCH, /// 夜灯开机自动开启 0-关 1-开
    STORE_ID_USER_BIND_INFO = 600,          /// 用户绑定信息
    STORE_ID_SYS_BIND_STATUS,               /// 绑定状态
    STORE_ID_IMG_RES_OTA_INFO = 700,        /// 图片资源OTA信息
    STORE_ID_LANG_PACK_OTA_INFO,            /// 语言包OTA断点信息
    STORE_ID_MAX
} persist_store_id_e;

typedef enum {
    STORE_CRC_TYPE_SYS_CONFIG = 0,
    STORE_CRC_TYPE_SYS_STATE,
    STORE_CRC_TYPE_PUMP_INFO,
    STORE_CRC_TYPE_PUMP_MODE_STEPS_INFO,
    STORE_CRC_TYPE_PUMP_BURY_INFO,
    STORE_CRC_TYPE_PUMP_USER_DATA_HEAD,
    STORE_CRC_TYPE_IMG_RES_OTA_INFO,
    STORE_CRC_TYPE_LANG_PACK_OTA_INFO,
    STORE_CRC_TYPE_MAX
} store_crc_type_id_e;

struct rt_device_node;

/*
 * 描述恢复出厂设置时，重启或不重启的枚举变量
*/
typedef enum { 
    RESET_NOT_REBOOT = 0,   /// 仅重启
    RESET_AND_REBOOT = 1,   /// 重置并解绑
    RESET_AND_NO_UNBIND = 2,  /// 仅重置不解绑
} factory_reset_type_e;

struct storage_manager_context;

typedef struct {
    struct storage_manager_context *ctx;
    int arg1;
    int arg2;
    void *val;
} store_func_args_t;

/// 写入存储处理接口.
typedef int (*put_store_process_func)(store_func_args_t args);

/// 读取存储处理接口.
typedef int (*get_store_process_func)(store_func_args_t args);

/// 删除存储处理接口.
typedef int (*del_store_process_func)(store_func_args_t args);

/// 写入系统ram值处理接口.
typedef int (*put_store_ram_val_func)(store_func_args_t args);

/// 读取存储处理接口.
typedef int (*get_store_ram_val_func)(store_func_args_t args);

/// 描述写入存储处理结构.
typedef struct {
    uint16_t store_id;                             /// 存储id   
    put_store_process_func store_process;   /// 存储处理接口.
} put_store_process_handle_t;

/// 描述读取存储处理结构.
typedef struct {
    uint16_t store_id;                             /// 存储id   
    get_store_process_func store_process;   /// 存储处理接口.
} get_store_process_handle_t;

/// 删除存储处理接口.
typedef struct {
    uint16_t store_id;                             /// 存储id   
    del_store_process_func store_process;   /// 存储处理接口.
} del_store_process_handle_t;

/// 描述写入系统ram值处理结构.
typedef struct {
    uint16_t store_id;                             /// 存储id   
    put_store_ram_val_func store_process;    /// 写入系统ram值接口.
} put_store_ram_val_handle_t;

/// 描述读取存储处理结构.
typedef struct {
    uint16_t store_id;                             /// 存储id   
    get_store_ram_val_func store_process;    /// 获取系统ram值接口.
} get_store_ram_val_handle_t;

struct rt_device_node;
struct broadcast_obj;
// struct extflash_interface;

typedef struct storage_manager_context {
/// public
    int (*init)(void);
    int (*factory_reset)(factory_reset_type_e is_reboot);
    sys_persist_info_t *(*get_persist_sys_info)(void);
    sec_sys_ver_t *(*get_persist_ver_info)(void);
    factory_data_store_t *(*get_factory_data)(void);
    /// 通用数据存储写读操作接口.
    int (*put_store_val)(persist_store_id_e id, int arg1, int arg2, void *val_in);
    int (*get_store_val)(persist_store_id_e id, int arg1, int arg2, void *val_out);
    int (*del_store_val)(persist_store_id_e id, int arg1, int arg2, void *val_in);
    int (*put_store_ram_val)(persist_store_id_e id, int arg1, int arg2, void *val_in);
    int (*get_store_ram_val)(persist_store_id_e id, int arg1, int arg2, void *val_out);
    int (*user_database_reset)(void);
    int (*flash_protect)(int protect_enable);  /// 保护flash读写操作.
    /// 外部hook点.
    struct __storage_hook {
        int (*on_post_init_route)(struct storage_manager_context *st);
        int (*on_perpare_init_route)(struct storage_manager_context *st);
        void (*on_storage_broadcast_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata);
        void (*on_power_off_force_save_route)(struct storage_manager_context *st);
    } *hook;

/// private
    struct __storage_private {
        struct broadcast_obj *mbdc;
        /// 数据库.
        struct rt_device_node *db_dev;
        struct rt_device_node *factory_flash_dev;
        /// 外挂flash
        // struct extflash_interface *extflash_if;
        os_mutex_id storage_rw_lock;
        /// 系统用户数据存储.
        sys_persist_info_t sys_info;
        /// 记录sys_info的crc,如果有改变则需要更新到flash.
        uint16_t sys_info_crc[STORE_CRC_TYPE_MAX];
        /// 系统版本信息.
        sec_sys_ver_t ver_info;
        /// 工厂数据集合.
        factory_data_store_t factory_data;
        breast_pump_data_head_t pump_data_head;

        /// 埋点信息.
        data_bury_info_t data_bury_info;
        //资源ota信息
        img_res_ota_info_t img_res_ota_info;
        img_res_ota_info_t lang_pack_ota_info;

        /// 存储处理列表.
        const put_store_process_handle_t *put_store_process_list;
        const get_store_process_handle_t *get_store_process_list;
        const del_store_process_handle_t *del_store_process_list;
        const put_store_ram_val_handle_t *put_store_ram_val_list;
        const get_store_ram_val_handle_t *get_store_ram_val_list;
        /// 存储处理列表长度.
        uint16_t put_store_process_list_len;
        uint16_t get_store_process_list_len;
        uint16_t del_store_process_list_len;
        uint16_t put_store_ram_val_list_len;
        uint16_t get_store_ram_val_list_len;
    } priv;
} storage_manager_context_t;


#ifdef __cplusplus
}
#endif

#endif 


