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
static int is_key_event_shielded(uint8_t shield_level)
{
    return shield_level < thiz->priv.key_evt_shield_level;
}

static int dispatch_key_event_client(uint16_t keycode, uint16_t keytype)
{
    list_head_t *head = &thiz->priv.key_evt_list;
    int ret = -1;
    MUTEX_LOCK_KEY(thiz->priv.key_evt_lock);
    LIST_FOR_EACH(pos, (head)) { 
        key_evt_client_node_t *node = CONTAINER_OF(pos, key_evt_client_node_t, list);
        if (is_key_event_shielded(node->handler.shield_level)) {
            continue;
        }

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
        case KEY_EVENT_LONG_4S:
            return ":4s";
        case KEY_EVENT_LONG_5S:
            return ":5s";
        case KEY_EVENT_LONG_6S:
            return ":6s";
        case KEY_EVENT_LONG_7S:
            return ":7s";
        case KEY_EVENT_LONG_8S:
            return ":8s";
        case KEY_EVENT_LONG_9S:
            return ":9s";
        case KEY_EVENT_LONG_10S:
            return ":10s";
        case KEY_EVENT_LONG_AFTER_DOWN:
            return ":long_press_after_down";
        case KEY_EVENT_LONG_AFTER_CLICK:
            return ":long_press_after_click";
        default:
            LOGW("keytype: %ld", keytype);
            return "unknow";
    }
}

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

    if (ctx->hook && ctx->hook->get_key_code_name) {
        const char *name = ctx->hook->get_key_code_name(keycode);
        LOG_KEY("event: %d/%s/%s\n", keycode, name, to_event_name(keytype));
    } else {
        LOG_KEY("event: %d/%s\n", keycode, to_event_name(keytype));
    }
    
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    if (wm && wm->post_msg_to_window) {
        wm->post_msg_to_window(__func__, (&__msg));
    }

    return 0;
}

static void internal_keyevent_callback(void *user_data, uint16_t keycode, uint16_t keytype)
{
    key_input_context_t *input = thiz;
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

static int internal_is_key_triggered(uint16_t key_index)
{
    EXCEPTION_CHECK_BOOL(thiz->priv.key_dev);
    int args[] = {key_index};
    return thiz->priv.key_dev->ioctl(thiz->priv.key_dev, OP_CODE_INPUT_IS_TRIGGERED, (void *)args) > 0 ? 1 : 0;
}

static void *list_del_condition_cb(void *data, list_head_t *pos)
{
    on_key_event_handler_fptr cb = (on_key_event_handler_fptr)data;

    key_evt_client_node_t *node = CONTAINER_OF(pos, key_evt_client_node_t, list);
    if (cb == node->handler.cb) {
        LOG_KEY("remove: cb: %s, keyevent:(%d/%s)", node->handler.tag, node->handler.keycode,
             to_event_name(node->handler.keytype));
        return node;
    }

    return NULL;
}

static int on_post_key_input_init(void *ptr)
{
    key_input_context_t *ctx = ptr;
    ctx->priv.key_evt_shield_level = KEY_HANDLER_SHIELD_NONE;
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

static void add_keyevent_handler(const char *tag, void *user_data, uint16_t keycode, uint16_t keytype,
    on_key_event_handler_fptr cb, uint8_t shield_level)
{
    EXCEPTION_CHECK_VOID(thiz);
    list_head_t *head = &thiz->priv.key_evt_list; 
    MUTEX_LOCK_KEY(thiz->priv.key_evt_lock);
    LIST_FOR_EACH(pos, (head)) { 
        key_evt_client_node_t *node = CONTAINER_OF(pos, key_evt_client_node_t, list);
        if (node->user_data == user_data && node->handler.cb == cb && 
            node->handler.keycode == keycode && node->handler.keytype == keytype
            && node->handler.shield_level == shield_level) {
            LOGW("ignore dumplicate add.");
            MUTEX_UNLOCK_KEY(thiz->priv.key_evt_lock);
            return;
        }
    }
    key_evt_client_node_t *node = u_malloz(sizeof(key_evt_client_node_t));
    node->handler.cb = cb;
    node->handler.keycode = keycode;
    node->handler.keytype = keytype;
    node->handler.shield_level = shield_level;
    node->user_data = user_data;
    node->handler.tag = tag;
    __list_add(&node->list, head);
    LOG_KEY("add: tag: %s, keyevent: (%d/%s), shield:%d", tag, keycode, to_event_name(keytype), shield_level);
    MUTEX_UNLOCK_KEY(thiz->priv.key_evt_lock);
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

static void set_keyevent_shield_level(uint8_t level)
{
    EXCEPTION_CHECK_VOID(thiz);
    MUTEX_LOCK_KEY(thiz->priv.key_evt_lock);
    if (level >= KEY_HANDLER_SHIELD_FORBIDDEN) {
        level = KEY_HANDLER_SHIELD_FORBIDDEN;
    }
    thiz->priv.key_evt_shield_level = level;
    LOG_KEY("==>shield level: %d<==", level);
    MUTEX_UNLOCK_KEY(thiz->priv.key_evt_lock);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_key_input_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, key_input_context_t);

    ptr->read_btn_id = internal_read_btn_id;
    ptr->is_key_triggered = internal_is_key_triggered;
    ptr->add_keyevent_handler = add_keyevent_handler;
    ptr->dispatch_client_input_key = dispatch_key_event_client;
    ptr->dispatch_key_event_wrapper = dispatch_key_event_wrapper;
    ptr->remove_keyevent_handler = remove_keyevent_handler;
    ptr->set_keyevent_shield_level = set_keyevent_shield_level;
    thiz = ptr;

    return ADD_MODULE("key_input", ID_MODULE_KEY_INPUT, on_perpare_key_input_init, on_post_key_input_init, 
        ptr, list_head, sizeof(key_input_context_t));
}