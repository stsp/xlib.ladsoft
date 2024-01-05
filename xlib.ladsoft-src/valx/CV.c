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
#include <time.h>
#include <dos.h>

#include "langext.h"
#include "defines.h"
#include "types.h"
#include "subs.h"
#include "globals.h"
#include "cvinfo.h"
#include "cvexefmt.h"

#undef Mod
void linnum_new_module(bit_32 filenum, char *name)
BeginDeclarations
   module_ptr                             mod ;
#define Mod                             (*mod)
EndDeclarations
BeginCode

   mod = (module_ptr) allocate_memory(sizeof(module_type) + strlen(name) + 1) ;
   If filenum Is 1
    Then 
     Mod.debtypseg = cvdebtyp_lseg ;
     Mod.debsymseg = cvdebsym_lseg ;
     Mod.cseg = cseg_lseg ;
     Mod.dseg = dseg_lseg ;
     Mod.bseg = bseg_lseg ;
     Mod.constseg = constseg_lseg ;
     Mod.stringseg = stringseg_lseg ;
     If cseg_lseg IsNotNull
      Then
       (*cseg_lseg).hassyms = True ;
      EndIf ;
     If dseg_lseg IsNotNull
      Then
       (*dseg_lseg).hassyms = True ;
      EndIf ;
     If bseg_lseg IsNotNull
      Then
       (*bseg_lseg).hassyms = True ;
      EndIf ;
    EndIf
    
   strcpy(Mod.name,name) ;
   If n_modules NotLessThan max_modules
    Then
     max_modules += 64 ;
     modules = (module_ptr_array)
                           reallocate_memory(modules,
                                       (Bit_32(max_modules)+1L) *
                                        Bit_32(sizeof(module_ptr)));

    EndIf
  
  modules[n_modules++] = current_module = mod ;

EndCode
#undef Mod

void linnum_new_data(byte_ptr data, bit_32 len)
BeginDeclarations
#define Current_module                  (*current_module)
EndDeclarations
BeginCode
  If current_module IsNotNull
   Then
    Current_module.linnum_data = (byte_ptr)
                           reallocate_memory(Current_module.linnum_data,
                                       Current_module.linnum_len + len) ;
    memcpy(Current_module.linnum_data + Current_module.linnum_len, data, len) ;
    Current_module.linnum_len += len ;
   EndIf
EndCode
#undef Current_module

void cv_globaltypes_data(byte_ptr *type_data, bit_32_ptr type_len, byte_ptr * offset_data, bit_32_ptr offset_len)
BeginDeclarations
bit_32                                    i;
int_32                                    length ;
bit_32                                    pos ;
int_32                                    fieldslen ;
bit_32                                    reclen ;
bit_32                                    typeofs ;
bit_32                                    typecount ;
byte_ptr                                  dragofs ;
byte_ptr                                  field_ptr ;
byte_ptr                                  typedat ;
bit_32_ptr                                offsets ;
bit_32                                    max_offsets ;
bit_32                                    n_offsets ;
module_ptr                                mod ;
#define Mod                             (*mod)
#define NEWTYPE(x) (x) = ((x) >= CV_FIRST_NONPRIM ? (x) + typeofs - CV_FIRST_NONPRIM : (x))
#define LEAFSIZE(x) ((*(bit_16_ptr)(x)) > 32767 ? 6 : 2) /* only two types of numeric leaf allowed */
EndDeclarations
BeginCode
   length = 0 ;
   offsets = Null ;
   max_offsets = n_offsets = 0 ;
   For i =0; i < n_modules; i++
    BeginFor
     If modules[i]->debtypseg IsNotNull
      Then
       length += (*modules[i]->debtypseg).length - 4;
      EndIf
    EndFor

   typeofs = CV_FIRST_NONPRIM ;

   typedat = (byte_ptr) allocate_memory(length) ;

   pos = 0 ;

   For i =0; i < n_modules; i++
    BeginFor
     mod = modules[i] ;
     If Mod.debtypseg IsNotNull
      Then
       length = (*modules[i]->debtypseg).length - 4;
       dragofs = (*modules[i]->debtypseg).data + 4 ;
       Mod.type_offset = typeofs ;
       typecount = 0 ;
       While length GreaterThan 0
        BeginWhile
         reclen = (*(bit_16_ptr)dragofs) + 2 ;
         memcpy(typedat + pos, dragofs, reclen) ;
         If n_offsets >= max_offsets
          Then
           max_offsets += 2048 ;
           offsets = (bit_32_ptr)reallocate_memory(offsets, max_offsets * 4) ;
          EndIf
         offsets[n_offsets ++] = pos ;
         dragofs += reclen ;
         length -= reclen ;
         typecount++ ;
         Using *(bit_16_ptr)(typedat + pos + 2)
          BeginCase
           When LF_POINTER:
            { 
               lfPointer *p = (lfPointer *)(typedat + pos + 2);
               NEWTYPE(p->u.utype) ;
            }
            break ;
           When LF_ARRAY:
		   When LF_VARARRAY:
            { 
               lfArray *p = (lfArray *)(typedat + pos + 2);
               NEWTYPE(p->idxtype) ;
               NEWTYPE(p->elemtype) ;
            }
            break ;
           When LF_UNION:
            { 
               lfUnion *p = (lfUnion *)(typedat + pos + 2);
               NEWTYPE(p->field) ;
            }
            break ;
           When LF_CLASS:
           When LF_STRUCTURE:
            { 
               lfClass *p = (lfClass *)(typedat + pos + 2);
               NEWTYPE(p->field) ;
               NEWTYPE(p->derived) ;
               NEWTYPE(p->vshape) ;
            }
            break ;
           When LF_ENUM:
            { 
               lfEnum *p = (lfEnum *)(typedat + pos + 2);
               NEWTYPE(p->utype) ;
               NEWTYPE(p->field) ;
            }
            break ;
           When LF_BITFIELD:
            { 
               lfBitfield *p = (lfBitfield *)(typedat + pos + 2) ;
               NEWTYPE(p->type) ;
            }
            break ;
           When LF_FIELDLIST:
            fieldslen = reclen - 4 ;
            field_ptr = (byte_ptr)(typedat + pos + 4) ;
            While fieldslen GreaterThan 0
             BeginWhile
              Using *(bit_16_ptr)field_ptr
               BeginCase
                When LF_ENUMERATE:
                 { 
                     lfEnumerate *p =(lfEnumerate *)field_ptr;
                     bit_32 len = LEAFSIZE(p->value) ;
                     len += p->value[len] + sizeof(lfEnumerate) ;
                     field_ptr += len ;
                     fieldslen -= len ;
                 }
                 break ;
                When LF_MEMBER:
                 { 
                     lfMember *p = (lfMember *)field_ptr;
                     bit_32 len = LEAFSIZE(p->offset) ;
                     len += p->offset[len] + sizeof(lfMember) ;
                     NEWTYPE(p->index) ;
                     field_ptr += len ;
                     fieldslen -= len ;
                 }
                 break ;
                When LF_INDEX:
                 { 
                     lfIndex *p = (lfIndex *)field_ptr;
                     NEWTYPE(p->index) ;
                 }
                 field_ptr += sizeof(lfIndex) ;
                 fieldslen -= sizeof(lfIndex) ;
                 break ;
                Otherwise:
                 printf("CV - unknown debtype fieldlist %X in module %s\n",*(bit_16_ptr)field_ptr,Mod.name) ;
                 field_ptr += sizeof(lfIndex) ;
                 fieldslen -= sizeof(lfIndex) ;
               EndCase
              If fieldslen GreaterThan 0 AndIf 
                     ((*field_ptr And 0xF0) IsEqualTo 0xF0)
               Then
                fieldslen -= *field_ptr And 15 ;
                field_ptr += *field_ptr And 15 ;
               EndIf
             EndWhile
            break ;
           Otherwise:
            printf("CV - unknown debtyp field %X in module %s\n",*(bit_16_ptr)(typedat + pos + 2),Mod.name);
          EndCase

         pos += reclen ;
        EndWhile

       typeofs += typecount ;
      EndIf
    EndFor
   For i = 0; i < n_offsets; i++
    BeginFor
     offsets[i] += n_offsets * sizeof(bit_32) + sizeof(bit_32) ;
    EndFor

   *type_data = typedat ;
   *type_len = pos ;

   *offset_data = offsets ;
   *offset_len = n_offsets * sizeof(bit_32) ;
EndCode

#undef Mod
#undef NEWTYPE
#undef LEAFSIZE

void GlobalAddress(module_ptr mod, bit_32_ptr ofs, bit_16_ptr seg)
BeginDeclarations
#define Mod                                (*mod)
EndDeclarations
   Using *seg
    BeginCase
     When 1:
      *ofs += Mod.cseg->address ;
      If pefile.val IsFalse AndIf lefile.val IsFalse
	   Then
        *seg = 0 ;
       Else
        *ofs -= imageBase.val ;
        *seg = PE_code_seg+1 ;  
       EndIf
      break ;
     When 2:
      *ofs += Mod.dseg->address ;
      If pefile.val IsFalse AndIf lefile.val IsFalse
       Then
        *seg = 0 ;
       Else
        *seg = PE_data_seg+1 ;  
        *ofs -= imageBase.val ;
       EndIf
      break ;
     When 3:
      *ofs += Mod.bseg->address ;
      If pefile.val IsFalse AndIf lefile.val IsFalse
       Then
        *seg = 0 ;
       Else
        *seg = PE_bss_seg+1 ;  
        *ofs -= imageBase.val ;
       EndIf
      break ;
     When 4:
      *ofs += Mod.constseg->address ;
      If pefile.val IsFalse AndIf lefile.val IsFalse
       Then
        *seg = 0 ;
       Else
        *ofs -= imageBase.val ;
        *seg = PE_code_seg+1 ;  
       EndIf
      break ;
     When 5:
      *ofs += Mod.stringseg->address ;
      If pefile.val IsFalse AndIf lefile.val IsFalse
       Then
        *seg = 0 ;
       Else
        *ofs -= imageBase.val ;
        *seg = PE_code_seg+1 ;  
       EndIf
      break ;
    EndCase
BeginCode
EndCode
#undef Mod

void cv_globalsyms_data(byte_ptr *globals, bit_32_ptr globallen)
BeginDeclarations
byte                                         align_copy[512];
bit_32                                       align_base ;
bit_32                                       align_len ;
byte_ptr                                     align_ptr ;
bit_32                                       blocklevel ;
bit_32                                       copy_to_global ;
byte_ptr                                     cp ;
bit_32                                       i ;
Generic_Element_ptr                          generic;
byte                                         global_copy[512] ;
bit_32                                       global_len ;
bit_32                                       global_max ;
byte_ptr                                     global_ptr ;
bit_32                                       length ;
module_ptr                                   mod ;
bit_32                                       global_base ;
bit_32                                       l;
LABELSYM32                                  *lab;
bit_32_ptr                                   nextptr_global ;
bit_32_ptr                                   nextptr_align ;
bit_32_ptr                                   oldptr_global ;
public_entry_ptr                             pub;
lseg_ptr                                     lseg ;
SEARCHSYM                                   *search_ptr_align ;
SEARCHSYM                                   *search_ptr_global ;
bit_32                                       symlen ;
byte_ptr                                     symptr ;
#define Mod                                (*mod)
#define Pub                                (*pub)
#define Lseg                               (*lseg)
#define Lab                                (*lab)
#define NEWTYPE(x) (x) = ((x) >= CV_FIRST_NONPRIM ? (x) + Mod.type_offset - CV_FIRST_NONPRIM : (x))
EndDeclarations
BeginCode
   blocklevel = 0 ;
   global_max = 65536 ;
   global_ptr = (byte_ptr) allocate_memory(global_max) ;
   search_ptr_global = (SEARCHSYM *)global_ptr ;
   search_ptr_global->reclen = sizeof(SEARCHSYM) ;
   search_ptr_global->reclen = ((search_ptr_global->reclen + 3) And ~3) - 2 ;
   search_ptr_global->rectyp = S_SSEARCH ;
   global_len = search_ptr_global->reclen + 2 ;


   global_base = global_len -4 ; /* makes up for the 0002 in the obj file */

   nextptr_align = Null ;
   nextptr_global = Null ;

   For i = 0; i < n_modules; i++
    BeginFor
     mod = modules[i] ;
     lseg = Mod.debsymseg ;
     If lseg IsNull
      Then
       LoopIf(Mod.publics.first IsNull) ;

       Mod.align_len = 65536 ;
       Mod.align_data = (byte_ptr)allocate_memory(Mod.align_len) ;

       align_base = -4 ;
       align_len = 0 ;

	   align_ptr = (byte_ptr)Mod.align_data ;
       search_ptr_align = (SEARCHSYM *)Mod.align_data;
       search_ptr_align->reclen = sizeof(SEARCHSYM) ;
       search_ptr_align->reclen = ((search_ptr_align->reclen + 3) And ~3) - 2 ;
       search_ptr_align->rectyp = S_SSEARCH ;
       search_ptr_align->startsym = 0 ;
       search_ptr_align->seg = 0 ;
       align_len += search_ptr_align->reclen + 2 ;
       search_ptr_align = NULL ;

       align_base += align_len ;

       TraverseList(Mod.publics, generic)
        BeginTraverse
         pub = (public_entry_ptr)((byte_ptr)generic - PUB_OFFSET_MODPUBS) ;
         lab = (LABELSYM32 *)align_copy;
         Lab.reclen = sizeof(LABELSYM32) + (l = Pub.length) ;
         cp = Pub.symbol;
         If cp[0] Is '_'
          Then
           Lab.reclen--;
           l--;
           cp++;
          EndIf;
         If Lab.reclen % 4
          Then
           Lab.reclen = Lab.reclen + 4 - Lab.reclen % 4 ;
          EndIf
         Lab.reclen -= 2 ;
         Lab.rectyp = S_LABEL32 ;
         memset(&Lab.flags,0,sizeof(Lab.flags)) ;
         Lab.name[0] = l ;
         memcpy(Lab.name+1,cp, l) ;
         Lab.seg = Pub.Internal.segment_index ;
         Lab.off = Pub.Internal.offset ;
         GlobalAddress(mod,&Lab.off, &Lab.seg) ;
         length = Lab.reclen + 2 ;
         If global_len + length > global_max
          Then
           global_max += 65536 ;
           oldptr_global = (bit_32_ptr)global_ptr ;
           global_ptr = (byte_ptr) reallocate_memory(global_ptr, global_max) ;
           If nextptr_global IsNotNull
            Then
             nextptr_global = (bit_32_ptr)((bit_32)nextptr_global + (bit_32)global_ptr - (bit_32) oldptr_global) ;
            EndIf
           If search_ptr_global IsNotNull
            Then
             search_ptr_global = (SEARCHSYM *)((bit_32)search_ptr_global + (bit_32)global_ptr - (bit_32) oldptr_global) ;
            EndIf
          EndIf
         memcpy(global_ptr + global_len, align_copy, length) ;
         global_len += length ;
         If align_len + length > Mod.align_len
          Then
           Mod.align_len += 65536 ;
           align_ptr = (byte_ptr) reallocate_memory(align_ptr, Mod.align_len) ;
          EndIf
         memcpy(align_ptr + align_len, align_copy, length) ;
         align_len += length ;
        EndTraverse
      Else
       Mod.align_len = Lseg.length ;
       Mod.align_data = (byte_ptr)allocate_memory(Mod.align_len + sizeof(SEARCHSYM)) ;

       symptr = Lseg.data + 4;
       symlen = Lseg.length -4;

       align_base = -4 ;
       align_len = 0 ;

	   align_ptr = (byte_ptr)Mod.align_data ;
       search_ptr_align = (SEARCHSYM *)Mod.align_data;
       search_ptr_align->reclen = sizeof(SEARCHSYM) ;
       search_ptr_align->reclen = ((search_ptr_align->reclen + 3) And ~3) - 2 ;
       search_ptr_align->rectyp = S_SSEARCH ;
       align_len += search_ptr_align->reclen + 2 ;

       align_base += align_len ;

       While symlen GreaterThan 0
        BeginWhile
         memcpy(align_copy,symptr,(*(bit_16_ptr)symptr) + 2) ;
         Using *(bit_16_ptr)(symptr+2)
          BeginCase
           When S_UDT:
            { 
              UDTSYM *p = (UDTSYM *)align_copy ;
              NEWTYPE(p->typind) ;
            }
            memcpy(global_copy,align_copy , (*(bit_16 *)align_copy) + 2) ;
            copy_to_global = True ;
            break ;
           When S_BLOCK32:
            {
              BLOCKSYM32 *p = (BLOCKSYM32 *)align_copy ;
              p->pParent += align_base ;
              p->pEnd += align_base ;
              GlobalAddress(mod,&p->off, &p->seg) ;
              If copy_to_global IsTrue
               Then
                memcpy(global_copy,symptr,(*(bit_16_ptr)symptr) + 2) ;
                p = global_copy ;
                GlobalAddress(mod,&p->off, &p->seg) ;
                p->pParent += global_len - align_len + global_base ;
                p->pEnd += global_len - align_len + global_base;
               EndIf
            }
            blocklevel++ ;
            break ;
           When S_REGISTER:
            {
              REGSYM *p = (REGSYM *)align_copy ;
              NEWTYPE(p->typind) ;
              memcpy(global_copy,align_copy , (*(bit_16 *)align_copy) + 2) ;
            }
            break ;
           When S_BPREL32:
            { 
              BPRELSYM32 *p = (BPRELSYM32 *)align_copy ;
              NEWTYPE(p->typind) ;
              memcpy(global_copy,align_copy , (*(bit_16 *)align_copy) + 2) ;
            }
            break ;
		   When S_REGREL32:
		    {
              REGREL32 *p = (REGREL32 *)align_copy ;
              //NEWTYPE(p->typind) ; not setting type here...
              memcpy(global_copy,align_copy , (*(bit_16 *)align_copy) + 2) ;
			}
			break;
           When S_LDATA32:
            If blocklevel IsZero
             Then
              copy_to_global = False ;
             EndIf ;
            {
              DATASYM32 *p = (DATASYM32 *)align_copy ;
              NEWTYPE(p->typind) ;
              GlobalAddress(mod,&p->off, &p->seg) ;
              If copy_to_global IsTrue
               Then
                memcpy(global_copy,align_copy,(*(bit_16_ptr)symptr) + 2) ;
               EndIf
            }
            break ;
           When S_GDATA32:
            copy_to_global = True ;
            {
              DATASYM32 *p = (DATASYM32 *)align_copy ;
              NEWTYPE(p->typind) ;
              GlobalAddress(mod,&p->off, &p->seg) ;
              memcpy(global_copy,align_copy,(*(bit_16_ptr)symptr) + 2) ;
            }
            break ;
           When S_END:
            blocklevel-- ;
            memcpy(global_copy,symptr,(*(bit_16_ptr)symptr) + 2) ;
            break ;
           When S_ENDARG:
            memcpy(global_copy,symptr,(*(bit_16_ptr)symptr) + 2) ;
            break ;
           When S_LPROC32:
            copy_to_global = False ;
            If nextptr_align IsNotNull
             Then
              *nextptr_align = align_len;
              nextptr_align = Null ;
             EndIf
            {
              PROCSYM32 *p = (PROCSYM32 *)align_copy ;
              NEWTYPE(p->typind) ;
              GlobalAddress(mod,&p->off, &p->seg) ;
              p->pEnd += align_base ;
              p->pNext = 0;
              nextptr_align = align_ptr + align_len + ((byte_ptr)&p->pNext - (byte_ptr)p) ;
              If search_ptr_align IsNotNull
               Then
                search_ptr_align->startsym = align_len ;
                search_ptr_align->seg = p->seg ;
                search_ptr_align = NULL ;
               EndIf
            }
            blocklevel++ ;
            break ;
           When S_GPROC32:
            copy_to_global = True ;
            If nextptr_align IsNotNull
             Then
              *nextptr_align = align_len;
              nextptr_align = Null ;
             EndIf
            If nextptr_global IsNotNull
             Then     
              *nextptr_global = global_len ;
              nextptr_global = Null ;
             EndIf
            {
              PROCSYM32 *p = (PROCSYM32 *)align_copy ;
              NEWTYPE(p->typind) ;
              GlobalAddress(mod, &p->off, &p->seg) ;
              p->pEnd += align_base ;
              p->pNext = 0;
              nextptr_align = align_ptr + align_len + ((byte_ptr)&p->pNext - (byte_ptr)p);
              If search_ptr_align IsNotNull
               Then
                search_ptr_align->startsym = align_len ;
                search_ptr_align->seg = p->seg ;
                search_ptr_align = NULL ;
               EndIf
              memcpy(global_copy,symptr,(*(bit_16_ptr)symptr) + 2) ;
              p = global_copy ;
              NEWTYPE(p->typind) ;
              GlobalAddress(mod, &p->off, &p->seg) ;
              p->pEnd += global_len - align_len + global_base ;
              p->pNext = 0;
              nextptr_global = global_ptr + global_len + ((byte_ptr)&p->pNext - (byte_ptr)p);
              If search_ptr_global IsNotNull
               Then
                search_ptr_global->startsym = global_len ;
                search_ptr_global->seg = p->seg ;
                search_ptr_global = NULL ;
               EndIf
            }
            blocklevel++ ;
            break ;
           When S_LABEL32:
            {
              LABELSYM32 *p = (LABELSYM32 *)align_copy ;
              GlobalAddress(mod,&p->off, &p->seg) ;
              copy_to_global = False ;
            }
            break ;
           When S_RETURN:
            memcpy(global_copy,align_copy,(*(bit_16_ptr)symptr) + 2) ;
            break ;
           When S_COMPILE:
            copy_to_global = False ;
            break ;
           When S_OBJNAME:
            copy_to_global = False ;
            break ;
           Otherwise:
            printf("CV - unknown symbol record\n") ;
          EndCase
         length = (*(bit_16 *)symptr) + 2 ;
         If copy_to_global IsTrue
          Then
           If global_len + length > global_max
            Then
             global_max += 65536 ;
             oldptr_global = global_ptr ;
             global_ptr = (byte_ptr) reallocate_memory(global_ptr, global_max) ;
             If nextptr_global IsNotNull
              Then
               nextptr_global = (bit_32_ptr)((bit_32)nextptr_global + (bit_32)global_ptr - (bit_32) oldptr_global) ;
              EndIf
             If search_ptr_global IsNotNull
              Then
               search_ptr_global = (SEARCHSYM *)((bit_32)search_ptr_global + (bit_32)global_ptr - (bit_32) oldptr_global) ;
              EndIf
            EndIf
           memcpy(global_ptr + global_len, global_copy, length) ;
           global_len += length ;
          EndIf
         memcpy(align_ptr + align_len, align_copy, length) ;
         align_len += length ;
         symptr += length ;
         symlen -= length ;
        EndWhile
      EndIf
     Mod.align_len = align_len ;
    EndFor
   *globallen = global_len ;
   *globals = global_ptr ;
EndCode
#undef Mod
#undef Pub
#undef Lseg
#undef Lab

void create_codeview_data(pe_header_ptr pehead, byte_ptr *data_ptr, bit_32_ptr data_len)
BeginDeclarations
bit_32                                          i, j ;
byte_ptr                                        globals ;
bit_32                                          global_len ;
byte_ptr                                        linnum_ptr ;
module_ptr                                      mod ;
#define Mod                                   (*mod)
byte_ptr                                        types ;
bit_32                                          type_len ;
byte_ptr                                        types_offsets ;
bit_32                                          types_offsets_len ;
byte_ptr                                        tables_ptr ;
bit_32                                          tables_size ;
byte_ptr                                        dir_ptr ;
bit_32                                          dir_size ;
byte_ptr                                        data ;
EndDeclarations
BeginCode
   cv_globaltypes_data(&types, &type_len, &types_offsets, &types_offsets_len);
   cv_globalsyms_data( & globals, & global_len);
   tables_size = sizeof(OMFSignature) +32 ;
   if (global_len)
      tables_size += global_len + sizeof(OMFSymHash) ;
   if (type_len)
      tables_size += type_len + sizeof(OMFGlobalTypes) - sizeof(bit_32) 
                     + types_offsets_len ;

   dir_size = sizeof(OMFDirHeader) ;
   if (type_len)
      dir_size += sizeof(OMFDirEntry);
   if (global_len)
      dir_size += sizeof(OMFDirEntry);

   For i =0; i < n_modules; i++
    BeginFor
     mod = modules[i] ;
     If Mod.debsymseg IsNotNull Or Mod.publics.first IsNotNull
      Then
       dir_size+= sizeof(OMFDirEntry) ; /* module entry */
       tables_size += sizeof(OMFModule) - sizeof(OMFSegDesc);
       If Mod.cseg AndIf Mod.cseg->length
        Then
         tables_size += sizeof(OMFSegDesc) ;
        EndIf
       If Mod.dseg AndIf Mod.dseg->length
        Then
         tables_size += sizeof(OMFSegDesc) ;
        EndIf
       If Mod.bseg AndIf Mod.bseg->length
        Then
         tables_size += sizeof(OMFSegDesc) ;
        EndIf
       tables_size += strlen(Mod.name) ;
       tables_size = (tables_size + 3) & ~3 ;
       If Mod.align_len IsNotZero
        Then
         dir_size+= sizeof(OMFDirEntry) ; /* align entry */
         tables_size += Mod.align_len ;
        EndIf 
       If Mod.linnum_len IsNotZero
        Then
         dir_size+= sizeof(OMFDirEntry) ; /* linnum entry */
         tables_size += Mod.linnum_len + sizeof(bit_32);
         tables_size = (tables_size + 3) & ~3 ;
        EndIf 
      EndIf
    EndFor

   data = (byte_ptr) allocate_memory(dir_size + tables_size) ;

   tables_ptr = data + 32;
   dir_ptr = data + tables_size ;

   {
     OMFSignature *p = (OMFSignature *)tables_ptr ;
     p->Signature[0] = 'L' ;
     p->Signature[1] = 'S' ;
     p->Signature[2] = '1' ;
     p->Signature[3] = '2' ;
     p->filepos = tables_size - 32 ;
	 p->timeStamp = pehead->time;
     tables_ptr += sizeof(OMFSignature) ;
   }
   {
     OMFDirHeader *p = (OMFDirHeader *)dir_ptr ;
     p->cbDirHeader = sizeof(OMFDirHeader) ;
     p->cbDirEntry = sizeof(OMFDirEntry) ;
     p->cDir = (dir_size - sizeof(OMFDirHeader)) / sizeof(OMFDirEntry) ;
     p->lfoNextDir = 0 ;
     p->flags = 0 ;
     dir_ptr += sizeof(OMFDirHeader) ;
   }
   For i = 0 ; i < n_modules; i++
    BeginFor
     OMFModule *m = (OMFModule *)tables_ptr ;
     OMFSegDesc *p ;
     
     mod = modules[i] ;
     If Mod.debsymseg IsNotNull Or Mod.publics.first IsNotNull
      Then
       m->ovlNumber = 0 ;
       m->iLib = 0 ;
       m->cSeg = 0 ;
       m->Style[0] = 'C' ;
       m->Style[1] = 'V' ; 
       p = &m->SegInfo[0] ;
       If Mod.cseg IsNotZero AndIf Mod.cseg->length IsNotZero
        Then
         p->Seg = 1 ;
         p->pad = 0 ;
         p->Off = 0 ;
         p->cbSeg = Mod.cseg->length ;
         GlobalAddress(mod, &p->Off, &p->Seg) ;
         m->cSeg++ ;
         p ++ ;
        EndIf
       If Mod.dseg IsNotZero AndIf Mod.dseg->length IsNotZero
        Then
         p->Seg = 2 ;
         p->pad = 0 ;
         p->Off = 0 ;
         p->cbSeg = Mod.dseg->length ;
         GlobalAddress(mod, &p->Off, &p->Seg) ;
         m->cSeg++ ;
         p ++ ;
        EndIf
       If Mod.bseg IsNotZero AndIf Mod.bseg->length IsNotZero
        Then
         p->Seg = 3 ;
         p->pad = 0 ;
         p->Off = 0 ;
         p->cbSeg = Mod.bseg->length ;
         GlobalAddress(mod, &p->Off, &p->Seg) ;
         m->cSeg++ ;
         p ++ ;
        EndIf
       *(byte_ptr)p = strlen(Mod.name) ;
       memcpy(((byte_ptr)p)+1,Mod.name, *(byte_ptr)p) ;
       (byte_ptr)p += 1 + *(byte_ptr)p ;
       If (((byte_ptr)p - tables_ptr) And 3) IsNotZero
        Then
         (byte_ptr)p += 4 - (((byte_ptr)p -tables_ptr) And 3) ;
        EndIf

       {
        OMFDirEntry * q = (OMFDirEntry *)dir_ptr ;
        q->SubSection = sstModule ;
        q->iMod = i + 1 ;
        q->lfo = tables_ptr - data - 32 ;
        q->cb = (byte_ptr)p - (byte_ptr)tables_ptr ;
        dir_ptr += sizeof(OMFDirEntry);
       }
       tables_ptr = (byte_ptr) p ;
      EndIf
    EndFor
   For i = 0; i < n_modules; i++
    BeginFor
     mod = modules[i] ;
     If Mod.debsymseg IsNotNull Or Mod.publics.first IsNotNull
      Then
       If Mod.align_len IsNotZero 
        Then
         memcpy(tables_ptr,Mod.align_data,Mod.align_len) ;
         {
           OMFDirEntry *p = (OMFDirEntry *)dir_ptr ;
           p->SubSection = sstAlignSym ;
           p->iMod = i + 1 ;
           p->lfo = tables_ptr - data - 32;
           p->cb = Mod.align_len ;
           dir_ptr += sizeof(OMFDirEntry) ;
         }
         tables_ptr += Mod.align_len ;
        EndIf
       If Mod.linnum_len 
        Then
         linnum_ptr = tables_ptr + sizeof(bit_16);

         *((bit_16_ptr)linnum_ptr)++ = Mod.linnum_len / 6 ;
         For j = 0 ; j < Mod.linnum_len; j += 6 
          BeginFor
           (*(bit_16_ptr)tables_ptr) = 1 ;
           *(bit_32 *)linnum_ptr = *(bit_32_ptr) (Mod.linnum_data + j + sizeof(bit_16)) ;
           GlobalAddress(mod,linnum_ptr, tables_ptr) ;
           linnum_ptr += sizeof(bit_32) ;
          EndFor
         For j = 0; j < Mod.linnum_len; j+= 6
          BeginFor
           *((bit_16_ptr)linnum_ptr)++ = *(bit_16_ptr) (Mod.linnum_data + j) ;
          EndFor
         {
           OMFDirEntry *p = dir_ptr ;
           p->SubSection = sstSrcModule ;
           p->iMod = i + 1 ;
           p->lfo = tables_ptr - data - 32 ;
           p->cb = (byte_ptr)linnum_ptr - (byte_ptr)tables_ptr ;
           dir_ptr += sizeof(OMFDirEntry) ;
         }
         tables_ptr = linnum_ptr ;
         If (tables_ptr - data ) And 3
          Then
           tables_ptr += 4 - ((tables_ptr - data) And 3) ;
          EndIf
        EndIf
      EndIf
    EndFor
   {
    OMFSymHash *p = tables_ptr ;
    p->symhash = OMFHASH_NONE ;
    p->addrhash = OMFHASH_NONE ;
    p->cbSymbol = global_len ;
    p->cbHSym = 0;
    p->cbHAddr = 0 ;
    memcpy(tables_ptr + sizeof(OMFSymHash), globals, global_len) ;
    {
      OMFDirEntry *p = dir_ptr ;
      p->SubSection = sstGlobalSym ;
      p->iMod = 0xffff ;
      p->lfo = tables_ptr - data - 32 ;
      p->cb = global_len + sizeof(OMFSymHash) ;
      dir_ptr += sizeof(OMFDirEntry) ;
    }
    tables_ptr += global_len + sizeof(OMFSymHash) ;
   }
   If type_len GreaterThan 0
    Then
     OMFGlobalTypes *p = tables_ptr ;
     p->flags.sig = 0xa4 ; /* wasn't specified */
     p->cTypes = types_offsets_len / sizeof(bit_32) ;
     memcpy(p->typeOffset,types_offsets, types_offsets_len) ;
     memcpy((byte_ptr)p->typeOffset + types_offsets_len,types, type_len) ;
     {
       OMFDirEntry *p = dir_ptr ;
       p->SubSection = sstGlobalTypes ;
       p->iMod = 0xffff ;
       p->lfo = tables_ptr - data - 32 ;
       p->cb = sizeof(OMFGlobalTypes) - sizeof(bit_32) + types_offsets_len + type_len ;
       dir_ptr += sizeof(OMFDirEntry) ;
       tables_ptr += p->cb ;
     }
    EndIf
    If tables_ptr - data IsNotEqualTo tables_size
     Then
      printf("CV - module data mismatch") ;
     EndIf
    If dir_ptr - data IsNotEqualTo tables_size + dir_size
     Then
      printf("CV - dir entry mismatch") ;
     EndIf
    *data_ptr = data ;
    *data_len = tables_size + dir_size ;
EndCode
#undef Mod