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
/*                                EXECFILE.C                               */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             make_EXE_header                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void make_EXE_header()
BeginDeclarations
bit_16                                 checksum;
bit_32                                 image_size;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
bit_32                                 n_uninitialized_bytes;
segment_entry_ptr                      seg;
#define Seg                            (*seg)
EndDeclarations
BeginCode
 image_size                      = exe_header_size+highest_uninitialized_byte;
 Exe_header.signature            = 0x5A4D;
 Exe_header.image_length_MOD_512 = Bit_16(image_size Mod 512L);
 Exe_header.image_length_DIV_512 = Bit_16(image_size / 512L);
 If Exe_header.image_length_MOD_512 IsNotZero
  Then
   Exe_header.image_length_DIV_512++;
  EndIf;
 Exe_header.n_header_paragraphs  = Bit_16(exe_header_size ShiftedRight 4L);
 n_uninitialized_bytes           = (*segment_list.last).address + 
                                   (*segment_list.last).length - 
                                   highest_uninitialized_byte;
 Exe_header.min_paragraphs_above = Bit_16((n_uninitialized_bytes + \
                  AlignmentGap(n_uninitialized_bytes, 0xFL)) ShiftedRight 4L);
 Exe_header.max_paragraphs_above = CPARMAXALLOC.val;
 If stack_segment_found IsTrue
  Then
   Exe_header.initial_SS = CanonicFrame(Largest_stack_seg.address);
   Exe_header.initial_SP = Bit_16(largest_stack_seg_length + \
                           Largest_stack_seg.address -        \
                            (Bit_32(Exe_header.initial_SS) ShiftedLeft 4L));
  Else
   Exe_header.initial_SS = 0;
   Exe_header.initial_SP = 0;
  EndIf;
 Exe_header.initial_CS                 = initial_CS;
 Exe_header.initial_IP                 = initial_IP;
 Exe_header.offset_to_relocation_table = 0x1E;
 Exe_header.always_one                 = 1;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |    Run a checksum on all the bytes in the soon-to-exist EXE file.       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If exechecksum.val IsTrue
  Then
   checksum = word_checksum(Bit_16(exe_header_size), 0, BytePtr(exe_header));
   TraverseList(segment_list, seg)
    BeginTraverse
     ExitIf(Seg.address NotLessThan highest_uninitialized_byte);
     TraverseList(Seg.lsegs, lseg)
      BeginTraverse
       ExitIf(Lseg.address NotLessThan highest_uninitialized_byte);
       checksum += word_checksum(Bit_16(Lseg.length), 
                                 Bit_16(Lseg.address),
                                 Lseg.data);
      EndTraverse;
    EndTraverse;
  Else
   checksum = 0xFFFF;
  EndIf;
 Exe_header.checksum = Complement checksum;
 file_write(BytePtr(exe_header), exe_header_size);
 return;
EndCode
#undef Lseg
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         write_symbol_file                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void write_symbol_file()
BeginDeclarations
char                                   buf[256] ;
file_info_ptr                          file;
#define File                           (*file)
group_entry_ptr                        grp;
#define Grp                            (*grp)
bit_16                                 i;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
lseg_ptr                               last_location_lseg;
public_entry_ptr                       pub;
#define Pub                            (*pub)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
bit_32                                 stop_address;
bit_32                                 temp ;
EndDeclarations
BeginCode
   If symbol_table.val IsFalse
    Then
     return ;
    EndIf ;
   file_open_for_write(sym_file_list.first);
   buf[0] = 'S' ;
   buf[1] = 'Y' ;
   buf[2] = '0' ;
   buf[3] = '1' ;
   file_write(BytePtr(buf), Bit_32(4));
     public_sort_array = (public_entry_ptr_array)
                          allocate_memory(
                           Bit_32(sizeof(public_entry_ptr)) *
                            Bit_32(MAX_PUBLICS_IN_LIBRARY));
   n_publics_to_sort = 0;
   TraverseList(external_list, pub)
    BeginTraverse
     LoopIf(Pub.type_entry IsNot internal);
     public_sort_array[n_publics_to_sort++] = pub;
    EndTraverse;
   TraverseList(lib_file_list, file)
    BeginTraverse
     TraverseList(File.external_list, pub)
      BeginTraverse
       LoopIf(Pub.type_entry IsNot internal);
       public_sort_array[n_publics_to_sort++] = pub;
      EndTraverse;
    EndTraverse;
   If n_publics_to_sort Exceeds 0
    Then
     sort_publics_by_value(0, n_publics_to_sort-1);
     For i=0; i LessThan n_publics_to_sort; i++
      BeginFor
       pub = public_sort_array[i];
       buf[0] = 1 ;
       temp = public_target_address(pub)-public_frame_address(pub) ;
       buf[1] = temp ;
       buf[2] = temp >> 8 ;
       buf[3] = 0 ;
       buf[4] = 0 ;
       temp = CanonicFrame(public_frame_address(pub)) ;
       buf[5] = temp ;
       buf[6] = temp >> 8 ;
       buf[7] = strlen(Pub.symbol) ;
       strcpy(buf+8,Pub.symbol) ;
       file_write(BytePtr(buf), 8 + buf[7]) ;
      EndFor;

    EndIf ;

   file_close_for_write() ;
EndCode
#undef File
#undef Grp
#undef Pub
#undef Seg
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         write_executable_image                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void write_executable_image()
BeginDeclarations
byte_ptr                               cvdata ;
bit_32                                 cvdata_len ;
bit_32                                 data_index;
bit_32                                 gap;
bit_32                                 partial_length;
lseg_ptr                               lseg;
public_entry_ptr                       import ;
#define Lseg                           (*lseg)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
EndDeclarations
BeginCode
 exec_image_start_time = Now;

 If lxfile.val IsNotZero OrIf lefile.val IsNotZero
  Then 
   write_lx_image() ;
   return ;
  EndIf ;

 If pefile.val IsNotZero
   Then
     write_pe_image() ;
     return ;
   EndIf ;

/*
 If debug.val IsNotZero
  Then
   create_codeview_data(&cvdata, &cvdata_len) ;
   file_open_for_write(cv_file_list.first);
   file_write(cvdata+32, cvdata_len-32) ;
   file_close_for_write() ;
  EndIf
*/
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Validate start address.                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If start_address_found IsTrue
  Then
   fixup                 = start_address;
   initial_CS            = CanonicFrame(frame());
   initial_IP            = Bit_16(target(&import) - frame());
   If (comfile.val IsTrue)      AndIf 
      (initial_CS IsNotZero)    AndIf 
      (initial_IP IsNot 0x0100)
    Then  /* COM file start address must be 0000:0100 */
      linker_error(4, "Start address for COM file is not 0000:0100.\n");
    Else
     If (sysfile.val IsTrue)   AndIf
        (initial_CS IsNotZero) AndIf 
        (initial_IP IsNotZero)
      Then  /* SYS file start address must be 0000:0000 */
        linker_error(4, "Start address for SYS file is not 0000:0000.\n");
      EndIf;
    EndIf;
  Else  /* No start address found. */
   If sysfile.val IsFalse
    Then
     linker_error(4,"No start address.\n");
	EndIf;
   initial_CS = 0;
   initial_IP = 0;
  EndIf;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                        Validate stack segment.                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If (comfile.val IsTrue) AndIf (stack_segment_found IsTrue)
  Then  /* COM file should not have a stack segment. */
    linker_error(4, "COM file should not have a stack segment.\n");
  Else
   If (sysfile.val IsTrue) AndIf (stack_segment_found IsTrue)
    Then  /* SYS file should not have a stack segment. */
      linker_error(4, "SYS file should not have a stack segment.\n");
    Else
     If (exefile IsTrue) AndIf (stack_segment_found IsFalse)
      Then  /* EXE file should have a stack segment. */
       linker_error(4, "EXE file should have a stack segment.\n");
      EndIf;
    EndIf;
  EndIf;
 
 If pause.val IsTrue
  Then
   printf("About to write \"%s\".\n", (*exe_file_list.first).filename);
   printf("Press [RETURN] key to continue.\n");
   gets(CharPtr(object_file_element));
  EndIf;
 file_open_for_write(exe_file_list.first);
 If exefile IsTrue
  Then
   make_EXE_header();
  Else
   highest_uninitialized_byte = (*segment_list.last).address +
                                (*segment_list.last).length;
  EndIf;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |     Well, we have everything we need to write the executable image.     |
  |                            So let's do it!                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 /* We will use object_file_element as a source for the gaps caused by
    the alignment of segments.  We will fill the gaps with zeros. */
 far_set(BytePtr(object_file_element), 0, MAX_ELEMENT_SIZE);
 next_available_address = address_base;
 TraverseList(segment_list, seg)
  BeginTraverse
   LoopIf((*Seg.lsegs.first).align Is absolute_segment);
   ExitIf(Seg.address NotLessThan highest_uninitialized_byte);
   TraverseList(Seg.lsegs, lseg)
    BeginTraverse
     ExitIf(Lseg.address NotLessThan highest_uninitialized_byte);
     If Lseg.address LessThan next_available_address
      Then
       LoopIf((Lseg.address+Lseg.length) NotGreaterThan \
              next_available_address);
       data_index     = next_available_address - Lseg.address;
       partial_length = Lseg.length - data_index;
       If Seg.combine IsNot blank_common_combine
        Then
         file_write(Addr(Lseg.data[Bit_16(data_index)]), partial_length);
        Else
         write_gap(partial_length);
        EndIf;
      Else
       write_gap(Lseg.address - next_available_address) ;
       next_available_address += Lseg.address - next_available_address ;
       If (Lseg.address + Lseg.length) Exceeds highest_uninitialized_byte
        Then
         partial_length = highest_uninitialized_byte - Lseg.address;
         If Seg.combine IsNot blank_common_combine
          Then
           file_write(Lseg.data, partial_length);
          Else
           write_gap(partial_length);
          EndIf;
        Else
         If Seg.combine IsNot blank_common_combine
          Then
           file_write(Lseg.data, Lseg.length);
          Else
           write_gap(Lseg.length);
          EndIf;
         next_available_address += Lseg.length;
        EndIf;
      EndIf;
    EndTraverse;
  EndTraverse;
 file_close_for_write();
 write_symbol_file() ;
 return;
EndCode
#undef Lseg
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                write_gap                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void write_gap(bit_32 length)
BeginDeclarations
EndDeclarations
BeginCode
 While length Exceeds 0
  BeginWhile
   If length Exceeds Bit_32(MAX_ELEMENT_SIZE)
    Then
     file_write(BytePtr(object_file_element), Bit_32(MAX_ELEMENT_SIZE));
     length -= Bit_32(MAX_ELEMENT_SIZE);
    Else
     file_write(BytePtr(object_file_element), length);
     length = 0L;
    EndIf;
  EndWhile;
 return;
EndCode
