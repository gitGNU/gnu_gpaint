/* $Id: about.c,v 1.4 2004/12/29 02:44:02 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003, 2007  Li-Cheng (Andy) Tai
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

#include <libintl.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "version.h"
#include "about.h"
#include "global.h"

#define _(str) gettext(str)

static gchar *authors[] = { "Li-Cheng (Andy) Tai <atai@gnu.org>", 
            "Michael A. Meffie III  <meffiem@neo.rr.com>",
            NULL } ;

                             
                              
static gchar *website = "http://www.gnu.org/software/gpaint";                                         

/*
 * on_about_menu_activate()  [callback]
 *
 * Create the about dialog box.
 */
void
on_about_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gchar *copyright = _("Copyright 2000--2003, 2007  Li-Cheng (Andy) Tai\n"
                         "Copyright 2002 Michael A. Meffie III\n"
                         "\n"
                         "based on xpaint, by David Koblas and Torsten Martinsen\n"
                         "Copyright 1992--1996");
   
    gchar *license = _(
   "This program is free software; you can redistribute it and/or\n"
   "modify it under the terms of the GNU General Public License\n"
   "as published by the Free Software Foundation; either version 3\n"
   "of the License, or (at your option) any later version.\n"
   "\n"                           
   "This program is distributed in the hope that it will be\n"
   "useful, but WITHOUT ANY WARRANTY; without even the implied\n"
   "warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR\n"
   "PURPOSE. See the GNU General Public License for more details.\n"
   "\n"
   "You should have received a copy of the GNU General Public License\n"
   "along with this program.  If not, see <http://www.gnu.org/licenses/>.\n" );
                              
                               
    gchar *comments = _("a small-scale painting program for GNOME,"
                        " the GNU Desktop.");  
                                              
    GtkWidget *widget, *top_level;
    widget = gtk_menu_get_attach_widget (GTK_MENU(GTK_WIDGET(menuitem)->parent));
    top_level = gtk_widget_get_toplevel(widget);
    if (top_level && GTK_WIDGET_TOPLEVEL(top_level)) ;
    else top_level = 0;             

    gtk_show_about_dialog(GTK_WINDOW(top_level),               
                                 "authors",
                                 authors,
                                 "license",
                                 license, 
                                 "copyright", 
                                 copyright,
                                 "comments",
                                 comments,
                                 "name",
                                 PROGRAM_TITLE,
                                 "version", 
                                 VERSION,
                                 "website",
                                 website,
                                 NULL);
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

