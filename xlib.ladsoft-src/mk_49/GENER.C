#include "cl.h"

#define MAKEFILE  "MAKEFILE."

static FILE *frsp;

static void open_rsp(void) {
    frsp = fopen( MAKEFILE, "wt" );
    if ( frsp == NULL ) {
        fprintf( stderr, "Error open file : '%s'\n", MAKEFILE );
        exit(-1);
    }
}

static void close_rsp( void ) {
    fclose( frsp );
}

static void exec( char *prg, char *arg ) {
    int err;
    static char *args[3];

    args[0] = prg;
    args[1] = arg;
    args[2] = NULL;

    err = spawnvp( P_OVERLAY, prg, args );

    if ( err > 0 ) {
        fprintf( stderr, "'%s' exit code = %Xh\n", prg, err );
        exit(err);
    }
    else if ( err < 0 ) {
        perror( prg );
        exit( -1 );
    }
}


static void make_cpp(char *data) {
    fprintf( frsp, "### %s\n", data );
    fprintf( frsp, "%s : %s\n", file2curdir( data, ".OBJ" ), data );
    fprintf( frsp, "\t $(CC) $(CC_OPT) %s\n", data );
    fprintf( frsp, "\t $(AS) $(AS_OPT)" );

    switch ( assembler ) {

        case ASSEMBLER_TASM:
        case ASSEMBLER_TASMX:
        case ASSEMBLER_TASM32:
            if ( options & _OPT_LSTFILE )
                fprintf( frsp, " /la" );
            fprintf( frsp, " %s,", file2curdir( data, ".ASM" ) );
            fprintf( frsp, " %s,", file2curdir( data, ".OBJ" ) );
            if ( options & _OPT_LSTFILE )
                fprintf( frsp, " %s,NUL", file2curdir( data, ".LST" ) );
            else
                fprintf( frsp, " NUL,NUL", file2curdir( data, ".LST" ) );
            break;

        case ASSEMBLER_NASM:
            if ( options & _OPT_LSTFILE )
                fprintf( frsp, " -l %s", file2curdir( data, ".LST" ) );
            fprintf( frsp, " %s", file2curdir( data, ".ASM" ) );
            break;

        case ASSEMBLER_MASM:
            if ( options & _OPT_LSTFILE )
                fprintf( frsp, " /la" );
            fprintf( frsp, " %s,", file2curdir( data, ".ASM" ) );
            fprintf( frsp, " %s,", file2curdir( data, ".OBJ" ) );
            if ( options & _OPT_LSTFILE )
                fprintf( frsp, " %s,NUL", file2curdir( data, ".LST" ) );
            else
                fprintf( frsp, " NUL,NUL", file2curdir( data, ".LST" ) );
            break;

        case ASSEMBLER_ML6X:
            if ( options & _OPT_LSTFILE )
                fprintf( frsp, " /Fl" );
            fprintf( frsp, " %s", file2curdir( data, ".ASM" ) );
            break;

        case ASSEMBLER_LASM:
            if ( options & _OPT_LSTFILE )
                fprintf( frsp, " /L" );
            fprintf( frsp, " %s", file2curdir( data, ".ASM" ) );
            break;

        case ASSEMBLER_WASM:
            if ( options & _OPT_LSTFILE )
                fprintf( frsp, " -fe" );
            fprintf( frsp, " %s", file2curdir( data, ".ASM" ) );
            break;
    }

    fprintf( frsp, "\n\n" );
}

static void make_asm( char *data, int temp ) {
    if ( temp )
        return;
    fprintf( frsp, "### %s\n", data );
    fprintf( frsp, "%s : %s\n", file2curdir( data, ".OBJ" ), data );
    fprintf( frsp, "\t $(AS) $(AS_OPT)" );
    if ( assembler == ASSEMBLER_NASM ) {
        if ( options & _OPT_LSTFILE )
            fprintf( frsp, " -l" );
        fprintf( frsp, " -o %s", file2curdir( data, ".OBJ" ) );
        fprintf( frsp, " %s", file2curdir( data, ".ASM" ) );
    }
    else {
        if ( options & _OPT_LSTFILE )
            fprintf( frsp, " /la" );
        fprintf( frsp, " %s,", file2curdir( data, ".ASM" ) );
        fprintf( frsp, " %s,", file2curdir( data, ".OBJ" ) );
        if ( options & _OPT_LSTFILE )
            fprintf( frsp, " %s,NUL", file2curdir( data, ".LST" ) );
        else
            fprintf( frsp, " NUL,NUL", file2curdir( data, ".LST" ) );
    }
    fprintf( frsp, "\n\n" );
}

static int first = 0;
static void make_path(char *data) { fprintf( frsp, "%c%s", first ? ' ' : ';', data ); first = 0; }
static void make_list(char *data) { fprintf( frsp, "%s \\\n", data ); }

static void compiler_opt( char *opt ) { fprintf( frsp,   " %s", opt ); }
static void compiler_def( char *opt ) { fprintf( frsp, " -D%s", opt ); }

static void tasm_def( char *def ) { fprintf( frsp, " /d%s", def ); }
static void masm_def( char *def ) { fprintf( frsp, " /D%s", def ); }
static void nasm_def( char *def ) { fprintf( frsp, " -d%s", def ); }

static void wlink_obj( char *data ) { fprintf( frsp, "\tfile %s \n", data ); }
static void wlink_lib( char *data ) { fprintf( frsp, "\tlibrary %s \n", data ); }

static char *libpath = NULL;
static void linker_libpath( char *data ) { if ( libpath == NULL ) libpath = data; }

static void delete_file( char *data, int temp ) {
    if (temp) fprintf( frsp, "\t Del %s\n", data);
}


void make( void ) {
//  static char args[ 12 ];

    open_rsp();

    fprintf( frsp, "#\n# generate MK386\n#\n\n" );

    fprintf( frsp, "### INCLUDE paths\n" );
    first = 1;
    fprintf( frsp, "PATH_INC=" );
    iter0( PATH_INC, make_path );
    fprintf( frsp, "\n\n" );

    fprintf( frsp, "### C compiler\n" );
    switch ( compiler ) {
        case COMPILER_CC386:
            fprintf( frsp, "CC      = CC386\n" );
            break;
        default:
            fprintf( stderr, "Use unknow compiler!\n" );
            exit(-1);
    }

    fprintf( frsp, "CC_OPT  = /c %ci %ce %cl %cA",
            ( ( options & _OPT_PPRFILE ) ? '+' : '-' ),
            ( ( options & _OPT_ERRFILE ) ? '+' : '-' ),
            ( ( options & _OPT_LSTFILE ) ? '+' : '-' ),
            ( ( options & _OPT_EXTEND  ) ? '-' : '+' ) );

    iter0( OPT_CPP,  compiler_opt );

    if ( assembler == ASSEMBLER_NASM )
        fprintf( frsp, " -C+N");
    else if (assembler == ASSEMBLER_MASM ||
             assembler == ASSEMBLER_ML6X ||
             assembler == ASSEMBLER_WASM ||
             assembler == ASSEMBLER_LASM )
        fprintf( frsp, " -C+M");
    else
        fprintf( frsp, " -C-N");

    iter0( DEFINED,  compiler_def );

    fprintf( frsp, " \"-I$(PATH_INC)\"\n\n" );

    fprintf( frsp, "### Assembler\n" );
    switch ( assembler ) {
        case ASSEMBLER_TASM:
            fprintf( frsp, "AS      = TASM\n" );
            fprintf( frsp, "AS_OPT  = /t /ml /m9 /z%c /i$(PATH_INC)",
                ( ( options & _OPT_DEBUG ) ? 'i' : 'n' ) );
            iter0( DEFINED, tasm_def );
            break;
        case ASSEMBLER_TASMX:
            fprintf( frsp, "AS      = TASMX\n" );
            fprintf( frsp, "AS_OPT  = /t /ml /m9 /z%c /i$(PATH_INC)",
                ( ( options & _OPT_DEBUG ) ? 'i' : 'n' ) );
            iter0( DEFINED, tasm_def );
            break;
        case ASSEMBLER_TASM32:
            fprintf( frsp, "AS      = TASM32\n" );
            fprintf( frsp, "AS_OPT  = /t /ml /m9 /z%c /i$(PATH_INC)",
                ( ( options & _OPT_DEBUG ) ? 'i' : 'n' ) );
            iter0( DEFINED, tasm_def );
            break;
        case ASSEMBLER_MASM:
            fprintf( frsp, "AS      = MASM\n" );
            fprintf( frsp, "AS_OPT  = /t /Ml %s /I$(PATH_INC)",
                ( ( options & _OPT_DEBUG ) ? "/Zi" : "" ) );
            iter0( DEFINED, masm_def );
            break;
        case ASSEMBLER_NASM:
            fprintf( frsp, "AS      = NASM\n" );
            fprintf( frsp, "AS_OPT  = -f obj \"-i$(PATH_INC)\"" );
            iter0( DEFINED, nasm_def );
            break;
        case ASSEMBLER_ML6X:
            fprintf( frsp, "AS      = ML\n" );
            fprintf( frsp, "AS_OPT  = /nologo /c /Cp %s /I$(PATH_INC)",
                ( ( options & _OPT_DEBUG ) ? "/Zi /Zd" : "" ) );
            iter0( DEFINED, masm_def );
            break;
        case ASSEMBLER_LASM:
            fprintf( frsp, "AS      = LASM\n" );
            fprintf( frsp, "AS_OPT  = /G1 /O /S /U /Y  %s /I$(PATH_INC)",
                ( ( options & _OPT_DEBUG ) ? "/ZH" : "" ) );
            iter0( DEFINED, masm_def );
            break;
        case ASSEMBLER_WASM:
            fprintf( frsp, "AS      = WASM\n" );
            fprintf( frsp, "AS_OPT  = -4ps -fpi87 -q -mf %s -i=$(PATH_INC)",
                ( ( options & _OPT_DEBUG ) ? "-d1" : "" ) );
            iter0( DEFINED, masm_def );
            break;
        default:
            fprintf( stderr, "Use unknow assembler !\n" );
            exit(-1);
    }
    fprintf( frsp, "\n\n" );

    fprintf( frsp, "### Linker\n" );
    switch ( linker ) {

        case LINKER_TLINK:

            if ( dosx != DOSX_PMODE ) {
                fprintf( stderr, "Warning : TLINK use only with PMODE !\n" );
                dosx = DOSX_PMODE;
            }

            fprintf( frsp, "LNK     = TLINK\n"
                           "LNK_OPT = /3/c/d" );

            if ( options & _OPT_MAPFILE )
                    fprintf( frsp, "/m/l/s" );
            else
                    fprintf( frsp, "/x" );

            if ( options & _OPT_DEBUG )
                fprintf( frsp, "/v" );

            break;

        case LINKER_LINK:

            if ( dosx != DOSX_PMODE ) {
                fprintf( stderr, "Warning : LINK use only with PMODE !\n" );
                dosx = DOSX_PMODE;
            }

            fprintf( frsp, "LNK     = LINK\n"
                           "LNK_OPT = /NOIGNORECASE" );

            if ( options & _OPT_MAPFILE )
                fprintf( frsp, " /MAP" );

            break;

        case LINKER_VALX:
            if ( dosx == DOSX_PMODE ) {
	            fprintf( frsp, "LNK     = VALX\n" 
                           "LNK_OPT = -NOCA -USE32" );
                dosx = DOSX_PMODE;
            }
			else
			{

	            fprintf( frsp, "LNK     = VALX\n" 
                           "LNK_OPT = -NOCA -LE" );
			}
            if ( options & _OPT_DEBUG )
                fprintf( frsp, " -DEB" );

            if ( options & _OPT_MAPFILE )
                fprintf( frsp,  " -MAP" );
            else
                fprintf( frsp, " -NOMAP" );

            break;

        case LINKER_WLINK:
            if ( dosx == DOSX_PMODE ) {
                fprintf( stderr, "Warning : WLINK can't use with PMODE ! Use DOS4G ...\n" );
                dosx = DOSX_DOS4G;
            }

            fprintf( frsp, "LNK     = WLINK\n"
                           "LNK_OPT =" );

            if ( options & _OPT_DEBUG )
                fprintf( frsp, " debug all op symf" );

            break;

        default:
            fprintf( stderr, "Use unknow linker !\n" );
            exit(-1);
    }
    fprintf( frsp, "\n\n" );

    fprintf( frsp, "### Path to default lib & obj\n" );
    iter0( PATH_LIB, linker_libpath );
    if ( libpath == NULL )
        libpath = ".";
    fprintf( frsp, "LIBPATH = %s\n\n", libpath );

    fprintf( frsp, "### Default lib & obj\n" );
    if ( !( options & _OPT_NODEFLIB ) ) {
        if ( dosx == DOSX_PMODE ) {
            if ( options & _OPT_DEBUG )
                fprintf( frsp, "DEFOBJ  = $(LIBPATH)\\C0DOSD.OBJ\n" );
            else
                fprintf( frsp, "DEFOBJ  = $(LIBPATH)\\C0DOS.OBJ\n" );
        }
        else
            if ( options & _OPT_DEBUG )
                fprintf( frsp, "DEFOBJ  = $(LIBPATH)\\C0DOSWD.OBJ\n" );
            else
                fprintf( frsp, "DEFOBJ  = $(LIBPATH)\\C0DOSW.OBJ\n" );
        fprintf( frsp, "DEFLIB  = $(LIBPATH)\\CLDOS.LIB\n" );
    }
    else {
        fprintf( frsp, "DEFOBJ  =\n" );
        fprintf( frsp, "DEFLIB  =\n" );
    }
    fprintf( frsp, "\n" );

    fprintf( frsp, "### Name of .EXE file\n" 
                   "EXENAME = %s\n\n", exename );

    fprintf( frsp, "### Name of .MAP file\n" );
    if ( options & _OPT_MAPFILE )
        fprintf( frsp, "MAPNAME = %s.MAP\n\n", exename );
    else
        fprintf( frsp, "MAPNAME = NUL\n\n", exename );

    fprintf( frsp, "### .OBJ file(s)\n" 
                   "OBJS    = \\\n" );
    iter0( FILE_OBJ, make_list );
    fprintf( frsp, "\n" );

    fprintf( frsp, "### .LIB file(s)\n" 
                   "LIBS    = \\\n" );
    iter0( FILE_LIB, make_list );
    fprintf( frsp, "\n\n" );

    fprintf( frsp, "### Main depend\n" 
                   "$(EXENAME).EXE : makefile. $(OBJS) $(LIBS)\n" );
    
    fprintf( frsp, "\t $(LNK) $(LNK_OPT) @&&|\n" );

    switch( linker ) {
        case LINKER_TLINK:
        case LINKER_LINK:
            fprintf( frsp, "\t$(DEFOBJ) $(OBJS)\n\t @echo $(EXENAME).EXE\n\t @echo $(MAPNAME)\n\t @echo $(DEFLIB) $(LIBS)\n" );
            break;
        case LINKER_VALX:
            switch ( dosx ) {
				case DOSX_PMODE:
					break;
                case DOSX_PMODEW :
                    fprintf( frsp, " -STB:($(LIBPATH)\\PMODEW.EXE)" );
                break;
                case DOSX_WDOSX :
                    fprintf( frsp, " -STB:($(LIBPATH)\\WDOSXLE.EXE)" );
                break;
                case DOSX_DOS32A :
                    fprintf( frsp, " -STB:($(LIBPATH)\\DOS32A.EXE)" );
                break;
                case DOSX_ZRDX :
                    fprintf( frsp, " -STB:($(LIBPATH)\\ZRDX.EXE)" );
                break;
                case DOSX_CAUSEWAY :
                    fprintf( frsp, " -STB:($(LIBPATH)\\CWSTUB.EXE)" );
                break;
                default:
                case DOSX_DOS4G :
                    fprintf( frsp, " -STB:($(LIBPATH)\\4GSTUB.EXE)" );
                    break;
            }
            fprintf( frsp, "\t\"$(DEFOBJ)\" $(OBJS),$(EXENAME).EXE,$(MAPNAME),\"$(DEFLIB)\" $(LIBS)\n" );
            break;
        case LINKER_WLINK:
            fprintf( frsp, "\tformat os2 le\n"
                           "\top nod\n"
                           "\top quiet\n" );
            switch ( dosx ) {
                case DOSX_PMODEW :
                    fprintf( frsp, "\top osname='CC386+PMODE/W'\n"
                                   "\top stub=$(LIBPATH)\\PMODEW.EXE\n" );
                break;
                case DOSX_WDOSX :
                    fprintf( frsp, "\top osname='CC386+WDOSX'\n"
                                   "\top stub=$(LIBPATH)\\WDOSXLE.EXE\n" );
                break;
                case DOSX_DOS32A :
                    fprintf( frsp, "\top osname='CC386+DOS/32A'\n"
                                   "\top stub=$(LIBPATH)\\DOS32A.EXE\n" );
                break;
                case DOSX_ZRDX :
                    fprintf( frsp, "\top osname='CC386+ZRDX'\n"
                                   "\top stub=$(LIBPATH)\\ZRDX.EXE\n" );
                break;
                case DOSX_CAUSEWAY :
                    fprintf( frsp, "\top osname='CC386+CAUSEWAY'\n"
                                   "\top stub=$(LIBPATH)\\CWSTUB.EXE\n" );
                break;
                default:
                case DOSX_DOS4G :
                    fprintf( frsp, "\top osname='CC386+DOS/4G[W]'\n"
                                   "\top stub=$(LIBPATH)\\4GSTUB.EXE\n" );
                    break;
            }
            fprintf( frsp, "\tname $(EXENAME).EXE\n"
                           "\top map=$(MAPNAME)\n" );

            if ( !( options & _OPT_NODEFLIB ) )
                fprintf( frsp, "\tfile $(DEFOBJ)\n" );

            iter0( FILE_OBJ, wlink_obj );

            if ( !( options & _OPT_NODEFLIB ) )
                fprintf( frsp, "\tlibrary $(DEFLIB)\n" );

            iter0( FILE_LIB, wlink_lib );
            break;

        default:
            fprintf( stderr, "Use unknow linker !\n" );
            exit(-1);
    }

    fprintf( frsp, "|\n" );


    if ( !(options & _OPT_KEEPRSP ) )
        fprintf( frsp, "\t Del $(EXENAME).LNK\n" );

    if ( !(options & _OPT_KEEPGEN ) ) {
        iter( FILE_ASM, delete_file );
//      iter( FILE_OBJ, delete_file );
    }

    fprintf( frsp, "\n");

//  fprintf( frsp, "### .LNK file\n" );
//  fprintf( frsp, "$(EXENAME).LNK : makefile. $(OBJS) $(LIBS)\n" );

    iter0( FILE_CPP, make_cpp );
    iter( FILE_ASM, make_asm );

    close_rsp();

    if ( options & _OPT_RUNMAKE ) {
        switch( maker ) {
            case MAKER_MAKE :
                exec( "MAKE", ( options & _OPT_BUILDALL ) ? "-B" : "" );
                break;
            case MAKER_IMAKE :
                exec( "IMAKE", ( options & _OPT_BUILDALL ) ? "-B" : ""  );
                break;
            case MAKER_WMAKE :
                exec( "WMAKE", ( options & _OPT_BUILDALL ) ? "/a /u" : "/u"  );
                break;
            default:
                fprintf( stderr, "Use unknow maker !\n" );
                exit(-1);
        }
    }
}