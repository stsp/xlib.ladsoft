#define LSVERSION "2.0.1.0"

#pragma pack(1)

#define ID_TOUPPER ID_USER
#define ID_TOLOWER (ID_USER + 1)
#define ID_AUTOINDENT (ID_USER + 2)
#define ID_AUTOFORMAT (ID_USER + 3)
#define ID_PUSHRIGHT (ID_USER + 4)
#define ID_PUSHLEFT (ID_USER + 5)
#define ID_COLORIZENONE (ID_USER + 6)
#define ID_COLORIZEAUTO (ID_USER + 7)
#define ID_COLORIZEC    (ID_USER + 8)
#define ID_COLORIZEASM  (ID_USER + 9)
#define ID_SYNTAX (ID_USER + 10)

#define COLORIZE_NONE 0
#define COLORIZE_AUTO 1
#define COLORIZE_C 2
#define COLORIZE_ASM 3

#define UNDO_MAX 100
#define UNDO_INSERT 1
#define UNDO_DELETE 2
#define UNDO_BACKSPACE 3
#define UNDO_MODIFY 4
#define UNDO_DELETESELECTION 5
#define UNDO_INSERTSELECTION 6

/* not currently supported */
#define UNDO_CASECHANGE 7
#define UNDO_AUTOEND 8
#define UNDO_AUTOBEGIN 9
#define UNDO_AUTOCHAINBEGIN 10

// a list of UNDO structures describe each operation the user performs.
// The list is traversed backwards if 'undo' is pressed.
typedef struct
{
    int preselstartline;
    int preselstartcolumn;
    int preselendline;
    int preselendcolumn;
	int precurrline;
	int recurrcolumn;
    int postselstartline;
    int postselstartcolumn;
    int postselendline;
    int postselendcolumn;
	int postcurrline;
	int postcurrcolumn;
	int charpos;
    int len;
    int max;
    unsigned char *data;
    char type;
    char modified: 1;
} UNDO;

typedef struct _editstruct
{
	char *filename;
	unsigned char colorizeType;
    UNDO undolist[UNDO_MAX];
	int undohead;
	int undotail;
	int matchingEnd;
	int matchingStart;
	int keyboardNesting;
	char undoing;
	char matchEndAttrib;
	char matchStartAttrib;
} EDITSTRUCT ;

#define CharPos(wnd, row, col) (TextLine(wnd, row) + (col))
void FormatBuffer(ATTRCHR *buf, int start, int end, int type);
void FormatBufferFromScratch(ATTRCHR *buf, int start, int end, int
    type);
void FormatLine(WINDOW wnd, ATTRCHR *buf, int type);

UNDO *undo_deletesel(WINDOW p);
UNDO *undo_casechange(WINDOW p);
UNDO *undo_insertsel(WINDOW p, char *s);
UNDO *undo_deletechar(WINDOW p, int ch, int type);
UNDO *undo_modifychar(WINDOW p, int ch);
UNDO *undo_insertchar(WINDOW p, int ch);

int doundo(WINDOW wnd);
int ColorizeType(WINDOW wnd);
void insertcrtabs(WINDOW p);
void InsertBeginTabs(WINDOW p);
void InsertEndTabs(WINDOW p, int newend);
void SelectIndent(WINDOW p, int insert);
void DeletePound(WINDOW p);
void DeletePercent(WINDOW p);
void CancelParenMatch(WINDOW wnd);
void FindParenMatch(WINDOW wnd);
void InsertBeginTabs(WINDOW wnd);
void InsertEndTabs(WINDOW wnd, int newend);
void SelectIndent(WINDOW wnd, int insert);

#define CFG_EDITFLAGS 0
#define AUTO_INDENT 1
#define AUTO_FORMAT 2

#define CFG_COLORIZE 1

#define pTab ('\t' + (unsigned char)0x80)
#define sTab ('\f' + (unsigned char)0x80)
#define whitespace(x) (isspace(x) || (x) == pTab || (x) == sTab)
