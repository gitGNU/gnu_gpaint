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

#include <stdlib.h>
#include <string.h>

#include "print.h"
#include <gtk/gtk.h>
#include <gtk/gtk.h>
#include "debug.h"


#ifdef HAVE_GTK_PRINT

static void begin_print(GtkPrintOperation *print, GtkPrintContext *context, gpointer data)
{

    gtk_print_operation_set_n_pages(print, 1);
}

static void draw_page(GtkPrintOperation *print, GtkPrintContext *context, gint page, gpointer data)
{
    gpaint_image *image = (gpaint_image*) data;
    gdouble pwidth, pheight;
    cairo_surface_t *src_img;
    cairo_t *cr;
    long size;
    int w, h, rowstride, pixelsize;
    unsigned char *buf ;
    g_assert(image);
    
    cr = gtk_print_context_get_cairo_context(context);
    pwidth = gtk_print_context_get_width(context);
    pheight = gtk_print_context_get_height(context);

    size = (h = image_height(image)) * image_rowstride(image);
    w = image_width(image);
    rowstride = image_rowstride(image);
    pixelsize = image_pixelsize(image);
    buf = malloc(sizeof(buf[0]) * size);
    memcpy(buf, image_pixels(image), size);
    {
        int x, y;
        unsigned char c, *t2 = buf, *t;
        for (y = 0; y < h; y++)
        {
            t = t2;
            for (x = 0; x < w; x++)
            {  /* swap R and B; this may be little endian specific */
                c = t[0];
                t[0] = t[2];
                t[2] = c;
                t += pixelsize;
            }
            t2 += rowstride;
        }
    }

    src_img = cairo_image_surface_create_for_data(buf,
                          CAIRO_FORMAT_ARGB32, 
                          w,
                          h, 
                          rowstride);
    cairo_set_source_surface(cr, src_img, 0, 0);
    cairo_paint(cr);                          
                          
    cairo_surface_destroy(src_img);
    free(buf);
}
                      

int
do_print(gpaint_image * image, const gchar * name)
{
    GtkWidget *main_window = NULL; 
    GtkPrintOperation *print;
    static GtkPrintSettings *settings = NULL;
    GtkPrintOperationResult r;
    
    print = gtk_print_operation_new();
    g_assert(print);
    g_signal_connect(print, "draw_page", G_CALLBACK(draw_page), (gpointer) image);
    g_signal_connect(print, "begin_print", G_CALLBACK(begin_print), (gpointer) image);
    
    if (settings) gtk_print_operation_set_print_settings(print, settings); 
    gtk_print_operation_set_job_name(print, name);
    r = gtk_print_operation_run(print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                          GTK_WINDOW(main_window), NULL);
                          
    if (r == GTK_PRINT_OPERATION_RESULT_APPLY)
    {
        if (settings)
            g_object_unref(settings);
        settings = g_object_ref(gtk_print_operation_get_print_settings(print));
    }
    g_object_unref(print);
}

int
do_print_preview(gpaint_image * image, const gchar * name)
{

}



#elif defined(HAVE_GNOME_PRINT)

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
