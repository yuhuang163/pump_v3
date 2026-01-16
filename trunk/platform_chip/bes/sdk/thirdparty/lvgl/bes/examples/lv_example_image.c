#include "../../examples/lv_examples.h"
#if defined(LV_USE_IMAGE) && defined(LV_FS_MEMFS_LETTER)
#include "src/misc/lv_fs_private.h"
#include "hal_location.h"
#include "hal_cache.h"
#include "string.h"

LV_IMAGE_DECLARE(img_cogwheel_argb);
LV_IMAGE_DECLARE(img_cogwheel_rgb);
LV_IMAGE_DECLARE(img_cogwheel_indexed16);
LV_IMAGE_DECLARE(img_mario_i2);
LV_IMAGE_DECLARE(img_mario_i4);
LV_IMAGE_DECLARE(img_mario_i8);

#define VAR_IMG_COUNT   6
static const lv_image_dsc_t *var_images[VAR_IMG_COUNT] = {
    &img_cogwheel_argb,
    &img_cogwheel_rgb,
    &img_cogwheel_indexed16,
    &img_mario_i2,
    &img_mario_i4,
    &img_mario_i8,
};

static PSRAM_DATA_LOC ALIGNED(64) uint8_t bin_data[] = {
#include "./res/img_mario_bin.txt"
};

struct img_data_t {
    uint8_t *data;
    uint32_t size;
};

#if LV_USE_BESJPEG
static PSRAM_DATA_LOC uint8_t jpeg_data0[] = {
#include "./res/jpeg_yuv422_128x128.txt"
};
static PSRAM_DATA_LOC uint8_t jpeg_data1[] = {
#include "./res/jpeg_yuv444_100x68.txt"
};
static PSRAM_DATA_LOC uint8_t jpeg_data2[] = {
#include "./res/jpeg_yuv444_65x65.txt"
};
static PSRAM_DATA_LOC uint8_t jpeg_data3[] = {
#include "./res/jpeg_yuv444_61x58.txt"
};
static PSRAM_DATA_LOC uint8_t jpeg_data4[] = {
#include "./res/jpeg_yuv420_100x72.txt"
};
static PSRAM_DATA_LOC uint8_t jpeg_data5[] = {
#include "./res/jpeg_yuv420_75x100.txt"
};
#define JPEG_IMG_COUNT  6
static struct img_data_t jpeg_images[JPEG_IMG_COUNT] = {
    { jpeg_data0, sizeof(jpeg_data0) },
    { jpeg_data1, sizeof(jpeg_data1) },
    { jpeg_data2, sizeof(jpeg_data2) },
    { jpeg_data3, sizeof(jpeg_data3) },
    { jpeg_data4, sizeof(jpeg_data4) },
    { jpeg_data5, sizeof(jpeg_data5) },
};
#endif

#if LV_USE_BESPNG
static PSRAM_DATA_LOC ALIGNED(64) uint8_t png_data0[] = {
#include "./res/png_200x200_icon.txt"
};
static PSRAM_DATA_LOC ALIGNED(64) uint8_t png_data1[] = {
#include "./res/png_180x180_seashore.txt"
};
static PSRAM_DATA_LOC ALIGNED(64) uint8_t png_data2[] = {
#include "./res/png_128x128_mario.txt"
};
static PSRAM_DATA_LOC ALIGNED(64) uint8_t png_data3[] = {
#include "./res/png_200x100_30fps.txt"
};
static PSRAM_DATA_LOC ALIGNED(64) uint8_t png_data4[] = {
#include "./res/png_80x40_battery.txt"
};
static PSRAM_DATA_LOC ALIGNED(64) uint8_t png_data5[] = {
#include "./res/png_128x128_lena.txt"
};
#define PNG_IMG_COUNT  6
static struct img_data_t png_images[PNG_IMG_COUNT] = {
    { png_data0, sizeof(png_data0) },
    { png_data1, sizeof(png_data1) },
    { png_data2, sizeof(png_data2) },
    { png_data3, sizeof(png_data3) },
    { png_data4, sizeof(png_data4) },
    { png_data5, sizeof(png_data5) },
};
#endif

#if LV_USE_BESSVG
static PSRAM_DATA_LOC ALIGNED(64) uint8_t svg_data0[] = {
#include "./res/svg_fill_01.txt"
};
static PSRAM_DATA_LOC ALIGNED(64) uint8_t svg_data1[] = {
#include "./res/svg_stroke_02.txt"
};
static PSRAM_DATA_LOC ALIGNED(64) uint8_t svg_data2[] = {
#include "./res/svg_paths_03.txt"
};
static PSRAM_DATA_LOC ALIGNED(64) uint8_t svg_data3[] = {
#include "./res/svg_polyline_04.txt"
};
static PSRAM_DATA_LOC ALIGNED(64) uint8_t svg_data4[] = {
#include "./res/svg_fish_05.txt"
};
static PSRAM_DATA_LOC ALIGNED(64) uint8_t svg_data5[] = {
#include "./res/svg_tiger_06.txt"
};
#define SVG_IMG_COUNT  6
static struct img_data_t svg_images[SVG_IMG_COUNT] = {
    { svg_data0, sizeof(svg_data0) },
    { svg_data1, sizeof(svg_data1) },
    { svg_data2, sizeof(svg_data2) },
    { svg_data3, sizeof(svg_data3) },
    { svg_data4, sizeof(svg_data4) },
    { svg_data5, sizeof(svg_data5) },
};
#endif

#if LV_USE_BESETC2
static PSRAM_DATA_LOC ALIGNED(64) uint8_t etc2_data0[] = {
#include "./res/etc2pkm_160x160.txt"
};
static PSRAM_DATA_LOC ALIGNED(64) uint8_t etc2_data1[] = {
#include "./res/etc2pkm_142x139.txt"
};
#define ETC2_IMG_COUNT  2
static struct img_data_t etc2_images[ETC2_IMG_COUNT] = {
    { etc2_data0, sizeof(etc2_data0) },
    { etc2_data1, sizeof(etc2_data1) },
};
#endif

static lv_obj_t *img_var, *img_bin, *img_sym;
#if LV_USE_BESJPEG
static lv_obj_t *img_jpeg;
#endif
#if LV_USE_BESPNG
static lv_obj_t *img_png;
#endif
#if LV_USE_BESSVG
static lv_obj_t *img_svg;
#endif
#if LV_USE_BESETC2
static lv_obj_t *img_etc2;
#endif

#if LV_USE_QRCODE
static lv_obj_t *qrcode;
#define QRSTR_COUNT   6
const char *qr_str[QRSTR_COUNT] = {
    "https://lvgl.io",
    "AAAAAAAAAAAAAAAA",
    "Hello World!",
    "LVGL IMAGES",
    "BES",
    "OK, All is done!",
};
#endif

static void change_images(lv_timer_t * tmr)
{
    static uint32_t index = 0;
    (void) tmr;

#if LV_USE_BESJPEG
    lv_fs_path_ex_t jpg_path;
    uint8_t *jpeg_data = jpeg_images[index % JPEG_IMG_COUNT].data;
    uint32_t jpeg_size = jpeg_images[index % JPEG_IMG_COUNT].size;
    lv_fs_make_path_from_buffer(&jpg_path, LV_FS_MEMFS_LETTER, jpeg_data, jpeg_size, "jpg");
    lv_image_set_src(img_jpeg, (const void *)&jpg_path);
#endif

#if LV_USE_BESPNG
    lv_fs_path_ex_t png_path;
    uint8_t *png_data = png_images[index % PNG_IMG_COUNT].data;
    uint32_t png_size = png_images[index % PNG_IMG_COUNT].size;
    lv_fs_make_path_from_buffer(&png_path, LV_FS_MEMFS_LETTER, png_data, png_size, "png");
    lv_image_set_src(img_png, (const void *)&png_path);
#endif

#if LV_USE_BESSVG
    lv_fs_path_ex_t svg_path;
    uint8_t *svg_data = svg_images[index % SVG_IMG_COUNT].data;
    uint32_t svg_size = svg_images[index % SVG_IMG_COUNT].size;
    lv_fs_make_path_from_buffer(&svg_path, LV_FS_MEMFS_LETTER, svg_data, svg_size, "svg");
    lv_image_set_src(img_svg, (const void *)&svg_path);
#endif

#if LV_USE_BESETC2
    lv_fs_path_ex_t etc2_path;
    uint8_t *etc2_data = etc2_images[index % ETC2_IMG_COUNT].data;
    uint32_t etc2_size = etc2_images[index % ETC2_IMG_COUNT].size;
    lv_fs_make_path_from_buffer(&etc2_path, LV_FS_MEMFS_LETTER, etc2_data, etc2_size, "pkm");
    lv_image_set_src(img_etc2, (const void *)&etc2_path);
#endif

#if LV_USE_QRCODE
    lv_qrcode_update(qrcode, qr_str[index % QRSTR_COUNT], strlen(qr_str[index % QRSTR_COUNT]));
#endif

    lv_image_set_src(img_var, var_images[index % VAR_IMG_COUNT]);
    index++;
}

void lv_example_image(void)
{
    //lv_obj_set_style_bg_color(lv_screen_active(), lv_color_make(0,0,0), LV_PART_MAIN);

    /* SRC_VARIBLE */
    img_var = lv_image_create(lv_screen_active());
    lv_image_set_src(img_var, &img_cogwheel_argb);
    lv_obj_align(img_var, LV_ALIGN_CENTER, -140, 80);

    /* SRC_FILE */
    img_bin = lv_image_create(lv_screen_active());
    lv_fs_path_ex_t img_path;
    lv_fs_make_path_from_buffer(&img_path, LV_FS_MEMFS_LETTER, bin_data, sizeof(bin_data), "bin");
    //lv_obj_set_style_image_recolor(img_bin, (lv_color_t)LV_COLOR_MAKE(0xff, 0, 0), LV_PART_MAIN);
    //lv_obj_set_style_image_recolor_opa(img_bin, LV_OPA_COVER, LV_PART_MAIN);
    lv_image_set_src(img_bin, (const void *)&img_path);
    lv_obj_align(img_bin, LV_ALIGN_CENTER, 0, 80);

    /* SRC_SYMBOL */
    img_sym = lv_image_create(lv_screen_active());
    lv_image_set_src(img_sym, LV_SYMBOL_OK "Accept");
    lv_obj_align_to(img_sym, img_bin, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

#if LV_USE_BESJPEG
    /* JPEG */
    img_jpeg = lv_image_create(lv_screen_active());
    lv_fs_path_ex_t jpg_path;
    lv_fs_make_path_from_buffer(&jpg_path, LV_FS_MEMFS_LETTER, jpeg_data0, sizeof(jpeg_data0), "jpg");
    lv_image_set_src(img_jpeg, (const void *)&jpg_path);
    lv_obj_align(img_jpeg, LV_ALIGN_CENTER, 140, 80);
#endif

#if LV_USE_BESPNG
    /* PNG */
    img_png = lv_image_create(lv_screen_active());
    lv_fs_path_ex_t png_path;
    lv_fs_make_path_from_buffer(&png_path, LV_FS_MEMFS_LETTER, png_data0, sizeof(png_data0), "png");
    lv_image_set_src(img_png, (const void *)&png_path);
    lv_obj_align(img_png, LV_ALIGN_CENTER, -70, -120);
#endif

#if LV_USE_BESSVG
    /* SVG */
    img_svg = lv_image_create(lv_screen_active());
    lv_fs_path_ex_t svg_path;
    lv_fs_make_path_from_buffer(&svg_path, LV_FS_MEMFS_LETTER, svg_data0, sizeof(svg_data0), "svg");
    lv_image_set_src(img_svg, (const void *)&svg_path);
    lv_obj_align(img_svg, LV_ALIGN_CENTER, -60, -80);
#endif

#if LV_USE_BESETC2
    /* ETC2 */
    img_etc2 = lv_image_create(lv_screen_active());
    lv_fs_path_ex_t etc2_path;
    lv_fs_make_path_from_buffer(&etc2_path, LV_FS_MEMFS_LETTER, etc2_data0, sizeof(etc2_data0), "pkm");
    lv_image_set_src(img_etc2, (const void *)&etc2_path);
    lv_obj_align(img_etc2, LV_ALIGN_CENTER, -70, -120);
#endif

#if LV_USE_QRCODE
    /* qrcode */
    lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 5);
    lv_color_t fg_color = lv_palette_darken(LV_PALETTE_BLUE, 4);
    qrcode = lv_qrcode_create(lv_screen_active());
    lv_qrcode_set_size(qrcode, 120);
    lv_qrcode_set_dark_color(qrcode, fg_color);
    lv_qrcode_set_light_color(qrcode, bg_color);
    lv_obj_align(qrcode, LV_ALIGN_CENTER, 120, -80);
    lv_obj_set_style_border_color(qrcode, bg_color, 0);
    lv_obj_set_style_border_width(qrcode, 5, 0);

    lv_qrcode_update(qrcode, qr_str[0], strlen(qr_str[0]));
#endif

    lv_timer_t * t = lv_timer_create(change_images, 200, NULL);
    lv_timer_ready(t);
}

#endif
