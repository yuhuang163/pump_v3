/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static tests_manager_context_t *thiz = NULL;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void local_run_smoke_tests(void)
{
    LOGI("tests: run_smoke_tests");
}

static void local_run_integration_tests(void)
{
    LOGI("tests: run_integration_tests");
}

static void local_run_hw_diagnostics(void)
{
    LOGI("tests: run_hw_diagnostics");
}

static int on_prepare_tests_init(void *self)
{
    tests_manager_context_t *ctx = (tests_manager_context_t *)self;
    (void)ctx;
    return 0;
}

static int on_post_tests_init(void *self)
{
    tests_manager_context_t *ctx = (tests_manager_context_t *)self;
    (void)ctx;
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_tests_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, tests_manager_context_t);

    ptr->run_smoke_tests = local_run_smoke_tests;
    ptr->run_integration_tests = local_run_integration_tests;
    ptr->run_hw_diagnostics = local_run_hw_diagnostics;

    thiz = ptr;
    return ADD_MODULE("tests", ID_MODULE_TESTS, on_prepare_tests_init, on_post_tests_init,
                      ptr, list_head, sizeof(tests_manager_context_t));
}


