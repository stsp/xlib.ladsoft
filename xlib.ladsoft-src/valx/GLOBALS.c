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
/*                                 GLOBALS.H                               */

/* Global variables used by the linker in approximate alphabetical order.  */

bit_32                                 address_base;
bit_32_switch_type                     align32 = 
                                        {0, 0, 0x100000, 0, False };
bit_32                                 align_mask[7]=
                                        {0L, 0L, 1L, 15L, 255L, 3L,4095L};
boolean_switch_type                    align_exe_header = {True};
char                                  *align_text[7]=
                                        {"Absolute",
                                         "Byte",
                                         "Word",
                                         "Paragraph",
                                         "Page",
                                         "Dword",
										 "MemPage"};
segment_entry_ptr                      active_segment;
#define Active_segment                 (*active_segment)
string_ptr                             ampersand_string;
string_ptr                             and_string;
string_ptr                             at_string;
string_ptr                             backslash_dot_string;
string_ptr                             backslash_dot_dot_string;
string_ptr                             backslash_string;
bit_32                                 bad_link ;
string_ptr                             bar_string;
lseg_ptr                               bseg_lseg ;
lname_entry_ptr                        BSS_lname;
bit_32_switch_type                     buffer_size = 
                                        {0x8000, 512, 0xFE00, 0x8000, False};
boolean_switch_type                    build_DLL  = { False } ;
lname_entry_ptr                        c_common_lname;
boolean_switch_type                    case_ignore = {True};
string_ptr                             class_string;
string_ptr                             close_paren_string;
string_ptr                             close_angle_string;
lname_entry_ptr                        CODE_lname;
lname_entry_ptr                        codeview_class_DEBBROWSE;
lname_entry_ptr                        codeview_class_DEBSYM;
lname_entry_ptr                        codeview_class_DEBTYP;
bit_16                                 codeview_information_present = False;
lname_entry_ptr                        codeview_segment_BROWSE;
lname_entry_ptr                        codeview_segment_SYMBOLS;
lname_entry_ptr                        codeview_segment_TYPES;
string_ptr                             colon_string;
string_ptr                             com_extension_string;
char                                  *combine_text[10]=
                                        {"Private",
                                         "Undefined",
                                         "Public",
                                         "Undefined",
                                         "Public",
                                         "Stack",
                                         "Common",
                                         "Public"
                                         "Undefined",
                                         "Blank Common"};
boolean_switch_type                    comfile = {False};
string_ptr                             comma_string;
lseg_ptr                               constseg_lseg ;
bit_32_switch_type                     CPARMAXALLOC =
                                        {0xFFFF, 0x0, 0xFFFF, 0xFFFF, False};
lseg_ptr                               cseg_lseg ;
string_ptr                             current_filename;
module_ptr                             current_module;
string_ptr                             current_path;
obj_record_header_ptr                  current_record_header;
#define Current_record_header          (*current_record_header)
bit_32                                 current_record_offset;
lseg_ptr                               cvdebsym_lseg ;
lseg_ptr                               cvdebtyp_lseg ;
string_ptr                             cv_extension_string;
file_info_list                         cv_file_list = { Null, Null };
lname_entry_ptr                        DATA_lname;
boolean_switch_type                    debug = {False};
string_ptr                             def_extension_string;
file_info_list                         def_file_list = {Null, Null};
bit_32                                 def_forward_id;
bit_32                                 def_lineno;
string_ptr                             default_directory_string;
string_ptr                             default_drive_string;
string_ptr                             default_extension;
string_ptr                             default_filename;
string_ptr                             default_lx_stub_string ;
char_ptr                               default_prompt;
lname_entry_ptr                        desc_lname;
bit_32_switch_type                     detail_level = 
                                        {0, 0, 5, 0, False};
string_ptr                             device_AUX;
string_ptr                             device_CON;
string_ptr                             device_PRN;
lname_entry_ptr                        DGROUP_lname;
string_ptr                             dll_extension_string;
boolean_switch_type                    DOSSEG = {False};
string_ptr                             dot_string;
lseg_ptr                               dseg_lseg ;
DTA_type                               DTA;
segment_entry_ptr                      edata_segment = Null;
obj_ptr_type                           end_of_record;
obj_ptr_type                           end_of_last_LIDATA_record;
segment_entry_ptr                      end_segment = Null;
string_ptr                             env_extension_string;
string_ptr                             exclamation_string;
string_ptr                             exe_extension_string;
file_info_list                         exe_file_list = {Null, Null};
EXE_header_ptr                         exe_header;
#define Exe_header                     (*exe_header)
bit_32                                 exe_header_size;
bit_32                                 exec_image_start_time;
boolean_switch_type                    exechecksum = {False};
bit_16                                 exefile;
public_entry_ptr_array                 exports;
public_entry_list                      external_list = {Null, Null};
public_entry_ptr_array                 externals;
string_ptr                             false_string;
lname_entry_ptr                        FAR_BSS_lname;
public_entry_ptr                       far_communals = Null;
bit_32_switch_type                     fileAlign =
																				{ 0x200,0x200,0x10000,0x200,False };
bit_32                                 first_pe_section_address ;
fixup_type                             fixup;
bit_16                                 FIXUPP_contains_only_threads;
bit_16                                 fixup_index;
fixup_list_ptr						   fixup_list ;
fixup_list_ptr						   fixup_list_tail ;
bit_32                                 fixup_start_time;
fixup_list_ptr						   fixup_temp ;
bit_32                                 FLAT_address ;
lname_entry_ptr                        FLAT_lname;
bit_16                                 frame_absolute;
thread_type                            frame_thread[4];
lname_entry_ptr                        generated_lname;
group_entry_ptr_array                  gnames;
group_entry_ptr_array                  group_hash_table;
group_entry_list                       group_list = {Null, Null};
bit_32_switch_type                     group_table_hash_size = 
                                          {25, 1, 1023, 25, False};
string_ptr                             group_string;
bit_32_switch_type                     heapCommitSize  =
                                                            { 0x2000, 0, 0xffffffff,0x0, False } ;
bit_32_switch_type                     heapSize = 
                                                            { 0x100000, 0, 0xffffffff,0x10000, False } ;
boolean_switch_type                    help;
string_ptr                             help_extension_string;
string_ptr                             help_filename;
bit_32                                 highest_uninitialized_byte;
lname_entry_ptr                        HUGE_BSS_lname;
public_entry_ptr                       huge_communals = Null;
bit_32_switch_type                     imageBase =
																				{ 0x400000, 0,0xffffffff,0x400000, False } ;
lseg_ptr                               import_thunk_table = NULL ;
public_entry_ptr_array                 imports;
file_type                              infile;
byte_ptr                               infile_buffer;
byte_ptr                               infile_buffer_2;
bit_16                                 initial_IP = 0;
bit_16                                 initial_CS = 0;
segment_entry_ptr                      largest_stack_seg;
#define Largest_stack_seg              (*largest_stack_seg)
bit_32                                 largest_stack_seg_length;
lseg_ptr                               last_LxDATA_lseg;
#define last_LxDATA_Lseg               (*last_LxDATA_lseg)
bit_32                                 last_LxDATA_offset;
bit_8                                  last_LxDATA_record_type;
obj_record_header_ptr                  last_LIDATA_record_header;
#define Last_LIDATA_record_header      (*last_LIDATA_record_header)
bit_8                                  last_LIDATA_record[MAX_ELEMENT_SIZE];
boolean_switch_type                    lefile;
string_ptr                             lib_extension_string;
text_switch_type                       lib_directory = { Null };
file_info_list                         lib_file_list = {Null, Null};
file_info_list                         lib_file_new_list = {Null, Null};
bit_32                                 library_directory_start_time;
bit_32                                 library_processing_start_time;
bit_16                                 library_request_count = 0;
bit_16                                 LIDATA_index;
bit_32                                 LIDATA_offset;
bit_16                                 link_step = 0;
bit_32                                 linker_end_time;
bit_32                                 linker_start_time;
lname_entry_ptr_array                  lname_hash_table;
bit_32_switch_type                     lname_table_hash_size = 
                                        {25, 1, 1023, 25, False};
lname_entry_ptr_array                  lnames;
byte_ptr                               lseg_data_ptr;
string_ptr                             lst_extension_string;
file_info_list                         lst_file_list = {Null, Null};
lx_fixup_hold_ptr_array                lx_fixup_array = Null ;
lx_section_ptr_array                   lx_outList = Null ;
bit_32_switch_type                     lx_page_shift =
                                         {12,7,15,12, False };
bit_32_switch_type                     lx_page_size =
                                         {4096,128,32768,4096, False };
bit_32                                 lx_stack_seg ;
bit_32_switch_type                     lx_version =
                                         { 0, 0, 0xffffffff,0, False};
boolean_switch_type                    lxfile;
bit_32_switch_type                     map =
                                        {2048, 1, 16384, 2048, False};
bit_32                                 map_start_time;
bit_32                                 max_exports = 512;
bit_32                                 max_externals = 512;
bit_32                                 max_groups = 512;
bit_32                                 max_imports = 512;
bit_32                                 max_lnames = 512;
bit_32                                 max_lx_fixups = 0 ;
bit_32                                 max_modules = 0;
bit_32                                 max_pe_fixups = 0;
bit_32                                 max_pe_import_fixups = 0;
bit_32                                 max_segments = 512;
string_ptr                             minus_string;
char                                  *mode_text[2]=
                                        {"Self-relative",
                                         "Segment-relative"};
Generic_Element_list                   module_publics;
module_ptr_array                       modules;
bit_16                                 more_tokens;
bit_16                                 n_exports;
bit_16                                 n_externals;
bit_16                                 n_groups;
bit_16                                 n_imports;
bit_16                                 n_lnames;
bit_32                                 n_lx_fixups;
bit_32                                 n_lx_sections;
bit_32                                 n_modules;
bit_32                                 n_pe_fixups =0;
bit_32                                 n_pe_import_fixups =0;
bit_32                                 n_pe_sections = 0 ;
bit_16                                 n_publics = 0;
bit_16                                 n_publics_to_sort;
bit_16                                 n_relocation_items = 0;
bit_16                                 n_resources = 0 ;
bit_16                                 n_segments;
public_entry_ptr                       near_communals = Null;
bit_32                                 next_available_address;
string_ptr                             next_token;
lname_entry_ptr                        none_lname;
string_ptr                             not_string;
string_ptr                             null_string;
string_ptr                             obj_extension_string;
file_info_list                         obj_file_list = {Null, Null};
obj_ptr_type                           obj_ptr;
boolean_switch_type                    objchecksum = {False};
byte                                   object_file_element[MAX_ELEMENT_SIZE];
bit_32                                 object_module_start_time;
bit_32_switch_type                     objectAlign =
																				{ 0x1000, 0x800, 0x10000000, 0x1000, False };
string_ptr                             open_angle_string;
string_ptr                             open_paren_string;
string_ptr                             or_string;
byte_ptr                               order_expression_char_ptr;
bit_32                                 order_start_time;
text_switch_type                       ordering = {Null};
file_type                              outfile;
byte_ptr                               outfile_buffer;
pe_section_ptr_array                   outList = Null;
bit_32_switch_type                     pack_code =
                                        {0xFFFA, 1, 0xFFFF, 0xFFFA, False};
boolean_switch_type                    pad_segments  = { False } ;
string_ptr                             parm_string;
boolean_switch_type                    pause = {False};
pe_change_attribs_list                 pe_change_attribs;
bit_32                                 PE_bss_seg ;
bit_32                                 PE_code_seg ;
bit_32                                 PE_data_seg ;
fixup_hold_ptr_array                   pe_fixup_array = Null ;
pe_import_fixup_ptr_array              pe_import_fixup_array = Null ;
boolean_switch_type                    pefile;
string_ptr                             plus_string;
byte                                   processing_library;
string_ptr                             program_config_file_string ;
string_ptr                             program_directory_string;
bit_16                                 prompting_for;
bit_16                                 prompt_next_stdin = False;
public_entry_ptr_array                 public_hash_table;
public_entry_ptr_array                 public_sort_array;
bit_32_switch_type                     public_table_hash_size =
                                        {525, 1, 1023, 525, False};
bit_16                                 quoted ;
string_ptr                             resource_extension_string;
file_info_list                         resource_file_list;
file_info_list                         resource_file_list = {Null, Null};
resource_data_type										 resource_head ; /* Relying on BSS to zero this out */
bit_32                                 resource_start_time;
bit_32                                 secondary_init_start_time;
segment_entry_ptr_array                segment_hash_table;
segment_entry_list                     segment_list = {Null, Null};
string_ptr                             segment_string;
bit_32_switch_type                     segment_table_hash_size =
                                        {25, 1, 1023, 25, False};
segment_entry_list                     segments_ordered_list;
segment_entry_list                     segments_unordered_list;
string_ptr                             semicolon_string;
lseg_ptr_array                         snames;
string_ptr                             space_string;
bit_32_switch_type                     stack =
                                        {0, 0, 0xFFFF, 0, False};
lname_entry_ptr                        STACK_lname;
bit_16                                 stack_segment_found = False;
bit_32_switch_type                     stackCommitSize =
																				{ 0x2000, 0x1000, 0xffffffff, 0x2000, False } ;
bit_32_switch_type                     stackSize =
                                                            { 0x100000, 0x4000, 0xffffffff, 0x200000, False } ;
string_ptr                             star_string;
fixup_type                             start_address;
bit_16                                 start_address_found = False;
boolean_switch_type                    statistics = {False};
bit_32                                 statistics_start_time;
bit_32                                 step_time;
lseg_ptr                               stringseg_lseg ;
file_info_list                         stub_file_list = { Null };
text_switch_type                       stub_filename = {Null};
boolean_switch_type                    symbol_table = {False} ;
string_ptr                             sym_extension_string;
file_info_list                         sym_file_list = { Null };
string_ptr                             sym_filename_string;
string_ptr                             sys_extension_string;
boolean_switch_type                    sysfile = {False};
thread_type                            target_thread[4];
byte                                   temp_near_string[TEMP_STRING_SIZE];
string_ptr                             temp_string;
string_ptr                             tilde_string;
char                                   time_array[16];
lname_entry_ptr                        tmodule_name;
bit_16                                 tmodule_number = 0;
string_ptr                             token;
byte                                   token_break_char;
bit_16                                 token_is_number;
bit_16                                 token_is_hex_number;
bit_32                                 token_numeric_value;
token_stack_list                       token_stack = {Null, Null};
token_stack_list                       token_stack_free_list = {Null, Null};
token_stack_ptr                        token_stack_member;
token_class_type                       token_type;
bit_32                                 total_time;
string_ptr                             true_string;
char                                  *type_text[5]=
                                        {"Low Byte",
                                         "Offset",
                                         "Base",
                                         "Pointer",
                                         "High Byte"};
bit_32                                 user_input_start_time;
boolean_switch_type                    use32 = {False};
public_entry_type 		     						*virtual_segs;
boolean_switch_type                    win_subsystem = { False } ;

switch_table_type                      switch_table[] =
{
 {2, "ah",  "alignexeheader",  &align_exe_header,        scan_set_switch},
 {2, "a32", "align32",         &align32,                 scan_bit_32_switch},
 {3, "bdl", "buildDll",        &build_DLL,               scan_set_switch},
 {2, "bsz", "buffersize",      &buffer_size,             scan_bit_32_switch},
 {2, "ci",  "caseignore",      &case_ignore,             scan_set_switch},
 {2, "co",  "comfile",         &comfile,                 scan_set_switch},
 {3, "con", "conSubSys",       &win_subsystem,           scan_reset_switch},
 {2, "cp",  "cparmaxalloc",    &CPARMAXALLOC,            scan_bit_32_switch},
 {2, "deb", "debug",           &debug,                   scan_set_switch} ,
 {2, "det", "detaillevel",     &detail_level,            scan_bit_32_switch},
 {2, "do",  "dosseg",          &DOSSEG,                  scan_set_switch},
 {2, "ex",  "exechecksum",     &exechecksum,             scan_set_switch},
 {2, "fa",  "fileAlign",       &fileAlign,               scan_bit_32_align_switch},
 {2, "ghs", "grouphashsize",   &group_table_hash_size,   scan_bit_32_switch},
 {2, "hlp", "help",            &help,                    scan_help_switch},
 {2, "?",   "help",            &help,                    scan_help_switch},
 {2, "h",   "help",            &help,                    scan_help_switch},
 {2, "l",   "libdir",          &lib_directory,           scan_string_switch} ,
 {2, "le",  "le",              &lefile,                  scan_set_switch} ,
 {2, "lhs", "lnamehashsize",   &lname_table_hash_size,   scan_bit_32_switch},
 {8, "lx",  "lx",              &lxfile,                  scan_set_switch},
 {8, "lxp", "lxpagesize",      &lx_page_size,            scan_bit_32_switch},
 {8, "lxs", "lxpageshift",     &lx_page_shift,           scan_bit_32_switch},
 {3, "lxv", "lxversion",       &lx_version,              scan_bit_32_switch},
 {3, "mp",  "map",             &map,                     scan_opt_bit_16},
 {4, "n32", "nouse32",         &use32,                   scan_reset_switch},
 {4, "nah", "noalignexeheader",&align_exe_header,        scan_reset_switch},
 {4, "nci", "nocaseignore",    &case_ignore,             scan_reset_switch},
 {4, "nco", "nocomfile",       &comfile,                 scan_reset_switch},
 {3, "ndl", "nobuildDll",      &build_DLL,               scan_reset_switch} ,
 {4, "ndo", "nodosseg",        &DOSSEG,                  scan_reset_switch},
 {4, "nex", "noexechecksum",   &exechecksum,             scan_reset_switch},
 {2, "nle", "nole",            &lefile,                  scan_reset_switch} ,
 {4, "nlx", "nolx",            &lxfile,                  scan_reset_switch},
 {4, "nmp", "nomap",           &map,                     scan_reset_bit_16},
 {4, "nob", "noobjchecksum",   &objchecksum,             scan_reset_switch},
 {5, "npc", "nopackcode",      &pack_code,               scan_reset_bit_16},
 {4, "npe", "nope",            &pefile,                  scan_reset_switch},
 {5, "npa", "nopause",         &pause,                   scan_reset_switch},
 {3, "nps", "nopadSegments",   &pad_segments,            scan_reset_switch} ,
 {5, "nsym","nosymboltable",   &symbol_table,            scan_reset_switch},
 {5, "nsy", "nosysfile",       &sysfile,                 scan_reset_switch},
 {4, "nst", "nostatistics",    &statistics,              scan_reset_switch},
 {2, "oa",  "objectAlign",     &objectAlign,             scan_bit_32_align_switch},
 {2, "ob",  "objchecksum",     &objchecksum,             scan_set_switch},
 {2, "ord", "order",           &ordering,                scan_text_switch},
 {3, "pac", "packcode",        &pack_code,               scan_opt_bit_16},
 {3, "pa",  "pause",           &pause,                   scan_set_switch},
 {3, "pas", "padSegments",     &pad_segments,            scan_set_switch} ,
 {4, "pe",  "pe",              &pefile,                  scan_set_switch},
 {2, "ph",  "peHeapSize",      &heapSize,                scan_bit_32_switch},
 {2, "phc", "peHeapCommit",    &heapCommitSize,          scan_bit_32_switch},
 {2, "phs", "publichashsize",  &public_table_hash_size,  scan_bit_32_switch},
 {2, "pib", "peImageBase",     &imageBase,               scan_bit_32_switch},
 {2, "ps",  "peStackSize",     &stackSize,               scan_bit_32_switch},
 {2, "psc", "peStackCommit",   &stackCommitSize,         scan_bit_32_switch},
 {2, "shs", "segmenthashsize", &segment_table_hash_size, scan_bit_32_switch},
 {2, "stb", "stubfile",        &stub_filename,           scan_string_switch},
 {4, "stk", "stack",           &stack,                   scan_bit_32_switch},
 {2, "sts", "statistics",      &statistics,              scan_set_switch},
 {3, "sym", "symbolTable",     &symbol_table,            scan_set_switch},
 {3, "sy",  "sysfile",         &sysfile,                 scan_set_switch},
 {3, "win", "winSubSys",       &win_subsystem,           scan_set_switch},
 {4, "32",  "use32",           &use32,                   scan_set_switch},
 {0, Null,  Null,              Null,                     Null}
};