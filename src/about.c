/* $Id: about.c,v 1.4 2004/12/29 02:44:02 meffie Exp $
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

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "ui.h"
#include "version.h"
#include "about.h"

/*
 * on_about_menu_activate()  [callback]
 *
 * Create the about dialog box.
 */
void
on_about_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   GtkWidget *about = create_about_dialog ();
   gtk_widget_show(about);
}

/*
 * on_about_dialog_pixmap_realize()  [callback]
 *
 * Display the gpaint logo graphic in the about dialog.
 */
void
on_about_dialog_pixmap_realize         (GtkWidget       *widget,
                                        gpointer         user_data)
{
    gchar     **about_pict_xpm = (gchar**)user_data;
    GdkPixmap *gdkpixmap;
    GdkBitmap *mask;
    GtkWidget *dialog;
   
    /* Create the logo pixmap */
    if (user_data)
    {
        dialog = gtk_widget_get_toplevel(widget);
        g_assert(dialog);
        gdkpixmap = gdk_pixmap_create_from_xpm_d(
                        dialog->window,
                        &mask,
                        NULL,
                        about_pict_xpm);
        g_assert(gdkpixmap);

        /* Load the pixmap into the widget */
        gtk_image_set_from_pixmap(GTK_IMAGE(widget), gdkpixmap, mask);
        gdk_pixmap_unref(gdkpixmap);
        gdk_pixmap_unref(mask);
    }
}

/*
 * on_about_dialog_version_label_realize()  [callback]
 *
 * Display the program title and version number in the about box.
 * The version number is set by the project's configure script.
 */
void
on_about_dialog_version_label_realize  (GtkWidget       *widget,
                                        gpointer         user_data)
{
    char tmp[200];
    sprintf(tmp, "%s %s", PROGRAM_TITLE, VERSION_STRING);
    gtk_label_set(GTK_LABEL(widget), tmp);
}

/*
 * on_about_gnome_activate()  [callback]
 *
 * Display the gnome web site with the system's web browser (default
 * is mozilla).
 */
void
on_about_gnome_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gnome_url_show("http://www.gnome.org/");
}


/*
 * on_about_gnu_activate()  [callback]
 *
 * Display the gnu web site with the system's web browser (default
 * is mozilla).
 */
void
on_about_gnu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gnome_url_show("http://www.gnu.org/");
}

/*
 * on_about_dialog_ok_button_clicked      
 *
 * Dismiss the about dialog.
 */
void
on_about_dialog_ok_button_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
   gtk_widget_destroy(window);
}
