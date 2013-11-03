/* $Id: print.h,v 1.3 2004/03/13 03:49:19 meffie Exp $
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

#ifndef __PRINT_H__
#define __PRINT_H__


#include "image.h"

int do_print(gpaint_image *image, const gchar *name);
int do_print_preview(gpaint_image *image, const gchar *name);



#endif
