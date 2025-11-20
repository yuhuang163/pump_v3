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

/// 图片数据包装初始化.
#if CUST_LVGL_SUPPORT 
#define UI_IMG_DSC_PARMS_INIT(lv, w0, h0, x0, y0, alloc_flag) \
do { \
    memset((lv), 0, sizeof(ui_img_dsc_wrapper_t)); \
    (lv)->img_dsc.header.cf = UI_IMG_CF_TRUE_COLOR; \
    (lv)->img_dsc.header.w = (w0); \
    (lv)->img_dsc.header.h = (h0); \
    (lv)->img_dsc.data_size = IMG_SIZE_RGB565((w0), (h0)); \
    if (alloc_flag) { \
        (lv)->img_dsc.data = u_malloz(IMG_SIZE_RGB565((w0), (h0))); \
        if (!(lv)->img_dsc.data) { \
            LOGE("err: malloc failure."); \
        } \
    } \
    (lv)->x = (x0); \
    (lv)->y = (y0); \
} while (0)
#else
#define UI_IMG_DSC_PARMS_INIT(lv, w0, h0, x0, y0, alloc_flag) \
do { \
    memset((lv), 0, sizeof(ui_img_dsc_wrapper_t)); \
    (lv)->x = (x0); \
    (lv)->y = (y0); \
} while (0)
#endif

/// 叠图送给LVGL显示的步骤
 typedef enum {
    OVERLAY_IMG_DIS_STEP_FULL_BG,
    OVERLAY_IMG_DIS_STEP_SYNC_FB,
    OVERLAY_IMG_DIS_STEP_LVGL_SHOW,
    OVERLAY_IMG_DIS_STEP_MAX
} overlay_img_dis_step_e;

/// UI绘制.
#define FULL_COORDINATE(x0, y0) do { x = x0; y = y0; } while (0)

#define FULL_UI_PROPERTY_AUTO(draw, _id, _x, _y) \
    FULL_UI_PROPERTY(draw, _id, -1, -1, _x, _y)

#define FULL_UI_PROPERTY(draw, _id, _w, _h2, _x, _y) \
    FULL_UI_PROPERTY_OFFSET(draw, _id, _w, _x, _y, _w, _h2, 0, 0)

#define FULL_UI_PROPERTY_OFFSET(draw, _id, _w, _x, _y, _w2, _h2, _x0, _y0) \
    do { \
        ui_draw_property_t *d = (draw); \
        (d)->x = (_x); \
        (d)->y = (_y); \
        (d)->w = (_w); \
        (d)->x0 = (_x0); \
        (d)->y0 = (_y0); \
        (d)->w2 = (_w2); \
        (d)->h2 = (_h2); \
        (d)->res_id = (_id); \
        (d)->full_color = (0); \
    } while (0)

#define FULL_UI_COLOR(draw, _color0, _w0, _h0, _x0, _y0) \
    do { \
        ui_draw_property_t *d = (draw); \
        (d)->x = (_x0); \
        (d)->y = (_y0); \
        (d)->w = (_w0); \
        (d)->h = (_h0); \
        (d)->color = (_color0); \
        (d)->full_color = (1); \
    } while (0)

/// 动画绘制参数.
typedef struct {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    int16_t x0;
    int16_t y0;
    int16_t w2;
    int16_t h2;
    uint16_t res_id;
    uint16_t color;
    uint8_t full_color;
} ui_draw_property_t;

/// 动画绘制参数.
#define FULL_UI_ANI_PROPERTY(_user_data, _x, _y)  \
    do {    \
        ui_process_cb_parms_t *_user_bridge = (ui_process_cb_parms_t *)_user_data; \
        _user_bridge->x = (_x); \
        _user_bridge->y = (_y); \
    }while(0)



/// 动画绘制合并数组长度.
#ifndef MAX_UI_DRAW_PROP
#define MAX_UI_DRAW_PROP (8)
#endif

typedef struct _ui_play_context_t {
    /// 动画绘制上下文参数.
    ui_animation_play_t *ani;
    /// 动画绘制合并数组
    ui_draw_property_t draw[MAX_UI_DRAW_PROP];
    /// 合并数组长度.
    uint16_t draw_count;
    /// 动画绘制参数.
    ui_process_cb_parms_t *parms;
    /// 获取资源缓冲区
    // res_obj_desc_t *(*get_res_buf)(get_res_buf_args_t * p_args);

    #if IS_SUPPORT_OVERLAY_IMG
    /// 叠图图片数据初始化接口
    void (*overlay_img_wrap_init_cb)(void *user_data);
    /// 叠图接口，如果某个页面需要叠图，在页面实现 overlay_img_display_cb 即可
    void (*overlay_img_display_cb)(void *user_data, overlay_img_dis_step_e step);
    /// 动画绘制合并数组
    ui_draw_property_t overlay_img_draw[OVERLAY_IMG_NUM_MAX][MAX_UI_DRAW_PROP];
    /// 合并数组长度
    uint16_t overlay_img_draw_count[OVERLAY_IMG_NUM_MAX];
    /// 是否叠图显示
    uint8_t is_display_overlay_img_mode;
    #endif
} ui_play_context_t;

typedef struct {
    /// 传入 get_res_buf使用，一般为 window_context_t
    void *user_data;
    /// 需要获取的图片资源ID号
    uint16_t res_id; 
    /// 存放图片数据 draw_buffer
    uint8_t *dest;
    /// 移动的指针，用来提高刷新率，加载不同的图片时移动 dest_addr 位置即可，不需要memcpy
    uint8_t **dest_addr;
    /// 移动的指针 dest_addr 原始地址，当在内存列表里面找不到图片时，让 dest_addr 回到原位再从Flash读取数据
    uint8_t **dest_addr_base;
} get_res_buf_args_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 


