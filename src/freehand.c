/* $Id: freehand.c,v 1.3 2004/12/25 04:41:58 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003  Li-Cheng (Andy) Tai
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

#include "freehand.h"
#include "debug.h"

typedef struct _gpaint_closed_freehand
{
    gpaint_tool         tool;
    gpaint_point        last;
    gpaint_point        anchor;
    gpaint_point_array *points;
    gboolean            fill;
} gpaint_closed_freehand;

#define GPAINT_CFH(tool)  ((gpaint_closed_freehand*)(tool))


static void     freehand_destroy(gpaint_tool *tool);
static void     freehand_select(gpaint_tool *tool);
static void     freehand_deselect(gpaint_tool *tool);
static gboolean freehand_attribute(gpaint_tool *tool, gpaint_attribute attribute, gpointer data);
static void     freehand_draw_line(gpaint_tool* tool, int x, int y);
static void     freehand_button_release(gpaint_tool *tool, int x, int y);

static void     freehand_button_press(gpaint_tool* tool, int x, int y);
static void     freehand_button_release(gpaint_tool *tool, int x, int y);
static void     freehand_fill_area(gpaint_drawing *drawing, gpaint_point_array *point);



/*
 * Create closed freehand drawing tool.
 */
gpaint_tool *closed_freehand_create(const char *name)
{
    gpaint_closed_freehand *cfh = g_new0(gpaint_closed_freehand, 1);
    g_assert(cfh);

    GPAINT_TOOL(cfh)->name = name;
    GPAINT_TOOL(cfh)->cursor = gdk_cursor_new(GDK_PENCIL);
    GPAINT_TOOL(cfh)->destroy        = freehand_destroy;
    GPAINT_TOOL(cfh)->select         = freehand_select;
    GPAINT_TOOL(cfh)->deselect       = freehand_deselect;
    GPAINT_TOOL(cfh)->attribute      = freehand_attribute;
    GPAINT_TOOL(cfh)->button_press   = freehand_button_press;
    GPAINT_TOOL(cfh)->motion         = freehand_draw_line;
    GPAINT_TOOL(cfh)->button_release = freehand_button_release;
    
    cfh->points = point_array_new();  
    return GPAINT_TOOL(cfh);
}

static void 
freehand_destroy(gpaint_tool *tool)
{
    debug_fn();
    point_array_delete(GPAINT_CFH(tool)->points);
    gdk_cursor_destroy(tool->cursor);
    g_free(tool);
}

static void
freehand_select(gpaint_tool *tool)
{
    debug_fn();
}

static void
freehand_deselect(gpaint_tool *tool)
{
    gpaint_closed_freehand *pen = GPAINT_CFH(tool);
    debug_fn();
    clear_point(&pen->last);
    point_array_remove_all(pen->points);
}

static gboolean 
freehand_attribute(gpaint_tool *tool, gpaint_attribute attribute, gpointer data)
{
    if (attribute==GpaintFillShape)
    {
        GPAINT_CFH(tool)->fill = *((gboolean*)data);
        return TRUE;
    }
    return FALSE;
}

static void
freehand_draw_line(gpaint_tool* tool, int x, int y)
{
    gpaint_closed_freehand *pen = GPAINT_CFH(tool);
    if (!is_point_defined(&pen->last))
    {
        set_point(&pen->last, x, y);
    }
    gdk_draw_line(tool->drawing->backing_pixmap, tool->drawing->gc,
                  pen->last.x, pen->last.y, x, y);
    gdk_draw_line(tool->drawing->window, tool->drawing->gc,
                  pen->last.x, pen->last.y, x, y);
    drawing_modified(tool->drawing);
    set_point(&pen->last, x, y);
    point_array_append(pen->points, x, y);
}


static void
freehand_button_press(gpaint_tool* tool, int x, int y)
{
    set_point(&(GPAINT_CFH(tool)->anchor), x, y);
    freehand_draw_line(tool, x, y);
}

static void
freehand_button_release(gpaint_tool *tool, int x, int y)
{
    gpaint_closed_freehand *pen = GPAINT_CFH(tool);
    gdk_draw_line(tool->drawing->backing_pixmap, tool->drawing->gc,
                  x, y, pen->anchor.x, pen->anchor.y);
    gdk_draw_line(tool->drawing->window, tool->drawing->gc,
                  x, y, pen->anchor.x, pen->anchor.y);
    drawing_modified(tool->drawing);
    point_array_append(pen->points, pen->anchor.x, pen->anchor.y);
 
    if (pen->fill)
    {
       freehand_fill_area(tool->drawing, pen->points); 
    }

    point_array_remove_all(pen->points);
    clear_point(&pen->anchor);
    clear_point(&pen->last);
}

static void
freehand_fill_area(gpaint_drawing *drawing, gpaint_point_array *points)
{
    if (point_array_size(points) > 2)
    {
        gdk_draw_polygon(
              drawing->backing_pixmap,
              drawing->gc, 
              TRUE,
              point_array_data(points),
              point_array_size(points));
        gdk_draw_polygon(
              drawing->window,
              drawing->gc,
              TRUE,
              point_array_data(points),
              point_array_size(points));
        drawing_modified(drawing);
    } 
}
