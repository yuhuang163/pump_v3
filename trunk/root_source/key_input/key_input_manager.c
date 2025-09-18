/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "key_input_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define KEY_LOCK_LOG                     LOG_KEY
#if 0
#define MUTEX_LOCK_KEY(__lock)           do { KEY_LOCK_LOG("=>LOCK_KEY"); MUTEX_LOCK(__lock); } while (0)
#define MUTEX_UNLOCK_KEY(__lock)         do {  MUTEX_UNLOCK(__lock); KEY_LOCK_LOG("=>UNLOCK_KEY"); } while (0)
#else
#define MUTEX_LOCK_KEY(__lock)           do { MUTEX_LOCK(__lock); } while (0)
#define MUTEX_UNLOCK_KEY(__lock)         do { MUTEX_UNLOCK(__lock); } while (0)
#endif

typedef struct {
    const char *caller;
    uint16_t keycode;
    uint16_t keytype;
    on_key_event_handler_fptr cb;
} key_handler_t;

typedef struct {
    void *user_data;
    key_handler_t handler;
    list_head_t list;
} key_evt_client_node_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static key_input_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int dispatch_key_event_wrapper(void *context, int keycode, int keytype);

static int dispatch_key_event_client(uint16_t keycode, uint16_t keytype)
{
    list_head_t *head = &thiz->priv.key_evt_list;
    int ret = -1;
    
    MUTEX_LOCK_KEY(thiz->priv.key_evt_lock);
    LIST_FOR_EACH(pos, (head)) { 
        key_evt_client_node_t *node = CONTAINER_OF(pos, key_evt_client_node_t, list);
        if ((node->handler.keycode & keycode) && (node->handler.keytype & keytype)) {
            node->handler.cb(node->user_data, keycode, keytype);
            ret = 0;
        }
    }
    MUTEX_UNLOCK_KEY(thiz->priv.key_evt_lock); 
    return ret;
}

static const char *to_event_name(uint32_t keytype)
{
    switch (keytype) {
        case KEY_EVENT_DOWN:
            return ":down";
        case KEY_EVENT_UP:
            return ":up";
        case KEY_EVENT_CLICK:
            return ":click";
        case KEY_EVENT_DOUBLE:
            return ":double";
        case KEY_EVENT_LONG_2S:
            return ":2s";
        case KEY_EVENT_LONG_3S:
            return ":3s";
        case KEY_EVENT_LONG_10S:
            return ":10s";
        default:
            LOGE("keytype: %ld", keytype);
            return "unknow";
    }
}

#if IS_SUPPORT_KEY_SOFT_RESET
/// 长按15s强制reset系统.
static int on_spec_press_key_handler(void *user_data, uint16_t keycode, uint16_t keytype)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    static uint32_t down_time = 0;
    uint32_t diff_time = 0;

    LOG_KEY("keycode: %x, keytype: %x", keycode, keytype);

    if (keytype == KEY_EVENT_DOWN) {
        down_time = get_timestamp();
        return 0;  
    } else if (keytype == CUST_KEY_EVENT_FORCE_RESET) {
        diff_time = get_timestamp() - down_time;
        if (diff_time < 7000) {
            LOGE("ignore diff_time: %ld", diff_time);
            return 0;
        }  
    } else {
        LOGE("invalid event, ignore: %d, %d", keycode, keytype);
        return -1;
    }

    pm->acquire_wakelock("reboot", 10);

    LOGE("(%ld)Oops! we will reset system!", diff_time);
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);

    LOGE("==========RESET_AND_REBOOT==========");
    st->factory_reset(RESET_AND_REBOOT);

    return 0;
}
#endif

static void internal_keyevent_callback(void *user_data, uint16_t keycode, uint16_t keytype)
{
    key_input_context_t *input = thiz;

#if IS_SUPPORT_KEY_SOFT_RESET      
    if (keycode == KEY_CODE_POWER) {
        if (keytype == KEY_EVENT_DOWN || keytype == CUST_KEY_EVENT_FORCE_RESET) {
            on_spec_press_key_handler(NULL, keycode, keytype);
        }
    }
#endif 

    /// 提供外部钩子接管的能力.
    struct __key_input_hook *hook = input->hook;
    if (hook && hook->input_keyevent_route) {
        if (hook->input_keyevent_route(input, keycode, keytype, 0)) {
            return;
        }
    }
    dispatch_key_event_wrapper(thiz, keycode, keytype);
}

static int internal_read_btn_id(void *context)
{
    uint16_t btn_id = 0;

    if (btn_id != thiz->priv.key_event.keycode) {
        btn_id = thiz->priv.key_event.keycode;
    }
    
    if (thiz->priv.key_event.keycode > 0) {
        thiz->priv.key_event.keycode = 0;
    }

    return btn_id;
}

static void *list_del_condition_cb(void *data, list_head_t *pos)
{
    on_key_event_handler_fptr cb = (on_key_event_handler_fptr)data;

    key_evt_client_node_t *node = CONTAINER_OF(pos, key_evt_client_node_t, list);
    if (cb == node->handler.cb) {
        LOG_KEY("remove: cb: %s, keyevent:(%d/%s)", node->handler.caller, node->handler.keycode,
             to_event_name(node->handler.keytype));
        return node;
    }

    return NULL;
}

static int on_post_key_input_init(void *ptr)
{
    key_input_context_t *ctx = ptr;


    #if (!IS_INPUT_KEY_TASK_POST_UI_THREAD)
    RUN_NEW_THREAD(&ctx->priv.key_evt_ctx.keyevent_thd, "keyevent_thd", 
        key_input_internal_dispatch_thread, RT_PRIO_KEY_INPUT_DISPATCH_THREAD, INPUT_KEY_THREAD_STACKSIZE);
    #endif
    return 0;
}

static int on_perpare_key_input_init(void *self)
{
    key_input_context_t *ctx = self;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);

    ctx->priv.key_dev = pbus->find_device(ID_DEVICE_INPUT_KEY);
    if (ctx->priv.key_dev) {
        DEVICE_IOCTL_ARGS(ctx->priv.key_dev, OP_CODE_INPUT_ADD_CALLBACK, internal_keyevent_callback);
    }
    MUTEX_INIT(&ctx->priv.key_evt_lock, key_evt_mutex);
    USE_INIT_LIST_HEAD(&ctx->priv.key_evt_list);

    return 0;
}

#if (!IS_INPUT_KEY_TASK_POST_UI_THREAD)
static void key_input_internal_dispatch_thread(const void *args)
{
    thread_context_t *thd = args;
    multi_func_msg_data_t msg = {0};
    uint16_t keycode = 0;
    uint16_t keytype = 0;
    
    key_input_context_t *input = CONTAINER_OF(thd, key_input_context_t, priv.key_evt_ctx.keyevent_thd);
    window_manager_context_t *wm = find_module(ID_MODULE_WM);

    CREATE_MSG_QUEUE(&thd->msg_id, INPUT_DISPATCH_KEY_EVT_MESQ_NUM);

    do {
        osMessageGetEx(thd->msg_id, &msg, osWaitForever);
        keycode = msg.union_body.parms_16[0];
        keytype = msg.union_body.parms_16[1];

        if (wm->is_on_window_dispatch()) {
            LOGE("on_window_dispatch, ignore: key_event(%d, %d)", keycode, keytype);
            continue;
        }

        //LOG_KEY("recever: key_event(%d, %d)", keycode, keytype);

        #if 1
        if (dispatch_client(keycode, keytype) == 0) {
            thiz->key_event.keycode = keycode;
            thiz->key_event.keytype = keytype;
        }
        #endif
    } 
    while (1);
}
#endif 

/// 注册按键响应:带调试tag
static void add_keyevent_handler_extra(const char *caller, void *user_data, uint16_t keycode, uint16_t keytype, on_key_event_handler_fptr cb)
{
    EXCEPTION_CHECK_VOID(thiz);
    list_head_t *head = &thiz->priv.key_evt_list; 
    
    MUTEX_LOCK_KEY(thiz->priv.key_evt_lock);
    LIST_FOR_EACH(pos, (head)) { 
        key_evt_client_node_t *node = CONTAINER_OF(pos, key_evt_client_node_t, list);
        if (node->user_data == user_data 
            && node->handler.cb == cb
            && node->handler.keycode == keycode
            && node->handler.keytype == keytype) {
            LOGE("ignore dumplicate add.");
            MUTEX_UNLOCK_KEY(thiz->priv.key_evt_lock);
            return;
        }
    }

    key_evt_client_node_t *node = u_malloz(sizeof(key_evt_client_node_t));
    node->handler.cb = cb;
    node->handler.keycode = keycode;
    node->handler.keytype = keytype;
    node->user_data = user_data;
    node->handler.caller = caller;
    __list_add(&node->list, head);
    LOG_KEY("add: caller: %s, keyevent: (%d/%s)", caller, keycode, to_event_name(keytype));
    MUTEX_UNLOCK_KEY(thiz->priv.key_evt_lock);
}

static void add_keyevent_handler(void *user_data, uint16_t keycode, uint16_t keytype, on_key_event_handler_fptr cb)
{
    add_keyevent_handler_extra("default", user_data, keycode, keytype, cb);
}

/// 注消按键响应.
static void remove_keyevent_handler(on_key_event_handler_fptr cb)
{
    EXCEPTION_CHECK_VOID(thiz);
    list_head_t *head = &thiz->priv.key_evt_list;
    
    MUTEX_LOCK_KEY(thiz->priv.key_evt_lock);
    __list_del(head, cb, list_del_condition_cb);
    MUTEX_UNLOCK_KEY(thiz->priv.key_evt_lock);
}

#if IS_INPUT_KEY_TASK_POST_UI_THREAD
static int dispatch_key_event_wrapper(void *context, int keycode, int keytype)
{
    key_input_context_t *ctx = thiz;

    ctx->priv.key_event.keycode = keycode;
    ctx->priv.key_event.keytype = keytype;

    multi_func_msg_data_t __msg = {
        .msg_type = MSG_TYPE_INPUT_KEY_EVENT,
        .union_body.key_event_data.keycode = keycode,
        .union_body.key_event_data.keytype = keytype,
    };
    LOG_KEY("event: %d/%d\n", keycode, keytype);
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    if (wm->post_msg_to_window) {
        wm->post_msg_to_window(__func__, (&__msg));
    }

    return 0;
}
#else
static int dispatch_key_event_wrapper(void *context, int keycode, int keytype)
{
    key_input_context_t *key_input = thiz;
    thread_context_t *thd = &(key_input->priv.key_evt_ctx.keyevent_thd);

    if (thd->msg_id) {
        multi_func_msg_data_t msg = {0};
        msg.union_body.parms_16[0] = keycode;
        msg.union_body.parms_16[1] = keytype;
        return MSG_QUEUE_SEND(thd->msg_id, &msg, 0);
    }

    return -1;
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_key_input_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, key_input_context_t);

    ptr->read_btn_id = internal_read_btn_id;
    ptr->add_keyevent_handler = add_keyevent_handler;
    ptr->dispatch_client_input_key = dispatch_key_event_client;
    ptr->dispatch_key_event_wrapper = dispatch_key_event_wrapper;
    ptr->remove_keyevent_handler = remove_keyevent_handler;

    thiz = ptr;

    return ADD_MODULE("key_input", ID_MODULE_KEY_INPUT, on_perpare_key_input_init, on_post_key_input_init, 
        ptr, list_head, sizeof(key_input_context_t));
}