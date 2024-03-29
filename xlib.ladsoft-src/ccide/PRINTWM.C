#include <stdio.h>
#include <windows.h>
typedef struct hi
{
    char *name;
    int id;
} WMSG;

static WMSG array[] = 
{
    {
        "WM_ACTIVATE", 6
    }
    , 
    {
        "WM_ACTIVATEAPP", 28
    }
    , 
    {
        "WM_ASKCBFORMATNAME", 780
    }
    , 
    {
        "WM_CANCELJOURNAL", 75
    }
    , 
    {
        "WM_CANCELMODE", 31
    }
    , 
    {
        "WM_CAPTURECHANGED", 533
    }
    , 
    {
        "WM_CHANGECBCHAIN", 781
    }
    , 
    {
        "WM_CHAR", 258
    }
    , 
    {
        "WM_CHARTOITEM", 47
    }
    , 
    {
        "WM_CHILDACTIVATE", 34
    }
    , 
    {
        "WM_CHOOSEFONT_GETLOGFONT", 1025
    }
    , 
    {
        "WM_CHOOSEFONT_SETLOGFONT", 1125
    }
    , 
    {
        "WM_CHOOSEFONT_SETFLAGS", 1126
    }
    , 
    {
        "WM_CLEAR", 771
    }
    , 
    {
        "WM_CLOSE", 16
    }
    , 
    {
        "WM_COMMAND", 273
    }
    , 
    {
        "WM_COMPACTING", 65
    }
    , 
    {
        "WM_COMPAREITEM", 57
    }
    , 
    {
        "WM_CONTEXTMENU", 123
    }
    , 
    {
        "WM_COPY", 769
    }
    , 
    {
        "WM_COPYDATA", 74
    }
    , 
    {
        "WM_CREATE", 1
    }
    , 
    {
        "WM_CTLCOLORBTN", 309
    }
    , 
    {
        "WM_CTLCOLORDLG", 310
    }
    , 
    {
        "WM_CTLCOLOREDIT", 307
    }
    , 
    {
        "WM_CTLCOLORLISTBOX", 308
    }
    , 
    {
        "WM_CTLCOLORMSGBOX", 306
    }
    , 
    {
        "WM_CTLCOLORSCROLLBAR", 311
    }
    , 
    {
        "WM_CTLCOLORSTATIC", 312
    }
    , 
    {
        "WM_CUT", 768
    }
    , 
    {
        "WM_DEADCHAR", 259
    }
    , 
    {
        "WM_DELETEITEM", 45
    }
    , 
    {
        "WM_DESTROY", 2
    }
    , 
    {
        "WM_DESTROYCLIPBOARD", 775
    }
    , 
    {
        "WM_DEVICECHANGE", 537
    }
    , 
    {
        "WM_DEVMODECHANGE", 27
    }
    , 
    {
        "WM_DISPLAYCHANGE", 126
    }
    , 
    {
        "WM_DRAWCLIPBOARD", 776
    }
    , 
    {
        "WM_DRAWITEM", 43
    }
    , 
    {
        "WM_DROPFILES", 563
    }
    , 
    {
        "WM_ENABLE", 10
    }
    , 
    {
        "WM_ENDSESSION", 22
    }
    , 
    {
        "WM_ENTERIDLE", 289
    }
    , 
    {
        "WM_ENTERMENULOOP", 529
    }
    , 
    {
        "WM_ENTERSIZEMOVE", 561
    }
    , 
    {
        "WM_ERASEBKGND", 20
    }
    , 
    {
        "WM_EXITMENULOOP", 530
    }
    , 
    {
        "WM_EXITSIZEMOVE", 562
    }
    , 
    {
        "WM_FONTCHANGE", 29
    }
    , 
    {
        "WM_GETDLGCODE", 135
    }
    , 
    {
        "WM_GETFONT", 49
    }
    , 
    {
        "WM_GETHOTKEY", 51
    }
    , 
    {
        "WM_GETICON", 127
    }
    , 
    {
        "WM_GETMINMAXINFO", 36
    }
    , 
    {
        "WM_GETTEXT", 13
    }
    , 
    {
        "WM_GETTEXTLENGTH", 14
    }
    , 
    {
        "WM_HELP", 83
    }
    , 
    {
        "WM_HOTKEY", 786
    }
    , 
    {
        "WM_HSCROLL", 276
    }
    , 
    {
        "WM_HSCROLLCLIPBOARD", 782
    }
    , 
    {
        "WM_ICONERASEBKGND", 39
    }
    , 
    {
        "WM_IME_CHAR", 646
    }
    , 
    {
        "WM_IME_COMPOSITION", 271
    }
    , 
    {
        "WM_IME_COMPOSITIONFULL", 644
    }
    , 
    {
        "WM_IME_CONTROL", 643
    }
    , 
    {
        "WM_IME_ENDCOMPOSITION", 270
    }
    , 
    {
        "WM_IME_KEYDOWN", 656
    }
    , 
    {
        "WM_IME_KEYUP", 657
    }
    , 
    {
        "WM_IME_NOTIFY", 642
    }
    , 
    {
        "WM_IME_SELECT", 645
    }
    , 
    {
        "WM_IME_SETCONTEXT", 641
    }
    , 
    {
        "WM_IME_STARTCOMPOSITION", 269
    }
    , 
    {
        "WM_INITDIALOG", 272
    }
    , 
    {
        "WM_INITMENU", 278
    }
    , 
    {
        "WM_INITMENUPOPUP", 279
    }
    , 
    {
        "WM_INPUTLANGCHANGE", 81
    }
    , 
    {
        "WM_INPUTLANGCHANGEREQUEST", 80
    }
    , 
    {
        "WM_KEYDOWN", 256
    }
    , 
    {
        "WM_KEYUP", 257
    }
    , 
    {
        "WM_KILLFOCUS", 8
    }
    , 
    {
        "WM_LBUTTONDBLCLK", 515
    }
    , 
    {
        "WM_LBUTTONDOWN", 513
    }
    , 
    {
        "WM_LBUTTONUP", 514
    }
    , 
    {
        "WM_MBUTTONDBLCLK", 521
    }
    , 
    {
        "WM_MBUTTONDOWN", 519
    }
    , 
    {
        "WM_MBUTTONUP", 520
    }
    , 
    {
        "WM_MDIACTIVATE", 546
    }
    , 
    {
        "WM_MDICASCADE", 551
    }
    , 
    {
        "WM_MDICREATE", 544
    }
    , 
    {
        "WM_MDIDESTROY", 545
    }
    , 
    {
        "WM_MDIGETACTIVE", 553
    }
    , 
    {
        "WM_MDIICONARRANGE", 552
    }
    , 
    {
        "WM_MDIMAXIMIZE", 549
    }
    , 
    {
        "WM_MDINEXT", 548
    }
    , 
    {
        "WM_MDIREFRESHMENU", 564
    }
    , 
    {
        "WM_MDIRESTORE", 547
    }
    , 
    {
        "WM_MDISETMENU", 560
    }
    , 
    {
        "WM_MDITILE", 550
    }
    , 
    {
        "WM_MEASUREITEM", 44
    }
    , 
    {
        "WM_MENUCHAR", 288
    }
    , 
    {
        "WM_MENUSELECT", 287
    }
    , 
    {
        "WM_MOUSEACTIVATE", 33
    }
    , 
    {
        "WM_MOUSEMOVE", 512
    }
    , 
    {
        "WM_MOVE", 3
    }
    , 
    {
        "WM_MOVING", 534
    }
    , 
    {
        "WM_NCACTIVATE", 134
    }
    , 
    {
        "WM_NCCALCSIZE", 131
    }
    , 
    {
        "WM_NCCREATE", 129
    }
    , 
    {
        "WM_NCDESTROY", 130
    }
    , 
    {
        "WM_NCHITTEST", 132
    }
    , 
    {
        "WM_NCLBUTTONDBLCLK", 163
    }
    , 
    {
        "WM_NCLBUTTONDOWN", 161
    }
    , 
    {
        "WM_NCLBUTTONUP", 162
    }
    , 
    {
        "WM_NCMBUTTONDBLCLK", 169
    }
    , 
    {
        "WM_NCMBUTTONDOWN", 167
    }
    , 
    {
        "WM_NCMBUTTONUP", 168
    }
    , 
    {
        "WM_NCMOUSEMOVE", 160
    }
    , 
    {
        "WM_NCPAINT", 133
    }
    , 
    {
        "WM_NCRBUTTONDBLCLK", 166
    }
    , 
    {
        "WM_NCRBUTTONDOWN", 164
    }
    , 
    {
        "WM_NCRBUTTONUP", 165
    }
    , 
    {
        "WM_NEXTDLGCTL", 40
    }
    , 
    {
        "WM_NOTIFY", 78
    }
    , 
    {
        "WM_NOTIFYFORMAT", 85
    }
    , 
    {
        "WM_NULL", 0
    }
    , 
    {
        "WM_PAINT", 15
    }
    , 
    {
        "WM_PAINTCLIPBOARD", 777
    }
    , 
    {
        "WM_PAINTICON", 38
    }
    , 
    {
        "WM_PALETTECHANGED", 785
    }
    , 
    {
        "WM_PALETTEISCHANGING", 784
    }
    , 
    {
        "WM_PARENTNOTIFY", 528
    }
    , 
    {
        "WM_PASTE", 770
    }
    , 
    {
        "WM_PENWINFIRST", 896
    }
    , 
    {
        "WM_PENWINLAST", 911
    }
    , 
    {
        "WM_POWER", 72
    }
    , 
    {
        "WM_POWERBROADCAST", 536
    }
    , 
    {
        "WM_PRINT", 791
    }
    , 
    {
        "WM_PRINTCLIENT", 792
    }
    , 
    {
        "WM_PSD_ENVSTAMPRECT", 1029
    }
    , 
    {
        "WM_PSD_FULLPAGERECT", 1025
    }
    , 
    {
        "WM_PSD_GREEKTEXTRECT", 1028
    }
    , 
    {
        "WM_PSD_MARGINRECT", 1027
    }
    , 
    {
        "WM_PSD_MINMARGINRECT", 1026
    }
    , 
    {
        "WM_PSD_PAGESETUPDLG", 1024
    }
    , 
    {
        "WM_PSD_YAFULLPAGERECT", 1030
    }
    , 
    {
        "WM_QUERYDRAGICON", 55
    }
    , 
    {
        "WM_QUERYENDSESSION", 17
    }
    , 
    {
        "WM_QUERYNEWPALETTE", 783
    }
    , 
    {
        "WM_QUERYOPEN", 19
    }
    , 
    {
        "WM_QUEUESYNC", 35
    }
    , 
    {
        "WM_QUIT", 18
    }
    , 
    {
        "WM_RBUTTONDBLCLK", 518
    }
    , 
    {
        "WM_RBUTTONDOWN", 516
    }
    , 
    {
        "WM_RBUTTONUP", 517
    }
    , 
    {
        "WM_RENDERALLFORMATS", 774
    }
    , 
    {
        "WM_RENDERFORMAT", 773
    }
    , 
    {
        "WM_SETCURSOR", 32
    }
    , 
    {
        "WM_SETFOCUS", 7
    }
    , 
    {
        "WM_SETFONT", 48
    }
    , 
    {
        "WM_SETHOTKEY", 50
    }
    , 
    {
        "WM_SETICON", 128
    }
    , 
    {
        "WM_SETREDRAW", 11
    }
    , 
    {
        "WM_SETTEXT", 12
    }
    , 
    {
        "WM_SETTINGCHANGE", 26
    }
    , 
    {
        "WM_SHOWWINDOW", 24
    }
    , 
    {
        "WM_SIZE", 5
    }
    , 
    {
        "WM_SIZECLIPBOARD", 779
    }
    , 
    {
        "WM_SIZING", 532
    }
    , 
    {
        "WM_SPOOLERSTATUS", 42
    }
    , 
    {
        "WM_STYLECHANGED", 125
    }
    , 
    {
        "WM_STYLECHANGING", 124
    }
    , 
    {
        "WM_SYSCHAR", 262
    }
    , 
    {
        "WM_SYSCOLORCHANGE", 21
    }
    , 
    {
        "WM_SYSCOMMAND", 274
    }
    , 
    {
        "WM_SYSDEADCHAR", 263
    }
    , 
    {
        "WM_SYSKEYDOWN", 260
    }
    , 
    {
        "WM_SYSKEYUP", 261
    }
    , 
    {
        "WM_TCARD", 82
    }
    , 
    {
        "WM_TIMECHANGE", 30
    }
    , 
    {
        "WM_TIMER", 275
    }
    , 
    {
        "WM_UNDO", 772
    }
    , 
    {
        "WM_USER", 1024
    }
    , 
    {
        "WM_USERCHANGED", 84
    }
    , 
    {
        "WM_VKEYTOITEM", 46
    }
    , 
    {
        "WM_VSCROLL", 277
    }
    , 
    {
        "WM_VSCROLLCLIPBOARD", 778
    }
    , 
    {
        "WM_WINDOWPOSCHANGED", 71
    }
    , 
    {
        "WM_WINDOWPOSCHANGING", 70
    }
    , 
    {
        "WM_WININICHANGE", 26
    }
    , 
};

static FILE *out;

void printwmsg(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    int i;
    char buf[256];
    char buf2[256];
	memset(buf,0, sizeof(buf));
	memset(buf2, 0, sizeof(buf2));
    SendMessage(hwnd, WM_GETTEXT, 256, (LPARAM)buf);
    GetClassName(hwnd, buf2, 256);
    if (!out)
        out = fopen("wlog.log", "w");
    if (!out)
        return ;
    fprintf(out,"%s:", buf2);
    for (i = 0; i < sizeof(array) / sizeof(WMSG); i++)
    if (array[i].id == message)
    {
        fprintf(out, "%s:", array[i].name);
	break;
    }
	if (i >= sizeof(array)/sizeof(WMSG))
		fprintf(out, "UNKNOWN:");
    fprintf(out, "%d:%d:%d\n", message, wParam, lParam);
    fflush(out);
}
