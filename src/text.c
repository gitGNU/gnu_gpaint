/* $Id: text.c,v 1.4 2005/01/27 02:53:01 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003  Li-Cheng (Andy) Tai
 *
 * Authors: Li-Cheng (Andy) Tai
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

#include "text.h"
#include "debug.h"
#include <gdk/gdkkeysyms.h>

#define MAX_TEXT_LENGTH        2000
#define TEXT_CURSOR_WIDTH      2
#define TEXT_CURSOR_BLINK_RATE 500
#define FONT_NAME  "-*-helvetica-*-r-normal--*-120-*-*-*-*-iso8859-1"

typedef struct _gpaint_text
{
    gpaint_tool tool;
    GdkFont *font; 
    gpaint_point location;
    gint timer;
    gint flash_state;
    char textbuf[MAX_TEXT_LENGTH];
} gpaint_text;


#define GPAINT_TEXT(tool)  ((gpaint_text*)(tool))

static void text_destroy(gpaint_tool *tool);
static void text_select(gpaint_tool *tool);
static void text_deselect(gpaint_tool *tool);
static gboolean text_attribute(gpaint_tool* tool, gpaint_attribute attrib, gpointer data);
static void text_button_press(gpaint_tool *tool, int x, int y);
static void text_button_release(gpaint_tool *tool, int x, int y);
static void text_key_release(gpaint_tool *tool, GdkEventKey *key);
static void text_focus_change(gpaint_tool *tool, gpaint_change change, gpointer data);

static void text_clear_cursor(gpaint_text *text);
static void text_draw_cursor(gpaint_text *text);

static void text_draw_string(gpaint_text *text);
static gint text_handle_timeout(gpaint_text *text);

gpaint_tool *text_create(const char *name)
{
    gpaint_text *text = g_new0(gpaint_text, 1);
    g_assert(text);
    GPAINT_TOOL(text)->name = name;
    GPAINT_TOOL(text)->cursor = gdk_cursor_new(GDK_XTERM);
    GPAINT_TOOL(text)->destroy        = text_destroy;
    GPAINT_TOOL(text)->select         = text_select;
    GPAINT_TOOL(text)->deselect       = text_deselect;
    GPAINT_TOOL(text)->attribute      = text_attribute;
    GPAINT_TOOL(text)->button_press   = text_button_press;
    GPAINT_TOOL(text)->button_release = text_button_release;
    GPAINT_TOOL(text)->key_release    = text_key_release;
    GPAINT_TOOL(text)->change         = text_focus_change;
      
    return GPAINT_TOOL(text);
}

static void 
text_destroy(gpaint_tool *tool)
{
    debug_fn();
    gdk_cursor_destroy(tool->cursor);
    g_free(tool);
}

static void
text_select(gpaint_tool *tool)
{
    gpaint_text *text = GPAINT_TEXT(tool);
    debug_fn();
    strcpy(text->textbuf, "");
    text->timer = gtk_timeout_add(TEXT_CURSOR_BLINK_RATE,
                     (GtkFunction)(text_handle_timeout), text);

    if (!text->font)
    {
        text->font = gdk_font_load(FONT_NAME);
    }
}

static void
text_deselect(gpaint_tool *tool)
{
    gpaint_text *text = GPAINT_TEXT(tool);
    debug_fn();

    g_assert(text->timer);
    gtk_timeout_remove(text->timer);
    text->timer = 0;
    
    text_clear_cursor(text);
    if (is_point_defined(&text->location) && strlen(text->textbuf))
    {
        text_draw_string(text);
    }
    clear_point(&text->location);
}

static gboolean
text_attribute(gpaint_tool* tool, gpaint_attribute attrib, gpointer data)
{
    debug_fn();
    if (attrib == GpaintFont)
    {
        gpaint_text *text = GPAINT_TEXT(tool);
        GdkFont *font = (GdkFont*)data;
        if (text->font)
        {
            gdk_font_unref(text->font);
        }
        text->font = font;
        gdk_font_ref(font);
        return TRUE;
    }
    return FALSE;
}

static void
text_focus_change(gpaint_tool *tool, gpaint_change change, gpointer data)
{
    debug_fn();
    if (change==GpaintFocusIn || change==GpaintFocusOut)
    {
        text_clear_cursor(GPAINT_TEXT(tool));
    }
}

static void
text_button_press(gpaint_tool *tool, int x, int y)
{
    gpaint_text *text = GPAINT_TEXT(tool);
    if (is_point_defined(&text->location))
    {
        if (strlen(text->textbuf))
        {
            text_draw_string(text);
        }
        text_clear_cursor(text);
        strcpy(text->textbuf, "");
        clear_point(&text->location);
    }
}

static void
text_button_release(gpaint_tool *tool, int x, int y)
{
    gpaint_text *text = GPAINT_TEXT(tool);
    GtkWidget *widget = GTK_WIDGET(tool->canvas->drawing_area);
    gtk_widget_grab_focus(widget);
    gtk_widget_grab_default(widget);
    set_point(&text->location, x, y);
}

static void
text_key_release(gpaint_tool *tool, GdkEventKey *keyevent)
{
    gpaint_text *text = GPAINT_TEXT(tool);
    gpaint_drawing *drawing = tool->drawing;
    GdkGCValues gcvalues;
    GdkColor white;
    int length;
    char tmp[MAX_TEXT_LENGTH];
   
    text_clear_cursor(text);
    gdk_gc_get_values(drawing->gc, &gcvalues);
    strcpy(tmp, text->textbuf);
    length = strlen(text->textbuf);
    if ((keyevent->keyval == GDK_BackSpace) || (keyevent->keyval == GDK_Delete))
    {
        if (length)
        {
            text->textbuf[length - 1] = 0;
        }
    }
    else if (keyevent->string)
    {
        strcat(text->textbuf, keyevent->string);
    }
    else if ((keyevent->keyval >= GDK_space) && (keyevent->keyval < GDK_Shift_L))
    {
        /* FIXME: buffer overflow */
        sprintf(text->textbuf, "%s%c", tmp, keyevent->keyval);
    }
    if (keyevent->keyval == GDK_Escape)
    {
        strcpy(text->textbuf, "");
    }
   
    gdk_gc_set_function(drawing->gc, GDK_INVERT);
    gdk_color_white(gdk_rgb_get_cmap(), &white);
   
    gdk_gc_set_foreground(drawing->gc, &white);   
    if (strlen(tmp))
    {
        gdk_draw_string(drawing->window,
                        text->font,
                        drawing->gc,
                        text->location.x, text->location.y,
                        tmp);
        gdk_draw_string(drawing->backing_pixmap,
                        text->font, 
                        drawing->gc,
                        text->location.x, text->location.y,
                        tmp);
    }
    
    if (strlen(text->textbuf))
    {
        gdk_draw_string(drawing->window,
                        text->font,
                        drawing->gc,
                        text->location.x, text->location.y,
                        text->textbuf);
        gdk_draw_string(drawing->backing_pixmap,
                        text->font,
                        drawing->gc,
                        text->location.x, text->location.y,
                        text->textbuf);
        drawing_modified(drawing);
    }
   
   
    gdk_gc_set_function(drawing->gc, gcvalues.function);   
    gdk_gc_set_foreground(drawing->gc, &(gcvalues.foreground));
   
    if (keyevent->keyval == GDK_Return)
    {
        if (strlen(text->textbuf))
        {
            gdk_draw_string(drawing->window, 
                            text->font,
                            drawing->gc,
                            text->location.x, text->location.y,
                            text->textbuf);
            gdk_draw_string(drawing->backing_pixmap, 
                            text->font,
                            drawing->gc,
                            text->location.x, text->location.y,
                            text->textbuf);
            drawing_modified(drawing);
        }
        strcpy(text->textbuf, "");
      
        text->location.y +=  (int) (gdk_string_height(text->font, "My") * 1.1 + 0.5);
    }
    else if (keyevent->keyval == GDK_Escape)
    {
        clear_point(&text->location);
    }
}

static void
text_draw_string(gpaint_text *text)
{
    gpaint_drawing *drawing = GPAINT_TOOL(text)->drawing;
    gdk_draw_string(drawing->window,
                    text->font,
                    drawing->gc,
                    text->location.x, text->location.y,
                    text->textbuf);
    gdk_draw_string(drawing->backing_pixmap,
                    text->font,
                    drawing->gc,
                    text->location.x, text->location.y,
                    text->textbuf);
    drawing_modified(drawing);
}

static gint
text_handle_timeout(gpaint_text *text)
{
    if (GPAINT_TOOL(text)->canvas->has_focus)
    {    
        //debug("tick");
        text_draw_cursor(text);
    }
    else
    {    
        text_clear_cursor(text);
    }   
    return 1;
}

static void
text_clear_cursor(gpaint_text *text)
{
   // debug_fn();
   // debug1("text->flash_state=%d", text->flash_state);
    if (text->flash_state)
    {
        debug("calling text_draw_cursor()");
        text_draw_cursor(text);
        debug1("after text_draw_cursor() text->flash_state=%d", text->flash_state);
    }
}

static void
text_draw_cursor(gpaint_text *text)
{
    gpaint_drawing *drawing = GPAINT_TOOL(text)->drawing;
   
    GdkGCValues gcvalues;
    gdk_gc_get_values(drawing->gc, &gcvalues);

    if (is_point_defined(&text->location))
    {
        gint width, ascent, descent, lbearing, rbearing, junk;
        GdkColor white;

        gdk_color_white(gdk_rgb_get_cmap(), &white);
   
        gdk_gc_set_foreground(drawing->gc, &white);   
 
        gdk_string_extents(text->font, "Wy", 
                           &junk, &junk, &junk, &ascent, &descent);
        gdk_string_extents(text->font, text->textbuf,
                           &lbearing, &rbearing, &width, &junk, &junk);

        gdk_gc_set_function(drawing->gc, GDK_INVERT);
        gdk_gc_set_line_attributes(drawing->gc,
                                   TEXT_CURSOR_WIDTH, 
                                   GDK_LINE_SOLID,
                                   GDK_CAP_PROJECTING,
                                   GDK_JOIN_ROUND);
        gdk_draw_line(drawing->window,
                      drawing->gc,
                      text->location.x + rbearing + 2,
                      text->location.y + descent,
                      text->location.x + rbearing + 2,
                      text->location.y - ascent);   
        gdk_draw_line(drawing->backing_pixmap,
                      drawing->gc, 
                      text->location.x + rbearing + 2, 
                      text->location.y + descent, 
                      text->location.x + rbearing + 2, 
                      text->location.y - ascent);   

        gdk_gc_set_function(drawing->gc, gcvalues.function);   
        gdk_gc_set_line_attributes(drawing->gc, 
                                   gcvalues.line_width,
                                   gcvalues.line_style,
                                   gcvalues.cap_style, 
                                   gcvalues.join_style);
        gdk_gc_set_foreground(drawing->gc, &(gcvalues.foreground));

        text->flash_state = !(text->flash_state); 
    }
}


