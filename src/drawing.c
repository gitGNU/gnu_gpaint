/* $Id: drawing.c,v 1.8 2005/01/27 02:48:45 meffie Exp $
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

#include "drawing.h"
#include "image.h"
#include "image_processing.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <gdk/gdkx.h>  /* for gdk_root_parent */
 	

#include <libintl.h>
#define _(String) gettext (String)


/* default filename */
#define UNTITLED   _("untitled.png")

static void drawing_create_pixmap(gpaint_drawing *drawing, int w, int h);
static void drawing_update_title(gpaint_drawing *drawing);


void drawing_set_filename(gpaint_drawing *drawing, const gchar *filename);

gpaint_drawing*
drawing_new_blank(GtkDrawingArea *drawing_area, GdkGC *gc, gint width, gint height)
{
    gpaint_drawing *drawing = (gpaint_drawing*)g_new0(gpaint_drawing, 1);
    GtkWidget *widget = GTK_WIDGET(drawing_area); 
    g_assert(drawing);
    g_assert(drawing_area);
    g_assert(gc);

    drawing->top_level = gtk_widget_get_toplevel(widget);
    drawing->window = widget->window;       /* reference to the drawing area window */
    drawing->gc = gc;                       /* reference to the gc */
    gdk_gc_ref(gc);

    if (width==0 || height==0)
    {
        gdk_window_get_size(widget->window, &width, &height);
    }

    drawing->filename = g_string_new(UNTITLED); 
    drawing->untitled = TRUE; /* true until until the file is saved */
    drawing_update_title(drawing);

    /* create pixmap, which is stored in the X server's memory */
    drawing_create_pixmap(drawing, width, height);

    /* initial drawing is filled with white to look like a blank page */ 
    gdk_draw_rectangle(
            drawing->backing_pixmap,
            drawing->top_level->style->white_gc,
            TRUE,
            0, 0,
            drawing->width,
            drawing->height );
   
    debug1("new drawing=%p", drawing);
    return drawing;
}

gpaint_drawing*
drawing_new_from_file(GtkDrawingArea *drawing_area, GdkGC *gc, const gchar *filename)
{
    gpaint_drawing *drawing = (gpaint_drawing*)g_new0(gpaint_drawing, 1);
    GtkWidget *widget = GTK_WIDGET(drawing_area); 
    GtkWidget *top_level = gtk_widget_get_toplevel(widget);
    gpaint_image *image;

    g_assert(drawing);
    g_assert(drawing_area);
    g_assert(gc);
    g_assert(filename);
    g_assert(*filename);

    drawing->top_level = gtk_widget_get_toplevel(widget);
    drawing->window = widget->window; /* reference to the drawing area window */
    drawing->gc = gc;                       /* reference to the gc */
    gdk_gc_ref(gc);
    
    /* read the image from the file */
    image = image_new_readfile(filename);
    if (!image)
    {
        debug1("cannot open filename %s", filename);
        return 0;
    }

    drawing->filename = g_string_new(UNTITLED); 
    drawing->untitled = TRUE;
    drawing_set_filename(drawing, filename);
    
    /* create pixmap, which is stored in the X server's memory */
    drawing_create_pixmap(drawing, image_width(image), image_height(image));

    /* render the image on to the backing pixmap */ 
    image_draw_on_pixmap(image, &drawing->backing_pixmap, drawing->gc);
    image_free(image);  /* free the memory buffer */

    debug1("new drawing=%p", drawing);
    return drawing;
}

gpaint_drawing*
drawing_new_from_desktop(GtkDrawingArea *drawing_area, GdkGC *gc)
{
    gpaint_drawing *drawing = (gpaint_drawing*)g_new0(gpaint_drawing, 1);
    GtkWidget *widget = GTK_WIDGET(drawing_area); 
    GtkWidget *top_level = gtk_widget_get_toplevel(widget);
    gpaint_image *image;

    g_assert(drawing);
    g_assert(drawing_area);
    g_assert(gc);

    drawing->top_level = gtk_widget_get_toplevel(widget);
    drawing->window = widget->window;       /* reference to the drawing area window */
    drawing->gc = gc;                       /* reference to the gc */
    gdk_gc_ref(gc);
    
    image = image_new_from_desktop();
    if (!image)
    {
        debug("cannot create image from desktop");
        return 0;
    }

    drawing->filename = g_string_new(UNTITLED); 
    drawing->untitled = TRUE; /* true until until the file is saved */
    drawing->modified = TRUE; /* true because we grabbed the image and haven't saved yet */
    drawing_update_title(drawing);
    
    /* create pixmap, which is stored in the X server's memory */
    drawing_create_pixmap(drawing, image_width(image), image_height(image));

    /* render the image on to the backing pixmap */ 
    image_draw_on_pixmap(image, &drawing->backing_pixmap, drawing->gc);
    image_free(image);  /* free the memory buffer */

    debug1("new drawing=%p", drawing);
    return drawing;
}

void
drawing_destroy(gpaint_drawing *drawing)
{
    g_assert(drawing);
    debug_fn1("drawing=%p", drawing);           
    gdk_gc_unref(drawing->gc);
    gdk_pixmap_unref(drawing->backing_pixmap);
    memset(drawing, 0xBEBE, sizeof(drawing)); /* debugging aid */
    g_free(drawing);
}

int
drawing_in_bounds(gpaint_drawing *drawing, int x, int y)
{
   return (0<=x && x<=drawing->width && 0<=y && y<=drawing->height);
}

void
drawing_copy_to_desktop(gpaint_drawing *drawing)
{
    GdkWindow *root = gdk_window_lookup(gdk_x11_get_default_root_xwindow());
    int width, height;
    
    debug_fn();
    gdk_window_get_size(root, &width, &height);
    gdk_window_set_back_pixmap(root, drawing->backing_pixmap, FALSE);
    gdk_window_clear(root);
}

int
drawing_save(gpaint_drawing *drawing)
{
    gchar *filename = drawing->filename->str;
   
    /* Do not clobber existing file without consent. */
    if (drawing->untitled)
    {
        if (g_file_test(filename, G_FILE_TEST_EXISTS))
        {
                GtkWidget *dialog;
                gint result;
                dialog = gtk_message_dialog_new(
                    GTK_WINDOW(drawing->top_level), 
                    GTK_DIALOG_MODAL, 
                    GTK_MESSAGE_QUESTION, 
                    GTK_BUTTONS_YES_NO, 
                    _("The file %s already exists.\n\nDo you want to overwrite it?"),
                    filename);
                result = gtk_dialog_run(GTK_DIALOG(dialog));
                gtk_widget_destroy(GTK_WIDGET(dialog));
                if (result==GTK_RESPONSE_NO)
                {
                   return FALSE;
                }
         }
    }

        
    return drawing_save_as(drawing, filename);
}

int
drawing_save_as(gpaint_drawing *drawing, const gchar *filename)
{ 
    gpaint_image *image = NULL;
    GError *error = NULL;
    int saved = FALSE; 

    
    g_assert(drawing);
    image = drawing_create_image(drawing);
    saved = image_write(image, filename, &error);
    image_free(image);

    if (saved)
    {
        drawing->modified = FALSE;
        drawing_set_filename(drawing, filename);
    }
    else
    {
        GtkWidget *dialog = gtk_message_dialog_new(
                         GTK_WINDOW(drawing->top_level),
					     GTK_DIALOG_MODAL,
					     GTK_MESSAGE_WARNING,
					     GTK_BUTTONS_OK,
					     _("Failed to save file %s.\n\n%s"), 
                         drawing->filename->str, 
                         (error && error->message) ? error->message : "");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(GTK_WIDGET(dialog)); 
        g_free(error);  /* allocated by gdk-pixbuf library */
    }
    return saved;
}

void
drawing_modified(gpaint_drawing *drawing)
{
    if (!drawing->modified)
    {    
        drawing->modified = TRUE;
        drawing_update_title(drawing);
    }
}

void
drawing_set_filename(gpaint_drawing *drawing, const gchar *filename)
{
    g_assert(drawing);
    g_assert(filename);
    g_assert(drawing->filename);
  
    if (!*filename)
    {
        g_warning("refusing to set empty filename.");
        return;
    }
    debug1("filename=[%s]", filename); 
    g_string_assign(drawing->filename, filename); 
    drawing->untitled = FALSE;
    drawing_update_title(drawing);
}

static void
drawing_update_title(gpaint_drawing *drawing)
{
    GString  *title;
    gchar *basename;
    
    /* Display the filename in the title bar. */
    title = g_string_new(_(""));
    if (drawing->untitled)
    {
       g_string_append(title, _("Untitled"));     
    }
    else
    {
        basename = g_strrstr(drawing->filename->str, "/");
        if (basename)
        {
            g_string_append(title, basename+1);
        }
        else
        {
            g_string_append(title, drawing->filename->str);
        }
    }
    if (drawing->modified)
    {
        g_string_append(title, _(" (modified)"));
    }
    g_string_append(title, _(" - gpaint"));
    gtk_window_set_title(GTK_WINDOW(drawing->top_level), title->str);
    g_string_free(title, TRUE);
}

gpaint_image *
drawing_create_image(gpaint_drawing *drawing)
{
    GdkRectangle area = {0, 0, drawing->width, drawing->height};
    gpaint_image *img = image_new_from_pixmap(drawing->backing_pixmap, area);
    return img;
}

void
drawing_clear(gpaint_drawing *drawing)
{
    GdkGCValues gcvalues;
    gdk_gc_get_values(drawing->gc, &gcvalues);
    gdk_gc_set_foreground(drawing->gc, &(gcvalues.background));
    gdk_draw_rectangle(
            drawing->backing_pixmap,
            drawing->gc,
            TRUE,
            0, 0,
            drawing->width,
            drawing->height );
    gdk_gc_set_foreground(drawing->gc, &(gcvalues.foreground));
}

void
drawing_clear_selection(gpaint_drawing *drawing, gpaint_point_array *points)
{
   GdkGCValues gcvalues;
   GdkRectangle clientrect;

   if (point_array_size(points) > 2)
   {
      clientrect.x = clientrect.y = 0;
      clientrect.width  = drawing->width;
      clientrect.height = drawing->height; 
   
      gdk_gc_get_values(drawing->gc, &gcvalues);
      gdk_gc_set_clip_origin(drawing->gc, 0, 0);
      gdk_gc_set_clip_rectangle(drawing->gc, &clientrect);
   
      gdk_gc_set_foreground(drawing->gc, &(gcvalues.background));
   
      gdk_draw_polygon(
              drawing->backing_pixmap,
              drawing->gc, 
              TRUE,
              point_array_data(points),
              point_array_size(points));
      gdk_draw_polygon(
              drawing->window,
              drawing->gc,
              TRUE,
              point_array_data(points),
              point_array_size(points));
   
      gdk_gc_set_foreground(drawing->gc, &(gcvalues.foreground));
   } 
}


static void
drawing_create_pixmap(gpaint_drawing *drawing, int w, int h)
{
    GdkRectangle rect;
    
    /* create pixmap, which is stored in the X server's memory */
    drawing->width = w;
    drawing->height = h; 
    drawing->backing_pixmap = gdk_pixmap_new(
                                drawing->top_level->window,
                                drawing->width,
                                drawing->height, -1);
    g_assert(drawing->backing_pixmap);
    rect.x = 0;
    rect.y = 0;
    rect.width = drawing->width;
    rect.height = drawing->height;
    gdk_gc_set_clip_origin(drawing->gc, 0, 0);
    gdk_gc_set_clip_rectangle(drawing->gc, &rect);
}

gboolean
drawing_prompt_to_save(gpaint_drawing *drawing)
{
    gboolean cancel = FALSE;
    debug_fn();
    if (drawing->modified)
    {
        GtkWidget *dialog;
        gint result;
        dialog = gtk_message_dialog_new(
               GTK_WINDOW(drawing->top_level), 
               GTK_DIALOG_MODAL, 
               GTK_MESSAGE_QUESTION, 
               GTK_BUTTONS_NONE, 
               _("Do you want to save the changes you made to \"%s\"?\nYour changes will be lost if you don't save them."),
               drawing->filename->str);
        gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_SAVE,GTK_RESPONSE_YES);
        gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL);
        gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_NO,GTK_RESPONSE_NO);
        
        result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(GTK_WIDGET(dialog));
        while (gtk_events_pending())
        {
           gtk_main_iteration();
        }

        switch (result)
        {
            case GTK_RESPONSE_YES:
                if (drawing_save(drawing))
                {
                    cancel = FALSE;
                }
                else
                {
                    cancel = TRUE;
                }
                break;
            case GTK_RESPONSE_CANCEL:
                cancel = TRUE;
                break;
         }
    }
    return cancel;
}

void
drawing_rotate(gpaint_drawing *drawing, double degrees)
{
    gpaint_image *image = drawing_create_image(drawing);
    if (image)
    {
        image_rotate(image, degrees);    
        
        /* copy rotated image on the pixmap */
        gdk_pixmap_unref(drawing->backing_pixmap);
        drawing_create_pixmap(drawing, image_width(image), image_height(image));
        image_draw_on_pixmap(image, &drawing->backing_pixmap, drawing->gc);
        image_free(image);
        drawing_modified(drawing);
    }
}
