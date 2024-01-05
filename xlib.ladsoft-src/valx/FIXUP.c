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
/*                                 FIXUP.C                                 */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            fixup_FIXUPP_record                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_FIXUPP_record()
BeginDeclarations
byte_ptr                               byte_location;
int_32                                 IP_distance_to_target;
bit_16                                 fbval;
bit_32                                 foval;
bit_32                                 frame_address;
bit_32                                 location_address;
bit_32                                 target_address;
bit_16                                *word_location;
bit_32                                *dword_location;
bit_32                                 fixup32;
public_entry_ptr                       import ;
EndDeclarations
BeginCode
 far_move(Addr(fixup),BytePtr((*fixup_temp).data), sizeof(fixup)) ;
 frame_address  = frame();
 target_address = target(&import);
 location_address = (*(*fixup_temp).lseg).address +
                    Bit_32((*fixup_temp).offset) ;
 byte_location  = 
              Addr((*(*fixup_temp).lseg).data[(*fixup_temp).offset]);

 If import
  Then
   If lxfile.val IsTrue OrIf lefile.val IsTrue
    Then
   	   linker_error(4, "Fixup error:\n"
                   "\t Module:  \"%s\"\n"
                   "\t   File:  \"%s\"\n"
                   "\tSegment:  \"%s\"\n"
                   "\t Offset:  %08X\n"
                   "\t  Error:  Invalid fixup reference to import in LX file.\n",
                   (*(*(*fixup_temp).lseg).tmodule).symbol,
                   (*(*(*fixup_temp).lseg).file).filename,
                (*(*(*(*fixup_temp).lseg).segment).segment_name).symbol,
                   (*fixup_temp).offset);
    EndIf ;
   If pefile.val IsTrue
    Then
     If fixup.mode IsNotZero 
     	OrIf fixup.location_type IsNotEqualTo offset32_location 
             AndIf fixup.location_type IsNotEqualTo secondary_offset32_location
		  Then
#ifdef XXXXX
   	   linker_error(4, "Fixup error:\n"
                   "\t Module:  \"%s\"\n"
                   "\t   File:  \"%s\"\n"
                   "\tSegment:  \"%s\"\n"
                   "\t Offset:  %08X\n"
                   "\t  Error:  Invalid fixup reference to import.\n",
                   (*(*(*fixup_temp).lseg).tmodule).symbol,
                   (*(*(*fixup_temp).lseg).file).filename,
                (*(*(*(*fixup_temp).lseg).segment).segment_name).symbol,
                   (*fixup_temp).offset);
#endif
				// relies on data not being relocated between now and when imports are applied
            enter_pe_import_fixup(byte_location, location_address, import,1) ;
      Else                                 	
				// relies on data not being relocated between now and when imports are applied
            enter_pe_import_fixup(byte_location, location_address, import,0) ;
      EndIf ;
    Else
	   linker_error(4, "Fixup error:\n"
                   "\t Module:  \"%s\"\n"
                   "\t   File:  \"%s\"\n"
                   "\tSegment:  \"%s\"\n"
                   "\t Offset:  %08X\n"
                   "\t  Error:  Import fixup needs a PE file.\n",
                   (*(*(*fixup_temp).lseg).tmodule).symbol,
                   (*(*(*fixup_temp).lseg).file).filename,
                (*(*(*(*fixup_temp).lseg).segment).segment_name).symbol,
                   (*fixup_temp).offset);
    EndIf
	   return ;
  EndIf ;
 If ((target_address LessThan frame_address) OrIf
     (target_address Exceeds (frame_address + 65535L)) AndIf
		 Not use32.val) AndIf
    (fixup.frame_method IsNot 6) AndIf (frame_absolute IsFalse)
  Then
   linker_error(4, "Fixup error:\n"
                   "\t Module:  \"%s\"\n"
                   "\t   File:  \"%s\"\n"
                   "\tSegment:  \"%s\"\n"
                   "\t Offset:  %08X\n"
                   "\t  Error:  Target not within frame.\n",
                   (*(*(*fixup_temp).lseg).tmodule).symbol,
                   (*(*(*fixup_temp).lseg).file).filename,
                (*(*(*(*fixup_temp).lseg).segment).segment_name).symbol,
                   (*fixup_temp).offset);
  EndIf
 
				
 word_location  = (bit_16     *) byte_location;
 dword_location = (bit_32     *) byte_location;
 If fixup.mode IsZero
  Then /* Self-relative fixup */
   Using fixup.location_type
    BeginCase
     When lobyte_location:
      location_address ++ ;
      IP_distance_to_target = Int_32(target_address) -
                              Int_32(location_address);
      If (IP_distance_to_target < -128L) OrIf
         (IP_distance_to_target > 127L)
       Then
        linker_error(4, "Byte self-relative fixup error:\n"
                        "\t Module:  \"%s\"\n"
                        "\t   File:  \"%s\"\n"
                        "\tSegment:  \"%s\"\n"
                        "\t Offset:  %08X\n"
                        "\t  Error:  Distance to target out of range.\n",
                        (*(*(*fixup_temp).lseg).tmodule).symbol,
                        (*(*(*fixup_temp).lseg).file).filename,
                (*(*(*(*fixup_temp).lseg).segment).segment_name).symbol,
                        (*fixup_temp).offset);
       EndIf;
      *byte_location += Bit_8(IP_distance_to_target);
      break;
     When offset_location:
     When secondary_offset_location:
      location_address += 2; 
      IP_distance_to_target = target_address - location_address;
      *word_location += Bit_16(IP_distance_to_target);
      break;
     When base_location:       /* Undefined action */
     When pointer_location:    /* Undefined action */
     When hibyte_location:     /* Undefined action */
	 When pointer32_location:  /* Undefined action */
      break;
     When offset32_location:
     When secondary_offset32_location:
			location_address += 4 ;
      IP_distance_to_target = Int_32(target_address) - Int_32(location_address);
      *dword_location += IP_distance_to_target;
      break;
    EndCase;
  Else /* Segment-relative fixup */
   enter_pe_fixup(location_address,fixup.location_type) ;
   fbval          = Bit_16(frame_address ShiftedRight 4);
   If pefile.val IsTrue OrIf lxfile.val IsTrue OrIf lefile.val IsTrue
      Then
   		foval          = target_address;
    Else
         foval          = target_address - frame_address;
      EndIf ;
   foval = enter_lx_fixup(foval,*dword_location,location_address,fixup.location_type) ;
   If (frame_absolute IsFalse)                     AndIf
      (exefile IsFalse)                            AndIf
      ((fixup.location_type Is base_location)      OrIf
       (fixup.location_type Is pointer_location))
    Then  /* Count the relocation items we should not be getting. */
	 If fixup_temp->lseg->segment->loaded
	  Then
       n_relocation_items++;
	  EndIf
    EndIf;
   Using fixup.location_type
    BeginCase
     When lobyte_location:
      *byte_location += Bit_8(foval);
      break;
     When secondary_offset_location:
     When offset_location:
      *word_location += Bit_16(foval);
      break;
	 When pointer32_location:
	  If pefile.val OrIf lxfile.val OrIf lefile.val
	   Then 
	    *(bit_16 *)(dword_location + 1) = 
				(*(*(*fixup_temp).lseg).segment).pe_section_number;
	   Else
	    *(bit_16 *)(dword_location + 1) = fbval;
	   EndIf
	  /* fall through */
     When secondary_offset32_location:
     When offset32_location:
      *dword_location += foval;
      break;
     When base_location:
      *word_location += fbval;
      If exefile IsTrue
       Then
	 	If fixup_temp->lseg->segment->loaded
	     Then
          Exe_header.relocation_table[Exe_header.n_relocation_items++] =		  
           segment_offset((*fixup_temp).lseg, (*fixup_temp).offset);
		 EndIf
       EndIf;
      break;
     When pointer_location:
      *word_location++ += Bit_16(foval);
      *word_location   += fbval;
      If exefile IsTrue
       Then
	    If fixup_temp->lseg->segment->loaded
	     Then
          Exe_header.relocation_table[Exe_header.n_relocation_items++] =
           segment_offset((*fixup_temp).lseg, (*fixup_temp).offset+2);
		 EndIf
       EndIf;
      break;
     When hibyte_location:
      *byte_location += Bit_8(foval ShiftedRight 8);
      break;
    EndCase;
  EndIf;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           fixup_FORREF_record                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_FORREF_record()
BeginDeclarations
bit_16                                 len;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
bit_32                                 offset;
bit_32                                 value;
bit_8                                  size;
bit_32                                 forref32;
EndDeclarations
BeginCode
 lseg          = (*fixup_temp).lseg;
 len           = (*fixup_temp).rec_len;
 far_move( BytePtr(object_file_element), (*fixup_temp).data, len) ;
 obj_ptr.b8       = object_file_element;
 end_of_record.b8 = Addr(obj_ptr.b8[len]);
 size             = *obj_ptr.b8++;
 forref32 = (*fixup_temp).rec_typ & 1;

 Using size
  BeginCase
   When 0:
    While obj_ptr.b8 IsNot end_of_record.b8
     BeginWhile
      If forref32
       Then
        offset = *obj_ptr.b32++;
        value  = *obj_ptr.b32++;
       Else
        offset = *obj_ptr.b16++;
        value  = *obj_ptr.b16++;
       EndIf
      Lseg.data[offset] += value And 0xff;
			enter_pe_fixup(Lseg.address + offset , lobyte_location) ;
     EndWhile;
    break;
   When 1:
    While obj_ptr.b8 IsNot end_of_record.b8
     BeginWhile
      If forref32
       Then
        offset = *obj_ptr.b32++;
        value  = *obj_ptr.b32++;
       Else
        offset = *obj_ptr.b16++;
        value  = *obj_ptr.b16++;
       EndIf
      *((bit_16     *) Addr(Lseg.data[offset])) += value And 0xffff;
			enter_pe_fixup(Lseg.address + offset , offset_location) ;
     EndWhile;
    break;
   When 2: /* should only be allowed for 32-bit records, but, don't know
            * how to put an error here - DAL
            */
    While obj_ptr.b8 IsNot end_of_record.b8
     BeginWhile
      If forref32
       Then
        offset = *obj_ptr.b32++;
        value  = *obj_ptr.b32++;
       Else
        offset = *obj_ptr.b16++;
        value  = *obj_ptr.b16++;
       EndIf
      *((bit_32     *) Addr(Lseg.data[offset])) += value;
			enter_pe_fixup(Lseg.address + offset , offset32_location) ;
     EndWhile;
    break;
   Otherwise:
    linker_error(4, "Translator error:\n"
                    "\t Module:  \"%s\"\n"
                    "\t   File:  \"%s\"\n"
                    "\tSegment:  \"%s\"\n"
                    "\t  Error:  Invalid FORREF record.\n",
                    (*(*(*fixup_temp).lseg).tmodule).symbol,
                    (*(*(*fixup_temp).lseg).file).filename,
            (*(*(*(*fixup_temp).lseg).segment).segment_name).symbol);
    break;
  EndCase;
 return;
EndCode
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           fixup_LEDATA_record                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_LEDATA_record()
BeginDeclarations
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
EndDeclarations
BeginCode
 lseg          = (*fixup_temp).lseg;
 lseg_data_ptr = Addr(Lseg.data[(*fixup_temp).offset]);
 far_move(lseg_data_ptr, (*fixup_temp).data, (*fixup_temp).rec_len) ;
 return;
EndCode
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            fixup_LIDATA_IDB                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_LIDATA_IDB()
BeginDeclarations
bit_16                                 block_count;
bit_8                                 *content;
bit_16                                 i;
bit_16                                 j;
bit_16                                 len;
bit_16                                 repeat_count;
EndDeclarations
BeginCode
 repeat_count = *obj_ptr.b16++;
 block_count  = *obj_ptr.b16++;
 If block_count IsNotZero
  Then  /* Handle recursive case:  Content is iterated data block */
   content = obj_ptr.b8;
   For i=0; i<repeat_count; i++
    BeginFor
     obj_ptr.b8 = content;
     For j=0; j<block_count; j++
      BeginFor
       fixup_LIDATA_IDB();
      EndFor;
    EndFor;
  Else  /* Handle non-recursive case:  Content is data. */
   len = Bit_16(*obj_ptr.b8++);
   For i=0; i<repeat_count; i++
    BeginFor
     far_move(lseg_data_ptr, obj_ptr.b8, len);
     lseg_data_ptr += len;
    EndFor;
   obj_ptr.b8 += len;
  EndIf;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           fixup_LIDATA_record                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_LIDATA_record()
BeginDeclarations
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
EndDeclarations
BeginCode
 lseg          = (*fixup_temp).lseg;
 lseg_data_ptr = Addr(Lseg.data[(*fixup_temp).offset]);
 far_move(BytePtr(object_file_element),(*fixup_temp).data, (*fixup_temp).rec_len) ;
 obj_ptr.b8       = object_file_element;
 end_of_record.b8 = Addr(obj_ptr.b8[(*fixup_temp).rec_len]);
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   fixup_LIDATA_IDB();
  EndWhile;
 return;
EndCode
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 frame                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 frame()
BeginDeclarations
bit_32                                 frame_address;
group_entry_ptr                        grp;
#define Grp                            (*grp)
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
public_entry_ptr                       pub;
#define Pub                            (*pub)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
EndDeclarations
BeginCode
 Using fixup.frame_method
  BeginCase
   When 0:  /* Frame is segment relative */
    lseg           = (lseg_ptr) fixup.frame_referent;
    seg            = Lseg.segment;
	/*
	If use32.val AndIf Not pefile.val AndIf Not lxfile.val AndIf Not lefile.val AndIf Seg.owning_group
		AndIf (fixup.location_type Is secondary_offset32_location
			OrIf fixup.location_type Is offset32_location)
	 Then
	  grp            = Seg.owning_group;
      seg            = Grp.first_segment;
	 EndIf
	 */
    frame_absolute = Lseg.align Is absolute_segment;
    frame_address  = Seg.address;
    break;
   When 1:  /* Frame is group relative */
    grp            = (group_entry_ptr) fixup.frame_referent;
    seg            = Grp.first_segment;
    If seg IsNull
     Then
      frame_absolute = False; /* hack for FLAT groups (no segs) */
      frame_address = FLAT_address;
     Else
      lseg           = Seg.lsegs.first;
      frame_absolute = Lseg.align Is absolute_segment;
      frame_address  = Seg.address;
     EndIf;
    break;
   When 2:  /* Frame is relative to external */
    pub = (public_entry_ptr) fixup.frame_referent;
    frame_address = public_frame_address(pub);
    break;
   When 3:  /* Frame is absolute */
    frame_absolute = True;
    frame_address  = Bit_32(fixup.frame_referent);
    break;
   When 4:  /* Frame is segment containing location */
    lseg           = (*fixup_temp).lseg;
    seg            = Lseg.segment;
	/*
	If use32.val AndIf Not pefile.val AndIf Not lxfile.val AndIf Not lefile.val AndIf Seg.owning_group
		AndIf (fixup.location_type Is secondary_offset32_location
			OrIf fixup.location_type Is offset32_location)
	 Then
	  grp            = Seg.owning_group;
      seg            = Grp.first_segment;
	 EndIf
	 */
    frame_absolute = Lseg.align Is absolute_segment;
    frame_address  = Seg.address;
    break;
   When 5:  /* Frame is defined by target */
    Using fixup.target_method
     BeginCase
      When 0:  /* Target is segment relative */
       lseg           = (lseg_ptr) fixup.target_referent;
	    seg            = Lseg.segment;
		/*
		If use32.val AndIf Not pefile.val AndIf Not lxfile.val AndIf Not lefile.val AndIf Seg.owning_group
			AndIf (fixup.location_type Is secondary_offset32_location
				OrIf fixup.location_type Is offset32_location)
		 Then
		  grp            = Seg.owning_group;
	      seg            = Grp.first_segment;
		 EndIf
		 */
	   frame_absolute = Lseg.align Is absolute_segment;
	   frame_address  = Seg.address;
       break;
      When 1:  /* Target is group relative */
       grp = (group_entry_ptr) fixup.target_referent;
       seg            = Grp.first_segment;
       lseg           = Seg.lsegs.first;
       frame_absolute = Lseg.align Is absolute_segment;
       frame_address  = Seg.address;
       break;
      When 2:  /* Target is relative to an external */
       pub = (public_entry_ptr) fixup.target_referent;
       frame_address = public_frame_address(pub);
       break;
      When 3:  /* Target is absolute */
       frame_absolute = True;
       frame_address  = Bit_32(fixup.target_referent);
       break;
     EndCase;
    break;
   When 6:  /* No frame */
    frame_absolute = False;
    frame_address = 0L;
    break;
  EndCase;
 return(frame_address & 0xFFFFFFF0L);
EndCode
#undef Grp
#undef Lseg
#undef Pub
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               pass_two                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void pass_two()
BeginDeclarations
group_entry_ptr                        dgroup_entry;
segment_entry_ptr                      seg;
#define Group                          (*dgroup_entry)
#define Seg                            (*seg)
EndDeclarations
BeginCode
 fixup_start_time = Now;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |      First, we will figure out how long the EXE header will be.         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If exefile IsTrue
  Then
   exe_header_size  = Bit_32(sizeof(EXE_header_type)) - 
                      Bit_32(sizeof(bit_32)) + 
                     (Bit_32(sizeof(bit_32)) * Bit_32(n_relocation_items));
   If align_exe_header.val IsTrue
    Then
     exe_header_size += AlignmentGap(exe_header_size, 0xFL);
    Else
     exe_header_size += AlignmentGap(exe_header_size, 0x1FFL);
    EndIf;
   exe_header       = (EXE_header_ptr)
                       allocate_memory(exe_header_size);
   far_set(BytePtr(exe_header), 0, Bit_16(exe_header_size));
  EndIf;
 fixup_temp = fixup_list ;
 While fixup_temp IsNotNull
  BeginWhile
   Using (*fixup_temp).rec_typ And Complement 1
    BeginCase
     When FIXUPP_record:
      fixup_FIXUPP_record();
      break;
     When FORREF_record:
      fixup_FORREF_record();
      break;
     When LEDATA_record:
      fixup_LEDATA_record();
      break;
     When LIDATA_record:
      fixup_LIDATA_record();
      break;
     Otherwise:
      linker_error(16, "Internal logic error:  Invalid temp file record.\n");
      break;
    EndCase;
   fixup_temp = (*fixup_temp).next ;
  EndWhile;
 return;
EndCode
#undef Group
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          public_frame_address                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 public_frame_address(public_entry_ptr pub)
BeginDeclarations
bit_32                                 address;
#define Pub                            (*pub)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
EndDeclarations
BeginCode
 frame_absolute = False;
 If Pub.type_entry IsNot internal
  Then
   seg = (*(*fixup_temp).lseg).segment;
   If Not fixup.external_error_detected And Not Pub.did_external_error
    Then
     linker_error(-1, "Error: Unresolved External \"%s\" in Module \"%s\"\n",
                     unmangle(Pub.symbol),
                     (*(*(*fixup_temp).lseg).tmodule).symbol);
     fixup.external_error_detected = True;
     Pub.did_external_error = True ;
    EndIf;
   address = 0L;
  Else
	 If Pub.type_qualifier IsEqualTo public_import
		Then
			return 0;
		EndIf ;
   If Pub.Internal.group IsNull
    Then
     If Pub.Internal.lseg IsNull
      Then
       frame_absolute = True;
       address        = (Bit_32(Pub.Internal.frame) ShiftedLeft 4);
      Else
       frame_absolute = (*Pub.Internal.lseg).align Is absolute_segment;
	   /*
	   If use32.val AndIf Not pefile.val AndIf Not lxfile.val AndIf Not lefile.val
	   		AndIf (*(*Pub.Internal.lseg).segment).owning_group IsNotNull
				AndIf (fixup.location_type Is secondary_offset32_location
					OrIf fixup.location_type Is offset32_location)
	    Then
         address        = (*(*(*(*Pub.Internal.lseg).segment).owning_group).first_segment).address;
		Else
		*/
         address        = (*(*Pub.Internal.lseg).segment).address;
		//EndIf
      EndIf;
    Else
     If (*Pub.Internal.group).first_segment IsZero
      Then
        /* hack for flat groups */
        address = FLAT_address ;
#ifdef XXXXX
       If Pub.Internal.lseg IsNull
        Then
         address        = (Bit_32(Pub.Internal.frame) ShiftedLeft 4);
        Else
         address        = (*(*Pub.Internal.lseg).segment).address;
        EndIf;
#endif
      Else
       frame_absolute = 
               (*(*(*Pub.Internal.group).first_segment).lsegs.first).align Is
               absolute_segment;
       address        = (*(*Pub.Internal.group).first_segment).address;
      EndIf;
    EndIf;
  EndIf;
 return(address);
EndCode
#undef Pub
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         public_target_address                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 public_target_address(public_entry_ptr pub)
BeginDeclarations
bit_32                                 address;
#define Pub                            (*pub)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
EndDeclarations
BeginCode
 If Pub.type_entry IsNot internal
  Then
   seg = (*(*fixup_temp).lseg).segment;
   If Not fixup.external_error_detected And Not Pub.did_external_error
    Then
     linker_error(-1, "Error: Unresolved External \"%s\" in Module \"%s\"\n",
                     unmangle(Pub.symbol),
                     (*(*(*fixup_temp).lseg).tmodule).symbol);
     Pub.did_external_error = True ;
     fixup.external_error_detected = True;
    EndIf;
   address = 0L;
  Else
	 If Pub.type_qualifier IsEqualTo public_import
		Then
			return 0;
		EndIf ;
   If Pub.Internal.lseg IsNull
    Then
     address = (Bit_32(Pub.Internal.frame) ShiftedLeft 4);
    Else
     address = (*Pub.Internal.lseg).address;
    EndIf;
  EndIf;
 return(address + Bit_32(Pub.Internal.offset));
EndCode
#undef Pub
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              segment_offset                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 segment_offset(lseg_ptr lseg, bit_32 offset)
BeginDeclarations
#define Lseg                           (*lseg)
bit_32                                 totaloffs;
EndDeclarations
BeginCode
 totaloffs = Bit_32(offset) + Target(lseg);
 totaloffs = ((totaloffs And 0xffff0000) ShiftedLeft 12) Or 
                  (totaloffs And 0xffff);
 return ((Frame(lseg) ShiftedLeft 12L) Or totaloffs);
EndCode
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 target                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 target(public_entry_ptr *import)
BeginDeclarations
group_entry_ptr                        grp;
#define Grp                            (*grp)
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_32                                 target_address;
EndDeclarations
BeginCode
 (*import) = NULL ;
 Using fixup.target_method
  BeginCase
   When 0:  /* Target is segment relative */
    lseg = (lseg_ptr) fixup.target_referent;
    target_address = Lseg.address + Bit_32(fixup.target_offset);
    break;
   When 1:  /* Target is group relative */
    grp = (group_entry_ptr) fixup.target_referent;
    target_address = (*Grp.first_segment).address +
                      Bit_32(fixup.target_offset);
    break;
   When 2:  /* Target is relative to an external */
    pub = (public_entry_ptr) fixup.target_referent;
//    Pub.use_count++;
    If Pub.type_qualifier IsEqualTo public_import
      Then
        (*import) = pub ;
      EndIf ;
     target_address = public_target_address(pub) +
                      Bit_32(fixup.target_offset);
    break;
   When 3:  /* Target is absolute */
    target_address = Bit_32(fixup.target_referent) +
                      Bit_32(fixup.target_offset);
    break;
  EndCase;
 return(target_address);
EndCode
#undef Grp
#undef Lseg
#undef Pub

bit_32 import_count(void)
BeginDeclarations
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_32                                 n;
EndDeclarations
BeginCode
 n = 0;
 fixup_temp = fixup_list ;
 While fixup_temp IsNotNull
  BeginWhile
   far_move(Addr(fixup),BytePtr((*fixup_temp).data), sizeof(fixup)) ;
   If  ((*fixup_temp).rec_typ And Complement 1) Is FIXUPP_record
	Then
	 If fixup.target_method Is 2
	  Then
       pub = (public_entry_ptr) fixup.target_referent;
	   If Pub.type_qualifier Is public_import AndIf Not Pub.use_count
	    Then
		 n++;
		EndIf
	   Pub.use_count++;
	  EndIf
	EndIf
   fixup_temp = (*fixup_temp).next ;	
  EndWhile
  return n;
EndCode
#undef Pub
