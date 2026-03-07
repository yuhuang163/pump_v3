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
#if 0
    if(mon_p == NULL) {
        return;
    }

    if(lv_custom_heap == NULL) {
        return;
    }

    multi_heap_info_t info = {0};
    heap_get_info(lv_custom_heap, &info);

    mon_p->total_size = info.total_bytes;
    mon_p->free_cnt = info.free_blocks;
    mon_p->free_size = info.total_free_bytes;
    mon_p->free_biggest_size = info.largest_free_block;
    mon_p->used_cnt = info.allocated_blocks;

    if(info.total_bytes >= info.minimum_free_bytes) {
        mon_p->max_used = info.total_bytes - info.minimum_free_bytes;
    }
    else {
        mon_p->max_used = info.total_allocated_bytes;
    }

    if(mon_p->total_size > 0) {
        mon_p->used_pct = (uint8_t)((100U * (mon_p->total_size - mon_p->free_size)) / mon_p->total_size);
    }
    else {
        mon_p->used_pct = 0;
    }

    if(mon_p->free_size > 0) {
        mon_p->frag_pct = (uint8_t)(100U - ((mon_p->free_biggest_size * 100U) / mon_p->free_size));
    }
    else {
        mon_p->frag_pct = 0;
    }
#endif
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
 