/* 
XLIB Librarian
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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cmdline.h"

extern uint prm_errcount;

/*
 * Issue an error, quit if too many encountered
 */
void Error(char *fmt, ...)
{
    va_list argptr;

    va_start(argptr, fmt);
    printf("Error: ");
    vprintf(fmt, argptr);
    va_end(argptr);
    putc('\n', stdout);
    if (--prm_errcount == 0)
    {
        printf("fatal: Too many errors or warnings encountered");
        exit(2);
    }
}
