
/*
   based on testprint.c from gnome-print 0.25
   
    Copyright 2000  Li-Cheng (Andy) Tai

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "print.h"
#include <gnome.h>
#include <gtk/gtk.h>

#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-meta.h>
#include <libgnomeprint/gnome-print-preview.h>
#include <libgnomeprint/gnome-print-pixbuf.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-printer-dialog.h>
#include <libgnomeprint/gnome-print-master.h>
#include <libgnomeprint/gnome-print-master-preview.h>
#include <libgnomeprint/gnome-print-dialog.h>

int do_print(image_buf *ibuf)
{
   GnomePrintContext *pc;
   GnomePrintMaster *gpm = NULL;
   int copies = 1;
   int collate = 0;
   int do_preview = 0;
   
   GnomePrintDialog *gpd;

   gpd = GNOME_PRINT_DIALOG (gnome_print_dialog_new("Print image", GNOME_PRINT_DIALOG_COPIES));
   gnome_print_dialog_set_copies(gpd, copies, collate);

   switch (gnome_dialog_run(GNOME_DIALOG(gpd))) {
   case GNOME_PRINT_PRINT:
	   do_preview = 0;
	   break;
   case GNOME_PRINT_PREVIEW:
	   do_preview = 1;
	   break;
   case GNOME_PRINT_CANCEL:
	   return 0;
   }


   /* transfer dialog data to output context */
   gpm = gnome_print_master_new();
   gnome_print_dialog_get_copies(gpd, &copies, &collate);
   gnome_print_master_set_copies(gpm, copies, collate);
   gnome_print_master_set_printer(gpm, gnome_print_dialog_get_printer(gpd));
   gnome_dialog_close (GNOME_DIALOG(gpd));
   pc = gnome_print_master_get_context(gpm);
   image_buf_print(ibuf, pc);
   gnome_print_master_close(gpm);
   if (do_preview) 
   {
      GnomePrintMasterPreview *pmp;
      pmp = gnome_print_master_preview_new(gpm, "Preview");
      gtk_signal_connect(GTK_OBJECT(pmp), "destroy", gtk_widget_destroy, GTK_OBJECT(pmp));
      gtk_widget_show(GTK_WIDGET(pmp));
      gtk_window_set_modal (GTK_WINDOW(pmp), TRUE);
      gtk_main ();
   }
   else
      gnome_print_master_print(gpm);
      
   return 1;   
}



int do_print_preview(image_buf *ibuf)
{
   GtkWidget *toplevel, *canvas, *sw;
   GnomePrintContext *pc = 0;
   

   gtk_widget_set_default_colormap (gdk_rgb_get_cmap ());
   gtk_widget_set_default_visual (gdk_rgb_get_visual ());

   toplevel = gtk_window_new (GTK_WINDOW_TOPLEVEL);
   gtk_widget_set_usize (toplevel, 700, 700);
   sw = gtk_scrolled_window_new (NULL, NULL);
   canvas = gnome_canvas_new_aa ();
   gtk_container_add (GTK_CONTAINER (toplevel), sw);
   gtk_container_add (GTK_CONTAINER (sw), canvas);

   pc = gnome_print_preview_new (GNOME_CANVAS(canvas), "US-Letter");

   gtk_widget_show_all (toplevel);
   gtk_window_set_modal (GTK_WINDOW(toplevel), TRUE);
   image_buf_print(ibuf, pc);
   gnome_print_context_close (pc);
   gtk_main();
   gtk_object_unref (GTK_OBJECT (pc));
   
   return 1;
}





int image_buf_print(image_buf *ibuf, GnomePrintContext *pc)
{
   double matrix[6] = {1, 0, 0, 1, 0, 0};
   int width = image_buf_width(ibuf);
   int height = image_buf_height(ibuf);
   image_buf_pixmap_to_rgbbuf(ibuf, 0); /* get the latest version of the image in memory */
   gnome_print_beginpage(pc, ibuf->name);
    
   gnome_print_concat (pc, matrix);
   gnome_print_translate(pc, 0, 0);
   gnome_print_scale(pc, width, height);
   
   gnome_print_rgbaimage (pc, image_buf_rgbbuf(ibuf), image_buf_width(ibuf), image_buf_height(ibuf), image_buf_rowstride(ibuf));
   gnome_print_showpage (pc);

   return 1;
}   
