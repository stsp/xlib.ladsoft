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

#include "helpid.h"
#include "header.h"
#include <ctype.h>
#include <stdlib.h>

#define N_EDITDONE -4001

extern int making;
extern enum DebugStates uState;
extern HWND hwndToolEdit, hwndToolDebug, hwndToolBuild;
extern LOGFONT EditFont;
extern HINSTANCE hInstance;
extern HWND hwndClient, hwndStatus, hwndFrame;
extern char szTargetFilter[], szWorkAreaFilter[], szSourceFilter[], szProjectFilter[], szNewFileFilter[] ;
extern char szCWSFilter[], szCTGFilter[];
extern HWND hwndTab, hwndWatch;
extern HWND hwndTabCtrl;
extern char szInstallPath[];
extern HANDLE codeCompSem;


HWND hwndProject;
char szWorkAreaName[MAX_PATH] = "Default.cwa";
char szWorkAreaTitle[256];
int defaultWorkArea = TRUE;
PROJECTITEM *activeProject;
PROJECTITEM *workArea;

static CRITICAL_SECTION projectSect;
static WNDPROC oldLVProc;
static int noWorkArea = TRUE;
static char szProjectClassName[] = "xccProjectClass";
static HWND treeWindow;
static HBITMAP folderClose, folderOpen, mainIml;
static int ilfolderClose, ilfolderOpen;
static HIMAGELIST treeIml;
static int treeViewSelected;
static HTREEITEM selectedItem;
static HCURSOR dragCur, noCur;
static int newMode;
static char newName[MAX_PATH];
static char newTitle[MAX_PATH];
static char browsePath[MAX_PATH];
static LOGFONT fontdata = 
{
    -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH
        | FF_MODERN | FF_DONTCARE,
		CONTROL_FONT
};
static LOGFONT italicFontData = 
{
    -12, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH
        | FF_MODERN | FF_DONTCARE,
		CONTROL_FONT
};
static LOGFONT boldFontData = 
{
    -12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH
        | FF_MODERN | FF_DONTCARE,
		CONTROL_FONT
};
static HFONT projFont, boldProjFont, italicProjFont;
static HANDLE initSem ;
static HWND hwndEdit;
static PROJECTITEM *editItem;
static char *szprjEditClassName = "xccprjEditClass";
static WNDPROC oldEditProc;

static char *extensionMap[][2] =
{
    ".c", "Source Files",
    ".cpp", "Source Files",
    ".cxx", "Source Files",
    ".asm", "Source Files",
    ".nas", "Source Files",
    ".def", "Source Files",
    ".h", "Include Files",
    ".hpp", "Include Files",
    ".hxx", "Include Files",
    ".p", "Include Files",
    ".inc", "IncludeFiles",
    ".rc", "Resource Files",
    ".dlg", "Resource Files",
    ".bmp", "Resource Files",
    ".cur", "Resource Files",
    ".ico", "Resource Files",
};

LRESULT CALLBACK lvProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam);

static void SetExt(OPENFILENAME *ofn, char *ext)
{
    char *p = strrchr(ofn->lpstrFileTitle, '.');
    if (!p || stricmp(p, ext))
        strcat(ofn->lpstrFileTitle, p); 
}

void MarkChanged(PROJECTITEM *item, BOOL ws)
{
	if (ws)
	{
		while (item && item->type != PJ_WS)
			item = item->parent ;
		if (item)
			item->changed = TRUE;
	}
	else
	{
		while (item && item->type != PJ_PROJ)
			item = item->parent ;
		if (item)
			item->changed = TRUE;
	}
}
//-------------------------------------------------------------------------

int imageof(PROJECTITEM *data, char *name)
{
	if (data->type == PJ_WS)
        return IL_CWA;
	if (data->type == PJ_FOLDER)
		return ilfolderClose;
    name = strrchr(name, '.');
	if (data->type == PJ_PROJ)
	{
	    if (!xstricmpz(name, ".exe"))
	        return IL_EXE;
	    if (!xstricmpz(name, ".lib"))
	        return IL_LIB;
	    if (!xstricmpz(name, ".dll"))
	        return IL_DLL;
		return IL_EXE;
	}
    if (!name)
        return IL_FILES;
    if (!xstricmpz(name, ".asm"))
        return IL_ASM;
    if (!xstricmpz(name, ".c") || !xstricmpz(name, ".cpp"))
        return IL_C;
    if (!xstricmpz(name, ".rc") || !xstricmpz(name, ".bmp") 
			|| !xstricmpz(name, ".cur") || !xstricmpz(name, ".ico"))
        return IL_RES;
    if (!xstricmpz(name, ".h"))
        return IL_H;
    if (!xstricmpz(name, ".cwa"))
        return IL_CWA;
    return IL_FILES;
}

//-------------------------------------------------------------------------

PROJECTITEM *GetItemInfo(HTREEITEM *item)
{
    TV_ITEM xx ;
    if (!item)
        return NULL;
    xx.hItem = item;
    xx.mask = TVIF_PARAM ;
    if (SendMessage(treeWindow, TVM_GETITEM,0 ,(LPARAM)&xx))
        return (PROJECTITEM *)xx.lParam ;
    return NULL;
}

//-------------------------------------------------------------------------

static void TVInsertItem(HWND hTree, HTREEITEM hParent, HTREEITEM after, 
					   PROJECTITEM *data)
{
    TV_INSERTSTRUCT t;
    memset(&t, 0, sizeof(t));
    t.hParent = hParent;
    t.hInsertAfter = after;
    t.UNION item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    t.UNION item.hItem = 0;
    t.UNION item.pszText = data->displayName;
    t.UNION item.cchTextMax = strlen(data->displayName);
    t.UNION item.iImage = t.UNION item.iSelectedImage = imageof(data, data->realName);
    t.UNION item.lParam = (LPARAM)data ;
    data->hTreeItem = TreeView_InsertItem(hTree, &t);
}
void ExpandParents(PROJECTITEM *p)
{
	while (p)
	{
		TreeView_Expand(treeWindow, p->hTreeItem, TVE_EXPAND);
		p = p->parent;
	}
}
void RecursiveCreateTree(HTREEITEM parent, HTREEITEM pos, PROJECTITEM *proj)
{
	while (proj)
	{
	   	TVInsertItem(treeWindow, parent, pos, proj);
		if (proj->children)
		{
			RecursiveCreateTree(proj->hTreeItem, TVI_LAST, proj->children);
            TreeView_Expand(treeWindow, proj->hTreeItem, proj->expanded ? TVE_EXPAND : TVE_COLLAPSE);
		}
		proj = proj->next;
	}
}
PROJECTITEM *CreateFolder(PROJECTITEM *p, char *name, BOOL always )
{
	PROJECTITEM *folder;
	if (!always)
	{
		PROJECTITEM **ins = &p->children;
		while (*ins && stricmp((*ins)->displayName, name) < 0 && (*ins)->type == PJ_FOLDER)
		{
			ins = &(*ins)->next;
		}
		if (*ins && !stricmp((*ins)->displayName, name))
			return *ins;
	}
	folder = calloc(1, sizeof(PROJECTITEM));
	if (folder)
	{
		HTREEITEM pos = TVI_FIRST;
		PROJECTITEM **ins = &p->children;
		while (*ins && stricmp((*ins)->displayName, name) < 0 && (*ins)->type == PJ_FOLDER)
		{
			pos = (*ins)->hTreeItem;
			ins = &(*ins)->next;
		}
		strcpy(folder->displayName, name);
		folder->type = PJ_FOLDER;
		folder->parent = p;
		folder->next = *ins;
		*ins = folder;
		TVInsertItem(treeWindow, p->hTreeItem, pos, folder);
		MarkChanged(folder, FALSE);
	}
	return folder;
}
static int ParseNewProjectData(HWND hwnd)
{
    int rv = 1;
    GetWindowText(GetDlgItem(hwnd, IDC_FILENEWPROJECT), newTitle, sizeof(newTitle));
    GetWindowText(GetDlgItem(hwnd, IDC_PROJECTNEWPROJECT), newName, sizeof(newName));
    newMode = ListView_GetSelectionMark(GetDlgItem(hwnd, IDC_LVNEWPROJECT));
    if (newTitle[0] == 0 || newName[0] == 0)
    {
        ExtendedMessageBox("Add New Project", 0, "Please specify a new project name");
        rv = 0;
    }
    else
    {
        if (newName[strlen(newName)-1] != '\\')
        strcat(newName, "\\");
        strcat(newName, newTitle);
        switch(newMode)
        {
            case BT_CONSOLE:
            case BT_WINDOWS:
            default:
                strcat(newName, ".exe");
                break;
            case BT_DLL:
                strcat(newName, ".dll");
                break;
            case BT_LIBRARY:
                strcat(newName, ".lib");
                break;
        }
    }
    return rv;
}
static int CreateNewProjectData(HWND hwnd)
{
    int items = 0;
    LV_ITEM item;
    RECT r;
    HWND hwndLV = GetDlgItem(hwnd, IDC_LVNEWPROJECT);
    LV_COLUMN lvC;
    ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_FULLROWSELECT);
    ListView_SetImageList(hwndLV, ImageList_Duplicate(treeIml), LVSIL_SMALL);

    GetWindowRect(hwndLV, &r);
    lvC.mask = LVCF_WIDTH | LVCF_SUBITEM ;
    lvC.cx = r.right - r.left;
    lvC.iSubItem = 0;
    ListView_InsertColumn(hwndLV, 0, &lvC);

    memset(&item, 0, sizeof(item));
    item.iItem = items++;
    item.iSubItem = 0;
    item.mask = LVIF_TEXT | LVIF_IMAGE;
    item.iImage = IL_EXE;
    item.pszText = "Console";
    ListView_InsertItem(hwndLV, &item);
    
    item.iItem = items++;
    item.iSubItem = 0;
    item.mask = LVIF_TEXT | LVIF_IMAGE;
    item.iImage = IL_EXE;
    item.pszText = "Windows GUI";
    ListView_InsertItem(hwndLV, &item);
    
    item.iItem = items++;
    item.iSubItem = 0;
    item.mask = LVIF_TEXT | LVIF_IMAGE;
    item.iImage = IL_DLL;
    item.pszText = "Windows DLL";
    ListView_InsertItem(hwndLV, &item);
    
    item.iItem = items++;
    item.iSubItem = 0;
    item.mask = LVIF_TEXT | LVIF_IMAGE;
    item.iImage = IL_LIB;
    item.pszText = "Static Library";    
    ListView_InsertItem(hwndLV, &item);
    
    ListView_SetSelectionMark(hwndLV, 0);
    ListView_SetItemState(hwndLV, 0, LVIS_SELECTED, LVIS_SELECTED);

    newTitle[0] = 0;
    if (!defaultWorkArea)
    {
        char *p;
        strcpy(newName,workArea->realName);
        p = strrchr(newName, '\\');
        if (p)
            p[0] = 0;
    }
    else
    {
        GetDefaultProjectsPath(newName);
    }
    SendDlgItemMessage(hwnd, IDC_FILENEWPROJECT, WM_SETTEXT, 0, (LPARAM) newTitle);
    SendDlgItemMessage(hwnd, IDC_PROJECTNEWPROJECT, WM_SETTEXT, 0, (LPARAM) newName);
	oldLVProc = (WNDPROC)GetWindowLong(hwndLV, GWL_WNDPROC);
	SetWindowLong(hwndLV, GWL_WNDPROC, (long)lvProc);
	SendMessage(hwndLV, WM_USER + 10000, 0, (LPARAM)hwnd);
    return items;
}
static int CustomDrawNewProject(HWND hwnd, LPNMLVCUSTOMDRAW draw)
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

long APIENTRY NewProjectProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM
    lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
           if (!CreateNewProjectData(hwnd))
		   {
                EndDialog(hwnd, 1);
		   }
            else
            {
                CenterWindow(hwnd);
            }
            return 1;
        case WM_NOTIFY:
			if (((LPNMHDR)lParam)->code == NM_CUSTOMDRAW)
			{
				SetWindowLong(hwnd, DWL_MSGRESULT, CustomDrawNewProject(hwnd, (LPNMLVCUSTOMDRAW)lParam));
				return TRUE;
			}
            return 0;
		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_INSERT:
					if (GetKeyState(VK_CONTROL) & 0x80000000)
					{
					    HWND hwndLV = GetDlgItem(hwnd, IDC_LVNEWPROJECT);
                        ListView_SetSelectionMark(hwndLV, -1);
					}
					else
					{
					    HWND hwndLV = GetDlgItem(hwnd, IDC_LVNEWPROJECT);
						int i = ListView_GetSelectionMark(hwndLV);
                        ListView_SetSelectionMark(hwndLV, i);
                        ListView_SetItemState(hwndLV, i, LVIS_SELECTED, LVIS_SELECTED);
					}
					break;
			}
			break;
        case WM_COMMAND:
            switch (wParam &0xffff)
            {
            case IDC_NEWPROJECTBROWSE:
                browsePath[0] = 0;
                BrowseForFile(hwndFrame, "New Project Directory", browsePath, MAX_PATH);
                SendDlgItemMessage(hwnd, IDC_PROJECTNEWPROJECT, WM_SETTEXT, 0, (LPARAM)browsePath);
                break;
            case IDOK:
                if (ParseNewProjectData(hwnd))
                    EndDialog(hwnd, IDOK);
                break;
            case IDCANCEL:
                EndDialog(hwnd, IDCANCEL);
                break;
            case IDHELP:
                ContextHelp(IDH_NEW_PROJECT_DIALOG);
                break;
            }
            break;
        case WM_CLOSE:
            PostMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
            break;
    }
    return 0;
}
void ProjectNewProject(void)
{
    PROJECTITEM *data = workArea;
	if (data && data->type == PJ_WS)
	{
        if (DialogBox(hInstance, "IDD_NEWPROJECT", hwndFrame, (DLGPROC)NewProjectProc) == IDOK)
        {
			PROJECTITEM *p = workArea->children;
			while (p)
				if (!stricmp(p->realName, newName))
					return;
				else
					p = p->next;
			p = calloc(1, sizeof(PROJECTITEM));
			if (p)
			{
				PROJECTITEM **ins = &workArea->children, *temp;
				HTREEITEM pos = TVI_FIRST;
                char num[32];
                int imagetype;
                SETTING *set;
				strcpy( p->displayName, newTitle);
				strcpy( p->realName, newName);
				p->expanded = TRUE;
				p->parent = workArea;
				p->type = PJ_PROJ;
				p->loaded = TRUE;
                imagetype = imageof(p, p->realName);
                set = calloc(1, sizeof(SETTING));
                set->next = p->settings;
                p->settings = set;
                set->id = strdup("__PROJECTTYPE");
                set->type = e_numeric;
                
                sprintf(num ,"%d", newMode);
                set->value = strdup(num);
				if (!activeProject)
                {
					activeProject = p;
                    MarkChanged(activeProject, TRUE);
                }
				while (*ins && stricmp((*ins)->displayName, p->displayName) < 0)
				{
					pos = (*ins)->hTreeItem;
					ins = &(*ins)->next;
				}
				p->next = *ins;
				*ins = p;
				TVInsertItem(treeWindow, workArea->hTreeItem, pos, p);
				CreateFolder(p, "Include Files", FALSE);
				CreateFolder(p, "Source Files", FALSE);
				ExpandParents(p);
				MarkChanged(p, TRUE);
				MarkChanged(p, FALSE);
			}
		}
	}
}
void ProjectExistingProject(void)
{
    PROJECTITEM *data = workArea;
	if (data && data->type == PJ_WS)
	{
	    OPENFILENAME ofn;

		if (OpenFileDialog(&ofn, 0, hwndProject, FALSE, FALSE, szProjectFilter,
						   	"Open existing project"))
		{
			PROJECTITEM *p = workArea->children;
			char buf[MAX_PATH];
			char *q;
            q = stristr(ofn.lpstrFile,".cpj");
            if (q)
                *q = 0;            
			while (p)
				if (!stricmp(p->realName, ofn.lpstrFile))
					return;
				else
					p = p->next;
			p = calloc(1, sizeof(PROJECTITEM));
			if (p)
			{
				PROJECTITEM **ins = &workArea->children, *temp;
				HTREEITEM pos = TVI_FIRST;
				strcpy(p->realName, ofn.lpstrFile);
				strcpy(p->displayName, ofn.lpstrFileTitle);
				q = stristr(p->displayName, ".cpj");
				if (q)
					*q = 0;
				p->type = PJ_PROJ;
				p->parent = data;
				while (*ins && stricmp((*ins)->displayName, p->displayName) < 0)
				{
					pos = (*ins)->hTreeItem;
					ins = &(*ins)->next;
				}
				p->parent = data;
				RestoreProject(p);
				RecursiveCreateTree(workArea->hTreeItem, pos, p);
				p->next = *ins;
				*ins = p;
				ExpandParents(p);
                activeProject = p;
                MarkChanged(activeProject, TRUE);
			}
		}
	}
}
void ProjectNewFolder(void)
{
	PROJECTITEM *data = GetItemInfo(selectedItem);
	if (data && (data->type == PJ_FOLDER || data->type == PJ_PROJ))
	{
		CreateFolder(data, "New Folder", TRUE);
		ExpandParents(data);
	}
}
PROJECTITEM *HasFile(PROJECTITEM *data, char *name)
{
    PROJECTITEM *rv = NULL;
    data = data->children;
    while (data && !rv)
    {
        if (data->type == PJ_FOLDER || data->type == PJ_PROJ)
            rv = HasFile(data, name);
        else if (data->type == PJ_FILE)
        {
            if (!stricmp(data->realName, name))
                rv = data;
        }
        data = data->next;
    }
    return rv;
}
PROJECTITEM *AddFile(PROJECTITEM *data, char *name, BOOL automatic)
{
	PROJECTITEM *file;
	PROJECTITEM **ins;
	HTREEITEM pos = TVI_FIRST;
    char *p;
    if (file = HasFile(data, name))
        return file;
	p = strrchr(name, '\\');
	if (p)
		p ++;
	else
		p = name;
	if (data->type == PJ_PROJ && automatic)
	{
        if (!strnicmp(szInstallPath, name, strlen(szInstallPath)))
        {
            return NULL;
        }
        else
        {
            int i;
            for (i = 0; i < sizeof(extensionMap)/sizeof(extensionMap[0]); i++)
            {
                if (strlen(extensionMap[i][0]) < strlen(name) &&
                    !stricmp(name +strlen(name)- strlen(extensionMap[i][0]), extensionMap[i][0]))
                    {
                        data = CreateFolder(data, extensionMap[i][1], FALSE);
                        break;
                    }
            }
        }
	}
	ins = &data->children;
	while (*ins && (*ins)->type == PJ_FOLDER)
	{
		pos = (*ins)->hTreeItem;
		ins = &(*ins)->next;
	}
	while (*ins && stricmp((*ins)->displayName, p) < 0)
	{
		pos = (*ins)->hTreeItem;
		ins = &(*ins)->next;
	}
    file = RetrieveInternalDepend(name);
    if (!file)
    	file = calloc(1, sizeof(PROJECTITEM));
	if (file)
	{
		file->type = PJ_FILE;
		file->parent = data;
		strcpy(file->realName, name);
		strcpy(file->displayName, p);
		
		file->next = *ins;
		*ins = file;
		TVInsertItem(treeWindow, data->hTreeItem, pos, file);
		ExpandParents(file);
		MarkChanged(file, FALSE);
	}
	return file;
}
void ProjectOpenFile(BOOL existing)
{
    PROJECTITEM *data = GetItemInfo(selectedItem);
	if (data && (data->type == PJ_PROJ || data->type == PJ_FOLDER))
	{
	    OPENFILENAME ofn;
		if (OpenFileDialog(&ofn, 0, hwndProject, !existing, existing, szNewFileFilter,
						   	existing ? "Open existing file" : "Create new file"))
		{
			char *path = ofn.lpstrFile;
			if (existing && path[strlen(path)+1])
			{
				char buf[MAX_PATH];
				char *q = path + strlen(path) + 1;
				while (*q)
				{
					sprintf(buf, "%s\\%s", path, q);
					AddFile(data, buf, TRUE);
					q += strlen(q) + 1;
				}
			}
			else
			{
                if (!strrchr(path, '.'))
                {
                    char *p = szNewFileFilter + strlen(szNewFileFilter) + 1;
                    if (ofn.nFilterIndex)
                        ofn.nFilterIndex--;
                    while (ofn.nFilterIndex)
                    {
                        p = p + strlen(p) + 1;
                        p = p + strlen(p) + 1;
                        ofn.nFilterIndex--;
                    }
                    if (p[2]!= '*')
                        strcat(path, p+1);
                }
				AddFile(data, path, TRUE);
				if (!existing)
				{
					DWINFO info;
                    strcpy(info.dwName, ofn.lpstrFile);
                    strcpy(info.dwTitle, ofn.lpstrFileTitle);
                    info.dwLineNo =  - 1;
                    info.logMRU = FALSE;
					info.newFile = FALSE;
                    CreateDrawWindow(&info, TRUE);
				}
			}			
		}
	}
}
void MoveChildrenUp(PROJECTITEM *data)
{
	PROJECTITEM *children = data->children;
	data->children = NULL;
	while (children)
	{
		PROJECTITEM **ins = &data->parent->children;
		HTREEITEM pos = TVI_FIRST;
		PROJECTITEM *next = children->next;
		while (*ins && stricmp((*ins)->displayName, children->displayName) < 0)
		{
			pos = (*ins)->hTreeItem;
			ins =  & (*ins)->next;
		}
		children->next = *ins;
		(*ins) = children;
		children->parent = data->parent;
		TVInsertItem(treeWindow, data->parent->hTreeItem, pos, children);
        TreeView_Expand(treeWindow, children->hTreeItem, children->expanded ? TVE_EXPAND : TVE_COLLAPSE);
		children = next;
	}
}
void FreeSubTree(PROJECTITEM *data)
{
	PROJECTITEM *children = data->children;
	PROJECTITEM **rmv ;
	while (children)
	{
		PROJECTITEM *next = children->next;
		FreeSubTree(children);
		children = next;
	}
	if (data->parent)
	{
		rmv	= &data->parent->children;
		while (*rmv)
		{
			if ((*rmv) == data)
			{
				*rmv = (*rmv)->next;
				free(data);
				break;
			}
			rmv = &(*rmv)->next;
		}
	}
	else
	{
		free(data);
	}
}
void ProjectRemove(void)
{
	PROJECTITEM *data = GetItemInfo(selectedItem);
	if (data)
	{
		BOOL del = FALSE;
		switch( data->type)
		{
			case PJ_PROJ:
                del = ExtendedMessageBox("Project Remove Item", MB_YESNO, 
                    "Remove this project from the Work Area?") == IDYES;
                if (del && data->changed)
                    SaveProject(data);
				break;
			case PJ_FILE:
				del = TRUE;
				break;				
			case PJ_FOLDER:
                del = ExtendedMessageBox("Project Remove Item", MB_YESNO, 
                    "Remove this folder from the Work Area?") == IDYES;
                if (del)
    				MoveChildrenUp(data);
				break;
		}
		if (del)
		{
			MarkChanged(data, data->type == PJ_PROJ);
			TreeView_DeleteItem(treeWindow, data->hTreeItem);
			if (data == activeProject)
			{
				activeProject = workArea->children;
				while (activeProject && !activeProject->loaded)
					activeProject = activeProject->next;
                MarkChanged(activeProject, TRUE);
				InvalidateRect(treeWindow,0,1);
			}
            if (data->type != PJ_FILE || !RetrieveInternalDepend(data->realName))
    			FreeSubTree(data);
		}
	}
}
void ProjectRename(void)
{
	PROJECTITEM *data = GetItemInfo(selectedItem);
	if (data && (data->type == PJ_PROJ || data->type == PJ_FOLDER || data->type == PJ_FILE))
	{
		RECT r, s;
		editItem = data;
        TreeView_GetItemRect(treeWindow, data->hTreeItem, &r, TRUE);
        TreeView_GetItemRect(treeWindow, data->hTreeItem, &s, FALSE);
        hwndEdit = CreateWindow(szprjEditClassName,
            data->displayName, WS_CHILD | ES_AUTOHSCROLL | WS_BORDER, r.left, r.top,
            s.right - r.left, r.bottom - r.top, hwndProject, (HMENU)
            449, (HINSTANCE)GetWindowLong(GetParent(hwndProject),
            GWL_HINSTANCE), 0);
        SendMessage(hwndEdit, EM_SETSEL, 0, (LPARAM) - 1);
        SendMessage(hwndEdit, EM_SETLIMITTEXT, 64, 0);
        ShowWindow(hwndEdit, SW_SHOW);
        SetFocus(hwndEdit);
	}
}
void DoneRenaming(void)
{
	char buf[MAX_PATH];
	char oldName[MAX_PATH];
	strcpy(oldName, editItem->realName);
	buf[0] = 0;
	GetWindowText(hwndEdit, buf, MAX_PATH);
	if (buf[0] != 0)
	{
		if (strcmp(buf, editItem->displayName))
		{
			PROJECTITEM **rmv = &editItem->parent->children;
			switch (editItem->type)
			{
				char buf2[MAX_PATH], buf3[MAX_PATH], *p;
				case PJ_FILE:
				case PJ_PROJ:
					strcpy(buf2, editItem->realName);
					p = strrchr(buf2, '\\');
					if (p)
						p++;
					else
						p = buf2;
					strcpy(p, buf);
					strcpy(buf3, editItem->realName);
					if (editItem->type == PJ_PROJ)
					{
						strcat(buf2, ".cpj");
						strcat(buf3, ".cpj");
					}
					if (!MoveFileEx( buf3, buf2, 0))
					{
						DestroyWindow(hwndEdit);
						return;
					}
					if (editItem->type == PJ_FILE)
					{
						EditRenameFile(editItem->realName, buf2);
					}
				break;
			}
			if (editItem->type == PJ_FILE || editItem->type == PJ_PROJ || editItem->type == PJ_WS)
			{
				char *p = stristr(editItem->realName, editItem->displayName);
				if (!p)
					return;
				strcpy(p, buf);
			}
			strcpy(editItem->displayName, buf);
			while (*rmv && (*rmv) != editItem)
				rmv = &(*rmv)->next;
			if (*rmv)
			{
				*rmv = (*rmv)->next;
			}
			TreeView_DeleteItem(treeWindow, editItem->hTreeItem);
			switch (editItem->type)
			{
				case PJ_FILE:
					AddFile(editItem->parent, editItem->realName, FALSE);
					free(editItem);
					break;
				case PJ_FOLDER:
					CreateFolder(editItem->parent, editItem->displayName, TRUE);
					free(editItem);
					break;
				case PJ_PROJ:
				{
						PROJECTITEM **ins = &editItem->parent->children;
						HTREEITEM pos = TVI_FIRST;
						while (*ins && stricmp((*ins)->displayName, editItem->displayName) < 0)
						{
						   pos = (*ins)->hTreeItem;
						   ins = &(*ins)->next;
						}
						editItem->next = NULL;
						RecursiveCreateTree(editItem->parent->hTreeItem, pos, editItem);
						editItem->next = *ins;
						*ins = editItem;
						MarkChanged(editItem, FALSE);
						MarkChanged(editItem, TRUE);
				}
				break;
			}
		}
	}
	DestroyWindow(hwndEdit);
}
void ProjectSetActive()
{
	PROJECTITEM *data = GetItemInfo(selectedItem);
	if (data && data->type == PJ_PROJ)
	{
        TV_ITEM t;
        memset(&t, 0, sizeof(t));
		activeProject = data;
		MarkChanged(data, TRUE);
        t.mask = TVIF_TEXT;
        t.hItem = data->hTreeItem;
        t.pszText = data->displayName;
        t.cchTextMax = strlen(data->displayName);
        TreeView_SetItem(treeWindow, &t);
        InvalidateRect(treeWindow, 0, 1);
	}
}
void DragTo(HTREEITEM dstItem, HTREEITEM srcItem)
{
	PROJECTITEM *srcData = GetItemInfo(srcItem);
	PROJECTITEM *dstData = GetItemInfo(dstItem);
	if (srcData && dstData && srcData->parent != dstData)
	{
		PROJECTITEM *p = dstData->parent;
		while(p)
			if (p == srcData)
				return;
			else
				p = p->parent;
		if (srcData->type == PJ_FOLDER || srcData->type == PJ_FILE)
		{
			if (dstData->type == PJ_FOLDER || dstData->type == PJ_PROJ)
			{
				PROJECTITEM **rmv = &srcData->parent->children;
				MarkChanged(srcData,FALSE);
				while (*rmv && *rmv != srcData)
					rmv = &(*rmv)->next;
				if (*rmv)
				{
					PROJECTITEM **ins = &dstData->children;
					HTREEITEM pos = TVI_FIRST;
					(*rmv) = (*rmv)->next;

					TreeView_DeleteItem(treeWindow, srcData->hTreeItem);
					if (srcData->type == PJ_FILE)
					{
						while (*ins && (*ins)->type == PJ_FOLDER)
						{
							pos = (*ins)->hTreeItem;
							ins = &(*ins)->next;
						}
					}
					while (*ins && (*ins)->type == srcData->type && stricmp((*ins)->displayName, srcData->displayName) < 0)
					{
						pos = (*ins)->hTreeItem;
						ins = &(*ins)->next;
					}
					srcData->parent = dstData;
					srcData->next = NULL;
					RecursiveCreateTree(dstData->hTreeItem, pos, srcData);
					srcData->next = *ins;
					*ins = srcData;
					MarkChanged(srcData,FALSE);
				}
			}
		}
	}
}
//-------------------------------------------------------------------------

HTREEITEM FindItemRecursive(PROJECTITEM *l, DWINFO *info)
{
	while (l)
	{
		if (l->type == PJ_PROJ || l->type == PJ_FOLDER)
		{
			HTREEITEM rv = FindItemRecursive(l->children, info);
			if (rv)
				return rv;
		}
		else
		{
			if (!stricmp(l->realName, info->dwName))
				return l->hTreeItem;
		}
		l = l->next;
	}
    return NULL;
}
HTREEITEM FindItemByWind(HWND hwnd)
{
    DWINFO *info = (DWINFO*)GetWindowLong(hwnd, 0);
    if (!info)
        return 0 ;
	if (!workArea)
		return 0;
	return FindItemRecursive(workArea->children, info);
}

void CreateProjectMenu(void)
{
	PROJECTITEM *cur = GetItemInfo(selectedItem);
	HMENU menu = NULL;
	if (cur)
	{
		switch(cur->type)
		{
			case PJ_WS:
                menu = LoadMenuGeneric(hInstance, "WORKAREAMENU");
				break;
			case PJ_PROJ:
				if (cur->loaded)
	                menu = LoadMenuGeneric(hInstance, "PROJECTMENU");
				else
	                menu = LoadMenuGeneric(hInstance, "PROJECTREMOVEMENU");
				break;
			case PJ_FOLDER:

                menu = LoadMenuGeneric(hInstance, "FOLDERMENU");
				break;
			case PJ_FILE:
                menu = LoadMenuGeneric(hInstance, "FILEMENU");
				break;
			default:
				menu = NULL;
				break;
		}
	}
	else
	{
    	menu = LoadMenuGeneric(hInstance, "WORKAREAMENU");
	}
	if (menu)
	{
		HMENU popup = GetSubMenu(menu, 0);
		POINT pos;
        GetCursorPos(&pos);
		InsertBitmapsInMenu(popup);
        TrackPopupMenuEx(popup, TPM_TOPALIGN | TPM_LEFTBUTTON, pos.x,
            pos.y, hwndFrame, NULL);
        DestroyMenu(menu);
	}
}
//-------------------------------------------------------------------------
void GetDefaultWorkAreaName(char *buf)
{
    GetDefaultProjectsPath(buf);
//	GetUserDataPath(buf);
    strcat(buf,"\\default.cwa");
}
//-------------------------------------------------------------------------

void SetWorkAreaMRU(PROJECTITEM *workArea)
{
    DWINFO info;
	char buf[MAX_PATH];
	GetDefaultWorkAreaName(buf);
    if (stricmp(buf, workArea->realName))
    {
	    strcpy(info.dwName, workArea->realName);
    	strcpy(info.dwTitle, workArea->displayName);
        InsertMRU(&info, 1);
        MRUToMenu(1);
    }
}
//-------------------------------------------------------------------------
void SaveAllProjects(PROJECTITEM *workArea, BOOL always)
{
	if (workArea)
	{
		PROJECTITEM *p = workArea->children;
		if (always || workArea->changed)
			SaveWorkArea(workArea);
		while (p)
		{
			if (p->changed)
            {
                char *q = strrchr(p->realName, '\\');
                if (q)
                    q++;
                else
                    q = p->realName;
                if (ExtendedMessageBox("Save projects", MB_YESNO, "Project %s has changed.\nDo you want to save it?", q) == IDYES)
    				SaveProject(p);
            }
			p = p->next;
		}
	}
}
void LoadProject(char *name)
{
	PROJECTITEM *p = calloc(1, sizeof(PROJECTITEM)), **ins;
	if (p)
	{
		strcpy(p->realName, name);
		p->type = PJ_PROJ;
		p->displayName[0] = 0;
		RestoreProject(p);
		if (p->displayName[0])
		{
			while ((*ins) && stricmp((*ins)->displayName, p->displayName) < 0)
				ins = &(*ins)->next;
			p->next = *ins;
			*ins = p;
			TreeView_DeleteAllItems(treeWindow);
			RecursiveCreateTree(TVI_ROOT, TVI_FIRST, workArea);
		}
		else
		{
			free(p);
		}
	}
}
void LoadWorkArea(char *name, BOOL existing)
{
	PROJECTITEM *oldWorkArea;
	if (workArea)
	{
		SaveAllProjects(workArea, TRUE);
	}
	EnterCriticalSection(&projectSect);
    ResetInternalAutoDepends();
	TreeView_DeleteAllItems(treeWindow);
	oldWorkArea = workArea;
	activeProject = NULL;
    defaultWorkArea = !!stristr(name, "default.cwa");
	WaitForSingleObject(codeCompSem, INFINITE);
	workArea=RestoreWorkArea(name);
	if (workArea)
	{
		PROJECTITEM *p = workArea->children;
		while (p)
		{
			RestoreProject(p);
			p = p->next;
		}
		if (activeProject && !activeProject->loaded)
			activeProject = NULL;
		if (!activeProject)
		{
			activeProject = workArea->children;
			while (activeProject && !activeProject->loaded)
				activeProject = activeProject->next;
            MarkChanged(activeProject, TRUE);
		}
		RecursiveCreateTree(TVI_ROOT, TVI_FIRST, workArea);
	}
	else if (!existing)
	{
		workArea= calloc(1, sizeof(PROJECTITEM));
		if (workArea)
		{
			char *p;
			workArea->type = PJ_WS;
			strcpy(workArea->realName, name);
			p = strrchr(workArea->realName, '\\');
			if (p)
				p++;
			else
				p = workArea->realName;
		        sprintf(workArea->displayName,"WorkArea: %s", p);
		   	TVInsertItem(treeWindow, TVI_ROOT, TVI_FIRST, workArea);
		}
	}
    else
    {
        workArea = oldWorkArea;
        oldWorkArea = NULL;
	if (workArea)
	{
	        CalculateProjectDepends(workArea);
		RecursiveCreateTree(TVI_ROOT, TVI_FIRST, workArea);
	}
    }
    SetEvent(codeCompSem);
	LeaveCriticalSection(&projectSect);
	if (oldWorkArea)
		FreeSubTree(oldWorkArea);
    FreeBrowseInfo();
	FreeJumpSymbols();
	LoadJumpSymbols();
}
void CloseWorkArea(void)
{
	char buf[MAX_PATH];
	GetDefaultWorkAreaName(buf);
	LoadWorkArea(buf, FALSE);
}
//-------------------------------------------------------------------------

void IndirectProjectWindow(DWINFO *info)
{
    DWORD handle;
	MSG msg;
    dmgrHideWindow(DID_TABWND, FALSE);
	LoadWorkArea(info->dwName, TRUE);
}

void OpenWorkArea(BOOL existing)
{
    OPENFILENAME ofn;
    SaveAllProjects(workArea, TRUE);
	if (!existing)
	{
	    if (!(SaveFileDialog(&ofn, 0, hwndClient, TRUE, szWorkAreaFilter,
	        "New Work Area"))) 
	    {
	        return ;
	    }
        SetExt(&ofn, ".cwa");
        unlink(ofn.lpstrFile);
	}
	else
	{
	    if (!(OpenFileDialog(&ofn, 0, hwndClient, TRUE, FALSE, szWorkAreaFilter,
	        "Open Work Area"))) 
	    {
	        return ;
	    }
        SetExt(&ofn, ".cwa");
	}
	LoadWorkArea(ofn.lpstrFile, existing);
    SetWorkAreaMRU(workArea);
}
static int CustomDraw(HWND hwnd, LPNMTVCUSTOMDRAW draw)
{
	PROJECTITEM *p;
    switch(draw->nmcd.dwDrawStage)
	{
	    case CDDS_PREPAINT :
	        return CDRF_NOTIFYITEMDRAW ;
	    case CDDS_ITEMPREPAINT:
			p = (PROJECTITEM *)draw->nmcd.lItemlParam;
			if (p->type == PJ_PROJ && !p->loaded)
			{
				draw->clrText= RetrieveSysColor(COLOR_GRAYTEXT);
				draw->clrTextBk = RetrieveSysColor(COLOR_WINDOW);
				SelectObject(draw->nmcd.hdc, italicProjFont);
		        return CDRF_NEWFONT;
			}
			else
			{
                if (activeProject == p)
                {
    				SelectObject(draw->nmcd.hdc, boldProjFont);
	    	        return CDRF_NEWFONT;    
                }
				else if (((PROJECTITEM *)draw->nmcd.lItemlParam)->hTreeItem == TreeView_GetDropHilight(treeWindow))
				{
					draw->clrText= RetrieveSysColor(COLOR_HIGHLIGHTTEXT);
					draw->clrTextBk = RetrieveSysColor(COLOR_HIGHLIGHT);
				}
				else if (draw->nmcd.uItemState & (CDIS_SELECTED ))
				{
					draw->clrText= RetrieveSysColor(COLOR_HIGHLIGHT);
					draw->clrTextBk = RetrieveSysColor(COLOR_WINDOW);
				}
				else if (draw->nmcd.uItemState & (CDIS_HOT))
				{
					draw->clrText= RetrieveSysColor(COLOR_INFOTEXT);
					draw->clrTextBk = RetrieveSysColor(COLOR_INFOBK);
				}
				else if (draw->nmcd.uItemState == 0)
				{
					draw->clrText= RetrieveSysColor(COLOR_WINDOWTEXT);
					draw->clrTextBk = RetrieveSysColor(COLOR_WINDOW);
				}
				SelectObject(draw->nmcd.hdc, projFont);
		        return CDRF_NEWFONT;    
			}
			return CDRF_DODEFAULT;
	}
}
static BOOL SendImportCommand(char *fname, BOOL target)
{
	DWORD bRet;			
    STARTUPINFO stStartInfo;
	PROCESS_INFORMATION stProcessInfo;
	DWORD retCode;
	char cmd[5000];
	sprintf(cmd, "\"%s\\bin\\upgradedb.exe\" %s \"%s\"", szInstallPath, target?"proj":"ws", fname);
    memset(&stStartInfo, 0, sizeof(STARTUPINFO));
	memset(&stProcessInfo, 0, sizeof(PROCESS_INFORMATION));

    stStartInfo.cb = sizeof(STARTUPINFO);
    bRet = CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, 0, 
			szInstallPath,  &stStartInfo, &stProcessInfo);
    if (!bRet)
    {
        return FALSE;
    }
    MsgWait(stProcessInfo.hProcess);
    GetExitCodeProcess(stProcessInfo.hProcess, &retCode);
    return retCode == 0;
}
static void Import(BOOL ctg)
{
	    OPENFILENAME ofn;

		if (OpenFileDialog(&ofn, 0, hwndProject, FALSE, FALSE, ctg? szCTGFilter : szCWSFilter,
						   	ctg ? "Open Old Target File" : "Open Old Workspace File"))
		{
            char *q = stristr(ofn.lpstrFile,ctg?".ctg" : ".cws");
            if (!q)
                strcat(ofn.lpstrFile, ctg?".ctg" : ".cws");
            
            if (!SendImportCommand(ofn.lpstrFile, ctg))
            {
                ExtendedMessageBox("Conversion error",0, "File %s could not be converted", ofn.lpstrFile);
            }
            else
            {
                if (ctg)
                {
                    if (workArea)
                    {
            			PROJECTITEM *p = workArea->children;
            			char buf[MAX_PATH];
                        q = strrchr(ofn.lpstrFile, '.');
                        if (q)
                            *q = 0;
            			while (p)
            				if (!stricmp(p->realName, ofn.lpstrFile))
            					return;
            				else
            					p = p->next;
            			p = calloc(1, sizeof(PROJECTITEM));
            			if (p)
            			{
            				PROJECTITEM **ins = &workArea->children, *temp;
            				HTREEITEM pos = TVI_FIRST;
            				strcpy(p->realName, ofn.lpstrFile);
            				strcpy(p->displayName, ofn.lpstrFileTitle);
            				q = stristr(p->displayName, ".cpj");
            				if (q)
            					*q = 0;
            				p->type = PJ_PROJ;
            				p->parent = workArea;
            				while (*ins && stricmp((*ins)->displayName, p->displayName) < 0)
            				{
            					pos = (*ins)->hTreeItem;
            					ins = &(*ins)->next;
            				}
            				p->parent = workArea;
            				RestoreProject(p);
            				RecursiveCreateTree(workArea->hTreeItem, pos, p);
            				p->next = *ins;
            				*ins = p;
            				ExpandParents(p);
                            activeProject = p;
                            MarkChanged(activeProject, TRUE);
            			}
                    }                    
                }
                else
                {
                    q = strrchr(ofn.lpstrFile, '.');
                    if (q)
                        *q = 0;
                    strcat(ofn.lpstrFile, ".cwa");
                	LoadWorkArea(ofn.lpstrFile, TRUE);
                    SetWorkAreaMRU(workArea);
                }
            }
        }
}
//-------------------------------------------------------------------------

LRESULT CALLBACK ProjectProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam)
{
    int i;
    RECT rs,  *rt;
    NM_TREEVIEW *nm;
    DWINFO info;
    char buf[256];
    LPNMTVKEYDOWN key;
    PROJECTITEM *data;
	TVHITTESTINFO hittest;
	HWND win;
	HTREEITEM oldSel;
	static HCURSOR origCurs;
	static BOOL dragging;
	static BOOL inView;
	static HTREEITEM srcItem, dstItem;
    switch (iMessage)
    {
        case WM_SYSCOMMAND:
            if (wParam == SC_CLOSE)
                SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case WM_SETTEXT:
            return SendMessage(hwndTab, iMessage, wParam, lParam);
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            SetFocus(hwnd);
            break;
        case WM_NOTIFY:
            nm = (NM_TREEVIEW*)lParam;
            switch (nm->hdr.code)
            {
			case NM_CUSTOMDRAW:
				return CustomDraw(hwnd, (LPNMTVCUSTOMDRAW)nm);
			case N_EDITDONE:
				DoneRenaming();
				break;
			case TVN_BEGINDRAG:
				GetCursorPos(&hittest.pt);
				ScreenToClient(treeWindow, &hittest.pt);
				srcItem = TreeView_HitTest(treeWindow, &hittest);
				data = GetItemInfo(srcItem);
				if (data && (data->type == PJ_FILE || data->type == PJ_FOLDER))
				{
					dragging = TRUE;
					SetCapture(hwnd);
					origCurs = SetCursor(dragCur);
					inView = TRUE;
				}
				break;
            case TVN_KEYDOWN:
                key = (LPNMTVKEYDOWN)lParam;
                switch (key->wVKey)
                {
	                case VK_INSERT:
	                    if (GetKeyState(VK_CONTROL) &0x80000000)
	                    {
	                        data = GetItemInfo(selectedItem);
							if (data)
							{
								int msg = -1;
								switch (data->type)
								{
									case PJ_WS:
										msg = IDM_EXISTINGPROJECT;
										break;
									case PJ_PROJ:
										msg = IDM_NEWFOLDER;
										break;
									case PJ_FOLDER:
										msg = IDM_EXISTINGFILE;
										break;
								}
								if (msg != -1)
		                            PostMessage(hwnd, WM_COMMAND, msg, 0);
							}
	                    }
	                    else if (GetKeyState(VK_SHIFT) &0x80000000)
	                    {
	                        data = GetItemInfo(selectedItem);
							if (data)
							{
								int msg = -1;
								switch (data->type)
								{
									case PJ_WS:
										msg = IDM_NEWPROJECT;
										break;
									case PJ_PROJ:
										msg = IDM_NEWFOLDER;
										break;
									case PJ_FOLDER:
										msg = IDM_NEWFILE_P;
										break;
								}
								if (msg != -1)
		                            PostMessage(hwnd, WM_COMMAND, msg, 0);
							}
	                    }
	                    else 
						{
	                        data = GetItemInfo(selectedItem);
							if (data && (data->type != PJ_WS))
		                        PostMessage(hwnd, WM_COMMAND, IDM_RENAME, 0);
						}
	                    break;
	                case VK_DELETE:
	                    if (!(GetKeyState(VK_CONTROL) &0x80000000) && !(GetKeyState(VK_SHIFT) &0x8000000))
						{
	                        data = GetItemInfo(selectedItem);
							if (data && (data->type == PJ_FOLDER || data->type == PJ_FILE))
		                        PostMessage(hwnd, WM_COMMAND, IDM_REMOVE, 0);
						}
	                    break;
	                case VK_RETURN:
	                    SendMessage(hwnd, WM_COMMAND, IDM_OPENFILES, 0);
	                    break;
	                }
	                break;
	            case NM_DBLCLK:
					oldSel = selectedItem;
					GetCursorPos(&hittest.pt);
					ScreenToClient(treeWindow, &hittest.pt);
					selectedItem = TreeView_HitTest(treeWindow, &hittest);
					if (selectedItem)
		                PostMessage(hwnd, WM_COMMAND, IDM_OPENFILES, 0);
					selectedItem = oldSel;
	                return 0;
	            case NM_RCLICK:
					GetCursorPos(&hittest.pt);
					ScreenToClient(treeWindow, &hittest.pt);
					selectedItem = TreeView_HitTest(treeWindow, &hittest);
	                if (selectedItem)
					{
	                    TreeView_SelectItem(treeWindow, selectedItem);
					}
					CreateProjectMenu();
	                break;
	            case TVN_SELCHANGED:
	                nm = (NM_TREEVIEW*)lParam;
	                selectedItem = nm->itemNew.hItem;
					if (selectedItem == 0)
						selectedItem = workArea->hTreeItem;
	                break;
	            case TVN_ITEMEXPANDED:
	                nm = (NM_TREEVIEW *)lParam;
	                data = GetItemInfo(nm->itemNew.hItem);
	                if (data)
					{
						if (data->type == PJ_FOLDER)
		                {
							TV_ITEM setitem;
							memset(&setitem, 0, sizeof(setitem));
							setitem.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
							setitem.iImage = setitem.iSelectedImage = 
								nm->action == TVE_EXPAND ? ilfolderOpen : ilfolderClose;
							setitem.hItem = nm->itemNew.hItem;
							TreeView_SetItem(treeWindow, &setitem);
						}
	                    if (nm->action == TVE_EXPAND)
						{
							data->expanded = TRUE;
						}
            	        else
							data->expanded = FALSE;
						return 0;
	                }
	                break;
	            case TVN_DELETEITEM:
	                nm = (NM_TREEVIEW *)lParam;
	                if (nm->itemOld.hItem == selectedItem)
	                    selectedItem = TreeView_GetSelection(treeWindow);
	                break;
            }
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                DWORD id;
            case IDM_IMPORT_CWS:
                Import(FALSE);
                break;
            case IDM_IMPORT_CTG:
                Import(TRUE);
                break;
			case IDM_DOSWINDOW:
			{
                DosWindow(activeProject ? activeProject->realName : NULL, NULL, NULL, NULL, NULL);
			}
				break;
			case IDM_MAKEWINDOW:
			{
                char exec[MAX_PATH];
                sprintf(exec, "%s\\bin\\imake.exe", szInstallPath);
                DosWindow(activeProject ? activeProject->realName : NULL, exec, "", "Custom Make", "Make Is Complete.");
			}
				break;
			case IDM_RUN:
				SaveWorkArea(workArea);
                dbgRebuildMain(wParam);
				break;
			case IDM_SETACTIVEPROJECT:
				ProjectSetActive();
				break;
            case IDM_NEWFILE_P:
                ProjectOpenFile(FALSE);
                PostMessage(hwndFrame, WM_REDRAWTOOLBAR, 0, 0);
                break;
			case IDM_EXISTINGFILE:
                ProjectOpenFile(TRUE);
                PostMessage(hwndFrame, WM_REDRAWTOOLBAR, 0, 0);
				break;
            case IDM_NEWPROJECT:
                ProjectNewProject();
                PostMessage(hwndFrame, WM_REDRAWTOOLBAR, 0, 0);
                break;
            case IDM_EXISTINGPROJECT:
				ProjectExistingProject();
                PostMessage(hwndFrame, WM_REDRAWTOOLBAR, 0, 0);
                break ;
            case IDM_REMOVE:
                ProjectRemove();
                PostMessage(hwndFrame, WM_REDRAWTOOLBAR, 0, 0);
                break;
			case IDM_RENAME:
				ProjectRename();
				break;
			case IDM_NEWFOLDER:
				ProjectNewFolder();
                PostMessage(hwndFrame, WM_REDRAWTOOLBAR, 0, 0);
				break;
            case IDM_NEWWS:
                if (uState != notDebugging)
                {
                    if (ExtendedMessageBox("WorkArea", MB_YESNO, 
                        "WorkArea Open requires the debugger be stopped.\r\nStop the debugger now?") != IDYES)
                    {
                        break;
                    }
                    abortDebug();
                }
                dmgrHideWindow(DID_TABWND, FALSE);
                OpenWorkArea(FALSE);
                break;
            case IDM_OPENWS:
                if (uState != notDebugging)
                {
                    if (ExtendedMessageBox("WorkArea", MB_YESNO, 
                        "WorkArea Open requires the debugger be stopped.\r\nStop the debugger now?") != IDYES)
                    {
                        break;
                    }
                    abortDebug();
                }
                dmgrHideWindow(DID_TABWND, FALSE);
                OpenWorkArea(TRUE);
                break;
            case IDM_CLOSEWS:
                if (making)
                    break;
                if (uState != notDebugging)
                {
                    if (ExtendedMessageBox("WorkArea", MB_YESNO, 
                        "WorkArea Close requires the debugger be stopped.\r\nStop the debugger now?") != IDYES)
                    {
                        break;
                    }
                    abortDebug();
                }
                CloseWorkArea();
                break;
            case IDM_SAVEWS:
                SaveAllProjects(workArea, TRUE);
                break;
            case IDM_COMPILEFILE:
                win = (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0);
                if (IsWindow(win) && !IsSpecialWindow(win))
                {
                    HTREEITEM item = FindItemByWind(win);
                    PROJECTITEM *data = GetItemInfo(item);
                    if (data) {
                        Maker(data, FALSE);
                    }
                }
                break;
            case IDM_GENMAKE:
                if (workArea && workArea->children)
                {
                    genMakeFile(workArea);
                }
                else
                {
                    ExtendedMessageBox("Makefile Generation", MB_SETFOREGROUND |
                        MB_SYSTEMMODAL, 
                        "You need at least one project to generate a make file");
                }
                break;
            case IDM_MAKE:
                if (HIWORD(wParam))
                    if (GetKeyState(VK_CONTROL) &0x80000000)
                        SendMessage(hwnd, WM_COMMAND, IDM_COMPILEFILE, 0);
                    else if (GetKeyState(VK_SHIFT) &0x80000000)
                        Maker(activeProject, FALSE);
                    else
                        Maker(workArea, FALSE);
                else
                    Maker(workArea, FALSE);
                break;
            case IDM_BUILDALL:
                Maker(workArea, TRUE);
                break;
            case IDM_BUILDSELECTED:
                Maker(activeProject, FALSE);
                break;
            case IDM_STOPBUILD:
                StopBuild();
                break;
            case IDM_CALCULATEDEPENDS:
                CalculateProjectDepends(GetItemInfo(selectedItem));
                break;
			case IDM_RUNNODEBUG:
			{
				SaveWorkArea(workArea);
				RunProgram(activeProject);
				break;
			}
            case IDM_ACTIVEPROJECTPROPERTIES:
                selectedItem = activeProject->hTreeItem;
                // fall through
            case IDM_PROJECTPROPERTIES:
                data = GetItemInfo(selectedItem);
                ShowBuildProperties(data);
                break;
            case IDM_OPENFILES:
                data = GetItemInfo(selectedItem);
                if (data)
                    if (data->type == PJ_FILE)
                    {
                        strcpy(info.dwName, data->realName);
                        strcpy(info.dwTitle, data->displayName);
                        info.dwLineNo =  - 1;
                        info.logMRU = FALSE;
						info.newFile = FALSE;
                        CreateDrawWindow(&info, TRUE);
                    }
                break;
            case IDM_CLOSE:
                SendMessage(hwnd, WM_CLOSE, 0, 0);
                break;
            default:
                return DefWindowProc(hwnd, iMessage, wParam, lParam);
            }
            break;
		case WM_LBUTTONUP:
			if (dragging)
			{
				SetCursor(origCurs);
				ReleaseCapture();
				dragging = FALSE;
				TreeView_SelectDropTarget(treeWindow, NULL);
				if (inView && dstItem != srcItem && srcItem && dstItem)
				{
					DragTo(dstItem, srcItem);
				}
			}
			break;
		case WM_MOUSEMOVE:
			if (dragging)
			{
				hittest.pt.x = (long)(short)LOWORD(lParam);
				hittest.pt.y = (long)(short)HIWORD(lParam);
				
				dstItem = TreeView_HitTest(treeWindow, &hittest);
				if (dstItem && dstItem != srcItem)
				{
					PROJECTITEM *srcData = GetItemInfo(srcItem);
					data = GetItemInfo(dstItem);
					if (srcData && data)
					{
						PROJECTITEM *p = data->parent;
						while (p)
							if (p == srcData)
								break;
							else
								p = p->parent;
						if (p)
						{
							if (inView)
							{
								inView = FALSE;
								SetCursor(noCur);
								TreeView_SelectDropTarget(treeWindow, NULL);
							}
							break;
						}
					}
					if (data && (data->type == PJ_PROJ || data->type == PJ_FOLDER))
					{
						if (!inView)
						{
							inView = TRUE;
							SetCursor(dragCur);
						}
						TreeView_SelectDropTarget(treeWindow, dstItem);
					}
					else
					{
						if (inView)
						{
							inView = FALSE;
							SetCursor(noCur);
							TreeView_SelectDropTarget(treeWindow, NULL);
						}
					}
				}
				else
				{
					if (inView)
					{
						inView = FALSE;
						SetCursor(noCur);
						TreeView_SelectDropTarget(treeWindow, NULL);
					}
				}
			}
			break;
        case WM_SETFOCUS:
            PostMessage(hwndFrame, WM_REDRAWTOOLBAR, 0, 0);
			SetFocus(treeWindow);
            break;
        case WM_CREATE:
            hwndProject = hwnd;
            GetClientRect(hwnd, &rs);

            treeViewSelected = 0;
			dragCur = LoadCursor(hInstance, "ID_DRAGCUR");
			noCur = LoadCursor(hInstance, "ID_NODRAGCUR");
			folderClose = LoadBitmap(hInstance, "ID_FOLDERCLOSE");
			folderOpen = LoadBitmap(hInstance, "ID_FOLDEROPEN");
            treeIml = ImageList_Create(16, 16, ILC_COLOR24, IL_IMAGECOUNT+2, 0);
            
			mainIml = LoadBitmap(hInstance, "ID_FILES");
            ChangeBitmapColor(mainIml, 0xffffff, RetrieveSysColor(COLOR_WINDOW));
			ImageList_Add(treeIml, mainIml, NULL);
            ilfolderClose = ImageList_Add(treeIml, folderClose, 0);
            ilfolderOpen = ImageList_Add(treeIml, folderOpen, 0);
			DeleteObject(folderClose);
			DeleteObject(folderOpen);
            DeleteObject(mainIml);
            treeWindow = CreateWindowEx(0, WC_TREEVIEW, "", WS_VISIBLE |
                WS_CHILD | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_TRACKSELECT,
                0, 0, rs.right, rs.bottom, hwnd, (HMENU)ID_TREEVIEW,
                hInstance, NULL);
			i = GetWindowLong(treeWindow, GWL_STYLE);
            TreeView_SetImageList(treeWindow, treeIml, TVSIL_NORMAL);
            projFont = CreateFontIndirect(&fontdata);
			boldProjFont = CreateFontIndirect(&boldFontData);
			italicProjFont = CreateFontIndirect(&italicFontData);
            SendMessage(treeWindow, WM_SETFONT, (WPARAM)boldProjFont, 0);
            return 0;
        case WM_CLOSE:
            SaveAllProjects(workArea, FALSE);
            break;
        case WM_DESTROY:
			FreeSubTree(workArea);
            DestroyWindow(treeWindow);
            DeleteObject(projFont);
			DeleteObject(boldProjFont);
			DeleteObject(italicProjFont);
			DestroyCursor(dragCur);
			DestroyCursor(noCur);
            hwndProject = 0;
            break;
        case WM_SIZE:
            MoveWindow(treeWindow, 0, 0, LOWORD(lParam), HIWORD(lParam), 0);
            break;
        default:
            break;
    }
    return DefWindowProc(hwnd, iMessage, wParam, lParam);
}
static LRESULT CALLBACK extEditWndProc(HWND hwnd, UINT iMessage, WPARAM wParam,
    LPARAM lParam)
{
    LRESULT rv;
    RECT r;
	NMHDR nm;
    switch (iMessage)
    {
        case WM_CHAR:
            if (wParam == 13)
            {
				nm.code = N_EDITDONE;
				nm.hwndFrom = hwnd;
				nm.idFrom = GetDlgItem(hwnd, GWL_ID);
                SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&nm);
                return 0;
            }
			if (wParam == 27)
			{
				DestroyWindow(hwnd);
				return 0;
			}
			break;
		case WM_KILLFOCUS:
			DestroyWindow(hwnd);
			return 0;
    }
    return CallWindowProc(oldEditProc, hwnd, iMessage, wParam, lParam);
}

//-------------------------------------------------------------------------

void RegisterProjectWindow(void)
{
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = 0;
    wc.lpfnWndProc = &ProjectProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(void*);
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = szProjectClassName;
    RegisterClass(&wc);

    GetClassInfo(0, "edit", &wc);
    oldEditProc = wc.lpfnWndProc;
    wc.lpfnWndProc = &extEditWndProc;
    wc.lpszClassName = szprjEditClassName;
    wc.hInstance = hInstance;
    RegisterClass(&wc);

	InitializeCriticalSection(&projectSect);	
}

//-------------------------------------------------------------------------

void CreateProjectWindow(void)
{
    RECT rect;
    HWND parent;
    parent = hwndTabCtrl;
    GetTabRect(&rect);
    hwndProject = CreateWindow(szProjectClassName, szWorkAreaTitle,
        WS_VISIBLE | WS_CHILD, rect.left, rect.top, rect.right - rect.left,
        rect.bottom - rect.top, parent, 0, hInstance, 0);
}

