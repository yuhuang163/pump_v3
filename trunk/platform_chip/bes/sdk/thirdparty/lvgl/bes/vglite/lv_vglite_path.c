/**
 * @file lv_vglite_path.c
 *
 */

/**
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_vglite_path.h"

#if LV_USE_DRAW_BES_VGLITE

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

void vglite_create_rect_path_data(int32_t * path_data, uint32_t * path_data_size,
                                  int32_t radius,
                                  const lv_area_t * coords)
{
    int32_t rect_width = lv_area_get_width(coords);
    int32_t rect_height = lv_area_get_height(coords);

    /* Get the final radius. Can't be larger than the half of the shortest side */
    int32_t shortest_side = LV_MIN(rect_width, rect_height);
    int32_t final_radius = LV_MIN(radius, shortest_side / 2);

    /* Path data element index */
    uint8_t pidx = 0;

    if((radius == (int32_t)LV_RADIUS_CIRCLE) && (rect_width == rect_height)) {

        /* Get the control point offset for rounded cases */
        int32_t cpoff = (int32_t)((float)final_radius * BEZIER_OPTIM_CIRCLE);

        /* Circle case */
        /* Starting point */
        path_data[pidx++] = VLC_OP_MOVE;
        path_data[pidx++] = coords->x1 + final_radius;
        path_data[pidx++] = coords->y1;

        /* Top-right arc */
        path_data[pidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = cpoff;
        path_data[pidx++] = 0;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = final_radius - cpoff;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = final_radius;

        /* Bottom-right arc*/
        path_data[pidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0;
        path_data[pidx++] = cpoff;
        path_data[pidx++] = cpoff - final_radius;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = final_radius;

        /* Bottom-left arc */
        path_data[pidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0 - cpoff;
        path_data[pidx++] = 0;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = cpoff - final_radius;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = 0 - final_radius;

        /* Top-left arc*/
        path_data[pidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0;
        path_data[pidx++] = 0 - cpoff;
        path_data[pidx++] = final_radius - cpoff;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = 0 - final_radius;

        /* Ending point */
        path_data[pidx++] = VLC_OP_END;
    }
    else if(radius > 0) {
        /* Get the control point offset for rounded cases */
        int32_t cpoff = (int32_t)((float)final_radius * BEZIER_OPTIM_CIRCLE);

        /* Rounded rectangle case */
        /* Starting point */
        path_data[pidx++] = VLC_OP_MOVE;
        path_data[pidx++] = coords->x1 + final_radius;
        path_data[pidx++] = coords->y1;

        /* Top side */
        path_data[pidx++] = VLC_OP_LINE;
        path_data[pidx++] = coords->x2 - final_radius + 1;  /*Extended for VGLite*/
        path_data[pidx++] = coords->y1;

        /* Top-right corner */
        path_data[pidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = cpoff;
        path_data[pidx++] = 0;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = final_radius - cpoff;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = final_radius;

        /* Right side */
        path_data[pidx++] = VLC_OP_LINE;
        path_data[pidx++] = coords->x2 + 1;                 /*Extended for VGLite*/
        path_data[pidx++] = coords->y2 - final_radius + 1;  /*Extended for VGLite*/

        /* Bottom-right corner*/
        path_data[pidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0;
        path_data[pidx++] = cpoff;
        path_data[pidx++] = cpoff - final_radius;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = final_radius;

        /* Bottom side */
        path_data[pidx++] = VLC_OP_LINE;
        path_data[pidx++] = coords->x1 + final_radius;
        path_data[pidx++] = coords->y2 + 1;                 /*Extended for VGLite*/

        /* Bottom-left corner */
        path_data[pidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0 - cpoff;
        path_data[pidx++] = 0;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = cpoff - final_radius;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = 0 - final_radius;

        /* Left side*/
        path_data[pidx++] = VLC_OP_LINE;
        path_data[pidx++] = coords->x1;
        path_data[pidx++] = coords->y1 + final_radius;

        /* Top-left corner */
        path_data[pidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0;
        path_data[pidx++] = 0 - cpoff;
        path_data[pidx++] = final_radius - cpoff;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = 0 - final_radius;

        /* Ending point */
        path_data[pidx++] = VLC_OP_END;
    }
    else {
        /* Non-rounded rectangle case */
        /* Starting point */
        path_data[pidx++] = VLC_OP_MOVE;
        path_data[pidx++] = coords->x1;
        path_data[pidx++] = coords->y1;

        /* Top side */
        path_data[pidx++] = VLC_OP_LINE;
        path_data[pidx++] = coords->x2 + 1; /*Extended for VGLite*/
        path_data[pidx++] = coords->y1;

        /* Right side */
        path_data[pidx++] = VLC_OP_LINE;
        path_data[pidx++] = coords->x2 + 1; /*Extended for VGLite*/
        path_data[pidx++] = coords->y2 + 1; /*Extended for VGLite*/

        /* Bottom side */
        path_data[pidx++] = VLC_OP_LINE;
        path_data[pidx++] = coords->x1;
        path_data[pidx++] = coords->y2 + 1; /*Extended for VGLite*/

        /* Left side*/
        path_data[pidx++] = VLC_OP_LINE;
        path_data[pidx++] = coords->x1;
        path_data[pidx++] = coords->y1;

        /* Ending point */
        path_data[pidx++] = VLC_OP_END;
    }

    /* Resulting path size */
    *path_data_size = pidx * sizeof(int32_t);
}

void vglite_create_border_path_data(uint8_t * opcode, uint8_t * opcnt, int32_t * path_data,
                                  int32_t radius, int32_t line_width, const lv_area_t * coords,
                                  lv_border_side_t side)
{
    int32_t rect_width = lv_area_get_width(coords);
    int32_t rect_height = lv_area_get_height(coords);

    /* Get the final radius. Can't be larger than the half of the shortest side */
    int32_t shortest_side = LV_MIN(rect_width, rect_height);
    int32_t final_radius = LV_MIN(radius, shortest_side / 2);

    /* Path data element index */
    uint8_t cidx = 0;
    uint8_t pidx = 0;
    int32_t w = line_width / 2;
    if (w <= 0)
        w = 1;

    if (side != LV_BORDER_SIDE_FULL) { /* not FULL borders */
        int32_t x1, y1, x2, y2;

        /* Top border */
        if (side & LV_BORDER_SIDE_TOP) {
            x1 = coords->x1 - w;
            y1 = coords->y1 - w;
            x2 = coords->x2 + w;
            y2 = coords->y1 + w;

            opcode[cidx++] = VLC_OP_MOVE;
            path_data[pidx++] = x1;
            path_data[pidx++] = y1;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x2;
            path_data[pidx++] = y1;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x2;
            path_data[pidx++] = y2;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x1;
            path_data[pidx++] = y2;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x1;
            path_data[pidx++] = y1;
        }

        /* Bottom border */
        if (side & LV_BORDER_SIDE_BOTTOM) {
            x1 = coords->x1 - w;
            y1 = coords->y2 - w;
            x2 = coords->x2 + w;
            y2 = coords->y2 + w;

            opcode[cidx++] = VLC_OP_MOVE;
            path_data[pidx++] = x1;
            path_data[pidx++] = y1;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x2;
            path_data[pidx++] = y1;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x2;
            path_data[pidx++] = y2;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x1;
            path_data[pidx++] = y2;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x1;
            path_data[pidx++] = y1;
        }

        /* Left border */
        if (side & LV_BORDER_SIDE_LEFT) {
            x1 = coords->x1 - w;
            y1 = coords->y1 - w;
            x2 = coords->x1 + w;
            y2 = coords->y2 + w;

            opcode[cidx++] = VLC_OP_MOVE;
            path_data[pidx++] = x1;
            path_data[pidx++] = y1;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x2;
            path_data[pidx++] = y1;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x2;
            path_data[pidx++] = y2;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x1;
            path_data[pidx++] = y2;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x1;
            path_data[pidx++] = y1;
        }

        /* Right border */
        if (side & LV_BORDER_SIDE_RIGHT) {
            x1 = coords->x2 - w;
            y1 = coords->y1 - w;
            x2 = coords->x2 + w;
            y2 = coords->y2 + w;

            opcode[cidx++] = VLC_OP_MOVE;
            path_data[pidx++] = x1;
            path_data[pidx++] = y1;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x2;
            path_data[pidx++] = y1;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x2;
            path_data[pidx++] = y2;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x1;
            path_data[pidx++] = y2;

            opcode[cidx++] = VLC_OP_LINE;
            path_data[pidx++] = x1;
            path_data[pidx++] = y1;
        }

        /* Ending point */
        opcode[cidx++] = VLC_OP_END;

    } else if((radius == (int32_t)LV_RADIUS_CIRCLE) && (rect_width == rect_height)) {

        /* Get the control point offset for rounded cases */
        int32_t cpoff = (int32_t)((float)final_radius * BEZIER_OPTIM_CIRCLE);

        /* Outer circle */

        final_radius += w;

        /* Starting point */
        opcode[cidx++] = VLC_OP_MOVE;
        path_data[pidx++] = coords->x1 + final_radius;
        path_data[pidx++] = coords->y1;

        /* Top-right arc */
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = cpoff;
        path_data[pidx++] = 0;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = final_radius - cpoff;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = final_radius;

        /* Bottom-right arc*/
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0;
        path_data[pidx++] = cpoff;
        path_data[pidx++] = cpoff - final_radius;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = final_radius;

        /* Bottom-left arc */
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0 - cpoff;
        path_data[pidx++] = 0;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = cpoff - final_radius;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = 0 - final_radius;

        /* Top-left arc*/
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0;
        path_data[pidx++] = 0 - cpoff;
        path_data[pidx++] = final_radius - cpoff;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = 0 - final_radius;

        /* Inner circle */

        final_radius -= w * 2;

        /* Starting point */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = coords->x1 + final_radius;
        path_data[pidx++] = coords->y1;

        /* Top-right arc */
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = cpoff;
        path_data[pidx++] = 0;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = final_radius - cpoff;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = final_radius;

        /* Bottom-right arc*/
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0;
        path_data[pidx++] = cpoff;
        path_data[pidx++] = cpoff - final_radius;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = final_radius;

        /* Bottom-left arc */
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0 - cpoff;
        path_data[pidx++] = 0;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = cpoff - final_radius;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = 0 - final_radius;

        /* Top-left arc*/
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0;
        path_data[pidx++] = 0 - cpoff;
        path_data[pidx++] = final_radius - cpoff;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = 0 - final_radius;

        /* Ending point */
        opcode[cidx++] = VLC_OP_END;
    }
    else if(radius > 0) {
        /* Get the control point offset for rounded cases */
        int32_t cpoff = (int32_t)((float)final_radius * BEZIER_OPTIM_CIRCLE);

        int32_t x1, y1, x2, y2;

        /* Outer rounded rectangle */

        x1 = coords->x1 - w;
        y1 = coords->y1 - w;
        x2 = coords->x2 + w;
        y2 = coords->y2 + w;

        /* Starting point */
        opcode[cidx++] = VLC_OP_MOVE;
        path_data[pidx++] = x1 + final_radius;
        path_data[pidx++] = y1;

        /* Top side */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x2 - final_radius;
        path_data[pidx++] = y1;

        /* Top-right corner */
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = cpoff;
        path_data[pidx++] = 0;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = final_radius - cpoff;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = final_radius;

        /* Right side */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x2;
        path_data[pidx++] = y2 - final_radius;

        /* Bottom-right corner*/
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0;
        path_data[pidx++] = cpoff;
        path_data[pidx++] = cpoff - final_radius;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = final_radius;

        /* Bottom side */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x1 + final_radius;
        path_data[pidx++] = y2;

        /* Bottom-left corner */
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0 - cpoff;
        path_data[pidx++] = 0;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = cpoff - final_radius;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = 0 - final_radius;

        /* Left side*/
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x1;
        path_data[pidx++] = y1 + final_radius;

        /* Top-left corner */
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0;
        path_data[pidx++] = 0 - cpoff;
        path_data[pidx++] = final_radius - cpoff;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = 0 - final_radius;

        /* Inner rounded rectangle */

        x1 = coords->x1 + w;
        y1 = coords->y1 + w;
        x2 = coords->x2 - w;
        y2 = coords->y2 - w;

        /* Starting point */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x1 + final_radius;
        path_data[pidx++] = y1;

        /* Top side */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x2 - final_radius;
        path_data[pidx++] = y1;

        /* Top-right corner */
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = cpoff;
        path_data[pidx++] = 0;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = final_radius - cpoff;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = final_radius;

        /* Right side */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x2;
        path_data[pidx++] = y2 - final_radius;

        /* Bottom-right corner*/
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0;
        path_data[pidx++] = cpoff;
        path_data[pidx++] = cpoff - final_radius;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = final_radius;

        /* Bottom side */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x1 + final_radius;
        path_data[pidx++] = y2;

        /* Bottom-left corner */
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0 - cpoff;
        path_data[pidx++] = 0;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = cpoff - final_radius;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = 0 - final_radius;

        /* Left side*/
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x1;
        path_data[pidx++] = y1 + final_radius;

        /* Top-left corner */
        opcode[cidx++] = VLC_OP_CUBIC_REL;
        path_data[pidx++] = 0;
        path_data[pidx++] = 0 - cpoff;
        path_data[pidx++] = final_radius - cpoff;
        path_data[pidx++] = 0 - final_radius;
        path_data[pidx++] = final_radius;
        path_data[pidx++] = 0 - final_radius;

        opcode[cidx++] = VLC_OP_END;
    }
    else
    {
        int32_t x1, y1, x2, y2;

        /* Outer rectangle */

        x1 = coords->x1 - w;
        y1 = coords->y1 - w;
        x2 = coords->x2 + w;
        y2 = coords->y2 + w;

        /* Starting point */
        opcode[cidx++] = VLC_OP_MOVE;
        path_data[pidx++] = x1;
        path_data[pidx++] = y1;

        /* Top side */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x2;
        path_data[pidx++] = y1;

        /* Right side */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x2;
        path_data[pidx++] = y2;

        /* Bottom side */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x1;
        path_data[pidx++] = y2;

        /* Left side*/
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x1;
        path_data[pidx++] = y1;

        /* Inner rectangle */

        x1 = coords->x1 + w;
        y1 = coords->y1 + w;
        x2 = coords->x2 - w;
        y2 = coords->y2 - w;

        /* Starting point */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x1;
        path_data[pidx++] = y1;

        /* Top side */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x2;
        path_data[pidx++] = y1;

        /* Right side */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x2;
        path_data[pidx++] = y2;

        /* Bottom side */
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x1;
        path_data[pidx++] = y2;

        /* Left side*/
        opcode[cidx++] = VLC_OP_LINE;
        path_data[pidx++] = x1;
        path_data[pidx++] = y1;

        /* Ending point */
        opcode[cidx++] = VLC_OP_END;
    }

    *opcnt = cidx;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*LV_USE_DRAW_BES_VGLITE*/
