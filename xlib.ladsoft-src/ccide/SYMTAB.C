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
#include <ctype.h>

#include "header.h"
#include "cvinfo.h"
#include "cvexefmt.h"
#include "pefile.h"

extern PROCESS *activeProcess;
extern enum DebugState uState;

unsigned bitmask[] = 
{
    1, 3, 7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff,
        0x3fff, 0x7fff, 0xffff, 0x1ffff, 0x3ffff, 0x7ffff, 0xfffff, 0x1fffff,
        0x3fffff, 0x7fffff, 0xffffff, 0x1ffffff, 0x3ffffff, 0x7ffffff,
        0xfffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff, 

};

int DeclType(char *typetab, VARINFO *v);


//-------------------------------------------------------------------------

DEBUG_INFO *GetDebugInfo(HANDLE hProcess, DWORD base)
{
    DWORD dbgBase;
    DWORD read;
    if (ReadProcessMemory(hProcess, (LPVOID)(base+0x3c), &dbgBase, 4, &read))
    {
        if (ReadProcessMemory(hProcess, &((struct pe_header_struct *)(base + dbgBase))->debug_rva, &dbgBase, 4, &read))
        {
            if (dbgBase)
            {
                unsigned char buf[512];
                if (ReadProcessMemory(hProcess, (LPVOID)(base + dbgBase), buf, MAX_PATH + 33, &read))
                {
                    if (!memcmp(buf, "LS12", 4))
                    {
                        FILE *fil;
                        int len;
                        unsigned char *info;
                        DEBUG_INFO *rv;
                        DWORD timeStamp = *(DWORD *)(buf + 4);
                        
                        buf[33+buf[32]] = 0;
                        fil = fopen(buf + 33, "rb");
                        if (!fil)
                            return NULL;
                        fseek(fil, 0L, SEEK_END);
                        len = ftell(fil);
                        fseek(fil, 0L, SEEK_SET);
                        info = calloc(1,len);
                        if (info == NULL)
                        {
                            fclose(fil);
                            return NULL;
                        }
                        fread(info, len, 1, fil);
                        fclose(fil);
                        if (memcmp(info, "LS12", 4))
                        {
                            free(info);
                            return 0;
                        }
                        if (memcmp(info + 8, &timeStamp, 4))
                        {
                            free(info);
                            return 0;
                        }
                        rv = (DEBUG_INFO*)calloc(1,sizeof(DEBUG_INFO));
                        if (!rv)
                        {
                            free(info);
                            return 0;
                        }
                        rv->size = len;
                        rv->info = info;
                        rv->base = base;
                        return rv;
                        /* if we get here everything is kosher and it loaded ok. */
                    }
                }
            }
        }
    }
    return NULL;
}

//-------------------------------------------------------------------------

void FreeDebugInfo(DEBUG_INFO *dbg)
{
    if (dbg)
    {
        free(dbg->info);
        free(dbg);
    }
}

//-------------------------------------------------------------------------

static void getname(char *buf, unsigned char *cname)
{
    memcpy(buf, cname + 1, cname[0]);
    buf[cname[0]] = 0;
}

//-------------------------------------------------------------------------

DWORD GetMainAddress(DEBUG_INFO *dbg)
{
    OMFSignature *sig;
    OMFDirHeader *dh;
    OMFDirEntry *e;
    int i;

    if (!dbg)
        return 0;

    sig = dbg->info;
    dh = dbg->info + sig->filepos;
    e = (unsigned char*)dh + dh->cbDirHeader;

    for (i = 0; i < dh->cDir; i++)
    {
        if (e->SubSection == sstGlobalSym)
        {
            int base = e->lfo + sizeof(OMFSymHash);
            int count = e->cb - sizeof(OMFSymHash);
            if (*((short*)(dbg->info + base + 2)) == S_SSEARCH)
            {
                SEARCHSYM *s = base + dbg->info;
                if (s->startsym)
                {
                    int pos = base + s->startsym;
                    PROCSYM32 *p = dbg->info + pos;
                    char namebuf[256];
                    while (p->rectyp == S_GPROC32)
                    {
                        getname(namebuf, p->name);
                        if (!strcmp(namebuf, "main"))
                            return p->off + dbg->base;
                        if (!strcmp(namebuf, "WinMain"))
                            return p->off + dbg->base;
                        if (!strcmp(namebuf, "DllEntryPoint"))
                            return p->off + dbg->base;
                        pos = base + p->pNext;
                        p = dbg->info + pos;
                    }
                }
            }
            return 0;
        }

        e++;
    }
    return 0;
}
static DEBUG_INFO *findDebug(int Address)
{
	DEBUG_INFO *dbg = activeProcess->dbg_info;
    DLL_INFO *dll = activeProcess->dll_info;
    if (!activeProcess->dbg_info)
        return NULL;
    while (dll)
    {
		if ((unsigned)Address >= (unsigned)dll->base && 
			((unsigned)dll->base > (unsigned)activeProcess->dbg_info->base || 
			 (unsigned)Address < (unsigned)activeProcess->dbg_info->base))
			dbg = dll->dbg_info;
        dll = dll->next;
    }
	return dbg;
}

//-------------------------------------------------------------------------

int GetBreakpointLine(int Address, char *module, int *linenum, BOOL next)
{
    OMFSignature *sig;
    OMFDirHeader *dh;
    OMFDirEntry *e;
    int i, j;
    DEBUG_INFO *dbg ;

	dbg = findDebug(Address);

    if (!dbg)
        return 0;

    Address -= dbg->base;
    *linenum = 0;
    sig = dbg->info;
    dh = dbg->info + sig->filepos;
    e = (unsigned char*)dh + dh->cbDirHeader;

    for (i = 0; i < dh->cDir; i++)
    {
        if (e->SubSection == sstModule)
        {
            OMFModule *m;
            OMFSegDesc *s;
            m = dbg->info + e->lfo;
            s = &m->SegInfo;
            for (j = 0; j < m->cSeg; j++)
            {
                if (s->Seg == 1)
                {
                     /* code seg */
                    if (Address >= s->Off && Address < s->Off + s->cbSeg)
                    {
                        char *str = (char*)m->SegInfo + sizeof(OMFSegDesc) *m
                            ->cSeg;
                        OMFDirEntry *e1;
                        memcpy(module, str + 1, str[0]);
                        module[str[0]] = 0;
                        e1 = (unsigned char*)dh + dh->cbDirHeader;
                        for (i = 0; i < dh->cDir; i++)
                        {
                            if (e1->SubSection == sstSrcModule && e->iMod == e1
                                ->iMod)
                            {
                                OMFSourceLine *sl = dbg->info + e1->lfo;
                                short *lines = (char*) &sl->offset + 4 * sl
                                    ->cLnOff;
                                int k;
                                *linenum = lines[0];
                                for (i = 1; i < sl->cLnOff; i++)
                                {
                                    if (next)
                                        *linenum = lines[i];
                                    if (Address < sl->offset[i])
                                        break;
                                    *linenum = lines[i];
                                }
                                for (k=i ; Address != sl->offset[k] && k < sl->cLnOff; k++);
                                if (k < sl->cLnOff)
                                    *linenum = lines[k];
                                if (i > 0)
                                {
                                    return sl->offset[i - 1] + dbg->base;
                                }
                                return 1;
                            }
                            e1++;
                        }
                        return 0;
                    }
                }
                s++;
            }

        }
        e++;
    }
    return 0;
}

//-------------------------------------------------------------------------

static int MatchedModule(char *module, char *dbgname)
{
    char buf[256],  *p;
    memcpy(buf, dbgname + 1, dbgname[0]);
    buf[dbgname[0]] = 0;
    if (!xstricmpz(module, buf))
        return TRUE;
    p = strrchr(module, '\\');
    if (p)
        return !xstricmpz(buf, p + 1);

    return FALSE;
}

//-------------------------------------------------------------------------

int GetBreakpointAddressByProgram(char *module, int *linenum, DEBUG_INFO *dbg,
    int inmodule, int *nearest)
{
    OMFSignature *sig;
    OMFDirHeader *dh;
    OMFDirEntry *e;
    int i, j;

    if (!dbg)
        return 0;
    sig = dbg->info;
    dh = dbg->info + sig->filepos;
    e = (unsigned char*)dh + dh->cbDirHeader;
    for (i = 0; i < dh->cDir; i++)
    {
        if (e->SubSection == sstModule)
        {
            OMFModule *m = dbg->info + e->lfo;
            char *str = (char*)m->SegInfo + sizeof(OMFSegDesc) *m->cSeg;
            if (MatchedModule(module, str))
            {
                OMFDirEntry *e1 = (unsigned char*)dh + dh->cbDirHeader;
                for (i = 0; i < dh->cDir; i++)
                {
                    if (e1->SubSection == sstSrcModule && e->iMod == e1->iMod)
                    {
                        OMFSourceLine *sl = dbg->info + e1->lfo;
                        short *lines = (char*) &sl->offset + 4 * sl->cLnOff;
                        int rv = 0;
                        BOOL tagged = FALSE;
                        for (i = 0; i < sl->cLnOff; i++)
                        {
                            if (*linenum == lines[i])
                            {
								if (nearest)
									*nearest = lines[i];
                                return sl->offset[i] + dbg->base;
                            }
                            else if (*linenum < lines[i])
                            {
                                if (!tagged)
                                {
                                    tagged = TRUE;
    								if (nearest)
	    								*nearest = lines[i];
                                    if (inmodule && i)
                                        rv = sl->offset[i - 1] + dbg->base;
                                }
                            }
                        }
                        return rv;
                    }
                    e1++;
                }
                return 0;
            }

        }
        e++;
    }
    return 0;
}

//-------------------------------------------------------------------------

int GetBreakpointAddress(char *module, int *linenum, int inmodule)
{
    DLL_INFO *dll = activeProcess->dll_info;
    int rv = GetBreakpointAddressByProgram(module, linenum,
        activeProcess->dbg_info, inmodule, NULL);
    if (rv)
        return rv;

    while (dll)
    {
        rv = GetBreakpointAddressByProgram(module, linenum, dll->dbg_info,
            inmodule, NULL);
        if (rv)
            return rv;
        dll = dll->next;
    }
    return 0;
}

//-------------------------------------------------------------------------

int GetBreakpointNearestLine(char *module, int linenum, int inmodule)
{
    if (uState == notDebugging)
    	return linenum;
    if (activeProcess)
    {
        DLL_INFO *dll = activeProcess->dll_info;
    	int nearest = -1;
        int rv ;
    	int ln = linenum;
    	rv = GetBreakpointAddressByProgram(module, &ln,
            activeProcess->dbg_info, inmodule, &nearest);
        if (nearest != -1)
            return nearest;
    
        while (dll)
        {
            rv = GetBreakpointAddressByProgram(module, &ln, dll->dbg_info,
                inmodule, &nearest);
            if (nearest != -1)
                return nearest;
            dll = dll->next;
        }
    }
    return -1;
}

//-------------------------------------------------------------------------

SHORT *GetLineTableByDBG(char *module, DEBUG_INFO *dbg, int *count)
{
    OMFSignature *sig;
    OMFDirHeader *dh;
    OMFDirEntry *e;
    int i, j;

    if (!dbg)
        return 0;
    sig = dbg->info;
    dh = dbg->info + sig->filepos;
    e = (unsigned char*)dh + dh->cbDirHeader;
    for (i = 0; i < dh->cDir; i++)
    {
        if (e->SubSection == sstModule)
        {
            OMFModule *m = dbg->info + e->lfo;
            char *str = (char*)m->SegInfo + sizeof(OMFSegDesc) *m->cSeg;
            if (MatchedModule(module, str))
            {
                OMFDirEntry *e1 = (unsigned char*)dh + dh->cbDirHeader;
                for (i = 0; i < dh->cDir; i++)
                {
                    if (e1->SubSection == sstSrcModule && e->iMod == e1->iMod)
                    {
                        OMFSourceLine *sl = dbg->info + e1->lfo;
                        *count = sl->cLnOff;
                        return (char*) &sl->offset + 4 * sl->cLnOff;
                    }
                    e1++;
                }
                return 0;
            }

        }
        e++;
    }
    return 0;
}

//-------------------------------------------------------------------------

SHORT *GetLineTable(char *module, int *count)
{
    DLL_INFO *dll = activeProcess->dll_info;
    short *rv = 0;
    *count = 0;
    rv = GetLineTableByDBG(module, activeProcess->dbg_info, count);
    if (rv)
        return rv;

    while (dll)
    {
        rv = GetLineTableByDBG(module, dll->dbg_info, count);
        if (rv)
            return rv;
        dll = dll->next;
    }
    return 0;
}

//-------------------------------------------------------------------------

static int FindFunctionInSymtab(DEBUG_INFO *dbg, int symtab, int address)
{
    char *symtab_base = dbg->info + symtab;
    int offset = 0;

    SEARCHSYM *s = symtab_base;
    if (s->rectyp == S_SSEARCH)
    {
        offset = s->startsym;
        while (offset)
        {
            PROCSYM32 *p = symtab_base + offset;
            if (p->rectyp != S_GPROC32 && p->rectyp != S_LPROC32)
                return 0;
            if (address >= p->off && address < p->off + p->len)
            {
                return offset;
            }
            offset = p->pNext;
        }
    }
    return 0;
}

//-------------------------------------------------------------------------

static int FindFunctionByAddress(DEBUG_INFO *dbg, char **symtab, int *offset,
    int Address)
{
    OMFSignature *sig;
    OMFDirHeader *dh;
    OMFDirEntry *e;
    int i, j, k, l;

    sig = dbg->info;
    dh = dbg->info + sig->filepos;
    e = (unsigned char*)dh + dh->cbDirHeader;
    for (i = 0; i < dh->cDir; i++)
    {
        if (e->SubSection == sstModule)
        {
            OMFModule *m;
            OMFSegDesc *s;
            m = dbg->info + e->lfo;
            s = &m->SegInfo;
            for (j = 0; j < m->cSeg; j++)
            {
                if (s->Seg == 1)
                {
                     /* code seg */
                    if (Address >= s->Off && Address < s->Off + s->cbSeg)
                    {
                        OMFDirEntry *e1 = (unsigned char*)dh + dh->cbDirHeader;
                        for (k = 0; k < dh->cDir; k++)
                        {
                            if (e1->SubSection == sstAlignSym && e->iMod == e1
                                ->iMod)
                            {
                                DWORD offs = FindFunctionInSymtab(dbg, e1->lfo,
                                    Address);
                                if (offs)
                                {
                                    *symtab = dbg->info + e1->lfo;
                                    if (offset)
                                        *offset = offs;
                                    return TRUE;
                                }
                                break;
                            }
                            e1++;
                        }
                    }
                    break;
                }
                s++;
            }
        }
        e++;
    }
    e = (unsigned char*)dh + dh->cbDirHeader;
    for (i = 0; i < dh->cDir; i++)
    {
        if (e->SubSection == sstGlobalSym)
        {
            DWORD offs = FindFunctionInSymtab(dbg, e->lfo + sizeof(OMFSymHash),
                Address);
            if (offs)
            {
                *symtab = dbg->info + e->lfo + sizeof(OMFSymHash);
                if (offset)
                    *offset = offs;
                return TRUE;
            }
            else
                return FALSE;
        }
        e++;
    }
    return 0;
}

//-------------------------------------------------------------------------

static int namematch(SYMTYPE *s, char *name)
{
    char *funcname;
    switch (s->rectyp)
    {
        case S_BPREL32:
            funcname = ((BPRELSYM32*)s)->name;
            break;
        case S_REGISTER:

            funcname = ((REGSYM*)s)->name;
            break;
        case S_LDATA32:
        case S_GDATA32:
            funcname = ((DATASYM32*)s)->name;
            break;
        case S_LPROC32:
        case S_GPROC32:
            funcname = ((PROCSYM32*)s)->name;
            break;
        case S_UDT:
            funcname = ((UDTSYM*)s)->name;
            break;
        case S_LABEL32:
            funcname = ((LABELSYM32*)s)->name;
            break;
        default:
            return FALSE;
    }
    if (strlen(name) != funcname[0])
        return 0;
    return !strncmp(funcname + 1, name, funcname[0]);
}

//-------------------------------------------------------------------------

static int FindSymbolInFunction(char *symtab, int offset, char *name, int
    Address)
{
    PROCSYM32 *func = symtab + offset;
    int endoffs = func->pEnd;
    int blockoffs = offset;
    int findoffs = blockoffs;
    BLOCKSYM32 *blk;

    // First find the enclosing block
    while (findoffs < endoffs)
    {
        BLOCKSYM32 *b = symtab + findoffs;
        if (b->rectyp == S_BLOCK32)
        {
            if (b->off <= Address && Address < b->off + b->len)
            {
                blockoffs = findoffs;
                endoffs = b->pEnd;
                findoffs += b->reclen + 2;
            }
            else
                findoffs = b->pEnd;
        }
        else
            findoffs += b->reclen + 2;
    }
    // now blockoffs has the point to the innermost block
    blk = symtab + blockoffs;
    while (blk)
    {
        int end = blk->rectyp == S_BLOCK32 ? blk->pEnd: func->pEnd;
        findoffs = (unsigned char*)blk - (unsigned char*)symtab + blk->reclen +
            2;
        while (findoffs < end)
        {
            BLOCKSYM32 *p = findoffs + symtab;
            if (p->rectyp == S_BLOCK32)
                findoffs = p->pEnd;
            else
            {
                if (namematch(p, name))
                {
                    return findoffs;
                }
                findoffs += p->reclen + 2;
            }
        }
        if (blk->rectyp == S_BLOCK32)
            blk = symtab + blk->pParent;
        else
            blk = 0;
    }
    return 0;
}

//-------------------------------------------------------------------------

static int FindSymbolInSymtab(char *symtab, int len, char *name)
{
    int offset = 0;

    while (offset < len)
    {
        struct SYMTYPE *s = symtab + offset;
        if (s->rectyp == S_LPROC32 || s->rectyp == S_GPROC32)
        {
            PROCSYM32 *p = s;
            if (namematch(s, name))
                return offset;
            offset = p->pEnd;
        } 
        else
        {
            if (namematch(s, name))
            {
                return offset;
            }
            offset = offset + s->reclen + 2;
        }
    }
    return 0;
}

//-------------------------------------------------------------------------

SCOPE *FindSymbol(DEBUG_INFO **dbg_info, char **typetab, char **symtab, int
    *offset, SCOPE *scope, char *name)
{
    OMFSignature *sig;
    OMFDirHeader *dh;
    OMFDirEntry *e;
    int i, j;
    DEBUG_INFO *dbg ;
    int funcoffs;
    SCOPE *rv = scope;

    *typetab = 0;

    if (!scope)
       return 0;
	dbg = findDebug(scope->address);

	if (!dbg)
		return 0;

    *dbg_info = dbg;
    // Find the types table
    sig = dbg->info;
    dh = dbg->info + sig->filepos;

    // See if it is a function argument or variable
    // walks up through the stack...
    while (rv)
    {
        funcoffs = FindFunctionByAddress(dbg, symtab, offset, rv->address - dbg->base);
        if (funcoffs)
        {
            int offs = FindSymbolInFunction(*symtab,  *offset, name, rv->address - dbg
                ->base);
            if (offs)
            {
                e = (unsigned char*)dh + dh->cbDirHeader;
                for (i = 0; i < dh->cDir; i++)
                {
                    if (e->SubSection == sstGlobalTypes)
                    {
                        *typetab = dbg->info + e->lfo;
                        break;
                    }
                    e++;
                }
                if (offset)
                    *offset = offs;
                return rv;
            }
        }
        rv = rv->next;
    }
    if (dbg)
    {
        sig = dbg->info;
        dh = dbg->info + sig->filepos;
        *dbg_info = dbg;
        // check the local symbol table
        e = (unsigned char*)dh + dh->cbDirHeader;
        for (i = 0; i < dh->cDir; i++)
        {
            if (e->SubSection == sstModule)
            {
                OMFModule *m;
                OMFSegDesc *s;
                m = dbg->info + e->lfo;
                s = &m->SegInfo;
                for (j = 0; j < m->cSeg; j++)
                {
                    if (s->Seg == 1)
                    {
                         /* code seg */
                        int k, l;
                        OMFDirEntry *e1 = (unsigned char*)dh + dh->cbDirHeader;

                        for (k = 0; k < dh->cDir; k++)
                        {
                            if (e1->SubSection == sstAlignSym && e->iMod == e1
                                ->iMod)
                            {
                                int offs = FindSymbolInSymtab(dbg->info + e1
                                    ->lfo, e1->cb, name);
                                if (offs)
                                {
                                    e = (unsigned char*)dh + dh->cbDirHeader;
                                    for (l = 0; l < dh->cDir; l++)
                                    {
                                        if (e->SubSection == sstGlobalTypes)
                                        {
                                            *typetab = dbg->info + e->lfo;
                                            break;
                                        }
                                        e++;
                                    }
                                    *symtab = dbg->info + e1->lfo;
                                    if (offset)
                                        *offset = offs;
                                    return scope;
                                }
                                break;
                            }
                            e1++;
                        }
                        //                  }
                        break;
                    }
                    s++;
                }
            }
            e++;
        }
        // And check the global symtab for the program this is a part of
        e = (unsigned char*)dh + dh->cbDirHeader;
        for (i = 0; i < dh->cDir; i++)
        {
            if (e->SubSection == sstGlobalSym)
            {
                int offs = FindSymbolInSymtab(dbg->info + e->lfo + sizeof
                    (OMFSymHash), e->cb - sizeof(OMFSymHash), name);
                if (offs)
                {
                    OMFDirEntry *e1 = (unsigned char*)dh + dh->cbDirHeader;
                    for (i = 0; i < dh->cDir; i++)
                    {
                        if (e1->SubSection == sstGlobalTypes)
                        {
                            *typetab = dbg->info + e1->lfo;
                            break;
                        }
                        e1++;
                    }
                    *symtab = dbg->info + e->lfo + sizeof(OMFSymHash);
                    if (offset)
                        *offset = offs;
                    return scope;
                }
                else
                    return NULL;
            }
            e++;
        }
    }
    return NULL;
}

//-------------------------------------------------------------------------

int FindAddressInSymtab(DEBUG_INFO *dbg, char *symtab, int len, int address,
    char *buf)
{
    int offset = 0;

    while (offset < len)
    {
        struct SYMTYPE *s = symtab + offset;
        switch (s->rectyp)
        {
            case S_LDATA32:
            case S_GDATA32:
                if (address == ((DATASYM32*)s)->off + dbg->base)
                {
                    memcpy(buf, ((DATASYM32*)s)->name + 1, ((DATASYM32*)s)
                        ->name[0]);
                    buf[((DATASYM32*)s)->name[0]] = 0;
                    return offset;
                }
                offset = offset + s->reclen + 2;
                break;
            case S_LPROC32:
            case S_GPROC32:
                if (address == ((PROCSYM32*)s)->off + dbg->base)
                {
                    memcpy(buf, ((PROCSYM32*)s)->name + 1, ((PROCSYM32*)s)
                        ->name[0]);
                    buf[((PROCSYM32*)s)->name[0]] = 0;
                    return offset;
                }
                offset = ((PROCSYM32*)s)->pEnd;
                break;
            case S_LABEL32:
                if (address == ((LABELSYM32*)s)->off + dbg->base)
                {
                    memcpy(buf, ((LABELSYM32*)s)->name + 1, ((LABELSYM32*)s)
                        ->name[0]);
                    buf[((LABELSYM32*)s)->name[0]] = 0;
                    return offset;
                }
                // fallthrough
            default:
                offset = offset + s->reclen + 2;
                break;

        }
    }
    return 0;
}

//-------------------------------------------------------------------------

int FindSymbolByAddress(DEBUG_INFO **dbg_info, char **typetab, char **symtab,
    int *offset, int Address, char *buf)
{
    OMFSignature *sig;
    OMFDirHeader *dh;
    OMFDirEntry *e;
    int i, j;
    DEBUG_INFO *dbg;
    int funcoffs;

    *typetab = 0;

	dbg = findDebug(Address);
    if (!dbg)
        return 0;

    *dbg_info = dbg;
    // Find the types table
    sig = dbg->info;
    dh = dbg->info + sig->filepos;

    if (dbg)
    {
        sig = dbg->info;
        dh = dbg->info + sig->filepos;
        *dbg_info = dbg;
        // check the local symbol table
        e = (unsigned char*)dh + dh->cbDirHeader;
        for (i = 0; i < dh->cDir; i++)
        {
            if (e->SubSection == sstModule)
            {
                OMFModule *m;
                OMFSegDesc *s;
                m = dbg->info + e->lfo;
                s = &m->SegInfo;
                for (j = 0; j < m->cSeg; j++)
                {
                    if (s->Seg == 1)
                    {
                         /* code seg */
                        if (Address - dbg->base >= s->Off && Address - dbg
                            ->base < s->Off + s->cbSeg)
                        {
                            OMFDirEntry *e1 = (unsigned char*)dh + dh
                                ->cbDirHeader;
                            for (i = 0; i < dh->cDir; i++)
                            {
                                if (e1->SubSection == sstAlignSym && e->iMod ==
                                    e1->iMod)
                                {
                                    int offs = FindAddressInSymtab(dbg, dbg
                                        ->info + e1->lfo, e1->cb, Address, buf);
                                    if (offs)
                                    {
                                        e = (unsigned char*)dh + dh
                                            ->cbDirHeader;
                                        for (i = 0; i < dh->cDir; i++)
                                        {
                                            if (e->SubSection == sstGlobalTypes)
                                            {
                                                *typetab = dbg->info + e->lfo;
                                                break;
                                            }
                                            e++;
                                        }
                                        *symtab = dbg->info + e1->lfo;
                                        if (offset)
                                            *offset = offs;
                                        return 1;
                                    }
                                }
                                e1++;
                            }
                        }
                        break;
                    }
                    s++;
                }
            }
            e++;
        }
        // And check the global symtab for the program this is a part of
        e = (unsigned char*)dh + dh->cbDirHeader;
        for (i = 0; i < dh->cDir; i++)
        {
            if (e->SubSection == sstGlobalSym)
            {
                int offs = FindAddressInSymtab(dbg, dbg->info + e->lfo + sizeof
                    (OMFSymHash), e->cb - sizeof(OMFSymHash), Address, buf);
                if (offs)
                {
                    OMFDirEntry *e1 = (unsigned char*)dh + dh->cbDirHeader;
                    for (i = 0; i < dh->cDir; i++)
                    {
                        if (e1->SubSection == sstGlobalTypes)
                        {
                            *typetab = dbg->info + e1->lfo;
                            break;
                        }
                        e1++;
                    }
                    *symtab = dbg->info + e->lfo + sizeof(OMFSymHash);
                    if (offset)
                        *offset = offs;
                    return TRUE;
                }
                else
                    return FALSE;
            }
            e++;
        }
    }


    return 0;
}

//-------------------------------------------------------------------------

PROCSYM32 *FindFunctionSymbol(int Address)
{
    char *symtab;
    DEBUG_INFO *dbg;
    int offset;

	dbg = findDebug(Address);
    if (dbg && FindFunctionByAddress(dbg, &symtab, &offset, Address - dbg->base))
    {

        PROCSYM32 *p = symtab + offset;
        return p;
    }
    return 0;
}

//-------------------------------------------------------------------------

int FindFunctionName(char *buf, int Address)
{
    char *symtab;
    DEBUG_INFO *dbg;
    int offset;
    buf[0] = 0;


	dbg = findDebug(Address);
    if (dbg && FindFunctionByAddress(dbg, &symtab, &offset, Address - dbg->base))
    {
        PROCSYM32 *p = symtab + offset;
        getname(buf, p->name);
        return 1;
    }
    return 0;
}

//-------------------------------------------------------------------------

int GetCalleePushedRegs(int Address, int *ebx, int *esi, int *edi)
{
    char *symtab;
    DEBUG_INFO *dbg;
    int offset;
    int rv = 0;

	dbg = findDebug(Address);
    if (dbg && FindFunctionByAddress(dbg, &symtab, &offset, Address - dbg->base))
    {
        PROCSYM32 *p = symtab + offset;
        SYMTYPE *st = (char *)p + p->reclen+2;
        REGREL32 *rr ;
        if (st->rectyp == S_RETURN)
            rr = (char *)st + st->reclen+2;
        else
            rr = (REGREL32 *)st;
        while (rr->rectyp == S_REGREL32)
        {
            rv |= (1 << rr->reg);
            switch (rr->reg)
            {
                case CV_REG_EBX:
                    *ebx = rr->off;
                    break;
                case CV_REG_ESI:
                    *esi = rr->off;
                    break;
                case CV_REG_EDI:
                    *edi = rr->off;
                    break;
            }
            rr = (char *)rr + rr->reclen + 2;
        }
    }
    return rv;
}

//-------------------------------------------------------------------------

char *LookupType(char *typetab, int typenum)
{
    OMFGlobalTypes *p = typetab;
    int offset;
    typenum -= CV_FIRST_NONPRIM;
    if (typenum < 0 || typenum >= p->cTypes)
        return 0;

    offset = p->typeOffset[typenum] + (char *) &p->cTypes - typetab;

    return offset + typetab;
}

//-------------------------------------------------------------------------

int GetNumericLeaf(short *ptr)
{
    int val = *((unsigned short *)ptr);
    if (val < LF_NUMERIC)
        return val;
    // LF_LONG is the only thing we are supporting
    return *(int*)(ptr + 1);
}

//-------------------------------------------------------------------------

int GetNumericLeafSize(short *ptr)
{
    int val = *((unsigned short *)ptr);
    if (val < LF_NUMERIC)
        return 2;
    // LF_LONG is the only thing we are supporting
    return 5;
}

//-------------------------------------------------------------------------

void GetStructInfo(char *typetab, short *type, VARINFO *var)
{
    short *typeptr = LookupType(typetab, ((lfStructure*)(type + 1))->field);
    VARINFO **v = &var->subtype;
    if (!typeptr)
        return ;
    while (*(typeptr + 1) == LF_FIELDLIST)
    {
        int done = FALSE;
        int len;
        len =  *typeptr - 2;
        typeptr += 2;
        while (len && !done)
        {
            VARINFO *vx;
            int xlen;
            char *nmptr;
            switch (*typeptr)
            {
                case LF_MEMBER:
                    vx =  *v = calloc(sizeof(VARINFO), 1);
                    if (!vx)
                        return ;
                    vx->thread = var->thread;
                    v = &(*v)->link;
                    vx->offset = GetNumericLeaf(&((lfMember*)typeptr)->offset);
                    vx->address = var->address + vx->offset;
                    vx->type = ((lfMember*)typeptr)->index;
                    vx->size = DeclType(typetab, vx);
                    xlen = sizeof(lfMember) - CV_ZEROLEN;
                    if ((*(short*)((char*)typeptr) + xlen) < LF_NUMERIC)
                        xlen += 2;
                    else
                        xlen += 6;
                    nmptr = (char*)typeptr + xlen;
                    strncpy(vx->membername, nmptr + 1,  *nmptr);
                    vx->membername[ *nmptr] = 0;
                    xlen +=  *nmptr + 1;
                    if ((*(((char*)typeptr) + xlen) &0xf0) == 0xf0)
                        xlen += *(((char*)typeptr) + xlen) &15;
                    (char*)typeptr += xlen;
                    break;
                case LF_INDEX:
                    typeptr = LookupType(typetab, ((lfIndex*)typeptr)->index);
                    done = TRUE;
                    break;
                default:
                    return ;
            }
        }
    }
}

//-------------------------------------------------------------------------

int basictypesize(int type)
{
    if (CV_MODE(type) != CV_TM_DIRECT)
        return 4;
    switch (CV_TYPE(type))
    {
        case CV_SIGNED:
        case CV_UNSIGNED:
        case CV_BOOLEAN:
            return 1 << CV_SUBT(type);
        case CV_REAL:
        case CV_IMAGINARY:
            switch (CV_SUBT(type))
            {
            case CV_RC_REAL32:
                return 4;
            case CV_RC_REAL64:
                return 8;
            default:
                return 10;
            }
            break;
        case CV_COMPLEX:
            switch (CV_SUBT(type))
            {
            case CV_RC_REAL32:
                return 8;
            case CV_RC_REAL64:
                return 16;
            default:
                return 20;
            }
            break;
        case CV_INT:
            return 1 << (CV_SUBT(type) / 2);
    }
    return 0;
}

//-------------------------------------------------------------------------

void GetArrayInfo(char *typetab, short *type, VARINFO *var)
{
    int i = 0, z = 0;
    VARINFO **nextptr = &var->subtype;
	if (var->argument)
	{
		DWORD len;
		/* indirection for arrays specified as parameters */
        ReadProcessMemory(activeProcess->hProcess, (LPVOID)var->address, 
						  (LPVOID)&var->address, 4, &len);
	}
    while (i < var->arraysize)
    {
        VARINFO *vx = calloc(sizeof(VARINFO), 1);
        if (!vx)
            return ;
        vx->thread = var->thread;
        vx->offset = i;
        vx->address = var->address;
        if (var->vararray && var->vararraylist && var->vararraylevel < var
            ->vararraylist[0] - 1)
        {
            vx->vararraylevel = var->vararraylevel + 1;
            vx->vararraylist = var->vararraylist;
            vx->vararray = TRUE;
            vx->array = TRUE;
        }
        vx->type = var->type;
        sprintf(vx->membername, "%s[%d]", var->membername, z++);
        i += var->itemsize = vx->size = DeclType(typetab, vx);
        *nextptr = vx;
        nextptr = &vx->link;
    }
}

//-------------------------------------------------------------------------

void GetVararrayInfo(char *typetab, short *type, VARINFO *var)
{
    int aa[1024];
    if (ReadValue(var->address + 4, aa, 4, var) == 4)
    {
        ReadValue(var->address + 8, aa + 1, (aa[0] + 1) *4, var);
    }
    else
        aa[0] = 0;

    if (aa[0] > 98)
        aa[0] = 0;
    ReadValue(var->address, &var->address, 4, var);

    var->vararraylisttofree = var->vararraylist = calloc((aa[0] + 2), sizeof
        (int));
    if (var->vararraylist)
        memcpy(var->vararraylist, aa, (aa[0] + 2) *sizeof(int));
}

//-------------------------------------------------------------------------

void GetPointerInfo(char *typetab, VARINFO *v)
{
    VARINFO *vx = calloc(sizeof(VARINFO), 1);
    if (!vx)
        return ;
    v->subtype = vx;
    vx->type = v->type;
    vx->thread = v->thread;
    sprintf(vx->membername, "*%s", v->membername);
    vx->size = DeclType(typetab, vx);
}

//-------------------------------------------------------------------------

int DeclType(char *typetab, VARINFO *v)
{
    short *typeptr;
    char *p;
    if (v->vararray && v->vararraylist)
    {
        v->arraysize = v->vararraylist[v->vararraylevel + 1];
        GetArrayInfo(typetab, typeptr, v);
        return v->arraysize;
    }
    if (CV_IS_PRIMITIVE(v->type))
    {
        if (CV_TYP_IS_PTR(v->type))
        {
            v->pointer = TRUE;
			v->derefaddress = -1;
            v->type = CV_NEWMODE(v->type, CV_TM_DIRECT);
            v->subtype = 0;
            return 4;
        }
        return basictypesize(v->type);
    }
    typeptr = LookupType(typetab, v->type);
    if (!typeptr)
        return 0;
    v->typetab = typeptr;

    switch (typeptr[1])
    {
        case LF_POINTER:
            v->pointer = TRUE;
			v->derefaddress = -1;
            v->type = ((lfPointer*)(typeptr + 1))->u.utype;
            v->typetab = LookupType(typetab, v->type);
            v->subtype = 0;
            return 4;
        case LF_ARRAY:
            v->array = TRUE;
            v->type = ((lfArray*)(typeptr + 1))->elemtype;
            v->arraysize = GetNumericLeaf(&((lfArray*)(typeptr + 1))->data);
            GetArrayInfo(typetab, typeptr, v);
            return v->arraysize;
        case LF_VARARRAY:
            v->array = TRUE;
            v->vararray = TRUE;
            v->type = ((lfArray*)(typeptr + 1))->elemtype;
            GetVararrayInfo(typetab, typeptr, v);
            v->arraysize = v->size = DeclType(typetab, v);
            return v->arraysize;
        case LF_UNION:
            v->unionx = TRUE;
            GetStructInfo(typetab, typeptr, v);
            v->arraysize = GetNumericLeaf(((lfUnion*)(typeptr + 1))->data);
            p = (char*)(((lfUnion*)(typeptr + 1))->data) + GetNumericLeafSize((
                (lfUnion*)(typeptr + 1))->data);
            memcpy(v->structtag, p + 1,  *p);
            return GetNumericLeaf(((lfUnion*)(typeptr + 1))->data);
        case LF_STRUCTURE:
        case LF_CLASS:
            v->structure = TRUE;
            GetStructInfo(typetab, typeptr, v);
            v->arraysize = GetNumericLeaf(((lfClass*)(typeptr + 1))->data);
            p = (char*)(((lfClass*)(typeptr + 1))->data) + GetNumericLeafSize((
                (lfClass*)(typeptr + 1))->data);
            memcpy(v->structtag, p + 1,  *p);
            return GetNumericLeaf(((lfClass*)(typeptr + 1))->data);
        case LF_ENUM:
            v->enumx = TRUE;
            v->type = ((lfEnum*)(typeptr + 1))->utype;
            v->typetab = LookupType(typetab, ((lfEnum*)(typeptr + 1))->field);
            p = (char*)(((lfEnum*)(typeptr + 1))->Name);
            memcpy(v->structtag, p + 1,  *p);
            return basictypesize(v->type);
        case LF_BITFIELD:
            // Should only appear in structures
            v->bitfield = TRUE;
            v->bitstart = ((lfBitfield*)(typeptr + 1))->position;
            v->bitlength = ((lfBitfield*)(typeptr + 1))->length;
            v->type = ((lfBitfield*)(typeptr + 1))->type;
            return 0;
    }
    return 0;
}

//-------------------------------------------------------------------------

VARINFO *GetVarInfo(DEBUG_INFO *dbg, char *typetab, char *symtab, int offset,
    char *name, SCOPE *scope, THREAD *thread)
{
    SYMTYPE *s = symtab + offset;
    VARINFO *v = calloc(sizeof(VARINFO), 1);
    if (!v)
        return 0;
    strcpy(v->membername, name);
    v->thread = thread;
    v->scope = scope;
    switch (s->rectyp)
    {
        case S_BPREL32:
            v->address = ((BPRELSYM32*)s)->off + scope->basePtr;
            v->type = ((BPRELSYM32*)s)->typind;
			v->argument = ((BPRELSYM32*)s)->off > 0;
            break;
        case S_REGISTER:
            v->inreg = TRUE;
            v->outofscopereg = scope->basePtr != thread->regs.Ebp;
            v->address = ((REGSYM*)s)->reg;
            v->type = ((REGSYM*)s)->typind;
            break;
        case S_LDATA32:
        case S_GDATA32:
            v->address = ((DATASYM32*)s)->off + dbg->base;
            v->type = ((DATASYM32*)s)->typind;
            break;
        case S_LPROC32:
        case S_GPROC32:
            v->address = ((PROCSYM32*)s)->off + dbg->base;
            v->type = T_PVOID;
            break;
        case S_UDT:
            v->udt = TRUE;
            v->type = ((UDTSYM*)s)->typind;
            v->address = 0;
            break;
        case S_LABEL32:
            v->constant = TRUE;
            v->type = T_PVOID;
            v->ival = ((LABELSYM32*)s)->off + dbg->base;
            break;
        default:
            free(v);
            return 0;
    }
    v->size = DeclType(typetab, v);
    return v;

}

//-------------------------------------------------------------------------

void FreeVarInfo(VARINFO *info)
{
    while (info)
    {
        VARINFO *chain = info->link;
        FreeVarInfo(info->subtype);
        if (info->vararraylisttofree)
            free(info->vararraylisttofree);
        free(info);
        info = chain;
    }
}

//-------------------------------------------------------------------------

int ReadValue(int address, void *val, int size, VARINFO *var)
{
    int len;
    if (address < 0x1000 && var->thread && var->thread)
    {
        CONTEXT *regs = &var->thread->regs;
        // register
        switch (address)
        {
            case CV_REG_AL:
                *(char*)val = regs->Eax &0xff;
                return 1;
            case CV_REG_CL:
                *(char*)val = regs->Ecx &0xff;
                return 1;
            case CV_REG_DL:
                *(char*)val = regs->Edx &0xff;
                return 1;
            case CV_REG_BL:
                *(char*)val = regs->Ebx &0xff;
                return 1;
            case CV_REG_AH:
                *(char*)val = (regs->Eax >> 8) &0xff;
                return 1;
            case CV_REG_CH:
                *(char*)val = (regs->Ecx >> 8) &0xff;
                return 1;
            case CV_REG_DH:
                *(char*)val = (regs->Edx >> 8) &0xff;
                return 1;
            case CV_REG_BH:
                *(char*)val = (regs->Ebx >> 8) &0xff;
                return 1;
            case CV_REG_AX:
                *(short*)val = regs->Eax &0xffff;
                return 2;
            case CV_REG_CX:
                *(short*)val = regs->Ecx &0xffff;
                return 2;
            case CV_REG_DX:
                *(short*)val = regs->Edx &0xffff;
                return 2;
            case CV_REG_BX:
                *(short*)val = regs->Ebx &0xffff;
                return 2;
            case CV_REG_SP:
                *(short*)val = regs->Esp &0xffff;
                return 2;
            case CV_REG_BP:
                *(short*)val = regs->Ebp &0xffff;
                return 2;
            case CV_REG_SI:
                *(short*)val = regs->Esi &0xffff;
                return 2;
            case CV_REG_DI:
                *(short*)val = regs->Edi &0xffff;
                return 2;
            case CV_REG_EAX:
                *(int*)val = regs->Eax;
                return 4;
            case CV_REG_ECX:
                *(int*)val = regs->Ecx;
                return 4;
            case CV_REG_EDX:
                *(int*)val = regs->Edx;
                return 4;
            case CV_REG_EBX:
                if (var->scope->regfl & (1 << CV_REG_EBX))
                    *(int *)val = var->scope->ebx;
                else
                    *(int*)val = regs->Ebx;
                return 4;
            case CV_REG_ESP:
                *(int*)val = regs->Esp;
                return 4;
            case CV_REG_EBP:
                *(int*)val = regs->Ebp;
                return 4;
            case CV_REG_ESI:
                if (var->scope->regfl & (1 << CV_REG_ESI))
                    *(int *)val = var->scope->esi;
                else
                    *(int*)val = regs->Esi;
                return 4;
            case CV_REG_EDI:
                if (var->scope->regfl & (1 << CV_REG_EDI))
                    *(int *)val = var->scope->edi;
                else
                    *(int*)val = regs->Edi;
                return 4;
            case CV_REG_ST0:
                // not supported
            case CV_REG_ST1:
            case CV_REG_ST2:
            case CV_REG_ST3:
            case CV_REG_ST4:
            case CV_REG_ST5:
            case CV_REG_ST6:
            case CV_REG_ST7:
                return 0;
        }
    }
    else
    {
        ReadProcessMemory(activeProcess->hProcess, (LPVOID)address, (LPVOID)val,
            size, &len);
        return len;
    }
    return 0;
}

//-------------------------------------------------------------------------

int HintBasicType(VARINFO *info, int *signedtype, char *data)
{
    if (CV_TYP_IS_REAL(info->type) || CV_TYP_IS_IMAGINARY(info->type))
    {
        switch (info->type)
        {
            case T_REAL32:
            case T_IMAGINARY32:
                ReadValue(info->address, data, 4, info);
                break;
            case T_REAL64:
            case T_IMAGINARY64:
                ReadValue(info->address, data, 8, info);
                break;
            case T_REAL80:
            case T_IMAGINARY80:
                ReadValue(info->address, data, 10, info);
                break;
            default:
                return 0;
        }
        return info->type;
    }
    else if (CV_TYP_IS_COMPLEX(info->type)) 
    {
        switch (info->type)
        {
            case T_CPLX32:
                ReadValue(info->address, data, 8, info);
                break;
            case T_CPLX64:
                ReadValue(info->address, data, 16, info);
                break;
            case T_CPLX80:
                ReadValue(info->address, data, 20, info);
                break;
            default:
                return 0;
        }
        return info->type;
	}
	else if (info->type != T_VOID)
    {
        int size = 4;
        *(int*)data = 0;
        if (CV_TYP_IS_SIGNED(info->type))
            *signedtype = TRUE;
        else
            *signedtype = FALSE;
        switch (CV_TYPE(info->type))
        {
            case CV_INT:
                size = 1 << (CV_SUBT(info->type) / 2);
                break;
            case CV_SIGNED:
            case CV_UNSIGNED:
                size = 1 << CV_SUBT(info->type);
                break;
            case CV_BOOLEAN:
                ReadValue(info->address, data, 1, info);
                return info->type;
        }
        ReadValue(info->address, data, size, info);
        if (*signedtype)
        {
            switch (size)
            {
                case 1:
                    if (*data &0x80)
                        *(int*)data |= 0xffffff00;
                    break;
                case 2:
                    if (*(short*)data &0x8000)
                        *(int*)data |= 0xffff0000;
                    break;
                case 4:
                    #ifndef BORLANDC
                        if (*(int*)data &0x80000000)
                            *(LLONG_TYPE*)data |= 0xffffffff00000000;
                    #endif 
                    break;
                case 8:
                    return T_INT8;
                default:
                    return 0;
            }
        }
        else if (size == 8)
            return T_INT8;
        return T_INT4;
    }
}

//-------------------------------------------------------------------------

void HintEnum(char *typetab, VARINFO *info, char *buf, int toenum, int onevalue)
{
    int signedtype;
    LLONG_TYPE v = 0;
    short *typeptr ;
    int cont = TRUE;
	info->size = DeclType(typetab, info);
	typeptr = info->typetab;
    HintBasicType(info, &signedtype, &v);
    while (cont && *(typeptr + 1) == LF_FIELDLIST)
    {
        int len;
		cont = FALSE;
        len =  *typeptr - 2;
        typeptr += 2;
        while (len > 0 && !cont)
        {
            int xlen;
            char *nmptr;
            switch (*typeptr)
            {
                case LF_ENUMERATE:
                    xlen = sizeof(lfEnumerate) - 1;
                    if (v < LF_NUMERIC)
                        xlen += 2;
                    else
                        xlen += 6;
                    nmptr = ((char*)typeptr) + xlen;
                    if (GetNumericLeaf(&((lfEnumerate*)typeptr)->value) == v)
                    {
                        char buf1[256];
                        memset(buf1, 0, 256);
                        strncpy(buf1, nmptr + 1,  *nmptr);
                        if (toenum)
                            sprintf(buf, "ENUM: %s(%u)", buf1, v);
                        else if (onevalue)
                            sprintf(buf, "%s", buf1);
                        else
                            sprintf(buf, "%s(%u)", buf1, v);
                        return ;
                    }
                    xlen +=  *nmptr + 1;
                    if ((*((char*)typeptr + xlen) &0xf0) == 0xf0)
                        xlen += *((char*)typeptr + xlen) &15;
                    (char*)typeptr += xlen;
                    len -= xlen;
                    break;
                case LF_INDEX:
                    typeptr = LookupType(typetab, ((lfIndex*)typeptr)->index);
                    cont = TRUE;
                    break;
                default:
                    sprintf(buf, "ENUM: (UNKNOWN)(%u)", v);
                    return ;
            }
        }
    }
    sprintf(buf, "ENUM: (UNKNOWN)(%u)", v);
}

//-------------------------------------------------------------------------

int HintBf(VARINFO *info, int *signedtype)
{
    char data[20];
    int v;
    HintBasicType(info, signedtype, &data);
    v = *(int*)data;
    if (*signedtype)
    {
        v <<= 32-info->bitstart - info->bitlength;
        v >>= 32-info->bitlength;
    }
    else
    {
        v >>= info->bitstart;
        v &= bitmask[info->bitlength - 1];
    }
    return v;
}

//-------------------------------------------------------------------------

void GetStringValue(VARINFO *info, char *buf, int len, int address)
{
    int i;
    char buf2[256],  *p;
    if (info->type == T_CHAR && ReadValue(address, buf2, len, info))
    {
        int j;
        buf += strlen(buf);
        *buf++ = '\"';
        p = buf2;
        for (i = 0; i < len &&  *p; i++)
            if (isprint(*p))
                *buf++ =  *p++;
            else
        {
            char temp[5];
            int l;
            *buf++ = '\\';
            itoa((unsigned char) *p++, temp, 8);
            l = strlen(temp);
            for (j = 0; j < (3-l); j++)
                    *buf++ = '0';
            for (j = 0; j < l; j++)
                *buf++ = temp[j];
        }
        *buf++ = '"';
        *buf++ = 0;
    }
}

//-------------------------------------------------------------------------

void HintValue(char *typetab, VARINFO *info, char *buf)
{
    int i;
    if (info->outofscope)
//         || info->outofscopereg)
        strcpy(buf, "out of scope");
    else if (info->constant)
    {
		if (info->type == T_VOID)
			sprintf(buf,"%p", info->address);
        if (CV_TYP_IS_REAL(info->type) || CV_TYP_IS_IMAGINARY(info->type))
            sprintf(buf, "%f", (double)info->fval);
        else if (CV_TYP_IS_COMPLEX(info->type))
            sprintf(buf, "%f + %f * I", (double)info->fval,(double)info->fvali) ;
        else
            sprintf(buf, "%lld(%llx)", info->ival);
    }
    else if (info->structure)
    {
        sprintf(buf, "STRUCTURE: %p", info->address);
    }
    else if (info->unionx)
    {
        sprintf(buf, "UNION: %p", info->address);
    }
    else if (info->pointer)
    {
        int val;
        char buf2[256],  *p;
		if (info->derefaddress != -1)
		{
            sprintf(buf, "POINTER: %p ", info->derefaddress);
            GetStringValue(info, buf + strlen(buf), 32, info->derefaddress);
		}
        else if (ReadValue(info->address, &val, 4, info))
        {
            sprintf(buf, "POINTER: %p ", val);
            GetStringValue(info, buf + strlen(buf), 32, val);
        }
        else
            sprintf(buf, "POINTER: <UNKNOWN>");
    }
    else if (info->enumx)
        HintEnum(typetab, info, buf, TRUE, FALSE);
    else if (info->bitfield)
    {
        int signedtype;
        int v = HintBf(info, &signedtype);
        if (signedtype)
            sprintf(buf, "%d(%x)", v, v);
        else
            sprintf(buf, "%u(%x)", v, v);
    }
    else if (info->array)
    {
        sprintf(buf, "ARRAY: %p ", info->address);
        GetStringValue(info, buf + strlen(buf), 32, info->address);
    }
    else
    {
        int signedtype;
        char buf1[20];
        LLONG_TYPE v;
        switch (HintBasicType(info, &signedtype, buf1))
        {
            case T_INT8:
                #ifndef BORLANDC
                    v = *(LLONG_TYPE*)buf1;
                    if (signedtype)
                        sprintf(buf, "%lld(0x%llx)", v, v);
                    else
                        sprintf(buf, "%llu(0x%llx)", v, v);
                    break;
                #endif 
            default:
                sprintf(buf, "unknown type");
                break;
            case T_INT4:
                v = *(int*)buf1;
                if (signedtype)
                    sprintf(buf, "%d(0x%x)", (int)v, (int)v);
                else
                    sprintf(buf, "%u(0x%x)", (int)v, (int)v);
                break;
            case T_BOOL08:
                if (buf1[0])
                    sprintf(buf, "True");
                else
                    sprintf(buf, "False");
                break;
            case T_REAL32:
            case T_IMAGINARY32:
                sprintf(buf, "%f", (double)*(float*)buf1);
                break;
            case T_REAL80:
            case T_IMAGINARY80:
                *(double*)buf1 = *(long double*)buf1;
            case T_REAL64:
            case T_IMAGINARY64:
                sprintf(buf, "%f", *(double*)buf1);
                break;
            case T_CPLX32:
                sprintf(buf, "%f + %f * I", (double)*(float *)buf1, (double) *(float *)(buf1+4));
                break;
            case T_CPLX64:
                sprintf(buf, "%f + %f * I", *(double*)buf1, *(double *)(buf1+8));
                break;
            case T_CPLX80:
                sprintf(buf, "%f + %f * I", (double)*(long double*)buf1, (double)*(long double *)(buf1+10));
                break;
        }
    }
}

//-------------------------------------------------------------------------

void SimpleTypeName(char *buf, int type)
{
    char *p = buf;
    if (CV_TYP_IS_REAL(type) || CV_TYP_IS_IMAGINARY(type))
    {
        switch (type)
        {
            case T_REAL32:
                sprintf(buf, "float ");
                break;
            case T_REAL64:
                sprintf(buf, "double ");
                break;
            case T_REAL80:
                sprintf(buf, "long double ");
                break;
            case T_IMAGINARY32:
                sprintf(buf, "float imaginary ");
                break;
            case T_IMAGINARY64:
                sprintf(buf, "double imaginary ");
                break;
            case T_IMAGINARY80:
                sprintf(buf, "long double imaginary ");
                break;
            default:
                sprintf(buf, "unknown");
                break;
        }
    }
    else if (CV_TYP_IS_COMPLEX(type))
    {
        switch(type)
        {
            case T_CPLX32:
                sprintf(buf, "float complex ");
                break;
            case T_CPLX64:
                sprintf(buf, "double complex ");
                break;
            case T_CPLX80:
                sprintf(buf, "long double complex ");
                break;
        }
    }
	else if (type == T_VOID)
	{
		sprintf(p, "void *");
	}
	else
    {
        int size;
        if (!CV_TYP_IS_SIGNED(type))
        {
            sprintf(p, "unsigned ");
            p += strlen(p);
        }
        switch (CV_TYPE(type))
        {
            case CV_INT:
                size = 1 << (CV_SUBT(type) / 2);
                break;
            case CV_SIGNED:
            case CV_UNSIGNED:
                size = 1 << CV_SUBT(type);
                break;
            case CV_BOOLEAN:
                size =  - 1;
                break;
        }
        switch (size)
        {
            case  - 1: sprintf(p, "bool ");
            break;
            case 1:
                sprintf(p, "char ");
                break;
            case 2:
                sprintf(p, "short ");
                break;
            case 4:
                sprintf(p, "int ");
                break;
            case 8:
                sprintf(p, "long long ");
                break;
            default:
                break;
        }
        p += strlen(p);
    }

    return ;
}

//-------------------------------------------------------------------------

char *SymTypeName(char *buf, char *typetab, VARINFO *v)
{
    short *typeptr;
    char *p = buf;
    p[0] = 0;
    if (!v)
        return buf;
    if (v->constant)
    {
		if (v->type == T_VOID)
			strcpy(buf, "void * ");
		else
	        strcpy(buf, "constant ");
        return buf;
    }
    else if (v->pointer)
    {
        if (CV_IS_PRIMITIVE(v->type))
		{
            SimpleTypeName(buf, v->type &~CV_MMASK);
            strcat(buf, "* ");
		}
        else
        {
            int replace = !v->subtype;
            if (!v->subtype)
                GetPointerInfo(typetab, v);
            SymTypeName(buf + strlen(buf), typetab, v->subtype);
            strcat(buf, "* ");
            if (replace)
            {
                FreeVarInfo(v->subtype);
                v->subtype = 0;
            }
        }
    }
    else if (CV_IS_PRIMITIVE(v->type))
    {
        SimpleTypeName(buf, v->type &~CV_MMASK);
        if (CV_TYP_IS_PTR(v->type))
            strcat(buf, "* ");
    }
    else if (v->unionx)
    {
        sprintf(buf, "union ");
        p = buf + strlen(buf);
        sprintf(p, "%s", v->structtag);
    }
    else if (v->structure)
    {
        sprintf(buf, "struct ");
        p = buf + strlen(buf);
        sprintf(p, "%s", v->structtag);
    }
    else if (v->enumx)
    {
        sprintf(buf, "enum ");
        p = buf + strlen(buf);
        sprintf(p, "%s", v->structtag);
    }
    if (v->array)
    {
        strcat(buf, "[] ");
    }
    if (v->bitfield)
    {
        p = buf + strlen(buf);
        sprintf(p, ":%d", v->bitlength);
    }
    return buf;
}
