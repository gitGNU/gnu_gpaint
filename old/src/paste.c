/* $Id: paste.c,v 1.4 2005/01/27 02:50:09 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003, 2007  Li-Cheng (Andy) Tai
 *
 * Authors: Li-Cheng (Andy) Tai <atai@gnu.org>
 *          Michael A. Meffie III <meffiem@neo.rr.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "paste.h"
#include "debug.h"
#include "selection.h"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

typedef struct _gpaint_paste
{
    gpaint_tool   tool;
    gpaint_image *backup_image;
    gpaint_image *overlay_image;
    gpaint_point  last_position;
    gint          offset_x;
    gint          offset_y;
} gpaint_paste;



#define GPAINT_PASTE(tool) ((gpaint_paste*)(tool))

static void paste_destroy(gpaint_tool * tool);
static void paste_select(gpaint_tool * tool);
static void paste_deselect(gpaint_tool * tool);
static void paste_button_press(gpaint_tool * tool, int x, int y);
static void paste_motion(gpaint_tool * tool, int x, int y);
static void paste_button_release(gpaint_tool * tool, int x, int y);
static void paste_key_release(gpaint_tool *tool, GdkEventKey *keyevent);


static void paste_move_overlay(gpaint_paste *paste, int x, int y);
static void paste_draw_overlay(gpaint_paste *paste, int x, int y);
static void paste_erase_overlay(gpaint_paste *paste);


gpaint_tool *
paste_create(const char *name)
{
    gpaint_paste *paste = g_new0(gpaint_paste, 1);
    gpaint_tool *tool = GPAINT_TOOL(paste);

    debug_fn();
    tool->name = name;
    tool->cursor = gdk_cursor_new(GDK_X_CURSOR);
    tool->destroy        = paste_destroy;
    tool->select         = paste_select;
    tool->deselect       = paste_deselect;
    tool->button_press   = paste_button_press;
    tool->button_release = paste_button_release;
    tool->motion         = paste_motion;
    tool->key_release    = paste_key_release;
    return tool;
}

static void
paste_destroy(gpaint_tool *tool)
{
    debug_fn();
    gdk_cursor_destroy(tool->cursor);
    g_free(tool);
}

static GdkAtom*
paste_clipboard_wait_for_targets (gint *n_targets)
{
    GtkClipboard *gtk_clipboard;
  
    gtk_clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
	                                             GDK_SELECTION_CLIPBOARD);

    if (gtk_clipboard)
    {
        GtkSelectionData *data;

        data = gtk_clipboard_wait_for_contents (gtk_clipboard,
                                              gdk_atom_intern ("TARGETS",
                                                               FALSE));
        if (data)
        {
            GdkAtom  *targets;
            gboolean  success;

            success = gtk_selection_data_get_targets (data, &targets, n_targets);

            gtk_selection_data_free (data);

            if (success)
            {
                gint i;

                for (i = 0; i < *n_targets; i++)
                    g_print ("offered type: %s\n", gdk_atom_name (targets[i]));

                g_print ("\n");

                return targets;
            }
        }
    }

    return NULL;
}

static GdkAtom
paste_clipboard_wait_for_buffer (gpaint_canvas *canvas)
{
    gpaint_clipboard *clipboard = canvas_clipboard(canvas);
    GdkAtom *targets;
    gint n_targets;
    GdkAtom result = GDK_NONE;

    targets = paste_clipboard_wait_for_targets (&n_targets);

    if (targets)
    {
	GSList *list;

        for (list = clipboard->pixbuf_formats; list; list = g_slist_next (list))
        {
          GdkPixbufFormat  *format = list->data;
          gchar           **mime_types;
          gchar           **type;

          g_print ("checking pixbuf format '%s'\n",
                   gdk_pixbuf_format_get_name (format));

          mime_types = gdk_pixbuf_format_get_mime_types (format);

          for (type = mime_types; *type; type++)
          {
              gchar   *mime_type = *type;
              GdkAtom  atom      = gdk_atom_intern (mime_type, FALSE);
              gint     i;

              g_print (" - checking mime type '%s'\n", mime_type);

              for (i = 0; i < n_targets; i++)
              {
                  if (targets[i] == atom)
                  {
                      result = atom;
                      break;
                  }
              }

              if (result != GDK_NONE)
                break;
          }

          g_strfreev (mime_types);

          if (result != GDK_NONE)
              break;
        }

        g_free (targets);
    }

    return result;
}

GdkPixbuf*
paste_selection_data_get_pixbuf (GtkSelectionData *selection)
{
    GdkPixbufLoader *loader;
    GdkPixbuf       *pixbuf = NULL;
    GError          *error  = NULL;
  
    g_return_val_if_fail (selection != NULL, NULL);
  
    if ((selection->format != 8) || (selection->length < 1))
    {
        g_warning ("Received invalid image data!");
        return NULL;
    }
  
    loader = gdk_pixbuf_loader_new ();
  
    if (gdk_pixbuf_loader_write (loader,
                                 selection->data, selection->length, &error) &&
                                 gdk_pixbuf_loader_close (loader, &error))
    {
        pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
        g_object_ref (pixbuf);
    }
    else
    {
        g_warning ("%s: %s", G_STRFUNC, error->message);
        g_error_free (error);
    }
  
    g_object_unref (loader);
  
    return pixbuf;
}

static void
paste_select(gpaint_tool *tool)
{
    gpaint_paste *paste = GPAINT_PASTE(tool);
    gpaint_canvas *canvas   = tool->canvas;
    gpaint_drawing *drawing = tool->drawing;
    gpaint_clipboard *clipboard = canvas_clipboard(tool->canvas);
    GtkClipboard *gtk_clipboard;
 
    /* make a backup of the current drawing */
    paste->backup_image = drawing_create_image(drawing);
    
 /* check for image data on real clipboard */
     gtk_clipboard = gtk_clipboard_get_for_display (gdk_display_get_default(),
 							GDK_SELECTION_CLIPBOARD);
     if (gtk_clipboard && gtk_clipboard_get_owner (gtk_clipboard) != G_OBJECT (canvas))
     {
 	GdkAtom atom = paste_clipboard_wait_for_buffer (canvas);
 
 	if (atom != GDK_NONE)
 	{
 	    GtkSelectionData *data;
 
 	    data = gtk_clipboard_wait_for_contents (gtk_clipboard, atom);
 
 	    if (data)
 	    {
 		GdkPixbuf *pixbuf = paste_selection_data_get_pixbuf (data);
 
 		gtk_selection_data_free (data);
 
 		if (pixbuf)
 		{
 		    gpaint_point_array *clipboard_points;
 		    int width, height;
 		    GdkPixbuf** pixbufp = &pixbuf;
 
 		    if (clipboard->image)
 		    {
 			image_free(clipboard->image);
 		    }
 
 		    width = gdk_pixbuf_get_width (pixbuf);
 		    height = gdk_pixbuf_get_height (pixbuf);
 		    clipboard->image = image_new_copy ((gpaint_image*)pixbufp);
 
 /* define clipboard points */
 		    point_array_remove_all (clipboard->points);
 		    point_array_append (clipboard->points, 0, 0);
 		    point_array_append (clipboard->points, width, 0);
 		    point_array_append (clipboard->points, width, height);
 		    point_array_append (clipboard->points, 0, height);
 		    point_array_append (clipboard->points, 0, 0);
 
 		    g_object_unref (pixbuf);
 		}
 	    }
 	}
     }
 
 /* check for invalid paste */
     if (!point_array_size(clipboard->points))
 	return;
     
    /* make a copy of the current clipboad contents and selection points */
    paste->overlay_image   = image_new_copy(clipboard->image);
    selection_set_points(canvas->selection, clipboard->points);
    /* draw the overlay at the top left corner of the drawing */
    paste_move_overlay(paste, 0, 0);   
}

static void
paste_deselect(gpaint_tool *tool)
{
    gpaint_paste *paste = GPAINT_PASTE(tool);
    selection_remove_points(tool->canvas->selection);
    if (paste->overlay_image)
    {
    image_free(paste->overlay_image);
    }
    image_free(paste->backup_image);
    paste->offset_x = 0;
    paste->offset_y = 0;
}

static void
paste_button_press(gpaint_tool *tool, int x, int y)
{
    gpaint_paste *paste = GPAINT_PASTE(tool);
    GdkRectangle overlay;
    gpaint_point_array *points = selection_points(tool->canvas->selection);
    
    debug_fn();
    
    /* End paste mode if the user clicks outside the 
     * paste image rectangle */
    point_array_bounding_rectangle(points, &overlay);
    if ( (x < overlay.x) || (overlay.x + overlay.width  < x ) ||
         (y < overlay.y) || (overlay.y + overlay.height < y))
    {
        canvas_end_paste_mode(tool->canvas);
    }
    else
    {
        /* Capture the offset to the corner of the bounding rectangle
         * to drag the image into place. */
        paste->offset_x = overlay.x - x;
        paste->offset_y = overlay.y - y;
    }
}


static void
paste_motion(gpaint_tool * tool, int x, int y)
{
    gpaint_paste *paste = GPAINT_PASTE(tool);
    debug_fn();
    
    /*  Drag the image to be pasted into place. */
    paste_move_overlay(paste, x, y);
}

static void
paste_button_release(gpaint_tool * tool, int x, int y)
{
    gpaint_paste *paste = GPAINT_PASTE(tool);
    debug_fn();
    paste_move_overlay(paste, x, y);
}

static void
paste_key_release(gpaint_tool *tool, GdkEventKey *keyevent)
{
    debug_fn();
    switch (keyevent->keyval)
    {
        case GDK_Escape:
            canvas_end_paste_mode(tool->canvas);
            break;
   
        case GDK_Return:
            /* make what's in the pixmap into the rgb buffer */
            // ?? image_buf_pixmap_to_rgbbuf(ibuf, 0);   
            break;
    }
}

static void
paste_move_overlay(gpaint_paste *paste, int x, int y)
{
    gpaint_selection *selection = GPAINT_TOOL(paste)->canvas->selection;

    /* Position with respect to the paste image rectangle. */
    x += paste->offset_x;
    y += paste->offset_y;
    
    selection_disable_flash(selection);
    paste_erase_overlay(paste);
    paste_draw_overlay(paste, x, y);
    selection_move(selection, x, y);
    selection_enable_flash(selection);
    set_point(&paste->last_position, x, y); 
}   

static void
paste_erase_overlay(gpaint_paste *paste)
{
    GdkRectangle overlay_rect;
    gpaint_drawing *drawing = GPAINT_TOOL(paste)->drawing;
    gpaint_canvas  *canvas  = GPAINT_TOOL(paste)->canvas;
    int x, y;

    if (!is_point_defined(&paste->last_position))
    {
        return;
    }
   
    x = paste->last_position.x;
    y = paste->last_position.y;
   
    if (!paste->overlay_image)
    {
       gdk_beep();
       return;
    }

    if (x < 0) 
    {
        x = 0;
    }
    if (y < 0)
    {
        y = 0;
    }
    
    overlay_rect.x = x;
    overlay_rect.y = y;
    overlay_rect.width  = image_width(paste->overlay_image);
    overlay_rect.height = image_height(paste->overlay_image);

    if (x + overlay_rect.width >= drawing->width)
    {
        overlay_rect.width = drawing->width - x;
    }
   
    if (y + overlay_rect.height >= drawing->height)
    {
        overlay_rect.height = drawing->height - y;
    }
      
    if ((drawing->width - x < 0) || (drawing->height - y < 0))
    {
       return;
    }
    if ((x + overlay_rect.width < 0) || (y + overlay_rect.height < 0))   
    {
       return;
    }
  
    /* Restore the original image to erase the overlay. */
    image_draw_region_on_pixmap(
            paste->backup_image,
            overlay_rect,
            &(drawing->backing_pixmap),
            drawing->gc);
   
    gtk_widget_queue_draw_area(
            GTK_WIDGET(canvas->drawing_area),
            overlay_rect.x, overlay_rect.y,
            overlay_rect.width, overlay_rect.height);
}   


static void
paste_draw_overlay(gpaint_paste *paste, int x, int y)
{
    gpaint_drawing *drawing = GPAINT_TOOL(paste)->drawing;
    gpaint_canvas  *canvas  = GPAINT_TOOL(paste)->canvas;
    GdkRectangle rect;
   
    if (!paste->overlay_image)
    {
       gdk_beep();
       return;
    }
   
    rect.x = x;
    rect.y = y;
    rect.width  = image_width(paste->overlay_image);
    rect.height = image_height(paste->overlay_image);
   
    if (x + rect.width >= drawing->width)
    {
        rect.width = drawing->width - x;
    }
    if (y + rect.height >= drawing->height)
    {
        rect.height = drawing->height - y;
    }
    
    if ((drawing->width - x < 0) || (drawing->height - y < 0))
    {
       return;
    }  
    if ((x + rect.width < 0) || (y + rect.height < 0))
    {
       return;
    }
  
    image_render(paste->overlay_image, drawing->backing_pixmap, 0, 0, rect); 

    gtk_widget_queue_draw_area(
            GTK_WIDGET(canvas->drawing_area),
            rect.x, rect.y, rect.width, rect.height);

    gtk_widget_draw(GTK_WIDGET(canvas->drawing_area), &rect);

    drawing_modified(drawing);
}



