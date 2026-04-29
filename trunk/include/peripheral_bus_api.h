#ifndef __PERIPHERAL_BUS_API_H__
#define __PERIPHERAL_BUS_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "broadcast_manager_api.h"

typedef void (*key_input_evt_cb_fptr)(void *user_data, uint16_t keycode, uint16_t keytype);

/*
* 描述各个外设ID枚举，通过此ID查找对应的驱动api.
*/ 
typedef enum {
    ID_DEVICE_BEGIN = 0,
    ID_DEVICE_IRQ,
    ID_DEVICE_UART,
    ID_DEVICE_POWER,
    ID_DEVICE_PMU,
    ID_DEVICE_FLASH,
    ID_DEVICE_FATFS,
    ID_DEVICE_DB,
    ID_DEVICE_LED,
    ID_DEVICE_DISPLAY_PANEL,
    ID_DEVICE_INPUT_KEY,
    ID_DEVICE_BATTERY,
    ID_DEVICE_BLE,
    ID_DEVICE_WIFI,
    ID_DEVICE_IMU,
    ID_DEVICE_FSENSOR,
    ID_DEVICE_AIR_PRESSURE,
    ID_DEVICE_LIGHT_SENSOR,
    ID_DEVICE_MOTOR,
    ID_DEVICE_AUDIO,
    ID_DEVICE_MAX
} device_desc_id_e;

/* 
* 描述各个外设状态
*/ 
typedef enum {
    DEVICE_BUS_DEINIT = 0,
    DEVICE_BUS_SUSPEND = 0,
    DEVICE_BUS_INIT = 1,  
    DEVICE_BUS_RESUME = 2,  
} device_status_e;

/*
* 描述设备驱动抽象接口
*/ 
typedef struct rt_device_node {
    const char *name;
    device_desc_id_e id;
    device_status_e status;
    list_head_t list;
    int (*open)(struct rt_device_node *dev);
    int (*close)(struct rt_device_node *dev);
    int (*read)(struct rt_device_node *dev, void *buf, uint32_t n_bytes);
    int (*write)(struct rt_device_node *dev, const void *buf, uint32_t n_bytes);
    int (*ioctl)(struct rt_device_node *dev, uint32_t op_code, void *args);
    int (*suspend)(struct rt_device_node *dev);
    int (*resume)(struct rt_device_node *dev);
} rt_device_node_t;

/*
 * 设备驱动操作宏定义封装-begin.
 */
#define DEVICE_READ(dev) \
    ((rt_device_node_t *)(dev))->read((dev), (NULL), (0))

#define DEVICE_READ_ARGS(dev0, buf, n_bytes) \
        ((rt_device_node_t *)(dev0))->read((dev0), (buf), (n_bytes)); 

#define DEVICE_READ_NO_RET(dev0, buf, n_bytes) \
do { \
    rt_device_node_t *__dev = dev0; \
    if ((__dev) && (__dev)->read) { \
        DEVICE_READ((__dev), (buf), (n_bytes)); \
    } \
} while (0)

#define DEVICE_WRITE(dev0, buf, n_bytes) \
    ((rt_device_node_t *)(dev0))->write((dev0), (const void *)(buf), (n_bytes));

#define DEVICE_WRITE_NO_RET(dev0, buf, n_bytes) \
do { \
    rt_device_node_t *__dev = dev0; \
    if ((__dev) && (__dev)->write) { \
        DEVICE_WRITE((__dev), (const void *)(buf), (n_bytes)); \
    } \
} while (0)

#define DEVICE_CLOSE(dev0) \
do { \
    rt_device_node_t *__dev = dev0; \
    if ((__dev)) { \
        ((rt_device_node_t *)(__dev))->close((__dev)); \
    } \
} while (0)

#define DEVICE_IOCTL(dev, op_code) \
    ((rt_device_node_t *)(dev))->ioctl((dev), (op_code), (void *)NULL)

#define DEVICE_IOCTL_ARGS(dev, op_code, args) \
    ((rt_device_node_t *)(dev))->ioctl((dev), (uint32_t)(op_code), (void *)(args))

#define DEVICE_IOCTL_NO_RET(dev0, op_code) \
do { \
    rt_device_node_t *__dev = dev0; \
    if ((__dev) && (__dev)->ioctl) { \
        DEVICE_IOCTL(__dev, op_code); \
    } \
} while (0)

#define DEVICE_IOCTL_HAS_RET(dev0, op_code, __ret) \
int __ret = -1; \
do { \
    rt_device_node_t *__dev = dev0; \
    if ((__dev) && (__dev)->ioctl) { \
        __ret = DEVICE_IOCTL(__dev, op_code); \
    } \
} while (0)

#define DEVICE_IOCTL_ARGS_NO_RET(dev0, op_code, args) \
do { \
    rt_device_node_t *__dev = dev0; \
    if ((__dev) && (__dev)->ioctl) { \
        DEVICE_IOCTL_ARGS(__dev, op_code, args); \
    } \
} while (0)

#define DEVICE_IOCTL_ARGS_HAS_RET(dev0, op_code, args, __ret) \
int __ret = -1; \
do { \
    rt_device_node_t *__dev = dev0; \
    if ((__dev) && (__dev)->ioctl) { \
        __ret = DEVICE_IOCTL_ARGS(__dev, op_code, args); \
    } \
} while (0)


/*
* 外设总线上下文
* 所有的外设驱动都挂在这个结构内部.
*/ 
typedef struct peripheral_bus_context {
/// public:
    int (*resume)(void);
    int (*suspend)(void);
    int (*pmu_sleep_poll)(void);
    int (*add_period_wakeup_cb)(const char *tag, uint32_t period, void (*cb)(const char *reason, wakeup_type_e wakeup_type));
    int (*del_period_wakeup_cb)(void);
    void *(*get_persist_memroy)(void);
    bool (*device_init_completed)(void);

    /// 通过ID获取对应的设备驱动实例.
    void *(*find_device)(device_desc_id_e id);

    /// 外部钩子函数.
    struct __peripheral_bus_hook {
        int (*on_module_init)(struct peripheral_bus_context *context);
        int (*on_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
        int (*uart_recv_cb_dispatch_route)(struct peripheral_bus_context *context, const char *cmd, int length);
        void (*wait_dispose_complete)(list_head_t *__list_head, device_status_e status);
        void (*wait_ship_mode_exit)(void);
    } *hook;

/// private:
    struct __peripheral_private {
        broadcast_obj_t *receiver;
        list_head_t device_obj_list;
        int is_init_ready;
    } priv;

} peripheral_bus_context_t;


#ifdef __cplusplus
}
#endif

#endif 

