/* $Id: callbacks.h,v 1.12 2005/01/03 19:33:22 meffie Exp $
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

#include <gtk/gtk.h>
#include <gtk/gtk.h>
#include "pixmaps.h"

void
on_mainwindow_realize                  (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_mainwindow_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_new_file_menu_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_open_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_as_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_print_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_print_setup_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_exit_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_cut_menu_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_copy_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_paste_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_clear_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_properties_menu_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_flip_x_axis_menu_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_flip_y_axis_menu_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rotate_p90_menu_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rotate_p45_menu_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rotate_n45_menu_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rotate_n90_menu_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rotate_p180_menu_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rotate_menu_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_crop_to_region_menu_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_reset_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_revert_menu_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_sharpen_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_smooth_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_directional_smooth_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_despeckle_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_edge_detect_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_emboss_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_oil_paint_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_add_noise_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_spread_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_pixelize_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_blend_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_solarize_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_normalize_contrast_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quantize_color_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_convert_to_greyscale_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_sharpen_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_sharpen_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_fat_bits_menu_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_visible_grid_menu_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_snap_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_snap_spacing_menu_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_change_background_menu_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_change_size1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_undo_levels_menu_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_auto_crop_menu_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_change_zoom_menu_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_font_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_bold_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_italic_button_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_underline_button_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_small_label_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_line_width_combo_add                (GtkContainer    *container,
                                        GtkWidget       *widget,
                                        gpointer         user_data);

void
on_line_width_combo_check_resize       (GtkContainer    *container,
                                        gpointer         user_data);

void
on_new_button_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_open_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_save_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_save_as_button_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_print_button_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_erase_button_clicked                (GtkToggleButton       *button,
                                        gpointer         user_data);

void
on_lasso_button_clicked                (GtkToggleButton       *button,
                                        gpointer         user_data);

void
on_line_button_clicked                 (GtkToggleButton       *button,
                                        gpointer         user_data);

void
on_rectangle_button_clicked            (GtkToggleButton       *button,
                                        gpointer         user_data);

void
on_freehand_button_clicked             (GtkToggleButton       *button,
                                        gpointer         user_data);

void
on_pen_button_clicked                  (GtkToggleButton       *button,
                                        gpointer         user_data);

void
on_polselect_button_clicked            (GtkToggleButton       *button,
                                        gpointer         user_data);

void
on_text_button_clicked                 (GtkToggleButton       *button,
                                        gpointer         user_data);

void
on_arc_button_clicked                  (GtkToggleButton       *button,
                                        gpointer         user_data);

void
on_oval_button_clicked                 (GtkToggleButton       *button,
                                        gpointer         user_data);

void
on_brush_button_clicked                (GtkToggleButton       *button,
                                        gpointer         user_data);

void
on_new_canvas_ok_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_new_canvas_cancel_button_clicked    (GtkButton       *button,
                                        gpointer         user_data);

void
on_pen_button_realize                  (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_pen_button_realize                  (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_pen_button_realize                  (GtkWidget       *widget,
                                        gpointer         user_data);


void
on_scroll_frame_realize                (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_scroll_frame_map                    (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_fontpicker_font_set                 (GtkFontButton *gnomefontpicker,
                                        gchar *arg1,
                                        gpointer         user_data);

void
on_foreground_color_picker_color_set   (GtkColorButton *gnomecolorpicker,
                                        guint            arg1,
                                        guint            arg2,
                                        guint            arg3,
                                        guint            arg4,
                                        gpointer         user_data);

void
on_background_color_picker_color_set   (GtkColorButton *gnomecolorpicker,
                                        guint            arg1,
                                        guint            arg2,
                                        guint            arg3,
                                        guint            arg4,
                                        gpointer         user_data);

void
on_erase_button_realize                (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_lasso_button_realize                (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_fill_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_fill_button_clicked                 (GtkToggleButton       *button,
                                        gpointer         user_data);

void
on_line_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_rectangle_button_realize            (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_freehand_button_realize             (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_pen_button_realize                  (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_polselect_button_realize            (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_text_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_arc_button_realize                  (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_oval_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_brush_button_realize                (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_erase_button_realize                (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_lasso_button_realize                (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_fill_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_rectangle_button_realize            (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_freehand_button_realize             (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_pen_button_realize                  (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_polselect_button_realize            (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_text_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_arc_button_realize                  (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_oval_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_brush_button_realize                (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_line_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_about_dialog_pixmap_realize         (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_label8_realize                      (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_about_dialog_ok_button_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_about_dialog_version_label_realize  (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_emboss_menu_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_line_width_combo_combo_entry_changed
                                        (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_line_width_combo_combo_entry_changed
                                        (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_background_color_picker_realize     (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_foreground_color_picker_realize     (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_line_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_multiline_button_realize            (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_curve_button_realize                (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_unfilled_button_realize             (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_filled_button_realize               (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_closed_freehand_button_realize      (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_filled_button_realize               (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_mainwindow_focus_in_event           (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
on_mainwindow_focus_out_event          (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

void
on_mainwindow_set_focus_child          (GtkContainer    *container,
                                        GtkWidget       *widget,
                                        gpointer         user_data);


void
on_mainwindow_set_focus                (GtkWindow       *window,
                                        GtkWidget       *widget,
                                        gpointer         user_data);

void
on_fontpicker_map                      (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_background_color_picker_map         (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_foreground_color_picker_map         (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_fontpicker_map_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_background_color_picker_map_event   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_foreground_color_picker_map_event   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_set_as_background_centered_activate (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_set_as_background_titled_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_foreground_color_picker_color_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_foreground_color_picker_color_pressed
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_foreground_color_picker_color_released
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_color_palette_entry_map             (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_color_palette_entry_draw            (GtkWidget       *widget,
                                        GdkRectangle    *area,
                                        gpointer         user_data);

gboolean
on_color_palette_entry_key_press_event (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

gboolean
on_color_palette_entry_key_release_event
                                        (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

void
on_color_palette_entry_realize         (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_color_palette_entry_button_press_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_color_palette_entry_button_release_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_color_palette_entry_expose_event    (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

void
on_select_all_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_print_preview_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_get_desktop_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_undo_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about_gnome_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about_gnu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_about_dialog_button_release_event   (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_about_dialog_button_release_event   (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_exit_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_tool_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_tool_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_tool_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_fille_button_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_filled_button_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

GtkWidget*
create_drawing_area_in_scroll_frame (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_tool_palette_realize                (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_image_effect_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_image_effect_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_image_effect_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_new_canvas_window_realize           (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_new_canvas_window_destroy           (GtkObject       *object,
                                        gpointer         user_data);

void
on_fontpicker_realize                  (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_rotate_menu_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_create_new_window_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_close_this_window_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_close_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_close_this_window_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_close_this_window_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);



void
on_close_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_mainwindow_destroy_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_tool_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_toot_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_tool_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_tool_freehand_button_realize        (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_image_effect_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_image_effect_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_color_palette_entry_expose_event    (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

GtkWidget*
create_drawing_area_in_scroll_frame (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_tool_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_tool_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_tool_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_color_palette_entry_expose_event    (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

void
on_tool_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_tool_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_tool_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_tool_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_tool_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_tool_button_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_mainwindow_destroy                  (GtkObject       *object,
                                        gpointer         user_data);

void
on_invert_menu_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_windows_menu_item_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_close_this_window_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_close_window_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_create_new_window                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rotate_menu_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rotate_menu_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rotate_menu_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
