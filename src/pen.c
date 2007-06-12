/* $Id: pen.c,v 1.3 2004/12/25 04:41:58 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003, 2007  Li-Cheng (Andy) Tai
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

#include "pen.h"
#include "debug.h"

typedef struct _gpaint_pen
{
    gpaint_tool tool;
    int width;
    gpaint_point last;
} gpaint_pen;


#define GPAINT_PEN(tool)  ((gpaint_pen*)(tool))


static void pen_destroy(gpaint_tool *tool);
static void pen_select(gpaint_tool *tool);
static void pen_deselect(gpaint_tool *tool);
static void pen_draw_line(gpaint_tool* tool, int x, int y);
static void pen_button_release(gpaint_tool *tool, int x, int y);


/*
 * Create a new pen tool object.
 */
gpaint_tool *pen_create(const char *name)
{
    gpaint_pen *pen = g_new0(gpaint_pen, 1);
    g_assert(pen);

    GPAINT_TOOL(pen)->name = name;
    GPAINT_TOOL(pen)->cursor = gdk_cursor_new(GDK_PENCIL);
    GPAINT_TOOL(pen)->destroy        = pen_destroy;
    GPAINT_TOOL(pen)->select         = pen_select;
    GPAINT_TOOL(pen)->deselect       = pen_deselect;
    GPAINT_TOOL(pen)->button_press   = pen_draw_line;
    GPAINT_TOOL(pen)->motion         = pen_draw_line;
    GPAINT_TOOL(pen)->button_release = pen_button_release;
    
    pen->width = 1;
    return GPAINT_TOOL(pen);
}

static void 
pen_destroy(gpaint_tool *tool)
{
    debug_fn();
    gdk_cursor_destroy(tool->cursor);
    g_free(tool);
}

static void
pen_select(gpaint_tool *tool)
{
    debug_fn();
}

static void
pen_deselect(gpaint_tool *tool)
{
    debug_fn();
    clear_point(&(GPAINT_PEN(tool)->last));
}

static void
pen_draw_line(gpaint_tool* tool, int x, int y)
{
    gpaint_pen *pen = GPAINT_PEN(tool);
    if (!pen->last.defined)
    {
        set_point(&pen->last, x, y);
    }
    gdk_draw_line(tool->drawing->backing_pixmap, tool->drawing->gc,
                  pen->last.x, pen->last.y, x, y);
    gdk_draw_line(tool->drawing->window, tool->drawing->gc,
                  pen->last.x, pen->last.y, x, y);
    drawing_modified(tool->drawing);
    set_point(&pen->last, x, y);
}

static void
pen_button_release(gpaint_tool *tool, int x, int y)
{
    clear_point(&(GPAINT_PEN(tool)->last));
}


