/* $Id: image_processing.h,v 1.3 2004/03/13 03:49:19 meffie Exp $
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

#ifndef __IMAGE_PROCESSING_H__
#define __IMAGE_PROCESSING_H__

#include "image.h"

typedef void (*ImageEffect)(gpaint_image*,gpaint_image*);    

ImageEffect image_lookup_effect(const gchar*);


#endif
