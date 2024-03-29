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

#define FINDDLG 10000
#define REPLACEDLG 10100

#define IDC_JUMPLIST 101

#define IDC_LVNEWPROJECT 100
#define IDC_FILENEWPROJECT 101
#define IDC_PROJECTNEWPROJECT 102
#define IDC_NEWPROJECTBROWSE 103

#define IDC_LVNEWFILE 100
#define IDC_FILENEWFILE 101

#define IDC_BUILDRULEEDIT	104
#define IDC_BUILDRULEREMOVE	103
#define IDC_BUILDRULEADD	102

#define ID_TBFIND 1697
#define ID_TBTHREADS 1698
#define ID_TBPROCEDURE 1699

#define IDC_COMBOFINDFIND       101
#define IDC_COMBOFINDTYPE       102
#define IDC_COMBOFINDWHERE      103
#define IDC_COMBOFINDPATH       104
#define IDC_COMBOFINDHOW        105
#define IDC_ICONFINDOPTIONS     106
#define IDC_CHECKFINDWHOLE      107
#define IDC_CHECKFINDCASE       108
#define IDC_CHECKFINDRECURSIVE  109
#define IDC_CHECKFINDUP         110
#define IDC_ICONFINDOUTPUT      111
#define IDC_RADIOFINDDOCUMENT   112
#define IDC_RADIOFIND1          113
#define IDC_RADIOFIND2          114
#define IDC_GROUPFINDOPTIONS	115
#define IDC_GROUPFINDOUTPUT		116
#define IDC_FINDNEXT			117
#define IDC_FINDBROWSE          118

#define IDC_COMBOREPLACEFIND    201
#define IDC_COMBOREPLACE        202
#define IDC_COMBOREPLACETYPE    203
#define IDC_COMBOREPLACEWHERE   204
#define IDC_COMBOREPLACEPATH    205
#define IDC_COMBOREPLACEHOW     206
#define IDC_ICONREPLACEOPTIONS  207
#define IDC_CHECKREPLACEWHOLE   208
#define IDC_CHECKREPLACECASE    209
#define IDC_CHECKREPLACERECURSIVE   210
#define IDC_GROUPREPLACEOPTIONS	211
#define IDC_REPLACEFINDNEXT 212
#define IDC_REPLACENEXT 213
#define IDC_REPLACEALL 214
#define IDC_REPLACEBROWSE 215

#define IDC_PDCOMBO	101
#define IDC_PDLIST	102
#define IDC_BOTTOMMARGIN	108
#define IDC_TOPMARGIN	107
#define IDC_RIGHTMARGIN	106
#define IDC_LEFTMARGIN 105
#define IDC_PRINTFOOTERBUTTON	104
#define IDC_PRINTHEADERBUTTON	103
#define IDC_PRINTFOOTER	102
#define IDC_PRINTHEADER	101

#define IDC_TOOLCUSTOM 101

#define IDC_RADIO_ALL_TEXT	102
#define IDC_RADIO_SELECTED_TEXT	103

#define IDC_EAI_DIRCOMBO 10000

#define IDC_PROGRESSBAR	101
#define IDC_PBNAME 120
#define IDC_PBPERCENT 121

#define IDC_TOLIBRARIAN	104
#define IDC_TOLINKER	103
#define IDC_TOASSEMBLER	102
#define IDC_TOCOMPILER	101

#define IDC_FILELIST 100
#define IDC_SELECTALL 101
#define IDC_DESELECTALL 102

#define IDC_BROWSETO 100

#define IDC_ADDDATABP 100
#define IDC_REMOVEDATABP 101
#define IDC_BPLIST 102
#define IDC_EDDATABP 103

#define IDC_EXT_NAME	102
#define IDC_EXT_COMMAND	103
#define IDC_EXT_ARG	104
#define IDC_EXT_WD	105
#define IDC_EXT_CW	106
#define IDC_BUTTON1	101

#define MAX_CHILDREN 100
#define MAX_MRU 5
#define MAX_WINMENU 10

#define EXTENSION_BASE 10000
#define WM_WINMENU (EXTENSION_BASE +100)
#define EM_GETINSERTSTATUS (EXTENSION_BASE + 101)
#define WM_REDRAWTOOLBAR (EXTENSION_BASE + 102)
#define IDC_CHECKBOX2	103
#define IDC_PARENCHECK	103
#define WM_RETRIEVECOLOR (EXTENSION_BASE + 104)
#define WM_SETEDITORSETTINGS (EXTENSION_BASE + 105)
#define WM_BREAKPOINT (EXTENSION_BASE + 106)
#define WM_EXCEPTION (EXTENSION_BASE + 107)
#define WM_GETCURSORADDRESS (EXTENSION_BASE +108)
#define WM_UNUSED (EXTENSION_BASE + 109)
#define WM_UNUSED1 (EXTENSION_BASE + 110)
#define WM_UNUSED2 (EXTENSION_BASE + 111) 
#define WM_WORDUNDERCURSOR (EXTENSION_BASE + 112)
#define WM_ADDWATCH (EXTENSION_BASE + 113)
#define WM_RESTACK (EXTENSION_BASE + 114)
#define WM_SELERRWINDOW (EXTENSION_BASE + 115)
#define WM_SETHISTORY (EXTENSION_BASE + 116)
#define WM_SAVEHISTORY (EXTENSION_BASE + 117)
#define WM_SETMODIFY (EXTENSION_BASE + 118)
#define WM_FILETITLE (EXTENSION_BASE + 119)
#define WM_FILENAME  (EXTENSION_BASE + 120)

//#define WM_ERRDONESCAN (EXTENSION_BASE +121) 

#define EM_LANGUAGE (EXTENSION_BASE+122)
#define EM_GETTEXTHEIGHT (EXTENSION_BASE + 123)
#define EN_LINECHANGE (EXTENSION_BASE + 124)
#define EM_GETCOLUMN (EXTENSION_BASE +125)
#define WM_RESETTABS (EXTENSION_BASE + 126)
#define WM_GETHEIGHT (EXTENSION_BASE + 127)
#define WM_SETACTIVETAB (EXTENSION_BASE + 128)
#define EM_GETREADONLY (EXTENSION_BASE + 129)
#define EM_TOUPPER (EXTENSION_BASE + 130)
#define EM_TOLOWER (EXTENSION_BASE + 131)
#define WM_ADDWATCHINDIRECT (EXTENSION_BASE+132)
#define EM_SELECTINDENT (EXTENSION_BASE + 133)
#define EN_SETCURSOR (EXTENSION_BASE + 134) 
#define EM_GETEDITDATA (EXTENSION_BASE + 135)
#define EM_UPDATESIBLING (EXTENSION_BASE + 136)
#define WM_SETCOLOR (EXTENSION_BASE + 137)
#define EM_CANREDO (EXTENSION_BASE + 138)
#define WM_REDO (EXTENSION_BASE + 139)
#define EM_SELECTCOMMENT (EXTENSION_BASE + 140)
#define EM_FOCUS (EXTENSION_BASE + 141)
#define EN_NEEDFOCUS (EXTENSION_BASE + 142)
#define WM_WORDUNDERPOINT (EXTENSION_BASE + 143)
#define WM_HIDEDEBUGWINDOWS (EXTENSION_BASE + 144)
#define WM_ACTIVATEME (EXTENSION_BASE + 145)
#define WM_MOUSEINPARENT (EXTENSION_BASE + 146)
#define WM_CODECOMPLETE (EXTENSION_BASE + 147)
#define WM_SHOWFUNCTION (EXTENSION_BASE + 148)
#define WM_TOOLBARDROPDOWN (EXTENSION_BASE + 149)
#define WM_TOOLBARDROPDOWN2 (EXTENSION_BASE + 150)
#define WM_SETLINENUMBERMODE (EXTENSION_BASE + 151)
#define WM_FOCUSFIND (EXTENSION_BASE + 152)
#define WM_ENABLEFIND (EXTENSION_BASE + 153)
#define WM_SETTEXT2 (EXTENSION_BASE + 154)
#define EM_GETSIZE (EXTENSION_BASE + 155)

#define IDC_GOTO 101
#define IDC_RETURN 102
#define IDC_ESCAPE 103

#define IDC_EDWATCH	101

#define IDC_OUTPUTPATH 101
#define IDC_CHECKBOX1	107
#define IDC_INCLUDEPATH 102
#define IDC_BUILDTYPE 103
#define IDC_MAPFILE 104
#define IDC_DEBUGINFO 105
#define IDC_COMPILEVIAASM 106
#define IDC_CHOSENPROJECT 107
#define IDC_SHOWWARNINGS	108
#define IDC_LIBTYPE	109
#define IDC_C99 110
#define IDC_ANSI 111
#define IDC_ALIGNSTACK 112

#define IDC_BMGOTO 101
#define IDC_BMLISTBOX 102

#define IDC_BRLISTBOX 100

#define IDC_EXTOOLCUSTOM 100
#define IDC_EXTOOLSADD	101
#define IDC_EXTOOLSEDIT	102
#define IDC_EXTOOLSREMOVE	103
#define IDC_EXTOOLSMOVEUP	104
#define IDC_EXTOOLSMOVEDOWN	105
#define IDC_BUTTON2	106

#define IDC_HELPPATH 106

#define IDC_BROWSEINFO 101
#define IDC_CODECOMP	102

#define IDC_BACKUPFILE 101
#define IDC_BACKUPPROJ 102
#define IDC_AUTOINDENT 103
#define IDC_AUTOFORMAT 104

#define ID_QUERYSAVE	100
#define ID_QUERYHASFILE 101
#define ID_STATUS_WINDOW 102
#define ID_MAIN_MENU 103

#define IDC_BPSSENABLE 100
#define IDC_BPEDIT1 101
#define IDC_BPMODE1 102
#define IDC_BPSIZE1 103
#define IDC_BPENABLE1 104
#define IDC_BPEDIT2 111
#define IDC_BPMODE2 112
#define IDC_BPSIZE2 113
#define IDC_BPENABLE2 114
#define IDC_BPEDIT3 121
#define IDC_BPMODE3 122
#define IDC_BPSIZE3 123
#define IDC_BPENABLE3 124
#define IDC_BPEDIT4 131
#define IDC_BPMODE4 132
#define IDC_BPSIZE4 133
#define IDC_BPENABLE4 134

#define IDM_OPEN	120
#define IDM_PRINT 121
#define IDM_CONTEXTHELP 122
#define IDM_FORWARD 123
#define IDM_BACK 124
#define IDM_DEFERREDOPEN 125

#define IDM_GENERALPROPERTIES 130
#define IDM_VIEWPROJECT 131
#define IDM_VIEWERROR   132
#define IDM_VIEWASM	133
#define IDM_TOOLCUSTOM 134
#define IDM_BUILDRULES 135
#define IDM_VIEWMEM    136
#define IDM_VIEWWATCH 137
#define IDM_VIEWSTACK 138
#define IDM_VIEWTHREAD 141
#define IDM_VIEWREGISTER 142
#define IDM_SAVEALL2 143
#define IDM_RTLHELP 144
#define IDM_SAVE	145
#define IDM_SAVEAS	146
#define IDM_CLOSE 	147
#define IDM_EXIT	148
#define IDM_ABOUT	149
#define IDM_SPECIFIEDHELP 150
#define IDM_TOOLSHELP 151
#define IDM_CCIDEHELP 152
#define IDM_SAVEALL 153

#define IDM_CLOSEALLWINDOWS 154
#define IDM_CASCADE  155
#define IDM_TILEHORIZ 156
#define IDM_TILEVERT 157
#define IDM_ARRANGE  158
#define IDM_CLOSEWINDOW 159


#define IDM_CUT	        160
#define IDM_COPY        161
#define IDM_PASTE       162
#define IDM_UNDO        163
#define IDM_SELECTALL   164
#define IDM_GOTO	165
#define IDM_SETLINE 166
#define IDM_NEWWINDOW 167
#define IDM_SPLITWINDOW 168
#define IDM_LANGUAGEHELP 169

#define IDM_FIND	170
#define IDM_FINDNEXT	171
#define IDM_REPLACE	172
#define IDM_FINDINFILES 173
#define IDM_INDENT 174
#define IDM_UNINDENT 175
#define IDM_TOUPPER 176
#define IDM_TOLOWER 177
#define IDM_COMMENT 178
#define IDM_UNCOMMENT 179

#define IDM_OPENWS 180
#define IDM_CLOSEWS 181
#define IDM_NEWWS 182
#define IDM_DOSWINDOW 183
#define IDM_MAKEWINDOW 184

#define IDM_OPENFILES 185
#define IDM_NEWPROJECT 186
#define IDM_NEWFILE_P 187
#define IDM_SETACTIVEPROJECT 188
#define IDM_PROJECTPROPERTIES 189
#define IDM_SAVEWS 190
#define IDM_EXISTINGPROJECT 191
#define IDM_REDO 192
#define IDM_NEWFILE 193
#define IDM_EXISTINGFILE 194
#define IDM_NEWFOLDER 195 
#define IDM_EXISTINGFOLDER 196
#define IDM_RENAME 197
#define IDM_REMOVE 198
#define IDM_ACTIVEPROJECTPROPERTIES 199

#define IDM_MAKE 200
#define IDM_BUILDALL 201
#define IDM_COMPILEFILE 202
#define IDM_BUILDSELECTED 203
#define IDM_GENMAKE 204
#define IDM_STOPBUILD 205
#define IDM_CALCULATEDEPENDS 206

#define IDM_RUN 210
#define IDM_RUNTO 211
#define IDM_STEPOVER 212
#define IDM_STEPOUT 213
#define IDM_STEPIN 214
#define IDM_BREAKPOINT 215
#define IDM_STOP	216
#define IDM_STOPDEBUGGING 218
#define IDM_RUNNODEBUG 219
#define IDM_ADDWATCH 220
#define IDM_ADDWATCHINDIRECT 221
#define IDM_DELETEWATCH 222
#define IDM_REMOVEALLBREAKPOINTS 223
#define IDM_ENABLEALLBREAKPOINTS 224
#define IDM_DISABLEALLBREAKPOINTS 225
#define IDM_HBREAK 226
#define IDM_DELETEALLWATCH 227
#define IDM_SCROLLTOBP 228
#define IDM_BROWSE 229
#define IDM_BROWSEBACK 230
#define IDM_BROWSETO 231
#define IDM_DATABREAKPOINT 232
#define IDM_DATABREAKPOINTINDIRECT 233

#define IDM_BOOKMARK 240
#define IDM_NEXTBOOKMARK 241
#define IDM_PREVBOOKMARK 242
#define IDM_NEXTBOOKMARKFILE 243
#define IDM_PREVBOOKMARKFILE 244
#define IDM_BOOKMARKWINDOW 245
#define IDM_REMOVEBOOKMARKS 246

#define IDM_IMPORT_CWS 250
#define IDM_IMPORT_CTG 251

#define ID_MRU_LIST  380
#define ID_MRU_PROJ_LIST 390

#define ID_REDRAWSTATUS 300

#define IDC_KEYWORDCOLOR 310
#define IDC_COMMENTCOLOR 311
#define IDC_NUMBERCOLOR 312
#define IDC_ESCAPECOLOR 313
#define IDC_STRINGCOLOR 314
#define IDC_TABSTOPS 315
#define IDC_TEXTCOLOR	316
#define IDC_BACKGROUNDCOLOR	317
#define IDC_TABSASSPACES 318
#define IDC_SELECTFONT 319
#define IDC_HIGHLIGHTCOLOR 320

#define IDC_DEBUGEXCEPTION 340
#define IDC_BREAKDLL 341
#define IDC_DEBUGTOOLTIPS 343 
#define IDC_EDITCMDLINE 344
#define IDC_SHOWRETURNCODE 345

#define IDC_PREBUILDLABEL 350
#define IDC_PREBUILDSTEPS 351
#define IDC_POSTBUILDLABEL 352
#define IDC_POSTBUILDSTEPS 353

#define IDC_DEFINENAME 360
#define IDC_DEFINEVALUE 361
#define IDC_DEFINELIST 362
#define IDC_DEFINEADD 363
#define IDC_DEFINEREMOVE 364
#define IDC_DEFINEPASSTONASM 365
#define IDC_DFADD 366

#define IDM_PF12HR 400
#define IDM_PF24HR 401
#define IDM_PFUSADATE 402
#define IDM_PFEURODATE 403
#define IDM_PFCURPAGE 404
#define IDM_PFNUMPAGE 406
#define IDM_PFFILENAME 407
#define IDM_PFCENTER 420
#define IDM_PFLEFT 421
#define IDM_PFRIGHT 422

#define ID_SETADDRESS 900
#define ID_SETCONTEXT 901
#define ID_CHECKCHANGE 902

#define IDM_BYTE 910
#define IDM_WORD 911
#define IDM_DWORD 912
#define IDM_EAX 913
#define IDM_EBX 914
#define IDM_ECX 915
#define IDM_EDX 916
#define IDM_ESP 917
#define IDM_EBP 918
#define IDM_ESI 919
#define IDM_EDI 920

#define IDM_FIRSTCHILD 1000
#define ID_EDITCHILD 1000

#define ID_TREEVIEW 2000

#define ID_EDITTB 3000
#define ID_BUILDTB 3001
#define ID_DEBUGTB 3002
#define ID_NAVTB 3003
#define ID_BOOKMARKTB 3004
#define ID_EXTRA 3005
#define ID_WINDOW_LIST 5000
#define ID_TOOLBARCUSTOM 5000
#define IDM_WINDOW_MORE (ID_WINDOW_LIST + MAX_WINMENU)


#define ID_FILEBROWSE_LIST 6000

#define IDM_EDITEXTERNALTOOLS 6999
#define ID_EXTERNALTOOLS 7000
#define MAX_EXTERNALTOOLS 20

#define WindowMenuItem 8
#define ToolsMenuItem 7
