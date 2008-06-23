
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

#ifndef __CLIPBOARD_H__
#define __CLIPBOARD_H__

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "image_buf.h"

void image_buf_cut(image_buf *ibuf);

void image_buf_copy(image_buf *ibuf);

void image_buf_delete(image_buf *ibuf);

void image_buf_paste(image_buf *ibuf);

extern GdkPixbuf *clipboard; 

void image_buf_copy_clipboard_to_region_buf(image_buf *ibuf); /* put the clipboard into the selected region buf*/



#endif
