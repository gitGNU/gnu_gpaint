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

#include "rectselect.h"
#include "selection.h"
#include "debug.h"

#define min(x,y) (x)<(y)?(x):(y)
#define max(x,y) (x)<(y)?(y):(x)

typedef struct _gpaint_rectangle_select
{
    gpaint_tool       tool;
    gpaint_point      v1;
    gpaint_point      v2;
    gpaint_selection *selection;
} gpaint_rectangle_select;

#define GPAINT_RECTANGLE_SELECT(tool) ((gpaint_rectangle_select*)(tool))

static void rectangle_select_destroy(gpaint_tool * tool);
static void rectangle_select_select(gpaint_tool * tool);
static void rectangle_select_deselect(gpaint_tool * tool);
static void rectangle_select_button_press(gpaint_tool * tool, int x, int y);
static void rectangle_select_motion(gpaint_tool * tool, int x, int y);
static void rectangle_select_button_release(gpaint_tool * tool, int x, int y);

static void select_rectangle(gpaint_selection *selection, gpaint_point *v1, gpaint_point *v2)
{
    selection_remove_points(selection);
    selection_add_point(selection, min(v1->x, v2->x), min(v1->y, v2->y));
    selection_add_point(selection, max(v1->x, v2->x), min(v1->y, v2->y));
    selection_add_point(selection, max(v1->x, v2->x), max(v1->y, v2->y));
    selection_add_point(selection, min(v1->x, v2->x), max(v1->y, v2->y));
    selection_add_point(selection, min(v1->x, v2->x), min(v1->y, v2->y));
}



gpaint_tool *
rectangle_select_create(const char *name)
{
    gpaint_rectangle_select *rectangle_select = g_new0(gpaint_rectangle_select, 1);
    gpaint_tool *tool = GPAINT_TOOL(rectangle_select);

    tool->name = name;
    tool->cursor = gdk_cursor_new(GDK_CROSSHAIR);
    tool->destroy        = rectangle_select_destroy;
    tool->select         = rectangle_select_select;
    tool->deselect       = rectangle_select_deselect;
    tool->button_press   = rectangle_select_button_press;
    tool->button_release = rectangle_select_button_release;
    tool->motion         = rectangle_select_motion;
    return tool; 
}

static void
rectangle_select_destroy(gpaint_tool *tool)
{
    debug_fn();
    gdk_cursor_destroy(tool->cursor);
    g_free(tool);
}

static void
rectangle_select_select(gpaint_tool *tool)
{
    gpaint_rectangle_select *rectangle_select = GPAINT_RECTANGLE_SELECT(tool);
    rectangle_select->selection = tool->canvas->selection;
    selection_enable_flash(rectangle_select->selection);
}

static void
rectangle_select_deselect(gpaint_tool *tool)
{
    gpaint_rectangle_select *rectangle_select = GPAINT_RECTANGLE_SELECT(tool);
    selection_remove_points(rectangle_select->selection);
}

static void
rectangle_select_focus_change(gpaint_tool *tool)
{
    gpaint_rectangle_select *rectangle_select = GPAINT_RECTANGLE_SELECT(tool);
    debug_fn();
    if (tool->canvas->has_focus)
    {
        selection_enable_flash(rectangle_select->selection);
    }
    else
    {
        selection_disable_flash(rectangle_select->selection);
    }
}

static void
rectangle_select_button_press(gpaint_tool * tool, int x, int y)
{
    gpaint_rectangle_select *rs = GPAINT_RECTANGLE_SELECT(tool);
    debug_fn();

    if (drawing_in_bounds(tool->drawing, x, y))
    {
        set_point(&rs->v1, x, y);
        set_point(&rs->v2, x, y);
        select_rectangle(rs->selection, &rs->v1, &rs->v2);
    }
}

static void
rectangle_select_motion(gpaint_tool * tool, int x, int y)
{
    gpaint_rectangle_select *rs = GPAINT_RECTANGLE_SELECT(tool);
    debug_fn();

    if (drawing_in_bounds(tool->drawing, x, y))
    {
        select_rectangle(rs->selection, &rs->v1, &rs->v2);
        selection_draw_flash(rs->selection);
        set_point(&rs->v2, x, y);
    }
}

static void
rectangle_select_button_release(gpaint_tool * tool, int x, int y)
{
    gpaint_rectangle_select *rs = GPAINT_RECTANGLE_SELECT(tool);
    debug_fn();

    if (drawing_in_bounds(tool->drawing, x, y))
    {
        set_point(&rs->v2, x, y);
        select_rectangle(rs->selection, &rs->v1, &rs->v2);
    }
}
