#ifndef __BROADCAST_MANAGER_API_H__
#define __BROADCAST_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "root_list_api.h" // IWYU pragma: keep
#include "handler_post.h" // IWYU pragma: keep

#ifndef MAX_SIZE_FOR_PDATA
#define MAX_SIZE_FOR_PDATA (64)
#endif

#define USER_DATA_TO_PRIV_DATA(_user_data, __pdata) \
    uint32_t __pdata[1] = { (uint32_t)(_user_data) }

#define PRIV_DATA_TO_USER_DATA(_user_data, pdata, __type) \
    __type *_user_data = NULL; \
    do { \
        uint32_t tmp[1]; \
        memcpy(tmp, pdata->data, sizeof(uint32_t)); \
        _user_data = (__type *)tmp[0]; \
    } while (0)

#define PRIV_DATA_TO_USER_DATA_V2(_user_data, pdata, __type) \
    do { \
        uint32_t tmp[1]; \
        memcpy(tmp, pdata->data, sizeof(uint32_t)); \
        _user_data = (__type *)tmp[0]; \
    } while (0)

typedef enum {
    ID_BROADCAST_BEGIN = 0,
    ID_BROADCAST_UPDATE_BATTERY_INFO = (1 << 0),            // 电量更新.
    ID_BROADCAST_CHARGE_PLUG_IN_STATE = (1 << 1),           // 充电拔插
    ID_BROADCAST_PM_STATE_CHANGE = (1 << 2),                // 休眠唤醒.
    ID_BROADCAST_SYS_BOOT_COMPLETED = (1 << 3),             // 系统启动完成.
    ID_BROADCAST_BT_CONNECT_STATE_CHANGE = (1 << 4),        // 蓝牙连接状态改变.
    ID_BROADCAST_OTA_UPGREADING_CHANGE = (1 << 5),          // ota升级进度广播
    ID_BROADCAST_ENTER_NEW_SCREEN = (1 << 6),               // 进入新窗口.
    ID_BROADCAST_PM_WAKEUP_BY_USER = (1 << 7),              // PM被外部唤醒
    ID_BROADCAST_SYS_PERSIST_INFO_SAVE = (1 << 8),          // 持久化存储.
    ID_BROADCAST_REQ_PM_SUSPEND_BY_USER = (1 << 9),         // User请求主动睡下去.
    ID_BROADCAST_FACTORY_TEST_CMD = (1 << 10),              // 工厂测试指令.
    ID_BROADCAST_SYS_BREATH_PUMP_MODE_CHANGE = (1 << 11),         // 吸奶模式切换.
    ID_BROADCAST_THEME_CHANGE = (1 << 12),                  // 主题切换
    ID_BROADCAST_DELETE_MODE_UPDATE = (1 << 13),            // 删除模式后刷新页面.
    ID_BROADCAST_OTA_UI_SCREEN_STATE_CHANGE = (1 << 14),    // ota升级ui进入/退出
    ID_BROADCAST_SYS_BREATH_PUMP_MODE_SOUND = (1 << 15),          // 模式声音播放
    ID_BROADCAST_SYS_SHUTDOWN = (1 << 16),                  // 系统关机.
    ID_BROADCAST_SET_BLACKLIGHT_BY_USER = (1 << 17),        // 设置背光状态.
    ID_BROADCAST_PM_FREEZE_STATE_CHANGE = (1 << 18),        // PM freeze状态改变.
    ID_BROADCAST_VOICE_SWITCH_STATE_CHANGE = (1 << 19),     // 语音开关状态改变.
    ID_BROADCAST_SYS_PLAY_SOUND = (1 << 20),                // 语音播放
    ID_BROADCAST_SYS_PUMP_STATE_CHANGE = (1 << 21),         // 泵状态改变.
    ID_BROADCAST_AIRPLANE_MODE_STATE_CHANGE = (1 << 22),    // 飞行模式状态改变.
    ID_BROADCAST_SYS_BREATH_PUMP_EVENT_SOUND_NOTIC = (1 << 23),   // 吸奶事件语音提醒
    ID_BROADCAST_UPDATE_MOTOR_WORK_PARAMETER = (1 << 24),   // 更新马达运行参数.
    ID_BROADCAST_BREATH_PUMP_AREA_CHANGE = (1 << 25),             // 吸奶面变化
    
    // add new broadcast id above this.
    ID_BROADCAST_END,
} broadcast_rcv_id_enum_t;

typedef enum {
    ID_BROADCAST_V2_BEGIN = 0,
    ID_BROADCAST_V2_OTA_INSTALL_START = (1 << 0),            // OTA安装开始.
    ID_BROADCAST_V2_END,
} broadcast_rcv_id_v2_enum_t;

typedef struct {
    uint32_t code;
    uint32_t code_v2;
} broadcast_mask_id_t;

typedef struct broadcast_priv_data {
    uint8_t data[MAX_SIZE_FOR_PDATA];
    int size;
} broadcast_priv_data_t;

typedef struct broadcast_obj {
    uint32_t margic;
    const char *name;
    void *context;
    void (*receiver_cb)(broadcast_mask_id_t id, void *context, int32_t arg1, int32_t arg2, broadcast_priv_data_t *pdata);
    broadcast_mask_id_t id;
    uint8_t run_on_ui_thd;
} broadcast_obj_t;

/*
* 描述广播分发数据包.
*/
typedef struct bcr_pkg_struct {
    broadcast_mask_id_t id;
    int32_t arg1;
    int32_t arg2; 
    broadcast_obj_t obj;
    uint32_t timestamp;
    broadcast_priv_data_t priv_data;
    uint16_t buf_crc;
} bcr_pkg_struct_t;

/**
 * 描述广播节点结构.
*/
typedef struct {
    list_head_t list;
    broadcast_obj_t obj;
} broadcast_node_t;

/**
 * 描述广播发送包装结构.
*/
typedef struct broadcast_send_wrapper{
    broadcast_mask_id_t id;
    int32_t arg1;
    int32_t arg2;
    uint8_t *pdata;
    uint16_t pdata_len;
    uint8_t sync;
} broadcast_send_wrapper_t;

/*
 * 全局广播监听器机制.
 * 用于比较低频率的数据传递，如果是高帧率的数据传递建议用专用的监听通道.
*/
void init_broadcast_context(void);
void register_broadcast_receiver(broadcast_obj_t *obj);
void unregister_broadcast_receiver(broadcast_obj_t *obj);
void send_broadcast(broadcast_send_wrapper_t *send);
int is_broadcast_queue_busy(void);
bool is_broadcast_inited(void);
void broadcast_register_ui_thd_cb(os_message_id ui_msg_id, task_post_process_fptr **cb);

///////////////////////////////////////////////(全局广播封装)///////////////////////////////////////////////////

/**
 * id.code
*/
#define __SEND_BROADCAST(__code, __code_v2, __arg1, __arg2, __pdata, __pdata_len, __sync) \
do {  \
    broadcast_send_wrapper_t __send = { \
        .id.code = (__code), \
        .id.code_v2 = (__code_v2), \
        .arg1 = (__arg1), \
        .arg2 = (__arg2), \
        .pdata = (uint8_t *)(__pdata), \
        .pdata_len = (__pdata_len), \
        .sync = (__sync), \
    }; \
    if (!__send.pdata) { \
        __send.pdata_len = 0; \
    } \
    if (__send.pdata_len >= MAX_SIZE_FOR_PDATA) { \
        LOGE("pdata_len MUST < %d", MAX_SIZE_FOR_PDATA); \
        break; \
    } \
    send_broadcast(&__send); \
} while (0)

/**
 * id.code
*/
#define __SEND_BROADCAST_ARGS(id, pdata, pdata_len, sync) __SEND_BROADCAST(id, 0, 0, 0, pdata, pdata_len, sync)
#define __SEND_BROADCAST_ARGS0(id, sync) __SEND_BROADCAST(id, 0, 0, 0, NULL, 0, sync)
#define __SEND_BROADCAST_ARGS1(id, args1, sync) __SEND_BROADCAST(id, 0, args1, 0, NULL, 0, sync)
#define __SEND_BROADCAST_ARGS2(id, args1, args2, sync) __SEND_BROADCAST(id, 0, args1, args2, NULL, 0, sync)
#define __SEND_BROADCAST_ARGS3(id, args1, args2, pdata, pdata_len, sync) __SEND_BROADCAST(id, 0, args1, args2, pdata, pdata_len, sync)
#define __SEND_BROADCAST_ARGS4(id, args1, pdata, pdata_len, sync) __SEND_BROADCAST(id, 0, args1, 0, pdata, pdata_len, sync)

#define __SEND_BROADCAST_ARGS_V2(id, pdata, pdata_len, sync) __SEND_BROADCAST(0, id, 0, 0, pdata, pdata_len, sync)
#define __SEND_BROADCAST_ARGS0_V2(id, sync) __SEND_BROADCAST(0, id, 0, 0, NULL, 0, sync)
#define __SEND_BROADCAST_ARGS1_V2(id, args1, sync) __SEND_BROADCAST(0, id, args1, 0, NULL, 0, sync)
#define __SEND_BROADCAST_ARGS2_V2(id, args1, args2, sync) __SEND_BROADCAST(0, id, args1, args2, NULL, 0, sync)
#define __SEND_BROADCAST_ARGS3_V2(id, args1, args2, pdata, pdata_len, sync) __SEND_BROADCAST(0, id, args1, args2, pdata, pdata_len, sync)
#define __SEND_BROADCAST_ARGS4_V2(id, args1, pdata, pdata_len, sync) __SEND_BROADCAST(0, id, args1, 0, pdata, pdata_len, sync)

#define SEND_BROADCAST_ARGS_EXTRA(id, pdata, pdata_len)                             __SEND_BROADCAST_ARGS(id, (pdata), (pdata_len), false)
#define SEND_BROADCAST_ARGS3_EXTRA(id, args1, args2, pdata, pdata_len)              __SEND_BROADCAST_ARGS3(id, (args1), (args2), (pdata), (pdata_len), false)
#define SEND_BROADCAST_ARGS4_EXTRA(id, args1, pdata, pdata_len)                     __SEND_BROADCAST_ARGS4(id, (args1), (pdata), (pdata_len), false)

#define SEND_BROADCAST_ARGS_V2_EXTRA(id, pdata, pdata_len)                          __SEND_BROADCAST_ARGS_V2(id, (pdata), (pdata_len), false)
#define SEND_BROADCAST_ARGS3_V2_EXTRA(id, args1, args2, pdata, pdata_len)           __SEND_BROADCAST_ARGS3_V2(id, (args1), (args2), (pdata), (pdata_len), false)
#define SEND_BROADCAST_ARGS4_V2_EXTRA(id, args1, pdata, pdata_len)                  __SEND_BROADCAST_ARGS4_V2(id, (args1), (pdata), (pdata_len), false)

/**
 * pdata固定指针字节长度版本.
*/
#define SEND_BROADCAST_ARGS0(id)                                                    __SEND_BROADCAST_ARGS0(id, false)
#define SEND_BROADCAST_ARGS1(id, args1)                                             __SEND_BROADCAST_ARGS1(id, (args1), false)
#define SEND_BROADCAST_ARGS2(id, args1, args2)                                      __SEND_BROADCAST_ARGS2(id, (args1), (args2), false)

#define SEND_BROADCAST_ARGS0_V2(id)                                                 __SEND_BROADCAST_ARGS0_V2(id, false)
#define SEND_BROADCAST_ARGS1_V2(id, args1)                                          __SEND_BROADCAST_ARGS1_V2(id, (args1), false)
#define SEND_BROADCAST_ARGS2_V2(id, args1, args2)                                   __SEND_BROADCAST_ARGS2_V2(id, (args1), (args2), false)

/*
* 同步广播, 尽量少用同步广播.
*/
#define SEND_BROADCAST_SYNC_ARGS0(id)                                               __SEND_BROADCAST_ARGS0(id, true)
#define SEND_BROADCAST_SYNC_ARGS1(id, args1)                                        __SEND_BROADCAST_ARGS1(id, (args1), true)
#define SEND_BROADCAST_SYNC_ARGS2(id, args1, args2)                                 __SEND_BROADCAST_ARGS2(id, (args1), (args2), true)

/*
* 注册到广播上下文.
*/
#define REGISTER_TO_BROCASE_THD_POOL(ui_msg_id, process_cb_p) broadcast_register_ui_thd_cb(ui_msg_id, process_cb_p)

#ifdef __cplusplus
}
#endif

#endif 


