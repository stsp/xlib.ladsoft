/* 
CCIDE
Copyright 2001-2011 David Lindauer.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
strcpy*
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
#include "helpid.h"
#include "header.h"
#include "codecomp.h"
#include "regexp.h"
#include <ctype.h>


#define EDITOR_OFFSET 35
#define LINENO_DIGITS 7

extern PROJECTITEM *activeProject;
extern HWND hwndFindInternal;
extern HWND hwndSrcTab;
extern LOGFONT EditFont;
extern HINSTANCE hInstance;
extern HWND hwndClient, hwndStatus, hwndFrame, hwndASM;
extern HANDLE hMenuMain;
extern char szSourceFilter[];
extern char szNewFileFilter[];
extern enum DebugState uState;
extern char highlightText[256] ;
extern int highlightCaseSensitive;
extern int highlightWholeWord;
extern int finding;
extern DWORD maximizeClient;
extern SCOPE *activeScope;
extern SCOPE *StackList;
extern THREAD *activeThread, *stoppedThread;
POINT rightclickPos;
HANDLE editHeap;
char szDrawClassName[] = "xccDrawClass";
char szUntitled[] = "New File";

DWINFO *newInfo;
HANDLE codeCompSem;
CRITICAL_SECTION ewMutex;

DWINFO *editWindows;

HIMAGELIST tagImageList;

static WNDPROC oldLVProc;
static DWORD ccThreadId;
static HANDLE ccThreadExit;
static BOOL stopCCThread;
void recolorize(DWINFO *ptr);
void AsyncOpenFile(DWINFO *newInfo);

void SetTitle(HWND hwnd);
void EditorRundown(void)
{
}
int xstricmpz(char *str1, char *str2)
{
    while (*str2)
        if (toupper(*str1++) != toupper(*str2++))
            return 1;
    return  *str1 !=  *str2;
} 
int xstricmp(char *str1, char *str2)
{
    while (*str1 && *str2)
    {
        if (toupper(*str1) != toupper(*str2))
            break;
        str1++, str2++;
    }
    if (toupper(*str1) != toupper(*str2))
        return toupper(*str1) < toupper(*str2) ? -1 : 1;
    return 0;
}

//-------------------------------------------------------------------------

char *stristr(char *str1, char *str2)
{
    int l = strlen(str2);
    while (*str1)
    {
        char *str3 = str1, *str4 = str2;
        while (*str3 && *str4)
        {
            if (toupper(*str3) != toupper(*str4))
                break;
            str3++, str4++;
        }
        if (!*str4)
            return str1;
        str1++;
    }
    return 0;
}
void ResetEditTitles(void )
{
    DWINFO *ptr = editWindows;
    while (ptr)
    {
		SetTitle(ptr->self);
        ptr = ptr->next;
	}
}

void rehighlight(char *text, int whole, int casesensitive)
{
    DWINFO *ptr = editWindows;
	strcpy(highlightText, text);
	highlightWholeWord = whole;
	highlightCaseSensitive = casesensitive;
    while (ptr)
    {
        recolorize(ptr);
        ptr = ptr->next;
	}
}
//-------------------------------------------------------------------------

void ApplyEditorSettings(void)
{
    DWINFO *ptr = editWindows;
	LOGFONT lf;
	int tabs = PropGetInt(NULL, "TAB_INDENT") * 4;
	memcpy(&lf, &EditFont, sizeof(lf));
	PropGetFont(NULL, "FONT", &lf);	
	LoadColors();
    while (ptr)
    {
        HFONT fnt = CreateFontIndirect(&lf);
        PostMessage(ptr->dwHandle, WM_SETFONT, (WPARAM)
            fnt, 1);
		PostMessage(ptr->dwHandle, EM_SETTABSTOPS, 0, tabs);
        PostMessage(ptr->dwHandle, WM_SETEDITORSETTINGS,
            0, 0);
        PostMessage(ptr->self, WM_SETLINENUMBERMODE, 0, 0);
        ptr = ptr->next;
    }
}

//-------------------------------------------------------------------------

void InvalidateByName(char *name)
{
    DWINFO *ptr = editWindows;
    DWINFO info;
    strcpy(info.dwName, name);
    while (ptr)
    {
        if (SendMessage(ptr->self, WM_COMMAND, ID_QUERYHASFILE, (LPARAM)
            &info))
            InvalidateRect(ptr->self, 0, 0);
        ptr = ptr->next;
    }
}
DWINFO *GetFileInfo(char *name)
{
    DWINFO *ptr = editWindows;
    DWINFO info;
    strcpy(info.dwName, name);
    while (ptr)
    {
        if (SendMessage(ptr->self, WM_COMMAND, ID_QUERYHASFILE, (LPARAM)
            &info))
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
}
void EditRenameFile(char *oldName, char *newName)
{
    DWINFO *ptr = editWindows;
    DWINFO info;
    strcpy(info.dwName, oldName);
    while (ptr)
    {
        if (SendMessage(ptr->self, WM_COMMAND, ID_QUERYHASFILE, (LPARAM)
            &info))
		{
			char *p;
			strcpy(ptr->dwName, newName);
			p=strrchr(newName, '\\');
			if (p)
				p++;
			else 
				p = newName;
			strcpy(ptr->dwTitle, p);
			SendMessage(hwndSrcTab, TABM_RENAME, (WPARAM)newName, (LPARAM)oldName);
			SetTitle(ptr->self);
		}
        ptr = ptr->next;
    }
}

//-------------------------------------------------------------------------

int ApplyBreakAddress(char *module, int linenum)
{
    char nmodule[260];
    int i;
    nmodule[0] = 0;
    TagBreakpoint(module, linenum);
    if (linenum)
    {
        char *p;
        static DWINFO x;
		// there was a call to FindModuleName
        if (module[0])
        {
            strcpy(x.dwName, module);
            p = strrchr(module, '\\');
            if (p)
                strcpy(x.dwTitle, p + 1);
            else
                strcpy(x.dwTitle, module);
            x.dwLineNo = BPLine(x.dwName);
            x.logMRU = TRUE;
            x.newFile = FALSE ;
    		CreateDrawWindow(&x, TRUE);
        }
    }
}

//-------------------------------------------------------------------------

static int FileAttributes(char *name)
{
    int rv = GetFileAttributes(name);
    if (rv ==  - 1)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
            return 0;
        return  - 1; // any other error, it is read only file
    }
    else
        return rv;
}

LRESULT CALLBACK lvProc(HWND hwnd, UINT iMessage, WPARAM wParam,
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
//-------------------------------------------------------------------------

static int CreateFileSaveData(HWND hwnd, int changed)
{
    DWINFO *ptr = editWindows;
    int items = 0;
    LV_ITEM item;
    RECT r;
    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
    LV_COLUMN lvC;
    ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

    GetWindowRect(hwndLV, &r);
    lvC.mask = LVCF_WIDTH | LVCF_SUBITEM ;
    lvC.cx = 20;
    lvC.iSubItem = 0;
    ListView_InsertColumn(hwndLV, 0, &lvC);
    lvC.mask = LVCF_WIDTH | LVCF_SUBITEM;
    lvC.cx = 32;
    lvC.iSubItem = 1;
    ListView_InsertColumn(hwndLV, 1, &lvC);
    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
    lvC.fmt = LVCFMT_LEFT;
    lvC.cx = r.right - r.left - 56;
    lvC.iSubItem = 2;
    ListView_InsertColumn(hwndLV, 2, &lvC);


    while (ptr)
    {
        int rv;
        FILETIME time;
        if (changed)
        {
            int a = FileAttributes(ptr->dwName);
            rv = FALSE;
            if (a ==  - 1)
                a = 0;
            if (FileTime(&time, ptr->dwName))
            {
                rv = (time.dwHighDateTime != ptr->time.dwHighDateTime ||
                    time.dwLowDateTime != ptr->time.dwLowDateTime);
                ptr->time = time;
            }
            if (a &FILE_ATTRIBUTE_READONLY)
                SendMessage(ptr->dwHandle, EM_SETREADONLY, 1, 0);
            else
                SendMessage(ptr->dwHandle, EM_SETREADONLY, 0, 0);
        }
        else
            rv = SendMessage(ptr->dwHandle, EM_GETMODIFY, 0, 0);
        if (rv)
        {
            int v;
            item.iItem = items++;
            item.iSubItem = 0;
            item.mask = LVIF_PARAM;
            item.lParam = (LPARAM)ptr;
            item.pszText = ""; // LPSTR_TEXTCALLBACK ;
            v = ListView_InsertItem(hwndLV, &item);
            ListView_SetCheckState(hwndLV, v, TRUE);
        }
        ptr = ptr->next;
    }
    if (items)
    {
        ListView_SetSelectionMark(hwndLV, 0);
        ListView_SetItemState(hwndLV, 0, LVIS_SELECTED, LVIS_SELECTED);
    }

	oldLVProc = (WNDPROC)GetWindowLong(hwndLV, GWL_WNDPROC);
	SetWindowLong(hwndLV, GWL_WNDPROC, (long)lvProc);
	SendMessage(hwndLV, WM_USER + 10000, 0, (LPARAM)hwnd);
    return items;
}

static void SetOKText(HWND hwnd, char *text)
{
    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
	HWND okbut = GetDlgItem(hwnd, IDOK);
    int i;
    for (i = 0;; i++)
    {
	    LV_ITEM item;
        item.iItem = i;
        item.iSubItem = 0;
        item.mask = LVIF_PARAM;
        if (!ListView_GetItem(hwndLV, &item))
            break;
        if (ListView_GetCheckState(hwndLV, i))
		{
			SetWindowText(okbut, text);
			return;
		}
    }
	SetWindowText(okbut, "Ok");
}
//-------------------------------------------------------------------------

static void ParseFileSaveData(HWND hwnd, BOOL changed)
{
    LV_ITEM item;
    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
    int i;
    for (i = 0;; i++)
    {
        DWINFO *ptr;
        item.iItem = i;
        item.iSubItem = 0;
        item.mask = LVIF_PARAM;
        if (!ListView_GetItem(hwndLV, &item))
            break;
        ptr = (DWINFO*)item.lParam;
        if (changed)
        {
            if (ListView_GetCheckState(hwndLV, i))
			{						
                LoadFile(ptr->self, ptr, TRUE);
			}
        }
        else
            if (ListView_GetCheckState(hwndLV, i))
                SendMessage(ptr->self, WM_COMMAND, IDM_SAVE,
                    0);
            else
            {
                TagLinesAdjust(ptr->dwName, TAGM_DISCARDFILE);
            }

    }
}

static int CustomDraw(HWND hwnd, LPNMLVCUSTOMDRAW draw)
{
    switch(draw->nmcd.dwDrawStage)
	{
	    case CDDS_PREPAINT :
		case CDDS_ITEMPREPAINT:
	        return CDRF_NOTIFYSUBITEMDRAW;
	    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
			if (draw->nmcd.uItemState & (CDIS_SELECTED ))
			{
				draw->clrText = RetrieveSysColor(COLOR_HIGHLIGHTTEXT);
				draw->clrTextBk = RetrieveSysColor(COLOR_HIGHLIGHT);
			}
			else
			{
				draw->clrText = RetrieveSysColor(COLOR_WINDOWTEXT);
				draw->clrTextBk = RetrieveSysColor(COLOR_WINDOW);
			}
			return CDRF_NEWFONT;
		default:
			return CDRF_DODEFAULT;
	}
}
//-------------------------------------------------------------------------

long APIENTRY FileSaveProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM
    lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
           if (!CreateFileSaveData(hwnd, FALSE))
		   {
                EndDialog(hwnd, 1);
		   }
            else
                CenterWindow(hwnd);
            return 1;
        case WM_NOTIFY:
			if (((LPNMHDR)lParam)->code == NM_CUSTOMDRAW)
			{
				SetWindowLong(hwnd, DWL_MSGRESULT, CustomDraw(hwnd, (LPNMLVCUSTOMDRAW)lParam));
				return TRUE;
			}
            if (wParam == IDC_FILELIST)
            {
                if (((LPNMHDR)lParam)->code == LVN_GETDISPINFO)
                {
                    LV_DISPINFO *plvdi = (LV_DISPINFO*)lParam;
                    DWINFO *ptr;
                    plvdi->item.mask |= LVIF_TEXT | LVIF_DI_SETITEM;
                    plvdi->item.mask &= ~LVIF_STATE;
                    switch (plvdi->item.iSubItem)
                    {
                    case 2:
                        ptr = (DWINFO*)plvdi->item.lParam;
                        plvdi->item.pszText = ptr->dwTitle;
                        break;
                    default:
                        plvdi->item.pszText = "";
                        break;
                    }
                }
				else if (((LPNMHDR)lParam)->code == LVN_ITEMCHANGED)
                {
					SetOKText(hwnd, "Save");
				}
            }
            return 0;
		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_INSERT:
					if (GetKeyState(VK_CONTROL) & 0x80000000)
					{
					    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
						ListView_SetCheckState(hwndLV, -1, TRUE);
                		SetOKText(hwnd, "Save");
					}
					else
					{
					    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
						int i = ListView_GetSelectionMark(hwndLV);
						ListView_SetCheckState(hwndLV, i, TRUE);
					}
					break;
				case VK_DELETE:
					if (GetKeyState(VK_CONTROL) & 0x80000000)
					{
					    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
						ListView_SetCheckState(hwndLV, -1, FALSE);
                		SetOKText(hwnd, "Save");
					}
					else
					{
					    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
						int i = ListView_GetSelectionMark(hwndLV);
						ListView_SetCheckState(hwndLV, i, FALSE);
					}
					break;
			}
			break;
        case WM_COMMAND:
            switch (wParam &0xffff)
            {
            case IDOK:
                ParseFileSaveData(hwnd, FALSE);
                EndDialog(hwnd, IDOK);
                break;
            case IDCANCEL:
                EndDialog(hwnd, IDCANCEL);
                break;
            case IDC_SELECTALL:
				{
				    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
					ListView_SetCheckState(hwndLV, -1, TRUE);
            		SetOKText(hwnd, "Save");
				}
                break;
            case IDC_DESELECTALL:
				{
				    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
					ListView_SetCheckState(hwndLV, -1, FALSE);
            		SetOKText(hwnd, "Save");
				}
                break;
            case IDHELP:
                ContextHelp(IDH_SAVE_FILE_DIALOG);
                break;
            }
            break;
        case WM_CLOSE:
            PostMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
            break;
    }
    return 0;
}

//-------------------------------------------------------------------------

long APIENTRY FileChangeProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM
    lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            if (!CreateFileSaveData(hwnd, TRUE))
                EndDialog(hwnd, 1);
            else
                CenterWindow(hwnd);
            return 1;
        case WM_NOTIFY:
			if (((LPNMHDR)lParam)->code == NM_CUSTOMDRAW)
			{
				SetWindowLong(hwnd, DWL_MSGRESULT, CustomDraw(hwnd, (LPNMLVCUSTOMDRAW)lParam));
				return TRUE;
			}
            if (wParam == IDC_FILELIST)
            {
                NMITEMACTIVATE *ia;
                LV_ITEM item;
                int state;
                if (((LPNMHDR)lParam)->code == LVN_GETDISPINFO)
                {
                    LV_DISPINFO *plvdi = (LV_DISPINFO*)lParam;
                    DWINFO *ptr;
                    plvdi->item.mask |= LVIF_TEXT | LVIF_DI_SETITEM;
                    plvdi->item.mask &= ~LVIF_IMAGE;
                    switch (plvdi->item.iSubItem)
                    {
                    case 2:
                        ptr = (DWINFO*)plvdi->item.lParam;
                        plvdi->item.pszText = ptr->dwTitle;
                        break;
                    default:
                        plvdi->item.pszText = "";
                        break;
                    }
                }
				else if (((LPNMHDR)lParam)->code == LVN_ITEMCHANGED)
                {            
					SetOKText(hwnd, "Reload");
				}
			}
            break;
		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_INSERT:
					if (GetKeyState(VK_CONTROL) & 0x80000000)
					{
    				    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
	    				ListView_SetCheckState(hwndLV, -1, TRUE);
                		SetOKText(hwnd, "Reload");
					}
					else
					{
					    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
						int i = ListView_GetSelectionMark(hwndLV);
						ListView_SetCheckState(hwndLV, i, TRUE);
					}
					break;
				case VK_DELETE:
					if (GetKeyState(VK_CONTROL) & 0x80000000)
					{
    				    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
	    				ListView_SetCheckState(hwndLV, -1, FALSE);
                		SetOKText(hwnd, "Reload");
					}
					else
					{
					    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
						int i = ListView_GetSelectionMark(hwndLV);
						ListView_SetCheckState(hwndLV, i, FALSE);
					}
					break;
			}
			break;
        case WM_COMMAND:
            switch (wParam &0xffff)
            {
            case IDOK:
                ParseFileSaveData(hwnd, TRUE);
                EndDialog(hwnd, IDOK);
                break;
            case IDCANCEL:
                EndDialog(hwnd, IDCANCEL);
                break;
            case IDC_SELECTALL:
				{
				    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
    				ListView_SetCheckState(hwndLV, -1, TRUE);
            		SetOKText(hwnd, "Reload");
				}
                break;
            case IDC_DESELECTALL:
				{
				    HWND hwndLV = GetDlgItem(hwnd, IDC_FILELIST);
    				ListView_SetCheckState(hwndLV, -1, FALSE);
            		SetOKText(hwnd, "Reload");
				}
                break;
            case IDHELP:
                ContextHelp(IDH_RELOAD_FILE_DIALOG);
                break;
            }
            break;
        case WM_CLOSE:
            PostMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
            break;
    }
    return 0;
}

//-------------------------------------------------------------------------

void PASCAL CheckEditWindowChangedThread(void *aa)
{
    static int sem;
    if (!sem)
    {
        sem++;
        DialogBox(hInstance, "DLG_FILECHANGE", hwndFrame, (DLGPROC)FileChangeProc);
        sem--;
    }
}

//-------------------------------------------------------------------------

void CheckEditWindowChanged(void)
{
    DWORD threadhand;
    CloseHandle(CreateThread(0,0,(LPTHREAD_START_ROUTINE)CheckEditWindowChangedThread,
							 (LPVOID)NULL,0,&threadhand)) ;
}

//-------------------------------------------------------------------------

int QuerySaveAll(void)
{
    return DialogBox(hInstance, "DLG_FILESAVE", hwndFrame, (DLGPROC)FileSaveProc);
}

//-------------------------------------------------------------------------

void SaveDrawAll(void)
{
    DWINFO *ptr = editWindows;
    while (ptr)
    {
        if (SendMessage(ptr->dwHandle, EM_GETMODIFY, 0, 0))
            SendMessage(ptr->self, WM_COMMAND, IDM_SAVE, 0);
        ptr = ptr->next;
    }
}

//-------------------------------------------------------------------------

int AnyModified(void)
{
    DWINFO *ptr = editWindows;
    int rv = 0;
    while (ptr)
    {
        rv |= SendMessage(ptr->dwHandle, EM_GETMODIFY, 0, 0);
        ptr = ptr->next;
    }
    return rv;
}

//-------------------------------------------------------------------------

void CloseAll(void)
{
    
    DWINFO *ptr, *old;
	MSG msg;
	EnterCriticalSection(&ewMutex);
    ptr = old = editWindows;
    editWindows = NULL;
	LeaveCriticalSection(&ewMutex);
    while (ptr)
    {
        ShowWindow(ptr->self, SW_HIDE);
        ptr = ptr->next;
    }
    ptr = old;
    while (ptr)
    {
        PostMessage(ptr->self, WM_CLOSE, 0, 0);
        ptr = ptr->next;
    }
	while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		ProcessMessage(&msg);
}

//-------------------------------------------------------------------------

void RedrawAllBreakpoints(void)
{
    DWINFO *ptr = editWindows;
    while (ptr)
    {
        InvalidateRect(ptr->self, 0, 0);
        ptr = ptr->next;
    }
}

//-------------------------------------------------------------------------

char *GetEditData(HWND hwnd)
{
    int l;
    char *buf;
    l = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
//	buf = HeapAlloc(editHeap, HEAP_ZERO_MEMORY, l + 1);
	buf = calloc(l+1, 1);
    if (!buf)
    {
        return 0;
    }
    SendMessage(hwnd, WM_GETTEXT, l+1, (LPARAM)buf);
	
    return buf;
}

void FreeEditData(char *buf)
{
	free(buf);
//	HeapFree(editHeap, 0, buf);
//	HeapCompact(editHeap, 0);
}
//-------------------------------------------------------------------------

int SetEditData(HWND hwnd, char *buf, BOOL savepos)
{
    SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), WM_SETTEXT, savepos, (LPARAM)buf);
    FreeEditData(buf);
    return TRUE;
}

//-------------------------------------------------------------------------

void backup(char *name)
{
    char newname[256], buffer[512];
    char *s;
    HANDLE in, out;
    int size;
    BY_HANDLE_FILE_INFORMATION info;
    strcpy(newname, name);
    s = strrchr(newname, '.');
    strcat(newname, ".bak");

    in = CreateFile(name, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
    if (in == INVALID_HANDLE_VALUE)
        return ;
    if (!GetFileInformationByHandle(in, &info))
    {
        CloseHandle(in);
        return ;
    }
    out = CreateFile(newname, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (out == INVALID_HANDLE_VALUE)
    {
        ExtendedMessageBox("File Error", MB_SETFOREGROUND | MB_SYSTEMMODAL, 
            "Backup file is not writeable");
        CloseHandle(in);
        return ;
    }
    while (1)
    {
        DWORD read, written;
        if (!ReadFile(in, buffer, 512, &read, 0) || !read)
            break;
        WriteFile(out, buffer, read, &written, 0);
    }
    SetFileTime(out, &info.ftCreationTime, &info.ftLastAccessTime,
        &info.ftLastWriteTime);
    CloseHandle(out);
    CloseHandle(in);
}

//-------------------------------------------------------------------------

int SaveFile(HWND hwnd, DWINFO *info)
{
    char *buf = GetEditData(GetDlgItem(hwnd, ID_EDITCHILD));
    FILE *out;
    int l, i;

    if (PropGetBool(NULL, "BACKUP_FILES"))
        backup(info->dwName);
    if (!buf)
        return FALSE;
    if (info->dosStyle)
        out = fopen(info->dwName, "w");
    else
        out = fopen(info->dwName, "wb");
    if (!out)
    {
        ExtendedMessageBox("File Error", MB_SETFOREGROUND | MB_SYSTEMMODAL, 
            "Output file is not writeable");
        free(buf);
        return FALSE;
    }
    fputs(buf, out);
    fclose(out);
    FreeEditData(buf);
    FileTime(&info->time, info->dwName);
    return TRUE;
}

//-------------------------------------------------------------------------

int LoadFile(HWND hwnd, DWINFO *info, BOOL savepos)
{
    long size;
    char *buf,*p,*q;
    FILE *in = fopen(info->dwName, "rb");
    info->dosStyle = FALSE;
    if (!in)
    {
        info->dosStyle = TRUE;
        ShowWindow(info->dwHandle, SW_SHOW);
        return FALSE;
    }
    fseek(in, 0L, SEEK_END);
    size = ftell(in);
	buf = calloc(size+1, 1);
    if (!buf)
    {
        info->dosStyle = TRUE;
        fclose(in);
        ShowWindow(info->dwHandle, SW_SHOW);
        return FALSE;
    }
    fseek(in, 0L, SEEK_SET);
    size = fread(buf, 1, size, in);
    if (size < 0)
        size = 0;
    buf[size] = 0;
    fclose(in);
    p = q = buf;
    while (*p)
    {
        if (*p != '\r')
        {
            *q++ = *p++;
        }
        else
        {
            info->dosStyle = TRUE;
            p++;
        }
    }
    *q = 0;
    SetEditData(hwnd, buf, savepos);
    SendMessage(info->dwHandle, EM_SETMODIFY, 0, 0);
    recolorize(info);
    if (FileAttributes(info->dwName) &FILE_ATTRIBUTE_READONLY)
        SendMessage(info->dwHandle, EM_SETREADONLY, 1, 0);
    FileTime(&info->time, info->dwName);
    return TRUE;

}

//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
void SetTitle(HWND hwnd)
{
	DWINFO *info = (DWINFO *)GetWindowLong(hwnd, 0);
	char buf[MAX_PATH];
	EDITDATA *dt = (EDITDATA *)SendMessage(info->dwHandle, EM_GETEDITDATA, 0, 0);
    int mod = SendMessage(info->dwHandle, EM_GETMODIFY, 0, 0);
    strcpy(buf, info->dwName);
    if (buf[0] == 0)
        strcpy(buf, info->dwTitle);
    if (activeProject)
    {
        strcpy(buf, relpath(buf, activeProject->realName));
    }
    else
    {
        char dir[MAX_PATH];
        dir[0] = 0;
        GetCurrentDirectory(MAX_PATH, dir);
        strcat(dir,"\\hithere");
        strcpy(buf, relpath(buf, dir));
    }
	if (dt->id)
		sprintf(buf + strlen(buf), " (%d)", dt->id + 1);
    if (mod)
        strcat(buf, " *");
    SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)buf);
	SendMessage(hwndSrcTab, TABM_SETMODIFY, mod, (LPARAM)hwnd);
}
void drawParams(DWINFO *info, HWND hwnd)
{
    char buf[512];
    int start, ins, col, sel;
    int readonly = SendMessage(info->dwHandle, EM_GETREADONLY, 0, 0);
    int mod = SendMessage(info->dwHandle, EM_GETMODIFY, 0, 0);
    int maxLines = SendMessage(info->dwHandle, EM_GETLINECOUNT, 0, 0) + 1; 
    int textSize = SendMessage(info->dwHandle, EM_GETSIZE, 0, 0);
	EDITDATA *dt ;
    CHARRANGE a;
    SendMessage(info->dwHandle, EM_GETSEL, (WPARAM) &sel, 0);
    SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_EXGETSEL, 0, (LPARAM) &a);
    sel = a.cpMin;
    start = SendMessage(info->dwHandle, EM_EXLINEFROMCHAR, 0, sel);
    ins = SendMessage(info->dwHandle, EM_GETINSERTSTATUS, 0, 0);
    col = SendMessage(info->dwHandle, EM_GETCOLUMN, 0, 0);
    sprintf(buf, "Size: %d", textSize);
    SendMessage(hwndStatus, SB_SETTEXT, 1 | SBT_NOBORDERS, (LPARAM)buf);
    sprintf(buf, "Lines: %d", maxLines);
    SendMessage(hwndStatus, SB_SETTEXT, 2 | SBT_NOBORDERS, (LPARAM)buf);
    sprintf(buf, "Line: %d", start + 1);
    SendMessage(hwndStatus, SB_SETTEXT, 3 | SBT_NOBORDERS, (LPARAM)buf);
    sprintf(buf, "Col: %d", col + 1);
    SendMessage(hwndStatus, SB_SETTEXT, 4 | SBT_NOBORDERS, (LPARAM)buf);
    SendMessage(hwndStatus, SB_SETTEXT, 5 | SBT_NOBORDERS, (LPARAM)(ins ? "INS" : "OVR"));
    if (readonly)
        SendMessage(hwndStatus, SB_SETTEXT, 6 | SBT_NOBORDERS, (LPARAM)("READ-ONLY"));
    else
        SendMessage(hwndStatus, SB_SETTEXT, 6 | SBT_NOBORDERS, (LPARAM)(mod ? "MODIFIED" : 
            "    "));

	SetTitle(hwnd);
    if (info->jumplistLineno != start)
    {
        info->jumplistLineno = start;
        SetJumplistPos(hwnd, start);
    }
}

//-------------------------------------------------------------------------

void eraseParams(HWND hwnd)
{
    SendMessage(hwndStatus, SB_SETTEXT, 1 | SBT_NOBORDERS, (LPARAM)"    ");
    SendMessage(hwndStatus, SB_SETTEXT, 2 | SBT_NOBORDERS, (LPARAM)"    ");
    SendMessage(hwndStatus, SB_SETTEXT, 3 | SBT_NOBORDERS, (LPARAM)"    ");
    SendMessage(hwndStatus, SB_SETTEXT, 4 | SBT_NOBORDERS, (LPARAM)"    ");

}
static int GetLog10(int val)
{
    int count = 1;
    if (val > 9999999)
        count = 8;
    else if (val > 999999)
        count = 7;
    else if (val > 99999)
        count = 6;
    else if (val > 9999)
        count = 5;
    else if (val > 999)
        count = 4;
    else if (val > 99)
        count = 3;
    else if (val > 9)
        count = 2;
    return count;
}
//-------------------------------------------------------------------------

int PaintBreakpoints(HWND hwnd, HDC dc, PAINTSTRUCT *paint, RECT *rcl)
{
    int count;
    HBRUSH graybrush, graybrush1;
    RECT r, r1;
    int i;
    DWINFO *ptr = (DWINFO*)GetWindowLong(hwnd, 0);
    int linenum = SendMessage(ptr->dwHandle, EM_GETFIRSTVISIBLELINE, 0, 0) + 1;
    int chpos1 = SendMessage(ptr->dwHandle, EM_LINEINDEX, linenum, 0);
    int maxLines = SendMessage(ptr->dwHandle, EM_GETLINECOUNT, 0, 0); 
    int ypos;
    int lines, offset = 0;
    int height;
    short *lt;
    int lc;
    int oldbk = SetBkColor(dc, RetrieveSysColor(COLOR_BTNFACE));
    int oldfg = SetTextColor(dc, RetrieveSysColor(COLOR_BTNTEXT));
    HPEN linePen;
    HFONT xfont;
    POINTL pt;
    int bpline = BPLine(ptr->dwName);
    SendMessage(ptr->dwHandle, EM_POSFROMCHAR, (WPARAM) &pt, chpos1);
    ypos = pt.y;

    SendMessage(ptr->dwHandle, EM_GETRECT, 0, (LPARAM) &r1);
    lines = r1.bottom / (height = SendMessage(ptr->dwHandle, EM_GETTEXTHEIGHT,
        0, 0));

    count = GetLog10(linenum + lines);
    if (count > ptr->lineNumberDigits)
    {
        PostMessage(hwnd, WM_SETLINENUMBERMODE, count, 0);
        // will invalidate the rect and force it to repaint
        return 0;
    }

    if (ypos < height)
        offset = ypos - height;

    graybrush = CreateSolidBrush(RetrieveSysColor(COLOR_BTNFACE));
    FillRect(dc, rcl, graybrush);
    DeleteObject(graybrush);
	linePen = CreatePen(PS_SOLID, 0, RetrieveSysColor(COLOR_BTNSHADOW));
    linePen = SelectObject(dc, linePen);
    MoveToEx(dc, ptr->editorOffset - 1, 0, 0);
    LineTo(dc, ptr->editorOffset - 1, rcl->bottom);
    linePen = SelectObject(dc, linePen);
    DeleteObject(linePen);
    if (uState != notDebugging)
    {
        lt = GetLineTable(ptr->dwName, &lc);
        if (lt)
        {
            int j = 0;
            graybrush1 = RetrieveSysBrush(COLOR_BTNSHADOW);
            r.left = ptr->editorOffset - 5;
            r.right = ptr->editorOffset - 1;
            for (i = linenum; i <= linenum + lines; i++)
            {
                int oldline = TagOldLine(ptr->dwName, i);
                int k;
                while (lt[j] < oldline)
                    j++;
                for (k=j; k < lc; k++)
                    if (lt[k] == oldline)
                    {
                        r.top = offset + (i - linenum) *height;
                        r.bottom = r.top + height;
                        FillRect(dc, &r, graybrush1);
                        break;
                    }
            }
            DeleteObject(graybrush1);
        }
    }
    xfont = (HFONT)SendMessage(ptr->dwHandle, WM_GETFONT, 0, 0);
    xfont = SelectObject(dc, xfont);
   for (i = linenum; i <= linenum + lines; i++)
    {
        int obj = -1;
        int type = IsTagged(ptr->dwName, i);
        switch (type)
        {
            case TAG_BP:
                if (bpline == i)
                {
                    obj = IML_STOPBP;
                }
                else
                {
                    obj = IML_BP;
                }
                break;
            case TAG_FIF1:
                obj = IML_FIF1;
                break;
            case TAG_FIF2:
                obj = IML_FIF2;
                break;
            case TAG_BOOKMARK:
                obj = IML_BOOKMARK;
                break;
            case TAG_BPGRAYED:
                obj = IML_BPGRAYED;
                break;
            case -1:            
                if (bpline == i)
                    obj = IML_STOP;
				else if (activeScope && !stricmp(activeScope->fileName, ptr->dwName) && i == activeScope->lineno)
                    if (StackList == activeScope)
                        if (activeThread == stoppedThread)
                            obj = IML_STOPBP;
                        else
                            obj = IML_STOP;
                    else
    					obj = IML_CONTINUATION;
                break;
        }
        if (obj != -1)
        {
            ImageList_Draw(tagImageList, obj, dc, 12, (i - linenum) *height + offset, ILD_NORMAL);
        }
        if (ptr->editorOffset != EDITOR_OFFSET && i <= maxLines+1)
        {
            char buf[256];
            sprintf(buf, "%d", i);
            TextOut(dc, EDITOR_OFFSET - 4 + (ptr->lineNumberDigits - GetLog10(i)) * 
                    ((EDITDATA *)SendMessage(ptr->dwHandle, EM_GETEDITDATA, 0, 0))->cd->txtFontWidth , 
                    (i - linenum) *height + offset, buf, strlen(buf));
        }
    }
    xfont = SelectObject(dc, xfont);
    SetTextColor(dc, oldfg);
    SetBkColor(dc, oldbk);
    return 0;
}

//-------------------------------------------------------------------------

LRESULT CALLBACK gotoProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam)
{
    char buf[3];
    switch (iMessage)
    {
        case WM_COMMAND:
            if (wParam == IDOK)
            {
                int i = GetEditFieldValue(hwnd, IDC_GOTO);
                EndDialog(hwnd, i);
                break;
            }
            if (HIWORD(wParam) == EN_CHANGE)
            {
                DisableControl(hwnd, IDOK, !GetWindowText((HWND)lParam, buf, 2))
                    ;
                break;
            }
            if (wParam != IDCANCEL)
                break;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            break;
        case WM_INITDIALOG:
            CenterWindow(hwnd);
            SetEditField(hwnd, IDC_GOTO, "");
            DisableControl(hwnd, IDOK, 1);
			return 1;
            break;
    }
    return 0;
}

//-------------------------------------------------------------------------

void recolorize(DWINFO *ptr)
{
    int language = LANGUAGE_NONE;
    if (stristr(ptr->dwName, ".c") == ptr->dwName + strlen(ptr->dwName) - 2 ||
        stristr(ptr->dwName, ".cpp") == ptr->dwName + strlen(ptr->dwName) - 4 
        || stristr(ptr->dwName, ".h") == ptr->dwName + strlen(ptr->dwName) - 2)
        language = LANGUAGE_C;
    else if (stristr(ptr->dwName, ".asm") == ptr->dwName + strlen(ptr->dwName) 
        - 4 || stristr(ptr->dwName, ".asi") == ptr->dwName + strlen(ptr->dwName)
        - 4 || stristr(ptr->dwName, ".inc") == ptr->dwName + strlen(ptr->dwName)
        - 4 || stristr(ptr->dwName, ".nas") == ptr->dwName + strlen(ptr->dwName)
        - 4)
        language = LANGUAGE_ASM;
    else if (stristr(ptr->dwName, ".rc") == ptr->dwName + strlen(ptr->dwName) - 3)
        language = LANGUAGE_RC;
    SendMessage(ptr->dwHandle, EM_LANGUAGE, 0, language);
}

//-------------------------------------------------------------------------

void asyncLoadFile(DWINFO *ptr)
{
    recolorize(ptr);
    if (ptr->dwName[0])
        LoadFile(ptr->self, ptr, FALSE);
    else
        ShowWindow(ptr->dwHandle, SW_SHOW);
    if (ptr->dwLineNo !=  - 1)
    {
        PostMessage(ptr->self, WM_COMMAND, IDM_SETLINE, ptr->dwLineNo);
    }
}
void MsgWait(HANDLE event)
{
	while (1)
	{
		switch (MsgWaitForMultipleObjects(1, &event, FALSE, INFINITE, QS_ALLINPUT))
		{
			case WAIT_OBJECT_0:
			{
				return;
			}
			case WAIT_OBJECT_0 + 1:
			{
				MSG msg;
				GetMessage(&msg,NULL,0,0);
				ProcessMessage(&msg);
			}
		}
	}
}
static void installparse(char *name, BOOL remove)
{
	char *p = (char *)calloc(1, strlen(name) + 1);
	if (p)
	{
		strcpy(p, name);
		PostThreadMessage(ccThreadId, WM_USER, remove, (LPARAM)p);
	}
}
void InstallForParse(HWND hwnd)
{
	if (PropGetBool(NULL, "CODE_COMPLETION"))
	{
		int i;
		DWINFO *info = (DWINFO *)GetWindowLong(hwnd, 0);
		char *name = info->dwName;
		int len = strlen(name);
		if (name[len-2] == '.')
		{
			if (tolower(name[len - 1]) == 'c')
				installparse(name, FALSE);
			else if (tolower(name[len-1]) == 'h')
            {
                DWINFO *ptr = editWindows;
                while (ptr)
                {
					if (IsWindow(ptr->self))
					{
						name = ptr->dwName;
						len = strlen(name);
						if (name[len -2] == '.' && tolower(name[len-1]) == 'c')
							installparse(name, FALSE);
					}
                    ptr = ptr->next;
				}
            }
		}
	}
}
static void deleteParseData(char *name)
{
	int len = strlen(name);
	if (name[len-2] == '.')
	{
		if (tolower(name[len - 1]) == 'c')
		{
			installparse(name, TRUE);
		}
	}
}
void ScanParse(void)
{
	while (1)
	{
		MSG msg;
		GetMessage(&msg, NULL, 0, 0);
		if (msg.message == WM_USER)
		{
			WaitForSingleObject(codeCompSem, INFINITE);
			if (msg.wParam)
				deleteFileData((char *)msg.lParam);
			else
				doparse((char *)msg.lParam);
			SetEvent(codeCompSem);
			free((void *)msg.lParam);
		}
		if (stopCCThread)
			break;
		Sleep(50);
	}
	SetEvent(ccThreadExit);
}
//-------------------------------------------------------------------------

LRESULT CALLBACK DrawProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam)
{
    DWORD threadhand;
	LRESULT rv;
    DWINFO *ptr,  *ptr1;
	EDITDATA *ed;
    OPENFILENAME ofn;
    HDC dc;
    HPEN hpen, oldpen;
    RECT r;
    HBRUSH graybrush;
    LOGBRUSH lbrush;
    PAINTSTRUCT paint;
	LPCREATESTRUCT createStruct;
    int childheight;
    int startpos, endpos, flag, i;
    HWND win;
    FILETIME time;
    NMHDR *nm;
    CHARRANGE s;
    switch (iMessage)
    {
		case WM_LBUTTONDOWN:
            ptr = (DWINFO*)GetWindowLong(hwnd, 0);
			if (!ptr->timing)
			{
				POINT pt;
                int linenum;
				RECT r;
				GetCursorPos(&pt);
				GetWindowRect(ptr->dwHandle, &r);
				pt.x = 0;
				pt.y -= r.top;
				GetClientRect(ptr->dwHandle, &r);
				if (pt.y < r.bottom - r.top)
				{
	                linenum = SendMessage(ptr->dwHandle, EM_CHARFROMPOS, 0, (LPARAM)&pt);
/*
					if (uState == notDebugging)
						ToggleBookMark(linenum);
					else
*/
					{
	    	            linenum = SendMessage(ptr->dwHandle, EM_EXLINEFROMCHAR, 0,
    	    	            linenum) + 1;
	            	    Tag(TAG_BP, ptr->dwName, linenum, 0, 0, 0, 0);
					}
				}
			}
			break;
        case EN_LINECHANGE:
            EndFind();
            ptr = (DWINFO*)GetWindowLong(hwnd, 0);
            TagLineChange(ptr->dwName, wParam + 1, lParam);
            FileBrowseLineChange(ptr, wParam + 1, lParam);
            GetClientRect(hwnd, &r);
            r.right = ptr->editorOffset;
            InvalidateRect(hwnd, &r, 0);
            break;
        case WM_NOTIFY:
            nm = (NMHDR*)lParam;
            if (nm->code == NM_RCLICK)
            {
                HMENU menu = LoadMenuGeneric(hInstance, "EDITMENU");
                HMENU popup = GetSubMenu(menu, 0);
                POINT pos, pos1;
				RECT rect;
                ptr = (DWINFO*)GetWindowLong(hwnd, 0);
                SendMessage(hwnd, EN_SETCURSOR, 0, 0);
                if (!SendMessage(ptr->dwHandle, EM_GETMODIFY, 0, 0))
                    EnableMenuItem(menu, IDM_SAVE, MF_GRAYED);
                if (uState != atBreakpoint && uState != atException)
                {
                    EnableMenuItem(menu, IDM_RUNTO, MF_GRAYED);
                    EnableMenuItem(menu, IDM_ADDWATCHINDIRECT, MF_GRAYED);
					EnableMenuItem(menu, IDM_DATABREAKPOINTINDIRECT, MF_GRAYED);
                }
                GetCursorPos(&pos);
                pos1.x = pos.x;
                pos1.y = pos.y;
				rightclickPos = pos;
				GetWindowRect(ptr->dwHandle, &rect);
				rightclickPos.x -= rect.left;
				rightclickPos.y -= rect.top;
				InsertBitmapsInMenu(popup);
                TrackPopupMenuEx(popup, TPM_BOTTOMALIGN | TPM_LEFTBUTTON, pos.x,
                    pos.y, hwndFrame, NULL);
                DestroyMenu(menu);
	            return 0;
            }
			break;
        case WM_SYSCOMMAND:
            if (wParam == SC_CLOSE)
            {
                SendMessage(hwnd, WM_COMMAND, IDM_CLOSE, 0);
                return 0;
            }
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
			case IDM_NEWWINDOW:
                ptr = (DWINFO*)GetWindowLong(hwnd, 0);
				newInfo = ptr ;
				openfile(ptr, TRUE, TRUE);
				break ;
            case IDM_SPECIFIEDHELP:
			case IDM_RTLHELP:
			case IDM_LANGUAGEHELP:
                ptr = (DWINFO*)GetWindowLong(hwnd, 0);
                i = SendMessage(ptr->dwHandle, WM_COMMAND, wParam, lParam);
                break;
            case IDM_GOTO:
                lParam = DialogBox(hInstance, "GOTODIALOG", hwnd, (DLGPROC)
                    gotoProc);
                if (lParam == 0)
                    break;
                // fall through
            case IDM_SETLINE:
                ptr = (DWINFO*)GetWindowLong(hwnd, 0);
                SendMessage(ptr->dwHandle, EM_GETLINECOUNT, 0, 0); 
                    // force update of vertical scroll range
                i = SendMessage(ptr->dwHandle, EM_LINEINDEX, lParam - 1, 0);
                s.cpMin = i;
                s.cpMax = i;
	            SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_HIDESELECTION, 1, 0);
                SendMessage(ptr->dwHandle, EM_EXSETSEL, 0, (LPARAM) &s);
                SendMessage(ptr->dwHandle, EM_SCROLLCARET, 0, 1);
    	        SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_HIDESELECTION, 0, 0);
                drawParams(ptr, hwnd);
                EndFind();
                InvalidateRect(hwnd, 0, 0);
                break;
            case ID_REDRAWSTATUS:
                ptr = (DWINFO*)GetWindowLong(hwnd, 0);
                drawParams(ptr, hwnd);
                InvalidateRect(hwnd, 0, 0);
                break;
            case ID_QUERYHASFILE:
                ptr = (DWINFO*)GetWindowLong(hwnd, 0);
                ptr1 = (DWINFO*)lParam;
                if (!xstricmpz(ptr->dwName, ptr1->dwName))
				{
					return SendMessage(ptr->dwHandle, EM_GETEDITDATA, 0, 0);
				}
				return 0 ;
            case ID_QUERYSAVE:
                ptr = (DWINFO*)GetWindowLong(hwnd, 0);
                rv = SendMessage(ptr->dwHandle, EM_GETMODIFY, 0, 0);
                if (rv)
                {
                    return ExtendedMessageBox("File Changed",
                        MB_YESNOCANCEL, 
                        "File %s has changed.  Do you wish to save it?", ptr
                        ->dwTitle);
                }
                else
                    return IDNO;
            case IDM_SAVEAS:
                dialog: ptr = (DWINFO*)GetWindowLong(hwnd, 0);
                if (!SaveFileDialog(&ofn, ptr->dwName, hwnd, TRUE,
                    szSourceFilter, 0))
                    break;
                if (activeProject && ptr->dwName[0] == 0)
                {
                    if (ExtendedMessageBox("Project Query", MB_TASKMODAL |
                        MB_YESNO, "Add file to active project?") == IDYES)
                    {
                            AddFile(activeProject, ofn.lpstrFile, TRUE);
							
                    }
                }
                strcpy(ptr->dwTitle, ofn.lpstrFileTitle);
                strcpy(ptr->dwName, ofn.lpstrFile);
                recolorize(ptr);
                SendMessage(ptr->dwHandle, EM_SETMODIFY, 1, 0);
            case IDM_SAVE:
                ptr = (DWINFO*)GetWindowLong(hwnd, 0);
				if (LOWORD(wParam) == IDM_SAVE)
				{
	                rv = SendMessage(ptr->dwHandle, EM_GETMODIFY, 0, 0);
    	            if (!rv)
        	            break;
				}
                if (ptr->dwName[0] == 0)
                    goto dialog;
                rv = SaveFile(hwnd, (char*)GetWindowLong(hwnd, 0));
                TagLinesAdjust(ptr->dwName, TAGM_SAVEFILE);
                SendMessage(ptr->dwHandle, EM_SETMODIFY, 0, 0);
                drawParams(ptr, hwnd);
                CalculateFileAutoDepends(ptr->dwName);
//				InstallForParse(hwnd);
                return rv;
            case IDM_CLOSE:
                {
                    rv = SendMessage(hwnd, WM_COMMAND, ID_QUERYSAVE, 0);
                    switch (rv)
                    {
                    case IDYES:
                        if (SendMessage(hwnd, WM_COMMAND, IDM_SAVE, 0))
                           SendMessage(hwnd, WM_CLOSE, 0, 0);
                        break;
                    case IDNO:
                        ptr = (DWINFO*)GetWindowLong(hwnd, 0);
                        TagLinesAdjust(ptr->dwName, TAGM_DISCARDFILE);
                        SendMessage(hwnd, WM_CLOSE, 0, 0);
                        break;
                    case IDCANCEL:
                        break;
                    }
                    return rv;
                }
                break;
            case IDM_UNDO:
                EndFind();
                SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), WM_UNDO, 0, 0);
                break;
            case IDM_REDO:
                EndFind();
                SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), WM_REDO, 0, 0);
                break;
            case IDM_CUT:
                EndFind();
                SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), WM_CUT, 0, 0);
                break;
            case IDM_COPY:
                EndFind();
                SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), WM_COPY, 0, 0);
                break;
            case IDM_PASTE:
                EndFind();
                SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), WM_PASTE, 0, 0);
                break;
            case IDM_SELECTALL:
                SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_SETSEL, 0,  - 1);
                break;
            case IDM_FIND:
                OpenFindDialog();
                break;
            case IDM_REPLACE:
                OpenReplaceDialog();
                break;
            case IDM_FINDNEXT:
			{
				DWORD id;
                CloseHandle(CreateThread(NULL,0,
										 (LPTHREAD_START_ROUTINE)(finding ? DoFindNext : DoReplaceNext), 
										 hwndFindInternal, 0, &id));
                break;
			}
            case IDM_TOUPPER:
                SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_TOUPPER, 0, 0);
                break;
            case IDM_TOLOWER:
                SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_TOLOWER, 0, 0);
                break;
			case IDM_COMMENT:
				SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_SELECTCOMMENT, 0, 1);
				break;
			case IDM_UNCOMMENT:
				SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_SELECTCOMMENT, 0, 0);
				break;
            case IDM_INDENT:
                SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_SELECTINDENT, 0,
                    1);
                break;
            case IDM_UNINDENT:
                SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_SELECTINDENT, 0,
                    0);
                break;
            case EN_SETFOCUS:
                break;
			case EN_NEEDFOCUS:
				if ((HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0) == hwnd)
					SendMessage(hwnd, WM_SETFOCUS, 0, 0);
				else
					SendMessage(hwndClient, WM_MDIACTIVATE, (WPARAM)hwnd, NULL);
				break;
            case EN_CHANGE:
            {
                CHARRANGE xx;
                int lineno;
                ptr = (DWINFO*)GetWindowLong(hwnd, 0);
                SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_EXGETSEL, 0, (LPARAM)&xx);
                lineno = SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_LINEFROMCHAR, xx.cpMin, 0);
                FileBrowseLineChange(ptr, lineno + 1, 0);
                break;
            }
            default:
                return DefMDIChildProc(hwnd, iMessage, wParam, lParam);
            }
            
            break;
		case EM_CANREDO:
            return SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_CANREDO, 0, 0)
                ;
        case EM_CANUNDO:
            return SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_CANUNDO, 0, 0)
                ;
        case EN_SETCURSOR:
            EndFind();
            ptr = (DWINFO*)GetWindowLong(hwnd, 0);
            drawParams(ptr, hwnd);
            GetClientRect(hwnd, &r);
            r.right = EDITOR_OFFSET - 3;
            InvalidateRect(hwnd, &r, 0);
            break;
        case WM_PAINT:
            GetClientRect(hwnd, &r);
            dc = BeginPaint(hwnd, &paint);
			/*
            hpen = CreatePen(PS_SOLID, 1, 0xcccccc);
			*/
			/*
            oldpen = SelectObject(dc, hpen);
            MoveToEx(dc, EDITOR_OFFSET - 2, 0, 0);
            LineTo(dc, EDITOR_OFFSET - 2, r.bottom);
            SelectObject(dc, oldpen);
            DeleteObject(hpen);
			*/
            PaintBreakpoints(hwnd, dc, &paint, &r);
            EndPaint(hwnd, &paint);
            return 0;
        case WM_CREATE:
            //         maximized = TRUE ;			
			rv = DefWindowProc(hwnd, iMessage, wParam, lParam);
			if (rv)
				return rv;
			createStruct = (LPCREATESTRUCT)lParam;
			ed = (EDITDATA *)((LPMDICREATESTRUCT)(createStruct->lpCreateParams))->lParam;
            ptr = calloc(1, sizeof(DWINFO));
            SetWindowLong(hwnd, 0, (int)ptr);
            SetWindowLong(hwnd, 4, (int)EDITSIG);
            if (ed != -1)
            {
                strcpy(ptr->dwTitle, newInfo->dwTitle);
                SetWindowText(hwnd, newInfo->dwTitle);
                strcpy(ptr->dwName, newInfo->dwName);
            }
            else
            {
                strcpy(ptr->dwTitle, szUntitled);
                SetWindowText(hwnd, ptr->dwTitle);
            }
            if (!ptr->newFile)
                FileTime(&ptr->time, ptr->dwName);
            ptr->dwHandle = CreateWindowEx(0, "xedit", 0, WS_CHILD +
                WS_CLIPSIBLINGS + WS_CLIPCHILDREN + WS_HSCROLL + WS_VSCROLL + ES_LEFT + WS_VISIBLE +
                ES_MULTILINE + ES_NOHIDESEL + ES_AUTOVSCROLL + ES_AUTOHSCROLL,
                EDITOR_OFFSET, 0, 0, 0, hwnd, (HMENU)ID_EDITCHILD, hInstance, 
				(ed && ed != (DWINFO *)-1) ? (void *)ed : NULL)
                ;
            ptr->self = hwnd;
            ptr->dwLineNo =  - 1;
            if (newInfo && !newInfo->newFile && newInfo->dwLineNo !=  -
                1)
            {
                ptr->dwLineNo = newInfo->dwLineNo;
                newInfo->dwLineNo =  - 1;
            }
            if (!ed && newInfo->logMRU)
            {
                InsertMRU(ptr, 0);
                MRUToMenu(0);
            }
            //         CloseHandle(CreateThread(0,0,(LPTHREAD_START_ROUTINE)asyncLoadFile,(LPVOID)ptr,0,&threadhand)) ;
			if (!ed)
			{
    	        if (ptr->dwName[0] && (!newInfo || !newInfo->newFile))
        	        LoadFile(ptr->self, ptr, FALSE);
	            if (ptr->dwLineNo !=  - 1)
    	        {
        	        PostMessage(ptr->self, WM_COMMAND, IDM_SETLINE, ptr->dwLineNo);
            	}
	            recolorize(ptr);
			}
			EnterCriticalSection(&ewMutex);
            if (editWindows)
                editWindows->prev = ptr;
            ptr->next = editWindows;
            ptr->prev = NULL;
            editWindows = ptr;
			LeaveCriticalSection(&ewMutex);
            SendMessage(hwnd, WM_SETLINENUMBERMODE, 2, 0);
		SendMessage(hwndSrcTab, TABM_ADD, (WPARAM)ptr->dwName, (LPARAM)hwnd);
		ptr->editData = (EDITDATA *)SendMessage(ptr->dwHandle, EM_GETEDITDATA, 0, 0);
       		InstallForParse(hwnd);
			return rv;

        case WM_CLOSE:
            ptr = (DWINFO*)GetWindowLong(hwnd, 0);
            FileBrowseClose(ptr);
			deleteParseData(ptr->dwName);
            eraseParams(ptr->dwHandle);
            break;
        case WM_DESTROY:
            ptr = (DWINFO*)GetWindowLong(hwnd, 0);
			PostMessage(hwndSrcTab, TABM_REMOVE, 0, (LPARAM)hwnd);
			EnterCriticalSection(&ewMutex);
            if (ptr->prev)
                ptr->prev->next = ptr->next;
            if (ptr->next)
                ptr->next->prev = ptr->prev;
            if (ptr == editWindows)
                editWindows = ptr->next;
     		LeaveCriticalSection(&ewMutex);
            free((void*)GetWindowLong(hwnd, 0));
            PostMessage(hwndFrame, WM_REDRAWTOOLBAR, 0, 0);
            break;
        case WM_SIZE:
            ptr = (DWINFO*)GetWindowLong(hwnd, 0);
			if (wParam == SIZE_MAXIMIZED)
            {
				maximizeClient = TRUE;
                SavePreferences();
            }
			if (wParam == SIZE_RESTORED)
            {
				maximizeClient = FALSE;
                SavePreferences();
            }
            MoveWindow(GetDlgItem(hwnd, ID_EDITCHILD), ptr->editorOffset, 0, 
                (lParam &65535) - ptr->editorOffset, lParam >> 16, 1);
            break;
		// timer being used to prevent a click in the margin which activates
		// the window from setting a breakpoint...
        // it also is used to re-establish the focus as some instances with
        // using an OPENFILEDIALOG create weird timing conditions that don't
        // change the focus...
		case WM_TIMER:
			KillTimer(hwnd, 1000);
            ptr = (DWINFO*)GetWindowLong(hwnd, 0);
			ptr->timing = FALSE;
			if ((HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0) == hwnd)
		            PostMessage(hwnd, WM_COMMAND, EN_NEEDFOCUS, 0);
			break;
        case WM_MDIACTIVATE:
			if ((HWND)lParam != hwnd)
			{
				break;
			}
            ptr = (DWINFO*)GetWindowLong(hwnd, 0);
            ptr->jumplistLineno = -1;
			EnterCriticalSection(&ewMutex);
            if (ptr->prev)
                ptr->prev->next = ptr->next;
            else
                editWindows = ptr->next;
            if (ptr->next)
                ptr->next->prev = ptr->prev;
            if (editWindows)
                editWindows->prev = ptr;
            ptr->next = editWindows;
            ptr->prev = NULL;
            editWindows = ptr;
			LeaveCriticalSection(&ewMutex);
            EndFind();
			ptr->timing = TRUE;
			SetTimer(hwnd, 1000, 100, NULL);
			break;
        case WM_SETFOCUS:
            ptr = (DWINFO*)GetWindowLong(hwnd, 0);
            drawParams(ptr, hwnd);
            InvalidateRect(hwnd, 0, 0);
            PostMessage(hwndFrame, WM_REDRAWTOOLBAR, 0, 0);
            PostMessage(hwndSrcTab, TABM_SELECT, 0, (LPARAM)hwnd);
			SetFocus(GetDlgItem(hwnd, ID_EDITCHILD));
			return 0;
        case WM_KILLFOCUS:
//			SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), iMessage, wParam, lParam);
            break;
			return 0;
        case WM_SETLINENUMBERMODE:
        {
            BOOL lineNumbers = PropGetBool(NULL, "LINE_NUMBERS");
            ptr = (DWINFO*)GetWindowLong(hwnd, 0);
            if (wParam)
            {
                    ptr->lineNumberDigits = wParam; // never goes down while the window is open...
            }
            ptr->editorOffset = EDITOR_OFFSET +  
                lineNumbers * (ptr->lineNumberDigits * ((EDITDATA *)SendMessage(ptr->dwHandle, EM_GETEDITDATA, 0, 0))->cd->txtFontWidth + 4);
            GetClientRect(hwnd, &r);
            MoveWindow(GetDlgItem(hwnd, ID_EDITCHILD), ptr->editorOffset, r.top, 
                r.right - ptr->editorOffset, r.bottom - r.top, 1);
            InvalidateRect(hwnd, 0, 1);
            break;
        }
        case WM_INITMENUPOPUP:
            SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), EM_GETSEL, (WPARAM)
                &startpos, (LPARAM) &endpos);
            flag = startpos < endpos;
            EnableMenuItem(hMenuMain, IDM_CUT, flag);
            EnableMenuItem(hMenuMain, IDM_COPY, flag);
            EnableMenuItem(hMenuMain, IDM_PASTE, 1);
            EnableMenuItem(hMenuMain, IDM_UNDO, SendMessage(GetDlgItem(hwnd,
                ID_EDITCHILD), EM_CANUNDO, 0, 0));
            EnableMenuItem(hMenuMain, IDM_BROWSE, flag);
            //EnableMenuItem(hMenuMain,IDM_BROWSEBACK,flag) ;
            EnableMenuItem(hMenuMain, IDM_BOOKMARK, flag);
            //EnableMenuItem(hMenuMain,IDM_NEXTBOOKMARK,flag) ;
            //EnableMenuItem(hMenuMain,IDM_PREVBOOKMARK,flag) ;
            return 0;
        case WM_WORDUNDERCURSOR:
		case WM_WORDUNDERPOINT:
            ptr = (DWINFO*)GetWindowLong(hwnd, 0);
			{
            	int rv = SendMessage(ptr->dwHandle, iMessage, wParam, lParam);
				return rv;
			}
        case WM_FILETITLE:
            ptr = (DWINFO*)GetWindowLong(hwnd, 0);
            return ptr->dwTitle;
        case WM_FILENAME:
            ptr = (DWINFO*)GetWindowLong(hwnd, 0);
            return ptr->dwName;
        default:
			if (iMessage >= WM_USER)
				return SendMessage(GetDlgItem(hwnd, ID_EDITCHILD), iMessage, wParam, lParam);
            break;
    }
    return DefMDIChildProc(hwnd, iMessage, wParam, lParam);
}

//-------------------------------------------------------------------------

void RegisterDrawWindow(void)
{
    HBITMAP bitmap;
    WNDCLASS wc;
//	editHeap = HeapCreate(0, 2 * 1024 * 1024, 128  * 1024 * 1024);
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = &DrawProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(void*) * 2;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = szDrawClassName;
    RegisterClass(&wc);
    bitmap = LoadBitmap(hInstance, "ID_TAG");
    ChangeBitmapColor(bitmap, 0xc0c0c0, RetrieveSysColor(COLOR_BTNFACE));
    tagImageList = ImageList_Create(16, 16, ILC_COLOR24, ILEDIT_IMAGECOUNT, 0);
	ImageList_Add(tagImageList, bitmap, NULL);
    DeleteObject(bitmap);
    
	ccThreadExit = CreateEvent(0,0,0,0);
	InitializeCriticalSection(&ewMutex);
	codeCompSem = CreateEvent(NULL,FALSE,TRUE,NULL);
	CloseHandle(CreateThread(0,0, (LPTHREAD_START_ROUTINE)ScanParse, 0, 0, &ccThreadId));
}

//-------------------------------------------------------------------------

HWND openfile(DWINFO *newInfo, int newwindow, int visible)
{
    BOOL maximized;
	HWND rv, last ;
	int i;
	void *extra = newInfo == (DWINFO *)-1 ? newInfo : NULL ;
	MSG msg;
	while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		ProcessMessage(&msg);
    if (newInfo && newInfo != (DWINFO*) - 1)
    {
        DWINFO *ptr = editWindows;
        while (ptr)
        {
		    if (SendMessage(ptr->self, WM_COMMAND, ID_QUERYHASFILE, 
		        (LPARAM)newInfo))
		    {
				if (newwindow)
				{
					extra = (EDITDATA *)SendMessage(ptr->self, EM_GETEDITDATA, 0, 0);
				}
				else
				{
					HWND active = (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0);
					if (GetParent(hwndASM) == active || ptr->self == active)
					{
						SetFocus(active);
					}
					else
					{
						PostMessage(hwndClient, WM_MDIACTIVATE, (WPARAM)ptr->self, 0);
					}
		        	if (newInfo->dwLineNo !=  - 1)
		            	    PostMessage(ptr->self, WM_COMMAND, IDM_SETLINE, newInfo
		                	    ->dwLineNo);
			        return ptr->self;
				}
		    }
			ptr = ptr->next;
        }
    }
    SendMessage(hwndClient, WM_MDIGETACTIVE, 0, (LPARAM) &maximized);
	rv = CreateMDIWindow(szDrawClassName, szUntitled, (visible ? WS_VISIBLE : 0) |
    	WS_CHILD | WS_OVERLAPPEDWINDOW | WS_SYSMENU | MDIS_ALLCHILDSTYLES | 
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
        WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | (maximized ? WS_MAXIMIZE : 0),
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndClient, hInstance, 
		(LPARAM)extra); 
    return rv;
}
//-------------------------------------------------------------------------

HWND CreateDrawWindow(DWINFO *baseinfo, int visible)
{
    int i;
    static DWINFO temp;
    OPENFILENAME ofn;

    newInfo = baseinfo;
    if (!newInfo)
    {
        newInfo = &temp;
        newInfo->dwLineNo =  - 1;
        newInfo->logMRU = TRUE;
        newInfo->newFile = FALSE;
        if (OpenFileDialog(&ofn, 0, 0, FALSE, TRUE, szSourceFilter, 0))
        {
            char *q = ofn.lpstrFile, path[256];
            strcpy(path, ofn.lpstrFile);
            q += strlen(q) + 1;
            if (! *q)
            {
                strcpy(newInfo->dwTitle, ofn.lpstrFileTitle);
                strcpy(newInfo->dwName, ofn.lpstrFile);
            }
            else
            {
                while (*q)
                {
                    strcpy(newInfo->dwTitle, q);
                    sprintf(newInfo->dwName, "%s\\%s", path, q);
                    openfile(newInfo, FALSE, visible);
                    q += strlen(q) + 1;
                }
                return 0;
            }
        }
        else
        {

            //               ExtendedMessageBox("File Open",MB_SETFOREGROUND | MB_SYSTEMMODAL,"Could not open file %s %d",newInfo->dwName,GetLastError()) ;
            return 0;
        }
    } else if (newInfo == (DWINFO *)-1)
    {
        newInfo = &temp;
        newInfo->dwLineNo =  - 1;
        newInfo->logMRU = TRUE;
        newInfo->newFile = TRUE;
        if (SaveFileDialog(&ofn, 0, 0, TRUE, szNewFileFilter, "Open New File"))
        {
            char *q = ofn.lpstrFile, path[256];
            strcpy(path, ofn.lpstrFile);
            q += strlen(q) + 1;
            if (! *q)
            {
                strcpy(newInfo->dwTitle, ofn.lpstrFileTitle);
                strcpy(newInfo->dwName, ofn.lpstrFile);
            }
            else
            {
                while (*q)
                {
                    strcpy(newInfo->dwTitle, q);
                    sprintf(newInfo->dwName, "%s\\%s", path, q);
                    openfile(newInfo, FALSE, visible);
                    q += strlen(q) + 1;
                }
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    return openfile(newInfo, FALSE, visible);
}
