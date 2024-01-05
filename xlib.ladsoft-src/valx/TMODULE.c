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
#include <string.h>
#include <ctype.h>
#include <string.h>
#include <dos.h>
#include <time.h>

#include "langext.h"
#include "defines.h"
#include "types.h"
#include "subs.h"
#include "globals.h"
/*                                 TMODULE.C                               */

/*
Object modules are parsed via recursive descent as defined below:

     obj_t_module::      obj_THEADR obj_seg_grp {obj_component} obj_modtail

     obj_seg_grp::       {obj_LNAMES | obj_SEGDEF | obj_EXTDEF | obj_GRPDEF}
                         {obj_TYPDEF | obj_EXTDEF | obj_GRPDEF}

     obj_component::     obj_data | obj_debug_record

     obj_data::          obj_content_def | obj_thread_def | obj_COMDEF |
                         obj_TYPDEF | obj_PUBDEF | obj_EXTDEF |
                         obj_FORREF | obj_MODPUB | obj_MODEXT


     obj_debug_record::  obj_LINNUM

     obj_content_def::   obj_data_record {obj_FIXUPP}

     obj_thread_def::    obj_FIXUPP  (containing only thread fields)

     obj_data_record::   obj_LIDATA | obj_LEDATA

     obj_modtail::       obj_MODEND
*/

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               need32                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

void Need32(void)
BeginDeclarations
EndDeclarations
BeginCode
  If Current_record_header.rec_typ And 1
   Then
    If Not use32.val
     Then
         linker_error(12, "Command error:\n"
                       "\tModule:  \"%s\"\n"
                       "\t  File:  \"%s\"\n"
                       "\tOffset:  %08lX\n"
                       "\t Error:  32-bit records disabled.\n"
                       "\t         Try again with Use32 switch.\n",
                       (*tmodule_name).symbol,
                       (*infile.file_info).filename,
                       current_record_offset);
		 EndIf
	 EndIf
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_COMDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_COMDEF()
BeginDeclarations
bit_32                                 element_count;
bit_32                                 element_size;
bit_32																 enclosing_segment ;
bit_8                                  element_type;
bit_8                                  expected_type;
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot COMDEF_record
  Then
   return(False);
  EndIf;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   If n_externals NotLessThan max_externals
    Then
		 max_externals += 64 ;
     externals = (public_entry_ptr_array)
                           reallocate_memory(externals,
                                       (Bit_32(max_externals)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));
    EndIf;
   len         = obj_name_length();
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), len);
    EndIf;
   pub         = lookup_public(len, obj_ptr.b8, 0);
   obj_ptr.b8 += len;
   obj_name_length();  /* Eat the type index. */
   externals[++n_externals] = pub;
   element_type  = *obj_ptr.b8++;
   Using element_type
    BeginCase
     When 0x61:
      expected_type = far_communal;
      element_count = obj_leaf_descriptor();
      element_size  = obj_leaf_descriptor();
      break;
     When 0x62:
      expected_type = near_communal;
      element_size  = obj_leaf_descriptor();
      element_count = 1L;
      break;
     Otherwise:
			If element_type Exceeds 0x5F
				Then
      		linker_error(12, "Translator error:\n"
                       "\tModule:  \"%s\"\n"
                       "\t  File:  \"%s\"\n"
                       "\tOffset:  %08lX\n"
                       "\t Error:  Communal type of \"%02X\" is illegal.\n",
                       (*tmodule_name).symbol,
                       (*infile.file_info).filename,
                       current_record_offset,
                       element_type);
				Else
					expected_type = virtual_segment ;
					enclosing_segment = element_type ;
					element_size = obj_leaf_descriptor() ;
				EndIf ;
    EndCase;
   If Pub.type_entry Is unused
    Then
     Insert pub AtEnd InList external_list EndInsert;
     Pub.type_entry             = expected_type;
     Pub.Communal.element_size  = element_size;
     Pub.Communal.element_count = element_count;
     Using element_type
     BeginCase
      When 0x61:
    	 Pub.Communal.element_size  = element_size;
     	 Pub.Communal.element_count = element_count;
       Pub.Communal.next_communal = far_communals;
       far_communals              = pub;
       break;
      When 0x62:
	     Pub.Communal.element_size  = element_size;
  	   Pub.Communal.element_count = element_count;
       Pub.Communal.next_communal = near_communals;
       near_communals             = pub;
       break;
		  Otherwise:
			 Pub.Virtual.element_size   = element_size ;
			 Pub.Virtual.enclosing_segment = snames[enclosing_segment] ;
          Pub.Virtual.lseg = obj_generate_segment(lookup_lname(Pub.length, Pub.symbol),
                               snames[enclosing_segment]->segment->class_name,
                               virtual_combine,
                               5,              /* DWORD aligned */
                               none_lname,
                               0L,
                               0L,             /* not absolute segment */
                               element_size);
			(*Pub.Virtual.lseg).virtualseg = True ;
			 If n_segments NotLessThan max_segments
  			Then
	 			 max_segments += 64 ;
   			 snames = (public_entry_ptr_array)
                           reallocate_memory(snames,
                                       (Bit_32(max_segments)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));
  			EndIf;
			 Pub.Virtual.next_virtual   = virtual_segs ;
 			 snames[Pub.Virtual.seg_index = ++n_segments]  = Pub.Virtual.lseg;
			 virtual_segs 						  = pub ;
     EndCase;
    Else
     If Pub.type_entry Is expected_type
      Then
			 If expected_type Is virtual_segment
				Then
				 If element_size IsNot Pub.Virtual.element_size
					Then
           linker_error(4, "Translator error:\n"
                           "\tModule:  \"%s\"\n"
                           "\t  File:  \"%s\"\n"
                           "\tOffset:  %08lX\n"
                           "\t Error:  virtual segment \"%s\" "
                                       "is a different size..\n",
                           (*tmodule_name).symbol,
                           (*infile.file_info).filename,
                           current_record_offset,
                           Pub.symbol);
					EndIf ;
               snames[Pub.Virtual.seg_index = ++n_segments]  = Pub.Virtual.lseg;
				Else
         If (element_size              * element_count)              Exceeds 
            (Pub.Communal.element_size * Pub.Communal.element_count)
          Then /* We need the largest common */
           Pub.Communal.element_size  = element_size;
           Pub.Communal.element_count = element_count;
          EndIf;
        EndIf ;
      Else
       If (Pub.type_entry Is near_communal) OrIf
          (Pub.type_entry Is far_communal) OrIf 
					(Pub.type_entry Is virtual_segment)
        Then
         linker_error(4, "Translator error:\n"
                         "\tModule:  \"%s\"\n"
                         "\t  File:  \"%s\"\n"
                         "\tOffset:  %08lX\n"
                         "\t Error:  Communal \"%s\" is declared both near "
                                     "and far.\n",
                         (*tmodule_name).symbol,
                         (*infile.file_info).filename,
                         current_record_offset,
                         Pub.symbol);
		 Else
		  If Pub.type_entry Is external
		   Then
	        Pub.type_entry             = expected_type;
	        Pub.Communal.element_size  = element_size;
	        Pub.Communal.element_count = element_count;
	        Using element_type
	         BeginCase
	          When 0x61:
	    	   Pub.Communal.element_size  = element_size;
	     	   Pub.Communal.element_count = element_count;
	           Pub.Communal.next_communal = far_communals;
	           far_communals              = pub;
	           break;
	          When 0x62:
		       Pub.Communal.element_size  = element_size;
	  	       Pub.Communal.element_count = element_count;
	           Pub.Communal.next_communal = near_communals;
	           near_communals             = pub;
	           break;
		      Otherwise:
			   Pub.Virtual.element_size   = element_size ;
			   Pub.Virtual.enclosing_segment = snames[enclosing_segment] ;
               Pub.Virtual.lseg = obj_generate_segment(lookup_lname(Pub.length, Pub.symbol),
                               snames[enclosing_segment]->segment->class_name,
                               virtual_combine,
                               5,              /* DWORD aligned */
                               none_lname,
                               0L,
                               0L,             /* not absolute segment */
                               element_size);
			   (*Pub.Virtual.lseg).virtualseg = True ;
			   If n_segments NotLessThan max_segments
  			    Then
	 		     max_segments += 64 ;
   			     snames = (public_entry_ptr_array)
                           reallocate_memory(snames,
                                       (Bit_32(max_segments)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));
  			    EndIf;
			   Pub.Virtual.next_virtual   = virtual_segs ;
 			   snames[Pub.Virtual.seg_index = ++n_segments]  = Pub.Virtual.lseg;
			   virtual_segs 						  = pub ;
             EndCase;
		   EndIf;
         EndIf;
      EndIf;
    EndIf;
  EndWhile;
 obj_next_record();
 return(True);
EndCode
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                Import                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void Import(void)
BeginDeclarations
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_16                                 ordflag ;
string_ptr                             tstr ;
#define Tstr                           (*tstr)
EndDeclarations
BeginCode
   If n_imports NotLessThan max_imports
    Then
		 max_imports += 64 ;
     imports = (public_entry_ptr_array)
                           reallocate_memory(imports,
                                       (Bit_32(max_imports)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));
    EndIf;
   ordflag = *obj_ptr.b8++ ;
   len         = obj_name_length();
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), len);
    EndIf;
   pub         = lookup_public(len, obj_ptr.b8, 0);
   obj_ptr.b8 += len;
   If Pub.type_entry Is internal AndIf (Pub.type_qualifier Is public_normal OrIf Pub.type_qualifier Is public_import)
    Then
     If Not processing_library
      Then
       linker_error(-1, "Error: Duplicate definition of public \"%s\" in module \"%s\". \n",
                     unmangle(Pub.symbol),
                     (*tmodule_name).symbol);
      EndIf;
     obj_ptr.b8 += obj_name_length() ;
     If ordflag
			Then
       obj_ptr.b16++ ;
      Else
       obj_ptr.b8 += obj_name_length() ;
      EndIf ;
    Else
     If Pub.type_entry IsNot internal
      Then
       len = obj_name_length();
       If case_ignore.val
        Then
         far_to_lower(BytePtr(obj_ptr.b8), len);
        EndIf;
       tstr = allocate_string(len);
       Tstr.length = len ;
       memcpy(Tstr.text,obj_ptr.b8,len) ;
       obj_ptr.b8 += len ;
       Pub.moduleident = tstr ;
       If ordflag
        Then
         Pub.ordinal = *obj_ptr.b16++ ;
         Pub.entryident = NULL ;
		     obj_ptr.b16++;
        Else
         Pub.ordinal = 0 ;
         len = obj_name_length();
				 If len IsNotZero
          Then
           If case_ignore.val
            Then
             far_to_lower(BytePtr(obj_ptr.b8), len);
            EndIf;
           tstr = allocate_string(len);
           Tstr.length = len ;
           memcpy(Tstr.text,obj_ptr.b8,len) ;
           obj_ptr.b8 += len ;
           Pub.entryident = tstr ;
          Else
           Pub.entryident = NULL ;
          EndIf ;  
        EndIf ; 
       If (Pub.type_entry Is public_in_library) AndIf
          (Pub.Library.requested)
        Then
         library_request_count--;
         (*Pub.Library.lib_file).request_count--;
        EndIf;
       Pub.type_entry = internal ;
       Pub.use_count = 0 ;
       Pub.type_qualifier = public_import;
	   Pub.isdata = True; // assume it is a data import
       imports[n_imports++] = pub ;
      Else /* If it already exists as something else, leave it */
	  	 obj_ptr.b8 += obj_name_length() ;
       If ordflag
			  Then
         obj_ptr.b16++ ;
        Else
         obj_ptr.b8 += obj_name_length() ;
        EndIf ;
      EndIf ;
    EndIf ;
EndCode
#undef Tstr
#undef Pub


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                Export                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void Export(void)
BeginDeclarations
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_16                                 ordflag ;
string_ptr                             tstr ;
#define Tstr                           (*tstr)
byte_ptr                               exp_name ;
bit_16                                 exp_len ;
EndDeclarations
BeginCode
   If n_exports NotLessThan max_exports
    Then
		 max_exports += 64 ;
     exports = (public_entry_ptr_array)
                           reallocate_memory(exports,
                                       (Bit_32(max_exports)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));
    EndIf;
   ordflag = *obj_ptr.b8++ ;
   exp_len         = obj_name_length();
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), exp_len);
    EndIf;
   exp_name = obj_ptr.b8 ;
   obj_ptr.b8 += exp_len ;
   len = obj_name_length() ;
	 exp_name[exp_len] = 0 ;
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), exp_len);
    EndIf;
   pub         = lookup_public(exp_len, exp_name, 0);
   obj_ptr.b8 += len;
   If Pub.type_entry Is internal AndIf Pub.type_qualifier Is public_normal 
				OrIf Pub.type_entry Is external OrIf Pub.type_entry Is unused
    Then
     tstr = allocate_string(exp_len);
     Tstr.length = exp_len ;
     memcpy(Tstr.text,exp_name,exp_len) ;
     Pub.entryident = tstr ;
     If ordflag And 0x80
      Then
       Pub.ordinal = *obj_ptr.b16++ ;
      Else 
       Pub.ordinal = 0xffffffff ;
      EndIf ;
     If Pub.type_entry IsNot internal
      Then
       Pub.use_count = 0;
       Insert pub AtEnd InList external_list EndInsert;
      EndIf ;
     If (Pub.type_entry Is public_in_library) AndIf
        (Pub.Library.requested)
      Then
       library_request_count--;
       (*Pub.Library.lib_file).request_count--;
      EndIf;
		 If Pub.type_entry Is unused
			Then
       Pub.type_entry = external ;
       Pub.type_qualifier = public_normal;
      EndIf ;
     exports[n_exports++] = pub ;
    Else 
     linker_error(4, "Attempt to recast a typed public\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  Public is already typed, can't become export\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset) ;
    EndIf ;
EndCode
#undef Tstr
#undef Pub


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_COMENT                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_COMENT()
BeginDeclarations
bit_8                                  comment_class;
bit_8                                  filenum ;
char                                   name[256];
bit_32                                 namelen ;
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot COMENT_record
  Then
   return(False);
  EndIf;
 obj_ptr.b8++;
 comment_class = *obj_ptr.b8++;
 Using comment_class
  BeginCase
   When 158:
    DOSSEG.val = True;
    break;
   When 160:
    Using *obj_ptr.b8++
     BeginCase
      When 1:
       Import() ;
       break ;
      When 2:
       Export() ;
       break ;
      Otherwise:
       break ;
     EndCase ; 
    break ;
   When 161:
    codeview_information_present = True;
    break;
   When 163:
		/* lib mod */
      namelen = *obj_ptr.b8++ ;
      memcpy(name,obj_ptr.b8,namelen) ;
      name[namelen] = 0;
	  add_files_to_list(&lib_file_new_list, string(name));	  
      process_library_directories();
	  break ;
   When 232:
    codeview_information_present = True;
    If debug.val IsNotZero 
     Then
      filenum = *obj_ptr.b8++ ;
      namelen = *obj_ptr.b8++ ;
      memcpy(name,obj_ptr.b8,namelen) ;
      name[namelen] = 0;
      linnum_new_module(filenum, name) ;
     EndIf
    break ;
   Otherwise:
    break;
  EndCase;
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_component                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_component:: obj_data | obj_debug_record */
bit_16 obj_component()
BeginDeclarations
EndDeclarations
BeginCode
 If obj_data() OrIf obj_debug_record() OrIf obj_LNAMES()
  Then
   return(True);
  EndIf;
 return(False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            obj_content_def                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_content_def:: obj_data_record {obj_FIXUPP} */
bit_16 obj_content_def()
BeginDeclarations
EndDeclarations
BeginCode
 If Not obj_data_record()
  Then
   return(False);
  EndIf;
 While obj_FIXUPP()
  BeginWhile
  EndWhile;
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_data                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_data:: obj_content_def |
              obj_thread_def  |
              obj_TYPDEF      |
              obj_PUBDEF      |
              obj_EXTDEF */

bit_16 obj_data()
BeginDeclarations
EndDeclarations
BeginCode
 If obj_content_def() OrIf
    obj_thread_def()  OrIf
    obj_TYPDEF()      OrIf
    obj_PUBDEF()      OrIf
    obj_EXTDEF()      OrIf
    obj_FORREF()      OrIf
    obj_COMDEF()      OrIf
    obj_MODEXT()      OrIf
    obj_MODPUB()
  Then
   return(True);
  EndIf;
 return(False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            obj_data_record                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/*  obj_data_record:: obj_LIDATA | obj_LEDATA */

bit_16 obj_data_record()
BeginDeclarations
EndDeclarations
BeginCode
 If obj_LIDATA() OrIf obj_LEDATA()
  Then
   return(True);
  EndIf;
 return(False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_debug_record                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_debug_record:: obj_LINNUM */

bit_16 obj_debug_record()
BeginDeclarations
EndDeclarations
BeginCode
 If obj_LINNUM()
  Then
   return(True);
  EndIf;
 return(False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_EXTDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_EXTDEF()
BeginDeclarations
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot EXTDEF_record
  Then
   return(False);
  EndIf;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   If n_externals NotLessThan max_externals
    Then
		 max_externals += 64 ;
     externals = (public_entry_ptr_array)
                           reallocate_memory(externals,
                                       (Bit_32(max_externals)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));
    EndIf;
   len         = obj_name_length();
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), len);
    EndIf;
   pub         = lookup_public(len, obj_ptr.b8, 0);
   obj_ptr.b8 += len;
   obj_name_length();  /* Eat the type index. */
   externals[++n_externals] = pub;
   If Pub.type_entry Is unused
    Then
     Insert pub AtEnd InList external_list EndInsert;
     Pub.type_entry = external;
    Else
     If (Pub.type_entry Is public_in_library) AndIf
        (Not Pub.Library.requested)
      Then
       library_request_count++;
       (*Pub.Library.lib_file).request_count++;
       Pub.Library.requested = True;
      EndIf;
    EndIf;
  EndWhile;
 obj_next_record();
 return(True);
EndCode
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_FIXUPP                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_FIXUPP()
BeginDeclarations
EndDeclarations
BeginCode
 If (Current_record_header.rec_typ And Complement 1) IsNot FIXUPP_record
  Then
   return(False);
  EndIf;
 Need32();
 FIXUPP_contains_only_threads = True;
 If Not (*last_LxDATA_lseg).hasFixups
  Then
   While obj_ptr.b8 IsNot end_of_record.b8
    BeginWhile
	 If Not (*last_LxDATA_lseg).hasFixups
	  Then
       If (*obj_ptr.TRD_DAT).type_fixupp_record IsZero
        Then
         obj_FIXUPP_thread();
        Else
         FIXUPP_contains_only_threads = False;
         obj_FIXUPP_fixup();
        EndIf;
	  EndIf;
    EndWhile;
  EndIf ;
 (*last_LxDATA_lseg).hasFixups = (*last_LxDATA_lseg).virtualseg;
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_FIXUPP_fixup                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_FIXUPP_fixup()
BeginDeclarations
FIX_DAT_type                           FIX_DAT;
bit_16                                 frame_method;
LOCAT_type                             LOCAT;
bit_16                                 target_method;
bit_8                                  temp;
bit_16                                 thread_number;
bit_32                                 fixup32;
EndDeclarations
BeginCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  | The LOCAT field in a FIXUPP record has its low and high bytes swapped   |
  | because the high order bit must be 0 for threads and 1 for fixups.      |
  | Since that bit could not be placed in the offset, the bytes were        |
  | swapped instead.                                                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 fixup32               = Current_record_header.rec_typ And 1;
 temp                  = obj_ptr.b8[0];
 obj_ptr.b8[0]         = obj_ptr.b8[1];
 obj_ptr.b8[1]         = temp;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |          Pick up the two required fields (LOCAT and FIX_DAT)            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 LOCAT                 = *obj_ptr.LOCAT;
 obj_ptr.b16++;
 FIX_DAT               = *obj_ptr.FIX_DAT;
 obj_ptr.b8++;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |          A fixup consists of a location, mode, frame and target.        |
  |                         Process the location part.                      |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 fixup_index           = LOCAT.data_record_offset;
 fixup.location_type   = LOCAT.loc;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the mode part.                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 fixup.mode            = LOCAT.m;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the frame part.                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If FIX_DAT.f IsZero
  Then  /* Frame is specified explicitly */
   frame_method         = FIX_DAT.frame;
   fixup.frame_method   = frame_method;
   Using frame_method
    BeginCase
     When 0:
      fixup.frame_referent = (void     *) snames[obj_index_segment()];
      break;
     When 1:
      fixup.frame_referent = (void     *) gnames[obj_index_group()];
      break;
     When 2:
      fixup.frame_referent = (void     *) externals[obj_index_external()];
      break;
     When 3:
      fixup.frame_referent =
       (void     *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
	  break;
	  /* four and five handled elsewhere */
     Otherwise:
      fixup.frame_referent = Null;
      break;
    EndCase;
  Else  /* Frame is specified by a thread */
   thread_number        = FIX_DAT.frame;
   If Not frame_thread[thread_number].thread_defined
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  Reference to frame thread %u which has "
                      "\t         been defined.n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      thread_number);
    EndIf;
   fixup.frame_referent = frame_thread[thread_number].referent;
   fixup.frame_method   = frame_thread[thread_number].method;
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the target part.                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If FIX_DAT.t IsZero
  Then  /* Target is specified explicitly */
   target_method       = FIX_DAT.targt;
   fixup.target_method = target_method;
   Using target_method
    BeginCase
     When 0:  /* Target is the segment referenced by the index */
      fixup.target_referent = (void     *) snames[obj_index_segment()];
      break;
     When 1:  /* Target is the lowest seg in the group referenced 
                 by the index */
      fixup.target_referent = (void     *) gnames[obj_index_group()];
      break;
     When 2:
      fixup.target_referent = (void     *) externals[obj_index_external()];
      break;
     When 3:
      fixup.target_referent =
       (void     *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
      break;
    EndCase;
  Else  /* Target is specified by a thread */
   thread_number         = FIX_DAT.targt;
   If Not target_thread[thread_number].thread_defined
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  Reference to target thread %u which has "
                                  "been defined.n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      thread_number);
    EndIf;
   fixup.target_referent = target_thread[thread_number].referent;
   fixup.target_method   = target_thread[thread_number].method;
  EndIf;

 If FIX_DAT.p IsZero
  Then  /* There is a target displacement */
   If fixup32
    Then
     fixup.target_offset = *obj_ptr.b32++;
    Else
     fixup.target_offset = *obj_ptr.b16++;
    EndIf
  Else  /* The target displacement is zero */
   fixup.target_offset = 0;
  EndIf;

 fixup.external_error_detected = False;

 If fixup.location_type Exceeds secondary_offset_location
  Then
   If  (fixup.location_type Is offset32_location) OrIf
       (fixup.location_type Is secondary_offset32_location) OrIf
	   (fixup.location_type Is pointer32_location)
    Then
     If Not use32.val
      Then
       linker_error(12, "Translator error:\n"
                   "\tModule:  \"%s\"\n"
                   "\t  File:  \"%s\"\n"
                   "\tOffset:  %08lX\n"
                   "\t Error:  32-bit fixups disabled\n",
                   "\t         Try again with Use32 switch.\n",
                   (*tmodule_name).symbol,
                   (*infile.file_info).filename,
                   current_record_offset);
      EndIf;
    Else
       linker_error(12, "Translator error:\n"
                   "\tModule:  \"%s\"\n"
                   "\t  File:  \"%s\"\n"
                   "\tOffset:  %08lX\n"
                   "\t Error:  Unknown fixup type \"%02X\".\n",
                   (*tmodule_name).symbol,
                   (*infile.file_info).filename,
                   current_record_offset,
                   fixup.location_type);
    EndIf;
  EndIf;
 If (fixup.mode IsZero) AndIf
                             ((fixup.location_type Is base_location)    OrIf
                              (fixup.location_type Is pointer_location) OrIf
                              (fixup.location_type Is hibyte_location))
  Then /* Undefined fixup action */
   linker_error(4, "Possible translator error:\n"
                   "\tModule:  \"%s\"\n"
                   "\t  File:  \"%s\"\n"
                   "\tOffset:  %08lX\n"
                   "\t Error:  Base, pointer or hibyte self-relative fixups\n"
                   "\t         are undefined.\n",
                   (*tmodule_name).symbol,
                   (*infile.file_info).filename,
                   current_record_offset);
  EndIf;

 If (last_LxDATA_record_type And Complement 1) Is LEDATA_record
  Then
   If ((fixup.location_type Is base_location)     OrIf
       (fixup.location_type Is pointer_location)) AndIf
      (exefile IsTrue)
    Then /* Base and pointer locations will require a relocation item
            in the EXE header */
     n_relocation_items++;
    EndIf;
   write_fixupp(Current_record_header.rec_typ,
                   last_LxDATA_lseg,
                   last_LxDATA_offset + fixup_index,
                   BytePtr(Addr(fixup)),
                   sizeof(fixup));
  Else
   If fixup.mode IsZero
    Then
     linker_error(4, "Translator warning:\n"
                     "\tModule:  \"%s\"\n"
                     "\t  File:  \"%s\"\n"
                     "\tOffset:  %08lX\n"
                     "\t Error:  Self-relative fixup not permitted for "
                                 "LIDATA.\n",
                     (*tmodule_name).symbol,
                     (*infile.file_info).filename,
                     current_record_offset);
    Else
  		obj_fixup_LIDATA();
    EndIf;
  EndIf;

 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            obj_fixup_LIDATA                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_fixup_LIDATA()
BeginDeclarations
obj_ptr_type                           old_obj_ptr;
EndDeclarations
BeginCode
 LIDATA_index  = 0;
 LIDATA_offset = last_LxDATA_offset;
 old_obj_ptr   = obj_ptr;
 obj_ptr.b8    = Last_LIDATA_record_header.variant_part;
 end_of_last_LIDATA_record.b8 =
  (byte *)
   Addr(Last_LIDATA_record_header.variant_part \
    [Last_LIDATA_record_header.rec_len-1]);
 obj_index_segment();
 obj_ptr.b16++;
 While obj_ptr.b8 IsNot end_of_last_LIDATA_record.b8
  BeginWhile
   obj_fixup_LIDATA_IDB();
  EndWhile;
 obj_ptr = old_obj_ptr;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_fixup_LIDATA_IDB                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_fixup_LIDATA_IDB()
BeginDeclarations
bit_16                                 block_count;
bit_8                                 *content;
bit_16                                 i;
bit_16                                 j;
bit_16                                 len;
bit_16                                 old_index;
bit_16                                 repeat_count;
EndDeclarations
BeginCode
 repeat_count = *obj_ptr.b16++;  LIDATA_index += sizeof(bit_16);
 block_count  = *obj_ptr.b16++;  LIDATA_index += sizeof(bit_16);
 content      = obj_ptr.b8;
 old_index    = LIDATA_index;
 If block_count IsNotZero
  Then  /* Handle recursive case:  Content is iterated data block */
   For i=0; i<repeat_count; i++
    BeginFor
     obj_ptr.b8 = content;
     LIDATA_index = old_index;
     For j=0; j<block_count; j++
      BeginFor
       obj_fixup_LIDATA_IDB();
      EndFor;
    EndFor;
  Else  /* Handle non-recursive case:  Content is data. */
   For i=0; i<repeat_count; i++
    BeginFor
     obj_ptr.b8   = content;
     LIDATA_index = old_index;
     len          = Bit_16(*obj_ptr.b8++);  LIDATA_index += sizeof(bit_8);
     If (fixup_index NotLessThan LIDATA_index)        AndIf
        (fixup_index LessThan   (LIDATA_index + len))
      Then
       write_fixupp(Current_record_header.rec_typ,
                       last_LxDATA_lseg,
                       LIDATA_offset + fixup_index - LIDATA_index,
                       BytePtr(Addr(fixup)),
                       sizeof(fixup));
       If ((fixup.location_type Is base_location)     OrIf
           (fixup.location_type Is pointer_location)) AndIf
          (exefile IsTrue)
        Then /* Base and pointer locations will require a relocation item
                in the EXE header */
         n_relocation_items++;
        EndIf;
      EndIf;
     LIDATA_offset += len;
    EndFor;
   obj_ptr.b8    += len;
   LIDATA_index  += len;
  EndIf;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_FIXUPP_thread                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_FIXUPP_thread()
BeginDeclarations
bit_16                                 method;
bit_16                                 thread;
TRD_DAT_type                           TRD_DAT;
EndDeclarations
BeginCode
 TRD_DAT = *obj_ptr.TRD_DAT;
 obj_ptr.b8++;
 thread  = TRD_DAT.thred;
 method  = TRD_DAT.method;
 If TRD_DAT.d IsZero
  Then  /* This is a target thread */
   target_thread[thread].method = Bit_8(method);
   target_thread[thread].thread_defined = True;
   Using method
    BeginCase
     When 0:
      target_thread[thread].referent =
       (void     *) snames[obj_index_segment()];
      break;
     When 1:
      target_thread[thread].referent =
       (void     *) gnames[obj_index_group()];
      break;
     When 2:
      target_thread[thread].referent =
       (void     *) externals[obj_index_external()];
      break;
     When 3:
      target_thread[thread].referent =
       (void     *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
     Otherwise:
      target_thread[thread].referent = Null;
      break;
    EndCase;
  Else  /* This is a frame thread */
   frame_thread[thread].method = Bit_8(method);
   frame_thread[thread].thread_defined = True;
   Using method
    BeginCase
     When 0:
      frame_thread[thread].referent =
       (void     *) snames[obj_index_segment()];
      break;
     When 1:
      frame_thread[thread].referent =
       (void     *) gnames[obj_index_group()];
      break;
     When 2:
      frame_thread[thread].referent =
       (void     *) externals[obj_index_external()];
      break;
     When 3:
      frame_thread[thread].referent =
       (void     *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
     Otherwise:
      frame_thread[thread].referent = Null;
      break;
    EndCase;
  EndIf;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_FORREF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_FORREF()
BeginDeclarations
bit_16                                 len;
bit_16                                 segment_index;
EndDeclarations
BeginCode
 If (Current_record_header.rec_typ And Complement 1) IsNot FORREF_record
  Then
   return(False);
  EndIf;
 Need32();
 segment_index = obj_index_segment();
 len           = Current_record_header.rec_len - 2;
 If segment_index Exceeds 127
  Then
   len--;
  EndIf;
 write_fixupp(Current_record_header.rec_typ,
                 snames[segment_index],
                 0,
                 BytePtr(obj_ptr.b8),
                 len);
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_generate_segment                          |
  |                                                                         |
  +------------------------------------------------------------------------+*/
lseg_ptr obj_generate_segment(lname_entry_ptr segment_lname,
                              lname_entry_ptr class_lname,
                              combine_type    combine,
                              bit_8           align,
                              lname_entry_ptr tmodule,
                              file_info_ptr   file,
                              bit_32          address,
                              bit_32          length)
BeginDeclarations
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
EndDeclarations
BeginCode
 If combine Is stack_combine
  Then
   length += AlignmentGap(length, 1L); /* Stacks should be an integral
                                          number of words. */
  EndIf;
 seg             = lookup_segment(segment_lname, class_lname, combine);
 If (combine IsNot common_combine AndIf combine IsNot virtual_combine) 
			OrIf (Seg.lsegs.first IsNull)
  Then
   Seg.address   = address;
   Seg.length   += length;
   lseg          = (lseg_ptr) 
                    allocate_memory(Bit_32(sizeof(lseg_type)));
   Lseg.segment  = seg;
   Lseg.tmodule  = tmodule;
   Lseg.file     = file;
   Lseg.address  = address;
   Lseg.length   = length;
   Lseg.align    = align;
   Lseg.fixupp_offset = -1;
   If (combine IsNot common_combine)      AndIf
      (combine IsNot blank_common_combine) AndIf
	  (combine IsNot virtual_combine)
    Then  /* Don't allocate common data yet.  (We will wait until we
             know how long the common block will be.) */
     Lseg.data   = allocate_memory(length);
    EndIf;

   Lseg.highest_uninitialized_byte = 0L;

   Insert lseg AtEnd InList Seg.lsegs EndInsert;

  Else  /* Not the first occurrence of this common */
   lseg = Seg.lsegs.first;
   If length Exceeds Seg.length
    Then  /* Expand common block to be big enough to hold this entry. */
     Seg.length  =
     Lseg.length = length;
    EndIf;
   If align Exceeds Lseg.align
    Then  /* Align to largest boundary. */
     Lseg.align = align;
    EndIf;
  EndIf;
 If Seg.combine Is stack_combine
  Then
   If Not stack_segment_found
    Then
     largest_stack_seg        = seg;
     largest_stack_seg_length = Seg.length;
     stack_segment_found      = True;
    Else
     If Seg.length Exceeds largest_stack_seg_length
      Then
       largest_stack_seg        = seg;
       largest_stack_seg_length = Seg.length;
      EndIf;
    EndIf;
  EndIf;
 return(lseg);
EndCode
#undef Lseg 
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_GRPDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_GRPDEF()
BeginDeclarations
group_entry_ptr                        group;
#define Group                          (*group)
bit_16                                 group_index;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
bit_16                                 segment_index;
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot GRPDEF_record
  Then
   return(False);
  EndIf;
 group_index         = obj_index_LNAME();
 group               = lookup_group(lnames[group_index]);
 If n_groups NotLessThan max_groups
  Then
	 max_groups += 64 ;
   gnames = (public_entry_ptr_array)
                           reallocate_memory(gnames,
                                       (Bit_32(max_groups)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));
  EndIf;
 gnames[++n_groups]  = group;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   If *obj_ptr.b8++ IsNot 0xFF
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  GRPDEF record has a group component "
                                  "descriptor which\n"
                      "\t         does not start with 0xFF.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset);
    EndIf;
   segment_index = obj_index_segment();
   lseg          = snames[segment_index];
   seg           = Lseg.segment;
   If Seg.owning_group IsNull
    Then
     Seg.owning_group = group;
    Else
     If Seg.owning_group IsNot group
      Then
       linker_error(4, "Attempt to place segment \"%s\" into group \"%s\"\n"
                       "\twhen it is already in group \"%s\".\n"
                       "\tRequest to place in group \"%s\" ignored.\n",
                       (*Seg.segment_name).symbol, (*Group.group_name).symbol,
                       (*(*Seg.owning_group).group_name).symbol,
                       (*Group.group_name).symbol);
     EndIf;
    EndIf;
  EndWhile;
 obj_next_record();
 return(True);
EndCode
#undef Seg
#undef Group
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_index_external                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_index_external()
BeginDeclarations
bit_16                                 index;
EndDeclarations
BeginCode
 If *obj_ptr.b8 Is 0xc0
  Then
   index = (Bit_16(*obj_ptr.b8++ And 0x3f) ShiftedLeft 8) ;
   index += Bit_16(*obj_ptr.b8++);
  Else  
   If *obj_ptr.b8 LessThan 128
    Then
     index = Bit_16(*obj_ptr.b8++);
    Else
     index = (Bit_16(*obj_ptr.b8++ - 128) ShiftedLeft 8);
     index += Bit_16(*obj_ptr.b8++);
    EndIf;
  EndIf;
   If index Exceeds n_externals
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  Invalid external index (%u) with only %u "
                                  "externals defined.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      index, n_externals);
    EndIf;
 return(index);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_index_group                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_index_group()
BeginDeclarations
bit_16                                 index;
EndDeclarations
BeginCode
 If *obj_ptr.b8 LessThan 128
  Then
   index = Bit_16(*obj_ptr.b8++);
  Else
   index = (Bit_16(*obj_ptr.b8++ - 128) ShiftedLeft 8);
   index += Bit_16(*obj_ptr.b8++);
  EndIf;
   If index Exceeds n_groups
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  Invalid group index (%u) with only %u "
                                  "groups defined.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      index, n_groups);
    EndIf;
 return(index);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_index_LNAME                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_index_LNAME()
BeginDeclarations
bit_16                                 index;
EndDeclarations
BeginCode
 If *obj_ptr.b8 LessThan 128
  Then
   index = Bit_16(*obj_ptr.b8++);
  Else
   index = (Bit_16(*obj_ptr.b8++ - 128) ShiftedLeft 8);
   index += Bit_16(*obj_ptr.b8++);
  EndIf;
   If index Exceeds n_lnames
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  Invalid LNAME index (%u) with only %u "
                                  "LNAMEs defined.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      index, n_lnames);
    EndIf;
 return(index);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_index_segment                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_index_segment()
BeginDeclarations
bit_16                                 index;
bit_16                                 virtual = 0 ;
public_entry_ptr											 pub ;
#define Pub														(*pub)
EndDeclarations
BeginCode
 If *obj_ptr.b8 Is 0xc0
  Then
   index = (Bit_16(*obj_ptr.b8++ And 0x3f) ShiftedLeft 8);
   index += Bit_16(*obj_ptr.b8++);
   virtual = 1 ;
  Else  
   If *obj_ptr.b8 LessThan 128
    Then
     index = Bit_16(*obj_ptr.b8++);
    Else
     index = (Bit_16(*obj_ptr.b8++ And 0x7f) ShiftedLeft 8);
     index += Bit_16(*obj_ptr.b8++);
    EndIf;
  EndIf ;
 If virtual IsTrue
  Then
    If index Exceeds n_externals
     Then
      linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  Invalid virtual segment index (%u) with only %u "
                                  "externals defined.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      index, n_externals);
     Else
      pub = externals[index] ;
      index = Pub.Virtual.seg_index ;
     EndIf ;
  EndIf ;
   If index Exceeds n_segments
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  Invalid segment index (%u) with only %u "
                                  "segments defined.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      index, n_segments);
    EndIf;
 return(index);
EndCode
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         obj_iterated_data_block                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_iterated_data_block(bit_32 use32)
BeginDeclarations
bit_16                                 block_count;
bit_8                                 *content;
bit_16                                 i;
bit_16                                 j;
bit_16                                 len;
bit_16                                 repeat_count;
EndDeclarations
BeginCode
 If use32
  Then
/* fixme - fixupps will not work with this.  We are only using for BSS tho */
   repeat_count = *obj_ptr.b32++;
  Else
   repeat_count = *obj_ptr.b16++;
  EndIf ;
 block_count  = *obj_ptr.b16++;
 If block_count IsNotZero
  Then  /* Handle recursive case:  Content is iterated data block */
   content = obj_ptr.b8;
   For i=0; i<repeat_count; i++
    BeginFor
     obj_ptr.b8 = content;
     For j=0; j<block_count; j++
      BeginFor
       obj_iterated_data_block(use32);
      EndFor;
    EndFor;
  Else  /* Handle non-recursive case:  Content is data. */
   len = Bit_16(*obj_ptr.b8++);
   For i=0; i<repeat_count; i++
    BeginFor
     far_move(Addr(last_LxDATA_Lseg.data[LIDATA_offset]), 
              obj_ptr.b8, len);
     LIDATA_offset += len;
    EndFor;
   obj_ptr.b8 += len;
  EndIf;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                     obj_iterated_data_block_length                      |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 obj_iterated_data_block_length(bit_32 use32)
BeginDeclarations
bit_16                                 block_count;
bit_16                                 i;
bit_16                                 len;
bit_32                                 length;
bit_32                                 repeat_count;
EndDeclarations
BeginCode
 If use32
  Then
/* fixme - fixupps will not work with this.  We are only using for BSS tho */
   repeat_count = *obj_ptr.b32++;
  Else
   repeat_count = *obj_ptr.b16++;
  EndIf ;
 block_count  = *obj_ptr.b16++;
 If repeat_count IsZero
  Then /* This is a translator error. */
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%s\"\n"
                    "\t  File:  \"%s\"\n"
                    "\tOffset:  %08lX\n"
                    "\t Error:  Repeat count in LIDATA iterated data block "
                                "is zero.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  EndIf;
 length       = 0L;
 If block_count IsNotZero
  Then  /* Handle recursive case:  Content is iterated data block */
   For i=0; i<block_count; i++
    BeginFor
     length     += Bit_32(repeat_count) * obj_iterated_data_block_length(use32);
    EndFor;
  Else  /* Handle non-recursive case:  Content is data. */
   len         = Bit_16(*obj_ptr.b8++);
   obj_ptr.b8 += len;
   length      = Bit_32(repeat_count) * Bit_32(len);
  EndIf;
 return(length);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_leaf_descriptor                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 obj_leaf_descriptor()
BeginDeclarations
bit_8                                  element_size;
EndDeclarations
BeginCode
 element_size = *obj_ptr.b8++;
 If element_size LessThan 129
  Then
   return(Bit_32(element_size));
  Else
   If element_size Is 129
    Then
     return(Bit_32(*obj_ptr.b16++));
    Else
     If element_size Is 132
      Then
       obj_ptr.b8--;
       return((*obj_ptr.b32++) And 0x00FFFFFFL);
      Else
       If element_size Is 136
        Then
         return(*obj_ptr.b32++);
        Else
         linker_error(12, "Translator error:\n"
                        "\tModule:  \"%s\"\n"
                        "\t  File:  \"%s\"\n"
                        "\tOffset:  %08lX\n"
                        "\t Error:  Communal element size of %u is illegal.\n",
                        (*tmodule_name).symbol,
                        (*infile.file_info).filename,
                        current_record_offset,
                        element_size);
        EndIf;
      EndIf;
    EndIf;
  EndIf;
 return(0L);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_LEDATA                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_LEDATA()
BeginDeclarations
bit_32                                 next_byte;
bit_16                                 len;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
bit_32                                 offset;
bit_16                                 segment_index;
bit_32                                 ledata32;
bit_8                                  *oop;
EndDeclarations
BeginCode
 If (Current_record_header.rec_typ And Complement 1) IsNot LEDATA_record
  Then
   return(False);
  EndIf;
 Need32();
 ledata32 = Current_record_header.rec_typ And 1;

 last_LxDATA_record_type = Current_record_header.rec_typ;
 len                     = Current_record_header.rec_len - 1;
 oop                     = obj_ptr.b8;
 segment_index           = obj_index_segment();
 last_LxDATA_lseg        =
 lseg                    = snames[segment_index];
 If ledata32
  Then
   last_LxDATA_offset =
   offset             = *obj_ptr.b32++;
  Else
   last_LxDATA_offset =
   offset             = *obj_ptr.b16++;
  EndIf
 // for virtual segs, only gather the data and fixups once...
 // borland has mismatched virtual segs sometimes...
 //
 If Lseg.virtualseg AndIf (int_32)offset LessThanOrEqualTo Lseg.fixupp_offset
  Then
   Lseg.hasFixups = True;
   obj_next_record();
   Return(True);
  EndIf;
 Lseg.hasFixups = False;
 Lseg.fixupp_offset = offset;
 len -= (obj_ptr.b8 - oop) ;
 next_byte          = Bit_32(offset) + Bit_32(len);
 If next_byte Exceeds Lseg.length
  Then
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%s\"\n"
                    "\t  File:  \"%s\"\n"
                    "\tOffset:  %08lX\n"
                    "\t Error:  Attempt to initialize past end of LSEG.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  EndIf;
 If next_byte Exceeds Lseg.highest_uninitialized_byte
  Then
   Lseg.highest_uninitialized_byte = next_byte;
  EndIf;
 If (*last_LxDATA_Lseg.segment).combine IsNot common_combine 
	AndIf (*last_LxDATA_Lseg.segment).combine IsNot virtual_combine 
  Then
   far_move(Addr(Lseg.data[offset]), obj_ptr.b8, len);
  Else  /* We must save the initialization data out to the tmp file until
      later when we know the length. */
   write_fixupp(Current_record_header.rec_typ,
                 last_LxDATA_lseg,
                 last_LxDATA_offset,
                 BytePtr(obj_ptr.b8),
                 len);
  EndIf;
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_LIDATA                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_LIDATA()
BeginDeclarations
bit_16                                 len;
bit_32                                 LIDATA_length;
bit_32                                 next_byte;
bit_16                                 segment_index;
bit_32                                 lidata32;
EndDeclarations
BeginCode
 If (Current_record_header.rec_typ And Complement 1) IsNot LIDATA_record
  Then
   return(False);
  EndIf;
 Need32();
 lidata32 = Current_record_header.rec_typ And 1;
 far_move(BytePtr(last_LIDATA_record), 
          BytePtr(object_file_element),
          Current_record_header.rec_len + sizeof(obj_record_header_type) - 1);
 last_LxDATA_record_type = Current_record_header.rec_typ;
 segment_index           = obj_index_segment();
 last_LxDATA_lseg        = snames[segment_index];
 If lidata32
  Then
   LIDATA_offset           =
   last_LxDATA_offset      = *obj_ptr.b32++;
  Else
   LIDATA_offset           =
   last_LxDATA_offset      = *obj_ptr.b16++;
  EndIf
  
 LIDATA_length           = obj_LIDATA_length(lidata32);
 next_byte               = last_LxDATA_offset + LIDATA_length;
 If next_byte Exceeds last_LxDATA_Lseg.length
  Then
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%s\"\n"
                    "\t  File:  \"%s\"\n"
                    "\tOffset:  %08lX\n"
                    "\t Error:  Attempt to initialize past end of LSEG.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  EndIf;
 If next_byte Exceeds last_LxDATA_Lseg.highest_uninitialized_byte
  Then
   last_LxDATA_Lseg.highest_uninitialized_byte = next_byte;
  EndIf;
 If (*last_LxDATA_Lseg.segment).combine IsNot common_combine AndIf
	(*last_LxDATA_Lseg.segment).combine IsNot virtual_combine
  Then
   While obj_ptr.b8 IsNot end_of_record.b8
   BeginWhile
   obj_iterated_data_block(lidata32);
   EndWhile;
  Else  /* We must save the initialization data out to the tmp file until
           later when we know the length. */
   len                     = Current_record_header.rec_len - 4;
   If segment_index Exceeds 127
    Then
     len--;
    EndIf;
   write_fixupp(Current_record_header.rec_typ,
                   last_LxDATA_lseg,
                   last_LxDATA_offset,
                   BytePtr(obj_ptr.b8),
                   len);
  EndIf;
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_LIDATA_length                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 obj_LIDATA_length(bit_32 use32)
BeginDeclarations
bit_32                                 length;
bit_8                                  *start;
EndDeclarations
BeginCode
 start  = obj_ptr.b8;
 length = 0L;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   length += obj_iterated_data_block_length(use32);
  EndWhile;
 obj_ptr.b8 = start;
 return(length);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_LINNUM                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_LINNUM()
BeginDeclarations
EndDeclarations
BeginCode
 If (Current_record_header.rec_typ And Complement 1) IsNot LINNUM_record
  Then
   return(False);
  EndIf;
 Need32();
 If debug.val IsNotZero
  Then
   linnum_new_data(obj_ptr.b8+2, Current_record_header.rec_len-3) ;
  EndIf
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_LNAMES                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_LNAMES()
BeginDeclarations
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot LNAMES_record AndIf
 	Current_record_header.rec_typ IsNot LLNAMES_record
  Then
   return(False);
  EndIf;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   If n_lnames NotLessThan max_lnames
    Then
	   max_lnames += 64 ;
     lnames = (public_entry_ptr_array)
                           reallocate_memory(lnames,
                                       (Bit_32(max_lnames)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));
    EndIf;
   lnames[++n_lnames] = obj_name();
  EndWhile;
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_MODEND                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_MODEND()
BeginDeclarations
FIX_DAT_type                           END_DAT;
bit_16                                 frame_method;
MOD_TYP_type                           MOD_TYP;
bit_16                                 target_method;
bit_16                                 thread_number;
EndDeclarations
BeginCode
 If (Current_record_header.rec_typ And Complement 1) IsNot MODEND_record
  Then
   return(False);
  EndIf;
 Need32();
 MOD_TYP = *obj_ptr.MOD_TYP;
 obj_ptr.b8++;

 If MOD_TYP.zeros IsNotZero
  Then
   linker_error(4, "Translator error:\n"
                   "\tModule:  \"%s\"\n"
                   "\t  File:  \"%s\"\n"
                   "\tOffset:  %08lX\n"
                   "\t Error:  Bits 1 thru 5 of MOD TYP must be zero.\n",
                   (*tmodule_name).symbol,
                   (*infile.file_info).filename,
                   current_record_offset);
  EndIf;

 If (MOD_TYP.mattr IsNot 1) AndIf (MOD_TYP.mattr IsNot 3)
  Then  /* We have no starting address */
   return(True);
  EndIf;

 If MOD_TYP.l IsNot 1
  Then
   linker_error(4, "Translator error:\n"
                   "\tModule:  \"%s\"\n"
                   "\t  File:  \"%s\"\n"
                   "\tOffset:  %08lX\n"
                   "\t Error:  Bit 0 of MOD TYP must be one.\n",
                   (*tmodule_name).symbol,
                   (*infile.file_info).filename,
                   current_record_offset);
  EndIf;

 If start_address_found IsTrue
  Then
   linker_error(4, "Multiple start address encountered.  The start address\n"
                   "in module \"%s\" of file \"%s\" has been ignored.\n",
                   (*tmodule_name).symbol,
                   (*infile.file_info).filename);
   return;
  EndIf;

 start_address_found = True;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                  Pick up the required field END_DAT.                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 END_DAT = *obj_ptr.FIX_DAT;
 obj_ptr.b8++;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the frame part.                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If END_DAT.f IsZero
  Then  /* Frame is specified explicitly */
   frame_method                 = END_DAT.frame;
   start_address.frame_method   = frame_method;
   Using frame_method
    BeginCase
     When 0:
      start_address.frame_referent =
       (void     *) snames[obj_index_segment()];
      break;
     When 1:
      start_address.frame_referent =
       (void     *) gnames[obj_index_group()];
      break;
     When 2:
      start_address.frame_referent =
       (void     *) externals[obj_index_external()];
      break;
     When 3:
      start_address.frame_referent =
       (void     *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
     Otherwise:
      start_address.frame_referent = Null;
      break;
    EndCase;
  Else  /* Frame is specified by a thread */
   thread_number                = END_DAT.frame;
   If Not frame_thread[thread_number].thread_defined
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  Reference to frame thread %u which has "
                                  "been defined.n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      thread_number);
    EndIf;
   start_address.frame_referent = frame_thread[thread_number].referent;
   start_address.frame_method   = frame_thread[thread_number].method;
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the target part.                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If END_DAT.t IsZero
  Then  /* Target is specified explicitly */
   target_method               = END_DAT.targt;
   start_address.target_method = target_method;
   Using target_method
    BeginCase
     When 0:
      start_address.target_referent =
       (void     *) snames[obj_index_segment()];
      break;
     When 1:
      start_address.target_referent =
       (void     *) gnames[obj_index_group()];
      break;
     When 2:
      start_address.target_referent =
       (void     *) externals[obj_index_external()];
      break;
     When 3:
      start_address.target_referent =
       (void     *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
      break;
    EndCase;
  Else  /* Target is specified by a thread */
   thread_number                 = END_DAT.targt;
   If Not target_thread[thread_number].thread_defined
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  Reference to target thread %u which has "
                                  "been defined.n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      thread_number);
    EndIf;
   start_address.target_referent = target_thread[thread_number].referent;
   start_address.target_method   = target_thread[thread_number].method;
  EndIf;

 If END_DAT.p IsZero
  Then  /* There is a target displacement */
   If Current_record_header.rec_typ And 1
    Then
     start_address.target_offset = *obj_ptr.b32++;
    Else
     start_address.target_offset = *obj_ptr.b16++;
    EndIf
  Else  /* The target displacement is zero */
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%s\"\n"
                    "\t  File:  \"%s\"\n"
                    "\tOffset:  %08lX\n"
                    "\t Error:  Only primary fixups allowed in MODEND.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
   start_address.target_offset = 0;
  EndIf;
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_MODEXT                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_MODEXT()
BeginDeclarations
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)
EndDeclarations
BeginCode
 If (Current_record_header.rec_typ And Complement 1) IsNot MODEXT_record
  Then
   return(False);
  EndIf;
 Need32();
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   If n_externals NotLessThan max_externals
    Then
	   max_externals += 64 ;
     externals = (public_entry_ptr_array)
                           reallocate_memory(externals,
                                       (Bit_32(max_externals)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));
    EndIf;
   len         = obj_name_length();
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), len);
    EndIf;
   pub         = lookup_public(len, obj_ptr.b8, tmodule_number);
   obj_ptr.b8 += len;
   obj_name_length();  /* Eat the type index. */
   externals[++n_externals] = pub;
   If Pub.type_entry Is unused
    Then
     Insert pub AtEnd InList external_list EndInsert;
     Pub.type_entry = external;
    EndIf;
  EndWhile;
 obj_next_record();
 return(True);
EndCode
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_MODPUB                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_MODPUB()
BeginDeclarations
bit_16                                 group_index;
bit_16                                 frame;
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_16                                 segment_index;
bit_32                                 modpub32;
EndDeclarations
BeginCode
 If (Current_record_header.rec_typ And Complement 1) IsNot MODPUB_record
  Then
   return(False);
  EndIf;
 Need32();
 modpub32 = Current_record_header.rec_typ And 1;

 group_index = obj_index_group();
 segment_index = obj_index_segment();
 If (segment_index IsZero) AndIf (group_index IsZero)
  Then
   frame = *obj_ptr.b16++;
  EndIf;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   len = obj_name_length();
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), len);
    EndIf;
   pub = lookup_public(len, obj_ptr.b8, tmodule_number);
   obj_ptr.b8 += len;
   If Pub.type_entry Is internal
    Then
     If Not processing_library
      Then
       linker_error(-1, "Error: Duplicate definition of public \"%s\" in module \"%s\". \n",
                     unmangle(Pub.symbol),
                     (*tmodule_name).symbol);
      EndIf;
     If modpub32
      Then
       obj_ptr.b32++;      /* Eat offset. */
      Else
       obj_ptr.b16++;      /* Eat offset. */
      EndIf
     obj_name_length();  /* Eat type index. */
    Else
     If Pub.type_entry Is unused
      Then
       Insert pub AtEnd InList external_list EndInsert;
      EndIf;
     Pub.type_entry       = internal;
     Pub.Internal.group   = gnames[group_index];
     Pub.Internal.lseg    = snames[segment_index];
     Pub.Internal.frame   = frame;
     If modpub32
      Then
       Pub.Internal.offset  = *obj_ptr.b32++;
      Else
       Pub.Internal.offset  = *obj_ptr.b16++;
      EndIf
     obj_name_length();  /* Eat type index. */
    EndIf;
  EndWhile;
 obj_next_record();
 return(True);
EndCode
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              obj_modtail                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_modtail:: obj_MODEND */

bit_16 obj_modtail()
BeginDeclarations
EndDeclarations
BeginCode
 If obj_MODEND()
  Then
   return(True);
  EndIf;
 return(False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_name                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
lname_entry_ptr obj_name()
BeginDeclarations
lname_entry_ptr                        name;
bit_16                                 len;
EndDeclarations
BeginCode
 len = obj_name_length();
 If len IsZero
  Then
   name = none_lname;
  Else
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), len);
    EndIf;
   name        = lookup_lname(len, obj_ptr.b8);
   obj_ptr.b8 += len;
  EndIf;
 return(name);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            obj_name_length                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_name_length()
BeginDeclarations
bit_16 								index;
EndDeclarations
BeginCode
 If *obj_ptr.b8 LessThan 128
  Then
   return(Bit_16(*obj_ptr.b8++));
  Else
   index = (Bit_16(*obj_ptr.b8++ - 128) ShiftedLeft 8);
   index += Bit_16(*obj_ptr.b8++);
   return index;
  EndIf;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_next_record                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_next_record()
BeginDeclarations
EndDeclarations
BeginCode
 Repeat
  BeginRepeat
   file_read(object_file_element,  sizeof(obj_record_header_type) - 1);
   While (Current_record_header.rec_typ Is LINNUM_record) OrIf
         ((Current_record_header.rec_typ Is COMENT_record) AndIf
          (Current_record_header.rec_len Exceeds MAX_OBJECT_FILE_READ_SIZE))
    BeginWhile
     file_position(Bit_32(infile.byte_position) +
                   infile.start_of_buffer_position +
                   Bit_32(Current_record_header.rec_len));
     file_read(object_file_element,  sizeof(obj_record_header_type) - 1);
    EndWhile;
   current_record_offset = Bit_32(infile.byte_position) +
                           infile.start_of_buffer_position -
                           Bit_32(sizeof(obj_record_header_type)-1);
   If Current_record_header.rec_len Exceeds MAX_OBJECT_FILE_READ_SIZE
    Then
     linker_error(12, "Probable invalid OBJ format "
                      "or possible translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  Record too long.\n"
                      "\t         Max record length supported by this "
                                 "linker is %u bytes.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      MAX_OBJECT_FILE_READ_SIZE);
    EndIf;
   file_read(Current_record_header.variant_part, 
             Current_record_header.rec_len);
   If (objchecksum.val IsTrue) AndIf
      (Bit_8(checksum(Current_record_header.rec_len + \
                      sizeof(obj_record_header_type)-1, \
                     (byte *) current_record_header)) IsNotZero)
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  Checksum error.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset);
    EndIf;
   obj_ptr.b8 = Current_record_header.variant_part;
   end_of_record.b8 =
    (byte *)
     Addr(Current_record_header.variant_part[Current_record_header.rec_len-1]);
   RepeatIf obj_COMENT()
  EndRepeat;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_PUBDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_PUBDEF()
BeginDeclarations
bit_16                                 group_index;
bit_16                                 frame;
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_16                                 segment_index;
bit_16                                 pub32;
EndDeclarations
BeginCode
 If (Current_record_header.rec_typ And Complement 1) IsNot PUBDEF_record
  Then
   return(False);
  EndIf;
 Need32();
 pub32 = Current_record_header.rec_typ And 1;

 group_index = obj_index_group();
 segment_index = obj_index_segment();
 If (segment_index IsZero) AndIf (group_index IsZero)
  Then
   frame = *obj_ptr.b16++;
  EndIf;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   len = obj_name_length();
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), len);
    EndIf;
   pub = lookup_public(len, obj_ptr.b8, 0);
   obj_ptr.b8 += len;
   If Pub.type_entry Is internal AndIf (Pub.type_qualifier Is public_normal OrIf Pub.type_qualifier Is public_import)
    Then
     If Not processing_library
      Then
       linker_error(-1, "Error: Duplicate definition of public \"%s\" in module \"%s\". \n",
                     unmangle(Pub.symbol),
                     (*tmodule_name).symbol);
      EndIf;
     If pub32
      Then
       obj_ptr.b32++;      /* Eat offset. */
      Else
       obj_ptr.b16++;      /* Eat offset. */
      EndIf
     obj_name_length();  /* Eat type index. */
    Else
     If Pub.type_entry Is unused
      Then
       Pub.use_count = 0;
       Insert pub AtEnd InList external_list EndInsert;
       Insert &Pub.modpubs AtEnd InList module_publics EndInsert;
      Else
       If Pub.type_entry Is external Or Pub.type_entry Is public_in_library
        Then
         Insert &Pub.modpubs AtEnd InList module_publics EndInsert;
        EndIf ;
      EndIf;
     If (Pub.type_entry Is public_in_library) AndIf
        (Pub.Library.requested)
      Then
       library_request_count--;
       (*Pub.Library.lib_file).request_count--;
      EndIf;
     Pub.type_entry       = internal;
     Pub.type_qualifier   = public_normal ;
     Pub.Internal.group   = gnames[group_index];
     Pub.Internal.lseg    = snames[segment_index];
     Pub.Internal.frame   = frame;
     Pub.Internal.segment_index = segment_index;
     If pub32
      Then
       Pub.Internal.offset  = *obj_ptr.b32++;
      Else
       Pub.Internal.offset  = *obj_ptr.b16++;
      EndIf
     obj_name_length();  /* Eat type index. */
    EndIf;
  EndWhile;
 obj_next_record();
 return(True);
EndCode
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_SEGDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_SEGDEF()
BeginDeclarations
acbp_type                              acbp;
bit_32                                 address;
bit_8                                  align;
lname_entry_ptr                        class_lname;
bit_8                                  combine;
bit_32                                 length;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
bit_16                                 segment_index;
lname_entry_ptr                        segment_lname;
bit_32                                 seg32;
EndDeclarations
BeginCode
 If (Current_record_header.rec_typ And Complement 1 )IsNot SEGDEF_record
  Then
   return(False);
  EndIf;
 Need32();
 seg32 = Current_record_header.rec_typ And 1;


 acbp    = *obj_ptr.acbp;
 obj_ptr.b8++;
 If (pefile.val IsTrue OrIf lxfile.val IsTrue) AndIf acbp.p IsZero
  Then
       linker_error(12, "Command Error:\n"
                     "\tModule:  \"%s\"\n"
                     "\t  File:  \"%s\"\n"
                     "\tOffset:  %08lX\n"
                     "\t Error:  16-bit segments unsupported\n"
                     "\t         For LX and PE images.\n",
                     (*tmodule_name).symbol,
                     (*infile.file_info).filename,
                     current_record_offset);
 EndIf
 align   = Bit_8(acbp.a);
 If align Is absolute_segment
  Then
   If seg32
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  32-bit absolute segments not supported.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset);
    EndIf
   address  = (Bit_32(*obj_ptr.b16++) ShiftedLeft 4L);  /* Frame */
   address += Bit_32(*obj_ptr.b8++);                    /* Offset */
  Else
   address = 0L;
  EndIf;
 If align Exceeds fourk_aligned
  Then
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%s\"\n"
                    "\t  File:  \"%s\"\n"
                    "\tOffset:  %08lX\n"
                    "\t Error:  Align type of %u is undefined.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset,
                    align);
  EndIf;
 combine = Bit_8(acbp.c);
 If (combine Is 4) OrIf (combine Is 7)
  Then /* Treat combine types 4 and 7 the same as 2. */
   combine = public_combine;
  EndIf;
 If (combine Is 1) OrIf (combine Is 3)
  Then /* This is a translator error. */
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%s\"\n"
                    "\t  File:  \"%s\"\n"
                    "\tOffset:  %08lX\n"
                    "\t Error:  Combine type of %u is undefined.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset,
                    combine);
  EndIf;
 If seg32
  Then
   length = *obj_ptr.b32++;
  Else
   length = Bit_32(*obj_ptr.b16++);
  EndIf
 If acbp.b IsNotZero
  Then
   If length IsNotZero
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  SEGDEF has acbp.b of 1 and length not "
                                  "zero.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset);
    EndIf;
   If seg32
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  32-bit 4 GB segments not supported\n.",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset);
    EndIf
    length = 65536L;
  EndIf;
 segment_index         = obj_index_LNAME();
 segment_lname         = lnames[segment_index];
 class_lname           = lnames[obj_index_LNAME()];
 lseg = obj_generate_segment(segment_lname,
                             class_lname,
                             combine,
                             align,
                             tmodule_name,
                             infile.file_info,
                             address,
                             length);
 If n_segments NotLessThan max_segments
  Then
	 max_segments += 64 ;
   snames = (public_entry_ptr_array)
                           reallocate_memory(snames,
                                       (Bit_32(max_segments)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));
  EndIf;
 snames[++n_segments]  = lseg;

 If Not stricmp(class_lname->symbol,"CODE")
  Then
   cseg_lseg = lseg ;
  EndIf
 If Not stricmp(class_lname->symbol,"DATA")
  Then
   dseg_lseg = lseg ;
  EndIf
 If Not stricmp(class_lname->symbol,"BSS")
  Then
   bseg_lseg = lseg ;
  EndIf
 If Not stricmp(class_lname->symbol,"CONST")
  Then
   constseg_lseg = lseg ;
  EndIf
 If Not stricmp(class_lname->symbol,"STRING")
  Then
   stringseg_lseg = lseg ;
  EndIf
 If class_lname Is codeview_class_DEBSYM
  Then
   cvdebsym_lseg = lseg ;
  EndIf
 If class_lname Is codeview_class_DEBTYP
  Then
   cvdebtyp_lseg = lseg ;
  EndIf

 obj_next_record();
 return(True);
EndCode
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              obj_seg_grp                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_seg_grp:: {obj_LNAMES | obj_SEGDEF | obj_EXTDEF}
                 {obj_TYPDEF | obj_EXTDEF | obj_GRPDEF} */
bit_16 obj_seg_grp()
BeginDeclarations
EndDeclarations
BeginCode
 // the addition of obj_thread_def is to support some microsoft compilers, which
 // put thread fixup definitions before the lnames
 While obj_LNAMES() OrIf obj_SEGDEF() OrIf obj_EXTDEF() OrIf obj_GRPDEF() OrIf obj_thread_def()
  BeginWhile
  EndWhile;
 While obj_TYPDEF() OrIf obj_EXTDEF() OrIf obj_GRPDEF() OrIf obj_thread_def()
  BeginWhile
  EndWhile;
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_THEADR                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_THEADR()
BeginDeclarations
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot THEADR_record
  Then
   return(False);
  EndIf;
 tmodule_name = obj_name();
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_thread_def                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_thread_def:: obj_FIXUPP  (containing only thread fields) */

bit_16 obj_thread_def()
BeginDeclarations
EndDeclarations
BeginCode
 If obj_FIXUPP()
  Then
   If FIXUPP_contains_only_threads
    Then
     return(True);
    Else
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%s\"\n"
                      "\t  File:  \"%s\"\n"
                      "\tOffset:  %08lX\n"
                      "\t Error:  \"THREAD DEF\" FIXUPP encountered which "
                                  "did not contain\n"
                      "\t          only thread defs.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset);
    EndIf;
  EndIf;
 return(False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              obj_tmodule                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_t_module:: obj_THEADR obj_seg_grp {obj_component} obj_modtail */

bit_16 obj_tmodule()
BeginDeclarations
   module_ptr                             mod ;
#define Mod                             (*mod)
EndDeclarations
BeginCode
 far_set(BytePtr(externals), 0,
         sizeof(public_entry_ptr)*(max_externals+1));
 far_set(BytePtr(gnames),    0,
         sizeof(group_entry_ptr)*(max_groups+1));
 far_set(BytePtr(lnames),    0,
         sizeof(lname_entry_ptr)*(max_lnames+1));
 far_set(BytePtr(snames),    0,
         sizeof(lseg_ptr)*(max_segments+1));
 far_set(BytePtr(target_thread), 0, sizeof(thread_type)*4);
 far_set(BytePtr(frame_thread),  0, sizeof(thread_type)*4);

 cseg_lseg = Null ;
 dseg_lseg = Null ;
 bseg_lseg = Null ;
 constseg_lseg = Null ;
 stringseg_lseg = Null ;
 cvdebtyp_lseg = Null ;
 cvdebsym_lseg = Null ;

 n_externals =
 n_groups    =
 n_lnames    =
 n_segments  = 0;

 current_module = Null;
 module_publics.first = Null;
 module_publics.last = Null;

 tmodule_number++;
 tmodule_name = lookup_lname(31, (byte *) "(THEADR record not encountered)");
 obj_next_record();
 If Not obj_THEADR()
  Then
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%s\"\n"
                    "\t  File:  \"%s\"\n"
                    "\tOffset:  %08lX\n"
                    "\t Error:  T-MODULE record missing.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  EndIf;
 If Not obj_seg_grp()
  Then
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%s\"\n"
                    "\t  File:  \"%s\"\n"
                    "\tOffset:  %08lX\n"
                    "\t Error:  Segment/Group definition record(s) missing.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  EndIf;
 While obj_component() OrIf obj_GRPDEF() /* virtual segments come before groups */
  BeginWhile
  EndWhile;
 If Not obj_modtail()
  Then
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%s\"\n"
                    "\t  File:  \"%s\"\n"
                    "\tOffset:  %08lX\n"
                    "\t Error:  Unknown record type encountered: %x.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset,
                    Current_record_header.rec_typ) ;
  EndIf;
 If debug.val IsNotZero And current_module IsNull And cseg_lseg IsNotNull
   And dseg_lseg IsNotNull And bseg_lseg IsNotNull
  Then
    mod = (module_ptr) allocate_memory(sizeof(module_type) + (*tmodule_name).length+1) ;
    Mod.cseg = cseg_lseg ;
    Mod.dseg = dseg_lseg ;
    Mod.bseg = bseg_lseg ;
    Mod.constseg = constseg_lseg ;
    Mod.stringseg = stringseg_lseg ;
    Mod.publics = module_publics ;
    memcpy(Mod.name, (*tmodule_name).symbol, (*tmodule_name).length) ;
    Mod.name[(*tmodule_name).length] = 0 ;
    If n_modules NotLessThan max_modules
     Then
      max_modules += 64 ;
      modules = (module_ptr_array)
                           reallocate_memory(modules,
                                       (Bit_32(max_modules)+1L) *
                                        Bit_32(sizeof(module_ptr)));

     EndIf
    modules[n_modules++] = current_module = mod ;

  EndIf ;
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_TYPDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_TYPDEF()
BeginDeclarations
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot TYPDEF_record
  Then
   return(False);
  EndIf;
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              write_fixupp                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void write_fixupp(bit_8           rec_typ,
                     lseg_ptr        lseg,
                     bit_32          offset,
                     byte_ptr        data,
                     bit_16          len)
BeginDeclarations
fixup_list_ptr				p ;
EndDeclarations
BeginCode
 p = allocate_memory(sizeof(fixup_list_type)) ;
 (*p).rec_typ       = rec_typ;
 (*p).rec_len       = len;
 (*p).lseg          = lseg;
 (*p).offset        = offset;
 If fixup_list IsNull
   Then 
 	fixup_list = fixup_list_tail = p ;
   Else
    fixup_list_tail = (*fixup_list_tail).next = p ;
   EndIf
 If len Exceeds 0
  Then
   (*p).data = allocate_memory(len) ;
   far_move((*p).data,data,Bit_32(len)) ;
  EndIf;
 return;
EndCode
