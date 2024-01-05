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
/*                                 FILES.C                                 */
/* convert to standard C when possible */

#ifdef BORLANDC
int _dos_getpwd(char *string, int null)
{
	char buf[256],*p;
	int l;
	getcwd(buf,255);
	p = strchr(buf,':');
	if (p) {
		p++;
		if (*p == '\\')
			p++;
	}
	else
		p = buf;
	l = strlen(p);
	
	strcpy(string,p);
	return l;
}
#endif

/* This is a windows function, BC40+ will allow this.  The only other
 * solution was a hack that won't work on win95
 */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         default_drive                                   |
  |                                                                         |
  |                         O/S dependent                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr default_drive()
BeginDeclarations
string_ptr                             drive;
bit_32                                 driven;
EndDeclarations
BeginCode
 drive = make_constant_string((byte *) " :");
 _dos_getdrive(&driven);
 *String(drive) = (char) driven + 'a' - 1;
 return(drive);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       default_directory                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr default_directory(string_ptr drive, string_ptr directory)
BeginDeclarations
EndDeclarations
BeginCode
 Length(directory) = _dos_getpwd(String(directory)+1,0);
 *String(directory) = '\\';
 Length(directory) = far_index(String(directory), 0);
 far_to_lower(String(directory), Length(directory));
 If LastCharIn(directory) IsNot '\\'
  Then
   concat_string(directory,backslash_string);
  EndIf;
 return(directory);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         add_extension_to_file                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr add_extension_to_file(string_ptr fn, string_ptr ext)
BeginDeclarations
bit_32                              dot_index;
bit_32                              backslash_index;
EndDeclarations
BeginCode
 If index_string(fn,0,colon_string) IsNot 1
  Then  /* AUX:, CON:, or PRN: */
   return(fn);
  EndIf;
 dot_index = reverse_index_string(fn, Length(fn), dot_string);
 backslash_index = reverse_index_string(fn, Length(fn), backslash_string);
 If dot_index Is 0xFFFF 
 	OrIf backslash_index IsNot 0xFFFF AndIf dot_index LessThan backslash_index
  Then
   concat_string(fn,ext);
  EndIf;
return(fn);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         add_files_to_list                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void add_files_to_list(file_info_list *file_list, string_ptr fn)
BeginDeclarations
bit_16                                 already_in_list;
byte_ptr                               matched_file;
bit_16                                 compare_len;
file_info_ptr                          file_entry;
#define File_entry                     (*file_entry)
#define File_list                      (*file_list)
bit_16                                 rc;
EndDeclarations
BeginCode
 matched_file = String(current_filename);
 rc = start_file_search(fn, 0);
 If rc IsNotZero
  Then
   linker_error(4,"No matching files for file specification:\n"
                  "\t\"%s\"\n",
                  String(fn));
  EndIf;
 While rc IsZero
  BeginWhile
   compare_len = Length(current_filename) + 1;
   already_in_list = 0;
   TraverseList(File_list, file_entry)
    BeginTraverse
     already_in_list = far_compare(matched_file,
                                   BytePtr(File_entry.filename),compare_len)
                       IsZero;
     ExitIf(already_in_list);
    EndTraverse;
   If Not already_in_list
    Then
     file_entry = (file_info_ptr)
                   allocate_memory(Bit_32(sizeof(file_info_type)) +
                                   Bit_32(compare_len) - 1L);
     File_entry.attribute       = DTA.attrib;
     File_entry.time_stamp      = DTA.wr_time;
     File_entry.date_stamp      = DTA.wr_date;
     File_entry.file_size       = DTA.size;
     File_entry.pass_count      =
     File_entry.module_count    = 0;
     far_move(File_entry.filename, matched_file, compare_len);
     Insert file_entry AtEnd InList File_list EndInsert;
    EndIf;
   rc = continue_file_search();
  EndWhile;
 return;
EndCode
#undef File_entry
#undef File_list

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              change_extension                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr change_extension(string_ptr  fn, string_ptr ext)
BeginDeclarations
EndDeclarations
BeginCode
 trunc_string(fn, reverse_index_string(fn,0xFFFF,dot_string));
 concat_string(fn, ext);
 return(fn);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          file_close_for_read                            |
  |                                                                         |
  |                           O/S dependent                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_close_for_read()
BeginDeclarations
#define File                           infile
EndDeclarations
BeginCode
 If File.file_handle IsNot stdin And
    File.file_handle IsNot stderr And
    File.file_handle IsNot stdout
  Then  /* Only issue close if not one of the standard handles. */
	 fclose(File.file_handle);
  EndIf;
 return;
EndCode
#undef File

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         file_close_for_write                            |
  |                                                                         |
  |                           O/S dependent                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_close_for_write()
BeginDeclarations
#define File                           outfile
bit_32                                  size;
EndDeclarations
BeginCode
 If File.bytes_in_buffer Exceeds 0
  Then
   size = fwrite(File.buffer,1,File.bytes_in_buffer,File.file_handle);
   if (size != File.bytes_in_buffer) 
     linker_error(255,"Trouble writing file \"%s\" at byte %lu.\n",
             (*File.file_info).filename, File.next_buffer_position);
  EndIf;
 If File.file_handle IsNot stdin And
    File.file_handle IsNot stderr And
    File.file_handle IsNot stdout
  Then  /* Only issue close if not one of the standard handles. */
	 fclose(File.file_handle);
  EndIf;
 return;
EndCode
#undef File

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          file_exists                                    |
  |                                                                         |
  |                         O/S dependent                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 file_exists(string_ptr fn, bit_16 attr)
BeginDeclarations
FILE                                   *rc;
EndDeclarations
BeginCode
 rc = fopen(String(fn),"r");
 If rc IsNotNull
  Then
   fclose(rc);
  EndIf
 return(rc IsNotNull);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              file_delete                                |
  |                                                                         |
  |                             O/S dependent                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_delete(file_info_ptr file_info)
BeginDeclarations
#define File_info                      (*file_info)
EndDeclarations
BeginCode
 unlink(File_info.filename);
 return;
EndCode
#undef File_info

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            file_IO_limit                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_IO_limit(bit_16 limit)
BeginDeclarations
#define File                           infile
EndDeclarations
BeginCode
 If (limit IsZero) OrIf (limit Exceeds File.buffer_size)
  Then
   File.IO_limit = File.buffer_size;
  Else
   File.IO_limit = limit;
  EndIf;
 return;
EndCode
#undef File

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         file_open_for_read                              |
  |                                                                         |
  |                           O/S dependent                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_open_for_read(file_info_ptr file_info)
BeginDeclarations
#define File                           infile
#define File_info                      (*file_info)
FILE                                  *rc;
EndDeclarations
BeginCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                    Initialize the data structure                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 File.current_byte               = File.buffer;
 File.IO_limit                   = File.buffer_size;
 File.start_of_buffer_position   =
 File.next_buffer_position       = 0L;
 File.bytes_in_buffer            =
 File.bytes_left_in_buffer       =
 File.byte_position              = 0;
 File.file_info                  = file_info;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                    Open the file and save the handle                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 rc = fopen(File_info.filename,"rb");
 If rc IsNull
   Then
    linker_error(255,"Trouble opening \"%s\" for input.\n",
           File_info.filename);
   EndIf
 File.file_handle = rc;
 return;
EndCode
#undef File
#undef File_info

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         file_open_for_write                             |
  |                                                                         |
  |                           O/S dependent                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_open_for_write(file_info_ptr file_info)
BeginDeclarations
#define File                           outfile
#define File_info                      (*file_info)
FILE                                  *rc;
EndDeclarations
BeginCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                    Initialize the data structure                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 File.current_byte               = File.buffer;
 File.bytes_left_in_buffer       =
 File.IO_limit                   = File.buffer_size;
 File.start_of_buffer_position   =
 File.next_buffer_position       = 0L;
 File.bytes_in_buffer            =
 File.byte_position              = 0;
 File.file_info                  = file_info;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                    Open the file and save the handle                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 rc = fopen(File_info.filename,"wb");
 If rc IsNull
   Then
    linker_error(255,"Trouble opening \"%s\" for output.\n",
           File_info.filename);
   EndIf
 File.file_handle = rc;
 return;
EndCode
#undef File
#undef File_info

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           file_position                                 |
  |                                                                         |
  |                           O/S dependent                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 get_read_file_position(void)
BeginDeclarations
#define File                           infile
EndDeclarations
BeginCode
   return File.start_of_buffer_position + File.byte_position;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           file_position                                 |
  |                                                                         |
  |                           O/S dependent                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_position(bit_32 position)
BeginDeclarations
#define File                           infile
EndDeclarations
BeginCode
 If (position NotLessThan File.start_of_buffer_position) AndIf
    (position LessThan    File.next_buffer_position)
  Then
   File.byte_position        = Bit_16(position-File.start_of_buffer_position);
   File.current_byte         = Addr(File.buffer[File.byte_position]);
   File.bytes_left_in_buffer = File.bytes_in_buffer - File.byte_position;
  Else
   If fseek(File.file_handle,position,SEEK_SET) IsNotZero
     Then
       linker_error(255,"Trouble positioning file \"%s\" to byte %lu.\n",
             (*File.file_info).filename, position);
     EndIf
   File.start_of_buffer_position   =
   File.next_buffer_position       = position;
   File.byte_position              =
   File.bytes_in_buffer            =
   File.bytes_left_in_buffer       = 0;
   File.current_byte               = File.buffer;
  EndIf;
 return;
EndCode
#undef File

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               file_read                                 |
  |                                                                         |
  |                             O/S dependent                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_read(byte_ptr into, bit_32 length)
BeginDeclarations
#define File                           infile
int_32                                size;
EndDeclarations
BeginCode
 While length Exceeds 0
  BeginWhile
   If length Exceeds File.bytes_left_in_buffer
    Then
     If File.bytes_left_in_buffer Exceeds 0
      Then
       far_move(into, File.current_byte, File.bytes_left_in_buffer);
       length -= File.bytes_left_in_buffer;
       into   += File.bytes_left_in_buffer;
      EndIf;
     
     If (size = fread(File.buffer,1,File.IO_limit, File.file_handle)) LessThanOrEqualTo 0
      Then
        linker_error(255,"Trouble reading file \"%s\" at byte %lu.\n",
               (*File.file_info).filename, File.next_buffer_position);
      EndIf
     File.bytes_in_buffer            =
     File.bytes_left_in_buffer       = size ;
     File.current_byte               = File.buffer;
     File.byte_position              = 0;
     File.start_of_buffer_position   = File.next_buffer_position;
     File.next_buffer_position       = File.start_of_buffer_position +
                                       File.bytes_in_buffer;
    Else
     far_move(into, File.current_byte, length);
     into                      += length;
     File.current_byte         += length;
     File.bytes_left_in_buffer -= length;
     File.byte_position        += length;
     length                     = 0;
    EndIf;
  EndWhile;
 return;
EndCode
#undef File

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               file_read line                                |
  |                                                                         |
  |                             O/S dependent                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_read_line(byte_ptr into, bit_32 length)
BeginDeclarations
#define File                           infile
int_32                                size;
EndDeclarations
BeginCode
 While length Exceeds 0
  BeginWhile
   If File.bytes_left_in_buffer Exceeds 0
    Then
	 If (*File.current_byte) IsNot '\r'
	  Then
       *into                      = *(File.current_byte);
       into                      += 1;
	  EndIf
     File.current_byte         += 1;
     File.bytes_left_in_buffer -= 1;
     File.byte_position        += 1;
     length                    -= 1;
	 If (File.current_byte[-1]) Is '\n'
	  Then
       into[0] = '\0';
	   return;
	  EndIf
	Else
     If (size = fread(File.buffer,1,File.IO_limit, File.file_handle)) LessThan 0
      Then
        linker_error(255,"Trouble reading file \"%s\" at byte %lu.\n",
               (*File.file_info).filename, File.next_buffer_position);
      EndIf
	 If size IsEqualTo 0
	  Then
	   into[0] = '\0';
	   return;
	  EndIf;
     File.bytes_in_buffer            =
     File.bytes_left_in_buffer       = size ;
     File.current_byte               = File.buffer;
     File.byte_position              = 0;
     File.start_of_buffer_position   = File.next_buffer_position;
     File.next_buffer_position       = File.start_of_buffer_position +
                                       File.bytes_in_buffer;
    EndIf;
  EndWhile;
  into[-1] = '\0';
 return;
EndCode
#undef File

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              file_write                                 |
  |                                                                         |
  |                             O/S dependent                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_write(byte_ptr from, bit_32 length)
BeginDeclarations
#define File                           outfile
EndDeclarations
BeginCode
 While length Exceeds 0L
  BeginWhile
   If length Exceeds Bit_32(File.bytes_left_in_buffer)
    Then
     far_move(File.current_byte, from, File.bytes_left_in_buffer);
     length               -= Bit_32(File.bytes_left_in_buffer);
     from                 += File.bytes_left_in_buffer;
     File.bytes_in_buffer += File.bytes_left_in_buffer;
     If fwrite(File.buffer,1,File.bytes_in_buffer,File.file_handle) IsNot
        File.bytes_in_buffer
      Then
        linker_error(255,"Trouble writing file \"%s\" at byte %lu.\n",
               (*File.file_info).filename, File.next_buffer_position);
      EndIf
     File.current_byte               = File.buffer;
     File.bytes_left_in_buffer       = File.buffer_size;
     File.bytes_in_buffer            =
     File.byte_position              = 0;
     File.start_of_buffer_position   = File.next_buffer_position;
     File.next_buffer_position       = File.start_of_buffer_position +
                                       File.bytes_left_in_buffer;
    Else
     far_move(File.current_byte, from, Bit_16(length));
     from                      += Bit_16(length);
     File.current_byte         += Bit_16(length);
     File.bytes_left_in_buffer -= Bit_16(length);
     File.byte_position        += Bit_16(length);
     File.bytes_in_buffer      += Bit_16(length);
     length                     = 0;
    EndIf;
  EndWhile;
 return;
EndCode
#undef File

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       process_filename                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr process_filename(string_ptr fn)
BeginDeclarations
bit_16                                 left;
bit_16                                 right;
EndDeclarations
BeginCode
 lowercase_string(fn);
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       Check for AUX:, CON: & PRN:                       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If compare_string(substr(fn,0,4), device_AUX) IsZero
  Then
   copy_string(fn, device_AUX);
   return(fn);
  EndIf;
 If compare_string(substr(fn,0,4), device_CON) IsZero
  Then
   copy_string(fn, device_CON);
   return(fn);
  EndIf;
 If compare_string(substr(fn,0,4), device_PRN) IsZero
  Then
   copy_string(fn, device_PRN);
   return(fn);               
  EndIf;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                      Add drive designator if missing.                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If compare_string(substr(fn,1,1), colon_string) IsNotZero
  Then
   paste_string(fn, 0, default_drive_string);
  EndIf;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |          Substitute current directory if not based from root.           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If compare_string(substr(fn,2,1), backslash_string) IsNotZero
  Then
   default_directory(fn, default_directory_string);
   paste_string(fn, 2, default_directory_string);
  EndIf;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |            Scan out all \. and \.. from filename.                       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 left  = index_string(fn, -1, backslash_string);
 right = index_string(fn, left+1, backslash_string);
 While right IsNot 0xffff
  BeginWhile
   If compare_string(substr(fn,left,4), backslash_dot_dot_string) IsZero
    Then
     cut_string(fn, left, 3);
     right = left;
     left  = reverse_index_string(fn, right-1, backslash_string);
     If left Is 0xffff
      Then
       return(null_string);
      EndIf;
     cut_string(fn, left, right-left);
     right = index_string(fn, left+1, backslash_string);
     ContinueLoop;
    Else
     If compare_string(substr(fn,left,3), backslash_dot_string) IsZero
      Then
       cut_string(fn, left, 2);
       right = index_string(fn, left+1, backslash_string);
       ContinueLoop;
      EndIf;
    EndIf;
   left  = right;
   right = index_string(fn, left+1, backslash_string);
  EndWhile;
 return(fn);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       start_file_search                                 |
  |                                                                         |
  |                         O/S dependent                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 start_file_search(string_ptr fn, bit_16 attr)
BeginDeclarations
bit_16                                 rc;
EndDeclarations
BeginCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                  Remember the current file path                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 copy_string(current_path, fn);
 trunc_string(current_path,
              reverse_index_string(current_path, 0xFFFF, backslash_string)+1);
 rc = _dos_findfirst(String(fn),attr, &DTA);
 If rc IsNotZero
  Then
   copy_string(current_filename, null_string);
  Else
   far_to_lower(DTA.name, 12);
   copy_string(current_filename, current_path);
   concat_string(current_filename, string(DTA.name));
  EndIf;
 return(rc);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       continue_file_search                              |
  |                                                                         |
  |                         O/S dependent                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 continue_file_search()
BeginDeclarations
bit_16                                 rc;
EndDeclarations
BeginCode

 rc = _dos_findnext(&DTA);
 If rc IsNotZero
  Then
   copy_string(current_filename, null_string);
  Else
   far_to_lower(DTA.name, 12);
   copy_string(current_filename, current_path);
   concat_string(current_filename, string(DTA.name));
  EndIf;
 return(rc);
EndCode
