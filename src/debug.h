/* $Id: debug.h,v 1.5 2004/12/10 23:28:51 meffie Exp $
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

/*
 * Debugging macros. To include the debugging macros in all the 
 * modules, use the debug target on the make command:
 *
 *   $ make clean debug
 *   
 * or add #define GPAINT_DEBUG to the top of the module which
 * includes this file.
 *
 *    debug_fn()           -- print the name of the current function
 *    debug_fn1()          -- print the function and one param
 *    debug_fn2()          -- print the function and two params
 *    debug_fn3()          -- print the function and three params
 *
 *    debug(msg)           -- print the message string
 *    debug1(format,x)     -- print one param using the format string
 *    debug2(format,x,y)   -- print two params using the format string
 *    debug2(format,x,y,z) -- print three params using the format string
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>

#ifdef GPAINT_DEBUG
# define DEBUG_HDR      "** Debug : %s (%d) : "
# define DEBUG_HDR_FN   "** Debug : %s (%d) : %s() : "

# define debug(msg)             g_print(DEBUG_HDR "%s\n", __FILE__, __LINE__, msg);
# define debug1(format,x)       g_print(DEBUG_HDR format "\n", __FILE__, __LINE__, (x));
# define debug2(format,x,y)     g_print(DEBUG_HDR format "\n", __FILE__, __LINE__, (x),(y));
# define debug3(format,x,y,z)   g_print(DEBUG_HDR format "\n", __FILE__, __LINE__, (x),(y),(z));

# define debug_fn()               g_print(DEBUG_HDR_FN "\n", __FILE__, __LINE__, __PRETTY_FUNCTION__);
# define debug_fn1(format,x)      g_print(DEBUG_HDR_FN format "\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, (x));
# define debug_fn2(format,x,y)    g_print(DEBUG_HDR_FN format "\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, (x),(y));
# define debug_fn3(format,x,y,z)  g_print(DEBUG_HDR_FN format "\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, (x),(y),(z));
#else
# define debug(msg)
# define debug1(format,x)
# define debug2(format,x,y)
# define debug3(format,x,y,z)
# define debug_fn()
# define debug_fn1(format,x)
# define debug_fn2(format,x,y)
# define debug_fn3(format,x,y,z)
#endif

#endif
