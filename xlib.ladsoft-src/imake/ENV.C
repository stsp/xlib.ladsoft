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
#include <string.h>
#include "utype.h"
#include "umem.h"
#include "register.h"
#include "macros.h"
#include "maker.h"

extern char **_environ;
extern BOOL prm_ignoreenvmatch;

char *exepath = 0;
char *phipath = 0;
char *commandpath = 0;
void ReadEnvironment(void)
{
    char **p = _environ;
    while (*p)
    {
        char buf[512],  *q,  *r;
        short name[200], value[512];
        strcpy(buf,  *p++);
        q = strchr(buf, '=');
        if (!q)
            continue;
        r = q + 1;
        while (*(q - 1) == ' ')
            q--;
        *q = 0;
        ExpandFile(name, buf);
        ExpandFile(value, r);
        if (!strcmp(buf, "PATH"))
        {
            exepath = AllocateMemory(strlen(r) + 1);
            strcpy(exepath, r);
        }
        if (!strcmp(buf, "PHI"))
        {
            phipath = AllocateMemory(strlen(r) + 1);
            strcpy(phipath, r);
        }
        if (!strcmp(buf, "COMSPEC"))
        {
            commandpath = AllocateMemory(strlen(r) + 1);
            strcpy(commandpath, r);
        }
        RegisterMacro(name, value, TRUE, !prm_ignoreenvmatch);
    }
}
