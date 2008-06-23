
/*
    Copyright 2000, 2001  Li-Cheng (Andy) Tai (atai@gnu.org)

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

#include <assert.h>
#include <sys/param.h>
#include <gdk/gdkx.h>
#include <math.h>
#include "util.h"
#include "support.h"
#include "ui.h"
#include "callbacks.h"
#include "gtkscrollframe.h"
#include "pixmaps.h"
#include "clipboard.h"


GdkCursor *busy_cursor = 0;
GdkCursor *arrow_cursor = 0;


void image_buf_force_select_region(image_buf *ibuf)
{
   DRAWING_TOOL tool = ibuf->current_tool;
   if (tool == PASTE)
   {
       image_buf_copy_clipboard_to_region_buf(ibuf);
       return;
   }
   
   if (ibuf->num_pts == 0)
   {
       /* no points selected, then let's treat this as if the whole image is selected */
       image_buf_select_all(ibuf);
       image_buf_select_current_region(ibuf);
       ibuf->lpx = ibuf->lpy = 0;
       assert(ibuf->num_pts == 4); /* an image must be a rectangle, thus 4 points */
   }
   else
   {
       image_buf_select_current_region(ibuf); /* force currently selected region to be selected, regardless of the original content */
   }
}    


const int erase_size = 4;



GtkWidget *widget_get_toplevel_parent(GtkWidget *widget)
{
   GtkWidget *parent, *found_widget;
   found_widget = (GtkWidget*) gtk_object_get_data(GTK_OBJECT(widget), "toplevelparent");
   if (found_widget)
      return found_widget;
   found_widget = widget;
   for (;;)
     {
       if (GTK_IS_MENU (found_widget))
         parent = gtk_menu_get_attach_widget (GTK_MENU (found_widget));
       else
         parent = found_widget->parent;
       if (parent == NULL)
         break;
       found_widget = parent;
     }
   gtk_object_set_data(GTK_OBJECT(widget), "toplevelparent", (gpointer) found_widget);
   return found_widget;
}



image_buf *widget_get_image(GtkWidget *widget)
{
   image_buf *ibuf = (image_buf*) gtk_object_get_user_data(GTK_OBJECT(widget));
   GtkWidget *parent;
   
   if (!ibuf)
   {
      parent = widget_get_toplevel_parent(widget);
      ibuf = (image_buf*) gtk_object_get_user_data(GTK_OBJECT(parent));
      gtk_object_set_user_data(GTK_OBJECT(widget), ibuf);
   }
   
   
   return ibuf;
}         

GtkWidget *create_scroll_frame_widget(void)
{
   GtkWidget *scrolledwindow1; 
   scrolledwindow1 = gtk_scroll_frame_new (NULL, NULL);
   return scrolledwindow1;
}

GtkWidget *create_scroll_frame_widget_content(GtkWidget *scrolledwindow1, GtkWidget *mainwindow)
{
   GtkWidget *viewport1, *drawingarea;
   
   gtk_scroll_frame_set_policy (GTK_SCROLL_FRAME (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

   viewport1 = gtk_viewport_new (NULL, NULL);
   gtk_widget_set_name (viewport1, "viewport1");
   gtk_widget_ref (viewport1);
   gtk_object_set_data_full (GTK_OBJECT (mainwindow), "viewport1", viewport1,
                             (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (viewport1);
   gtk_container_add (GTK_CONTAINER (scrolledwindow1), viewport1);

   drawingarea = gtk_drawing_area_new ();
   gtk_widget_set_name (drawingarea, "drawingarea");
   gtk_widget_ref (drawingarea);
   gtk_object_set_data_full (GTK_OBJECT (mainwindow), "drawingarea", drawingarea,
                             (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (drawingarea);
   gtk_container_add (GTK_CONTAINER (viewport1), drawingarea);
   gtk_widget_set_events (drawingarea, GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_MOTION_MASK | GDK_BUTTON1_MOTION_MASK | GDK_BUTTON2_MOTION_MASK | GDK_BUTTON3_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_PROPERTY_CHANGE_MASK | GDK_VISIBILITY_NOTIFY_MASK | GDK_FOCUS_CHANGE_MASK | GDK_STRUCTURE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_PROPERTY_CHANGE_MASK);
   GTK_WIDGET_SET_FLAGS(drawingarea, GTK_CAN_FOCUS);
   GTK_WIDGET_SET_FLAGS(drawingarea, GTK_CAN_DEFAULT);
   
   
   gtk_signal_connect (GTK_OBJECT (drawingarea), "configure_event",
                       GTK_SIGNAL_FUNC (on_drawingarea_configure_event),
                       NULL);
   gtk_signal_connect (GTK_OBJECT (drawingarea), "button_press_event",
                       GTK_SIGNAL_FUNC (on_drawingarea_button_press_event),
                       NULL);
   gtk_signal_connect (GTK_OBJECT (drawingarea), "button_release_event",
                       GTK_SIGNAL_FUNC (on_drawingarea_button_release_event),
                       NULL);
   gtk_signal_connect (GTK_OBJECT (drawingarea), "motion_notify_event",
                       GTK_SIGNAL_FUNC (on_drawingarea_motion_notify_event),
                       NULL);
   gtk_signal_connect (GTK_OBJECT (drawingarea), "realize",
                       GTK_SIGNAL_FUNC (on_drawingarea_realize),
                       NULL);
   gtk_signal_connect (GTK_OBJECT (drawingarea), "expose_event",
                       GTK_SIGNAL_FUNC (on_drawingarea_expose_event),
                       NULL);
   gtk_signal_connect (GTK_OBJECT (drawingarea), "delete_event",
                       GTK_SIGNAL_FUNC (on_drawingarea_delete_event),
                       NULL);
   gtk_signal_connect (GTK_OBJECT (drawingarea), "focus_in_event",
                       GTK_SIGNAL_FUNC (on_drawingarea_focus_in_event),
                       NULL);
   gtk_signal_connect (GTK_OBJECT (drawingarea), "focus_out_event",
                       GTK_SIGNAL_FUNC (on_drawingarea_focus_out_event),
                       NULL);
   gtk_signal_connect (GTK_OBJECT (drawingarea), "key_release_event",
                       GTK_SIGNAL_FUNC (on_drawingarea_key_release_event),
                       NULL);
   return scrolledwindow1;
}

void new_canvas()
{
   GtkWidget *new_canvas_window = create_new_canvas_window ();
   GtkWidget *widthentry, *heightentry;
   char tmp[100];
   gint position = 0;
   widthentry = lookup_widget(new_canvas_window, "new_canvas_width_text_entry");
   heightentry = lookup_widget(new_canvas_window, "new_canvas_height_text_entry");
   sprintf(tmp, "640");
   gtk_editable_insert_text(GTK_EDITABLE(widthentry), tmp, strlen(tmp), &position);
   position = 0; 
   sprintf(tmp, "480");
   gtk_editable_insert_text(GTK_EDITABLE(heightentry), tmp, strlen(tmp), &position);
   
   
   gtk_widget_show (new_canvas_window);
}   

static void open_canvas_callback(GtkFileSelection *widget)
{
   char tmp[4000];
   image_buf *ibuf = (image_buf*) gtk_object_get_user_data(GTK_OBJECT(widget));
   strcpy(tmp, gtk_file_selection_get_filename(GTK_FILE_SELECTION(widget)));
   gtk_widget_destroy(GTK_WIDGET(widget));
   PROCESSING_GTK_EVENTS;
   image_buf_load(ibuf, tmp);
   
}

   
void open_canvas(image_buf *ibuf)
{
   GtkWidget *widget = gtk_file_selection_new("Open image");
   g_assert(widget);
   /* Set as modal */
   gtk_window_set_modal (GTK_WINDOW(widget),TRUE);

   /* And mark it as a transient dialog */
   gtk_window_set_transient_for (GTK_WINDOW(widget), GTK_WINDOW(ibuf->window));
   gtk_window_set_position(GTK_WINDOW(widget), GTK_WIN_POS_CENTER);

   gtk_object_set_user_data(GTK_OBJECT(widget), ibuf);
   
   {
      char pathbuf[MAXPATHLEN], *dir;
      if (ibuf->name)
         dir = g_dirname(ibuf->name);
      else
         dir = g_get_current_dir();

      /* FIXME: Buffer overflow? */
      sprintf(pathbuf, "%s%c*.*", dir, G_DIR_SEPARATOR);
      gtk_file_selection_set_filename(GTK_FILE_SELECTION(widget), pathbuf);
      g_free(dir);
   }
   
   gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(widget)->ok_button),
                             "clicked", GTK_SIGNAL_FUNC(open_canvas_callback),
                             GTK_OBJECT(widget));
   gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(widget)->cancel_button),
                             "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
                             GTK_OBJECT(widget));
   gtk_widget_show(widget);

}
static void save_canvas_callback(GtkFileSelection *widget)
{
   char tmp[4000];
   image_buf *ibuf = (image_buf*) gtk_object_get_user_data(GTK_OBJECT(widget));

   strcpy(tmp, gtk_file_selection_get_filename(GTK_FILE_SELECTION(widget)));
   gtk_widget_destroy(GTK_WIDGET(widget));
   PROCESSING_GTK_EVENTS;
   image_buf_save(ibuf, tmp);
   image_buf_set_name(ibuf, tmp);
}

void save_canvas(image_buf *ibuf, int saveas)
{
  
   GtkWidget *widget ;
   if ((saveas == 0) && strncmp(ibuf->name, UNTITLED_NAME, strlen(UNTITLED_NAME)))
   {
      image_buf_save(ibuf, ibuf->name);
      return ;
   }
   
   widget = gtk_file_selection_new("Save image");
   g_assert(widget);
   /* Set as modal */
   gtk_window_set_modal (GTK_WINDOW(widget),TRUE);

   /* And mark it as a transient dialog */
   gtk_window_set_transient_for(GTK_WINDOW(widget), GTK_WINDOW(ibuf->window));
   gtk_window_set_position(GTK_WINDOW(widget), GTK_WIN_POS_CENTER);
   if ((ibuf->name) && strncmp(ibuf->name, UNTITLED_NAME, strlen(UNTITLED_NAME)))
      gtk_file_selection_set_filename(GTK_FILE_SELECTION(widget), ibuf->name);

   gtk_object_set_user_data(GTK_OBJECT(widget), ibuf);
   gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(widget)->ok_button),
                             "clicked", GTK_SIGNAL_FUNC(save_canvas_callback),
                             GTK_OBJECT(widget));
   gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(widget)->cancel_button),
                             "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
                             GTK_OBJECT(widget));

   gtk_widget_show(widget);

}

static void gtk_container_remove_callback(GtkWidget *widget, gpointer data)
{
   gtk_container_remove(GTK_CONTAINER(data), widget);
}

void set_button_pixmap(GtkButton *button, unsigned char **pixmap)
{
   GdkPixmap *gdkpixmap;
   GdkBitmap *mask;
   GtkWidget *gtkpixmap;
   gdkpixmap = gdk_pixmap_create_from_xpm_d(widget_get_toplevel_parent(GTK_WIDGET(button))->window, &mask, NULL, (gchar**) pixmap);
   g_assert(gdkpixmap);
   gtkpixmap = gtk_pixmap_new(gdkpixmap, mask);
   g_assert(gtkpixmap);
   gdk_pixmap_unref (gdkpixmap);
   gdk_pixmap_unref (mask);
   
   gtk_container_foreach(GTK_CONTAINER(button), (GtkCallback) gtk_container_remove_callback, button);
   gtk_container_add(GTK_CONTAINER(button), (gtkpixmap));
   
   gtk_widget_show(gtkpixmap);
   
}

typedef struct toggle_button_info 
{
   const char *name;
   DRAWING_TOOL tool;
} toggle_button_info;
   
const  toggle_button_info toggle_button_infos[] =
{
   {"erase_button", ERASE},
   {"lasso_button", LASSO},
   {"fill_button", FILL},
   {"line_button", LINE},
   {"multiline_button", MULTILINE},
   {"rectangle_button", RECTANGLE},
   {"closed_freehand_button", CLOSED_FREEHAND},
   {"pen_button", PEN},
   {"polselect_button", POLSELECT},
   {"text_button", TEXT},
   {"arc_button", ARC},
   {"curve_button", CURVE},
   {"oval_button", OVAL},
   {"brush_button", BRUSH},
   {0, NONE},
   {0, PASTE}
};

void set_drawing_tool(image_buf *ibuf, DRAWING_TOOL tool)
{
   const char *buttonname = 0;
   GtkWidget *selected = 0;
   
   if (ibuf->modified)
   {
      image_buf_pixmap_to_rgbbuf(ibuf, 0);
   }
   
   if (tool < NONE)
      buttonname = toggle_button_infos[tool - ERASE].name;
   
   if (buttonname)
      selected = lookup_widget(ibuf->window, buttonname);
   
   if (selected)
      select_toolbar_toggle_button(GTK_TOGGLE_BUTTON(selected), 1);
   else
      image_buf_set_tool(ibuf, tool);
      
}



void select_toolbar_toggle_button(GtkToggleButton *selected, int setmode)
{
   const toggle_button_info *t;
   GtkWidget *b;
   GtkWidget *window ;
   int x, y;
   int inbutton = setmode;
   gtk_widget_get_pointer(GTK_WIDGET(selected), &x, &y);
   if ((x >= 0) && (y >= 0) && (x < GTK_WIDGET(selected)->allocation.width) && (y < GTK_WIDGET(selected)->allocation.height))
      inbutton = 1;
      
   if (!gtk_toggle_button_get_active(selected))
   {
      if (!inbutton)
         return;
      
      gtk_toggle_button_set_active(selected, TRUE);
      
   }
   
   window = widget_get_toplevel_parent(GTK_WIDGET(selected));
   g_assert(window); 
   if (inbutton)
   {
      image_buf *ibuf = widget_get_image(window);
      g_assert(ibuf);
      ibuf->current_button = GTK_WIDGET(selected);
   }
   
   for (t = toggle_button_infos; (t->name); t++)
   {
      b = lookup_widget(window, (t->name));
      g_assert(b);
      if (b != GTK_WIDGET(selected))      
      {
         if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(b)))
	 {
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(b), FALSE);
          
	 }        
         g_assert(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(b)));
      }
      else if (inbutton)
      {
	 image_buf *ibuf = widget_get_image(window);
	 g_assert(ibuf);
	 image_buf_set_tool(ibuf, t->tool);
      }
         
   }
}
      
void StateTimeStep()
{

}      


static void image_buf_draw_flash(image_buf *ibuf)
{

   GdkRectangle clientrect ;

   DRAWING_TOOL tool = ibuf->current_tool;
   GdkGCValues gcvalues;
   clientrect.x = clientrect.y = 0;
   clientrect.width = image_buf_width(ibuf);
   clientrect.height = image_buf_height(ibuf);
   
   gdk_gc_get_values(ibuf->gc, &gcvalues);
   gdk_gc_set_clip_origin(ibuf->gc, 0, 0);
   gdk_gc_set_clip_rectangle(ibuf->gc, &clientrect);
   
   switch(tool)
   {
   case TEXT:
   
      if ((ibuf->llx != INT_MIN) && (ibuf->lly != INT_MIN))
      {
	 gint width, ascent, descent, lbearing, rbearing, junk;
	 GdkColor white;
	 gdk_color_white(gdk_rgb_get_cmap(), &white);
   
         gdk_gc_set_foreground(ibuf->gc, &white);   
 
	 gdk_string_extents(ibuf->font, "My", &junk, &junk, &junk, &ascent, &descent);
	 gdk_string_extents(ibuf->font, ibuf->textbuf, &lbearing, &rbearing, &width, &junk, &junk);

	 gdk_gc_set_function(ibuf->gc, GDK_INVERT);
	 gdk_gc_set_line_attributes(ibuf->gc, 2, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
	 gdk_draw_line(ibuf->drawing_area->window, ibuf->gc, ibuf->llx + rbearing + 2, ibuf->lly + descent, ibuf->llx + rbearing + 2, ibuf->lly - ascent);   
	 gdk_draw_line(ibuf->pixmap, ibuf->gc, ibuf->llx + rbearing + 2, ibuf->lly + descent, ibuf->llx + rbearing + 2, ibuf->lly - ascent);   

	 gdk_gc_set_function(ibuf->gc, gcvalues.function);   
	 gdk_gc_set_line_attributes(ibuf->gc, gcvalues.line_width, gcvalues.line_style, gcvalues.cap_style, gcvalues.join_style);
	 gdk_gc_set_foreground(ibuf->gc, &(gcvalues.foreground));
	 ibuf->flash_state = !(ibuf->flash_state);
      }
      break;
      
   case POLSELECT:
   case LASSO:
   case PASTE:
      {
         gchar dash_length[2] = { 5, 5 };
	 GdkColor white;
	 gdk_color_white(gdk_rgb_get_cmap(), &white);
   
         gdk_gc_set_foreground(ibuf->gc, &white);   
 
	 gdk_gc_set_function(ibuf->gc, GDK_INVERT);
	 gdk_gc_set_line_attributes(ibuf->gc, 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_ROUND, GDK_JOIN_ROUND);
         gdk_gc_set_dashes(ibuf->gc,  ibuf->flash_state / 2, dash_length, sizeof(dash_length));
	 
	 gdk_draw_lines(ibuf->drawing_area->window, ibuf->gc, ibuf->pts, ibuf->num_pts);
	 gdk_draw_lines(ibuf->pixmap, ibuf->gc, ibuf->pts, ibuf->num_pts);
	 
	 gdk_gc_set_function(ibuf->gc, gcvalues.function);   
	 gdk_gc_set_line_attributes(ibuf->gc, gcvalues.line_width, gcvalues.line_style, gcvalues.cap_style, gcvalues.join_style);
	 gdk_gc_set_foreground(ibuf->gc, &(gcvalues.foreground));
	 ibuf->flash_state++;
	 ibuf->flash_state %= 4;
      }
      break;
      
   default:
      break;
   }
}

void image_buf_clear_flash(image_buf *ibuf)
{
   while (ibuf->flash_state)
      image_buf_draw_flash(ibuf);
}

void image_buf_set_fill(image_buf *ibuf, int filled)
{
   GtkWidget *b = lookup_widget(ibuf->window, "filled_button");
   int old = image_buf_get_fill(ibuf);
   assert(b);
   ibuf->filled = filled;
   if (filled != old)
   {
      if (filled)
         set_button_pixmap(GTK_BUTTON(b),  (unsigned char * *) filled_xpm);
      else
         set_button_pixmap(GTK_BUTTON(b),  (unsigned char * *) unfilled_xpm);
   }
   
}

static void clear_points(image_buf *ibuf)
{
   ibuf->num_pts = 0;

   memset(ibuf->pts, 0, sizeof(ibuf->pts));
}


void image_buf_set_tool(image_buf *ibuf, DRAWING_TOOL tool)
{
   image_buf_clear_flash(ibuf);
   switch(ibuf->current_tool) /* final mode cleanup */
   {
   case TEXT:
      if ((ibuf->llx > INT_MIN) && (ibuf->lly > INT_MIN) && strlen(ibuf->textbuf))
      {
	 gdk_draw_string(ibuf->drawing_area->window, ibuf->font, ibuf->gc, ibuf->llx, ibuf->lly, ibuf->textbuf);
	 gdk_draw_string(ibuf->pixmap, ibuf->font, ibuf->gc, ibuf->llx, ibuf->lly, ibuf->textbuf);
	 IMAGE_MODIFIED;
      }
      ibuf->llx = INT_MIN;
      ibuf->lly = INT_MIN;
      break;
   
   default:
      break;
   }
   
   ibuf->current_tool = tool;
   ibuf->flash_state = 0;
   switch(tool) /* new mode initialization */
   {
   case MULTILINE:
   
   case ERASE:
   case LINE:
   case ARC:
   case RECTANGLE:
   case OVAL:
   case LASSO:
   case POLSELECT:
   
      image_buf_set_cursor(ibuf, gdk_cursor_new(GDK_CROSSHAIR));
      break;
   
   case CLOSED_FREEHAND:
   case PEN:
      image_buf_set_cursor(ibuf, gdk_cursor_new(GDK_PENCIL));
      break;
    
   case TEXT:
      image_buf_set_cursor(ibuf, gdk_cursor_new(GDK_XTERM));
      strcpy(ibuf->textbuf, "");
      break;
      
   default:
      image_buf_set_cursor(ibuf, gdk_cursor_new(GDK_X_CURSOR));
      break;
   }
   ibuf->llx = INT_MIN;
   ibuf->lly = INT_MIN;
   ibuf->lx = INT_MIN;
   ibuf->ly = INT_MIN;
   ibuf->mx = INT_MIN;
   ibuf->my = INT_MIN;
   ibuf->lpx = INT_MIN;
   ibuf->lpy = INT_MIN;
   clear_points(ibuf);
}
   

static void insert_point(image_buf *ibuf, int x, int y)
{
   if (x < 0)
      x = 0;
   if (y < 0)
      y = 0;
   ibuf->pts[ibuf->num_pts].x = x;
   ibuf->pts[ibuf->num_pts].y = y;
   ibuf->num_pts++;
}


static void modify_point(image_buf *ibuf, int index, int x, int y)
{
   ibuf->pts[index].x = x;
   ibuf->pts[index].y = y;


}

void draw_rectangle(GdkDrawable *d, GdkGC *gc, gint filled, gint x1, gint y1, gint x2, gint y2)
{
   int ox, w, oy, h;
   if (x1 > x2)
   {
      ox = x2;
      w = x1 - x2;
   }
   else
   {
      ox = x1;
      w = x2 - x1;
   }
   
   if (y1 > y2)
   {
      oy = y2;
      h = y1 - y2;
   }
   else
   {
      oy = y1;
      h = y2 - y1;
   }
   if (filled)
      w++, h++;
   gdk_draw_rectangle(d, gc, filled, ox, oy, w, h);
}

static void draw_oval(GdkDrawable *d, GdkGC *gc, gint filled, gint x1, gint y1, gint x2, gint y2)
{
   int ox, w, oy, h;
   if (x1 > x2)
   {
      ox = x2;
      w = x1 - x2;
   }
   else
   {
      ox = x1;
      w = x2 - x1;
   }
   
   if (y1 > y2)
   {
      oy = y2;
      h = y1 - y2;
   }
   else
   {
      oy = y1;
      h = y2 - y1;
   }
   if (filled)
      w++, h++;
   gdk_draw_arc(d, gc, filled, ox, oy, w, h, 0, 360 * 64);
}


static void draw_arc(GdkDrawable *d, GdkGC *gc, gint filled, gint x1, gint y1, gint x2, gint y2)
{
   int ox, w, oy, h;
   int angle1, angle2;
   
   
   if (x1 < x2)
   {
      w = (x2 - x1) * 2;
     
      if (y1 < y2)
      {
         h = (y2 - y1) * 2;  
         ox = x2 - w;
	 oy = y1;
         angle1 = 90;
	 angle2 = -90;
      }
      else
      {
         ox = x1;
         h = (y1 - y2) * 2;  
	 oy = y2;
         angle1 = 180;
	 angle2 = -90;
      }
   }
   else /* x1 > x2 */
   {
      w = (x1 - x2) * 2;
      
      if (y1 < y2)
      {
         h = (y2 - y1) * 2;
	 ox = x1 - w;
	 oy = y2 - h;
         angle1 = 0;
	 angle2 = -90;
      }
      else
      {
         h = (y1 - y2) * 2;
	 oy = y1 - h;
	 ox = x2;
         angle1 = -90;
	 angle2 = -90;
      }
   }
      
   if (filled)
      w++, h++;
      
   gdk_draw_arc(d, gc, filled, ox, oy, w, h, angle1 * 64, angle2 * 64);
}



void image_buf_clear(image_buf *ibuf)
{
   GdkGCValues gcvalues;
   GdkRectangle clientrect ;
      
   
   clientrect.x = clientrect.y = 0;
   clientrect.width = image_buf_width(ibuf);
   clientrect.height = image_buf_height(ibuf);
   
   image_buf_clear_flash(ibuf);
   
   gdk_gc_get_values(ibuf->gc, &gcvalues);
   gdk_gc_set_clip_origin(ibuf->gc, 0, 0);
   gdk_gc_set_clip_rectangle(ibuf->gc, &clientrect);
   
   gdk_gc_set_foreground(ibuf->gc, &(gcvalues.background)); 

   draw_rectangle(ibuf->pixmap, ibuf->gc, TRUE, clientrect.x, clientrect.y, clientrect.width, clientrect.height);
   draw_rectangle(ibuf->drawing_area->window, ibuf->gc, TRUE, clientrect.x, clientrect.y, clientrect.width, clientrect.height);
   image_buf_pixmap_to_rgbbuf(ibuf, 0);

   
    gdk_gc_set_foreground(ibuf->gc, &(gcvalues.foreground)); 

}


void handle_button_press(image_buf *ibuf, int x, int y)
{
   DRAWING_TOOL tool = ibuf->current_tool;
   GdkGCValues gcvalues;
   GdkRectangle rect, clientrect ;
      
   rect.x = x;
   rect.y = y;
   rect.width = 10;
   rect.height = 10;
   clientrect.x = clientrect.y = 0;
   clientrect.width = image_buf_width(ibuf);
   clientrect.height = image_buf_height(ibuf);
   
   gdk_gc_get_values(ibuf->gc, &gcvalues);
   gdk_gc_set_clip_origin(ibuf->gc, 0, 0);
   gdk_gc_set_clip_rectangle(ibuf->gc, &clientrect);
   
   image_buf_clear_flash(ibuf);
   
   if (tool == PASTE)
      ;
   
   else if (tool != POLSELECT)
   {
      clear_points(ibuf);
      insert_point(ibuf, x, y);
   }
   else
   {
      if (ibuf->num_pts)
         modify_point(ibuf, ibuf->num_pts - 1, x, y);
      else
         insert_point(ibuf, x, y);
      
   }
   
   switch (tool)
   {
   case CLOSED_FREEHAND:
   case PEN:
      gdk_gc_set_line_attributes(ibuf->gc, gcvalues.line_width, gcvalues.line_style, GDK_CAP_ROUND, GDK_JOIN_ROUND);
      gdk_draw_line(ibuf->pixmap, ibuf->gc, x, y, x, y);
      gdk_draw_line(ibuf->drawing_area->window, ibuf->gc, x, y, x, y);
      IMAGE_MODIFIED;

      gdk_gc_set_line_attributes(ibuf->gc, gcvalues.line_width, gcvalues.line_style, gcvalues.cap_style, gcvalues.join_style);
      //gtk_widget_draw(ibuf->drawing_area, &rect);
      break;
    
     
   
   case MULTILINE:
      if ((ibuf->llx > INT_MIN) && (ibuf->lly > INT_MIN))
      {
	 gdk_draw_line(ibuf->drawing_area->window, ibuf->gc, x, y, ibuf->llx, ibuf->lly);
	 gdk_draw_line(ibuf->pixmap, ibuf->gc, x, y, ibuf->llx, ibuf->lly);
	 IMAGE_MODIFIED;

      }      
      
   case LINE:
      break;
     
   case OVAL:
      break;
   
   case ARC:
      break;
      
   case RECTANGLE:
      break;	 
	
   case LASSO:
   case POLSELECT:
      break;	
	
   case TEXT:
      if ((ibuf->llx != INT_MIN) && (ibuf->lly != INT_MIN)) /* previous round of text entry */
      {
	 if (strlen(ibuf->textbuf))
	 {
	    gdk_draw_string(ibuf->drawing_area->window, ibuf->font, ibuf->gc, ibuf->llx, ibuf->lly, ibuf->textbuf);
	    gdk_draw_string(ibuf->pixmap, ibuf->font, ibuf->gc, ibuf->llx, ibuf->lly, ibuf->textbuf);
   	    IMAGE_MODIFIED;
	 }
	 strcpy(ibuf->textbuf, "");
	 ibuf->llx = INT_MIN;
	 ibuf->lly = INT_MIN;
      }
      break;
   	
   case ERASE:
      gdk_gc_set_foreground(ibuf->gc, &(gcvalues.background));
      draw_rectangle(ibuf->pixmap, ibuf->gc, TRUE, x - erase_size, y - erase_size, x + erase_size, y + erase_size);
      draw_rectangle(ibuf->drawing_area->window, ibuf->gc, TRUE, x - erase_size, y - erase_size, x + erase_size, y + erase_size);
      IMAGE_MODIFIED;
      
      gdk_gc_set_foreground(ibuf->gc, &(gcvalues.foreground));
      break;
      
   case FILL:
      image_buf_flood_fill(ibuf, x, y, &(gcvalues.foreground));	
      gtk_widget_draw(ibuf->drawing_area, NULL);
      break;
      	 
       
   case PASTE:
      rect = compute_cover_rect(ibuf->pts, ibuf->num_pts);
      ibuf->ptdiffx = rect.x - x;
      ibuf->ptdiffy = rect.y - y;
      if ((ibuf->ptdiffx > 0) || (ibuf->ptdiffx <= -rect.width) || 
          (ibuf->ptdiffy > 0) || (ibuf->ptdiffy <= -rect.height))
      {
	 image_buf_leave_paste_mode(ibuf);
         return;
      }      
      image_buf_adjust_according_to_points(ibuf, &x, &y);
      image_buf_put_region_image(ibuf, x, y);
      break;
   
   case BRUSH:
      /* brain dead brush */
      draw_rectangle(ibuf->pixmap, ibuf->gc, TRUE, 
                     x - erase_size, y - erase_size, x + erase_size, y + erase_size);
      draw_rectangle(ibuf->drawing_area->window, ibuf->gc, TRUE,
                     x - erase_size, y - erase_size, x + erase_size, y + erase_size);
      IMAGE_MODIFIED;
      break;
                
   default:
      gdk_beep();
   }
   ibuf->lx = x;
   ibuf->ly = y;
   ibuf->mx = x; /* INT_MIN; */
   ibuf->my = y; /* INT_MIN; */
}   

void handle_button_move(image_buf *ibuf, int x, int y)
{
   DRAWING_TOOL tool = ibuf->current_tool;
   GdkGCValues gcvalues;
   GdkRectangle rect, clientrect ;
   
   if ((ibuf->lx == INT_MIN) || (ibuf->ly == INT_MIN))
      return;
      
   rect.x = x;
   rect.y = y;
   rect.width = 10;
   rect.height = 10;
   clientrect.x = clientrect.y = 0;
   clientrect.width = image_buf_width(ibuf);
   clientrect.height = image_buf_height(ibuf);
   
   gdk_gc_get_values(ibuf->gc, &gcvalues);
   gdk_gc_set_clip_origin(ibuf->gc, 0, 0);
   gdk_gc_set_clip_rectangle(ibuf->gc, &clientrect);
   
   image_buf_clear_flash(ibuf);
   if (tool == PASTE)
      ;
   else
   if (tool != POLSELECT)
      insert_point(ibuf, x, y);
   else
   {
      if ((ibuf->mx == INT_MIN) || (ibuf->my == INT_MIN))
	 insert_point(ibuf, x, y);
      else
         modify_point(ibuf, ibuf->num_pts - 1, x, y); 
      
   }
   image_buf_draw_flash(ibuf);
         
   switch (tool)
   {
   case CLOSED_FREEHAND:
   case PEN:
      gdk_gc_set_line_attributes(ibuf->gc, gcvalues.line_width, gcvalues.line_style, GDK_CAP_ROUND, GDK_JOIN_ROUND);
       
      if ((ibuf->mx == INT_MIN) && (ibuf->my == INT_MIN))
      {
	 gdk_draw_line(ibuf->pixmap, ibuf->gc, ibuf->lx, ibuf->ly, x, y);
	 gdk_draw_line(ibuf->drawing_area->window, ibuf->gc, ibuf->lx, ibuf->ly, x, y);
      }
      else
      {
	 gdk_draw_line(ibuf->pixmap, ibuf->gc, ibuf->mx, ibuf->my, x, y);
	 gdk_draw_line(ibuf->drawing_area->window, ibuf->gc, ibuf->mx, ibuf->my, x, y);
      }
      IMAGE_MODIFIED;
      gdk_gc_set_line_attributes(ibuf->gc, gcvalues.line_width, gcvalues.line_style, gcvalues.cap_style, gcvalues.join_style);
      //gtk_widget_draw(ibuf->drawing_area, &rect);
      break;
   
   
      
   case MULTILINE:
   case LINE:
      gdk_gc_set_function(ibuf->gc, GDK_INVERT);
      if ((ibuf->mx > INT_MIN) && (ibuf->my > INT_MIN))
      {
         gdk_draw_line(ibuf->drawing_area->window, ibuf->gc, ibuf->lx, ibuf->ly, ibuf->mx, ibuf->my);
      }
      gdk_draw_line(ibuf->drawing_area->window, ibuf->gc, ibuf->lx, ibuf->ly, x, y);
      gdk_gc_set_function(ibuf->gc, gcvalues.function);  
      gdk_gc_set_line_attributes(ibuf->gc, gcvalues.line_width, gcvalues.line_style, gcvalues.cap_style, gcvalues.join_style);
      break;
      
   case OVAL:
      gdk_gc_set_function(ibuf->gc, GDK_INVERT);
      if ((ibuf->mx > INT_MIN) && (ibuf->my > INT_MIN))
      {
         draw_oval(ibuf->drawing_area->window, ibuf->gc, FALSE, ibuf->lx, ibuf->ly, ibuf->mx, ibuf->my);
      }
      draw_oval(ibuf->drawing_area->window, ibuf->gc, FALSE, ibuf->lx, ibuf->ly, x, y);
      gdk_gc_set_function(ibuf->gc, gcvalues.function);  
      break;
   
   case ARC:
      gdk_gc_set_function(ibuf->gc, GDK_INVERT);
      if ((ibuf->mx > INT_MIN) && (ibuf->my > INT_MIN))
      {
         draw_arc(ibuf->drawing_area->window, ibuf->gc, FALSE, ibuf->lx, ibuf->ly, ibuf->mx, ibuf->my);
      }
      draw_arc(ibuf->drawing_area->window, ibuf->gc, FALSE, ibuf->lx, ibuf->ly, x, y);
      gdk_gc_set_function(ibuf->gc, gcvalues.function);  
      break;
   
   case RECTANGLE:
      gdk_gc_set_function(ibuf->gc, GDK_INVERT); 
      if ((ibuf->mx > INT_MIN) && (ibuf->my > INT_MIN))
      {
         draw_rectangle(ibuf->drawing_area->window, ibuf->gc, FALSE, ibuf->lx, ibuf->ly, ibuf->mx, ibuf->my);
      }
      draw_rectangle(ibuf->drawing_area->window, ibuf->gc, FALSE, ibuf->lx, ibuf->ly, x, y);
      gdk_gc_set_function(ibuf->gc, gcvalues.function); 
      
      break;	 
   
   case ERASE:
      image_buf_paint_interpolate(ibuf, x, y);
      break;
      
   case BRUSH: 
      image_buf_paint_interpolate(ibuf, x, y);
      break;
 
   case PASTE:
      image_buf_adjust_according_to_points(ibuf, &x, &y);
      image_buf_put_region_image(ibuf, x, y);
      break;
                
   default:
      
   }
   ibuf->mx = x;
   ibuf->my = y;
}   

void handle_button_release(image_buf *ibuf, int x, int y)
{
   DRAWING_TOOL tool = ibuf->current_tool;
   GdkGCValues gcvalues;
   GdkRectangle rect, clientrect ;
   
   if ((ibuf->lx == INT_MIN) || (ibuf->ly == INT_MIN))
      return;
      
   rect.x = x;
   rect.y = y;
   rect.width = 10;
   rect.height = 10;
   clientrect.x = clientrect.y = 0;
   clientrect.width = image_buf_width(ibuf);
   clientrect.height = image_buf_height(ibuf);
   
   gdk_gc_get_values(ibuf->gc, &gcvalues);
   gdk_gc_set_clip_origin(ibuf->gc, 0, 0);
   gdk_gc_set_clip_rectangle(ibuf->gc, &clientrect);
   
   image_buf_clear_flash(ibuf);
   
   if (tool == PASTE)
      ;
   else
   if (tool != POLSELECT)
      insert_point(ibuf, x, y);
   else
   {
      if ((ibuf->mx == INT_MIN) || (ibuf->my == INT_MIN))
	 insert_point(ibuf, x, y);
      else
         modify_point(ibuf, ibuf->num_pts - 1, x, y); 
      
   }
    
   
   switch (tool)
   {
   case CLOSED_FREEHAND:
   case PEN:
      gdk_gc_set_line_attributes(ibuf->gc, gcvalues.line_width, gcvalues.line_style, GDK_CAP_ROUND, GDK_JOIN_ROUND);
      if ((ibuf->mx == INT_MIN) && (ibuf->my == INT_MIN))
      {
         if (tool == CLOSED_FREEHAND)
	 {
	    gdk_draw_polygon(ibuf->pixmap, ibuf->gc, image_buf_get_fill(ibuf), ibuf->pts, ibuf->num_pts);
	    gdk_draw_polygon(ibuf->drawing_area->window, ibuf->gc, image_buf_get_fill(ibuf), ibuf->pts, ibuf->num_pts);
	 
	 }
	 else
	 {
	    gdk_draw_line(ibuf->pixmap, ibuf->gc, ibuf->lx, ibuf->ly, x, y);
	    gdk_draw_line(ibuf->drawing_area->window, ibuf->gc, ibuf->lx, ibuf->ly, x, y);
	 }
      	 IMAGE_MODIFIED;
      }
      else
      {
         if (tool == CLOSED_FREEHAND)
	 {
	    gdk_draw_polygon(ibuf->pixmap, ibuf->gc, image_buf_get_fill(ibuf), ibuf->pts, ibuf->num_pts);
	    gdk_draw_polygon(ibuf->drawing_area->window, ibuf->gc, image_buf_get_fill(ibuf), ibuf->pts, ibuf->num_pts);
	 }
	 else
	 {
	    gdk_draw_line(ibuf->pixmap, ibuf->gc, ibuf->mx, ibuf->my, x, y);
	    gdk_draw_line(ibuf->drawing_area->window, ibuf->gc, ibuf->mx, ibuf->my, x, y);
	 }
	 IMAGE_MODIFIED;

      }
      gdk_gc_set_line_attributes(ibuf->gc, gcvalues.line_width, gcvalues.line_style, gcvalues.cap_style, gcvalues.join_style);
      //gtk_widget_draw(ibuf->drawing_area, &rect);
      break;
      
   case MULTILINE:
   case LINE:
      gdk_gc_set_function(ibuf->gc, GDK_INVERT);
      if ((ibuf->mx > INT_MIN) && (ibuf->my > INT_MIN))
         gdk_draw_line(ibuf->drawing_area->window, ibuf->gc, ibuf->lx, ibuf->ly, ibuf->mx, ibuf->my);
      
      gdk_gc_set_function(ibuf->gc, gcvalues.function);   
      gdk_draw_line(ibuf->pixmap, ibuf->gc, ibuf->lx, ibuf->ly, x, y);
      gdk_draw_line(ibuf->drawing_area->window, ibuf->gc, ibuf->lx, ibuf->ly, x, y);
      IMAGE_MODIFIED;
      break;
      
   case OVAL:
      gdk_gc_set_function(ibuf->gc, GDK_INVERT);
      if ((ibuf->mx > INT_MIN) && (ibuf->my > INT_MIN))
         draw_oval(ibuf->drawing_area->window, ibuf->gc, FALSE, ibuf->lx, ibuf->ly, ibuf->mx, ibuf->my);
      
      gdk_gc_set_function(ibuf->gc, gcvalues.function);   
      draw_oval(ibuf->pixmap, ibuf->gc, image_buf_get_fill(ibuf), ibuf->lx, ibuf->ly, x, y);
      draw_oval(ibuf->drawing_area->window, ibuf->gc, image_buf_get_fill(ibuf), ibuf->lx, ibuf->ly, x, y);
      IMAGE_MODIFIED;
      break;
   
   case ARC:
      gdk_gc_set_function(ibuf->gc, GDK_INVERT);
      if ((ibuf->mx > INT_MIN) && (ibuf->my > INT_MIN))
         draw_arc(ibuf->drawing_area->window, ibuf->gc, FALSE, ibuf->lx, ibuf->ly, ibuf->mx, ibuf->my);
      
      gdk_gc_set_function(ibuf->gc, gcvalues.function);   
      draw_arc(ibuf->pixmap, ibuf->gc, image_buf_get_fill(ibuf), ibuf->lx, ibuf->ly, x, y);
      draw_arc(ibuf->drawing_area->window, ibuf->gc, image_buf_get_fill(ibuf), ibuf->lx, ibuf->ly, x, y);
      IMAGE_MODIFIED;
      break;
      
      
   case RECTANGLE:
      gdk_gc_set_function(ibuf->gc, GDK_INVERT);
      if ((ibuf->mx > INT_MIN) && (ibuf->my > INT_MIN))
         draw_rectangle(ibuf->drawing_area->window, ibuf->gc, FALSE, ibuf->lx, ibuf->ly, ibuf->mx, ibuf->my);
      
      gdk_gc_set_function(ibuf->gc, gcvalues.function);   
      draw_rectangle(ibuf->pixmap, ibuf->gc, image_buf_get_fill(ibuf), ibuf->lx, ibuf->ly, x, y);
      draw_rectangle(ibuf->drawing_area->window, ibuf->gc, image_buf_get_fill(ibuf), ibuf->lx, ibuf->ly, x, y);
      IMAGE_MODIFIED;
      break;	 
   
  
   case ERASE:
      gdk_gc_set_foreground(ibuf->gc, &(gcvalues.background));
      draw_rectangle(ibuf->pixmap, ibuf->gc, TRUE, x - erase_size, y - erase_size, x + erase_size, y + erase_size);
      draw_rectangle(ibuf->drawing_area->window, ibuf->gc, TRUE, x - erase_size, y - erase_size, x + erase_size, y + erase_size);
      IMAGE_MODIFIED;

      gdk_gc_set_foreground(ibuf->gc, &(gcvalues.foreground));
      break;
         
   case TEXT:
      gtk_widget_grab_focus(ibuf->drawing_area);
      gtk_widget_grab_default(ibuf->drawing_area);
      break;
   
   
   
   case PASTE:
      image_buf_adjust_according_to_points(ibuf, &x, &y);
      image_buf_put_region_image(ibuf, x, y);
      break;
         
   default:
      
   }
   ibuf->lx = INT_MIN;
   ibuf->ly = INT_MIN;
   ibuf->mx = INT_MIN;
   ibuf->my = INT_MIN;
   if (tool == MULTILINE)
   {
      ibuf->llx = x;
      ibuf->lly = y;
   }
   
   if (tool == PASTE)
      ;
   else
   if (tool == TEXT)
   {
      ibuf->llx = x;
      ibuf->lly = y;
   }
   else if (tool == LASSO)
   {
      insert_point(ibuf, ibuf->pts[0].x, ibuf->pts[0].y);
   }
   else if (tool == POLSELECT)
   {
      insert_point(ibuf, ibuf->pts[0].x, ibuf->pts[0].y);
   }
   else
      clear_points(ibuf);
}   


void handle_key_release(image_buf *ibuf, GdkEventKey *keyevent)
{
   GdkGCValues gcvalues;
   GdkColor white;
   int length;
   char tmp[MAX_TEXT_LENGTH];
   
   if (ibuf->current_tool == PASTE)
   {
      if (keyevent->keyval == GDK_Escape)
      {
         image_buf_clear_flash(ibuf);
	 image_buf_erase_dynamic_image(ibuf, ibuf->regionbuf, ibuf->lpx, ibuf->lpy);
	 image_buf_leave_paste_mode(ibuf);
	 return;
      }
      else if (keyevent->keyval == GDK_Return)
      {
         image_buf_pixmap_to_rgbbuf(ibuf, 0); /* make what's in the pixmap into the rgb buffer */
      }
   }
      
   
   if ((ibuf->llx == INT_MIN) || (ibuf->lly == INT_MIN) || (ibuf->current_tool != TEXT))
   {
      gdk_beep();
      return;
   }
   
   
   
   image_buf_clear_flash(ibuf);
   
   gdk_gc_get_values(ibuf->gc, &gcvalues);
   strcpy(tmp, ibuf->textbuf);
   length = strlen(ibuf->textbuf);
   if ((keyevent->keyval == GDK_BackSpace) || (keyevent->keyval == GDK_Delete))
   {
      if (length)
         ibuf->textbuf[length - 1] = 0;
   }
   else if (keyevent->string)
      strcat(ibuf->textbuf, keyevent->string);
   else if ((keyevent->keyval >= GDK_space) && (keyevent->keyval < GDK_Shift_L))
   {
      sprintf(ibuf->textbuf, "%s%c", tmp, keyevent->keyval);
   }
   if (keyevent->keyval == GDK_Escape)
   {
      strcpy(ibuf->textbuf, "");
   }
   
   gdk_gc_set_function(ibuf->gc, GDK_INVERT);
   gdk_color_white(gdk_rgb_get_cmap(), &white);
   
   gdk_gc_set_foreground(ibuf->gc, &white);   
   if (strlen(tmp))
   {
      gdk_draw_string(ibuf->drawing_area->window, ibuf->font, ibuf->gc, ibuf->llx, ibuf->lly, tmp);
      gdk_draw_string(ibuf->pixmap, ibuf->font, ibuf->gc, ibuf->llx, ibuf->lly, tmp);
   }
    
   if (strlen(ibuf->textbuf))
   {
      gdk_draw_string(ibuf->drawing_area->window, ibuf->font, ibuf->gc, ibuf->llx, ibuf->lly, ibuf->textbuf);
      gdk_draw_string(ibuf->pixmap, ibuf->font, ibuf->gc, ibuf->llx, ibuf->lly, ibuf->textbuf);
   }
   
   
   gdk_gc_set_function(ibuf->gc, gcvalues.function);   
   gdk_gc_set_foreground(ibuf->gc, &(gcvalues.foreground));
   
   if (keyevent->keyval == GDK_Return)
   {
      if (strlen(ibuf->textbuf))
      {
	 gdk_draw_string(ibuf->drawing_area->window, ibuf->font, ibuf->gc, ibuf->llx, ibuf->lly, ibuf->textbuf);
	 gdk_draw_string(ibuf->pixmap, ibuf->font, ibuf->gc, ibuf->llx, ibuf->lly, ibuf->textbuf);
	 IMAGE_MODIFIED;

      }
      strcpy(ibuf->textbuf, "");
      
      ibuf->lly +=  (int) (gdk_string_height(ibuf->font, "My") * 1.1 + 0.5);
   }
   else if (keyevent->keyval == GDK_Escape)
   {
      ibuf->llx = ibuf->lly = INT_MIN;
   }
}


int handle_timeout(image_buf *ibuf)
{
   if (image_buf_has_focus(ibuf)) 
      image_buf_draw_flash(ibuf);
   else
      image_buf_clear_flash(ibuf);
   return 1;
}

void image_buf_paint_interpolate(image_buf *ibuf, int x, int y)
{
   double dx;                /* delta x */
   double dy;                /* delta y */
   double lastx = ibuf->mx;  
   double lasty = ibuf->my;
   double distance;          /* distance moved */
   double s = 0.0;           /* distance painted */
   double step = 1.0;        /* paint spacing */
   double factor;            /* ratio of painted to moved */
   int    changed = 0;       /* true if image has been changed */

   /* TODO: object-oriented drawing tools */
   GdkGCValues gcvalues;
   gdk_gc_get_values(ibuf->gc, &gcvalues);

   /* calculate distance moved: horizontal, vertical, and diag. */
   dx = (double)(x - ibuf->mx);
   dy = (double)(y - ibuf->my);
   distance = sqrt(dx*dx + dy*dy);

   /* return if no movement. */
   if (distance < 1.0) return;

   /* TODO: object-oriented drawing tools */
   if (ibuf->current_tool==ERASE)
   {
      gdk_gc_set_foreground(ibuf->gc, &(gcvalues.background));
   }

   /* paint along the movement. */
   while ( s < distance ) {
      s += step;          
      factor = s / distance;
      x = (int)(lastx + factor * dx);
      y = (int)(lasty + factor * dy);

      /* TODO: object-oriented drawing tools */
      draw_rectangle(ibuf->pixmap, ibuf->gc, 
                     TRUE, x - erase_size, y - erase_size, x + erase_size, y + erase_size);
      draw_rectangle(ibuf->drawing_area->window, ibuf->gc, 
                     TRUE, x - erase_size, y - erase_size, x + erase_size, y + erase_size);

      changed = 1;
   }
   if (changed) { 
      IMAGE_MODIFIED;
   }

   /* TODO: object-oriented drawing tools */
   if (ibuf->current_tool==ERASE)
   {
      gdk_gc_set_foreground(ibuf->gc, &(gcvalues.foreground));
   }
}



void image_buf_select_all(image_buf *ibuf)
{
   long w, h;
   
   w = image_buf_width(ibuf);
   h = image_buf_height(ibuf);
   
   image_buf_clear_flash(ibuf);
   
   clear_points(ibuf);
   insert_point(ibuf, 0, 0);
   insert_point(ibuf, w - 1, 0);
   insert_point(ibuf, w - 1, h - 1);
   insert_point(ibuf, 0, h - 1);
}


void image_buf_save_tool(image_buf *ibuf)
{
   ibuf->saved_tool = ibuf->current_tool;
}

void image_buf_restore_tool(image_buf *ibuf)
{
   set_drawing_tool(ibuf, ibuf->saved_tool);
}

void image_buf_enter_paste_mode(image_buf *ibuf)
{
   if (ibuf->modified)
   {
      image_buf_pixmap_to_rgbbuf(ibuf, 0);
   }
   image_buf_save_tool(ibuf);
   set_drawing_tool(ibuf, PASTE);
}

void image_buf_leave_paste_mode(image_buf *ibuf)
{
   image_buf_pixmap_to_rgbbuf(ibuf, 0);
   image_buf_restore_tool(ibuf);
}



void image_buf_draw_dynamic_image(image_buf *ibuf, GdkPixbuf *new_image, int x, int y)
{
   GdkRectangle rect ;
   
   if (!new_image)
   {
      gdk_beep();
      return;
   }
   
   rect.x = x;
   rect.y = y;
   rect.width = gdk_pixbuf_get_width(new_image);
   rect.height = gdk_pixbuf_get_height(new_image);
   
   if (x + rect.width >= image_buf_width(ibuf))
      rect.width = image_buf_width(ibuf) - x - 1;
   
   if (y + rect.height >= image_buf_height(ibuf))
      rect.height = image_buf_height(ibuf) - y - 1;
   
   image_buf_clear_flash(ibuf);
   
   if ((image_buf_width(ibuf) - x - 1 < 0) || (image_buf_height(ibuf) - y - 1 < 0))
      return;
      
   if ((x + rect.width < 0) || (y + rect.height < 0))   
      return;
   
   image_buf_rgbbuf_to_pixmap(ibuf, &rect);
   
   gdk_pixbuf_render_to_drawable_alpha(new_image, ibuf->pixmap, 0, 0, x, y, rect.width, rect.height, GDK_PIXBUF_ALPHA_BILEVEL, 255 / 2, GDK_RGB_DITHER_MAX, 0, 0);
   gtk_widget_queue_draw_area(ibuf->drawing_area, rect.x, rect.y, rect.width, rect.height);
   gtk_widget_draw(ibuf->drawing_area, &rect);
}



void image_buf_erase_dynamic_image(image_buf *ibuf, GdkPixbuf *new_image, int x, int y)
{
   GdkRectangle rect ;
   
   if (!new_image)
   {
      gdk_beep();
      return;
   }
   
   rect.x = x;
   rect.y = y;
   rect.width = gdk_pixbuf_get_width(new_image);
   rect.height = gdk_pixbuf_get_height(new_image);
   
   if (x + rect.width >= image_buf_width(ibuf))
      rect.width = image_buf_width(ibuf) - x - 1;
   
   if (y + rect.height >= image_buf_height(ibuf))
      rect.height = image_buf_height(ibuf) - y - 1;
      
   image_buf_clear_flash(ibuf);
   
   if ((image_buf_width(ibuf) - x - 1 < 0) || (image_buf_height(ibuf) - y - 1 < 0))
      return;
   if ((x + rect.width < 0) || (y + rect.height < 0))   
      return;
   
   image_buf_rgbbuf_to_pixmap(ibuf, &rect);
   gtk_widget_queue_draw_area(ibuf->drawing_area, rect.x, rect.y, rect.width, rect.height);
}   

void image_buf_put_region_image(image_buf *ibuf, int x, int y)
{
   if ((ibuf->lpx != INT_MIN) && (ibuf->lpy != INT_MIN))
   {
      image_buf_erase_dynamic_image(ibuf, ibuf->regionbuf, ibuf->lpx, ibuf->lpy);
   }
   image_buf_draw_dynamic_image(ibuf, ibuf->regionbuf, x, y);
   image_buf_set_pts_top_left(ibuf, x, y); 
   ibuf->lpx = x;
   ibuf->lpy = y;   
   /*gtk_widget_grab_focus(ibuf->drawing_area);*/

}   

void image_buf_flip_x_region(image_buf *ibuf)
{
   if ((ibuf->lpx != INT_MIN) && (ibuf->lpy != INT_MIN))
   {
      image_buf_erase_dynamic_image(ibuf, ibuf->regionbuf, ibuf->lpx, ibuf->lpy);
      pts_flip_x(ibuf->pts, ibuf->num_pts);
      pixbuf_flip_x(& ibuf->regionbuf);
      image_buf_draw_dynamic_image(ibuf, ibuf->regionbuf, ibuf->lpx, ibuf->lpy);
      if (ibuf->current_tool != PASTE)
      {
         image_buf_pixmap_to_rgbbuf(ibuf, 0);
      }
   }
   

}

void image_buf_flip_y_region(image_buf *ibuf)
{
   if ((ibuf->lpx != INT_MIN) && (ibuf->lpy != INT_MIN))
   {
      image_buf_erase_dynamic_image(ibuf, ibuf->regionbuf, ibuf->lpx, ibuf->lpy);
      pts_flip_y(ibuf->pts, ibuf->num_pts);
      pixbuf_flip_y(& ibuf->regionbuf);
      image_buf_draw_dynamic_image(ibuf, ibuf->regionbuf, ibuf->lpx, ibuf->lpy);
      if (ibuf->current_tool != PASTE)
      {
         image_buf_pixmap_to_rgbbuf(ibuf, 0);
      }
      
   }

}


void image_buf_rotate_region(image_buf *ibuf, double rad)
{
  // rotate the regionbuf by rad radians
   double m[2][2];
   int mx, my;
   GdkRectangle rect;

   m[0][0] = cos(rad);
   m[0][1] = -sin(rad);
   m[1][0] = sin(rad);
   m[1][1] = cos(rad);
   
   rect = compute_cover_rect(ibuf->pts, ibuf->num_pts);
   mx = rect.x + rect.width / 2;
   my = rect.y + rect.height / 2;
   
   image_buf_erase_dynamic_image(ibuf, 0, ibuf->lpx, ibuf->lpy);
   image_buf_set_pts_top_left(ibuf, -mx, -my);
   
   
   pixbuf_apply_matrix22(& ibuf->regionbuf, m);
   pts_apply_matrix22(ibuf->pts, ibuf->num_pts, m);
   rect = compute_cover_rect(ibuf->pts, ibuf->num_pts);
   
   ibuf->lpx = mx - rect.width / 2;
   ibuf->lpy = my - rect.height / 2;
   
   image_buf_set_pts_top_left(ibuf, ibuf->lpx, ibuf->lpy);
   
   image_buf_draw_dynamic_image(ibuf, ibuf->regionbuf, ibuf->lpx, ibuf->lpy);
      
   if (ibuf->current_tool != PASTE)
   {
      image_buf_pixmap_to_rgbbuf(ibuf, 0);
   }
   
  
}




void image_buf_get_desktop(image_buf *ibuf)
{

   GdkWindow *root = (GdkWindow *) &gdk_root_parent;
   int w, h;
   
   gdk_window_get_size (root, &w, &h);
   image_buf_resize(ibuf, w, h);
   gdk_pixbuf_get_from_drawable(ibuf->rgbbuf, root, 0, 0, 0, 0, 0, w, h);
   image_buf_rgbbuf_to_pixmap(ibuf, 0);
   gtk_widget_draw(ibuf->drawing_area, NULL);
}
   


void image_buf_set_desktop_background(image_buf* ibuf, int type)
{/* type: 0:centered, 1: tiled */ /* not working yet */

   GdkWindow *root = (GdkWindow *) &gdk_root_parent;
   int w, h;
   
   gdk_window_get_size (root, &w, &h);
   gdk_window_set_back_pixmap(root, ibuf->pixmap, FALSE);
   gdk_window_clear(root);
   
}


void draw_palette_entry(image_buf *ibuf, GtkWidget *palette_entry)
{
   GdkRectangle clientrect ;
   gchar *name = gtk_widget_get_name(palette_entry);
   guint32 color;
   int i, n;
   GdkGCValues gcvalues;
   
   i = sscanf(name, "color_palette_entry%d", &n);
   assert(i);
   
   clientrect.x = clientrect.y = 0;
   clientrect.width = palette_entry->allocation.width;
   clientrect.height = palette_entry->allocation.height;
   
   gdk_gc_get_values(ibuf->gc, &gcvalues);
   gdk_gc_set_clip_origin(ibuf->gc, 0, 0);
   gdk_gc_set_clip_rectangle(ibuf->gc, &clientrect);
   color = ibuf->palettes[n].red >> 8 << 16 | ibuf->palettes[n].green >> 8 << 8 | ibuf->palettes[n].blue >> 8;
   
   gdk_rgb_gc_set_foreground(ibuf->gc, color);
   draw_rectangle(palette_entry->window, ibuf->gc, TRUE,   clientrect.x, clientrect.y, clientrect.x + clientrect.width, clientrect.y + clientrect.height);
   gdk_gc_set_foreground(ibuf->gc, &(gcvalues.foreground));
  
}


void GdkColor_to_rgb(const GdkColor *color, unsigned char *r, unsigned char *g, unsigned char *b)
{
   
   GdkColormap *cmap = gdk_rgb_get_cmap();
   GdkVisual *visual = gdk_rgb_get_visual();
   GdkColor *c;
   switch(visual->type)
   {
   case GDK_VISUAL_STATIC_COLOR:
   case GDK_VISUAL_PSEUDO_COLOR:
      c = cmap->colors + color->pixel;
  
      *r = c->red >> 8;
      *g = c->green >> 8;
      *b = c->blue >> 8;
      break;
   case GDK_VISUAL_TRUE_COLOR:
   case GDK_VISUAL_DIRECT_COLOR:
      *r = (color->pixel & visual->red_mask) >> visual->red_shift << (sizeof(unsigned char) * 8 - visual->red_prec);
      *g = (color->pixel & visual->green_mask) >> visual->green_shift << (sizeof(unsigned char) * 8 - visual->green_prec);
      *b = (color->pixel & visual->blue_mask) >> visual->blue_shift << (sizeof(unsigned char) * 8 - visual->blue_prec);
      break;
   
   default:
      assert(0);
      break;
   }
 }
      
#include "image_processing.h"

void spreadimage(image_buf * input, image_buf *output)
{
   gdk_pixbuf_copy_area(input->rgbbuf, 0, 0, image_buf_width(input), image_buf_height(input), output->rgbbuf, 0, 0);
   ImageSpread(output);
   image_buf_set_modified(output, 1);
 
}



GdkRectangle compute_cover_rect(GdkPoint *pts, int num_pts)
{
   
   GdkRectangle rect;
   int i;
   int x = INT_MAX, y = INT_MAX, x2 = INT_MIN, y2 = INT_MIN;
   assert(num_pts);
   for (i = 0; i < num_pts; i++)
   {
      if (x > pts[i].x)
         x = pts[i].x;
      if (y > pts[i].y)
         y = pts[i].y;
      if (x2 < pts[i].x)
         x2 = pts[i].x;
      if (y2 < pts[i].y)
         y2 = pts[i].y;
   }
   rect.x = x;
   rect.y = y;
   rect.width = x2 - x + 1;
   rect.height = y2 - y + 1;
   return rect;
}
   
void image_buf_adjust_according_to_points(image_buf *ibuf, int *x, int *y)
{
   
   *x += ibuf->ptdiffx;
   *y += ibuf->ptdiffy;
}

void pts_flip_x(GdkPoint *pts, int num_pts)
{
   GdkRectangle rect = compute_cover_rect(pts, num_pts);
   int i;
   int x = rect.x + ((rect.width)) / 2;
   if (rect.width & 1)
   {  /* odd */
      for (i = 0; i < num_pts; i++)
      {
	 pts[i].x = (int)(2 * x - pts[i].x);
      }
   }
   else
   {  /* even */
      for (i = 0; i < num_pts; i++)
      {
	 pts[i].x = (int)(2 * x - pts[i].x - 1);
      }
   }

}

void pts_flip_y(GdkPoint *pts, int num_pts)
{
   GdkRectangle rect = compute_cover_rect(pts, num_pts);
   int i;
   double  y = rect.y + ((rect.height)) / 2;
   if (rect.height & 1)
   {  /* odd */
      for (i = 0; i < num_pts; i++)
      {
	 pts[i].y = (int)(2 * y - pts[i].y);
      }
   }
   else
   {  /* even */
      for (i = 0; i < num_pts; i++)
      {
	 pts[i].y = (int)(2 * y - pts[i].y - 1);
      }
   }
}

void pixbuf_flip_x(GdkPixbuf** pixbuf)
{

   int width, height;
   GdkPixbuf *newpixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, gdk_pixbuf_get_bits_per_sample(*pixbuf), width = gdk_pixbuf_get_width(*pixbuf), height = gdk_pixbuf_get_height(*pixbuf));
   int rowstride = gdk_pixbuf_get_rowstride(newpixbuf);
   int x, y;
   int j;
   unsigned char *p, *p2, *p3, *p4;
   int pixel_size = gdk_pixbuf_get_bits_per_sample(*pixbuf) * gdk_pixbuf_get_n_channels(*pixbuf) / 8;

   g_assert(newpixbuf);
   for (p = gdk_pixbuf_get_pixels(newpixbuf), p2 = gdk_pixbuf_get_pixels(*pixbuf), y = 0; y < height; y++, p += rowstride, p2 += rowstride)
   {
      p4 = p2 + pixel_size * (width - 1);
      p3 = p;
      for (x = 0; x < width; x++)
      {
	 for (j = 0; j < pixel_size; j++)
	    *(p3 + j) = *(p4 + j);
         p3 += pixel_size;
	 p4 -= pixel_size;
      }
     
   }
   gdk_pixbuf_unref(*pixbuf); 
   *pixbuf = newpixbuf;     
}

void pixbuf_flip_y(GdkPixbuf** pixbuf)
{

   int width, height;
   GdkPixbuf *newpixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, gdk_pixbuf_get_bits_per_sample(*pixbuf), width = gdk_pixbuf_get_width(*pixbuf), height = gdk_pixbuf_get_height(*pixbuf));
   int rowstride = gdk_pixbuf_get_rowstride(newpixbuf);
   int x, y;
   int j;
   unsigned char *p, *p2, *p3, *p4;
   int pixel_size = gdk_pixbuf_get_bits_per_sample(*pixbuf) * gdk_pixbuf_get_n_channels(*pixbuf) / 8;

   g_assert(newpixbuf);
   for (p = gdk_pixbuf_get_pixels(newpixbuf), p2 = gdk_pixbuf_get_pixels(*pixbuf) + rowstride * (height - 1), y = 0; y < height; y++, p += rowstride, p2 -= rowstride)
   {
      p3 = p;
      p4 = p2;
      for (x = 0; x < width; x++)
      {
	 for (j = 0; j < pixel_size; j++)
	    *(p3 + j) = *(p4 + j);
         p3 += pixel_size;
	 p4 += pixel_size;
      }
      
   }
   gdk_pixbuf_unref(*pixbuf); 
   *pixbuf = newpixbuf;     

}

/* the below 2d transform routine contains code 
   taken from xpaint, with the following copyright notice */
/* +-------------------------------------------------------------------+ */
/* | Copyright 1992, 1993, David Koblas (koblas@netcom.com)            | */
/* | Copyright 1995, 1996 Torsten Martinsen (bullestock@dk-online.dk)  | */
/* |                                                                   | */
/* | Permission to use, copy, modify, and to distribute this software  | */
/* | and its documentation for any purpose is hereby granted without   | */
/* | fee, provided that the above copyright notice appear in all       | */
/* | copies and that both that copyright notice and this permission    | */
/* | notice appear in supporting documentation.  There is no           | */
/* | representations about the suitability of this software for        | */
/* | any purpose.  this software is provided "as is" without express   | */
/* | or implied warranty.                                              | */
/* |                                                                   | */
/* +-------------------------------------------------------------------+ */


#define XFORM(x,y,mat,nx,ny)	nx = mat[0][0] * x + mat[0][1] * y; \
				ny = mat[1][0] * x + mat[1][1] * y
#define COPY_MAT(s,d)	d[0][0] = s[0][0]; d[0][1] = s[0][1]; \
			d[1][0] = s[1][0]; d[1][1] = s[1][1]

#define INVERT_MAT(mat, inv) do {			\
		float _d = 1.0 / (mat[0][0] * mat[1][1] \
			      - mat[0][1] * mat[1][0]);	\
		(inv)[0][0] =  (mat)[1][1] * _d;	\
		(inv)[1][1] =  (mat)[0][0] * _d;	\
		(inv)[0][1] = -(mat)[0][1] * _d;	\
		(inv)[1][0] = -(mat)[1][0] * _d;	\
	} while (0)


#define ROUND(x) ((int)(x + 0.5))

void pixbuf_apply_matrix22(GdkPixbuf **pixbuf, double matrix[2][2])
{
   GdkPixbuf *newpixbuf;
   double invmatrix[2][2];
   int x, y, width, height, i, j;
   int rowstride1, rowstride2, pixel_size;
   unsigned char *p1, *p2, *p3, *p4;
   double tx[4], ty[4], minx, miny, maxx, maxy, nx, ny, width2, height2;
   /* find the new bounding box */
   
   width = gdk_pixbuf_get_width(*pixbuf);
   height = gdk_pixbuf_get_height(*pixbuf);
   XFORM(-width / 2, -height / 2, matrix, tx[0], ty[0]);
   XFORM(-width / 2, height / 2, matrix, tx[1], ty[1]);
   XFORM(width / 2, -height / 2, matrix, tx[2], ty[2]);
   XFORM(width / 2, height / 2, matrix, tx[3], ty[3]);
   minx = INT_MAX;
   miny = INT_MAX;
   maxx = -INT_MAX;
   maxy = -INT_MAX;
   for (i = 0; i < 4; i++)
   {
      if (minx > tx[i])
         minx = tx[i];
      if (maxx < tx[i])
         maxx = tx[i];
      if (miny > ty[i])
         miny = ty[i];
      if (maxy < ty[i])
         maxy = ty[i];
   }
   width2 = maxx - minx;
   height2 = maxy - miny;
   
   
   INVERT_MAT(matrix, invmatrix);
   
   p1 = gdk_pixbuf_get_pixels(*pixbuf);
   newpixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, gdk_pixbuf_get_bits_per_sample(*pixbuf), ROUND(width2), ROUND(height2));
   g_assert(newpixbuf);
   p2 = gdk_pixbuf_get_pixels(newpixbuf);
   rowstride1 = gdk_pixbuf_get_rowstride(*pixbuf);
   rowstride2 = gdk_pixbuf_get_rowstride(newpixbuf);
   pixel_size = gdk_pixbuf_get_bits_per_sample(*pixbuf) * gdk_pixbuf_get_n_channels(*pixbuf) / 8;
   
#define PIXEL(x, y) (p1 + ROUND(y) * rowstride1 + ROUND(x) * pixel_size)
   
   for (y = miny; y < maxy; y++, p2 += rowstride2)
   {
      p4 = p2;
      for (x = minx; x < maxx; x++, p4 += pixel_size)
      {
         XFORM(x, y, invmatrix, nx, ny);
	 nx += width / 2;
	 ny += height / 2;
	 g_assert(nx >= 0);
	 g_assert(ny >= 0);
	 g_assert(nx < gdk_pixbuf_get_width(*pixbuf));
	 g_assert(ny < gdk_pixbuf_get_height(*pixbuf));
	 
	 p3 = PIXEL(nx, ny);
	 for (j = 0; j < pixel_size; j++)
	    *(p4 + j) = *(p3 + j);
      }
   }
   
   gdk_pixbuf_unref(*pixbuf);      
   
   *pixbuf = newpixbuf;

}

void pts_apply_matrix22(GdkPoint *pts, int num_pts, double matrix[2][2])
{
   int i;
   double nx, ny;
   for (i = 0; i < num_pts; i++)
   {
      XFORM(pts[i].x, pts[i].y, matrix, nx, ny);
      pts[i].x = ROUND(nx);
      pts[i].y = ROUND(ny);
   }
}
