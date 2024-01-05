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
#include "winconst.h"

extern int changedProject;
extern int defaultWorkArea;
extern PROJECTITEM *workArea;

int browsing;

static DWINFO **browsebacklist;
static int browseCount, browseMax;

#define MAX(i,j) ((i) > (j) ? (i) : (j))
#define LIB_BUCKETS 37
#define LIB_FREEPOS 37
#define ROTR(x,bits) (((x << (16 - bits)) | (x >> bits)) & 0xffff)
#define ROTL(x,bits) (((x << bits) | (x >> (16 - bits))) & 0xffff)

typedef unsigned hv;
typedef struct
{
    hv block_x, bucket_x, block_d, bucket_d;
} HASH;

unsigned SymbolHash(char *sym, int max)
{
    unsigned rv = 0;
    while (*sym)
    {
        int ch = tolower(*sym);
        sym++;
        rv = rv * 128 + rv * 2 + rv + ch;
    }
    return rv % max;
}
FILE *LoadBrowseInfo(PROJECTITEM *pj, char *name)
{
    FILE *fil;
    int ofs;
    unsigned char buf[512];
   	int bottom;
    if (!pj->browseInfo)
    {
        pj->browseInfo = calloc(1, sizeof(IDENTBROWSE));
        if (pj->browseInfo)
        {
            char name[MAX_PATH], *p;
            strcpy(name, pj->realName);
            p = strrchr(name, '.');
            if (!p)
                p = name + strlen(name);
            strcpy(p, ".brw");
            pj->browseInfo->fileName = strdup(name);
             
        }
        else
        {
            return NULL;
        }
    }
    fil = fopen(pj->browseInfo->fileName, "rb");
    if (!fil)
        return NULL;
    fread(buf, 16, 1, fil);
    if (strncmp(buf, "$BRW", 4))
    {
        fclose(fil);
        return NULL;
    }
	if (*(int *)(buf + 8) != BROWSE_VERSION)
    {
        fclose(fil);
		return NULL;
    }
        
   	bottom = *(int *)(buf + 4);
    if (!pj->browseInfo->filecount)
    {
        unsigned short count, i;
    	unsigned size;
    	if (bottom == 0)
        {
            fclose(fil);
    		return NULL;
        }
    	fseek(fil, 0, SEEK_END);
    	size = ftell(fil);
    	if ((size - bottom)%512)
        {
            fclose(fil);
    		return NULL;
        }
        pj->browseInfo->dictPages = (size - bottom)/512;
        fseek(fil, 32, SEEK_SET);
        fread(&count, 2, 1, fil);
        pj->browseInfo->filenames = calloc(count, sizeof(char*));
        if (!pj->browseInfo->filenames)
        {
            fclose(fil);
            return NULL;
        }
        pj->browseInfo->filecount = count;
        for (i = 0; i < count; i++)
        {
            char name[256];
            int len = fgetc(fil);
            fread(name, len, 1, fil);
            name[len] = 0;
            pj->browseInfo->filenames[i] = strdup(name);
            if (!pj->browseInfo->filenames[i])
            {
                for (--i; i >= 0; --i)
                {
                    free(pj->browseInfo->filenames[i]);
                }
                free(pj->browseInfo->filenames);
                pj->browseInfo->filecount = 0;
                fclose(fil);
                return 0;
            }
        }
        pj->browseInfo->hash = calloc(IDENTHASHMAX, sizeof (struct _identHash *));
        if (!pj->browseInfo->hash)
        {
            fclose(fil);
            return NULL;
        }
        for (i=0; i < count; i++)
        {
            int hash = SymbolHash(pj->browseInfo->filenames[i], IDENTHASHMAX);
            struct _identHash *p = calloc(1, sizeof(struct _identHash));
            if (!p)
            {
                fclose(fil);
                return NULL;
            }
            p->name = pj->browseInfo->filenames[i];
            p->fileno = i;
            p->next = pj->browseInfo->hash[hash];
            pj->browseInfo->hash[hash] = p;
        }
    }
    if (name)
    {
        ofs = FindBrowseInfo(fil, pj->browseInfo->dictPages, name, bottom);
        if (ofs == 0)
        {
            fclose(fil);
            return NULL;
        }
        fseek(fil, ofs, SEEK_SET); // at identifier data
    }
    else
    {
        fseek(fil, bottom, SEEK_SET); // at dictionary
    }
    return fil;
}
static FILE *LoadBrowseInfoByHWND(HWND hwnd, char *name, PROJECTITEM **returnProj)
{
    char *filname = (char*)SendMessage(hwnd, WM_FILENAME, 0, 0);
    PROJECTITEM *pj = HasFile(workArea, filname);
    while (pj && pj->type != PJ_PROJ)
        pj = pj->parent;
    if (!pj)
    {
        return NULL;
    }    
    *returnProj = pj;
    return LoadBrowseInfo(pj, name);
}
//-------------------------------------------------------------------------

void FreeBrowseInfo(void)
{
	if (workArea)
	{
	    PROJECTITEM *pj = workArea->children;
	    while (pj)
	    {
	        int i;
	        IDENTBROWSE *br = pj->browseInfo;
	        pj->browseInfo = NULL;
            if (br)
            {
    	        free(br->fileName);
    	        for (i = 0; i < br->filecount; i++)
    	            free(br->filenames[i]);
    	        free(br->filenames);
                if (br->hash)
                {
                    for (i=0; i < IDENTHASHMAX; i++)
                    {
                        struct _identHash *p = br->hash[i];
                        while (p)
                        {
                            struct _identHash *next = p->next;
                            free(p);
                            p = next;
                        }
                    }
                    free(br->hash);
                }
    	        free(br);        
            }
            pj = pj->next;
	    }
	}
}

static void LibHash(const char *name, int blocks, HASH *h)
{
    int len = strlen(name);
    const char *pb = name,  *pe = name + len;
    const unsigned short blank = ' ';

    hv block_x = len | blank, bucket_d = len | blank;
    hv block_d = 0, bucket_x = 0;

    while (1)
    {
        unsigned short cback = *(--pe) | blank;
        unsigned short cfront = *(pb++) | blank;
        bucket_x = ROTR(bucket_x, 2) ^ cback;
        block_d = ROTL(block_d, 2) ^ cback;
        if (--len == 0)
            break;
        block_x = ROTL(block_x, 2) ^ cfront;
        bucket_d = ROTR(bucket_d, 2) ^ cfront;
    }
    h->block_x = block_x % blocks;
    h->block_d = block_d % blocks;
    h->block_d = MAX(h->block_d, 1);
    h->bucket_x = bucket_x % LIB_BUCKETS;
    h->bucket_d = bucket_d % LIB_BUCKETS;
    h->bucket_d = MAX(h->bucket_d, 1);
}
//-------------------------------------------------------------------------

static int FindBrowseInfo(FILE *fil, int pages, char *name, int root)
{
	unsigned char dptr[512];
    HASH lhr;
    int startblock;
	int len;
   	if (*name == 0)
        return 0;
	len = strlen(name);
    LibHash(name, pages, &lhr);
    startblock = lhr.block_x;
    do
    {
        int startbucket = lhr.bucket_x;
		fseek(fil, root + 512 * lhr.block_x, SEEK_SET);
		fread(dptr,512,1,fil);
        do
        {
            if (dptr[lhr.bucket_x])
            {
				char *p = dptr + dptr[lhr.bucket_x]*2;
				int n = (unsigned char)*p++;
				if (n == len && !memcmp(p, name, n))
				{
//					if (n & 1) n++;
					return *(int *)(p + n) & 0xffffff; // top byte is type
				}
            }
            lhr.bucket_x += lhr.bucket_d;
            if (lhr.bucket_x >= LIB_BUCKETS)
                lhr.bucket_x -= LIB_BUCKETS;
        }
        while (startbucket != lhr.bucket_x)
            ;
        nextblock: lhr.block_x += lhr.block_d;
        if (lhr.block_x >= pages)
            lhr.block_x -= pages;
    }
    while (startblock != lhr.block_x)
        ;
	return 0;
}

void InsertBrowse(char *filname, int curline)
{
    DWINFO *info;
    char *p;
    if (browseCount >= browseMax)
    {
        if (browseCount >= 20)
        {
            memmove(browsebacklist, browsebacklist + 1, (--browseCount)
                *sizeof(void*));
        }
        else
        {
            browsebacklist = realloc(browsebacklist, (browseMax += 20)
                *sizeof(void*));
            if (!browsebacklist)
            {
                browseMax = 0;
                return;
            }
        }
    }
    info = calloc(sizeof(DWINFO), 1);
    if (!info)
        return;
    strcpy(info->dwName, filname);
    info->dwLineNo = curline;
	info->newFile = FALSE;
    p = strrchr(info->dwName, '\\');
    if (p)
        strcpy(info->dwTitle, p + 1);
    browsebacklist[browseCount++] = info;
}
//-------------------------------------------------------------------------

static int FindBrowseData(PROJECTITEM *pj, char *filname, int curline, FILE *fil, char
    *hint, char *name, int *line, int insertbrowse)
{
    int recsize;
    unsigned char buf[256];
    unsigned char global[256];
    int fileno =  - 1, i;
    int found = FALSE;

    for (i = 0; i < pj->browseInfo->filecount; i++)
        if (!stricmp(pj->browseInfo->filenames[i], filname))
        {
            fileno = i;
            break;
        }
    memset(global, 0, sizeof(global));

    while (1)
    {
        if (fread(buf, 2, 1, fil) <= 0)
            return 0;
        if (*(short*)buf == 0)
            break;
        if (fread(buf + 2, (*(short*)buf) - 2, 1, fil) <= 0)
            return 0;
	    if (*(short *)(buf + 2) & 0x4000) // global var
            memcpy(global, buf, *(short*)buf);
        if (*(int*)(buf + 12) != fileno)
            continue;
        if (*(int*)(buf + 8) !=  - 1)
            if (*(int*)(buf + 4) <= curline)
        if (*(int*)(buf + 8) > curline || curline ==  - 2)
        {
            // -2 == static
            found = TRUE;
            break;
        }
    }
    if (!found && global)
    {
        memcpy(buf, global, *(short*)global);
        found = TRUE;
    }
    if (!found)
        return 0;
    // *(short *)(buf+15) == charpos eventually...
    if (hint)
    {
        memcpy(hint, buf + 20, buf[19]);
        hint[buf[19]] = 0;
    }
    if (line && name)
    {
        if (*(int*)(buf + 12) >= pj->browseInfo->filecount)
            return 0;
        strcpy(name, pj->browseInfo->filenames[*(int*)(buf + 12)]);
        *line = *(int*)(buf + 4);
        if (insertbrowse)
        {
			InsertBrowse(name, curline);
        }
    }
    return 1;
}

//-------------------------------------------------------------------------

void BrowseTo(HWND hwnd, char *msg)
{
    static char name[256];
    int ofs;
    if (defaultWorkArea)
        return ;
    if (!browsing)
    {
        if (msg)
        {
            strcpy(name, msg);
            browsing = TRUE;
        }
        else
            browsing = SendMessage(hwnd, WM_WORDUNDERCURSOR, 0, (LPARAM)name);
        if (!PropGetBool(NULL, "BROWSE_INFORMATION") && browsing)
        {
            ExtendedMessageBox("Browse Info Alert", MB_OK, 
                "Browse information not enabled");
            browsing = FALSE;
            return ;
        }
    }
    else
    {
        SendMessage(hwnd, WM_WORDUNDERCURSOR, 0, (LPARAM)name);
    }
    if (browsing)
    {
        char filename[256];
        FILE *fil;
        DWINFO info;
        CHARRANGE charrange;
        int curline;
        char *filname;
        PROJECTITEM *pj;
        if (msg)
        {
            curline =  - 2;
            filname = "";
        }
        else
        {
            SendDlgItemMessage(hwnd, ID_EDITCHILD, EM_EXGETSEL, (WPARAM)0, 
                (LPARAM) &charrange);
            curline = SendDlgItemMessage(hwnd, ID_EDITCHILD, EM_EXLINEFROMCHAR,
                0, (LPARAM)charrange.cpMin) + 1;
            filname = (char*)SendMessage(hwnd, WM_FILENAME, 0, 0);
        }
        memset(&info, 0, sizeof(info));
        fil = LoadBrowseInfoByHWND(hwnd, name, &pj);
        if (!fil)
        {
            return ;
        }
        if (ofs && FindBrowseData(pj, filname, curline, fil, 0, &info.dwName,
            &info.dwLineNo, TRUE))
        {
            char *p = strrchr(info.dwName, '\\');
            if (p)
                strcpy(info.dwTitle, p + 1);
            info.logMRU = FALSE;
			info.newFile = FALSE;
            CreateDrawWindow(&info, TRUE);
        }
        fclose(fil);
    }
    browsing = FALSE;
}

//-------------------------------------------------------------------------

char *BrowseHint(HWND hwnd)
{
    static char hint[256];
    char *rv = 0;
    char name[256];
    int ofs;
    if (!PropGetBool(NULL, "BROWSE_INFORMATION") || defaultWorkArea)
        return rv;
    if (SendMessage(hwnd, WM_WORDUNDERCURSOR, 0, (LPARAM)name))
    {
        CHARRANGE charrange;
        int curline;
        FILE *fil;
        char *filname = (char*)SendMessage(hwnd, WM_FILENAME, 0, 0);
        PROJECTITEM *pj;
        SendDlgItemMessage(hwnd, ID_EDITCHILD, EM_EXGETSEL, (WPARAM)0, (LPARAM)
            &charrange);
        curline = SendDlgItemMessage(hwnd, ID_EDITCHILD, EM_EXLINEFROMCHAR, 0, 
            (LPARAM)charrange.cpMin) + 1;
        if (!(fil = LoadBrowseInfoByHWND(hwnd, name, &pj)))
        {
            return rv;
        }
        if (ofs && FindBrowseData(pj, filname, curline, fil, hint, 0, 0, FALSE)
            )
            rv = hint;
        fclose(fil);
    }
    return rv;
}

//-------------------------------------------------------------------------

void BrowseBack(void)
{
    if (!browseCount)
        return ;
    browsebacklist[--browseCount]->logMRU = FALSE;
    CreateDrawWindow(browsebacklist[browseCount], TRUE);
    free(browsebacklist[browseCount]);
}
