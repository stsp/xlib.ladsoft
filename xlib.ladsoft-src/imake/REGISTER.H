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
#define R_PATH 		 1
#define R_DEST     2
#define R_EXPLICIT 3
#define R_IMPLICIT 4
#define R_MACRO    5

typedef struct registry
{
    struct registry *link;
    short *name;
    int canchange: 1;
    int isdef: 1;
    int isenv: 1;
    int noauto: 1;
    int type;

    short *depends;
    struct 
    {
        short *path;
        short *macro;
        short **commands;
    } x;
} REGISTRY;

#include "register.p"
