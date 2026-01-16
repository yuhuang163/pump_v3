/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "root_common_api.h"
#include "root_log_define.h"
#include "misc_manager.h"

/*
* DEFINES
*****************************************************************************************
*/
#ifndef MISC_POLLING_PERIOD
#define MISC_POLLING_PERIOD                               (100)  
#endif

#ifndef SYSINFO_PRINT_PERIOD
#define SYSINFO_PRINT_PERIOD                              (5000)
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static misc_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void *list_del_condition_cb(void *data, list_head_t *pos)
{
    on_misc_task_cb_fptr cb = (on_misc_task_cb_fptr)data;
    misc_task_node_t *node = CONTAINER_OF(pos, misc_task_node_t, list);
    
    if ((node->cb) == (cb)) {
        LOG_MISC("del: user_data: %lx, cb: %lx, task_id: %d", node->user_data, node->cb, node->task_id);
        return node;
    }

    return NULL;
}

static void add_misc_task(void *user_data, int task_id, uint32_t polling_period, misc_work_msg_id_mask_t msg_id_mask, on_misc_task_cb_fptr cb)
{
    misc_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.task_list;
    
    MUTEX_LOCK(obj->priv.m_lock);

    // check duplicate add.
    LIST_FOR_EACH(pos, (head)) { 
        misc_task_node_t *node = CONTAINER_OF(pos, misc_task_node_t, list);
        if (node->cb == cb && node->user_data == user_data) {
            LOG_MISC("ignore duplicate add: user_data: %lx, cb: %lx, task_id: %d", user_data, cb, task_id);
            MUTEX_UNLOCK(obj->priv.m_lock);
            return;
        }
    }

    misc_task_node_t *node = u_malloz(sizeof(misc_task_node_t));
    node->cb = cb;
    node->task_id = task_id;
    node->polling_period = polling_period;
    node->msg_id_mask = msg_id_mask;
    node->user_data = user_data;
    node->last_exec_time = get_timestamp();
    __list_add(&node->list, head);
    LOG_MISC("add: user_data: %lx, cb: %lx, task_id: %d, polling_period: %lu, msg_id_mask: 0x%x", 
        user_data, cb, task_id, polling_period, msg_id_mask);

    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void remove_misc_task(on_misc_task_cb_fptr cb)
{
    misc_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.task_list;
    
    MUTEX_LOCK(obj->priv.m_lock);
    __list_del(head, cb, list_del_condition_cb);
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void trigger_misc_work(misc_work_msg_id_mask_t msg_id_mask)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(thiz->priv.misc_work_thd.msg_id);
    MSG_QUEUE_SEND(thiz->priv.misc_work_thd.msg_id, &msg_id_mask, 0);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void misc_work_broadcast_receive(broadcast_mask_id_t id, void *context, int32_t args0, int32_t args1,
                                        broadcast_priv_data_t *pdata)
{
    misc_manager_context_t *ctx = (misc_manager_context_t *)context;
    if (id.code == ID_BROADCAST_PM_STATE_CHANGE) {
        int pm_state = args0;
        if (pm_state == ID_SYS_PM_SUSPEND) {
            LOG_MISC("STOP_TIMER: misc_timer");
            STOP_TIMER(ctx->priv.misc_timer);
        } else {
            LOG_MISC("START_TIMER: id = 0x%lx, misc_timer", id.code);
            START_TIMER(ctx->priv.misc_timer, MISC_POLLING_PERIOD);
        }
    } 
}

static void on_misc_timer_cb(void *handle)
{
    misc_work_msg_id_mask_t msg_mask = (misc_work_msg_id_mask_t)MISC_WORK_MSG_ID_TIMER;
    MSG_QUEUE_SEND(thiz->priv.misc_work_thd.msg_id, &msg_mask, 0);
}

static void misc_work_polling_thread(const void *args)
{
    uint8_t sys_ready = 0;
    thread_context_t *thd = (thread_context_t *)args;
    misc_manager_context_t *ctx = thiz;
    uint32_t current_time = 0;
    misc_work_msg_id_mask_t msg_mask = 0;
    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, MISC_WORK_DISPATCH_MESQ_NUM, sizeof(misc_work_msg_id_mask_t));
    CREATE_TIMER(&ctx->priv.misc_timer, "misc-timer", ctx, OS_TIMER_PERIODIC, on_misc_timer_cb);
    START_TIMER(ctx->priv.misc_timer, MISC_POLLING_PERIOD);
    for (;;) {
        MSG_QUEUE_RECV(thd->msg_id, &msg_mask, OS_WAIT_FOREVER);
        current_time = get_timestamp();
        MUTEX_LOCK(ctx->priv.m_lock);
        LIST_FOR_EACH(pos, (&ctx->priv.task_list)) {
            misc_task_node_t *node = CONTAINER_OF(pos, misc_task_node_t, list);
            /// 检查消息ID位掩码是否匹配
            if ((node->msg_id_mask & msg_mask) == 0) {
                continue;  /// 该任务不关心此消息ID掩码，跳过
            }
            uint32_t elapsed = current_time - node->last_exec_time;
            if (elapsed >= node->polling_period) {
                if (node->cb) {
                    node->cb(node->user_data); /// 执行回调.
                }
                node->last_exec_time = current_time; /// 更新执行时间.
            }
        }
        MUTEX_UNLOCK(ctx->priv.m_lock);
    }
}

static int on_post_misc_init(void *ptr)
{
    misc_manager_context_t *ctx = ptr;
    /// 注册广播监听.
    ctx->priv.misc_bcd = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.misc_bcd->context = ctx;
    ctx->priv.misc_bcd->name = "misc_bcd_ctx";
    ctx->priv.misc_bcd->id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    ctx->priv.misc_bcd->receiver_cb = misc_work_broadcast_receive;
    register_broadcast_receiver(ctx->priv.misc_bcd);
    ctx->add_misc_task(ctx, MISC_TASK_ID_SYSINFO_PRINT, SYSINFO_PRINT_PERIOD, MISC_WORK_MSG_ID_TIMER,
         platform_sysinfo_print);
    RUN_NEW_THREAD(&ctx->priv.misc_work_thd, "misc_thd",
        misc_work_polling_thread, RT_PRIO_MISC_DISPATCH_THREAD, MISC_THREAD_STACKSIZE);    

    return 0;
}

static int on_perpare_misc_init(void *self)
{
    misc_manager_context_t *ptr = self;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);    
    /// 初始化互斥锁和任务链表
    MUTEX_INIT(&ptr->priv.m_lock, misc_task_mutex);
    USE_INIT_LIST_HEAD(&ptr->priv.task_list);

    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_misc_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, misc_manager_context_t);
    ptr->add_misc_task = add_misc_task;
    ptr->remove_misc_task = remove_misc_task;
    ptr->trigger_misc_work = trigger_misc_work;
    thiz = ptr;
    return ADD_MODULE("misc", ID_MODULE_MISC, on_perpare_misc_init, on_post_misc_init, 
        ptr, list_head, sizeof(misc_manager_context_t));
}
