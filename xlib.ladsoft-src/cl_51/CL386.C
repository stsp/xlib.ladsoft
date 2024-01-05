#include "afx.h"

static char rundir[LINE_MAX];

char *getrundir(void) { return rundir; }

int main( int argc, char *argv[] ) {
	int ret;

	if ( argc == 1 )
		help();

	atexit( clear_all );

	fn_split( argv[0] );
	strcpy( rundir, fn_path() );
#if DEBUG
	printf("*** Run from '%s'\n", getrundir() );
#endif
/*
	strcat( strcpy( cfgfile, fn_name() ), ".cfg" ); 
*/
	/*
	** EnvVar, config filename, argv, argc
    */
	parse_params( "CL386", "cl386.cfg", argv, argc );

	if ( get_make_opt(OPT_COMPILE) )
		compile();
	if ( get_make_opt(OPT_ASSEMBLY) )
		assembly();
    xrc();
	if ( get_make_opt(OPT_LINKING) )
		linking();
   return 0 ;
}