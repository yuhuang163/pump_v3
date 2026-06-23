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
static list_head_t s_module_obj_list = {0}; /// 模块链表.

static const module_register_fptr s_module_register[] = {

    #if IS_SUPPORT_MODULE_PBUS
    register_module_for_peripheral_bus_new,
    #endif
    #if IS_SUPPORT_MODULE_BLE
    register_module_for_ble_manager_new,
    #endif
    #if IS_SUPPORT_MODULE_PM
    register_module_for_pm_new,
    #endif
    #if IS_SUPPORT_MODULE_STORAGE
    register_module_for_storage_new,
    #endif
    #if IS_SUPPORT_MODULE_RES
    register_module_for_res_manager_new,
    #endif
    #if IS_SUPPORT_MODULE_TIME_SYSTEM
    register_module_for_time_system_new,
    #endif
    #if IS_SUPPORT_MODULE_UI
    register_module_for_ui_manager_new,
    #endif
    #if IS_SUPPORT_MODULE_WM
    register_module_for_window_manager_new,
    #endif
    #if IS_SUPPORT_MODULE_KEY_INPUT
    register_module_for_key_input_manager_new,
    #endif
    #if IS_SUPPORT_MODULE_SENSOR
    register_module_for_sensor_manager_new,
    #endif
    #if IS_SUPPORT_MODULE_OTA
    register_module_for_ota_manager_new,
    #endif
    #if IS_SUPPORT_MODULE_MISC
    register_module_for_misc_manager_new,
    #endif
    #if IS_SUPPORT_MODULE_BATTERY
    register_module_for_battery_manager_new,
    #endif
    #if IS_SUPPORT_MODULE_BREAST_PUMP
    register_module_for_breast_pump_new,
    #endif
    #if IS_SUPPORT_MODULE_TESTS
    register_module_for_tests_manager_new,
    #endif
    #if IS_SUPPORT_MODULE_DATA_BURIAL
    register_module_for_data_burial_new,
    #endif
    #if IS_SUPPORT_MODULE_US_ALGO
    register_module_for_algo_fusion_new,
    #endif
    #if IS_SUPPORT_MODULE_MOTOR
    register_module_for_motor_manager_new,
    #endif
    #if IS_SUPPORT_MODULE_AUDIO
    register_module_for_audio_manager_new,
    #endif
    #if IS_SUPPORT_MODULE_AIRPLANE_MODE
    register_module_for_airplane_mode_new,
    #endif
    #if IS_SUPPORT_MODULE_PROTOCOL
    register_module_for_protocol_manager_new,
    #endif
    #if IS_SUPPORT_MODULE_SERVERS
    register_module_for_servers_new,
    #endif
    #if IS_SUPPORT_MODULE_SYS_ABNORMAL
    register_module_for_abnormal_manager_new,
    #endif
    #if IS_SUPPORT_MODULE_LED
    register_module_for_led_manager_new,
    #endif
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *add_new_module(const char *name, module_desc_id_e id, int (*perpare_init_cb)(void *ptr), int (*post_init_cb)(void *ptr), 
                    void *ptr, list_head_t *list_head, uint32_t mem_usage)
{
    MALLOC_FOR_PTR(node, module_node_t);

    node->name = name;
    node->id = id;
    node->post_init = post_init_cb;
    node->perpare_init = perpare_init_cb;
    node->module_ptr = ptr;
    node->mem_usage = (mem_usage + sizeof(module_node_t));
    __list_add_tail(&node->list, list_head);

    return node;
}

const void *find_module(module_desc_id_e id)
{
    static void *__cache[ID_MODULE_MAX] = {0};
    list_head_t *list_head = &s_module_obj_list;

    if (!(id > ID_MODULE_BEGIN && id < ID_MODULE_MAX)) {
        LOGE("invalid module id!");
        return NULL;
    }

    if (__cache[id] != NULL) {
        find_ok:
        return (const void *)__cache[id];
    }

    LIST_FOR_EACH(pos, list_head) {
        module_node_t *node = CONTAINER_OF(pos, module_node_t, list);
        if (node->id == id) {
            __cache[id] = node->module_ptr;
            goto find_ok;
        }
    }

    return NULL;
}

int register_module(void)
{
    list_head_t *list_head = &s_module_obj_list;
    uint32_t total_mem_usage = 0;

    for (int i = 0; i < LENGTH_OF_ARRAY(s_module_register); i++) {
        module_register_fptr f = s_module_register[i];
        module_node_t *node = f(list_head);
        if (node) {
            total_mem_usage += node->mem_usage;
            LOGI("%d: --- [%s/%d]has registered, mem_usage:%lu/%lu (Byte)", 
                i, node->name, node->id, node->mem_usage, total_mem_usage);
        }
    }

    return 0;
}


int init_module(void)
{
    list_head_t *list_head = &s_module_obj_list;

    /// 初始化准备
    LIST_FOR_EACH(pos, list_head) {
        module_node_t *node = CONTAINER_OF(pos, module_node_t, list);
        if (node->perpare_init) {
            node->perpare_init(node->module_ptr);
        }
    }

    /// 初始化开始.
    LIST_FOR_EACH(pos, list_head) {
        module_node_t *node = CONTAINER_OF(pos, module_node_t, list);
        if (node->post_init) {
            // uint32_t t0 = get_timestamp();
            node->post_init(node->module_ptr);
            // LOGI("node:[%s/%d],escaped: %lu ms",node->name, node->id, get_timestamp_ms(t0));
        }
    }
    return 0;
}

 