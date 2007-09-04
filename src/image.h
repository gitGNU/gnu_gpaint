/* $Id: image.h,v 1.3 2004/12/29 02:44:02 meffie Exp $
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

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "selection.h"
#include <gtk/gtk.h>

typedef struct _gpaint_image gpaint_image;

gpaint_image*  image_new(int,int);
gpaint_image*  image_new_readfile(const gchar* filename);
gpaint_image*  image_new_copy(const gpaint_image*);
gpaint_image*  image_new_from_pixmap(GdkPixmap *pixmap, GdkRectangle source);
gpaint_image*  image_new_from_desktop();
void           image_free(gpaint_image*);
int            image_draw_on_pixmap(gpaint_image*, GdkPixmap**, GdkGC*);
int            image_draw_region_on_pixmap(gpaint_image* image, GdkRectangle rect, GdkPixmap** ppixmap, GdkGC *gc);
int            image_render(gpaint_image *image, GdkPixmap *pixmap, gint x, gint y, GdkRectangle rect);
int            image_write(gpaint_image*, const gchar* filename, GError **error);
int            image_print(gpaint_image*);
int            image_width(gpaint_image* image);
int            image_height(gpaint_image* image);
guchar*        image_pixels(gpaint_image* image);
int            image_rowstride(gpaint_image *image);
int            image_pixelsize(gpaint_image *image);
gpaint_image*  image_from_selection(GdkPixmap *pixmap, gpaint_point_array *pa);
int            image_flip_x(gpaint_image *image);
int            image_flip_y(gpaint_image *image);
int            image_rotate(gpaint_image *image, double radians);
GdkPixbuf*     image_pixbuf(gpaint_image *image);
#endif
