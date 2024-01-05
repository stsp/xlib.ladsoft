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
#include <commdlg.h>
#include <richedit.h>
#include <stdio.h>
#include "header.h"

extern HANDLE hInstance;
extern HWND hwndWatch, hwndClient, hwndFrame, hwndTab;

static HWND hwndInfo;
static int rescroll;
static char szErrorClassName[] = "xccErrorClass";
static WNDPROC oldproc;
static HWND hwndCtrl, hwndTabCtrl;
static int infoIndex;
static int wheelIncrement;
static LOGFONT tabFontData = 
{
    -13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN |
        FF_DONTCARE,
        "Arial"
};
static LOGFONT Textfontdata =
{
    -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH |
        FF_DONTCARE,
		CONTROL_FONT
};

static char *nameTags[] = 
{
    "Build", "Debug", "Find in Files 1", "Find in Files 2"
};
static char *szErrorTitle = "Information Window";
int getfile(char *start, char *buffer, char end, DWINFO *info)
{
    char *t = buffer, *q;
    int rv;
    rv = atoi(++buffer);
    while (*buffer && isdigit(*buffer))
        buffer++;
    if (*buffer == end)
    {
		/* complexity is to allow white space in file and path names */
		q = strchr(start, ':');
		if (q && q != start)
		{
			char *r = strchr(start, '.');
			if (r && r < q)
			{
				q = r;
				while (q != start && isalnum(q[-1]))
					q--;
			}
			else
				q--;
		}
		else
		{
			q = strchr(start, '\\');
			if (!q)
			{
				q = t;					
		        while (q > start)
		        {
		            if (*(q - 1) == ' ')
		                break;
		            q--;
		        }
			}
			else
			{
				char *r = strchr(start, '.');
				if (r && r < q)
				{
					q = r;
					while (q != start && isalnum(q[-1]))
						q--;
				}
			}
		}
        strncpy(info->dwName, q, t - q);
        info->dwName[t - q] = 0;
        q = strrchr(info->dwName, '\\');
        if (q)
            strcpy(info->dwTitle, q + 1);
        else
            strcpy(info->dwTitle, info->dwName);
        return rv;
    }
    return  - 1;
}
void TextToClipBoard(HWND hwnd, char *text)
{
    if (OpenClipboard(hwnd))
    {
		int n = 0;
		char *p = text;
		int l = strlen(text);
		HGLOBAL glmem;
		while (p = strchr(p,'\n'))
			n++,p++;
        glmem = GlobalAlloc(GMEM_DDESHARE + GMEM_MOVEABLE, l+n+1);
        if (glmem != NULL)
        {
            char *data = GlobalLock(glmem),  *q = data;
            for (p = text; *p; p++)
            {
                if (*p == '\n')
                    *q++ = '\r';
                *q++ = *p;
            }
            *q++ = 0;
            GlobalUnlock(data);
            EmptyClipboard();
            SetClipboardData(CF_TEXT, glmem);
        }
        CloseClipboard();
    }
}
//-------------------------------------------------------------------------

void BumpToEditor(HWND hwnd)
{
    HWND wnd;
    DWINFO info;
    char name[256], title[256], buffer[256],  *t, *q;
    int lineno;
    int start;
    SendMessage(hwnd, EM_GETSEL, (WPARAM) &start, 0);
    lineno = SendMessage(hwnd, EM_EXLINEFROMCHAR, 0, start);
    *(short*)buffer = 255;
    lineno = SendMessage(hwnd, EM_GETLINE, lineno, (LPARAM)buffer);
    buffer[lineno] = 0;
    lineno =  - 1;
	if (infoIndex >= 2) /* find in file dialog */
	{
		t = info.dwName;
		q = buffer;
		while (*q && !isspace(*q))
		{
			*t++ = *q++;
		}
		*t = 0;
		while (isspace(*q))
			q++;
		lineno = atoi(q);
        q = strrchr(info.dwName, '\\');
        if (q)
            strcpy(info.dwTitle, q + 1);
        else
            strcpy(info.dwTitle, info.dwName);
	}
    else if ((t = strchr(buffer, '(')) && isdigit(*(t + 1)))
    {
        lineno = getfile(buffer, t, ')', &info);
    }
    else
    {
        t = buffer;
        while (1)
        {
            if (t = strchr(t + 1, ':'))
            {
                if (isdigit(*(t + 1)))
                {
                    lineno = getfile(buffer, t, ':', &info);
                    if (lineno !=  - 1)
                        break;
                }
            }
            else
                break;
        }
    }
    if (lineno !=  - 1)
    {
        info.dwLineNo = lineno;
        info.logMRU = FALSE;
		info.newFile = FALSE;
        CreateDrawWindow(&info, TRUE);
    }

}


//-------------------------------------------------------------------------

static void CopyText(HWND hwnd)
{
	char *p;
	int l;
	l = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
	p = malloc(l+1);
	if (p)
	{
		SendMessage(hwnd, WM_GETTEXT, l+1, (LPARAM)p);
		TextToClipBoard(hwnd, p);
		free(p);
	}
}
LRESULT CALLBACK buildEditProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam)

{
    char buffer[256];
    int pos, chars, line, i;
    LRESULT rv;
    POINTL point;
    HBRUSH bk;
    RECT r;
    CHARRANGE s;
    NMHDR *h;
    switch (iMessage)
    {
        case WM_CREATE:
            rv = CallWindowProc(oldproc, hwnd, iMessage, wParam, lParam);
            //         SendMessage(hwnd,EM_NOCOLORIZE,0,0) ;
            return rv;
        case WM_LBUTTONDOWN:
			SetFocus(hwnd);
            *(short*)buffer = 256;
            point.x = LOWORD(lParam);
            point.y = HIWORD(lParam);
            pos = SendMessage(hwnd, EM_CHARFROMPOS, 0, (LPARAM) &point);
            line = SendMessage(hwnd, EM_EXLINEFROMCHAR, 0, pos);
            i = SendMessage(hwnd, EM_GETLINE, line, (LPARAM) &buffer);
            chars = 0; // richedit 1.0 was tacking things on the end...
            while (i-- && buffer[chars] > 31)
                chars++;
            pos = SendMessage(hwnd, EM_LINEINDEX, line, 0);
            s.cpMin = pos;
            s.cpMax = pos + chars;
            SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) &s);
			SetFocus(hwnd);
            return 0;
        case WM_RBUTTONDOWN:
            {
				SetFocus(hwnd);
				if (infoIndex == 0)
				{
                    HMENU menu = LoadMenuGeneric(hInstance, "BUILDMENU");
                    HMENU popup = GetSubMenu(menu, 0);
                    POINT pos;
	                GetCursorPos(&pos);
					InsertBitmapsInMenu(popup);
	                TrackPopupMenuEx(popup, TPM_BOTTOMALIGN | TPM_LEFTBUTTON, pos.x,
	                    pos.y, hwndFrame, NULL);
                    DestroyMenu(menu);
				}
            }
            break;
        case WM_LBUTTONDBLCLK:
			BumpToEditor(hwnd);
			return 0;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
            return 0;
        
		case WM_KEYDOWN:
			switch(wParam)
			{
				case 'C':
                    if (GetKeyState(VK_CONTROL) &0x80000000)
					{
						CopyText(hwnd);
					}
					break;
				case VK_CONTROL:
					break;
				case VK_UP:
					SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);
					break;
				case VK_DOWN:
					SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
					break;
				case VK_PRIOR:
					SendMessage(hwnd, WM_VSCROLL, SB_PAGEUP, 0);
					break;
				case VK_NEXT:
					SendMessage(hwnd, WM_VSCROLL, SB_PAGEDOWN, 0);
					break;
				case VK_HOME:
					SendMessage(hwnd, WM_VSCROLL, SB_TOP, 0);
					break;
				case VK_END:
					SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0);
					break;
				default:
					MessageBeep(MB_OK);
					break;
			}
			return 0;
		case WM_MOUSEWHEEL:
            i = PropGetInt(NULL, "MOUSEWHEEL_SCROLL");
            if (i <=0 )
                i = 1;
			wheelIncrement -= GET_WHEEL_DELTA_WPARAM(wParam);
			if (wheelIncrement < 0)
			{
				while (wheelIncrement <= - WHEEL_DELTA)
				{
					SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);
					wheelIncrement += WHEEL_DELTA/i;
				}
			}
			else
			{
				while (wheelIncrement >= WHEEL_DELTA)
				{
					SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
					wheelIncrement -= WHEEL_DELTA/i;
				}
			}
			return 0;
		case WM_VSCROLL:
            if (wParam != SB_BOTTOM)
            {
    			rescroll= TRUE;
            }
			break;
    }
    return CallWindowProc(oldproc, hwnd, iMessage, wParam, lParam);
}

//-------------------------------------------------------------------------

LRESULT CALLBACK errorProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam)
{
    static HWND hsubwnds[4]; // build, debug, find1, find2
    static HFONT tabNormalFont, textFont;
	CHARFORMAT cfm;
    TC_ITEM tie;
    NMHDR *h;
    DRAWITEMSTRUCT *dr;
	MEASUREITEMSTRUCT *mr;
    HFONT font;
    RECT r, r1,  *pr;
    CHARRANGE s;
	HDC dc ;
	PAINTSTRUCT ps;
	HPEN pen;
	int clr;
	int i;
	static int colors[4];
	static int forecolor;
    switch (iMessage)
    {
        case WM_SYSCOMMAND:
            if (wParam == SC_CLOSE)
            {
                SendMessage(hwnd, WM_CLOSE, 0, 0);
            }
            break;
		case WM_LBUTTONDOWN:
            return SendMessage(hsubwnds[infoIndex], iMessage, wParam, lParam);
        case WM_NOTIFY:
            h = (NMHDR*)lParam;
            switch (h->code)
            {
				char *text;
	            case TABN_SELECTED:
				{
					LSTABNOTIFY *p = (LSTABNOTIFY *)h;
	                ShowWindow(hsubwnds[infoIndex], SW_HIDE);
					for (i=0; i < 4; i++)
						if (p->lParam == (LPARAM)hsubwnds[i])
							infoIndex = i;
	                ShowWindow(hsubwnds[infoIndex], SW_SHOW);
	                break;
				}
            }
            break;
        case WM_COMMAND:
            break;
		case WM_VSCROLL:
				break;
		case WM_SETCOLOR:
			colors[wParam] = lParam;
			break;
        case WM_SETTEXT2:
            if (!lParam)
			{
                SendMessage(hsubwnds[wParam], WM_SETTEXT, 0, (LPARAM)"");
				rescroll = FALSE;
			}
            else
            {
                int linfoIndex = SendMessage(hsubwnds[wParam], EM_LINEINDEX,
                    0x7fffffff, 0);
				int thumb = SendMessage(hsubwnds[wParam], EM_GETTHUMB, 0, 0);
                s.cpMin = s.cpMax = linfoIndex;
                SendMessage(hsubwnds[wParam], EM_EXSETSEL, 0, (LPARAM) &s);
				memset(&cfm, 0, sizeof(cfm));
				cfm.cbSize = sizeof(cfm);
				cfm.dwMask = CFM_COLOR;
				cfm.crTextColor = colors[wParam];
				SendMessage(hsubwnds[wParam], EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) & cfm);
                SendMessage(hsubwnds[wParam], EM_REPLACESEL, FALSE, lParam);
                SendMessage(hsubwnds[wParam], EM_SETSEL, -1, 0);
				if (rescroll)
				{
					SendMessage(hsubwnds[wParam], WM_VSCROLL, MAKELONG(SB_THUMBPOSITION, thumb), 0);
				}
				else
				{
					SendMessage(hsubwnds[wParam], WM_VSCROLL, SB_BOTTOM, 0);
				}

            }
			colors[wParam] = RetrieveSysColor(COLOR_WINDOWTEXT);
            return 0;
        case WM_SETFOCUS:
            PostMessage(hwndFrame, WM_REDRAWTOOLBAR, 0, 0);
			SendMessage(GetParent(hwnd), WM_ACTIVATEME, 0, 0);
            SetFocus(hsubwnds[infoIndex]);
			return 0;
        case WM_CREATE:
            hwndInfo = hwnd;
            GetClientRect(hwnd, &r);
            tabNormalFont = CreateFontIndirect(&tabFontData);
			textFont = CreateFontIndirect(&Textfontdata);
            hwndTabCtrl = CreateLsTabWindow(hwnd, TABS_BOTTOM | TABS_HOTTRACK | TABS_FLAT | WS_VISIBLE);
            SendMessage(hwndTabCtrl, WM_SETFONT, (WPARAM)tabNormalFont, 0);
            OffsetRect(&r,  - r.left,  - r.top);
			r.bottom -= 25;
            hsubwnds[0] = CreateWindowEx(0, "XBUILDEDIT", 0, WS_CHILD +
                WS_VISIBLE + WS_VSCROLL + ES_NOHIDESEL + ES_LEFT + 
                ES_MULTILINE + ES_READONLY, r.left, r.top, r.right - r.left, r.bottom - r.top,
                hwnd, (HMENU)ID_EDITCHILD, hInstance, 0);
            hsubwnds[1] = CreateWindowEx(0, "XBUILDEDIT", 0, WS_CHILD + 
                WS_VSCROLL + ES_NOHIDESEL + ES_LEFT + ES_MULTILINE + ES_READONLY,
                r.left, r.top, r.right - r.left, r.bottom - r.top, hwnd,
                (HMENU)ID_EDITCHILD, hInstance, 0);
            hsubwnds[2] = CreateWindowEx(0, "XBUILDEDIT", 0, WS_CHILD +
                WS_VSCROLL + ES_NOHIDESEL + ES_LEFT + ES_MULTILINE + ES_READONLY,
                r.left, r.top, r.right - r.left, r.bottom - r.top, hwnd,
                (HMENU)ID_EDITCHILD, hInstance, 0);
            hsubwnds[3] = CreateWindowEx(0, "XBUILDEDIT", 0, WS_CHILD +
                WS_VSCROLL + ES_NOHIDESEL + ES_LEFT + ES_MULTILINE + ES_READONLY,
                r.left, r.top, r.right - r.left, r.bottom - r.top, hwnd,
                (HMENU)ID_EDITCHILD, hInstance, 0);
			
            SendMessage(hsubwnds[0], EM_LIMITTEXT, 4 *65536, 0);
            SendMessage(hsubwnds[1], EM_LIMITTEXT, 4 *65536, 0);
            SendMessage(hsubwnds[2], EM_LIMITTEXT, 4 *65536, 0);
            SendMessage(hsubwnds[3], EM_LIMITTEXT, 4 *65536, 0);
            SendMessage(hsubwnds[0], WM_SETFONT, (WPARAM)textFont, 0);
            SendMessage(hsubwnds[1], WM_SETFONT, (WPARAM)textFont, 0);
            SendMessage(hsubwnds[2], WM_SETFONT, (WPARAM)textFont, 0);
            SendMessage(hsubwnds[3], WM_SETFONT, (WPARAM)textFont, 0);
			for (i=3; i >=0 ; i--)
				SendMessage(hwndTabCtrl, TABM_ADD, (WPARAM)nameTags[i], (LPARAM)hsubwnds[i]);
			colors[0] = colors[1] = colors[2] = colors[3] = RetrieveSysColor(COLOR_WINDOWTEXT);
            return 0;
        case WM_CLOSE:
			break;
		case WM_PAINT:
			dc = BeginPaint(hwnd, &ps);
			pen = CreatePen(PS_SOLID, 0, RetrieveSysColor(COLOR_3DSHADOW));
			pen = SelectObject(dc, pen);
			GetClientRect(hwnd, &r);
			MoveToEx(dc, r.left, r.bottom - 25, NULL);
			LineTo(dc, r.right, r.bottom - 25);
			pen = SelectObject(dc, pen);
			DeleteObject(pen);
			EndPaint(hwnd, &ps);
			return 0;
        case WM_DESTROY:
			DeleteObject(tabNormalFont);
			DeleteObject(textFont);
            DestroyWindow(hwndTabCtrl);
            DestroyWindow(hsubwnds[0]);
            DestroyWindow(hsubwnds[1]);
            DestroyWindow(hsubwnds[2]);
            DestroyWindow(hsubwnds[3]);
            DestroyWindow(hwndCtrl);
            break;
        case WM_SELERRWINDOW:
            ShowWindow(hsubwnds[infoIndex], SW_HIDE);
            infoIndex = (int)lParam;
            ShowWindow(hsubwnds[infoIndex], SW_SHOW);
			SendMessage(hwndTabCtrl, TABM_SELECT, 0, (LPARAM)hsubwnds[infoIndex]);
            SendMessage(hsubwnds[infoIndex], WM_SETTEXT, infoIndex, (LPARAM)"");
            rescroll = FALSE;
            break;
        case WM_SIZE:
            r.left = 0;
            r.right = LOWORD(lParam);
            r.top = 0;
            r.bottom = HIWORD(lParam);
            MoveWindow(hwndTabCtrl, r.left, r.bottom - 24, r.right - r.left, 24, 1);
            MoveWindow(hsubwnds[0], r.left, r.top, r.right - r.left, r.bottom -
                r.top - 25, 1);
            MoveWindow(hsubwnds[1], r.left, r.top, r.right - r.left, r.bottom -
                r.top - 25, 1);
            MoveWindow(hsubwnds[2], r.left, r.top, r.right - r.left, r.bottom -
                r.top - 25, 1);
            MoveWindow(hsubwnds[3], r.left, r.top, r.right - r.left, r.bottom -
                r.top - 25, 1);
            return 0;
        default:
            break;
    }
    return DefWindowProc(hwnd, iMessage, wParam, lParam);
}

//-------------------------------------------------------------------------

void RegisterInfoWindow(void)
{
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = 0;
    wc.lpfnWndProc = &errorProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = szErrorClassName;
    RegisterClass(&wc);

    GetClassInfo(0, "richedit", &wc);
    oldproc = wc.lpfnWndProc;
    wc.lpfnWndProc = &buildEditProc;
    wc.lpszClassName = "XBUILDEDIT";
    wc.hInstance = hInstance;
    wc.hbrBackground = 0;
    RegisterClass(&wc);


}

//-------------------------------------------------------------------------

void CreateInfoWindow(void)
{
    if (!hwndInfo)
	{
		hwndInfo = CreateDockableWindow(DID_ERRORWND, szErrorClassName, szErrorTitle, hInstance, 200, 500);
	}
}
void SetInfoColor(int window, DWORD color)
{
	PostMessage(hwndInfo, WM_SETCOLOR, window, color);
}
void SelectInfoWindow(int window)
{
    PostMessage(hwndInfo, WM_SELERRWINDOW, 0, window);
    SetFocus(hwndInfo);
//    PostMessage(hwndClient, WM_MDIACTIVATE, (WPARAM)hwndInfo, 0);
}
void SendInfoMessage(int window, char *buf)
{
	char *p ;
	if (buf)
		p = strdup(buf);
	else
		p = buf;
    PostMessage(hwndInfo, WM_SETTEXT2, window, (LPARAM)p);
}
