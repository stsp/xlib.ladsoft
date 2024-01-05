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
#define CW_USEDEFAULT   (0x80000000)
#define WS_BORDER       (0x800000L)
#define WS_CAPTION      (0xc00000L)
#define WS_CHILD        (0x40000000L)
#define WS_CHILDWINDOW  (0x40000000L)
#define WS_CLIPCHILDREN (0x2000000L)
#define WS_CLIPSIBLINGS (0x4000000L)
#define WS_DISABLED     (0x8000000L)
#define WS_DLGFRAME     (0x400000L)
#define WS_GROUP        (0x20000L)
#define WS_HSCROLL      (0x100000L)
#define WS_ICONIC       (0x20000000L)
#define WS_MAXIMIZE     (0x1000000L)
#define WS_MAXIMIZEBOX  (0x10000L)
#define WS_MINIMIZE     (0x20000000L)
#define WS_MINIMIZEBOX  (0x20000L)
#define WS_OVERLAPPED   (0L)
#define WS_OVERLAPPEDWINDOW     (0xcf0000L)
#define WS_POPUP        (0x80000000L)
#define WS_POPUPWINDOW  (0x80880000L)
#define WS_SIZEBOX      (0x40000L)
#define WS_SYSMENU      (0x80000L)
#define WS_TABSTOP      (0x10000L)
#define WS_THICKFRAME   (0x40000L)
#define WS_TILED        (0L)
#define WS_TILEDWINDOW  (0xcf0000L)
#define WS_VISIBLE      (0x10000000L)
#define WS_VSCROLL      (0x200000L)
#define MDIS_ALLCHILDSTYLES     (0x1)
#define BS_3STATE       (0x5L)
#define BS_AUTO3STATE   (0x6L)
#define BS_AUTOCHECKBOX (0x3L)
#define BS_AUTORADIOBUTTON      (0x9L)
#define BS_BITMAP       (0x80L)
#define BS_BOTTOM       (0x800L)
#define BS_CENTER       (0x300L)
#define BS_CHECKBOX     (0x2L)
#define BS_DEFPUSHBUTTON        (0x1L)
#define BS_GROUPBOX     (0x7L)
#define BS_ICON (0x40L)
#define BS_LEFT (0x100L)
#define BS_LEFTTEXT     (0x20L)
#define BS_MULTILINE    (0x2000L)
#define BS_NOTIFY       (0x4000L)
#define BS_OWNERDRAW    (0xbL)
#define BS_PUSHBOX      (0xcL)          /* FIXME!  What should this be?  */
#define BS_PUSHBUTTON   (0L)
#define BS_PUSHLIKE     (0x1000L)
#define BS_RADIOBUTTON  (0x4L)
#define BS_RIGHT        (0x200L)
#define BS_RIGHTBUTTON  (0x20L)
#define BS_TEXT (0L)
#define BS_TOP  (0x400L)
#define BS_USERBUTTON   (0x8L)
#define BS_VCENTER      (0xc00L)
#define CBS_AUTOHSCROLL (0x40L)
#define CBS_DISABLENOSCROLL     (0x800L)
#define CBS_DROPDOWN    (0x2L)
#define CBS_DROPDOWNLIST        (0x3L)
#define CBS_HASSTRINGS  (0x200L)
#define CBS_LOWERCASE   (0x4000L)
#define CBS_NOINTEGRALHEIGHT    (0x400L)
#define CBS_OEMCONVERT  (0x80L)
#define CBS_OWNERDRAWFIXED      (0x10L)
#define CBS_OWNERDRAWVARIABLE   (0x20L)
#define CBS_SIMPLE      (0x1L)
#define CBS_SORT        (0x100L)
#define CBS_UPPERCASE   (0x2000L)
#define ES_AUTOHSCROLL  (0x80L)
#define ES_AUTOVSCROLL  (0x40L)
#define ES_CENTER       (0x1L)
#define ES_LEFT (0L)
#define ES_LOWERCASE    (0x10L)
#define ES_MULTILINE    (0x4L)
#define ES_NOHIDESEL    (0x100L)
#define ES_NUMBER       (0x2000L)
#define ES_OEMCONVERT   (0x400L)
#define ES_PASSWORD     (0x20L)
#define ES_READONLY     (0x800L)
#define ES_RIGHT        (0x2L)
#define ES_UPPERCASE    (0x8L)
#define ES_WANTRETURN   (0x1000L)
#define LBS_DISABLENOSCROLL     (0x1000L)
#define LBS_EXTENDEDSEL (0x800L)
#define LBS_HASSTRINGS  (0x40L)
#define LBS_MULTICOLUMN (0x200L)
#define LBS_MULTIPLESEL (0x8L)
#define LBS_NODATA      (0x2000L)
#define LBS_NOINTEGRALHEIGHT    (0x100L)
#define LBS_NOREDRAW    (0x4L)
#define LBS_NOSEL       (0x4000L)
#define LBS_NOTIFY      (0x1L)
#define LBS_OWNERDRAWFIXED      (0x10L)
#define LBS_OWNERDRAWVARIABLE   (0x20L)
#define LBS_SORT        (0x2L)
#define LBS_STANDARD    (0xa00003L)
#define LBS_USETABSTOPS (0x80L)
#define LBS_WANTKEYBOARDINPUT   (0x400L)
#define SBS_BOTTOMALIGN (0x4L)
#define SBS_HORZ        (0L)
#define SBS_LEFTALIGN   (0x2L)
#define SBS_RIGHTALIGN  (0x4L)
#define SBS_SIZEBOX     (0x8L)
#define SBS_SIZEBOXBOTTOMRIGHTALIGN     (0x4L)
#define SBS_SIZEBOXTOPLEFTALIGN (0x2L)
#define SBS_SIZEGRIP    (0x10L)
#define SBS_TOPALIGN    (0x2L)
#define SBS_VERT        (0x1L)
#define SS_BITMAP       (0xeL)
#define SS_BLACKFRAME   (0x7L)
#define SS_BLACKRECT    (0x4L)
#define SS_CENTER       (0x1L)
#define SS_CENTERIMAGE  (0x200L)
#define SS_ENHMETAFILE  (0xfL)
#define SS_ETCHEDFRAME  (0x12L)
#define SS_ETCHEDHORZ   (0x10L)
#define SS_ETCHEDVERT   (0x11L)
#define SS_GRAYFRAME    (0x8L)
#define SS_GRAYRECT     (0x5L)
#define SS_ICON (0x3L)
#define SS_LEFT (0L)
#define SS_LEFTNOWORDWRAP       (0xcL)
#define SS_NOPREFIX     (0x80L)
#define SS_NOTIFY       (0x100L)
#define SS_OWNERDRAW    (0xdL)
#define SS_REALSIZEIMAGE        (0x800L)
#define SS_RIGHT        (0x2L)
#define SS_RIGHTJUST    (0x400L)
#define SS_SIMPLE       (0xbL)
#define SS_SUNKEN       (0x1000L)
#define SS_USERITEM     (0xaL)
#define SS_WHITEFRAME   (0x9L)
#define SS_WHITERECT    (0x6L)
#define DS_3DLOOK       (0x4L)
#define DS_ABSALIGN     (0x1L)
#define DS_CENTER       (0x800L)
#define DS_CENTERMOUSE  (0x1000L)
#define DS_CONTEXTHELP  (0x2000L)
#define DS_CONTROL      (0x400L)
#define DS_FIXEDSYS     (0x8L)
#define DS_LOCALEDIT    (0x20L)
#define DS_MODALFRAME   (0x80L)
#define DS_NOFAILCREATE (0x10L)
#define DS_NOIDLEMSG    (0x100L)
#define DS_SETFONT      (0x40L)
#define DS_SETFOREGROUND        (0x200L)
#define DS_SYSMODAL     (0x2L)

#define ESC_A (007)
#define ESC_B (010)
#define ESC_F (014)
#define ESC_N (012)
#define ESC_R (015)
#define ESC_T (011)
#define ESC_V (013)

typedef unsigned short CHARACTER;

typedef struct res_id
{
    int hasname;
    union
    {
        struct 
        {
            int len;
            CHARACTER *name;
        } n;
        int id;
    }
    v;
}

//-------------------------------------------------------------------------

RES_ID;

enum rc_type
{
    RT_CURSOR = 1, RT_BITMAP, RT_ICON, RT_MENU, RT_DIALOG, RT_STRING,
        RT_FONTDIR, RT_FONT, RT_ACCELERATOR, RT_RCDATA, RT_MESSAGETABLE,
        RT_GROUP_CURSOR, RT_GROUP_ICON = 14, RT_VERSION = 16, RT_DLGINCLUDE,
        RT_PLUGPLAY, RT_VXD, RT_ANICURSOR = 21, RT_ANIICON
};


struct accelerator
{
    struct accelerator *link;
    enum
    {
        ACC_VIRTKEY = 1, ACC_NOINVERT = 2, ACC_SHIFT = 4, ACC_CONTROL = 8,
            ACC_ALT = 16, ACC_LAST = 128
    } flags;
    int key;
    int id;
};

struct cursor
{
    int xhotspot;
    int yhotspot;
    long length;
    const unsigned char *data;
};

/* A group_cursor resource is a list of group_cursor structures.  */

struct group_cursor
{
    struct group_cursor *link;
    int width;
    int height;
    int planes;
    int bits;
    long bytes;
    int index;
};

struct dialog
{
    long style;
    long exstyle;
    int x;
    int y;
    int width;
    int height;
    struct res_id menu;
    struct res_id class ;
    CHARACTER *caption;
    int pointsize;
    CHARACTER *font;
    struct dialog_ex
    {
            long help;
            int weight;
            int italic;
            int charset;
    }
     *ex;
    struct dialog_control
    {
        struct dialog_control *link;
        enum
        {
            CTL_BUTTON = 0x80, CTL_EDIT, CTL_STATIC, CTL_LISTBOX, CTL_SCROLLBAR,
                CTL_COMBOBOX
        } id;
        long style;
        long exstyle;
        int x;
        int y;
        int width;
        int height;
        struct res_id class ;
        struct res_id text;
        struct rcdata_item *data;
        long help;
}

//-------------------------------------------------------------------------

 *controls;
};

struct fontdir
{
        struct fontdir *link;
        int index;
        long length;
        unsigned char *data;
};
struct group_icon
{
        struct group_icon *link;
        char width;
        char height;
        char colors;
        int planes;
        int bits;
        long bytes;
        int index;
};

/* A menu resource.  */

struct menu
{
        struct menuitem
        {
                struct menuitem *link;
                long type; /* for MENUEX */
                enum
                {
                        MI_GRAYED = 1, MI_INACTIVE = 2, MI_BITMAP = 4,
                            MI_CHECKED = 8, MI_POPUP = 16, MI_MENUBARBREAK = 32,
                            MI_MENUBREAK = 64, MI_ENDMENU = 128, MI_OWNERDRAW =
                            256, MI_SEPARATOR = 0x800, MI_HELP = 0x4000
                }
                flags; /* Not MENUEX */
                long state;
                int id;
                CHARACTER *text;
                struct menuitem *popup;
                long help;
            }
             *items;
            long help;
        };

        struct rcdata_item
        {
            struct rcdata_item *link;
            enum
            {
                RCDATA_WORD, RCDATA_DWORD, RCDATA_STRING, RCDATA_WSTRING,
                    RCDATA_BUFFER
            } type;
            union
            {
                int word;
                long dword;
                struct
                {
                    long length;
                    unsigned char *s;
                } string;
                struct
                {
                    long length;
                    CHARACTER *w;
                } wstring;
                struct
                {
                    long length;
                    unsigned char *data;
                } buffer;
            }
            u;
        };
        struct stringtable
        {
            struct 
            {
                int length;
                CHARACTER *string;
            } strings[16];
        };

        /* A versioninfo resource points to a versioninfo structure.  */

        struct versioninfo
        {
            struct fixed_versioninfo
            {
                long file_version_ms;
                long file_version_ls;
                long product_version_ms;
                long product_version_ls;
                long file_flags_mask;
                long file_flags;
                long file_os;
                long file_type;
                long file_subtype;
                long file_date_ms;
                long file_date_ls;
            }  *fixed;
            struct ver_info
            {
                struct ver_info *link;
                enum
                {
                    VERINFO_STRING, VERINFO_VAR
                } type;
                union
                {
                    struct
                    {
                        CHARACTER *language;
                        struct ver_stringinfo
                        {
                            struct ver_stringinfo *link;
                            CHARACTER *key;
                            CHARACTER *value;
                            int length;
                        }  *strings;
                    }
                    string;
                    struct
                    {
                        struct ver_varinfo
                        {
                            CHARACTER *key;
                            struct ver_varinfo *link;
                            struct ver_varlangchar
                            {
                                struct ver_varlangchar *link;
                                int language;
                                int charset;
                            }  *intident;
                        }
                         *var;
                    }
                    var;
                }
                u;
            }
             *var;
        };



        typedef struct res_info
        {
            int language;
            long characteristics;
            long version;
            enum
            {
                MF_MOVEABLE = 0x10, MF_PURE = 0x20, MF_PRELOAD = 0x40,
                    MF_DISCARDABLE = 0x1000
            } memflags;
        }
        RES_INFO;

        /* Resource data is stored in a res_resource structure.  */

        typedef struct res_res
        {
            /* The type of resource.  */
            enum res_type
            {
                RST_UNINITIALIZED, RST_ACCELERATOR, RST_BITMAP, RST_CURSOR,
                    RST_GROUP_CURSOR, RST_DIALOG, RST_DLGINCLUDE, RST_FONT,
                    RST_FONTDIR, RST_ICON, RST_GROUP_ICON, RST_MENU,
                    RST_MESSAGETABLE, RST_RCDATA, RST_STRINGTABLE, RST_USERDATA,
                    RST_VERSIONINFO
            } type;
            /* The data for the resource.  */
            union
            {
                struct
                {
                    long length;
                    const unsigned char *data;
                } data;
                struct accelerator *acc;
                struct cursor *cursor;
                struct group_cursor *group_cursor;
                struct dialog *dialog;
                struct fontdir *fontdir;
                struct group_icon *group_icon;
                struct menu *menu;
                struct rcdata_item *rcdata;
                struct stringtable *stringtable;
                struct rcdata_item *userdata;
                struct versioninfo *versioninfo;
            }
            u;
            struct res_info info;
        }
        RES_RES;

        typedef struct res_dir
        {
            struct res_dir *link;
            RES_ID type;
            RES_ID id;
            int language;
            RES_RES *res;
        } RES_DIR;

        typedef struct bindata
        {
            struct bindata *link;
            long length;
            unsigned char *data;
        } BINDATA;

        typedef struct strdef
        {
            struct strdef *link;
            char *string;
            long id;
            RES_INFO info;
        } STRDEF;

        #include "xrc.p"
