/* 
MKDLLLST
Copyright 1994-2011 David Lindauer

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  

You may contact the author at:

mailto::camille@bluegrass.net

or by snail mail at:

David Lindauer
850 Washburn Ave Apt 99
Louisville, KY 40222
 */
/*
 * mkliblst.c
 *
 * Add library module names to a file
 *
 * presently supports Borland and LADsoft library formats
 *
 * David Lindauer, gclind01@starbase.spd.louisville.edu
 *
 * August, 1996
 */

#include <stdio.h>

int main(int argc, char *argv[])
{
    int i, addret = 0, obj = 0;
    FILE *fil;
    if (argc < 3)
    {
        printf("usage: mkdlllst lst obj list");
        exit(1);
    }
    if (!strcmp(argv[2], ".obj"))
    {
        obj = 1;
    }
    fil = fopen(argv[1], "r");
    if (fil)
    {
        addret = 1;
        fclose(fil);
    }
    fil = fopen(argv[1], "a+");
    if (!fil)
    {
        printf("Can't open output file");
        exit(1);
    }
    for (i = 3; i < argc; i++)
    {
        if (addret)
        {
            if (obj)
                fputc('+', fil);
            fputc('\n', fil);
        }
#ifdef XXXXX
        if (obj)
        {
            fputc('+', fil);
        }
#endif
        addret = 1;
        fprintf(fil, "..\\object\\%s%s ", argv[i], argv[2]);
    }
    fclose(fil);
    return 0;
}
