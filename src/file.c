/* $Id: file.c,v 1.5 2005/01/27 02:54:37 meffie Exp $
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

#include "canvas.h"
#include "image.h"
#include "debug.h"
#include "ui.h"
#include "support.h"

#define MAX_WIDTH  4096
#define MAX_HEIGHT 4096

typedef struct _gpaint_new_drawing_dialog
{
    GtkWidget     *window;
    GtkWidget     *width_entry;
    GtkWidget     *height_entry;
    gpaint_canvas *canvas;
} gpaint_new_drawing_dialog;

/*
 * A filename for the image was selected. Save the name and the file.
 */
static void
on_save_filename_selected(GtkFileSelection *dialog)
{
    gpaint_canvas *canvas;
    const gchar *filename;
    gpaint_image* image;
    
    debug_fn();
    filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(dialog));
    debug1("filename: %s", filename);  
    canvas = (gpaint_canvas*)gtk_object_get_user_data(GTK_OBJECT(dialog));
    
    /* dismiss the dialog. flush gtk events to avoid double select */
    gtk_widget_destroy(GTK_WIDGET(dialog));
    while (gtk_events_pending())
    {
        gtk_main_iteration();
    }

    /* Do not clobber existing file without consent. */
    if (g_file_test(filename, G_FILE_TEST_EXISTS))
    {
        GtkWidget *dialog;
        gint result;
        dialog = gtk_message_dialog_new(
                    GTK_WINDOW(canvas->top_level), 
                    GTK_DIALOG_MODAL, 
                    GTK_MESSAGE_QUESTION, 
                    GTK_BUTTONS_YES_NO, 
                    _("The file %s already exists.\n\nDo you want to overwrite it?"),
                    filename);
        result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(GTK_WIDGET(dialog));
        if (result==GTK_RESPONSE_NO)
        {
            return;
        }
    }
   
    drawing_save_as(canvas->drawing, filename);
}

/*
 * Open the image file selected in the open file dialog.
 */
static void 
on_open_filename_selected(GtkFileSelection *dialog)
{
    GdkRectangle drawable_rect;
    gpaint_canvas *canvas = (gpaint_canvas*)gtk_object_get_user_data(GTK_OBJECT(dialog));
    gpaint_drawing *new_drawing;

    /* save the result before the widget is destroyed */
    GString *filename = g_string_new(gtk_file_selection_get_filename(GTK_FILE_SELECTION(dialog)));
    
    /* dismiss the dialog. flush gtk events to avoid double select */
    gtk_widget_destroy(GTK_WIDGET(dialog));
    while (gtk_events_pending())
    {
        gtk_main_iteration();
    }
  
    new_drawing = drawing_new_from_file(canvas->drawing_area, canvas->gc, filename->str);
    if (new_drawing)
    {
        canvas_set_drawing(canvas, new_drawing);
    }
    else
    {
        GtkWidget *msgbox;
        msgbox = gnome_message_box_new(
                _("Cannot open file."), 
                GNOME_MESSAGE_BOX_ERROR, 
                _(" OK "), NULL);
        gtk_window_set_modal(GTK_WINDOW(msgbox), TRUE);
        gtk_window_set_transient_for (GTK_WINDOW(msgbox), GTK_WINDOW(canvas->top_level));
        gtk_window_set_position(GTK_WINDOW(msgbox), GTK_WIN_POS_CENTER);
        gnome_dialog_run(GNOME_DIALOG(msgbox));
    }
    g_string_free(filename, TRUE);
}

void
file_save_as_dialog(gpaint_canvas *canvas)
{
    GtkWidget *dialog;
    gpaint_drawing *drawing = canvas->drawing;
    
    dialog = gtk_file_selection_new(_("Save as"));
    g_assert(dialog);

    /* Set as modal, and mark it as a transient dialog */
    gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(canvas->top_level));
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    
    g_assert(drawing);
    g_assert(drawing->filename); 
    g_assert(drawing->filename->len);
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(dialog), drawing->filename->str);
    
    gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(dialog)->ok_button),
                              "clicked",
                              GTK_SIGNAL_FUNC(on_save_filename_selected),
                              GTK_OBJECT(dialog));
    
    gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(dialog)->cancel_button),
                              "clicked",
                              GTK_SIGNAL_FUNC(gtk_widget_destroy),
                              GTK_OBJECT(dialog));
    
    gtk_object_set_user_data(GTK_OBJECT(dialog), canvas);
    gtk_widget_show(dialog);
}

void
file_open_dialog(gpaint_canvas *canvas)
{
    GtkWidget *dialog;
    gchar *dir;
    GString *filename;
    gpaint_drawing *drawing;
     
    g_assert(canvas);
    dialog = gtk_file_selection_new(_("Open image"));
    g_assert(dialog);
   
    gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(canvas->top_level));
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

    drawing = canvas->drawing;
    if (drawing->untitled)
    {
        dir = g_get_current_dir();
    }
    else
    {
        g_assert(drawing->filename);
        g_assert(drawing->filename->str);
        dir = g_dirname(drawing->filename->str);
    }
    filename = g_string_new(dir);
    g_string_append_c(filename, G_DIR_SEPARATOR);
    g_string_append(filename, "*.*");
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(dialog), filename->str);
    g_string_free(filename, TRUE);
    g_free(dir);
   
    gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(dialog)->ok_button),
                              "clicked",
                              GTK_SIGNAL_FUNC(on_open_filename_selected),
                              GTK_OBJECT(dialog));
    
    gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(dialog)->cancel_button),
                              "clicked",
                              GTK_SIGNAL_FUNC(gtk_widget_destroy),
                              GTK_OBJECT(dialog));
    
    gtk_object_set_user_data(GTK_OBJECT(dialog), canvas);
    gtk_widget_show(dialog);
}

void
on_new_canvas_ok_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
    gpaint_new_drawing_dialog *dialog;
    char *tmp;
    int width = 0, height = 0;
   
    dialog = (gpaint_new_drawing_dialog*)gtk_object_get_user_data(GTK_OBJECT(button));
    tmp = gtk_editable_get_chars(GTK_EDITABLE(dialog->width_entry), 0, -1);
    sscanf(tmp, "%d", &width);
    g_free(tmp);
    tmp = gtk_editable_get_chars(GTK_EDITABLE(dialog->height_entry), 0, -1);
    sscanf(tmp, "%d", &height);
    g_free(tmp);
    if (0<width && width<=MAX_WIDTH && 0<height && height<=MAX_HEIGHT)
    {
        gpaint_canvas *canvas = dialog->canvas;
        gpaint_drawing *new_drawing = drawing_new_blank(canvas->drawing_area, canvas->gc, width, height);
        if (new_drawing)
        {
            canvas_set_drawing(canvas, new_drawing);
        }
        gtk_widget_destroy(dialog->window);
    }
    else
    {
        GtkWidget *msgbox = gnome_message_box_new(
              _("Invalid width or height values"), GNOME_MESSAGE_BOX_ERROR, _(" OK "), NULL);
        gtk_window_set_modal(GTK_WINDOW(msgbox), TRUE);
        gtk_window_set_transient_for (GTK_WINDOW(msgbox), GTK_WINDOW(dialog->window));
        gtk_window_set_position(GTK_WINDOW(msgbox), GTK_WIN_POS_CENTER);
        gnome_dialog_run(GNOME_DIALOG(msgbox));
   }   
}

void
on_new_canvas_cancel_button_clicked    (GtkButton       *button,
                                        gpointer         user_data)
{
    gpaint_new_drawing_dialog *dialog;
    dialog = (gpaint_new_drawing_dialog*)gtk_object_get_user_data(GTK_OBJECT(button));
    gtk_widget_destroy(dialog->window);
}

void
on_new_canvas_window_destroy           (GtkObject       *object,
                                        gpointer         user_data)
{
    gpaint_new_drawing_dialog *dialog;
    debug_fn();
    dialog = (gpaint_new_drawing_dialog*)gtk_object_get_user_data(object);
    g_free(dialog);
}

void
file_new_dialog(gpaint_canvas *canvas)
{
    gpaint_new_drawing_dialog *dialog;
    char tmp[100];
    gint position = 0;
    GtkWidget *ok_button;
    GtkWidget *cancel_button;
 
    dialog = g_new0(gpaint_new_drawing_dialog, 1);
    g_assert(dialog);
    dialog->window = create_new_canvas_window();
    dialog->canvas = canvas;
    dialog->width_entry  = lookup_widget(dialog->window, "new_canvas_width_text_entry");
    dialog->height_entry = lookup_widget(dialog->window, "new_canvas_height_text_entry");
    sprintf(tmp, "%d", canvas->drawing->width);
    gtk_editable_insert_text(GTK_EDITABLE(dialog->width_entry), tmp, strlen(tmp), &position);
    position = 0; 
    sprintf(tmp, "%d", canvas->drawing->height);
    gtk_editable_insert_text(GTK_EDITABLE(dialog->height_entry), tmp, strlen(tmp), &position);

    /* Save the dialog state for the button click callbacks. */
    ok_button     = lookup_widget(dialog->window, "new_canvas_ok_button");
    cancel_button = lookup_widget(dialog->window, "new_canvas_cancel_button");
    gtk_object_set_user_data(GTK_OBJECT(ok_button), dialog);
    gtk_object_set_user_data(GTK_OBJECT(cancel_button), dialog);
    gtk_object_set_user_data(GTK_OBJECT(dialog->window), dialog);

    gtk_widget_show(dialog->window);
}


