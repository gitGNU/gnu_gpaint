
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


#ifndef __IMAGE_BUF_H__
#define __IMAGE_BUF_H__

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef enum  { ERASE, LASSO, FILL, LINE, MULTILINE, RECTANGLE, CLOSED_FREEHAND, PEN, POLSELECT, TEXT, ARC, CURVE, OVAL, BRUSH, NONE, PASTE = 1000}
DRAWING_TOOL ;


#define MAX_NUM_POINTS 5000
#define MAX_TEXT_LENGTH 2000

#define UNTITLED_NAME "*untitled"


#define NUM_PALETTE_ENTRIES (14 * 2)


#define IMAGE_MODIFIED {\
       image_buf_set_modified(ibuf, 1);\
       /*image_buf_pixmap_to_rgbbuf(ibuf, 0);*/\
    }
      



typedef struct image_buf
{
   GdkPixbuf *rgbbuf, *regionbuf; 
   GdkPixmap *pixmap; 
   GdkGC *gc;
   GtkWidget *window;
   GtkWidget *drawing_area;
   char *name;
   GtkWidget *current_button;
   DRAWING_TOOL current_tool, saved_tool;
   GdkCursor *cursor;
   GdkFont *font;
   int filled;
   int timer;
   int flash_state;
   
   GdkPoint pts[MAX_NUM_POINTS]; /* a temporary array for storing points */
   int num_pts;
   int ptdiffx;
   int ptdiffy;
   
   int modified;
   int has_focus;
   int lx;
   int ly;
   int mx;
   int my;
   int llx;
   int lly;
   
   int lpx;
   int lpy;
   
   char textbuf[MAX_TEXT_LENGTH];
   
   int current_palette;
   GdkColor palettes[NUM_PALETTE_ENTRIES];
} image_buf;


int image_buf_width(image_buf*);
int image_buf_height(image_buf*);
unsigned char *image_buf_rgbbuf(image_buf*);
int image_buf_rowstride(image_buf*);
int image_buf_pixelsize(image_buf*);

void image_buf_set_cursor(image_buf *, GdkCursor*);
void image_buf_set_font(image_buf *, GdkFont*, const char *);

void image_buf_process_in_place(image_buf *, void (*f)(image_buf *, image_buf *));
void image_buf_set_name(image_buf* ibuf, const char *n);

void image_buf_set_foreground_to_palette(image_buf *, int);
void image_buf_set_background_to_palette(image_buf *, int);

void image_buf_pixmap_to_rgbbuf(image_buf *, GdkRectangle *irect);
void image_buf_rgbbuf_to_pixmap(image_buf *, GdkRectangle *irect);




image_buf *create_image_buf(const char *name, int width, int height);
void image_buf_resize(image_buf *ibuf, int width, int height);

void close_image_buf(image_buf *ibuf);



void image_buf_flood_fill(image_buf *ibuf, int x, int y, const GdkColor *);

int image_buf_get_fill(image_buf *ibuf);

void image_buf_set_modified(image_buf *ibuf, int modified);

void image_buf_focus_obtained(image_buf*);
void image_buf_focus_lost(image_buf*);
int image_buf_has_focus(image_buf*);

void image_buf_set_pts_top_left(image_buf * ibuf, int x, int y);
void image_buf_get_selected_region_to_pixbuf(image_buf *ibuf, GdkPixbuf **pixbuf); /* copy the selected region of the image into a GdkPixbuf */
void image_buf_select_current_region(image_buf *ibuf);


#endif
