/* 
CCIDE
Copyright 2001-2011 David Lindauer.

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
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <float.h>
#include "helpid.h"
#include "header.h"
extern int _argc;
extern char **_argv;
extern HWND hwndFrame;

typedef struct _list {
	struct _list *next;
	char *data;
} LIST;
LIST *nameList;
static LPSTR lpszSlotName = "\\\\.\\mailslot\\ccide_mailslot"; 
static HANDLE ghMailSlot;
static CRITICAL_SECTION critical;
static BOOL started;
int SendFileName(char *msg)
{
	DWORD cbWritten;
	HANDLE hFile = CreateFile(lpszSlotName,
	    GENERIC_WRITE, 
	    FILE_SHARE_READ, /* required to write to a mailslot */ 
	    (LPSECURITY_ATTRIBUTES) NULL, 
	    OPEN_EXISTING, 
	    FILE_ATTRIBUTE_NORMAL, 
	    (HANDLE) NULL); 
	 
	if (hFile == INVALID_HANDLE_VALUE) { 
		return FALSE;
	} 
	 
	WriteFile(hFile, 
	    msg, 
	    (DWORD) strlen(msg) + 1,
	    &cbWritten, 
	    (LPOVERLAPPED) NULL); 
	 
	CloseHandle(hFile); 
	return TRUE; 
} 
void PassFilesToInstance(void)
{
	int argc = _argc;
	char **argv = _argv;
    AllowSetForegroundWindow(ASFW_ANY);
//	argv = CmdLineToC(&argc, GetCommandLineA());
	if (argv)
	{
		if (argc <= 1)
		{
			SendFileName("###TOP###");
		}
		else
		{
			int i;
			for (i=1; i < argc; i++)
			{
				char buf[260], *p = buf;
				strcpy(buf, argv[i]);
				while (isspace(*p)) p++;
					if (*p != '/' && *p != '-')
					{
						abspath(p, NULL);
						SendFileName(p);
					}
			}
		}
	}
}
int RetrieveInstanceFile(DWINFO *info)
{
	int rv = 0;
	if (nameList)
	{
		LIST *l ;
		char *p;
		EnterCriticalSection(&critical);
		l = nameList;
		nameList = nameList->next;
		LeaveCriticalSection(&critical);
		if (started)
		{
			strncpy(info->dwName, l->data, 260);
			info->dwName[259] = 0;
			p = strrchr(info->dwName, '\\');
			if (!p)
				p = info->dwName;
			else
				p++;
			strcpy(info->dwTitle, p);		
			info->dwLineNo = -1;
	        info->logMRU = FALSE;
	        info->newFile = FALSE;
			rv = 1;
		}
		free(l->data);
		free(l);		
	}
	return rv;
}
int PASCAL msThread(void *aa)
{
	HANDLE hMailSlot = (HANDLE)aa;
	InitializeCriticalSection(&critical);
	started = TRUE;
	while (TRUE)
	{
		char buffer[260], *name;
		DWORD cbRead;
		DWORD fResult = ReadFile(hMailSlot, 
		    buffer, 
		    sizeof(buffer), 
		    &cbRead, 
		    (LPOVERLAPPED) NULL); 

	    if (!fResult) { 
			started = FALSE;
			DeleteCriticalSection(&critical);			
    	    return FALSE; 
    	} 
        SetForegroundWindow(hwndFrame);
		if (strcmp(buffer,"###TOP###"))
		{
			name = strdup(buffer);
			if (name)
			{
				LIST *l = calloc(sizeof(LIST), 1);
				if (l)
				{
					l->data = name;
					EnterCriticalSection(&critical);
					l->next = nameList;
					nameList = l;	
					LeaveCriticalSection(&critical);
				}
				else
					free(name);
			}
		}
	}
    return TRUE; 
}
int StartInstanceComms(void)
{
	DWORD id;
    HANDLE hMailSlot = CreateMailslot(lpszSlotName, 
        0,                         /* no maximum message size         */ 
        MAILSLOT_WAIT_FOREVER,     /* no time-out for read operations */ 
        (LPSECURITY_ATTRIBUTES) NULL); /* no security attributes      */ 
 
    if (hMailSlot == INVALID_HANDLE_VALUE) { 
        return FALSE; 
    } 
	ghMailSlot = hMailSlot;
 	CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)msThread, 
							 (LPVOID)hMailSlot, 0, &id));
	
    return TRUE; 
}
void StopInstanceComms(void)
{
	CloseHandle(ghMailSlot);
}