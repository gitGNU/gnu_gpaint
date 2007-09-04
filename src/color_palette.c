/* $Id: color_palette.c,v 1.5 2004/12/03 23:06:33 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003, 2007  Li-Cheng (Andy) Tai
 * Copyright 2003       Michael A. Meffie
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

#include "canvas.h"
#include "callbacks.h"
#include "support.h"
#include "debug.h"
#include <gtk/gtk.h>

/*
 * Initial Palette colors. Red/Green/Blue values, 0 to 255.
 */
guint8 init_palette_values[][3] = 
{
    /* bottom row, left to right */
    {255, 255, 255},  
    {190, 190, 190}, 
    {255, 0,   0  },
    {255, 255, 0  },
    {0,   255, 0  },
    {0,   255, 255},
    {0,   0,   255},
    {255, 0,   255},
    {255, 255, 121},
    {0,   255, 121},
    {121, 255, 255},
    {134, 125, 255},
    {255, 0,   121},
    {255, 125, 65 },
   
    /* top row, left to right */
    {0,   0,   0  },
    {121, 125, 121},
    {121, 0,   0  },
    {121, 130, 0  },
    {0,   125, 0  },
    {0,   125, 121},
    {0,   0,   121},
    {121, 0,   121}, 
    {121, 125, 65 },
    {0,   65,  65 },
    {0,   130, 255},
    {65,  0,   255},
    {0,   125, 65 },
    {18,  52,  86 }
};

const int NUM_PALETTES = sizeof(init_palette_values)/sizeof(*init_palette_values);
const int RIGHT_BUTTON = 3;


typedef enum
{
    FOREGROUND,
    BACKGROUND
} gpaint_color_mode;

/*
 * Color swatch state data.
 */
typedef struct _gpaint_color_swatch
{
    GtkWidget *widget;  /* the drawing area for this swatch */
    GdkColor  color;    /* current color value */
    GdkGC     *gc;      /* the gc for this swatch */
} gpaint_color_swatch;

/*
 * Color selector dialog state data.
 */
typedef struct _gpaint_color_selector
{
    GtkWidget               *widget;
    gpaint_color_swatch     *swatch;
    gpaint_color_mode       mode;
    GtkColorSelection       *colorsel;
    GtkColorSelectionDialog *dialog;
} gpaint_color_selector;


static void
color_swatch_destroy(gpaint_color_swatch *swatch);

static void
on_color_selector_ok_button_clicked(GtkButton *button, gpointer user_data);

static void
on_color_selector_cancel_button_clicked(GtkButton *button, gpointer user_data);

static void
change_color(gpaint_color_swatch *swatch, gpaint_color_mode mode);

static void
change_foreground_color(gpaint_canvas *canvas, GdkColor *color);

static void
change_background_color(gpaint_canvas *canvas, GdkColor *color);
/*
 * Set the foreground color picker initial color.
 */
void
on_foreground_color_picker_realize     (GtkWidget       *widget,
                                        gpointer         user_data)
{
    GdkColor black; 
    gdk_color_black(gdk_colormap_get_system(), &black);
    gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), &black);
}

/*
 * Set the background color picker initial color.
 */
void
on_background_color_picker_realize     (GtkWidget       *widget,
                                        gpointer         user_data)
{
    GdkColor white; 
    gdk_color_white(gdk_colormap_get_system(), &white);
    gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),
            &white);
}

/*
 * Initialize a color swatch on the color palette tool bar.
 */
void
on_color_palette_entry_realize         (GtkWidget       *widget,
                                        gpointer         user_data)
{
    int n = 0;
    int i = 0;
    const gchar *name;
    gpaint_color_swatch *swatch = g_new(gpaint_color_swatch, 1);
    
    debug_fn2("widget=%s swatch=%p", gtk_widget_get_name(widget), swatch);
    g_assert(swatch);
    swatch->widget = widget;
    swatch->gc = gdk_gc_new(widget->window);
    g_assert(swatch->gc);
    
    /* Get the color for this swatch from the color table. */
    name = gtk_widget_get_name(widget);
    n = sscanf(name, "color_palette_entry%d", &i);
    g_assert(n==1);
    g_assert(0<=i && i<NUM_PALETTES);
    swatch->color.pixel = 0;
    swatch->color.red   = color16(init_palette_values[i][0]);
    swatch->color.green = color16(init_palette_values[i][1]);
    swatch->color.blue  = color16(init_palette_values[i][2]);
    
    gdk_color_alloc(gdk_colormap_get_system(), &(swatch->color));
    gdk_gc_set_foreground(swatch->gc, &(swatch->color));
    gtk_object_set_data_full(GTK_OBJECT(widget), "color_swatch", swatch, 
                              (GtkDestroyNotify)color_swatch_destroy);    
   
}

static void
color_swatch_destroy(gpaint_color_swatch *swatch)
{
    debug_fn1("swatch=%p", swatch);
    gdk_gc_unref(swatch->gc);
    memset(swatch, 0xBEBE, sizeof(gpaint_color_swatch));
    g_free(swatch);
}

/*
 * Paint the color palette using the gc created in the
 * realize callback.
 */
gboolean
on_color_palette_entry_expose_event    (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
    gpaint_color_swatch *swatch = 
        (gpaint_color_swatch*)gtk_object_get_data(GTK_OBJECT(widget), "color_swatch");
    debug_fn1("widget=%s", gtk_widget_get_name(widget));
    g_assert(swatch);
    gdk_draw_rectangle(
            widget->window, swatch->gc, TRUE,
            event->area.x, event->area.y,
            event->area.width, event->area.height );
    return FALSE;
}

/*
 * Handle mouse clicks in the color swatch. A single left or center
 * click selects this color to be the foreground color. A single
 * right click sets this color to be the background color. A double 
 * click creates a color selection dialog to let the user pick a
 * custom color.
 */
gboolean
on_color_palette_entry_button_press_event
                                        (GtkWidget      *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
    gpaint_color_swatch *swatch =
        (gpaint_color_swatch*)gtk_object_get_data(GTK_OBJECT(widget), "color_swatch");
    gpaint_color_mode mode;
    
    /* Change the background on right click, foregound on left click 
     * or center click. */
    g_assert(swatch);
    mode = (event->button==RIGHT_BUTTON ? BACKGROUND : FOREGROUND);
    if (event->type==GDK_BUTTON_PRESS)
    {
        change_color(swatch, mode);
    }
    else if (event->type==GDK_2BUTTON_PRESS)
    {
        /* show color selection dialog on double click */
        gdouble color[4];
        gpaint_color_selector *selector = g_new(gpaint_color_selector, 1);

        GtkWidget *toplevel = gtk_widget_get_toplevel(widget);
        GtkWidget *dialog = gtk_color_selection_dialog_new("Select Color");
       
        g_assert(toplevel);
        g_assert(selector);
        g_assert(dialog);
        
        selector->swatch    = swatch;
        selector->dialog    = GTK_COLOR_SELECTION_DIALOG(dialog);
        selector->colorsel  = GTK_COLOR_SELECTION(selector->dialog->colorsel);
        selector->mode      = mode;
        
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(toplevel));
        gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
        gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
        
        gtk_signal_connect(
                GTK_OBJECT(selector->dialog->ok_button),
                "clicked",
                GTK_SIGNAL_FUNC(on_color_selector_ok_button_clicked),
                (gpointer)selector);
        
        gtk_signal_connect(
                GTK_OBJECT(selector->dialog->cancel_button),
                "clicked",
                GTK_SIGNAL_FUNC(on_color_selector_cancel_button_clicked),
                (gpointer)selector);
       
        /* Set initial color to the swatch color. */
        color[0] = (gdouble)swatch->color.red   / MAX_16_BIT_COLOR; 
        color[1] = (gdouble)swatch->color.green / MAX_16_BIT_COLOR; 
        color[2] = (gdouble)swatch->color.blue  / MAX_16_BIT_COLOR; 
        color[3] = 0.0;
        gtk_color_selection_set_color(selector->colorsel, color);
        gtk_widget_show(GTK_WIDGET(dialog));
    }
    return FALSE;
}

/*
 * Set the custom color to the swatch and the canvas.
 */
static void
on_color_selector_ok_button_clicked (GtkButton *button,
                                    gpointer   user_data)
{
    gdouble color[4];  
    gpaint_color_selector *selector = (gpaint_color_selector*)user_data;		
    gpaint_color_swatch *swatch = selector->swatch;
    
    /* Get the selected colors: red, green, blue, opacity.
     * Range 0.0 to 1.0 */
    gtk_color_selection_get_color(selector->colorsel, color);
    
    /* Set the new swatch color. */
    swatch->color.red   = (guint16)(color[0] * MAX_16_BIT_COLOR);
    swatch->color.green = (guint16)(color[1] * MAX_16_BIT_COLOR);
    swatch->color.blue  = (guint16)(color[2] * MAX_16_BIT_COLOR);
    gdk_color_alloc(gdk_colormap_get_system(), &(swatch->color));
    gdk_gc_set_foreground(swatch->gc, &(swatch->color));
    gtk_widget_queue_draw(swatch->widget);

    /* Set picker color and canvas color. */
    change_color(swatch, selector->mode);
   
    /* clean up */
    gtk_widget_destroy(GTK_WIDGET(selector->dialog));
    g_free(selector);
}

/*
 * Dismiss the custom color selection dialog.
 */
static void
on_color_selector_cancel_button_clicked (GtkButton  *button,
                                         gpointer  user_data)
{
    gpaint_color_selector *selector = (gpaint_color_selector*)user_data;		
    gtk_widget_destroy(GTK_WIDGET(selector->dialog));
    g_free(selector);
}					

/*
 * Set the current foreground or backgound color and the 
 * associated color picker. 
 */
static void
change_color(gpaint_color_swatch *swatch, gpaint_color_mode mode)
{
    GtkColorButton *picker;
    GdkGCValues      gcvalues;
    gpaint_canvas    *canvas;
    GdkColor color = {0, swatch->color.red, 
                swatch->color.green, 
                swatch->color.blue} ;
    g_assert(swatch);
    canvas = canvas_lookup(swatch->widget);
    gdk_gc_get_values(swatch->gc, &gcvalues);
    
    if (mode==FOREGROUND)
    {
        change_foreground_color(canvas, &(gcvalues.foreground));
        picker = (GtkColorButton*)lookup_widget(
                swatch->widget, "foreground_color_picker");
    }
    else
    {
        change_background_color(canvas, &(gcvalues.foreground));
        picker = (GtkColorButton*)lookup_widget(
                swatch->widget, "background_color_picker");
    }
    g_assert(picker);
    gtk_color_button_set_color(
                picker, 
                &color);
}

/*
 * Set the canvas foreground color from the color picker selection.
 */
void
on_foreground_color_picker_color_set   (GtkColorButton *gnomecolorpicker,
                                        guint            arg1,
                                        guint            arg2,
                                        guint            arg3,
                                        guint            arg4,
                                        gpointer         user_data)
{
    GdkColor color = {0, arg1, arg2, arg3};
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(gnomecolorpicker));
    
    gdk_color_alloc(gdk_colormap_get_system(), &color);
    change_foreground_color(canvas, &color);
}

/*
 * Set the canvas background color from the color picker selection.
 */
void
on_background_color_picker_color_set   (GtkColorButton *gnomecolorpicker,
                                        guint            arg1,
                                        guint            arg2,
                                        guint            arg3,
                                        guint            arg4,
                                        gpointer         user_data)
{
    GdkColor color = {0, arg1, arg2, arg3};
    gpaint_canvas *canvas = canvas_lookup(GTK_WIDGET(gnomecolorpicker));

    gdk_color_alloc(gdk_colormap_get_system(), &color);
    change_background_color(canvas, &color);
}

/*
 * Change the current foreground color.
 */
static void
change_foreground_color(gpaint_canvas *canvas, GdkColor *color)
{
    gpaint_tool *tool = canvas->active_tool;
    GdkGC *gc         = canvas->gc;
    gboolean changed = FALSE;
    
    if (tool && tool->attribute)
    {
        changed = (*tool->attribute)(tool, GpaintForegroundColor, (gpointer*)color);
    }
    if (!changed)
    {
        gdk_gc_set_foreground(gc, color);
    }
}

/*
 * Change the current background color.
 */
static void
change_background_color(gpaint_canvas* canvas, GdkColor* color)
{
    gpaint_tool *tool = canvas->active_tool;
    GdkGC *gc         = canvas->gc;
    gboolean changed = FALSE;
    
    if (tool && tool->attribute)
    {
        changed = (*tool->attribute)(tool, GpaintBackgroundColor, (gpointer*)color);
    }
    if (!changed)
    {
        gdk_gc_set_background(gc, color);
    }
}
