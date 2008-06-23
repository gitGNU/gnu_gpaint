
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


#include "rgb.h"

inline void set_rgb_pixel(unsigned char *rgb,  int rowstride, int pixelsize, int width, int height, int x, int y, unsigned char r, unsigned char g, unsigned char b)
{

   unsigned char *p = rgb + rowstride * y + x * 3;
   if (x >= width) 
      return;
   if (y >= height)
      return;   
   if (x < 0)
      return;
   if (y < 0)
      return;
   *p = r;
   *(p + 1) = g;
   *(p + 2) = b;
}

     
/* Draw a rectangle on the screen */
void draw_brush( image_buf *ibuf,
                        int    x,
                        int    y, 
			int width, 
			int height)
{
  GdkRectangle update_rect;
  
  int tx, ty;
  int iwidth = image_buf_width(ibuf);
  int iheight = image_buf_height(ibuf);
  int rowstride = image_buf_rowstride(ibuf);
  int pixelsize = image_buf_pixelsize(ibuf);
  unsigned char *rgb = image_buf_rgbbuf(ibuf);
  
  update_rect.x = x - width / 2;
  update_rect.y = y - height / 2;
  update_rect.width = width;
  update_rect.height = height;
  
  for (ty = (int) y - height / 2; ty < (int) y + height / 2; ty++)
     for (tx = (int) x - width / 2; tx < (int) x + width / 2; tx++)
        set_rgb_pixel(rgb, rowstride, pixelsize, iwidth, iheight, tx, ty, 255, 255, 0);
	

  gtk_widget_draw (ibuf->drawing_area, &update_rect);
}

