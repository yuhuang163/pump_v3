/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "tests_manager.h"  // IWYU pragma: keep
#include "product_tests_common.h" 
#include "factory_tests_handler.h" 
#include "hw_self_tests_handler.h" 

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

// 填充公共测试的get/set接口
const static get_common_tests_process_handle_t get_common_tests_process_list[] = {

};

const static set_common_tests_process_handle_t set_common_tests_process_list[] = { 

};

// 填充硬件自检测试的get/set接口
const static get_hw_self_tests_process_handle_t get_hw_self_tests_process_list[] = {

};

const static set_hw_self_tests_process_handle_t set_hw_self_tests_process_list[] = { 

};

// 填充工厂测试的get/set接口
const static get_factory_tests_process_handle_t get_factory_tests_process_list[] = {
    {FAC_TST_READ_SW_VER_EVENT, get_factory_tests_sw_ver},
    {FAC_TST_AGING_TEST_GET_CURRENT_AGE_TIME_EVENT, get_factory_tests_aging_test_current_age_time},
};

const static set_factory_tests_process_handle_t set_factory_tests_process_list[] = { 
    {FAC_TST_SUCTION_TEST_EVENT, factory_tests_suction_test},
    {FAC_TST_AGING_TEST_ENTER_EVENT, factory_tests_aging_test_enter},
    {FAC_TST_AGING_TEST_EXIT_EVENT, factory_tests_aging_test_exit},
    {FAC_TST_ENTER_EVENT, factory_tests_enter}, 
    {FAC_TST_EXIT_EVENT, factory_tests_exit},
};


// 获取公共测试接口列表长度
const static uint16_t get_common_tests_process_list_len = sizeof(get_common_tests_process_list) / sizeof(get_hw_self_tests_process_handle_t);
const static uint16_t set_common_tests_process_list_len = sizeof(set_common_tests_process_list) / sizeof(set_hw_self_tests_process_handle_t);
const static uint16_t get_factory_tests_process_list_len = sizeof(get_factory_tests_process_list) / sizeof(get_factory_tests_process_handle_t);
const static uint16_t set_factory_tests_process_list_len = sizeof(set_factory_tests_process_list) / sizeof(set_factory_tests_process_handle_t);
const static uint16_t get_hw_self_tests_process_list_len = sizeof(get_hw_self_tests_process_list) / sizeof(get_hw_self_tests_process_handle_t);
const static uint16_t set_hw_self_tests_process_list_len = sizeof(set_hw_self_tests_process_list) / sizeof(set_hw_self_tests_process_handle_t); 

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int on_product_tests_br_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
    on_factory_tests_br_receiver_route(id, context, arg1, arg2, user_data);
    on_hw_self_tests_br_receiver_route(id, context, arg1, arg2, user_data);

    return 0;
}

static int product_tests_thread_route(struct tests_manager_context *ctx, void *msg)
{
    factory_tests_thread_handler(ctx, msg);
    hw_self_tests_thread_handler(ctx, msg);

    return 0;
}

static int on_post_product_tests_init_route(struct tests_manager_context *ctx)
{
    on_post_common_test_init(ctx);
    on_post_factory_tests_init(ctx);
    on_post_hw_self_tests_init(ctx);

    return 0;
}

static int on_perpare_product_tests_init_route(struct tests_manager_context *ctx)
{
    on_perpare_common_test_init(ctx);
    on_perpare_factory_tests_init(ctx);
    on_perpare_hw_self_tests_init(ctx);

    return 0;
}


/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_product_tests_hook(tests_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct product_tst_manager_hook));
    ctx->hook->on_perpare_init_route = on_post_product_tests_init_route;
    ctx->hook->on_post_init_route = on_perpare_product_tests_init_route;
    ctx->hook->on_product_tests_br_receiver_route = on_product_tests_br_receiver_route;
    ctx->hook->product_tests_thread_route = product_tests_thread_route;

    ctx->priv.get_commom_tests_process_list = get_common_tests_process_list;
    ctx->priv.set_commom_tests_process_list = set_common_tests_process_list;
    ctx->priv.get_common_tests_process_list_len = get_common_tests_process_list_len;
    ctx->priv.set_common_tests_process_list_len = set_common_tests_process_list_len;
    ctx->priv.get_factory_tests_process_list = get_factory_tests_process_list;
    ctx->priv.set_factory_tests_process_list = set_factory_tests_process_list;
    ctx->priv.get_factory_tests_process_list_len = get_factory_tests_process_list_len;
    ctx->priv.set_factory_tests_process_list_len = set_factory_tests_process_list_len;
    ctx->priv.get_hw_self_tests_process_list = get_hw_self_tests_process_list;
    ctx->priv.set_hw_self_tests_process_list = set_hw_self_tests_process_list;
    ctx->priv.get_hw_self_tests_process_list_len = get_hw_self_tests_process_list_len;
    ctx->priv.set_hw_self_tests_process_list_len = set_hw_self_tests_process_list_len;
}
