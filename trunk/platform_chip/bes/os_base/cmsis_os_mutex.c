/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "cmsis_os_api.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep
#include "heap_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
typedef struct mutex_module_t {
    os_mutex_id id;
    os_mutex_def_t *mutex_def;
    list_head_t list;
} mutex_module_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
osMutexDef(mutex_module_mutex);
static os_mutex_id mutex_module_mutex_id = NULL;
static list_head_t list_mutex_module = {0};

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
    os_mutex_id id = (os_mutex_id)data;
    mutex_module_t *node = CONTAINER_OF(pos, mutex_module_t, list);
    if (id == node->id) {
        u_free(node->mutex_def);
        return node;
    }
    return NULL;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
os_status_e os_mutex_wait_wrapper(os_mutex_id mutex_id, uint32_t millisec)
{
    return osMutexAcquire(mutex_id, millisec);
}

os_status_e os_mutex_release_wrapper(os_mutex_id mutex_id)
{
    return osMutexRelease(mutex_id);
}

os_status_e os_mutex_delete_wrapper(os_mutex_id mutex_id)
{
    os_mutex_id id = mutex_id;
    os_status_e status = osMutexDelete(mutex_id);
    if (status != osOK) {
        LOGE("Oops! osMutexDelete: fail %p", mutex_id);
        return status;
    }
    os_mutex_wait_wrapper(mutex_module_mutex_id, OS_WAIT_FOREVER);
    __list_del(&list_mutex_module, id, list_del_condition_callback);
    os_mutex_release_wrapper(mutex_module_mutex_id);
    return status;
}

int os_mutex_create_wrapper(os_mutex_id *mutex_id, const char *name)
{
    os_mutex_def_t *def = (os_mutex_def_t *)u_malloz(sizeof(os_mutex_def_t));
    def->name = (char *)name;
    def->attr_bits = osMutexRecursive | osMutexPrioInherit | osMutexRobust;
    def->cb_mem = NULL;
    def->cb_size = 0;
    *mutex_id = osMutexCreate(def);
    if (*mutex_id == NULL) {
        u_free(def);
        LOGE("Oops! osMutexCreate: fail %s", name);
        return -1;
    }

    /* add to list */
    os_mutex_wait_wrapper(mutex_module_mutex_id, OS_WAIT_FOREVER);
    mutex_module_t *module = (mutex_module_t *)u_malloz(sizeof(mutex_module_t));
    module->id = *mutex_id;
    module->mutex_def = def;
    __list_add(&(module->list), &list_mutex_module);
    os_mutex_release_wrapper(mutex_module_mutex_id);

    return 0;
}

int os_mutex_module_init(void)
{
    mutex_module_mutex_id = osMutexCreate((osMutex(mutex_module_mutex)));
    if (mutex_module_mutex_id == NULL) {
        LOGE("mutex create failed: %s", "mutex_module");
        return -1;
    }
    return 0;
}