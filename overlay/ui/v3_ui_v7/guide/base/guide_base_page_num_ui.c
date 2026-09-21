/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define GUIDE_PAGE_NUM_CURRENT_COLOR                (0xFFFFFF)
#define GUIDE_PAGE_NUM_TOTAL_COLOR                  (0x999999)

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_page_num_ui_remove(guide_base_page_num_info_t *page_num)
{
    EXCEPTION_CHECK_VOID(page_num);
    memset(page_num, 0, sizeof(*page_num));
}

void guide_page_num_ui_build(guide_base_page_num_info_t *page_num, lv_obj_t *parent,
    uint8_t current, uint8_t total)
{
    char cur_buf[4];
    char total_buf[8];
    lv_coord_t current_w;
    lv_coord_t total_w;
    lv_coord_t group_w;
    lv_coord_t parent_w;
    lv_coord_t current_x;
    lv_coord_t total_x;

    EXCEPTION_CHECK_VOID(page_num && parent);

    lv_snprintf(cur_buf, sizeof(cur_buf), "%u", current);
    lv_snprintf(total_buf, sizeof(total_buf), "/ %u", total);

    if (page_num->current == NULL || !lv_obj_is_valid(page_num->current)) {
        page_num->current = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(page_num->current);
    }
    lv_label_set_text(page_num->current, cur_buf);
    lv_obj_set_style_text_color(page_num->current, lv_color_hex(GUIDE_PAGE_NUM_CURRENT_COLOR), 0);
    lv_obj_set_style_text_font(page_num->current, UI_FONT_AEONIK_29, 0);
    lv_obj_update_layout(page_num->current);
    current_w = lv_obj_get_width(page_num->current);

    if (page_num->total == NULL || !lv_obj_is_valid(page_num->total)) {
        page_num->total = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(page_num->total);
    }
    lv_label_set_text(page_num->total, total_buf);
    lv_obj_set_style_text_color(page_num->total, lv_color_hex(GUIDE_PAGE_NUM_TOTAL_COLOR), 0);
    lv_obj_set_style_text_font(page_num->total, UI_FONT_AEONIK_19, 0);
    lv_obj_update_layout(page_num->total);
    total_w = lv_obj_get_width(page_num->total);

    lv_obj_update_layout(parent);
    parent_w = lv_obj_get_width(parent);
    group_w = current_w + GUIDE_PAGE_NUM_GAP_X + total_w;
    current_x = (parent_w - group_w) / 2;
    total_x = current_x + current_w + GUIDE_PAGE_NUM_GAP_X;

    lv_obj_align(page_num->current, LV_ALIGN_TOP_LEFT, current_x, GUIDE_PAGE_NUM_CURRENT_Y);
    lv_obj_align(page_num->total, LV_ALIGN_TOP_LEFT, total_x, GUIDE_PAGE_NUM_TOTAL_Y);
}
