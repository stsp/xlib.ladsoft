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
#include <process.h>
 
#include "langext.h"
#include "defines.h"
#include "types.h"
#include "subs.h"
#include "globals.h"

bit_16 osMajor =4;
bit_16 osMinor =0;
bit_16 subsysMajor =4 ;
bit_16 subsysMinor =0;
bit_16 userMajor = 4;
bit_16 userMinor = 0;
static byte defaultStub[]={
    0x4D,0x5A,0x6C,0x00,0x01,0x00,0x00,0x00,
    0x04,0x00,0x11,0x00,0xFF,0xFF,0x03,0x00,
    0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
    0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x0E,0x1F,0xBA,0x0E,0x00,0xB4,0x09,0xCD,
    0x21,0xB8,0x00,0x4C,0xCD,0x21,0x54,0x68,
    0x69,0x73,0x20,0x70,0x72,0x6F,0x67,0x72,
    0x61,0x6D,0x20,0x72,0x65,0x71,0x75,0x69,
    0x72,0x65,0x73,0x20,0x57,0x69,0x6E,0x33,
    0x32,0x0D,0x0A,0x24,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

static int_32 defaultStubSize=sizeof(defaultStub);

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           createOutputSection                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 createOutputSection(byte_ptr name, bit_32 winflags)
BeginDeclarations
  bit_32 rv = n_pe_sections++ ;
EndDeclarations
BeginCode
	outList = (pe_section_ptr_array)reallocate_memory(outList,sizeof(pe_section_ptr) * n_pe_sections) ;
  outList[rv] = (pe_section_ptr)allocate_memory(sizeof(pe_section_type)) ;
  outList[rv]->winFlags = winflags ;
	memcpy(outList[rv]->name,name,8) ;
	return rv ;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          enter_pe_import_Fixup                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void enter_pe_import_fixup(byte_ptr location_ptr, 
													 bit_32 location_address,
                           public_entry_ptr import, bit_32 isdata) 
BeginDeclarations
EndDeclarations
BeginCode
	If pefile.val IsTrue
    Then
    	If n_pe_import_fixups IsEqualTo max_pe_import_fixups
		    Then
    			max_pe_import_fixups += 20 ;
		    	pe_import_fixup_array = (pe_import_fixup_ptr_array)reallocate_memory(pe_import_fixup_array, max_pe_import_fixups*sizeof(pe_import_fixup_ptr)) ;
		    EndIf ;
            import->isdata = isdata ;
			pe_import_fixup_array[n_pe_import_fixups] = (pe_import_fixup_ptr) allocate_memory(sizeof(pe_import_fixup_type)) ;
			pe_import_fixup_array[n_pe_import_fixups]->pub = import ;
			pe_import_fixup_array[n_pe_import_fixups]->location_ptr = location_ptr;
         pe_import_fixup_array[n_pe_import_fixups]->isdata = isdata;
			pe_import_fixup_array[n_pe_import_fixups++]->location_addr = location_address;
    EndIf ;	
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         apply_pe_import_fixups                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void apply_pe_import_fixups(pe_object_ptr objectTable)
BeginDeclarations
  bit_32 i,j ;
  pe_import_fixup_ptr fixup ;
#define Pub (*pub)
#define Fixup (*fixup) 
EndDeclarations
BeginCode
  For i=0; i LessThan n_pe_import_fixups; i++
    BeginFor
			fixup = pe_import_fixup_array[i] ;
         If Fixup.isdata
           Then
             *(bit_32_ptr)(Fixup.location_ptr) = Fixup.pub->Internal.offset + import_thunk_table->address + *(bit_32_ptr)(Fixup.location_ptr);
             enter_pe_fixup(Fixup.location_addr, offset32_location) ;
           Else
             *(bit_32_ptr)(Fixup.location_ptr) = Fixup.pub->Internal.offset + 
										import_thunk_table->address - Fixup.location_addr - 4 ;
           EndIf
    EndFor ;
EndCode
#undef Fixup
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              BuildPEImports                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void BuildPEImports(int_32 sectNum,pe_object_ptr objectTable, pe_header_ptr pehead)
BeginDeclarations

    bit_32 i,j,k;
		bit_32 dllCount = 0;
    bit_32 importCount = 0 ;
		bit_32 nameSize = 0;
		bit_32 impNameSize = 0;
		bit_32 namePos, lookupPos,hintPos,addressPos ;
		bit_32 reqCount = 0;
    bit_32 raw ;
    bit_32 impThunkAddress = 0;
    pe_section_ptr sect;
		pe_import_module_ptr modules =0;
    pe_object_ptr lastObjectTable ;
    pe_import_dir_ptr impdir ;
    pe_import_lookup_ptr lookup, lookup2 ;
		string_ptr name ;
#define Lookup (*lookup)
#define Lookup2 (*lookup2)
#define Sect (*sect )
#define ImpDir (*impdir) 
#define ObjectTable (*objectTable)
#define LastObjectTable (*lastObjectTable)
#define PEHead (*pehead)
EndDeclarations
BeginCode

    ReturnIf (sectNum LessThan 0) ;
		sect = outList[sectNum] ;

		For i=0; i < n_imports; i++
			BeginFor
			  For j = i+1; j < n_imports; j++
			    BeginFor
				  If imports[i] IsEqualTo imports[j]
				    Then
					  memcpy( imports +j, imports + j + 1, (n_imports - j - 1) * sizeof(imports[0]));
					  --n_imports;
					EndIf
				EndFor
            EndFor

    For i=0;i<n_imports;i++
     BeginFor
			LoopIf(!imports[i]->use_count);
			 reqCount++ ;
			 For j=0; j<dllCount; j++
        BeginFor
					ExitIf( Not compare_string(modules[j].name,imports[i]->moduleident)) ;
				EndFor ;
			 If j IsEqualTo dllCount 
				Then
					modules = (pe_import_module_ptr)reallocate_memory(modules,(++dllCount)*sizeof(pe_import_module_type)) ;
					modules[j].name = imports[i]->moduleident;
					modules[j].n_imports = modules[j].max_imports = 0;
					modules[j].funcnames = modules[j].ordinals = 0;
					modules[j].pubs = 0 ;
					nameSize += Length(modules[j].name) + 1;
          If nameSize And 1
						Then
							 nameSize++;
						EndIf ;
				EndIf ;
			 If modules[j].n_imports IsEqualTo modules[j].max_imports
				Then
					modules[j].max_imports += 16 ;
					modules[j].funcnames = (string_ptr_array)reallocate_memory(modules[j].funcnames, modules[j].max_imports * sizeof(string_ptr)) ;
					modules[j].ordinals = (bit_32_ptr)reallocate_memory(modules[j].ordinals,modules[j].max_imports*sizeof(bit_32)) ;
					modules[j].pubs = (public_entry_ptr_array)reallocate_memory(modules[j].pubs,modules[j].max_imports * sizeof(public_entry_ptr)) ;
				EndIf ;
			 name = imports[i]->entryident ;
			 /* If the entry identifier does not exist we have an ordinal;
        * but just in case windows ordinals change places we will import
        * by the public name.
        */
			 If name IsNull
				Then	
					name = (string_ptr)&imports[i]->max_length ;
				EndIf ;
			 modules[j].funcnames[modules[j].n_imports] = name;
			 modules[j].ordinals[modules[j].n_imports] = imports[i]->ordinal ;
			 modules[j].pubs[modules[j].n_imports++] = imports[i] ;
			 impNameSize += Length(name) + 1 + 2 ;
  		 If impNameSize And 1
			 	 Then
			 		impNameSize++;
			 	 EndIf ;
			 importCount++ ;
		 EndFor ;
    ReturnIf(!importCount) ;

		 namePos = (dllCount+1) *sizeof(pe_import_dir_type) ;
		 lookupPos = namePos + nameSize ;
		 If lookupPos Mod 4
      Then
       lookupPos += 4 - (lookupPos Mod 4) ;
			EndIf ;
		 hintPos = lookupPos + (importCount  + dllCount) * sizeof(pe_import_lookup_type) ;
     addressPos = hintPos + impNameSize ;
		 If addressPos Mod 4
			Then
				addressPos += 4- (addressPos Mod 4) ;
			EndIf ;
		 Sect.initlength = Sect.length = addressPos + (importCount  + dllCount) * sizeof(pe_import_lookup_type) ;
		 Sect.data = (byte_ptr) allocate_memory(Sect.length) ;
		 memset(Sect.data,0,Sect.length) ;


    lastObjectTable = objectTable + sectNum-1 ;
    objectTable = lastObjectTable + 1;
    k = LastObjectTable.raw_ptr + LastObjectTable.raw_size ;
    k+=fileAlign.val-1;
    k&=(0xffffffff-(fileAlign.val-1)); /* aligned */

    /* k is now physical location of this object */
    ObjectTable.raw_ptr = k ;

    k = LastObjectTable.virtual_size + LastObjectTable.virtual_addr ;
    ObjectTable.virtual_addr = k ;
    raw = k ;

    Sect.base=k+imageBase.val; /* get base address of section */

		impdir = (pe_import_dir_ptr)Sect.data ;
		lookup = (pe_import_lookup_ptr)(Sect.data + lookupPos) ;
		lookup2 = (pe_import_lookup_ptr)(Sect.data + addressPos) ;
		For i=0; i < dllCount; i++
			BeginFor
				ImpDir.time = 0;
				ImpDir.version = 0;
				ImpDir.dllName = namePos + raw ;
				ImpDir.thunkPos = (byte_ptr)lookup - (byte_ptr)Sect.data + raw ;
				ImpDir.thunkPos2 = (byte_ptr)lookup2 - (byte_ptr)Sect.data + raw ;
				impdir++ ;
        memcpy(Sect.data + namePos,String(modules[i].name),Length(modules[i].name)) ;
				namePos += Length(modules[i].name) + 1;
				If namePos And 1
					Then
						namePos++ ;
					EndIf;
				For j=0; j < modules[i].n_imports; j++
				  BeginFor
						If modules[i].funcnames[j] IsNotNull
							Then
								Lookup.ord_or_rva = hintPos + raw ;
								Lookup2.ord_or_rva = hintPos + raw ;
								*(bit_16_ptr)(Sect.data + hintPos) = 0; // modules[i].ordinals[j] ;
								hintPos += 2 ;
        				memcpy(Sect.data + hintPos,String(modules[i].funcnames[j]),Length(modules[i].funcnames[j])) ;
								hintPos += Length(modules[i].funcnames[j]) + 1;
								If hintPos And 1
									Then
										hintPos++ ;
									EndIf;
							Else
								Lookup.ord_or_rva = Lookup2.ord_or_rva = modules[i].ordinals[j];
								Lookup.import_by_ordinal = 1 ;
								Lookup2.import_by_ordinal = 1 ;
							EndIf ;

							/* now make a thunk that has nothing to do with PE files
							 * but it allows direct jumps to import functions rather than
							 * having the compiler generate indirect calls.
						         * this is desirable to keep from having to prototype everything
							 */
//                     If modules[i].pubs[j]->isdata 
//                       Then
//                         modules[i].pubs[j]->Internal.offset =
//                                 (byte_ptr)lookup - (byte_ptr)Sect.data + Sect.base ;
//                         enter_pe_fixup(modules[i].pubs[j]->Internal.offset, offset32_location) ;
//                       Else
                         modules[i].pubs[j]->Internal.offset = impThunkAddress ; /* Store the offset to our generated thunk table... */
                         enter_pe_fixup(impThunkAddress + import_thunk_table->address+2, offset32_location) ;
                         *(bit_16_ptr)(impThunkAddress + import_thunk_table->data) = 0x25FF ; /* jump indirect */
                         impThunkAddress += 2 ;
                         *(bit_32_ptr)(impThunkAddress + import_thunk_table->data) = 
                                 (byte_ptr)lookup - (byte_ptr)Sect.data + Sect.base ;
                         impThunkAddress += 4 ;
						 if (impThunkAddress > import_thunk_table->highest_uninitialized_byte)
						 {
							 printf("table overflow");
							 exit(1);
						 }
							
							lookup++ ;
							lookup2++;
								
					EndFor ;	
				lookup++;
				lookup2++ ;
				
			EndFor ;

    k=Sect.length;
    k+=objectAlign.val-1;
    k&=(0xffffffff-(objectAlign.val-1));
    Sect.virtualSize=k;

	  ObjectTable.virtual_size = k ;
    ObjectTable.raw_size = (Sect.length + fileAlign.val - 1) & -fileAlign.val  ;

    PEHead.import_rva = outList[sectNum]->base-imageBase.val ;
    PEHead.import_size = outList[sectNum]->length ;

		apply_pe_import_fixups(objectTable);
}
#undef ImpDir
#undef PEHead
#undef ObjectTable 
#undef LastObjectTable 
#undef Sect 
#undef Lookup
#undef Lookup2

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             enter_pe_Fixup                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void enter_pe_fixup(bit_32 address,loc_type location) 
BeginDeclarations
EndDeclarations
BeginCode
	If pefile.val IsTrue
    Then
    	If location IsNotEqualTo offset32_location AndIf location IsNotEqualTo secondary_offset32_location
				AndIf location IsNotEqualTo pointer32_location
		    Then
			    linker_error(4,"PE Fixup error:\n"
                        " Error:  segment-relative fixups in PE files must be 32 bits\n");
    			return ;
		    EndIf ;
    	If n_pe_fixups IsEqualTo max_pe_fixups
		    Then
    			max_pe_fixups += 10000 ;
		    	pe_fixup_array = (fixup_hold_ptr_array)reallocate_memory(pe_fixup_array, max_pe_fixups*sizeof(fixup_hold_ptr)) ;
		    EndIf ;
			pe_fixup_array[n_pe_fixups] = (fixup_hold_ptr)allocate_memory(sizeof(fixup_hold_type));
			pe_fixup_array[n_pe_fixups]->offset = address ;
			pe_fixup_array[n_pe_fixups++]->type = PE_FIXUP_HIGHLOW ;
    EndIf ;	
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                SortFixups                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static int sortcmp(fixup_hold_ptr *left, fixup_hold_ptr *right)
{
    if ((*left)->offset < (*right)->offset)
        return -1;
    if ((*left)->offset > (*right)->offset)
        return 1;
    return 0;        
}
static void SortFixups(void)
BeginDeclarations
  bit_32 i,j;
  fixup_hold_ptr temp ;
EndDeclarations
BeginCode
  qsort(pe_fixup_array,n_pe_fixups,sizeof(void *),(int (*)(const void *, const void *))sortcmp);
#ifdef XXXXX
  For i=0 ; i < n_pe_fixups; i++
    BeginFor
      For j=i+1; j < n_pe_fixups; j++
        BeginFor
          If pe_fixup_array[i]->offset > pe_fixup_array[j]->offset
            Then
              temp = pe_fixup_array[i] ;
              pe_fixup_array[i] = pe_fixup_array[j] ;
              pe_fixup_array[j] = temp ;
            EndIf ;
        EndFor ;
    EndFor ;
#endif
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              BuildPERelocs                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void BuildPERelocs(bit_32 sectNum, pe_object_ptr objectTable, pe_header_ptr pehead)
BeginDeclarations
  fixup_block_type block ;
  int_32 pos ;
  int_32 base ;
  int_32 current ;
  pe_section_ptr sect ;
  int_32 k ;
  pe_object_ptr lastObjectTable ;
  int_32 current_size;
#define PEHead (*pehead)
#define LastObjectTable (*lastObjectTable)
#define ObjectTable (*objectTable) 
#define Sect (*sect)
EndDeclarations
BeginCode
  SortFixups() ;
  pos = 0;
  current_size = 0;
  sect = outList[sectNum] ;

  // Make winnt happy if no fixups 
  If n_pe_fixups IsZero
    Then
      block.rva = 0 ;
      block.size = 12;
      block.data[0] = block.data[1] = 0;
      Sect.data = allocate_memory(block.size) ;
      memcpy(Sect.data, &block, block.size);
      Sect.length = Sect.initlength = block.size ;
    EndIf ;
  While pos LessThan n_pe_fixups
    BeginWhile
      base = pe_fixup_array[pos]->offset And Complement 4095;
      current = 0;
      block.rva = base - imageBase.val ;
      block.size = 8 ;
      While pos LessThan n_pe_fixups AndIf base IsEqualTo (pe_fixup_array[pos]->offset And Complement 4095)
        BeginWhile
          block.size += 2 ;
          block.data[current++] = (pe_fixup_array[pos]->offset And 4095) Or (pe_fixup_array[pos]->type ShiftedLeft 12);
  		  pos++ ;
        EndWhile ;
      If block.size And 3
        Then
          block.size += 2 ;
          block.data[current++] = 0;
        EndIf ;
      if (Sect.length + block.size >= current_size) {  
          current_size += (n_pe_fixups - pos) * (8 + 1024) + 10000 ;
          if (current_size < Sect.length + block.size) // should never happen
            current_size = Sect.length + block.size;
        Sect.data = reallocate_memory(Sect.data, current_size) ;
      }
      memcpy(Sect.data + Sect.length, &block, block.size);
      Sect.initlength = Sect.length += block.size ;
// fixme max lengths 

    EndWhile ;
    lastObjectTable = objectTable + sectNum-1 ;
    objectTable = lastObjectTable + 1;

    
    k=Sect.length + objectAlign.val-1 ;
    k&=(0xffffffff-(objectAlign.val-1));
    Sect.virtualSize=k;
    ObjectTable.virtual_size = k ;
    ObjectTable.raw_size = (Sect.length +fileAlign.val -1 ) & -fileAlign.val;

    k = LastObjectTable.raw_size  + LastObjectTable.raw_ptr + fileAlign.val - 1;
    k&=(0xffffffff-(fileAlign.val-1)); /* aligned */

    /* k is now physical location of this object */
    ObjectTable.raw_ptr = k ;

    k = ObjectTable.virtual_addr = LastObjectTable.virtual_size + LastObjectTable.virtual_addr ;

    Sect.base=k+imageBase.val; /* relocate section */
		PEHead.fixup_rva  = Sect.base - imageBase.val ;
		PEHead.fixup_size  = Sect.length;

    return;
}
#undef ObjectTable
#undef Sect
#undef PEHead


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             BuildPEExports                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
int expfunc(const void *a, const void *b)
{
    return strcmp(String(((public_entry_ptr)*(void **)a)->entryident), String(((public_entry_ptr)*(void **)b)->entryident));
}
void BuildPEExports(int_32 sectNum,pe_object_ptr objectTable, pe_header_ptr pehead)
BeginDeclarations
    bit_32 i,j,k;
    pe_section_ptr  sect;
    bit_32 nameLen;
    bit_32 numNames=0;
    bit_32 RVAStart;
    bit_32 nameRVAStart;
    bit_32 ordinalStart;
    bit_32 nameSpaceStart;
    bit_32 minOrd;
    bit_32 maxOrd;
    bit_32 numOrds;
    pe_object_ptr lastObjectTable ;
		public_entry_ptr pub ;
		bit_32_ptr ordinals ;
		bit_16_ptr ordinals2 ;
		bit_32_ptr rvas ;
		export_header_ptr exportHeader ;
		byte_ptr nm,nm1,nm2 ;
		public_entry_ptr_array nameList ;
#define PEHead (*pehead)
#define Sect (*sect)
#define ObjectTable (*objectTable)
#define Pub (*pub)
#define LastObjectTable (*lastObjectTable)
#define ExportHeader (*exportHeader)
EndDeclarations
BeginCode

		ReturnIf (Not n_exports OrIf sectNum < 0) ;

		For i=0; i < n_exports; i++
			BeginFor
			  For j = i+1; j < n_exports; j++
			    BeginFor
				  If exports[i] IsEqualTo exports[j]
				    Then
					  memcpy( exports +j, exports + j + 1, (n_exports - j - 1) * sizeof(exports[0]));
					  --n_exports;
					EndIf
				EndFor
            EndFor
    sect=outList[sectNum];
		lastObjectTable = objectTable + sectNum - 1;
		objectTable = lastObjectTable + 1;

		nm = exe_file_list.first->filename ;
		nm1 = strrchr(nm,'\\') ;
		If nm1 IsNotNull
			Then
			  nm = nm1 + 1;
			Else
				nm1 = strrchr(nm,':');
				If nm1 IsNotNull
					Then
						nm = nm1 + 1;
					EndIf ;
			EndIf ;
		nameLen = strlen(nm) ;

    Sect.initlength = Sect.length=sizeof(export_header_type)+nameLen+1;
    /* min section size= header size + num exports * pointer size */
    /* plus space for null-terminated name */

    minOrd=0xffffffff; /* max ordinal num */
    maxOrd=0;
    numOrds=n_exports ;

		For i=0; i < n_exports; i++
			BeginFor
				pub = exports[i] ;
				If Length(Pub.entryident) IsNotZero
					Then
						Sect.initlength = Sect.length += Length(Pub.entryident) + 1 + 6;
						numNames++ ;
					EndIf ;
				If Pub.ordinal IsNotEqualTo 0xffffffff
					Then
						If Pub.ordinal LessThan minOrd
							Then
								minOrd = Pub.ordinal ;
							EndIf ;
						If Pub.ordinal GreaterThan maxOrd
							Then
								maxOrd = Pub.ordinal ;
							EndIf ;
					EndIf ;
			EndFor ;

		If maxOrd GreaterThanOrEqualTo minOrd
			Then
				numOrds = numOrds > (maxOrd-minOrd + 1) ? numOrds : maxOrd-minOrd + 1;
			Else
				minOrd = maxOrd = 1 ;
			EndIf ;
		
		Sect.initlength = Sect.length += 4* numOrds ;
    exportHeader = (export_header_ptr) (Sect.data= allocate_memory(Sect.length)) ;

		
    k=Sect.length+objectAlign.val-1;
    k&=(0xffffffff-(objectAlign.val-1));
    Sect.virtualSize = ObjectTable.virtual_size = k;
		ObjectTable.raw_size = (Sect.length + fileAlign.val - 1 ) & -fileAlign.val;

		k = LastObjectTable.raw_size + LastObjectTable.raw_ptr + fileAlign.val - 1;
    k&=(0xffffffff-(fileAlign.val-1)); /* aligned */
		ObjectTable.raw_ptr = k ;

    k = ObjectTable.virtual_addr = LastObjectTable.virtual_addr + LastObjectTable.virtual_size ;

    Sect.base=k+imageBase.val; /* relocate section */

		memset(Sect.data,0,Sect.length) ;

		ExportHeader.time = time(0) ;
		ExportHeader.ord_base = minOrd ;
		ExportHeader.n_eat_entries = numOrds ;
		ExportHeader.n_name_ptrs = numNames ;

    RVAStart=sizeof(export_header_type); /* start address of RVA table */
    nameRVAStart=RVAStart+numOrds*4; /* start of name table entries */
    ordinalStart=nameRVAStart+numNames*4; /* start of associated ordinal entries */
    nameSpaceStart=ordinalStart+numNames*2; /* start of actual names */

		ExportHeader.address_rva = RVAStart + Sect.base - imageBase.val ;
		ExportHeader.name_rva = nameRVAStart + Sect.base - imageBase.val ;
		ExportHeader.ordinal_rva = ordinalStart + Sect.base - imageBase.val ;

    If numNames
     Then
		/* sort */
		nameList = (public_entry_ptr_array)allocate_memory(numNames * sizeof (public_entry_ptr));
        j=0; /* no entries yet */
		For i=0; i < n_exports; i++
			BeginFor
				pub = exports[i] ;
				If Length(Pub.entryident)
					Then ;
						nameList[j++] = pub ;
					EndIf ;
			EndFor ;
       qsort(nameList,numNames,sizeof(nameList[0]),expfunc);
     EndIf
    /* process numbered exports */
		rvas = (bit_32_ptr)(Sect.data + RVAStart) ;
    For i=0; i LessThan n_exports; i++
			BeginFor
				pub = exports[i] ;
				If Pub.type_entry Is external
				 Then
								linker_error(4,"Export error:\n"
														"\tExport:  \"%s\"\n"
														"\t   Ord: %08lx\n"
														"\t Error: Export entry point \n"
														"\t        does not exist\n",
														String(Pub.entryident),
														Pub.ordinal);
				 EndIf
				If Pub.ordinal IsNotEqualTo 0xffffffff
					Then
						k = rvas[Pub.ordinal - minOrd] ;
						If k IsNotZero
							Then
								linker_error(4,"Export error:\n"
														"\tExport:  \"%s\"\n"
														"\t   Ord: %08lx\n"
														"\t Error: Ordinal already assigned to \n"
														"\t        another export\n",
														String(Pub.entryident),
														Pub.ordinal);
							Else
   							  If Pub.Internal.lseg IsNull
    							Then
     								k = (Bit_32(Pub.Internal.frame) ShiftedLeft 4);
    							Else
     								k = (*Pub.Internal.lseg).address;
    							EndIf;
						 	  k = k + Pub.Internal.offset ;
							  rvas[Pub.ordinal - minOrd] = k -imageBase.val;
							EndIf ;
					EndIf ;
			EndFor ;
			
				
    /* process non-numbered exports */
		For i=0,j=0; i < n_exports; i++
			BeginFor
				pub = exports[i] ;
				If Pub.ordinal IsEqualTo 0xffffffff
					Then
						While rvas[j]
							BeginWhile
								j++ ;
							EndWhile ;
						
						Pub.ordinal = j + minOrd ;
						If Pub.Internal.lseg IsNull
							Then
 								k = (Bit_32(Pub.Internal.frame) ShiftedLeft 4);
							Else
 								k = (*Pub.Internal.lseg).address;
							EndIf;
						k = k + Pub.Internal.offset ;
						rvas[j++] = k -imageBase.val;
					EndIf
      EndFor ;

		If numNames
			Then
				nm1 = Sect.data + nameSpaceStart ;
				ordinals = (bit_32_ptr)(Sect.data + nameRVAStart) ;
				ordinals2 =  (bit_16_ptr)(Sect.data + ordinalStart) ;
			
        /* and store */
				For i=0; i < numNames; i++
					BeginFor
						pub = nameList[i] ;
						*(ordinals2)++ = Pub.ordinal - minOrd ;
						*(ordinals)++ = nm1 - Sect.data + Sect.base - imageBase.val ;
						nm2 = String(Pub.entryident);
					    For j=0; j < Length(Pub.entryident); j++
							BeginFor
								*nm1++ =*nm2++ ;
							EndFor ;
						*nm1++ = 0;
					EndFor
			EndIf ;

		If nameLen IsNotZero
			Then
				ExportHeader.exe_name_rva = nm1 - Sect.data + Sect.base - imageBase.val ;
				For j=0; j < nameLen; j++
					BeginFor
						*nm1++ =toupper(*nm++) ;
					EndFor ;
				*nm1++ = 0 ;
			EndIf ;
	PEHead.export_rva  = Sect.base - imageBase.val ;
	PEHead.export_size  = Sect.length;
EndCode ;
#undef ObjectTable
#undef Pub
#undef LastObjectTable
#undef ExportHeader
#undef Sect
#undef PEHead


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              CountResource                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void CountResource(resource_data_ptr resource, bit_32_ptr dirs, bit_32_ptr entries, bit_32_ptr data_headers, bit_32_ptr datasize, bit_32_ptr name_size, bit_32 typeorhead)
BeginDeclarations
  resource_data_ptr res ;
  #define Resource (*resource)
EndDeclarations
BeginCode
  If typeorhead OrIf (Resource.name_count + Resource.ident_count) IsNotZero
    Then
      (*dirs)++;
    EndIf ;
  *entries += Resource.name_count + Resource.ident_count ;
  If Resource.length 
    Then
      (*data_headers) ++ ;
      (*datasize) += (Resource.length + 3) & 0xFFFFFFFC ;
    EndIf ;
	If Resource.name
		Then
			(*name_size) += wstrlen(Resource.name) * 2 + 2 ;
		EndIf ;
  TraverseList(Resource.name_list,res)
    BeginTraverse
      CountResource(res,dirs,entries,data_headers,datasize, name_size, resource IsEqualTo &resource_head) ;
    EndTraverse ;
  TraverseList(Resource.ident_list,res)
    BeginTraverse
      CountResource(res,dirs,entries,data_headers,datasize, name_size, resource IsEqualTo &resource_head) ;
    EndTraverse ;

EndCode
#undef Resource


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              DumpResources                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void DumpResources(byte_ptr data, resource_data_ptr resource, bit_32_ptr dir_start, bit_32_ptr data_header_start, bit_32_ptr name_start, bit_32_ptr data_start, bit_32 va)
BeginDeclarations
	resource_data_ptr res ;
  resource_dir_table_ptr dirhead ;
	resource_dir_entry_ptr direntries ;
  resource_data_entry_ptr datahead ;
  bit_32 i;
#define DirHead (*dirhead)
#define DirEntries (*direntries)
#define Res (*res)
#define DataHead (*datahead)
#define Resource (*resource)

EndDeclarations
BeginCode
	If Resource.name_count + Resource.ident_count IsNotZero
		Then
			dirhead = (resource_dir_table_ptr)(data + *dir_start) ;
			direntries = (resource_dir_entry_ptr)(dirhead + 1) ;
			*dir_start = (byte_ptr) (direntries + (Resource.name_count + Resource.ident_count)) - data;
			DirHead.time = time(0) ;
			DirHead.name_entry = Resource.name_count ;
			DirHead.ident_entry = Resource.ident_count ;
			TraverseList(Resource.name_list,res)
				BeginTraverse
					DirEntries.rva_or_id = *name_start + 0x80000000 ;
					DirEntries.subdir_or_data = (bit_32)*dir_start ;
					DirEntries.escape = 1 ; /* Subdir */
					direntries++ ;
					*(bit_16_ptr)(data + *name_start) = i = wstrlen(Res.name) ;
					wstrcpy((bit_16_ptr)(data + *name_start + 2),Res.name) ;
					*name_start += i*2+2 ;
					DumpResources(data,res,dir_start, data_header_start, name_start, data_start,va) ;
				EndTraverse
			TraverseList(Resource.ident_list,res)
				BeginTraverse
					DirEntries.rva_or_id = Res.id ;
					DirEntries.escape = (Res.name_count + Res.ident_count) ? 1 : 0 ;
					If DirEntries.escape
						Then
							DirEntries.subdir_or_data = (bit_32)*dir_start;
						Else
							DirEntries.subdir_or_data = (bit_32)*data_header_start;
						EndIf ;
					direntries++ ;
					DumpResources(data,res,dir_start, data_header_start, name_start, data_start,va) ;
				EndTraverse
		Else
			If Resource.data IsNull
				Then
					If Resource.name_count
						Then
							res = Resource.name_list.first ;
						Else
							res = Resource.ident_list.first ;
						EndIf ;
				EndIf ;
			datahead = (resource_data_entry_ptr)(data + *data_header_start) ;
			*data_header_start += sizeof (resource_data_entry_type) ;
			memcpy(data + *data_start, Resource.data, Resource.length) ;
			DataHead.rva = *data_start + va ;
			*data_start += (Resource.length + 3) & 0xfffffffc ;
			DataHead.size = Resource.length ;
			DataHead.codepage = 0;
			DataHead.reserved = 0 ;
		EndIf ;
EndCode
#undef DirHead
#undef DirEntries
#undef Res
#undef DataHead
#undef Resource


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            BuildPEResources                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void BuildPEResources(int_32 sectNum,pe_object_ptr objectTable, pe_header_ptr pehead)
BeginDeclarations
  bit_32 num_dirs = 1 ;
  bit_32 num_dir_entries =0 ;
  bit_32 num_data_headers =0 ;
  bit_32 data_size = 0 ;
  bit_32 name_size = 0 ;
  bit_32 i,j,k ;
  bit_32 dir_start = 0;
  bit_32 data_start ;
  bit_32 data_header_start ;
  bit_32 name_start ;
  bit_32 total_size ;
  pe_object_ptr lastObjectTable ;
  pe_section_ptr  sect ;
#define ObjectTable (*objectTable)
#define LastObjectTable (*lastObjectTable)
#define Sect (*sect)
#define PEHead (*pehead)
EndDeclarations
BeginCode
  ReturnIf(sectNum LessThan 0 OrIf Not n_resources) ;

  CountResource(&resource_head,&num_dirs, &num_dir_entries, &num_data_headers, & data_size, &name_size, True) ;
  data_header_start = num_dirs * sizeof(resource_dir_table_type) +
                      num_dir_entries * sizeof(resource_dir_entry_type) ;
  name_start = data_header_start + num_data_headers * sizeof(resource_data_entry_type) ;
  data_start = (name_start + name_size  + 3) & 0xfffffffc;
  total_size = (data_start + data_size + 3) & 0xfffffffc;

  lastObjectTable = objectTable + sectNum - 1;
  objectTable = lastObjectTable + 1 ;
  k = LastObjectTable.raw_ptr + LastObjectTable.raw_size +fileAlign.val-1 ;
  k&=(0xffffffff-(fileAlign.val-1)); /* aligned */
  ObjectTable.raw_ptr = k ;
  k = ObjectTable.virtual_addr = LastObjectTable.virtual_addr + LastObjectTable.virtual_size ;

  sect = outList[sectNum] ;
  Sect.length = Sect.initlength = total_size ;
  Sect.data = allocate_memory(total_size) ;
  Sect.base = k + imageBase.val ;
  k=total_size + objectAlign.val - 1;
  k&=(0xffffffff-(objectAlign.val-1));
  Sect.virtualSize=ObjectTable.virtual_size = k;
  ObjectTable.raw_size = (total_size + fileAlign.val -1) & -fileAlign.val;

  DumpResources(Sect.data, &resource_head, &dir_start, &data_header_start, &name_start, &data_start, ObjectTable.virtual_addr) ;

	PEHead.resource_rva  = Sect.base - imageBase.val ;
	PEHead.resource_size  = Sect.length;
EndCode
#undef PEHead
#undef Sect
#undef ObjectTable
#undef LastObjecTable


void DumpPEDebug(int_32 sectNum,pe_object_ptr objectTable, pe_header_ptr pehead)
BeginDeclarations
  bit_32                                  total_size ;
  byte_ptr                                data ;
EndDeclarations
BeginCode
  ReturnIf (debug.val IsFalse) ;
  create_codeview_data(pehead,  &data, &total_size) ;
  file_open_for_write(cv_file_list.first);
  file_write(data+32, total_size-32) ;
  file_close_for_write() ;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            BuildPEDebug                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void BuildPEDebug(int_32 sectNum,pe_object_ptr objectTable, pe_header_ptr pehead)
BeginDeclarations
  bit_32                                  k ;
  bit_32                                  total_size = 260 + 33;
  bit_8                                	  *data;
  pe_object_ptr                           lastObjectTable ;
  pe_section_ptr                          sect ;
#define ObjectTable (*objectTable)
#define LastObjectTable (*lastObjectTable)
#define Sect (*sect)
#define PEHead (*pehead)
EndDeclarations
BeginCode
  ReturnIf(sectNum LessThan 0) ;

  ReturnIf (debug.val IsFalse) ;
  data = allocate_memory(total_size);
  data[0] = 'L';
  data[1] = 'S';
  data[2] = '1';
  data[3] = '2';

  lastObjectTable = objectTable + sectNum - 1;
  objectTable = lastObjectTable + 1 ;
  k = LastObjectTable.raw_ptr + LastObjectTable.raw_size + fileAlign.val-1 ;
  k&=(0xffffffff-(fileAlign.val-1)); /* aligned */
  ObjectTable.raw_ptr = k ;
  k = ObjectTable.virtual_addr = LastObjectTable.virtual_addr + LastObjectTable.virtual_size ;

  sect = outList[sectNum] ;
  Sect.length = Sect.initlength = total_size ;
  Sect.data = data;
  memcpy(Sect.data, data, total_size);
  Sect.base = k + imageBase.val ;
  k=total_size + objectAlign.val - 1;
  k&=(0xffffffff-(objectAlign.val-1));
  Sect.virtualSize=ObjectTable.virtual_size = k;
  ObjectTable.raw_size = (total_size + fileAlign.val - 1 ) & -fileAlign.val;

  PEHead.debug_rva  = Sect.base - imageBase.val ;
  PEHead.debug_size  = Sect.length;

  *(bit_32 *)(data + 4) = PEHead.time;
  *(bit_32 *)(data + 16) = total_size - 32 ;
  *(bit_32 *)(data + 20) = ObjectTable.virtual_addr + 0x20 ;
  *(bit_32 *)(data + 24) = ObjectTable.raw_ptr + 0x20 ;
  data[32] = strlen(cv_file_list.first->filename);
  strcpy(data + 33, cv_file_list.first->filename);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 load_stub                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void load_stub(byte_ptr *pstubData,bit_32_ptr pstubSize)
BeginDeclarations
    EXE_header_type headbuf;
    byte_ptr headerdata;
    bit_32 imageSize;
    bit_32 headerSize;
    bit_32 relocSize;
    bit_32 relocStart;
    int_32 i;
EndDeclarations
BeginCode

    If stub_file_list.first IsNotNull
      Then
        file_open_for_read(stub_file_list.first) ;
        file_read((byte_ptr)&headbuf,sizeof(headbuf)) ;
        If headbuf.signature IsNotEqualTo 0x5a4d
          Then
            linker_error(4,"Invalid EXE Stub File:\n"
                          "\t  File:  \"%s\"\n"
                          "\tOffset:  %08lx\n"
                          "\t Error:  Invalid header\n",
                          stub_file_list.first->filename,0L) ;
            file_close_for_read() ;
            return ;
        	EndIf ;
        /* get size of image */
        imageSize=headbuf.image_length_MOD_512+(headbuf.image_length_DIV_512<<9);
        If (imageSize Mod 512) IsNotZero 
          Then
            imageSize-=512;
          EndIf ;
        headerSize=(headbuf.n_header_paragraphs)<<4;
        relocSize= headbuf.n_relocation_items << 2;
        imageSize-=headerSize;

        /* allocate buffer for load image */
        headerdata=allocate_memory(imageSize+0x40+((relocSize+15) And Complement 15));

        relocStart=headbuf.offset_to_relocation_table ;
        /* load relocs */
        file_position(relocStart) ;
        file_read(headerdata+0x40,relocSize) ;

        /* paragraph align reloc size */
        relocSize += 15;
        relocSize &= Complement 15;

        /* move to start of data */
        file_position(headerSize) ;

        /* new header is 4 paragraphs long + relocSize*/
        headbuf.offset_to_relocation_table = 0x40 ;       
		headerSize = relocSize + 0x40;
        headbuf.n_header_paragraphs = headerSize >> 4;

        /* load data into correct position */
        file_read(headerdata+headerSize,imageSize) ;

        imageSize+=headerSize; /* total file size */

        headbuf.image_length_MOD_512 = imageSize%512; /* size mod 512 */
        headbuf.image_length_DIV_512 = (imageSize+511)>>9; /* number of 512-byte pages */

        /* copy header */
        memcpy(headerdata,(byte_ptr)&headbuf,sizeof(headbuf)) ;

        /* store pointer and size */
        (*pstubData)=headerdata;
        (*pstubSize)=imageSize;

      Else
	  	char *p = "This program was compiled with CC386/VALX.  It requires "
				  "WIN32 or HXDOS to run\r\n$";
        (*pstubData)=defaultStub;
        (*pstubSize)=defaultStubSize;
		memcpy(defaultStub + 0x4e, p, strlen(p));
      EndIf ;
EndCode


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           CreateImportLibrary                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void CreateImportLibrary(void)
BeginDeclarations
 string_ptr                     workname;
 string_ptr                     filename;
 string_ptr                     deffilename;
 string_ptr                     exefilename;
 bit_32                         n;
EndDeclarations
BeginCode
  filename = allocate_string(512);
  copy_string(filename, string((*exe_file_list.first).filename));
  n = index_string(filename, 0, dot_string);	
  If n IsNot 0xffff
   Then
    cut_string(filename, n,1000);
   EndIf
  add_extension_to_file(filename, lib_extension_string);
  unlink(String(filename));
  workname         = allocate_string(512);
  copy_string(workname, string("\""));
  concat_string(workname, filename);
  concat_string(workname, string("\""));
  If def_file_list.first IsNotNull
   Then
    deffilename         = allocate_string(512);
    copy_string(deffilename, string("\""));
	concat_string(deffilename, string((*def_file_list.first).filename));
	concat_string(deffilename, string("\""));
    spawnlp(P_WAIT, "import", "import", String(workname), String(deffilename), NULL);
   Else
    exefilename         = allocate_string(512);
    copy_string(exefilename, string("\""));
	concat_string(exefilename, string((*exe_file_list.first).filename));
	concat_string(exefilename, string("\""));
    spawnlp(P_WAIT, "import", "import", String(workname), String(exefilename), NULL);
   EndIf
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              write_pe_image                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void write_pe_image(void)
BeginDeclarations
    pe_change_attribs_ptr change_attribs;
    bit_32 i,j,k;
    byte_ptr headbuf;
    pe_header_ptr pehead ;
#define PEHead (*pehead)
    pe_object_ptr peobject ;
#define PEObject (*peobject)
    byte_ptr stubData;
    FILE *outfile;
    bit_32 headerSize;
    bit_32 headerVirtSize;
    bit_32 stubSize;
    bit_32 sectionStart;
    bit_32 headerStart;
    int_32 relocSectNum,importSectNum,exportSectNum,resourceSectNum,cvSectNum;
    bit_32 written ;
    bit_32 highest_data_sect = n_pe_sections - 1 ;
    bit_32 current_section ;
    public_entry_ptr import ;
    segment_entry_ptr seg;
    lseg_ptr lseg ;
    bit_32 data_index ;
    bit_32 partial_length ;
#define Seg (*seg )
#define Lseg (*lseg)
EndDeclarations
BeginCode

    ReturnIf (!n_pe_sections) ;

	if (n_pe_sections == 3)
	{
		outList[1]->length += outList[2]->length;
		outList[1]->virtualSize += outList[2]->virtualSize;
		n_pe_sections--;
		highest_data_sect--;
	}
    /* allocate section entries for imports, exports and relocs if required */
    If n_imports
      Then
        importSectNum=createOutputSection(".idata",
            WINF_INITDATA | WINF_READABLE | WINF_WRITEABLE | WINF_NEG_FLAGS);
      Else
        importSectNum=-1;
      EndIf ;

    If n_exports
      Then
        exportSectNum=createOutputSection(".edata",
            WINF_INITDATA | WINF_READABLE | WINF_NEG_FLAGS);
      Else
        exportSectNum=-1;
      EndIf ;

    /* Windows NT requires a reloc section to relocate image files, even */
    /* if it contains no actual fixups */
    relocSectNum=createOutputSection(".reloc",
        WINF_INITDATA | WINF_SHARED | WINF_READABLE | WINF_NEG_FLAGS);

    If n_resources
      Then
        resourceSectNum=createOutputSection(".rsrc",
            WINF_INITDATA | WINF_READABLE | WINF_WRITEABLE | WINF_NEG_FLAGS);
      Else
        resourceSectNum=-1;
      EndIf ;

    If debug.val
     Then
      cvSectNum = createOutputSection(".debug",
            WINF_INITDATA | WINF_SHARED | WINF_READABLE) ;
     Else
       cvSectNum = -1;
     EndIf
    /* build header */
    load_stub(&stubData,&stubSize);

    headerStart=stubSize; /* get start of PE header */
    headerStart+=7;
    headerStart&=0xfffffff8; /* align PE header to 8 byte boundary */

    headerSize=sizeof(pe_header_type)+n_pe_sections*sizeof(pe_object_type)+stubSize + PE_OPTIONAL_HEADER_SIZE ;
    headerSize+=(fileAlign.val-1);
    headerSize&=(0xffffffff-(fileAlign.val-1));
    headerVirtSize=headerSize+(objectAlign.val-1);
    headerVirtSize&=(0xffffffff-(objectAlign.val-1));


    headbuf=allocate_memory(headerSize);

    memset(headbuf,0,headerSize);

    memcpy(headbuf,stubData,stubSize ) ;

    *(bit_32_ptr)( headbuf + 0x3c) = headerStart ; /* Pointer to PE header */
    pehead = (pe_header_ptr)(headbuf + headerStart) ;

    PEHead.sig = 0x00004550 ; /* PE__ */
    PEHead.cpu_type = PE_INTEL386 ;
    /* store time/date of creation */
    PEHead.time=(bit_32)time(NULL);
    PEHead.nt_hdr_size = PE_OPTIONAL_HEADER_SIZE ;

//    i=PE_FILE_EXECUTABLE | PE_FILE_32BIT | PE_FILE_REVERSE_BITS_HIGH |
//				PE_FILE_REVERSE_BITS_LOW | PE_FILE_LOCAL_SYMBOLS_STRIPPED | PE_FILE_LINE_NUMBERS_STRIPPED  ;
	i = 0x30e;
    If build_DLL.val IsTrue
      Then
        i|= PE_FILE_LIBRARY;                /* if DLL, flag it */
      EndIf ;
    PEHead.flags = i ;

    PEHead.magic = PE_MAGICNUM ;

    PEHead.image_base = imageBase.val ;

    PEHead.file_align = fileAlign.val ;
    PEHead.object_align = objectAlign.val ;

    PEHead.os_major_version = osMajor ;
    PEHead.os_minor_version = osMinor ;
    PEHead.user_major_version = userMajor ;
    PEHead.user_minor_version = userMinor ;
    PEHead.subsys_major_version = subsysMajor ;
    PEHead.subsys_minor_version = subsysMinor ;
    PEHead.subsystem = win_subsystem.val ? PE_SUBSYS_WINDOWS : PE_SUBSYS_CONSOLE ;

    PEHead.num_rvas = PE_NUM_VAS ;
    PEHead.header_size = headerSize;
    PEHead.heap_size = heapSize.val ;
    PEHead.heap_commit = heapCommitSize.val ;
    If build_DLL.val IsFalse
	 Then
      PEHead.stack_size = stackSize.val ;
      PEHead.stack_commit = stackCommitSize.val ;
	 EndIf;


    /* shift segment start addresses up into place and build section headers */
    sectionStart=headerSize ;
		peobject = (pe_object_ptr)(headbuf + headerStart + sizeof(pe_header_type)) ;

    // So we don't write the BSS to the file
    If PE_bss_seg IsNotZero
     Then
      outList[PE_bss_seg]->initlength = 0;
     EndIf ;

    TraverseList( pe_change_attribs, change_attribs)
	 BeginTraverse
	  For i=0; i < n_pe_sections; i++
	   BeginFor
	    If Not strnicmp(outList[i]->name, change_attribs->name->text, change_attribs->name->length)
		 Then
		  outList[i]->winFlags &= ~(WINF_READABLE | WINF_WRITEABLE | WINF_EXECUTE | WINF_SHARED);
		  outList[i]->winFlags |= change_attribs->flags;
		  ExitLoop;
		 EndIf
	   EndFor
	  If i >= n_pe_sections
	   Then
          linker_error(4, "PE segment \"%s\" not located.  Flags could not be changed\n",
					     String(change_attribs->name));
	    
	   EndIf
	 EndTraverse;
    For i=0;i<n_pe_sections;i++,peobject++
      BeginFor
				memcpy(PEObject.name,outList[i]->name,8) ;
				PEObject.virtual_size = k = outList[i]->virtualSize ;

        If Not pad_segments.val /* if not padding segments, reduce space consumption */
          Then 
						k = PEObject.raw_size = (outList[i]->initlength  + fileAlign.val - 1) & -fileAlign.val;
					Else
						PEObject.raw_size = k ;
          EndIf ;

				PEObject.raw_ptr = sectionStart ;


        k+=fileAlign.val-1;
        k&=(0xffffffff-(fileAlign.val-1)); /* aligned initialised length */
        sectionStart+=k; /* update section start address for next section */

//        outList[i]->base+=headerVirtSize;
				PEObject.virtual_addr = outList[i]->base - imageBase.val ;
//				outList[i]->base += imageBase.val ;

        PEObject.flags = (outList[i]->winFlags ^ WINF_NEG_FLAGS) & WINF_IMAGE_FLAGS; /* get characteristice for section */
      EndFor ;

    PEHead.num_objects = n_pe_sections ;

    /* build import, export and relocation sections */
		peobject = (pe_object_ptr) (headbuf + headerStart + sizeof(pe_header_type)) ;
    BuildPEImports(importSectNum,peobject,pehead);
    BuildPEExports(exportSectNum,peobject,pehead);
    BuildPERelocs(relocSectNum,peobject,pehead);
    BuildPEResources(resourceSectNum,peobject,pehead);
    BuildPEDebug(  cvSectNum, peobject, pehead);

    If start_address_found IsTrue
     Then
      fixup                 = start_address;
			PEHead.entry_point    = target(&import) - imageBase.val ; /* RVA */
      If build_DLL.val IsTrue  /* if library */
        Then
          /* flag that entry point should always be called */
				  PEHead.dll_flags = 0 ;
			  EndIf ;
     Else  /* No start address found. */
      linker_error(4,"No start address.\n");
     EndIf;

		peobject = (pe_object_ptr)(headbuf + headerStart+sizeof (pe_header_type) + (n_pe_sections-1)*sizeof(pe_object_type));

		i = PEObject.virtual_addr + PEObject.virtual_size ;
		PEHead.image_size = i ;

		PEHead.code_base = outList[0]->base - imageBase.val ;
		PEHead.data_base = outList[1]->base - imageBase.val ;
		PEHead.data_size = outList[1]->virtualSize ;
		PEHead.bss_size = 0;
		peobject = (pe_object_ptr)(headbuf + headerStart+sizeof (pe_header_type));
		PEHead.code_size = outList[0]->virtualSize;
        For i=0;i<n_pe_sections;i++,peobject++
		 BeginFor
//		  If outList[i]->winFlags And WINF_INITDATA
//		   Then
//			PEHead.data_size += PEObject.raw_size;
//		   EndIf
		  If outList[i]->winFlags And WINF_UNINITDATA
		   Then
			PEHead.bss_size += (outList[i]->length + fileAlign.val - 1 ) & -fileAlign.val;
		   EndIf			  
		 EndFor
    /* zero out section start for all zero-length segments */
		peobject = (pe_object_ptr)(headbuf + headerStart + sizeof(pe_header_type)) ;
		For i=0; i LessThan n_pe_sections; i++,peobject++
			BeginFor
				If PEObject.raw_size IsZero
					Then
						PEObject.raw_ptr = 0;
					EndIf ;
			EndFor ;

    // We have to dump the codeview data now so the time stamps will be
    // correct for CCIDE
    DumpPEDebug(  cvSectNum, peobject, pehead);

		file_open_for_write(exe_file_list.first) ;
		file_write(headbuf,headerSize) ;

 /* We will use object_file_element as a source for the gaps caused by
    the alignment of segments.  We will fill the gaps with zeros. */
 current_section = 0;
 far_set(BytePtr(object_file_element), 0, MAX_ELEMENT_SIZE);
 next_available_address = outList[0]->base;
 TraverseList(segment_list, seg)
  BeginTraverse
   LoopIf((*Seg.lsegs.first).align Is absolute_segment);
   LoopIf(Seg.pe_section_number Is PE_bss_seg);
	 If current_section IsNotEqualTo Seg.pe_section_number
    Then
     If next_available_address Mod fileAlign.val IsNotZero
  		Then
	 			write_gap(fileAlign.val - (next_available_address %fileAlign.val));
	  		next_available_address += fileAlign.val - (next_available_address %fileAlign.val) ;
			EndIf ;
			
		 If pad_segments.val IsTrue
      Then
				write_gap(Seg.address-next_available_address) ;
			EndIf ;
		 current_section = Seg.pe_section_number ;
		 next_available_address = Seg.address ;
    EndIf ;

   TraverseList(Seg.lsegs, lseg)
    BeginTraverse
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
			 next_available_address = Lseg.address ;
       If Seg.combine IsNot blank_common_combine
        Then
         file_write(Lseg.data, Lseg.length);
        Else
         write_gap(Lseg.length);
        EndIf;
       next_available_address += Lseg.length;
      EndIf;

    EndTraverse;

  EndTraverse;
     If next_available_address Mod fileAlign.val IsNotZero
  		Then
	 			write_gap(fileAlign.val - (next_available_address %fileAlign.val));
	  		next_available_address += fileAlign.val - (next_available_address %fileAlign.val );
	  	EndIf ;
		 If pad_segments.val IsTrue
      Then
				write_gap(outList[highest_data_sect + 1]->base -next_available_address) ;
			EndIf ;

    For i=highest_data_sect + 1; i<n_pe_sections; i++
			BeginFor
				/* align section - for this to work objectAlign >= file_align*/
        If  pad_segments.val IsTrue AndIf i IsNotEqualTo highest_data_sect + 1
					Then
						write_gap(outList[i]->base - next_available_address);
          EndIf ;
        next_available_address=outList[i]->base;
				file_write(outList[i]->data,written = outList[i]->initlength);
				next_available_address += written ;
				If written %fileAlign.val IsNotZero
				 Then
        		  write_gap(fileAlign.val - (written % fileAlign.val)) ;
				  next_available_address += fileAlign.val - written % fileAlign.val ;
				 EndIf
			EndFor
		file_close_for_write() ;
		If build_DLL.val IsTrue
		 Then
		  CreateImportLibrary();
		 EndIf
EndCode
#undef PEObject
#undef PEHead
#undef Seg
#undef Lseg