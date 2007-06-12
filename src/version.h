/* $Id: version.h,v 1.7 2005/01/27 02:53:32 meffie Exp $
 *
 * GNU Paint 
 * Copyright 2000-2003, 2007  Li-Cheng (Andy) Tai
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

#ifndef __VERSION_H__
#define __VERSION_H__

#ifdef PACKAGE
# define PROGRAM_TITLE PACKAGE 
#else
# define PROGRAM_TITLE "GNU Paint"
#endif

#ifdef VERSION
# define VERSION_STRING VERSION
#else
# define VERSION_STRING "0.3.1"
#endif

#endif
