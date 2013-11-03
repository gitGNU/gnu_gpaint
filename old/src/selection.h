/* $Id: selection.h,v 1.2 2004/03/13 03:32:27 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003, 2007  Li-Cheng (Andy) Tai
 *
 * Authors: Li-Cheng (Andy) Tai <atai@gnu.org>
 *          Michael A. Meffie III <meffiem@neo.rr.com>
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

#ifndef __SELECTION_H__
#define __SELECTION_H__

#include <gtk/gtk.h>

typedef struct _gpaint_point_array gpaint_point_array;
typedef struct _gpaint_selection gpaint_selection;
typedef struct _gpaint_point     gpaint_point;


/* Simple point structure. Like a GdkPoint, but with an extra 
 * field to signify if the point has been defined. */
struct _gpaint_point
{
	gboolean defined;
	gint x;
	gint y;
};

inline static void set_point(gpaint_point *p, gint x, gint y)
{
    p->defined = TRUE;
    p->x = x;
    p->y = y;
}

inline static void clear_point(gpaint_point *p)
{
    p->defined = FALSE;
}

inline static gboolean is_point_defined(gpaint_point *p)
{
    return p->defined;
}


gpaint_point_array * point_array_new();
void point_array_delete(gpaint_point_array *pa);
int  point_array_size(gpaint_point_array *pa);
GdkPoint* point_array_data(gpaint_point_array *pa);
void point_array_set(gpaint_point_array *pa, int index, int x, int y);
void point_array_append(gpaint_point_array *pa, int x, int y);
void point_array_remove_all(gpaint_point_array *pa);
void point_array_copy(gpaint_point_array *dst, gpaint_point_array *src);
void point_array_bounding_rectangle(gpaint_point_array *pa, GdkRectangle *rp);


gpaint_selection * selection_new(GtkDrawingArea *drawing_area);
void selection_delete(gpaint_selection *selection);
void selection_size(gpaint_selection *selection, int width, int height);
gpaint_point_array* selection_points(gpaint_selection *selection);
int  selection_num_points(gpaint_selection *selection);
GdkPoint* selection_data(gpaint_selection *selection);
void selection_set_rectangle(gpaint_selection *selection, GdkRectangle rect);
void selection_set_points(gpaint_selection *selection, gpaint_point_array *points);
void selection_add_point(gpaint_selection *selection, int x, int y);
void selection_change_last(gpaint_selection *selection, int x, int y);
void selection_remove_points(gpaint_selection *selection);
void selection_close_loop(gpaint_selection *selection);
void selection_move(gpaint_selection *selection, int x, int y);
void selection_clear_flash(gpaint_selection *selection);
void selection_draw_flash(gpaint_selection *selection);
void selection_enable_flash(gpaint_selection *selection);
void selection_disable_flash(gpaint_selection *selection);

#endif
