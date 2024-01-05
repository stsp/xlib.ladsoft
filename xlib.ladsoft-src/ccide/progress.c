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
#include <stdio.h>
#include <commctrl.h>
#include <stdarg.h>

#include "header.h"
#include "progress.h"
#include "winconst.h"

static char *szProgName = "LADPROGRESS";
// ==============================================================
//
/*
 * Progress bar window, or zero if no progress bar
 */
HWND hWndProgress;

/* limit of progress bar */
static int proglim = 0;

// ==============================================================
//
/* progress bar dialog box */
int FAR PASCAL ProgressProc(HWND hWndDlg, UINT wmsg, WPARAM wparam, LPARAM
    lparam)
{
    switch (wmsg)
    {
        case WM_INITDIALOG:
            CenterWindow(hWndDlg);
            SetProgress(0, "");
            return TRUE;
        case WM_CLOSE:
            PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wparam))
            {
            case IDCANCEL:
                /* They pressed cancel, get out */
                DeleteProgress();
                break;
            }
    }
    return 0;
}

// ==============================================================
//
/* Build the progress bar, set a limit and title */
void MakeProgress(HWND hWnd, HINSTANCE hInst, LPCSTR title, long value)
{
    /* Can't have a progress bar with a limit of 0 */
    if (value == 0)
        return ;

    /* Set new limit */
    proglim = value;

    /* Get out if bar already up */
    if (hWndProgress)
        return ;

    /* Make window and set title */
    hWndProgress = CreateDialog(hInst, "DLG_PROGRESS", hWnd, ProgressProc);
    SetWindowText(hWndProgress, title);

    SendMessage(hWndProgress, PBM_SETSTEP, 4, 0);
}

// ==============================================================
//
/* Delete the progress bar window */
void DeleteProgress(void)
{
    if (hWndProgress)
    {
        DestroyWindow(hWndProgress);
        hWndProgress = NULL;
    }
}

// ==============================================================
//
/* Set the text field and the progress bar current value fields */
void SetProgress(long value, LPCSTR fmt, ...)
{
    /* calculate percentage */
    long percent = value * 100 / proglim;
    char string[64], text[1024];
    HWND hWndTemp;
    va_list argptr;

    /* Never greater than 100 percent */
    if (percent > 100)
        percent = 100;

    /* Collect the string for the caption field */

    va_start(argptr, fmt);
    vsprintf(text, fmt, argptr);
    va_end(argptr);

    /* Now set the text of the caption field */
    hWndTemp = GetDlgItem(hWndProgress, IDC_PBNAME);
    SendMessage(hWndTemp, WM_SETTEXT, 0, (LPARAM)text);

    /* Now set the percentage field on the right */
    sprintf(string, "%d%%   ", percent);
    hWndTemp = GetDlgItem(hWndProgress, IDC_PBPERCENT);
    SendMessage(hWndTemp, WM_SETTEXT, 0, (LPARAM)string);

    /* Now set the progress bar percentage */
    hWndTemp = GetDlgItem(hWndProgress, IDC_PROGRESSBAR);
    SendMessage(hWndTemp, PBM_SETPOS, percent, 0);
}
