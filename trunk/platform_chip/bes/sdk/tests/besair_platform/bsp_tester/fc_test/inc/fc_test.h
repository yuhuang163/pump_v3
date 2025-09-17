#ifndef __FC_TEST_H__
#define __FC_TEST_H__

#include <stdint.h>

typedef struct ModelState_s ModelState_t;

#ifdef __cplusplus
extern "C" {
#endif

ModelState_t* model_create();

void model_destroy(ModelState_t* st);

void loop_dtln(ModelState_t* model_st);

void run_fc_test(void);

#ifdef __cplusplus
}
#endif

#endif