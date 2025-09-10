#ifndef __STORAGE_MANAGER_API_H__
#define __STORAGE_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "system_persist_data.h"

typedef uint16_t database_key_t;
#define ADDR_FACTORY_DATA_OFFSET                          (0)

/*
 * 描述存储操作事务id
*/
typedef enum {
    STORE_ID_FACTORY_DEV_INFO = 100,        // 工厂设备信息
    STORE_ID_FACTORY_DEV_DID,               // 工厂设备did
    STORE_ID_FACTORY_DEV_SN,                // 工厂设备sn
    STORE_ID_FACTORY_DEV_BODY_COLOR,        // 工厂设备body color
    STORE_ID_FACTORY_DEV_REGION_ID,         // 工厂设备区域id
    STORE_ID_FACTORY_DEV_PRODUCT_ID,        // 工厂设备产品id
    STORE_ID_FACTORY_FLAG_INFO,             // 工厂标志信息
    STORE_ID_FACTORY_DEPLOY_FLAG,           // 工厂部署标志
    STORE_ID_FACTORY_AGE_FLAG,              // 工厂老化标志
    STORE_ID_FACTORY_AGE_DISPLAY_FLAG,      // 工厂老化展示标志
    STORE_ID_FACTORY_FSENSOR_CALI_FLAG,     // 工厂fsensor校准标志
    STORE_ID_FACTORY_IMU_CALI_DATA,         // 工厂陀螺仪校准数据
    STORE_ID_AUDIO_PACKET_INFO = 200,       // 语音包信息
    STORE_ID_THEME_PACKET_INFO,             // 主题包信息
    STORE_ID_BREATH_PUMP_INFO = 300,               // 吸奶信息
    STORE_ID_BREATH_PUMP_TIME,                     // 吸奶时间
    STORE_ID_BREATH_PUMP_LEVEL,                    // 吸奶力度
    STORE_ID_REMIND_BREATH_PUMP_HEAD,              // 吸奶刷头提醒
    STORE_ID_REMIND_BREATH_PUMP_EXCEPTION,         // 吸奶异常提醒
    STORE_ID_REMIND_BREATH_PUMP_HORIZONTAL,        // 吸奶水平提醒
    STORE_ID_REMIND_BREATH_PUMP_MODE_CHANGE,       // 吸奶模式切换提醒
    STORE_ID_REMIND_BREATH_PUMP_AREA_CHANGE,       // 吸奶区域切换提醒
    STORE_ID_REMIND_BREATH_PUMP_OVER_PRESS,        // 吸奶过压提醒
    STORE_ID_REMIND_BREATH_PUMP_OVER_PRESS_LED,    // 吸奶过压LED提示
    STORE_ID_REMIND_BREATH_PUMP_FINISH,            // 吸奶完成提醒
    STORE_ID_BREATH_PUMP_DATA_HEAD,                // 吸奶数据头
    STORE_ID_BREATH_PUMP_DATA_BODY,                // 吸奶数据体
    STORE_ID_SYS_CONFIG_INFO = 400,         // 系统配置信息
    STORE_ID_SYS_TIME_ZONE,                 // 系统时区
    STORE_ID_SYS_AP_MODE_ONOFF,             // 系统飞航模式开关
    STORE_ID_SYS_SILENT_UPGRADE,            // 系统静默升级开关
    STORE_ID_SYS_UI_STYLE,                  // 系统UI风格
    STORE_ID_SYS_LOG_SWITCH,                // 系统UART日志开关
    STORE_ID_SYS_LANGUAGE,                  // 系统语言类型
    STORE_ID_SYS_SOUND_MODE_ONOFF,          // 系统语音提醒开关
    STORE_ID_SYS_MICROSEISM_ONOFF,          // 系统微震开关
    STORE_ID_SYS_AI_MODE_ONOFF,             // 系统AI模式开关
    STORE_ID_SYS_UTC_TIMESTAMP,             // 系统时间戳
    STORE_ID_SYS_STATE_INFO = 500,          // 系统状态信息
    STORE_ID_SYS_GUIDE_DEPLOYED,            // 向导部署标志
    STORE_ID_SYS_BATTERY_HEALTH,            // 电池健康
    STORE_ID_SYS_BREATH_PUMP_HEADER_LIFE,          // 吸嘴寿命
    STORE_ID_SYS_SYNC_SYS_TIME,             // 保存第一次同步系统时间时 本地经历的最终时间点 用于计算吸奶记录的差值.
    STORE_ID_SYS_LOG_FILTER,                // 系统日志过滤
    STORE_ID_USER_BIND_INFO = 600,          // 用户绑定信息
    STORE_ID_MAX
} persist_store_id_e;

typedef enum {
    STORE_CRC_TYPE_SYS_CONFIG = 0,
    STORE_CRC_TYPE_SYS_STATE,
    STORE_CRC_TYPE_SYS_BREATH_PUMP_INFO,
    STORE_CRC_TYPE_MAX
} store_crc_type_id_e;

struct rt_device_node;
typedef uint16_t database_key_t;

/*
 * 描述恢复出厂设置时，重启或不重启的枚举变量
*/
typedef enum { 
    RESET_NOT_REBOOT = 0,   //仅重启
    RESET_AND_REBOOT = 1,   //重置并解绑
    RESET_AND_NO_UNBIND = 2,  //仅重置不解绑
} factory_reset_type_e;

struct rt_device_node;
// struct extflash_interface;

typedef struct storage_context_struct {
// public
    int (*init)(void);
    int (*factory_reset)(factory_reset_type_e is_reboot);
    sys_persist_info_t *(*get_persist_sys_info)(void);
    sec_sys_ver_t *(*get_persist_ver_info)(void);
    factory_data_store_t *(*get_factory_data)(void);
    // 通用数据存储写读操作接口.
    int (*put_store_val)(persist_store_id_e id, int arg1, int arg2,  const void *val_in);
    int (*get_store_val)(persist_store_id_e id, int arg1, int arg2,  void *val_out);
    int (*del_store_val)(persist_store_id_e id, int arg1, int arg2,  const void *val_in);
    void (*put_store_ram_val)(persist_store_id_e id, int args);
    int (*get_store_ram_val)(persist_store_id_e id);

    int (*get_record_breath_pump_count)(void);
    int (*get_record_breath_pump_info)(int num, void *pump_info);
    int (*del_record_breath_pump_info)(int num);
    // 外部hook点.
    struct __storage_hook {
        int (*read_old_sys_info)(struct storage_context_struct *st, persist_store_id_e id, void *val_in);
    } *hook;

// private
    broadcast_obj_t mbdc;
    // 数据库.
    struct rt_device_node *db_dev;
    // 外挂flash
    // struct extflash_interface *extflash_if;
    os_mutex_id db_rw_lock;
    os_mutex_id storage_rw_lock;
    os_mutex_id breath_pump_data_rw_lock;
    // 系统用户数据存储.
    sys_persist_info_t sys_info;
    // 记录sys_info的crc,如果有改变则需要更新到flash.
    uint16_t sys_info_crc[STORE_CRC_TYPE_MAX];
    // 系统版本信息.
    sec_sys_ver_t ver_info;
    // 工厂数据集合.
    factory_data_store_t factory_data;
    breath_pump_data_head_t breath_pump_data_head;
} storage_context_t;


#ifdef __cplusplus
}
#endif

#endif 


