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
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "htmlhelp.h"
#include "header.h"
extern char szInstallPath[];
extern HWND hwndFrame;

char szHelpPath[1024]; // so editing will work...
HMODULE htmlLib;
static FARPROC HelpFunc;

//-------------------------------------------------------------------------

int InitHelp(void)
{
    strcpy(szHelpPath, (char*)ProfileToString("HelpPath", ""));
    if (htmlLib)
        return TRUE;
    htmlLib = LoadLibrary("hhctrl.ocx");
    if (htmlLib)
    {
        HelpFunc = GetProcAddress(htmlLib, ATOM_HTMLHELP_API_ANSI);
	    HelpFunc(GetDesktopWindow(), 0, HH_INITIALIZE, 0);
        return TRUE;
    }

    return FALSE;
}

//-------------------------------------------------------------------------

void RundownHelp(void)
{
    if (htmlLib)
    {
	    HelpFunc(GetDesktopWindow(), 0, HH_UNINITIALIZE, 0);
        FreeLibrary(htmlLib);
        htmlLib = 0;
    }
    StringToProfile("HelpPath", szHelpPath);
}

//-------------------------------------------------------------------------

int SpecifiedHelp(char *string)
{
    HH_AKLINK hl;
    char buf[256];
    if (!szHelpPath[0] || !InitHelp())
        return FALSE;
		if (!string)
			string = "";
    hl.cbStruct = sizeof(HH_AKLINK);
    hl.fReserved = FALSE;
    hl.pszKeywords = string;
    hl.pszUrl = NULL;
    hl.pszMsgText = NULL;
    hl.pszMsgTitle = NULL;
    hl.pszWindow = NULL;
    hl.fIndexOnFail = TRUE;
    HelpFunc(GetDesktopWindow(), buf, HH_DISPLAY_TOPIC, 0);
    return HelpFunc(GetDesktopWindow(), buf, HH_KEYWORD_LOOKUP, &hl);
//   	return WinHelp(hwndFrame, szHelpPath, HELP_KEY, (DWORD)string);
}
int RTLHelp(char *string)
{
    HH_AKLINK hl;
    char buf[256];
    strcpy(buf, szInstallPath);
    strcat(buf, "\\help\\crtl.chm");
    if (!InitHelp())
        return FALSE;
		if (!string)
			string = "";
    hl.cbStruct = sizeof(HH_AKLINK);
    hl.fReserved = FALSE;
    hl.pszKeywords = string;
    hl.pszUrl = NULL;
    hl.pszMsgText = NULL;
    hl.pszMsgTitle = NULL;
    hl.pszWindow = NULL;
    hl.fIndexOnFail = TRUE;
    HelpFunc(GetDesktopWindow(), buf, HH_DISPLAY_TOPIC, 0);
    HelpFunc(GetDesktopWindow(), buf, HH_KEYWORD_LOOKUP, &hl);
//    WinHelp(hwndFrame, buf, HELP_KEY, (DWORD)string);
		
	return TRUE;
}
int LanguageHelp(char *string)
{
    HH_AKLINK hl;
    char buf[256];
    strcpy(buf, szInstallPath);
    strcat(buf, "\\help\\chelp.chm");
    if (!InitHelp())
        return FALSE;
		if (!string)
			string = "";
    hl.cbStruct = sizeof(HH_AKLINK);
    hl.fReserved = FALSE;
    hl.pszKeywords = string;
    hl.pszUrl = NULL;
    hl.pszMsgText = NULL;
    hl.pszMsgTitle = NULL;
    hl.pszWindow = NULL;
    hl.fIndexOnFail = TRUE;
    HelpFunc(GetDesktopWindow(), buf, HH_DISPLAY_TOPIC, 0);
    HelpFunc(GetDesktopWindow(), buf, HH_KEYWORD_LOOKUP, &hl);
//    WinHelp(hwndFrame, buf, HELP_KEY, (DWORD)string);
		
	return TRUE;
}
//-------------------------------------------------------------------------

void ContextHelp(int id)
{
    char buf[256];
    strcpy(buf, szInstallPath);
    strcat(buf, "\\help\\ccide.chm");
    if (!InitHelp())
        return ;
    HelpFunc(GetDesktopWindow(), buf, HH_DISPLAY_TOPIC, 0);
    HelpFunc(GetDesktopWindow(), buf, HH_HELP_CONTEXT, id);
//    WinHelp(0, buf, HELP_CONTEXT, id);
}
void GenericHelp(char *str, int id)
{
    if (!InitHelp())
        return ;
	if (id == -1)
	    HelpFunc(GetDesktopWindow(), str, HH_DISPLAY_TOPIC, 0);
	else
	{
	    HelpFunc(GetDesktopWindow(), str, HH_DISPLAY_TOPIC, 0);
	    HelpFunc(GetDesktopWindow(), str, HH_HELP_CONTEXT, id);
	}
}