#ifndef __TESTS_MANAGER_API_H__
#define __TESTS_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

typedef struct tests_manager_context {
    void (*run_smoke_tests)(void);
    void (*run_integration_tests)(void);
    void (*run_hw_diagnostics)(void);

    struct __tests_private {
        thread_context_t tests_thd;
        os_timer_id tests_timer;
        uint8_t enabled;
    } priv;
} tests_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


