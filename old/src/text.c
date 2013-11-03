/* $Id: text.c,v 1.4 2005/01/27 02:53:01 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003, 2007  Li-Cheng (Andy) Tai
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

#include <string.h>

#ifdef HAVE_STRING_H
#  include <string.h>
#endif

#include <gdk/gdkkeysyms.h>
#include <gdk/gdk.h>
#include <pango/pango.h>
#include <gtk/gtk.h>

#include "text.h"
#include "debug.h"


#define TEXT_CURSOR_WIDTH      2
#define TEXT_CURSOR_BLINK_RATE 500

typedef struct _gpaint_text
{
    gpaint_tool tool;
    
    gpaint_point location;
    gint timer;
    gint flash_state;
    GString *textbuf;
    int max_width;
    int max_height;
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
static void text_commit_change(gpaint_tool *);

static void text_clear_cursor(gpaint_text *text);
static void text_draw_cursor(gpaint_text *text);

static void text_draw_string(gpaint_text *text);
static gint text_handle_timeout(gpaint_text *text);
static void text_draw_current_string(gpaint_tool *tool);

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
    GPAINT_TOOL(text)->current_draw   = text_draw_current_string;
    GPAINT_TOOL(text)->commit_change  = text_commit_change;
    
    text->textbuf =   g_string_new(0);
    return GPAINT_TOOL(text);
}

static void 
text_destroy(gpaint_tool *tool)
{
    gpaint_text *text = GPAINT_TEXT(tool);
    debug_fn();
    gdk_cursor_destroy(tool->cursor);
    g_string_free(text->textbuf, TRUE);
    if (text->timer)
        gtk_timeout_remove(text->timer);
  
    g_free(tool);
}

static void
text_select(gpaint_tool *tool)
{
    gpaint_text *text = GPAINT_TEXT(tool);
    debug_fn();
    g_string_printf(text->textbuf, "");
    text->timer = g_timeout_add(TEXT_CURSOR_BLINK_RATE,
                     (GtkFunction)(text_handle_timeout), text);
}

static void
text_clear(gpaint_text *text)
{
    clear_point(&(text->location));
    text->max_width = text->max_height = 0;
    g_string_printf(text->textbuf, "");
  
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
    if (is_point_defined(&text->location) && g_utf8_strlen(text->textbuf->str, -1))
    {
        text_draw_string(text);
    }
    text_clear(text);
   
}

static gboolean
text_attribute(gpaint_tool* tool, gpaint_attribute attrib, gpointer data)
{
    debug_fn();
    GtkStyle *style;
    GtkWidget *widget = GTK_WIDGET(tool->canvas->drawing_area);
    style = gtk_widget_get_style(widget);
    g_assert(style);
      
    if (attrib == GpaintFont)
    {
        gpaint_text *text = GPAINT_TEXT(tool);
        gtk_widget_modify_font(widget, pango_font_description_from_string((char*) data));
      
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
        if (g_utf8_strlen(text->textbuf->str, -1))
        {
            text_draw_string(text);
        }
        text_clear_cursor(text);
     
        text_clear(text);
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


static void set_layout_foreground_color(PangoLayout *, GdkColor  *color);

static void
text_key_release(gpaint_tool *tool, GdkEventKey *keyevent)
{
    gpaint_text *text = GPAINT_TEXT(tool);
    gpaint_drawing *drawing = tool->drawing;
    PangoLayout *layout = 0;
    GtkStyle *style;
    GdkGCValues gcvalues;
    int length;
    char *tmp = 0;
    int width, height;
   
    text_clear_cursor(text);
    gdk_gc_get_values(drawing->gc, &gcvalues);
    tmp = strdup(text->textbuf->str);
    length = g_utf8_strlen(text->textbuf->str, -1);
    if ((keyevent->keyval == GDK_BackSpace) || (keyevent->keyval == GDK_Delete))
    {
        if (length)
        {
            g_string_set_size(text->textbuf, length - 1);/* not UTF8 safe */
        }
    }
    else if (keyevent->string)
    {
        g_string_append(text->textbuf, keyevent->string);
    }
    else if ((keyevent->keyval >= GDK_space) && (keyevent->keyval < GDK_Shift_L))
    {
        /* FIXME: buffer overflow */
        g_string_printf(text->textbuf, "%s%c", tmp, keyevent->keyval);
    }
    if (keyevent->keyval == GDK_Escape)
    {
        g_string_printf(text->textbuf, "");
    }
   
    layout = gtk_widget_create_pango_layout(GTK_WIDGET(tool->canvas->drawing_area), tmp);
    style = gtk_widget_get_style(GTK_WIDGET(tool->canvas->drawing_area));
    
    pango_layout_get_pixel_size(layout, &width, &height);
    if (text->max_width < width) text->max_width = width;
    if (text->max_height < height) text->max_height = height;
    
    
    if (tmp && g_utf8_strlen(tmp, -1))
    {
        gdk_draw_pixmap(drawing->window, drawing->gc, drawing->backing_pixmap, 
                         text->location.x, text->location.y,
                         text->location.x, text->location.y,
                         text->max_width, text->max_height);
    }
    
    pango_layout_set_text(layout, text->textbuf->str, -1);
    pango_layout_get_pixel_size(layout, &width, &height);
    if (text->max_width < width) text->max_width = width;
    if (text->max_height < height) text->max_height = height;
    {
        gdk_draw_pixmap(drawing->window, drawing->gc, drawing->backing_pixmap, 
                         text->location.x, text->location.y,
                         text->location.x, text->location.y,
                         text->max_width, text->max_height);
        set_layout_foreground_color(layout, &(gcvalues.foreground));
        gtk_paint_layout(style, drawing->window,
                        GTK_STATE_NORMAL, TRUE, NULL, 
                        GTK_WIDGET(tool->canvas->drawing_area), NULL, 
                        text->location.x, text->location.y,
                        layout);
       
    }
   
   
    if (keyevent->keyval == GDK_Return)
    {
        g_string_append(text->textbuf, "\n");
    }
    else if (keyevent->keyval == GDK_Escape)
    {
        text_clear(text);

    }
    free(tmp);
    g_object_unref(layout);
}

static void text_draw_current_string(gpaint_tool *tool)
{

    gpaint_text *text = GPAINT_TEXT(tool);
    GdkGCValues gcvalues;
    if (text && is_point_defined(&text->location))
    {
        gpaint_drawing *drawing = tool->drawing;    
        gdk_gc_get_values(drawing->gc, &gcvalues);
    
        PangoLayout *layout = gtk_widget_create_pango_layout(GTK_WIDGET(tool->canvas->drawing_area), text->textbuf->str);
      
        GtkStyle *  style = gtk_widget_get_style(GTK_WIDGET(tool->canvas->drawing_area));
        
        set_layout_foreground_color(layout, &(gcvalues.foreground));
        gtk_paint_layout(style, drawing->window,
                        GTK_STATE_NORMAL, TRUE, NULL, 
                        GTK_WIDGET(tool->canvas->drawing_area), NULL, 
                        text->location.x, text->location.y,
                        layout);
       
        g_object_unref(layout);
        
    }
    

}

static void
text_draw_string(gpaint_text *text) /* draw string permanently */
{
    gpaint_tool *tool = GPAINT_TOOL(text);
    gpaint_drawing *drawing = tool->drawing;
    GdkGCValues gcvalues;
    PangoLayout *layout = gtk_widget_create_pango_layout(GTK_WIDGET(tool->canvas->drawing_area), text->textbuf->str);
  
    GtkStyle *  style = gtk_widget_get_style(GTK_WIDGET(tool->canvas->drawing_area));
    gdk_gc_get_values(drawing->gc, &gcvalues);
   
    set_layout_foreground_color(layout, &(gcvalues.foreground));
    gtk_paint_layout(style, drawing->backing_pixmap,
                    GTK_STATE_NORMAL, TRUE, NULL, 
                    GTK_WIDGET(tool->canvas->drawing_area), NULL, 
                    text->location.x, text->location.y,
                    layout);
    drawing_modified(drawing);
    g_object_unref(layout);
}


static void
text_commit_change(gpaint_tool * tool)
{

    gpaint_text *text = GPAINT_TEXT(tool);
    if (text && is_point_defined(&text->location))
    {
        text_draw_string(text);
        text_clear(text);
    }

}
static gint
text_handle_timeout(gpaint_text *text)
{
    if (text == 0)
        return 0;
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
    (text->flash_state) = 0;
    {
        debug("calling text_draw_cursor()");
        text_draw_cursor(text);
        debug1("after text_draw_cursor() text->flash_state=%d", text->flash_state);
    }
}

static void
text_draw_cursor(gpaint_text *text)
{
    gpaint_tool *tool = GPAINT_TOOL(text);
    gpaint_drawing *drawing = tool->drawing;
    GdkColor white;
    GdkGCValues gcvalues;
    PangoLayout *layout = 0;
    PangoRectangle rect, rect2;
    GtkStyle *  style = gtk_widget_get_style(GTK_WIDGET(tool->canvas->drawing_area));
  
    gdk_gc_get_values(drawing->gc, &gcvalues);
    layout = gtk_widget_create_pango_layout(GTK_WIDGET(tool->canvas->drawing_area), text->textbuf->str);
    
    if (is_point_defined(&text->location))
    {
        pango_layout_get_pixel_size(layout, &(rect.width), &(rect.height));
        
        if (text->max_width < rect.width) text->max_width = rect.width;
        if (text->max_height < rect.height) text->max_height = rect.height;
        
        pango_layout_index_to_pos(layout, g_utf8_strlen(text->textbuf->str, -1), &rect);
        rect.x = PANGO_PIXELS(rect.x);
        rect.y = PANGO_PIXELS(rect.y);
        rect.width = PANGO_PIXELS(rect.width);
        rect.height = PANGO_PIXELS(rect.height);
        
        gdk_draw_pixmap(drawing->window, drawing->gc, drawing->backing_pixmap, 
                         text->location.x + rect.x , text->location.y + rect.y,
                         text->location.x + rect.x , text->location.y + rect.y,
                         TEXT_CURSOR_WIDTH + 2, rect.height + 2);
	        

        gdk_color_white(gdk_rgb_get_cmap(), &white);
        gdk_gc_set_foreground(drawing->gc, &white); 

        gdk_gc_set_function(drawing->gc, GDK_INVERT);
        gdk_gc_set_line_attributes(drawing->gc,
                                   TEXT_CURSOR_WIDTH / 2, 
                                   GDK_LINE_SOLID,
                                   GDK_CAP_PROJECTING,
                                   GDK_JOIN_ROUND);
    
        if (text->flash_state != 0)                 
            gdk_draw_line(drawing->window,
                          drawing->gc,
                          text->location.x + rect.x + TEXT_CURSOR_WIDTH / 2,
                          text->location.y + rect.y,
                          text->location.x + rect.x + TEXT_CURSOR_WIDTH / 2,
                          text->location.y + rect.y + rect.height - 1); 

        gdk_gc_set_function(drawing->gc, gcvalues.function);   
        gdk_gc_set_line_attributes(drawing->gc, 
                                   gcvalues.line_width,
                                   gcvalues.line_style,
                                   gcvalues.cap_style, 
                                   gcvalues.join_style);
        gdk_gc_set_foreground(drawing->gc, &(gcvalues.foreground));
        text->flash_state = !(text->flash_state); 
       
    }
    g_object_unref(layout);
}

static void set_layout_foreground_color(PangoLayout *layout, GdkColor *foreground)
{

    PangoAttrList *attrs = pango_layout_get_attributes(layout);
    PangoAttrColor *color_attr;
    const char *text = pango_layout_get_text(layout);
    unsigned char red, blue, green;
    if (g_utf8_strlen(text, -1) == 0)
        return;
    convert_color(foreground, &red, &green, &blue);
    color_attr = (PangoAttrColor*) pango_attr_foreground_new(color16(red), color16(green), color16(blue));
    
    
    if (attrs == 0)
        attrs = pango_attr_list_new();
    else
        pango_attr_list_ref(attrs);
    color_attr->attr.start_index = 0;
    color_attr->attr.end_index = g_utf8_strlen(text, -1);
    pango_attr_list_change(attrs, (PangoAttribute *) color_attr);
    pango_layout_set_attributes(layout, attrs);
    
    pango_attr_list_unref(attrs);
  
}

   


