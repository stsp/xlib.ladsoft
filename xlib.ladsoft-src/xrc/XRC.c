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
#include        <stdio.h>
#include				<malloc.h>
#include 				<string.h>
#include 				<setjmp.h>
#include				<signal.h>
#include				<stdlib.h>
#include          <dir.h>
#include				"utype.h"	
#include				"cmdline.h"	
#include				"umem.h"
#include          "..\version.h"

char *usage_text = "[options] file"
"\n"
"\t@filename  use response file\n"
"\t/D  Define something\n"
"\t/fo  set output file\n"
"\t/i  set include file path\n"
"\t/I  set include file path\n"
"\t/r  reserved for compatibility\n"
"\t/t  reserved for compatibility\n"
"\t/v  reserved for compatibility\n" ;

extern int total_errors ;
extern char *errfile ;
int prm_listfile = FALSE ;
int prm_warning = FALSE ;
int prm_extwarning = FALSE ;
int prm_quieterrors = FALSE ;
int prm_errfile = FALSE ;
int prm_maxerr = 0 ;
int prm_ansi = FALSE ;
int prm_cplusplus = FALSE ;
int prm_trigraph = FALSE ;
int prm_cmangle = FALSE ;
char version[256];
char *listFile ;
char *errFile ;
char *cppFile ;
char *inputFile ;
char *outputFile[200] ;

char *prm_searchpath = 0 ;
LIST *deflist ;

void incl_setup(char select, char *string);
void ofile_setup(char select, char *string);
void bool_setup(char select, char *string);
void def_setup(char select, char *string);
ARGLIST ArgList[] = {
	{ 'D', ARG_CONCATSTRING, def_setup },
	{ 'f', ARG_CONCATSTRING, ofile_setup },
	{ 'i', ARG_CONCATSTRING, incl_setup },
	{ 'I', ARG_CONCATSTRING, incl_setup },
	{ 'r', ARG_BOOL, bool_setup },
	{ 't', ARG_BOOL, bool_setup },
	{ 'v', ARG_BOOL, bool_setup },
	{0,0 },
} ;
void bool_setup(char select, char *string)
{
}
void setglbldefs(void)
{
	LIST *l = deflist;
	int major, minor;
	char buf[256];
	while (l) {
		char *s = l->data;
		char *n = s;
		while (*s && *s != '=')
			s++;
		if (*s == '=')
			*s++=0;
		glbdefine(n,s);
		l = l->link;
	}
	sscanf(CC_STRING_VERSION, "%d.%d", &major, &minor);
	sprintf(buf, "%d", major *100+minor);
	glbdefine("__CC386__", buf);
	sscanf(XRC_STRING_VERSION, "%d.%d", &major, &minor);
	sprintf(buf, "%d", major *100+minor);
	glbdefine("__XRC__", buf);
    glbdefine("__CCDL__","");
    glbdefine("__386__","");
    glbdefine("__i386__","");
    glbdefine("RC_INVOKED","");
}
void def_setup(char select, char *string)
/*
 * activation for command line #defines
 */
{
	char *s = strdup(string);
	LIST *l = malloc(sizeof(LIST));
	l->link = deflist;
	deflist = l;
	l->data = s;
}
void addinclude(void)
/*
 * Look up the INCLUDE environment variable and append it to the
 * search path
 */
{
	char *string = getenv("CCINCL");
	if (string && string[0]) {
      incl_setup('I',string) ;
	}
}
void incl_setup(char select, char * string)
/*
 * activation for include paths
 */
{
   if (prm_searchpath) {
      prm_searchpath = realloc(prm_searchpath,strlen(string)+strlen(prm_searchpath)+2);
      strcat(prm_searchpath,";") ;
   } else {
		prm_searchpath = malloc(strlen(string)+1);
		prm_searchpath[0] = 0;
	}
	strcat(prm_searchpath,string);
}
void ofile_setup(char select, char *string)
{
	if (string[0] != 'o')
		fatal("Invalid paramter -f") ;
	strcpy(outputFile,string+1) ;
	
}
void *AllocateMemory(size_t size)
{
	void *rv ;
	if (size == 0)
		return 0 ;
   rv = calloc(size,1) ;
	if (!rv)
		fatal("out of memory") ;
	return rv ;
}
char *litlate(char *in)
{
	char *rv = AllocateMemory(strlen(in) +1 );
	strcpy(rv,in) ;
	return rv ;
}

void DeallocateMemory(void *val)
{
	free(val) ;
}
void setfile(char *buf,char *orgbuf,char *ext)
/*
 * Get rid of a file path an add an extension to the file name
 */
{
	char *p = strrchr(orgbuf,'\\');
	if (!p) p = orgbuf;
	else p++;
	strcpy(buf,p);
	StripExt(buf);
	AddExt(buf,ext);
}
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
int parse_arbitrary(char *string)
/*
 * take a C string and and convert it to ARGC, ARGV format and then run
 * it through the argument parser
 */
{
	char *argv[40];
   char output[1024] ;
	int rv,i;
	int argc = 1;
	if (!string || !*string)
		return 1;
   scan_env(output,string) ;
   string = output ;
	while (1) {
      int quoted = ' ' ;
		while (*string == ' ') 
			string++;
		if (!*string)
			break;
      if (*string == '"')
         quoted = *string++ ;
		argv[argc++] = string;
      while (*string && *string != quoted) string++;
		if (!*string)
			break;
		*string = 0;
		string++;
	}
  rv = parse_args(&argc,argv,TRUE);
  if (argc > 1)
  {
  	for (i=1; i < argc; i++)
	{
		if (argv[i][0])
		   fatal("Invalid config file") ;
	}
  }
  return rv;
}
int parseconfigfile(char *name)
{
   char buf[256],*p ;
   strcpy(buf,name) ;
   p = strrchr(buf,'\\') ;
   if (p) {
      FILE *temp;
      strcpy(p+1,"XRC.CFG") ;
      if (!(temp = fopen(buf,"r")))
         return 0 ;
      while (!feof(temp)) {
         buf[0] = 0;
         fgets(buf,256,temp);
         if (buf[strlen(buf)-1] == '\n')
            buf[strlen(buf)-1] = 0;
         if (!parse_arbitrary(buf))
            break;
      }
      fclose(temp);
   }
   return 0 ;

}
void parsefile(char select, char *string)
/*
 * parse arguments from an input file
 */
{
	FILE *temp;
	if (!(temp = fopen(string,"r")))
		fatal("Argument file not found");
	while (!feof(temp)) {
		char buf[256];
		buf[0] = 0;
		fgets(buf,256,temp);
		if (buf[strlen(buf)-1] == '\n')
			buf[strlen(buf)-1] = 0;
		if (!parse_arbitrary(buf))
			break;
	}
	fclose(temp);
}
void abspath(char *name)
{
  char projname[280],*p,*nname = name ;
  if (name[0] == 0)
   return ;
   if (name[1] == ':')
      return ;

   getcwd(projname,280) ;
   strcat(projname,"\\hidave") ;
   p = strrchr(projname,'\\') ;
   if (!p)
      return ;
   p-- ;
   if (!strstr(name,"..\\")) {
      if (name[0] != '\\') {
         strcpy(p+2,name) ;
         strcpy(nname,projname) ;
      }
      return ;
   }
   while (!strncmp(name,"..\\",3)) {
      while (p> projname && *p-- != '\\') ;
      name += 3 ;
   }
   *++p = '\\' ;
   p++ ;
   strcpy(p,name) ;
   strcpy(nname,projname) ;
}
void filePathToInclude(char *str)
{
   char buf[280],*p ;
   strcpy(buf,str) ;
   abspath(buf) ;
   p = strrchr(buf,'\\') ;
   if (p) {
      p[1] = 0 ;
      incl_setup('I',buf) ;
   }
}

int main(int argc, char **argv)
{
    int index = 1 ;
   strcpy(version,XRC_STRING_VERSION) ;

  banner("XRC Version %s %s",version, GNU_COPYRIGHT);
  if (!parse_args(&argc, argv, TRUE) || argc < 2)
    usage(argv[0]);
   parseconfigfile(argv[0]) ;
	symini() ;
	preprocini() ;
	initsym() ;
	initerr() ;
	kwini() ;



    if (argv[1][0] == '@') {
        if (argc < 3)
            usage(argv[0]) ;
        index++ ;
        parsefile(0,argv[1]+1) ;
    }
	/* tack the environment includes in */
	setglbldefs() ;
	addinclude();

    errfile = inputFile = strdup(argv[index]) ;
   filePathToInclude(inputFile) ;
	if (!outputFile[0])
		setfile(outputFile,inputFile,".res") ;

	write_resource(outputFile,parse_rc_file(inputFile)) ;

	if (total_errors) {
			remove(outputFile);
			return 1 ;
	}
	return 0;
}