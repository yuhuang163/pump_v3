/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" 
#include "root_common_register.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static peripheral_bus_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static const device_register_fptr s_device_register[] = {

    #if IS_SUPPORT_DEVICE_IRQ
    register_device_for_irq,
    #endif
    #if IS_SUPPORT_DEVICE_LOG_UART
    register_device_for_log_uart,
    #endif
    #if IS_SUPPORT_DEVICE_POWER
    register_device_for_power,
    #endif
    #if IS_SUPPORT_DEVICE_PMU
    register_device_for_pmu,
    #endif
    #if IS_SUPPORT_DEVICE_DATABASE
    register_device_for_database,
    #endif
    #if IS_SUPPORT_DEVICE_FLASH
    register_device_for_flash,
    #endif
    #if IS_SUPPORT_DEVICE_LED
    register_device_for_led,
    #endif
    #if IS_SUPPORT_DEVICE_LCD
    register_device_for_lcd,
    #endif
    #if IS_SUPPORT_DEVICE_INPUT_KEY
    register_device_for_input_key,
    #endif
    #if IS_SUPPORT_DEVICE_BATTERY
    register_device_for_battery,
    #endif
    #if IS_SUPPORT_DEVICE_IMU
    register_device_for_imu,
    #endif
    #if IS_SUPPORT_DEVICE_FSENSOR
    register_device_for_force_sensor,
    #endif
    #if IS_SUPPORT_DEVICE_MOTOR
    register_device_for_motor,
    #endif
    #if IS_SUPPORT_DEVICE_AUDIO
    register_device_for_audio,
    #endif
    #if IS_SUPPORT_DEVICE_BLE
    register_device_for_ble,
    #endif
    #if IS_SUPPORT_DEVICE_WIFI
    register_device_for_wifi,
    #endif
};

static void on_device_register_receiver(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    peripheral_bus_context_t *ctx = context;

    if (ctx->hook && ctx->hook->on_receiver_route) {
        ctx->hook->on_receiver_route(id, context, arg1, arg2, pdata);
    }

    LOG_PBUS("device: init_ready");
    ctx->priv.is_init_ready = 1;
}

#if UART_DEBUG_RECEIVER_SUPPORT
static void uart_recv_cb_dispatch(const char *cmd, int length)
{
    peripheral_bus_context_t *ctx = thiz;

    if (ctx->hook && ctx->hook->uart_recv_cb_dispatch_route) {
        if (ctx->hook->uart_recv_cb_dispatch_route(ctx, cmd, length)) {
            return;
        }
    }

    uart_recv_cb_internal(cmd, length);
}
#endif

static bool device_init_completed(void)
{
    if (thiz) {
        return thiz->priv.is_init_ready == 1;
    }
    return 0;
}

static int on_post_pbus_init(void *ptr)
{
    peripheral_bus_context_t *ctx = ptr;
    list_head_t *__list_head = &ctx->priv.device_obj_list;

    #if IS_SUPPORT_DEVICE_FS
    if (ctx->fs_remount) {
        ctx->fs_remount();
    }
    #endif

    if (ctx->hook && ctx->hook->on_module_init) {
        ctx->hook->on_module_init(ctx);
    }

    LIST_FOR_EACH(pos, __list_head) {
        rt_device_node_t *node = CONTAINER_OF(pos, rt_device_node_t, list);
        if (node->open) {
            // LOG_PBUS("node:(%s/%d) ",node->name, node->id);
            int ret = node->open(node);
            if (ret) {
                LOGE("node:(%s/%d) open fail, ret=%d", node->name, node->id, ret);
            }
        }
    }

    #if UART_DEBUG_RECEIVER_SUPPORT
    rt_device_node_t *uart_dev = ctx->find_device(ID_DEVICE_LOG_UART);
    DEVICE_IOCTL_ARGS_NO_RET(uart_dev, OP_CODE_UART_ADD_RECV_CB, uart_recv_cb_dispatch);
    #endif

    // 注册全局广播.
    ctx->priv.receiver = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.receiver->context = ctx;
    ctx->priv.receiver->name = "device_register";
    ctx->priv.receiver->id.code |= ID_BROADCAST_SYS_BOOT_COMPLETED;
    ctx->priv.receiver->receiver_cb = on_device_register_receiver;
    register_broadcast_receiver(ctx->priv.receiver);

    return 0;
}

static int on_pbus_resume(void)
{
    list_head_t *__list_head = &thiz->priv.device_obj_list;

    LIST_FOR_EACH(pos, __list_head) {
        rt_device_node_t *node = CONTAINER_OF(pos, rt_device_node_t, list);
        if (node->resume) {
            // LOG_PBUS("node:(%s/%d) ",node->name, node->id);
            int ret = node->resume(node);
            if (ret) {
                LOGE("node:(%s/%d) resume fail, ret=%d", node->name, node->id, ret);
            }
        }
    }
    
    return 0;
}

static int on_pbus_suspend(void)
{
    list_head_t *__list_head = &thiz->priv.device_obj_list;

    LIST_FOR_EACH(pos, __list_head) {
        rt_device_node_t *node = CONTAINER_OF(pos, rt_device_node_t, list);
        if (node->suspend) {
            // LOG_PBUS("node:(%s/%d) ",node->name, node->id);
            int ret = node->suspend(node);
            if (ret) {
                LOGE("node:(%s/%d) suspend fail, ret=%d", node->name, node->id, ret);
            }
        }
    }

    return 0;
}

static int pmu_sleep_poll(void)
{
    rt_device_node_t *dev = thiz->find_device(ID_DEVICE_PMU);
    DEVICE_IOCTL_HAS_RET(dev, OP_CODE_PMU_SLEEP_POLL, ret);

    return ret;
}

// 注册周期性唤醒callback.
static int add_period_wakeup_cb(const char *tag, uint32_t period, void (*cb)(const char *reason, wakeup_type_e wakeup_type))
{
    rt_device_node_t *pmu_dev = thiz->find_device(ID_DEVICE_PMU);

    LOGI("(tag=%s)__add: %ld", tag, period);

    if (pmu_dev && pmu_dev->ioctl) {
        DEVICE_IOCTL_ARGS(pmu_dev, OP_CODE_PMU_REGISTER_TIMER_WAKEUP, &period);
        DEVICE_IOCTL_ARGS(pmu_dev, OP_CODE_PMU_ADD_WAKEUP_CB, cb);
        return 0;
    }

    return -1;
}

// 注消周期性唤醒callback.
static int del_period_wakeup_cb(void)
{
    rt_device_node_t *dev = thiz->find_device(ID_DEVICE_PMU);

    LOGI("__del");
    DEVICE_IOCTL_NO_RET(dev, OP_CODE_PMU_UNREGISTER_TIMER_WAKEUP);
    DEVICE_IOCTL_NO_RET(dev, OP_CODE_PMU_REMOVE_WAKEUP_CB);

    return 0;
}

static void *find_device(device_desc_id_e id)
{
    peripheral_bus_context_t *ctx = thiz;
    list_head_t *__list_head = &ctx->priv.device_obj_list;
    static void *__cache[ID_DEVICE_MAX] = {0};

    if (id >= ID_DEVICE_MAX || id <= ID_DEVICE_BEGIN) {
        LOGE("error input id: %d", id);
        return NULL;
    }

    if (__cache[id] != NULL) {
        return __cache[id];
    }

    LIST_FOR_EACH(pos, __list_head) {
        rt_device_node_t *node = CONTAINER_OF(pos, rt_device_node_t, list);
        if (node->id == id) {
            // 缓存,加速.
            if (__cache[id] == NULL) {
                __cache[id] = node;
            }
            return node;
        }
    }

    LOGE("cannot find device: %d", id);

    return NULL;
}

static int register_everyone_device(list_head_t *__list_head)
{
    for (int i = 0; i < LENGTH_OF_ARRAY(s_device_register); i++) {
        rt_device_node_t *node = s_device_register[i](__list_head);
        if (node) {
            // LOG_PBUS("%d:node:%p,%s(%d) has registered.", i, node, node->name, node->id);
        }
    }

    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* @brief 所有外设驱动都挂在pbus下.
*/
void *register_module_for_peripheral_bus_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, peripheral_bus_context_t);

    thiz = ptr;
    ptr->suspend = on_pbus_suspend;
    ptr->resume = on_pbus_resume;
    ptr->pmu_sleep_poll = pmu_sleep_poll;
    ptr->add_period_wakeup_cb = add_period_wakeup_cb;
    ptr->del_period_wakeup_cb = del_period_wakeup_cb;
    ptr->find_device = find_device;
    ptr->device_init_completed = device_init_completed;
    register_everyone_device(&ptr->priv.device_obj_list);

    return ADD_MODULE("device-bus", ID_MODULE_DEVICE_BUS, 0, on_post_pbus_init, 
        ptr, list_head, sizeof(peripheral_bus_context_t));
}
