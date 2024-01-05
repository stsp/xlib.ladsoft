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
/*
 * 68K/386 32-bit C compiler.
 *
 * copyright (c) 1997, David Lindauer
 * 
 * This compiler is intended for educational use.  It may not be used
 * for profit without the express written consent of the author.
 *
 * It may be freely redistributed, as long as this notice remains intact
 * and either the original sources or derived sources 
 * are distributed along with any executables derived from the originals.
 *
 * The author is not responsible for any damages that may arise from use
 * of this software, either idirect or consequential.
 *
 * v1.35 March 1997
 * David Lindauer, gclind01@starbase.spd.louisville.edu
 *
 * Credits to Mathew Brandt for original K&R C compiler
 *
 */
/*
 * Display usage information and exit.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cmdline.h"
/*
 * Main program must define the usage text
 */
extern char *usage_text;

/* Program banner */
void banner(char *fmt, ...)
{
    va_list argptr;

    putc('\n', stdout);

    va_start(argptr, fmt);
    vprintf(fmt, argptr);
    va_end(argptr);

    putc('\n', stdout);
}

/* Print usage info */
void usage(char *prog_name)
{
    char *short_name;
    char *extension;

    short_name = strrchr(prog_name, '\\');
    if (short_name == NULL)
        short_name = strrchr(prog_name, '/');
    if (short_name == NULL)
        short_name = strrchr(prog_name, ':');
    if (short_name)
        short_name++;
    else
        short_name = prog_name;

    extension = strrchr(short_name, '.');
    if (extension != NULL)
        *extension = '\0';
    printf("\nUsage: %s %s\n", short_name, usage_text);
    exit(1);
}
