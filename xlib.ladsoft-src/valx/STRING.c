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
/*                                 STRING.C                                */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                        allocate_string                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr allocate_string(bit_16 len)
BeginDeclarations
string_ptr                             temp;
#define Temp                           (*temp)
EndDeclarations
BeginCode
 temp = StringPtr(allocate_memory(Bit_32(sizeof(string_type))+ \
                                        Bit_32(len)));
 Temp.max_length = len;
 Temp.length     = 0;
 Temp.text[0]    = '\000';
 return(temp);
EndCode
#undef Temp

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         compare_string                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
int_16 compare_string(string_ptr left, string_ptr right)
BeginDeclarations
bit_16                                 left_len;
bit_16                                 right_len;
EndDeclarations
BeginCode
 left_len  = Length(left);
 right_len = Length(right);
 If (left_len IsZero) AndIf (right_len IsZero)
  Then
   return(0);
  EndIf;
 If left_len LessThan right_len
  Then
   return(-1);
  EndIf;
 If left_len GreaterThan right_len
  Then
   return(1);
  EndIf;
 return(far_compare(String(left), String(right), left_len));
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         compare_short_string                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
int_16 compare_short_string(string_ptr left, string_ptr right)
BeginDeclarations
bit_16                                 left_len;
EndDeclarations
BeginCode
 left_len = Length(left);
 If left_len IsZero
  Then
   return(0);
  EndIf;
 return(far_compare(String(left), String(right), left_len));
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         concat_string                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr concat_string(string_ptr dest, string_ptr source)
BeginDeclarations
bit_16                                 temp;
EndDeclarations
BeginCode
 If (Length(source) + Length(dest)) Exceeds MaxLength(dest)
  Then
   linker_error(8,"Destination string to small (%u bytes) to hold "
                  "concatination of:\n"
                  "\t\"%s\" (%u bytes) and\n"
                  "\t\"%s\" (%u bytes)\n",
                  MaxLength(dest),
                  String(dest), Length(dest),
                  String(source), Length(dest));
  EndIf;
 temp = Length(source);
 far_move(Addr(String(dest)[Length(dest)]), String(source), temp + 1);
 Length(dest) += temp;
 return(dest);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         concat_char_to_string                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr concat_char_to_string(string_ptr dest, byte c)
BeginDeclarations
EndDeclarations
BeginCode
 If (Length(dest) + 1) Exceeds MaxLength(dest)
  Then
   linker_error(8,"Destination string to small (%u bytes) to add \"%c\" "
                  "to string:\n"
                  "\t\"%s\" (%u bytes)\n",
                  MaxLength(dest), c,
                  String(dest), Length(dest));
  EndIf;
 String(dest)[Length(dest)++] = c;
 String(dest)[Length(dest)]   = '\000';
 return(dest);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         copy_string                                     |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr copy_string (string_ptr dest, string_ptr source)
BeginDeclarations
EndDeclarations
BeginCode
 If Length(source) Exceeds MaxLength(dest)
  Then
   linker_error(8,"Destination string to small (%u bytes) to hold:\n"
                  "\t\"%s\" (%u bytes)\n",
                  MaxLength(dest),
                  String(source), Length(dest));
  EndIf;
 Length(dest) = Length(source);
 far_move(String(dest), String(source), Length(source)+1);
 return(dest);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            cut_string                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr cut_string(string_ptr s, bit_16 at, bit_16 len)
BeginDeclarations
bit_16                                 length_to_right;
bit_16                                 string_length;
EndDeclarations
BeginCode
 string_length = Length(s);
 If string_length NotGreaterThan at
  Then
   return(s);
  EndIf;
 If string_length LessThan (at + len)
  Then
   len = string_length - at;
  EndIf;
 Length(s)       -= len;
 length_to_right  = string_length - (at+len);
 far_move(Addr(String(s)[at]), Addr(String(s)[at+len]), length_to_right+1);
 return(s);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            duplicate_string                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr duplicate_string(string_ptr s)
BeginDeclarations
string_ptr                             temp;
#define Temp                           (*temp)
EndDeclarations
BeginCode
 temp = StringPtr(allocate_memory( \
                                  Bit_32(sizeof(string_type))+ \
                                  Bit_32(Length(s))));
 Temp.max_length = Length(s);
 copy_string(temp, s);
 return(temp);
EndCode
#undef Temp

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           edit_number_string                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr edit_number_string(string_ptr s, char_ptr format, ...)
BeginDeclarations
va_list                                argptr;
bit_16                                 i;
EndDeclarations
BeginCode
 va_start(argptr,format);
 vsprintf((char_ptr) temp_near_string, format, argptr);
 copy_string(s, string(temp_near_string));
 i = Length(s);
 While i Exceeds 3
  BeginWhile
   i -= 3;
   paste_string(s, i, comma_string);
  EndWhile;
 return(s);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           index_string                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 index_string(string_ptr s, bit_16 from, string_ptr pattern)
BeginDeclarations
bit_16                                 i;
bit_16                                 iteration_count;
byte_ptr                               left;
bit_16                                 len;
byte_ptr                               pat;
bit_16                                 pattern_length;
EndDeclarations
BeginCode
 pattern_length = Length(pattern);
 pat            = String(pattern);
 len            = Length(s);
 If (len - from) LessThan pattern_length
  Then
   return(0xFFFF);
  EndIf;
 iteration_count = len - pattern_length + 1;
 left  = Addr(String(s)[from]);
 For i=from; i LessThan iteration_count; i++
  BeginFor
   If far_compare(left++, pat, pattern_length) IsZero
    Then
     return(i);
    EndIf;
  EndFor;
 return(0xFFFF);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       lowercase_string                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr lowercase_string(string_ptr s)
BeginDeclarations
EndDeclarations
BeginCode
 far_to_lower(String(s), Length(s));
 return(s);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                      make_constant_string                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr make_constant_string(byte *s)
BeginDeclarations
bit_16                                 len;
string_ptr                             temp;
EndDeclarations
BeginCode
 len  = strlen(CharPtr(s));
 temp = StringPtr(allocate_memory(  \
                                  Bit_32(sizeof(string_type))+Bit_32(len)));
 Length(temp)    =
 MaxLength(temp) = len++;
 far_move(String(temp), BytePtr(s), len);
 return(temp);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             match_pattern                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 match_pattern(string_ptr pattern, string_ptr s)
BeginDeclarations
bit_16                                 i;
bit_16                                 n_searches;
byte_ptr                               source;
EndDeclarations
BeginCode
 If FirstCharIn(pattern) Is '*'
  Then
   cut_string(pattern, 0, 1);
   If LastCharIn(pattern) Is '*'
    Then /* We must perform exhaustive search */
     cut_string(pattern, 0, Length(pattern)-1);
     If Length(pattern) Exceeds Length(s)
      Then
       return(False);
      EndIf;
     n_searches = Length(s) - Length(pattern) + 1;
     source = String(s);
     For i=0; i<n_searches; i++
      BeginFor
       If far_match(String(pattern), source++, Length(pattern))
        Then
         return(True);
        EndIf;
      EndFor;
    Else /* We must match only the tail */
     If Length(pattern) Exceeds Length(s)
      Then
       return(False);
      Else
       return(far_match(String(pattern),
                        BytePtr(Addr(String(s)[Length(s)-Length(pattern)])),
                        Length(pattern)));
      EndIf;
    EndIf;
  Else /* We must match only the front */
   return(far_match(String(pattern), String(s), Length(pattern)));
  EndIf;
 return(False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             near_string                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
byte *near_string(string_ptr s)
BeginDeclarations
EndDeclarations
BeginCode
 far_move(temp_near_string, String(s), Length(s)+1);
 return(temp_near_string);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            paste_string                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr paste_string(string_ptr dest, bit_16 at, string_ptr s)
BeginDeclarations
bit_16                                 length_inserted_string;
bit_16                                 length_string_to_right;
bit_16                                 length_string;
EndDeclarations
BeginCode
 length_string          = Length(dest);
 length_inserted_string = Length(s);
 If (length_string + length_inserted_string) Exceeds MaxLength(dest)
  Then
   linker_error(8,"Destination string too small (%u bytes) to insert:\n"
                  "\t\"%s\" (%u bytes) into\n"
                  "\t\"%s\" (%u bytes)\n",
                  MaxLength(dest),
                  String(s), length_inserted_string,
                  String(dest), length_string);
  EndIf;
 If length_inserted_string IsZero
  Then
   return(dest);
  EndIf;
 If at Exceeds length_string
  Then
   at = length_string;
  EndIf;
 length_string_to_right = length_string - at;
 far_move_left(Addr(String(dest)[at+length_inserted_string]),
               Addr(String(dest)[at]),
               length_string_to_right+1);
 far_move(Addr(String(dest)[at]), String(s), length_inserted_string);
 Length(dest) += length_inserted_string;
 return(dest);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         reverse_index_string                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 reverse_index_string(string_ptr s, bit_16 from, string_ptr pattern)
BeginDeclarations
bit_16                                 i;
bit_16                                 iteration_count;
bit_16                                 len;
byte_ptr                               right;
byte_ptr                               pat;
bit_16                                 pattern_length;
EndDeclarations
BeginCode
 pattern_length = Length(pattern);
 len            = Length(s);
 pat            = String(pattern);
 If len LessThan pattern_length
  Then
   return(0xFFFF);
  EndIf;
 If from Exceeds (len - pattern_length)
  Then
   from = len - pattern_length;
  EndIf;
 iteration_count = from + 1;
 right = Addr(String(s)[from]);
 For i=0; i LessThan iteration_count; i++, from--
  BeginFor
   If far_compare(right--, pat, pattern_length) IsZero
    Then
     return(from);
    EndIf;
  EndFor;
 return(0xFFFF);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               string                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr string(byte_ptr s)
BeginDeclarations
bit_16                                 len;
EndDeclarations
BeginCode
 len  = far_index(s, 0);
 If len Exceeds MaxLength(temp_string)
  Then
   linker_error(8,"Destination string too small (%u bytes) to hold:\n"
                  "\t\"%s\" (%u bytes)\n",
                  MaxLength(temp_string),
                  s, len);
  EndIf;
 far_move(String(temp_string), s, len+1);
 Length(temp_string) = len;
 return(temp_string);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               substr                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr substr(string_ptr s, bit_16 at, bit_16 len)
BeginDeclarations
bit_16                                 string_length;
EndDeclarations
BeginCode
 string_length = Length(s);
 If string_length IsZero
  Then
   return(null_string);
  EndIf;
 If at NotLessThan string_length
  Then
   at = string_length - 1;
  EndIf;
 If len Exceeds Bit_16(string_length - at)
  Then
   len = string_length - at;
  EndIf;
 If len Exceeds MaxLength(temp_string)
  Then
   linker_error(8,"Destination string too small in SUBSTR operation\n");
  EndIf;
 far_move(String(temp_string), Addr(String(s)[at]), len+1);
 Length(temp_string) = len;
 return(temp_string);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            trunc_string                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr trunc_string(string_ptr s, bit_16 at)
BeginDeclarations
EndDeclarations
BeginCode
 If Length(s) NotGreaterThan at+1
  Then
   return(s);
  EndIf;
 Length(s)     = at;
 String(s)[at] = '\000';
 return(s);
EndCode

