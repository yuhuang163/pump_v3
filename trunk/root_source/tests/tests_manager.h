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

void *register_module_for_tests_manager_new(list_head_t *list_head);

#ifdef __cplusplus
}
#endif

#endif 


