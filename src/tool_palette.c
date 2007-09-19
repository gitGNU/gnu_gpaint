/* $Id: tool_palette.c,v 1.4 2004/11/22 02:59:53 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003, 2007  Li-Cheng (Andy) Tai
 *
 * Authors: Li-Cheng (Andy) Tai
 *          Michael A. Meffie III <meffiem@neo.rr.com>
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

#include <string.h>
#include <stdlib.h>
#include "tool_palette.h"
#include "debug.h"
#include "util.h"
#include "pixmaps.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib.h>

/* tools */
#include "pen.h"
#include "freehand.h"
#include "brush.h"
#include "shape.h"
#include "lasso.h"
#include "polyselect.h"
#include "rectselect.h"
#include "fill.h"
#include "text.h"

/*
 * Tool palette state data.
 */
typedef struct _gpaint_tool_palette
{
    GtkToggleButton  *selected;
    GHashTable       *tool_hash;
} gpaint_tool_palette;

/*
 * Tool palette toggle button state.
 */
typedef struct _gpaint_tool_button
{
    GtkToggleButton  *widget;
    void             (*handle_click)(struct _gpaint_tool_button*);
    gpaint_tool      *tool;
} gpaint_tool_button;

/*
 * Shape fill/unfill toggle button state.
 */
typedef struct _gpaint_fill_button
{
    GtkToggleButton     *button;
    gpaint_tool_palette *tool_palette;
    gboolean             fill;
} gpaint_fill_button;


/*
 * Button name, create function record.
 */
typedef struct _gpaint_tool_create
{
    const gchar *button_name;
    const gchar *tool_name;
    const gchar **icon;
    ToolCreate   create;
} gpaint_tool_create;


/*
 * Function table for the tool palette buttons.
 */
static const gpaint_tool_create tool_table[] =
{
    {"erase_button",            "eraser",     eraseOp_xpm,    eraser_create},
    {"lasso_button",            "lasso",      lassoOp_xpm,    lasso_select_create},
    {"fill_button",             "fill",       fillOp_xpm,     fill_create},
    {"line_button",             "line",       lineOp_xpm,     line_shape_create},
    {"multiline_button",        "mline",      clineOp_xpm,    multiline_shape_create},
    {"rectangle_button",        "rectangle",  boxOp_xpm,      rectangle_shape_create},
    {"closed_freehand_button",  "freehand",   freehandOp_xpm, closed_freehand_create},
    {"pen_button",              "pen",        pencilOp_xpm,   pen_create},
    {"polselect_button",        "polyselect", selpolyOp_xpm,  polygon_select_create},
    {"rectselect_button",       "rectselect", selrectOp_xpm,  rectangle_select_create},
    {"text_button",             "text",       textOp_xpm,     text_create},
    {"arc_button",              "arc",        arcOp_xpm,      arc_shape_create},
    {"curve_button",            "",           curveOp_xpm,    NULL},
    {"oval_button",             "oval",       ovalOp_xpm,     oval_shape_create},
    {"brush_button",            "brush",      brushOp_xpm,    paint_brush_create},
    {0, 0, 0, 0} /* end sentinel */
};

/* Local functions */
static void tool_palette_destroy(gpaint_tool_palette *tp);
static void tool_button_destroy(gpaint_tool_button *tb);
static const gpaint_tool_create* lookup_tool_create(const gchar *button_name);
static gpaint_tool_palette* lookup_tool_palette_data(GtkWidget *widget);
static void on_tool_select(gpaint_tool_button* tb);
static void on_tool_reselect(gpaint_tool_button* tb);
static void on_tool_deselect(gpaint_tool_button* tb);
static void set_button_pixmap(GtkToggleButton *button, const char **pixmap);
static guint tool_hash_function(gconstpointer key);
static gint  tool_hash_compare(gconstpointer a, gconstpointer b);
static void  tool_hash_change_fill(gpointer key, gpointer value, gpointer user_data);

static void filled_button_destroy(gpaint_fill_button *fb);

/*
 * Look up the tool palette widget in the widget tree.
 */
GtkWidget*
lookup_tool_palette(GtkWidget *widget)
{
    GtkWidget *tp = lookup_widget(widget, "tool_palette_table");
    g_assert(tp);
    return tp;
}

gpaint_tool*
tool_palette_get_tool(GtkWidget *widget, const gchar* name)
{
    gpaint_tool_palette *tp;
    gpaint_tool *tool;

    tp = lookup_tool_palette_data(widget);
    tool = (gpaint_tool*)g_hash_table_lookup(tp->tool_hash, name);
    return tool;
}

/*
 * Lookup the tool_palette data object. Create the initial object
 * on the first time this function is called.
 */
static gpaint_tool_palette*
lookup_tool_palette_data(GtkWidget *widget)
{
    GtkObject *object = GTK_OBJECT(lookup_tool_palette(widget));
    gpaint_tool_palette *tp =
        (gpaint_tool_palette*)gtk_object_get_data(object, "tool_palette");

    debug_fn();

    /* create the tool palette data object the first time requested.
     * This function may be called before the tool palette is realized. */
    if (!tp)
    {
        tp = g_new(gpaint_tool_palette, 1);
        g_assert(tp);
        debug1("new tool_palette=%p", tp);
        tp->selected = NULL;
        tp->tool_hash = g_hash_table_new(tool_hash_function, tool_hash_compare);
        gtk_object_set_data_full(
                object,
                "tool_palette",
                tp,
                (GtkDestroyNotify)tool_palette_destroy);
    }
    return tp;
}

/*
 * Initialize the tool palette widget.
 */
void
on_tool_palette_realize                 (GtkWidget       *widget,
                                        gpointer         user_data)
{
     debug_fn();
}

static void
tool_palette_destroy(gpaint_tool_palette *tp)
{
    debug_fn1("tool_palette=%p", tp);
    g_hash_table_destroy(tp->tool_hash);
    memset(tp, 0xBEBE, sizeof(gpaint_tool_palette));
    g_free(tp);
    debug("tool_palette_destroy() returning");
}

/*
 * Initialize a tool button.
 */
void
on_tool_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data)
{
    const char *button_name = gtk_widget_get_name(widget);
    const gpaint_tool_create *tc;
    gpaint_tool_button *tb;
    gpaint_tool_palette *tp;
    
    debug_fn1("button_name=%s", button_name);

    tc = lookup_tool_create(button_name);
    tb = g_new(gpaint_tool_button, 1);
    
    tb->widget = GTK_TOGGLE_BUTTON(widget);
    tb->handle_click = on_tool_select;
    g_assert(tc);
    if (!tc->create)
    {
        tb->tool = NULL;
    }
    else
    {
        g_assert(tc->tool_name);
        debug2("creating %s for %s", tc->tool_name, button_name);
        tb->tool = (*tc->create)(tc->tool_name);
        g_assert(tb->tool);
        g_assert(tb->tool->name);
        g_assert(tb->tool->destroy);
    
        /* Add the tool object to the tool palette hash table 
         * The hash table is destroyed when the tool_palette is
         * destroyed. */
        tp = lookup_tool_palette_data(widget);  
        g_hash_table_insert(tp->tool_hash,
                (gpointer)tb->tool->name, (gpointer)tb->tool);
    }
    
    gtk_object_set_data_full(
            GTK_OBJECT(widget), 
            "tool_button", tb, (GtkDestroyNotify)tool_button_destroy);

    /* place the tool icon in the button */
    if (tc->icon)
    {
        set_button_pixmap(tb->widget, tc->icon);
    }
    else
    {
	    g_warning("missing icon data");
    }
    
}

static void
tool_button_destroy(gpaint_tool_button *tb)
{
    debug_fn1("tool_button=%p", tb);
   
    /* Remove the tool object if there is one for this
     * button. Some buttons are just placeholders until 
     * the tool object has been implemented for it. */
    g_assert(tb);
    if (tb->tool) 
    {
        gpaint_tool *tool = tb->tool;
        g_assert(tool->name);
        g_assert(tool->destroy);
        debug1("destroying %s", tool->name);

        /* destroy the drawing tool object associated with this button */
        (*tool->destroy)(tool);
    }
    memset(tb, 0xBEBE, sizeof(gpaint_tool_button));
    g_free(tb);
}

/*
 * Handle tool button click events.
 */
void
on_tool_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
    gpaint_tool_button *tb = 
        (gpaint_tool_button*)gtk_object_get_data(GTK_OBJECT(button), "tool_button");
    g_assert(tb);
    (*tb->handle_click)(tb);
}
    
/*
 * Initialize the shape fill toggle button.
 */
void
on_filled_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data)
{
    gpaint_fill_button *fb;
    
    /* attach state data object */
    fb = g_new(gpaint_fill_button, 1);
    fb->button = GTK_TOGGLE_BUTTON(widget);
    fb->fill = FALSE;      /* start as unfill */
    gtk_object_set_data_full(GTK_OBJECT(widget), "fill_button", fb, 
                                (GtkDestroyNotify)filled_button_destroy);
    
    /* set the unfill shape icon in the button */
    set_button_pixmap(fb->button, (const char **)unfilled_xpm);
}

static void
filled_button_destroy(gpaint_fill_button *fb)
{
    debug_fn();
}

/*
 * Invert the shape fill/unfill state for each tool object.
 */
void
on_filled_button_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    gpaint_tool_palette *tp = lookup_tool_palette_data(GTK_WIDGET(togglebutton));
    gpaint_fill_button *fb = gtk_object_get_data(GTK_OBJECT(togglebutton), "fill_button");
    const char **icon;
    
    if (fb->fill)
    {
        fb->fill = FALSE;
        icon = unfilled_xpm;
    }
    else
    {
        fb->fill = TRUE;
        icon = filled_xpm;
    }
    g_hash_table_foreach(tp->tool_hash, tool_hash_change_fill, (gpointer)(&(fb->fill)));
    set_button_pixmap(togglebutton, icon);   
}

static void
tool_hash_change_fill(gpointer key, gpointer value, gpointer user_data)
{
    gpaint_tool *tool = (gpaint_tool*)value;
    if (tool && tool->attribute)
    {
        (*tool->attribute)(tool, GpaintFillShape, user_data);
    }
}

/*
 * Look up the tool create function for a tool button.
 */
static const gpaint_tool_create*
lookup_tool_create(const gchar *button_name)
{
    const gpaint_tool_create *p = tool_table;
    g_assert(button_name);
    while (p->button_name)
    {
        if (strcmp(p->button_name,button_name)==0)
        {
            return p;
        }
        p++;
    }
    debug1("Unexpected tool button name: %s", button_name);
    return NULL;
}

/*
 * Handle the selection of a tool.
 */
static void
on_tool_select(gpaint_tool_button* tb)
{
    /* deselect the previous tool, if one */
    gpaint_tool_palette *palette = lookup_tool_palette_data(GTK_WIDGET(tb->widget));
    GtkToggleButton *selected = palette->selected;
    if (selected)
    {
        gpaint_tool_button *p = 
            (gpaint_tool_button*)gtk_object_get_data(GTK_OBJECT(selected), "tool_button");
        g_assert(p);
        p->handle_click = on_tool_deselect;
    
        /* Toggle off the previous button. This will generate a click event
         * which is handled in on_tool_deselect(). */
        gtk_toggle_button_set_active(palette->selected, FALSE);
    }
    
    /* select the new tool */
    palette->selected = tb->widget;
    tb->handle_click = on_tool_reselect;
    canvas_set_tool(canvas_lookup(GTK_WIDGET(tb->widget)), tb->tool);
}

/*
 * The user clicked on a tool button that was already selected.
 * The mouse click will reset the toggle button, so toggle it
 * back to the selected state.
 */
static void
on_tool_reselect(gpaint_tool_button* tb)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb->widget), TRUE);
}

/*
 * Transitional state while the button is being deselected. 
 */
static void
on_tool_deselect(gpaint_tool_button* tb)
{
    tb->handle_click = on_tool_select;
}

/*
 * Helper function for set_button_pixmap().
 */
static void 
gtk_container_remove_callback(GtkWidget *widget, gpointer data)
{
    gtk_container_remove(GTK_CONTAINER(data), widget);
}

static
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

/*
 * Set the pixmap icon in a button.
 */
static void 
set_button_pixmap(GtkToggleButton *button, const char **pixmap)
{
    GdkPixmap *gdkpixmap = 0;
    GdkBitmap *mask = 0;
    GtkWidget *gtkpixmap = 0;
    static GtkWidget *widget = 0;
   
    if (!widget)
    {
        widget = widget_get_toplevel_parent(GTK_WIDGET(button));
    }
    if (widget)
    {
        gdkpixmap = gdk_pixmap_create_from_xpm_d(widget->window, &mask, NULL, (gchar**)pixmap);
        g_assert(gdkpixmap);
	
        gtkpixmap = gtk_pixmap_new(gdkpixmap, mask);
        g_assert(gtkpixmap);
     
	gtk_container_foreach(GTK_CONTAINER(button), (GtkCallback)gtk_container_remove_callback, button);
        gtk_container_add(GTK_CONTAINER(button), (gtkpixmap));

        gtk_widget_show(gtkpixmap); 
	
	gdk_pixmap_unref(gdkpixmap);
        gdk_pixmap_unref(mask); 
    }
}

static guint
tool_hash_function(gconstpointer key)
{
    const char *s_key = (const char*)key;
    guint hash = 0;
    if (s_key)
    {
        int i;
        int length = strlen(s_key);
        for (i=0; i<length; i++)
        {
            hash = (hash<<4) + (hash ^ (guint)s_key[i]);
        }
    }
    return hash;
}

static gint
tool_hash_compare(gconstpointer a, gconstpointer b)
{
    return (!strcmp((const char*)a, (const char*)b));
}


void tool_palette_set_active_button(GtkWidget *widget, const char *button_name)
{
    
    GtkWidget *button = lookup_widget(widget, button_name);
    gpaint_tool_button *tb = 
        (gpaint_tool_button*)gtk_object_get_data(GTK_OBJECT(button), "tool_button");
    
    on_tool_select(tb);


}
