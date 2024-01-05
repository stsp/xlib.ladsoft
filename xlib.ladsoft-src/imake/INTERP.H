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
#ifndef _INTERP_H_
    #define _INTERP_H_

    #define SYM_VAC		(0x7f)
    #define SYM_EQ		((4<<7)+33)
    #define SYM_NE		((4<<7)+37)
    #define SYM_ALTSTAR     ((4<<7)+38)
    #define SYM_ALTDIV	((4<<7)+42)
    #define SYM_LE		((4<<7)+50)
    #define SYM_GE		((4<<7)+51)
    #define SYM_SHL		((4<<7)+122)
    #define SYM_SHR		((4<<7)+123)
    #define SYM_LB 		((1<<7)+0x49)
    #define SYM_CENT	((1<<7)+0x4a)
    #define SYM_YEN		((1<<7)+0x4b)

    #define BOLD 16
    #define FLASH 8
    #define HIDDEN 4
    #define UNDERLINE 2
    #define REVERSE 1

    #define DKBLACK 0
    #define DKRED 1
    #define DKORANGE 2
    #define DKYELLOW 3
    #define DKGREEN 4
    #define DKBLUE 5
    #define DKPURPLE 6
    #define DKWHITE 7
    #define LTBLACK 8
    #define LTRED 9
    #define LTORANGE 10
    #define LTYELLOW 11
    #define LTGREEN 12
    #define LTBLUE 13
    #define LTPURPLE 14
    #define LTWHITE 15

    typedef struct
    {
        int size;
        int style;
        int bgc;
        int fgc;
        int attrib;
        int bank;
        int cwb;
    } PHITEXT;

    #include "interp.p"

#endif /* _INTERP_H_ */
