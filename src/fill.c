/* $Id: fill.c,v 1.3 2004/12/25 04:41:58 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003, 2007  Li-Cheng (Andy) Tai
 *
 * Authors: Li-Cheng (Andy) Tai
 *          Michael A. Meffie III <meffiem@neo.rr.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "fill.h"
#include "image.h"
#include "debug.h"

#define STACKSIZE 10000

typedef struct _gpaint_fill
{
    gpaint_tool tool;
} gpaint_fill;


struct fillinfo
{
   unsigned char *rgb; 
   int rowstride;
   int pixelsize; 
   int width;
   int height;
   unsigned char or, og, ob, r, g, b;
};

struct fillpixelinfo
{
   int y, xl, xr, dy;
};


#define GPAINT_FILL(tool)  ((gpaint_fill*)(tool))

#define SHIFT_COLOR(NAME) \
 (color->pixel & visual->NAME##_mask) >> \
 visual->NAME##_shift << \
 (sizeof(unsigned char) * 8 - visual->NAME##_prec);

#define PUSH(py, pxl, pxr, pdy) \
{ \
    struct fillpixelinfo *p = sp;\
    if (((py) + (pdy) >= 0) && ((py) + (pdy) < info->height))\
    {\
        p->y = (py);\
        p->xl = (pxl);\
        p->xr = (pxr);\
        p->dy = (pdy);\
        sp++; \
    }\
}
   
#define POP(py, pxl, pxr, pdy) \
{\
    sp--;\
    (py) = sp->y + sp->dy;\
    (pxl) = sp->xl;\
    (pxr) = sp->xr;\
    (pdy) = sp->dy;\
}


static void fill_destroy(gpaint_tool *tool);
static void fill_bounded_area(gpaint_tool *tool, int x, int y);
static void convert_color(const GdkColor *color, unsigned char *r, unsigned char *g, unsigned char *b);
static void flood_fill_algo(struct fillinfo *info, int x, int y);
static void set_new_pixel_value(struct fillinfo *info, int x, int y);
static int is_old_pixel_value(struct fillinfo *info, int x, int y);

gpaint_tool *fill_create(const char *name)
{
    gpaint_fill *fill = g_new0(gpaint_fill, 1);
    g_assert(fill);

    GPAINT_TOOL(fill)->name = name;
    GPAINT_TOOL(fill)->cursor = gdk_cursor_new(GDK_CROSSHAIR); 
    GPAINT_TOOL(fill)->destroy = fill_destroy;
    GPAINT_TOOL(fill)->button_press = fill_bounded_area;
    return GPAINT_TOOL(fill);
}

static void 
fill_destroy(gpaint_tool *tool)
{
    debug_fn();
    gdk_cursor_destroy(tool->cursor);
    g_free(tool);
}

static void
fill_bounded_area(gpaint_tool* tool, int x, int y)
{
    gpaint_drawing *drawing = tool->drawing;
    GdkGCValues gcvalues;
    const GdkColor *color;
    unsigned char r, g, b;
    struct fillinfo fillinfo;
    unsigned char *p ;
    gpaint_image *img;

    img = drawing_create_image(drawing);
    gdk_gc_get_values(tool->drawing->gc, &gcvalues);
    color = &(gcvalues.foreground); 
    convert_color(color, &r, &g, &b);
   
    fillinfo.rgb = image_pixels(img);       
    fillinfo.width = image_width(img);      
    fillinfo.height = image_height(img);    
    fillinfo.rowstride = image_rowstride(img);
    fillinfo.pixelsize = image_pixelsize(img); 
    fillinfo.r = r;
    fillinfo.g = g;
    fillinfo.b = b;
    p = fillinfo.rgb + y * fillinfo.rowstride + x * fillinfo.pixelsize;
    fillinfo.or = *p;
    fillinfo.og = *(p + 1);
    fillinfo.ob = *(p + 2);
    flood_fill_algo(&fillinfo, x, y);

    image_draw_on_pixmap(img, &(drawing->backing_pixmap), drawing->gc);

    /* TODO: Is it possible to determine the bounding rectangle so we 
             don't have to invalidate the whole drawing? This is wasteful. */
    gtk_widget_queue_draw_area(GTK_WIDGET(tool->canvas->drawing_area), 0, 0, drawing->width, drawing->height);
    image_free(img);

    drawing_modified(drawing);
}

static void
convert_color(const GdkColor *color, unsigned char *r, unsigned char *g, unsigned char *b)
{
    GdkColormap *cmap = gdk_rgb_get_cmap();
    GdkVisual *visual = gdk_rgb_get_visual();
    GdkColor *c;
    switch(visual->type)
    {
        case GDK_VISUAL_STATIC_COLOR:
        case GDK_VISUAL_PSEUDO_COLOR:
            c = cmap->colors + color->pixel;
            *r = c->red >> 8;
            *g = c->green >> 8;
            *b = c->blue >> 8;
            break;

        case GDK_VISUAL_TRUE_COLOR:
        case GDK_VISUAL_DIRECT_COLOR:
            *r = SHIFT_COLOR(red);
            *g = SHIFT_COLOR(green);
            *b = SHIFT_COLOR(blue);
            break;
   
       default:
            g_assert_not_reached();
            break;
    }
}

/*
 * algorithm based on SeedFill.c from GraphicsGems 1
 */
static void
flood_fill_algo(struct fillinfo *info, int x, int y)
{
    /* TODO: check for stack overflow? */
    /* TODO: that's a lot of memory! esp if we never use it */
    struct fillpixelinfo stack[STACKSIZE];
    struct fillpixelinfo * sp = stack;
    int l, x1, x2, dy;
      
    if ((x >= 0) && (x < info->width) && (y >= 0) && (y < info->height))
    {
        if ((info->or == info->r) && (info->og == info->g) && (info->ob == info->b))
        {
            return;
        }
        PUSH(y, x, x, 1);
        PUSH(y + 1, x, x, -1);
        while (sp > stack)  
        {
            POP(y, x1, x2, dy);
            for (x = x1; (x >= 0) && is_old_pixel_value(info, x, y); x--)
            { 
                set_new_pixel_value(info, x, y);
            }
            if (x >= x1)
            {
                goto skip;
            }
            l = x + 1;
            if (l < x1)
            {
                PUSH(y, l, x1 - 1, -dy);
            }
            x = x1 + 1;
            do
            {
                for (; (x < info->width) && is_old_pixel_value(info, x, y); x++)
                {
                    set_new_pixel_value(info, x, y);
                }
                PUSH(y, l, x - 1, dy);
                if (x > x2 + 1)
                {
                    PUSH(y, x2 + 1, x - 1, -dy);
                }
skip:
                for (x++; x <= x2 && !is_old_pixel_value(info, x, y); x++) 
                {
                    /* empty */ ;
                }
                l = x;
            } while (x <= x2);
        }
    }
}  


static int
is_old_pixel_value(struct fillinfo *info, int x, int y)
{
    unsigned char *p = info->rgb + y * info->rowstride + x * info->pixelsize;
    unsigned char or, og, ob;
    or = *p;
    og = *(p + 1);
    ob = *(p + 2);
    if ((or == info->or) && (og == info->og) && (ob == info->ob))
    {
        return 1;
    }
    return 0;
}


static void
set_new_pixel_value(struct fillinfo *info, int x, int y)
{
    unsigned char *p = info->rgb + y * info->rowstride + x * info->pixelsize;
    *p = info->r;
    *(p + 1) = info->g;
    *(p + 2) = info->b;
}
