/* $Id: canvas.h,v 1.4 2005/01/07 02:50:52 meffie Exp $
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

#ifndef __CANVAS_H__
#define __CANVAS_H__

#include <gtk/gtk.h>
#include "image.h"
#include "drawing.h"
#include "selection.h"

#define DEFAULT_WIDTH  640
#define DEFAULT_HEIGHT 480

typedef struct _gpaint_canvas    gpaint_canvas;
typedef struct _gpaint_tool      gpaint_tool;

typedef enum _gpaint_attribute 
{
    GpaintForegroundColor,
    GpaintBackgroundColor,
    GpaintFillShape,
    GpaintFont
} gpaint_attribute;

typedef enum _gpaint_change
{
    GpaintFocusIn,
    GpaintFocusOut
} gpaint_change;

/*
 * Drawing Tool
 */
typedef gpaint_tool* (*ToolCreate)       (const char*);
typedef void         (*ToolDestroy)      (gpaint_tool*);
typedef void         (*ToolSelect)       (gpaint_tool*);
typedef void         (*ToolDeselect)     (gpaint_tool*);
typedef gboolean     (*ToolAttribute)    (gpaint_tool*, gpaint_attribute, gpointer);
typedef void         (*ToolChange)       (gpaint_tool*, gpaint_change, gpointer);
typedef void         (*ToolButtonPress)  (gpaint_tool*, int, int); 
typedef void         (*ToolMotion)       (gpaint_tool*, int, int); 
typedef void         (*ToolButtonRelease)(gpaint_tool*, int, int); 
typedef void         (*ToolKeyRelease)   (gpaint_tool*, GdkEventKey*);

struct _gpaint_tool
{
    const char       *name;
    gpaint_drawing   *drawing;
    gpaint_canvas    *canvas;
    GdkCursor        *cursor;
    ToolDestroy       destroy;
    ToolSelect        select;
    ToolDeselect      deselect;
    ToolAttribute     attribute;
    ToolChange        change;
    ToolButtonPress   button_press;
    ToolMotion        motion;
    ToolButtonRelease button_release;
    ToolKeyRelease    key_release;
};

/* cast macro */
#define GPAINT_TOOL(object)  ((gpaint_tool*)object)

typedef struct _gpaint_clipboard
{
    gpaint_image       *image;
    gpaint_point_array *points;
} gpaint_clipboard;

struct _gpaint_canvas 
{
    /* public */
    GtkWidget        *top_level;       
    GtkDrawingArea   *drawing_area;
    GdkGC            *gc;
    gpaint_tool      *active_tool;           
    gpaint_drawing   *drawing;
    gboolean          has_focus;
    gpaint_selection *selection;

    /* private */
    GdkCursor        *cursor;
    GdkCursor        *busy_cursor;
    GdkCursor        *arrow_cursor;
    gpaint_tool      *paste_tool;
    gpaint_tool      *saved_tool;
};

void canvas_init(int argc, char *argv[]);
void canvas_destroy(gpaint_canvas *canvas);
gpaint_canvas* canvas_lookup(GtkWidget *widget);

void canvas_set_drawing(gpaint_canvas *canvas, gpaint_drawing *drawing);
void canvas_resize(gpaint_canvas *canvas);
void canvas_set_tool(gpaint_canvas *canvas, gpaint_tool *new_tool);

void canvas_begin_busy_cursor(gpaint_canvas *canvas);
void canvas_end_busy_cursor(gpaint_canvas *canvas);
void canvas_focus_gained(gpaint_canvas *canvas);
void canvas_focus_lost(gpaint_canvas *canvas);
void canvas_redraw(gpaint_canvas *canvas);

void canvas_begin_paste_mode(gpaint_canvas *canvas);
void canvas_end_paste_mode(gpaint_canvas *canvas);
void canvas_cut(gpaint_canvas *canvas);
void canvas_copy(gpaint_canvas *canvas);
void canvas_clear(gpaint_canvas *canvas);
void canvas_select_all(gpaint_canvas *canvas);
gboolean canvas_has_selection(gpaint_canvas *canvas);
gpaint_clipboard *canvas_clipboard(gpaint_canvas *canvas);

#endif
