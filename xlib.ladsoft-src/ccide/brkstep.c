//THREAD *GetThread(PROCESS *proc, DWORD id);
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

#define TF_BIT 0x100

#include "header.h"

extern PROCESS *activeProcess;
extern THREAD *activeThread;
extern HWND hwndClient;
extern HWND hwndASM;
extern enum DebugState uState;
extern HANDLE BreakpointSem;

BOOL SingleStepping;
static int LastSkipAddr;


//-------------------------------------------------------------------------

void SingleStep(DWORD procID, DWORD threadID)
{
    activeThread->regs.EFlags |= TF_BIT;
    SingleStepping = TRUE;
}

//-------------------------------------------------------------------------

void ClearTraceFlag(DWORD procID, DWORD threadID)
{
    SingleStepping = FALSE;
}

int StepOverIncrement(DEBUG_EVENT *dbe)
{
    unsigned char buf[16];
    int word = 0;
    int address;
    int skiplen = 0;
    ReadProcessMemory(activeProcess->hProcess, (LPVOID)(address = dbe
        ->u.Exception.ExceptionRecord.ExceptionAddress), (LPVOID)buf, 16, 0);
    switch (buf[0])
    {
        case 0xCE:
            skiplen = 1;
            break;
        case 0xCD:
            skiplen = 2;
            break;
        case 0xE8:
            skiplen = 5;
            break;
        default:
            word = (*(short*)buf) &0x38ff;
            if (word == 0x10ff || word == 0x18ff)
            {
                skiplen = 2;
                if ((word = (buf[1] &0xc7)) < 0xc0)
                {
                    // not indirect through reg
                    if (word == 6)
                    // Offset
                        skiplen += 4;
                    else
                    {
                        if (word >= 8)
                            if (word >= 0x80)
                                skiplen += 4;
                            else
                                skiplen += 1;
                        word &= 7;
                        if (word == 4)
                        {
                            skiplen++; /* has a SIB */
                            if (*(buf + 1) < 0xc0)
                            {
                                word = *(buf + 2);
                                if ((word &7) == 5)
                                {
                                     /* EBP special cases */
                                    if (*(buf + 1) &0x40)
                                        skiplen += 4;
                                    else
                                        skiplen += 1;
                                }
                            }
                        }
                    }
                }
            }
            break;
    }
    if (skiplen)
        return address + skiplen;
    return 0;
}

//-------------------------------------------------------------------------

int DoStepOver(DEBUG_EVENT *dbe)
{
    int skipaddr = StepOverIncrement(dbe);
    if (skipaddr)
    {
        SetTempBreakPoint(dbe->dwProcessId, dbe->dwThreadId, skipaddr);
    }
    else
    {
        SingleStep(dbe->dwProcessId, dbe->dwThreadId);
    }
}

//-------------------------------------------------------------------------

int DoStepIn(DEBUG_EVENT *dbe)
{
    LastSkipAddr = StepOverIncrement(dbe);
    SingleStep(dbe->dwProcessId, dbe->dwThreadId);
}

//-------------------------------------------------------------------------

int IsStepping(DEBUG_EVENT *dbe)
{
    char module[256];
    int line;
    if (uState == SteppingOver)
    {
        int v;
        if ((v = GetBreakpointLine((int)dbe
            ->u.Exception.ExceptionRecord.ExceptionAddress, module, &line,FALSE)) !=
            dbe->u.Exception.ExceptionRecord.ExceptionAddress)
        {
            DoStepOver(dbe);
            return TRUE;
        }
        else
        {
            uState = Running;
            return FALSE;
        }
    }
    else if (uState == SteppingIn)
    {
        int addr = GetBreakpointLine((int)dbe
            ->u.Exception.ExceptionRecord.ExceptionAddress, module, &line, FALSE);
        if (addr == dbe->u.Exception.ExceptionRecord.ExceptionAddress)
        {
            uState = Running;
            return FALSE;
        }
        else if (LastSkipAddr)
        {
            if (dbe->u.Exception.ExceptionRecord.ExceptionAddress !=
                LastSkipAddr)
            {
                if (addr)
                {
                    uState = SteppingOver;
                    DoStepOver(dbe);
                    return TRUE;
                }
                else
                {
                    SetTempBreakPoint(dbe->dwProcessId, dbe->dwThreadId,
                        LastSkipAddr);
                    uState = StepInOut;
                    return TRUE;
                }
            }
            else
            {
                DoStepIn(dbe);
                return TRUE;
            }
        }
        else
        {
            DoStepIn(dbe);
            return TRUE;
        }
    }
    else if (uState == StepInOut)
    {
        int addr = GetBreakpointLine((int)dbe
            ->u.Exception.ExceptionRecord.ExceptionAddress, module, &line, FALSE);
        if (addr == dbe->u.Exception.ExceptionRecord.ExceptionAddress)
        {
            uState = Running;
            return FALSE;
        }
        else
        {
            uState = SteppingIn;
            DoStepIn(dbe);
            return TRUE;
        }
    }
    else if (uState == FinishStepOut)
    {
        int addr = GetBreakpointLine((int)dbe
            ->u.Exception.ExceptionRecord.ExceptionAddress, module, &line, FALSE);
        if (!addr || addr == dbe->u.Exception.ExceptionRecord.ExceptionAddress)
        {
            uState = Running;
            return FALSE;
        }
        else
        {
            uState = SteppingIn;
            DoStepIn(dbe);
            return TRUE;
        }
    }
    return FALSE;
}

//-------------------------------------------------------------------------

void StepOver(DEBUG_EVENT *dbe)
{
    if (dmgrGetHiddenState(DID_ASMWND) || (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0) != GetParent(hwndASM))
	{
        uState = SteppingOver;
	}
	else
		SetStatusMessage("Disassembly window open - stepping via assembly", FALSE);
    SaveRegisterContext();
    DoStepOver(dbe);
    ReleaseSemaphore(BreakpointSem, 1, 0);
}

//-------------------------------------------------------------------------

void StepOut(DEBUG_EVENT *dbe)
{
    uState = SteppingOut;
    SaveRegisterContext();
    ReleaseSemaphore(BreakpointSem, 1, 0);
}

//-------------------------------------------------------------------------

void StepIn(DEBUG_EVENT *dbe)
{
    if (dmgrGetHiddenState(DID_ASMWND))
	{
        uState = SteppingIn;
	}
	else
		SetStatusMessage("Disassembly window open - stepping via assembly", FALSE);
    SaveRegisterContext();
    DoStepIn(dbe);
    ReleaseSemaphore(BreakpointSem, 1, 0);
}

//-------------------------------------------------------------------------

int RunTo(DEBUG_EVENT *dbe)
{
    int addr;
    HWND wnd = GetFocus();
    if (wnd == hwndASM)
    {
        if (!(addr = SendMessage(hwndASM, WM_GETCURSORADDRESS, 0, 0)))
            return FALSE;
        SetTempBreakPoint(dbe->dwProcessId, dbe->dwThreadId, addr);
        SaveRegisterContext();
        ReleaseSemaphore(BreakpointSem, 1, 0);
    }
    else
    {
        wnd = GetParent(wnd);
        if (!IsSpecialWindow(wnd))
        {
            DWINFO *ptr = (DWINFO*)GetWindowLong(wnd, 0);
            int linenum;
            SendMessage(ptr->dwHandle, EM_GETSEL, (WPARAM) &linenum, 0);
            linenum = SendMessage(ptr->dwHandle, EM_EXLINEFROMCHAR, 0, linenum)
                + 1;
            linenum = TagOldLine(ptr->dwName, linenum);
            addr = GetBreakpointAddress(ptr->dwName, &linenum, FALSE);
            if (addr)
            {
                SetTempBreakPoint(dbe->dwProcessId, dbe->dwThreadId, addr);
                SaveRegisterContext();
                ReleaseSemaphore(BreakpointSem, 1, 0);
            }
            else
                return FALSE;
        }
        else
            return FALSE;
    }
    return TRUE;
}

//-------------------------------------------------------------------------

int isSteppingOut(DEBUG_EVENT *dbe)
{
    if (uState == SteppingOut)
    {
        unsigned char bf = 0;
        ReadProcessMemory(activeProcess->hProcess, (LPVOID)activeThread
            ->regs.Eip, (LPVOID) &bf, 1, 0);
        if (bf == 0xc2 || bf == 0xc3)
        {
            SingleStep(dbe->dwProcessId, dbe->dwThreadId);
            uState = FinishStepOut;
        }
        else
            DoStepOver(dbe);
        return TRUE;
    }
    return FALSE;
}

