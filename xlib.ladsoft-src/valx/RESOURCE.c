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

#include "langext.h"
#include "defines.h"
#include "types.h"
#include "subs.h"
#include "globals.h"


/* note that text in resource files is in unicode... */

static byte resource_header[32]={
					 0,0,0,0,0x20,0,0,0,0xff,0xff,0,0,0xff,0xff,0,0,
				   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

bit_32 wstrlen(bit_16_ptr str)
{
  bit_32 rv = 0;
	while (*str++)
		rv ++ ;
  return rv ;
}
void wstrcpy(bit_16_ptr dst, bit_16_ptr src)
{
  while (*src)
    *dst++ = *src++ ;
  *dst++ = *src++ ;
}
int_32 wstrcmp(bit_16_ptr arg1, bit_16_ptr arg2)
{
  while (*arg1 == *arg2 && *arg1 && *arg2)
		arg1++, arg2++ ;
  if (*arg1 < *arg2)
		return -1 ;
  if (*arg1 > *arg2)
    return 1 ;
  return 0 ;
}

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             InternalError                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void InternalError(void)
BeginDeclarations
EndDeclarations
BeginCode
         		linker_error(12, "Invalid Resource File:\n"
                       "\t  File:  \"%s\"\n"
											 "\tOffset:  %08lx\n"
                       "\t Error:  Resource file internal error.\n",
                       infile.file_info->filename,
											 infile.start_of_buffer_position + infile.bytes_left_in_buffer) ;
						file_close_for_read() ;
EndCode


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 ReadName                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static bit_32 ReadName(byte_ptr hdr, bit_32 hdrsize, bit_32 i, bit_16_ptr name, bit_32_ptr id)
BeginDeclarations
  bit_32 j;
	bit_16_ptr nm  = (bit_16_ptr)(hdr +i);
EndDeclarations
BeginCode
	If *nm IsEqualTo 0xffff
		Then
	    	*name=0;
      		*id=nm[1] ;
	    i+=4;
		Else
			*id = 0 ;
			j = wstrlen(nm) ;
			If j+i GreaterThan hdrsize
	    		Then
					InternalError();
					
					return 0;
				EndIf ;
	    wstrcpy(name,hdr+i);
       i+= j*2+2; // +3;
//       i&=0xfffffffc;
    EndIf ;
  If i GreaterThan hdrsize
	  Then
			InternalError();
			return 0;
		EndIf ;
	
	return i ;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              NewResource                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static resource_data_ptr NewResource(resource_data_list_ptr list,
										bit_16_ptr name,bit_16 id,byte_ptr data,bit_32 length)
BeginDeclarations
  resource_data_ptr resource,*xlist,llist ;
  bit_32 i ;
#define Resource (*resource )
#define List (*list)
#define LList (*llist)
EndDeclarations
BeginCode 
						resource = (resource_data_ptr) allocate_memory(sizeof(resource_data_type)) ;
						memset(resource,0,sizeof(resource_data_type)) ;
						Resource.id = id ;
						If name IsNotNull
							Then
	
								Resource.name = (bit_16_ptr)allocate_memory(i = wstrlen(name)*2 + 2) ;
								memcpy(Resource.name,name,i);
							EndIf ;
						Resource.length = length;
						Resource.data = data ;
            llist = List.first ;
            xlist = &List.first ;
						While llist IsNotNull
							BeginWhile
								If name IsNull OrIf name[0] IsZero
									Then
										If LList.name IsNotNull AndIf LList.name[0] IsNotZero
											Then
				                xlist = &(LList.next) ;
        				        llist = LList.next ;
												ContinueLoop ;
											EndIf ;
										If id LessThan LList.id
											Then
												(*xlist) = resource ;
												Resource.next = llist ;
												return resource ;
											EndIf
									Else
                    If wstrcmp(name,LList.name) LessThan 0
											Then
												(*xlist) = resource ;
												Resource.next = llist ;
												return resource ;
											EndIf
									EndIf
                xlist = &(LList.next) ;
                llist = LList.next ;
							EndWhile
						(*xlist) = resource ;
						return resource ;
EndCode
#undef Resource
#undef List
#undef LList


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  Subdir2                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void Subdir2(resource_data_ptr rdata, bit_16 id, byte_ptr data, bit_32 datasize)
BeginDeclarations
	resource_data_ptr resource;
#define Resource (*resource )
#define Rdata (*rdata)
EndDeclarations
BeginCode
				TraverseList(Rdata.ident_list,resource)
					BeginTraverse
						If id IsEqualTo Resource.id
							Then
								linker_error(4,"Resource error:\n"
														"\t  Error: two resources have same type/name/language\n"
														"           Skipping second resource\n") ;
								return ;
							EndIf ;
					EndTraverse ;
				NewResource(&Rdata.ident_list,0,id,data,datasize) ;
				Rdata.ident_count++ ;
EndCode
#undef Resource
#undef Data


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  Subdir1                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void Subdir1(resource_data_ptr rdata,bit_16_ptr name, bit_16 nameid,
											bit_16 lang, byte_ptr data, bit_32 datasize)
BeginDeclarations
	resource_data_ptr resource;
#define Resource (*resource )
#define Rdata (*rdata)
EndDeclarations
BeginCode
		If name[0] IsZero
			Then
				TraverseList(Rdata.ident_list,resource)
					BeginTraverse
						If nameid IsEqualTo Resource.id
							Then
								Subdir2(resource,lang,data,datasize) ;
								ExitLoop ;
							EndIf ;
					EndTraverse ;
				If resource IsNull
					Then
						resource = NewResource(&Rdata.ident_list,name,nameid,0,0) ;
						Rdata.ident_count ++ ;
						Subdir2(resource,lang,data,datasize) ;
					EndIf
			Else
				TraverseList(Rdata.name_list,resource)
					BeginTraverse
						If !wstrcmp(name,Resource.name)
							Then
								Subdir2(resource,lang,data,datasize) ;
								ExitLoop ;
							EndIf ;
					EndTraverse ;
				If resource IsNull
					Then
						resource = NewResource(&Rdata.name_list,name,nameid,0,0) ;
						Rdata.name_count++ ;
						Subdir2(resource,lang,data,datasize) ;
					EndIf
			EndIf
EndCode
#undef Resource
#undef Data

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         InsertResourceData                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void	InsertResourceData(byte_ptr data,bit_32 datasize,bit_16_ptr typename,
												bit_16_ptr name,bit_16 typeid,bit_16 nameid,bit_16 lang)
BeginDeclarations
	resource_data_ptr resource ;
#define Resource (*resource )
EndDeclarations
BeginCode
		If typename[0] IsZero
			Then
				TraverseList(resource_head.ident_list,resource)
					BeginTraverse
						If typeid IsEqualTo Resource.id
							Then
								Subdir1(resource,name,nameid,lang,data,datasize) ;
								ExitLoop ;
							EndIf ;
					EndTraverse ;
				If resource IsNull
					Then
						resource = NewResource(&resource_head.ident_list,typename,typeid,0,0) ;
						resource_head.ident_count++ ;
						Subdir1(resource,name,nameid,lang,data,datasize) ;
					EndIf
			Else
				TraverseList(resource_head.name_list,resource)
					BeginTraverse
						If !wstrcmp(typename,Resource.name)
							Then
								Subdir1(resource,name,nameid,lang,data,datasize) ;
								ExitLoop ;
							EndIf ;
					EndTraverse ;
				If resource IsNull
					Then
						resource = NewResource(&resource_head.name_list,typename,typeid,0,0) ;
						resource_head.name_count ++ ;
						Subdir1(resource,name,nameid,lang,data,datasize) ;
					EndIf
			EndIf
EndCode
#undef Resource

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       process_resource_files                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void process_resource_files(void)
BeginDeclarations
    byte buf[32];
    bit_32 i,j;
    byte_ptr data;
    byte_ptr hdr;
		resource_header_type reshead ;
		bit_16 name[256];
		bit_16 typename[256];
		bit_32 typeid, nameid ;
		bit_16 id ;
		bit_16 lang ;
EndDeclarations
BeginCode 
    resource_start_time = Now;

    ReturnIf(Not pefile.val) ;

    ReturnIf(resource_file_list.first IsNull) ;

    file_open_for_read(resource_file_list.first) ;

		file_read(buf,32) ;

    If memcmp(buf,resource_header,32)
     Then
         linker_error(4, "Invalid Resource File:\n"
                       "\t  File:  \"%s\"\n"
                       "\t Error:  Resource file has bad header.\n"
                       "\t         File being skipped.\n",
                       infile.file_info->filename) ;
        file_close_for_read();
				return ;
		 EndIf ;

    While Not feof(infile.file_handle) OrIf infile.bytes_left_in_buffer
			BeginWhile
        file_read((void *)&reshead,sizeof(resource_header_type)) ;
				If reshead.hdrsize LessThan 16
					Then
						InternalError() ;
						return ;
					EndIf ;
        hdr=allocate_memory(reshead.hdrsize);
				file_read(hdr,reshead.hdrsize-8) ;

				/* if this is a NULL resource, then skip */
				If Not reshead.datasize AndIf reshead.hdrsize IsEqualTo 32
							AndIf !memcmp(resource_header+8,hdr,24)
					Then
            release_memory(hdr);
	    			continue;
					EndIf ;

				If reshead.datasize
					Then
	    			data= allocate_memory(reshead.datasize) ;
						file_read(data,reshead.datasize) ;
					Else
						data = NULL ;
					EndIf ;
				i=0;
				reshead.hdrsize-=8;

				/* Read type name */
        ReturnIf (Not (i = ReadName(hdr,reshead.hdrsize,i,typename, &typeid))) ;
				/* Read name */
        ReturnIf (Not (i = ReadName(hdr,reshead.hdrsize,i,name, &nameid))) ;
				If i And 3
					Then
						i += 4 - (i % 4) ;
          			EndIf ;

				i+=6; /* point to Language ID */
				lang = *(bit_16_ptr)(hdr + i) ;
				i += 10 ; /* Point past header */
				If i And 3
					Then
						i += 4 - (i % 4) ;
		          EndIf ;
				If i GreaterThan reshead.hdrsize
					Then
					  InternalError() ;
						return ;
					EndIf ;
				InsertResourceData(data,reshead.datasize,typename,name,typeid,nameid,lang);
        n_resources++;
        release_memory(hdr);

				/* Now pass by any padding */
            i = infile.start_of_buffer_position + infile.byte_position;
			If i And 3
				Then
					file_read(buf,4-(i%4));
          		EndIf ;
			EndWhile ;
		file_close_for_read() ;
EndCode