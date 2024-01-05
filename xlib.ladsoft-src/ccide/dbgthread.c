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
#include "operands.h"
#include "opcodes.h"
#include <ctype.h>
#include "cvinfo.h"

extern HINSTANCE hInstance;
extern HWND hwndClient, hwndStatus, hwndFrame;
extern HWND hwndRegister, hwndWatch, hwndStack, hwndThread, hwndMem, hwndASM;
extern THREAD *activeThread;
extern PROCESS *activeProcess;
extern enum DebugState uState;
extern HWND hwndTbThreads;

HWND hwndThread;

static char szThreadClassName[] = "xccThreadClass";
static int DisplayThreads;
static HWND hwndLV;
static int curSel;
static HIMAGELIST tagImageList;
static WNDPROC oldLVProc;

static LOGFONT fontdata = 
{
    -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_MODERN | FF_DONTCARE, 
		CONTROL_FONT
};
static HFONT ThreadFont;
static char *szThreadTitle = "Thread Window";

static LRESULT CALLBACK lvProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam)
{
	static HWND hwndFriend;
	switch (iMessage)
	{
		case WM_USER + 10000:
			hwndFriend = (HWND)lParam;
			return 0;
		case WM_KEYDOWN:
		case WM_KEYUP:
			SendMessage( hwndFriend, iMessage, wParam, lParam);
            break;
	}
	return CallWindowProc(oldLVProc, hwnd, iMessage, wParam, lParam);
}
static void CopyText(HWND hwnd)
{
	int count;
    THREAD *list = activeProcess->threads;
	char *p;
    while (list)
    {
        list = list->next;
		count++;
    }
	p = malloc(256 * count);
	if (p)
	{
		int i;
		p[0] = 0;
		list = activeProcess->threads;
		while (list)
        {
            char name[256];
            int eip = list->regs.Eip;
            if (!FindFunctionName(name, eip))
                name[0] = 0;
            if (list->idThread == activeThread->idThread)
                strcat(name, "*");
            sprintf(p + strlen(p), "%d\t%s\n", list->idThread, name);
            list = list->next;
        }
		TextToClipBoard(hwnd, p);
		free(p);
	}
}

//-------------------------------------------------------------------------

LRESULT CALLBACK ThreadProc(HWND hwnd, UINT iMessage, WPARAM
    wParam, LPARAM lParam)
{
    LV_ITEM item;
    LV_COLUMN lvC;
    RECT r;
    int i;
    char module[256];
    THREAD *sl;
    LPNMHDR nmh;
    switch (iMessage)
    {
        case WM_TIMER:
            KillTimer(hwnd, 100);
            ListView_SetItemState(hwndLV, curSel, 0, LVIS_SELECTED);
            break;
        case WM_NOTIFY:
            nmh = (LPNMHDR)lParam;
            if (nmh->code == NM_SETFOCUS)
            {
                PostMessage(hwndFrame, WM_REDRAWTOOLBAR, 0, 0);
    			SendMessage(GetParent(hwnd), WM_ACTIVATEME, 0, 0);
            }
			else if (nmh->code == LVN_GETDISPINFO)
            {
                LV_DISPINFO *p = (LV_DISPINFO *)lParam;
                THREAD *x = (THREAD *)p->item.lParam;
                char name[256];
                if (p->item.iSubItem == 2)
                {
                    int eip = x->regs.Eip;
                    if (!FindFunctionName(name, eip))
                        name[0] = 0;
                }
                else
                {
                    sprintf(name, "%d", x->idThread);
                }
                p->item.mask |= LVIF_TEXT | LVIF_DI_SETITEM;
                p->item.mask &= ~LVIF_STATE;
                p->item.pszText = name;
            }
			else if (nmh->code == LVN_ITEMCHANGED)
            {
                LPNMLISTVIEW p = (LPNMHDR)lParam;
                if (p->uChanged & LVIF_STATE)
                {
                    if (p->uNewState & LVIS_SELECTED)
                    {
                        i = 0;
                        PostMessage(hwnd, WM_USER, p->iItem, 0);
                        SetTimer(hwnd, 100, 400, 0);
                    }
                }
            }
            break;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case ID_TBTHREADS:
                    if (HIWORD(wParam) == CBN_SELENDOK)
                    {
                        int i = SendMessage(hwndTbThreads, CB_GETCURSEL, 0, 0);
                        if (i != CB_ERR)
                        {
                            SendMessage(hwnd, WM_USER, i, 0);
                            curSel = i;
                            
                        }
                    }
                    break;
            }
            break;
        case WM_USER:
        {
            memset(&item, 0, sizeof(item));
            item.iItem = curSel;
            item.iSubItem = 0;
            item.mask = LVIF_IMAGE;
            item.iImage = 8;
            ListView_SetItem(hwndLV, &item);

            curSel = wParam;
            sl = activeProcess->threads;
            while (sl && wParam--)
                sl = sl->next;
            activeThread = sl;
            item.iItem = curSel;
            item.mask = LVIF_IMAGE;
            item.iImage = 4;
            ListView_SetItem(hwndLV, &item);
            if (hwndRegister)
                PostMessage(hwndRegister, WM_COMMAND, ID_SETADDRESS, (LPARAM)
                    activeThread->hThread);
            PostMessage(hwndWatch, WM_COMMAND, ID_SETADDRESS, 0);
            if (hwndStack)
                PostMessage(hwndStack, WM_RESTACK, (WPARAM)1, 0);
            if (hwndMem)
                PostMessage(hwndMem, WM_RESTACK, 0, 0);
            if (hwndASM)
                SendMessage(hwndASM, WM_COMMAND, ID_SETADDRESS, (LPARAM)
							activeThread->regs.Eip);
            SendMessage(hwndTbThreads, CB_SETCURSEL, curSel, 0);

        }
            break;
        case WM_CREATE:
            hwndThread = hwnd;
            GetClientRect(hwnd, &r);
            hwndLV = CreateWindowEx(0, WC_LISTVIEW, "", 
                           LVS_REPORT | LVS_SINGLESEL | WS_CHILD | WS_VISIBLE | WS_BORDER,
                           0,0,r.right-r.left, r.bottom - r.top, hwnd, 0, hInstance, 0);
            ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
            ThreadFont = CreateFontIndirect(&fontdata);
            SendMessage(hwndLV, WM_SETFONT, (WPARAM)ThreadFont, 0);
            lvC.mask = LVCF_WIDTH | LVCF_SUBITEM ;
            lvC.cx = 20;
            lvC.iSubItem = 0;
            ListView_InsertColumn(hwndLV, 0, &lvC);
            lvC.mask = LVCF_WIDTH | LVCF_SUBITEM | LVCF_TEXT;
            lvC.cx = 60;
            lvC.iSubItem = 1;
            lvC.pszText = "Id";
            ListView_InsertColumn(hwndLV, 1, &lvC);
            lvC.mask = LVCF_WIDTH | LVCF_SUBITEM | LVCF_TEXT;
            lvC.cx = 120;
            lvC.iSubItem = 2;
            lvC.pszText = "Location";
            ListView_InsertColumn(hwndLV, 2, &lvC);
            ListView_SetImageList(hwndLV, tagImageList, LVSIL_SMALL);
        	oldLVProc = (WNDPROC)GetWindowLong(hwndLV, GWL_WNDPROC);
        	SetWindowLong(hwndLV, GWL_WNDPROC, (long)lvProc);
        	SendMessage(hwndLV, WM_USER + 10000, 0, (LPARAM)hwnd);
            break;
        case WM_SIZE:
            r.left = r.top = 0;
            r.right = LOWORD(lParam);
            r.bottom = HIWORD(lParam);
            MoveWindow(hwndLV, r.left, r.top, r.right - r.left,
                r.bottom - r.top, 1);
            break;
        case WM_DESTROY:
            hwndThread = 0;
            DeleteObject(ThreadFont);
            break;
        case WM_RESTACK:
            EnableWindow(hwndLV, uState != notDebugging && wParam);
            EnableWindow(hwndTbThreads, uState != notDebugging && wParam);
            if (uState != notDebugging && wParam)
            {
                
                int i = 0;
                THREAD *list = activeProcess->threads;
                ListView_DeleteAllItems(hwndLV);
                memset(&item, 0, sizeof(item));
                SendMessage(hwndTbThreads, CB_RESETCONTENT, 0, 0);
                while (list)
                {
                    char buf[260];
                    item.iItem = i;
                    item.iSubItem = 0;
                    item.mask = LVIF_IMAGE | LVIF_PARAM;
                    if (list->idThread == activeThread->idThread)
                    {
                        item.iImage = 4;
                        curSel = i;
                    }
                    else
                    {
                        item.iImage = 8;
                    }
                    item.lParam = (LPARAM)list;
                    ListView_InsertItem(hwndLV, &item);
                    
                    item.iSubItem = 1;
                    item.mask = LVIF_PARAM | LVIF_TEXT;
                    item.lParam = (LPARAM)list;
                    item.pszText = "";
                    ListView_InsertItem(hwndLV, &item);

                    item.iSubItem = 2;
                    item.mask = LVIF_PARAM | LVIF_TEXT;
                    item.lParam = (LPARAM)list;
                    item.pszText = "";
                    ListView_InsertItem(hwndLV, &item);
                    sprintf(buf, "%d %s", list->idThread, list->name);
                    SendMessage(hwndTbThreads, CB_ADDSTRING, 0, (LPARAM)buf);
                    i++, list = list->next;
                }
                SendMessage(hwndTbThreads, CB_SETCURSEL, curSel, 0);
            }
            break;
        case WM_SETFOCUS:
            break;
        case WM_KILLFOCUS:
            break;
        case WM_KEYDOWN:
            switch (wParam)
            {
			case 'C':
                if (GetKeyState(VK_CONTROL) &0x80000000)
				{
                    MessageBeep(0);
					CopyText(hwnd);
				}
				break;
            case VK_UP:
                if (curSel > 0)
                    SendMessage(hwnd, WM_USER, curSel-1, 0);
                break;
            case VK_DOWN:
                if (curSel < ListView_GetItemCount(hwndLV) - 1)
                    SendMessage(hwnd, WM_USER, curSel + 1, 0);
                break;
            }
            break;
    }
    return DefWindowProc(hwnd, iMessage, wParam, lParam);
}

//-------------------------------------------------------------------------

void RegisterThreadWindow(void)
{
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_HREDRAW + CS_VREDRAW + CS_DBLCLKS;
    wc.lpfnWndProc = &ThreadProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = 0; // GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = szThreadClassName;
    RegisterClass(&wc);
}

//-------------------------------------------------------------------------

HWND CreateThreadWindow(void)
{
    if (hwndThread)
    {
        SendMessage(hwndThread, WM_SETFOCUS, 0, 0);
    }
	else
	{
        HBITMAP bitmap;
        bitmap = LoadBitmap(hInstance, "ID_TAG");
        ChangeBitmapColor(bitmap, 0xc0c0c0, RetrieveSysColor(COLOR_WINDOW));
        tagImageList = ImageList_Create(16, 16, ILC_COLOR24, ILEDIT_IMAGECOUNT, 0);
    	ImageList_Add(tagImageList, bitmap, NULL);
        DeleteObject(bitmap);
		hwndThread = CreateDockableWindow(DID_THREADWND, szThreadClassName, szThreadTitle, hInstance, 30 * 8, 19 * 8);
	}
    return hwndThread;
}
