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
/*                                 MEMORY.C                                */

/* I rewrote this entirely */


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            initialize_memory                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

void initialize_memory (void)
BeginDeclarations
byte_ptr rv;
EndDeclarations
BeginCode
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            allocate_memory                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

byte_ptr allocate_memory (bit_32 size)
BeginDeclarations
byte_ptr rv;
EndDeclarations
BeginCode
 If size IsZero
  Then
   return NULL;
  EndIf;
 rv = calloc(size,1);
 If rv IsNull
  Then
   linker_error(12,"Out of memory\n");
  EndIf;
 return rv;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            reallocate_memory                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

byte_ptr reallocate_memory (ptr mem, bit_32 size)
BeginDeclarations
byte_ptr rv;
EndDeclarations
BeginCode
 If size IsZero
  Then
   free(mem) ;
   return NULL;
  EndIf;
 If mem IsNull 
  Then
   rv = calloc(size,1) ;
  Else
   rv = realloc(mem, size);
  EndIf
 If rv IsNull
  Then
   linker_error(12,"Out of memory\n");
  EndIf;
 return rv;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              release_memory                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

void release_memory(byte_ptr mem)
BeginDeclarations
EndDeclarations
BeginCode
 free(mem);
EndCode

