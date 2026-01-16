#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>
#include "hal_location.h"

#include "disp_conf.h"

/* display configs */
#define LV_HOR_RES_MAX                      (LCDC_WIDTH)
#define LV_VER_RES_MAX                      (LCDC_HEIGHT)

#ifdef CONFIG_LCD_RGB565
#define LV_COLOR_DEPTH                      16
#elif defined(CONFIG_LCD_RGB888)
#define LV_COLOR_DEPTH                      24
#else
#define LV_COLOR_DEPTH                      32
#endif

#define LV_DEF_REFR_PERIOD                  16      /* ms, 60fps */
#define LV_DPI_DEF                          218

#ifdef LVGL_GPU_VGLITE
#define LV_USE_DRAW_BES_VGLITE              1
#define LV_USE_BES_VGLITE_DRAW_THREAD       1
#ifdef CONFIG_LCDC_COMPRESS
#define LV_USE_BES_VGLITE_COMPRESS          1
#define LV_USE_DRAW_SW                      0
#if (CONFIG_LCDC_COMPRESS == 2)
#define LV_BES_VGLITE_COMPRESS_MODE         2
#else
#define LV_BES_VGLITE_COMPRESS_MODE         1
#endif
#endif
#define LV_USE_VECTOR_GRAPHIC               1
#else
#define LV_USE_DRAW_BES_VGLITE              0
#endif

#ifdef LVGL_BES_JPEG
#define LV_USE_BESJPEG                      1
#ifdef LVGL_BES_BAJ
#define LV_USE_BESBAJ                       1
#endif
#else
#define LV_USE_BESJPEG                      0
#endif

#ifdef LVGL_BES_PNG
#define LV_USE_BESPNG                       1
#else
#define LV_USE_BESPNG                       0
#endif

#ifdef LVGL_BES_ETC2
#define LV_USE_BESETC2                      1
#else
#define LV_USE_BESETC2                      0
#endif

#ifdef LVGL_BES_SVG
#define LV_USE_BESSVG                       1
#else
#define LV_USE_BESSVG                       0
#endif

#ifdef LVGL_SNAPSHOT
#define LV_USE_SNAPSHOT                     1
#else
#define LV_USE_SNAPSHOT                     0
#endif

/* memory configs */
#define LV_MEM_SIZE                         (5 * 1024U * 1024U)
// #define LV_CACHE_DEF_SIZE                   (2 * 1024U * 1024U)
// #define LV_IMAGE_HEADER_CACHE_DEF_CNT       128
#define LV_ATTRIBUTE_MEM_ALIGN_SIZE         64
#define LV_ATTRIBUTE_MEM_ALIGN              __attribute__((aligned(64)))

#define LV_ATTRIBUTE_LARGE_CONST            PSRAM_DATA_LOC
#define LV_ATTRIBUTE_LARGE_RAM_ARRAY        PSRAM_BSS_LOC

#define LV_DRAW_THREAD_STACK_SIZE           (64 * 1024U)

/* enable logging */
#define LV_USE_LOG                          1

/* enable system monitor */
#define LV_USE_SYSMON                       0
#define LV_USE_PERF_MONITOR                 0

/* font configs */
#define LV_FONT_MONTSERRAT_14               1
#define LV_FONT_MONTSERRAT_16               0
#define LV_FONT_MONTSERRAT_18               0
#define LV_FONT_MONTSERRAT_20               1
#define LV_FONT_MONTSERRAT_22               0
#define LV_FONT_MONTSERRAT_24               1
#define LV_FONT_MONTSERRAT_26               1
#define LV_FONT_MONTSERRAT_32               0
#define LV_FONT_MONTSERRAT_40               0
#define LV_FONT_MONTSERRAT_48               0
#define LV_USE_FONT_COMPRESSED              0

#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW    0
#define LV_FONT_SIMSUN_16_CJK               1

#define LV_FONT_DEFAULT                     &lv_font_montserrat_14

/* lvgl features */
#define LV_USE_IMGFONT                      1
#define LV_USE_MATRIX                       1
#define LV_USE_FLOAT                        1

/* enable freetype */
#ifdef LVGL_FREETYPE
#define LV_USE_FREETYPE                     1
#define LV_FREETYPE_USE_LVGL_PORT           1
#endif

/* enable memfs */
#define LV_USE_FS_MEMFS                     1
#define LV_FS_MEMFS_LETTER                  'A'

/* examples and demos */
#define LV_BUILD_EXAMPLES                   1
#define LV_USE_DEMO_WIDGETS                 1
#define LV_USE_DEMO_KEYPAD_AND_ENCODER      0
#define LV_USE_DEMO_BENCHMARK               1
#define LV_USE_DEMO_RENDER                  0
#define LV_USE_DEMO_STRESS                  1
#define LV_USE_DEMO_MUSIC                   1
#define LV_USE_DEMO_FLEX_LAYOUT             1
#define LV_USE_DEMO_MULTILANG               0
#define LV_USE_DEMO_TRANSFORM               0
#define LV_USE_DEMO_SCROLL                  1
#define LV_USE_DEMO_VECTOR_GRAPHIC          0

#define LV_ATTRIBUTE_IMAGE_IMG_COGWHEEL_ARGB    PSRAM_DATA_LOC
#define LV_ATTRIBUTE_IMAGE_IMG_COGWHEEL_RGB     PSRAM_DATA_LOC

#define LV_USE_OS                           LV_OS_CUSTOM
#define LV_OS_CUSTOM_INCLUDE                <lv_os_rtos.h>

#define LV_BIN_DECODER_RAM_LOAD             1
#define LV_USE_LZ4_INTERNAL                 1

#define LV_USE_QRCODE                       1

#endif /* LV_CONF_H */
