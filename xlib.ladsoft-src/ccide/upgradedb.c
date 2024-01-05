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
#include <richedit.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define PROJVERS 5
#define WSPVERS 7
#define NEWPROJVERS 5
#define NEWWSPVERS 5

#include "xml.h"

#define MAX_HISTORY 10
#define TAG_BP 0
#define TAG_BOOKMARK 3
#define TAG_MAX 5

#define BF_DEBUGINFO  1
#define BF_MAPFILE    2
#define BF_COMPILEVIAASM 4
#define BF_CRTDLL 8 /* obsolete */
#define BF_BROWSEINFO 16
#define BF_SHOWWARNINGS 32
#define BF_C99 64
#define BF_ANSI 128
#define BF_SHOWRETURNCODE 0x10000
#define BF_BREAKDLL 0x20000
#define BF_DEBUGEXCEPTION 0x40000
#define BF_DEBUGTOOLTIPS 0x80000
#define BF_HWBP 0x100000
#define BF_CHANGEDTARGET 0x40000000
#define BF_ALIGNSTACK    0x80000000

typedef struct
{
    int flags;
    int oldflags;
    int hidden;
    int flexparams;
    int rowindex; // row for top & bottom, col for other
    int colindex; // col for top & bottom, row for other 
    int hiddenwidth; // adjusted width for top & bottom, height for other
    RECT oldsize;
    RECT position;
    RECT lastposition;
    RECT flexposition;
} CCD_params;

struct tag
{
    struct tag *next;
    int drawnLineno;
    int editingLineno;
    int debugLineno;
    int fileLineno;
    int charpos;
    int enabled;
    void *extra;
};
typedef struct defines
{
    struct defines *next;
    char name[256];
    char value[256];
} DEFINES;
struct tagfile
{
    char *next;
    char name[256];
    struct tag *tagArray[TAG_MAX];
    struct tagchangeln *changedLines;
};
typedef struct dependslist
{
    struct dependslist *next;
    char name[280];
    char title[280];
} DEPENDSLIST;
typedef struct filelist
{
    struct filelist *next;
    char name[256];
    char title[256];
    DEPENDSLIST *depends;
} FILELIST;

typedef struct projdeps
{
        struct projdeps * next;
        struct projlist *proj;
} PROJDEPS;

typedef struct projlist
{
    struct projlist *next;
    char name[256];
    char title[256];
    char projectName[256];
    char outputPath[1024];
    char includePath[1024];
	PROJDEPS *projdeps;
    FILELIST *sourceFiles, *resourceFiles, *otherFiles;
    DEFINES *defines;
    FILETIME timex;
    enum
    {
        BT_CONSOLE, BT_WINDOWS, BT_DLL, BT_LIBRARY, BT_DOS, BT_RAW
    } buildType;
    enum
    {
        LT_STANDARD, LT_LSCRTDLL, LT_NONE, LT_CRTDLL, LT_MSVCRT
    } libType;
    unsigned buildFlags;
	unsigned defineFlags;
    unsigned treeOpenFlags;
    unsigned dbgview;
    int rebuild: 1;
    int changed: 1;
    char cmdline[1024];
    char compileopts[1024];
    char assembleopts[1024];
    char linkopts[1024];
    char libopts[1024];
    char prebuildlabel[81];
    char postbuildlabel[81];
    char prebuildsteps[1024];
    char postbuildsteps[1024];
    char bkNames[4][256];
    unsigned char bkModes[4], bkSizes[4];
}

//-------------------------------------------------------------------------

PROJLIST;
#define MAX_WINDOWS 20

static WINDOWPLACEMENT wp;
static int maximizeClient;
static char *findhist[MAX_HISTORY];
static char *fifdirhist[MAX_HISTORY];
static char *replacehist[MAX_HISTORY];
static char *watchhist[MAX_HISTORY];
static char *memhist[MAX_HISTORY];

static CCD_params d[40];
static int ids[40];
static int dockCount;
static LOGFONT lf;
static int leftmargin;
static int rightmargin;
static int topmargin;
static int bottommargin;
static char printHeader[512];
static char printFooter[512];
static int keywordColor;
static int numberColor;
static int commentColor;
static int stringColor;
static int escapeColor;
static int backgroundColor;
static int textColor;
static int highlightColor;
static int custColors[16];
static int tabspacing;
static int editFlags;
static int memoryWordSize;
static int findflags;
static int replaceflags;
static char *windowNames[MAX_WINDOWS];
static char *windowTitles[MAX_WINDOWS];
static MDICREATESTRUCT mc[MAX_WINDOWS];
static int lineNo[MAX_WINDOWS];
int windowCount;
static struct tagfile *tagFileList;
static int browseInfo;
static int completionEnabled;
static int parenthesisCheck;
static int hbpEnables;
static int hbpNames[4][256];
static int hbpModes[4];
static int hbpSizes[4];
static struct projectData *projectNames = NULL;

static char szWorkspaceName[MAX_PATH];
static char ipathbuf[2048];

static int errs;

static void SaveProject(char *name, PROJLIST *project);
static void SaveWorkSpace(char *name);

//-------------------------------------------------------------------------

static void abspath(char *name, char *path)
{
    char projname[256],  *p,  *nname = name;
	if (!path)
	{
		path = projname;
		GetCurrentDirectory(256, projname);
		strcat(projname,"\\hi");
	}
    if (!path[0])
        return ;
    if (name[0] == 0)
        return ;
    if (name[1] == ':')
        return ;
    strcpy(projname, path);
    p = strrchr(projname, '\\');
    if (!p)
        return ;
    p--;
    if (!strstr(name, "..\\"))
    {
        if (name[0] != '\\')
        {
            strcpy(p + 2, name);
            strcpy(nname, projname);
        }
        return ;
    }
    while (!strncmp(name, "..\\", 3))
    {
        while (p > projname &&  *p-- != '\\')
            ;
        name += 3;
    }
    *++p = '\\';
    p++;
    strcpy(p, name);
    strcpy(nname, projname);
}

//-------------------------------------------------------------------------

static char *relpath(char *name, char *path)
{
    static char projname[256], localname[256];
    char *p = localname,  *q = projname,  *r,  *s;
    if (!path[0])
        return name;
    if (toupper(name[0]) != toupper(path[0]))
        return name;

    strcpy(localname, name);
    strcpy(projname, path);
    r = strrchr(localname, '\\');
    if (r)
        *r++ = 0;
    // r has the point to the file name
    else
        r = localname;
    s = strrchr(projname, '\\');
    if (!s)
        return name;
    if (*s)
        *s = 0;

    while (*p &&  *q && toupper(*p) == toupper(*q))
	{
        p++, q++;
	}
    if (!(*p |  *q))
        return r;
    else if (*(p - 1) == '\\' && *(p - 2) == ':')
        return name;
    else
    {
        int count =  *q != 0;
		if (*q != '\\')
	        while (p > localname &&  *p != '\\')
    	        p--;
        while (*q && (q = strchr(q + 1, '\\')))
            count++;
        projname[0] = 0;
        while (count--)
            strcat(projname, "..\\");
        if (*p)
        {
            strcat(projname, p + 1);
            strcat(projname, "\\");
        }
        strcat(projname, r);
        return projname;
    }
}

//-------------------------------------------------------------------------

static void absincludepath(char *name, char *path)
{
    char *p = ipathbuf,  *dest1 = name,  *dest2 = name;
    strcpy(ipathbuf, name);
    name[0] = 0;
    do
    {
        char *q;
        if (*p == ';')
            p++;
        q = p;
        dest1 = dest2;
        p = strchr(p, ';');
        if (!p)
            p = ipathbuf + strlen(ipathbuf);
        while (q != p)*dest2++ =  *q++;
        *dest2 = 0;
        abspath(dest1, path);
        dest2 = dest1 + strlen(dest1);
        if (*p)
            *dest2++ = ';';
    }
    while (*p)
        ;
}

//-------------------------------------------------------------------------

static char *relincludepath(char *name, char *path)
{
    char *p = name,  *dest1 = ipathbuf,  *dest2 = ipathbuf;
    ipathbuf[0] = 0;
    do
    {
        char *q;
        if (*p == ';')
            p++;
        q = p;
        dest1 = dest2;
        p = strchr(p, ';');
        if (!p)
            p = name + strlen(name);
        while (q != p)*dest2++ =  *q++;
        *dest2 = 0;
        strcpy(dest1, relpath(dest1, path));
        dest2 = dest1 + strlen(dest1);
        if (*p)
            *dest2++ = ';';
    }
    while (*p)
        ;
    return ipathbuf;
}

static PROJLIST *LoadErr(struct xmlNode *root, char *name, PROJLIST *list)
{
    xmlFree(root);
    printf("Load Error - Target File %s is the wrong format", name);
    errs++;
    return 0;
} 
static PROJLIST *NoMemory(struct xmlNode *root, char *list)
{
    xmlFree(root);
    printf("Load Error - out of memory");
    errs++;
    return 0;
} 

static void NoMemoryWS(void)
{
    printf("Load Error - Out of memory");
    errs++;
}

//-------------------------------------------------------------------------

static int IsNode(struct xmlNode *node, char *name)
{
    return (!strcmp(node->elementType, name));
} 
static int IsAttrib(struct xmlAttr *attr, char *name)
{
    return (!strcmp(attr->name, name));
} 
static void addcr(char *buf)
{
    char buf2[2048],  *p = buf2;
    strcpy(buf2, buf);
    while (*p)
    {
        if (*p == '\n')
            *p++ = '\r';
        *buf++ =  *p++;
    }
    *buf = 0;


}

//-------------------------------------------------------------------------

static void RestorePlacement(struct xmlNode *node, int version)
{
    struct xmlAttr *attribs = node->attribs;
    while (attribs)
    {
        if (IsAttrib(attribs, "VALUE"))
        {
            wp.length = sizeof(WINDOWPLACEMENT);
            sscanf(attribs->value, "%d %d %d %d %d %d %d %d %d %d %d",  &wp.flags,
                &wp.showCmd,  &wp.ptMinPosition.x, &wp.ptMinPosition.y, 
                &wp.ptMaxPosition.x, &wp.ptMaxPosition.y, 
                &wp.rcNormalPosition.left, &wp.rcNormalPosition.top, 
                &wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom, &maximizeClient);
            wp.flags = 0;
        } 
		attribs = attribs->next;
    }
}

//-------------------------------------------------------------------------

static void RestoreDocks(struct xmlNode *node, int version)
{
    int count = 0;
    struct xmlNode *dchildren = node->children;
    while (dchildren && count < 20)
    {
        if (IsNode(dchildren, "DOCK"))
        {
            struct xmlAttr *attribs = dchildren->attribs;
            while (attribs)
            {
                if (IsAttrib(attribs, "ID"))
                    ids[count] = atoi(attribs->value);
                else if (IsAttrib(attribs, "VALUE"))
                {
                    sscanf(attribs->value, 
                        "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d>\n",  
                        &d[count].flags, &d[count].flexparams,  
                        &d[count].rowindex, &d[count].colindex,  
                        &d[count].hidden, &d[count].hiddenwidth,  
                        &d[count].oldsize.left, &d[count].oldsize.top, 
                        &d[count].oldsize.right, &d[count].oldsize.bottom,  
                        &d[count].position.left, &d[count].position.top, 
                        &d[count].position.right, &d[count].position.bottom,  
                        &d[count].lastposition.left, &d[count].lastposition.top, 
                        &d[count].lastposition.right, &d[count].lastposition.bottom);						
                } attribs = attribs->next;
            }
            count++;
        }
        dchildren = dchildren->next;
    }
    dockCount = count;
}

//-------------------------------------------------------------------------

static void RestoreHistory(struct xmlNode *node, int version)
{
    int count = 0;
    char buf[256];
    char **histitem = 0;
    struct xmlAttr *attribs = node->attribs;
    struct xmlNode *children = node->children;
    while (attribs)
    {
        if (IsAttrib(attribs, "TYPE"))
        {
            if (!strcmp(attribs->value, "FIND"))
                histitem = findhist;
            else if (!strcmp(attribs->value, "FIFDIR"))
                histitem = fifdirhist;
            else if (!strcmp(attribs->value, "REPLACE"))
                histitem = replacehist;
            else if (!strcmp(attribs->value, "WATCH"))
                histitem = watchhist;
            else if (!strcmp(attribs->value, "MEMORY"))
                histitem = memhist;
        } 
		attribs = attribs->next;
    }
    if (!histitem)
        return ;
    for (count = 0; count < MAX_HISTORY; count++)
    if (histitem[count])
    {
        free(histitem[count]);
        histitem[count] = 0;
    }
    count = 0;
    while (children && count < MAX_HISTORY)
    {
        if (IsNode(children, "HISTITEM"))
        {
            attribs = children->attribs;
            while (attribs && count < MAX_HISTORY)
            {
                if (IsAttrib(attribs, "VALUE"))
                {
                    histitem[count++] = strdup(attribs->value);
                }
                attribs = attribs->next;
            }
        }
        children = children->next;
    }
}

//-------------------------------------------------------------------------

static int RestoreFont(struct xmlNode *node, int version)
{
    int i, j = 0, n;
    char *bf = node->textData;
    for (i = 0; i < sizeof(LOGFONT); i++)
    {
        sscanf(bf, "%d%n", &n, &j);
        *(((unsigned char*) &lf) + i) = n;
        bf += j;
    } 
}

static int RestorePrinter(struct xmlNode *node, int version)
{
    struct xmlAttr *attribs = node->attribs;
	struct xmlNode * children = node->children;
    while (attribs)
    {
        if (IsAttrib(attribs, "LEFT"))
            leftmargin = atoi(attribs->value);
        else if (IsAttrib(attribs, "RIGHT"))
            rightmargin = atoi(attribs->value);
        else if (IsAttrib(attribs, "TOP"))
            topmargin = atoi(attribs->value);
        else if (IsAttrib(attribs, "BOTTOM"))
            bottommargin = atoi(attribs->value);

        attribs = attribs->next;
	}
	while (children)
	{
		if (IsNode(children, "HEADER"))
		{
			int l = strlen(children->textData);
			if (l)
			{
				strcpy(printHeader, children->textData+ 1);
				if (l > 1)
				{
					printHeader[l-2] = 0;
				}
			}
		}
		else if (IsNode(children, "FOOTER"))
		{
			int l = strlen(children->textData);
			if (l)
			{
				strcpy(printFooter, children->textData+ 1);
				if (l > 1)
				{
					printFooter[l-2] = 0;
				}
			}
		}
		children = children->next;
	}
}
//-------------------------------------------------------------------------

static void RestoreColors(struct xmlNode *node, int version)
{
    int i, j = 0;
    struct xmlAttr *attribs = node->attribs;
    char *bf = node->textData;
    while (attribs)
    {
        if (IsAttrib(attribs, "KEYWORD"))
            keywordColor = atoi(attribs->value);
        else if (IsAttrib(attribs, "NUMBER"))
            numberColor = atoi(attribs->value);
        else if (IsAttrib(attribs, "COMMENT"))
            commentColor = atoi(attribs->value);
        else if (IsAttrib(attribs, "STRING"))
            stringColor = atoi(attribs->value);
        else if (IsAttrib(attribs, "ESCAPE"))
            escapeColor = atoi(attribs->value);
        else if (IsAttrib(attribs, "BACKGROUND"))
            backgroundColor = atoi(attribs->value);
        else if (IsAttrib(attribs, "TEXT"))
            textColor = atoi(attribs->value);
        else if (IsAttrib(attribs, "HIGHLIGHT"))
            highlightColor = atoi(attribs->value);

        attribs = attribs->next;
    } 
	for (i = 0; i < 16; i++)
    {
        sscanf(bf, "%d%n", &custColors[i], &j);
        bf += j;
    }
}

//-------------------------------------------------------------------------

static void RestoreTabs(struct xmlNode *node, int version)
{
    struct xmlAttr *attribs = node->attribs;
    while (attribs)
    {
        if (IsAttrib(attribs, "SPACING"))
            tabspacing = atoi(attribs->value);
        attribs = attribs->next;
    } 
}

//-------------------------------------------------------------------------

static void RestoreEditflags(struct xmlNode *node, int version)
{
    struct xmlAttr *attribs = node->attribs;
    while (attribs)
    {
        if (IsAttrib(attribs, "FLAGS"))
            editFlags = atoi(attribs->value);
        attribs = attribs->next;
    } 
}
static void RestoreMemoryWindowSettings(struct xmlNode *node, int version)
{
    struct xmlAttr *attribs = node->attribs;
    while (attribs)
    {
        if (IsAttrib(attribs, "WORDSIZE"))
            memoryWordSize = atoi(attribs->value);
        attribs = attribs->next;
    } 
}
//-------------------------------------------------------------------------

static void RestoreFindflags(struct xmlNode *node, int version)
{
    struct xmlAttr *attribs = node->attribs;
    while (attribs)
    {
        if (IsAttrib(attribs, "FLAGS1"))
            findflags = atoi(attribs->value);
        if (IsAttrib(attribs, "FLAGS2"))
            replaceflags = atoi(attribs->value);
        attribs = attribs->next;
    } 
}

//-------------------------------------------------------------------------

static void RestoreWindows(struct xmlNode *node, int version)
{
	int old = maximizeClient;
    int i=0;
    struct xmlNode *child = node->children;
	HWND hwnd = NULL;
    while (child)
    {
        if (IsNode(child, "WINDOW"))
        {
            if (i < MAX_WINDOWS)
            {
                struct xmlAttr *attribs = child->attribs;
                while (attribs)
                {
                    if (IsAttrib(attribs, "NAME"))
                    {
                        windowNames[i]  = strdup(attribs->value);
                    } 
                    else if (IsAttrib(attribs, "TITLE"))
                        windowTitles[i]  = strdup(attribs->value);
                    else if (IsAttrib(attribs, "X"))
                        mc[i].x = atoi(attribs->value);
                    else if (IsAttrib(attribs, "Y"))
                        mc[i].y = atoi(attribs->value);
                    else if (IsAttrib(attribs, "WIDTH"))
                        mc[i].cx = atoi(attribs->value);
                    else if (IsAttrib(attribs, "HEIGHT"))
                        mc[i].cy = atoi(attribs->value);
                    else if (IsAttrib(attribs, "LINENO"))
                        lineNo[i] = atoi(attribs->value);
                    attribs = attribs->next;
                }
                i++;
                windowCount++;
            }
        }
        child = child->next;
    }
}

//-------------------------------------------------------------------------
/*
void RestoreChangeLn(struct xmlNode *node, int version)
{
    struct xmlNode *children = node->children;
    struct tagfile *l,  **ls = &tagFileList;
    struct xmlAttr *attribs;
    while (*ls)
        ls = &(*ls)->next;
    while (children)
    {
        if (IsNode(children, "FILE"))
        {
            struct xmlNode *fchildren = children->children;
            struct tagchangeln *c = 0,  **x = &c;
            *ls = calloc(sizeof(struct tagfile), 1);
            if (!ls)
            {
                NoMemoryWS();
                return ;
            } attribs = children->attribs;
            while (attribs)
            {
                if (IsAttrib(attribs, "NAME"))
                {
                    strcpy((*ls)->name, attribs->value);
                    abspath((*ls)->name, szWorkspaceName);
                }
                attribs = attribs->next;
            }
            l = tagFileList;
            while (l)
            {
                if (!strcmp(l->name, (*ls)->name))
                    break;
                l = l->next;
            }
            if (l !=  *ls)
            {
                free(*ls);
                *ls = 0;
            }
            else
                ls = &(*ls)->next;
            while (fchildren)
            {
                if (IsNode(fchildren, "LINE"))
                {
                    // assumes enumerated in order
                    *x = calloc(sizeof(struct tagchangeln), 1);
                    if (! *x)
                    {
                        NoMemoryWS();
                        return ;
                    } attribs = fchildren->attribs;
                    while (attribs)
                    {
                        if (IsAttrib(attribs, "NUM"))
                            (*x)->lineno = atoi(attribs->value);
                        else if (IsAttrib(attribs, "DELTA"))
                            (*x)->delta = atoi(attribs->value);
                        attribs = attribs->next;
                    }
                    x = &(*x)->next;
                }
                fchildren = fchildren->next;
            }
            l->changedLines = c;
        }
        children = children->next;
    }
}
*/
//-------------------------------------------------------------------------

static void RestoreTags(struct xmlNode *node, int version)
{
    struct tagfile *l,  **ls = &tagFileList;
    struct tag *t,  **ts;
    struct xmlAttr *attribs = node->attribs;
    struct xmlNode *children = node->children;
    int tagid =  - 1;
    while (attribs)
    {
        if (IsAttrib(attribs, "ID"))
            tagid = atoi(attribs->value);
        attribs = attribs->next;
    } 
    if (tagid ==  - 1 || tagid != TAG_BP && tagid != TAG_BOOKMARK)
        return ;
    while (*ls)
        ls = &(*ls)->next;
    while (children)
    {
        if (IsNode(children, "FILE"))
        {
            struct xmlNode *fchildren = children->children;
            *ls = calloc(sizeof(struct tagfile), 1);
            if (!ls)
            {
                NoMemoryWS();
                return ;
            } attribs = children->attribs;
            while (attribs)
            {
                if (IsAttrib(attribs, "NAME"))
                {
                    strcpy((*ls)->name, attribs->value);
                }
                attribs = attribs->next;
            }
            l = tagFileList;
            while (l)
            {
                if (!strcmp(l->name, (*ls)->name))
                    break;
                l = l->next;
            }
            if (l !=  *ls)
            {
                free(*ls);
                *ls = 0;
            }
            else
                ls = &(*ls)->next;
            ts = &l->tagArray[tagid];
            while (fchildren)
            {
                if (IsNode(fchildren, "TAGITEM"))
                {
                    *ts = calloc(sizeof(struct tag), 1);
                    if (! *ts)
                    {
                        NoMemoryWS();
                        return ;
                    } 
					attribs = fchildren->attribs;
                    while (attribs)
                    {
                        if (IsAttrib(attribs, "LINENO"))
                            (*ts)->drawnLineno = (*ts)->editingLineno = (*ts)
                                ->debugLineno = atoi(attribs->value);
                        else if (IsAttrib(attribs, "CP"))
                            (*ts)->charpos = atoi(attribs->value);
                        else if (IsAttrib(attribs, "ENABLED"))
                            (*ts)->enabled = atoi(attribs->value);
                        attribs = attribs->next;
                    }
                    if (tagid == TAG_BOOKMARK)
                    {
                        (*ts)->extra = strdup(fchildren->textData);
                    }
                    ts = &(*ts)->next;
                }
                fchildren = fchildren->next;
            }
        }
        children = children->next;
    }
}

//-------------------------------------------------------------------------

static void RestoreBrowse(struct xmlNode *node, int version)
{
    struct xmlAttr *attribs = node->attribs;
    while (attribs)
    {
        if (IsAttrib(attribs, "VALUE"))
			if (version >= 6) // resetting to the new default at this version
	            browseInfo = atoi(attribs->value);
        if (IsAttrib(attribs, "COMPLETION"))
            completionEnabled = atoi(attribs->value);
        if (IsAttrib(attribs, "PARENTHESIS"))
            parenthesisCheck = atoi(attribs->value);
        attribs = attribs->next;
    } 
}

//-------------------------------------------------------------------------

static void RestoreWatch(struct xmlNode *node, int version)
{
    struct xmlAttr *attribs = node->attribs;
    int id = 0;
    while (attribs)
    {
        if (IsAttrib(attribs, "ID"))
        {
            // ID MUST BE FIRST!!!!!
            id = atoi(attribs->value) - 1;
            if (id < 0 || id > 3)
                return ;
        } 
        else if (IsAttrib(attribs, "ENABLE"))
        {
            if (atoi(attribs->value))
                hbpEnables |= (1 << id);
        }
        else if (IsAttrib(attribs, "NAME"))
        {
            strncpy(hbpNames[id], attribs->value, 255);
            hbpNames[id][255] = 0;
        }
        else if (IsAttrib(attribs, "MODE"))
        {
            hbpModes[id] = atoi(attribs->value);
        }
        else if (IsAttrib(attribs, "SIZE"))
        {
            hbpSizes[id] = atoi(attribs->value);
        }
        attribs = attribs->next;
    }
}

//-------------------------------------------------------------------------
/*
void RestoreToolBars(struct xmlNode *node, int version)
{
    int id = 0;
    HWND hwnd;
    char *horiz = 0,  *vert = 0;
    struct xmlAttr *attribs = node->attribs;
	if (version != WSPVERS)
		return;
    while (attribs)
    {
        if (IsAttrib(attribs, "ID"))
            id = atoi(attribs->value);
        else if (IsAttrib(attribs, "HORIZ"))
            horiz = attribs->value;
        else if (IsAttrib(attribs, "VERT"))
            vert = attribs->value;
        attribs = attribs->next;
    } 
	if (id && horiz && vert)
    {
        switch (id)
        {
            case DID_EDITTOOL:
                hwnd = hwndToolEdit;
                break;
            case DID_BUILDTOOL:
                hwnd = hwndToolBuild;
                break;
            case DID_DEBUGTOOL:
                hwnd = hwndToolDebug;
                break;
            default:
                return ;
        }
    }
}
*/
//-------------------------------------------------------------------------

struct projectData
{
	char *name; 
	int flags;
	int dbgview;
} ;
static struct projectData *RestoreProjectNames(struct xmlNode *node, int version)
{
    struct xmlNode *children = node->children;
	static struct projectData data[1024];
    int i = 0;
    memset(data,0,sizeof(data));
    while (children)
    {
        if (IsNode(children, "FILE"))
        {
            struct xmlAttr * attribs = children->attribs ;
            while (attribs)
            {
                if (IsAttrib(attribs, "NAME"))
                {
                    data[i].name = strdup(attribs->value);
                }
                else if (IsAttrib(attribs, "FLAGS"))
				{
                    data[i].flags = strtol(attribs->value,0,16);
				}
                else if (IsAttrib(attribs, "DEBVIEW"))
				{
                   data[i].dbgview = strtol(attribs->value,0,16);
				}
                attribs = attribs->next ;
            }
			i++;
        }
        children = children->next ;
    }
    return data;
}

static FILELIST *RestoreFileData(struct xmlNode *root, PROJLIST *plist, PROJLIST *pl, 
        struct xmlNode *children)
{
    FILELIST *list = 0, **l = &list;
    children = children->children ; // down one level to the file list
    while (children) {
        if (IsNode(children, "FILE")) {
            DEPENDSLIST *dlist = 0,  **dl = &dlist;
            struct xmlNode *fchildren = children->children;
            struct xmlAttr *attribs;
            *l = calloc(sizeof(FILELIST), 1);
            if (! *l)
                return NoMemory(root, plist);
            attribs = children->attribs;
            while (attribs)
            {
                if (IsAttrib(attribs, "NAME"))
                {
                    strcpy((*l)->name, attribs->value);
                } 
                else if (IsAttrib(attribs, "TITLE"))
                {
                    strcpy((*l)->title, attribs->value);
                }
                attribs = attribs->next;
            }
            while (fchildren)
            {
                if (IsNode(fchildren, "DEPENDENCY"))
                {
                    *dl = calloc(sizeof(DEPENDSLIST), 1);
                    if (! *dl)
                        return NoMemory(root, plist);
        
                    attribs = fchildren->attribs;
                    while (attribs)
                    {
                        if (IsAttrib(attribs, "NAME"))
                        {
                            strcpy((*dl)->name, attribs->value);
                        }
                        else if (IsAttrib(attribs, "TITLE"))
                        {
                            strcpy((*dl)->title, attribs->value);
                        }
                        attribs = attribs->next;
                    }
                }
                dl = &(*dl)->next;
                fchildren = fchildren->next;
            }
            (*l)->depends = dlist;
            l = &(*l)->next;
        }
        children = children->next ;
    }
    return list ;
}

//-------------------------------------------------------------------------

static int TranslateLoadList(char *name)
{
    PROJLIST *plist = 0;
    int i=0;
    char buf[2049];
    int temp, projectVersion;
    struct xmlNode *root;
    struct xmlNode *targets,  *children;
    struct xmlAttr *attribs;
    char projname[256];
    FILE *in;
    strcpy(projname, name);
    abspath(projname, szWorkspaceName);
    in = fopen(projname, "r");
    if (!in)
    {
        printf("Load Error - Project File %s Not Found",projname);
        errs++;
        return 0;
    }
    root = xmlReadFile(in);
    fclose(in);
    if (!root || !IsNode(root, "CC386TARGET"))
    {
        return LoadErr(root, projname, plist);
    }
    targets = root->children;
    while (targets)
    {
        if (IsNode(targets, "VERSION"))
        {
            attribs = targets->attribs;
            while (attribs)
            {
                if (IsAttrib(attribs, "ID"))
                    projectVersion = atoi(attribs->value);
                attribs = attribs->next;
            }
        }
        else if (IsNode(targets, "TARGET"))
        {
            FILELIST *flist = 0, *rlist = 0, *olist = 0;
            plist = calloc(sizeof(*plist), 1);
            if (! plist)
                return NoMemory(root, plist);
            strcpy(plist->projectName, projname);
            attribs = targets->attribs;
            while (attribs)
            {
                if (IsAttrib(attribs, "NAME"))
                {
                    strcpy(plist->name, attribs->value);
                    abspath(plist->name, plist->projectName);
                }
                else if (IsAttrib(attribs, "TITLE"))
                {
                    strcpy(plist->title, attribs->value);
                }
                attribs = attribs->next;
            }
            children = targets->children;
            while (children)
            {
                if (IsNode(children, "BUILD"))
                {
                    attribs = children->attribs;
                    while (attribs)
                    {
                        if (IsAttrib(attribs, "TYPE"))
                            plist->buildType = atoi(attribs->value);
                        else if (IsAttrib(attribs, "FLAGS"))
                            plist->buildFlags = atoi(attribs->value);
                        else if (IsAttrib(attribs, "LIBTYPE"))
                            plist->libType = atoi(attribs->value);
                        attribs = attribs->next;
                    }
                }
                else if (IsNode(children, "PATHS"))
                {
                    attribs = children->attribs;
                    while (attribs)
                    {
                        if (IsAttrib(attribs, "INCLUDE"))
                        {
                            strcpy(plist->includePath, attribs->value);
                        }
                        else if (IsAttrib(attribs, "OUTPUT"))
                        {
                            strcpy(plist->outputPath, attribs->value);
                        }
                        attribs = attribs->next;
                    }
                }
                else if (IsNode(children, "PREBUILD"))
                {
                }
                else if (IsNode(children, "POSTBUILD"))
                {
                }
                else if (IsNode(children, "OPTS"))
                {
                    struct xmlNode *ochildren = children->children;
                    while (ochildren)
                    {
                        if (IsNode(ochildren, "COMPILER"))
                            strcpy(plist->compileopts, ochildren->textData);
                        if (IsNode(ochildren, "ASSEMBLER"))
                            strcpy(plist->assembleopts, ochildren->textData);
                        if (IsNode(ochildren, "LINKER"))
                            strcpy(plist->linkopts, ochildren->textData);
                        if (IsNode(ochildren, "LIBRARIAN"))
                            strcpy(plist->libopts, ochildren->textData);
                        ochildren = ochildren->next;
                    } 
                }
                else if (IsNode(children, "CMDLINE"))
                {
                    int len;
                    strcpy(plist->cmdline, children->textData);
                }
                else if (IsNode(children, "DEFINE"))
                {
                    DEFINES *d = calloc(sizeof(DEFINES), 1),  **dl = &plist
                        ->defines;
                    if (!d)
                        return NoMemory(root, plist);
                    attribs = children->attribs;
                    while (attribs)
                    {
                        if (IsAttrib(attribs, "NAME"))
                            strcpy(d->name, attribs->value);
                        else if (IsAttrib(attribs, "VALUE"))
                            strcpy(d->value, attribs->value);
                        attribs = attribs->next;
                    }
                    while (*dl)
                        dl = &(*dl)->next;
                    *dl = d;
                }
                else if (IsNode(children, "DEFINEFLAGS"))
                {
                    attribs = children->attribs;
                    while (attribs)
                    {
                        if (IsAttrib(attribs, "VALUE"))
                            plist->defineFlags = atoi(attribs->value);
                        attribs = attribs->next;
                    }
                }
                else if (IsNode(children, "SOURCES"))
                {
                    flist = RestoreFileData(root, plist, plist, children);
                }
                else if (IsNode(children, "RESOURCES"))
                {
                    rlist = RestoreFileData(root, plist, plist, children);
                }
                else if (IsNode(children, "OTHER"))
                {
                    olist = RestoreFileData(root, plist, plist, children);
                }
                children = children->next;
            }
            plist->sourceFiles = flist;
            plist->otherFiles = olist;
            plist->resourceFiles = rlist;
        }
        targets = targets->next;
    }
    xmlFree(root);
    SaveProject(name, plist);
}

//-------------------------------------------------------------------------

int TranslateWorkSpace(char *name, int toerr)
{
    int version;
    FILE *in;
    struct xmlNode *root;
    struct xmlNode *nodes,  *children;
    struct xmlAttr *attribs;
    char buf[256],  *p; 
    strcpy(szWorkspaceName, name);
    in = fopen(name, "r");
    if (!in)
    {
        if (toerr)
            printf("Load Error - Workspace File Not Found");
        errs++;
        return 0;
    }
    root = xmlReadFile(in);
    fclose(in);
    if (!root || !IsNode(root, "CC386WORKSPACE"))
    {
        printf("Load Error - Workspace is invalid file format");
        return 0;
    }
    nodes = root->children;
    while (nodes)
    {
        if (IsNode(nodes, "VERSION"))
        {
            struct xmlAttr *attribs = nodes->attribs;
            while (attribs)
            {
                if (IsAttrib(attribs, "ID"))
                    version = atoi(attribs->value);
                attribs = attribs->next;
            } 
        }
        else if (IsNode(nodes, "COLORS"))
            RestoreColors(nodes, version);
        else if (IsNode(nodes, "TABS"))
            RestoreTabs(nodes, version);
        else if (IsNode(nodes, "EDITOR"))
            RestoreEditflags(nodes, version);
        else if (IsNode(nodes, "MEMWND"))
            RestoreMemoryWindowSettings(nodes, version);
		else if (IsNode(nodes, "PRINTER"))
			RestorePrinter(nodes, version);
        else if (IsNode(nodes, "FIND"))
            RestoreFindflags(nodes, version);
        else if (IsNode(nodes, "HISTORY"))
            RestoreHistory(nodes, version);
        else if (IsNode(nodes, "FONT"))
            RestoreFont(nodes, version);
        else if (IsNode(nodes, "EDITWINDOWS"))
            RestoreWindows(nodes, version);
        else if (IsNode(nodes, "TAG"))
            RestoreTags(nodes, version);
        else if (IsNode(nodes, "PLACEMENT"))
            RestorePlacement(nodes, version);
        else if (IsNode(nodes, "DOCKS"))
            RestoreDocks(nodes, version);
        else if (IsNode(nodes, "TOOLBAR"))
            ; //RestoreToolBars(nodes, version);
        else if (IsNode(nodes, "BROWSE"))
            RestoreBrowse(nodes, version);
        else if (IsNode(nodes, "WATCH"))
            RestoreWatch(nodes, version);
        else if (IsNode(nodes, "CHANGELN"))
            ; //RestoreChangeLn(nodes, version);
        else if (IsNode(nodes, "TARGETS"))
            projectNames = RestoreProjectNames(nodes, version);
        nodes = nodes->next;
    }
    // needs to be before the XML free because the project names are taken in-place
    if (projectNames)
	{
        int i;
		for (i=0; i <1024; i++)
        {
            char buf[MAX_PATH];
            if (projectNames[i].name)
            {
                strcpy(buf, projectNames[i].name);
                abspath(buf, szWorkspaceName);
                TranslateLoadList(buf);
            }
        }
    }
    xmlFree(root);
    SaveWorkSpace(name);
	return 1;
}
static void SaveFiles(FILE *out, PROJLIST *proj, FILELIST *children)
{
	while (children)
	{
		int i;
		fprintf(out, "\t\t\t\t<FILE NAME=\"%s\" TITLE=\"%s\" CLEAN=\"%d\"/>\n",children->name, children->title, 0);
		children = children->next;
	}
}
static void SetPropX(FILE *out, char *item, char *value)
{
    fprintf(out, "\t\t\t\t<PROP ID=\"%s\">%s</PROP>\n", item, value);
}
static void SaveProjectProps(FILE *out, PROJLIST *proj)
{
    char *n;
    char defbuf[5000];
    struct defines *df;
    BOOL first;
    switch(proj->buildType)
    {
        case BT_CONSOLE:
        default:
            n = "";
            break;
        case BT_WINDOWS:
            n = "1";
            break;
        case BT_DLL:
            n = "2";
            break;
        case BT_LIBRARY:
            n = "3";
            break;
        case BT_DOS:
            n = "4";
            break;
        case BT_RAW:
            n = "5";
            break;
    }
    if (n[0])
        SetPropX(out, "__PROJECTTYPE", n);
    switch(proj->libType)
    {
        case LT_STANDARD:
        default:
            n = "";
            break;
        case LT_LSCRTDLL:
            n = "2";
            break;
        case LT_CRTDLL:
            n = "3";
            break;
        case LT_MSVCRT:
            n = "4";
            break;
        case LT_NONE:
            n = "0";
            break;
    }
    if (n[0])
        SetPropX(out, "__LIBRARYTYPE", n);
    if (!(proj->buildFlags & BF_DEBUGINFO))
    {
        SetPropX(out, "__DEBUG", "0");
    }
    if (proj->buildFlags & BF_SHOWWARNINGS)
    {
        SetPropX(out, "__WARNINGS", "");
    }
    if (!(proj->buildFlags & BF_C99))
    {
        SetPropX(out, "__C99", "");
    }
    if (proj->buildFlags & BF_ANSI)
    {
        SetPropX(out, "__ANSI", "/A");
    }
    if (!(proj->buildFlags & BF_SHOWRETURNCODE))
    {
        SetPropX(out, "__SHOW_RETURN_CODE", "0");
    }
    if (proj->buildFlags & BF_BREAKDLL)
    {
        SetPropX(out, "__BREAK_DLL", "1");
    }
    if (proj->buildFlags & BF_DEBUGEXCEPTION)
    {
        SetPropX(out, "__STOP_FIRST_CHANCE", "1");
    }
    
    if (proj->buildFlags & BF_ALIGNSTACK)
    {
        SetPropX(out, "__ALIGNSTACK", "/C+s");
    }
    df = proj->defines; //__DEFINE "str" __ASMDEFINE
    first = TRUE;
    while (df)
    {
        if (!first)
            strcat(defbuf, " ");
        first = FALSE;
        strcat(defbuf,df->name);
        if (df->value[0])
        {
            strcat(defbuf, "=");
            strcat(defbuf, df->value);
        }
        df = df->next;
    }
    if (proj->defines)
    {
        SetPropX(out, "__DEFINE", defbuf);
        SetPropX(out, "__ASMDEFINE", defbuf);
    }
    if (proj->cmdline[0])
        SetPropX(out, "__DEBUG_ARGUMENTS", proj->cmdline);
    if (proj->includePath[0])
    {
        SetPropX(out, "__INCLUDE", proj->includePath);
        SetPropX(out, "__ASMINCLUDE", proj->includePath);
    }
    if (proj->compileopts[0])
        SetPropX(out, "__CFLAGS", proj->compileopts);
    if (proj->assembleopts[0])
        SetPropX(out, "__AFLAGS", proj->assembleopts);
    if (proj->linkopts[0])
        SetPropX(out, "__LFLAGS", proj->linkopts);
}
//-------------------------------------------------------------------------
static void SaveProject(char *name, PROJLIST *project)
{
    FILE *out ;
    char abbrevName[MAX_PATH];
    char *p;
    strcpy(abbrevName, name);
    p  = strrchr(abbrevName, '.');
    if (p)
        *p = 0;
	sprintf(name, "%s.cpj", abbrevName);
    out = fopen(name, "w");
    if (!out)
    {
        printf("Save Error - Could not save project %s", abbrevName);
		return;
    }
    fprintf(out, "<CC386PROJECT>\n\t<VERSION ID=\"%d\"/>\n", NEWPROJVERS);
    p = strrchr(abbrevName, '\\');
    if (!p)
        p = abbrevName;
    else
        p++;
    fprintf(out, "\t<TARGET TITLE=\"%s\">\n", p);
	fprintf(out, "\t\t<PROPERTIES>\n");
    SaveProjectProps(out, project);
	fprintf(out, "\t\t</PROPERTIES>\n");
	fprintf(out, "\t\t<FILES>\n");
	fprintf(out, "\t\t\t<FOLDER TITLE=\"Source Files\">\n");
	SaveFiles(out, project, project->sourceFiles);
   	fprintf(out, "\t\t\t</FOLDER>\n");
	fprintf(out, "\t\t\t<FOLDER TITLE=\"Resource Files\">\n");
	SaveFiles(out, project, project->resourceFiles);
	fprintf(out, "\t\t\t</FOLDER>\n");
	fprintf(out, "\t\t\t<FOLDER TITLE=\"Other Files\">\n");    
	SaveFiles(out, project, project->otherFiles);
	fprintf(out, "\t\t\t</FOLDER>\n");
	fprintf(out, "\t\t</FILES>\n");
    fprintf(out, "\t</TARGET>\n");
    fprintf(out, "</CC386PROJECT>\n");
    fclose(out);
}
static void SaveWatchpoints(FILE *out)
{
    int i;
    for (i = 0; i < 4; i++)
    {
        fprintf(out, 
            "\t<WATCH ID=\"%d\" ENABLE=\"%d\" NAME=\"%s\" MODE=\"%d\" SIZE=\"%d\"/>\n", i + 1, !!(hbpEnables &(1 << i)), xmlConvertString(hbpNames[i], TRUE), hbpModes[i], hbpSizes[i]);
    }
}
void SaveDocks(FILE *out)
{
    int i;
    fprintf(out, "\t<DOCKS>\n");
    for (i = 0; i < dockCount; i++)
    {
        fprintf(out, 
            "\t\t<DOCK ID=\"%d\" VALUE=\"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\"/>\n", 
                ids[i],
                d[i].flags, d[i].flexparams, 
                d[i].rowindex, d[i].colindex, 
                d[i].hidden, d[i].hiddenwidth, 
                d[i].oldsize.left, d[i].oldsize.top, 
                d[i].oldsize.right, d[i].oldsize.bottom, 
                d[i].position.left, d[i].position.top, 
                d[i].position.right, d[i].position.bottom, 
                d[i].lastposition.left, d[i].lastposition.top, 
                d[i].lastposition.right, d[i].lastposition.bottom);
    }
    fprintf(out, "\t</DOCKS>\n");
}
void SaveWindows(FILE *out)
{
    int i;
    fprintf(out, "\t<EDITWINDOWS>\n");
    for (i=0; i < windowCount; i++)
    {
        fprintf(out, 
            "\t\t<WINDOW NAME=\"%s\" TITLE=\"%s\" X=\"%d\" Y=\"%d\" WIDTH=\"%d\" HEIGHT=\"%d\" LINENO=\"%d\"/>\n", 
            windowNames[i], windowTitles[i], mc[i].x, mc[i].y, mc[i].cx, mc[i].cy, lineNo[i]  );
    }
    fprintf(out, "\t</EDITWINDOWS>\n");
}
void onehistsave(FILE *out, char **hist, char *name)
{
    int i;
    fprintf(out, "\t<HISTORY TYPE=\"%s\">\n", name);
    for (i = 0; i < MAX_HISTORY; i++)
        if (hist[i])
            fprintf(out, "\t\t<HISTITEM VALUE=\"%s\"/>\n", xmlConvertString(hist[i],TRUE));
    fprintf(out, "\t</HISTORY>\n");
}

//-------------------------------------------------------------------------

int SaveHistory(FILE *out)
{
    onehistsave(out, findhist, "FIND");
    onehistsave(out, replacehist, "REPLACE");
    onehistsave(out, watchhist, "WATCH");
    onehistsave(out, memhist, "MEMORY");
}
void saveonetag(FILE *out, int tag)
{
    struct tagfile *list = tagFileList;
    fprintf(out, "\t<TAG ID=\"%d\">\n", tag);
    while (list)
    {
        if (list->tagArray[tag])
        {
            struct tag *data = list->tagArray[tag];
            fprintf(out, "\t\t<FILE NAME=\"%s\">\n", list->name);
            while (data)
            {
                fprintf(out, 
                    "\t\t\t<TAGITEM LINENO=\"%d\" CP=\"%d\" ENABLED=\"%d\"",
                    data->drawnLineno, data->charpos, data->enabled);

                if (tag == TAG_BOOKMARK)
                    fprintf(out, ">\n%s\n\t\t\t</TAGITEM>\n", xmlConvertString
                        (data->extra, FALSE));
                else
                    fprintf(out, "/>\n");
                data = data->next;
            } fprintf(out, "\t\t</FILE>\n");
        }
        list = list->next;
    }
    fprintf(out, "\t</TAG>\n");
}

//-------------------------------------------------------------------------

void SaveTags(FILE *out)
{
    saveonetag(out, TAG_BP);
    saveonetag(out, TAG_BOOKMARK);
}
void SaveProjectNames(FILE *out)
{
    int i =0;
    char buf[256], *p;
    strcpy(buf, projectNames[0].name ? projectNames[0].name : "");
    p = strrchr(buf, '.');
    if (p)
        *p = 0;
    p = strrchr(buf, '.');
    if (p)
        *p = 0;
    fprintf(out,"\t<PROJECTS SELECTED=\"%s\">\n", buf);
    while (projectNames[i].name)
    {
        strcpy(buf, projectNames[i].name);
        p = strrchr(buf, '.');
        if (p)
            *p = 0;
        fprintf(out,"\t\t<FILE NAME=\"%s\" CLEAN=\"0\"/>\n", buf);
        i++;
    }
    fprintf(out,"\t</PROJECTS>\n");
}
static void SaveWorkSpace(char *name)
{
	FILE *out;
    char buf[MAX_PATH], *p;
    strcpy(buf, name);
    p = strchr(buf, '.');
    if (p)
        *p = 0;
    strcat(buf, ".cwa");
    out = fopen(buf, "w");
    if (!out)
    {
        printf("Save Error - Could not save WorkArea");
        return ;
    }
    fprintf(out, "<CC386WORKAREA>\n");
    fprintf(out, "\t<VERSION ID=\"%d\"/>\n", NEWWSPVERS);
    SaveWatchpoints(out);
    SaveDocks(out);
    SaveWindows(out);
    SaveHistory(out);
    SaveTags(out);
    SaveProjectNames(out);
    fprintf(out, "</CC386WORKAREA>\n");
    fclose(out);
}
int main(int argc, char **argv)
{
    if (argc == 3)
    {
        if (!strcmp(argv[1], "proj"))
            TranslateLoadList(argv[2]);
        else
            TranslateWorkSpace(argv[2], TRUE);
        return errs;
    }
    return 1;
}