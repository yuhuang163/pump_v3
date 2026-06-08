/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "cmsis_os_api.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "handler_post.h" 

/*
* DEFINES
*****************************************************************************************
*/
#ifndef HDR_WORK_THREAD_NUM
#define HDR_WORK_THREAD_NUM (2)
#endif

#ifndef DEBUG_HDR_TASK
#define DEBUG_HDR_TASK (1)
#endif

#ifndef HDR_WM_QUEUE_SEND_RETRY_TIMEOUT_MS
#define HDR_WM_QUEUE_SEND_RETRY_TIMEOUT_MS (30)
#endif

#ifndef HDR_TIMER_TIMING_WARN_MS
#define HDR_TIMER_TIMING_WARN_MS (30)
#endif

#define HDR_OBJ_MARGIC_CODE (0xaabbccdd)

#if DEBUG_HDR_TASK
#define dump_hdr_node(tag, _node) \
do { \
    LOG_HDR("%s: name:%s, node: 0x%lx, excpect_time: %lu, cb: 0x%lx, arg1: %ld, arg2: %ld, user_data: 0x%lx",  \
        tag, _node->args.name, TO_PRINT(_node), _node->excpect_time, TO_PRINT(_node->args.cb), \
        _node->args.arg1, _node->args.arg2, TO_PRINT(_node->args.user_data)); \
} while (0)
#else
#define dump_hdr_node(tag, _node) do { } while (0)
#endif

typedef struct {
    os_mutex_id hdr_lock;
    os_timer_id hdr_timer;
    uint32_t start_ticks;
    uint32_t elapsed_time;
    list_head_t hdr_post_list;
    thread_context_t hdr_post_thd[HDR_WORK_THREAD_NUM];
    os_message_id wm_msg_id;
} hdr_post_context_t;

typedef struct {
    int id;
    uint32_t total_run_time;
} task_running_time_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static char *s_hdr_thd_name[] = {
    "hdr_thd0",
    "hdr_thd1",
    "hdr_thd2",
    "hdr_thd3",
};

static hdr_post_context_t *thiz = {0};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void hdr_dispatch_task(hdr_post_context_t *ctx, hdr_post_args_t *post)
{
    thread_context_t *thd = (thread_context_t *)ctx->hdr_post_thd;
    task_running_time_t running[HDR_WORK_THREAD_NUM] = {0};
    multi_func_msg_data_t msg = { 0 };
    hdr_post_args_t *hdr_post = &msg.union_body.hdr_post;
    int i = 0;
    int j = 0;
    memcpy(hdr_post, post, sizeof(hdr_post_args_t));

    /// 是否来自wm任务.
    if (hdr_post->run_on_wm_thd == 0x1) {
        if (ctx->wm_msg_id) {
            hdr_post->timestamp = get_timestamp();
            #if DEBUG_HDR_TASK
            LOG_HDR("post(wm):%p(time:%ld) -> %s", ctx->wm_msg_id, hdr_post->timestamp, post->name);
            #endif
            msg.msg_type = MSG_TYPE_HANDLER_POST_EVENT;
            MSG_QUEUE_SEND(ctx->wm_msg_id, (void *)&msg, 0);
            return;
        }
    }

    /// 默认推到线程1，若线程1忙则推到线程2.
    for (i = 0; i < HDR_WORK_THREAD_NUM; i++) {
        uint32_t run_timestamp = get_thread_running(&thd[i]);
        if (run_timestamp == 0) {
            hdr_post->timestamp = get_timestamp();
            #if DEBUG_HDR_TASK
            LOG_HDR("post:%d(%s/time:%ld) -> %s", i, 
                THREAD_NAME_GET(&thd[i]), hdr_post->timestamp, post->name);
            #endif
            MSG_QUEUE_SEND(thd[i].msg_id, (void *)&msg, 0);
            break;
        }

        /// 新任务不投递到独占任务线程.
        if (thd[i].exclusive_task != 0) {
            continue;
        }

        /// 运行中的短时任务加入到数组中保存,用于策略调度.
        running[j].id = i;
        running[j].total_run_time = get_timestamp_ms(run_timestamp);
        j++;
    }

    /// 将各个线程运行任务时间排序，投递到运行任务时间最长的线程中.
    if (i >= HDR_WORK_THREAD_NUM) {
        task_running_time_t r0 = { .id = 0, .total_run_time = 0 };
        for (int k = 0; k < j; k++) {
            uint32_t t = running[k].total_run_time;
            if (r0.total_run_time < t) {
                memcpy(&r0, &running[k], sizeof(task_running_time_t));
            }
            LOGE("LOG_HDR:%d #id=%d(total_run_time:%ld)", k, r0.id, r0.total_run_time);
        }
        hdr_post->timestamp = get_timestamp();
        LOGE("LOG_HDR:post#%d(%s/time:%ld) -> %s", r0.id, 
            THREAD_NAME_GET(&thd[r0.id]), hdr_post->timestamp, post->name);
        MSG_QUEUE_SEND(thd[r0.id].msg_id, (void *)&msg, 0);
    }
}

static int hdr_post_process_cb(multi_func_msg_data_t *msg, thread_context_t *thd)
{
    uint32_t exec_time, diff_time;
    hdr_post_args_t *target = &msg->union_body.hdr_post;

    if (!(target->margic == HDR_OBJ_MARGIC_CODE)) {
        LOGE("invald MARGIC_CODE!(%lx)", target->margic);
        return -1;
    }
    target->thd = thd;

    /// 是否独占线程任务.
    if (target->exclusive_task) {
        thd->exclusive_task = 1;
    }

    #if DEBUG_HDR_TASK
    LOG_HDR("(%s)(from:%lu->%lu#name: %s/time=%ld), arg1: %ld, arg2: %ld******", THREAD_NAME_GET(thd),
        target->from_tid, (uint32_t)(thd->tid) % 1000, target->name, target->timestamp, target->arg1, target->arg2);
    #endif
    if (target->cb == NULL) {
        LOGE("null cb, name: %s", target->name ? target->name : "null");
        return -1;
    }

    exec_time = get_timestamp();
    thd->task_name = (char *)target->name;
    target->cb(target);
    thd->task_name = "idle";
    diff_time = (get_timestamp_ms(exec_time));
    if (thd->exclusive_task) {
        thd->exclusive_task = 0;
    }

    #if DEBUG_HDR_TASK
    LOGW("(%s)endtime(from:%lu->%lu#name: %s/time=%ld), exec_time: %ld(ms)******", THREAD_NAME_GET(thd),
        target->from_tid, (uint32_t)(thd->tid) % 1000, target->name, target->timestamp, diff_time);
    #else
    if (diff_time >= 30) {
        LOGW("(%s)endtime(from:%lu->%lu#name: %s/time=%ld), exec_time: %ld(ms)******", THREAD_NAME_GET(thd),
            target->from_tid, (uint32_t)(thd->tid) % 1000, target->name, target->timestamp, diff_time);
    }
    #endif
    return 0;
}

static void task_exec_thread(const void *args)
{
    thread_context_t *thd = (thread_context_t *)args;
    multi_func_msg_data_t msg = {0};

    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, BCR_THD_MESQ_NUM /2, sizeof(multi_func_msg_data_t));
    LOG_HDR("init: %ld", (uint32_t)thd->tid);

    thd->task_name = "idle";

    do {
        set_thread_running(thd, false);
        MSG_QUEUE_RECV(thd->msg_id, (void *)&msg, OS_WAIT_FOREVER);
        set_thread_running(thd, true);
        hdr_post_process_cb(&msg, thd);
    } while (1);
}

static void post_timer_callback(void *handle)
{   
    hdr_post_context_t *ctx = thiz;
    list_head_t *head = &ctx->hdr_post_list;
    list_head_t *prev = NULL;
    uint32_t elapsed_time = ctx->elapsed_time;
    int32_t next_arrival = INT32_MAX;

    /// 先停掉.
    STOP_TIMER(ctx->hdr_timer);
    MUTEX_LOCK(ctx->hdr_lock);
    for (list_head_t *pos = head->next; pos != NULL; ) {
        hdr_post_node_t *node = CONTAINER_OF(pos, hdr_post_node_t, list);

        /// 1. 依次将每个task的时间减掉该次已经消耗的时间
        if (node->arrival_time > elapsed_time) {
            node->arrival_time -= elapsed_time;
        } else {
            node->arrival_time = 0;
        }

        /// 2. 任务时间到,推送执行.
        if (node->arrival_time == 0) {
            /// 计算时间补偿.
            int exec_time = get_timestamp_ms(node->create_time);
            if (exec_time < node->excpect_time) {
                node->arrival_time = node->excpect_time - exec_time;
                #if DEBUG_HDR_TASK
                if (node->arrival_time > HDR_TIMER_TIMING_WARN_MS) {
                    LOG_HDR("make amends(node=0x%lx) name:%s, arrival_time: %lu",
                        TO_PRINT(node), node->args.name, node->arrival_time);
                }
                #endif
                /// 尝试下次调度.
                goto make_amends;
            }
            #if DEBUG_HDR_TASK
            if ((uint32_t)exec_time > node->excpect_time + HDR_TIMER_TIMING_WARN_MS) {
                LOGW("POST_EXEC late(node=0x%lx) name:%s, excpect_time:%lu, real_time: %u",
                    TO_PRINT(node), node->args.name, node->excpect_time, exec_time);
            }
            #endif
            hdr_dispatch_task(ctx, &node->args);

            /// delete node.
            if (pos == head->next) {
                head->next = pos->next;
            } else {
                prev->next = pos->next;
            }
            pos = pos->next;
            u_free(node);
            continue;
        }

make_amends:
        prev = pos;
        pos = pos->next;

        /// 3. 找出最早的任务节点.
        if (next_arrival > node->arrival_time) {
            next_arrival = node->arrival_time;
        }
    }

    MUTEX_UNLOCK(ctx->hdr_lock);

    /// 3. 新一轮的调度.
    if (next_arrival == INT32_MAX) {
        LOG_HDR("[STOP.]");
    } else {
        ctx->elapsed_time = next_arrival;
        START_TIMER(ctx->hdr_timer, next_arrival);
        /// 记录当前的系统时间.
        ctx->start_ticks = get_timestamp();
    }
}

static void *handler_post_list_del_cb(void *data, list_head_t *pos)
{
    const char *task_name = (const char *)data;
    hdr_post_node_t *node = CONTAINER_OF(pos, hdr_post_node_t, list);

    if (task_name != NULL 
        && node->args.name != NULL 
        && (strncmp(node->args.name, task_name, strlen(node->args.name)) == 0)) {
        dump_hdr_node("@remove", node);
        return node;
    }

    return NULL;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
/**
* @brief 判断投递队列是否繁忙.
*/
int is_handler_post_queue_busy(void)
{
    hdr_post_context_t *ctx = thiz;
    thread_context_t *thd = (thread_context_t *)ctx->hdr_post_thd;
    char *task_name = NULL;

    /// 需要排除的任务.
    const char *m_skip_hdr_task_name[] = { HDR_SKIP_TASK_SET() };

    for (int i = 0; i < HDR_WORK_THREAD_NUM; i++) {
        if (get_thread_running(&thd[i]) > 0) {
            task_name = thd[i].task_name;
            int found = 0;
            for (int j = 0; j < LENGTH_OF_ARRAY(m_skip_hdr_task_name); j++) {
                if (task_name != NULL && strstr(task_name, m_skip_hdr_task_name[j]) != NULL) {
                    found = 1;
                    break;
                }
            }

            if (found) {
                continue;
            }

            if (thd[i].exclusive_task) {
                continue;
            }

            if (task_name != NULL && strstr(task_name, __OTA_UPGRADE_RES_TASK) != NULL) {
                return -1;
            }

            LOG_HDR("#thread: %d(%s) busy!", i, task_name);
            return -2;
        }
    }

    return 0;
}

/**
* @brief 将wm线程消息句柄加入到投递线程池中.
*/
void handle_post_register_wm_thd_cb(os_message_id wm_msg_id, task_post_process_fptr *cb)
{
    hdr_post_context_t *ctx = thiz;
    ctx->wm_msg_id = wm_msg_id;
    *cb = hdr_post_process_cb;
}

/**
* @brief 移除已经投递的延迟任务.
*/
int remove_task_handler(const char *task_name)
{
    hdr_post_context_t *ctx = thiz;
    list_head_t *head = &ctx->hdr_post_list;
    int ret;

    MUTEX_LOCK(ctx->hdr_lock);
    ret = __list_del(head, (void *)task_name, handler_post_list_del_cb);
    MUTEX_UNLOCK(ctx->hdr_lock);

    return ret;
}

/**
* @brief 投递新延迟任务.
*/
void post_task_handler(hdr_post_args_t *args, uint32_t delay_ms)
{
    uint32_t delay_diff = 0;
    hdr_post_context_t *ctx = thiz;
    list_head_t *head = &ctx->hdr_post_list;
    int task_count = 0;
    char new_task[32];
    /// 打上标签.
    args->margic = HDR_OBJ_MARGIC_CODE;
    /// 源线程.
    args->from_tid = platform_get_tid();
    /// 加入任务链表,填充延迟时间.
    MUTEX_LOCK(ctx->hdr_lock);
    /// 提供直接推送异步执行的途径.
    if (delay_ms < 15) {
        hdr_dispatch_task(ctx, args);
        MUTEX_UNLOCK(ctx->hdr_lock);
        return;
    }

    LIST_FOR_EACH(pos, head) {
        task_count++;
    }
    sprintf(new_task, "add-new-task(%d)", task_count);
    /// 记录被打断的时间差，需要作为定时补偿.
    hdr_post_node_t *node = u_malloz(sizeof(hdr_post_node_t));
    node->create_time = get_timestamp();
    delay_diff = node->create_time - ctx->start_ticks;
    node->arrival_time = delay_ms + delay_diff;
    node->excpect_time = delay_ms;
    memcpy(&node->args, args, sizeof(hdr_post_args_t));
    __list_add(&node->list, head);
    dump_hdr_node(new_task, node);
    MUTEX_UNLOCK(ctx->hdr_lock);
    #if DEBUG_HDR_TASK
    LOG_HDR("delay_diff: %lu", delay_diff);
    #endif
    ctx->elapsed_time = delay_diff;
    STOP_TIMER(ctx->hdr_timer);
    post_timer_callback((void *)ctx);
}
 
/**
* @brief 初始化投递机制上下文.
*/
void init_handler_post_context(void)
{
    TRACE_FUNC_ENTRY();
    hdr_post_context_t *ctx;

    LOG_HDR("hdr_post_args_t size: %d", sizeof(hdr_post_args_t));
    thiz = u_malloz(sizeof(hdr_post_context_t));
    ctx = thiz;
    if (!ctx) {
        LOG_HDR("thiz u_malloz fail!");
        return;
    }

    CREATE_TIMER(&ctx->hdr_timer, "hdr_post_timer", ctx, OS_TIMER_PERIODIC, post_timer_callback);
    if (ctx->hdr_timer == NULL) {
        LOG_HDR("handler_post_timer is null!");
        return;
    }

    USE_INIT_LIST_HEAD(&ctx->hdr_post_list);
    MUTEX_INIT(&ctx->hdr_lock, m_hdr_mutex);
    for (int i = 0; i < HDR_WORK_THREAD_NUM; i++) {
        MUTEX_INIT(&ctx->hdr_post_thd[i].lock_id, "hd_lock");
        RUN_NEW_THREAD(&ctx->hdr_post_thd[i], s_hdr_thd_name[i], 
            task_exec_thread, RT_PRIO_HDR_POST_THREAD, (HDR_THREAD_STACKSIZE));
    }
}
 