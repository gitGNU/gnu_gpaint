/* $Id: menu.c,v 1.10 2005/01/27 02:54:21 meffie Exp $
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

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "canvas.h"
#include "image.h"
#include "image_processing.h"
#include "tool_palette.h"
#include "menu.h"
#include "print.h"
#include "file.h"
#include "text.h"

#include <gtk/gtk.h>


/*
 * Save the image to a file. Show the file selection dialog if the
 * image is untitled.
 */
void
on_save_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem)); 
    gpaint_drawing *drawing = canvas->drawing;
    
    canvas_begin_busy_cursor(canvas);
    if (drawing->untitled)
    {
        file_save_as_dialog(canvas);
    }
    else
    {
        drawing_save(drawing);
    }
    canvas_end_busy_cursor(canvas);
}

/*
 * Same as the Save menu item.
 */
void
on_save_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(button)); 
    gpaint_drawing *drawing = canvas->drawing;

    canvas_begin_busy_cursor(canvas);
    if (drawing->untitled)
    {
        file_save_as_dialog(canvas);
    }
    else
    {
        drawing_save(drawing);
    }
    canvas_end_busy_cursor(canvas);
}

/*
 * Select a filename to save the image.
 */
void
on_save_as_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    canvas_begin_busy_cursor(canvas);
    file_save_as_dialog(canvas);
    canvas_end_busy_cursor(canvas);
}

/*
 * Same as save as menu item.
 */
void
on_save_as_button_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(button));
    canvas_begin_busy_cursor(canvas);
    file_save_as_dialog(canvas);
    canvas_end_busy_cursor(canvas);
}



void
on_new_file_menu_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    /* Give the user a chance to save their work before we create
     * a new drawing! */
    if (!drawing_prompt_to_save(canvas->drawing))
    {
        file_new_dialog(canvas);
    }
}

void
on_open_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    canvas_begin_busy_cursor(canvas);
    if (!drawing_prompt_to_save(canvas->drawing))
    {
        file_open_dialog(canvas);
    }
    canvas_end_busy_cursor(canvas);
}

void
on_open_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(button));
    canvas_begin_busy_cursor(canvas);
    if (!drawing_prompt_to_save(canvas->drawing))
    {
        file_open_dialog(canvas);
    }
    canvas_end_busy_cursor(canvas);
}

void
on_cut_menu_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    canvas_cut(canvas);
}

void
on_copy_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    canvas_copy(canvas);
}

void
on_paste_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    canvas_begin_paste_mode(canvas_lookup(GTK_WIDGET(menuitem)));
}


void
on_clear_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    canvas_clear(canvas);
}

void
on_select_all_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    canvas_select_all(canvas);
}

static void
on_print(GtkWidget *widget, gboolean preview)
{
    gpaint_canvas *canvas = canvas_lookup(widget);
    gpaint_drawing *drawing = canvas->drawing;
    gpaint_image *image;
    
    debug_fn();
    canvas_begin_busy_cursor(canvas);
    image = drawing_create_image(drawing);
    if (preview)
    {
        debug("starting do_print_preview()");
        do_print_preview(image, drawing->filename->str);
        debug("done do_print_preview()");
    }
    else
    {
        do_print(image, drawing->filename->str);
    }
    image_free(image);
    canvas_end_busy_cursor(canvas);
}

void
on_print_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    on_print(GTK_WIDGET(menuitem), FALSE);
}


void
on_print_button_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    on_print(GTK_WIDGET(button), FALSE);
}

void
on_print_preview_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    on_print(GTK_WIDGET(menuitem), TRUE);
}

void
on_new_button_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(button));

    /* Give the user a chance to save their work before we create
     * a new drawing! */
    if (!drawing_prompt_to_save(canvas->drawing))
    {
        file_new_dialog(canvas);
    }
}

/*
 * Create an X client side image from the backing pixmap and
 * pass it to the named image processing function, then
 * render the processed image. This function is called for each 
 * image effect menu item. The name of the menu item widget
 * is used to select the image processing function.
 */
void
on_image_effect_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    const gchar *name = gtk_widget_get_name(GTK_WIDGET(menuitem));
    gpaint_drawing *drawing = canvas->drawing;
    gpaint_image *input;
    gpaint_image *output;
    ImageEffect effect;

    effect = image_lookup_effect(name);
    g_assert(effect);
  
    input  = drawing_create_image(drawing);
    output = image_new(drawing->width, drawing->height);
    
    (*effect)(input, output);
    
    image_draw_on_pixmap(output, &drawing->backing_pixmap, drawing->gc);
    drawing_modified(drawing);
    image_free(input);
    image_free(output);
   
    canvas_redraw(canvas);
}

void
on_line_width_combo_combo_entry_changed(GtkEditable     *editable,
                                        gpointer         user_data)
{
    char *tmp;
    int t;
    GdkGCValues gcvalues;
    gpaint_canvas *canvas;
    gpaint_drawing *drawing;
    int position = 0;
   
    debug_fn();
    canvas = canvas_lookup(GTK_WIDGET(editable));  
    drawing = canvas->drawing;
    
    tmp = gtk_editable_get_chars(editable, 0, -1);
    g_assert(tmp);
    if (strlen(tmp) > 0) {
        sscanf(tmp, "%d", &t);
        g_free(tmp);
        if (t > 0) {
            gdk_gc_get_values(drawing->gc, &gcvalues);
            gdk_gc_set_line_attributes(drawing->gc, t, gcvalues.line_style, gcvalues.cap_style, gcvalues.join_style);

            /* force the line width entry widget to give up focus */
            gtk_widget_grab_focus(GTK_WIDGET(canvas->drawing_area));
        } else {
            gdk_gc_get_values(drawing->gc, &gcvalues);
            tmp = g_strdup_printf("%d", gcvalues.line_width);
            gtk_editable_delete_text(editable, 0, -1);
            gtk_editable_insert_text(editable, tmp, strlen(tmp), &position);
            gtk_editable_set_position(editable, -1);
            g_free(tmp);
        }
    } else {
      g_free(tmp);
    }
}

void
on_fontpicker_realize                  (GtkWidget       *widget,
                                        gpointer         user_data)
{
}

void
on_fontpicker_font_set                 (GtkFontButton *fontpicker,
                                        gchar *arg1,
                                        gpointer         user_data)
{
    gpaint_tool *tool = tool_palette_get_tool(GTK_WIDGET(fontpicker), "text");

    const gchar* name = gtk_font_button_get_font_name(fontpicker);
    debug1("font name = %s", name);
  

    if (tool && tool->attribute)
    {
        (*tool->attribute)(tool, GpaintFont, (gpointer*)name);
    }
}

void
on_fontpicker_map                      (GtkWidget       *widget,
                                        gpointer         user_data)
{ 
}

gboolean
on_fontpicker_map_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
/*
   GtkWidget *window = widget_get_toplevel_parent(GTK_WIDGET(widget)); 
   image_buf *ibuf = widget_get_image(widget);
   gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(ibuf->window));
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_window_set_modal (GTK_WINDOW(window), TRUE);
 */
  return FALSE;
}

void
on_font_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
}

void
on_bold_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
}


void
on_italic_button_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
}


void
on_underline_button_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
}


void
on_get_desktop_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    /* Give the user a chance to save their work before we create
     * a new drawing! */
    if (!drawing_prompt_to_save(canvas->drawing))
    {
        gpaint_drawing *drawing = drawing_new_from_desktop(canvas->drawing_area, canvas->gc);
        if (drawing)
        {
            canvas_set_drawing(canvas, drawing);
        }
    }
}

void
on_change_background_menu_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    drawing_copy_to_desktop(canvas->drawing);
}

void
on_set_as_background_centered_activate (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    drawing_copy_to_desktop(canvas->drawing);
}


void
on_set_as_background_titled_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    drawing_copy_to_desktop(canvas->drawing);
}


void
on_flip_x_axis_menu_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    gpaint_drawing *drawing = canvas->drawing;
    gpaint_image *image;

    canvas_focus_lost(canvas);
    image = drawing_create_image(drawing);
    image_flip_x(image);
    image_draw_on_pixmap(image, &drawing->backing_pixmap, drawing->gc);
    image_free(image);
    canvas_redraw(canvas);
    canvas_focus_gained(canvas);
}


void
on_flip_y_axis_menu_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    gpaint_drawing *drawing = canvas->drawing;
    gpaint_image *image;

    canvas_focus_lost(canvas);
    image = drawing_create_image(drawing);
    image_flip_y(image);
    image_draw_on_pixmap(image, &drawing->backing_pixmap, drawing->gc);
    image_free(image);
    canvas_redraw(canvas);
    canvas_focus_gained(canvas);
}

void
on_rotate_menu_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    const gchar *name = gtk_widget_get_name(GTK_WIDGET(menuitem));
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(menuitem));
    char sign;
    int degrees;

    sscanf(name, "rotate_%c%d_menu", &sign, &degrees);
    debug2("sign = %c  degrees = %d", sign, degrees);
    if (sign=='n')
    {
        degrees *= -1;
    }
    canvas_focus_lost(canvas);
    drawing_rotate(canvas->drawing, degrees);
    canvas_resize(canvas);
    canvas_focus_gained(canvas);
}

