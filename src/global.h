/* 
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


#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <gtk/gtk.h>

#define GLADE_DATA_FILE (PACKAGE_DATA_DIR "/gpaint/glade/gpaint.glade")

static const char *GLADE_XML = "GLADE_XML";
static const char *DRAWING_AREA = "drawing_area";

GtkWidget* create_about_dialog (void);
GtkWidget* create_new_canvas_window (void);

#endif

