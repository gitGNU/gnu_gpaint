/* $Id: drawing.h,v 1.5 2005/01/07 02:50:52 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003, 2007  Li-Cheng (Andy) Tai
 *
 * Authors: Li-Cheng (Andy) Tai <atai@gnu.org>
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

#ifndef __DRAWING_H__
#define __DRAWING_H__

#include "image.h"
#include "selection.h"
#include <gtk/gtk.h>


/*
 * Drawing
 */
typedef struct _gpaint_drawing
{
    GtkWidget        *top_level;      /* reference to the top level window */
    GdkDrawable      *window;         /* reference to the drawing area window */
    GdkPixmap        *backing_pixmap; /* stored in X server memory */
    gboolean          modified;      
    GdkGC            *gc;             /* reference to the gc */
    GString          *filename;
    gboolean          untitled;       /* false if the filename has been set by the user */
    gint              width;      
    gint              height;
} gpaint_drawing;


gpaint_drawing* drawing_new_blank(GtkDrawingArea *drawing_area, GdkGC *gc, gint width, gint height);
gpaint_drawing* drawing_new_from_file(GtkDrawingArea *drawing_area, GdkGC *gc, const gchar* filename);
gpaint_drawing* drawing_new_from_desktop(GtkDrawingArea *drawing_area, GdkGC *gc);
void drawing_destroy(gpaint_drawing *drawing);
int drawing_in_bounds(gpaint_drawing *drawing, int x, int y);
int drawing_save(gpaint_drawing *drawing);
int drawing_save_as(gpaint_drawing *drawing, const gchar *filename);
void drawing_modified(gpaint_drawing *drawing);
void drawing_copy_to_desktop(gpaint_drawing *drawing);
gpaint_image *drawing_create_image(gpaint_drawing *drawing);
void drawing_clear(gpaint_drawing *drawing);
void drawing_clear_selection(gpaint_drawing *drawing, gpaint_point_array *points);
gboolean drawing_prompt_to_save(gpaint_drawing *drawing);
void drawing_rotate(gpaint_drawing *drawing, double degrees);

#endif
