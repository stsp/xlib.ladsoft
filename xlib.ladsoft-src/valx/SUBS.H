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
/*                                 SUBS.H                                  */

string_ptr
      add_extension_to_file            (string_ptr fn,
                                        string_ptr ext);
void
      add_files_to_list                (file_info_list    *file_list,
                                        string_ptr         fn);
void
      align_active_segment             (void);
byte_ptr
      allocate_memory                  (bit_32             size);
string_ptr
      allocate_string                  (bit_16             length);
string_ptr
      change_extension                 (string_ptr         fn,
                                        string_ptr         ext);
bit_16
      checksum                         (bit_32             len,
                                        byte              *sym);
int_16
      compare_short_string             (string_ptr         left,
                                        string_ptr         right);
int_16
      compare_string                   (string_ptr         left,
                                        string_ptr         right);
void
      complete_a_filename_token        (void);
string_ptr
      concat_string                    (string_ptr         dest,
                                        string_ptr         source);
string_ptr
      concat_char_to_string            (string_ptr         dest,
                                        byte               c);
string_ptr
      copy_string                      (string_ptr         dest,
                                        string_ptr         source);
void
      create_codeview_data             (pe_header_ptr      pehead,
										byte_ptr 		  *data_ptr, 
                                        bit_32_ptr		   data_len);

bit_32 
      createLXOutputSection            (bit_32             flags) ;

bit_32 
      createOutputSection              (byte_ptr           name, 
                                        bit_32             winflags) ;
string_ptr
      cut_string                       (string_ptr         dest,
                                        bit_16             at,
                                        bit_16             len);
bit_16
      continue_file_search             (void);
string_ptr
      default_directory                (string_ptr         drive,
                                        string_ptr         directory);
string_ptr
      default_drive                    (void);

string_ptr
      duplicate_string                 (string_ptr          s);
void
      eat_white_space                  (void);
string_ptr
      edit_number_string               (string_ptr         s,
                                        char_ptr           format,
                                        ...);
char_ptr
      elapsed_time                     (bit_32             start,
                                        bit_32             stop);
void
      end_linker                       (bit_16             return_code);
	  
bit_32
      lx_frame_address                 (public_entry_ptr pub);
	  
bit_32
      lx_target_address                (public_entry_ptr pub);
	  
bit_32 
      enter_lx_fixup                   (bit_32             target, 
										bit_32             offset,
                                        bit_32             address, 
                                        loc_type location)  ;

void
      enter_pe_fixup                   (bit_32             address,
                                        loc_type           location) ;
void
      enter_pe_import_fixup            (byte_ptr           location_ptr,
				        bit_32             location_address,
                                        public_entry_ptr   import,
                                        bit_32             isdata) ;
int_32
      far_compare                      (byte_ptr           left,
                                        byte_ptr           right,
                                        bit_32             len);
bit_32
      far_index                        (byte_ptr           dest,
                                        byte               c);
bit_32
      far_match                        (byte_ptr           pattern,
                                        byte_ptr           source,
                                        bit_32             len);
byte_ptr
      far_move                         (byte_ptr           dest,
                                        byte_ptr           source,
                                        bit_32             len);
byte_ptr
      far_move_left                    (byte_ptr           dest,
                                        byte_ptr           source,
                                        bit_32             len);
byte_ptr
      far_set                          (byte_ptr           dest,
                                        byte               source,
                                        bit_32             len);
byte_ptr
      far_to_lower                     (byte_ptr           dest,
                                        bit_32             len);
void
      file_close_for_read              (void);
void
      file_close_for_write             (void);
void
      file_delete                      (file_info_ptr      file_info);
bit_16
      file_exists                      (string_ptr         fn,
                                        bit_16             attr);
void
      file_IO_limit                    (bit_16             limit);
void
      file_open_for_read               (file_info_ptr      file_info);
void
      file_open_for_write              (file_info_ptr      file_info);
void
      file_position                    (bit_32             position);
void
      file_read                        (byte_ptr           into,
                                        bit_32             length);
void 
      file_read_line                   (byte_ptr           into,
                                        bit_32             length);
void
      file_write                       (byte_ptr           from,
                                        bit_32             length);
void
      fixup_FIXUPP_record              (void);
void
      fixup_FORREF_record              (void);
void
      fixup_LEDATA_record              (void);
void
      fixup_LIDATA_IDB                 (void);
void
      fixup_LIDATA_record              (void);
void
      fixups                           (void);
bit_32
      frame                            (void);
void
      get_filename_token               (bit_16             required,
                                        file_info_list    *list);
void
      get_filenames_from_user          (bit_16             argc,
                                        byte              *argv[]);
bit_32 
      get_read_file_position           (void);
token_stack_ptr
      get_free_token_source_element    (void);
void
      get_order_token                  (void);
bit_32
      get_time                         (void);
bit_32
      import_count                     (void);
bit_16
      index_string                     (string_ptr         s,
                                        bit_16             from,
                                        string_ptr         pattern);
void
      initialize_memory                (void);
FILE
     *input_open                       (string_ptr         fn);
void
      library_directory_hash           (byte_ptr           sym,
                                        bit_32             len,
                                        bit_16            *starting_block,
                                        bit_16            *delta_block,
                                        bit_16            *starting_entry,
                                        bit_16            *delta_entry);
void
      linker_error                     (int_16             severity,
                                        char_ptr           format,
                                        ...);
void
      linker_message                   (char_ptr           format,
                                        ...);
void
      linker_statistics                (void);
void
      link_map                         (void);
void
      ListDelete                       (Generic_Element_ptr   elem,
                                        Generic_Element_list     *lst);
void
      ListInsert                       (Generic_Element_ptr   elem,
                                        bit_16                type_insert,
                                        Generic_Element_ptr   aftr,
                                        Generic_Element_list     *lst);
void
      ListPop                          (Generic_Element_list     *lst,
                                        Generic_Element_ptr  *elem);
void 
      load_stub                        (byte_ptr          *pstubData,
                                        bit_32_ptr         pstubSize) ;
group_entry_ptr
      lookup_group                     (lname_entry_ptr    group_lname);
lname_entry_ptr
      lookup_lname                     (bit_16             len,
                                        byte              *sym);
public_entry_ptr
      lookup_public                    (bit_16             len,
                                        byte              *sym,
                                        bit_16             module);
segment_entry_ptr
      lookup_segment                   (lname_entry_ptr    segment_lname,
                                        lname_entry_ptr    class_lname,
                                        combine_type       combine);
string_ptr
      lowercase_string                 (string_ptr         s);
string_ptr
      make_constant_string             (byte              *s);
void
      make_EXE_header                  (void);
void 
      map_memory                       (byte_ptr           data,
                                        bit_32             address,
                                        bit_32             length);
bit_16
      match_pattern                    (string_ptr         pattern,
                                        string_ptr         s);
byte
     *near_string                      (string_ptr         s);
bit_16
      obj_COMDEF                       (void);
bit_16
      obj_COMENT                       (void);
bit_16
      obj_component                    (void);
bit_16
      obj_content_def                  (void);
bit_16
      obj_data                         (void);
bit_16
      obj_data_record                  (void);
bit_16
      obj_debug_record                 (void);
bit_16
      obj_EXTDEF                       (void);
bit_16
      obj_FIXUPP                       (void);
void
      obj_FIXUPP_fixup                 (void);
void
      obj_fixup_LIDATA                 (void);
void
      obj_fixup_LIDATA_IDB             (void);
void
      obj_FIXUPP_thread                (void);
bit_16
      obj_FORREF                       (void);
lseg_ptr 
      obj_generate_segment             (lname_entry_ptr    segment_lname,
                                        lname_entry_ptr    class_lname,
                                        combine_type       combine,
                                        bit_8              align,
                                        lname_entry_ptr    tmodule,
                                        file_info_ptr      file,
                                        bit_32             address,
                                        bit_32             length);
bit_16
      obj_GRPDEF                       (void);
bit_16
      obj_index_external               (void);
bit_16
      obj_index_group                  (void);
bit_16
      obj_index_LNAME                  (void);
bit_16
      obj_index_segment                (void);
void
      obj_iterated_data_block          (bit_32 use32);
bit_32
      obj_iterated_data_block_length   (bit_32 use32);
bit_32
      obj_leaf_descriptor              (void);
bit_16
      obj_LEDATA                       (void);
bit_16
      obj_LIDATA                       (void);
bit_32
      obj_LIDATA_length                (bit_32 use32);
bit_16
      obj_LINNUM                       (void);
bit_16
      obj_LNAMES                       (void);
bit_16
      obj_MODEND                       (void);
bit_16
      obj_MODEXT                       (void);
bit_16
      obj_MODPUB                       (void);
bit_16
      obj_modtail                      (void);
lname_entry_ptr
      obj_name                         (void);
bit_16
      obj_name_length                  (void);
void
      obj_next_record                  (void);
bit_16
      obj_PUBDEF                       (void);
bit_16
      obj_SEGDEF                       (void);
bit_16
      obj_seg_grp                      (void);
bit_16
      obj_thread_def                   (void);
bit_16
      obj_tmodule                      (void);
bit_16
      obj_THEADR                       (void);
bit_16
      obj_TYPDEF                       (void);
void
      order_and_align_segments         (void);
bit_16
      order_expression                 (void);
bit_16
      order_factor                     (void);
void
      order_prologue                   (void);
bit_16
      order_primary                    (void);
bit_16
      order_term                       (void);
void
      order_token_get_char             (void);
void
      parse_object_module              (void);
void
      pass_two                         (void);    
string_ptr
      paste_string                     (string_ptr         dest,
                                        bit_16             at,
                                        string_ptr         s);
void
      primary_linker_initialization    (byte              *program_directory);
void
      print                            (char_ptr           format,
                                        ...);
void 
      process_def_file                 (void);
string_ptr
      process_filename                 (string_ptr         fn);
void
      process_libraries                (void);
void
      process_library                  (file_info_ptr      lib_file);
void
      process_library_directories      (void);
void
      process_library_directory        (file_info_ptr      lib_file);
void
      process_object_modules           (void);
void 
      process_resource_files           (void);
void
      process_switch                   (void);
void
      process_user_input_files         (file_info_list    *list,
                                        bit_16             required);
void
      process_user_output_file         (file_info_list    *list,
                                        bit_16             required);
bit_32
      public_frame_address             (public_entry_ptr   pub);
bit_32
      public_target_address            (public_entry_ptr   pub);
byte_ptr
      reallocate_memory                (ptr                mem, 
                                        bit_32             size);
void
      release_memory                   (byte_ptr mem);
bit_16
      reverse_index_string             (string_ptr         s,
                                        bit_16             from,
                                        string_ptr         pattern);
void 
      scan_bit_32_align_switch         (switch_table_ptr   current_switch);
void
      scan_bit_32_switch               (switch_table_ptr   current_switch);
void
      scan_help_switch                 (switch_table_ptr   current_switch);
void
      scan_opt_bit_16                  (switch_table_ptr   current_switch);
void
      scan_out_token                   (void);
void
      scan_reset_bit_16                (switch_table_ptr   current_switch);
void
      scan_reset_switch                (switch_table_ptr   current_switch);
void
      scan_set_switch                  (switch_table_ptr   current_switch);
void
      scan_string_switch               (switch_table_ptr   current_switch);
void
      scan_text_switch                 (switch_table_ptr   current_switch);
void
      secondary_linker_initialization  (void);
bit_32
      segment_offset                   (lseg_ptr           lseg,
                                        bit_32             offset);
void
      sort_directory                   (bit_16             left,
                                        bit_16             right);
void
      sort_publics_by_name             (bit_16             left,
                                        bit_16             right);
void
      sort_publics_by_value            (bit_16             left,
                                        bit_16             right);
bit_16
      start_file_search                (string_ptr         fn,
                                        bit_16             attr);
string_ptr
      string                           (byte_ptr           s);
string_ptr
      substr                           (string_ptr         s,
                                        bit_16             at,
                                        bit_16             len);
bit_32
      target                           (public_entry_ptr  *import);
void
      token_get_char                   (void);
string_ptr
      trunc_string                     (string_ptr         s,
                                        bit_16             at);

char 
      *unmangle                        (char *name) ;

bit_16
      word_checksum                    (bit_32             len,
                                        bit_32             address,
                                        byte_ptr           data);
void
      write_executable_image           (void);
void 
	  write_fixupp					   (bit_8           	rec_typ,
                     					lseg_ptr        	lseg,
                     					bit_32          	offset,
                     					byte_ptr        	data,
                     					bit_16          	len) ;
void 
      write_gap                        (bit_32             length);
void 
      Write_lx_image                     (void);
void
      write_pe_image                   (void);
int_32
      wstrcmp                          (bit_16_ptr         arg1,
                                        bit_16_ptr         arg2);
void
      wstrcpy                          (bit_16_ptr         dst,
                                        bit_16_ptr         src);
bit_32
      wstrlen                          (bit_16_ptr         str) ;
