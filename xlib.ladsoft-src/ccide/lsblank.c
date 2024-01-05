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
#include "lsctrl.h"

extern HWND hwndClient;
static char *szBlankClassName = "ladSoftBlankWindow";

//-------------------------------------------------------------------------

static LRESULT CALLBACK BlankWindowWndProc(HWND hwnd, UINT iMessage,
    WPARAM wParam, LPARAM lParam)
{
    RECT r;
    PAINTSTRUCT ps;
    HDC dc;
    CCW_params *ptr;
    switch (iMessage)
    {
        case WM_NOTIFY:
            SendMessage(GetParent(hwnd), iMessage, wParam, lParam);
            break;
        case WM_COMMAND:
            SendMessage(GetParent(hwnd), iMessage, wParam, lParam);
            break;
        case WM_PAINT:
            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
            dc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            break;
        case WM_CREATE:
            ptr = (CCW_params*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
            SetWindowLong(hwnd, 0, (DWORD)ptr);
            return 0;
        case WM_DESTROY:
            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
			free(ptr);
            break;
        case WM_CLOSE:
            break;
        case WM_SIZE:
            break;
        case WM_MOVE:
            break;
    }
    return DefWindowProc(hwnd, iMessage, wParam, lParam);
}

//-------------------------------------------------------------------------

void RegisterBlankWindow(HINSTANCE hInstance)
{
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_PARENTDC;
    wc.lpfnWndProc = &BlankWindowWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(LPVOID);
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH)RetrieveSysBrush(COLOR_BTNFACE);
    wc.lpszMenuName = 0;
    wc.lpszClassName = szBlankClassName;
    RegisterClass(&wc);
}

//-------------------------------------------------------------------------

CCW_params *CreateBlankWindow(HWND parent)
{
	CCW_params *p = calloc(sizeof(CCW_params), 1);
	if (p)
	{
		p->hwnd = CreateWindow(szBlankClassName, 0, WS_CLIPSIBLINGS |
        	WS_CLIPCHILDREN | WS_CHILD, 0, 0, 10, 10, parent, 0, 
			(HINSTANCE)GetWindowLong(parent, GWL_HINSTANCE), p);
		p->type = LSBLANK;
	}
    return p;
}

