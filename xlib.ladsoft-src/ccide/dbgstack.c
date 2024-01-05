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
extern enum DebugState uState;
extern THREAD *activeThread, *stoppedThread;
extern PROCESS *activeProcess;
extern HWND hwndTbProcedure;

HWND hwndStack;
SCOPE *activeScope;
SCOPE *StackList;

static char szStackClassName[] = "xccStackClass";

static LOGFONT fontdata = 
{
    -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH |
        FF_DONTCARE,
		CONTROL_FONT

};
static HIMAGELIST tagImageList;
static HFONT StackFont;
static int curSel;
static char *szStackTitle = "Call Stack Window";
static HWND hwndLV;
static WNDPROC oldLVProc;

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
    SCOPE *list = StackList;
	int count = 0; 
	char *p;
	while (list)
	{
		count++;
		list = list->next;
	}
	p = malloc(count * 256);
	if (p)
	{
		list = StackList;
		p[0] = 0;
		while (list)
		{
	        sprintf(p + strlen(p), "%08X\t%s\n", list->address, list->name);
			list = list->next;
		}
		TextToClipBoard(hwnd, p);
		free(p);
	}
}
static void SetScope(SCOPE *newScope)
{
	activeScope = newScope;
	RedrawAllBreakpoints;
}

//-------------------------------------------------------------------------

void ClearStackArea(HWND hwnd)
{
    while (StackList)
    {
        SCOPE *s = StackList->next;
        free(StackList);
        StackList = s;
    }
    curSel = 0;
}

//-------------------------------------------------------------------------

int eipReal(int eip)
{
    int len, i;
    unsigned char buf[16];
    ReadProcessMemory(activeProcess->hProcess, (LPVOID)(eip - 16), (LPVOID) &buf,
        16, &len);
    if (buf[11] == 0xe8)
        return eip - 5;
    for (i = 14; i >= 0; i--)
        if (buf[i] == 0xff && ((buf[i] &0x38) == 0x10))
            return eip - (16-i);
    return eip;
}

//-------------------------------------------------------------------------

int readStackedData(int inebp, int *outebp)
{
    DWORD eip = 0;
    int len = 0;
    ReadProcessMemory(activeProcess->hProcess, (LPVOID)(inebp + 4), (LPVOID) &eip,
        4, &len);
    ReadProcessMemory(activeProcess->hProcess, (LPVOID)inebp, (LPVOID)outebp, 4,
        &len);
    return eip;
}

//-------------------------------------------------------------------------

void SetStackArea(HWND hwnd)
{
    int ebp = activeThread->regs.Ebp;
    int eip = activeThread->regs.Eip;
    int fl=0, ebx=0, esi=0, edi=0; 
    SCOPE *stackbase = 0,  *stackptr,  *newStack;

    if (uState != atBreakpoint && uState != atException)
        return ;
    while (1)
    {
        eip = eipReal(eip);
        newStack = calloc(1,sizeof(SCOPE));
        if (!newStack)
            return ;
        newStack->next = 0;
        newStack->ebx = ebx;
        newStack->esi = esi;
        newStack->edi = edi;
        newStack->regfl  = fl;
        ebx = 0;
        esi = 0;
        edi = 0;
        fl = 0;
        FindFunctionName(newStack->name, eip);
        fl = GetCalleePushedRegs(eip, &ebx, &esi, &edi);
        GetBreakpointLine(eip, &newStack->fileName, &newStack->lineno, stackbase != NULL);
        newStack->address = eip;
        newStack->basePtr = ebp;
        if (fl & (1 << CV_REG_EBX))
        {
            int len = 0;
            ReadProcessMemory(activeProcess->hProcess, (LPVOID)(ebp + ebx), (LPVOID) &ebx,
                4, &len);
        }
        if (fl & (1 << CV_REG_ESI))
        {
            int len = 0;
            ReadProcessMemory(activeProcess->hProcess, (LPVOID)(ebp + esi), (LPVOID) &esi,
                4, &len);
        }
        if (fl & (1 << CV_REG_EDI))
        {
            int len = 0;
            ReadProcessMemory(activeProcess->hProcess, (LPVOID)(ebp + edi), (LPVOID) &edi,
                4, &len);
        }
        if (stackbase)
            stackptr = stackptr->next = newStack;
        else
            stackbase = stackptr = newStack;
        eip = readStackedData(ebp, &ebp);
        if (!eip)
            break;
    }
    StackList = stackbase;
}

//-------------------------------------------------------------------------

LRESULT CALLBACK StackProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam)
{
    LV_ITEM item;
    LV_COLUMN lvC;    
    RECT r;
    LPNMHDR nmh;
    char module[256];
    SCOPE *sl;
    int i, lines;
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
                SCOPE *x = (SCOPE *)p->item.lParam;
                char addr[256];
                p->item.mask |= LVIF_TEXT | LVIF_DI_SETITEM;
                p->item.mask &= ~LVIF_STATE;
                if (p->item.iSubItem == 2)
                {
                    p->item.pszText = x->name;
                }
                else
                {
                    sprintf(addr,"%8X", x->address);
                    p->item.pszText = addr;
                }
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
                case ID_TBPROCEDURE:
                    if (HIWORD(wParam) == CBN_SELENDOK)
                    {
                        int i = SendMessage(hwndTbProcedure, CB_GETCURSEL, 0 , 0);
                        if (i != CB_ERR)
                        {
                            SendMessage(hwnd, WM_USER, i, 0);
                        }
                    }
                    break;
            }
            break;
        case WM_USER:
        {
            memset(&item, 0, sizeof(item));
            if (curSel != 0)
            {
                item.iItem = curSel;
                item.iSubItem = 0;
                item.mask = LVIF_IMAGE;
                item.iImage = IML_BLANK;
                ListView_SetItem(hwndLV, &item);
            }

            curSel = wParam;
            if (curSel != 0)
            {
                item.iItem = curSel;
                item.mask = LVIF_IMAGE;
                item.iImage = IML_CONTINUATION;
                ListView_SetItem(hwndLV, &item);
            }
            sl = StackList;
            lines = curSel;
            while (sl && lines)
            {
                sl = sl->next;
                lines--;
            }
            if (sl)
            {
                if (GetBreakpointLine(sl->address, module, &lines, curSel != 0))
                {
                    char *p;
                    static DWINFO x;
                    strcpy(x.dwName, sl->fileName);
                    p = strrchr(module, '\\');
                    if (p)
                        strcpy(x.dwTitle, p + 1);
                    else
                        strcpy(x.dwTitle, module);
                    x.dwLineNo = sl->lineno;
                    x.logMRU = FALSE;
					x.newFile = FALSE;
                    SetScope(sl);
                    CreateDrawWindow(&x, TRUE);
                }
            }
        }
            break;
        case WM_CREATE:
            hwndStack = hwnd;
            GetClientRect(hwnd, &r);
            hwndLV = CreateWindowEx(0, WC_LISTVIEW, "", 
                           LVS_REPORT | LVS_SINGLESEL | WS_CHILD | WS_VISIBLE | WS_BORDER,
                           0,0,r.right-r.left, r.bottom - r.top, hwnd, 0, hInstance, 0);
            ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
            StackFont = CreateFontIndirect(&fontdata);
            SendMessage(hwndLV, WM_SETFONT, (WPARAM)StackFont, 0);
            lvC.mask = LVCF_WIDTH | LVCF_SUBITEM ;
            lvC.cx = 20;
            lvC.iSubItem = 0;
            ListView_InsertColumn(hwndLV, 0, &lvC);
            lvC.mask = LVCF_WIDTH | LVCF_SUBITEM | LVCF_TEXT;
            lvC.cx = 80;
            lvC.iSubItem = 1;
            lvC.pszText = "Address";
            ListView_InsertColumn(hwndLV, 1, &lvC);
            lvC.mask = LVCF_WIDTH | LVCF_SUBITEM | LVCF_TEXT;
            lvC.cx = 200;
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
            // fall through
        case WM_RESTACK:
            SetScope(NULL);
            ClearStackArea(hwnd);
            EnableWindow(hwndLV, uState != notDebugging && wParam);
            EnableWindow(hwndTbProcedure, uState != notDebugging && wParam);
            if (uState != notDebugging && wParam)
            {
                int i = 0;
                char buf[256];
                SCOPE *list;
                SetStackArea(hwnd);
                
                list = StackList;
                ListView_DeleteAllItems(hwndLV);
                memset(&item, 0, sizeof(item));
                SendMessage(hwndTbProcedure, CB_RESETCONTENT, 0, 0);
                while (list)
                {

                    item.iItem = i;
                    item.iSubItem = 0;
                    item.mask = LVIF_IMAGE | LVIF_PARAM;
                    if (i == 0)
                    {
                        if (activeThread == stoppedThread)
                            item.iImage = IML_STOPBP;
                        else
                            item.iImage = IML_STOP;
                        if (i == curSel)
                            SetScope(list);
                            
                    }
                    else if (i == curSel)
                    {
                        item.iImage = IML_CONTINUATION;
                        SetScope(list);
                    }
                    else
                    {
                        item.iImage = IML_BLANK;
                    }
                    item.lParam = (LPARAM)list;
                    ListView_InsertItem(hwndLV, &item);
                    
                    item.iSubItem = 1;
                    item.mask = LVIF_PARAM | LVIF_TEXT;
                    item.lParam = (LPARAM)list;
                    item.pszText = "";
                    ListView_InsertItem(hwndLV, &item);
                    i++;
                    sprintf(buf, "%08x %s", list->address, list->name);
                    SendMessage(hwndTbProcedure, CB_ADDSTRING, 0, (LPARAM)buf);
                    list = list->next;
                }
                SendMessage(hwndTbProcedure, CB_SETCURSEL, curSel, 0);
            }
            break;
        case WM_DESTROY:
            ClearStackArea(hwnd);
            hwndStack = 0;
            DeleteObject(StackFont);
            break;
		case WM_SETFOCUS:
			SendMessage(GetParent(hwnd), WM_ACTIVATEME, 0, 0);
			break;
        case WM_KILLFOCUS:
            break;
        case WM_KEYDOWN:
            switch (wParam)
            {
			case 'C':
                if (GetKeyState(VK_CONTROL) &0x80000000)
				{
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

void RegisterStackWindow(void)
{
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_HREDRAW + CS_VREDRAW + CS_DBLCLKS;
    wc.lpfnWndProc = &StackProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = 0; //GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = szStackClassName;
    RegisterClass(&wc);
}

//-------------------------------------------------------------------------

HWND CreateStackWindow(void)
{
    if (hwndStack)
    {
        SendMessage(hwndStack, WM_SETFOCUS, 0, 0);
    }
	else
	{
        HBITMAP bitmap;
        bitmap = LoadBitmap(hInstance, "ID_TAG");
        ChangeBitmapColor(bitmap, 0xc0c0c0, RetrieveSysColor(COLOR_WINDOW));
        tagImageList = ImageList_Create(16, 16, ILC_COLOR24, ILEDIT_IMAGECOUNT, 0);
    	ImageList_Add(tagImageList, bitmap, NULL);
        DeleteObject(bitmap);
		hwndStack = CreateDockableWindow(DID_STACKWND, szStackClassName, szStackTitle, hInstance, 30 * 8, 19 * 8);
	}
    return hwndStack;
}
