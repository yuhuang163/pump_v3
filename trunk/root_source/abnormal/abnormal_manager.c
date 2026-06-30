/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "abnormal_manager.h" // IWYU pragma: keep

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static abnormal_manager_context_t *thiz = NULL;

static const uint8_t s_abnormal_severity[SYS_ABNORMAL_TYPE_MAX] = {
    [SYS_ABNORMAL_BATTERY_TEMP_HIGH] = SYS_ABNORMAL_SEVERITY_ERROR,
    [SYS_ABNORMAL_STORAGE_FAILURE] = SYS_ABNORMAL_SEVERITY_FATAL,
    [SYS_ABNORMAL_COMMUNICATION_ERROR] = SYS_ABNORMAL_SEVERITY_ERROR,
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int abnormal_type_to_index(uint8_t type)
{
    if (type >= SYS_ABNORMAL_TYPE_MAX) {
        return -1;
    }
    return (int)type;
}

static void dispatch_abnormal_state_client(abnormal_manager_context_t *obj, sys_abnormal_state_info_t *info)
{
    list_head_t *head = &obj->priv.client_list;

    MUTEX_LOCK(obj->priv.m_lock);
    LIST_FOR_EACH(pos, head) {
        abnormal_evt_client_node_t *node = CONTAINER_OF(pos, abnormal_evt_client_node_t, list);
        if (node->cb) {
            node->cb(node->user_data, info);
        }
    }
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void notify_abnormal_state_change(sys_abnormal_state_info_t *info)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(info);
    dispatch_abnormal_state_client(thiz, info);
}

static uint8_t get_abnormal_state(uint8_t type)
{
    int index = abnormal_type_to_index(type);

    EXCEPTION_CHECK_UINT(thiz);
    if (index < 0) {
        return SYS_ABNORMAL_STATE_CLEARED;
    }
    return thiz->priv.state[index];
}

static uint8_t is_abnormal_active(uint8_t type)
{
    return get_abnormal_state(type) == SYS_ABNORMAL_STATE_ACTIVE ? 1u : 0u;
}

static uint8_t get_abnormal_severity(uint8_t type)
{
    int index = abnormal_type_to_index(type);

    if (index < 0) {
        return SYS_ABNORMAL_SEVERITY_ERROR;
    }
    return s_abnormal_severity[index];
}

static uint8_t is_new_task_forbidden(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    for (uint8_t i = 0; i < SYS_ABNORMAL_TYPE_MAX; i++) {
        if (thiz->priv.state[i] == SYS_ABNORMAL_STATE_ACTIVE) {
            return 1u;
        }
    }
    return 0u;
}

static uint8_t is_core_locked(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    if (thiz->priv.state[SYS_ABNORMAL_STORAGE_FAILURE] == SYS_ABNORMAL_STATE_ACTIVE) {
        return 1u;
    }
    return 0u;
}

static uint8_t is_app_communication_forbidden(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    for (uint8_t i = 0; i < SYS_ABNORMAL_TYPE_MAX; i++) {
        if (thiz->priv.state[i] == SYS_ABNORMAL_STATE_ACTIVE) {
            return 1u;
        }
    }
    return 0u;
}

static uint8_t set_abnormal_state(uint8_t type, uint8_t state)
{
    int index = abnormal_type_to_index(type);
    sys_abnormal_state_info_t info = {0};

    EXCEPTION_CHECK_UINT(thiz);
    if (index < 0) {
        LOGW("invalid abnormal type: %u", type);
        return 0;
    }
    if (state != SYS_ABNORMAL_STATE_CLEARED && state != SYS_ABNORMAL_STATE_ACTIVE) {
        LOGW("invalid abnormal state: %u", state);
        return 0;
    }
    if (thiz->priv.state[index] == state) {
        return 0;
    }

    thiz->priv.state[index] = state;
    info.type = type;
    info.state = state;
    LOG_MISC("abnormal change type=%u sev=%u state=%u",
        type, s_abnormal_severity[index], state);
    dispatch_abnormal_state_client(thiz, &info);
    return 1;
}

static void add_abnormal_state_cb(void *user_data, on_abnormal_state_cb_ptr cb)
{
    abnormal_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.client_list;

    EXCEPTION_CHECK_VOID(obj);
    MUTEX_LOCK(obj->priv.m_lock);
    LIST_FOR_EACH(pos, head) {
        abnormal_evt_client_node_t *node = CONTAINER_OF(pos, abnormal_evt_client_node_t, list);
        if (node->cb == cb && node->user_data == user_data) {
            LOGW("ignore dumplicate add.");
            MUTEX_UNLOCK(obj->priv.m_lock);
            return;
        }
    }
    abnormal_evt_client_node_t *node = u_malloz(sizeof(abnormal_evt_client_node_t));
    node->cb = cb;
    node->user_data = user_data;
    __list_add(&node->list, head);
    LOG_MISC("abnormal add cb user_data:%lx cb:%lx", user_data, cb);
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void *list_del_abnormal_state_condition_cb(void *data, list_head_t *pos)
{
    on_abnormal_state_cb_ptr cb = (on_abnormal_state_cb_ptr)data;
    abnormal_evt_client_node_t *node = CONTAINER_OF(pos, abnormal_evt_client_node_t, list);

    if (node->cb == cb) {
        LOG_MISC("abnormal del cb user_data:%lx cb:%lx", node->user_data, node->cb);
        return node;
    }
    return NULL;
}

static void remove_abnormal_state_cb(on_abnormal_state_cb_ptr cb)
{
    abnormal_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.client_list;

    EXCEPTION_CHECK_VOID(obj);
    MUTEX_LOCK(obj->priv.m_lock);
    __list_del(head, cb, list_del_abnormal_state_condition_cb);
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static int on_post_abnormal_manager_init(void *ptr)
{
    abnormal_manager_context_t *abnormal = (abnormal_manager_context_t *)ptr;

    EXCEPTION_CHECK_INT(abnormal);
    if (abnormal->hook && abnormal->hook->on_post_init_route) {
        abnormal->hook->on_post_init_route(abnormal);
    }
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_abnormal_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, abnormal_manager_context_t);

    MUTEX_INIT(&ptr->priv.m_lock, abnormal_state_mutex);
    USE_INIT_LIST_HEAD(&ptr->priv.client_list);
    ptr->set_abnormal_state = set_abnormal_state;
    ptr->get_abnormal_state = get_abnormal_state;
    ptr->is_abnormal_active = is_abnormal_active;
    ptr->get_abnormal_severity = get_abnormal_severity;
    ptr->is_new_task_forbidden = is_new_task_forbidden;
    ptr->is_app_communication_forbidden = is_app_communication_forbidden;
    ptr->is_core_locked = is_core_locked;
    ptr->add_abnormal_state_cb = add_abnormal_state_cb;
    ptr->remove_abnormal_state_cb = remove_abnormal_state_cb;
    ptr->notify_abnormal_state_change = notify_abnormal_state_change;
    thiz = ptr;
    return ADD_MODULE("abnormal", ID_MODULE_SYS_ABNORMAL, NULL, on_post_abnormal_manager_init,
        ptr, list_head, sizeof(abnormal_manager_context_t));
}
