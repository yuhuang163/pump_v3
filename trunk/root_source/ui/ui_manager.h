#ifndef __UI_MANAGER_H__
#define __UI_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "project_config.h" // IWYU pragma: keep

#if IS_SUPPORT_LVGL
/// 删除LVGL对象
#define LVGL_OBJ_DELETE(obj) \
do { \
    if (obj && lv_obj_is_valid(obj)) { \
        lv_obj_del(obj); \
        obj = NULL; \
    } \
} while (0)

#define LVGL_OBJ_DELETE_ASYNC(obj) \
do { \
    if (obj && lv_obj_is_valid(obj)) { \
        lv_obj_del_async(obj); \
        obj = NULL; \
    } \
} while (0)

/// 清理LVGL对象
#define LVGL_OBJ_CLEAN() \
do { \
    lv_obj_t *screen = lv_screen_active(); \
    if (screen) { \
        lv_obj_clean(screen); \
    } \
} while (0)

/// 创建LVGL对象
#define LVGL_OBJ_CREATE(obj, parent) \
do { \
    if (obj == NULL || !lv_obj_is_valid(obj)) { \
        obj = lv_obj_create(parent); \
        return 1; \
    } \
    return 0; \
} while (0)

#endif

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 


