/* $Id: brush.h,v 1.2 2004/03/13 03:25:59 meffie Exp $
 *
 * GNU Paint
 * Copyright 2000-2003  Li-Cheng (Andy) Tai
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA
 */

#ifndef __BRUSH_H__
#define __BRUSH_H__

#include "canvas.h"

gpaint_tool* paint_brush_create(const char *name);
gpaint_tool* eraser_create(const char *name);

#endif
