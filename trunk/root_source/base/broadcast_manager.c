/*
 * INCLUDE FILES
 *****************************************************************************************
 */
 #include "root_common_api.h"  // IWYU pragma: keep
 #include "broadcast_manager_api.h"
 
 /*
  * DEFINES
  *****************************************************************************************
  */
 
 #define DEBUG_BROADCAST_LIST 1
 #define DEBUG_BCR_DISPATCH (1)
 
 #if DEBUG_BROADCAST_LIST
 #define DUMP_NODE(tag, _node) \
 do { \
     LOG_BCR("%s: node: %lx(%s), id: 0x%lx/0x%lx, context: %lx, receiver: %lx", \
         tag, \
         TO_PRINT(_node), \
         _node->obj.name, \
         _node->obj.id.code, \
         _node->obj.id.code_v2, \
         TO_PRINT(_node->obj.context), \
         TO_PRINT(_node->obj.receiver_cb)); \
 } while (0)
 
 #define DUMP_OBJ_BEGAN(tag, _pkg, _tid, _thd_name) \
 do { \
     LOG_BCR("%s(%s:%d): bcr_id: 0x%lx/0x%lx, name: %s/time=%ld, context: 0x%lx, arg1: %ld, arg2: %ld", \
         tag, \
         (_thd_name), \
         (_tid), \
         (_pkg)->id.code, \
         (_pkg)->id.code_v2, \
         (_pkg)->obj.name, \
         (_pkg)->timestamp, \
         TO_PRINT((_pkg)->obj.context), \
         TO_PRINT((_pkg)->arg1), \
         TO_PRINT((_pkg)->arg2) \
         ); \
 } while (0)
 
 #define DUMP_OBJ_END(tag, _pkg, _tid, _thd_name, _exec_time) \
 do { \
     LOG_BCR("%s(%s:%d): brc_id: 0x%lx/0x%lx, name: %s/time=%ld, exec_time: %ld(ms)", \
         tag, \
         (_thd_name), \
         (_tid), \
         (_pkg)->id.code, \
         (_pkg)->id.code_v2, \
         (_pkg)->obj.name, \
         (_pkg)->timestamp, \
         (_exec_time)); \
 } while (0)
 
 
 #define DUMP_ALL_LIST(head) \
 do { \
     LIST_FOR_EACH(pos, (head)) {  \
         broadcast_node_t *_node = CONTAINER_OF(pos, broadcast_node_t, list);  \
         DUMP_NODE("all", _node); \
     } \
 } while (0)
 
 #else
 #define DUMP_NODE(...)
 #define DUMP_OBJ_BEGAN(...)
 #define DUMP_OBJ_END(...)
 #define DUMP_ALL_LIST(...)
 #endif
 
 #define BCR_OBJ_MARGIC_CODE (0x11223344)
 
 #ifndef BCR_WORK_THREAD_NUM
 #define BCR_WORK_THREAD_NUM (2)
 #endif
 
 #define WAIT_TASK_EXEC_FINISH(id, thd, name, max_time) \
     int __timeout = 0; \
     do { \
         LOG_BCR("wait task [%s/%lx] finish: %d", (name), id, __timeout); \
         msleep(30); \
     } while ((get_thread_running(thd) > 0) && (++__timeout < max_time))
 
 /*
 * @brief 描述广播调度上下文.
 */
 typedef struct {
     os_mutex_id m_lock;
     list_head_t list;
     thread_context_t bcr_work_thd[BCR_WORK_THREAD_NUM];
     os_message_id ui_msg_id;
 } bcr_context_t;
 
 /*
  * LOCAL VARIABLE DEFINITIONS
  *****************************************************************************************
  */
 const static char *s_bcr_thd_name[] = {
     "bcr_work_0",
     "bcr_work_1",
     "bcr_work_2",
 };
 
 static bcr_context_t *thiz = NULL;
 /*
  * GLOBAL VARIABLE DEFINITIONS
  *****************************************************************************************
  */
 
 /*
  * LOCAL FUNCTION DEFINITIONS
  *****************************************************************************************
  */
 static void *list_del_condition_callback(void *data, list_head_t *pos)
 {
     broadcast_obj_t *obj = data;
 
     broadcast_node_t *node = CONTAINER_OF(pos, broadcast_node_t, list);
     if ((node->obj.id.code == obj->id.code) 
         && (node->obj.id.code_v2 == obj->id.code_v2) 
         && (node->obj.receiver_cb == obj->receiver_cb)
         && (node->obj.context == obj->context)) {
         DUMP_NODE("del", node);
         return node;
     }
 
     return NULL;
 }
 
 static void post_bcr_task(bcr_context_t *ctx, broadcast_send_wrapper_t *send, broadcast_obj_t *obj, int is_v2, int index)
 {
     thread_context_t *thd = (thread_context_t *)ctx->bcr_work_thd;
     multi_func_msg_data_t msg = {0};
     bcr_pkg_struct_t *pkg = &msg.union_body.bcr_pkg;
     int i = 0;
 
     pkg->id.code = send->id.code;
     pkg->id.code_v2 = send->id.code_v2;
     pkg->arg1 = send->arg1;
     pkg->arg2 = send->arg2;
 
     memcpy(&pkg->obj, obj, sizeof(broadcast_obj_t));
     memset(&pkg->priv_data, 0, sizeof(broadcast_priv_data_t));
 
     pkg->priv_data.size = send->pdata_len;
     if ((send->pdata != NULL) && (pkg->priv_data.size < MAX_SIZE_FOR_PDATA && pkg->priv_data.size > 0)) {
         memcpy((void *)pkg->priv_data.data, send->pdata, pkg->priv_data.size);
     }
  
     pkg->buf_crc = CRC16(&pkg->priv_data, sizeof(broadcast_priv_data_t));
 
     /// 是否需要推到UI线程.
     if (obj->run_on_ui_thd == 0x01) {
         if (ctx->ui_msg_id) {
             pkg->timestamp = get_timestamp();
             #if DEBUG_BCR_DISPATCH
             LOG_BCR("=>now(%d):0x%lx(%s/time=%ld) -> UI", index, (is_v2 ? send->id.code_v2 : send->id.code), obj->name, pkg->timestamp);
             #endif
             msg.msg_type = MSG_TYPE_BROADCAST_EVENT;
             MSG_QUEUE_SEND(ctx->ui_msg_id, (void *)&msg, 0);
             return;
         }
     }
 
     for (i = 0; i < BCR_WORK_THREAD_NUM; i++) {
         if (get_thread_running(&thd[i]) == 0) {
             pkg->timestamp = get_timestamp();
             #if DEBUG_BCR_DISPATCH
             LOG_BCR("=>now(%d):0x%lx(%s/time=%ld) -> %d", index, (is_v2 ? send->id.code_v2 : send->id.code), obj->name, pkg->timestamp, i);
             #endif
             MSG_QUEUE_SEND(thd[i].msg_id, (void *)&msg, 0);
             if (send->sync) {
                 WAIT_TASK_EXEC_FINISH((is_v2 ? send->id.code_v2 : send->id.code), &thd[i], obj->name, 150);
             }
             break;
         }
     }
 
     if (i >= BCR_WORK_THREAD_NUM) {
         pkg->timestamp = get_timestamp();
         LOGW("=>pending(%d):0x%lx(%s/time=%ld) -> %d", index, (is_v2 ? send->id.code_v2 : send->id.code), obj->name, pkg->timestamp, 0);
         MSG_QUEUE_SEND(thd[0].msg_id, (void *)&msg, 0);
         if (send->sync) {
             WAIT_TASK_EXEC_FINISH((is_v2 ? send->id.code_v2 : send->id.code), &thd[0], obj->name, 150);
         }
     }
 }
 
 static int bcr_process_pkg_cb(multi_func_msg_data_t *msg, thread_context_t *thd)
 {
     bcr_pkg_struct_t *pkg = &msg->union_body.bcr_pkg;
 
     if (pkg->obj.margic == BCR_OBJ_MARGIC_CODE) {
         if (pkg->obj.receiver_cb) {
             uint32_t exec_time = get_timestamp();
             thd->task_name = (char *)pkg->obj.name;
 
             uint16_t crc = CRC16(&pkg->priv_data, sizeof(broadcast_priv_data_t));
             if (crc != pkg->buf_crc) {
                 LOGE("=====>CRC cmp fail: %s, timestamp: %ld", pkg->obj.name, pkg->timestamp);
             }
 
             DUMP_OBJ_BEGAN("EXEC_BCR_TASK", pkg, (int)(thd->tid) % 1000, THREAD_NAME_GET(thd));
             pkg->obj.receiver_cb(pkg->id, pkg->obj.context, pkg->arg1, pkg->arg2, &pkg->priv_data);
 
             uint32_t diff_time = (get_timestamp() - exec_time);
             if (diff_time >= 10) {
                 DUMP_OBJ_END("EXEC_BCR_TASK-endtime", pkg, (int)(thd->tid) % 1000, THREAD_NAME_GET(thd), diff_time);
             }
 
             thd->task_name = "idle";
             return 0;
         }
     }
 
     LOGE("LOG_BCR invalid margic, ignore!");
 
     return -1;
 }
 
 static void bcr_work_thd(const void *args)
 {
     thread_context_t *thd = (thread_context_t *)args;
     multi_func_msg_data_t msg = {0};
 
     CREATE_MSG_TYPE_QUEUE(&thd->msg_id, BCR_THD_MESQ_NUM /2, sizeof(multi_func_msg_data_t));
     thd->task_name = "idle";
 
     for (;;) {
         set_thread_running(thd, false);
         MSG_QUEUE_RECV(thd->msg_id, (void *)&msg, OS_WAIT_FOREVER);
         set_thread_running(thd, true);
         bcr_process_pkg_cb(&msg, thd);
     } 
 }
 
 /*
  * GLOBAL FUNCTION DEFINITIONS
  *****************************************************************************************
  */
 
 /*
 * @brief 将UI线程消息句柄添加到投递线程任务中.
 */
 void broadcast_register_ui_thd_cb(os_message_id ui_msg_id, task_post_process_fptr **cb)
 {
     bcr_context_t *ctx = thiz;
 
     ctx->ui_msg_id = ui_msg_id;
     *cb = (task_post_process_fptr *) bcr_process_pkg_cb;
 }
 
 /*
 * @brief 广播上下文初始化. 
 */
 void init_broadcast_context(void)
 {
     bcr_context_t *ctx = (bcr_context_t *)u_malloz(sizeof(bcr_context_t));
     thiz = ctx;
     
     USE_INIT_LIST_HEAD(&ctx->list);
     MUTEX_INIT(&ctx->m_lock, m_broadcast_mutex);
 
     for (int i = 0; i < BCR_WORK_THREAD_NUM; i++) {
         ctx->bcr_work_thd[i].user_data = ctx;
         MUTEX_INIT(&ctx->bcr_work_thd[i].lock_id, "bcr_lock");
         RUN_NEW_THREAD(&ctx->bcr_work_thd[i], s_bcr_thd_name[i], 
             bcr_work_thd, RT_PRIO_BRC_DISPATCH_THREAD, BCR_THREAD_STACKSIZE);
     }
 }
 
 /*
 * @brief 广播上下文是否初始化成功.
 */
 bool is_broadcast_inited(void)
 {
     return thiz != NULL;
 }
 
 /*
 * @brief 注册广播实例.
 */
 void register_broadcast_receiver(broadcast_obj_t *obj)
 {
     bcr_context_t *ctx = thiz;
     list_head_t *head = &ctx->list;
 
     LOG_BCR("(name=%s)", obj->name);
     MUTEX_LOCK(ctx->m_lock);
     
     broadcast_node_t *node = u_malloz(sizeof(broadcast_node_t));
     memcpy(&node->obj, obj, sizeof(broadcast_obj_t));
 
     node->obj.margic = BCR_OBJ_MARGIC_CODE;
     if (node->obj.name == NULL) {
         node->obj.name = "null";
     }
 
     LIST_FOR_EACH(pos, (head)) { 
         broadcast_node_t *node = CONTAINER_OF(pos, broadcast_node_t, list);
         if (memcmp(&node->obj, obj, sizeof(broadcast_obj_t)) == 0) {
             LOGW("dumplicate add(%s), ignore", obj->name);
             MUTEX_UNLOCK(ctx->m_lock);
             return;
         }
     }
 
     __list_add_tail(&node->list, head);
     MUTEX_UNLOCK(ctx->m_lock);
     DUMP_NODE("add", node);
 }
 
 /*
 * @brief 注销广播实例. 
 */
 void unregister_broadcast_receiver(broadcast_obj_t *obj)
 {
     bcr_context_t *ctx = thiz;
     list_head_t *head = &ctx->list;
 
     LOG_BCR("(name=%s)", obj->name);
     MUTEX_LOCK(ctx->m_lock);
     __list_del(head, obj, list_del_condition_callback);
     MUTEX_UNLOCK(ctx->m_lock);
 }
 
 /*
 * @brief 广播分发
 */
 void send_broadcast(broadcast_send_wrapper_t *send)
 {
     bcr_context_t *ctx = thiz;
     list_head_t *head = &ctx->list;
     int n = 0;
 
     MUTEX_LOCK(ctx->m_lock);
 
     LIST_FOR_EACH(pos, (head)) { 
         broadcast_node_t *node = CONTAINER_OF(pos, broadcast_node_t, list);
 
         if (node->obj.id.code & send->id.code) {
             post_bcr_task(ctx, send, &node->obj, 0, n);
         }
 
         if (node->obj.id.code_v2 & send->id.code_v2) {
            post_bcr_task(ctx, send, &node->obj, 1, n);
         }
 
         n++;
     }
 
     MUTEX_UNLOCK(ctx->m_lock);
 }
 
 /*
 * @brief 广播调度队列是否繁忙.
 */
 int is_broadcast_queue_busy(void)
 {
     bcr_context_t *ctx = thiz;
     thread_context_t *thd = ctx->bcr_work_thd;
     char *task_name = NULL;
 
     for (int i = 0; i < BCR_WORK_THREAD_NUM; i++) {
         if (get_thread_running(&thd[i]) > 0) {
             /// __SYSTEM_REBOOT_TASK 是重启的任务，需要排除掉.
             task_name = thd[i].task_name;
             if (task_name != NULL && strstr(task_name, __SYSTEM_REBOOT_TASK) != NULL) {
                 continue;
             }
             LOGW("@thread: %d(%s) busy!", i, task_name);
             return -2;
         }
     }
 
     return 0;
 }
 
 