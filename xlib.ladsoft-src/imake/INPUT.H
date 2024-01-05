/* 
IMAKE make utility
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

You may contact the author at:
	mailto::camille@bluegrass.net
 */
#define COMMANDLINES 500
#define PERIOD 0x2e
#define EXCLAMATION 0x21
#define STAR 0x2a
#define AMPERSAND 0x26
#define AT 0x40
#define QUESTION 0x3f
#define QUOTE 0x22
#define LESS 0x3c
#define GREATER 0x3e
#define COLON 0x3a
#define COLON2 ((1<<7)+0x61)
#define EQUAL 0x3d
#define ALTEQUAL ((4 << 7) + 33)
#define EXTENDER1 0x5c
#define EXTENDER2 ((4<<7) +0x28)
#define MACROSTART 0x24
#define MACROLEFT1 0x28
#define MACRORIGHT1 0x29
#define MACROLEFT2 ((4 <<7) + 0x36)
#define MACRORIGHT2 ((4 <<7) + 0x37)
#define QMARK 0x3f

#define INTERNALBUFSIZE 32000
#define INCLUDELEVELS 10
#define BUFLEN 256

#include "input.p"
