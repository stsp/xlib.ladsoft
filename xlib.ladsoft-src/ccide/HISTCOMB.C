/* 
CCIDE
Copyright 2001-2006 David Lindauer.

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
#include "header.h"

extern HINSTANCE hInstance;

static int wndoffs;
static WNDPROC oldComboProc;
static WNDPROC oldEditProc;

static short *propname = L"HISTBUF";

LRESULT CALLBACK historyEditComboProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam)
{
	switch (iMessage)
	{
		case WM_KEYDOWN:
			switch(wParam)
			{
				case VK_RETURN:
				case VK_ESCAPE:
					SendMessage(GetParent(hwnd), WM_COMMAND, (wParam << 16) + 1698, (LPARAM)hwnd);
					return 0;
			}
			break;
		case WM_KEYUP:
			switch(wParam)
			{
				case VK_RETURN:
				case VK_ESCAPE:
					return 0;
			}
            break;
		default:
			break;
	}
    return CallWindowProc(oldEditProc, hwnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK historyComboProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam)
{
    char **cbptr;
    char buf[MAX_PATH];
    int i;
    POINT pt;
    HWND hwndedit;
    pt.x = pt.y = 1;
    hwndedit = ChildWindowFromPoint(hwnd, pt);
    switch (iMessage)
    {
        case WM_SETMODIFY:
            return SendMessage(hwndedit, WM_SETTEXT, 0, (LPARAM)" "); 
                // hack to fix a timing varagary that kept the find button disabled even though text was changing
        case WM_COMMAND:
			if (LOWORD(wParam) == 1698)
			{
				// return or escape key pressed in edit box...
				PostMessage((HWND)GetWindowLong(hwnd, GWL_HWNDPARENT), WM_COMMAND, (4000 + HIWORD(wParam)) , (LPARAM)hwnd);
			}
            //         if ((wParam >> 16) == EN_CHANGE || (wParam >> 16) == EN_UPDATE)
            //            return SendMessage(GetParent(hwnd),WM_COMMAND,(wParam & 0xffff0000) + GetWindowLong(hwnd,GWL_ID), (LPARAM)hwnd) ;
            break;
        case WM_SETHISTORY:
            cbptr = (char **)lParam;
            SetProp(hwnd, propname, cbptr);
            for (i = 0; i < MAX_COMBO_HISTORY; i++)
            if (cbptr[i])
            {
                SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)cbptr[i]);
            }
            SendMessage(hwnd, CB_SETCURSEL, 0, 0);

            return (0);
        case WM_SAVEHISTORY:
            cbptr = GetProp(hwnd, propname);
            if (cbptr)
            {
                int sel; // = SendMessage(hwnd, CB_GETCURSEL,0,0) ;
                buf[SendMessage(hwndedit, WM_GETTEXT, MAX_PATH, (LPARAM)buf)] = 0;
                sel = SendMessage(hwnd, CB_FINDSTRINGEXACT, 0, (LPARAM)buf);
                if (sel == CB_ERR)
                {
                    if (buf[0])
                    {
                        if (cbptr[MAX_COMBO_HISTORY - 1])
                        {
                            free(cbptr[MAX_COMBO_HISTORY - 1]);
                        }
                        memmove(cbptr + 1, cbptr, (MAX_COMBO_HISTORY - 1)
                            *sizeof(char*));
                        cbptr[0] = strdup(buf);
                        SendMessage(hwnd, CB_INSERTSTRING, 0, (LPARAM)buf);
                        //                  SendMessage(hwnd,CB_SETCURSEL,-1,0 );
                        //                  SendMessage(hwndedit,WM_SETTEXT,0,(LPARAM)buf) ;
                    }
                }
                else
                {
                    char *s = cbptr[sel];
                    memmove(cbptr + 1, cbptr, sel *sizeof(char*));
                    cbptr[0] = s;
                }
            }
            return 0;
        case WM_DESTROY:
            RemoveProp(hwnd, propname);
            break;
			
    }
    return CallWindowProc(oldComboProc, hwnd, iMessage, wParam, lParam);

}

//-------------------------------------------------------------------------

void SubClassHistoryCombo(HWND combo)
{
	POINT pt;
	HWND editWnd;
	pt.x = 5;
	pt.y = 5;
	editWnd = ChildWindowFromPoint(combo, pt);
    SetWindowLong(combo, GWL_WNDPROC, (int)historyComboProc);
	SetWindowLong(editWnd, GWL_WNDPROC, (int)historyEditComboProc);
}

//-------------------------------------------------------------------------

void RegisterHistoryComboWindow(void)
{
    WNDCLASS wc;
    GetClassInfo(0, "combobox", &wc);
    oldComboProc = wc.lpfnWndProc;
    GetClassInfo(0, "edit", &wc);
    oldEditProc = wc.lpfnWndProc;
    return ;
    wc.lpfnWndProc = &historyComboProc;
    wc.lpszClassName = "historycombo";
    wc.hInstance = hInstance;
    wc.cbWndExtra += (4-wc.cbWndExtra % 4);
    wndoffs = wc.cbWndExtra;
    wc.cbWndExtra += 4;
    RegisterClass(&wc);
}
