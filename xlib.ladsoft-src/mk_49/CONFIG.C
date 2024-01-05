#include "cl.h"

void scan_env(char *output,char *string)
{
   char name[256],*p = name ;
   while (*string) {
      if (*string == '%') {
         p = name ;
         string++ ;
         while (*string && *string != '%')
            *p++ = *string++ ;
         if (*string)
            string++ ;
         *p = 0 ;
         p = getenv(name) ;
         if (p) {
            strcpy(output,p) ;
            output += strlen(output) ;
         }

      } else
         *output++ = *string++ ;
   }
   *output = 0 ;
}
int config_file( char *file ) {
    char str[ STR_MAX ];
    char str1[ STR_MAX ];
    FILE *fcfg = fopen( file, "rt" );

    if ( fcfg != NULL ) {
        while( fgets( str, STR_MAX, fcfg ) != NULL ) {
            scan_env(str1, str) ;
            config_str( str1 );
        }
        fclose( fcfg );
        return 1;
    }
    else
        return 0;
}

void config_str( char *s ) {
    char arg[ STR_MAX ];
    int  i;

	while(1) {
		while ( *s && isspace(*s) )	/* skip spaces */
			s++;
		if ( !*s )	/* end of line */
			return;
		if ( *s == '\"') {	/* " */
			s++;
			i = 0;
			while ( *s && *s != '\"' )
                arg[i++] = *s, s++;
            arg[i] = 0;
            config_arg(arg);
            if (!*s)
            	return;
			s++;
		} else {
			i = 0;
			while ( *s && !isspace(*s))
                arg[i++] = *s, s++;
            arg[i] = 0;
            config_arg(arg);
		}
	}
}

#define is_arg(s) \
    (  ( (s)[0] == '-' ) || ( (s)[0] == '+' ) || ( (s)[0] == '/' )  )
#define is_rsp(s) \
    ( (s)[0] == '@' )

void config_arg( char *arg ) {
    if ( is_arg(arg) )
        get_param( arg );
    else if ( is_rsp(arg) ) {
        if (! config_file( arg + 1 ) ) {
            fprintf( stderr, "File '%s' not found !", arg + 1 );
            exit(-1);
        }
    }
    else
        get_file( arg );
}