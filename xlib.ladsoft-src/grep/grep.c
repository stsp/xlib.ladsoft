/* 
GREP
Copyright 2007-2011 David Lindauer.

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

This program is derived from the cc68k complier by 
Matthew Brandt (mailto::mattb@walkingdog.net) 

You may contact the author of this derivative at:

mailto::camille@bluegrass.net
 */
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <cmdline.h>
#include <regexp.h>
#include "..\version.h"
 
#define DOBANNER banner("%s Version %s %s", PROGNAME, GREP_STRING_VERSION, PRODUCT_COPYRIGHT)

#define PROGNAME "GREP"

extern int fileCount;
extern char **files;

char *usage_text = "[-rxlcnvidzwo?] searchstring file[s]\n";
static char *help_text = "[options] searchstring file[s]"
					"\n"
					"   -c  show Match Count only        -d  Recurse Subdirectories\n"
					"   -i  case Insensitive             -l  show File Names only\n"
					"   -n  Show Line Numbers            -o  UNIX Output Format\n"
					"   -r  Regular Expressions          -v  Non Matching Lines\n"
					"   -w  Complete Words Only          -x  Disable regular expressions\n"
					"   -z  Verbose                      -?  this help\n"
					"\n"
					"Regular expressions special characters:\n"
					"   .  Match any character   *  Match zero or more  +  Match one or more\n"
					"   ?  Match zero or one     |  Match either\n"
					"   ^  Start of line         $  End of line         \\  Quote next character\n"
					"   \\b Word break            \\B Inside word         \\w Word beginning\n"
					"   \\W Word end              \\< Word constituent    \\> Word non-constituent\n"
					"\n"
					"Use brackets to match one characters out of a set:\n"
					"   [aeiou4-7]   match vowels or the numbers from 4 to 7 inclusive\n"
					"   [^aeiou4-7]  match anything but vowels or the numbers from 4 to 7\n"
					"Use \\( and \\) to set a match region, \\x where x is a digit to access it\n"
					"Use \\{ and \\} to set an interval:\n"
					"   a\\{2,4\\} matches from two to four 'a' characters\n"
					"\n"
			        "Time: " __TIME__ "  Date: " __DATE__;

void bool_setup(char select, char *string);

int prm_recurseDirs;
int prm_insensitive;
int prm_completeWords;
int prm_regular = TRUE;

static int prm_matchCount;
static int prm_fileNamesOnly;
static int prm_lineNumbers;
static int prm_unix;
static int prm_nonMatching;
static int prm_verbose;
static int prm_help;

static int bHeaderFileName = TRUE;
static int bFileNames = FALSE;
static int bLineNumbers = FALSE;

ARGLIST ArgList[] = 
{
    {
        'c', ARG_BOOL, bool_setup
    }
    , 
    {
        'd', ARG_BOOL, bool_setup
    }
    , 
    {
        'i', ARG_BOOL, bool_setup
    }
    , 
    {
        'l', ARG_BOOL, bool_setup
    }
	,
    {
        'n', ARG_BOOL, bool_setup
    }
    , 
    {
        'o', ARG_BOOL, bool_setup
    }
	,
    {
        'r', ARG_BOOL, bool_setup
    }
	,
    {
        'v', ARG_BOOL, bool_setup
    }
    , 
    {
        'w', ARG_BOOL, bool_setup
    }
	,
    {
        'x', ARG_BOOL, bool_setup
    }
	,
    {
        'z', ARG_BOOL, bool_setup
    }
	,
    {
        '?', ARG_BOOL, bool_setup
    }
	,
    {
        0, 0, 0
    }
} ;
static void bool_setup(char select, char *string)
{
	switch(select)
	{
		case 'c':
			prm_matchCount = TRUE;
			break;
		case 'd':
			prm_recurseDirs = TRUE;
			break;
		case 'i':
			prm_insensitive = TRUE;
			break;
		case 'l':
			prm_fileNamesOnly = TRUE;
			break;
		case 'n':
			prm_lineNumbers = TRUE;
			break;
		case 'o':
			prm_unix = TRUE;
			break;
		case 'r':
			prm_regular = TRUE;
			break;
		case 'v':
			prm_nonMatching = TRUE;
			break;
		case 'w':
			prm_completeWords = TRUE;
			break;
		case 'x':
			prm_regular = FALSE;
			break;
		case 'z':
			prm_verbose = TRUE;
			break;
		case '?':
			prm_help = TRUE;
			break;
	}
}
static void SetMode(void)
{

	if (prm_lineNumbers)
		bLineNumbers = TRUE;
	if (prm_verbose)
	{
		prm_unix = FALSE;
		bHeaderFileName = TRUE;
		bFileNames = FALSE;
		bLineNumbers = TRUE;
		prm_fileNamesOnly = FALSE;
	}
	else if (prm_matchCount)
	{
		prm_unix = FALSE;
		prm_fileNamesOnly = TRUE;
		bLineNumbers = FALSE;
		bFileNames= FALSE;
		bHeaderFileName = TRUE;
	}
	else if (prm_fileNamesOnly)
	{
		bLineNumbers = FALSE;
		bFileNames= FALSE;
		bHeaderFileName = TRUE;
	}
	else if (prm_unix)
	{
		bFileNames = TRUE;
		bHeaderFileName = FALSE;
	}
}
static void DisplayMatch(fileName, matchCount, lineno, text)
char *fileName;
int *matchCount;
int lineno; 
char *text;
{
	if (*matchCount == 0 && bHeaderFileName)
	{
		printf("File %s", fileName);
		if (prm_verbose || !prm_matchCount)
			printf("\n");
	}
	if (!prm_fileNamesOnly)
	{		
		if (prm_unix)
		{
			if (bFileNames)
			{
				char format[256];
				int n = strlen(fileName);
				n +=8;
				n = (n / 8) * 8;
				sprintf(format, "%%-%ds", n);
				printf(format,fileName);
			}
			if (bLineNumbers)
			{
				printf("%-8d", lineno);
			}
			printf("%s\n", text);
		}
		else
		{
			if (bFileNames)
			{
				char format[256];
				int n = strlen(fileName);
				n +=8;
				n = (n / 8) * 8;
				sprintf(format, "%%-%ds", n);
				printf(format,fileName);
			}
			if (bLineNumbers)
			{
				printf("%-8d", lineno);
			}
			printf("%s\n", text);
		}
	}
	(*matchCount)++;
}
int main(argc, argv)
int *argv[];
char argc;
{
	int i;
	RE_CONTEXT *reContext;
    if (!parse_args(&argc, argv, TRUE))
        usage(argv[0]);
	if (prm_help || argc >= 2 && !strcmp(argv[1], "?"))
	{
		DOBANNER;
		printf("%s %s", PROGNAME, help_text);
		exit(0);
	}
        if (argc < 2)
	{
        usage(argv[0]);
	}
	if (prm_verbose)
	{
		DOBANNER;
	}
	SetMode();
	reContext = re_init(argv[1],
			(prm_regular ? RE_F_REGULAR : 0) |
			(prm_insensitive ? RE_F_INSENSITIVE : 0) |
			(prm_completeWords ? RE_F_WORD : 0), NULL);
	if (!reContext)
		fatal("Invalid regular expression");
	GatherFiles(argv + 2);
	if (fileCount == 0)
	{
	  if (isatty (fileno (stdin))) {
		printf("No files found");
		exit(1);
	  }
	  fileCount = 1;
	  files = realloc(files, sizeof(char *));
	  if (!files)
	    fatal("out of memory");
	  files[0] = "stdin";
	}
	for (i=0; i < fileCount; i++)
	{
		FILE *fil;
		if (!strcmp (files[i], "stdin")) fil = stdin;
		else fil = fopen(files[i], "rt");
		if (fil)
		{
			int matchCount = 0;
			int lineno = 0;
			char buf[1024];
			while (!feof(fil))
			{
				int len;
				buf[0] = 0;
				fgets(buf, 1020, fil);
				buf[1020] = 0;
				len = strlen(buf);
				if (buf[len-1] == '\n')
					buf[len-1] = 0;
				lineno++;
				if (re_matches(reContext, buf, 0, strlen(buf)))
				{
					if (!prm_nonMatching)
					{
						DisplayMatch(files[i], &matchCount, lineno, buf);
					}
				}
				else if (prm_nonMatching)
				{
					DisplayMatch(files[i], &matchCount, lineno, buf);
				}
			}
			if (fil != stdin) fclose(fil);
			if (matchCount && prm_verbose)
			{
				if (prm_nonMatching)
				{
					printf("%d Non-Matching lines\n", matchCount);
				}
				else
				{
					printf("%d Matching lines\n", matchCount);
				}
			}
			else if (matchCount && prm_matchCount)
			{
				printf(": %d\n", matchCount);
			}
		}
	}
	re_free(reContext);
	return 0;
}
