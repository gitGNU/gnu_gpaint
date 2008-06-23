
/*
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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <gtk/gtk.h>
#include "image_buf.h"
#include "fileio.h"

#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480

extern GdkCursor *busy_cursor, *arrow_cursor; 

#define PROCESSING_GTK_EVENTS    while (gtk_events_pending()) gtk_main_iteration();

GtkWidget *widget_get_toplevel_parent(GtkWidget *widget);

image_buf *widget_get_image(GtkWidget *widget);

void new_canvas();

GtkWidget *create_scroll_frame_widget(void);
GtkWidget *create_scroll_frame_widget_content(GtkWidget *, GtkWidget *);

void open_canvas(image_buf *ibuf);

void save_canvas(image_buf *ibuf, int saveas);


void set_button_pixmap(GtkButton *button, unsigned char **pixmap);

void set_drawing_tool(image_buf *, DRAWING_TOOL);

void select_toolbar_toggle_button(GtkToggleButton *selected, int setmode);

void StateTimeStep();


void handle_button_press(image_buf *, int x, int y);
void handle_button_move(image_buf *, int x, int y);
void handle_button_release(image_buf *, int x, int y);

void handle_key_release(image_buf *, GdkEventKey *);

int handle_timeout(image_buf * );

void image_buf_set_tool(image_buf *ibuf, DRAWING_TOOL tool);

void image_buf_set_fill(image_buf *ibuf, int filled);

void image_buf_clear_flash(image_buf *ibuf);

void image_buf_select_all(image_buf *ibuf);

void spreadimage(image_buf * input, image_buf *output);

void GdkColor_to_rgb(const GdkColor *color, unsigned char *r, unsigned char *g, unsigned char *b);

void image_buf_clear(image_buf *ibuf);

void draw_rectangle(GdkDrawable *d, GdkGC *gc, gint filled, gint x1, gint y1, gint x2, gint y2);

void draw_palette_entry(image_buf *ibuf, GtkWidget *palette_entry);


void image_buf_save_tool(image_buf *);
void image_buf_restore_tool(image_buf *);

void image_buf_enter_paste_mode(image_buf *);
void image_buf_leave_paste_mode(image_buf *);

void image_buf_put_region_image(image_buf *, int, int);

void image_buf_draw_dynamic_image(image_buf *, GdkPixbuf *, int, int);
void image_buf_erase_dynamic_image(image_buf *, GdkPixbuf *, int, int);

void image_buf_flip_x_region(image_buf *);
void image_buf_flip_y_region(image_buf *);
void image_buf_rotate_region(image_buf *, double rad);

GdkRectangle compute_cover_rect(GdkPoint *pts, int num_pts);

void image_buf_adjust_according_to_points(image_buf *ibuf, int *, int *);

void image_buf_get_desktop(image_buf *);

void image_buf_set_desktop_background(image_buf*, int);

void pts_flip_x(GdkPoint *, int);
void pts_flip_y(GdkPoint *, int);
void pixbuf_flip_x(GdkPixbuf**);
void pixbuf_flip_y(GdkPixbuf**);
void pixbuf_apply_matrix22(GdkPixbuf **pixbuf, double matrix[2][2]);
void pts_apply_matrix22(GdkPoint *pts, int num_pts, double matrix[2][2]);

void image_buf_force_select_region(image_buf *ibuf);

void image_buf_paint_interpolate(image_buf *ibuf, int x, int y);


#endif
