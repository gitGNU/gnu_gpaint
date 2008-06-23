/* $Id: tool_palette.h,v 1.2 2004/03/13 03:33:51 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2003  Li-Cheng (Andy) Tai, Michael Meffie
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

#ifndef __TOOL_PALETTE_H__
#define __TOOL_PALETTE_H__

#include <gtk/gtk.h>
#include "canvas.h"

GtkWidget *lookup_tool_palette(GtkWidget*);
gpaint_tool *tool_palette_get_tool(GtkWidget*,const gchar*);

#endif
