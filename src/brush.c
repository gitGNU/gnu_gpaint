/* $Id: brush.c,v 1.3 2004/12/25 04:41:58 meffie Exp $
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

#include "brush.h"
#include "debug.h"
#include <math.h>

static const double EPSILON = 0.00001;

typedef struct _gpaint_brush
{
    gpaint_tool base;
    void (*draw)(struct _gpaint_brush*,int,int);    
    double spacing;        
    int    size;
    double distance;
    gpaint_point drag;
} gpaint_brush;

typedef struct _gpaint_eraser
{
    gpaint_brush base;
    GdkColor *fgcolor;  /* to restore the foreground color  */ 
} gpaint_eraser;

#define GPAINT_BRUSH(tool) ((gpaint_brush*)(tool))
#define GPAINT_ERASER(tool) ((gpaint_eraser*)(tool))

static void eraser_select(gpaint_tool *tool);
static void eraser_deselect(gpaint_tool *tool);
static void eraser_draw(gpaint_brush *brush, int x, int y);
static gboolean eraser_attribute(gpaint_tool *tool, gpaint_attribute, gpointer);

static void eraser_set_foreground_color(gpaint_eraser *eraser, GdkColor* color);
static void eraser_set_background_color(gpaint_eraser *eraser, GdkColor* color);

static void paint_brush_draw(gpaint_brush *brush, int x, int y);
   
static void brush_destroy(gpaint_tool *tool);
static void brush_select(gpaint_tool *tool);
static void brush_button_press(gpaint_tool *tool, int x, int y);
static void brush_button_release(gpaint_tool *tool, int x, int y);
static void brush_motion(gpaint_tool *tool, int x, int y);
static void brush_interpolate(gpaint_brush *brush, int x, int y);


/*
 * Create the erase tool object.
 */
gpaint_tool *
eraser_create(const char *name)
{
    gpaint_eraser *eraser = g_new0(gpaint_eraser,1);

    GPAINT_TOOL(eraser)->name   = name;
    GPAINT_TOOL(eraser)->cursor = gdk_cursor_new(GDK_X_CURSOR);
    GPAINT_TOOL(eraser)->destroy        = brush_destroy;
    GPAINT_TOOL(eraser)->select         = brush_select;
    GPAINT_TOOL(eraser)->button_press   = brush_button_press;
    GPAINT_TOOL(eraser)->button_release = brush_button_release;
    GPAINT_TOOL(eraser)->motion         = brush_motion;
    GPAINT_TOOL(eraser)->select         = eraser_select;
    GPAINT_TOOL(eraser)->deselect       = eraser_deselect;
    GPAINT_TOOL(eraser)->attribute      = eraser_attribute;
    
    GPAINT_BRUSH(eraser)->draw    = eraser_draw;    
    GPAINT_BRUSH(eraser)->spacing = 3.0;        
    GPAINT_BRUSH(eraser)->size = 15;
    GPAINT_BRUSH(eraser)->distance = 0;
    return GPAINT_TOOL(eraser);
}

/*
 * Set up the eraser tool when selected. Use the current
 * background color as the foreground to simulate erasing.
 */
static void
eraser_select(gpaint_tool *tool)
{
    GdkGCValues gcvalues;
    GPAINT_BRUSH(tool)->drag.defined = FALSE;
    gdk_gc_get_values(tool->drawing->gc, &gcvalues);
    g_assert(!GPAINT_ERASER(tool)->fgcolor);
    GPAINT_ERASER(tool)->fgcolor = gdk_color_copy(&(gcvalues.foreground));
    gdk_gc_set_foreground(tool->drawing->gc, &(gcvalues.background));
}

/*
 * Restore the foreground color when the eraser is deselect.
 */
static void
eraser_deselect(gpaint_tool *tool)
{
    gdk_gc_set_foreground(tool->drawing->gc, GPAINT_ERASER(tool)->fgcolor);
    gdk_color_free(GPAINT_ERASER(tool)->fgcolor);
    GPAINT_ERASER(tool)->fgcolor = NULL;
}

/*
 * Erase
 */
static void
eraser_draw(gpaint_brush *brush, int x, int y)
{
    int radius = brush->size / 2;
    x -= radius;
    y -= radius;
    gdk_draw_rectangle(
            GPAINT_TOOL(brush)->drawing->window,
            GPAINT_TOOL(brush)->drawing->gc,
            TRUE, x, y, brush->size, brush->size);
    gdk_draw_rectangle(
            GPAINT_TOOL(brush)->drawing->backing_pixmap,
            GPAINT_TOOL(brush)->drawing->gc,
            TRUE, x, y, brush->size, brush->size);
    drawing_modified(GPAINT_TOOL(brush)->drawing);
}

/*
 * Save the new foreground/backgound color.
 */
static gboolean 
eraser_attribute(gpaint_tool* tool, gpaint_attribute attrib, gpointer data)
{
    gboolean handled = FALSE;
    gpaint_eraser *eraser = GPAINT_ERASER(tool);
    debug_fn();
    if (attrib == GpaintForegroundColor)
    {
        eraser_set_foreground_color(eraser, (GdkColor*)data);
        handled = TRUE;
    }
    else if (attrib == GpaintBackgroundColor)
    {
        eraser_set_background_color(eraser, (GdkColor*)data);
        handled = TRUE;
    }
    return handled;
}

/*
 * Save the new foreground color.
 */
static void
eraser_set_foreground_color(gpaint_eraser *eraser, GdkColor* color)
{
    debug_fn();
    g_assert(eraser->fgcolor);
    gdk_color_free(eraser->fgcolor);
    eraser->fgcolor = gdk_color_copy(color);
}

/*
 * Set the new background color.
 */
static void
eraser_set_background_color(gpaint_eraser *eraser, GdkColor* color)
{
    gpaint_tool *tool = GPAINT_TOOL(eraser);
    debug_fn();
    gdk_gc_set_background(tool->drawing->gc, color);
    gdk_gc_set_foreground(tool->drawing->gc, color);
}

/*
 * Create the paint brush tool object.
 */
gpaint_tool *
paint_brush_create(const char *name)
{
    gpaint_brush *brush = g_new0(gpaint_brush, 1);
    g_assert(brush); 
    GPAINT_TOOL(brush)->name = name;
    GPAINT_TOOL(brush)->cursor = gdk_cursor_new(GDK_CROSSHAIR);
    GPAINT_TOOL(brush)->destroy        = brush_destroy;
    GPAINT_TOOL(brush)->select         = brush_select;
    GPAINT_TOOL(brush)->button_press   = brush_button_press;
    GPAINT_TOOL(brush)->button_release = brush_button_release;
    GPAINT_TOOL(brush)->motion         = brush_motion;
    brush->draw = paint_brush_draw;    
    brush->spacing = 3.0;        
    brush->size = 15;
    brush->distance = 0;
    return GPAINT_TOOL(brush);
}

/*
 * Paint
 * 
 * TODO: brush styles.
 */
static void
paint_brush_draw(gpaint_brush *brush, int x, int y)
{
    int radius = brush->size / 2;
    x -= radius;
    y -= radius;
    gdk_draw_arc( GPAINT_TOOL(brush)->drawing->window,
                  GPAINT_TOOL(brush)->drawing->gc,
                  TRUE, x, y, brush->size, brush->size, 0, 360 * 64);
    gdk_draw_arc( GPAINT_TOOL(brush)->drawing->backing_pixmap,
                  GPAINT_TOOL(brush)->drawing->gc,
                  TRUE, x, y, brush->size, brush->size, 0, 360 * 64);
    drawing_modified(GPAINT_TOOL(brush)->drawing);
}

/*
 * Free the brush tool object.
 */ 
static void
brush_destroy(gpaint_tool *tool)
{
    debug_fn();
    gdk_cursor_destroy(tool->cursor);
    g_free(tool);
}

/*
 * Set up the brush tool when selected.
 */
static void
brush_select(gpaint_tool *tool)
{
    GPAINT_BRUSH(tool)->drag.defined = FALSE;
}

/*
 * Draw a point where the brush was pressed.
 */ 
static void 
brush_button_press(gpaint_tool *tool, int x, int y)
{ 
    gpaint_brush *brush = GPAINT_BRUSH(tool);
    debug_fn();
    set_point(&brush->drag, x, y);
    (*(brush->draw))(brush, x, y);
}

/*
 * Handle paint brush motion.
 */
static void 
brush_motion(gpaint_tool *tool, int x, int y)
{
    gpaint_brush* brush = GPAINT_BRUSH(tool);
    if (!is_point_defined(&brush->drag))
    {
        set_point(&brush->drag, x, y);
    }
    brush_interpolate(brush, x, y);
    set_point(&brush->drag, x, y);
}

/*
 * Handle paint brush release.
 */ 
static void
brush_button_release(gpaint_tool *tool, int x, int y)
{
    debug_fn();
    clear_point(&(GPAINT_BRUSH(tool)->drag));
}


/* 
 * Paint along the mouse movement. 
 * adapted from paint_core.c in the GIMP.
 */
static void 
brush_interpolate(gpaint_brush *brush, int x, int y)
{
    int    initial_x = x;
    int    initial_y = y;

    double dx;         /* delta x */
    double dy;         /* delta y */
    double moved;      /* mouse movement */       
    double initial;    /* initial paint distance */
    double final;      /* final paint distance   */

    double points;     /* number of paint points */
    double next;       /* distance to the next point */
    double percent;    /* linear interplotation, 0 to 1.0 */   
  
    /* calculate mouse move distance */
    dx = (double)(x - brush->drag.x);
    dy = (double)(y - brush->drag.y);
    moved = sqrt(dx*dx + dy*dy);

    initial = brush->distance;
    final = initial + moved;

    /* paint along the movement */
    while (brush->distance < final)
    {
        /* calculate distance to the next point */
        points = (int) (brush->distance / brush->spacing + 1.0 + EPSILON); 
        next = points * brush->spacing - brush->distance;
        brush->distance += next;          
        if (brush->distance <= (final + EPSILON))
        {   
           /* calculate interpolation */ 
           percent = (brush->distance - initial) / moved;
           x = (int)(brush->drag.x + percent * dx);
           y = (int)(brush->drag.y + percent * dy);

           (*brush->draw)(brush, x, y);
        }
     }
     brush->distance = final;
}

