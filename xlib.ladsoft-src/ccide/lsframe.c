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

static char *szFrameClassName = "ladSoftFrameWindow";
static HCURSOR hcurs, vcurs;

#define FRAMEBODY 2

static void DrawFrame(HDC dc, RECT *r, int vertical)
{
    HBRUSH brush, oldbrush;
    HPEN pen1, pen2, oldpen;
    pen1 = CreatePen(PS_SOLID, 2, RetrieveSysColor(COLOR_BTNHIGHLIGHT));
    pen2 = CreatePen(PS_SOLID, 1, RetrieveSysColor(COLOR_BTNSHADOW));
    brush = CreateSolidBrush(RetrieveSysColor(COLOR_BTNFACE));
    oldbrush = SelectObject(dc, brush);
    FillRect(dc, r, brush);

    oldpen = SelectObject(dc, pen1);
    MoveToEx(dc, r->left, r->top, 0);
    LineTo(dc, r->right, r->top);
    //         MoveToEx(dc,r->left,r->top+1,0 );
    //         LineTo(dc,r->right,r->top+1) ;

    SelectObject(dc, pen2);

    if (vertical)
    {
        //            MoveToEx(dc,r->left,r->top,0 );
        //            LineTo(dc,r->right,r->top) ;
    }
    else
    {

        //           MoveToEx(dc,r->left,r->bottom-1,0 );
        //          LineTo(dc,r->right,r->bottom-1) ;
        //           MoveToEx(dc,r->left,r->bottom-2,0 );
        //          LineTo(dc,r->right,r->bottom-2) ;
    }

    if (vertical)
    {
        SelectObject(dc, pen1);
        MoveToEx(dc, r->left, r->top + 1, 0);
        LineTo(dc, r->left, r->bottom - 2);
        //           MoveToEx(dc,r->left+1,r->top,0 );
        //           LineTo(dc,r->left+1,r->bottom-1) ;

        SelectObject(dc, pen2);
        //           MoveToEx(dc,r->left,r->top,0 );
        //           LineTo(dc,r->left,r->bottom-1) ;
        MoveToEx(dc, r->right - 1, r->top, 0);
        LineTo(dc, r->right - 1, r->bottom - 1);
    }
    else
    {
        SelectObject(dc, pen1);
        MoveToEx(dc, r->left + 1, r->top, 0);
        LineTo(dc, r->right - 1, r->top);

        SelectObject(dc, pen2);
        MoveToEx(dc, r->left + 1, r->bottom - 1, 0);
        LineTo(dc, r->right - 1, r->bottom - 1);
    }
    SelectObject(dc, oldpen);
    SelectObject(dc, oldbrush);
    DeleteObject(pen1);
    DeleteObject(pen2);
    DeleteObject(brush);
}

//-------------------------------------------------------------------------

static LRESULT CALLBACK FrameWndProc(HWND hwnd, UINT iMessage,
    WPARAM wParam, LPARAM lParam)
{
    RECT r;
    PAINTSTRUCT ps;
    HDC dc;
    CCW_params *ptr;
    static int dragging;
	POINT temppt;
    switch (iMessage)
    {
        case WM_NOTIFY:
            break;
        case WM_COMMAND:
            break;
        case WM_LBUTTONDOWN:
            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
            dragging = TRUE;
			temppt.x = LOWORD(lParam);
			temppt.y = HIWORD(lParam);
			ClientToScreen(hwnd, &temppt);
            dmgrSizeBarStartMove(ptr, &temppt);
            SetCapture(hwnd);
            break;
        case WM_LBUTTONUP:
            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
            dmgrSizeBarEndMove(ptr);
            dragging = FALSE;
            ReleaseCapture();
            break;
        case WM_MOUSEMOVE:
            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
            SetCursor(ptr->vertical ? hcurs : vcurs);
            if (dragging)
			{
				temppt.x = (long)(short)LOWORD(lParam);
				temppt.y = (long)(short)HIWORD(lParam);
				ClientToScreen(hwnd, &temppt);
                dmgrSizeBarMove(ptr, &temppt);
			}
            break;
        case WM_ERASEBKGND:
            return 0;
        case WM_PAINT:
            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
            dc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &r);
            DrawFrame(dc, &r, ptr->vertical);
            EndPaint(hwnd, &ps);
            return 0;
        case WM_CREATE:
            ptr = (CCW_params*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
            SetWindowLong(hwnd, 0, (DWORD)ptr);
            break;
        case WM_DESTROY:
            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
			free(ptr);
            break;
        case WM_CLOSE:
			break;
        case WM_SIZE:
            break ;
        case WM_MOVE:
            break ;
    }
    return DefWindowProc(hwnd, iMessage, wParam, lParam);
}

//-------------------------------------------------------------------------

void RegisterFrameWindow(HINSTANCE hInstance)
{
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = 0;
    wc.lpfnWndProc = &FrameWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(LPVOID);
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = NULL;
    wc.hbrBackground = RetrieveSysBrush(COLOR_BTNFACE);
    wc.lpszMenuName = 0;
    wc.lpszClassName = szFrameClassName;
    RegisterClass(&wc);
    //         hcurs = LoadCursor(hInstance,"ID_SIZEHCUR") ;
    //         vcurs = LoadCursor(hInstance,"ID_SIZEVCUR") ;
    hcurs = LoadCursor(0, IDC_SIZEWE);
    vcurs = LoadCursor(0, IDC_SIZENS);

}

//-------------------------------------------------------------------------

CCW_params *CreateFrameWindow(HWND parent)
{
	CCW_params *p = calloc(sizeof(CCW_params), 1);
	if (p)
	{
		p->hwnd = CreateWindow(szFrameClassName, 0, WS_CLIPSIBLINGS |
        	WS_CLIPCHILDREN | WS_CHILD, 0, 0, 10, 10, parent, 0, 
			(HINSTANCE)GetWindowLong(parent, GWL_HINSTANCE), p);
		p->type = LSFRAME;
	}
    return p;
}
