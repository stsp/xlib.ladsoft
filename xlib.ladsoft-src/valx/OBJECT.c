/* 
VALX linker
Copyright 1997-2011 David Lindauer.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Original 16-bit linker written by:
David Troendle

You may contact the author of this derivative at:
	mailto::camille@bluegrass.net
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dos.h>
#include <time.h>

#include "langext.h"
#include "defines.h"
#include "types.h"
#include "subs.h"
#include "globals.h"
/*                                 OBJECT.C                                */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          process_object_modules                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void process_object_modules()
BeginDeclarations
file_info_ptr                          obj_file;
EndDeclarations
BeginCode
 object_module_start_time = Now;
 TraverseList(obj_file_list, obj_file)
  BeginTraverse
   file_open_for_read(obj_file);
   obj_tmodule();
   file_close_for_read();
  EndTraverse;
 return;
EndCode

