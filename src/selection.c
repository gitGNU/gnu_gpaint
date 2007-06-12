/* $Id: selection.c,v 1.3 2004/11/22 02:59:53 meffie Exp $
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_STRING_H
#  include <string.h>
#endif

#include "selection.h"
#include "debug.h"

#define FLASH_RATE  500 /* milliseconds */

static int selection_on_tick(gpaint_selection *s);


struct _gpaint_point_array
{
    GArray *array;
};


gpaint_point_array *
point_array_new()
{
    gpaint_point_array *pa = g_new0(gpaint_point_array,1);
    pa->array = g_array_new(FALSE, FALSE, sizeof(GdkPoint)); 
    return pa;
}

void
point_array_delete(gpaint_point_array *pa)
{
    g_array_free(pa->array, TRUE);
    g_free(pa); 
}

int
point_array_size(gpaint_point_array *pa)
{
    return pa->array->len;
}

GdkPoint*
point_array_data(gpaint_point_array *pa)
{
    return (GdkPoint*)(pa->array->data);
}

void
point_array_set(gpaint_point_array *pa, int index, int x, int y)
{
    g_assert(index>=0 && index<pa->array->len);
    g_array_index(pa->array, GdkPoint, index).x = x;
    g_array_index(pa->array, GdkPoint, index).y = y;
}

void
point_array_append(gpaint_point_array *pa, int x, int y)
{
    GdkPoint point;
    point.x = x;
    point.y = y;
    g_array_append_val(pa->array, point);
}

void
point_array_remove_all(gpaint_point_array *pa)
{
    g_array_set_size(pa->array, 0);
}

void
point_array_copy(gpaint_point_array *dst, gpaint_point_array *src)
{
    g_array_set_size(dst->array, src->array->len);
    memcpy(dst->array->data, src->array->data, sizeof(GdkPoint) * dst->array->len);
}

void
point_array_bounding_rectangle(gpaint_point_array *pa, GdkRectangle *rp)
{
   int i;
   int x = INT_MAX, y = INT_MAX, x2 = INT_MIN, y2 = INT_MIN;
   GdkPoint* pts = point_array_data(pa);

   if (pa->array->len==0)
   {
       rp->x = 0;
       rp->y = 0;
       rp->width = 0;
       rp->height = 0;
       return;
   }
   
   for (i = 0; i < pa->array->len; i++)
   {
      if (x > pts[i].x)
      {
         x = pts[i].x;
      }
      if (y > pts[i].y)
      {
         y = pts[i].y;
      }
      if (x2 < pts[i].x)
      {
         x2 = pts[i].x;
      }
      if (y2 < pts[i].y)
      {
         y2 = pts[i].y;
      }
   }
   rp->x = x;
   rp->y = y;
   rp->width  = x2 - x + 1;
   rp->height = y2 - y + 1;
}



/* ---------------------------------------------------------------------------- */

struct _gpaint_selection
{
    gpaint_point_array *points;
    GtkDrawingArea  *drawing_area_ref;
    GdkGC           *gc;
    gboolean         flash_enabled;
    gint             flash_state;
    gint             timer;
};

gpaint_selection *
selection_new(GtkDrawingArea *drawing_area)
{
    GdkDrawable *d = GTK_WIDGET(drawing_area)->window;
    GdkColor black; 
    GdkColor white; 
    
    gpaint_selection *selection = (gpaint_selection*)g_new0(gpaint_selection, 1);
    selection->points = point_array_new();
    selection->drawing_area_ref = drawing_area;
    selection->timer = gtk_timeout_add(FLASH_RATE, (GtkFunction)(selection_on_tick), selection);

    /* Create the gc from the drawing area window. */
    selection->gc = gdk_gc_new(d);
    g_assert(selection->gc);
    gdk_color_black(gdk_colormap_get_system(), &black);
    gdk_color_white(gdk_colormap_get_system(), &white);
    gdk_gc_set_foreground(selection->gc, &black);
    gdk_gc_set_background(selection->gc, &white);
    gdk_gc_set_function(selection->gc, GDK_INVERT);
    gdk_gc_set_line_attributes(selection->gc, 1,
                               GDK_LINE_ON_OFF_DASH,
                               GDK_CAP_ROUND,
                               GDK_JOIN_ROUND);
    return selection;
}

void
selection_delete(gpaint_selection *s)
{
    g_assert(s->timer);
    gtk_timeout_remove(s->timer);
    gdk_gc_unref(s->gc);
    s->timer = 0;
    selection_clear_flash(s);
    g_free(s);
}

void 
selection_size(gpaint_selection *selection, int width, int height)
{
    GdkRectangle rect;
    rect.x = 0;
    rect.y = 0;
    rect.width = width;
    rect.height = height;
    gdk_gc_set_clip_origin(selection->gc, 0, 0);
    gdk_gc_set_clip_rectangle(selection->gc, &rect);
}

gpaint_point_array*
selection_points(gpaint_selection *s)
{
    return s->points;
}

GdkPoint*
selection_data(gpaint_selection *s)
{
    return point_array_data(s->points);
}

int
selection_num_points(gpaint_selection *s)
{
    return point_array_size(s->points);
}

void
selection_enable_flash(gpaint_selection *selection)
{
    selection->flash_enabled = TRUE;
}

void
selection_disable_flash(gpaint_selection *selection)
{
    selection_clear_flash(selection);
    selection->flash_enabled = FALSE;
}

void
selection_set_rectangle(gpaint_selection *selection, GdkRectangle rect)
{
    gboolean flash = selection->flash_enabled;
    if (flash)
    {
        selection_disable_flash(selection);
    }
 
    point_array_remove_all(selection->points);
    point_array_append(selection->points, rect.x, rect.y);
    point_array_append(selection->points, rect.x + rect.width, rect.y);
    point_array_append(selection->points, rect.x + rect.width, rect.y + rect.height);
    point_array_append(selection->points, rect.x, rect.y + rect.height);
    point_array_append(selection->points, rect.x, rect.y);

    if (flash)
    {
        selection_enable_flash(selection);
    }
}

void
selection_set_points(gpaint_selection *selection, gpaint_point_array *points)
{
    gboolean flash = selection->flash_enabled;
    
    if (flash)
    {
        selection_disable_flash(selection);
    }
    point_array_copy(selection->points, points);
    if (flash)
    {
        selection_enable_flash(selection);
    }
}

void
selection_add_point(gpaint_selection *selection, int x, int y)
{
    if (x < 0)
    {
        x = 0;
    }
    if (y < 0)
    {
        y = 0;
    }

    selection_clear_flash(selection);
    point_array_append(selection->points, x, y);
}

void
selection_change_last(gpaint_selection *selection, int x, int y)
{
    int size = point_array_size(selection->points);
    if (size)
    {
        int i = size - 1;
        selection_clear_flash(selection);
        point_array_set(selection->points, i, x, y);
    }
}

void 
selection_remove_points(gpaint_selection *selection)
{
    selection_clear_flash(selection);
    point_array_remove_all(selection->points);
}

void
selection_close_loop(gpaint_selection *selection)
{
    if (point_array_size(selection->points))
    {
        GdkPoint *p = point_array_data(selection->points);
        int x0 = p[0].x;
        int y0 = p[0].y;
        selection_add_point(selection, x0, y0);
    }
}

void 
selection_move(gpaint_selection *selection, int x, int y)
{
    GdkRectangle rect;
    int dx, dy;
    int i;
    gboolean flashing = selection->flash_enabled;
    GdkPoint *point = point_array_data(selection->points);
    int num_points = point_array_size(selection->points);

    if (num_points)
    {
        if (flashing)
        {
            selection_disable_flash(selection);
        }
        point_array_bounding_rectangle(selection->points, &rect);
        dx = x - rect.x;
        dy = y - rect.y;
        for (i = 0; i < num_points; i++)
        {
            point[i].x += dx;
            point[i].y += dy;
        }
        if (flashing)
        {
            selection_enable_flash(selection);
        }
    }
}

static int
selection_on_tick(gpaint_selection *selection)
{
    if (selection->flash_enabled)
    {
        selection_draw_flash(selection);
    }
    return 1;
}

void
selection_clear_flash(gpaint_selection *selection)
{
    debug_fn();
    while (selection->flash_state)
    {
        selection_draw_flash(selection);
    }
}

void
selection_draw_flash(gpaint_selection *selection)
{
    gchar dash_length[2] = {5, 5};

    if (point_array_size(selection->points))
    {
    	gdk_gc_set_dashes(selection->gc,
                      selection->flash_state / 2,
                      dash_length,
                      sizeof(dash_length));
    
    	gdk_draw_lines(GTK_WIDGET(selection->drawing_area_ref)->window,
                   selection->gc, 
                   point_array_data(selection->points),
                   point_array_size(selection->points));

    	selection->flash_state++;
    	selection->flash_state %= 4;
    }
}

