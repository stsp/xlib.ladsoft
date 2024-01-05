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
#include "header.h"
#include <dir.h>

extern HWND hwndFrame, hwndClient, hwndProject, hwndRegister, hwndWatch;
extern HINSTANCE hInstance;

HWND hwndTab;
HWND hwndTabCtrl;
static char szTabClassName[] = "xccTabClass";
static char szTabTitle[] = "Workspace";
static HFONT tabBoldFont, tabNormalFont;
static char *nameTags[] = 
{
    "Files", "Register"
};
static HBITMAP projBitmap, regsBitmap;
static HBITMAP *bitmaps[] = 
{
     &projBitmap, &regsBitmap
};

static LOGFONT Boldfontdata = 
{
    -12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE,
		"Arial"
};
static LOGFONT Normalfontdata = 
{
    -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, 
		"Arial"
};
void GetTabRect(RECT *rect)
{
    GetClientRect(hwndTabCtrl, rect);
    //   SendMessage(hwndCtrl,LCF_ADJUSTRECT,0,(LPARAM)rect) ;
    TabCtrl_AdjustRect(hwndTabCtrl, FALSE, rect);

}

//-------------------------------------------------------------------------

LRESULT CALLBACK TabWndProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam)
{
    static int selected, sizingbottom;
	static HIMAGELIST tabIml;
	static int ilProj;
    RECT r,  *pr;
    TC_ITEM tie;
    NMHDR *h;
    DRAWITEMSTRUCT *dr;
    HFONT font;
    HBITMAP hbmp;
    HDC hMemDC;
    HDWP deferstruct;
    switch (iMessage)
    {
        case WM_SYSCOMMAND:
            if (wParam == SC_CLOSE)
            {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
                return 0;
            }
            break;
        case WM_NOTIFY:
            h = (NMHDR*)lParam;
            switch (h->code)
            {
            case TCN_SELCHANGE:
                switch (selected = TabCtrl_GetCurSel(hwndTabCtrl))
                {
                case 0:
                    //                     ShowWindow(hwndRegister, SW_HIDE) ;
                    //                     ShowWindow(hwndProject, SW_SHOW) ;
                    break;
                case 1:
                    //                     ShowWindow(hwndProject, SW_HIDE) ;
                    //                     ShowWindow(hwndRegister, SW_SHOW) ;
                    break;
                }
            }
            break;
        case WM_SETTEXT:
            return SendMessage(hwndTabCtrl, iMessage, wParam, lParam);
        case WM_DRAWITEM:
            dr = (DRAWITEMSTRUCT*)lParam;
            if (dr->itemState &ODS_SELECTED)
                font = tabBoldFont;
            else
                font = tabNormalFont;
            hMemDC = CreateCompatibleDC(dr->hDC);
            hbmp = SelectObject(hMemDC,  *bitmaps[dr->itemID]);
            BitBlt(dr->hDC, dr->rcItem.left + 2, dr->rcItem.bottom - 18, 16, 16,
                hMemDC, 0, 0, SRCCOPY);
            font = SelectObject(dr->hDC, font);
            TextOut(dr->hDC, dr->rcItem.left + 18, dr->rcItem.bottom - 16,
                nameTags[dr->itemID], strlen(nameTags[dr->itemID]));
            font = SelectObject(dr->hDC, font);
            SelectObject(dr->hDC, hbmp);
            DeleteDC(hMemDC);
            break;
        case WM_CREATE:
            hwndTab = hwnd;
            GetClientRect(hwnd, &r);
            hwndTabCtrl = CreateWindow(WC_TABCONTROL, 0, WS_CHILD +
                WS_CLIPSIBLINGS + WS_VISIBLE + TCS_FLATBUTTONS /*+ TCS_OWNERDRAWFIXED */
                + TCS_FOCUSNEVER /*+ TCS_FIXEDWIDTH*/ + TCS_BOTTOM, r.left, r.top,
                r.right - r.left, r.bottom - r.top, hwnd, 0, hInstance, 0);
            tabBoldFont = CreateFontIndirect(&Boldfontdata);
            tabNormalFont = CreateFontIndirect(&Normalfontdata);
            SendMessage(hwndTabCtrl, WM_SETFONT, (WPARAM)tabNormalFont, 0);
//			SendMessage(hwndTabCtrl, TCM_SETEXTENDEDSTYLE, TCS_EX_FLATSEPARATORS, TCS_EX_FLATSEPARATORS);
            projBitmap = LoadImage(hInstance, "ID_PROJBMP", IMAGE_BITMAP,0,0,LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS);
//		    ChangeBitmapColor(projBitmap, 0xc0c0c0, RetrieveSysColor(COLOR_3DFACE));
            //         regsBitmap = LoadBitmap(hInstance, "ID_REGSBMP") ;
            //         SendMessage(hwndTabCtrl,WM_SETFONT,(WPARAM)tabBoldFont,0) ;
            tabIml = ImageList_Create(16, 16, ILC_COLOR24, 1, 0);
            ilProj = ImageList_Add(tabIml, projBitmap, 0);
            TabCtrl_SetImageList(hwndTabCtrl, tabIml);

            CreateProjectWindow();
            //         CreateRegisterWindow(0, hwndTabCtrl) ;
            tie.mask = TCIF_TEXT | TCIF_IMAGE;
            tie.iImage = ilProj;
            tie.pszText = nameTags[0];
            TabCtrl_InsertItem(hwndTabCtrl, 0, &tie);
            //         tie.mask = TCIF_TEXT | TCIF_IMAGE ;
            //         tie.iImage = -1 ;
            //         tie.pszText  = nameTags[1] ;
            //         TabCtrl_InsertItem(hwndTabCtrl,1, &tie) ;
            //         ShowWindow(hwndRegister,SW_HIDE) ;
            return 0;
        case WM_COMMAND:
            break;
        case WM_DESTROY:
            //         DestroyWindow(hwndRegister) ;
            DestroyWindow(hwndProject);
            DestroyWindow(hwndTabCtrl);
            DeleteObject(projBitmap);
            //         DeleteObject(regsBitmap) ;
            DeleteObject(tabNormalFont);
            DeleteObject(tabBoldFont);
            hwndTab = 0;
            break;
        case WM_SIZE:
            r.right = LOWORD(lParam);
            r.bottom = HIWORD(lParam);
            r.left = r.top = 0;
            MoveWindow(hwndTabCtrl, r.left, r.top, r.right - r.left, r.bottom -
                r.top, 1);
            GetTabRect(&r);
            MoveWindow(hwndProject, r.left, r.top, r.right - r.left, r.bottom -
                r.top, 1);
            //         MoveWindow( hwndRegister,r.left,r.top,r.right-r.left,r.bottom-r.top,TRUE) ;
            break;
        case WM_CLOSE:
			break;
    }
    return DefWindowProc(hwnd, iMessage, wParam, lParam);
}

//-------------------------------------------------------------------------

void RegisterTabWindow(void)
{
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = 0;
    wc.lpfnWndProc = &TabWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = szTabClassName;
    RegisterClass(&wc);

}

//-------------------------------------------------------------------------

void CreateTabWindow(void)
{
	hwndTab = CreateDockableWindow(DID_TABWND, szTabClassName, szTabTitle, hInstance, 200, 200);
}
