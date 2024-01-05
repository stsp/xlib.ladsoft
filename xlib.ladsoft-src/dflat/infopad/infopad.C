/* --------------- infopad.c ----------- */

#include "dflat.h"
#include "dide.h"
#include "signal.h"

extern DBOX PrintSetup;
extern int EditorProc(WINDOW, MESSAGE, PARAM, PARAM);
extern void PrintPad(WINDOW);
extern int PrintSetupProc(WINDOW, MESSAGE, PARAM, PARAM);
extern void SaveFile(WINDOW, int);
extern void DeleteFile(WINDOW);
extern void PadWindow(WINDOW, char *);
extern void NewFile(WINDOW);
extern void SelectFile(WINDOW);
extern void ChangeCase(WINDOW, BOOL);
char DFlatApplication[] = "infopad";

int wndpos;

static int infopadProc(WINDOW, MESSAGE, PARAM, PARAM);
static void FixTabMenu(WINDOW wnd);
static void ReformatAll(WINDOW wnd);
static void reconfigColors(WINDOW wnd);
static void reconfigTabBar(WINDOW wnd);
static void SetColorizeToggle(WINDOW wnd);

#ifndef TURBOC
void Calendar(WINDOW);
#endif

static WINDOW mainWindow;

void onExit(int bb)
{
	signal(SIGBREAK, onExit);
	signal(SIGINT, onExit);
}
#define CHARSLINE 80
#define LINESPAGE 66
void main(int argc, char *argv[])
{
	int vectors[256];
	int i;
	int *p = 0;
	for (i=8; i < 256; i++)
		vectors[i] = p[i];
    if (!init_messages())
		return;
    Argv = argv;
	if (!LoadConfig())
	{
		cfg.ScreenLines = SCREENHEIGHT;
		cfg.appdata[CFG_EDITFLAGS] = AUTO_INDENT | AUTO_FORMAT;
		cfg.appdata[CFG_COLORIZE] = COLORIZE_AUTO;
	}
	cfg.Intense = TRUE;
	ConsoleBackgroundMode(TRUE);
    mainWindow = CreateWindow(APPLICATION,
                        "InfoPad ",
                        0, 0, -1, -1,
                        &MainMenu,
                        NULL,
                        infopadProc,
                        MOVEABLE  |
                        SIZEABLE  |
                        HASBORDER |
						MINMAXBOX |
                        HASSTATUSBAR |
						HASTABBAR
                        );

    LoadHelpFile(DFlatApplication);
    SendMessage(mainWindow, SETFOCUS, TRUE, 0);
    while (argc > 1)    {
        PadWindow(mainWindow, argv[1]);
        --argc;
        argv++;
    }
	signal(SIGBREAK, SIG_IGN);
	signal(SIGINT, SIG_IGN);
    while (dispatch_message())
        ;
	for (i=8; i < 256; i++)
		p[i] = vectors[i];
	return 0;
}
/* ------- window processing module for the
                    infopad application window ----- */
static int infopadProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2)
{
	int rtn;
    switch (msg)    {
		case CREATE_WINDOW:
		    rtn = DefaultWndProc(wnd, msg, p1, p2);
			if (cfg.InsertMode)
				SetCommandToggle(&MainMenu, ID_INSERT);
			if (cfg.WordWrap)
				SetCommandToggle(&MainMenu, ID_WRAP);
			if (cfg.TabsAsSpaces)
				SetCommandToggle(&MainMenu, ID_TABSASSPACES);
			if (cfg.appdata[CFG_EDITFLAGS] & AUTO_INDENT)
				SetCommandToggle(&MainMenu, ID_AUTOINDENT);
			if (cfg.appdata[CFG_EDITFLAGS] & AUTO_FORMAT)
				SetCommandToggle(&MainMenu, ID_AUTOFORMAT);
			SetColorizeToggle(wnd);
			FixTabMenu(wnd);
			return rtn;
		case SEARCHDONE:
			ReformatAll(wnd);
			return TRUE;					
        case COMMAND:
            switch ((int)p1)    {
                case ID_NEW:
                    NewFile(wnd);
                    return TRUE;
                case ID_OPEN:
                    SelectFile(wnd);
                    return TRUE;
                case ID_SAVE:
                    SaveFile(inFocus, FALSE);
                    return TRUE;
                case ID_SAVEAS:
                    SaveFile(inFocus, TRUE);
                    return TRUE;
                case ID_DELETEFILE:
                    DeleteFile(inFocus);
                    return TRUE;
				case ID_PRINTSETUP:
					DialogBox(wnd, &PrintSetup, TRUE, PrintSetupProc);
					return TRUE;
                case ID_PRINT:
                    PrintPad(inFocus);
                    return TRUE;
				case ID_EXIT:	
					if (!YesNoBox("Exit InfoPad?"))
						return FALSE;
					break;
				case ID_WRAP:
			        cfg.WordWrap = GetCommandToggle(&MainMenu, ID_WRAP);
    	            return TRUE;
				case ID_INSERT:
			        cfg.InsertMode = GetCommandToggle(&MainMenu, ID_INSERT);
    	            return TRUE;
				case ID_TABSASSPACES:
					cfg.TabsAsSpaces = GetCommandToggle(&MainMenu, ID_TABSASSPACES);
					return TRUE;
				case ID_AUTOINDENT:
					cfg.appdata[CFG_EDITFLAGS] &= ~AUTO_INDENT;
					cfg.appdata[CFG_EDITFLAGS] |= 
						GetCommandToggle(&MainMenu, ID_AUTOINDENT) ? AUTO_INDENT : 0;
					return TRUE;
				case ID_AUTOFORMAT:
					cfg.appdata[CFG_EDITFLAGS] &= ~AUTO_FORMAT;
					cfg.appdata[CFG_EDITFLAGS] |= 
						GetCommandToggle(&MainMenu, ID_AUTOFORMAT) ? AUTO_FORMAT : 0;
					return TRUE;
				case ID_DISPLAY:
				    rtn = DefaultWndProc(wnd, msg, p1, p2);
					reconfigColors(wnd);
					reconfigTabBar(wnd);
					return rtn;
				case ID_TAB2:
					cfg.Tabs = 2;
					FixTabMenu(wnd);
                    return TRUE;
				case ID_TAB4:
					cfg.Tabs = 4;
					FixTabMenu(wnd);
                    return TRUE;
				case ID_TAB6:
					cfg.Tabs = 6;					
					FixTabMenu(wnd);
                    return TRUE;
				case ID_TAB8:
					cfg.Tabs = 8;
					FixTabMenu(wnd);
                    return TRUE;
				case ID_TOUPPER:
					ChangeCase(inFocus, TRUE);
					return TRUE;
				case ID_TOLOWER:
					ChangeCase(inFocus, FALSE);
					return TRUE;
				case ID_PUSHRIGHT:
					if (TextBlockMarked(inFocus))
					{
						SelectIndent(inFocus, TRUE);
					}
					break;
				case ID_PUSHLEFT:
					if (TextBlockMarked(inFocus))
					{
						SelectIndent(inFocus, FALSE);
					}
					break;
				case ID_COLORIZENONE:
					cfg.appdata[CFG_COLORIZE] = COLORIZE_NONE;
					SetColorizeToggle(wnd);
					ReformatAll(wnd);
					return TRUE;
				case ID_COLORIZEAUTO:
					cfg.appdata[CFG_COLORIZE] = COLORIZE_AUTO;
					SetColorizeToggle(wnd);
					ReformatAll(wnd);
					return TRUE;
				case ID_COLORIZEC:
					cfg.appdata[CFG_COLORIZE] = COLORIZE_C;
					SetColorizeToggle(wnd);
					ReformatAll(wnd);
					return TRUE;
				case ID_COLORIZEASM:
					cfg.appdata[CFG_COLORIZE] = COLORIZE_ASM;
					SetColorizeToggle(wnd);
					ReformatAll(wnd);
					return TRUE;
				case ID_CALENDAR:
#ifndef TURBOC
					Calendar(wnd);
#endif
					return TRUE;
                case ID_ABOUT:
                    MessageBox(
                        "        About InfoPad        ",
						"                            \n"
						" Copyright (C) LADSoft, 2013\n"
						"                            \n"
						" Version: " LSVERSION "\n"
						" Date: " __DATE__ "\n"
						" Time: " __TIME__ "\n"
						);
                    return TRUE;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}
static void FixTabMenu(WINDOW wnd)
{
	char *cp = GetCommandText(&MainMenu, ID_TABS);
    WINDOW wnd1 = FirstWindow(wnd);
	if (cp != NULL)	{
		cp = strchr(cp, '(');
		if (cp != NULL)	{
			*(cp+1) = cfg.Tabs + '0';
			if (GetClass(inFocus) == POPDOWNMENU)
				SendMessage(inFocus, PAINT, 0, 0);
		}
	}
    while (wnd1 != NULL)    {
		if (wnd1->text)
		{
			CollapseTabs(wnd1);
			ExpandTabs(wnd1);
		}
       	wnd1 = NextWindow(wnd1);
	}
}
static void ReformatAll(WINDOW wnd)
{
    WINDOW wnd1 = FirstWindow(wnd);
    while (wnd1 != NULL) {
		if (GetClass(wnd1) == EDITOR) 
		{
			EDITSTRUCT *eds = wnd1->extension ;
			FormatBufferFromScratch(GetText(wnd1), 0, wnd1->textlen, ColorizeType(wnd1));
			SendMessage(wnd1, PAINT, 0, 0);
		}
       	wnd1 = NextWindow(wnd1);
	}
}
void PrepFileMenu(void *w, struct Menu *mnu)
{
	WINDOW wnd = w;
	DeactivateCommand(&MainMenu, ID_SAVE);
	DeactivateCommand(&MainMenu, ID_SAVEAS);
	DeactivateCommand(&MainMenu, ID_DELETEFILE);
	DeactivateCommand(&MainMenu, ID_PRINT);
	if (wnd != NULL && GetClass(wnd) == EDITOR) {
		if (isMultiLine(wnd))	{
			ActivateCommand(&MainMenu, ID_SAVE);
			ActivateCommand(&MainMenu, ID_SAVEAS);
			ActivateCommand(&MainMenu, ID_DELETEFILE);
			ActivateCommand(&MainMenu, ID_PRINT);
		}
	}
}

void PrepSearchMenu(void *w, struct Menu *mnu)
{
	WINDOW wnd = w;
	DeactivateCommand(&MainMenu, ID_SEARCH);
	DeactivateCommand(&MainMenu, ID_REPLACE);
	DeactivateCommand(&MainMenu, ID_SEARCHNEXT);
	if (wnd != NULL && GetClass(wnd) == EDITOR) {
		if (isMultiLine(wnd))	{
			ActivateCommand(&MainMenu, ID_SEARCH);
			ActivateCommand(&MainMenu, ID_REPLACE);
			ActivateCommand(&MainMenu, ID_SEARCHNEXT);
		}
	}
}

void PrepEditMenu(void *w, struct Menu *mnu)
{
	WINDOW wnd = w;
	DeactivateCommand(&MainMenu, ID_CUT);
	DeactivateCommand(&MainMenu, ID_COPY);
	DeactivateCommand(&MainMenu, ID_CLEAR);
	DeactivateCommand(&MainMenu, ID_DELETETEXT);
	DeactivateCommand(&MainMenu, ID_PARAGRAPH);
	DeactivateCommand(&MainMenu, ID_PASTE);
	DeactivateCommand(&MainMenu, ID_UNDO);
	DeactivateCommand(&MainMenu, ID_TOUPPER);
	DeactivateCommand(&MainMenu, ID_TOLOWER);
	DeactivateCommand(&MainMenu, ID_PUSHLEFT);
	DeactivateCommand(&MainMenu, ID_PUSHRIGHT);
	if (wnd != NULL && GetClass(wnd) == EDITOR) {
		if (isMultiLine(wnd))	{
			EDITSTRUCT *eds = wnd->extension;
			if (TextBlockMarked(wnd))	{
				ActivateCommand(&MainMenu, ID_CUT);
				ActivateCommand(&MainMenu, ID_COPY);
				ActivateCommand(&MainMenu, ID_CLEAR);
				ActivateCommand(&MainMenu, ID_DELETETEXT);
				ActivateCommand(&MainMenu, ID_TOUPPER);
				ActivateCommand(&MainMenu, ID_TOLOWER);
				ActivateCommand(&MainMenu, ID_PUSHLEFT);
				ActivateCommand(&MainMenu, ID_PUSHRIGHT);
			}
			ActivateCommand(&MainMenu, ID_PARAGRAPH);
			if (!TestAttribute(wnd, READONLY) &&
						Clipboard != NULL)
				ActivateCommand(&MainMenu, ID_PASTE);
		    if (eds->undohead != eds->undotail)
				ActivateCommand(&MainMenu, ID_UNDO);
		}
	}
}
void reconfigColors(WINDOW wnd)
{
	if (!cfg.mono)
	{
	    WINDOW wnd1 = FirstWindow(wnd);
	    while (wnd1 != NULL)    {
			if (GetClass(wnd1) == EDITOR)
			{
				WindowClientColor(wnd1, BLACK, WHITE);
				SendMessage(wnd1, PAINT, 0, 0);
			}
	       	wnd1 = NextWindow(wnd1);
		}
	}
}
void reconfigTabBar(WINDOW wnd)
{
    WINDOW wnd1 = FirstWindow(wnd);
    while (wnd1 != NULL)    {
		if (GetClass(wnd1) == EDITOR)
		{
			EDITSTRUCT *eds = wnd1->extension;
			SendMessage(wnd, TAB_ADD, (PARAM)eds->filename, (PARAM)wnd1);
		}
       	wnd1 = NextWindow(wnd1);
	}

}
void SetColorizeToggle(WINDOW wnd)
{
	switch (cfg.appdata[CFG_COLORIZE])
	{
		case COLORIZE_NONE:
			SetCommandToggle(&MainMenu, ID_COLORIZENONE);
			ClearCommandToggle(&MainMenu, ID_COLORIZEAUTO);
			ClearCommandToggle(&MainMenu, ID_COLORIZEC);
			ClearCommandToggle(&MainMenu, ID_COLORIZEASM);
			break;
		case COLORIZE_AUTO:
			ClearCommandToggle(&MainMenu, ID_COLORIZENONE);
			SetCommandToggle(&MainMenu, ID_COLORIZEAUTO);
			ClearCommandToggle(&MainMenu, ID_COLORIZEC);
			ClearCommandToggle(&MainMenu, ID_COLORIZEASM);
			break;
		case COLORIZE_C:
			ClearCommandToggle(&MainMenu, ID_COLORIZENONE);
			ClearCommandToggle(&MainMenu, ID_COLORIZEAUTO);
			SetCommandToggle(&MainMenu, ID_COLORIZEC);
			ClearCommandToggle(&MainMenu, ID_COLORIZEASM);
			break;
		case COLORIZE_ASM:
			ClearCommandToggle(&MainMenu, ID_COLORIZENONE);
			ClearCommandToggle(&MainMenu, ID_COLORIZEAUTO);
			ClearCommandToggle(&MainMenu, ID_COLORIZEC);
			SetCommandToggle(&MainMenu, ID_COLORIZEASM);
			break;
	}
}
