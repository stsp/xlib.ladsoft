#include "cl.h"

#define VERSION "4.9"

static char usage[] = /* Usage */ \
"MK386 Version " VERSION " (c) Kirill Joss. Compile '" __DATE__ " " __TIME__ "'\n"
"Usage: MK386.EXE [options] files\n"
"( default parameters if first )     @fname    specify response file\n"
"  /A[-|+]   disable extensions      /?,/h,/H  this help\n"
"  /Dxxx     define something        /Ennn     max number of errors\n"
"  /Ipath    specify include path    /Lpath    specify .LIB & .OBJ dir\n"
"  /Oxxx     optimize parameters     /Cxxx     codegen parameters\n"
"  /K[-|+]   keep response file      /k[-|+]   keep generate files\n"
"  /e[+|-]   dump errors to file     /m[-|+]   generate .MAP file\n"
"  /l[-|+]   dump listing file       /i[-|+]   dump preprocessed file\n"
"  /w-xxx    disable a warning       /n[-|+]   no default .LIB & .OBJ\n"
"  /e=name   set name of .EXE file   /v[-|+]   debug info\n"
"  /M[+|-]   run MAKE                /B[-|+]   build all\n"
"\n"
"  /$C=name  alternate C compiler    CC386\n"
"  /$A=name  alternate assembler     NASM, TASM, TASMX, TASM32, MASM, ML, WASM,\n"
"                                    LASM\n"
"  /$L=name  alternate linker        VALX, TLINK, LINK, WLINK\n"
"  /$D=name  alternate DOS extender  PMODE, PMODEW, DOS4G, WDOSX, DOS32A, ZRDX,\n"
"                                    CAUSEWAY\n"
"  /$M=name  alternate MAKE          IMAKE, MAKE, WMAKE\n";


void help(void) { puts( usage); exit(0); }

int options = _OPT_COMPILE | _OPT_ASSEMBLE | _OPT_LINK |
				_OPT_ERRFILE | _OPT_EXTEND | _OPT_RUNMAKE;

int compiler  = COMPILER_CC386;
int assembler = ASSEMBLER_NASM;
int linker    = LINKER_VALX;
int maker     = MAKER_IMAKE;
int dosx      = DOSX_PMODE;

char exename[ _MAX_PATH ] = "";

static int setopt( char *param, int opt ) {
	if ( param[2] == '-' && param[3] == '\0' )
		options &= ~opt;
	else if ( ( param[2] == '+' && param[3] == '\0' ) || param[2] == '\0' )
		options |= opt;
	else
		return 0;
	return 1;
}

static void add_path4inc( char *path );
static void add_path4lib( char *path );
static void add_path4obj( char *path );

static int compiler_ok( char *s );
static int assembler_ok( char *s );
static int linker_ok( char *s );
static int maker_ok( char *s );
static int dosx_ok( char *s );

// #define arg_yes(a)  ( (a)[2] == '\0'|| ( (a)[2] == '+' && (a)[3] == '\0' ) )
// #define arg_no(a)   ( (a)[2] == '-' && (a)[3] == '\0' )

void get_param( char *param ) {
	char _path[ _MAX_PATH ];
	int len;

	switch( param[ 1 ] ) {

		case 'e' : // dump errors to file
			if ( !setopt( param, _OPT_ERRFILE ) )
				if ( param[2] == '=' || param[2] == '#' )
					strupr( strncpy( exename, param + 3, _MAX_PATH ) );
				else
					break;
			return;

		case 'i' : // dump preprocessed file
			if ( setopt( param, _OPT_PPRFILE ) )
				return;
			break;

		case 'l' : // dump listing file
			if ( setopt( param, _OPT_LSTFILE ) )
				return;
			break;

		case 'A' : // disable extensions
			if ( setopt( param, _OPT_EXTEND ) )
				return;
			break;

		case 'm' : // gen map file
			if ( setopt( param, _OPT_MAPFILE ) )
				return;
			break;

		case 'n' : // no default lib
			if ( setopt( param, _OPT_NODEFLIB ) )
				return;
			break;

		case 'v' : // debug info
			if ( setopt( param, _OPT_DEBUG ) )
				return;
			break;

		case 'K' : // keep response files
			if ( setopt( param, _OPT_KEEPRSP ) )
				return;
			break;

		case 'k' : // keep generate files
			if ( setopt( param, _OPT_KEEPGEN ) )
				return;
			break;

		case 'B' : // building all
			if ( setopt( param, _OPT_BUILDALL ) )
				return;
			break;

		case 'M' : // run make, else ONLY generate MAKEFILE.
			if ( setopt( param, _OPT_RUNMAKE ) )
				return;
/*			break;

		case 'a' :
			options &= ~( _OPT_COMPILE | _OPT_ASSEMBLE | _OPT_LINK );
			if ( setopt( param, _OPT_COMPILE ) )
				return;
			break;

		case 'c' :
			options &= ~( _OPT_COMPILE | _OPT_ASSEMBLE | _OPT_LINK );
			if ( setopt( param, _OPT_ASSEMBLE | _OPT_COMPILE ) )
				return;
			break;
*/
		case '?' :
		case 'h' :
		case 'H' :
			help(); /* Not return ! */

		case 'D' : // define
			add_defined( param + 2 );
			return;

		case 'I' : // include path
			add_path4inc( param + 2 );
			return;

		case 'L' : // lib path
			add_path4lib( param + 2 );
			return;
/*
		case 'o' : // output obj path
			add_path4obj( param + 2 );
			return;
*/
		case 'w' :
		case 'C' :
		case 'E' :
		case 'O' :
			add_option( param );
			return;

		case '$' :
			switch ( toupper( param[2] ) ) {
				case 'C' :
					if ( compiler_ok( param + 3 ) )
						return;
					break;
				case 'A' :
					if ( assembler_ok( param + 3 ) )
						return;
					break;
				case 'L' :
					if ( linker_ok( param + 3 ) )
						return;
					break;
			  case 'M' :
					if ( maker_ok( param + 3 ) )
						return;
					break;
			  case 'D' :
					if ( dosx_ok( param + 3 ) )
						return;
					break;
			}
			break;
	}
	fprintf( stderr, "Warning! Parameter error : %s\n", param );
}

static char *del_last_slash( char *s ) {
	int len = strlen( s );

	if ( len > 0 && s[ len-1 ] == '\\' )
		s[ len-1] = '\0';
	return s;
}

void add_path4inc( char *path ) {
    int len = strlen( path );
    char *p, *q;
    char _path[ _MAX_PATH ];

    strncpy( _path, path, _MAX_PATH );
	for( p = _path;;) {
        q = strchr( p, ';' );
        if ( q != NULL ) {
            *q = '\0';
            add_path_inc( del_last_slash( p ) );
            p = q + 1;
            if( p - _path >= len )
                break;
        }
		else {
            add_path_inc( del_last_slash( p ) );
            break;
        }
    }
}

void add_path4lib( char *path ) {
    int len = strlen(path);
    char *p, *q;
    char _path[ _MAX_PATH ];

	strncpy( _path, path, _MAX_PATH );
	for( p = _path;;) {
		q = strchr( p, ';' );
		if ( q != NULL ) {
			*q = '\0';
			add_path_lib( del_last_slash( p ) );
			p = q + 1;
			if( p - _path >= len )
				break;
		}
		else {
			add_path_lib( del_last_slash( p ) );
			break;
		}
	}
}

/*
void add_path4obj( char *path ) {
	strupr( strncpy( objpath, del_last_slash( path ), _MAX_PATH ) );
}
*/

int compiler_ok( char *s ) {
	if ( s[0] == '=' || s[0] == '#' ) {
		if ( stricmp( s+1, "CC386" ) == 0 ) {
			compiler = COMPILER_CC386;
			return 1;
		}
	}
	return 0;
}

int assembler_ok( char *s ) {
	if ( s[0] == '=' || s[0] == '#' ) {
		if ( stricmp( s+1, "TASM" ) == 0 ) {
			assembler = ASSEMBLER_TASM;
			return 1;
		}
        if ( stricmp( s+1, "TASMX" ) == 0 ) {
            assembler = ASSEMBLER_TASMX;
			return 1;
        }
        if ( stricmp( s+1, "TASM32" ) == 0 ) {
            assembler = ASSEMBLER_TASM32;
			return 1;
        }
        if ( stricmp( s+1, "MASM" ) == 0 ) {
			assembler = ASSEMBLER_MASM;
			return 1;
		}
		if ( stricmp( s+1, "NASM" ) == 0 ) {
			assembler = ASSEMBLER_NASM;
			return 1;
        }
        if ( stricmp( s+1, "ML" ) == 0 ) {
            assembler = ASSEMBLER_ML6X;
            return 1;
        }
        if ( stricmp( s+1, "LASM" ) == 0 ) {
            assembler = ASSEMBLER_LASM;
            return 1;
        }
        if ( stricmp( s+1, "WASM" ) == 0 ) {
            assembler = ASSEMBLER_WASM;
            return 1;
        }
	}
	return 0;
}

int linker_ok( char *s ) {
	if ( s[0] == '=' || s[0] == '#' ) {
        if ( stricmp( s+1, "VALX" ) == 0 ) {
            linker = LINKER_VALX;
            return 1;
        }
        if ( stricmp( s+1, "TLINK" ) == 0 ) {
			linker = LINKER_TLINK;
			return 1;
		}
        if ( stricmp( s+1, "LINK" ) == 0 ) {
            linker = LINKER_LINK;
            return 1;
        }
        if ( stricmp( s+1, "WLINK" ) == 0 ) {
			linker = LINKER_WLINK;
			return 1;
		}
	}
	return 0;
}

int maker_ok( char *s ) {
	if ( s[0] == '=' || s[0] == '#' ) {
        if ( stricmp( s+1, "IMAKE" ) == 0 ) {
            maker = MAKER_IMAKE;
            return 1;
        }
        if ( stricmp( s+1, "MAKE" ) == 0 ) {
			maker = MAKER_MAKE;
			return 1;
		}
		if ( stricmp( s+1, "WMAKE" ) == 0 ) {
			maker = MAKER_WMAKE;
			return 1;
		}
	}
	return 0;
}

int dosx_ok( char *s ) {
	if ( s[0] == '=' || s[0] == '#' ) {
		if ( stricmp( s+1, "PMODE" ) == 0 ) {
			dosx = DOSX_PMODE;
			return 1;
		}
		if ( stricmp( s+1, "PMODEW" ) == 0 ) {
			dosx = DOSX_PMODEW;
			return 1;
		}
		if ( stricmp( s+1, "DOS4G" ) == 0 ) {
			dosx = DOSX_DOS4G;
			return 1;
		}
		if ( stricmp( s+1, "WDOSX" ) == 0 ) {
			dosx = DOSX_WDOSX;
			return 1;
		}
        if ( stricmp( s+1, "DOS32A" ) == 0 ) {
            dosx = DOSX_DOS32A;
			return 1;
        }
        if ( stricmp( s+1, "ZRDX" ) == 0 ) {
            dosx = DOSX_ZRDX;
			return 1;
        }
        if ( stricmp( s+1, "CAUSEWAY" ) == 0 ) {
            dosx = DOSX_CAUSEWAY;
			return 1;
        }
    }
	return 0;
}
