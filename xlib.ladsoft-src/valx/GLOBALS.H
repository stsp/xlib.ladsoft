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
/*                                 GLOBALS.H                               */

/* Global variables used by the linker in approximate alphabetical order.  */

extern bit_32                                 address_base;
extern bit_32_switch_type                     align32;
extern bit_32                                 align_mask[7];
extern boolean_switch_type                    align_exe_header ; 
extern char                                  *align_text[7];
extern segment_entry_ptr                      active_segment;
#define Active_segment                 (*active_segment)
extern string_ptr                             ampersand_string;
extern string_ptr                             and_string;
extern string_ptr                             at_string;
extern string_ptr                             backslash_dot_string;
extern string_ptr                             backslash_dot_dot_string;
extern string_ptr                             backslash_string;
extern bit_32                                 bad_link ;
extern string_ptr                             bar_string;
extern lseg_ptr                               bseg_lseg ;      
extern lname_entry_ptr                        BSS_lname;
extern bit_32_switch_type                     buffer_size ; 
extern boolean_switch_type                    build_DLL ;
extern lname_entry_ptr                        c_common_lname;
extern boolean_switch_type                    case_ignore ; 
extern string_ptr                             class_string;
extern string_ptr                             close_paren_string;
extern string_ptr                             close_angle_string;
extern lname_entry_ptr                        CODE_lname;
extern lname_entry_ptr                        codeview_class_DEBBROWSE;
extern lname_entry_ptr                        codeview_class_DEBSYM;
extern lname_entry_ptr                        codeview_class_DEBTYP;
extern bit_16                                 codeview_information_present ;
extern lname_entry_ptr                        codeview_segment_BROWSE;
extern lname_entry_ptr                        codeview_segment_SYMBOLS;
extern lname_entry_ptr                        codeview_segment_TYPES;
extern string_ptr                             colon_string;
extern string_ptr                             com_extension_string;
extern char                                  *combine_text[10];
extern boolean_switch_type                    comfile ;
extern string_ptr                             comma_string;
extern lseg_ptr                               constseg_lseg ;
extern bit_32_switch_type                     CPARMAXALLOC ;
extern lseg_ptr                               cseg_lseg ;      
extern string_ptr                             current_filename;
extern module_ptr                             current_module;
extern string_ptr                             current_path;
extern obj_record_header_ptr                  current_record_header;
#define Current_record_header          (*current_record_header)
extern bit_32                                 current_record_offset;
extern lseg_ptr                               cvdebsym_lseg ;
extern lseg_ptr                               cvdebtyp_lseg ;
extern string_ptr                             cv_extension_string;
extern file_info_list                         cv_file_list ;
extern lname_entry_ptr                        DATA_lname;
extern boolean_switch_type                    debug ;
extern string_ptr                             def_extension_string;
extern file_info_list                         def_file_list;
extern bit_32                                 def_forward_id;
extern bit_32                                 def_lineno;
extern string_ptr                             default_directory_string;
extern string_ptr                             default_drive_string;
extern string_ptr                             default_extension;
extern string_ptr                             default_filename;
extern string_ptr                             default_lx_stub_string ;
extern char_ptr                               default_prompt;
extern lname_entry_ptr                        desc_lname;
extern bit_32_switch_type                     detail_level ; 
extern string_ptr                             device_AUX;
extern string_ptr                             device_CON;
extern string_ptr                             device_PRN;
extern lname_entry_ptr                        DGROUP_lname;
extern string_ptr                             dll_extension_string;
extern boolean_switch_type                    DOSSEG ;
extern string_ptr                             dot_string;
extern lseg_ptr                               dseg_lseg ;      
extern DTA_type                               DTA;
extern segment_entry_ptr                      edata_segment ;
extern obj_ptr_type                           end_of_record;
extern obj_ptr_type                           end_of_last_LIDATA_record;
extern segment_entry_ptr                      end_segment ;
extern string_ptr                             env_extension_string;
extern string_ptr                             exclamation_string;
extern string_ptr                             exe_extension_string;
extern file_info_list                         exe_file_list ;
extern EXE_header_ptr                         exe_header;
#define Exe_header                     (*exe_header)
extern bit_32                                 exe_header_size;
extern bit_32                                 exec_image_start_time;
extern boolean_switch_type                    exechecksum ;
extern bit_16                                 exefile;
extern public_entry_ptr_array                 exports;
extern public_entry_list                      external_list ;
extern public_entry_ptr_array                 externals;
extern string_ptr                             false_string;
extern lname_entry_ptr                        FAR_BSS_lname;
extern public_entry_ptr                       far_communals ;
extern bit_32                                 first_pe_section_address ;
extern bit_32_switch_type                     fileAlign ;
extern fixup_type                             fixup;
extern bit_16                                 FIXUPP_contains_only_threads;
extern bit_16                                 fixup_index;
extern fixup_list_ptr						  fixup_list ;
extern fixup_list_ptr						  fixup_list_tail ;
extern bit_32                                 fixup_start_time;
extern fixup_list_ptr						  fixup_temp ;
extern bit_32                                 FLAT_address ;
extern lname_entry_ptr                        FLAT_lname;
extern bit_16                                 frame_absolute;
extern thread_type                            frame_thread[4];
extern lname_entry_ptr                        generated_lname;
extern group_entry_ptr_array                  gnames;
extern group_entry_ptr_array                  group_hash_table;
extern group_entry_list                       group_list ;
extern bit_32_switch_type                     group_table_hash_size ; 
extern string_ptr                             group_string;
extern bit_32_switch_type                     heapCommitSize ;
extern bit_32_switch_type                     heapSize ;
extern boolean_switch_type                    help;
extern string_ptr                             help_extension_string;
extern string_ptr                             help_filename;
extern bit_32                                 highest_uninitialized_byte;
extern lname_entry_ptr                        HUGE_BSS_lname;
extern public_entry_ptr                       huge_communals ;
extern bit_32_switch_type                     imageBase ;
extern lseg_ptr                               import_thunk_table ;
extern public_entry_ptr_array                 imports;
extern file_type                              infile;
extern byte_ptr                               infile_buffer;
extern byte_ptr                               infile_buffer_2;
extern bit_16                                 initial_IP ;
extern bit_16                                 initial_CS ;
extern segment_entry_ptr                      largest_stack_seg;
#define Largest_stack_seg              (*largest_stack_seg)
extern bit_32                                 largest_stack_seg_length;
extern lseg_ptr                               last_LxDATA_lseg;
#define last_LxDATA_Lseg               (*last_LxDATA_lseg)
extern bit_32                                 last_LxDATA_offset;
extern bit_8                                  last_LxDATA_record_type;
extern obj_record_header_ptr                  last_LIDATA_record_header;
extern boolean_switch_type                    lefile;
#define Last_LIDATA_record_header      (*last_LIDATA_record_header)
extern bit_8                                  last_LIDATA_record[MAX_ELEMENT_SIZE];
extern string_ptr                             lib_extension_string;
extern text_switch_type                       lib_directory ;
extern file_info_list                         lib_file_list ;
extern file_info_list                         lib_file_new_list ;
extern bit_32                                 library_directory_start_time;
extern bit_32                                 library_processing_start_time;
extern bit_16                                 library_request_count ;
extern bit_16                                 LIDATA_index;
extern bit_32                                 LIDATA_offset;
extern bit_16                                 link_step ;
extern bit_32                                 linker_end_time;
extern bit_32                                 linker_start_time;
extern lname_entry_ptr_array                  lname_hash_table;
extern bit_32_switch_type                     lname_table_hash_size ; 
extern lname_entry_ptr_array                  lnames;
extern byte_ptr                               lseg_data_ptr;
extern string_ptr                             lst_extension_string;
extern file_info_list                         lst_file_list ;
extern lx_fixup_hold_ptr_array                lx_fixup_array ;
extern lx_section_ptr_array                   lx_outList ;
extern bit_32_switch_type                     lx_page_shift ;
extern bit_32_switch_type                     lx_page_size ;
extern bit_32                                 lx_stack_seg ;
extern bit_32_switch_type                     lx_version ;
extern boolean_switch_type                    lxfile;
extern bit_32_switch_type                     map ;
extern bit_32                                 map_start_time;
extern bit_32                                 max_exports ;
extern bit_32                                 max_externals ;
extern bit_32                                 max_groups ;
extern bit_32                                 max_imports ;
extern bit_32                                 max_lnames ;
extern bit_32                                 max_lx_fixups ;
extern bit_32                                 max_modules ;
extern bit_32                                 max_pe_fixups ;
extern bit_32                                 max_pe_import_fixups ;
extern bit_32                                 max_segments ;
extern string_ptr                             minus_string;
extern char                                  *mode_text[2];
extern Generic_Element_list                   module_publics;
extern module_ptr_array                       modules;
extern bit_16                                 more_tokens;
extern bit_16                                 n_exports ;
extern bit_16                                 n_externals;
extern bit_16                                 n_groups;
extern bit_16                                 n_imports ;
extern bit_16                                 n_lnames;
extern bit_32                                 n_lx_fixups;
extern bit_32                                 n_lx_sections;
extern bit_32                                 n_modules;
extern bit_32                                 n_pe_fixups ;
extern bit_32                                 n_pe_import_fixups ;
extern bit_32                                 n_pe_sections ;
extern bit_16                                 n_publics ;
extern bit_16                                 n_publics_to_sort;
extern bit_16                                 n_relocation_items ;
extern bit_16                                 n_resources ;
extern bit_16                                 n_segments;
extern public_entry_ptr                       near_communals ;
extern bit_32                                 next_available_address;
extern string_ptr                             next_token;
extern lname_entry_ptr                        none_lname;
extern string_ptr                             not_string;
extern string_ptr                             null_string;
extern string_ptr                             obj_extension_string;
extern file_info_list                         obj_file_list ;
extern obj_ptr_type                           obj_ptr;
extern boolean_switch_type                    objchecksum ;
extern byte                                   object_file_element[MAX_ELEMENT_SIZE];
extern bit_32                                 object_module_start_time;
extern bit_32_switch_type                     objectAlign ;
extern string_ptr                             open_angle_string;
extern string_ptr                             open_paren_string;
extern string_ptr                             or_string;
extern byte_ptr                               order_expression_char_ptr;
extern bit_32                                 order_start_time;
extern text_switch_type                       ordering ;
extern file_type                              outfile;
extern byte_ptr                               outfile_buffer;
extern pe_section_ptr_array                   outList ;
extern bit_32_switch_type                     pack_code ;
extern boolean_switch_type                    pad_segments ;
extern string_ptr                             parm_string;
extern boolean_switch_type                    pause ;
extern pe_change_attribs_list                 pe_change_attribs;
extern bit_32                                 PE_bss_seg ;
extern bit_32                                 PE_code_seg ;
extern bit_32                                 PE_data_seg ;
extern fixup_hold_ptr_array                   pe_fixup_array ;
extern pe_import_fixup_ptr_array              pe_import_fixup_array ;
extern boolean_switch_type                    pefile;
extern string_ptr                             plus_string;
extern byte                                   processing_library;
extern string_ptr                             program_config_file_string ;
extern string_ptr                             program_directory_string;
extern bit_16                                 prompting_for;
extern bit_16                                 prompt_next_stdin ;
extern public_entry_ptr_array                 public_hash_table;
extern bit_32_switch_type                     public_table_hash_size ;
extern public_entry_ptr_array                 public_sort_array;
extern bit_16                                 quoted ;
extern string_ptr                             resource_extension_string;
extern file_info_list                         resource_file_list;
extern resource_data_type                     resource_head ;
extern bit_32                                 resource_start_time;
extern bit_32                                 secondary_init_start_time;
extern segment_entry_ptr_array                segment_hash_table;
extern segment_entry_list                     segment_list ;
extern string_ptr                             segment_string;
extern bit_32_switch_type                     segment_table_hash_size ;
extern segment_entry_list                     segments_ordered_list;
extern segment_entry_list                     segments_unordered_list;
extern string_ptr                             semicolon_string;
extern lseg_ptr_array                         snames;
extern string_ptr                             space_string;
extern bit_32_switch_type                     stack ;
extern lname_entry_ptr                        STACK_lname;
extern bit_16                                 stack_segment_found ;
extern bit_32_switch_type                     stackCommitSize ;
extern bit_32_switch_type                     stackSize ;
extern string_ptr                             star_string;
extern fixup_type                             start_address;
extern bit_16                                 start_address_found ;
extern boolean_switch_type                    statistics ;
extern bit_32                                 statistics_start_time;
extern bit_32                                 step_time;
extern lseg_ptr                               stringseg_lseg ;
extern file_info_list                         stub_file_list;
extern text_switch_type                       stub_filename ;
extern boolean_switch_type                    symbol_table ;
extern string_ptr                             sym_extension_string;
extern file_info_list                         sym_file_list;
extern string_ptr                             sym_filename_string;
extern string_ptr                             sys_extension_string;
extern boolean_switch_type                    sysfile ;
extern thread_type                            target_thread[4];
extern byte                                   temp_near_string[TEMP_STRING_SIZE];
extern string_ptr                             temp_string;
extern string_ptr                             tilde_string;
extern char                                   time_array[16];
extern lname_entry_ptr                        tmodule_name;
extern bit_16                                 tmodule_number ;
extern string_ptr                             token;
extern byte                                   token_break_char;
extern bit_16                                 token_is_number;
extern bit_16                                 token_is_hex_number;
extern bit_32                                 token_numeric_value;
extern token_stack_list                       token_stack ;
extern token_stack_list                       token_stack_free_list ;
extern token_stack_ptr                        token_stack_member;
extern token_class_type                       token_type;
extern bit_32                                 total_time;
extern string_ptr                             true_string;
extern char                                  *type_text[5];
extern bit_32                                 user_input_start_time;
extern boolean_switch_type                    use32 ;
extern public_entry_type		     *virtual_segs;
extern boolean_switch_type                    win_subsystem ;

extern switch_table_type                      switch_table[] ;