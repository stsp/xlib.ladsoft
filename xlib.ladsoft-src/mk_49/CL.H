#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>
#include <ctype.h>

/*
**  LIST.C
**********/

/************************ type of item */
#define FILE_CPP    1   /* file C/C++ */
#define FILE_ASM    2   /* file ASM */
#define FILE_OBJ    3   /* file OBJ */
#define FILE_LIB    4   /* file LIB */
#define PATH_INC    5   /* path to INC */
#define PATH_LIB    6   /* path to LIB */
#define OPT_CPP     7   /* option for CPP */
#define DEFINED     8   /* defined symbol */

void add( int what, int type, char *data );                 // add to list
void clear( void );                                         // clear list
void iter( int what, void (*fiter)( char *data, int temp) );// iter item
void iter0(int what, void (*fiter)( char *data) );          // iter item
int  find( int what, char *data );                          // find
void print(void);

#define add_file_cpp(file)          add( FILE_CPP, 0, (file) )
#define add_file_asm(file, temp)    add( FILE_ASM, (temp), (file) )
#define add_file_obj(file, temp)    add( FILE_OBJ, (temp), (file) )
#define add_file_lib(file)          add( FILE_LIB, 0, (file) )

#define add_path_inc(path)          add( PATH_INC, 0, (path) )
#define add_path_lib(path)          add( PATH_LIB, 0, (path) )

#define add_option(opt)             add( OPT_CPP, 0, (opt) )

#define add_defined(def)            add( DEFINED, 0, (def) )

#define find_file_cpp(file)         find( FILE_CPP, (file) )
#define find_file_asm(file)         find( FILE_ASM, (file) )
#define find_file_obj(file)         find( FILE_OBJ, (file) )
#define find_file_lib(file)         find( FILE_LIB, (file) )


/*
**  CONFIG.C
************/

#define STR_MAX 256 /* max len of string */

int config_file( char *file );
void config_str( char *s );
void config_arg( char *arg );


/*
**  FILE.C
**********/
extern int nofiles;  /* TRUE, if no file in arguments */

void get_file( char *file );
void config( char *exe_name, char *env_var, int argc, char *argv[] );
char *file2curdir( char *file, char *ex );

/*
**  PARAM.C
***********/

#define _OPT_LSTFILE    0x0001
#define _OPT_PPRFILE    0x0002
#define _OPT_ERRFILE    0x0004
#define _OPT_MAPFILE    0x0008

#define _OPT_KEEPGEN    0x0010
#define _OPT_KEEPRSP    0x0020

#define _OPT_RUNMAKE    0x0040
#define _OPT_BUILDALL   0x0080

#define _OPT_EXTEND     0x0100

#define _OPT_DEBUG      0x0200
#define _OPT_NODEFLIB   0x0400

#define _OPT_COMPILE    0x1000
#define _OPT_ASSEMBLE   0x2000
#define _OPT_LINK       0x4000

extern int  options;
extern char exename[];        // name of .EXE file

#define COMPILER_CC386 0
extern int compiler;    // name of C/C++ compiler

#define ASSEMBLER_NASM   0
#define ASSEMBLER_TASM   1
#define ASSEMBLER_TASMX  2
#define ASSEMBLER_TASM32 3
#define ASSEMBLER_MASM   4
#define ASSEMBLER_ML6X   5
#define ASSEMBLER_WASM   6
#define ASSEMBLER_LASM   7
extern int assembler;   // type of ASSEMBLER

#define LINKER_VALX  0
#define LINKER_TLINK 1
#define LINKER_LINK  2
#define LINKER_WLINK 3
extern int linker;      // type of LINKER

#define MAKER_IMAKE 0
#define MAKER_MAKE  1
#define MAKER_WMAKE 2
extern int maker;       // name of make util

#define DOSX_PMODE  0
#define DOSX_PMODEW 1
#define DOSX_DOS4G  2
#define DOSX_DOS32A 3
#define DOSX_WDOSX  4
#define DOSX_ZRDX   5
#define DOSX_CAUSEWAY 6
extern int dosx;    // name of DOS extender

void help(void);
void get_param( char *param );


/*
**  GENER.C
***********/
void make( void );
