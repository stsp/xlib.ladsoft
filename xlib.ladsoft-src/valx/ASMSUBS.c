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
/*                                ASMSUBS.C                                 */

/* had to totally rewrite for 32-bit, DAL */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <dos.h>
#include <string.h>

#include "langext.h"
#include "defines.h"
#include "types.h"
#include "subs.h"

#pragma inline

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               checksum                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 checksum(bit_32 len, byte *sym)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 For i=0; i LessThan len; i++
  BeginFor
   sum += sym[i];
  EndFor;
 return(sum); */

 asm            push    esi
 asm            push    edi
 asm            push    ebx
 asm            mov     esi,[sym]
 asm            mov     ecx,[len]
 asm            xor     eax,eax
 asm            jecxz   hash_x
 asm            mov     ebx,eax
hash_loop:
 asm            mov     bl,[esi]
 asm            add     ax,bx
 asm            inc     esi
 asm            loop    hash_loop
hash_x:
 asm            pop     ebx
 asm            pop     edi
 asm            pop     esi
 return(_EAX);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          far_compare                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
int_32 far_compare (byte_ptr left, byte_ptr right, bit_32 len)
BeginDeclarations
int								rv;
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 While len-- IsNotZero
  BeginWhile
   If *left IsNotEqualTo *right
    Then
     If *left LessThan *right
      Then
       return(-1);
      Else
       return(1);
      EndIf;
    EndIf;
   left++;
   right++;
  EndWhile;
 return(0); */

 asm            push    esi
 asm            push    edi
 asm            cld
 asm            mov     ecx,[len]
 asm            mov     edi,[right]
 asm            mov     esi,[left]
 asm    rep     cmpsb
 asm            pop     edi
 asm            pop     esi
 asm            mov     [rv],-1
 asm            jb      xit
 asm            mov     [rv], 1
 asm            ja      xit
 asm            mov     [rv], 0
xit:
 return(rv);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            far_index                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 far_index(byte_ptr dest, byte c)
BeginDeclarations
bit_32                                 i = 0;
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 For i=0; (*dest++ IsNotEqualTo c) AndIf (i IsNotEqualTo 0xFFFF); i++
  BeginFor
  EndFor;
 return(i); */

 asm            push    edi
 asm            xor     ecx,ecx
 asm            mov     al,[c]
 asm            mov     edi,[dest]
search_loop:
 asm            cmp     al,[edi]
 asm            je      search_done
 asm            inc     edi
 asm            inc     ecx
 asm            jne     search_loop
 asm            dec     ecx
search_done:
 asm            mov     [i],ecx
 asm            pop     edi

 return(i);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              far_match                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 far_match (byte_ptr pattern, byte_ptr s, bit_32 len)
BeginDeclarations
	int									rv;
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 While len Exceeds 0
  BeginWhile
   If *pattern Is '*'
    Then
     return(True);
    EndIf;
   If (*pattern IsNot '?') AndIf (*pattern IsNot *source)
    Then
     return(False);
    EndIf;
   source++;
   pattern++;
  EndWhile;
 return(True); */
 	rv = False;
 asm            push    esi
 asm            push    edi
 asm            mov     ecx,[len]
 asm            mov     edi,[pattern]
 asm            mov     esi,[s]
pattern_loop:
 asm            mov     al,[edi]
 asm            cmp     al,'?'
 asm            je      a_match
 asm            cmp     al,'*'
 asm            je      succeeded
 asm            cmp     al,[esi]
 asm            jne     failed
a_match:
 asm            inc     esi
 asm            inc     edi
 asm            loop    pattern_loop
succeeded:
	rv = True;
failed:
 asm            pop     edi
 asm            pop     esi
 return(rv);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          far_move                                       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
byte_ptr far_move (byte_ptr dest, byte_ptr source, bit_32 len)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 While len-- IsNotZero
  BeginWhile
   *dest++ = *source++;
  EndWhile; */

 asm            push    esi
 asm            push    edi
 asm            cld
 asm            mov     ecx,[len]
 asm            mov     edi,[dest]
 asm            mov     esi,[source]
 asm    rep     movsb
 asm            pop     edi
 asm            pop     esi

 return(dest);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          far_move_left                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
byte_ptr far_move_left (byte_ptr dest, byte_ptr source, bit_32 len)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 dest   += len - 1;
 source += len - 1;
 While len-- IsNotZero
  BeginWhile
   *dest-- = *source--;
  EndWhile; */

 asm            push    esi
 asm            push    edi
 asm            std
 asm            mov     ecx,[len]
 asm            mov     edi,[dest]
 asm            mov     esi,[source]
 asm            add     edi,ecx
 asm            add     esi,ecx
 asm            dec     esi
 asm            dec     edi
 asm    rep     movsb
 asm            pop     edi
 asm            pop     esi

 return(dest);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                far_set                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
byte_ptr far_set (byte_ptr dest, byte source, bit_32 len)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 While len-- IsNotZero
  BeginWhile
   *dest++ = source;
  EndWhile; */

 asm            push    edi
 asm            cld
 asm            mov     ecx,[len]
 asm            mov     edi,[dest]
 asm            mov     al,[source]
 asm            stosb
 asm            dec     ecx
 asm    rep     stosb
 asm            pop     edi

 return(dest);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          far_to_lower                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
byte_ptr far_to_lower (byte_ptr dest, bit_32 len)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 While len-- IsNotZero
  BeginWhile
   tolower(*dest++);
  EndWhile; */

 asm            push    edi
 asm            mov     ecx,[len]
 asm            jecxz   xit
 asm            mov     edi,[dest]
lower_case_loop:
 asm            mov     al,[edi]
 asm            cmp     al,'A'
 asm            jb      next_byte
 asm            cmp     al,'Z'
 asm            ja      next_byte
 asm            add     al,'a'-'A'
 asm            mov     [edi],al
next_byte:
 asm            inc     edi
 asm            loop    lower_case_loop
xit:
 asm            pop     edi
 return(dest);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         library_directory_hash                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void library_directory_hash(byte_ptr      sym,
                            bit_32        len,
                            bit_16       *starting_block,
                            bit_16       *delta_block,
                            bit_16       *starting_entry,
                            bit_16       *delta_entry)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
BeginDeclarations
byte                                  *beg_str;
byte                                  *end_str;
bit_32                                 c;
byte                                   temp[33];
EndDeclarations
BeginCode
 beg_str = temp;
 end_str = Addr(temp[len]);
 temp[0] = Byte(len);
 far_move(BytePtr(Addr(temp[1]), sym, len);
 *starting_block =
 *delta_block    =
 *starting_entry =
 *delta_entry    = 0;
 While len-- Exceeds 0
  BeginWhile
   c = (Bit_16(*beg_str++) And 0xFF) Or 0x20;
   *starting_block = c Xor ((*starting_block ShiftedLeft   2)  Or
                            (*starting_block ShiftedRight 14));
   *delta_entry    = c Xor ((*delta_entry    ShiftedRight  2)  Or
                            (*delta_entry    ShiftedLeft  14));
   c = (Bit_16(*end_str--) And 0xFF) Or 0x20;
   *delta_block    = c Xor ((*delta_block    ShiftedLeft   2)  Or
                            (*delta_block    ShiftedRight 14));
   *starting_entry = c Xor ((*starting_entry ShiftedRight  2)  Or
                            (*starting_entry ShiftedLeft  14));
  EndWhile; */
 asm            push    ebx
 asm            push    esi
 asm            push    edi
 asm            xor     eax,eax
 asm            mov     cl,2
 asm            xor     ebx,ebx
 asm            xor     edx,edx
 asm            mov     ebx,[len]
 asm            mov     edx,[len]
 asm            or      bl,0x20
 asm            or      dl,0x20
 asm            mov     esi,[sym]
 asm            mov     edi,[len]
 asm            dec     di
up_loop:
 asm            mov     al,[esi]
 asm            or      al,0x20
 asm            rol     bx,cl
 asm            ror     dx,cl
 asm            xor     bx,ax
 asm            xor     dx,ax
 asm            inc     esi
 asm            dec     edi
 asm            jnz     up_loop
 asm            mov     edi,[starting_block]
 asm            mov     [edi],bx
 asm            mov     edi,[delta_entry]
 asm            mov     [edi],dx
 asm            mov     edi,[len]
 asm            xor     ebx,ebx
 asm            xor     edx,edx
down_loop:
 asm            mov     al,[esi]
 asm            or      al,0x20
 asm            rol     bx,cl
 asm            ror     dx,cl
 asm            xor     bx,ax
 asm            xor     dx,ax
 asm            dec     esi
 asm            dec     edi
 asm            jnz     down_loop
 asm            mov     edi,[delta_block]
 asm            mov     [edi],bx
 asm            mov     edi,[starting_entry]
 asm            mov     [edi],dx
 asm            pop     edi
 asm            pop     esi
 asm            pop     ebx
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              word_checksum                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 word_checksum(bit_32 len, bit_32 address, byte_ptr data)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
BeginDeclarations
Structure word_struct
 BeginStructure
  bit_8                                low_byte;
  bit_8                                high_byte;
 EndStructure;
bit_16                                 sum;
Structure word_struct                  word;
bit_16                                *word_ptr;
EndDeclarations
 word = (bit_16 *) Addr(word);
 For i=0; i LessThan len; i++
  BeginFor
   *word_ptr = 0;
   If (address-- And 1)
    Then
     word.high_byte = *data++;
    Else
     word.low_byte = *data++;
    EndIf;
   sum += *word_ptr;
  EndFor;
 return(sum); */

 asm            push    ebx
 asm            push    edi
 asm            xor     eax,eax
 asm            mov     edi,[data]
 asm            mov     ebx,[address]
 asm            mov     ecx,[len]
/* Handle case where we are starting at an odd location */
 asm            and     bl,1
 asm            je      at_even_location
 asm            mov     ah,[edi]
 asm            inc     edi
 asm            dec     ecx
at_even_location:
/* Loop while we have at least two bytes left */
 asm            cmp     ecx,2
 asm            jb      at_end_of_sum
 asm            add     ax,[edi]
 asm            inc     edi
 asm            inc     edi
 asm            dec     ecx
 asm            loop    at_even_location
at_end_of_sum:
 asm            cmp     ecx,1
 asm            jne     checksum_done
 asm            add     al,[edi]
 asm            adc     ah,0
checksum_done:
 asm            pop     edi
 asm            pop     ebx
 return(_EAX);
EndCode

