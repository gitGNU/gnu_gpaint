
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


#ifndef __PRINT_H__
#define __PRINT_H__

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-pixbuf.h>

#include "image_buf.h"

int do_print(image_buf *);
int do_print_preview(image_buf *ibuf);

int image_buf_print(image_buf *ibuf, GnomePrintContext *pc);


#endif
