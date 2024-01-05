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

/*+                              def.c                                      +*/

enum e_defToken {
edt_none,
edt_name,
edt_library,
edt_exports,
edt_imports,
edt_description,
edt_stacksize,
edt_heapsize,
edt_code,
edt_data,
edt_sections,
edt_noname,
edt_constant,
edt_private,
edt_read,
edt_write,
edt_execute,
edt_shared
} ;

Structure keywords
BeginStructure
  char *text;
  enum e_defToken id;
EndStructure ;

static Structure keywords table[] = {
{ "CODE", edt_code },
{ "CONSTANT", edt_constant },
{ "DATA", edt_data },
{ "DESCRIPTION", edt_description },
{ "EXECUTE", edt_execute },
{ "EXPORTS", edt_exports },
{ "HEAPSIZE", edt_heapsize },
{ "IMPORTS", edt_imports },
{ "LIBRARY", edt_library },
{ "NAME", edt_name },
{ "NONAME", edt_noname },
{ "PRIVATE", edt_private },
{ "READ", edt_read },
{ "SECTIONS", edt_sections },
{ "SHARED", edt_shared },
{ "STACKSIZE", edt_stacksize },
{ "WRITE", edt_write },

};

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             defToken                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
enum e_defToken defToken(char *token)
BeginDeclarations
    int top = sizeof(table)/sizeof(table[0]);
    int bottom =  - 1;
    int v;
	int mid;
EndDeclarations
BeginCode
  While top - bottom GreaterThan 1
   BeginWhile
    mid = (top + bottom) / 2;
    v = stricmp(token, table[mid].text);
    If v < 0
     Then
      top = mid;
     Else
      bottom = mid;
     EndIf
   EndWhile
  If bottom Is  - 1
   Then
    return 0;
   EndIf
  v = stricmp(token, table[bottom].text);
  If v IsNotZero
   Then
    return 0;
   EndIf
  return  table[bottom].id;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_need_eol                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void def_need_eol(char *cur_token_string)
BeginDeclarations
EndDeclarations
BeginCode
    cur_token_string = strtok(NULL, " \t\f\n");
	  If cur_token_string IsNotNull
	   Then
            linker_error(4, "Extra data at end of line.\n"
						    "File: \"%s\"\n"
							"Line: \"%d\"\n",
                    (*def_file_list.first).filename, def_lineno);
	   EndIf
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_number                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static bit_32 def_number(char *cur_token_string)
BeginDeclarations
char *                                 next;
bit_32                                 value;
EndDeclarations
BeginCode
  value = strtol(cur_token_string, &next, 0);
  If next[0] IsNotZero AndIf next[0] IsNot '\n'
   Then
        linker_error(8, "Invalid Number.\n"
					    "File: \"%s\"\n"
						"Line: \"%d\"\n",
                    (*def_file_list.first).filename, def_lineno);
   EndIf
  return value;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_change_attribs                     |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void def_change_attribs(char *segname, char *cur_token_string, int also_bss)
BeginDeclarations
pe_change_attribs_ptr                  change_attribs;
bit_32                                 flags = 0;
enum e_defToken                        tok;
EndDeclarations
BeginCode
 While cur_token_string
  BeginWhile
   cur_token_string = strtok(NULL, " \t\f\n");
   If cur_token_string IsNotNull
    Then
     tok = defToken(cur_token_string);
     Using tok
      BeginCase
        When edt_read:
	     flags |= WINF_READABLE;
	     break;
        When edt_write:
	     flags |= WINF_WRITEABLE;
	     break;
        When edt_execute:
	     flags |= WINF_EXECUTE;
	     break;
        When edt_shared:
	     flags |= WINF_SHARED;
	     break;
	    Otherwise:
          linker_error(8, "Expected Section type qualifier.\n"
					    "File: \"%s\"\n"
						"Line: \"%d\"\n",
						(*def_file_list.first).filename, def_lineno);
		  break;
	  EndCase
	EndIf
  EndWhile     
  change_attribs = (pe_change_attribs_ptr)allocate_memory(sizeof(pe_change_attribs_type));
  change_attribs->name = make_constant_string(segname);
  change_attribs->flags = flags ;
  Insert change_attribs AtEnd InList pe_change_attribs EndInsert;
  If also_bss
   Then
    change_attribs = (pe_change_attribs_ptr)allocate_memory(sizeof(pe_change_attribs_type));
    change_attribs->name = make_constant_string("BSS");
    change_attribs->flags = flags ;
    Insert change_attribs AtEnd InList pe_change_attribs EndInsert;
   EndIf
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_declare_import                     |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void def_declare_import(char *id, char *module, char *entry)
BeginDeclarations
public_entry_ptr                       pub;
#define Pub                            (*pub)
char *                                 next;
bit_16                                 ordflag = False;
bit_16                                 ord;
string_ptr                             tstr ;
#define Tstr                           (*tstr)
EndDeclarations
BeginCode
   If case_ignore.val
    Then
     far_to_lower(id, strlen(id));
     far_to_lower(entry, strlen(entry));
     far_to_lower(module, strlen(module));
    EndIf;
   If isdigit(entry[0])
    Then
	 If Not strcmp(entry, id)
	  Then
       linker_error(-1, "Expected entry point name. \n"
				    "File: \"%s\"\n"
					"Line: \"%d\"\n",
					(*def_file_list.first).filename, def_lineno);
	  EndIf
	 ordflag = True;
	 ord = strtoul(entry, &next, 0);
	 If next[0] AndIf next[0] IsNot '\n'
	  Then
       linker_error(-1, "Invalid ordinal value. \n"
				    "File: \"%s\"\n"
					"Line: \"%d\"\n",
					(*def_file_list.first).filename, def_lineno);
	  EndIf
	EndIf
   If n_imports NotLessThan max_imports
    Then
	 max_imports += 64 ;
     imports = (public_entry_ptr_array)
                           reallocate_memory(imports,
                                       (Bit_32(max_imports)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));
    EndIf;
   pub         = lookup_public(strlen(id), id, 0);
   If Pub.type_entry Is internal AndIf (Pub.type_qualifier Is public_normal OrIf Pub.type_qualifier Is public_import)
    Then
     linker_error(-1, "Error: Duplicate definition of public \"%s\". \n"
					    "File: \"%s\"\n"
						"Line: \"%d\"\n",
                        unmangle(Pub.symbol),
						(*def_file_list.first).filename, def_lineno);
    Else
     If Pub.type_entry IsNot internal
      Then
       tstr = allocate_string(strlen(module));
       Tstr.length = strlen(module) ;
       memcpy(Tstr.text, module, strlen(module)) ;
       Pub.moduleident = tstr ;
       If ordflag
        Then
         Pub.ordinal = ord ;
         Pub.entryident = NULL ;
        Else
         Pub.ordinal = 0 ;
         tstr = allocate_string(strlen(entry));
         Tstr.length = strlen(entry) ;
		 memcpy(Tstr.text, entry, strlen(entry));
         Pub.entryident = tstr ;
        EndIf ; 
       Pub.type_entry = internal ;
       Pub.use_count = 0 ;
       Pub.type_qualifier = public_import;
       imports[n_imports++] = pub ;
      EndIf ;
    EndIf ;
EndCode
#undef Tstr
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_declare_export                     |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void def_declare_export(char *id, char *module, char *entry, int ord, int byOrd)
BeginDeclarations
lseg_ptr                               lseg = NULL;
public_entry_ptr                       pub;
#define Pub                            (*pub)
string_ptr                             tstr ;
#define Tstr                           (*tstr)
char                                    tempName[256];
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
   If case_ignore.val
    Then
     far_to_lower(id, strlen(id));
     far_to_lower(entry, strlen(entry));
     far_to_lower(module, strlen(module));
    EndIf;
   If module[0]
    Then
	 static int hi;
	 /* forwarding */
	 sprintf(tempName, "def_fwd_imp_%d", def_forward_id);
	 def_declare_import(tempName, module, entry);
	 module[0] = 0;
     pub         = lookup_public(strlen(tempName), tempName, 0);
     lseg = obj_generate_segment(generated_lname,
                               generated_lname,
                               public_combine,
                               5,              /* DWORD aligned */
                               none_lname,
                               exe_file_list.first,
                               0L,             /* not absolute segment */
                               5);
	 lseg->highest_uninitialized_byte = 5;
	 lseg->data[0] = 0xe9; /* JMP REL */
	 memset(lseg->data + 1, 0, 4);
	 fixup.location_type = offset32_location;
	 fixup.mode = 0; /* self relative */
	 fixup.frame_method = 2; /* relative to external */
	 fixup.frame_referent = pub; 
	 fixup.target_method = 2;
	 fixup.target_referent = pub; 
	 fixup.target_offset  = 0;
	 fixup.external_error_detected = False;
	 fixup.reserved = 0;
     write_fixupp(FIXUPP32_record,
                   lseg,
                   1,
                   BytePtr(Addr(fixup)),
                   sizeof(fixup));
	 sprintf(tempName, "def_fwd_pub_%d", def_forward_id++);
	 strcpy(entry, tempName);
     pub         = lookup_public(strlen(entry), entry, 0);
	Else
     pub         = lookup_public(strlen(entry), entry, 0);
	EndIf 
   If Pub.type_entry Is internal AndIf Pub.type_qualifier Is public_normal 
				OrIf Pub.type_entry Is external OrIf Pub.type_entry Is unused
    Then
     tstr = allocate_string(strlen(id));
	 If Not byOrd
	  Then
       Tstr.length = strlen(id) ;
       memcpy(Tstr.text, id, strlen(id)) ;
	  EndIf
     Pub.entryident = tstr ;
     Pub.ordinal = ord ;
     If Pub.type_entry IsNot internal
      Then
       Pub.use_count = 0;
       Insert pub AtEnd InList external_list EndInsert;
      EndIf ;
     If Pub.type_entry Is unused
	  Then
	   If lseg IsNotNull
	    Then
		 Pub.type_entry = internal;
		Else
         Pub.type_entry = external ;
		EndIf
       Pub.type_qualifier = public_normal;
	   Pub.Internal.lseg = lseg;
	   Pub.Internal.offset = 0;
      EndIf ;
     exports[n_exports++] = pub ;
	EndIf;
EndCode
#undef Tstr
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_get_token                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static char *def_get_token(char *dest, char *data)
BeginDeclarations
EndDeclarations
BeginCode
  While (isspace(*data))
   BeginWhile
    data++;
   EndWhile
  If isalnum(*data) OrIf *data Is '_'
   Then
    While isalnum(*data) OrIf *data Is '_'
	 BeginWhile
	  *dest++ = *data++;
	 EndWhile
    While (isspace(*data))
     BeginWhile
      data++;
     EndWhile
   EndIf
  *dest = '\0';
  return data;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_read_export                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static int def_read_export(char *data)
BeginDeclarations
int                                byOrd = False;
char                                  *cur_token_string;
enum e_defToken                        eToken;
char                                   id[256];
char                                   entry[256];
char                                   module[256];
char                                  *next;
bit_32                                 ord = 0xffffffff;
EndDeclarations
BeginCode
  entry[0] = module[0] = '\0';
  data = def_get_token(id, data);
  If defToken(id) IsNot edt_none
   Then
    return 0;
   EndIf
  If data[0]
   Then
    If data[0] Is '='
	 Then
	  data = def_get_token(entry, data + 1);
	  If data[0] Is '.'
	   Then
	    strcpy(module, entry);
  	    data = def_get_token(entry, data + 1);
	   EndIf
	 Else
	  strcpy(entry, id);
	 EndIf
   Else
    strcpy(entry, id);
   EndIf
  cur_token_string = strtok(data, " \t\f\n");
  While cur_token_string
   BeginWhile
    eToken = defToken(cur_token_string);
	If eToken IsNot edt_none
	 Then
 	  Using eToken
	   BeginCase
	    When edt_noname:
	     byOrd = True;
		 break;
	    When edt_constant:
	    When edt_data:
	    When edt_private:
	     /* just ignoring these */
	     break;
	    Otherwise:
            linker_error(8, "Expected export qualifier.\n"
				   "File: \"%s\"\n"
				   "Line: \"%d\"\n",
                (*def_file_list.first).filename, def_lineno);
	   EndCase
	 Else
	  ord = strtoul(cur_token_string, &next, 0);
	  If next[0] && next[0] != '\n'
	   Then
            linker_error(8, "Expected export qualifier.\n"
				   "File: \"%s\"\n"
				   "Line: \"%d\"\n",
                (*def_file_list.first).filename, def_lineno);
	   EndIf
	 EndIf
    cur_token_string = strtok(NULL, " \t\f\n");
   EndWhile
  def_declare_export(id, module, entry, ord, byOrd);
  return 1;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_read_import                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static int def_read_import(char *data)
BeginDeclarations
char                                   entry[256];
char                                   id[256];
char                                   module[256];
EndDeclarations
BeginCode
  data = def_get_token(id, data);
  If defToken(id) IsNot edt_none
   Then
    return 0;
   EndIf
  If data[0] Is '.'
   Then
    strcpy(module, id);
	data = def_get_token(entry, data + 1);
    strcpy(id, entry);
   Else
    If data[0] Is '='
	 Then
	  data = def_get_token(module, data + 1);
	  If data[0] Is '.'
	   Then
	    data = def_get_token(entry, data + 1);
	   Else
           linker_error(8, "Expected '.'.\n"
						   "File: \"%s\"\n"
						   "Line: \"%d\"\n",
                        (*def_file_list.first).filename, def_lineno);
       EndIf
	 Else
	  If data[0] != '\0'
	   Then
           linker_error(8, "Expected '='.\n"
						   "File: \"%s\"\n"
						   "Line: \"%d\"\n",
                        (*def_file_list.first).filename, def_lineno);
	   EndIf
	 EndIf
   EndIf
  If data[0]
   Then
            linker_error(4, "Extra data at end of line.\n"
						    "File: \"%s\"\n"
							"Line: \"%d\"\n",
                    (*def_file_list.first).filename, def_lineno);
   EndIf
  def_declare_import(id, module, entry);
  return 1;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_name                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void def_name(char *cur_token_string)
BeginDeclarations
file_info_ptr                          file_entry;
#define File_entry                    (*file_entry)
char *                                 next;
bit_32                                 value;
string_ptr                             name;
EndDeclarations
BeginCode
	cur_token_string = strtok(NULL, ", \f\v\t\n");
	If cur_token_string IsNull
	 Then
            linker_error(8, "Expected file name in NAME directive.\n"
						    "File: \"%s\"\n"
							"Line: \"%d\"\n",
                    (*def_file_list.first).filename, def_lineno);
	 EndIf
    name = allocate_string(260);
	name->length = strlen(cur_token_string);
	strcpy(name->text, cur_token_string);
	
    If Length(name) Exceeds 0
     Then
      process_filename(name);
      add_extension_to_file(name, exe_extension_string);
	  If exe_file_list.first IsNotNull
	   Then
	    exe_file_list.first = exe_file_list.last = NULL;
	   EndIf
      file_entry = (file_info_ptr)
                    allocate_memory(Bit_32(sizeof(file_info_type)) +
                                    Bit_32(Length(name)));
      far_move(File_entry.filename, String(name), Length(name)+1);
      Insert file_entry AtEnd InList exe_file_list EndInsert;
     EndIf;

	cur_token_string = strtok(NULL, " \t\f\n");
    If cur_token_string IsNotNull
     Then
	  value = def_number(cur_token_string);
      If value < imageBase.min OrIf value > imageBase.max
	   Then
           linker_error(8, "Image base out of range.\n"
						   "File: \"%s\"\n"
						   "Line: \"%d\"\n",
                   (*def_file_list.first).filename, def_lineno);
	   EndIf
	  imageBase.val = value;
     EndIf
    def_need_eol(cur_token_string);
	build_DLL.val = False;
EndCode
#undef File_entry

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_library                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void def_library(char *cur_token_string)
BeginDeclarations
file_info_ptr                          file_entry;
#define File_entry                    (*file_entry)
char *                                 next;
bit_32                                 value;
string_ptr                             name;
EndDeclarations
BeginCode
	cur_token_string = strtok(NULL, ", \f\v\t\n");
	If cur_token_string IsNull
	 Then
            linker_error(8, "Expected file name in LIBRARY directive.\n"
						    "File: \"%s\"\n"
							"Line: \"%d\"\n",
                    (*def_file_list.first).filename, def_lineno);
	 EndIf
    name = allocate_string(260);
	name->length = strlen(cur_token_string);
	strcpy(name->text, cur_token_string);
	
    If Length(name) Exceeds 0
     Then
      process_filename(name);
      add_extension_to_file(name, dll_extension_string);
	  If exe_file_list.first IsNotNull
	   Then
	    exe_file_list.first = exe_file_list.last = NULL;
	   EndIf
      file_entry = (file_info_ptr)
                    allocate_memory(Bit_32(sizeof(file_info_type)) +
                                    Bit_32(Length(name)));
      far_move(File_entry.filename, String(name), Length(name)+1);
      Insert file_entry AtEnd InList exe_file_list EndInsert;
     EndIf;

	cur_token_string = strtok(NULL, " \t\f\n");
    If cur_token_string IsNotNull
     Then
	  value = def_number(cur_token_string);
	  If value < imageBase.min OrIf value > imageBase.max
	   Then
           linker_error(8, "Image base out of range.\n"
						   "File: \"%s\"\n"
						   "Line: \"%d\"\n",
                        (*def_file_list.first).filename, def_lineno);
	   EndIf
	  imageBase.val = value;
     EndIf
    def_need_eol(cur_token_string);
	build_DLL.val = True;
EndCode
#undef File_entry

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_exports                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void def_exports(char *cur_token_string)
BeginDeclarations
byte                                   data_buf[256];
int                                done = False;
bit_32                                 len;
bit_32                                 position;
bit_32                                 lineno;
enum e_defToken                        token;
EndDeclarations
BeginCode
  data_buf[0] = 0;
  position = get_read_file_position();
  lineno = def_lineno;
    cur_token_string = strtok(NULL, "\n");
  If cur_token_string IsNotNull
   Then
    strcpy(data_buf, cur_token_string);
   EndIf  
  While Not done
   BeginWhile
	
	While data_buf[0] Is '\0'
	 Then
         position = get_read_file_position();
		 lineno = def_lineno;
         def_lineno++;
	     file_read_line(data_buf, sizeof(data_buf));
		 If data_buf[0] IsZero
		  Then
		   return;
		  EndIf
		 len = strlen(data_buf);
		 If data_buf[len-1] Is '\n'
		  Then
		   data_buf[len-1] = 0;
		  EndIf
	EndWhile
    If Not def_read_export(data_buf)
     Then 
      file_position(position);
	  def_lineno = lineno;
	  done = True;
	 EndIf
	data_buf[0] = '\0';
   EndWhile
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_imports                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void def_imports(char *cur_token_string)
BeginDeclarations
byte                                   data_buf[256];
int                                done = False;
bit_32                                 len;
bit_32                                 lineno;
bit_32                                 position;
enum e_defToken                        token;
EndDeclarations
BeginCode
  position = get_read_file_position();
  lineno = def_lineno;
    cur_token_string = strtok(NULL, "\n");
  If cur_token_string IsNotNull
   Then
    strcpy(data_buf, cur_token_string);
   EndIf  
  While Not done
   BeginWhile
	
	While data_buf[0] Is '\0'
	 Then
         position = get_read_file_position();
		 lineno = def_lineno;
         def_lineno++;
	     file_read_line(data_buf, sizeof(data_buf));
		 If data_buf[0] IsZero
		  Then
		   return;
		  EndIf
		 len = strlen(data_buf);
		 If data_buf[len-1] Is '\n'
		  Then
		   data_buf[len-1] = 0;
		  EndIf
	EndWhile
    If Not def_read_import(data_buf)
     Then 
      file_position(position);
	  def_lineno = lineno;
	  done = True;
	 EndIf
	data_buf[0] = '\0';
   EndWhile
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_description                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void def_description(char *cur_token_string)
BeginDeclarations
 lseg_ptr                           lseg;  
EndDeclarations
BeginCode
	cur_token_string = strtok(NULL, "\n");
	If cur_token_string IsNull
	 Then
           linker_error(8, "Expected a description.\n"
						   "File: \"%s\"\n"
						   "Line: \"%d\"\n",
                        (*def_file_list.first).filename, def_lineno);
	 EndIf
	lseg = obj_generate_segment(
							desc_lname, 
							DATA_lname, 
							1, 
							public_combine, 
							none_lname,
                            exe_file_list.first,
                            0L,             /* not absolute segment */
                            strlen(cur_token_string));
   lseg->highest_uninitialized_byte = strlen(cur_token_string);		
   far_move(Addr(lseg->data[0]), cur_token_string, strlen(cur_token_string));									
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_stacksize                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void def_stacksize(char *cur_token_string)
BeginDeclarations
bit_32                                 value;
EndDeclarations
BeginCode
	cur_token_string = strtok(NULL, " \t\f\n,");
	If cur_token_string IsNull
	 Then
           linker_error(8, "Expected stack size qualifier.\n"
						   "File: \"%s\"\n"
						   "Line: \"%d\"\n",
                        (*def_file_list.first).filename, def_lineno);
	 EndIf
	value = def_number(cur_token_string);
	If value GreaterThan stackSize.max OrIf value LessThan stackSize.min
	 Then
           linker_error(8, "Stack size out of range.\n"
						   "File: \"%s\"\n"
						   "Line: \"%d\"\n",
                        (*def_file_list.first).filename, def_lineno);
	 EndIf
	stackSize.val = value;
	cur_token_string = strtok(NULL, " \t\f\n,");
	If cur_token_string IsNotNull
	 Then
	  value = def_number(cur_token_string);
	  If value GreaterThan stackCommitSize.max OrIf value LessThan stackCommitSize.min
	   Then
           linker_error(8, "Stack commit size out of range.\n"
						   "File: \"%s\"\n"
						   "Line: \"%d\"\n",
                        (*def_file_list.first).filename, def_lineno);
	   EndIf
	  stackCommitSize.val = value;
      cur_token_string = strtok(NULL, " \t\f\n,");
	 EndIf
    def_need_eol(cur_token_string);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_heapsize                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void def_heapsize(char *cur_token_string)
BeginDeclarations
bit_32                                 value;
EndDeclarations
BeginCode
	cur_token_string = strtok(NULL, " \t\f\n,");
	If cur_token_string IsNull
	 Then
           linker_error(8, "Expected stack size qualifier.\n"
						   "File: \"%s\"\n"
						   "Line: \"%d\"\n",
                        (*def_file_list.first).filename, def_lineno);
	 EndIf
	value = def_number(cur_token_string);
	If value GreaterThan heapSize.max OrIf value LessThan heapSize.min
	 Then
           linker_error(8, "Heap size out of range.\n"
						   "File: \"%s\"\n"
						   "Line: \"%d\"\n",
                        (*def_file_list.first).filename, def_lineno);
	 EndIf
	heapSize.val = value;
	cur_token_string = strtok(NULL, " \t\f\n,");
	If cur_token_string IsNotNull
	 Then
	  value = def_number(cur_token_string);
	  If value GreaterThan heapCommitSize.max OrIf value LessThan heapCommitSize.min
	   Then
           linker_error(8, "Heap commit size out of range.\n"
						   "File: \"%s\"\n"
						   "Line: \"%d\"\n",
                        (*def_file_list.first).filename, def_lineno);
	   EndIf
	  heapCommitSize.val = value;
      cur_token_string = strtok(NULL, " \t\f\n,");
	 EndIf
    def_need_eol(cur_token_string);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_code                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void def_code(char *cur_token_string)
BeginDeclarations
EndDeclarations
BeginCode
  def_change_attribs("CODE", cur_token_string, False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_data                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void def_data(char *cur_token_string)
BeginDeclarations
EndDeclarations
BeginCode
  def_change_attribs("DATA", cur_token_string, True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  def_sections                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
static void def_sections(char *cur_token_string)
BeginDeclarations
byte                                   data_buf[256];
int                                    done = False;
bit_32                                 len;
bit_32                                 lineno;
bit_32                                 position;
enum e_defToken                        token;
EndDeclarations
BeginCode
  position = get_read_file_position();
  lineno = def_lineno;
  While Not done
   BeginWhile
    cur_token_string = strtok(NULL, " \t\f\n");
	While cur_token_string IsNull
	 BeginWhile
         position = get_read_file_position();
		 lineno = def_lineno;
         def_lineno++;
	     file_read_line(data_buf, sizeof(data_buf));
		 If data_buf[0] IsZero
		  Then
		   return;
		  EndIf
		 If data_buf[0] Is '\n'
		  Then
		   ContinueLoop;
		  EndIf
		 len = strlen(data_buf);
		 If data_buf[len-1] Is '\n'
		  Then
		   data_buf[len-1] = 0;
		  EndIf
  		 cur_token_string = strtok(data_buf, " \t\f\n");
	EndWhile
	token = defToken(cur_token_string);
	If token IsNot edt_none
	 Then
      file_position(position);
	  def_lineno = lineno;
	  done = True;
	 Else
      def_change_attribs(cur_token_string, cur_token_string, False);
	 EndIf
   EndWhile
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             process_def_file                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void process_def_file(void)
BeginDeclarations
byte_ptr                               cur_token_string;
enum e_defToken                        cur_token;
byte                                   data_buf[512];
EndDeclarations
BeginCode
    def_lineno = 0;
	If def_file_list.first IsNotNull
	 Then
	  file_open_for_read(def_file_list.first);
      Repeat
	   BeginRepeat
	     def_lineno++;
	     file_read_line(data_buf, sizeof(data_buf));
		 If data_buf[0] IsNotZero AndIf data_buf[0] IsNot '\n'
          Then
  		   cur_token_string = strtok(data_buf, " \t\f\n");
		   If cur_token_string IsNotNull
		    Then
			 cur_token = defToken(cur_token_string);
   		     Using cur_token
		      BeginCase
		       When edt_name:
		   	    def_name(cur_token_string);
			    break;
		       When edt_library:
		   	    def_library(cur_token_string);
			    break;
		       When edt_exports:
		   	    def_exports(cur_token_string);
			    break;
		       When edt_imports:
		   	    def_imports(cur_token_string);
			    break;
		       When edt_description:
		   	    def_description(cur_token_string);
			    break;
		       When edt_stacksize:
		   	    def_stacksize(cur_token_string);
			    break;
		       When edt_heapsize:
		   	    def_heapsize(cur_token_string);
			    break;
		       When edt_code:
		   	    def_code(cur_token_string);
			    break;
		       When edt_data:
		   	    def_data(cur_token_string);
			    break;
		       When edt_sections:
		        def_sections(cur_token_string);
			    break;
		       Otherwise:
                linker_error(8, "\"%s\" is an illegal directive.\n"
						    "File: \"%s\"\n"
							"Line: \"%d\"\n",
                        cur_token_string, (*def_file_list.first).filename, def_lineno);
  		      EndCase
			EndIf
		  EndIf
 	    Until data_buf[0] Is 0 EndRepeat;
	  file_close_for_read();
	 EndIf
EndCode
