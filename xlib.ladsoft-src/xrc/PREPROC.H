/* 
XRC resource compiler
Copyright 1997, 1998 Free Software Foundation, Inc.
Written by Ian Lance Taylor, Cygnus Support.
Copyright 2006-2011 David Lindauer.

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

You may contact the author of this derivative at:
	mailto::camille@bluegrass.net
 */
enum e_sym
{
    /* first comes all the C++ overloadable operators */
    /* be sure to change expectlist if you insert things */
    kw_new, kw_delete, plus, minus, star, divide, lshift, rshift, modop, eq,
        neq, lt, leq, gt, geq, assign, asplus, asminus, astimes, asdivide,
        asmodop, aslshift, asrshift, asand, asor, asxor, autoinc, autodec,
        openbr, openpa, pointstar, pointsto, lor, land, not, or, and, uparrow,
        compl, 
    /* then generic stuff that isn't overloadable or is internal */
    ident, cconst, iconst, lconst, iuconst, luconst, sconst, lsconst, fconst,
        lrconst, rconst, hook, colon, begin, end, dot, dostar, closebr, closepa,
        ellipse, comma, semicolon, classsel, classselstar, dotstar, backslash, 
    /* rc DEFINITIONS */
    kw_accelerator, kw_alt, kw_ascii, kw_auto3state, kw_autocheckbox,
        kw_autoradiobutton, kw_begin, kw_bitmap, kw_block, kw_caption,
        kw_characteristics, kw_checkbox, kw_checked, kw_class, kw_combobox,
        kw_control, kw_ctext, kw_cursor, kw_defpushbutton, kw_dialog,
        kw_discardable, kw_dlginclude, kw_edittext, kw_end, kw_exstyle,
        kw_filedate, kw_fileflagmask, kw_fileflags, kw_fileos, kw_filesubtype,
        kw_filetype, kw_fileversion, kw_fixed, kw_font, kw_grayed, kw_groupbox,
        kw_help, kw_icon, kw_impure, kw_inactive, kw_language, kw_listbox,
        kw_loadoncall, kw_ltext, kw_menu, kw_menubarbreak, kw_menubreak,
        kw_menuitem, kw_messagetable, kw_moveable, kw_nondiscardable, kw_not,
        kw_popup, kw_preload, kw_productversion, kw_pushbutton, kw_radiobutton,
        kw_rcdata, kw_rcinclude, kw_rtext, kw_scrollbar, kw_separator, kw_shift,
        kw_state3, kw_stringtable, kw_style, kw_value, kw_version,
        kw_versioninfo, kw_virtkey, kw_italic, kw_weight, kw_dialogex,
        kw_menuex, kw_pure, kw_noinvert, 
    /* Finally tokens for keywords */
    //	kw_int, kw_void, kw_char, kw_float, kw_double, kw_struct, kw_union,
    //        kw_long, kw_short, kw_unsigned, kw_signed, kw_auto, kw_extern,
    //        kw_register, kw_typedef, kw_static, kw_goto, kw_return,
    //        kw_sizeof, kw_break, kw_continue, kw_if, kw_else, kw_for,
    //        kw_do, kw_while, kw_switch, kw_case, kw_default, kw_enum, kw_volatile,
    //	kw_const, kw_phitext, kw__trap, kw__interrupt, kw__abs, kw__genword,
    /* C++ */
    //	kw_public, kw_private, kw_protected, kw_class, kw_friend,
    //	kw_namespace, kw_this, kw_operator, kw_inline, kw_virtual,
    //	kw_try,kw_throw,kw_catch, kw_template, kw_bool, kw_true, kw_false,
    eol, eof
};

/* Global symbol table is a hash table */
#define HASHTABLESIZE 1023

/* #define tracking */
typedef struct
{
    char *string;
    short argcount;
    char **args;
} DEFSTRUCT;

/* struct for preprocessor if tracking */
typedef struct ifstruct
{
    struct ifstruct *link; /* link */
    short iflevel;
    short elsetaken;
} IFSTRUCT;

enum err
{
    ERR_PREPROCMATCH, ERR_COMMENTMATCH, ERR_NEEDCHAR, ERR_CONSTTOOLARGE,
        ERR_FPCON, ERR_CHAR4CHAR, ERR_ILLCHAR, ERR_IDEXPECT, ERR_PREPROCID,
        ERR_ERROR, ERR_USERERR, ERR_USERWARN, ERR_PREPIG, ERR_UPDOWN,
        ERR_INCLFILE, ERR_CANTOPEN, ERR_PUNCT, ERR_MACROSUBS,
        ERR_WRONGMACROARGS, ERR_UNEXPECT, ERR_INSERT, ERR_NEEDCONST,
        ERR_UNDEFINED, ERR_DUPSYM, ERR_IDENTEXPECT, ERR_EXPREXPECT, ERR_INTERP,
        ERR_STRINGTOOBIG, ERR_INVALIDSTRING, ERR_EXTRA_DATA_ON_LINE,
        ERR_BEGIN_EXPECTED, ERR_END_EXPECTED, ERR_RESOURCE_ID_EXPECTED,
        ERR_STRING_EXPECTED, ERR_ACCELERATOR_CONSTANT_EXPECTED,
        ERR_NO_ASCII_VIRTKEY, ERR_NOT_DIALOGEX,
        ERR_UNKNOWN_DIALOG_CONTROL_CLASS, ERR_VERSIONINFO_TYPE_1,
        ERR_UNKNOWN_RESOURCE_TYPE, ERR_INVALIDCLASS, ERR_FIXEDDATAEXPECTED,
        ERR_BLOCK_EXPECTED, ERR_INVALID_VERSION_INFO_TYPE, 
};
struct stab
{
    struct sym *head,  *tail;
};

/* symbols */
struct sym
{
    struct sym *next; /* next symbol (local tabs only) */
    char *name; /* symbol name */
    /* these fields depend on storage_class */
    union
    {
        long i; /* int val */
        unsigned long u; /* nsigned val */
        long double f; /* float val */
        char *s; /* string val */
    } value;
};
/* error list */
struct errl
{
    struct errl *link;
    short errornumber;
    void *data;
};
typedef struct stab TABLE;
typedef struct sym SYM;
#define ERRORS struct errl
#define BALANCE struct balance
#define BAL_PAREN   0
#define BAL_BRACKET 0
#define ERRORS struct errl

struct balance
{
    struct balance *back;
    short type;
    short count;
};
typedef struct kwblk
{
    struct kwblk *next;
    char *word;
    short stype;
    enum
    {
        KW_CPLUSPLUS = 1, KW_INLINEASM = 2, KW_NONANSI = 4
    } flags;
}

//-------------------------------------------------------------------------

KEYWORDS;

#define INPUT_BUFFER_LEN 32768

#include "preproc.p"
