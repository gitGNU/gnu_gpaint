/* $Id: canvas.c,v 1.6 2005/01/07 02:50:52 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003, 2007  Li-Cheng (Andy) Tai
 *
 * Authors: Li-Cheng (Andy) Tai <atai@gnu.org>
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

#include <gtk/gtk.h>
#include "debug.h"
#include "canvas.h"
#include "support.h"
#include "gtkscrollframe.h"
#include "image.h"
#include "image_processing.h"
#include "paste.h"
#include "selection.h"
#include "text.h"

#define GPAINT_CLIPBOARD_KEY "gpaint-clipboard"

/* Single clipboard to share selections between canvases. */
static gpaint_clipboard *clipboard = NULL;

/* macros to cast user_data in the callbacks */
#define CANVAS(user_data)      ((gpaint_canvas*)(user_data))
#define ACTIVE_TOOL(user_data) (((gpaint_canvas*)(user_data))->active_tool)
#define DRAWING(user_data)     (((gpaint_canvas*)(user_data))->drawing)

/* gobject canvas */
#define GPAINT_TYPE_CANVAS		(gpaint_canvas_get_type ())
#define GPAINT_CANVAS(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GPAINT_TYPE_CANVAS, gpaint_canvas))
#define GPAINT_CANVAS_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GPAINT_TYPE_CANVAS, gpaint_canvas_class))
#define GPAINT_IS_CANVAS(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPAINT_TYPE_CANVAS))
#define GPAINT_IS_CANVAS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GPAINT_TYPE_CANVAS))
#define GPAINT_CANVAS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GPAINT_TYPE_CANVAS, gpaint_canvas_class))

GType gpaint_canvas_get_type (void);

/* File to open when starting. */
static gchar** canvas_initial_filenames = 0;

static gpaint_canvas* canvas_new(GtkDrawingArea *drawing_area);
static void canvas_copy_selection_to_clipboard(gpaint_canvas *canvas);

static void canvas_clipboard_free (gpaint_clipboard *clipboard);
static gint canvas_clipboard_format_compare (GdkPixbufFormat *a, GdkPixbufFormat *b);
static void canvas_clipboard_send_buffer(GtkClipboard *gtk_clipboard, GtkSelectionData *selection_data, guint info, gpaint_canvas *canvas);

static void 
on_drawing_area_realize           (GtkWidget       *widget,
                                  gpointer         user_data);
static gboolean
on_drawing_area_expose_event            (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);
static gboolean
on_drawing_area_motion_notify_event     (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data);
static gboolean
on_drawing_area_button_press_event      (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);
static gboolean
on_drawing_area_button_release_event    (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);
static gboolean
on_drawing_area_focus_in_event          (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);
static gboolean
on_drawing_area_focus_out_event         (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);
static gboolean
on_drawing_area_key_release_event       (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

/*
 * Create the drawing area widget and place it in a scrolled view port. This
 * function is called from ui.c when the main application window is created.
 */
GtkWidget*
create_drawing_area_in_scroll_frame (
        gchar *widget_name,
        gchar *string1,
        gchar *string2,
        gint width,
        gint height)
{
    GtkWidget     *scrolledwindow;
    GtkWidget     *viewport;
    GtkWidget     *drawing_area;

    debug_fn();

    /* Create the scroll frame, view port, and drawing area widgets. */
    scrolledwindow = gtk_scroll_frame_new (NULL, NULL);
    gtk_scroll_frame_set_policy(GTK_SCROLL_FRAME (scrolledwindow),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    viewport = gtk_viewport_new (NULL, NULL);
    gtk_widget_set_name(viewport, "viewport");
    gtk_widget_ref(viewport);
    gtk_object_set_data_full(GTK_OBJECT (scrolledwindow), "viewport", viewport,
                            (GtkDestroyNotify) gtk_widget_unref);

    gtk_widget_show(viewport);
    gtk_container_add(GTK_CONTAINER (scrolledwindow), viewport);

    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_name(drawing_area, "drawing_area");
    gtk_widget_ref(drawing_area);
    gtk_object_set_data_full(GTK_OBJECT(viewport), "drawing_area", drawing_area,
                            (GtkDestroyNotify) gtk_widget_unref);
                 
    gtk_widget_show(drawing_area);
    gtk_container_add(GTK_CONTAINER(viewport), drawing_area);

    gtk_widget_set_events(GTK_WIDGET(drawing_area),
            GDK_EXPOSURE_MASK |
            GDK_POINTER_MOTION_MASK |
            GDK_POINTER_MOTION_HINT_MASK |
            GDK_BUTTON_MOTION_MASK |
            GDK_BUTTON1_MOTION_MASK |
            GDK_BUTTON2_MOTION_MASK |
            GDK_BUTTON3_MOTION_MASK |
            GDK_BUTTON_PRESS_MASK |
            GDK_BUTTON_RELEASE_MASK |
            GDK_KEY_PRESS_MASK |
            GDK_KEY_RELEASE_MASK |
            GDK_PROPERTY_CHANGE_MASK |
            GDK_VISIBILITY_NOTIFY_MASK |
            GDK_FOCUS_CHANGE_MASK |
            GDK_STRUCTURE_MASK |
            GDK_ENTER_NOTIFY_MASK |
            GDK_LEAVE_NOTIFY_MASK |
            GDK_PROPERTY_CHANGE_MASK);
    GTK_WIDGET_SET_FLAGS(drawing_area, GTK_CAN_FOCUS);
    GTK_WIDGET_SET_FLAGS(drawing_area, GTK_CAN_DEFAULT);
 
    /* connect the realize event to the function that will initialize
     * the canvas object. */
    gtk_signal_connect(GTK_OBJECT(drawing_area), "realize",
                       GTK_SIGNAL_FUNC(on_drawing_area_realize),
                       NULL);
    return scrolledwindow;
}

/*
 * Initialize the drawing area widget.
 */
static void
on_drawing_area_realize           (GtkWidget       *drawing_area,
                                  gpointer         user_data)
{
    /* Create the canvas object and initialize the window objects. */
    gpaint_canvas *canvas = canvas_new(GTK_DRAWING_AREA(drawing_area));
    g_assert(canvas);    
    debug_fn();

    /* attach the application specific data object to the drawing area */
    gtk_object_set_data_full(GTK_OBJECT(drawing_area),
                             "user_data",
                             canvas,
                             (GtkDestroyNotify)canvas_destroy);
    
    gtk_drawing_area_size(GTK_DRAWING_AREA(drawing_area),
                          canvas->drawing->width,
                          canvas->drawing->height);

    /* save a reference to the drawing area in the top_level window */
    gtk_widget_ref(drawing_area);
    gtk_object_set_data_full(GTK_OBJECT(canvas->top_level),
                             "drawing_area",
                             drawing_area,
                             (GtkDestroyNotify)gtk_widget_unref);

    /* Connect the event handlers and pass the canvas object to the 
     * callback functions. */
    gtk_signal_connect(GTK_OBJECT(drawing_area), "button_press_event",
                       GTK_SIGNAL_FUNC(on_drawing_area_button_press_event),
                       canvas);
    gtk_signal_connect(GTK_OBJECT(drawing_area), "button_release_event",
                       GTK_SIGNAL_FUNC(on_drawing_area_button_release_event),
                       canvas);
    gtk_signal_connect(GTK_OBJECT (drawing_area), "motion_notify_event",
                       GTK_SIGNAL_FUNC(on_drawing_area_motion_notify_event),
                       canvas);
    gtk_signal_connect(GTK_OBJECT(drawing_area), "expose_event",
                       GTK_SIGNAL_FUNC(on_drawing_area_expose_event),
                       canvas);
    gtk_signal_connect(GTK_OBJECT(drawing_area), "focus_in_event",
                       GTK_SIGNAL_FUNC(on_drawing_area_focus_in_event),
                       canvas);
    gtk_signal_connect(GTK_OBJECT(drawing_area), "focus_out_event",
                       GTK_SIGNAL_FUNC(on_drawing_area_focus_out_event),
                       canvas);
    gtk_signal_connect(GTK_OBJECT(drawing_area), "key_release_event",
                       GTK_SIGNAL_FUNC(on_drawing_area_key_release_event),
                       canvas);
}
 
/*
 * Copy the backing pixmap to the exposed area of the window.
 */
static gboolean
on_drawing_area_expose_event            (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = (gpaint_canvas*)user_data;
    gpaint_tool *tool = ACTIVE_TOOL(user_data);
    debug_fn();
    gdk_draw_pixmap(widget->window,
                    widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
                    canvas->drawing->backing_pixmap,
                    event->area.x, event->area.y,
                    event->area.x, event->area.y,
                    event->area.width, event->area.height);
    if (tool && tool->current_draw) tool->current_draw(tool);                
    return FALSE;
}

/*
 * Dispatch the motion event to the current tool object. The motion
 * hint mask is used to collapse motion events. This prevents the 
 * the application from falling behind if events are dispatched by
 * the X server faster than the application can process them.
 */
static gboolean
on_drawing_area_motion_notify_event     (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data)
{
    gpaint_tool *tool = ACTIVE_TOOL(user_data);
    int x, y;
    GdkModifierType state;

    if (tool && tool->motion)
    {
        if (event->is_hint)
        {
            gdk_window_get_pointer (event->window, &x, &y, &state);
        }
        else
        {
            x = event->x;
            y = event->y;
            state = event->state;
        }
    
        if (state & GDK_BUTTON1_MASK)
        {
            (*tool->motion)(tool, x, y);
        }
    }
    return TRUE;
}

/*
 * Dispatch the mouse button press to the active tool.
 */
static gboolean
on_drawing_area_button_press_event      (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
    gpaint_tool *tool = ACTIVE_TOOL(user_data);
    if (event->button==1 && tool && tool->button_press)
    {
        (*tool->button_press)(tool, event->x, event->y);
    }
    return FALSE;
}

/*
 * Dispatch the mouse button release to the active tool.
 */
static gboolean
on_drawing_area_button_release_event    (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
    gpaint_tool *tool = ACTIVE_TOOL(user_data);
    if (event->button==1 && tool && tool->button_release)
    {
        (*tool->button_release)(tool, event->x, event->y);
    }
    return FALSE;
}

/*
 * Dispatch the focus in event to the active tool.
 */
static gboolean
on_drawing_area_focus_in_event          (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = CANVAS(user_data);
    debug_fn1("canvas=%p", canvas);
    canvas_focus_gained(canvas);
    return FALSE;
}

/*
 * Dispatch the focus out event to the active tool.
 */
static gboolean
on_drawing_area_focus_out_event         (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
    gpaint_canvas *canvas = CANVAS(user_data);
    debug_fn1("canvas=%p", canvas);
    canvas_focus_lost(canvas);
    return FALSE;
}

/*
 * Dispatch the key release to the active tool.
 */
static gboolean
on_drawing_area_key_release_event       (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
    gpaint_tool *tool = ACTIVE_TOOL(user_data);
    if (tool && tool->key_release)
    {
        (*tool->key_release)(tool, event);
    }
    return FALSE;
}

/*
 * Save the application command line arguments. The first
 * argument is the filename to be initially opened. A
 * blank drawing is created if no file name is given.
 */
void
canvas_init_arg(int argc, char* argv[])
{
    int i;
    if (argc > 1)
    {
        canvas_initial_filenames = calloc(sizeof(char*), argc - 1);
        for (i = 1; i < argc; i++)
            canvas_initial_filenames[i - 1] = argv[i];
    }
    return;
}


/*
 * Lookup the canvas object from a widget.
 */
gpaint_canvas*
canvas_lookup(GtkWidget *widget)
{
    GtkWidget *cw;
    gpaint_canvas *canvas;
    cw = lookup_widget(widget, "drawing_area");
    g_assert(cw);
    canvas = (gpaint_canvas*)gtk_object_get_data(GTK_OBJECT(cw), "user_data");
    g_assert(canvas);
    return canvas;
}

/*
 * Create a new canvas object. The canvas object is created when the drawing
 * area window is realized.
 */
gpaint_canvas *
canvas_new(GtkDrawingArea *drawing_area)
{
    gpaint_canvas *canvas = 0; 
    gpaint_drawing *drawing = 0;
    GdkDrawable *d = GTK_WIDGET(drawing_area)->window;
    GdkGC *gc;
    GdkColor black; 
    GdkColor white; 
 
    /* Create the gc from the drawing area window. */
    gc = gdk_gc_new(d);
    g_assert(gc);
    gdk_gc_ref(gc);
    
    gdk_color_black(gdk_colormap_get_system(), &black);
    gdk_color_white(gdk_colormap_get_system(), &white);
    gdk_gc_set_foreground(gc, &black);
    gdk_gc_set_background(gc, &white);
    gdk_gc_set_line_attributes(gc, 1, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
   
    /* Create the canvas object */
//    canvas = (gpaint_canvas*)g_new0(gpaint_canvas, 1);
    canvas = g_object_new (GPAINT_TYPE_CANVAS, NULL);
    g_assert(canvas);
    canvas->top_level = gtk_widget_get_toplevel(GTK_WIDGET(drawing_area));
    canvas->drawing_area = drawing_area;    
    canvas->gc = gc;    
    canvas->busy_cursor = gdk_cursor_new(GDK_WATCH); 
    canvas->arrow_cursor = gdk_cursor_new(GDK_LEFT_PTR); 
    canvas->cursor = canvas->arrow_cursor;
    canvas->active_tool = 0;
    canvas->saved_tool = 0;
    canvas->paste_tool = paste_create("paste");
    canvas->selection = selection_new(drawing_area);
    
    /* Create the drawing object */
    if (canvas_initial_filenames)
    { /* only open the first one for now; open additonal ones TO DO */
    
        drawing = drawing_new_from_file(canvas->drawing_area, canvas->gc, canvas_initial_filenames[0]);
        if (!drawing)
        {
            g_message("Failed to open drawing file %s", canvas_initial_filenames[0]);
        }
        free(canvas_initial_filenames);
        canvas_initial_filenames = 0;  /* use only in the first window */
    }
    if (!drawing)
    {
        drawing = drawing_new_blank(canvas->drawing_area, canvas->gc, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    }
    g_assert(drawing);
    canvas->drawing = drawing;
    selection_size(canvas->selection, drawing->width, drawing->height);

    return canvas;
}  

GType
gpaint_canvas_get_type (void)
{
    static GType object_type = 0;

    if (!object_type)
    {
	static const GTypeInfo object_info =
	{
	    sizeof (gpaint_canvas_class),
	    (GBaseInitFunc) NULL,
	    (GBaseFinalizeFunc) NULL,
	    (GClassInitFunc) NULL,
	    NULL,	/* class_finalize */
	    NULL,	/* class_data */
	    sizeof (gpaint_canvas),
	    0,
	    (GInstanceInitFunc) NULL,
	};

	object_type = g_type_register_static (G_TYPE_OBJECT,
						"GpaintCanvas",
						&object_info, 0);
    }

    return object_type;
}

void
canvas_destroy(gpaint_canvas *canvas)
{
    debug_fn();
    g_assert(canvas);
    
    if (canvas->paste_tool)
    {
        gpaint_tool *tool = canvas->paste_tool; 
        g_assert(tool->destroy);
        (*tool->destroy)(tool);
    }
    
    drawing_destroy(canvas->drawing);    
    selection_delete(canvas->selection);
    gdk_gc_unref(canvas->gc);
    gdk_cursor_destroy(canvas->arrow_cursor);
    gdk_cursor_destroy(canvas->busy_cursor);
//    memset(canvas, 0xBEBE, sizeof(canvas)); /* debugging aid */
//    g_free(canvas);
    g_object_unref(canvas);
    debug("canvas_destroy() returning");
}

/*
 * Display the new drawing and destroy the current drawing.
 */
void
canvas_set_drawing(gpaint_canvas *canvas, gpaint_drawing *new_drawing)
{
    gpaint_drawing *old_drawing;
    g_assert(canvas);
    g_assert(new_drawing);
    
    old_drawing = canvas->drawing;
    
    canvas->drawing = new_drawing;
    selection_remove_points(canvas->selection);      
    if (canvas->active_tool)
    {
        canvas->active_tool->drawing = canvas->drawing;
    }
    gtk_drawing_area_size(
                GTK_DRAWING_AREA(canvas->drawing_area),
                canvas->drawing->width,
                canvas->drawing->height);
    selection_size(canvas->selection, canvas->drawing->width, canvas->drawing->height);
    drawing_destroy(old_drawing);
    canvas_redraw(canvas);
}

/*
 * Resize the canvas to the current drawing size. Should be called
 * when the drawing size has been changed.
 */
void
canvas_resize(gpaint_canvas *canvas)
{
    selection_remove_points(canvas->selection);      
    selection_size(canvas->selection, canvas->drawing->width, canvas->drawing->height);
    gtk_drawing_area_size(
                GTK_DRAWING_AREA(canvas->drawing_area),
                canvas->drawing->width,
                canvas->drawing->height);
    canvas_redraw(canvas);
}


/*
 * Set the active drawing tool object.
 */
void
canvas_set_tool(gpaint_canvas* canvas, gpaint_tool *new_tool)
{
    gpaint_tool *old_tool = canvas->active_tool;
    
    if (old_tool && old_tool->deselect) 
    {
        (*old_tool->deselect)(old_tool);
    }
    
    canvas->active_tool = new_tool;
    if (!canvas->active_tool)      /* null means remove active tool */
    {
        canvas->cursor = canvas->arrow_cursor;
    }
    else
    {
        new_tool->drawing = canvas->drawing;
        new_tool->canvas = canvas;
        if (new_tool->select)
        {
            (*new_tool->select)(new_tool);
        }
        canvas->cursor = new_tool->cursor;
    }
    gdk_window_set_cursor(canvas->drawing->window, canvas->cursor);
}

void
canvas_begin_busy_cursor(gpaint_canvas *canvas)
{
    canvas_commit_change(canvas);
    gdk_window_set_cursor(canvas->top_level->window, canvas->busy_cursor);
    gdk_window_set_cursor(canvas->drawing->window, canvas->busy_cursor);
    gdk_flush(); 
}

void 
canvas_end_busy_cursor(gpaint_canvas *canvas)
{
    gdk_window_set_cursor(canvas->top_level->window, canvas->arrow_cursor); 
    if (canvas->cursor) 
    {
        gdk_window_set_cursor(canvas->drawing->window, canvas->cursor);
    }
    else
    {
        gdk_window_set_cursor(canvas->drawing->window, canvas->arrow_cursor);
    }
    gdk_flush();
}

void
canvas_focus_gained(gpaint_canvas *canvas)
{
    gpaint_tool *tool = canvas->active_tool;
    debug_fn1("canvas=%p", canvas);
   
    selection_enable_flash(canvas->selection);
    canvas->has_focus = TRUE;
    if (tool && tool->change)
    {
        (*tool->change)(tool, GpaintFocusIn, NULL);
    }
}

void
canvas_focus_lost(gpaint_canvas *canvas)
{
    gpaint_tool *tool = canvas->active_tool;
    debug_fn1("canvas=%p", canvas);
    
    selection_disable_flash(canvas->selection);
    canvas->has_focus = FALSE;
    if (tool && tool->change)
    {
        (*tool->change)(tool, GpaintFocusOut, NULL);
    }
}

void
canvas_redraw(gpaint_canvas *canvas)
{
    GtkWidget *widget = GTK_WIDGET(canvas->drawing_area);
    int width;
    int height;

    gdk_window_get_size(widget->window, &width, &height);
    gtk_widget_queue_draw_area(widget, 0, 0, width, height);
    gtk_widget_queue_draw(widget);
}

void
canvas_begin_paste_mode(gpaint_canvas *canvas)
{
    gpaint_clipboard *clipboard = canvas_clipboard(canvas);
    canvas_commit_change(canvas);
//    if (clipboard && point_array_size(clipboard->points)) 
    {
        if (canvas->active_tool != canvas->paste_tool)
        {
            canvas->saved_tool = canvas->active_tool;
            canvas_set_tool(canvas, canvas->paste_tool);   
        }
    }
}

void
canvas_end_paste_mode(gpaint_canvas *canvas)
{
    if (canvas->active_tool == canvas->paste_tool)
    {
        canvas_set_tool(canvas, canvas->saved_tool);
    }
}

void
canvas_cut(gpaint_canvas *canvas)
{
    canvas_commit_change(canvas);
    if (canvas_has_selection(canvas))
    {
        selection_disable_flash(canvas->selection);
        canvas_copy_selection_to_clipboard(canvas);
        drawing_clear_selection(canvas->drawing, selection_points(canvas->selection));
        selection_enable_flash(canvas->selection);
    }
}

void
canvas_copy(gpaint_canvas *canvas)
{
    canvas_commit_change(canvas);
    if (canvas_has_selection(canvas))
    {
        selection_disable_flash(canvas->selection);
        canvas_copy_selection_to_clipboard(canvas);
        selection_enable_flash(canvas->selection);
    }
}

void
canvas_clear(gpaint_canvas *canvas)
{

    if (canvas_has_selection(canvas))
    {
        selection_disable_flash(canvas->selection);
        drawing_clear_selection(canvas->drawing, selection_points(canvas->selection));
        selection_enable_flash(canvas->selection);
    }
    else
    {
        drawing_clear(canvas->drawing);
        canvas_redraw(canvas);
    }
}

void
canvas_select_all(gpaint_canvas *canvas)
{
    GdkRectangle rect;
    canvas_commit_change(canvas);
    rect.x = 0;
    rect.y = 0;
    rect.width = canvas->drawing->width;
    rect.height = canvas->drawing->height;
    selection_set_rectangle(canvas->selection, rect);
}

gboolean 
canvas_has_selection(gpaint_canvas *canvas)
{
    return (selection_num_points(canvas->selection) > 2);
}

gpaint_clipboard *
canvas_clipboard(gpaint_canvas *canvas)
{
    /* create the single clipboard */
    if (!clipboard)
    {
	GSList *list;

        clipboard = g_new0(gpaint_clipboard, 1);
        clipboard->image = 0;
        clipboard->points = point_array_new();

/* create gobject for clipboard notifications */
	g_object_set_data_full (G_OBJECT (canvas), GPAINT_CLIPBOARD_KEY,
				clipboard, (GDestroyNotify) canvas_clipboard_free);

/* create list of pixbuf formats for clipboard format negotiation */

        clipboard->pixbuf_formats =
		g_slist_sort (gdk_pixbuf_get_formats(),
				(GCompareFunc) canvas_clipboard_format_compare);
	for (list = clipboard->pixbuf_formats; list; list = g_slist_next (list))
	{
	    GdkPixbufFormat *format = list->data;

	    if (gdk_pixbuf_format_is_writable (format))
	    {
		gchar **mime_types;
		gchar **type;

		mime_types = gdk_pixbuf_format_get_mime_types (format);

		for (type = mime_types; *type; type++)
		    clipboard->n_target_entries++;

		g_strfreev (mime_types);
	    }
	}

	if (clipboard->n_target_entries > 0)
	{
	    gint i = 0;

	    clipboard->target_entries = g_new0(GtkTargetEntry,
						clipboard->n_target_entries);
	    clipboard->savers 	      = g_new0(gchar*,
						clipboard->n_target_entries + 1);

	    for (list = clipboard->pixbuf_formats; list; list = g_slist_next (list))
	    {
	        GdkPixbufFormat *format = list->data;

	        if (gdk_pixbuf_format_is_writable (format))
	        {
		    gchar *format_name;
		    gchar **mime_types;
		    gchar **type;

		    format_name = gdk_pixbuf_format_get_name (format);
		    mime_types = gdk_pixbuf_format_get_mime_types (format);

		    for (type = mime_types; *type; type++)
		    {
			gchar *mime_type = *type;

			clipboard->target_entries[i].target = g_strdup (mime_type);
			clipboard->target_entries[i].flags  = 0;
			clipboard->target_entries[i].info   = i;

			clipboard->savers[i]                = g_strdup (format_name);

			i++;
		    }

		    g_strfreev (mime_types);
		    g_free (format_name);
		}
	    }
	}
    }
    return clipboard;
}

static void
canvas_clipboard_free (gpaint_clipboard *clipboard)
{
    g_slist_free (clipboard->pixbuf_formats);
    g_free (clipboard->target_entries);
    g_strfreev (clipboard->savers);
    g_free (clipboard);
}

static void
canvas_copy_selection_to_clipboard(gpaint_canvas *canvas)
{
    gpaint_clipboard *clipboard = canvas_clipboard(canvas);
    GtkClipboard *gtk_clipboard;
    
    if (clipboard->image)
    {
        image_free(clipboard->image);
    }
    clipboard->image = image_from_selection(
                            canvas->drawing->backing_pixmap,
                            selection_points(canvas->selection));
    point_array_copy(clipboard->points, selection_points(canvas->selection));

    gtk_clipboard = gtk_clipboard_get_for_display (gdk_display_get_default(),
						   GDK_SELECTION_CLIPBOARD);
    if (!gtk_clipboard)
        return;

    if (clipboard->image)
    {
        gtk_clipboard_set_with_owner (gtk_clipboard,
    					clipboard->target_entries,
					clipboard->n_target_entries,
					(GtkClipboardGetFunc) canvas_clipboard_send_buffer,
					(GtkClipboardClearFunc) NULL,
					G_OBJECT (canvas));
    }
    else if (gtk_clipboard_get_owner (gtk_clipboard) == G_OBJECT(canvas))
    {
        gtk_clipboard_clear (gtk_clipboard);
    }
}   

static GdkPixbuf* 
clipboard_pixbuf(gpaint_clipboard *clipboard) {
    return image_pixbuf(clipboard->image);
}


static void
canvas_clipboard_send_buffer(GtkClipboard	*gtk_clipboard,
			     GtkSelectionData	*selection_data,
			     guint		 info,
			     gpaint_canvas	*canvas)
{
    gpaint_clipboard *clipboard = canvas_clipboard(canvas);
    GdkPixbuf *pixbuf = clipboard_pixbuf(clipboard);

    if (pixbuf)
    {
        GdkAtom atom = gdk_atom_intern (clipboard->target_entries[info].target,
					FALSE);

	g_print ("sending pixbuf data as '%s' (%s)\n",
			clipboard->target_entries[info].target,
			clipboard->savers[info]);
	gchar *buffer;
	gsize buffer_size;
	GError *error = NULL;

	g_return_if_fail (selection_data != NULL);
	g_return_if_fail (atom != GDK_NONE);
	g_return_if_fail (GDK_IS_PIXBUF (pixbuf));
	g_return_if_fail (clipboard->savers[info] != NULL);

	if (gdk_pixbuf_save_to_buffer (pixbuf,
					&buffer, &buffer_size, clipboard->savers[info],
					&error, NULL))
	{
	    gtk_selection_data_set (selection_data, atom,
					8, (guchar*) buffer, buffer_size);
        }
    }
}

static gint
canvas_clipboard_format_compare (GdkPixbufFormat *a,
				 GdkPixbufFormat *b)
{
    gchar *a_name = gdk_pixbuf_format_get_name (a);
    gchar *b_name = gdk_pixbuf_format_get_name (b);
    gint retval = 0;

#ifdef GDK_WINDOWING_WIN32
    /*  move BMP to the front of the list  */
    if (g_ascii_strncasecmp (a_name, "bmp", strlen("bmp")) == 0)
        retval = -1;
    else if (g_ascii_strncasecmp (b_name, "bmp", strlen("bmp")) == 0)
        retval = 1;
    else
#endif

    /*  move PNG to the front of the list  */
    if (g_ascii_strncasecmp (a_name, "png", strlen("png")) == 0)
        retval = -1;
    else if (g_ascii_strncasecmp (b_name, "png", strlen("png")) == 0)
        retval = 1;

    /*  move JPEG to the end of the list  */
    else if (g_ascii_strncasecmp (a_name, "jpeg", strlen("jpeg")) == 0)
        retval = 1;
    else if (g_ascii_strncasecmp (b_name, "jpeg", strlen("jpeg")) == 0)
        retval = -1;

    /*  move GIF to the end of the list  */
    else if (g_ascii_strncasecmp (a_name, "gif", strlen("gif")) == 0)
        retval = 1;
    else if (g_ascii_strncasecmp (b_name, "gif", strlen("gif")) == 0)
        retval = -1;

    g_free (a_name);
    g_free (b_name);

    return retval;
}   

void canvas_commit_change(gpaint_canvas *canvas)
{
    gpaint_tool * tool = canvas->active_tool;
    if (tool && tool->commit_change)
        tool->commit_change(tool);
}
