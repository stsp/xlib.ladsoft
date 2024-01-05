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
/*                                 USERINP.C                               */

void scan_env(byte *output,byte *string)
BeginDeclarations
byte                       name[256];
byte                       *p = name ;
EndDeclarations
BeginCode
   While *string IsNotZero
     BeginWhile
       If *string == '%'
         Then
           p = name ;
           string++ ;
           While *string IsNotZero && *string != '%'
             BeginWhile
               *p++ = *string++ ;
             EndWhile ;
           If *string IsNotZero
             Then
               string++ ;
             EndIf ;
           *p = 0 ;
           p = getenv(name) ;
           If p IsNotNull 
             Then
               strcpy(output,p) ;
               output += strlen(output) ;
             EndIf ;
         Else
           *output++ = *string++ ;
         EndIf ;
     EndWhile ;
   *output = 0 ;
EndCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                      get_filenames_from_user                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void get_filenames_from_user(bit_16 argc, byte *argv[])
BeginDeclarations
byte                                  *env_var;
bit_16                                 i;
file_info_ptr                          file_entry;
#define File_entry                     (*file_entry)
token_stack_ptr                        source_element;
#define Source_element                 (*source_element)
FILE                                   *fil ;
byte                                   linebuf[1024] ;
byte                                   tempbuf[256] ;
char                                   buf[260] ;
EndDeclarations
BeginCode

 user_input_start_time = Now;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |           concatenate the environment files into parm_string            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 copy_string(parm_string, null_string);
 copy_string(default_filename, program_directory_string);
 change_extension(default_filename, env_extension_string);
 If file_exists(default_filename, 0)
  Then
   concat_string(parm_string, at_string);
   concat_string(parm_string, default_filename);
   concat_string(parm_string, space_string);
  EndIf;
 copy_string(token, default_filename);
 cut_string(token,
            0,
            reverse_index_string(token, 
                                 0xFFFF, 
                                 backslash_string) + 1);
 process_filename(token);
 If (file_exists(token, 0)) AndIf 
    (compare_string(token, default_filename) IsNotZero)
  Then
   concat_string(parm_string, at_string);
   concat_string(parm_string, token);
   concat_string(parm_string, space_string);
  EndIf;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |       concatenate the LINK environment variable into parm_string        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
#ifdef XXXXX
 env_var = (byte *) getenv("LINK");
 If env_var IsNull
  Then
#endif
   env_var = (byte *) "";
#ifdef XXXXX
  EndIf;
#endif
 concat_string(parm_string, string(BytePtr(env_var)));
 concat_string(parm_string, space_string);
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |               concatenate the config file into parm_string              |
  |                                                                         |
  | breaking with tradition so I can do line-oriented input                 |
  +-------------------------------------------------------------------------+*/

  If Length(program_config_file_string) IsNotZero
    Then
      If (fil = fopen(String(program_config_file_string),"r")) IsNotNull
        Then
          While Not feof(fil)
            BeginWhile
              linebuf[0] = 0;
              fgets(tempbuf,256,fil) ;
              scan_env(linebuf,tempbuf) ;
              If linebuf[0] IsNotZero
                Then
                  If linebuf[strlen(linebuf)-1] Is '\n'
                    Then
                      linebuf[strlen(linebuf)-1] = 0 ;
                    EndIf
                  concat_string(parm_string,string(linebuf)) ;
                  concat_string(parm_string, space_string) ;
                EndIf
            EndWhile
        EndIf
    EndIf
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |               concatenate the parm line into parm_string                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 For i=1; i<argc; i++
  BeginFor
   If i Exceeds 1
    Then
     concat_string(parm_string, space_string);
    EndIf;
   concat_string(parm_string, string(BytePtr(argv[i])));
  EndFor;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |             Start input processing with the parm line.                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 source_element                    = get_free_token_source_element();
 Source_element.source_file        = Null;
 Source_element.token_string       = parm_string;
 Source_element.token_string_index = 0;
 Push source_element OnTo token_stack EndPush;
 token_break_char                  = ' ';

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       Process OBJ file list                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 
 default_extension = obj_extension_string;
 copy_string(default_filename, null_string);
 default_prompt    = "OBJ file(s)%s:  ";
 prompting_for     = 1;
 process_user_input_files(Addr(obj_file_list),
                          True);
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process EXE file                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If comfile.val IsTrue
  Then
   default_extension = com_extension_string;
   default_prompt = "COM file[%s]:  ";
  Else
   If sysfile.val IsTrue
    Then
     default_extension = sys_extension_string;
     default_prompt = "SYS file[%s]:  ";
    Else
		  If build_DLL.val IsTrue
			  Then
     	    default_extension = dll_extension_string;
          default_prompt = "DLL file[%s]:  ";
				Else
     	    default_extension = exe_extension_string;
          default_prompt = "EXE file[%s]:  ";
		    EndIf ;
    EndIf;
  EndIf;
 copy_string(default_filename, string((*(obj_file_list.first)).filename));
 change_extension(default_filename, default_extension);
 prompting_for = 2;
 process_user_output_file(Addr(exe_file_list),
                          False);

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       Process CV output file                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

   copy_string(token, string((*(exe_file_list.first)).filename));
   change_extension(token, cv_extension_string);
   file_entry = (file_info_ptr)
                 allocate_memory(Bit_32(sizeof(file_info_type)) +
                                 Bit_32(Length(token)));
   far_move(File_entry.filename, String(token), Length(token)+1);
   Insert file_entry AtEnd InList cv_file_list EndInsert;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process MAP file                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 default_extension = lst_extension_string;
 copy_string(default_filename, null_string);
 default_prompt    = "MAP file:%s  ";
 prompting_for     = 3;
 process_user_output_file(Addr(lst_file_list),
                          False);
 If (First(lst_file_list) IsNull)                 AndIf
    ((map.set IsTrue) OrIf (detail_level.val Exceeds 0))
  Then
   copy_string(token, string((*(exe_file_list.first)).filename));
   change_extension(token, lst_extension_string);
   file_entry = (file_info_ptr)
                 allocate_memory(Bit_32(sizeof(file_info_type)) +
                                 Bit_32(Length(token)));
   far_move(File_entry.filename, String(token), Length(token)+1);
   Insert file_entry AtEnd InList lst_file_list EndInsert;
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       Process LIB file list                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 default_extension = lib_extension_string;
 copy_string(default_filename, null_string);
 default_prompt    = "LIB file(s):%s  ";
 prompting_for     = 4;
 process_user_input_files(Addr(lib_file_new_list),
                          False);

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       Process RESOURCE file list                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If lxfile.val IsTrue OrIf lefile.val IsTrue
  Then
   pefile.val = False ;
   build_DLL.val = False ;
   win_subsystem.val = False ;
   use32.val = True ;
  EndIf ;

 If pefile.val IsTrue
  Then
	 use32.val = True ; /* Default PE files to 32-bit */
   default_extension = resource_extension_string;
   copy_string(default_filename, null_string);
   default_prompt    = "RES file(s):%s  ";
   prompting_for     = 5;
   process_user_input_files(Addr(resource_file_list),
                            False);
   default_extension = def_extension_string;
   copy_string(default_filename, null_string);
   default_prompt    = "DEF file:%s  ";
   prompting_for     = 6;
   process_user_output_file(Addr(def_file_list),
                          False);
  EndIf
 If build_DLL.val IsTrue
	Then
	 use32.val = True ;
/*	 win_subsystem.val = PE_SUBSYS_WINDOWS ; */
  EndIf ;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process stub file                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
  If stub_filename.val IsNotNull
   Then
    If lib_directory.val IsNotNull
       Then
         strcpy(buf,String(stub_filename.val)) ;

         fil = SearchPath(buf, String(lib_directory.val),"r") ;
         If fil IsNotNull
            Then
               fclose(fil) ;
               strcpy(String(stub_filename.val),buf) ;
               Length(stub_filename.val) = strlen(buf) ;
            EndIf
       EndIf
      process_filename(stub_filename.val) ;
      If index_string(stub_filename.val, 0, colon_string) IsNot 1
       Then
        linker_error(4, "\"%s\" is an illegal stub file name.\n",
                        String(stub_filename.val));
       Else
         add_extension_to_file(stub_filename.val,exe_extension_string);
         add_files_to_list(&stub_file_list, stub_filename.val);
       EndIf;
	 EndIf

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process sym file                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
  If symbol_table.val IsTrue
   Then
    copy_string(sym_filename_string, string((*(exe_file_list.first)).filename));
    change_extension(sym_filename_string, sym_extension_string);
    file_entry = (file_info_ptr)
                 allocate_memory(Bit_32(sizeof(file_info_type)) +
                                 Bit_32(Length(sym_filename_string)));
    far_move(File_entry.filename, String(sym_filename_string), Length(sym_filename_string)+1);
    Insert file_entry AtEnd InList sym_file_list EndInsert;
   EndIf
 return;
EndCode
#undef File_entry
#undef Source_element

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                      process_user_input_files                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void process_user_input_files(file_info_list *list,
                              bit_16          required)
BeginDeclarations
#define List                           (*list)
byte                                   buf[256] ;
FILE                                   *fil ;
bit_32                                 dot_index;
EndDeclarations
BeginCode
 scan_out_token();
 Repeat
  BeginRepeat
   get_filename_token(required, list);
   If Length(token) Exceeds 0
    Then
        dot_index = index_string(token,0,dot_string);
		If dot_index Is 0xffff
         Then
            concat_string(token,default_extension);
         EndIf;
     
     // add_extension_to_file(token,default_extension);
     If lib_directory.val IsNotNull
       Then
         strcpy(buf,String(token)) ;

         fil = SearchPath(buf, String(lib_directory.val),"r") ;
         If fil IsNotNull
            Then
               fclose(fil) ;
               strcpy(String(token),buf) ;
               Length(token) = strlen(buf) ;
            EndIf
       EndIf
     process_filename(token);
      If index_string(token, 0, colon_string) IsNot 1
       Then
        linker_error(4, "\"%s\" is an illegal input file name.\n",
                        String(token));
       Else
         add_files_to_list(list, token);
       EndIf;
    EndIf;
   RepeatIf more_tokens
  EndRepeat;
 return;
EndCode
#undef List

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                      process_user_output_file                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void process_user_output_file(file_info_list *list,
                              bit_16          required)
BeginDeclarations
file_info_ptr                          file_entry;
#define File_entry                     (*file_entry)
#define List                           (*list)
EndDeclarations
BeginCode
 scan_out_token();
 Repeat
  BeginRepeat
   get_filename_token(required, list);
   If Length(token) Exceeds 0
    Then
     process_filename(token);
     add_extension_to_file(token,default_extension);
     If List.first IsNull
      Then
       file_entry = (file_info_ptr)
                     allocate_memory(Bit_32(sizeof(file_info_type)) +
                                     Bit_32(Length(token)));
       far_move(File_entry.filename, String(token), Length(token)+1);
       Insert file_entry AtEnd InList List EndInsert;
      EndIf;
    EndIf;
   RepeatIf more_tokens
  EndRepeat;
 return;
EndCode
#undef File_entry
#undef List
