/* $Id: print.c,v 1.10 2004/12/29 02:44:02 meffie Exp $
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

/* Based on testprint.c from gnome-print 0.25 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "print.h"
#include <gtk/gtk.h>
#include <gtk/gtk.h>
#include "debug.h"

#ifdef HAVE_GNOME_PRINT

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprintui/gnome-print-job-preview.h>

static int print_on_context(gpaint_image * image, const gchar * name,
                            GnomePrintContext * pc);


int
do_print(gpaint_image * image, const gchar * name)
{
    GnomePrintConfig *cfg = NULL;
    GnomePrintContext *ctx = NULL;
    GnomePrintJob *job = NULL;
    int copies = 1;
    int collate = 0;
    int do_preview = 0;
    int do_print = 0;
    int reply = 0;
    
    GnomePrintDialog *gpd = NULL;
    
    cfg = gnome_print_config_default();
    job = gnome_print_job_new(cfg);
    gpd = GNOME_PRINT_DIALOG(gnome_print_dialog_new(job, "Print", 0));
    gnome_print_dialog_set_copies(gpd, copies, collate);
    
    reply = gtk_dialog_run(GTK_DIALOG(gpd));
    debug1("print dialog response is %d", reply);
    switch (reply)
    {
    case GNOME_PRINT_DIALOG_RESPONSE_PRINT:
        debug("print dialog response print");
        do_preview = 0;
        do_print = 1;
        break;
    case GNOME_PRINT_DIALOG_RESPONSE_PREVIEW:
        debug("print dialog response preview");
        do_preview = 1;
        break;
    case GNOME_PRINT_DIALOG_RESPONSE_CANCEL:
        debug("print dialog reponse cancel");
        break;
    }

    gtk_widget_destroy(GTK_WIDGET(gpd)); 
    
    /* transfer dialog data to output context */
    ctx = gnome_print_job_get_context(job);
    print_on_context(image, name, ctx);
    gnome_print_job_close(job);
    
    if (do_preview)
    {
        GnomePrintJobPreview *pmp;
        pmp = GNOME_PRINT_JOB_PREVIEW(gnome_print_job_preview_new(job, "Print Preview"));
        g_signal_connect((gpointer)pmp, "destroy", G_CALLBACK(gtk_widget_destroy), (gpointer)pmp);
        gtk_window_set_modal(GTK_WINDOW(pmp), TRUE);
        gtk_widget_show(GTK_WIDGET(pmp));
    }
    else if (do_print)
    {
        gnome_print_job_print(job);
    }

    // todo: unref objects? 
    return 1;
}

int
do_print_preview(gpaint_image * image, const gchar * name)
{
    GnomePrintConfig *cfg = NULL;
    GnomePrintJob *job = NULL;
    GnomePrintJobPreview *pmp = NULL;
    GnomePrintContext *ctx = NULL;
        
    cfg = gnome_print_config_default();
    job = gnome_print_job_new(cfg);
    
    /* transfer dialog data to output context */
    ctx = gnome_print_job_get_context(job);
    print_on_context(image, name, ctx);
    gnome_print_job_close(job);
    
    pmp = GNOME_PRINT_JOB_PREVIEW(gnome_print_job_preview_new(job, "Print Preview"));
    g_signal_connect((gpointer)pmp, "destroy", G_CALLBACK(gtk_widget_destroy), (gpointer)pmp);
    gtk_window_set_modal(GTK_WINDOW(pmp), TRUE);
    // gtk_window_set_transient_for(GTK_WINDOW(?),GTK_WINDOW(?));
    gtk_widget_show(GTK_WIDGET(pmp));

/*
    gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
    gtk_widget_set_default_visual(gdk_rgb_get_visual());

    toplevel = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_usize(toplevel, 700, 700);
    sw = gtk_scrolled_window_new(NULL, NULL);
    canvas = gnome_canvas_new_aa();
    gtk_container_add(GTK_CONTAINER(toplevel), sw);
    gtk_container_add(GTK_CONTAINER(sw), canvas);

    cfg = gnome_print_config_default();
    pc = gnome_print_preview_new( cfg, GNOME_CANVAS(canvas));

    gtk_widget_show_all(toplevel);
    gtk_window_set_modal(GTK_WINDOW(toplevel), TRUE);
    print_on_context(image, name, pc);
    gnome_print_context_close(pc);
    gtk_main();
    gtk_object_unref(GTK_OBJECT(pc));
    
*/

    
    return 1;
}

static int
print_on_context(gpaint_image * image,
                 const gchar * name, GnomePrintContext * pc)
{
    double matrix[6] = { 1, 0, 0, 1, 0, 0 };
    gnome_print_beginpage(pc, name);

    gnome_print_concat(pc, matrix);
    gnome_print_translate(pc, 0, 0);
    gnome_print_scale(pc, image_width(image), image_height(image));

    gnome_print_rgbaimage(pc,
                          image_pixels(image),
                          image_width(image),
                          image_height(image), image_rowstride(image));

    gnome_print_showpage(pc);
    return 1;
}

#else

int
do_print(gpaint_image * image, const gchar * name)
{
    g_warning("Printing feature is not available.");
}

int
do_print_preview(gpaint_image * image, const gchar * name)
{
    g_warning("Printing feature is not available.");
}

#endif
