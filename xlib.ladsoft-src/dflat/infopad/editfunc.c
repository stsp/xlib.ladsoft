#include "dflat.h"
#include "dide.h"


#define C_BACKGROUND WHITE
#define C_TEXT ((C_BACKGROUND << 4) + BLACK)
#define C_HIGHLIGHT ((YELLOW << 4) + BLACK)
#define C_KEYWORD ((C_BACKGROUND << 4) + LIGHTBLUE)
#define C_COMMENT ((C_BACKGROUND << 4) + GREEN)
#define C_NUMBER ((C_BACKGROUND << 4) + LIGHTRED)
#define C_STRING ((C_BACKGROUND << 4) + LIGHTRED)
#define C_ESCAPE ((C_BACKGROUND << 4) + LIGHTCYAN)

// and the next is highlight data filled in by the find
extern char highlightText[256] ;
extern BOOL highlightCaseSensitive;
extern BOOL highlightWholeWord;

// The KEYLIST structure holds information about text which should be
// colorized.  It gives the name of a keyword (e.g. 'int') and the color
// to display it in.

typedef struct
{
    char *text;
	unsigned char attrib;
//    COLORREF *color;
} KEYLIST;

// The C_keywordList is a list of all keywords, with colorization info
static KEYLIST C_keywordList[] = 
{
    #include "..\..\ccide\c_kw.h"
};
static KEYLIST ASM_keywordList[] = 
{
    #include "..\..\ccide\asm_kw.h"
};
int ColorizeType(WINDOW wnd)
{

	EDITSTRUCT *eds = wnd->extension;
	switch( cfg.appdata[CFG_COLORIZE])
	{
		case COLORIZE_NONE:
			return COLORIZE_NONE;
		case COLORIZE_AUTO:
			return eds->colorizeType;
		case COLORIZE_C:
			return COLORIZE_C;
		case COLORIZE_ASM:
			return COLORIZE_ASM;
	}
}
/**********************************************************************
 * Colorize marks a range of text with a specific color and attributes
 **********************************************************************/
static void Colorize(ATTRCHR *buf, int start, int len, int color)
{
	int i;
	buf += start;
    for (i = 0; i < len; i++, buf++)
    {
        buf->attrib = color;
    }
}

/**********************************************************************
 * keysym returns true if it is a symbol that can be used in a keyword
 **********************************************************************/
static int keysym(unsigned char x)
{
    return isalnum(x) || x == '_';
}

/**********************************************************************
 * strpstr  finds a text string within a string organized as internal
 * characters.  Returns 0 if it couldn't find the string
 **********************************************************************/
static ATTRCHR *strpstr(ATTRCHR *t, unsigned char *text, int len)
{
    while (t->ch && len)
    {
        if (t->ch == text[0])
        {
            char *t1 = text;
            ATTRCHR *it1 = t;
            while (*t1 && it1->ch ==  *t1)
            {
                t1++;
                it1++;
            }
            if (! *t1)
                return t;
        }
        t++;
        len--;
    }
    return 0;

}

/**********************************************************************
 * strplen  finds the length of an internal_char array
 **********************************************************************/

static int strplen(ATTRCHR *t)
{
    int rv = 0;
    while (t->ch)
        rv++, t++;
    return rv;
}

/**********************************************************************
 * backalpha goes backwards to find out if the current (numeric)
 * character is part of an identifier, or if it is a standalone number.
 * returns TRUE if part of an identifier.  Used in colorizing numbers
 **********************************************************************/

static int backalpha(ATTRCHR *buf, int i)
{
    while (i >= 0)
    {
        if (isalpha(buf[i].ch))
            return TRUE;
        if (buf[i].ch == '_')
            return TRUE;
        if (!isdigit(buf[i].ch))
            return FALSE;
        i--;
    }
    return FALSE;

}

//-------------------------------------------------------------------------

static int pcmp(ATTRCHR *s, char *t, int preproc, int *retlen, int
    caseinsensitive, int bykey)
{
	char *n = t;
    *retlen = 0;
    while (*t && s->ch)
    {
        int val = s->ch;
		int ch = *t;
		if (whitespace(val))
			val = ' ';
        if (caseinsensitive)
		{
            val = tolower(val);
			ch = tolower(ch);
		}
        if (val <  ch)
            return  - 1;
        else if (val >  ch)
            return 1;
        else
        {
            if (ch == preproc)
            {
                while (whitespace(s[1].ch) && s[1].ch != '\n')
                    s++, (*retlen)++;
            }
            s++, t++, (*retlen)++;
        }
    }
    if (*t)
        return 1;
    return bykey && keysym(s->ch) ;
}

/**********************************************************************
 * See if a keyword matches the current text location
 **********************************************************************/
static KEYLIST *matchkeyword(KEYLIST *table, int tabsize, int preproc, ATTRCHR
    *t, int *retlen, int insensitive)
{
    int top = tabsize;
    int bottom =  - 1;
    int v;
    while (top - bottom > 1)
    {
        int mid = (top + bottom) / 2;
        v = pcmp(t, table[mid].text, preproc, retlen, insensitive, TRUE);
        if (v < 0)
        {
            top = mid;
        }
        else
        {
            bottom = mid;
        }
    }
    if (bottom ==  - 1)
        return 0;
    v = pcmp(t, table[bottom].text, preproc, retlen, insensitive, TRUE);
    if (v)
        return 0;
    return  &table[bottom];
}

/**********************************************************************
 * SearchKeywords searches a range of ATTRCHRs for keywords,
 * numbers, and strings, and colorizes them
 **********************************************************************/

static void SearchKeywords(ATTRCHR *buf, int chars, int start, int type)
{
    int i;
    KEYLIST *sr = C_keywordList;
    int size = sizeof(C_keywordList) / sizeof(KEYLIST);
    int preproc = '#';
    int hexflg = FALSE;
    int xchars = chars;
    ATTRCHR *t = buf + start;
    if (type == COLORIZE_ASM)
    {
        sr = ASM_keywordList;
        size = sizeof(ASM_keywordList) / sizeof(KEYLIST);
        preproc = '%';
    }
    while (t->ch && xchars > 0)
    {
        while (t->ch && t->attrib == C_COMMENT && xchars > 0)
            t++, xchars--;
        if (xchars > 0 && (t == buf || !keysym(t[ - 1].ch) && (keysym(t->ch) ||
            t->ch == preproc)))
        {
			int len;
            KEYLIST *p = matchkeyword(sr, size, preproc, t, &len, type ==
	            COLORIZE_ASM);
			if (p)
    	    {
        	    Colorize(buf, t - buf, len,  p->attrib);
            	t += len;
            	xchars -= len;
        	}
			else
			{
				if (t->attrib == C_HIGHLIGHT)
					t->attrib = C_TEXT;
            	t++, xchars--;
			}
        }
        else
		{
			if (t->attrib == C_HIGHLIGHT)
				t->attrib = C_TEXT;
            t++, xchars--;
		}
    }

    for (i = 0; i < chars; i++)
        if (buf[start + i].attrib != C_COMMENT)
		{
	        int len;
	        if (highlightText[0] && 
				!pcmp(buf + start + i, highlightText, preproc, &len, !highlightCaseSensitive, FALSE)
				&& (!highlightWholeWord || (i == buf || !isalnum(buf[i-1].ch)) && !isalnum(buf[i+len].ch)))
			{
	            Colorize(buf, start + i, len, C_HIGHLIGHT);
	            i += len - 1;
			}
			else if (type == COLORIZE_ASM && buf[start + i].ch == '$')
			{
				len =1;
				while (isxdigit(buf[start + i + len].ch))
					len++;
	            Colorize(buf, start + i, len, C_NUMBER);
	            i += len - 1;
			}
		    else if (isdigit(buf[start + i].ch))
		    {
		        if (!backalpha(buf, start + i - 1))
		        {
		            int j = i;
		            char ch = buf[start + i++].ch;
		            if (type == COLORIZE_C)
		            {
		                if (isdigit(ch) || ch == '.')
		                {
		                    char oc = ch;
		                    ch = buf[start + i].ch;
		                    hexflg = oc == '0' && (ch == 'x' || ch == 'X');
		                    if (hexflg)
		                        ch = buf [start + ++i].ch;
		                    while (ch == '.' || isdigit(ch) 
		                        || hexflg && (isxdigit(ch) || ch =='p' || ch == 'P') ||
		                          !hexflg && (ch == 'e' || ch== 'E'))
		                    {
		                            i++;
		                            if (!hexflg && ch >= 'A' || hexflg && (ch =='p' || ch == 'P'))
		                            {
		                                ch = buf[start+i].ch;
		                                if (ch == '-' || ch == '+')
		                                    i++;
		                            }
		                            ch = buf[start+i].ch ;
		                    }
		                    while (ch== 'L' || ch == 'l'
		                        || ch == 'U' || ch == 'u'
		                        || ch == 'F' || ch == 'f')
		                        ch = buf[start + ++i].ch;
		                }
		            }
		            else
		            {
		                while (isxdigit(buf[start + i].ch))
		                    i++;
		                if (buf[start + i].ch != 'H' && buf[start + i].ch != 'h')
		                {
		                    i = j;
		                    while (isdigit(buf[start + i].ch))
		                        i++;
		                }
		                else
		                    i++;
		            }
		            hexflg = FALSE;
		            Colorize(buf, start + j, i - j, C_NUMBER);
		            i--;
		        }
		    }
		    else if ((buf[start + i].ch == '"' 
		        || buf[start + i].ch == '\'') && (buf[start + i - 1].ch != '\\' || buf[start + i - 2].ch == '\\'))
		    {
		        int ch = buf[start + i].ch;
		        int j = i++;
		        while (buf[start + i].ch && (buf[start + i].ch != ch && buf[start +
		            i].ch != '\n' || buf[start + i - 1].ch == '\\' && buf[start + i -
		            2].ch != '\\') && i < chars)
		            i++;
		        Colorize(buf, start + j + 1, i - j - 1, C_STRING);
		    }
		}
}
/**********************************************************************
 * FormatBuffer colorizes comments over a range of text, 
 * then calls SearchKeywords to colorize keywords
 **********************************************************************/
static int instring(ATTRCHR *buf, ATTRCHR *t1)
{
    ATTRCHR *t2 = t1;
    int quotechar = 0;
    while (t2 != buf && t2[ - 1].ch != '\n')
        t2--;
    while (t2 != t1)
    {
        if (quotechar)
        {
            if (t2->ch == quotechar && t2[ - 1].ch != '\\')
                quotechar = 0;
        }
        else
        {
            if (t2->ch == '\'' || t2->ch == '"')
                quotechar = t2->ch;
        }
        t2++;
    }
    return !!quotechar;
}

//-------------------------------------------------------------------------
void FormatBuffer(ATTRCHR *buf, int start, int end, int type)
{
    if (type == COLORIZE_C)
    {
        ATTRCHR *t = buf + start;
        ATTRCHR *t1;
        while (TRUE)
        {
            t1 = strpstr(t, "/*", end - (t - buf));

            if (t1)
            {
                if ((t1 == buf || t1[ - 1].ch != '/') && !instring(buf, t1))
                {

                    t = strpstr(t1, "*/",  - 1);
                    if (!t)
                        t = t1 + strplen(t1);
                    else
                    {
                        t += 2;
                    }
                    Colorize(buf, t1 - buf, t - t1, C_COMMENT);
                }
                else
                    t = t1 + 1;
            }
            else
                break;
        }
        t = buf + start;
        t1 = strpstr(t, "//", end - (t - buf));
        while (t1)
        {
            if (!instring(buf, t1) && t1->attrib != C_COMMENT)
            {

                t = strpstr(t1, "\n",  - 1);
                while (1)
                {
                    if (!t)
                    {
                        t = t1 + strplen(t1);
                        break;
                    }
                    else
                    {
                        ATTRCHR *t2 = t;
                        while (t2 > buf && whitespace(t2[ - 1].ch))
                            t2--;
                        if (t2[ - 1].ch != '\\')
                            break;
                        t = strpstr(t + 1, "\n",  - 1);
                    }
                }
                Colorize(buf, t1 - buf, t - t1 + 1, C_COMMENT);
            }
            else
                t = t1 + 1;
            t1 = strpstr(t, "//", end - (t - buf));
        }
    }
    else if (type == COLORIZE_ASM)
    {
        ATTRCHR *t = buf + start;
        int type;
        ATTRCHR *t1;
        t1 = strpstr(t, ";", end - (t - buf));
        while (t1)
        {
            t = strpstr(t1, "\n",  - 1);
            if (!t)
            {
                t = t1 + strplen(t1);
            }
            Colorize(buf, t1 - buf, t - t1 + 1, C_COMMENT);
            t1 = strpstr(t, ";", end - (t - buf));
        }
    }
    SearchKeywords(buf, end - start, start, type);
}

//-------------------------------------------------------------------------

void FormatBufferFromScratch(ATTRCHR *buf, int start, int end, int
    type)
{
    int xend, xstart;
    xend = end;
    if (start < 0)
        start = 0;
    xstart = start;
    while (xstart && (buf[xstart - 1].ch != '\n' || buf[xstart - 1].attrib ==
        C_COMMENT))
        xstart--;
    while (buf[xend].ch && (buf[xend].ch != '\n' || buf[xend].attrib ==
                C_COMMENT))
            xend++;

    Colorize(buf, xstart, xend - xstart, C_TEXT);
	if (type != COLORIZE_NONE)
	    FormatBuffer(buf, xstart, xend, type);
}

/**********************************************************************
 * FormatLine is an optimized colorizer that just colorizes the current
 * line
 **********************************************************************/

void FormatLine(WINDOW wnd, ATTRCHR *buf, int type)
{
    FormatBufferFromScratch(buf, CurrChar - buf, CurrChar - buf, type);

}

/**********************************************************************
 * getundo creates an UNDO structure based on the user's last operation
 **********************************************************************/
UNDO *getundo(WINDOW wnd, int type)
{
	EDITSTRUCT *eds = wnd->extension;
    int x;
    UNDO *u;
    if (eds->undoing)
        return 0;
    if (type != UNDO_DELETESELECTION && type != UNDO_INSERTSELECTION && type !=
        UNDO_CASECHANGE && type != UNDO_AUTOBEGIN && type != UNDO_AUTOEND &&
        type != UNDO_AUTOCHAINBEGIN && eds->undohead != eds->undotail)
    {
        x = eds->undohead - 1;
        if (x < 0)
            x += UNDO_MAX;
        if (eds->undolist[x].type == type)
//        if (type != UNDO_BACKSPACE)
//        {
            if (wnd->CurrLine == eds->undolist[x].postselstartline &&
				wnd->CurrCol == eds->undolist[x].postselstartcolumn)
                return  &eds->undolist[x];
//        }
//        else
//        {
//            if (CurrChar + 1 == CharPos(wnd, eds->undolist[x].postselstartline, eds->undolist[x].postselstartcolumn))
//                return  &eds->undolist[x];
//        }
    }
    u = &eds->undolist[eds->undohead];
    if (++eds->undohead >= UNDO_MAX)
        eds->undohead = 0;
    if (eds->undohead == eds->undotail)
        if (++eds->undotail >= UNDO_MAX)
            eds->undotail = 0;
    u->len = 0;
	if (TextBlockMarked(wnd))
	{
	    u->preselstartline = wnd->BlkBegLine;
	    u->preselstartcolumn = wnd->BlkBegCol;
	    u->preselendline = wnd->BlkEndLine;
	    u->preselendcolumn = wnd->BlkEndCol;
	}
	else
	{
	    u->preselstartline = u->preselendline  =wnd->CurrLine;
	    u->preselstartcolumn = u->preselendcolumn =  wnd->CurrCol;
	}
    u->modified = wnd->TextChanged;
    ChangedText(wnd, TRUE);
    u->type = type;
    return u;
}

//-------------------------------------------------------------------------

int insertautoundo(WINDOW wnd, int type)
{
    return getundo(wnd, type);
}

/**********************************************************************
 * undo_deletesel gets the undo structure for a CUT operation
 **********************************************************************/
UNDO *undo_deletesel(WINDOW wnd)
{
    UNDO *u;
    ATTRCHR *start, *end;
    int i = 0;
	if (!TextBlockMarked(wnd))
		return 0;
    u = getundo(wnd, UNDO_DELETESELECTION);

    if (!u)
        return u;
	start = CharPos(wnd, wnd->BlkBegLine, wnd->BlkBegCol);
	end = CharPos(wnd, wnd->BlkEndLine, wnd->BlkEndCol);
    if (end - start > u->max)
    {
        char *temp = realloc(u->data, end - start);
        if (!temp)
            return 0;
        u->data = temp;
        u->max = end - start;
    }
    while (start < end)
    {
		if (start->ch == sTab)
			u->data[i++] = '\t',start++;
		else if (start->ch != pTab)
	        u->data[i++] = start++->ch;
		else
			start++;
    }
    u->len = i;
	if (TextBlockMarked(wnd))
	{
	    u->postselstartline = wnd->BlkBegLine;
	    u->postselstartcolumn = wnd->BlkBegCol;
	    u->postselendline = wnd->BlkEndLine;
	    u->postselendcolumn = wnd->BlkEndCol;
	}
	else
	{
		u->postselstartline = u->postselendline = wnd->CurrLine;
		u->postselstartcolumn = u->postselendcolumn = wnd->CurrCol;
	}
    return u;
}

//-------------------------------------------------------------------------

UNDO *undo_casechange(WINDOW wnd)
{
    UNDO *x = undo_deletesel(wnd);
	if (x)
	    x->type = UNDO_CASECHANGE;
    return x;
}

/**********************************************************************
 * undo_insertsel gets the undo structure for an operation which pasts
 **********************************************************************/
UNDO *undo_insertsel(WINDOW wnd, char *s)
{
    UNDO *u = getundo(wnd, UNDO_INSERTSELECTION);
    if (!u)
        return u;
    u->len = strlen(s);
	if (TextBlockMarked(wnd))
	{
	    u->postselstartline = wnd->BlkBegLine;
	    u->postselstartline = wnd->BlkBegCol;
	    u->postselendline = wnd->BlkEndLine;
	    u->postselendline = wnd->BlkEndCol;
	}
	else
	{
		u->postselstartline = u->postselendline = wnd->CurrLine;
		u->postselstartcolumn = u->postselendcolumn = wnd->CurrCol;
	}
    return u;
}

/**********************************************************************
 * undo_deletechar gets the undo structure for a character deletion
 **********************************************************************/
UNDO *undo_deletechar(WINDOW wnd, int ch, int type)
{
    UNDO *u = getundo(wnd, type);
	if (ch == sTab || ch == pTab)
		ch = '\t';
    if (!u)
        return u;
    if (u->max <= u->len)
    {
        char *temp = realloc(u->data, u->max + 64);
        if (!temp)
            return 0;
        u->data = temp;
        u->max += 64;
    }
    memmove(u->data + 1, u->data, u->len++);
    u->data[0] = ch;
    return u;
}

/**********************************************************************
 * undo_deletechar gets the undo structure for typing over a character
 **********************************************************************/
UNDO *undo_modifychar(WINDOW wnd, int ch)
{
    UNDO *u = getundo(wnd, UNDO_MODIFY);
	ATTRCHR *s;
    if (!u)
        return u;
    if (u->max <= u->len)
    {
        char *temp = realloc(u->data, u->max + 64);
        if (!temp)
            return 0;
        u->data = temp;
        u->max += 64;
    }
    memmove(u->data + 1, u->data, u->len++);
	s = CurrChar;
    u->data[0] = s->ch;
	if (ch == '\t')
	{
		int col = wnd->CurrCol;
	   	while (((wnd->CurrCol + 1) % cfg.Tabs) != 0)
		{
		    if (u->max <= u->len)
		    {
		        char *temp = realloc(u->data, u->max + 64);
		        if (!temp)
		            return 0;
		        u->data = temp;
		        u->max += 64;
		    }
			++wnd->CurrCol;
			s = CurrChar;
		    memmove(u->data + 1, u->data, u->len++);
			if (s->ch == '\n')
			{
				u->data[0] = ' ';
				wnd->CurrCol--;
			}
			else
				u->data[0] = s->ch;
		}
		wnd->CurrCol = col;
	}
    return u;
}

/**********************************************************************
 * undo_deletechar gets the undo structure for inserting a character
 **********************************************************************/
UNDO *undo_insertchar(WINDOW wnd, int ch)
{
    UNDO *u = getundo(wnd, UNDO_INSERT);
    if (!u)
        return u;
    u->len++;
	if (ch == '\t' && cfg.TabsAsSpaces)
	{
		int col = wnd->CurrCol + 1;
	   	while ((col % cfg.Tabs) != 0)
			col++, u->len++;
	}
    return u;
}
/**********************************************************************
 * undo_pchar undoes a delete character operation
 **********************************************************************/
static void undo_pchar(WINDOW wnd, int ch)
{
    insertchar(wnd, ch);
}

static void Insert(WINDOW wnd, char *s, int len)
{
	int blen = wnd->textlen + len + 64 + 2;
	char *pos = DFmalloc(blen), *pos2;
	ATTRCHR *p = CurrChar;
	int ofs =0, i;
	p -= wnd->CurrCol;
	for (i=0; i < wnd->CurrCol; i++)
		if (p[i].ch != pTab)
			ofs++;
	SendMessage(wnd, GETTEXT, (PARAM)pos, (PARAM)blen);
	pos2 = pos;
	for (i=0; i < wnd->CurrLine; i++)
	{
		while (*pos2++ != '\n');
	}
	pos2 += ofs;
	memmove(pos2 + len, pos2, pos + strlen(pos) + 1 - pos2);
	for (i=0; i < len; i++)
		pos2[i] = s[i];
	SendMessage(wnd, SETTEXT, (PARAM)pos, (PARAM)FALSE);
	FormatBufferFromScratch(GetText(wnd), 0, wnd->textlen, ColorizeType(wnd));
	free(pos);
	SendMessage(wnd, PAINT, 0, 0);
}
/**********************************************************************
 * doundo is the primary routine to traverse the undo list and perform
 * an undo operation
 **********************************************************************/

int doundo(WINDOW wnd)
{
    int rv = 0;
	EDITSTRUCT *eds = wnd->extension;
    if (eds->undohead != eds->undotail)
    {
        UNDO *u;
        int oldinsert = wnd->InsertMode;
		int oldwrap = wnd->WordWrapMode;
        int start, end, len;
        char *s;
		ATTRCHR *ps, *pe;
        int x, i;
        x = eds->undohead - 1;
        if (x < 0)
            x += UNDO_MAX;
        u = &eds->undolist[x];
        eds->undoing++;
        wnd->InsertMode = TRUE;
		wnd->WordWrapMode = FALSE;
        switch (u->type)
        {
            case UNDO_INSERT:
				ps = pe = CharPos(wnd, u->postselstartline, u->postselstartcolumn);
                ps--;
				len=1;
				u->len--;
				while (u->len && keysym(ps->ch) && keysym((--ps)->ch))
				{
					len++;
					u->len--;
				}
				if (len == 1) // for deleting EOLs
				{
	                wnd->BlkBegLine = wnd->BlkEndLine = 0;
	                wnd->BlkEndCol = 0;
	                wnd->BlkBegCol = 0;
					wnd->CurrLine = u->postselstartline;
					wnd->CurrCol = u->postselstartcolumn;
					insertchar(wnd, RUBOUT);
				}
				else
				{
	                wnd->BlkBegLine = wnd->BlkEndLine = u->postselstartline;
    	            wnd->BlkEndCol = u->postselstartcolumn;
	                wnd->BlkBegCol = u->postselstartcolumn - len;
					wnd->CurrLine = u->postselstartline;
					wnd->CurrCol = u->postselstartcolumn - len;
					insertchar(wnd, DEL);
				}
				
        	    if (u->len <= 0)
            	{
                	ChangedText(wnd, u->modified);
                	eds->undohead = x;
            	}
				u->postselstartline = u->postselendline = wnd->CurrLine;
				u->postselstartcolumn = u->postselendcolumn = wnd->CurrCol;
                break;
            case UNDO_DELETE:
				start = 1;
				while (start < u->len && keysym(u->data[start-1]) && keysym(u->data[start]))
				{
					start++;
				}
				s = DFmalloc(start + 1);
				for (i=0; i < start; i++)
					s[i] = u->data[start-i-1];
				s[start] = 0;
                wnd->BlkBegLine = wnd->BlkEndLine = 0;
                wnd->BlkBegCol = wnd->BlkEndCol = 0;
				wnd->CurrLine = u->preselstartline;
				wnd->CurrCol = u->preselstartcolumn;
				SendMessage(wnd, KEYBOARD_CURSOR, wnd->CurrCol-wnd->wleft, wnd->CurrLine-wnd->wtop);
				if (start == 1)
					undo_pchar(wnd, s[0]);
				else
					Insert(wnd, &s[0], start);
				free(s);
                if (u->len <= start)
                {
                    ChangedText(wnd, u->modified);
                    eds->undohead = x;
                    if (u->max > 64)
                    {
                        s = realloc(u->data, 64);
                        if (s)
                        {
                            u->data = s;
                            u->max = 64;
                        }
                    }
                }
                else
                {
                    memcpy(u->data, u->data + start, u->len - start);
                    u->len -= start;
                }
				u->postselstartline = u->postselendline = wnd->CurrLine;
				u->postselstartcolumn = u->postselendcolumn = wnd->CurrCol;
                break;
            case UNDO_BACKSPACE:
				start = 1;
				while (start < u->len && keysym(u->data[start-1]) && keysym(u->data[start]))
				{
					start++;
				}
                wnd->BlkBegLine = wnd->BlkEndLine = 0;
                wnd->BlkBegCol = wnd->BlkEndCol = 0;
				wnd->CurrLine = u->postselstartline;
				wnd->CurrCol = u->postselstartcolumn;
				SendMessage(wnd, KEYBOARD_CURSOR, wnd->CurrCol-wnd->wleft, wnd->CurrLine-wnd->wtop);
				if (start == 1)
					undo_pchar(wnd, u->data[0]);
				else
					Insert(wnd, &u->data[0], start);
                if (u->len <= start)
                {
                    ChangedText(wnd, u->modified);
                    eds->undohead = x;
                    if (u->max > 64)
                    {
                        s = realloc(u->data, 64);
                        if (s)
                        {
                            u->data = s;
                            u->max = 64;
                        }
                    }
                }
                else
                {
                    memcpy(u->data, u->data + start, u->len - start);
                    u->len -= start;
                }
				u->postselstartline = u->postselendline = wnd->CurrLine;
				u->postselstartcolumn = u->postselendcolumn = wnd->CurrCol;
                break;
            case UNDO_MODIFY:
                wnd->InsertMode = FALSE;
                start = 0;
                wnd->BlkBegLine = wnd->BlkEndLine = 0;
                wnd->BlkBegCol = wnd->BlkEndCol = 0;
				wnd->CurrLine = u->postselstartline;
				wnd->CurrCol = u->postselstartcolumn;
				SendMessage(wnd, KEYBOARD_CURSOR, wnd->CurrCol-wnd->wleft, wnd->CurrLine-wnd->wtop);
                if (keysym(u->data[0]))
	                while (keysym(u->data[start]) && u->len > start)
	                {
						insertchar(wnd, BS);
	                    undo_pchar(wnd, u->data[start++]);
						insertchar(wnd, BS);
	                }
                else
                {
					insertchar(wnd, BS);
                    undo_pchar(wnd, u->data[start++]);
					insertchar(wnd, BS);
                    if (u->data[start] == '\n')
                    {
						insertchar(wnd, BS);
                        undo_pchar(wnd, u->data[start++]);
						insertchar(wnd, BS);
                    }
                }
                if (u->len <= start)
                {
                    ChangedText(wnd, u->modified);
                    eds->undohead = x;
                    if (u->max > 64)
                    {
                        s = realloc(u->data, 64);
                        if (s)
                        {
                            u->data = s;
                            u->max = 64;
                        }
                    }
                }
                else
                {
                    memcpy(u->data, u->data + start, u->len - start);
                    u->len -= start;
                }
                wnd->BlkBegLine = wnd->BlkEndLine = 0;
                wnd->BlkBegCol = wnd->BlkEndCol = 0;
				u->postselstartline = wnd->CurrLine;
				u->postselstartcolumn = wnd->CurrCol;
                break;
            case UNDO_INSERTSELECTION:
                wnd->BlkBegLine = u->postselstartline;
                wnd->BlkBegCol = u->postselstartcolumn;
                wnd->BlkEndLine = u->postselendline;
                wnd->BlkEndCol = u->postselendcolumn;
				wnd->CurrLine = u->postselstartline;
				wnd->CurrCol = u->postselstartcolumn;
				SendMessage(wnd, KEYBOARD_CURSOR, wnd->CurrCol-wnd->wleft, wnd->CurrLine-wnd->wtop);
				insertchar(wnd, DEL);
                ChangedText(wnd, u->modified);
                eds->undohead = x;
                wnd->BlkBegLine = u->preselstartline;
                wnd->BlkBegLine = u->preselstartcolumn;
                wnd->BlkEndLine = u->preselendline;
                wnd->BlkEndLine = u->preselendcolumn;
				wnd->CurrLine = u->preselstartline;
				wnd->CurrCol = u->preselstartcolumn;
				SendMessage(wnd, KEYBOARD_CURSOR, wnd->CurrCol - wnd->wleft, wnd->CurrLine - wnd->wtop);
                eds->undohead = x;
                break;
            case UNDO_DELETESELECTION:
				wnd->CurrLine = u->postselstartline;
				wnd->CurrCol = u->postselstartcolumn;
				SendMessage(wnd, KEYBOARD_CURSOR, wnd->CurrCol-wnd->wleft, wnd->CurrLine-wnd->wtop);
                Insert(wnd, u->data, u->len);
                wnd->BlkBegLine = u->postselstartline;
                wnd->BlkBegCol = u->postselstartcolumn;
                wnd->BlkEndLine = u->postselendline;
                wnd->BlkEndCol = u->postselendcolumn;
                ChangedText(wnd, u->modified);
                eds->undohead = x;
                if (u->max > 64)
                {
                    s = realloc(u->data, 64);
                    if (s)
                    {
                        u->data = s;
                        u->max = 64;
                    }
                }
                wnd->BlkBegLine = u->preselstartline;
                wnd->BlkBegLine = u->preselstartcolumn;
                wnd->BlkEndLine = u->preselendline;
                wnd->BlkEndLine = u->preselendcolumn;
				wnd->CurrLine = u->preselstartline;
				wnd->CurrCol = u->preselstartcolumn;
				SendMessage(wnd, KEYBOARD_CURSOR, wnd->CurrCol-wnd->wleft, wnd->CurrLine-wnd->wtop);
                eds->undohead = x;
                break;
            case UNDO_CASECHANGE:
                wnd->BlkBegLine = u->postselstartline;
                wnd->BlkBegCol = u->postselstartcolumn;
                wnd->BlkEndLine = u->postselendline;
                wnd->BlkEndCol = u->postselendcolumn;
				wnd->CurrLine = u->postselstartline;
				wnd->CurrCol = u->postselstartcolumn;
                eds->undohead = x;
				start = 0;
				ps = CharPos(wnd, wnd->CurrLine, wnd->CurrCol);
				while (start < u->len)
				{
					if (ps->ch != pTab)
					{
						if (u->data[start] != '\t')
							ps->ch = u->data[start];
						start++;
					}
					ps++;
				}
				ChangedText(wnd, u->modified);
				SendMessage(wnd, PAINT, 0, 0);
                if (u->max > 64)
                {
                    s = realloc(u->data, 64);
                    if (s)
                    {
                        u->data = s;
                        u->max = 64;
                    }
                }
                wnd->BlkBegLine = u->preselstartline;
                wnd->BlkBegCol = u->preselstartcolumn;
                wnd->BlkEndLine = u->preselendline;
                wnd->BlkEndCol = u->preselendcolumn;
				wnd->CurrLine = u->preselstartline;
				wnd->CurrCol = u->preselstartcolumn;
				SendMessage(wnd, KEYBOARD_CURSOR, wnd->CurrCol-wnd->wleft, wnd->CurrLine-wnd->wtop);
                break;
            case UNDO_AUTOBEGIN:
	            eds->undohead = x;
    	        while (eds->undohead != eds->undotail && !doundo(wnd))
        	        ;
                break;
            case UNDO_AUTOCHAINBEGIN:
                eds->undohead = x;
                while (eds->undohead != eds->undotail && !doundo(wnd))
                    ;
                doundo(wnd); 
                    // will undo things recursively if there are more auto-begins
                break;
            case UNDO_AUTOEND:
                eds->undohead = x;
                wnd->BlkBegLine = u->preselstartline;
                wnd->BlkBegLine = u->preselstartcolumn;
                wnd->BlkEndLine = u->preselendline;
                wnd->BlkEndLine = u->preselendcolumn;
				wnd->CurrLine = u->preselstartline;
				wnd->CurrCol = u->preselstartcolumn;
				SendMessage(wnd, KEYBOARD_CURSOR, wnd->CurrCol-wnd->wleft, wnd->CurrLine-wnd->wtop);
                ChangedText(wnd, u->modified);
                rv = 1;
                break;
        }
        wnd->InsertMode = oldinsert;
		wnd->WordWrapMode = oldwrap;
        eds->undoing--;
			
//        ScrollCaretIntoView(wnd, p);
    }
    return rv;
}
int firstword(ATTRCHR *pos, char *name)
{
	int l = strlen(name);
	int i;
	for (i=0; i < l; i++)
		if (!pos[i].ch || pos[i].ch != name[i])
			return FALSE ;
	if (isalnum(pos[i].ch) || pos[i].ch == '_')
		return FALSE;
	return TRUE ;
}

/**********************************************************************
 * tab to the current line position
 **********************************************************************/
void insertcrtabs(WINDOW wnd)
{
	EDITSTRUCT *eds = (EDITSTRUCT *)wnd->extension;
	ATTRCHR *pos = GetText(wnd), *base = pos;
	int oldcol = wnd->CurrCol;
	int oldline = wnd->CurrLine;
    int n = 0, m;
    int oldinsert = wnd->InsertMode;
    if (ColorizeType(wnd) == COLORIZE_NONE)
        return ;
	if (eds->undoing)
		return;
//	if (!(cfg.appdata[CFG_EDITFLAGS] & AUTO_INDENT));
//        return ;
    wnd->InsertMode = TRUE;
    while (wnd->CurrLine > 0)
    {
		ATTRCHR *pos2;
		SendMessage(wnd, KEYBOARD, BS, 0);
		pos2 = CurrChar;
		SendMessage(wnd, KEYBOARD, HOME, 0);
		base = pos  = CurrChar;
		while (whitespace(pos->ch) && pos->ch != '\n')
		{
			pos++;
		}
        if (pos->ch != '#')
		{
			ATTRCHR *storepos;
			int parencount = 0;
			while (pos2-- > pos)
			{
				if (pos2->ch == '(')
				{
					storepos = pos2 ;
					if (++parencount > 0)
					{
						break;
					}
				}
				else
					if (pos2->ch == ')')
						parencount--;
			} 
			if (parencount > 0)
			{
				pos = storepos + 1 ;
				break;
			}
			else if (parencount == 0)
			{
	            break;
			}
		}
		wnd->CurrLine --;
    }
	n = pos - base;
    insertautoundo(wnd, UNDO_AUTOEND);
	if (firstword(pos, "if") || firstword(pos, "else")
		||firstword(pos, "do") || firstword(pos, "for")
		||firstword(pos, "while"))
		n += cfg.Tabs;
	SendMessage(wnd, KEYBOARD_CURSOR, oldcol-wnd->wleft, oldline-wnd->wtop);
	m = n;
    while (n >= cfg.Tabs)
    {
        insertchar(wnd, '\t');
        n -= cfg.Tabs;
    }
    while (n--)
        insertchar(wnd, ' ');
    insertautoundo(wnd, UNDO_AUTOCHAINBEGIN);
	wnd->InsertMode = oldinsert;
	SendMessage(wnd, KEYBOARD_CURSOR, m-wnd->wleft, oldline-wnd->wtop);
}
//-------------------------------------------------------------------------
void DeletePound(WINDOW wnd)
{
	EDITSTRUCT *eds = (EDITSTRUCT *)wnd->extension;
	ATTRCHR *s = CurrChar;
	int n = wnd->CurrCol;
    if (ColorizeType(wnd) != COLORIZE_C)
        return ;
	if (eds->undoing)
		return;
	if (wnd->CurrCol == 0)
		return;
	if (!(cfg.appdata[CFG_EDITFLAGS] & AUTO_FORMAT))
        return ;
	s--, n--;
	if (!n || s->ch != '#')
		return;
	do
	{
		s--;
		if (!whitespace(s->ch))
			return;
	} while (--n);
    insertautoundo(wnd, UNDO_AUTOEND);
	MarkTextBlock(wnd, wnd->CurrLine, 0, wnd->CurrLine, wnd->CurrCol-1);
	SendMessage(wnd, KEYBOARD_CURSOR, 0 - wnd->wleft, wnd->CurrLine -wnd->wtop);
	insertchar(wnd, DEL);
    insertautoundo(wnd, UNDO_AUTOCHAINBEGIN);
	MarkTextBlock(wnd, 0, 0, 0, 0);
	SendMessage(wnd, KEYBOARD_CURSOR, 1 - wnd->wleft, wnd->CurrLine -wnd->wtop);
}
//-------------------------------------------------------------------------


void DeletePercent(WINDOW wnd)
{
	EDITSTRUCT *eds = (EDITSTRUCT *)wnd->extension;
	ATTRCHR *s = CurrChar;
	int n = wnd->CurrCol;
    if (ColorizeType(wnd) != COLORIZE_ASM)
        return ;
	if (eds->undoing)
		return;
	if (wnd->CurrCol == 0)
		return;
	if (!(cfg.appdata[CFG_EDITFLAGS] & AUTO_FORMAT))
        return ;
	s--, n--;
	if (!n || s->ch != '%')
		return;
	do
	{
		s--;
		if (!whitespace(s->ch))
			return;
	} while (--n);
    insertautoundo(wnd, UNDO_AUTOEND);
	MarkTextBlock(wnd, wnd->CurrLine, 0, wnd->CurrLine, wnd->CurrCol-1);
	insertchar(wnd, DEL);
    insertautoundo(wnd, UNDO_AUTOCHAINBEGIN);
	MarkTextBlock(wnd, 0, 0, 0, 0);
	SendMessage(wnd, KEYBOARD_CURSOR, 1 - wnd->wleft, wnd->CurrLine -wnd->wtop);
}

void CancelParenMatch(WINDOW wnd)
{
	EDITSTRUCT *p = (EDITSTRUCT *)wnd->extension;
	if (p->matchingEnd != 0)
	{
		ATTRCHR *s = GetText(wnd);
		s[p->matchingEnd].attrib = p->matchEndAttrib;
		s[p->matchingStart].attrib = p->matchStartAttrib;
		p->matchingEnd = p->matchingStart = 0;
		SendMessage(wnd, PAINT, 0, 0);
	}
}
static int FindParenMatchBackward(WINDOW wnd, EDITSTRUCT *p)
{
	int skip,match;
	int level = 1;
	ATTRCHR *s = CurrChar;
	ATTRCHR *start = GetText(wnd);
	int quotechar = 0;
	if (TextBlockMarked(wnd) || s == start)
		return FALSE;		
	s--;
	if (s->ch == ')')
		skip = ')', match = '(';
	else if (s->ch == '}')
		skip = '}', match = '{';
	else if (s->ch == ']')
		skip = ']', match = '[';
	else
		return FALSE;
	while (--s != start)
	{
		if (quotechar == s->ch && (s == start || s[-1].ch != '\\' 
											   || s-start < 2 || s[-2].ch == '\\'))
			quotechar = 0;
		else if (!quotechar)
			if ((s->ch == '\'' || s->ch == '"')
				&& (s == start || s[-1].ch != '\\'
					|| s -start < 2 || s[-2].ch == '\\'))
				quotechar = s->ch;
			else if (s->ch == skip)
				level++;
			else if (s->ch == match)
				if (!--level)
					break;
	}
	if (level)
		return FALSE;
	p->matchingEnd = s - start;
	p->matchEndAttrib = s->attrib;
	s->attrib = (s->attrib & 0x0f) | (LIGHTGREEN << 4);
	s = CurrChar - 1;
	p->matchingStart = s-start;
	p->matchStartAttrib = s->attrib;
	s->attrib = (s->attrib & 0x0f) | (LIGHTGREEN << 4);
	SendMessage(wnd, PAINT, 0, 0);
	return TRUE;
}
static int FindParenMatchForward(WINDOW wnd, EDITSTRUCT *p)
{
	int skip,match;
	int level = 1;
	ATTRCHR *s = CurrChar;
	ATTRCHR *start = GetText(wnd);
	int quotechar = 0;
	if (TextBlockMarked(wnd) || s == start)
		return FALSE;
	s--;
	if (s->ch == '(')
		skip = '(', match = ')';
	else if (s->ch == '{')
		skip = '{', match = '}';
	else if (s->ch == '[')
		skip = '[', match = ']';
	else
		return FALSE;
	while ((++s) - start < wnd->textlen)
	{
		if (quotechar == s->ch && (s[-1].ch != '\\'
											   || s -start < 2 || s[-2].ch == '\\'))
			quotechar = 0; 
		else if (!quotechar)
			if ((s->ch == '\'' || s->ch == '"')
				&& (s[-1].ch != '\\' || s -start < 2 || s[-2].ch == '\\'))
				quotechar = s->ch;
			else if (s->ch == skip)
				level++;
			else if (s->ch == match)
				if (!--level)
					break;
	}
	if (level)
		return FALSE;
	p->matchingEnd = s - start;
	p->matchEndAttrib = s->attrib;
	s->attrib = (s->attrib & 0x0f) | (LIGHTGREEN << 4);
	s = CurrChar - 1;
	p->matchingStart = s - start;
	p->matchStartAttrib = s->attrib;
	s->attrib = (s->attrib & 0x0f) | (LIGHTGREEN << 4);
	SendMessage(wnd, PAINT, 0, 0);
	return TRUE;
}
void FindParenMatch(WINDOW wnd)
{
	EDITSTRUCT *eds = (EDITSTRUCT *)wnd->extension;
//	if (parenthesisCheck)
		if (!FindParenMatchForward(wnd, eds))
			FindParenMatchBackward(wnd, eds);
}


//-------------------------------------------------------------------------

static ATTRCHR *spacedend(WINDOW wnd, ATTRCHR *s)
{
    ATTRCHR *rv = 0;
    while (s != GetText(wnd) && s[- 1].ch != '\n')
	{
		--s;
        if (!whitespace(s->ch))
		{
            return 0;
		}
	}
    rv = s;
    while (s - GetText(wnd) < wnd->textlen && s->ch && whitespace(s->ch) && s->ch !=
        '\n')
        s++;
    if (s->ch == '}')
        return rv;
    else
        return 0;
}

//-------------------------------------------------------------------------

static ATTRCHR *preprocline(WINDOW wnd, ATTRCHR *s)
{
    ATTRCHR * rv;
    while (s != GetText(wnd) && s[- 1].ch != '\n')
        s--;
    rv = s;
    while (whitespace(s->ch))
       s++;
    if (s->ch == '#')
        return rv;
    else
        return  0;
}

/**********************************************************************
 * tab to the current line position
 **********************************************************************/
void InsertBeginTabs(WINDOW wnd)
{
	EDITSTRUCT *eds = (EDITSTRUCT *)wnd->extension;
    ATTRCHR *pos;
	int n;
	int parencount = 0;
	ATTRCHR *solpos, *eospos;
    int oldinsert = wnd->InsertMode;
	int startLine, startCol, endLine, endCol;
    if (ColorizeType(wnd) != COLORIZE_C)
        return ;
	if (eds->undoing)
		return ;
	if (!(cfg.appdata[CFG_EDITFLAGS] & AUTO_FORMAT))
        return ;
	wnd->InsertMode = TRUE;
	pos = CurrChar-1;
	solpos = pos;
	while (solpos != GetText(wnd) && solpos[ - 1].ch != '\n')
	{
		if (!whitespace(solpos[ - 1].ch))
		{
			wnd->InsertMode = oldinsert;
			return ;
		}
		solpos--;
	}
	if (solpos != GetText(wnd))
		pos = solpos-1 ;
	eospos = solpos;
	while (eospos - GetText(wnd) < wnd->textlen && whitespace(eospos->ch) &&
		   	eospos->ch != '\n')
		eospos++;
    while (1)
    {
		ATTRCHR *pos2 = pos ;
        while (pos && pos[ - 1].ch != '\n')
            pos--;
        while (pos->ch && whitespace(pos->ch) && pos->ch != '\n')
            pos++;
        if (pos->ch != '#')
		{
			while (pos2-- > pos &&
				   pos2->ch && pos2->ch != '\n')
			{
				if (pos2->ch == '(')
				{
					if (++parencount == 0)
						break;
				}
				else
					if (pos2->ch == ')')
						if (--parencount == 0)
							break;
			} 
			if (parencount >= 0)
	            break;
		}
        while (pos != GetText(wnd) && pos [- 1].ch != '\n')
            pos--;
        if (pos == GetText(wnd))
            break;
        pos--;
    }
	while (pos > GetText(wnd) && pos[-1].ch != '\n')
		pos--;
	while (whitespace(pos->ch) && pos - GetText(wnd) < wnd->textlen)
		pos++;
    insertautoundo(wnd, UNDO_AUTOEND);
    n = 0;
	while (pos != GetText(wnd) && pos[-1].ch != '\n')
		pos--, n++;
	startLine = endLine = 0;
	startCol = endCol = 0;
	while (TextLine(wnd, startLine + 1) <= solpos)
		   startLine++;
	startCol = solpos - TextLine(wnd, startLine);
	while (TextLine(wnd, endLine + 1) <= eospos)
		   endLine++;
	endCol = eospos - TextLine(wnd, endLine);
	if (eospos != solpos)
	{
		MarkTextBlock(wnd, startLine, startCol, endLine, endCol);
		insertchar(wnd, DEL);
		MarkTextBlock(wnd, 0, 0, 0, 0);
	}
	startLine -= wnd->wtop;
	startCol -= wnd->wleft;
	SendMessage(wnd, KEYBOARD_CURSOR, startCol, startLine);
    while (n >= cfg.Tabs && !cfg.TabsAsSpaces)
    {
        insertchar(wnd, '\t');
        n -= cfg.Tabs;
    }
    while (n--)
        insertchar(wnd, ' ');
	insertchar(wnd, FWD);
    insertautoundo(wnd, UNDO_AUTOCHAINBEGIN);
	wnd->InsertMode = oldinsert;
	SendMessage(wnd, PAINT, 0, 0);	
}
/**********************************************************************
 * tab to the current line position
 **********************************************************************/
void InsertEndTabs(WINDOW wnd, int newend)
{
	EDITSTRUCT *eds = (EDITSTRUCT *)wnd->extension;
    ATTRCHR *pos;
	int n;
    ATTRCHR * solpos, *eospos;
    ATTRCHR *lsolpos, *leospos;
	int startLine, startCol, endLine, endCol;
    int oldinsert = wnd->InsertMode;
    if (ColorizeType(wnd) != COLORIZE_C)
        return ;
	if (eds->undoing)
		return ;
    if (!newend)
        return ;
	if (!(cfg.appdata[CFG_EDITFLAGS] & AUTO_FORMAT))
        return ;
    wnd->InsertMode = TRUE;
    leospos = pos = CurrChar -1;
    while (leospos - GetText(wnd) < wnd->textlen && whitespace(leospos->ch)
		   && leospos->ch != '\n' )
        leospos++;
    if (lsolpos = spacedend(wnd, pos))
    {
        int indentlevel = 0;
        eospos = GetText(wnd);
        pos--;
        while (pos > GetText(wnd))
        {
            ATTRCHR *pos1 = preprocline(wnd, pos);
            if (pos1 !=  0)
                pos = pos1;
            else if (pos->attrib != C_COMMENT)
                if (pos->ch == '{')
		            if (!indentlevel)
		            {
		                while (pos != GetText(wnd) && pos[ - 1].ch != '\n')
		                        pos--;
		                while (whitespace(pos->ch))
		                    pos++;
		                eospos = pos;
		                break;
		            }
		            else
		                indentlevel--;
	            else if (pos->ch == '}')
	                indentlevel++;
            pos--;
        }
        insertautoundo(wnd, UNDO_AUTOEND);
	    n = 0;
		while (pos != GetText(wnd) && pos[-1].ch != '\n')
			pos--, n++;
		startLine = endLine = 0;
		startCol = endCol = 0;
		while (TextLine(wnd, startLine + 1) <= lsolpos)
			   startLine++;
		startCol = lsolpos - TextLine(wnd, startLine);
		while (TextLine(wnd, endLine + 1) <= leospos)
			   endLine++;
		endCol = leospos - TextLine(wnd, endLine);
		if (leospos != lsolpos)
		{
			MarkTextBlock(wnd, startLine, startCol, endLine, endCol);
			insertchar(wnd, DEL);
			MarkTextBlock(wnd, 0, 0, 0, 0);
		}
		startLine -= wnd->wtop;
		startCol -= wnd->wleft;
		SendMessage(wnd, KEYBOARD_CURSOR, startCol, startLine);
	    while (n >= cfg.Tabs && !cfg.TabsAsSpaces)
	    {
	        insertchar(wnd, '\t');
	        n -= cfg.Tabs;
	    }
        while (n--)
            insertchar(wnd, ' ');
		insertchar(wnd, FWD);
        insertautoundo(wnd, UNDO_AUTOCHAINBEGIN);
		SendMessage(wnd, PAINT, 0, 0);	
    }
    wnd->InsertMode = oldinsert;


}

//-------------------------------------------------------------------------
void SelectIndent(WINDOW wnd, int insert)
{
	EDITSTRUCT *eds = (EDITSTRUCT *)wnd->extension;
	int startLine = wnd->BlkBegLine, osl;
	int startCol = wnd->BlkBegCol;
	int endLine = wnd->BlkEndLine;
	int endCol = wnd->BlkEndCol;
	int decd = FALSE;
	int inverted = FALSE ;
    int oldinsert = wnd->InsertMode;
    if (ColorizeType(wnd) != COLORIZE_C)
        return ;
	if (eds->undoing)
		return ;
	if (!(cfg.appdata[CFG_EDITFLAGS] & AUTO_FORMAT))
        return ;
    if (!TextBlockMarked(wnd))
        return ;
    wnd->InsertMode = TRUE;
    if (endLine < startLine)
    {
		int temp = startLine;
		startLine = endLine;
		endLine = temp;
		temp = startCol;
		startCol = endCol;
		endCol = temp;
		
    }
	else
	if (endCol)
	{
		endLine++;
		endCol = 0;
	}
	startCol = 0;
	osl= startLine;
    insertautoundo(wnd, UNDO_AUTOEND);
	MarkTextBlock(wnd, 0, 0, 0, 0);
    while (startLine < endLine)
    {
		SendMessage(wnd, KEYBOARD_CURSOR, startCol - wnd->wleft, startLine - wnd->wtop);
        if (insert)
        {
			if (cfg.TabsAsSpaces)
			{
				int n = cfg.Tabs;
		        while (n--)
    		        insertchar(wnd, ' ');
			}
			else
			{
				insertchar(wnd, '\t');
			}
        }
        else
        {
			ATTRCHR *start = TextLine(wnd, startLine);
            if (whitespace(start->ch) && start->ch != '\n')
            {
				if (start->ch == sTab)
				{
					insertchar(wnd, DEL);
				}
				else
				{
					int i;
					for (i=0; i < cfg.Tabs; i++)
					{
						if (!whitespace(start->ch) || start->ch == '\n')
							break;
						insertchar(wnd, DEL);
					}
				}
            }
        }
		startLine++;
    }
	MarkTextBlock(wnd, osl, 0, endLine, 0);
	SendMessage(wnd, KEYBOARD_CURSOR, -wnd->wleft, endLine - wnd->wtop);
	wnd->InsertMode = oldinsert;
    insertautoundo(wnd, UNDO_AUTOBEGIN);
	SendMessage(wnd, PAINT, 0, 0);
}
