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
/*                                 TYPES.H                                 */
Type
 Structure acbp_struct                       acbp_type;
Type acbp_type                              *acbp_ptr;
Type
 Structure bit_32_switch_struct              bit_32_switch_type;
Type bit_32_switch_type                     *bit_32_switch_ptr;
Type
 Structure boolean_switch_struct             boolean_switch_type;
Type boolean_switch_type                    *boolean_switch_ptr;
Type
 Enumeration combine_enum                    combine_type;
Type
 Structure communal_entry_struct             communal_entry_type;
Type
 struct find_t                               DTA_type;
Type DTA_type                               *DTA_ptr;
Type
 Union entry_information_union               entry_information_type;
Type
 Structure EXE_header_struct                 EXE_header_type;
Type EXE_header_type                        *EXE_header_ptr;
Type 
 Structure export_header_struct              export_header_type ;
Type export_header_type                     *export_header_ptr ;
Type
 Structure external_entry_struct             external_entry_type;
Type
 Structure file_info_struct                  file_info_type;
Type file_info_type                         *file_info_ptr;
Type
 Structure file_struct                       file_type;
Type file_type                              *file_ptr;
Type
 Structure FIX_DAT_struct                    FIX_DAT_type;
Type FIX_DAT_type                           *FIX_DAT_ptr;
Type
 Structure fixup_struct                      fixup_type;
Type
 Structure fixup_hold_struct                 fixup_hold_type ;
Type fixup_hold_type                        *fixup_hold_ptr ;
Type fixup_hold_ptr                         *fixup_hold_ptr_array ;
Type
 Structure fixup_block_struct                fixup_block_type ;
Type fixup_block_type                       *fixup_block_ptr ;
Type
 Structure fixup_list_struct				 fixup_list_type ;
Type fixup_list_type						*fixup_list_ptr ;
Type
 Structure group_entry_struct                group_entry_type;
Type group_entry_type                       *group_entry_ptr;
Type group_entry_ptr                        *group_entry_ptr_array;
Type
 Structure internal_entry_struct             internal_entry_type;
Type 
 Structure le_object_page_struct             le_object_page_type ;
Type le_object_page_type                    *le_object_page_ptr ;
Type
 Structure library_directory_struct          library_directory_type;
Type library_directory_type                 *library_directory_ptr;
Type
 Structure library_entry_struct              library_entry_type;
Type
 Structure library_file_header_struct        library_file_header_type;
Type library_file_header_type               *library_file_header_ptr;
Type
 Structure library_symbol_entry_struct       library_symbol_entry_type;
Type library_symbol_entry_type              *library_symbol_entry_ptr;
Type
 Structure lname_entry_struct                lname_entry_type;
Type lname_entry_type                       *lname_entry_ptr;
Type lname_entry_ptr                        *lname_entry_ptr_array;
Type
 Enumeration loc_enum                        loc_type;
Type
 Structure LOCAT_struct                      LOCAT_type;
Type LOCAT_type                             *LOCAT_ptr;
Type
 Structure lseg_struct                       lseg_type;
Type lseg_type                              *lseg_ptr;
Type lseg_ptr                               *lseg_ptr_array;
Type
 Structure lx_fixup_hold_struct              lx_fixup_hold_type ;
Type lx_fixup_hold_type                     *lx_fixup_hold_ptr ;
Type lx_fixup_hold_ptr                      *lx_fixup_hold_ptr_array ;
Type 
 Structure lx_fixup_page_struct              lx_fixup_page_type ;
Type lx_fixup_page_type                     *lx_fixup_page_ptr ;
Type
 Structure lx_header_struct                  lx_header_type ;
Type lx_header_type                         *lx_header_ptr ;
Type 
 Structure lx_object_struct                  lx_object_type ;
Type lx_object_type                         *lx_object_ptr ;
Type 
 Structure lx_object_page_struct             lx_object_page_type ;
Type lx_object_page_type                    *lx_object_page_ptr ;
Type
 Structure lx_section_struct                 lx_section_type ;
Type lx_section_type                        *lx_section_ptr ;
Type lx_section_ptr                         *lx_section_ptr_array ;
Type
 Structure module_struct                     module_type ;
Type module_type                            *module_ptr ;
Type module_ptr                             *module_ptr_array ;
Type
 Structure MOD_TYP_struct                    MOD_TYP_type;
Type MOD_TYP_type                           *MOD_TYP_ptr;
Type
 Union obj_ptr_union                         obj_ptr_type;
Type
 Structure obj_record_header_struct          obj_record_header_type;
Type obj_record_header_type                 *obj_record_header_ptr;
Type
 Enumeration obj_mod_rec_type_enum           object_module_record_types;
Type
 Structure pe_change_attribs_struct          pe_change_attribs_type;
Type pe_change_attribs_type                 *pe_change_attribs_ptr;
Type
 Structure pe_header_struct                  pe_header_type ;
Type pe_header_type                         *pe_header_ptr ;
Type
 Structure pe_import_dir_struct              pe_import_dir_type ;
Type pe_import_dir_type                     *pe_import_dir_ptr ;
Type
 Structure pe_import_fixup_struct            pe_import_fixup_type ;
Type pe_import_fixup_type                   *pe_import_fixup_ptr ;
Type pe_import_fixup_ptr                    *pe_import_fixup_ptr_array ;
Type
 Structure pe_import_lookup_struct           pe_import_lookup_type ;
Type pe_import_lookup_type                  *pe_import_lookup_ptr ;
Type
 Structure pe_import_module_struct           pe_import_module_type ;
Type pe_import_module_type                  *pe_import_module_ptr ;
Type pe_import_module_ptr                   *pe_import_module_ptr_array ;
Type
 Structure pe_object_struct                  pe_object_type ;
Type pe_object_type                         *pe_object_ptr ;
Type
 Structure pe_section_struct                 pe_section_type ;
Type pe_section_type                        *pe_section_ptr ;
Type pe_section_ptr                         *pe_section_ptr_array ;
Type
 Enumeration public_entry_class_enum         public_entry_class_type;
Type
 Structure public_entry_struct               public_entry_type;
Type public_entry_type                      *public_entry_ptr;
Type public_entry_ptr                       *public_entry_ptr_array;
Type
 Structure resource_data_struct              resource_data_type ;
Type resource_data_type                     *resource_data_ptr ;
Type
 Structure resource_data_list_struct         resource_data_list_type ;
Type resource_data_list_type                *resource_data_list_ptr ;
Type
 Structure resource_data_entry_struct        resource_data_entry_type ;
Type resource_data_entry_type               *resource_data_entry_ptr ;
Type
 Structure resource_dir_entry_struct         resource_dir_entry_type ;
Type resource_dir_entry_type                *resource_dir_entry_ptr ;
Type
 Structure resource_dir_table_struct         resource_dir_table_type ;
Type resource_dir_table_type                *resource_dir_table_ptr ;
Type
 Structure resource_header_struct            resource_header_type ;
Type resource_header_type                   *resource_header_ptr ;
Type
 Structure segment_entry_struct              segment_entry_type;
Type segment_entry_type                     *segment_entry_ptr;
Type segment_entry_ptr                      *segment_entry_ptr_array;
Type
 Structure switch_table_struct               switch_table_type;
Type switch_table_type                      *switch_table_ptr;
Type
 Structure text_switch_struct                text_switch_type;
Type text_switch_type                       *text_switch_ptr;
Type
 Structure thread_struct                     thread_type;
Type
 Structure TRD_DAT_struct                    TRD_DAT_type;
Type TRD_DAT_type                           *TRD_DAT_ptr;
Type
 Enumeration token_class_enum                token_class_type;
Type
 Structure token_stack_struct                token_stack_type;
Type token_stack_type                       *token_stack_ptr;
Type
 Structure virtual_entry_struct              virtual_entry_type;

Structure acbp_struct
 BeginStructure
  unsigned                             p:1;
  unsigned                             b:1;
  unsigned                             c:3;
  unsigned                             a:3;
 EndStructure;

Enumeration align_enum
 BeginEnumeration
  absolute_segment,
  byte_aligned,
  word_aligned,
  paragraph_aligned,
  page_aligned,
  dword_aligned,
  fourk_aligned
 EndEnumeration;

Structure bit_32_switch_struct
 BeginStructure
  bit_32                               val;
  bit_32                               min;
  bit_32                               max;
  bit_32                               def;
  bit_32                               set;
 EndStructure;

Structure boolean_switch_struct
 BeginStructure
  bit_16                               val;
 EndStructure;

Enumeration combine_enum
 BeginEnumeration
  private_combine                      = 0,
  public_combine                       = 2,
  stack_combine                        = 5,
  common_combine                       = 6,
  blank_common_combine                 = 9,
  virtual_combine 		       = 15,
 EndEnumeration;

Structure EXE_header_struct
 BeginStructure
  bit_16                               signature;
  bit_16                               image_length_MOD_512;
  bit_16                               image_length_DIV_512;
  bit_16                               n_relocation_items;
  bit_16                               n_header_paragraphs;
  bit_16                               min_paragraphs_above;
  bit_16                               max_paragraphs_above;
  bit_16                               initial_SS;
  bit_16                               initial_SP;
  bit_16                               checksum;
  bit_16                               initial_IP;
  bit_16                               initial_CS;
  bit_16                               offset_to_relocation_table;
  bit_16                               overlay;
  bit_16                               always_one;
  bit_32                               relocation_table[1];
 EndStructure;

Structure external_entry_struct
 BeginStructure
  bit_16                               reserved;
 EndStructure;

Structure internal_entry_struct
 BeginStructure
  group_entry_ptr                      group;
  lseg_ptr                             lseg;
  bit_32                               segment_index;
  bit_32                               frame; /* also used to point to import thunks */
  bit_32                               offset;
 EndStructure;

Structure library_entry_struct
 BeginStructure
  file_info_ptr                        lib_file;
  bit_16                               page;
  bit_8                                requested;
 EndStructure;

Structure communal_entry_struct
 BeginStructure
  public_entry_ptr                     next_communal;
  bit_32                               element_size;
  bit_32                               element_count;
 EndStructure;

Structure virtual_entry_struct
 BeginStructure
  public_entry_ptr		       next_virtual;
  bit_32 			       element_size;
  bit_32			       seg_index ;
  lseg_ptr                             enclosing_segment;
  lseg_ptr                             lseg;
 EndStructure;

Union entry_information_union
 BeginUnion
  library_entry_type                   library_type;
  external_entry_type                  external_type;
  internal_entry_type                  internal_type;
  communal_entry_type                  communal_type;
  virtual_entry_type                   virtual_type;
 EndUnion;

Enumeration public_entry_class_enum
 BeginEnumeration
  unused,
  public_in_library,
  external,
  internal,
  far_communal,
  near_communal,
  virtual_segment,
 EndEnumeration;

Enumeration public_entry_type_qualifier_enum
 BeginEnumeration
  public_normal,
  public_import
 EndEnumeration ;
  
Structure public_entry_struct
 BeginStructure
  public_entry_ptr                     next;
  public_entry_ptr                     next_congruent;
  Generic_Element_ptr                  modpubs ;
#define PUB_OFFSET_MODPUBS 8
  bit_8                                type_entry;
  bit_8                                type_qualifier;
  entry_information_type               entry;
  bit_32                               isdata ;
  bit_16                               module;
  bit_16                               use_count ;
  bit_32                               ordinal; /* used for exports */
  string_ptr                           entryident; /* used for exports */
  string_ptr                           moduleident ; /* used for exports */
  bit_8                                did_external_error ;
  bit_8                                reserved ;
  bit_16                               max_length ; /* unused so we can fake as a string in pe.c */
  bit_16                               length;
  byte                                 symbol[1];
 EndStructure;
#define Library                        entry.library_type
#define External                       entry.external_type
#define Internal                       entry.internal_type
#define Communal                       entry.communal_type
#define Virtual			       entry.virtual_type

ListTypeOf(public_entry);

Structure file_info_struct
 BeginStructure
  file_info_ptr                        next;
  bit_8                                attribute;
  bit_16                               time_stamp;
  bit_16                               date_stamp;
  bit_32                               file_size;
  bit_16                               page_size;
  bit_16                               request_count;
  bit_16                               pass_count;
  bit_16                               module_count;
  public_entry_list                    external_list;
  byte                                 filename[1];
 EndStructure;

ListTypeOf(file_info);

Structure file_struct
 BeginStructure
  file_info_ptr                        file_info;
  FILE                                *file_handle;
  bit_32                               start_of_buffer_position;
  bit_32                               next_buffer_position;
  byte_ptr                             buffer;
  byte_ptr                             current_byte;
  bit_16                               buffer_size;
  bit_16                               IO_limit;
  bit_16                               bytes_in_buffer;
  bit_16                               bytes_left_in_buffer;
  bit_16                               byte_position;
 EndStructure;

Structure FIX_DAT_struct
 BeginStructure
  unsigned short                       targt:2;
  unsigned short                       p:1;
  unsigned short                       t:1;
  unsigned short                       frame:3;
  unsigned short                       f:1;
 EndStructure;

Structure fixup_struct
 BeginStructure
  void                                *frame_referent;
  void                                *target_referent;
  bit_32                               target_offset;
  unsigned short                       location_type:4;
  unsigned short                       mode:1;
  unsigned short                       frame_method:3;
  unsigned short                       target_method:2;
  unsigned short                       external_error_detected:1;
  unsigned short                       reserved:5;
 EndStructure;

Structure group_entry_struct
 BeginStructure
  group_entry_ptr                      next;
  group_entry_ptr                      next_congruent;
  lname_entry_ptr                      group_name;
  segment_entry_ptr                    first_segment;
 EndStructure;

ListTypeOf(group_entry);

Structure library_directory_struct
 BeginStructure
  bit_8                                offset_to_symbol[38];
  byte                                 symbol_area[474];
 EndStructure;

Structure library_file_header_struct
 BeginStructure
  bit_8                                flag;
  bit_16                               page_size;
  bit_32                               directory_position;
  bit_16                               n_directory_blocks;
 EndStructure;

 Structure library_symbol_entry_struct
  BeginStructure
   bit_8                               length_of_symbol;
   byte                                symbol[1];
  EndStructure;

Structure lname_entry_struct
 BeginStructure
  lname_entry_ptr                      next_congruent;
  bit_16                               lname_checksum;
  bit_16                               length;
  byte                                 symbol[1];
 EndStructure;

Structure LOCAT_struct
 BeginStructure
  unsigned short                       data_record_offset:10;
  unsigned short                       loc:4;
  unsigned short                       m:1;
  unsigned short                       type_fixupp_record:1;
 EndStructure;

Enumeration loc_enum
 BeginEnumeration
  lobyte_location,
  offset_location,
  base_location,
  pointer_location,
  hibyte_location,
  secondary_offset_location,
  offset32_location = 9,
  pointer32_location = 11,
  secondary_offset32_location = 13,
 EndEnumeration;

Structure lseg_struct
 BeginStructure
  lseg_ptr                             next;
  segment_entry_ptr                    segment;
  lname_entry_ptr                      tmodule;
  file_info_ptr                        file;
  bit_32                               address;
  bit_32                               length;
  bit_32                               highest_uninitialized_byte;
  int_32                               fixupp_offset;
  bit_8                                align;
  bit_8                                hassyms : 1; /* for debug support */
  bit_8								   virtualseg : 1 ;
  bit_8                                hasFixups: 1;
  bit_8								   reserved : 5;
  byte_ptr                             data;
 EndStructure;

ListTypeOf(lseg);

Structure lx_fixup_hold_struct
 BeginStructure
  bit_32 offset ;
  bit_32 target ;
  bit_8  target_object ;
  bit_8  type ;
 EndStructure ;

Structure fixup_list_struct
 BeginStructure
  Structure fixup_list_struct		  *next ;
  bit_8                                rec_typ;
  bit_16                               rec_len;
  lseg_ptr                             lseg;
  bit_32                               offset;
  byte_ptr							   data ;
 EndStructure;

Structure lx_fixup_page_struct
 BeginStructure
   bit_32 offset ;
 EndStructure ;

Structure lx_header_struct 
 BeginStructure
   bit_16 sig ;
   bit_8  bbyte ;
   bit_8  wword ;
   bit_32 format_level ;
   bit_16 cpu_level ;
   bit_16 os_type ;
   bit_32 module_version ;
   bit_32 module_flags ;
   bit_32 module_page_count ;
   bit_32 eip_object ;
   bit_32 eip ;
   bit_32 esp_object ;
   bit_32 esp ;
   bit_32 page_size ;
   bit_32 page_offset_shift ; /* for LE, this is bytes on last page */
   bit_32 fixup_section_size ;
   bit_32 fixup_section_checksum ;
   bit_32 loader_section_size ;
   bit_32 loader_section_checksum ;
   bit_32 object_table_offset ;
   bit_32 object_count ;
   bit_32 object_page_table_offset ;
   bit_32 object_iter_pages_offset ;
   bit_32 resource_table_offset ;
   bit_32 resource_table_entries ;
   bit_32 resident_name_table_offset ;
   bit_32 resident_name_table_entries ; /* le - entry table offset */
   bit_32 module_directives_offset ;
   bit_32 module_directives_count ;
   bit_32 fixup_page_table_offset ;
   bit_32 fixup_record_table_offset ;
   bit_32 import_module_table_offset ;
   bit_32 import_module_table_entries ;
   bit_32 import_proc_table_offset ;
   bit_32 per_page_checksum_offset ;
   bit_32 data_pages_offset ;
   bit_32 preload_pages_count ;
   bit_32 nonresident_name_table_offset ;
   bit_32 nonresident_name_table_length ;
   bit_32 nonresident_name_table_checksum ;
   bit_32 auto_ds_object ;
   bit_32 debug_info_offset ;
   bit_32 debug_info_length ;
   bit_32 instance_preload_count ;
   bit_32 instance_demand ;
   bit_32 heapsize ;
   bit_32 reserved[6] ;
 EndStructure ;

Structure lx_object_struct
 BeginStructure
   bit_32 virtual_size ;
   bit_32 reloc_base_addr ;
   bit_32 object_flags ;
   bit_32 page_table_index ;
   bit_32 page_table_entries ;
   bit_32 reserved ;
 EndStructure ;

Structure lx_object_page_struct
 BeginStructure
   bit_32 data_offset ;
   bit_16 data_size ;
   bit_16 flags ;
 EndStructure ;

Structure le_object_page_struct
 BeginStructure
  bit_8 high_offs ;
  bit_8 med_offs ;
  bit_8 low_offs ;
  bit_8 flags ;
 EndStructure ;

Structure lx_section_struct
 BeginStructure
  bit_32 length;
  bit_32 initlength ;
  bit_32 virtualSize;
  bit_32 page_table_entries ;
  bit_32 base;
  bit_32 Flags;
  byte_ptr data;
 EndStructure ;

Structure MOD_TYP_struct
 BeginStructure
  unsigned                             l:1;
  unsigned                             zeros:5;
  unsigned                             mattr:2;
 EndStructure;

Structure module_struct
 BeginStructure
  lseg_ptr                             cseg ;
  lseg_ptr                             dseg ;
  lseg_ptr                             bseg ;
  lseg_ptr                             stringseg ;
  lseg_ptr                             constseg ;
  lseg_ptr                             debtypseg ;
  lseg_ptr                             debsymseg ;
  Generic_Element_list                 publics ;
  bit_32                               type_offset ;
  bit_32                               linnum_len ;
  byte_ptr                             linnum_data ;
  bit_32                               align_len ;
  byte_ptr                             align_data ;
  char                                 name[1];
 EndStructure;

Union obj_ptr_union
 BeginUnion
  bit_8                               *b8;
  bit_16                              *b16;
  bit_32                              *b32;
  acbp_ptr                             acbp;
  FIX_DAT_ptr                          FIX_DAT;
  LOCAT_ptr                            LOCAT;
  MOD_TYP_ptr                          MOD_TYP;
  TRD_DAT_ptr                          TRD_DAT;
 EndUnion;

Enumeration obj_mod_rec_type_enum
 BeginEnumeration
  THEADR_record                        = 0x80,
  COMENT_record                        = 0x88,
  MODEND_record                        = 0x8a,
  MODEND32_record                      = 0x8b,
  EXTDEF_record                        = 0x8c,
  TYPDEF_record                        = 0x8e,
  PUBDEF_record                        = 0x90,
  PUBDEF32_record                      = 0x91,
  LINNUM_record                        = 0x94,
  LINNUM32_record                      = 0x95,
  LNAMES_record                        = 0x96,
  SEGDEF_record                        = 0x98,
  SEGDEF32_record                      = 0x99,
  GRPDEF_record                        = 0x9a,
  FIXUPP_record                        = 0x9c,
  FIXUPP32_record                      = 0x9d,
  LEDATA_record                        = 0xa0,
  LEDATA32_record                      = 0xa1,
  LIDATA_record                        = 0xa2,
  LIDATA32_record                      = 0xa3,
  COMDEF_record                        = 0xb0,
  FORREF_record                        = 0xb2,
  FORREF32_record                      = 0xb3,
  MODEXT_record                        = 0xb4,
  MODEXT32_record                      = 0xb5,
  MODPUB_record                        = 0xb6,
  MODPUB32_record                      = 0xb7,
  LLNAMES_record                       = 0xca
 EndEnumeration;

Structure obj_record_header_struct
 BeginStructure
  bit_8                                rec_typ;
  bit_16                               rec_len;
  byte                                 variant_part[1];
 EndStructure;

Structure segment_entry_struct
 BeginStructure
  segment_entry_ptr                    next;
  segment_entry_ptr                    next_congruent;
  lname_entry_ptr                      segment_name;
  lname_entry_ptr                      class_name;
  bit_8                                loaded;
  bit_8                                combine;
  bit_32                               address;
  bit_32                               length;
  bit_32                               highest_uninitialized_byte;
  bit_32                               pe_section_number ;
  lseg_list                            lsegs;
  group_entry_ptr                      owning_group;
 EndStructure;

ListTypeOf(segment_entry);

Structure switch_table_struct
 BeginStructure
  bit_8                                min_length;
  char                                *abbr_name;
  char                                *full_name;
  void                                *affected_thing;
  void                  (*switch_processor)(switch_table_ptr current_switch);
 EndStructure;

Structure text_switch_struct
 BeginStructure
  string_ptr                           val;
 EndStructure;

Structure thread_struct
 BeginStructure
  bit_8                                method;
  void                                *referent;
  bit_8                                thread_defined;
 EndStructure;

Enumeration token_class_enum
 BeginEnumeration
  continuation_token_type,
  end_of_command_line_token_type,
  filename_token_type,
  indirect_file_token_type,
  line_end_token_type,
  number_token_type,
  separator_token_type,
  switch_token_type,
  switch_end_token_type,
  text_token_type,
  terminator_token_type
 EndEnumeration;

Structure token_stack_struct
 BeginStructure
  token_stack_ptr                      next;
  FILE                                *source_file;
  byte                                 break_char;
  string_ptr                           token_string;
  bit_16                               token_string_index;
 EndStructure;

ListTypeOf(token_stack);

Structure TRD_DAT_struct
 BeginStructure
  unsigned short                       thred:2;
  unsigned short                       method:3;
  unsigned short                       z:1;
  unsigned short                       d:1;
  unsigned short                       type_fixupp_record:1;
 EndStructure;

Structure pe_header_struct
 BeginStructure
  bit_32                                sig ;
  bit_16                                cpu_type ;
  bit_16                                num_objects ;
  bit_32                                time ;
  bit_32                                symbol_ptr ;
  bit_32                                num_symbols ;
  bit_16                                nt_hdr_size ;
  bit_16                                flags ;
  bit_16                                magic ;
  bit_8                                 linker_minor_version ;
  bit_8                                 linker_major_version ;
  bit_32                                code_size ;
  bit_32                                data_size ; 
  bit_32                                bss_size ;
  bit_32                                entry_point ;
  bit_32                                code_base ;
  bit_32                                data_base ;
  bit_32                                image_base ;
  bit_32                                object_align ;
  bit_32                                file_align ;
  bit_16                                os_major_version ;
  bit_16                                os_minor_version ;
  bit_16                                user_major_version ;
  bit_16                                user_minor_version ;
  bit_16                                subsys_major_version ;
  bit_16                                subsys_minor_version ;
  bit_32				uu_1 ;
  bit_32                                image_size ;
  bit_32                                header_size ;
  bit_32                                chekcsum ;
  bit_16                                subsystem ;
  bit_16                                dll_flags ;
  bit_32                                stack_size ;
  bit_32                                stack_commit ;
  bit_32                                heap_size ;
  bit_32                                heap_commit ;
  bit_32                                loader_flags ;
  bit_32                                num_rvas ;
  bit_32                                export_rva ;
  bit_32                                export_size ;
  bit_32                                import_rva ;
  bit_32                                import_size ;
  bit_32                                resource_rva ;
  bit_32                                resource_size ;
  bit_32                                exception_rva ;
  bit_32                                exception_size ;
  bit_32                                security_rva ;
  bit_32                                security_size ;
  bit_32                                fixup_rva ;
  bit_32                                fixup_size ;
  bit_32                                debug_rva ;
  bit_32                                debug_size ;
  bit_32                                desc_rva ;
  bit_32                                desc_size ;
  bit_32                                mspec_rva ;
  bit_32                                mspec_size ;
  bit_32                                tls_rva ;
  bit_32                                tls_size ;
  bit_32                                loadconfig_rva ;
  bit_32                                loadconfig_size ;
  bit_32                                boundimp_rva ;
  bit_32                                boundimp_size ;
  bit_32                                iat_rva ;
  bit_32                                iat_size ;
  bit_32				res1_rva,res1_size ;
  bit_32				res2_rva,res2_size ;
  bit_32				res3_rva,res3_size ;
 EndStructure;

Structure pe_object_struct
 BeginStructure
  char                                  name[8] ;
  bit_32                                virtual_size ;
  bit_32                                virtual_addr ;
  bit_32                                raw_size ;
  bit_32                                raw_ptr ;
  bit_32				reserved[3] ;
  bit_32                                flags ;
 EndStructure ;

Structure resource_header_struct
 BeginStructure
  bit_32 datasize ;
  bit_32 hdrsize ;
 EndStructure ;

ListTypeOf(resource_data) ;
Structure resource_data_struct
 BeginStructure
  resource_data_ptr next ;
  resource_data_list_type name_list;
  resource_data_list_type ident_list;
  bit_16_ptr name;
  bit_16   id ; /* if name is null */
  bit_32   length ;
  byte_ptr data ;
  bit_32   name_count ;
  bit_32   ident_count ;
 EndStructure ;

Structure resource_dir_table_struct
 BeginStructure
  bit_32 resource_flags ;
  bit_32 time ;
  bit_32 version ;
  bit_16 name_entry ;
  bit_16 ident_entry ;
 EndStructure ;

Structure resource_dir_entry_struct
 BeginStructure
  bit_32 rva_or_id ;
  bit_32 subdir_or_data : 31 ;
  bit_32 escape : 1 ;
 EndStructure ;

Structure resource_data_entry_struct
 BeginStructure
  bit_32 rva ;
  bit_32 size ;
  bit_32 codepage ;
  bit_32 reserved ;
 EndStructure ;

Structure pe_section_struct
 BeginStructure
  byte name[8];
  bit_32 length;
  bit_32 initlength ;
  bit_32 virtualSize;
  bit_32 base;
  bit_32 winFlags;
  unsigned short attr;
  byte_ptr data;
 EndStructure ;

Structure export_header_struct
  BeginStructure
		bit_32 flags ;
		bit_32 time ;
		bit_32 version ;
		bit_32 exe_name_rva ;
		bit_32 ord_base ;
		bit_32 n_eat_entries ;
		bit_32 n_name_ptrs ;
		bit_32 address_rva ;
		bit_32 name_rva ;
		bit_32 ordinal_rva ;
  EndStructure ;

Structure fixup_hold_struct
 BeginStructure
  bit_32 offset ;
  bit_8  type ;
 EndStructure ;

Structure pe_import_fixup_struct
 BeginStructure
  public_entry_ptr pub ;
  byte_ptr location_ptr ;
  bit_32 location_addr ;
  bit_32 isdata ;
 EndStructure ;

Structure fixup_block_struct
 BeginStructure
  bit_32 rva ;
  bit_32 size ;
  bit_16 data[2048] ;
 EndStructure ;

Structure pe_import_dir_struct
 BeginStructure
  bit_32 thunkPos2 ;
  bit_32 time ;
  bit_32 version ;
  bit_32 dllName ;
  bit_32 thunkPos ;
 EndStructure ;

Structure pe_import_lookup_struct
 BeginStructure
  bit_32 ord_or_rva :31;
  bit_32 import_by_ordinal : 1 ;
 EndStructure ;


Structure pe_import_module_struct
 BeginStructure
  string_ptr name ;
  bit_32 n_imports ;
  bit_32 max_imports ;
  string_ptr_array funcnames ;
  bit_32_ptr ordinals ;
  public_entry_ptr_array pubs ;
 EndStructure ;
 
Structure pe_change_attribs_struct
 BeginStructure
  Structure pe_change_attribs_struct *next;
  string_ptr name;
  bit_32 flags;
 EndStructure;  
 
ListTypeOf(pe_change_attribs);

