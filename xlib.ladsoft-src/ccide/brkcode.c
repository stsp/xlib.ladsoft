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
#include <richedit.h>

#include "header.h"

extern PROCESS *activeProcess;
extern HWND hwndASM;
extern DWINFO *editWindows;
extern enum DebugState uState;

//-------------------------------------------------------------------------

void SetTempBreakPoint(int procid, int threadid, int address)
{
    activeProcess->breakpoints.address = address;
    activeProcess->idTempBpThread = threadid;
}

//-------------------------------------------------------------------------

void ClearTempBreakPoint(int procid)
{
    activeProcess->breakpoints.address = 0;
}

//-------------------------------------------------------------------------

int SittingOnBreakPoint(DEBUG_EVENT *dbe)
{
    return isBreakPoint((int)dbe->u.Exception.ExceptionRecord.ExceptionAddress);
}

//-------------------------------------------------------------------------

void WriteBreakPoint(HANDLE hProcess, int address, int value)
{
    int bf = value;
    MEMORY_BASIC_INFORMATION mbi;
    DWORD dwOldProtect;

    //	if (!IsNT() && address >= 0x80000000)
    //		return ;
    if (!VirtualQueryEx(hProcess, (LPVOID)address, &mbi, sizeof(mbi)))
        ExtendedMessageBox("Debugger", MB_SYSTEMMODAL, "Could not query pages %d",
            GetLastError());
	// the following fails on win98
    /*if*/ (!VirtualProtectEx(hProcess, mbi.BaseAddress, mbi.RegionSize,
        PAGE_EXECUTE_WRITECOPY, &mbi.Protect));
//        ExtendedMessageBox("Debugger", MB_SYSTEMMODAL, 
//            "Could not protect pages %d", GetLastError());

    if (!WriteProcessMemory(hProcess, (LPVOID)address, (LPVOID) &bf, 1, 0))
        ExtendedMessageBox("Debugger", MB_SYSTEMMODAL, 
            "Could not write breakpoint address %x %d", address, GetLastError())
            ;

    VirtualProtectEx(hProcess, mbi.BaseAddress, mbi.RegionSize, mbi.Protect,
        &dwOldProtect);
    FlushInstructionCache(hProcess, (LPVOID)address, 1);
}

//-------------------------------------------------------------------------

void allocBreakPoint(HANDLE hProcess, BREAKPOINT *pt)
{
    unsigned char bf;
    if (!pt->active && pt->address)
    {
        DWORD len;
        if (ReadProcessMemory(hProcess, (LPVOID)pt->address, (LPVOID) &bf, 1,
            &len))
        {
            if (!len)
                return ;
            if (bf == 0xcc)
                return ;
            WriteBreakPoint(hProcess, pt->address, 0xcc);
            pt->active = TRUE;
            pt->tempVal = bf;

        }
    }
}

//-------------------------------------------------------------------------

void freeBreakPoint(HANDLE hProcess, BREAKPOINT *pt)
{
    if (pt->active)
    {
        WriteBreakPoint(hProcess, pt->address, pt->tempVal);
        pt->active = FALSE;
    }
}

//-------------------------------------------------------------------------

void SetBreakPoints(int procid)
{
    BREAKPOINT *p = &activeProcess->breakpoints;
    hbpSetBP();
	databpSetBP(activeProcess->hProcess);
    while (p)
    {
        if (p->address)
            allocBreakPoint(activeProcess->hProcess, p);
        p = p->next;
    }

}

//-------------------------------------------------------------------------

void ClearBreakPoints(int procid)
{
    BREAKPOINT *p = &activeProcess->breakpoints;
    THREAD *th = activeProcess->threads;
    hbpResetBP();
	databpResetBP(activeProcess->hProcess);
    while (p)
    {
        freeBreakPoint(activeProcess->hProcess, p);
        p = p->next;
    }
    while (th)
    {
        freeBreakPoint(activeProcess->hProcess, &th->breakpoint);
        th = th->next;
    }

}

//-------------------------------------------------------------------------

int isBreakPoint(int addr)
{
    BREAKPOINT *p = activeProcess->breakpoints.next;
    THREAD *th = activeProcess->threads;
    while (p)
    {
        if (addr == p->address)
            return TRUE;
        p = p->next;
    }
    return FALSE;
}

//-------------------------------------------------------------------------

int isLocalBreakPoint(int addr)
{
    THREAD *th = activeProcess->threads;
    while (th)
    {
        if (addr == th->breakpoint.address)
            return TRUE;
        th = th->next;
    }
    return isBreakPoint(addr);
}

//-------------------------------------------------------------------------

#ifdef XXXXX
    int IsBreakpointLine(char *module, int line)
    {
        BREAKPOINT *p = activeProcess->breakpoints.next;
        while (p)
        {
            if (p->linenum)
                if (p->linenum == line && !xstricmp(module, p->module))
                    return TRUE;
            p = p->next;
        }
        return FALSE;
    }
#endif 

//-------------------------------------------------------------------------

int dbgSetBreakPoint(char *name, int linenum, char *extra)
{
    BREAKPOINT **p = &activeProcess->breakpoints.next;
    if (uState == notDebugging)
        return 1;
    // will be checked when entering debugger
    if (!name)
    {
        // it was from the assembly window
        int addr = linenum;
        while (*p)
        {
            if ((*p)->address == addr)
                return 1 ;
            p = &(*p)->next;
        }
        *p = calloc(1,sizeof(BREAKPOINT));
        if (*p)
        {
            memset(*p, 0, sizeof(BREAKPOINT));
            (*p)->address = addr;
            (*p)->extra = extra;
            GetBreakpointLine(addr, &(*p)->module, &(*p)->linenum, FALSE);
            if (hwndASM)
                InvalidateRect(hwndASM, 0, 0);
//            Tag(TAG_BP, (*p)->module, (*p)->linenum, 0, 0, 0, 0);
            if (uState == Running)
                allocBreakPoint(activeProcess->hProcess,  *p);
        }
        return 1;
    }
    else
    {
        // it was from a source module
        int addr = GetBreakpointAddress(name, &linenum, FALSE);
        if (addr)
        {
            while (*p)
            {
                if ((*p)->address == addr)
                    return 1 ;
                p = &(*p)->next;
            }
            *p = calloc(1,sizeof(BREAKPOINT));
            if (*p)
            {
                memset(*p, 0, sizeof(BREAKPOINT));
                (*p)->address = addr;
                (*p)->extra = extra;
                strcpy((*p)->module, name);
                (*p)->linenum = linenum;
                InvalidateRect(hwndASM, 0, 0);
                if (uState == Running)
                    allocBreakPoint(activeProcess->hProcess,  *p);
            }
            return 1;
        }
        else
            return 0;
        // couldn't locate line, invalid line...
    }
}

//-------------------------------------------------------------------------

void dbgClearBreakPoint(char *name, int linenum)
{
    BREAKPOINT **p = &activeProcess->breakpoints.next;
    if (uState == notDebugging)
        return ;
    if (!name)
    {
        // from ASM window
        int addr = linenum;
        while (*p)
        {

            if ((*p)->address == addr)
            {
                BREAKPOINT *q =  *p;

                *p = (*p)->next;
//                Tag(TAG_BP, q->module, q->linenum, 0, 0, 0, 0);
                if (uState == Running)
                    freeBreakPoint(activeProcess->hProcess, q);
                free(q->extra);
                free(q);
                InvalidateRect(hwndASM, 0, 0);
                return ;
            }
            p = &(*p)->next;
        }
    }
    else
    {
        int addr = GetBreakpointAddress(name, &linenum, FALSE);
        if (addr)
        {
            while (*p)
            {
                if ((*p)->address == addr)
                {
                    BREAKPOINT *q =  *p;
                    *p = (*p)->next;
                    if (uState == Running)
                        freeBreakPoint(activeProcess->hProcess, q);
                    free(q->extra);
                    free(q);
                    if (hwndASM)
                        InvalidateRect(hwndASM, 0, 0);
                    return ;
                }
                p = &(*p)->next;
            }
        }
    }
}

//-------------------------------------------------------------------------

void SetBP(DEBUG_EVENT *dbe)
{
    BREAKPOINT **p = &activeProcess->breakpoints.next;
    if (hwndASM && GetFocus() == hwndASM)
    {
        int addr = SendMessage(hwndASM, WM_GETCURSORADDRESS, 0, 0);
        Tag(TAG_BP, 0, addr, 0, 0, 0, 0);
        if (uState == Running)
            allocBreakPoint(activeProcess->hProcess,  *p);
    }
    else
    {
        HWND hWnd = GetFocus();
        DWINFO *ptr = editWindows;
        while (ptr)
        {
            if (ptr->dwHandle == hWnd)
            {
                int addr, linenum;
                SendMessage(ptr->dwHandle, EM_GETSEL, (WPARAM) &linenum, 0);
                linenum = SendMessage(ptr->dwHandle, EM_EXLINEFROMCHAR, 0,
                    linenum) + 1;
                Tag(TAG_BP, ptr->dwName, linenum, 0, 0, 0, 0);
                break;
            }
            ptr = ptr->next;
        }
    }
	if (hwndASM)
	    InvalidateRect(hwndASM, 0, 0);
}

