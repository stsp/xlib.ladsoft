/* 
Browse Linker
Copyright 2003-2011 David Lindauer.

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

static char **filenames;
static int filecount;
static int LoadBrowseInfo(FILE *fil)
{
    char buf[12];
    unsigned short count, i;
    fread(buf, 12, 1, fil);
    if (strncmp(buf, "$BRW", 4))
        return 0;
    fseek(fil, 32, SEEK_SET);
    fread(&count, 2, 1, fil);
    filecount = count;
    filenames = calloc(count, sizeof(char*));
    if (!filenames)
        return 0;
    for (i = 0; i < count; i++)
    {
        char name[256];
        int len = fgetc(fil);
        fread(name, len, 1, fil);
        name[len] = 0;
        filenames[i] = strdup(name);
        if (!filenames[i])
        {
            for (--i; i >= 0; --i)
            {
                free(filenames[i]);
            }
            free(filenames);
            return 0;
        }
    }

    return *(int*)(buf + 4);
}

//-------------------------------------------------------------------------

static int FreeBrowseInfo(void)
{
    int i;
    for (i = 0; i < filecount; i++)
        free(filenames[i]);
    free(filenames);
    filenames = 0;
    filecount = 0;
}

//-------------------------------------------------------------------------

int FindBrowseInfo(FILE *fil, char *name, int root)
{
    while (1)
    {
        char buf[128 *9],  *p = buf;
        int count;
        fseek(fil, root, SEEK_SET);
        fread(buf, 128 *9, 1, fil);
        count =  *p++;
        while (count)
        {
            if (*p ==  *name)
            {
                name++;
                if (*name == 0)
                    return *(int*)(p + 5);
                root = *(int*)(p + 1);
                break;
            }
            count--;
            p += 9;
        }
        if (!count)
            return 0;
    }
}

//-------------------------------------------------------------------------

void main(int argc, char **argv)
{
    FILE *fil = fopen(argv[1], "rb");
    int ofs;
    if (fil)
    {
        if (ofs = LoadBrowseInfo(fil))
        {
            printf("info: %x\n", FindBrowseInfo(fil, argv[2], ofs));
        }
        else
            printf("not a browse file or out of memory");
    }
    else
        printf("can't open file");
}
