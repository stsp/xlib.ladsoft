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
#include "winconst.h"
#include "header.h"
 
extern HWND hwndSrcTab;
extern HWND hwndClient;
static char *szContainerClassName = "ladSoftContainerWindow";
static HCURSOR mcurs;
//-------------------------------------------------------------------------

LRESULT CALLBACK ContainerProc(HWND hwnd, UINT iMessage, WPARAM
    wParam, LPARAM lParam)
{
    CCW_params *ptr,  *ptr1;
    LRESULT rv;
    RECT r;
    HDC dc;
    PAINTSTRUCT ps;
	LPCREATESTRUCT createStruct;
    static HCURSOR oldCursor;
    static int dragging, sizing;
    POINT temppt;
    switch (iMessage)
    {
		case WM_ACTIVATEME:
			SendMessage(hwndClient, WM_MDIACTIVATE, (WPARAM)hwnd, 0);
			break;
		case WM_GETTEXT:
        case WM_SETTEXT:
            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
            if (ptr->child)
    			return SendMessage(ptr->child->hwnd, iMessage, wParam, lParam);
            else
                return 0;
        case WM_MDIACTIVATE:
			if (hwnd == (HWND)lParam)
			{
	            PostMessage(hwndSrcTab, WM_SETACTIVETAB, 0, (LPARAM)hwnd);
			}
            break;
        case WM_SYSCOMMAND:
            if (wParam == SC_CLOSE)
            {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
            break;
        case WM_NCLBUTTONDOWN:
			switch(wParam)
			{
				case HTCAPTION:
		            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
					temppt.x = LOWORD(lParam);
					temppt.y = HIWORD(lParam);
	                dmgrStartMoveClient(ptr, &temppt);
    	            oldCursor = SetCursor(mcurs);
        	        SetCapture(hwnd);
            	    dragging = TRUE;
					return 0;
				default:
					break;
	        }
            break;
        case WM_NCLBUTTONUP:
		case WM_LBUTTONUP:
			if (dragging)
			{
	            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
                SetCursor(oldCursor);
	            oldCursor = 0;
                ReleaseCapture();
	            dragging = FALSE;
				temppt.x = (long)(short)LOWORD(lParam);
				temppt.y = (long)(short)HIWORD(lParam);
				ClientToScreen(hwnd, &temppt);
	            dmgrEndMoveClient(ptr, &temppt);
				return 0;
			}
            break;
        case WM_NCMOUSEMOVE:
		case WM_MOUSEMOVE:
            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
            if (dragging)
            {
				temppt.x = (long)(short)LOWORD(lParam);
				temppt.y = (long)(short)HIWORD(lParam);
				if (iMessage == WM_MOUSEMOVE)
				{
					GetWindowRect(hwnd, &r);
					temppt.x += r.left + GetSystemMetrics(SM_CXFRAME);
					temppt.y += r.top + GetSystemMetrics (SM_CYCAPTION);
				}
                dmgrMoveClient(ptr, &temppt, FALSE);
				return 0;
            }
            break;
        case WM_NOTIFY:
        case WM_COMMAND:
            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
            if (ptr->child)
    			SendMessage(ptr->child->hwnd, iMessage, wParam, lParam);
            break;
        case WM_CREATE:
			createStruct = (LPCREATESTRUCT)lParam;
			ptr = (CCW_params *)((LPMDICREATESTRUCT)(createStruct->lpCreateParams))->lParam;
            SetWindowLong(hwnd, 0, (long)ptr);
			break;
        case WM_PAINT:
            dc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &r);
            EndPaint(hwnd, &ps);
            break;
        case WM_DESTROY:
			PostMessage(hwndSrcTab, TABM_REMOVE, 0, (LPARAM)hwnd);
            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
			free(ptr);
            break;
        case WM_SIZE:
            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
            GetClientRect(hwnd, &r);
            if (ptr->child)
                MoveWindow(ptr->child->hwnd, 0, 0, r.right, r.bottom,1);
//            InvalidateRect(ptr->child->hwnd,0,TRUE);
            break;
        case WM_CLOSE:
			PostMessage(hwndSrcTab, TABM_REMOVE, 0, (LPARAM)hwnd);
            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
            if (ptr->child)
                dmgrHideWindow(ptr->child->id, TRUE);
            return 0;
		case WM_SHOWWINDOW:
            ptr = (CCW_params*)GetWindowLong(hwnd, 0);
            if (ptr->child)
			{
				char buf[256];
                
				GetWindowText(ptr->child->hwnd, buf, 256);
				if (wParam)
				{
					PostMessage(hwndSrcTab, TABM_ADD, (WPARAM)buf, (LPARAM)hwnd);
				}
				else
				{
					PostMessage(hwndSrcTab, TABM_REMOVE, 0, (LPARAM)hwnd);
				}
			}
			break;
			
		case WM_MOVE:
			break;
    }
    return DefMDIChildProc(hwnd, iMessage, wParam, lParam);
}


//-------------------------------------------------------------------------

void RegisterContainerWindow(HINSTANCE hInstance)
{
    WNDCLASS wc;

    memset(&wc, 0, sizeof(wc));
    wc.style = 0;
    wc.lpfnWndProc = &ContainerProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 4;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszMenuName = 0;
    wc.lpszClassName = szContainerClassName;
    RegisterClass(&wc);
    mcurs = LoadCursor(NULL, IDC_SIZEALL);
}

//-------------------------------------------------------------------------

CCW_params *CreateContainerWindow(HWND parent, CCW_params *child, RECT *r)
{
	CCW_params *p = calloc(sizeof(CCW_params), 1);
	if (p)
	{
		p->child = child;
		p->hwnd = CreateMDIWindow(szContainerClassName, "",  WS_VISIBLE|	  
    	WS_CHILD | WS_OVERLAPPEDWINDOW -WS_MAXIMIZEBOX | WS_SYSMENU | MDIS_ALLCHILDSTYLES | 
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
        WS_SIZEBOX | WS_MINIMIZEBOX,
        	r->left,
			r->top,
			r->right -r->left, 
			r->bottom - r->top,
			parent, (HINSTANCE)GetWindowLong(parent, GWL_HINSTANCE), (LPARAM)p);
		p->type = LSCONTAINER;
	}
    return p;
}
