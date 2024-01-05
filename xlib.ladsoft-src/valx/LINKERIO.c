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
/*                                 LINKERIO.C                              */



/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           linker_error                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void linker_error(int_16 severity, char_ptr format, ...)
BeginDeclarations
va_list                                argptr;
EndDeclarations
BeginCode
 If severity NotLessThan 0
  Then
   fprintf(stdout,"\nLinker Error (Severity %d)\n",severity);
  Else
   bad_link = 1 ;
  EndIf;
 va_start(argptr,format);
 vfprintf(stdout,format,argptr);
 If severity Exceeds 7
  Then
   end_linker(severity);
  Else
   return;
  EndIf;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            linker_message                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void linker_message(char_ptr format, ...)
BeginDeclarations
va_list                                argptr;
EndDeclarations
BeginCode
 va_start(argptr,format);
 vfprintf(stdout,format,argptr);
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 print                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void print(char_ptr format, ...)
BeginDeclarations
va_list                                argptr;
bit_16                                 len;
EndDeclarations
BeginCode
 va_start(argptr,format);
 vsprintf(CharPtr(object_file_element), format, argptr);
 len = strlen(CharPtr(object_file_element));
 If len IsZero
  Then
   return;
  EndIf;
 If object_file_element[len-1] Is '\n'
  Then
   object_file_element[len-1] = '\000';
   strcat(CharPtr(object_file_element), "\r\n");
   len++;
  EndIf;
 file_write(BytePtr(object_file_element), Bit_32(len));
 return;
EndCode
