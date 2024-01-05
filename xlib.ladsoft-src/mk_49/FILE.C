#include "cl.h"

int nofiles = 1;

static char disk[ _MAX_DRIVE ];
static char path[ _MAX_DIR   ];
static char name[ _MAX_FNAME ];
static char ext [ _MAX_EXT   ];

static void get_files( char *fpath, char *fname );
static void get_one_file( char *file );

void get_file( char *file ) {
    static char findpath[ _MAX_PATH ];
    static char findname[ _MAX_PATH ];

    _splitpath( file, disk, path, name, ext );

    if ( strchr( file, '?' ) || strchr( file, '*' ) ) {  /* Check for '*' and '?' */
        _makepath( findpath, disk, path, NULL, NULL );
        _makepath( findname, NULL, NULL, name, ext );
        get_files( findpath, findname );
    }
    else
        get_one_file( file );
}


#ifdef __TURBOC__

#include <dir.h>
#include <dos.h>

static void get_files( char *fpath, char *fname ) {
    int fpath_len = strlen( fpath );
    struct ffblk fb;

    if ( !findfirst( strcat( fpath, fname), &fb, FA_ARCH) )
    do {
        fpath[ fpath_len ] = '\0';
        get_one_file( strcat( fpath, fb.ff_name) );
    } while (! findnext(&fb) );
}


#else

#include <dos.h>

static void get_files( char *fpath, char *fname ) {
    int fpath_len = strlen( fpath );
    struct find_t fb;

    if (!_dos_findfirst( strcat( fpath, fname), _A_NORMAL, &fb ) )
    do {
        fpath[ fpath_len ] = '\0';
        get_one_file( strcat( fpath, fb.name) );
    } while( ! _dos_findnext( &fb ) );
}

#endif

static char ASM[] = ".ASM";
static char OBJ[] = ".OBJ";
static char LIB[] = ".LIB";
static char CFG[] = ".CFG";

void get_one_file( char *srcfile ) {
    int temp = 0;
    char file[ _MAX_PATH ];

    nofiles = 0;
    strncpy( file, srcfile, _MAX_PATH );
    _splitpath( strupr( file ) , disk, path, name, ext );

    if ( strlen( exename ) == 0 )
        strncpy( exename, name, _MAX_PATH );

    if ( strcmp( ext, ASM ) != 0 && strcmp( ext, OBJ ) != 0 &&
                                    strcmp( ext, LIB ) != 0 ) {
        if ( find_file_cpp( file ) )
            fprintf( stderr, "Warning! File already append : %s\n", file );
        else
            add_file_cpp( file );
        temp = 1;
        strncpy( file, file2curdir( file, ASM ), _MAX_PATH  );
        strcpy( ext, ASM );
//      _makepath( file, disk, path, name, strcpy( ext, ASM ) );
    }

    if ( strcmp( ext, ASM ) == 0 ) {
        if ( find_file_asm( file ) )
            fprintf( stderr, "Warning! File already append : %s\n", file );
        else
            add_file_asm( file, temp );
        temp = 1;
        strncpy( file, file2curdir( file, OBJ ), _MAX_PATH  );
        strcpy(ext, OBJ);
    }

    if ( strcmp( ext, OBJ ) == 0 ) {
        if ( find_file_obj( file ) )
            fprintf( stderr, "Warning! File already append : %s\n", file );
        else
            add_file_obj( file, temp );
    }

    if ( strcmp( ext, LIB ) == 0 ) {
        if ( find_file_lib( file ) )
            fprintf( stderr, "Warning! File already append : %s\n", file );
        else
            add_file_lib( file );
    }
}


void config( char *exe_name, char *env_var, int argc, char *argv[] ) {
	char _file[ _MAX_PATH ];
//	char _path[ _MAX_PATH ];
	int i;
	char *env;

	_splitpath( exe_name, disk, path, name, ext );
	_makepath( _file, NULL, NULL, name, CFG );
	if ( ! config_file( _file ) ) {
		_makepath( _file, disk, path, name, CFG );
		config_file( _file );
	}

    if ( (env = getenv( env_var ) ) != NULL )
        config_str( env );

    for ( i = 0; i < argc; i++ )
        config_arg( argv[ i ] );
}

char *file2curdir( char *file, char *ex ) {
    static char _path[ _MAX_PATH ];
    _splitpath( file, disk, path, name, ext );
    _makepath( _path, NULL, NULL, name, ex );
    return _path;
}