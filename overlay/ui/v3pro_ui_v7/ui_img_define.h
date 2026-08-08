#ifndef __UI_IMG_DEFINE_H__
#define __UI_IMG_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#if IS_SUPPORT_LVGL
#include "lv_user_font.h" // IWYU pragma: keep
#include "resource_id_define.h" // IWYU pragma: keep
#endif

typedef struct {
    const uint8_t *data;
    uint32_t size;
} ui_img_data_t;

#define UI_IMG_RES_ID_NONE                        (0)

#ifdef __cplusplus
}
#endif

#endif 
