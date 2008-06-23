
/*
    Copyright 2000  Li-Cheng (Andy) Tai

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "fileio.h"
#include <stdlib.h>
#include <string.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "image_buf.h"
#include "util.h"


int image_buf_save(image_buf *ibuf, const char *name)
{
   int i;
   int x, y;
   GdkImlibImage *tmp = 0;
   int rowstride = image_buf_rowstride(ibuf), pixelsize = image_buf_pixelsize(ibuf);
   unsigned char *tmpbuf = calloc(3, image_buf_width(ibuf) * image_buf_height(ibuf)), *p, *p2, *d;
 
   g_assert(ibuf);
   g_assert(tmpbuf);
   image_buf_pixmap_to_rgbbuf(ibuf, 0);
   p = image_buf_rgbbuf(ibuf);
   d = tmpbuf;
   for (y = 0; y < image_buf_height(ibuf); y++, p += rowstride)
      for (p2 = p, x = 0; x < image_buf_width(ibuf); x++, p2 += pixelsize, d += 3)
      {
         *d = *p2;
	 *(d + 1) = *(p2 + 1);
	 *(d + 2) = *(p2 + 2);
      }
      
   tmp = gdk_imlib_create_image_from_data((tmpbuf), 0, image_buf_width(ibuf), image_buf_height(ibuf));
   /* cast away const for the library call. */
   i = gdk_imlib_save_image(tmp, (char*)name, 0);
   gdk_imlib_kill_image(tmp);
   free(tmpbuf);
   return i;
}
   
   
int image_buf_load(image_buf *ibuf, const char *name)
{
   
   GdkPixbuf *tmp = 0;
   g_assert(ibuf);
   
   tmp = gdk_pixbuf_new_from_file(name);
   if (tmp)
   {
      GdkGCValues gcvalues; /* used to preserve the GC values acrdss file loading */
      GdkGC *dummy;
      int filled = image_buf_get_fill(ibuf);
      gdk_gc_get_values(ibuf->gc, &gcvalues);
      
      image_buf_resize(ibuf, gdk_pixbuf_get_width(tmp), gdk_pixbuf_get_height(tmp));
      gdk_pixbuf_copy_area(tmp, 0, 0, image_buf_width(ibuf), image_buf_height(ibuf), ibuf->rgbbuf, 0, 0); 
      image_buf_set_name(ibuf, (name));
      gdk_pixbuf_unref(tmp);
      image_buf_rgbbuf_to_pixmap(ibuf, 0);
      dummy = gdk_gc_new_with_values(ibuf->drawing_area->window, &gcvalues, (GDK_GC_FOREGROUND | GDK_GC_BACKGROUND | GDK_GC_LINE_WIDTH | GDK_GC_LINE_STYLE | GDK_GC_CAP_STYLE | GDK_GC_JOIN_STYLE));
      gdk_gc_copy(ibuf->gc, dummy);
      gdk_gc_unref(dummy);
      image_buf_set_fill(ibuf, filled);
      return 1; 
   }
   else 
      return 0;
   
}
