/* $Id: lasso.c,v 1.2 2004/03/13 03:29:27 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003  Li-Cheng (Andy) Tai
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

#include "lasso.h"
#include "selection.h"
#include "debug.h"

typedef struct _gpaint_lasso
{
    gpaint_tool       tool;
    gpaint_selection *selection;
} gpaint_lasso;

#define GPAINT_LASSO(tool) ((gpaint_lasso*)(tool))

static void lasso_destroy(gpaint_tool * tool);
static void lasso_select(gpaint_tool * tool);
static void lasso_deselect(gpaint_tool * tool);
static void lasso_button_press(gpaint_tool * tool, int x, int y);
static void lasso_motion(gpaint_tool * tool, int x, int y);
static void lasso_button_release(gpaint_tool * tool, int x, int y);

gpaint_tool *
lasso_select_create(const char *name)
{
    gpaint_lasso *lasso = g_new0(gpaint_lasso, 1);
    gpaint_tool *tool = GPAINT_TOOL(lasso);
    tool->name = name;
    tool->cursor = gdk_cursor_new(GDK_CROSSHAIR);
    tool->destroy        = lasso_destroy;
    tool->select         = lasso_select;
    tool->deselect       = lasso_deselect;
    tool->button_press   = lasso_button_press;
    tool->button_release = lasso_button_release;
    tool->motion         = lasso_motion;
    return tool; 
}

static void
lasso_destroy(gpaint_tool *tool)
{
    debug_fn();
    gdk_cursor_destroy(tool->cursor);
    g_free(tool);
}

static void
lasso_select(gpaint_tool *tool)
{
    gpaint_lasso *lasso = GPAINT_LASSO(tool);
    lasso->selection = tool->canvas->selection;
    selection_enable_flash(lasso->selection);
}

static void
lasso_deselect(gpaint_tool *tool)
{
    gpaint_lasso *lasso = GPAINT_LASSO(tool);
    selection_disable_flash(lasso->selection);
    selection_remove_points(lasso->selection);
}

static void
lasso_focus_change(gpaint_tool *tool)
{
    gpaint_lasso *lasso = GPAINT_LASSO(tool);
    debug_fn();
    if (tool->canvas->has_focus)
    {
        selection_enable_flash(lasso->selection);
    }
    else
    {
        selection_disable_flash(lasso->selection);
    }
}

static void
lasso_button_press(gpaint_tool * tool, int x, int y)
{
    gpaint_lasso *lasso = GPAINT_LASSO(tool);
    debug_fn();
    if (drawing_in_bounds(tool->drawing, x, y))
    {
        selection_remove_points(lasso->selection);
        selection_add_point(lasso->selection, x, y);
    }
}

static void
lasso_motion(gpaint_tool * tool, int x, int y)
{
    gpaint_lasso *lasso = GPAINT_LASSO(tool);
    debug_fn();
    if (drawing_in_bounds(tool->drawing, x, y))
    {
        selection_add_point(lasso->selection, x, y);
        selection_draw_flash(lasso->selection);
    }
}

static void
lasso_button_release(gpaint_tool * tool, int x, int y)
{
    gpaint_lasso *lasso = GPAINT_LASSO(tool);
    debug_fn();
    if (drawing_in_bounds(tool->drawing, x, y))
    {
        selection_add_point(lasso->selection, x, y);
        selection_close_loop(lasso->selection);
        selection_draw_flash(lasso->selection);
    }
}

