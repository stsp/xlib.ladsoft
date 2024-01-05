/* 
GREP
Copyright 2007-2011 David Lindauer.

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

This program is derived from the cc68k complier by 
Matthew Brandt (mailto::mattb@walkingdog.net) 

You may contact the author of this derivative at:

mailto::camille@bluegrass.net
 */
#include <stdio.h>
#include <stdlib.h>
#include <dir.h>
#include <string.h>
#include "cmdline.h"

#define MAX_PATH 260
 
extern int prm_recurseDirs;

int fileCount;
char **files;

static int maxFiles;

static void oneFile(path, fileName)
char *path;
char *fileName;
{
	char localName[MAX_PATH];
	struct ffblk blk;
	sprintf(localName, "%s%s", path, fileName);
	if (!findfirst(localName, &blk, 0))
	{
		do
		{
			if (!(blk.ff_attrib & _A_SUBDIR) && !(blk.ff_attrib & _A_VOLID) &&
				!(blk.ff_attrib & _A_HIDDEN))
			{
				char buf[MAX_PATH];
				if (fileCount >= maxFiles)
				{
					maxFiles += 64;
					files = realloc(files, sizeof(char *) * maxFiles);
					if (!files)
						fatal("out of memory");
				}
				sprintf(buf, "%s%s", path, blk.ff_name);
				files[fileCount++] = strdup(buf);
			}
		}
		while (!findnext(&blk));
	}
}
void RecurseDirs(path, fileName)
char *path;
char *fileName;
{
	struct ffblk blk;
	char localPath[256];
	sprintf(localPath, "%s*.*", path);
	if (!findfirst(localPath, &blk, _A_SUBDIR))
	{
		do {
			if (blk.ff_name[0] != '.')
			{
				int l = strlen(path);
				sprintf(path + l, "%s\\", blk.ff_name);
				oneFile(path, fileName);
				RecurseDirs(path, fileName);
				path[l] = 0;
			}
		} while (!findnext(&blk));
	}
	else
	{
		oneFile(path, fileName);
	}
}
void GatherFiles(ppFile)
char **ppFile;
{
	while (*ppFile)
	{
		char path[MAX_PATH];
		char file[MAX_PATH];
		char *p;
		
		p = strrchr(*ppFile, '\\');
		if (p)
		{
			strcpy(path, *ppFile);
			*(strrchr(path, '\\') + 1) = 0;
			strcpy(file, p + 1);
		}
		else
		{
			if ((*ppFile)[0] == ':')
			{
				path[0] = (*ppFile)[0];
				path[1] = (*ppFile)[1];
				path[2] = 0;
				strcpy(file, *ppFile + 2);
			}
			else
			{
				path[0] = 0;
				strcpy(file, *ppFile);
			}
		}
		oneFile(path, file);
		if (prm_recurseDirs)
		{
			RecurseDirs(path, file);
		}
		ppFile++;
	}
}
