/* $Id: image.c,v 1.7 2005/02/01 02:42:50 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003, 2007  Li-Cheng (Andy) Tai
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#include "image.h"
#include "debug.h"
#include <math.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>  /* for gdk_root_parent */
#include <gdk-pixbuf/gdk-pixbuf.h>


static int cmp_int(const void *a, const void *b);
static void fill_polygon(
        unsigned char *data, 
        int pixelsize,
        int rowstride, 
        unsigned char value[4], 
        unsigned char mask[4], 
        GdkPoint *pts, 
        int num_pts,
        int xoffset,
        int yoffset);

const int BITS_PER_SAMPLE = 8;

struct _gpaint_image
{
    GdkPixbuf *pixbuf;    /* X client side image for processing */ 
};


typedef struct {
  int rowstride;
  int pixelsize;
  unsigned char value[4];
  unsigned char mask[4];
  unsigned char * data;
} GdkSpanFillData;


gpaint_image*
image_new(int width, int height)
{
    gpaint_image *image = g_new(gpaint_image, 1);
    g_assert(image);
    image->pixbuf = gdk_pixbuf_new(
            GDK_COLORSPACE_RGB, TRUE, BITS_PER_SAMPLE, width, height);
    g_assert(image->pixbuf);
    return image;
}

gpaint_image*
image_new_copy(const gpaint_image *source)
{
    gpaint_image *image = g_new(gpaint_image, 1);
    g_assert(image);
    g_assert(source);
    image->pixbuf = gdk_pixbuf_copy(source->pixbuf);
    return image;
}

gpaint_image* 
image_new_from_pixmap(GdkPixmap* pixmap, GdkRectangle source)
{
    gpaint_image *image = image_new(source.width, source.height);
    gdk_pixbuf_get_from_drawable(
                image->pixbuf,
                pixmap,
                gdk_rgb_get_cmap(), 
                source.x, source.y,
                0, 0, /* destination x, y */ 
                source.width, source.height);
    g_assert(image->pixbuf);
    return image;
}

gpaint_image*
image_new_from_desktop()
{
    gpaint_image *image; 
    GdkWindow *root = gdk_window_lookup(gdk_x11_get_default_root_xwindow());
    int width, height;
    
    gdk_window_get_size(root, &width, &height);
    image = image_new(width, height);
    gdk_pixbuf_get_from_drawable(image->pixbuf, root, 0, 0, 0, 0, 0, width, height);
    g_assert(image->pixbuf);
    return image;
}

void
image_free(gpaint_image* image)
{
    g_assert(image);    
    gdk_pixbuf_unref(image->pixbuf);
    g_free(image);
}

int
image_draw_on_pixmap(gpaint_image* image, GdkPixmap** ppixmap, GdkGC *gc)
{
    int width;
    int height;

    g_assert(image && image->pixbuf);
    g_assert(ppixmap && &ppixmap);
    g_assert(gc);
    
    width = image_width(image);
    height = image_height(image);
    gdk_pixbuf_render_to_drawable(
            image->pixbuf,
            *ppixmap,
            gc,
            0, 0,
            0, 0, width, height,
            GDK_RGB_DITHER_NORMAL, 0, 0);

    return 0; 
}

int
image_draw_region_on_pixmap(
        gpaint_image* image,
        GdkRectangle rect,
        GdkPixmap** ppixmap,
        GdkGC *gc)
{
    g_assert(image && image->pixbuf);
    g_assert(ppixmap && &ppixmap);
    g_assert(gc);
    gdk_pixbuf_render_to_drawable(
            image->pixbuf,
            *ppixmap,
            gc,
            rect.x, rect.y,
            rect.x, rect.y, rect.width, rect.height,
            GDK_RGB_DITHER_NORMAL, 0, 0);
    return 0; 
}

int
image_render(gpaint_image *image, GdkPixmap *pixmap, gint x, gint y, GdkRectangle rect)
{
    gdk_pixbuf_render_to_drawable_alpha( 
           image->pixbuf, pixmap,
           x, y,
           rect.x, rect.y, rect.width, rect.height,
           GDK_PIXBUF_ALPHA_BILEVEL,
           255 / 2,
           GDK_RGB_DITHER_MAX, 0, 0);
    return 0;
}

gpaint_image*
image_new_readfile(const gchar* filename)
{
    GdkPixbuf *pixbuf = NULL;
    gpaint_image *image = NULL;
    
    pixbuf = gdk_pixbuf_new_from_file(filename,0);
    if (pixbuf)
    {
        image = g_new(gpaint_image, 1);
        image->pixbuf = gdk_pixbuf_new_from_file(filename,0);
    }
    return image;
}

int
image_write(gpaint_image* image, const gchar* filename, GError **perror)
{
    gboolean saved = FALSE;
    gchar *ext = NULL;
    gchar *type = NULL;
    
    ext = g_strrstr(filename, ".");
    if (!ext)
    {
       ext = ".png";  /* default is PNG */
    }
    debug1("extension=[%s]", ext);

    if (!g_strcasecmp(ext,".jpg") || !g_strcasecmp(ext,".jpeg"))
    {
        debug("saving jpeg");
        saved = gdk_pixbuf_save(image->pixbuf, filename, "jpeg", perror, "quality", "100", NULL);
    }
    else
    {
        type = g_ascii_strdown(ext+1,-1);
        debug1("type=[%s]",type);
        saved = gdk_pixbuf_save(image->pixbuf, filename, type, perror, NULL);
        g_free(type);
    }
    if (!saved && *perror)
    {
        g_warning("Could not save image %s: %s\n", filename, (*perror)->message);
    }      
    return saved;
}

int
image_print(gpaint_image* image)
{
    return 0; 
}

int
image_width(gpaint_image* image)
{
    return gdk_pixbuf_get_width(image->pixbuf);
}

int
image_height(gpaint_image* image)
{
    return gdk_pixbuf_get_height(image->pixbuf);
}

guchar
*image_pixels(gpaint_image* image)
{
    return gdk_pixbuf_get_pixels(image->pixbuf);
}

int
image_rowstride(gpaint_image *image)
{
   return gdk_pixbuf_get_rowstride(image->pixbuf);
}

int
image_pixelsize(gpaint_image *image)
{
   return gdk_pixbuf_get_n_channels(image->pixbuf) 
          * gdk_pixbuf_get_bits_per_sample(image->pixbuf) / BITS_PER_SAMPLE;
}


gpaint_image *
image_from_selection(GdkPixmap *pixmap, gpaint_point_array *pa)
{
    gpaint_image *image = g_new0(gpaint_image, 1);
    GdkPixbuf *pixbuf = 0;
    GdkRectangle pixmap_rect;
    GdkRectangle sel_rect;
    GdkRectangle rect;
    unsigned char alphaonly[4] = {0, 0, 0, 1};
    unsigned char *data = 0, *c, *d;
    unsigned char default_pixel[4] = {0, 0, 0, 255};
    int width, height; 
    int x, y;
    int pixel_size;
    GdkPixbuf *temp_pixbuf;
    GdkPoint *pts = point_array_data(pa);
    int num_pts   = point_array_size(pa);
    const int BITS_PER_SAMPLE = 8;
   
    point_array_bounding_rectangle(pa, &sel_rect);
    gdk_window_get_size((GdkWindow*)pixmap, &width, &height);
    pixmap_rect.x = 0;
    pixmap_rect.y = 0;
    pixmap_rect.width = width;
    pixmap_rect.height = height;

    if (!gdk_rectangle_intersect(&pixmap_rect, &sel_rect, &rect))
    {
        g_warning("selection does not intersect the drawing");
        return 0;
    }
    debug1("rect.x=%d", rect.x);
    debug1("rect.y=%d", rect.y);
    debug1("rect.width=%d", rect.width);
    debug1("rect.height=%d", rect.height);
    
    temp_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, BITS_PER_SAMPLE, rect.width, rect.height);   
    gdk_pixbuf_get_from_drawable(temp_pixbuf, 
           pixmap, 
           gdk_rgb_get_cmap(), 
           rect.x, rect.y, 0, 0, rect.width, rect.height);

    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, rect.width, rect.height);
    g_assert(pixbuf);
    pixel_size = gdk_pixbuf_get_n_channels(pixbuf) * gdk_pixbuf_get_bits_per_sample(pixbuf) / 8;
   
    gdk_pixbuf_copy_area(temp_pixbuf, 0, 0, rect.width, rect.height, pixbuf, 0, 0);
    gdk_pixbuf_unref(temp_pixbuf);
   
    data = gdk_pixbuf_get_pixels(pixbuf);
    for (y = 0, c = data; y < rect.height; y++, c += gdk_pixbuf_get_rowstride(pixbuf))
    {
        d = c;
        for (x = 0; x < rect.width; x++, d += pixel_size)
        {
            *(d + 3) = 0;/* default everything is transparent */
        }
    }
   
    fill_polygon(data, 
                pixel_size,
                gdk_pixbuf_get_rowstride(pixbuf),
                default_pixel,
                alphaonly,
                pts,
                num_pts, 
                rect.x, rect.y);

    debug("fill_polygon() done");
    image->pixbuf = pixbuf;
    return image;
}

/* taken from gd.c in gd 1.8.3 */
static int
cmp_int(const void *a, const void *b)
{
	return (*(const int *)a) - (*(const int *)b);
}



static void gdk_span_fill(GdkSpan *span, gpointer data) {
  GdkSpanFillData *fill_data = (GdkSpanFillData *) data;
  unsigned char *c = fill_data->data + (fill_data->rowstride * span->y) + (fill_data->pixelsize * span->x);
  int i;
  int j;

  for (i = 0; i <span->width; ++i) {
    for (j = 0; j < fill_data->pixelsize; ++j, ++c) {
      if (fill_data->mask[j]) {
        *c = fill_data->value[j];
      }
    }
  }
}


static void
fill_polygon(
        unsigned char *data,
        int pixelsize,
        int rowstride,
        unsigned char value[4], 
        unsigned char mask[4],
        GdkPoint *pts, 
        int num_pts, 
        int xoffset,
        int yoffset)
{
   /* this assumes the data is in RGBA format, each channel takes 1 bytes (thus 4 bytes per pixel) */

   int i;
   GdkRegion *region;
   GdkRectangle bbox;
   GdkSpan *spans;
   GdkSpanFillData fill_data;

   g_assert(xoffset >= 0);
   g_assert(yoffset >= 0);

   debug_fn();

   if (!num_pts)
     return;

   region  = gdk_region_polygon(pts, num_pts, GDK_EVEN_ODD_RULE);

   fill_data.rowstride = rowstride;
   fill_data.pixelsize = pixelsize;
   fill_data.data = data;
   for (i = 0; i < 4; ++i) {
     fill_data.value[i] = value[i];
     fill_data.mask[i] = mask[i];
   }

   gdk_region_offset(region, -xoffset, -yoffset);

   gdk_region_get_clipbox(region, &bbox);

   spans = (GdkSpan *) g_new0(GdkSpan, bbox.height);
   for(i = 0; i < bbox.height; ++i) {
     spans[i].x = bbox.x;
     spans[i].width = bbox.width;
     spans[i].y = bbox.y + i;
   }
   
   gdk_region_spans_intersect_foreach(region, spans, bbox.height, TRUE, gdk_span_fill, &fill_data);

   g_free(spans);
   gdk_region_destroy(region);

}

int
image_flip_x(gpaint_image *image)
{
    int width, height;
    GdkPixbuf *newpixbuf = gdk_pixbuf_new(
                            GDK_COLORSPACE_RGB,
                            TRUE,
                            gdk_pixbuf_get_bits_per_sample(image->pixbuf),
                            width = gdk_pixbuf_get_width(image->pixbuf),
                            height = gdk_pixbuf_get_height(image->pixbuf));
    int rowstride = gdk_pixbuf_get_rowstride(newpixbuf);
    int x, y;
    int j;
    unsigned char *p, *p2, *p3, *p4;
    int pixel_size = gdk_pixbuf_get_bits_per_sample(image->pixbuf) *
                        gdk_pixbuf_get_n_channels(image->pixbuf) / 8;

    g_assert(newpixbuf);
    for (p = gdk_pixbuf_get_pixels(newpixbuf), p2 = gdk_pixbuf_get_pixels(image->pixbuf), y = 0;
         y < height;
         y++, p += rowstride, p2 += rowstride)
    {
        p4 = p2 + pixel_size * (width - 1);
        p3 = p;
        for (x = 0; x < width; x++)
        {
	        for (j = 0; j < pixel_size; j++)
            {
	            *(p3 + j) = *(p4 + j);
            }
            p3 += pixel_size;
	        p4 -= pixel_size;
      }
    }
    gdk_pixbuf_unref(image->pixbuf); 
    image->pixbuf = newpixbuf;     
    return 0;
}

int
image_flip_y(gpaint_image *image)
{
    int width, height;
    GdkPixbuf *newpixbuf = gdk_pixbuf_new(
                            GDK_COLORSPACE_RGB,
                            TRUE,
                            gdk_pixbuf_get_bits_per_sample(image->pixbuf), 
                            width = gdk_pixbuf_get_width(image->pixbuf),
                            height = gdk_pixbuf_get_height(image->pixbuf));
   
    int rowstride = gdk_pixbuf_get_rowstride(newpixbuf);
    int x, y;
    int j;
    unsigned char *p, *p2, *p3, *p4;
    int pixel_size = gdk_pixbuf_get_bits_per_sample(image->pixbuf) *
                        gdk_pixbuf_get_n_channels(image->pixbuf) / 8;

    g_assert(newpixbuf);
    for (p = gdk_pixbuf_get_pixels(newpixbuf),
            p2 = gdk_pixbuf_get_pixels(image->pixbuf) + rowstride * (height - 1), y = 0;
         y < height;
         y++, p += rowstride, p2 -= rowstride)
    {
        p3 = p;
        p4 = p2;
        for (x = 0; x < width; x++)
        {
	        for (j = 0; j < pixel_size; j++)
            {
	            *(p3 + j) = *(p4 + j);
            }
            p3 += pixel_size;
            p4 += pixel_size;
        } 
    }
    gdk_pixbuf_unref(image->pixbuf); 
    image->pixbuf = newpixbuf;     
    return 0;
}


/* the below 2d transform routine contains code 
   taken from xpaint, with the following copyright notice */
/* +-------------------------------------------------------------------+ */
/* | Copyright 1992, 1993, David Koblas (koblas@netcom.com)            | */
/* | Copyright 1995, 1996 Torsten Martinsen (bullestock@dk-online.dk)  | */
/* |                                                                   | */
/* | Permission to use, copy, modify, and to distribute this software  | */
/* | and its documentation for any purpose is hereby granted without   | */
/* | fee, provided that the above copyright notice appear in all       | */
/* | copies and that both that copyright notice and this permission    | */
/* | notice appear in supporting documentation.  There is no           | */
/* | representations about the suitability of this software for        | */
/* | any purpose.  this software is provided "as is" without express   | */
/* | or implied warranty.                                              | */
/* |                                                                   | */
/* +-------------------------------------------------------------------+ */


#define XFORM(x,y,mat,nx,ny)	nx = mat[0][0] * x + mat[0][1] * y; \
				ny = mat[1][0] * x + mat[1][1] * y
#define COPY_MAT(s,d)	d[0][0] = s[0][0]; d[0][1] = s[0][1]; \
			d[1][0] = s[1][0]; d[1][1] = s[1][1]

#define INVERT_MAT(mat, inv) do {			\
		float _d = 1.0 / (mat[0][0] * mat[1][1] \
			      - mat[0][1] * mat[1][0]);	\
		(inv)[0][0] =  (mat)[1][1] * _d;	\
		(inv)[1][1] =  (mat)[0][0] * _d;	\
		(inv)[0][1] = -(mat)[0][1] * _d;	\
		(inv)[1][0] = -(mat)[1][0] * _d;	\
	} while (0)


#define ROUND(x) ((int)(x + 0.5))

int
image_rotate(gpaint_image *image, double degrees)
{
    double matrix[2][2];
    GdkPixbuf *newpixbuf;
    double invmatrix[2][2];
    int x, y, width, height, i, j;
    int rowstride1, rowstride2, pixel_size;
    unsigned char *p1, *p2, *p3, *p4;
    double tx[4], ty[4], minx, miny, maxx, maxy, nx, ny, width2, height2;
    double radians;

    radians = degrees * M_PI / 180;
    matrix[0][0] = cos(radians);
    matrix[0][1] = -sin(radians);
    matrix[1][0] = sin(radians);
    matrix[1][1] = cos(radians);
   
    /* find the new bounding box */
    width = gdk_pixbuf_get_width(image->pixbuf);
    height = gdk_pixbuf_get_height(image->pixbuf);

    if (width < height)
    {
        g_warning("TODO: cannot rotate if width is less than height.");
        return 1;
    }


    XFORM(-width / 2, -height / 2, matrix, tx[0], ty[0]);
    XFORM(-width / 2, height / 2, matrix, tx[1], ty[1]);
    XFORM(width / 2, -height / 2, matrix, tx[2], ty[2]);
    XFORM(width / 2, height / 2, matrix, tx[3], ty[3]);
    minx = INT_MAX;
    miny = INT_MAX;
    maxx = -INT_MAX;
    maxy = -INT_MAX;
    for (i = 0; i < 4; i++)
    {
        if (minx > tx[i])
            minx = tx[i];
        if (maxx < tx[i])
            maxx = tx[i];
        if (miny > ty[i])
            miny = ty[i];
        if (maxy < ty[i])
            maxy = ty[i];
    }
    width2 = maxx - minx;
    height2 = maxy - miny;
   
   
    INVERT_MAT(matrix, invmatrix);
   
    p1 = gdk_pixbuf_get_pixels(image->pixbuf);
    newpixbuf = gdk_pixbuf_new(
           GDK_COLORSPACE_RGB,
           TRUE, gdk_pixbuf_get_bits_per_sample(image->pixbuf),
           ROUND(width2), ROUND(height2));
    g_assert(newpixbuf);
    p2 = gdk_pixbuf_get_pixels(newpixbuf);
    rowstride1 = gdk_pixbuf_get_rowstride(image->pixbuf);
    rowstride2 = gdk_pixbuf_get_rowstride(newpixbuf);
    pixel_size = gdk_pixbuf_get_bits_per_sample(image->pixbuf) *
                  gdk_pixbuf_get_n_channels(image->pixbuf) / 8;
   
#define PIXEL(x, y) (p1 + ROUND(y) * rowstride1 + ROUND(x) * pixel_size)
   
    for (y = miny; y < maxy; y++, p2 += rowstride2)
    {
        p4 = p2;
        for (x = minx; x < maxx; x++, p4 += pixel_size)
        {
            XFORM(x, y, invmatrix, nx, ny);
            if (nx<0) nx = 0.0;
            if (ny<0) ny = 0.0;
	        nx += width / 2.0;
	        ny += height / 2.0;
   
            g_return_val_if_fail(nx >= 0, 1);
            g_return_val_if_fail(ny >= 0, 1);
            g_return_val_if_fail(nx <= gdk_pixbuf_get_width(image->pixbuf), 1);
            g_return_val_if_fail(ny <= gdk_pixbuf_get_height(image->pixbuf), 1);
            
            p3 = PIXEL(nx, ny);
	        for (j = 0; j < pixel_size; j++)
            {
	            *(p4 + j) = *(p3 + j);
            }
        }
    }
    gdk_pixbuf_unref(image->pixbuf);      
    image->pixbuf = newpixbuf;
    return 0;
}


GdkPixbuf* image_pixbuf(gpaint_image* image) {
    return image->pixbuf;
};
