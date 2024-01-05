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
/*                                 TIMER.C                                 */
/* standard C now, but we lost the hundredths of a second DAL */


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            get_time                                     |
  |                                                                         |
  |                          O/S dependent                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 get_time(void)
BeginDeclarations
bit_32                                 hhmmsscc;
time_t                                 curtime;
struct tm                             *tstruct;
EndDeclarations
BeginCode
 time(&curtime);
 tstruct = localtime(&curtime);
 hhmmsscc = tstruct->tm_sec *     100   + 
            tstruct->tm_min *    6000   +
            tstruct->tm_hour * 360000;
 return(hhmmsscc);
EndCode


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         elapsed_time                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
char_ptr elapsed_time(bit_32 start_time, bit_32 stop_time)
BeginDeclarations
 bit_16                                hh;
 bit_16                                mm;
 bit_16                                ss;
 bit_16                                cc;
 bit_32                                t;
EndDeclarations
BeginCode
 If start_time Exceeds stop_time
  Then /* We passed midnight and must add 24 hours to stop time */
   stop_time += 8640000L;
  EndIf;
 t   = stop_time - start_time;
 cc  = Bit_16(t Mod 100L);                     t /= 100L;
 ss  = Bit_16(t Mod 60L);                      t /= 60L;
 mm  = Bit_16(t Mod 60L);                      t /= 60L;
 hh  = Bit_16(t);
 If hh IsNotZero
  Then
   sprintf(time_array,"%u:%02u:%02u.%02u",hh,mm,ss,cc);
  ElseIf mm IsNotZero
   Then
    sprintf(time_array,"%u:%02u.%02u",mm,ss,cc);
   Else
    sprintf(time_array,"%u.%02u",ss,cc);
 EndIf;
 return(time_array);
EndCode

