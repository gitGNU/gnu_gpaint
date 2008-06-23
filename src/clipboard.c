
/*
    Copyright 2000  Li-Cheng (Andy) Tai (atai@gnu.org)

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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "util.h"

#include "clipboard.h"

#include "image_buf.h"

GdkPixbuf *clipboard = 0;
GdkPoint clipboard_pts[MAX_NUM_POINTS]; /* a temporary array for storing points */
int clipboard_num_pts = 0;

static void get_image_to_clipboard(image_buf *ibuf)
{
   int i;
   GdkRectangle rect;
   assert((ibuf->current_tool == LASSO) || (ibuf->current_tool == POLSELECT));
   image_buf_get_selected_region_to_pixbuf(ibuf, &clipboard);
   memcpy(clipboard_pts, ibuf->pts, sizeof(GdkPoint) * ibuf->num_pts);
   clipboard_num_pts = ibuf->num_pts;
   rect = compute_cover_rect(clipboard_pts, clipboard_num_pts);
   for (i = 0; i < clipboard_num_pts; i++)
   {
      clipboard_pts[i].x -= rect.x;
      clipboard_pts[i].y -= rect.y;
   }
}   
     

void image_buf_cut(image_buf *ibuf)
{
   g_assert(ibuf);
   if (ibuf->current_tool==LASSO || ibuf->current_tool==POLSELECT)
   {
      image_buf_copy(ibuf);
      image_buf_delete(ibuf);  
   }
}

void image_buf_copy(image_buf *ibuf)
{
   g_assert(ibuf);
   if (ibuf->current_tool==LASSO || ibuf->current_tool==POLSELECT)
   {
      image_buf_clear_flash(ibuf);
      get_image_to_clipboard(ibuf);
   }
}

void image_buf_delete(image_buf *ibuf)
{
   GdkGCValues gcvalues;
   GdkRectangle clientrect ;
   g_assert(ibuf);

   if (ibuf->current_tool == LASSO || ibuf->current_tool==POLSELECT)
   {
      clientrect.x = clientrect.y = 0;
      clientrect.width = image_buf_width(ibuf);
      clientrect.height = image_buf_height(ibuf);
   
      gdk_gc_get_values(ibuf->gc, &gcvalues);
      gdk_gc_set_clip_origin(ibuf->gc, 0, 0);
      gdk_gc_set_clip_rectangle(ibuf->gc, &clientrect);
      image_buf_clear_flash(ibuf);
   
      gdk_gc_set_foreground(ibuf->gc, &(gcvalues.background));
   
      gdk_draw_polygon(ibuf->pixmap, ibuf->gc, TRUE, ibuf->pts, ibuf->num_pts);
      gdk_draw_polygon(ibuf->drawing_area->window, ibuf->gc, TRUE, ibuf->pts, ibuf->num_pts);
   
      image_buf_pixmap_to_rgbbuf(ibuf, 0);

      gdk_gc_set_foreground(ibuf->gc, &(gcvalues.foreground));
   } 
}


void image_buf_paste(image_buf *ibuf)
{
   g_assert(ibuf);
  
   if (clipboard_num_pts)
   { 
      image_buf_clear_flash(ibuf);

      image_buf_enter_paste_mode(ibuf);
      memcpy(ibuf->pts, clipboard_pts, sizeof(GdkPoint) * clipboard_num_pts);
      ibuf->num_pts = clipboard_num_pts;
      image_buf_copy_clipboard_to_region_buf(ibuf);
      image_buf_put_region_image(ibuf, 0, 0);   
   }
}


         
void image_buf_copy_clipboard_to_region_buf(image_buf *ibuf) /* put the clipboard into the selected region buf*/
{  /* assuming the clipboard's boundary is already the region boundary of ibuf!!! */ /* we shall be in PASTE mode */
   assert(ibuf->num_pts);
   assert(ibuf->current_tool == PASTE);
   
   if (ibuf->regionbuf)
      gdk_pixbuf_unref(ibuf->regionbuf);
      
   assert(clipboard);
   ibuf->regionbuf = gdk_pixbuf_copy(clipboard);
}
