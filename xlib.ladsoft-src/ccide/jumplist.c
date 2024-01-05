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
#include <commdlg.h>
#include <richedit.h>
#include <stdio.h>

#include "header.h"
#include "winconst.h"

extern int defaultWorkArea;
extern PROJECTITEM *activeProject;
extern HANDLE hInstance;
extern HWND hwndFrame, hwndClient;
extern PROJECTITEM *workArea;

#define JT_NONE 0
#define JT_FUNC 1
#define JT_VAR 2
#define JT_DEFINE 4
#define JT_TYPEDATA 8
#define JT_LOCALDATA 0x10
#define JT_STATIC 0x20
#define JT_GLOBAL 0x40

typedef struct _JUMPLIST
{
	struct _JUMPLIST *next;
	char *name;
    PROJECTITEM *project;
	int fileOffs;
	int type;
} JUMPLIST;

HWND hwndJumpList;

static JUMPLIST *jumpList, **jumpListTail;
static JUMPLIST **jumpSymbols;
static int jumpListCount;
static char szJumpListClassName[] = "xccJumpListClass";
static HWND hwndTypeCombo;
static HWND hwndValueCombo;
static WNDPROC oldProcCombo, oldProcEdit;

static LOGFONT jumpListFontData = 
{
    -14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN |
        FF_DONTCARE,
		"Arial"
};

static int cf(const void *vleft, const void *vright)
{
	const JUMPLIST *left = *(const JUMPLIST**)vleft;
	const JUMPLIST *right = *(const JUMPLIST**)vright;
	return(stricmp(left->name, right->name));
}
static void EnterJumpSymbol(PROJECTITEM *pj, unsigned char *countedName, int fileOffset, int type)
{
	if (countedName)
	{
		char buf[256];
		if (!(type & JT_GLOBAL))
			return;
		memcpy(buf, countedName + 1, countedName[0]);
		buf[countedName[0]] = 0;
		if (buf[0] != '*')
		{
			JUMPLIST *njl = calloc(sizeof(JUMPLIST), 1);
			njl->name = strdup(buf);
			njl->fileOffs = fileOffset;
			njl->type = type;
            njl->project = pj;
			*jumpListTail = njl;
			jumpListTail = &njl->next;
			jumpListCount ++ ;
		}
	}
	else
	{
		int i;
		JUMPLIST **js = calloc(sizeof(JUMPLIST *), jumpListCount);
		for (i=0; jumpList; jumpList = jumpList->next, i++)
		{
			js[i] = jumpList;
    	}
		qsort(js, jumpListCount, sizeof(JUMPLIST *), cf);
        jumpListTail = &jumpList;
		for (i=0; i< jumpListCount; i++)
		{
            *jumpListTail = js[i];
            jumpListTail = &(*jumpListTail)->next;
            js[i]->next = NULL;
		}
		jumpSymbols = js;
	}
}
#define LIB_BUCKETS 37

static void PopulateCombo(void);
FILE *LoadJumpSymbols(void)
{
	if (workArea)
	{
	    PROJECTITEM *pj = workArea->children;
		FILE *fil;
	    if (!PropGetBool(NULL, "BROWSE_INFORMATION") || defaultWorkArea || !pj || jumpSymbols)
			return 0;
		jumpListTail = &jumpList;
		jumpListCount = 0;
	    while (pj)
	    {
	        fil = LoadBrowseInfo(pj, NULL);
	        if (fil)
	        {
	            int i;
	        	for (i=0; i < pj->browseInfo->dictPages; i++)
		        {
	                unsigned char buf[512];
	        		int j;
	        		memset(buf, 0, sizeof(buf));
	        		fread(buf, 512, 1, fil);
	        		for (j=0; j < LIB_BUCKETS; j++)
	        		{
	        			if (buf[j])
	        			{
	        				unsigned char *p = buf + buf[j] * 2, *q = p;
	        				int n;
	        				p += *p + 1;
	        				n = *(int *) p;
	        				if (n & 0x0f000000)
	        				{
	        					EnterJumpSymbol(pj, q, n & 0xffffff, n >> 24);
	        				}
	        			}
	                }
	            }
                fclose(fil);
	        }
            pj = pj->next;
	    }
		EnterJumpSymbol(0,0,0,0);
		PopulateCombo();
		return 1;
	}
	return 0;
}

void FreeJumpSymbols(void)
{
	JUMPLIST *p = jumpList;
	while (p)
	{
		JUMPLIST *next = p->next;
		free(p->name);
		free(p);
		p = next;
	}
	free (jumpSymbols);
	jumpList = 0;
	jumpSymbols = 0;
	SendMessage(hwndValueCombo, CB_RESETCONTENT, 0, 0);
}
static void PopulateCombo(void)
{
	int type = SendMessage(hwndTypeCombo, CB_GETCURSEL, 0, 0);
	int i;
	int count = 0;
	if (!jumpSymbols)
		return ;
	SendMessage(hwndValueCombo, CB_RESETCONTENT, 0, 0);
	for (i=0; i < jumpListCount; i++)
	{
		if (type == 0 && (jumpSymbols[i]->type & JT_FUNC)
			|| type == 1 && (jumpSymbols[i]->type & JT_VAR)
			|| type == 2 && (jumpSymbols[i]->type & JT_TYPEDATA)
			|| type == 3 && (jumpSymbols[i]->type & JT_DEFINE))
		{
            if (i == 0 || strcmp(jumpSymbols[i]->name, jumpSymbols[i-1]->name))
            {
    			SendMessage(hwndValueCombo, CB_ADDSTRING, 0, (LPARAM)jumpSymbols[i]->name);
	    		SendMessage(hwndValueCombo, CB_SETITEMDATA, count++, (LPARAM)jumpSymbols[i]);
            }
		}
	}
}

static JUMPLIST *GetJumpList(void)
{
	int index = SendMessage(hwndValueCombo, CB_GETCURSEL, 0, 0);
    JUMPLIST *jl = (JUMPLIST *)SendMessage(hwndValueCombo, CB_GETITEMDATA, index, 0);
    if (jl)
    {
        if (jl->next && !strcmp(jl->name, jl->next->name))
        {
            JUMPLIST *cur = jl;
            HWND hwnd = (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0);
            char *p = (char *)SendMessage(hwnd, WM_FILENAME, 0, 0);
            PROJECTITEM *pj = HasFile(workArea, p);
            while (pj && pj->type != PJ_PROJ) pj = pj->parent;
            while (!strcmp(cur->name, jl->name))
            {
                if (cur->project == pj)
                    break;
                cur = cur->next;
            }
            if (cur->project == pj)
                jl = cur;
        }
    }
    return jl;
}
static int JumpTo(void)
{
    JUMPLIST *jl;
    if (!PropGetBool(NULL, "BROWSE_INFORMATION") || defaultWorkArea)
		return 0;

    jl = GetJumpList();
	if (jl)
	{
        FILE *fil = LoadBrowseInfo(jl->project, NULL);
	    int recsize;
        unsigned char buf[512];
    	unsigned char *p;
        DWINFO info;
        if (!fil)
            return 0;
	    fseek(fil, jl->fileOffs, SEEK_SET);

	    while (1)
	    {
	        if (fread(buf, 2, 1, fil) <= 0)
			{
				fclose(fil);
	            return 0;
			}
	        if (*(short*)buf == 0)
			{
				fclose(fil);
				return 0;
			}
	        if (fread(buf + 2, (*(short*)buf) - 2, 1, fil) <= 0)
			{
				fclose(fil);
				return 0;
			}
	        if (*(short *)(buf + 2) & 0x4000)
				break;
	    }
    	fclose(fil);
        InsertBrowse(jl->project->browseInfo->filenames[*(int*)(buf + 12)], *(int*)(buf + 4));
    	memset(&info, 0, sizeof(info));
    	strcpy(info.dwName, jl->project->browseInfo->filenames[*(int*)(buf + 12)]);
    	p = strrchr(info.dwName, '\\');
    	if (p)
    		strcpy(info.dwTitle, p+ 1);
    	info.dwLineNo = *(int*)(buf + 4);
        CreateDrawWindow(&info, TRUE);
        return 1;
    }
    return 0;
}
void SetJumplistPos(HWND hwnd, int linepos)
{
    char *p;
    PROJECTITEM *pj;
    int lineno;
    if (!PropGetBool(NULL, "BROWSE_INFORMATION") || defaultWorkArea || !workArea)
		return;
    p = (char *)SendMessage(hwnd, WM_FILENAME, 0, 0);
    pj = HasFile(workArea, p);
    while (pj && pj->type != PJ_PROJ) pj= pj->parent;
    if (pj)
    {
        FILE *fil = LoadBrowseInfo(pj, NULL);
        if (fil)
        {
            int n = SymbolHash(p, IDENTHASHMAX);
            struct _identHash *ih = pj->browseInfo->hash[n];
            while (ih)
            {
                if (!strcmp(ih->name, p))
                    break;
                ih = ih->next;
            }
            if (ih)
            {
                int range[2], linerange[2];
                char len;
                int pos;
                linepos = TagOldLine(p, linepos+1);
                fseek(fil, 12, SEEK_SET);
                fread(&pos, 1, 4, fil);
                fseek(fil, pos+ ih->fileno * sizeof(int), SEEK_SET);
                fread(range, 1, 8, fil);
                while (range[0] < range[1])
                {
                    char buf[512];
                    fseek(fil, range[0], SEEK_SET);
                    fread(linerange, 1, 8, fil);
                    len = fgetc(fil);
                    if (linepos >= linerange[0] && linepos <= linerange[1])
                    {
                    	int type = SendMessage(hwndTypeCombo, CB_GETCURSEL, 0, 0);
                        int index;
                        if (type != 0)
                        {
                            SendMessage(hwndTypeCombo, CB_SETCURSEL, 0, 0);
                            PopulateCombo();
                        }
                        fread(buf, 1, len, fil);
                        buf[len] = 0;
                        index = SendMessage(hwndValueCombo, CB_FINDSTRINGEXACT, -1, (LPARAM)buf);
                        SendMessage(hwndValueCombo, CB_SETCURSEL, index, 0);
                        break;
                    }
                    range[0] += 8 + len + 1;
                }
                if (range[0] >= range[1])
                {
                    SendMessage(hwndValueCombo, CB_SETCURSEL, -1, 0);
                }
            }
            else
            {
                SendMessage(hwndValueCombo, CB_SETCURSEL, -1, 0);
            }
            fclose(fil);
        }
    }
    else
    {
        SendMessage(hwndValueCombo, CB_SETCURSEL, -1, 0);
    }
}
// this gets rid of the caret and selection, and routes WM_CHAR events to the parent
// effectively, it makes the window readonly without the readonly background :)
LRESULT CALLBACK _export ValueEditProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam)
{
    int rv;
    switch(iMessage)
    {
        case WM_CHAR:
            return SendMessage(GetParent(hwnd), iMessage, wParam, lParam);
        case WM_RBUTTONDBLCLK:
            iMessage = WM_RBUTTONDOWN;
            break;
        case WM_LBUTTONDBLCLK:
            iMessage = WM_LBUTTONDOWN;
            break;
        case EM_SETSEL:
        case EM_EXSETSEL:
            return 0;
        case WM_SETFOCUS:
            rv = CallWindowProc(oldProcEdit, hwnd, iMessage, wParam, lParam);
            DestroyCaret();
            return rv;
        
                        
    }
    return CallWindowProc(oldProcEdit, hwnd, iMessage, wParam, lParam);
}
LRESULT CALLBACK _export ValueComboProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam)
{
	static char buf[256];
	static int count;
	switch (iMessage)
	{
		case WM_CREATE:
			count = 0;
			buf[0] = 0;
			break;
		case WM_CHAR:
            if (isprint(wParam))
            {
    			if (isalnum(wParam) || wParam == '_')
    			{
    				if (count < sizeof(buf)-1)
    				{
    					int pos;
                        MessageBeep(0);
    					buf[count++] = wParam;
    					buf[count] = 0;
                        SetStatusMessage(buf , 0);
    					pos = SendMessage(hwnd, CB_FINDSTRING, -1, (LPARAM)buf);
    					if (pos != CB_ERR)						
    					{
    						SendMessage(hwnd, CB_SETCURSEL, pos, 0);
    					}
    				}
    				else
    				{
    					MessageBeep(MB_ICONEXCLAMATION);
    				}
    			}
   				return 0;   
            }
			break;
		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_UP:
				case VK_DOWN:
				case VK_PRIOR:
				case VK_NEXT:
				case VK_HOME:
				case VK_END:
				case VK_ESCAPE:
				case VK_RETURN:
					count = 0;
					buf[count] = 0;
					break;
				case VK_BACK:
					if (count)
					{
						int pos;
						buf[--count] = 0;
						if (count)
						{
							pos = SendMessage(hwnd, CB_FINDSTRING, -1, (LPARAM)buf);
							if (pos != CB_ERR)
								SendMessage(hwnd, CB_SETCURSEL, pos, 0);
						}
						else
						{
							SendMessage(hwnd, CB_SETCURSEL, -1, 0);
						}
					}
					return 0;
			}
			break;
		case CB_RESETCONTENT:
			count = 0;
			buf[count] = 0;
			break;
	}
	return CallWindowProc(oldProcCombo, hwnd, iMessage, wParam, lParam);
}
LRESULT CALLBACK _export JumpListProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam)
{
    RECT r, r1,  *pr;
    POINT pt;
    HWND editWnd;
    HFONT xfont;
    switch (iMessage)
    {
        case WM_SYSCOMMAND:
            if (wParam == SC_CLOSE)
            {
                SendMessage(hwnd, WM_CLOSE, 0, 0);
            }
            break;
		case WM_COMMAND:
			switch (HIWORD(wParam))
			{
				case CBN_SELENDOK:
					if (LOWORD(wParam) == 100)
					{
						// type
						PopulateCombo();
					}
					else if (LOWORD(wParam) == 200)
					{
						JumpTo();
					}
					break;
				default:
					break;
			}
        case WM_GETHEIGHT:
            return 24;
        case WM_SETFOCUS:
            break;
        case WM_CREATE:
            hwndJumpList = hwnd;
            GetClientRect(hwnd, &r);
            hwndTypeCombo = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD + WS_CLIPSIBLINGS +
                WS_BORDER + WS_VISIBLE + CBS_DROPDOWN + CBS_AUTOHSCROLL, 
                                r.left + 70, r.top, 200, 100, hwnd, (HMENU)100, hInstance, 0);
            xfont = CreateFontIndirect(&jumpListFontData);
            SendMessage(hwndTypeCombo, WM_SETFONT, (WPARAM)xfont, 1);
            hwndValueCombo = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD + WS_CLIPSIBLINGS +
                WS_BORDER + WS_VISIBLE + CBS_DROPDOWN + CBS_AUTOHSCROLL + WS_VSCROLL, 
                                r.left + 70, r.top, 200, 200, hwnd, (HMENU)200, hInstance, 0);
            xfont = CreateFontIndirect(&jumpListFontData);
            SendMessage(hwndValueCombo, WM_SETFONT, (WPARAM)xfont, 1);
			oldProcCombo = (WNDPROC)GetWindowLong(hwndValueCombo, GWL_WNDPROC);
			SetWindowLong(hwndValueCombo, GWL_WNDPROC, (long)ValueComboProc);

            pt.x = pt.y = 5;
        	editWnd = ChildWindowFromPoint(hwndTypeCombo, pt);
            oldProcEdit = (WNDPROC)GetWindowLong(editWnd, GWL_WNDPROC);
			SetWindowLong(editWnd, GWL_WNDPROC, (long)ValueEditProc);
        	editWnd = ChildWindowFromPoint(hwndValueCombo, pt);
			SetWindowLong(editWnd, GWL_WNDPROC, (long)ValueEditProc);
            
			SendMessage(hwndTypeCombo, CB_INSERTSTRING, 0, (LPARAM)"Global Functions");
			SendMessage(hwndTypeCombo, CB_INSERTSTRING, 1, (LPARAM)"Global Variables");
			SendMessage(hwndTypeCombo, CB_INSERTSTRING, 2, (LPARAM)"Global Types");
			SendMessage(hwndTypeCombo, CB_INSERTSTRING, 3, (LPARAM)"Preprocessor Macros");
			SendMessage(hwndTypeCombo, CB_SETCURSEL, 0, 0); 
            CalculateLayout( - 1, FALSE);
			return 0;
        case WM_CLOSE:
            return 0;
        case WM_DESTROY:
			DestroyWindow(hwndValueCombo);
			DestroyWindow(hwndTypeCombo);
            break;

        case WM_SIZE:
            r.left = 0;
            r.right = LOWORD(lParam);
            r.top = 0;
            r.bottom = HIWORD(lParam);
            MoveWindow(hwndTypeCombo, r.left + 30, r.top, (r.right-r.left-100)/2, 100, TRUE);
            MoveWindow(hwndValueCombo, r.left + 30 + (r.right-r.left-100)/2 + 40, r.top, (r.right-r.left-100)/2, 100, TRUE);
            break;
        default:
            break;
    }
    return DefWindowProc(hwnd, iMessage, wParam, lParam);
}

void CreateJumpListWindow(void)
{
    RECT r;
    if (hwndJumpList)
        return ;
    hwndJumpList = CreateWindow(szJumpListClassName, "", WS_CHILD,
        CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, hwndFrame, 0, hInstance, 0);
}
void RegisterJumpListWindow(void)
{
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_HREDRAW + CS_VREDRAW;
    wc.lpfnWndProc = &JumpListProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszMenuName = 0;
    wc.lpszClassName = szJumpListClassName;
    RegisterClass(&wc);
}
