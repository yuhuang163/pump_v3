/**
 * @file lv_mem_core_clib.c
 */

/*********************
 *      INCLUDES
 *********************/
 #include "../lv_mem.h"
 #if LV_USE_STDLIB_MALLOC == LV_STDLIB_CLIB
 #include "../../stdlib/lv_mem.h"
 #include <stdlib.h>
 #include "hal_location.h"
 #include "heap_api.h"
 #include "hal_trace.h"
 /*********************
  *      DEFINES
  *********************/
 
 /**********************
  *      TYPEDEFS
  **********************/
 
 /**********************
  *  STATIC PROTOTYPES
  **********************/
 
 /**********************
  *  STATIC VARIABLES
  **********************/
 
 /**********************
  *      MACROS
  **********************/
 /**********************
  *   GLOBAL FUNCTIONS
  **********************/
 static PSRAM_BSS_LOC uint8_t lv_heap_mem[LV_MEM_SIZE] ALIGNED(64);
 static heap_handle_t lv_custom_heap = NULL;
 void lv_mem_init(void)
 {
     if (!lv_custom_heap) {
         lv_custom_heap = heap_register(lv_heap_mem, LV_MEM_SIZE);
         ASSERT(lv_custom_heap != NULL, "%s: lv custom heap register failed.", __func__);
     }
     return; /*Nothing to init*/
 }
 
 void lv_mem_deinit(void)
 {
    //  if (!lv_custom_heap || !lv_heap_mem)
    //      return;
    //  heap_free(lv_custom_heap, lv_heap_mem);
     return; /*Nothing to deinit*/
 }
 
 lv_mem_pool_t lv_mem_add_pool(void * mem, size_t bytes)
 {
     /*Not supported*/
     LV_UNUSED(mem);
     LV_UNUSED(bytes);
     return NULL;
 }
 
 void lv_mem_remove_pool(lv_mem_pool_t pool)
 {
     /*Not supported*/
     LV_UNUSED(pool);
     return;
 }
 
 void * lv_malloc_core(size_t size)
 {
     if(lv_custom_heap)
         return heap_malloc(lv_custom_heap, size);
     else
         return malloc(size);
 }
 
 void * lv_realloc_core(void * p, size_t new_size)
 {
     if(lv_custom_heap)
         return heap_realloc(lv_custom_heap, p, new_size);
     else
         return realloc(p, new_size);
 }
 
 void lv_free_core(void * p)
 {
     if(lv_custom_heap)
         heap_free(lv_custom_heap, p);
     else
         free(p);
 }
 
 void lv_mem_monitor_core(lv_mem_monitor_t * mon_p)
 {
     /*Not supported*/
     LV_UNUSED(mon_p);
     return;
 }
 
 lv_result_t lv_mem_test_core(void)
 {
     /*Not supported*/
     return LV_RESULT_OK;
 }
 
 /**********************
  *   STATIC FUNCTIONS
  **********************/
 
 #endif /*LV_STDLIB_CLIB*/
 