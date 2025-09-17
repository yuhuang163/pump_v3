
#ifndef __INCLUDE_CHECK_RAW_IMAGE_H
#define __INCLUDE_CHECK_RAW_IMAGE_H

#include "stdio.h"
#include "string.h"
/**
 * Included Files
 */


    /*!
     @abstract The pixel layout in a buffer.

     @discussion
     Pixels in a buffer may be tiled  or linear.
     */
    typedef enum {
        PIX_LINEAR,
        PIX_TILED,
    } pix_layout_t;

    /*!
     @abstract Format of a buffer.

     @discussion
     The pixel type for a <code>raw_image_t</code> structure.
     */
    typedef enum  {
        IMAGE_RGBA8888,   /*! 32-bit RGBA format with 8 bits per color channel. Red is in bits 7:0, green in bits 15:8, blue in
                             bits 23:16, and the alpha channel is in bits 31:24. */
        IMAGE_BGRA8888,   /*! 32-bit RGBA format with 8 bits per color channel. Red is in bits 23:16, green in bits 15:8, blue in
                             bits 7:0, and the alpha channel is in bits 31:24. */
        IMAGE_RGBX8888,   /*! 32-bit RGBX format with 8 bits per color channel. Red is in bits 7:0, green in bits 15:8, blue in
                             bits 23:16, and the x channel is in bits 31:24. */
        IMAGE_BGRX8888,   /*! 32-bit RGBX format with 8 bits per color channel. Red is in bits 23:16, green in bits 15:8, blue in
                             bits 7:0, and the x channel is in bits 31:24. */
        IMAGE_RGB565,     /*! 16-bit RGB format with 5 and 6 bits per color channel. Red is in bits 4:0, green in bits 10:5, and
                             the blue color channel is in bits 15:11. */
        IMAGE_BGR565,     /*! 16-bit RGB format with 5 and 6 bits per color channel. Red is in bits 15:11, green in bits 10:5,
                             and the blue color channel is in bits 4:0. */
        IMAGE_RGBA4444,   /*! 16-bit RGBA format with 4 bits per color channel. Red is in bits 3:0, green in bits 7:4, blue in
                             bits 11:8 and the alpha channel is in bits 15:12. */
        IMAGE_BGRA4444,   /*! 16-bit RGBA format with 4 bits per color channel. Red is in bits 11:8, green in bits 7:4, blue in
                             bits 3:0 and the alpha channel is in bits 15:12. */
        IMAGE_BGRA5551,   /*! 16-bit RGBA format with 4 bits per color channel. Red is in bits 14:10, green in bits 9:5, blue in
                             bits 4:0 and the alpha channel is in bit 15:15. */
        IMAGE_A4,         /*! 4-bit alpha format. There are no RGB values. */
        IMAGE_A8,         /*! 8-bit alpha format. There are no RGB values. */
        IMAGE_L8,         /*! 8-bit luminance value. There is no alpha value. */
        IMAGE_YUYV,       /*! Packed YUV format, 32-bit for 2 pixels. Y0 is in bits 7:0 and V is in bits 31:23. */

        IMAGE_YUY2,       /*! New formats. */
        IMAGE_NV12,
        IMAGE_ANV12,
        IMAGE_AYUY2,

        IMAGE_YV12,
        IMAGE_YV24,
        IMAGE_YV16,
        IMAGE_NV16,

        IMAGE_YUY2_TILED,  /*! Tiled YUV formats. */
        IMAGE_NV12_TILED,
        IMAGE_ANV12_TILED,
        IMAGE_AYUY2_TILED,

        IMAGE_INDEX_1 = 100,    /*! Indexed format. */
        IMAGE_INDEX_2,
        IMAGE_INDEX_4,
        IMAGE_INDEX_8,

        IMAGE_RGBA2222,      /*! 8-bit RGBA format with 2 bits per color channel.Red is in bits 1:0,green in bits 3:2,blue in
                               bits 5:4 and the alpha channel is in bits 7:6*/
        IMAGE_BGRA2222,      /*! 8-bit RGBA format with 2 bits per color channel.Blue is in bits 1:0,green in bits 3:2,red in
                               bits 5:4 and the alpha channel is in bits 7:6*/
        IMAGE_ABGR2222,      /*! 8-bit RGBA format with 2 bits per color channel.Alpha is in bits 1:0,blue in bits 3:2,green in
                               bits 5:4 and the red channel is in bits 7:6*/
        IMAGE_ARGB2222,      /*! 8-bit RGBA format with 2 bits per color channel.Alpha is in bits 1:0,red in bits 3:2,green in
                               bits 5:4 and the blue channel is in bits 7:6*/
        IMAGE_ABGR4444,      /*! 16-bit RGBA format with 4 bits per color channel. Alpha is in bits 3:0, blue in bits 7:4, green in
                               bits 11:8 and the red channel is in bits 15:12. */
        IMAGE_ARGB4444,      /*! 16-bit RGBA format with 4 bits per color channel. Alpha is in bits 3:0, red in bits 7:4, green in
                               bits 11:8 and the blue channel is in bits 15:12. */
        IMAGE_ABGR8888,      /*! 32-bit RGBA format with 8 bits per color channel. Alpha is in bits 7:0, blue in bits 15:8, green in
                               bits 23:16, and the red channel is in bits 31:24. */
        IMAGE_ARGB8888,      /*! 32-bit RGBA format with 8 bits per color channel. Alpha is in bits 7:0, red in bits 15:8, green in
                               bits 23:16, and the blue channel is in bits 31:24. */
        IMAGE_ABGR1555,      /*! 16-bit RGBA format with 4 bits per color channel. Red is in bits 15:11, green in bits 10:6, blue in
                               bits 5:1 and the alpha channel is in bit 0:0. */
        IMAGE_RGBA5551,      /*! 16-bit RGBA format with 4 bits per color channel. Blue is in bits 14:10, green in bits 9:5, red in
                               bits 4:0 and the alpha channel is in bit 15:15. */
        IMAGE_ARGB1555,      /*! 16-bit RGBA format with 4 bits per color channel. Blue is in bits 15:11, green in bits 10:6, red in
                               bits 5:1 and the alpha channel is in bit 0:0. */
        IMAGE_XBGR8888,      /*! 32-bit RGBX format with 8 bits per color channel. X channel is in bits 7:0, blue in bits 15:8, green in
                               bits 23:16, and the red is in bits 31:24. */
        IMAGE_XRGB8888,      /*! 32-bit RGBX format with 8 bits per color channel. X channel is in bits 7:0, red in bits 15:8, green in
                               bits 23:16, and the blue is in bits 31:24. */
        IMAGE_RGBA8888_ETC2_EAC,    /*! ETC2 format. */
        IMAGE_RGB888,        /*! 24-bit RGB format with 8 bits per color channel. Red is in bits 7:0, green in bits 15:8, blue in
                               bits 23:16. */
        IMAGE_BGR888,        /*! 24-bit RGB format with 8 bits per color channel. Blue is in bits 7:0, green in bits 15:8, red in
                               bits 23:16. */
        IMAGE_ABGR8565,      /*! 24-bit RGBA format with 4 and 5 bits per color channel. Alpha channel is in bit 7:0, blue in bits 12:8, green in
                               bits 18:13 and the red in bits 23:19. */
        IMAGE_BGRA5658,      /*! 24-bit RGBA format with 4 and 5 bits per color channel. Blue is in bits 4:0, green in bits 10:5, red in
                               bits 15:11,alpha channel is in bit 23:16. */
        IMAGE_ARGB8565,      /*! 24-bit RGBA format with 4 and 5 bits per color channel. Alpha channel is in bits 7:0, red in bits 12:8, green in
                               bits 18:13, and the blue channel is in bits 23:19. */
        IMAGE_RGBA5658,      /*! 24-bit RGBA format with 4 and 5 bits per color channel. Red is in bits 4:0, green in bits 10:5, blue in
                               bits 15:11 and the alpha channel is in bits 23:16 */
        IMAGE_ABGR8565_PLANAR,/*! 24-bit RGBA format with 4 and 5 bits per color channel. Red is in bits 4:0, green in bits 10:5, blue in
                               bits 15:11 and the alpha channel is in bits 23:16 */
        IMAGE_BGRA5658_PLANAR,/*! 24-bit RGBA format with 4 and 5 bits per color channel. Blue is in bits 4:0, green in bits 10:5, red in
                               bits 15:11,alpha channel is in bit 23:16. */
        IMAGE_ARGB8565_PLANAR,/*! 24-bit RGBA format with 4 and 5 bits per color channel. Alpha channel is in bits 7:0, red in bits 12:8, green in
                               bits 18:13, and the blue channel is in bits 23:19. */
        IMAGE_RGBA5658_PLANAR,/*! 24-bit RGBA format with 4 and 5 bits per color channel. Red is in bits 4:0, green in bits 10:5, blue in
                               bits 15:11 and the alpha channel is in bits 23:16 */
    } pix_format_t;

    typedef struct {
        int32_t width;                  /*! Width of the buffer in pixels. */
        int32_t height;                 /*! Height of the buffer in pixels. */
        int32_t stride;                 /*! The number of bytes to move from one line in the buffer to the next line. */
        pix_layout_t tiled;       /*! Indicating the buffer memory layout is linear or tiled. */
        pix_format_t fmt;          /*! The pixel format of the buffer. */
        void * memory;                  /*! The logical pointer to the buffer's memory for the CPU. */
        bool globle_alpha;
        uint8_t alpha;
    } raw_image_t;


uint32_t argb888torgba888(uint32_t c);

uint16_t rgba888torgb565(uint32_t c);

uint32_t rgba888torgb565a(uint32_t c);

uint32_t rgba888toargb565(uint32_t c);

int rgba8888_equal(uint32_t c1, uint32_t c2 );

int rgb565_equal(uint16_t c1, uint16_t c2 );

int rgba565_equal(uint32_t c1, uint32_t c2);

int argb565_equal(uint32_t c1, uint32_t c2);


int check_test_result(raw_image_t *buff,uint32_t *src_rgba8888);


#endif /* __INCLUDE_CHECK_RAW_IMAGE_H */
