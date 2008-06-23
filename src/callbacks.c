#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include <gnome.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "callbacks.h"
#include "ui.h"
#include "support.h"

#include "image_buf.h"
#include "rgb.h"
#include "util.h"
#include "version.h"
#include "image_processing.h"
#include "clipboard.h"
#include "print.h"

void
on_mainwindow_realize                  (GtkWidget       *widget,
                                        gpointer         user_data)
{

}


gboolean
on_mainwindow_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(widget);
   g_assert(ibuf);
   close_image_buf(ibuf);
  return FALSE;
}


void
on_new_file_menu_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   new_canvas();
}


void
on_open_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   BEGIN_BUSY_CURSOR
   open_canvas(widget_get_image(GTK_WIDGET(menuitem)));
   END_BUSY_CURSOR
}


void
on_save_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   BEGIN_BUSY_CURSOR
   save_canvas(widget_get_image(GTK_WIDGET(menuitem)), 0);
   END_BUSY_CURSOR
}


void
on_save_as_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   BEGIN_BUSY_CURSOR
   save_canvas(widget_get_image(GTK_WIDGET(menuitem)), 1);
   END_BUSY_CURSOR

}


void
on_print_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   
   BEGIN_BUSY_CURSOR
   do_print(ibuf);
   END_BUSY_CURSOR
   


}


void
on_print_setup_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_exit_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   GtkWidget *mainwindow = widget_get_toplevel_parent(GTK_WIDGET(menuitem));
   image_buf *ibuf = widget_get_image(mainwindow);
   close_image_buf(ibuf);

}


void
on_cut_menu_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   image_buf_cut(ibuf);
}


void
on_copy_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   image_buf_copy(ibuf);
}


void
on_paste_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   image_buf_paste(ibuf);
}


void
on_clear_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   if (ibuf->num_pts)
   {
      image_buf_delete(ibuf);
   }
   else
   {
      image_buf_clear(ibuf);
   }
}


void
on_flip_x_axis_menu_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   BEGIN_BUSY_CURSOR
   image_buf_force_select_region(ibuf); /* for now */
    
   image_buf_flip_x_region(ibuf);
   ibuf->num_pts = 0;              /* to fix clear */
   END_BUSY_CURSOR
}


void
on_flip_y_axis_menu_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   BEGIN_BUSY_CURSOR
   image_buf_force_select_region(ibuf); /* for now */
   
   image_buf_flip_y_region(ibuf);
   ibuf->num_pts = 0;              /* to fix clear */
   END_BUSY_CURSOR

}


void
on_rotate_p90_menu_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   image_buf_force_select_region(ibuf); /* for now */
   image_buf_rotate_region(ibuf, 90 * M_PI / 180);

}


void
on_rotate_p45_menu_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   image_buf_force_select_region(ibuf); /* for now */
   image_buf_rotate_region(ibuf, 45 * M_PI / 180);

}


void
on_rotate_n45_menu_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   image_buf_force_select_region(ibuf); /* for now */
   image_buf_rotate_region(ibuf, -45 * M_PI / 180);

}


void
on_rotate_n90_menu_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   image_buf_force_select_region(ibuf); /* for now */
   image_buf_rotate_region(ibuf, -90 * M_PI / 180);


}


void
on_rotate_p180_menu_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   image_buf_force_select_region(ibuf); /* for now */
   image_buf_rotate_region(ibuf, 180 * M_PI / 180);

}


void
on_rotate_menu_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   image_buf_force_select_region(ibuf); /* for now */
   image_buf_rotate_region(ibuf, 60 * M_PI / 180);

}


void
on_reset_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_revert_menu_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}

/* ---- */
void
on_sharpen_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, ImageSharpen);
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}

void
on_smooth_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, ImageSmooth);
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}

void
on_directional_smooth_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, ImageDirectionalFilter);
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}


void
on_despeckle_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, ImageDespeckle);
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}

void
on_edge_detect_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, ImageEdge);
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}

void
on_emboss_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, ImageEmbose);
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}

void
on_oil_paint_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, ImageOilPaint);
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}

void
on_add_noise_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, ImageAddNoise);
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}

void
on_spread_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, spreadimage);
   
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}

void
on_pixelize_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, ImagePixelize);
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}

void
on_blend_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, ImageBlend);
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}

void
on_solarize_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, ImageNormContrast);
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}

void
on_normalize_contrast_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, ImageSharpen);
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}

void
on_quantize_color_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, ImageQuantize);
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}

void
on_convert_to_greyscale_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   g_assert(ibuf);
   BEGIN_BUSY_CURSOR
   image_buf_process_in_place(ibuf, ImageGrey);
   gtk_widget_draw(ibuf->drawing_area, NULL);
   END_BUSY_CURSOR
}

void
on_change_background_menu_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   image_buf_set_desktop_background(ibuf, 0);

}


void
on_about_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   GtkWidget *about = create_about_dialog ();
   
   
   gtk_widget_show (about);
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
on_small_label_realize                 (GtkWidget       *widget,
                                        gpointer         user_data)
{

}


void
on_line_width_combo_add                (GtkContainer    *container,
                                        GtkWidget       *widget,
                                        gpointer         user_data)
{

}


void
on_line_width_combo_check_resize       (GtkContainer    *container,
                                        gpointer         user_data)
{

}


void
on_new_button_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
   new_canvas();
}


void
on_open_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(button));
   BEGIN_BUSY_CURSOR
   open_canvas(widget_get_image(GTK_WIDGET(button)));
   END_BUSY_CURSOR
}


void
on_save_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(button));
   BEGIN_BUSY_CURSOR
   save_canvas(widget_get_image(GTK_WIDGET(button)), 0);
   END_BUSY_CURSOR
}


void
on_save_as_button_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(button));
   BEGIN_BUSY_CURSOR
   save_canvas(widget_get_image(GTK_WIDGET(button)), 1);
   END_BUSY_CURSOR

}


void
on_print_button_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(button));
   
   do_print(ibuf);
  

}


void
on_erase_button_clicked                (GtkToggleButton       *button,
                                        gpointer         user_data)
{

}


void
on_lasso_button_clicked                (GtkToggleButton       *button,
                                        gpointer         user_data)
{

}


void
on_line_button_clicked                 (GtkToggleButton       *button,
                                        gpointer         user_data)
{
   
}


void
on_rectangle_button_clicked            (GtkToggleButton       *button,
                                        gpointer         user_data)
{

}


void
on_closed_freehand_button_clicked             (GtkToggleButton       *button,
                                        gpointer         user_data)
{

}


void
on_pen_button_clicked                  (GtkToggleButton       *button,
                                        gpointer         user_data)
{

}


void
on_polselect_button_clicked            (GtkToggleButton       *button,
                                        gpointer         user_data)
{

}


void
on_text_button_clicked                 (GtkToggleButton       *button,
                                        gpointer         user_data)
{

}


void
on_arc_button_clicked                  (GtkToggleButton       *button,
                                        gpointer         user_data)
{

}


void
on_oval_button_clicked                 (GtkToggleButton       *button,
                                        gpointer         user_data)
{

}


void
on_brush_button_clicked                (GtkToggleButton       *button,
                                        gpointer         user_data)
{

}


gboolean
on_drawingarea_configure_event         (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data)
{

  return FALSE;
}


gboolean
on_drawingarea_button_press_event      (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  if (event->button == 1)
    handle_button_press (widget_get_image(widget), event->x, event->y);
  return FALSE;
}


gboolean
on_drawingarea_button_release_event    (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  if (event->button == 1)
    handle_button_release (widget_get_image(widget), event->x, event->y);

  return FALSE;
}


gboolean
on_drawingarea_motion_notify_event     (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data)
{
  int x, y;
  GdkModifierType state;

  if (event->is_hint)
    gdk_window_get_pointer (event->window, &x, &y, &state);
  else
    {
      x = event->x;
      y = event->y;
      state = event->state;
    }
    
  if (state & GDK_BUTTON1_MASK)
    handle_button_move (widget_get_image(widget), x, y);
  
  return TRUE;

  return FALSE;
}


gboolean
on_drawingarea_focus_in_event          (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(widget);
   image_buf_focus_obtained(ibuf);   
   return FALSE;
}


gboolean
on_drawingarea_focus_out_event         (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(widget);
   image_buf_focus_lost(ibuf);   
   return FALSE;
}

void
on_drawingarea_realize                 (GtkWidget       *widget,
                                        gpointer         user_data)
{

}


gboolean
on_drawingarea_expose_event            (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(widget);
  
   GdkRectangle fullrect ;
   GdkRectangle rect;
   fullrect.x = 0;
   fullrect.y = 0;
   fullrect.width = image_buf_width(ibuf);
   fullrect.height = image_buf_height(ibuf);
   
   
   
   gdk_rectangle_intersect(&fullrect, &(event->area), &rect);
   /*gdk_pixbuf_render_to_drawable(ibuf->rgbbuf, ibuf->pixmap, ibuf->gc, rect.x, rect.y, rect.x, rect.y, rect.width, rect.height, GDK_RGB_DITHER_NORMAL, 0, 0);
   */
   /*gdk_draw_rgb_image (widget->window, widget->style->fg_gc[GTK_STATE_NORMAL],
                      0, 0, ibuf->width, ibuf->height,
                     GDK_RGB_DITHER_MAX, ibuf->rgb, ibuf->width * 3);
   */
   
   gdk_draw_pixmap(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL], ibuf->pixmap, rect.x, rect.y, rect.x, rect.y, rect.width, rect.height);
  return FALSE;
}


gboolean
on_drawingarea_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{

  return FALSE;
}


void
on_new_canvas_ok_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *widthentry, *heightentry, *window = widget_get_toplevel_parent(GTK_WIDGET(button)); 
   char *tmp;
   int width = 0, height = 0;
   widthentry = lookup_widget(window, "new_canvas_width_text_entry");
   heightentry = lookup_widget(window, "new_canvas_height_text_entry");
   tmp = gtk_editable_get_chars(GTK_EDITABLE(widthentry), 0, -1);
   sscanf(tmp, "%d", &width);
   g_free(tmp);
   tmp = gtk_editable_get_chars(GTK_EDITABLE(heightentry), 0, -1);
   sscanf(tmp, "%d", &height);
   g_free(tmp);
   if ((width > 0) && (height > 0))
   {
      gtk_widget_destroy(window);


      create_image_buf(0, width, height);
   }
   else
   {
      GtkWidget  *msgbox = gnome_message_box_new("Invalid width or height values", GNOME_MESSAGE_BOX_ERROR, " OK ", NULL);
      gtk_window_set_modal(GTK_WINDOW(msgbox), TRUE);
       /* And mark it as a transient dialog */
      gtk_window_set_transient_for (GTK_WINDOW(msgbox), GTK_WINDOW(window));
      gtk_window_set_position(GTK_WINDOW(msgbox), GTK_WIN_POS_CENTER);
      gnome_dialog_run(GNOME_DIALOG(msgbox));
   }   
}


void
on_new_canvas_cancel_button_clicked    (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *window = widget_get_toplevel_parent(GTK_WIDGET(button)); 
   gtk_widget_destroy(window);
}



GtkWidget*
create_dreawing_area_in_scroll_frame (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
   return create_scroll_frame_widget();
}


void
on_scroll_frame_realize                (GtkWidget       *widget,
                                        gpointer         user_data)
{
}


void
on_scroll_frame_map                    (GtkWidget       *widget,
                                        gpointer         user_data)
{
}


void
on_fontpicker_font_set                 (GnomeFontPicker *gnomefontpicker,
                                        gchar *arg1,
                                        gpointer         user_data)
{

   image_buf *ibuf = widget_get_image(GTK_WIDGET(gnomefontpicker));
   g_assert(ibuf);
   image_buf_set_font(ibuf, gnome_font_picker_get_font(gnomefontpicker), gnome_font_picker_get_font_name(gnomefontpicker));
   

}


void
on_foreground_color_picker_color_set   (GnomeColorPicker *gnomecolorpicker,
                                        guint            arg1,
                                        guint            arg2,
                                        guint            arg3,
                                        guint            arg4,
                                        gpointer         user_data)
{
   GdkColor c = {0};
   short a;
   guint32 color;
   image_buf *ibuf = widget_get_image(GTK_WIDGET(gnomecolorpicker));
   g_assert(ibuf);
   c.red = (short) arg1;
   c.green = (short) arg2;
   c.blue = (short) arg3;
   a = (short) arg4;
   color = c.red >> 8 << 16 | c.green >> 8 << 8 | c.blue >> 8;
   gdk_rgb_gc_set_foreground(ibuf->gc, color);
   gnome_color_picker_set_i16(gnomecolorpicker, c.red, c.green, c.blue, a);
   
}


void
on_background_color_picker_color_set   (GnomeColorPicker *gnomecolorpicker,
                                        guint            arg1,
                                        guint            arg2,
                                        guint            arg3,
                                        guint            arg4,
                                        gpointer         user_data)
{
   GdkColor c = {0};
   short a;
   guint32 color;
   image_buf *ibuf = widget_get_image(GTK_WIDGET(gnomecolorpicker));
   g_assert(ibuf);
   c.red = (short) arg1;
   c.green = (short) arg2;
   c.blue = (short) arg3;
   a = (short) arg4;
   color = c.red >> 8 << 16 | c.green >> 8 << 8 | c.blue >> 8;
   gdk_rgb_gc_set_background(ibuf->gc, color);
   gnome_color_picker_set_i16(gnomecolorpicker, c.red, c.green, c.blue, a);
   

}


void
on_erase_button_realize                (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);
}


void
on_lasso_button_realize                (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);

}


void
on_fill_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);

}


void
on_fill_button_clicked                 (GtkToggleButton       *button,
                                        gpointer         user_data)
{

}


void
on_line_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);

}


void
on_rectangle_button_realize            (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);

}


void
on_closed_freehand_button_realize             (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);

}


void
on_pen_button_realize                  (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);

}


void
on_polselect_button_realize            (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);

}


void
on_text_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);

}


void
on_arc_button_realize                  (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);

}


void
on_oval_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);

}


void
on_brush_button_realize                (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);

}


void
on_about_dialog_pixmap_realize         (GtkWidget       *widget,
                                        gpointer         user_data)
{
   GdkPixmap *gdkpixmap;
   GdkBitmap *mask;
   GtkPixmap *gtkpixmap = GTK_PIXMAP(widget);
   gdkpixmap = gdk_pixmap_create_from_xpm_d(widget_get_toplevel_parent(widget)->window, &mask, NULL, (gchar**) user_data);
   g_assert(gdkpixmap);
   gtk_pixmap_set(gtkpixmap, gdkpixmap, mask);
   gdk_pixmap_unref (gdkpixmap);
   gdk_pixmap_unref (mask);

}


void
on_about_dialog_ok_button_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
   gtk_widget_destroy(window);
}


void
on_about_dialog_version_label_realize  (GtkWidget       *widget,
                                        gpointer         user_data)
{
   char tmp[200];
   sprintf(tmp, "%s %s", PROGRAM_TITLE, VERSION_STRING);
   gtk_label_set(GTK_LABEL(widget), tmp);
}


void
on_erase_button_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   select_toolbar_toggle_button(togglebutton, 0);

}


void
on_lasso_button_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   select_toolbar_toggle_button(togglebutton, 0);

}


void
on_fill_button_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   select_toolbar_toggle_button(togglebutton, 0);

}


void
on_line_button_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   select_toolbar_toggle_button(togglebutton, 0);

}


void
on_rectangle_button_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   select_toolbar_toggle_button(togglebutton, 0);

}


void
on_closed_freehand_button_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   select_toolbar_toggle_button(togglebutton, 0);

}


void
on_pen_button_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   select_toolbar_toggle_button(togglebutton, 0);

}


void
on_polselect_button_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   select_toolbar_toggle_button(togglebutton, 0);

}


void
on_text_button_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   select_toolbar_toggle_button(togglebutton, 0);

}


void
on_arc_button_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   select_toolbar_toggle_button(togglebutton, 0);

}


void
on_oval_button_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   select_toolbar_toggle_button(togglebutton, 0);

}


void
on_brush_button_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   select_toolbar_toggle_button(togglebutton, 0);

}



void
on_line_width_combo_combo_entry_changed
                                        (GtkEditable     *editable,
                                        gpointer         user_data)
{
   char *tmp;
   int t;
   image_buf *ibuf = widget_get_image(GTK_WIDGET(editable));
   g_assert(ibuf);
   tmp = gtk_editable_get_chars(editable, 0, -1);
   g_assert(tmp);
   sscanf(tmp, "%d", &t);
   g_assert(t != 0);
   gdk_gc_set_line_attributes(ibuf->gc, t, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
   g_free(tmp);
   gtk_widget_grab_focus(ibuf->drawing_area); /* force the line width entry widget to give up focus */
}





void
on_background_color_picker_realize     (GtkWidget       *widget,
                                        gpointer         user_data)
{
/*
   GdkColor c = {0};
   short a;
   guint32 color;
   image_buf *ibuf = widget_get_image(widget);
   g_assert(ibuf);
   c.red = 0;
   c.green = 0;
   c.blue = 0;
   a = 65535;
   color = c.red >> 8 << 16 | c.green >> 8 << 8 | c.blue >> 8;
   gdk_rgb_gc_set_foreground(ibuf->gc, color);
   gnome_color_picker_set_i16(GNOME_COLOR_PICKER(widget), c.red, c.green, c.blue, a);
*/
}


void
on_foreground_color_picker_realize     (GtkWidget       *widget,
                                        gpointer         user_data)
{
/*
   GdkColor c = {0};
   short a;
   guint32 color;
   image_buf *ibuf = widget_get_image(widget);
   g_assert(ibuf);
   c.red = 0;
   c.green = 0;
   c.blue = 0;
   a = 65535;
   color = c.red >> 8 << 16 | c.green >> 8 << 8 | c.blue >> 8;
   gdk_rgb_gc_set_foreground(ibuf->gc, color);
   gnome_color_picker_set_i16(GNOME_COLOR_PICKER(widget), c.red, c.green, c.blue, a);
*/
}




void
on_multiline_button_realize            (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);

}


void
on_multiline_button_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   select_toolbar_toggle_button(togglebutton, 0);

}


void
on_curve_button_realize                (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);

}


void
on_curve_button_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   select_toolbar_toggle_button(togglebutton, 0);

}




void
on_filled_button_realize               (GtkWidget       *widget,
                                        gpointer         user_data)
{
   set_button_pixmap(GTK_BUTTON(widget), (unsigned char * *) user_data);

}


void
on_filled_button_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(togglebutton));
   g_assert(ibuf);
   image_buf_set_fill(ibuf, gtk_toggle_button_get_active(togglebutton));


}




gboolean
on_drawingarea_key_release_event     (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(widget));
   g_assert(ibuf);

   handle_key_release(ibuf, event);
   if (event->string)
   fprintf(stderr, "%s", event->string);
   return FALSE;
}


gboolean
on_mainwindow_focus_in_event           (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(widget));
   gtk_widget_grab_focus(ibuf->drawing_area);
   gtk_widget_grab_default(ibuf->drawing_area);

  return FALSE;
}


gboolean
on_mainwindow_focus_out_event          (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(widget));
   image_buf_focus_lost(ibuf);
  return FALSE;
}


void
on_mainwindow_set_focus_child          (GtkContainer    *container,
                                        GtkWidget       *widget,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(container));
   if (widget != ibuf->drawing_area)
      image_buf_focus_lost(ibuf);

}



void
on_mainwindow_set_focus                (GtkWindow       *window,
                                        GtkWidget       *widget,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(window));
   if (widget != ibuf->drawing_area)
      image_buf_focus_lost(ibuf);

}


void
on_fontpicker_map                      (GtkWidget       *widget,
                                        gpointer         user_data)
{ 

}


void
on_background_color_picker_map         (GtkWidget       *widget,
                                        gpointer         user_data)
{

}


void
on_foreground_color_picker_map         (GtkWidget       *widget,
                                        gpointer         user_data)
{

}


gboolean
on_fontpicker_map_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
   return FALSE;
/*
   GtkWidget *window = widget_get_toplevel_parent(GTK_WIDGET(widget)); 
   image_buf *ibuf = widget_get_image(widget);
   gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(ibuf->window));
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_window_set_modal (GTK_WINDOW(window), TRUE);
  return FALSE;*/
}


gboolean
on_background_color_picker_map_event   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
   return FALSE;
/*   GtkWidget *window = widget_get_toplevel_parent(GTK_WIDGET(widget)); 
   image_buf *ibuf = widget_get_image(widget);
   gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(ibuf->window));
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_window_set_modal (GTK_WINDOW(window), TRUE);

  return FALSE;*/
}


gboolean
on_foreground_color_picker_map_event   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
   return FALSE;
/*   GtkWidget *window = widget_get_toplevel_parent(GTK_WIDGET(widget)); 
   image_buf *ibuf = widget_get_image(widget);
   gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(ibuf->window));
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_window_set_modal (GTK_WINDOW(window), TRUE);

  return FALSE;*/
}


void
on_set_as_background_centered_activate (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   image_buf_set_desktop_background(ibuf, 0);
}


void
on_set_as_background_titled_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   image_buf_set_desktop_background(ibuf, 1);

}


void
on_foreground_color_picker_color_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_foreground_color_picker_color_pressed
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_foreground_color_picker_color_released
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_color_palette_entry_map             (GtkWidget       *widget,
                                        gpointer         user_data)
{

}


void
on_color_palette_entry_draw            (GtkWidget       *widget,
                                        GdkRectangle    *area,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(widget);
   draw_palette_entry(ibuf, widget);

}


gboolean
on_color_palette_entry_key_press_event (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{

  return FALSE;
}


gboolean
on_color_palette_entry_key_release_event
                                        (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{

  return FALSE;
}


void
on_color_palette_entry_realize         (GtkWidget       *widget,
                                        gpointer         user_data)
{
}

struct color_selection_info
{
   image_buf *ibuf;
   GtkColorSelection *sel;
   GtkWidget *dialog;
};

static void on_color_selector_ok_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
   struct color_selection_info *info = (struct color_selection_info*) (user_data);		
   GnomeColorPicker *gnomecolorpicker = (GnomeColorPicker*) lookup_widget(info->ibuf->window, "foreground_color_picker");			
   gdouble tmp[4] = {0};
   char ctmp[100];
   GdkColor c = {0};
   short a;
   guint32 color;
   g_assert(info->ibuf);
   g_assert(gnomecolorpicker);
   gtk_color_selection_get_color(info->sel, tmp);
   
   c.red = (short) (tmp[0] * 65535);
   c.green = (short) (tmp[1] * 65535);
   c.blue = (short) (tmp[2] * 65535);
   a = (short) (tmp[3] * 65535);
   color = c.red >> 8 << 16 | c.green >> 8 << 8 | c.blue >> 8;
   gdk_rgb_gc_set_foreground(info->ibuf->gc, color);
   gnome_color_picker_set_i16(gnomecolorpicker, c.red, c.green, c.blue, a);
   
   info->ibuf->palettes[info->ibuf->current_palette] = c;
   sprintf(ctmp, "color_palette_entry%d", info->ibuf->current_palette);
   gtk_widget_queue_draw(lookup_widget(info->ibuf->window, ctmp));
   gtk_widget_destroy(info->dialog);
   
   free(info);
}

static void on_color_selector_cancel_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
   struct color_selection_info *info = (struct color_selection_info*) (user_data);		
   gtk_widget_destroy(info->dialog);
}					

gboolean
on_color_palette_entry_button_press_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(widget);
   struct color_selection_info *info;
   gchar *name = gtk_widget_get_name(widget);
   int i;
   sscanf(name, "color_palette_entry%d", &i);

   image_buf_set_foreground_to_palette(ibuf, i);
   
   if (event->type == GDK_2BUTTON_PRESS)
   {
      GtkWidget *window = gtk_color_selection_dialog_new("Select color");
      gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(ibuf->window));
      gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
      gtk_window_set_modal (GTK_WINDOW(window), TRUE);
      info = calloc(sizeof(struct color_selection_info), 1);
      info->ibuf = ibuf;
      info->sel = GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(window)->colorsel);
      info->dialog = window;
      gtk_signal_connect(GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(window)->ok_button), "clicked", GTK_SIGNAL_FUNC(on_color_selector_ok_button_clicked), (gpointer) info);
      gtk_signal_connect(GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(window)->cancel_button), "clicked", GTK_SIGNAL_FUNC(on_color_selector_cancel_button_clicked), (gpointer) info);
       
      gtk_widget_show(window);
   }
   
  return FALSE;
}


gboolean
on_color_palette_entry_button_release_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{

  return FALSE;
}


gboolean
on_color_palette_entry_expose_event    (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(widget);
   
   draw_palette_entry(ibuf, widget);
  return FALSE;
}


void
on_select_all_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   set_drawing_tool(ibuf, LASSO);
   image_buf_select_all(ibuf);
}
   



void
on_print_preview_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   
   do_print_preview(ibuf);
   

}


void
on_get_desktop_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   image_buf *ibuf = widget_get_image(GTK_WIDGET(menuitem));
   image_buf_get_desktop(ibuf);

}


void
on_undo_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_about_gnome_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gnome_url_show("http://www.gnome.org/");
}


void
on_about_gnu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gnome_url_show("http://www.gnu.org/");

}


gboolean
on_about_dialog_button_release_event   (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
    gnome_url_show("http://www.gnu.org/");


    return FALSE;
}


