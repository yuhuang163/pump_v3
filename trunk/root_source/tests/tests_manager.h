#ifndef __TESTS_MANAGER_H__
#define __TESTS_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "tests_manager_api.h" // IWYU pragma: keep


#define PRODUCT_TESTS_THREAD_POLLING_PERIOD         (1000)

typedef struct {
    void *user_data;  ///填充调用者上下文结构体指针
    on_product_test_evt_cb_fptr cb;
    int report_type;
    void *report_data;
    list_head_t list;
} product_tst_evt_client_node_t;

void *register_module_for_tests_manager_new(list_head_t *list_head);

#ifdef __cplusplus
}
#endif

#endif 


