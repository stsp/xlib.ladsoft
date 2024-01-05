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
#include <commdlg.h>
#include <commctrl.h>
#include <richedit.h>
#include <prsht.h>
#include <stdio.h>
#include "winconst.h"
#include "helpid.h"
#include "header.h"

#define GENERALPROPS "general.props"

#define IDC_AUXBUTTON 10000
#define IDC_LISTBOX 10001
#define IDC_SETFONTNAME 10002

extern char szInstallPath[];
extern HWND hwndFrame;
extern HINSTANCE hInstance;
extern BUILDRULE *buildRules;
extern unsigned int helpMsg;

#define ExtendedMessageBox(one,two,three,four)

int custColors[16];
PROJECTITEM generalProject;

static SETTING *generalPrototype;
static char *szGeneralPropsClassName = "xccGeneralProps";
static char *szAccept = "Accept";
static char *szApply = "Apply";
static char *szClose = "Close";
static char *szHelp = "Help";
static HBITMAP menuButtonBM, comboButtonBM;
static CRITICAL_SECTION propsMutex;

static struct _propsData generalProps =
{
    "General Properties",
    1,
    &generalPrototype,
    &generalProject,
} ;

void InitProps(void)
{
	char name[MAX_PATH];
	strcpy(name, szInstallPath);
	strcat(name, "\\bin\\");
	strcat(name, GENERALPROPS);
	generalPrototype = LoadRule(name);
    if (generalPrototype)
        generalProps.protocount = 1;
    generalProps.title = "General Properties";
    generalProps.saveTo = & generalProject;
    generalProps.prototype = &generalPrototype;
    InitializeCriticalSection(&propsMutex);
}

static BOOL MatchesExt(char *name, char *exts)
{
    char *p = strrchr(name, '.');
    if (p && p[1] != '\\')
    {
        int n =strlen(p);
        char *q = exts;
        while (q)
        {
            char *r = strchr(q, '.');
            if (r)
            {
                if (!strnicmp(r,p,n) && (r[n] == 0 || r[n] == ' '))
                {
                    return TRUE;
                }
            }
            q = strchr(q, ' ');
            if (q)
                while (isspace(*q))
                    q++;
        }
    }
    return FALSE;
}
static BOOL MatchesExtExt(char *source, char *exts)
{
    while (source && *source)
    {
        char buf[MAX_PATH];
        char *p = strchr(source, '.');
        if (p)
        {
            char *q = strchr(p, ' ');
            if (!q)
                q = p + strlen(p);
            strncpy(buf, p, q - p);
            buf[q-p] = 0;
            if (MatchesExt(buf, exts))
                return TRUE;
            p = q;
            while (isspace(*p)) p++;

        }
        source = p;
    }
    return FALSE;
}
static int SortRules(const void *left, const void *right)
{
    SETTING **lleft = (SETTING **)left;
    SETTING **lright = (SETTING **)right;
    if ((*lleft)->order < (*lright)->order)
        return -1;
    return ((*lleft)->order > (*lright)->order);
}
static void LookupDependentRules(struct _propsData *data, char *name)
{
    BUILDRULE *br = buildRules;
    while (br)
    {
        char buf[256];
        if (br->settings->select && !strcmp(name, br->settings->select))
        {
            data->prototype[data->protocount++] = br->settings;
            break;
        }                    
        br = br->next;
    }
}
static char *EvalDepsFind(SETTING *choice, PROJECTITEM *item, struct _propsData *data)
{
    SETTING *s = NULL;
    while (item && !s)
    {
        s = PropFind(item->settings, choice->id);
        item = item->parent;
    }
    if (!s)
    {
        s = PropFindAll(data->prototype, data->protocount, choice->id);
    }
    if (s)
        return s->value;
    return NULL;
}
void EvalDependentRules(SETTING *depends, PROJECTITEM *item, struct _propsData *data)
{
    while (depends)
    {
        if (depends->type == e_choose)
        {
            char *buf = EvalDepsFind(depends, item, data);
            if (buf && !strcmp(buf, depends->value))
            {
                EvalDependentRules(depends->children, item, data);
            }
        }
        else if (depends->type == e_assign)
        {
            LookupDependentRules(data, depends->id);
        }
        depends = depends->next;
    }
}
BUILDRULE *SelectRules(PROJECTITEM *item, struct _propsData *data)
{
    BUILDRULE *p = buildRules;
    char *cls = NULL;
    while (p)
    {
        switch(item->type)
        {
            case PJ_FILE:
                cls="FILE";
                break;
            case PJ_PROJ:
                cls="PROJECT";
                break;
            case PJ_WS:
                cls="WORKAREA";
                break;
        }
        if (cls)
        {
            if (!strcmp(p->settings->cls, cls))
            {
                SETTING *s = NULL;
                if (p->settings->command)
                {
                    s = PropFind(p->settings->command->children, "__SOURCE_EXTENSION");
                }
                if (!s || MatchesExt(item->realName, s->value))
                {
                    int i;
                    for (i=0; i < data->protocount; i++)
                        if (data->prototype[i] == p->settings)
                            break;
                    if (i >= data->protocount)
                    {
                        data->prototype[data->protocount++] = p->settings;
                    }
                }
            }
        }
        p = p->next;
    }
}
static void CullBasicRules(PROJECTITEM *item, struct _propsData *data, int first)
{
    do
    {
        if (item->children)
            CullBasicRules(item->children, data, FALSE);
        SelectRules(item, data);
        item = item->next;
    } while (item && !first);
}
void GetActiveRules(PROJECTITEM *item, struct _propsData *data)
{
    BOOL done = FALSE;
    int i;
    data->protocount = 0;
    CullBasicRules(item, data, TRUE);
    for (i=0; i < data->protocount; i++)
    {
        if (data->prototype[i]->depends)
            EvalDependentRules(data->prototype[i]->depends, item, data);
    }
    qsort(data->prototype, data->protocount, sizeof(SETTING *), SortRules);
}
static void PropGetPrototype(PROJECTITEM *item, struct _propsData *data, SETTING **arr)
{
    memset(data, 0, sizeof(*data));
    if (item == NULL || item == &generalProject)
        memcpy(data, &generalProps, sizeof (*data));
    else
    {
        
        data->title = "Project Properties";
        data->prototype = arr;
        data->protocount = 0;
        data->saveTo = item;
        GetActiveRules(item, data);
    }
}
static void PopulateTree(HWND hwnd, HTREEITEM hParent, SETTING *settings)
{
	while (settings)
	{
		if (settings->type == e_tree)
		{
		    TV_INSERTSTRUCT t;
		    memset(&t, 0, sizeof(t));
		    t.hParent = hParent;
		    t.hInsertAfter = TVI_LAST;
		    t.UNION item.mask = TVIF_TEXT | TVIF_PARAM;
		    t.UNION item.hItem = 0;
		    t.UNION item.pszText = settings->displayName;
		    t.UNION item.cchTextMax = strlen(settings->displayName);
		    t.UNION item.lParam = (LPARAM)settings ;
		    settings->hTreeItem = TreeView_InsertItem(hwnd, &t);
			PopulateTree(hwnd, settings->hTreeItem, settings->children);
		}
		settings = settings->next;
	}
}
static void CreateLVColumns(HWND hwnd, int width)
{
    LV_COLUMN lvC;
    ListView_SetExtendedListViewStyle(hwnd, /*LVS_EX_FULLROWSELECT | */LVS_EX_GRIDLINES);

	ListView_DeleteColumn(hwnd, 1);
	ListView_DeleteColumn(hwnd, 0);
    lvC.mask = LVCF_WIDTH | LVCF_SUBITEM | LVCF_TEXT;
    lvC.cx = width/2;
    lvC.iSubItem = 0;
	lvC.pszText = "Property";
	lvC.cchTextMax = strlen("Property");
    ListView_InsertColumn(hwnd, 0, &lvC);
    lvC.mask = LVCF_WIDTH | LVCF_SUBITEM | LVCF_TEXT;
    lvC.cx = width/2;
    lvC.iSubItem = 1;
	lvC.pszText = "Value";
	lvC.cchTextMax = strlen("Value");
    ListView_InsertColumn(hwnd, 1, &lvC);
}
static void ParseFont(LOGFONT *lf, char *text)
{
	BYTE *p = (BYTE *)lf;
	int size = sizeof(*lf);
	while (size)
	{
		char **n;
		*p++ = strtoul(text, &text, 10);
		size--;
	}
}
static void CreateItemWindow(HWND parent, HWND lv, RECT *r, SETTING *current)
{
	switch (current->type)
	{
		SETTINGCOMBO *sc;
        char *sel;
		case e_combo:
			current->hWnd = CreateWindow("xccCombo","", WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | CBS_DROPDOWNLIST,
										 r->left+1, r->top, r->right - r->left-1, r->bottom - r->top-1, parent, 0, hInstance, 0);
            SendMessage(current->hWnd, WM_SETFONT, (WPARAM)SendMessage(lv, WM_GETFONT, 0, 0), 1);
			sc = current->combo;
            sel = NULL;
			while (sc)
			{
				SendMessage(current->hWnd, CB_ADDSTRING, 0, (LPARAM)sc->displayName);
                if (!strcmp(sc->value, current->tentative))
                    sel = sc->displayName;
				sc = sc->next;
			}
            if (sel)
    			SendMessage(current->hWnd, CB_SELECTSTRING, 0, (LPARAM)sel);
			break;
		case e_text:
        case e_prependtext:
        case e_separatedtext:
        case e_multitext:
			current->hWnd = CreateWindow("edit","", WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | ES_AUTOHSCROLL,
										 r->left+1, r->top, r->right - r->left-1, r->bottom - r->top-1, parent, 0, hInstance, 0);
            SendMessage(current->hWnd, WM_SETFONT, (WPARAM)SendMessage(lv, WM_GETFONT, 0, 0), 1);
			SendMessage(current->hWnd, WM_SETTEXT, 0, (LPARAM)current->tentative);
			SendMessage(current->hWnd, EM_SETSEL, strlen(current->tentative), strlen(current->tentative));
			break;
		case e_numeric:
			current->hWnd = CreateWindow("edit","", WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | ES_NUMBER,
										 r->left+1, r->top, r->right - r->left-1, r->bottom - r->top-1, parent, 0, hInstance, 0);
            SendMessage(current->hWnd, WM_SETFONT, (WPARAM)SendMessage(lv, WM_GETFONT, 0, 0), 1);
			SendMessage(current->hWnd, WM_SETTEXT, 0, (LPARAM)current->tentative);
			SendMessage(current->hWnd, EM_SETSEL, strlen(current->tentative), strlen(current->tentative));
               SendMessage(current->hWnd, EM_LIMITTEXT, 32, 0);
			break;
		case e_color:
			current->hWnd = CreateWindow("xccColor","", WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS,
										 r->left+1, r->top, r->right - r->left-1, r->bottom - r->top-1, parent, 0, hInstance, 0);
			{
				DWORD val = strtoul(current->tentative, NULL, 0);
				SendMessage(current->hWnd, WM_SETCOLOR, 0, (LPARAM)val);
			}
			break;
		case e_font:
			current->hWnd = CreateWindow("xccFont","", WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS,
										 r->left+1, r->top, r->right - r->left-1, r->bottom - r->top-1, parent, 0, hInstance, 0);
			{
				LOGFONT lf;
				HFONT font;
				ParseFont(&lf, current->tentative);
				font = CreateFontIndirect(&lf);
				SendMessage(current->hWnd, WM_SETFONT, (WPARAM)font, 0);
			}
			break;
		case e_printformat:
			current->hWnd = CreateWindow("xccPrintFormat","", WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS,
										 r->left+1, r->top, r->right - r->left-1, r->bottom - r->top-1, parent, 0, hInstance, 0);
			SendMessage(current->hWnd, WM_SETTEXT, 0, (LPARAM)current->tentative);
			SendMessage(current->hWnd, EM_SETSEL, strlen(current->tentative), strlen(current->tentative));
            SendMessage(current->hWnd, WM_SETFONT, (WPARAM)SendMessage(lv, WM_GETFONT, 0, 0), 1);
			break;
	}
	if (current->hWnd)
	{
		SetParent(current->hWnd, lv);
	}
}

static void SaveItemData(SETTING *current)
{
	if (current->hWnd)
	{
		switch (current->type)
		{
			DWORD color;
			HFONT font;
			LOGFONT lf;
			BYTE *p;
            char *q;
			int i;
            char buf[1024];
    		SETTINGCOMBO *sc;
			
			case e_combo:
       				buf[SendMessage(current->hWnd, WM_GETTEXT, 1024, (LPARAM)buf)] = 0;
                    sc = current->combo;
                    while (sc)
                    {
                        if (!strcmp(sc->displayName, buf))
                        {
                            q= strdup(sc->value);
                            if (q)
                            {
            		     		free(current->tentative);
            	     			current->tentative = q;
                            }
                            break;
                        }
                        sc = sc->next;
                    }
                    break;
			case e_printformat:
			case e_text:
			case e_numeric:
            case e_prependtext:
            case e_separatedtext:
            case e_multitext:
                    i = SendMessage(current->hWnd, WM_GETTEXTLENGTH, 0, 0);
                    q= malloc(i+1);
                    if (q)
                    {
         				SendMessage(current->hWnd, WM_GETTEXT, i+1, (LPARAM)q);
                        q[i] = 0;
    		     		free(current->tentative);
    	     			current->tentative = q;
                    }
				break;
			case e_font:
				font = (HFONT)SendMessage(current->hWnd, WM_GETFONT, 0, 0);
				GetObject(font, sizeof(lf), &lf);
				p = (BYTE *)&lf;
				buf[0] = 0;
				for (i=0; i < sizeof(lf); i++)
					sprintf(buf + strlen(buf), "%d ", p[i]);
				free(current->tentative);
				current->tentative = strdup(buf);
				break;
			case e_color:
				color = SendMessage(current->hWnd, WM_RETRIEVECOLOR, 0, 0);
				sprintf(buf,"%d", color);
				free(current->tentative);
				current->tentative = strdup(buf);
				break;
		}
	}
}
static void DestroyItemWindow(SETTING *current)
{
	if (current->hWnd)
	{
		SaveItemData(current);
		DestroyWindow(current->hWnd);
		current->hWnd = 0;
	}
}
static void SaveItemDatas(SETTING *current)
{
	if (current)
	{
		current = current->children;
		while (current)
		{
			SaveItemData(current);
			current = current->next;
		}
	}
}
static void DestroyItemWindows(SETTING *current)
{
	if (current)
	{
		current = current->children;
		while (current)
		{
			DestroyItemWindow(current);
			current = current->next;
		}
	}
}
static void PopulateItems(HWND parent, HWND hwnd, SETTING *settings)
{
    LV_ITEM item;
	int items = 0;
	ListView_DeleteAllItems(hwnd);
	settings = settings->children;
	while (settings)
	{
		memset(&item, 0, sizeof(item));
		if (settings->type != e_tree)
			if (settings->type == e_separator)
			{
	            item.iItem = items++;
	            item.iSubItem = 0;
	            item.mask = LVIF_PARAM | LVIF_TEXT;
	            item.lParam = (LPARAM)settings;
	            item.pszText = "";
				item.cchTextMax = 0;
	            ListView_InsertItem(hwnd, &item);
			}
			else
			{
				RECT r;
	            item.iItem = items++;
	            item.iSubItem = 0;
	            item.mask = LVIF_PARAM | LVIF_TEXT;
	            item.lParam = (LPARAM)settings;
	            item.pszText = settings->displayName;
				item.cchTextMax = strlen(settings->displayName);
	            ListView_InsertItem(hwnd, &item);
				ListView_GetSubItemRect(hwnd, item.iItem, 1, LVIR_BOUNDS, &r);
				CreateItemWindow(parent, hwnd, &r, settings);
			}
		settings = settings->next;
	}
}
static int TreeCustomDraw(HWND hwnd, LPNMTVCUSTOMDRAW draw)
{
    switch(draw->nmcd.dwDrawStage)
	{
	    case CDDS_PREPAINT :
	        return CDRF_NOTIFYITEMDRAW;
	    case CDDS_ITEMPREPAINT:
			if (draw->nmcd.uItemState & (CDIS_SELECTED ))
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
			return CDRF_NEWFONT;
		default:
			return CDRF_DODEFAULT;
	}
}
static int LVCustomDraw(HWND hwnd, LPNMLVCUSTOMDRAW draw)
{
    switch(draw->nmcd.dwDrawStage)
	{
	    case CDDS_PREPAINT :
	        return CDRF_NOTIFYITEMDRAW;
		case CDDS_ITEMPREPAINT:
			draw->clrText = RetrieveSysColor(COLOR_WINDOWTEXT);
			draw->clrTextBk = RetrieveSysColor(COLOR_WINDOW);
			return CDRF_NEWFONT;
	    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
			draw->clrText = RetrieveSysColor(COLOR_WINDOWTEXT);
			draw->clrTextBk = RetrieveSysColor(COLOR_WINDOW);
			return CDRF_NEWFONT;
		default:
			return CDRF_DODEFAULT;
	}
}
void SetupCurrentValues(SETTING *setting, PROJECTITEM *saveTo)
{
	while (setting)
	{
		if (setting->type == e_tree)
		{
			SetupCurrentValues(setting->children, saveTo);
		}
		else if (setting->type != e_separator)
		{
            SETTING *value;
            EnterCriticalSection(&propsMutex);
            PropSearchProtos(saveTo, setting->id, &value);
            free(setting->tentative);
            setting->tentative = strdup(value->value);
            LeaveCriticalSection(&propsMutex);
		}
		setting = setting->next;
	}
}
void ApplyCurrentValues(SETTING *setting, PROJECTITEM *saveTo)
{
	while (setting)
	{
		if (setting->type == e_tree)
		{
			ApplyCurrentValues(setting->children, saveTo);
		}
		else if (setting->type != e_separator)
		{
            SETTING *value, *empty;
            EnterCriticalSection(&propsMutex);
            value = PropSearchProtos(saveTo, setting->id, &empty);
            if (value && !strcmp(value->value, setting->tentative))
            {
                SETTING **s = &saveTo->settings;
                while (*s)
                {
                    if (!strcmp((*s)->id, setting->id))
                    {
                        SETTING *p = *s;
                        *s = (*s)->next;
                        free(p);
                        MarkChanged(saveTo, FALSE);
                        saveTo->clean = TRUE;
                        break;
                    }
                    s = &(*s)->next;
                }
            }
            else
            {
                SETTING *s = PropFind(saveTo->settings, setting->id);
                if (!s)
                {
                    s = calloc(sizeof(SETTING),1);
                    s->type = e_text;
                    s->id = strdup(setting->id);
                    s->next = saveTo->settings;
                    saveTo->settings = s;
                }
                if (s)
                {
                    free(s->value);
                    s->value = strdup(setting->tentative);
                    MarkChanged(saveTo, FALSE);
                    saveTo->clean = TRUE;
                }
            }
            LeaveCriticalSection(&propsMutex);
		}
		setting = setting->next;
	}
}
void RemoveCurrentValues(SETTING *setting)
{
	while (setting)
	{
		if (setting->type == e_tree)
		{
			RemoveCurrentValues(setting->children);
		}
		else if (setting->type != e_separator)
		{
            EnterCriticalSection(&propsMutex);
			free(setting->tentative);
			setting->tentative = NULL;
            LeaveCriticalSection(&propsMutex);
		}
		setting = setting->next;
	}
	
}
static LRESULT CALLBACK GeneralWndProc(HWND hwnd, UINT iMessage,
    WPARAM wParam, LPARAM lParam)
{
	static HWND hwndTree;
	static HWND hwndLV;
	static SETTING *current;
	static HWND hAcceptBtn;
	static HWND hApplyBtn;
	static HWND hCancelBtn;
    static HWND hHelpBtn;
	static BOOL populating;
    static HWND hwndTTip;
	LPCREATESTRUCT cs;
	RECT r;
	POINT acceptPt;
	POINT cancelPt;
    POINT helpPt;
	static POINT pt;
	HDC dc;
    struct _propsData *pd;
    int i;
	
	switch (iMessage)
	{
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->code)
			{
				NM_TREEVIEW *nm;
    			TV_ITEM xx ;
				LPNMCUSTOMDRAW cd;
				SETTING *st;
				case LVN_ITEMCHANGING:
					return TRUE; // disable selection
				case NM_CUSTOMDRAW:
					cd = (LPNMCUSTOMDRAW)lParam;
					if (cd->hdr.hwndFrom == hwndTree)
					{
						return TreeCustomDraw(hwndTree, (LPNMTVCUSTOMDRAW)lParam);
					}
					else
					{
						return LVCustomDraw(hwndLV, (LPNMLVCUSTOMDRAW)lParam);
					}
	            case TVN_SELCHANGED:
	                nm = (NM_TREEVIEW*)lParam;
	                xx.hItem = nm->itemNew.hItem;
				    xx.mask = TVIF_PARAM ;
    				if (SendMessage(hwndTree, TVM_GETITEM,0 ,(LPARAM)&xx))
					{
						DestroyItemWindows(current);
        				current = (SETTING *)xx.lParam ;
               			EnableWindow(hHelpBtn, current->helpid != 0);

						populating = TRUE;
						PopulateItems(hwnd, hwndLV, current);
						populating = FALSE;
					}
	                return 0;
			}
			break;
		case WM_COMMAND:
            pd = (struct propsData *)GetWindowLong(hwnd, GWL_USERDATA);
			switch(LOWORD(wParam))
			{
                case IDHELP:
                    ContextHelp(current->helpid);
                    break;
				case IDOK:
					SaveItemDatas(current);
                    for (i=0;i < pd->protocount; i++)
    					ApplyCurrentValues(pd->prototype[i], pd->saveTo);
                    if (pd == &generalProps)
                    {
                        FreeBrowseInfo();
                    	FreeJumpSymbols();
                    	LoadJumpSymbols();
    					SavePreferences();
    					ApplyEditorSettings();
                    }
					SendMessage(hwnd, WM_CLOSE, 0,0);
					break;
				case IDCANCEL:
					SendMessage(hwnd, WM_CLOSE, 0,0);
					break;
				case IDC_AUXBUTTON:
					SaveItemDatas(current);
                    for (i=0;i < pd->protocount; i++)
    					ApplyCurrentValues(pd->prototype[i], pd->saveTo);
                    if (pd == &generalProps)
                    {
                        FreeBrowseInfo();
                    	FreeJumpSymbols();
                    	LoadJumpSymbols();
    					SavePreferences();
    					ApplyEditorSettings();
                    }
					EnableWindow(hAcceptBtn, FALSE);
					EnableWindow(hApplyBtn, FALSE);
					break;
				default:
					if (!populating && HIWORD(wParam) == EN_CHANGE)
					{
						EnableWindow(hAcceptBtn, TRUE);
						EnableWindow(hApplyBtn, TRUE);
					}
                         else if (HIWORD(wParam) == EN_KILLFOCUS)
                         {
                              int style = GetWindowLong((HWND)lParam, GWL_STYLE);
                              if (style & ES_NUMBER)
                              {
                                   char buf[256];
                                   SETTING *s = current->children;
                                   GetWindowText((HWND)lParam, buf, 256);
                                   while (s)
                                   {
                                        if (s->hWnd == (HWND)lParam)
                                             break;
                                        s = s->next;
                                   }
                                   if (s)
                                   {
                                        if (buf[0])
                                        {
                                             int low, high;
                                             int n = strtoul(buf, NULL, 0);
                                             low = strtoul(s->lowLimit, NULL, 0);
                                             high = strtoul(s->highLimit, NULL, 0);
                                             if (n < low || n > high)
                                             {
                                                 TOOLINFO t;
                                                 static char text[256];
                                                 RECT r, r1;
                                                 SetFocus((HWND)lParam);
                                                 sprintf(text, "The numeric range of this tool is %d to %d", low, high);
                                                 GetWindowRect(s->hWnd, &r1);
                                                 GetWindowRect(hwndLV, &r);
                                                 OffsetRect(&r1, -r.left, -r.top);
                                                 memset(&t, 0, sizeof(t));
                                                 t.cbSize = sizeof(t);
                                                 t.uFlags = TTF_SUBCLASS | TTF_ABSOLUTE | TTF_TRACK;
                                                 t.hwnd = hwndLV;
                                                 t.uId = 1000;
                                                 t.rect = r1;
//                                                 t.rect.left = 0;
//                                                 t.rect.right = 100;
//                                                 t.rect.top = 0;
//                                                 t.rect.bottom = 32;
                                                 t.lpszText = text;
                                                 SendMessage(hwndTTip, TTM_DELTOOL, 0, (LPARAM) &t);
                                                 SendMessage(hwndTTip, TTM_ADDTOOL, 0, (LPARAM) &t);
                                                 SendMessage(hwndTTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 1000);
                                                 SendMessage(hwndTTip, TTM_SETDELAYTIME, TTDT_INITIAL, 0);
                                                 SendMessage(hwndTTip, TTM_ACTIVATE, 0, 0);
                                                 SendMessage(hwndTTip, TTM_TRACKACTIVATE, TRUE, 0);
                                                 SendMessage(hwndTTip, TTM_TRACKPOSITION, 0, MAKELPARAM(r1.bottom,r1.left + 10));
                                                 SendMessage(hwndTTip, TTM_POPUP, 0, 0);
                                             }
                                        }
                                        else
                                        {
                                             SetWindowText((HWND)lParam, s->value);
                                        }
                                   }
                              }
                         }
					break;
			}
			break;
		case WM_CREATE:
            cs = ((LPCREATESTRUCT)lParam);
            pd = (struct _propsData *)cs->lpCreateParams;
            SetWindowLong(hwnd, GWL_USERDATA, (long)pd);
			populating = FALSE;
			current = NULL;
            hwndTTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
                                        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
                                        CW_USEDEFAULT, CW_USEDEFAULT,
                                        CW_USEDEFAULT, CW_USEDEFAULT,
                                        hwnd, NULL, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
                                        NULL);
            
            SetWindowPos(hwndTTip, HWND_TOPMOST,0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			CenterWindow(hwnd);
			GetClientRect(hwnd, &r);
            hwndTree = CreateWindowEx(0, WC_TREEVIEW, "", WS_VISIBLE | WS_BORDER | 
                WS_CHILD | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_TRACKSELECT,
                0, 0, (r.right - r.left)/3, r.bottom - r.top-32, hwnd, 0, hInstance, NULL);
            hwndLV = CreateWindowEx(0, WC_LISTVIEW, "", WS_VISIBLE | WS_BORDER |
				LVS_REPORT | LVS_SINGLESEL | WS_CHILD,
                (r.right - r.left)/3, 0, (r.right - r.left)*2/3, r.bottom - r.top-32, hwnd, 0, hInstance, NULL);
			dc = GetDC(hwnd);
			GetTextExtentPoint32(dc, szAccept, strlen(szAccept), &acceptPt);
			GetTextExtentPoint32(dc, szClose, strlen(szClose), &cancelPt);
			GetTextExtentPoint32(dc, szApply, strlen(szApply), &pt);
			GetTextExtentPoint32(dc, szHelp, strlen(szClose), &helpPt);
			ReleaseDC(hwnd, dc);
			if (acceptPt.x > pt.x)
				pt.x = acceptPt.x;
			if (acceptPt.y > pt.y)
				pt.y = acceptPt.y;
			if (cancelPt.x > pt.x)
				pt.x = cancelPt.x;
			if (cancelPt.y > pt.y)
				pt.y = cancelPt.y;
			if (helpPt.x > pt.x)
				pt.x = helpPt.x;
			if (helpPt.y > pt.y)
				pt.y = helpPt.y;
			hAcceptBtn = CreateWindowEx(0, "button", szAccept, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
										r.right - 1 * (pt.x + 20), r.bottom -pt.y - 12, pt.x+12, pt.y + 8,
										hwnd, (HMENU)IDOK, hInstance, NULL);
			hCancelBtn = CreateWindowEx(0, "button", szClose, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
										r.right - 2 * (pt.x + 20), r.bottom -pt.y - 12, pt.x+12, pt.y + 8,
										hwnd, (HMENU)IDCANCEL, hInstance, NULL);
			hApplyBtn = CreateWindowEx(0, "button", szApply, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
										r.right - 3 * (pt.x + 20), r.bottom -pt.y - 12, pt.x+12, pt.y + 8,
										hwnd, (HMENU)IDC_AUXBUTTON, hInstance, NULL);
			hHelpBtn = CreateWindowEx(0, "button", szHelp, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
										r.right - 5 * (pt.x + 20), r.bottom -pt.y - 12, pt.x+12, pt.y + 8,
										hwnd, (HMENU)IDHELP, hInstance, NULL);
			EnableWindow(hAcceptBtn, FALSE);
			EnableWindow(hApplyBtn, FALSE);
			EnableWindow(hHelpBtn, FALSE);
            for (i=0;i < pd->protocount; i++)
    			SetupCurrentValues(pd->prototype[i], pd->saveTo);
			CreateLVColumns(hwndLV, (r.right - r.left)*2/3);
            for (i=0;i < pd->protocount; i++)
    			PopulateTree(hwndTree, TVI_ROOT, pd->prototype[i]->children);
			return 0;
		case WM_DESTROY:
            pd = (struct propsData *)GetWindowLong(hwnd, GWL_USERDATA);
			DestroyItemWindows(current);
			RemoveCurrentValues(pd->prototype);
            if (pd != &generalProps)
            {
                free(pd->prototype);
                free(pd);
            }
			DestroyWindow(hwndLV);
			DestroyWindow(hwndTree);
			DestroyWindow(hAcceptBtn);
			DestroyWindow(hCancelBtn);
			DestroyWindow(hApplyBtn);
			break;
		case WM_SIZE:
			if (current)
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				MoveWindow(hwndTree, 0, 0, x/3, y-32, 1);
				MoveWindow(hwndLV, x/3, 0, x*2/3, y-32, 1);
				MoveWindow(hAcceptBtn, x - 3 * (pt.x + 20), y -pt.y - 12, pt.x+12, pt.y + 8, 1);
				MoveWindow(hCancelBtn, x - 2 * (pt.x + 20), y -pt.y - 12, pt.x+12, pt.y + 8, 1);
				MoveWindow(hApplyBtn, x - 1 * (pt.x + 20), y -pt.y - 12, pt.x+12, pt.y + 8, 1);
				CreateLVColumns(hwndLV, x*2/3);
				DestroyItemWindows(current);
				PopulateItems(hwnd, hwndLV, current);
			}
			break;
	}
	return DefWindowProc(hwnd, iMessage, wParam, lParam);
}
struct buttonWindow
{
	HWND parent;
	HWND button;
	HWND edit;
	HWND list;
	HFONT font;
	int lfHeight;
	DWORD color;
} ;
struct buttonWindow *CreateButtonWnd(HWND parent, BOOL staticText, BOOL combo)
{
	struct buttonWindow *ptr = calloc(1, sizeof(struct buttonWindow));
	if (ptr)
	{
		RECT r;
		GetClientRect(parent, &r);
		ptr->button = CreateWindow("button", "", WS_CHILD | WS_VISIBLE | BS_BITMAP | BS_FLAT,
								   r.right-18, r.top, 18, r.bottom - r.top, parent, (HMENU)IDC_AUXBUTTON, hInstance, NULL);
		if (combo)
			SendMessage(ptr->button, BM_SETIMAGE, 0, (LPARAM)comboButtonBM);
		else
			SendMessage(ptr->button, BM_SETIMAGE, 0, (LPARAM)menuButtonBM);
		if (staticText)
		{
			ptr->edit = CreateWindow("static", "", WS_CHILD | WS_VISIBLE,
									 r.left, r.top, r.right - r.left - 18, r.bottom - r.top, parent, 0, hInstance, NULL);
		}		
		else
		{
			ptr->edit = CreateWindow("edit", "", WS_CHILD | WS_VISIBLE,
									 r.left, r.top, r.right - r.left - 18, r.bottom - r.top, parent, 0, hInstance, NULL);
		}
		ptr->parent = GetParent(parent);
	}
	return ptr;
}
static LRESULT CALLBACK ColorWndProc(HWND hwnd, UINT iMessage,
    WPARAM wParam, LPARAM lParam)
{
	struct buttonWindow *ptr;
    if (iMessage == helpMsg)
        ContextHelp(IDH_CHOOSE_COLOR_DIALOG);
	else switch(iMessage)
	{
		RECT r;
		HBRUSH brush;
	    CHOOSECOLOR c;
		case WM_LBUTTONUP:
			SendMessage(hwnd, WM_COMMAND, IDC_AUXBUTTON, 0);
			break;
		case WM_COMMAND:
			ptr = (struct buttonWindow *)GetWindowLong(hwnd, 0);
			if (LOWORD(wParam) == IDC_AUXBUTTON)
			{
	            memset(&c, 0, sizeof(c));
	            c.lStructSize = sizeof(CHOOSECOLOR);
	            c.hwndOwner = hwnd;
	            c.rgbResult = GetWindowLong(hwnd, 0);
	            c.lpCustColors = custColors;
	            c.Flags = CC_RGBINIT | CC_SHOWHELP;
	            if (ChooseColor(&c))
	            {
					ptr->color = c.rgbResult;
	                InvalidateRect(ptr->edit, 0, 0);
					SendMessage(ptr->parent, WM_COMMAND, EN_CHANGE << 16, 0);
	            }
	            else
	                CommDlgExtendedError();
			}
			break;
		case WM_CTLCOLORBTN:
		{
			HBRUSH br = CreateSolidBrush(RetrieveSysColor(COLOR_WINDOW));
			return br;
		}
        case WM_CTLCOLORSTATIC:
		{
            HBRUSH br;
			ptr = (struct buttonWindow *)GetWindowLong(hwnd, 0);
			br = CreateSolidBrush(ptr->color);
			return (LRESULT)br;
		}
		case WM_SETCOLOR:
			ptr = (struct buttonWindow *)GetWindowLong(hwnd, 0);
			ptr->color = lParam;
            InvalidateRect(hwnd, 0, 0);
            break;
        case WM_RETRIEVECOLOR:
			ptr = (struct buttonWindow *)GetWindowLong(hwnd, 0);
            return ptr->color;
        case WM_CREATE:
			ptr = CreateButtonWnd(hwnd, TRUE, FALSE);
			SetWindowLong(hwnd, 0, (long)ptr);
            InvalidateRect(hwnd, 0, 0);
            break;
	}
	return DefWindowProc(hwnd, iMessage, wParam, lParam);
}
static LRESULT CALLBACK FontWndProc(HWND hwnd, UINT iMessage,
    WPARAM wParam, LPARAM lParam)
{
	struct buttonWindow *ptr;
    if (iMessage == helpMsg)
        ContextHelp(IDH_CHOOSE_FONT_DIALOG);
	else switch(iMessage)
	{
		HFONT hFont;
		LOGFONT lf;
		NONCLIENTMETRICS NonClientMetrics;
		case WM_CREATE:
			ptr = CreateButtonWnd(hwnd, TRUE, FALSE);
			SetWindowLong(hwnd, 0, (long)ptr);
  			NonClientMetrics.cbSize = sizeof(NonClientMetrics);
	  		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &NonClientMetrics, 0);
	  		ptr->font = CreateFontIndirect(&NonClientMetrics.lfMessageFont);
			ptr->lfHeight = NonClientMetrics.lfMenuFont.lfHeight;
			SendMessage(hwnd, WM_COMMAND, IDC_SETFONTNAME, 0);
			break;
		case WM_COMMAND:
			ptr = (struct buttonWindow *)GetWindowLong(hwnd, 0);
			switch (LOWORD(wParam))
			{
				CHOOSEFONT ch;
				LOGFONT lf;
				case IDC_AUXBUTTON:
	                memset(&ch, 0, sizeof(ch));
					GetObject(ptr->font, sizeof(lf), &lf);
	                ch.lStructSize = sizeof(ch);
	                ch.hwndOwner = hwnd;
	                ch.lpLogFont = &lf;
	                ch.Flags = CF_FIXEDPITCHONLY | CF_FORCEFONTEXIST |
	                    CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_SHOWHELP ;
	                if (ChooseFont(&ch))
					{
						ptr->font = CreateFontIndirect(&lf);
						SendMessage(ptr->parent, WM_COMMAND, EN_CHANGE << 16, 0);
						SendMessage(hwnd, WM_COMMAND, IDC_SETFONTNAME, 0);
					}
					break;
				case IDC_SETFONTNAME:
				{
					LOGFONT lf;
					HFONT font, oldFont;
					char buf[256];
					HDC dc;
					GetObject(ptr->font, sizeof(lf), &lf);
					dc = GetDC(hwnd);
					sprintf(buf,"%s (%d)", lf.lfFaceName, MulDiv(-lf.lfHeight, 72 ,GetDeviceCaps(dc, LOGPIXELSX)));
					ReleaseDC(hwnd, dc);
					lf.lfHeight = ptr->lfHeight;
					font = CreateFontIndirect(&lf);
					oldFont = (HFONT)SendMessage(ptr->edit, WM_GETFONT, 0, 0);
					SendMessage(ptr->edit, WM_SETFONT, (WPARAM)font, 0);
					DeleteObject(oldFont);
					SendMessage(ptr->edit, WM_SETTEXT, 0, (LPARAM)buf);
					break;
				}
			}
			break;
		case WM_SETFONT:
				ptr = (struct buttonWindow *)GetWindowLong(hwnd, 0);
				ptr->font = (HFONT)wParam;
				SendMessage(hwnd, WM_COMMAND, IDC_SETFONTNAME, 0);
			break;
		case WM_GETFONT:
			ptr = (struct buttonWindow *)GetWindowLong(hwnd, 0);
			return ptr->font;
		case WM_CTLCOLORBTN:
        case WM_CTLCOLORSTATIC:
		{
			HBRUSH br = CreateSolidBrush(RetrieveSysColor(COLOR_WINDOW));
			SetBkColor((HDC)wParam, RetrieveSysColor(COLOR_WINDOW));
			SetTextColor((HDC)wParam, RetrieveSysColor(COLOR_WINDOWTEXT));
			return (LRESULT)br;
		}
		case WM_DESTROY:
			ptr = (struct buttonWindow *)GetWindowLong(hwnd, 0);
			DestroyWindow(ptr->edit);
			DestroyWindow(ptr->button);
			DeleteObject(ptr->font);
			free(ptr);
			break;
			
	}
	return DefWindowProc(hwnd, iMessage, wParam, lParam);
}

static LRESULT CALLBACK PrintFormatWndProc(HWND hwnd, UINT iMessage,
    WPARAM wParam, LPARAM lParam)
{
	struct buttonWindow *ptr;
	switch(iMessage)
	{
		char *insert;
		case WM_CREATE:
			SetWindowLong(hwnd, 0, (long)CreateButtonWnd(hwnd, FALSE, FALSE));
			break;
		case WM_COMMAND:
			insert = NULL;
			switch (LOWORD(wParam))
			{
				case IDC_AUXBUTTON:
				{
            		HMENU menu = LoadMenuGeneric(hInstance, "PRINTFORMATMENU"), popup;
					RECT r;
					GetWindowRect(hwnd, &r);
	                popup = GetSubMenu(menu, 0);
    			    TrackPopupMenuEx(popup, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, r.right,
            		    r.top, hwnd, NULL);
            		DestroyMenu(menu);
				}
					break;
				case IDM_PF12HR:
					insert = "%T";
					break;
				case IDM_PF24HR:
					insert = "%t";
					break;
				case IDM_PFUSADATE:
					insert = "%D";
					break;
				case IDM_PFEURODATE:
					insert = "%d";
					break;
				case IDM_PFCURPAGE:
					insert = "%P";
					break;
				case IDM_PFNUMPAGE:
					insert = "%#";
					break;
				case IDM_PFFILENAME:
					insert = "%F";
					break;
				case IDM_PFCENTER:
					insert = "%C";
					break;
				case IDM_PFLEFT:
					insert = "%L";
					break;
				case IDM_PFRIGHT:
					insert = "%R";
					break;
				default:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						ptr = (struct buttonWindow *)GetWindowLong(hwnd, 0);
						SendMessage(ptr->parent, WM_COMMAND, EN_CHANGE << 16, 0);
					}
					break;
			}
			if (insert)
			{
				ptr = (struct buttonWindow *)GetWindowLong(hwnd, 0);
    	            SendMessage(ptr->edit, 
						EM_REPLACESEL, 1, (LPARAM)insert);
					SendMessage(ptr->parent, WM_COMMAND, EN_CHANGE << 16, 0);
			}
			break;
		case WM_SETTEXT:
		case WM_GETTEXT:
        case WM_GETTEXTLENGTH:
		case EM_SETSEL:
		case WM_SETFONT:
		case WM_GETFONT:
			ptr = (struct buttonWindow *)GetWindowLong(hwnd, 0);
			return SendMessage(ptr->edit, iMessage, wParam, lParam);

		case WM_CTLCOLORBTN:
		{
			HBRUSH br = CreateSolidBrush(RetrieveSysColor(COLOR_WINDOW));
			return (LRESULT)br;
		}
		case WM_DESTROY:
			ptr = (struct buttonWindow*)GetWindowLong(hwnd, 0);
			DestroyWindow(ptr->edit);
			DestroyWindow(ptr->button);
			DeleteObject(ptr->font);
			free(ptr);
			break;
			
	}
	return DefWindowProc(hwnd, iMessage, wParam, lParam);
}
static LRESULT CALLBACK ComboWndProc(HWND hwnd, UINT iMessage,
    WPARAM wParam, LPARAM lParam)
{
	struct buttonWindow *ptr;
	switch(iMessage)
	{
		RECT r;
		case WM_CREATE:
			ptr = CreateButtonWnd(hwnd, TRUE, TRUE);
			SetWindowLong(hwnd, 0, (long)ptr);
			GetClientRect(hwnd, &r);
			ptr->list = CreateWindow("listbox",0,WS_CHILD | LBS_NOTIFY | WS_VSCROLL | WS_BORDER, 
									 	r.left, r.bottom, 500, r.right-r.left,
										hwnd, (HMENU)IDC_LISTBOX, hInstance, 0);
			break;
		case WM_COMMAND:
			ptr = (struct buttonWindow*)GetWindowLong(hwnd, 0);
			switch (LOWORD(wParam))
			{
				case IDC_LISTBOX:
					if (HIWORD(wParam) == LBN_SELCHANGE)
					{
						int n = SendMessage(ptr->list, LB_GETCURSEL, 0, 0);
						char buf[256];
						SendMessage(ptr->list, LB_GETTEXT, n, (LPARAM)buf);
						SendMessage(ptr->edit, WM_SETTEXT, 0, (LPARAM)buf);
						SendMessage(ptr->parent, WM_COMMAND, EN_CHANGE << 16, 0);
					}
					else
						break;
					// fallthrough
				case IDC_AUXBUTTON:
				{
					GetClientRect(hwnd, &r);
					if (IsWindowVisible(ptr->list))
					{
						SetWindowPos(hwnd, 0, r.left, r.top, r.right-r.left, r.bottom - r.top - 500, SWP_NOACTIVATE | SWP_NOMOVE);
					}
					else
					{
						SetWindowPos(hwnd, 0, r.left, r.top, r.right-r.left, r.bottom - r.top + 500, SWP_NOACTIVATE | SWP_NOMOVE);
					}
				    SetWindowPos(ptr->list, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOMOVE | (IsWindowVisible(ptr->list) ? SWP_HIDEWINDOW : SWP_SHOWWINDOW));
					break;
				}
			}
			break;
		case WM_LBUTTONDOWN:
			SendMessage(hwnd, WM_COMMAND, IDC_AUXBUTTON, 0);
			break;
		case CB_ADDSTRING:
			ptr = (struct buttonWindow*)GetWindowLong(hwnd, 0);
			SendMessage(ptr->list, LB_ADDSTRING, wParam, lParam);
			break;
		case CB_SELECTSTRING:
		case WM_SETTEXT:
			ptr = (struct buttonWindow*)GetWindowLong(hwnd, 0);
			SendMessage(ptr->edit, WM_SETTEXT, wParam, lParam);
			break;
		case WM_GETTEXT:
			ptr = (struct buttonWindow*)GetWindowLong(hwnd, 0);
			return SendMessage(ptr->edit, WM_GETTEXT, wParam, lParam);
        case WM_GETTEXTLENGTH:
			ptr = (struct buttonWindow*)GetWindowLong(hwnd, 0);
			return SendMessage(ptr->edit, WM_GETTEXTLENGTH, wParam, lParam);
		case WM_CTLCOLORBTN:
        case WM_CTLCOLORSTATIC:
		{
			HBRUSH br = CreateSolidBrush(RetrieveSysColor(COLOR_WINDOW));
			SetBkColor((HDC)wParam, RetrieveSysColor(COLOR_WINDOW));
			SetTextColor((HDC)wParam, RetrieveSysColor(COLOR_WINDOWTEXT));
			return (LRESULT)br;
		}
		case WM_DESTROY:
			ptr = (struct buttonWindow*)GetWindowLong(hwnd, 0);
			DestroyWindow(ptr->edit);
			DestroyWindow(ptr->button);
			DestroyWindow(ptr->list);
			free(ptr);
			break;
        case WM_SETFONT:
			ptr = (struct buttonWindow*)GetWindowLong(hwnd, 0);
            SendMessage(ptr->edit, iMessage, wParam, lParam);        
            SendMessage(ptr->list, iMessage, wParam, lParam);        
            break;
	}
	return DefWindowProc(hwnd, iMessage, wParam, lParam);
}		

void RegisterPropWindows(HINSTANCE hInstance)
{

    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = 0;
    wc.lpfnWndProc = &GeneralWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(LPVOID);
    wc.hInstance = hInstance;
    wc.hIcon = 0; //LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = 0;
    wc.lpszClassName = szGeneralPropsClassName;
    RegisterClass(&wc);
	
	wc.lpfnWndProc = ColorWndProc;
	wc.lpszClassName = "xccColor";
	RegisterClass(&wc);
	wc.lpfnWndProc = FontWndProc;
	wc.lpszClassName = "xccFont";
	RegisterClass(&wc);
	wc.lpfnWndProc = PrintFormatWndProc;
	wc.lpszClassName = "xccPrintFormat";
	RegisterClass(&wc);
	wc.lpfnWndProc = ComboWndProc;
	wc.lpszClassName = "xccCombo";
	RegisterClass(&wc);
	menuButtonBM= LoadBitmap(hInstance, "ID_MENUBM");
	menuButtonBM = ConvertToTransparent(menuButtonBM, 0xc0c0c0);
	comboButtonBM= LoadBitmap(hInstance, "ID_COMBOBM");
	comboButtonBM = ConvertToTransparent(comboButtonBM, 0xc0c0c0);
}
void ShowGeneralProperties(void)
{	

	CreateWindow(szGeneralPropsClassName, generalProps.title, 
				 WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_CHILD,
							 0,0,700,500,
							 hwndFrame, 0, hInstance, (LPVOID)&generalProps);
}
void ShowBuildProperties(PROJECTITEM *projectItem)
{
    SETTING **arr = calloc(sizeof(SETTING *), 100);
    if (arr)
    {
        static char title[512];
        struct _propsData *data = calloc(sizeof(struct _propsData), 1);
        if (!data)
        {
            free(arr);
            return;
        }
        PropGetPrototype(projectItem, data, arr);
        if (data->title != &generalProps.title)
        {
            data->title = title;
            sprintf(title, "Build properties for %s", projectItem->displayName);
        }
    	CreateWindow(szGeneralPropsClassName, data->title, 
				 WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_CHILD,
							 0,0,700,500,
							 hwndFrame, 0, hInstance, (LPVOID)data);
    }
}
SETTING *PropFind(SETTING *props, char *id)
{
	SETTING *rv = NULL;
	while (props && !rv)
	{
		switch (props->type)
		{
			case e_tree:
				rv = PropFind(props->children, id);
		
        		break;
			case e_separator:
				break;
			default:
                if (props->type >= e_values)
    				if (!strcmp(props->id, id))
	    			{
		    			rv = props;
			    	}
				break;
		}
		props = props->next;
	}
	return rv;
}
static SETTING *PropFindAll(SETTING **list, int count, char *id)
{
	int i;
	SETTING *setting = NULL;
	for (i=0; i < count; i++)
		if ((setting = PropFind(list[i], id)) != NULL)
			break;
	return setting;			
}
SETTING *PropSearchProtos(PROJECTITEM *item, char *id, SETTING **value)
{
    SETTING *arr[100];
    struct _propsData data;
	SETTING *setting ;
	int i;
    PropGetPrototype(item, &data, arr);
    EnterCriticalSection(&propsMutex);
    *value = NULL;
    if (!item)
    {
        *value = PropFind(data.saveTo->settings, id);
    }
    else
    {
        while (item && !*value)
        {
            *value = PropFind(item->settings, id);
            item = item->parent;
        }
    }
    setting = PropFindAll(data.prototype, data.protocount, id);
    if (!*value)
	{
        *value = setting;
	}
    LeaveCriticalSection(&propsMutex);
    return setting;
}
void PropGetFont(PROJECTITEM *item, char *id, LPLOGFONT lplf)
{
    SETTING *value;
    SETTING *setting = PropSearchProtos(item, id, &value);
	if (value)
	{
		ParseFont(lplf, value->value);
	}
	else
	{
		ExtendedMessageBox("Unknown setting",0,"unknown setting %s", id);
	}
}
BOOL PropGetBool(PROJECTITEM *item, char *id)
{
    SETTING *value;
    SETTING *setting = PropSearchProtos(item, id, &value);
	if (value)
	{
		return atoi(value->value);
	}
	else
	{
		ExtendedMessageBox("Unknown setting",0,"unknown setting %s", id);
	}
	return 0;
}
int PropGetInt(PROJECTITEM *item, char *id)
{
    SETTING *value;
    SETTING *setting = PropSearchProtos(item, id, &value);
	if (value)
	{
		return strtoul( value->value, NULL, 0);
	}
	else
	{
		ExtendedMessageBox("Unknown setting",0,"unknown setting %s", id);
	}
	return 0;
}
void PropGetString(PROJECTITEM *item, char *id, char *string, int len)
{
    SETTING *value;
    SETTING *setting = PropSearchProtos(item, id, &value);
	if (value)
	{
		strncpy(string, value->value, len - 1);
		string[len-1] = 0;
	}
	else
	{
		ExtendedMessageBox("Unknown setting",0,"unknown setting %s", id);
	}
}
COLORREF PropGetColor(PROJECTITEM *item, char *id)
{
    SETTING *value;
    SETTING *setting = PropSearchProtos(item, id, &value);
	if (value)
	{
		return strtoul( value->value, NULL, 0);
	}
	else
	{
		ExtendedMessageBox("Unknown setting",0,"unknown setting %s", id);
	}
	return 0;
}