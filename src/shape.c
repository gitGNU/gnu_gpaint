/* $Id: shape.c,v 1.3 2004/12/25 04:41:58 meffie Exp $
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

#include "shape.h"
#include "debug.h"

#include <gtk/gtk.h>


typedef struct _gpaint_shape
{
    gpaint_tool base;
    GdkFunction normal_function;
    int width;
    gboolean banding;
    gpaint_point anchor;
    gpaint_point drag;
    gpaint_point release;
    gboolean     fill;
    void (*draw)(struct _gpaint_shape*, gboolean);
} gpaint_shape;

#define GPAINT_SHAPE(tool) ((gpaint_shape*)(tool))

static gpaint_shape* shape_create(const char *name);
static void shape_destroy(gpaint_tool *tool);
static void shape_select(gpaint_tool *tool);
static void shape_deselect(gpaint_tool *tool);
static gboolean shape_attribute(gpaint_tool *tool, gpaint_attribute attribute, gpointer data);
static void shape_button_press(gpaint_tool *tool, int x, int y);
static void shape_motion(gpaint_tool *tool, int x, int y);
static void shape_button_release(gpaint_tool *tool, int x, int y);

static void multiline_button_press(gpaint_tool *tool, int x, int y);

static void shape_start_banding(gpaint_shape* shape, int x, int y);
static void shape_stop_banding(gpaint_shape* shape);
static void draw_line(gpaint_shape* shape, gboolean fill);
static void draw_rectangle(gpaint_shape* shape, gboolean fill);
static void draw_oval(gpaint_shape *shape, gboolean fill);
static void draw_arc(gpaint_shape *shape, gboolean fill);


/*
 *  Template to generate shape constructors.
 */
#define SHAPE_CREATE(TYPE)                     \
gpaint_tool*                                   \
TYPE##_shape_create(const char *name)          \
{                                              \
    gpaint_shape *shape = shape_create(name);  \
    shape->draw = draw_##TYPE;                 \
    return (gpaint_tool*)shape;                \
}

SHAPE_CREATE(line);
SHAPE_CREATE(rectangle);
SHAPE_CREATE(oval);
SHAPE_CREATE(arc);

gpaint_tool*
multiline_shape_create(const char *name)
{
    gpaint_tool *tool = line_shape_create(name); 
    tool->button_press = multiline_button_press;
    return tool;
}

/*
 * Create a shape tool object for drawing things like
 * rectangles and ovals that can support rubber-banding
 * (ie. can be stretched into shape by dragging the mouse).
 */
static gpaint_shape*
shape_create(const char *name)
{
    gpaint_shape *shape = g_new0(gpaint_shape, 1);
    g_assert(shape);
    
    GPAINT_TOOL(shape)->name = name;
    GPAINT_TOOL(shape)->cursor = gdk_cursor_new(GDK_CROSSHAIR);
    GPAINT_TOOL(shape)->destroy        = shape_destroy;
    GPAINT_TOOL(shape)->select         = shape_select;
    GPAINT_TOOL(shape)->deselect       = shape_deselect;
    GPAINT_TOOL(shape)->attribute      = shape_attribute;
    GPAINT_TOOL(shape)->button_press   = shape_button_press;
    GPAINT_TOOL(shape)->button_release = shape_button_release;
    GPAINT_TOOL(shape)->motion         = shape_motion;
    shape->width = 1;
    return shape;
}

/*
 * Destroy the shape tool object.
 */
static void 
shape_destroy(gpaint_tool *tool)
{
    debug_fn();
    gdk_cursor_destroy(tool->cursor);
    g_free(tool);
}

/*
 * The shape tool has been selected as the active tool.
 */
static void
shape_select(gpaint_tool *tool)
{
    GdkGCValues gcvalues;
    gdk_gc_get_values(tool->drawing->gc, &gcvalues);
    GPAINT_SHAPE(tool)->normal_function = gcvalues.function;
}

/*
 * The shape tool has been deselected.
 */
static void
shape_deselect(gpaint_tool *tool)
{
    gpaint_shape *shape = GPAINT_SHAPE(tool);
    gdk_gc_set_function(tool->drawing->gc, GPAINT_SHAPE(tool)->normal_function); 
    clear_point(&shape->anchor);
    clear_point(&shape->drag);
    clear_point(&shape->release);
}

static gboolean 
shape_attribute(gpaint_tool *tool, gpaint_attribute attribute, gpointer data)
{
    if (attribute==GpaintFillShape)
    {
        GPAINT_SHAPE(tool)->fill = *((gboolean*)data);
        return TRUE;
    }
    return FALSE;
}

/*
 * Start rubber-banding the shape on a mouse click.
 */
static void
shape_button_press(gpaint_tool *tool, int x, int y)
{ 
    GPAINT_SHAPE(tool)->drag.defined = FALSE;
    shape_start_banding(GPAINT_SHAPE(tool), x, y);
}

/*
 * Rubber-band the shape on mouse drag.
 */
static void
shape_motion(gpaint_tool *tool, int x, int y)
{
    gpaint_shape *shape = GPAINT_SHAPE(tool);
    if (!shape->anchor.defined)
    {
        shape_start_banding(shape, x, y);
    }
    if (shape->drag.defined)
    {
        (*shape->draw)(shape, FALSE);
    }
    set_point(&shape->drag, x ,y);
    (*shape->draw)(shape, FALSE); 
}

/*
 * Draw the shape when the drag is done.
 */
static void
shape_button_release(gpaint_tool *tool, int x, int y)
{
    gpaint_shape *shape = GPAINT_SHAPE(tool);
    if (shape->anchor.defined)
    {
        if (shape->drag.defined)
        {
            (*shape->draw)(shape, FALSE);
        }
        shape_stop_banding(shape);      
        set_point(&shape->drag, x ,y);
        (*shape->draw)(shape, shape->fill);
    }
    clear_point(&shape->anchor);
    clear_point(&shape->drag);
    set_point(&shape->release, x, y);   
}

/*
 *  Start a new segment. 
 *  TODO: Make the segment between the last release and this 
 *        click rubberband. This feature still works just like 0.2.2. 
 */
static void
multiline_button_press(gpaint_tool *tool, int x, int y)
{ 
    gpaint_shape *shape = GPAINT_SHAPE(tool);
    if (shape->release.defined)
    {
        gdk_draw_line(tool->drawing->window, tool->drawing->gc,
                      shape->release.x, shape->release.y, x, y);
        gdk_draw_line(tool->drawing->backing_pixmap, tool->drawing->gc,
                      shape->release.x, shape->release.y, x, y);
        drawing_modified(tool->drawing);
    }
    shape_button_press(tool, x, y);
}


/*
 * Start rubberbanding the shape.
 */
static void
shape_start_banding(gpaint_shape* shape, int x, int y)
{
   set_point(&shape->anchor, x, y);
   gdk_gc_set_function(GPAINT_TOOL(shape)->drawing->gc, GDK_INVERT); 
   shape->banding = TRUE;
}

/*
 * Stop rubberbanding the shape.
 */
static void
shape_stop_banding(gpaint_shape* shape)
{
   gdk_gc_set_function(GPAINT_TOOL(shape)->drawing->gc, shape->normal_function); 
   shape->banding = FALSE;
}

/*
 * Draw a straight line.
 */
static void
draw_line(gpaint_shape *shape, gboolean fill)
{ 
    gdk_draw_line(GPAINT_TOOL(shape)->drawing->window, 
                  GPAINT_TOOL(shape)->drawing->gc,
                  shape->anchor.x, shape->anchor.y,
                  shape->drag.x,   shape->drag.y );
    if (!shape->banding) 
    {
        gdk_draw_line(GPAINT_TOOL(shape)->drawing->backing_pixmap, 
                      GPAINT_TOOL(shape)->drawing->gc,
                      shape->anchor.x, shape->anchor.y,
                      shape->drag.x,   shape->drag.y );
    }
    drawing_modified(GPAINT_TOOL(shape)->drawing);
}

/*
 * Draw a rectangle.
 */
static void
draw_rectangle(gpaint_shape *shape, gboolean filled)
{
    GdkDrawable *d = GPAINT_TOOL(shape)->drawing->window;
    GdkGC *gc = GPAINT_TOOL(shape)->drawing->gc;
    gint x1 = shape->anchor.x;
    gint y1 = shape->anchor.y;
    gint x2 = shape->drag.x;
    gint y2 = shape->drag.y;

    int ox, w, oy, h;
    if (x1 > x2)
    {
        ox = x2;
        w = x1 - x2;
    }
    else
    {
        ox = x1;
        w = x2 - x1;
    }
   
    if (y1 > y2)
    {
        oy = y2;
        h = y1 - y2;
    }
    else
    {
        oy = y1;
        h = y2 - y1;
    }
    if (filled)
    {
        w++;
        h++;
    }
    gdk_draw_rectangle(d, gc, filled, ox, oy, w, h);
    if (!shape->banding)
    {
        gdk_draw_rectangle(GPAINT_TOOL(shape)->drawing->backing_pixmap, gc, filled, ox, oy, w, h);
    }
    drawing_modified(GPAINT_TOOL(shape)->drawing);
}

/*
 *  Draw an ellipse.
 */
static void
draw_oval(gpaint_shape *shape, gboolean filled)
{
    GdkDrawable *d = GPAINT_TOOL(shape)->drawing->window;
    GdkGC *gc = GPAINT_TOOL(shape)->drawing->gc;
    gint x1 = shape->anchor.x;
    gint y1 = shape->anchor.y;
    gint x2 = shape->drag.x;
    gint y2 = shape->drag.y;
   
    int ox, w, oy, h;
    if (x1 > x2)
    {
        ox = x2;
        w = x1 - x2;
    }
    else
    {
        ox = x1;
        w = x2 - x1;
    }
   
    if (y1 > y2)
    {
        oy = y2;
        h = y1 - y2;
    }
    else
    {
        oy = y1;
        h = y2 - y1;
    }
    if (filled)
    {
        w++;
        h++;
    }
    gdk_draw_arc(d, gc, filled, ox, oy, w, h, 0, 360 * 64);
    if (!shape->banding)
    {
        gdk_draw_arc(GPAINT_TOOL(shape)->drawing->backing_pixmap, gc, filled, ox, oy, w, h, 0, 360 * 64);
    }
    drawing_modified(GPAINT_TOOL(shape)->drawing);
}

/*
 * Draw an arc.
 */
static void
draw_arc(gpaint_shape *shape, gboolean filled)
{
    GdkDrawable *d = GPAINT_TOOL(shape)->drawing->window;
    GdkGC *gc = GPAINT_TOOL(shape)->drawing->gc;
    gint x1 = shape->anchor.x;
    gint y1 = shape->anchor.y;
    gint x2 = shape->drag.x;
    gint y2 = shape->drag.y;
    int ox, w, oy, h;
    int angle1, angle2;
   
    if (x1 < x2)
    {
        w = (x2 - x1) * 2;
     
        if (y1 < y2)
        {
            h = (y2 - y1) * 2;  
            ox = x2 - w;
            oy = y1;
            angle1 = 90;
            angle2 = -90;
        }
        else
        {
            ox = x1;
            h = (y1 - y2) * 2;  
            oy = y2;
            angle1 = 180;
            angle2 = -90;
        }
    }
    else /* x1 > x2 */
    {
        w = (x1 - x2) * 2;
      
        if (y1 < y2)
        {
            h = (y2 - y1) * 2;
            ox = x1 - w;
            oy = y2 - h;
            angle1 = 0;
            angle2 = -90;
        }
        else
        {
            h = (y1 - y2) * 2;
            oy = y1 - h;
            ox = x2;
            angle1 = -90;
            angle2 = -90;
        }
    }
    if (filled)
    {
        w++;
        h++;
    }
    gdk_draw_arc(d, gc, filled, ox, oy, w, h, angle1 * 64, angle2 * 64);
    if (!shape->banding)
    {
        gdk_draw_arc(GPAINT_TOOL(shape)->drawing->backing_pixmap, gc, filled, ox, oy, w, h, angle1 * 64, angle2 * 64);
    }
    drawing_modified(GPAINT_TOOL(shape)->drawing);
}
