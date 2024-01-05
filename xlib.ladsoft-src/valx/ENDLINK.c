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
/*                                ENDLINK.C                               */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           end_linker                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void end_linker(bit_16 return_code)
BeginDeclarations
EndDeclarations
BeginCode
 If statistics.val IsTrue
  Then
   linker_statistics();
  EndIf;
 exit(return_code);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           linker_statistics                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void linker_statistics()
BeginDeclarations
file_info_ptr                          file;
#define File                           (*file)
EndDeclarations
BeginCode
 statistics_start_time = Now;
 statistics.val        = False;  /* Prevent recursive call. */
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       Memory Usage Statistics                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If link_step Exceeds 0
  Then
		/* not available */
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                        Object File Statistics                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If (link_step Exceeds 4) AndIf (obj_file_list.first IsNotNull)
  Then
   linker_message("\n"
                  "Object File Statistics:\n");
   linker_message("--Size--  -------------Object File-------------\n");
   TraverseList(obj_file_list, file)
    BeginTraverse
     edit_number_string(temp_string, "%lu", File.file_size);
     linker_message("%8s  %s\n", String(temp_string), File.filename);
    EndTraverse;
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                        Library File Statistics                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If (link_step Exceeds 4) AndIf (lib_file_list.first IsNotNull)
  Then
   linker_message("\n"
                  "Library File Statistics:\n");
   linker_message("--Size--  Modules  Passes  "
                  "-------------Library File------------\n");
   TraverseList(lib_file_list, file)
    BeginTraverse
     edit_number_string(temp_string, "%lu", File.file_size);
     linker_message("%8s  ",
                    String(temp_string));
     edit_number_string(temp_string, "%u",  File.module_count);
     linker_message("%7s  ",
                    String(temp_string));
     edit_number_string(temp_string, "%u",  File.pass_count);
     linker_message("%6s  %s\n",
                    String(temp_string),
                    File.filename);
    EndTraverse;
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       Time Usage Statistics                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 total_time = 0;
 linker_message("\n"
                "Time Usage Statistics:\n");
 If link_step Exceeds 0
  Then
   total_time += user_input_start_time - linker_start_time;
   linker_message("  Primary linker initialization time:%7s\n",
                  elapsed_time(linker_start_time,
                               user_input_start_time));
  EndIf;
 If link_step Exceeds 1
  Then
   total_time += secondary_init_start_time - user_input_start_time;
   linker_message("                     User input time:%7s\n",
                  elapsed_time(user_input_start_time,
                               secondary_init_start_time));
  EndIf;
 If link_step Exceeds 2
  Then
   total_time += library_directory_start_time -
                 secondary_init_start_time;
   linker_message("Secondary linker initialization time:%7s\n",
                  elapsed_time(secondary_init_start_time,
                               library_directory_start_time));
  EndIf;
 If link_step Exceeds 3
  Then
   total_time += object_module_start_time - library_directory_start_time;
   linker_message("   Library directory processing time:%7s\n",
                  elapsed_time(library_directory_start_time,
                               object_module_start_time));
  EndIf;
 If link_step Exceeds 4
  Then
   total_time += library_processing_start_time - object_module_start_time;
   linker_message("       Object module processing time:%7s\n",
                  elapsed_time(object_module_start_time,
                               library_processing_start_time));
  EndIf;
 If link_step Exceeds 5
  Then
   total_time += order_start_time - library_processing_start_time;
   linker_message("             Library processing time:%7s\n",
                  elapsed_time(library_processing_start_time,
                               order_start_time));
  EndIf;
 If link_step Exceeds 6
  Then
   total_time += fixup_start_time - order_start_time;
   linker_message("  Segment ordering and aligning time:%7s\n",
                  elapsed_time(order_start_time,
                               fixup_start_time));
  EndIf;
 If link_step Exceeds 7
  Then
   total_time += resource_start_time - fixup_start_time;
   linker_message("                          Fixup time:%7s\n",
                  elapsed_time(fixup_start_time,
                               resource_start_time));
  EndIf;
 If link_step Exceeds 8
  Then
   total_time += exec_image_start_time - resource_start_time;
   linker_message("         Resource file load time:%7s\n",
                  elapsed_time(resource_start_time,
                               exec_image_start_time));
  EndIf;
 If link_step Exceeds 9
  Then
   total_time += map_start_time - exec_image_start_time;
   linker_message("         Executable image write time:%7s\n",
                  elapsed_time(exec_image_start_time,
                               map_start_time));
  EndIf;
 If link_step Exceeds 10
  Then
   total_time += statistics_start_time - map_start_time;
   linker_message("                      Map write time:%7s\n",
                  elapsed_time(map_start_time,
                               statistics_start_time));
  EndIf;
 linker_end_time = Now;
 total_time += linker_end_time - statistics_start_time;
 linker_message("           Statistics reporting time:%7s\n",
                elapsed_time(statistics_start_time,
                             linker_end_time));
 linker_message("                                      ------\n"
                "                 Total Elaspsed Time:%7s\n",
                elapsed_time(0L, total_time));

 return;
EndCode
#undef File

