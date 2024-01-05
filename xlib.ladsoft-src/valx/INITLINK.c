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
#include <windows.h>
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
#include "..\version.h"

/*                                 INITLINK.C                              */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       primary_linker_initialization                     |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void primary_linker_initialization(byte *program_directory)
BeginDeclarations
  byte_ptr        p ;
  string_ptr config_name ;
EndDeclarations
BeginCode
 /* Note start time */
 linker_start_time = Now;

 /* Issue Signon message */
#ifndef MSDOS
 if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE)
#endif
    linker_message("VALX Version %s %s\n",VALX_STRING_VERSION, PRODUCT_COPYRIGHT);
// linker_message("VALX Experimental Linker  Compiled %s %s\n",__DATE__,__TIME__);
//  linker_message("%s",GetCommandLine()) ;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                     Memory Pool Initialization                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 initialize_memory();

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                     Constant String Initialization                      |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* Uninitialized strings */

 current_filename         = allocate_string( TEMP_STRING_LENGTH);
 current_path             = allocate_string( TEMP_STRING_LENGTH);
 default_directory_string = allocate_string( TEMP_STRING_LENGTH);
 default_filename         = allocate_string( TEMP_STRING_LENGTH);
 help_filename            = allocate_string( TEMP_STRING_LENGTH);
 next_token               = allocate_string( TEMP_STRING_LENGTH);
 parm_string              = allocate_string( 65530);
 sym_filename_string      = allocate_string( TEMP_STRING_LENGTH);
 temp_string              = allocate_string( TEMP_STRING_LENGTH);
 token                    = allocate_string( TEMP_STRING_LENGTH);

/* Initialized strings */

 ampersand_string         = make_constant_string(
                                                 (byte *) "&");
 and_string               = make_constant_string(
                                                 (byte *) "and");
 at_string                = make_constant_string(
                                                 (byte *) "@");
 backslash_string         = make_constant_string(
                                                 (byte *) "\\");
 backslash_dot_string     = make_constant_string(
                                                 (byte *) "\\.\\");
 backslash_dot_dot_string = make_constant_string(
                                                 (byte *) "\\..\\");
 bar_string               = make_constant_string(
                                                 (byte *) "|");
 class_string             = make_constant_string(
                                                 (byte *) "class");
 close_angle_string       = make_constant_string(
                                                 (byte *) ">");
 close_paren_string       = make_constant_string(
                                                 (byte *) ")");
 colon_string             = make_constant_string(
                                                 (byte *) ":");
 com_extension_string     = make_constant_string(
                                                 (byte *) ".com");
 comma_string             = make_constant_string(
                                                 (byte *) ",");
 cv_extension_string      = make_constant_string(
                                                 (byte *) ".lss");
 def_extension_string     = make_constant_string(
                                                 (byte *) ".def");
 default_drive_string     = default_drive();
 default_lx_stub_string   = make_constant_string(
                                                 (byte *) "stub32a.exe");
 device_AUX               = make_constant_string(
                                                 (byte *) "aux:");
 device_CON               = make_constant_string(
                                                 (byte *) "con:");
 device_PRN               = make_constant_string(
                                                 (byte *) "prn:");
 dll_extension_string     = make_constant_string(
                                                 (byte *) ".dll");
 dot_string               = make_constant_string(
                                                 (byte *) ".");
 env_extension_string     = make_constant_string(
                                                 (byte *) ".env");
 exclamation_string       = make_constant_string(
                                                 (byte *) "!");
 exe_extension_string     = make_constant_string(
                                                 (byte *) ".exe");
 false_string = make_constant_string(
                                                 (byte *) "false");
 group_string             = make_constant_string(
                                                 (byte *) "group");
 help_extension_string    = make_constant_string(
                                                 (byte *) ".txt");
 lib_extension_string     = make_constant_string(
                                                 (byte *) ".lib");
 lst_extension_string     = make_constant_string(
                                                 (byte *) ".map");
 minus_string             = make_constant_string(
                                                 (byte *) "-");
 not_string               = make_constant_string(
                                                 (byte *) "not");
 null_string              = make_constant_string(
                                                 (byte *) "");
 obj_extension_string     = make_constant_string(
                                                 (byte *) ".obj");
 open_paren_string        = make_constant_string(
                                                 (byte *) "<");
 open_paren_string        = make_constant_string(
                                                 (byte *) "(");
 or_string                = make_constant_string(
                                                 (byte *) "or");
 plus_string              = make_constant_string(
                                                 (byte *) "+");
 program_directory_string = make_constant_string(
                                                 program_directory);
 process_filename(program_directory_string);

 resource_extension_string= make_constant_string(
                                                 (byte *) ".res");
 segment_string           = make_constant_string(
                                                 (byte *) "segment");
 semicolon_string         = make_constant_string(
                                                 (byte *) ";");
 space_string             = make_constant_string(
                                                 (byte *) " ");
 star_string              = make_constant_string(
                                                 (byte *) "*");
 sym_extension_string     = make_constant_string(
                                                 (byte *) ".sym");
 sys_extension_string     = make_constant_string(
                                                 (byte *) ".sys");
 tilde_string             = make_constant_string(
                                                 (byte *) "~");
 true_string              = make_constant_string(
                                                 (byte *) "true");
 quoted = ' ' ;

 program_config_file_string = allocate_string( TEMP_STRING_LENGTH) ;
 copy_string( program_config_file_string, program_directory_string) ;
 p = strrchr(String(program_config_file_string),'\\') ;
 If p IsNotNull 
   Then
     config_name = make_constant_string( (byte *) "VALX.CFG") ;
     trunc_string(program_config_file_string,p+1-String(program_config_file_string)) ;
     concat_string(program_config_file_string,config_name) ;
     process_filename(program_config_file_string) ;
   Else
     trunc_string(program_config_file_string,0) ;
   EndIf
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          DOS Initialization                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          Other Initialization                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 current_record_header     = (obj_record_header_ptr) object_file_element;
 last_LIDATA_record_header = (obj_record_header_ptr) last_LIDATA_record;
 copy_string(help_filename, program_directory_string);
 change_extension(help_filename, help_extension_string);
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                     secondary_linker_initialization                     |
  |                                                                         |
  +-------------------------------------------------------------------------+

There are several steps in the initialization process which had to be
delayed because switches specified by the user affect the initialization
process.  After user input is processed, this procedure is called and that
initialization occurs. */

void secondary_linker_initialization()
BeginDeclarations
EndDeclarations
BeginCode

 secondary_init_start_time = Now;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Allocate the I/O buffer.                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 infile_buffer              = allocate_memory(
                                              Bit_32(buffer_size.val));
 infile_buffer_2            = allocate_memory(
                                              Bit_32(buffer_size.val));
 infile.buffer              = infile_buffer;
 infile.buffer_size         = buffer_size.val;

 outfile_buffer             = allocate_memory(
                                              Bit_32(buffer_size.val));
 outfile.buffer             = outfile_buffer;
 outfile.buffer_size        = buffer_size.val;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           Allocate hash tables.                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 externals          = (public_entry_ptr_array)
                       allocate_memory(
                                       (Bit_32(max_externals)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));

 exports            = (public_entry_ptr_array)
                       allocate_memory(
                                       (Bit_32(max_exports)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));

 imports            = (public_entry_ptr_array)
                       allocate_memory(
                                       (Bit_32(max_imports)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));

 gnames             = (group_entry_ptr_array)
                       allocate_memory(
                                       (Bit_32(max_groups)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));

 group_hash_table   = (group_entry_ptr_array)
                       allocate_memory(
                                       Bit_32(group_table_hash_size.val) *
                                        Bit_32(sizeof(group_entry_ptr)));

 lname_hash_table   = (lname_entry_ptr_array)
                       allocate_memory(
                                       Bit_32(lname_table_hash_size.val) *
                                        Bit_32(sizeof(lname_entry_ptr)));

 lnames             = (lname_entry_ptr_array)
                       allocate_memory(
                                       (Bit_32(max_lnames)+1L) *
                                        Bit_32(sizeof(lname_entry_ptr)));

 public_hash_table  = (public_entry_ptr_array)
                       allocate_memory(
                                       Bit_32(public_table_hash_size.val) *
                                       Bit_32(sizeof(public_entry_ptr)));

 segment_hash_table = (segment_entry_ptr_array)
                       allocate_memory(
                                       Bit_32(segment_table_hash_size.val) *
                                        Bit_32(sizeof(segment_entry_ptr)));

 snames             = (lseg_ptr_array)
                       allocate_memory(
                                       (Bit_32(max_segments)+1L) *
                                        Bit_32(sizeof(lseg_ptr)));

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                  Miscellaneous Secondary Initialization                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 c_common_lname  = lookup_lname(8, (byte *) "c_common");
 generated_lname = lookup_lname(11 ,(byte *) "valx");
 none_lname      = lookup_lname(6,  (byte *) "(none)");
 If case_ignore.val IsTrue
  Then
   CODE_lname               = lookup_lname(4, (byte *) "code");
   DATA_lname               = lookup_lname(4, (byte *) "data");
   desc_lname              = lookup_lname(5, (byte *) "desc");
   BSS_lname                = lookup_lname(3, (byte *) "bss");
   DGROUP_lname             = lookup_lname(6, (byte *) "dgroup");
   FAR_BSS_lname            = lookup_lname(7, (byte *) "far_bss");
   FLAT_lname               = lookup_lname(4, (byte *) "flat");
   HUGE_BSS_lname           = lookup_lname(8, (byte *) "huge_bss");
   STACK_lname              = lookup_lname(5, (byte *) "stack");
   codeview_class_DEBSYM    = lookup_lname(6, (byte *) "debsym");
   codeview_class_DEBTYP    = lookup_lname(6, (byte *) "debtyp");
   codeview_class_DEBBROWSE = lookup_lname(6, (byte *) "browse") ;
   codeview_segment_SYMBOLS = lookup_lname(9, (byte *) "$$symbols");
   codeview_segment_TYPES   = lookup_lname(7, (byte *) "$$types");
   codeview_segment_BROWSE  = lookup_lname(8, (byte *) "$$browse");
  Else
   CODE_lname               = lookup_lname(4, (byte *) "CODE");
   DATA_lname               = lookup_lname(4, (byte *) "DATA");
   desc_lname              = lookup_lname(5, (byte *) "DESC");
   BSS_lname                = lookup_lname(3, (byte *) "BSS");
   DGROUP_lname             = lookup_lname(6, (byte *) "DGROUP");
   FAR_BSS_lname            = lookup_lname(7, (byte *) "FAR_BSS");
   FLAT_lname               = lookup_lname(4, (byte *) "FLAT");
   HUGE_BSS_lname           = lookup_lname(8, (byte *) "HUGE_BSS");
   STACK_lname              = lookup_lname(5, (byte *) "STACK");
   codeview_class_DEBSYM    = lookup_lname(6, (byte *) "DEBSYM");
   codeview_class_DEBTYP    = lookup_lname(6, (byte *) "DEBTYP");
   codeview_class_DEBBROWSE = lookup_lname(6, (byte *) "BROWSE") ;
   codeview_segment_SYMBOLS = lookup_lname(9, (byte *) "$$SYMBOLS");
   codeview_segment_TYPES   = lookup_lname(7, (byte *) "$$TYPES");
   codeview_segment_BROWSE  = lookup_lname(8, (byte *) "$$BROWSE");
  EndIf;
 exefile = (comfile.val IsFalse) AndIf (sysfile.val IsFalse);
 return;
EndCode
