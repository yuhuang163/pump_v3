#include "../../examples/lv_examples.h"

#if LV_USE_FREETYPE
#include "src/misc/lv_fs_private.h"
#include "hal_location.h"

static PSRAM_DATA_LOC uint8_t ttf_data[] = {
#include "ttf-MiSans.txt"
};

void lv_example_freetype(void)
{
    /* Create a memfs path */
    lv_fs_path_ex_t font_path;
    lv_fs_make_path_from_buffer(&font_path, LV_FS_MEMFS_LETTER, ttf_data, sizeof(ttf_data), "ttf");

    lv_font_t *font_bitmap = lv_freetype_font_create((const char *)&font_path,
                                               LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                               32,
                                               LV_FREETYPE_FONT_STYLE_NORMAL);

    if (!font_bitmap) {
        LV_LOG_ERROR("freetype bitmap font create failed.");
        return;
    }

    lv_font_t *font_outline = lv_freetype_font_create((const char *)&font_path,
                                               LV_FREETYPE_FONT_RENDER_MODE_OUTLINE,
                                               32,
                                               LV_FREETYPE_FONT_STYLE_NORMAL);

    if (!font_outline) {
        LV_LOG_ERROR("freetype outline font create failed.");
        return;
    }

    static lv_style_t style1, style2;
    lv_obj_t *label1, *label2;

    /* label with bitmap font  */
    lv_style_init(&style1);
    lv_style_set_text_font(&style1, font_bitmap);
    lv_style_set_text_color(&style1, lv_color_make(0, 0, 0xff));
    lv_style_set_text_align(&style1, LV_TEXT_ALIGN_CENTER);

    label1 = lv_label_create(lv_screen_active());
    lv_obj_add_style(label1, &style1, 0);
    lv_label_set_text(label1, "Hello World!\n世界你好!");
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, -40);

    /* label with outline font */
    lv_style_init(&style2);
    lv_style_set_text_font(&style2, font_outline);
    lv_style_set_text_color(&style2, lv_color_make(0xff, 0, 0));
    lv_style_set_text_align(&style2, LV_TEXT_ALIGN_CENTER);

    label2 = lv_label_create(lv_screen_active());
    lv_obj_add_style(label2, &style2, 0);
    lv_label_set_text(label2, "海上升明月,天涯共此时!");
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 40);
}

#endif
