/* $Id: polyselect.c,v 1.2 2004/03/13 03:31:45 meffie Exp $
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

#include "polyselect.h"
#include "selection.h"
#include "debug.h"

typedef struct _gpaint_polygon_select
{
    gpaint_tool       tool;
    gpaint_point      anchor;
    gpaint_point      drag;
    gpaint_selection *selection;
} gpaint_polygon_select;

#define GPAINT_POLYGON_SELECT(tool) ((gpaint_polygon_select*)(tool))

static void polygon_select_destroy(gpaint_tool * tool);
static void polygon_select_select(gpaint_tool * tool);
static void polygon_select_deselect(gpaint_tool * tool);
static void polygon_select_button_press(gpaint_tool * tool, int x, int y);
static void polygon_select_motion(gpaint_tool * tool, int x, int y);
static void polygon_select_button_release(gpaint_tool * tool, int x, int y);

gpaint_tool *
polygon_select_create(const char *name)
{
    gpaint_polygon_select *polygon_select = g_new0(gpaint_polygon_select, 1);
    gpaint_tool *tool = GPAINT_TOOL(polygon_select);

    tool->name = name;
    tool->cursor = gdk_cursor_new(GDK_CROSSHAIR);
    tool->destroy        = polygon_select_destroy;
    tool->select         = polygon_select_select;
    tool->deselect       = polygon_select_deselect;
    tool->button_press   = polygon_select_button_press;
    tool->button_release = polygon_select_button_release;
    tool->motion         = polygon_select_motion;
    return tool; 
}

static void
polygon_select_destroy(gpaint_tool *tool)
{
    debug_fn();
    gdk_cursor_destroy(tool->cursor);
    g_free(tool);
}

static void
polygon_select_select(gpaint_tool *tool)
{
    gpaint_polygon_select *polygon_select = GPAINT_POLYGON_SELECT(tool);
    polygon_select->selection = tool->canvas->selection;
    selection_enable_flash(polygon_select->selection);
}

static void
polygon_select_deselect(gpaint_tool *tool)
{
    gpaint_polygon_select *polygon_select = GPAINT_POLYGON_SELECT(tool);
    selection_remove_points(polygon_select->selection);
}

static void
polygon_select_focus_change(gpaint_tool *tool)
{
    gpaint_polygon_select *polygon_select = GPAINT_POLYGON_SELECT(tool);
    debug_fn();
    if (tool->canvas->has_focus)
    {
        selection_enable_flash(polygon_select->selection);
    }
    else
    {
        selection_disable_flash(polygon_select->selection);
    }
}

static void
polygon_select_button_press(gpaint_tool * tool, int x, int y)
{
    gpaint_polygon_select *ps = GPAINT_POLYGON_SELECT(tool);
    debug_fn();
   
    if (drawing_in_bounds(tool->drawing, x, y))
    {
        if (selection_num_points(ps->selection))
        {
            selection_change_last(ps->selection, x, y); 
        }
        else
        {
            selection_add_point(ps->selection, x, y);
        }
        set_point(&ps->anchor, x, y);
    }
}

static void
polygon_select_motion(gpaint_tool * tool, int x, int y)
{
    gpaint_polygon_select *ps = GPAINT_POLYGON_SELECT(tool);
    debug_fn();
    
    if (drawing_in_bounds(tool->drawing, x, y))
    {
        if (is_point_defined(&ps->anchor))
        {
            if (is_point_defined(&ps->drag))
            {
                selection_change_last(ps->selection, x, y);
                selection_draw_flash(ps->selection);
            } 
            set_point(&ps->drag, x, y);
        }
    }
}

static void
polygon_select_button_release(gpaint_tool * tool, int x, int y)
{
    gpaint_polygon_select *ps = GPAINT_POLYGON_SELECT(tool);
    debug_fn();
    
    if (drawing_in_bounds(tool->drawing, x, y))
    {
        if (is_point_defined(&ps->drag))
        {
            selection_change_last(ps->selection, x, y);
        }
        else
        {
            selection_add_point(ps->selection, x, y);
        }
        selection_close_loop(ps->selection);
        selection_draw_flash(ps->selection);
    }
}
