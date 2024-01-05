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
/*                                  MAIN.C                                 */
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 main                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void main(bit_16 argc, byte *argv[])
BeginDeclarations
EndDeclarations
BeginCode
 primary_linker_initialization(argv[0]);   link_step++;
 get_filenames_from_user(argc, argv);      link_step++;
 secondary_linker_initialization();        link_step++;
 process_library_directories();            link_step++;
 process_def_file();                       link_step++;
 process_object_modules();                 link_step++;
 process_libraries();                      link_step++;
 order_and_align_segments();               link_step++;
 pass_two();                               link_step++;
 process_resource_files() ;                link_step++;
 write_executable_image();                 link_step++;
 link_map();
 If bad_link
  Then
   file_delete(exe_file_list.first) ;
  EndIf ;
 end_linker(bad_link);
EndCode
