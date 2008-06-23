
/*
    Copyright 2000  Li-Cheng (Andy) Tai (atai@gnu.org)

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
#include <stdlib.h>
#include <string.h>
#include "image_buf.h"
#include "support.h"
#include "ui.h"
#include "util.h"
#include "version.h"

unsigned char init_palette_values[NUM_PALETTE_ENTRIES][3] = 
{ {255, 255, 255}, 
  {190, 190, 190}, 
  {255, 0, 0},
  {255, 255, 0},
  {0, 255, 0},
  {0, 255, 255},
  {0, 0, 255},
  {255, 0, 255},
  {255, 255, 121},
  {0, 255, 121},
  {121, 255, 255},
  {134, 125, 255},
  {255, 0, 121},
  {255, 125, 65},
  {0, 0, 0},
  {121, 125, 121},
  {121, 0, 0},
  {121, 130, 0},
  {0, 125, 0},
  {0, 125, 121},
  {0, 0, 121},
  {121, 0, 121}, 
  {121, 125, 65},
  {0, 65, 65},
  {0, 130, 255},
  {65, 0, 255},
  {0, 125, 65}
};

static void set_palette_entries(image_buf *ibuf)
{
   int i;
   for (i = 0; i < NUM_PALETTE_ENTRIES; i++)
   {
      ibuf->palettes[i].red = (short) (init_palette_values[i][0] * 255);
      ibuf->palettes[i].green = (short) (init_palette_values[i][1] * 255);
      ibuf->palettes[i].blue = (short) (init_palette_values[i][2] * 255);
   }
}
   
static image_buf * make_image_buf()
{
   image_buf *buf = calloc(sizeof(image_buf), 1);
   
   set_palette_entries(buf);
 
   return buf;
}   


static void clear_image_buf(image_buf *ibuf)
{
   if (ibuf->rgbbuf)
      gdk_pixbuf_unref(ibuf->rgbbuf);
   if (ibuf->regionbuf)
      gdk_pixbuf_unref(ibuf->regionbuf);
   if (ibuf->pixmap)
      gdk_pixmap_unref(ibuf->pixmap);
   if (ibuf->gc)
      gdk_gc_unref(ibuf->gc);   
   if (ibuf->font)
      gdk_font_unref(ibuf->font);   
   if (ibuf->timer)
      gtk_timeout_remove(ibuf->timer);      
   if (ibuf->name)
      free(ibuf->name);
  
   ibuf->rgbbuf = 0;
   ibuf->regionbuf = 0;
   ibuf->pixmap = 0;
   ibuf->gc = 0;
   ibuf->name = 0;
   ibuf->font = 0;
   ibuf->timer = 0;
   ibuf->flash_state = 0;
   ibuf->modified = 0;
   
   memset(ibuf->textbuf, 0, sizeof(ibuf->textbuf));
   if (ibuf->drawing_area)
      gtk_drawing_area_size(GTK_DRAWING_AREA(ibuf->drawing_area), 1, 1);
      
   
}


static void free_image_buf(image_buf* ibuf)
{
   if (!ibuf)
      return;
      
   clear_image_buf(ibuf);
   free(ibuf);
}

const char *initial_font_name = "-*-helvetica-*-r-normal--*-120-*-*-*-*-iso8859-1";
const int white_pixel = 255;
const int black_palette = 14;
const int white_palette = 0;

static void image_buf_create_pixmap(image_buf *ibuf)
{
   int width, height;

   g_assert(ibuf);
   g_assert(ibuf->window);
   g_assert(ibuf->pixmap == 0);
   g_assert(ibuf->rgbbuf == 0);
   g_assert(ibuf->gc == 0);
   g_assert(ibuf->font == 0);
   g_assert(ibuf->timer == 0);
   width = ibuf->drawing_area->requisition.width;
   height = ibuf->drawing_area->requisition.height;
   
   if (width && height)
   {
      ibuf->rgbbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
      g_assert(ibuf->rgbbuf);

      /* set the initial image to white, so it looks like a blank sheet of paper */
      memset(image_buf_rgbbuf(ibuf), white_pixel, 
        image_buf_rowstride(ibuf) * image_buf_height(ibuf) * sizeof(unsigned char));
      
      ibuf->pixmap = gdk_pixmap_new(ibuf->window->window, image_buf_width(ibuf), image_buf_height(ibuf), -1);
      g_assert(ibuf->pixmap);
      ibuf->gc = gdk_gc_new(ibuf->pixmap);
      g_assert(ibuf->gc);

      ibuf->cursor = 0;
      ibuf->filled = 0;
      ibuf->flash_state = 0;
      image_buf_set_font(ibuf, gdk_font_load(initial_font_name), initial_font_name);
      image_buf_rgbbuf_to_pixmap(ibuf, 0);
      image_buf_set_foreground_to_palette(ibuf, black_palette);
      image_buf_set_background_to_palette(ibuf, white_palette);
      memset(ibuf->textbuf, 0, sizeof(ibuf->textbuf));
      ibuf->timer = gtk_timeout_add(500, (GtkFunction)(handle_timeout), ibuf);
   }
   else
      g_assert(0);
}

void image_buf_resize(image_buf *ibuf, int width, int height)
{
   if (ibuf->pixmap)
      gdk_pixmap_unref(ibuf->pixmap);   
   if (ibuf->rgbbuf)
      gdk_pixbuf_unref(ibuf->rgbbuf);
   if (ibuf->gc)
      gdk_gc_unref(ibuf->gc);   
  
   if (ibuf->font)
      gdk_font_unref(ibuf->font);   
   if (ibuf->timer)
      gtk_timeout_remove(ibuf->timer);   
  
   ibuf->rgbbuf = 0;
   ibuf->pixmap = 0;
   ibuf->gc = 0;
   ibuf->font = 0;
   ibuf->timer = 0;
   gtk_drawing_area_size(GTK_DRAWING_AREA(ibuf->drawing_area), width, height);
  
   image_buf_create_pixmap(ibuf);
}

int image_buf_width(image_buf* ibuf)
{
   if (ibuf->rgbbuf)
      return gdk_pixbuf_get_width(ibuf->rgbbuf);
   return 0;
}

int image_buf_height(image_buf* ibuf)
{
   if (ibuf->rgbbuf)
      return gdk_pixbuf_get_height(ibuf->rgbbuf);
   return 0;
}

unsigned char *image_buf_rgbbuf(image_buf* ibuf)
{
   if (ibuf->rgbbuf)
      return gdk_pixbuf_get_pixels(ibuf->rgbbuf);
   return 0;
}

int image_buf_rowstride(image_buf *ibuf)
{
   return gdk_pixbuf_get_rowstride(ibuf->rgbbuf);
}

int image_buf_pixelsize(image_buf *ibuf)
{
   return gdk_pixbuf_get_n_channels(ibuf->rgbbuf) * gdk_pixbuf_get_bits_per_sample(ibuf->rgbbuf) / 8;
}

GHashTable *images = 0;

image_buf *create_image_buf(const char *name, int width, int height)
{
   const int window_width = MIN(width+120, 740);
   const int window_height = MIN(height+220, 680);

   GtkWidget *widget = create_mainwindow();
   image_buf *ibuf;
   GtkWidget *scroll_frame = lookup_widget(widget, "scroll_frame");
   create_scroll_frame_widget_content(scroll_frame, widget);
   
   gtk_window_set_default_size(GTK_WINDOW(widget), window_width, window_height); 
   gtk_widget_show (widget);
      
   ibuf = make_image_buf(width, height);
   ibuf->window = widget;
   ibuf->drawing_area = lookup_widget(widget, "drawingarea");
   gtk_object_set_user_data(GTK_OBJECT(ibuf->window),  ibuf);
   gtk_object_set_user_data(GTK_OBJECT(ibuf->drawing_area), ibuf);
   gtk_drawing_area_size(GTK_DRAWING_AREA(ibuf->drawing_area), width, height);

   image_buf_create_pixmap(ibuf);
   
   image_buf_set_name(ibuf, name);
   
   set_drawing_tool(ibuf, PEN);
   return ibuf;
}

void close_image_buf(image_buf *ibuf)
{
   image_buf *t = g_hash_table_lookup(images, ibuf->name);
   g_assert(t == ibuf);
   
 
   
   g_hash_table_remove(images,  ibuf->name);
   if (ibuf->drawing_area)
      gtk_widget_destroy(ibuf->drawing_area);
   if (ibuf->window)
      gtk_widget_destroy(ibuf->window);
  
   if (ibuf->cursor)
      gdk_cursor_destroy(ibuf->cursor);
   ibuf->drawing_area = 0;
   ibuf->window = 0;
   ibuf->cursor = 0;
      
   free_image_buf(ibuf);
   if (g_hash_table_size(images) == 0)
      gtk_exit(0);
}

void image_buf_process_in_place(image_buf *ibuf, void (*f)(image_buf *, image_buf *))
{
   image_buf tmpbuf = *ibuf;
   int width = image_buf_width(ibuf);
   int height = image_buf_height(ibuf);
   image_buf_pixmap_to_rgbbuf(ibuf, 0);
   tmpbuf.rgbbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
   f(ibuf, &tmpbuf);
   gdk_pixbuf_unref(ibuf->rgbbuf);   
   ibuf->rgbbuf = tmpbuf.rgbbuf;
   image_buf_rgbbuf_to_pixmap(ibuf, 0);
}
   

void image_buf_set_name(image_buf* ibuf, const char *n)
{
   char tmp[2000];
   image_buf *t ;
   static int untitledno = 0;
   if (!images)
      images = g_hash_table_new(NULL, NULL);
   if (ibuf->name)
   {
      t = g_hash_table_lookup(images, ibuf->name);
      g_assert(t);
      g_hash_table_remove(images, ibuf->name);
   }
   if (ibuf->name)
      free(ibuf->name);
   
   if (n)
   {   
      ibuf->name = strdup(n);
      g_hash_table_insert(images, ibuf->name, ibuf);
      sprintf(tmp, "%s: %s", PROGRAM_TITLE, ibuf->name);
   }
   else
   {
      sprintf(tmp, "%s %d", UNTITLED_NAME, untitledno);
      untitledno++;
       
      ibuf->name = strdup(tmp);
      g_hash_table_insert(images, ibuf->name, ibuf);
      sprintf(tmp, "%s: %s", PROGRAM_TITLE, "untitled");
   }
   
   gtk_window_set_title(GTK_WINDOW(ibuf->window), tmp);
   
   
}
void image_buf_pixmap_to_rgbbuf(image_buf *ibuf, GdkRectangle *irect)
{
   GdkRectangle fullrect ;
   GdkRectangle rect;
   fullrect.x = 0;
   fullrect.y = 0;
   fullrect.width = image_buf_width(ibuf);
   fullrect.height = image_buf_height(ibuf);
   
   if (!ibuf->modified && irect)
      gdk_rectangle_intersect(&fullrect, irect, &rect);
   else
      rect = fullrect;
   gdk_pixbuf_get_from_drawable(ibuf->rgbbuf, ibuf->pixmap, gdk_rgb_get_cmap(), rect.x, rect.y, rect.x, rect.y, rect.width, rect.height);
   ibuf->modified = 0;
}

void image_buf_rgbbuf_to_pixmap(image_buf *ibuf, GdkRectangle *irect)
{
   GdkRectangle fullrect ;
   GdkRectangle rect;
   fullrect.x = 0;
   fullrect.y = 0;
   fullrect.width = image_buf_width(ibuf);
   fullrect.height = image_buf_height(ibuf);
   
   if (irect)
      gdk_rectangle_intersect(&fullrect, irect, &rect);
   else
      rect = fullrect;
   gdk_pixbuf_render_to_drawable(ibuf->rgbbuf, ibuf->pixmap, ibuf->gc, rect.x, rect.y, rect.x, rect.y, rect.width, rect.height, GDK_RGB_DITHER_NORMAL, 0, 0);
}

struct fillinfo
{
   unsigned char *rgb; 
   int rowstride;
   int pixelsize; 
   int width;
   int height;
   unsigned char or, og, ob, r, g, b;
};

struct fillpixelinfo
{
   int y, xl, xr, dy;
};


static int is_old_pixel_value(struct fillinfo *info, int x, int y)
{
   unsigned char *p = info->rgb + y * info->rowstride + x * info->pixelsize;
   unsigned char or, og, ob;
   or = *p;
   og = *(p + 1);
   ob = *(p + 2);
   if ((or == info->or) && (og == info->og) && (ob == info->ob))
      return 1;
   
   return 0;
}


static void set_new_pixel_value(struct fillinfo *info, int x, int y)
{
   unsigned char *p = info->rgb + y * info->rowstride + x * info->pixelsize;
   *p = info->r;
   *(p + 1) = info->g;
   *(p + 2) = info->b;
}


#define STACKSIZE 10000

/* algorithm based on SeedFill.c from GraphicsGems 1 */

static void flood_fill(struct fillinfo *info, int x, int y)
{
   struct fillpixelinfo stack[STACKSIZE];
   struct fillpixelinfo * sp = stack;
   
   int l, x1, x2, dy;
   
#define PUSH(py, pxl, pxr, pdy) \
{  struct fillpixelinfo *p = sp;\
   if (((py) + (pdy) >= 0) && ((py) + (pdy) < info->height))\
   {\
      p->y = (py);\
      p->xl = (pxl);\
      p->xr = (pxr);\
      p->dy = (pdy);\
      sp++; \
   }\
}
   
   
#define POP(py, pxl, pxr, pdy) \
{\
   sp--;\
   (py) = sp->y + sp->dy;\
   (pxl) = sp->xl;\
   (pxr) = sp->xr;\
   (pdy) = sp->dy;\
}

      
   if ((x >= 0) && (x < info->width) && (y >= 0) && (y < info->height))
   {
      if ((info->or == info->r) && (info->og == info->g) && (info->ob == info->b))
         return;
      
      PUSH(y, x, x, 1);
      PUSH(y + 1, x, x, -1);
      	 
      while (sp > stack)	
      {
         POP(y, x1, x2, dy);
	 for (x = x1; (x >= 0) && is_old_pixel_value(info, x, y); x--)
	    set_new_pixel_value(info, x, y);
	 if (x >= x1) goto skip;
	 l = x + 1;
	 if (l < x1)
	    PUSH(y, l, x1 - 1, -dy);
	 x = x1 + 1;
	 do
	 {
	    for (; (x < info->width) && is_old_pixel_value(info, x, y); x++)
	       set_new_pixel_value(info, x, y);
	    
	    PUSH(y, l, x - 1, dy);
	    if (x > x2 + 1)
	       PUSH(y, x2 + 1, x - 1, -dy);
skip:
            for (x++; x <= x2 && !is_old_pixel_value(info, x, y); x++) 
	       ;
	    
	    l = x;
	 } while (x <= x2);
	 	       
	
      }
   }

#undef POP
#undef PUSH
   	 
}   
   
void image_buf_flood_fill(image_buf *ibuf, int x, int y, const GdkColor *color)
{
   unsigned char r, g, b;
   struct fillinfo fillinfo;
   unsigned char *p ;
   image_buf_pixmap_to_rgbbuf(ibuf, 0);
   GdkColor_to_rgb(color, &r, &g, &b);
   
   fillinfo.rgb = image_buf_rgbbuf(ibuf);
   fillinfo.width = image_buf_width(ibuf);
   fillinfo.height = image_buf_height(ibuf);
   fillinfo.rowstride = image_buf_rowstride(ibuf);
   fillinfo.pixelsize = image_buf_pixelsize(ibuf);
   fillinfo.r = r;
   fillinfo.g = g;
   fillinfo.b = b;
   p = fillinfo.rgb + y * fillinfo.rowstride + x * fillinfo.pixelsize;
   fillinfo.or = *p;
   fillinfo.og = *(p + 1);
   fillinfo.ob = *(p + 2);
   flood_fill(&fillinfo, x, y);
   image_buf_rgbbuf_to_pixmap(ibuf, 0);
   ibuf->modified = 1;
}




void image_buf_set_cursor(image_buf *ibuf, GdkCursor* cursor)
{
   GdkCursor *oldcursor = ibuf->cursor;
   ibuf->cursor = cursor;
   gdk_window_set_cursor(ibuf->drawing_area->window, cursor);
   if (oldcursor)
      gdk_cursor_destroy(oldcursor);
}

void image_buf_set_font(image_buf *ibuf, GdkFont* font, const char *font_name)
{
   GdkFont *oldfont = ibuf->font;
   GnomeFontPicker *w = GNOME_FONT_PICKER(lookup_widget(ibuf->window, "fontpicker"));
   assert(w);
   ibuf->font = font;
   gnome_font_picker_set_mode(w, GNOME_FONT_PICKER_MODE_FONT_INFO);
   gnome_font_picker_fi_set_show_size(w, TRUE);
   gnome_font_picker_fi_set_use_font_in_label(w, TRUE, 12);
   gnome_font_picker_set_font_name(w, font_name);
   gdk_font_ref(ibuf->font);
   if (oldfont)
      gdk_font_unref(oldfont);
}
   

int image_buf_get_fill(image_buf *ibuf)
{
   return ibuf->filled;
}

void image_buf_set_modified(image_buf *ibuf, int modified)
{
   ibuf->modified = modified;
}

void image_buf_focus_obtained(image_buf *ibuf)
{
   ibuf->has_focus = 1;
}

void image_buf_focus_lost(image_buf *ibuf)
{
   image_buf_clear_flash(ibuf);
   ibuf->has_focus = 0;
   if (ibuf->current_tool == TEXT)
      image_buf_set_tool(ibuf, TEXT);
      
}

int image_buf_has_focus(image_buf *ibuf)
{
   return ibuf->has_focus;
}

/*
 * Convert the GdkColor object associated with a palette into
 * a flat 24-bit rgb integer. See set_palette_entries().
 */
static guint32 image_buf_get_palette_color(image_buf *ibuf, int ic)
{
   guint32 red, green, blue;
   g_assert(ibuf);
   g_assert(ic >= 0);
   g_assert(ic < NUM_PALETTE_ENTRIES);

   red   = ibuf->palettes[ic].red   >> 8;    /* convert 16-bits into 8-bits */
   green = ibuf->palettes[ic].green >> 8; 
   blue  = ibuf->palettes[ic].blue  >> 8;
   return (red << 16) | (green << 8) | blue; /* pack into one integer */
}

/*
 * Change a color picker to the color of the given palette.
 */
static void image_buf_set_picker(image_buf *ibuf, int ic, const char *widget_name)
{
   GnomeColorPicker *color_picker;
   g_assert(ibuf);
   g_assert(ic >= 0);
   g_assert(ic < NUM_PALETTE_ENTRIES);
   g_assert(widget_name);
   
   color_picker = (GnomeColorPicker*) lookup_widget(ibuf->window, widget_name);			
   g_assert(color_picker);
   gnome_color_picker_set_i16(color_picker, ibuf->palettes[ic].red, ibuf->palettes[ic].green, ibuf->palettes[ic].blue, 0);
}  

/*
 * Change the current foreground color.
 */
void image_buf_set_foreground_to_palette(image_buf *ibuf, int ic)
{
   guint32 color;
   g_assert(ibuf);

   color = image_buf_get_palette_color(ibuf, ic);
   gdk_rgb_gc_set_foreground(ibuf->gc, color);
   image_buf_set_picker(ibuf, ic, "foreground_color_picker");
   ibuf->current_palette = ic;
}   

/*
 * Change the current background color.
 */
void image_buf_set_background_to_palette(image_buf *ibuf, int ic)
{
   guint32 color;
   g_assert(ibuf);

   color = image_buf_get_palette_color(ibuf, ic);
   gdk_rgb_gc_set_background(ibuf->gc, color);
   image_buf_set_picker(ibuf, ic, "background_color_picker");
}
    
void image_buf_set_pts_top_left(image_buf * ibuf, int x, int y)
{
   GdkRectangle rect;
   int dx, dy;
   int i;
   if (!ibuf->num_pts)
      return;
   image_buf_clear_flash(ibuf);
   
   rect = compute_cover_rect(ibuf->pts, ibuf->num_pts);
   dx = -rect.x + x;
   dy = -rect.y + y;
   for (i = 0; i < ibuf->num_pts; i++)
   {
      ibuf->pts[i].x += dx;
      ibuf->pts[i].y += dy;
   }
}


/* taken from gd.c in gd 1.8.3 */
static int cmp_int(const void *a, const void *b)
{
	return (*(const int *)a) - (*(const int *)b);
}

static void fill_polygon(unsigned char *data, int pixelsize, int rowstride, unsigned char value[4], unsigned char mask[4], GdkPoint *pts, int num_pts, int xoffset, int yoffset)
{
   /* this assumes the data is in RGBA format, each channel takes 1 bytes (thus 4 bytes per pixel) */

   int x;
   int j;
   unsigned char *c;
   
/* taken from gd.c in gd 1.8.3 */
   
   int i;
   int y;
   int miny, maxy;
   int x1, y1;
   int x2, y2;
   int ind1, ind2;
   int ints;
   int * polyInts = 0;
   int polyAllocated = 0;
   
   assert(xoffset >= 0);
   assert(yoffset >= 0);
   
   if (!num_pts) {
	   return;
   }
   if (!polyAllocated) {
	   polyInts = (int *) malloc(sizeof(int) * num_pts);
	   polyAllocated = num_pts;
   }		

   miny = pts[0].y;
   maxy = pts[0].y;
   for (i=1; (i < num_pts); i++) {
	   if (pts[i].y < miny) {
		   miny = pts[i].y;
	   }
	   if (pts[i].y > maxy) {
		   maxy = pts[i].y;
	   }
   }
   /* Fix in 1.3: count a vertex only once */
   for (y=miny; (y <= maxy); y++) {
/*1.4		int interLast = 0; */
/*		int dirLast = 0; */
/*		int interFirst = 1; */
	   ints = 0;
	   for (i=0; (i < num_pts); i++) {
		   if (!i) {
			   ind1 = num_pts-1;
			   ind2 = 0;
		   } else {
			   ind1 = i-1;
			   ind2 = i;
		   }
		   y1 = pts[ind1].y;
		   y2 = pts[ind2].y;
		   if (y1 < y2) {
			   x1 = pts[ind1].x;
			   x2 = pts[ind2].x;
		   } else if (y1 > y2) {
			   y2 = pts[ind1].y;
			   y1 = pts[ind2].y;
			   x2 = pts[ind1].x;
			   x1 = pts[ind2].x;
		   } else {
			   continue;
		   }
		   if ((y >= y1) && (y < y2)) {
			   polyInts[ints++] = (y-y1) * (x2-x1) / (y2-y1) + x1;
		   } else if ((y == maxy) && (y > y1) && (y <= y2)) {
			   polyInts[ints++] = (y-y1) * (x2-x1) / (y2-y1) + x1;
		   }
	   }
	   qsort(polyInts, ints, sizeof(int), cmp_int);

	   for (i=0; (i < (ints)); i+=2) {
	      c = data + (y - yoffset) * rowstride + (polyInts[i] - xoffset) * pixelsize;
	      for (x = polyInts[i]; x <= polyInts[i+1] ; x++)
	      {
	         for (j = 0; j < pixelsize; j++, c++)
		 {
		    if (mask[j])
		       *c = value[j];
		 }
                 
	      }
	      
	   }
   }
   if (polyInts)
      free(polyInts);
}
   
void image_buf_select_current_region(image_buf *ibuf)
{
   if (ibuf->num_pts == 0)
   {
       return; /* nothing to select */
   }
   image_buf_get_selected_region_to_pixbuf(ibuf, & ibuf->regionbuf);
   
}
	 
void image_buf_get_selected_region_to_pixbuf(image_buf *ibuf, GdkPixbuf **pixbuf) /* copy the selected region of the image into a GdkPixbuf */
{
   GdkRectangle rect;
   unsigned char alphaonly[4] = {0, 0, 0, 1};
   unsigned char *data = 0, *c, *d;
   unsigned char default_pixel[4] = {0, 0, 0, 255};
   int x, y;
   int pixel_size;
   rect = compute_cover_rect(ibuf->pts, ibuf->num_pts);
   
   image_buf_pixmap_to_rgbbuf(ibuf, &rect);
   
   if (*pixbuf)
      gdk_pixbuf_unref(*pixbuf);
   *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, rect.width, rect.height);
   assert(*pixbuf);
   pixel_size = gdk_pixbuf_get_n_channels(*pixbuf) * gdk_pixbuf_get_bits_per_sample(*pixbuf) / 8;
   
   gdk_pixbuf_copy_area(ibuf->rgbbuf, rect.x, rect.y, rect.width, rect.height, *pixbuf, 0, 0);
   data = gdk_pixbuf_get_pixels( *pixbuf);
   for (y = 0, c = data; y < rect.height; y++, c += gdk_pixbuf_get_rowstride( *pixbuf))
   {
      d = c;
      for (x = 0; x < rect.width; x++, d += pixel_size)
      {
         *(d + 3) = 0;/* default everything is transparent */
      }
   }
   
   fill_polygon(data, pixel_size, gdk_pixbuf_get_rowstride( *pixbuf), default_pixel, alphaonly, ibuf->pts, ibuf->num_pts, rect.x, rect.y);
}
    
