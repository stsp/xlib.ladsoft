#include "dflat.h"
#include "dide.h"
extern int wndpos;
extern ATTRCHR *Clipboard;
extern unsigned ClipboardLength;

static char Untitled[] = "Untitled";

/* ------ display the row and column in the statusbar ------ */
static void ShowPosition(WINDOW wnd)
{
    char status[30];
    sprintf(status, "Line:%4d  Column: %2d",
        wnd->CurrLine + 1, wnd->CurrCol + 1);
    SendMessage(GetParent(wnd), ADDSTATUS, (PARAM) status, 0);
}

/* -- point to the name component of a file specification -- */
static char *NameComponent(char *FileName)
{
    char *Fname;
    if ((Fname = strrchr(FileName, '\\')) == NULL)
        if ((Fname = strrchr(FileName, ':')) == NULL)
            Fname = FileName-1;
    return Fname + 1;
}

static void SetColorizeType(WINDOW wnd, char *Buf)
{
	EDITSTRUCT *eds = wnd->extension ;
	if (!stricmp(Buf + strlen(Buf) - 2, ".c")
		|| !stricmp(Buf + strlen(Buf) - 4, ".cpp")
		|| !stricmp(Buf + strlen(Buf) - 2, ".h"))
		eds->colorizeType = COLORIZE_C;
	else
		if (!stricmp(Buf + strlen(Buf) - 4, ".asm"))
			eds->colorizeType = COLORIZE_ASM;
	if (eds->colorizeType != COLORIZE_NONE)
		FormatBufferFromScratch(GetText(wnd), 0, wnd->textlen, ColorizeType(wnd));
}
/* --- Load the notepad file into the editor text buffer --- */
static BOOL LoadFile(WINDOW wnd, WINDOW progress)
{
    char *Buf = NULL;
	int recptr = 0;
    FILE *fp;
	EDITSTRUCT *eds = wnd->extension ;
	
    if ((fp = fopen(eds->filename, "rt")) != NULL)    {
		unsigned long len,pos;
        ATTRCHR *abuf;
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		Buf = calloc((len/256 + 1) * 256, 1);
        abuf = calloc((len/256 + 1) * 256, sizeof(ATTRCHR));
        if (Buf && abuf)
        {
            int i;
            free(abuf);
    		pos = 0 ;
    		while (!feof(fp))
    		{
    			char msg[256];
    			int rv ;
    			handshake();				
    			rv = fread(Buf + pos, 1, 2048, fp);
    			if (rv > 0)
    			{
    				pos += rv;
    				
    			}
    			if (keyhit())
    				if (getkey() == '\x1b')
    				{
    					fclose(fp);
    					free(Buf);
    					return FALSE;
    				}
    			if ((pos % 8192 == 0 || feof(fp)) && len)
    			{
    				int i;
    				sprintf(msg,"Progress: ");
    				for (i=0; i < 20 * pos/ len; i++)
    					strcat(msg,".");
    				strcat(msg,"\nPress ESC to abort");
    				SendMessage(progress, SETTEXT, (PARAM)msg, 0);
    				SendMessage(progress, PAINT, 0, 0);
    			}
    		}
            fclose(fp);
    		handshake();
            for (i=0; i < len; i++)
                if (Buf[i] == 0)
                    Buf[i] = ' ';
                else if ((Buf[i] >126) || Buf[i] < 32 && !isspace(Buf[i]))
                    Buf[i] = '.';
	        SendMessage(wnd, SETTEXT, (PARAM) Buf, 0);
		    free(Buf);
			SetColorizeType(wnd, eds->filename);
        }
        else
        {
            ErrorMessage("File won't fit in memory");
        }
    }
	return TRUE;
}

/* ---------- save a file to disk ------------ */
void SaveFile(WINDOW wnd, int Saveas)
{
    FILE *fp;
	EDITSTRUCT *eds = wnd->extension ;
    char FileName[260];
top:
    if (eds->filename == NULL || Saveas)    {
        if (SaveAsDialogBox("*.*", NULL, FileName))    {
			Saveas = TRUE;
        }
        else
            return;
    }
	else
		strcpy(FileName, eds->filename);
	{
		WINDOW mwnd = MomentaryMessage("Progress:                     \n\n");
        if ((fp = fopen(FileName, "wt")) != NULL)    {
			char *s = DFmalloc(wnd->textlen + 1);
			int i, size;
			SendMessage(wnd, GETTEXT, (PARAM)s, wnd->textlen);
			size = strlen(s);
			for (i=0; i < size; i+= 2048)
			{
				int len = 2048 <=  size - i ? 2048 : size- i ;
				fwrite(s + i, len, 1, fp);
				if (keyhit())
					if (getkey() == '\x1b')
					{
						fclose(fp);
						free(s);
						unlink(FileName);
				        SendMessage(mwnd, CLOSE_WINDOW, 0, 0);
						return FALSE;
					}
				if (i % 8192 == 0 || i != 2048)
				{
					int j;
					char msg[256];
					sprintf(msg,"Progress: ");
					for (j=0; j < 20 * i/ size; j++)
						strcat(msg,".");
					strcat(msg,"\nPress ESC to abort");
					SendMessage(mwnd, SETTEXT, (PARAM)msg, 0);
					SendMessage(mwnd, PAINT, 0, 0);
				}
			}
            fclose(fp);
			free(s);
			ChangedText(wnd, FALSE);
        }
		else
		{
            MessageBox("Save Error", "Could not save file");
			Saveas = TRUE;
			goto top;
		}
        SendMessage(mwnd, CLOSE_WINDOW, 0, 0);
    }
	if (Saveas)
	{
        if (eds->filename != NULL)
            free(eds->filename);
        eds->filename = DFmalloc(strlen(FileName)+1);
        strcpy(eds->filename, FileName);
		SetColorizeType(wnd, eds->filename);
        AddTitle(wnd, NameComponent(eds->filename));
        SendMessage(wnd, BORDER, 0, 0);
		SendMessage(GetParent(wnd), TAB_REMOVE, 0, (PARAM)wnd);
		SendMessage(GetParent(wnd), TAB_ADD, (PARAM)eds->filename, (PARAM)wnd);
	}
}
static int KeyboardMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
	int rtn = TRUE;
	EDITSTRUCT *eds = (EDITSTRUCT *)wnd->extension;
	UNDO *u = NULL;
	if (!eds->keyboardNesting++)
		CancelParenMatch(wnd);
	switch (p1)
	{
		case SHIFT_HT:
			if (TextBlockMarked(wnd) && !eds->undoing)
				SelectIndent(wnd, FALSE);
			else
			    rtn = DefaultWndProc(wnd, KEYBOARD, p1, p2);			
			break;
		case F2:
			SaveFile(wnd, FALSE);
			break;
		case RUBOUT:
			if ((int)p2 & CTRLKEY)
			{
			    rtn = DefaultWndProc(wnd, KEYBOARD, p1, p2);
				break;
			}
			if (wnd->CurrCol == 0 && wnd->CurrLine == 0)
			{
				beep();
				break;
			}
			if (TextBlockMarked(wnd))
				u = undo_deletesel(wnd);
			else
			{
				ATTRCHR *s = CurrChar;
				if (s != GetText(wnd))
					u = undo_deletechar(wnd, s[-1].ch, UNDO_BACKSPACE);
				if (wnd->CurrCol == 0)
				{
					DefaultWndProc(wnd, KEYBOARD, BS, 0);
					p1 = DEL;
					goto join;
				}
			}
			/* fallthrough */
		case DEL:
			if (p1 == DEL)
				if (TextBlockMarked(wnd))
					u = undo_deletesel(wnd);
				else
				{
					ATTRCHR *s = CurrChar;
					u = undo_deletechar(wnd, s->ch, UNDO_DELETE);
				}
join:
		    rtn = DefaultWndProc(wnd, KEYBOARD, p1, p2);
			if (u)
			{
				u->postselstartline = u->postselendline = wnd->CurrLine;
				u->postselstartcolumn = u->postselendcolumn = wnd->CurrCol;
			}
			if (ColorizeType(wnd) != COLORIZE_NONE)
			{
				FormatLine(wnd, GetText(wnd), ColorizeType(wnd));
				SendMessage(wnd, PAINT, 0, 0);
			}
			break;
		case CTRL_C:
		    rtn = DefaultWndProc(wnd, KEYBOARD, p1, p2);
			break;
		case CTRL_X:
			u = undo_deletesel(wnd);
		    rtn = DefaultWndProc(wnd, KEYBOARD, p1, p2);
//			if (u)
//			{
//				u->postselstartline = u->postselendline = wnd->CurrLine;
//				u->postselstartcolumn = u->postselendcolumn = wnd->CurrCol;
//			}
			break;
		case CTRL_V:
			if (ClipboardLength)
			{
				if (TextBlockMarked(wnd))
					SendMessage(wnd, KEYBOARD, DEL, 0);
				u = undo_insertsel(wnd, Clipboard);
			    rtn = DefaultWndProc(wnd, KEYBOARD, p1, p2);
				if (u)
				{
					int line = wnd->CurrLine;
					int column = wnd->CurrCol;
					int len= ClipboardLength;
					ATTRCHR *s = Clipboard;
					while (len--)
					{
						if (s++->ch == '\n')
						{
							column = 0;
							line++;
						}
						else
						{
							column++;
						}
					}
					wnd->BlkBegLine = u->preselstartline;
					wnd->BlkBegCol = u->preselstartcolumn;
					wnd->BlkEndLine = line;
					wnd->BlkEndCol = column;
					u->postselstartline = wnd->BlkBegLine;
					u->postselendline = wnd->BlkEndLine;
					u->postselstartcolumn = wnd->BlkBegCol;
					u->postselendcolumn = wnd->BlkEndCol;
				}
			}
			else
				rtn = DefaultWndProc(wnd, KEYBOARD, p1, p2);
			break;
		case CTRL_Z:
			doundo(wnd);
			break;
		case '\t':
			if (TextBlockMarked(wnd) && !eds->undoing)
			{
				SelectIndent(wnd, TRUE);
				break;
			}
			/* fallthrough */
		default:
			if (p1 < 256 && isprint(p1) || p1 == '\r' || p1 == '\n' || p1 == '\t')
			{
				UNDO *u;
				if (TextBlockMarked(wnd))
					undo_deletesel(wnd);
				if (wnd->InsertMode)
				{
					u = undo_insertchar(wnd, p1);
				}
				else
				{
					u = undo_modifychar(wnd, p1);
				}
			    rtn = DefaultWndProc(wnd, KEYBOARD, p1, p2);
				
				if (u)
				{
					u->postselstartline = u->postselendline = wnd->CurrLine;
					u->postselstartcolumn = u->postselendcolumn = wnd->CurrCol;
					
				}
				if (p1 == '\r')
					insertcrtabs(wnd);
				if (p1 == '{')
					InsertBeginTabs(wnd);
				if (p1 == ' ' || p1 == '}')
					InsertEndTabs(wnd, p1 == '}');
				if (p1 == '%')
					DeletePercent(wnd);
				if (p1 == '#')
					DeletePound(wnd);
				if (ColorizeType(wnd) != COLORIZE_NONE)
				{
					if (p1 == '\r')
					{
						int start, end;
						start = TextLineNumber(wnd, CurrChar) - 1;
						if (start < 0)
							start = 0;
						end = TextLineNumber(wnd, CurrChar) + 1;
						if ( end > wnd->wlines)
						{
							end = GetText(wnd) + wnd->textlen;
						}
						FormatBufferFromScratch(GetText(wnd), start, end, ColorizeType(wnd));
					}
					else
						FormatLine(wnd, GetText(wnd), ColorizeType(wnd));
					SendMessage(wnd, PAINT, 0, 0);
				}
			}
			else
			{
			    rtn = DefaultWndProc(wnd, KEYBOARD, p1, p2);
			}
			break;
	}
	if (!--eds->keyboardNesting)
		FindParenMatch(wnd);
	return rtn;
}
/* ----- window processing module for the editboxes ----- */
static int EditorProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2)
{
    int rtn;
	EDITSTRUCT *eds ;
    switch (msg)    {
		case CREATE_WINDOW:
			wnd->extension = DFcalloc(sizeof EDITSTRUCT, 1);
			if (!cfg.mono)
				WindowClientColor(wnd, BLACK, WHITE);
			break;
        case SETFOCUS:
			if ((int)p1)	{
				wnd->InsertMode = GetCommandToggle(&MainMenu, ID_INSERT);
				wnd->WordWrapMode = GetCommandToggle(&MainMenu, ID_WRAP);
			}
            rtn = DefaultWndProc(wnd, msg, p1, p2);
            if ((int)p1 == FALSE)
                SendMessage(GetParent(wnd), ADDSTATUS, 0, 0);
            else 
			{
				SendMessage(GetParent(wnd), TAB_SELECT, 0 , (PARAM)wnd);
                ShowPosition(wnd);
			}
            return rtn;
        case KEYBOARD_CURSOR:
            rtn = DefaultWndProc(wnd, msg, p1, p2);
            ShowPosition(wnd);
            return rtn;
        case COMMAND:
			switch ((int) p1)	{
				case ID_HELP:
	                DisplayHelp(wnd, "MEMOPADDOC");
    	            return TRUE;
				case ID_WRAP:
					SendMessage(GetParent(wnd), COMMAND, ID_WRAP, 0);
					wnd->WordWrapMode = cfg.WordWrap;
    	            return TRUE;
				case ID_INSERT:
					SendMessage(GetParent(wnd), COMMAND, ID_INSERT, 0);
					wnd->InsertMode = cfg.InsertMode;
					SendMessage(NULL, SHOW_CURSOR, wnd->InsertMode, 0);
    	            return TRUE;
				case ID_UNDO:
					doundo(wnd);
					if (ColorizeType(wnd) != COLORIZE_NONE)
					{
						FormatBufferFromScratch(GetText(wnd), 0, wnd->textlen, ColorizeType(wnd));
						SendMessage(wnd, PAINT, 0, 0);
					}
					return TRUE;
				case ID_CUT:
				case ID_PASTE:
					eds = wnd->extension ;
					rtn = DefaultWndProc(wnd, msg, p1, p2);
					if (ColorizeType(wnd) != COLORIZE_NONE)
					{
						FormatBufferFromScratch(GetText(wnd), 0, wnd->textlen, ColorizeType(wnd));
						SendMessage(wnd, PAINT, 0, 0);
					}
					return rtn;
				default:
					break;
            }
            break;
        case CLOSE_WINDOW:
            if (wnd->TextChanged)    {
                char *cp = DFmalloc(25+strlen(GetTitle(wnd)));
                SendMessage(wnd, SETFOCUS, TRUE, 0);
                strcpy(cp, GetTitle(wnd));
                strcat(cp, "\nText changed. Save it?");
                if (YesNoBox(cp))
                    SendMessage(GetParent(wnd),
                        COMMAND, ID_SAVE, 0);
                free(cp);
            }
            wndpos = 0;
			SendMessage(GetParent(wnd), TAB_REMOVE, 0, (PARAM)wnd);
			eds = wnd->extension ;
            if (eds->filename != NULL)    {
                free(eds->filename);
                eds->filename = NULL;
            }
            break;
		case KEYBOARD:
			return KeyboardMsg(wnd, p1, p2);
        case LEFT_BUTTON:
			CancelParenMatch(wnd);
		    rtn = DefaultWndProc(wnd, msg, p1, p2);
			FindParenMatch(wnd);
			return rtn;
        default:
            break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}
void insertchar(WINDOW wnd, int ch)
{
	if (ch == '\n')
		ch = '\r';
	SendMessage(wnd, KEYBOARD, ch, 0);
}
/* -------- delete a file ------------ */
void DeleteFile(WINDOW wnd)
{
	EDITSTRUCT *eds = wnd->extension ;
    if (eds->filename != NULL)    {
        if (strcmp(eds->filename, Untitled))    {
            char *fn = NameComponent(eds->filename);
            if (fn != NULL)    {
                char msg[30];
                sprintf(msg, "Delete %s?", fn);
                if (YesNoBox(msg))    {
                    unlink(eds->filename);
                    SendMessage(wnd, CLOSE_WINDOW, 0, 0);
                }
            }
        }
    }
}

/* --- open a document window and load a file --- */
static void OpenPadWindow(WINDOW wnd, char *FileName)
{
    static WINDOW wnd1 = NULL;
	WINDOW wwnd;
    struct stat sb;
    char *Fname = FileName;
    char *ermsg;
	BOOL rv = FALSE;
    if (strcmp(FileName, Untitled))    {
        if (stat(FileName, &sb))    {
            ermsg = DFmalloc(strlen(FileName)+20);
            strcpy(ermsg, "No such file as\n");
            strcat(ermsg, FileName);
            ErrorMessage(ermsg);
            free(ermsg);
            return;
        }
        Fname = NameComponent(FileName);
    }
//	wwnd = WatchIcon();
	wwnd = MomentaryMessage("Progress:                     \n\n");
    wndpos += 2;
    if (wndpos == 10)
        wndpos = 2;
    wnd1 = CreateWindow(EDITOR,
                Fname,
                (wndpos-1)*2, wndpos, 20, 70,
                NULL, wnd, EditorProc,
                SHADOW     |
                MINMAXBOX  |
                CONTROLBOX |
                VSCROLLBAR |
                HSCROLLBAR |
                MOVEABLE   |
                HASBORDER  |
                SIZEABLE   |
                MULTILINE
    );
    if (strcmp(FileName, Untitled))    {
		EDITSTRUCT *eds = wnd1->extension;
        eds->filename= DFmalloc(strlen(FileName)+1);
        strcpy(eds->filename, FileName);
        rv = LoadFile(wnd1, wwnd);
    }
	SendMessage(wwnd, CLOSE_WINDOW, 0, 0);
    SendMessage(wnd1, SETFOCUS, TRUE, 0);
	SendMessage(GetParent(wnd1), TAB_ADD, (PARAM)FileName, (PARAM)wnd1);
	if (strcmp(FileName, Untitled) && !rv)
		SendMessage(wnd1, CLOSE_WINDOW, 0, 0);
}
/* --- The New command. Open an empty editor window --- */
void NewFile(WINDOW wnd)
{
    OpenPadWindow(wnd, Untitled);
}
/* --- The Open... command. Select a file  --- */
void SelectFile(WINDOW wnd)
{
    char FileName[260];
    if (OpenFileDialogBox("*.*", FileName))    {
        /* --- see if the document is already in a window --- */
        WINDOW wnd1 = FirstWindow(wnd);
        while (wnd1 != NULL)    {
			EDITSTRUCT *eds = wnd1->extension;
            if (eds && stricmp(FileName, eds->filename) == 0)    {
                SendMessage(wnd1, SETFOCUS, TRUE, 0);
                SendMessage(wnd1, RESTORE, 0, 0);
                return;
            }
            wnd1 = NextWindow(wnd1);
        }
        OpenPadWindow(wnd, FileName);
    }
}
/* ------ open text files and put them into editboxes ----- */
void PadWindow(WINDOW wnd, char *FileName)
{
    int ax, criterr = 1;
    struct ffblk ff;
    char path[MAXPATH+1];
    char *cp;

    CreatePath(path, FileName, FALSE, FALSE);
    cp = path+strlen(path);
    CreatePath(path, FileName, TRUE, FALSE);
    while (criterr == 1)    {
        ax = findfirst(path, &ff, 0);
        criterr = TestCriticalError();
    }
    while (ax == 0 && !criterr)    {
        strcpy(cp, ff.ff_name);
        OpenPadWindow(wnd, path);
        ax = findnext(&ff);
    }
}
void ChangeCase(WINDOW wnd, BOOL upper)
{
	if (TextBlockMarked(wnd))
	{
		UNDO *u = undo_casechange(wnd);
		if (u)
		{
			int start = 0;
			ATTRCHR *s = CharPos(wnd, wnd->BlkBegLine, wnd->BlkBegCol);
			while (start < u->len)
			{
				if (s->ch != pTab)
				{
					s->ch = upper ? toupper(s->ch) : tolower(s->ch);
					start++;
				}
				s++;
			}
			if (u)
			{
				wnd->BlkBegLine = wnd->CurrLine = u->preselstartline;
				wnd->BlkBegCol = wnd->CurrCol = u->preselstartcolumn;
				wnd->BlkEndLine = u->preselendline;
				wnd->BlkEndCol = u->preselendcolumn;
				u->postselstartline = wnd->BlkBegLine;
				u->postselstartcolumn = wnd->BlkBegCol;
				u->postselendline = wnd->BlkEndLine;
				u->postselendcolumn = wnd->BlkEndCol;
			}
			ChangedText(wnd, TRUE);
			SendMessage(wnd, PAINT, 0, 0);
		}
	}
}