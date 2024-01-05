#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

typedef struct {
	char *base;
	char *current;
} FileText;

char *defines[] = { 
	"_intrinsic","",
	"_import","",
	"_export","", 
	"__int64","long long",
	"__stdcall", "",
	"__cdecl", "" 
};


char *prm_x_searchpath = "";
char *sys_x_searchpath = "C:\\program files\\cc386\\include";
#define DEFINE_COUNT (sizeof(defines)/sizeof(char *) / 2)
static char *parsepath(char *path, char *buffer)
{
    char *pos = path;

    /* Quit if hit a ';' */
    while (*pos)
    {
        if (*pos == ';')
        {
            pos++;
            break;
        }
        *buffer++ =  *pos++;
    }
    *buffer = 0;

    /* Return a null pointer if no more data */
    if (*pos)
        return (pos);

    return (0);
}
FILE *x_searchpath(char *string, char *x_searchpath, char *mode)
{
    FILE *in;
    char *newpath = x_searchpath;

    /* Search local path */
    in = fopen((char*)string, mode);
    if (in)
    {
        return (in);
    }
    else if (!x_searchpath || x_searchpath[0] == 0)
        return 0;
    else
    {
        /* If no path specified we search along the search path */
        if (string[0] != '\\' && string[1] != ':')
        {
            char buffer[200];
            while (newpath)
            {
                /* Create a file name along this path */
                newpath = parsepath(newpath, buffer);
                if (buffer[strlen(buffer) - 1] != '\\')
                    strcat(buffer, "\\");
                strcat(buffer, (char*)string);

                /* Check this path */
                in = fopen(buffer, mode);
                if (in)
                {
                    strcpy(string, buffer);
                    return (in);
                }
            }
        }
    }
    return (0);
}
static FILE *srchpth(char *name, char *path, char *attrib, int sys)
{
    FILE *rv = x_searchpath(name, path, attrib);
    char buf[265],  *p;
    if (rv !=  NULL || !sys)
        return rv;
    strcpy(buf, name);
    p = strrchr(buf, '.');
    if (p && !stricmp(p, ".h"))
    {
        *p = 0;
        rv = x_searchpath(buf, path, attrib);
        if (rv !=  NULL)
            strcpy(name, buf);
    }
    return rv;
}
static const void * string_getline(FileText *arg)
{
	static char line[513];
	char *p = line;
	int n = 512;
	if (*arg->current == 0)
		return NULL;
	line[0] = 0;
	while (*arg->current && *arg->current != '\n' && n--)
	{
		if (*arg->current != '\r')
			*p++ = *(arg->current)++;
		else
			(arg->current)++;
	}
			
	*p = 0;
	if (*arg->current == '\n')
		++(arg->current);
	return line;
}

void __stdcall typecallback(char *symname, char *filename, 
					   int linestart, int lineend, type_t *typeinfo,
					   void *userdata)
{
	printf("\t%s(%d:%d):%s\n",filename, linestart, lineend, symname);
}
void __stdcall symcallback(char *symname, char *filename, 
					   int linestart, int lineend, type_t *typeinfo,
					   void *userdata)
{
	type_t *usertype;
	int indirect = 0;
	char *typenam = "";
	usertype = typeinfo;
	if (!typeinfo)
		return;
	while (usertype->btype == BT_POINTER)
	{
		usertype = usertype->link;
		indirect++;
	}
	switch(usertype->btype)
	{
		case BT_FUNC:
			typenam = "Func";
			break;
		case BT_STRUCT:
			typenam = "Struct";
			break;
		case BT_UNION:
			typenam = "Union";
			break;
		case BT_ENUM:
			typenam = "Enum";
			break;
	}
	printf("%s(%d:%d):%s(%d)(%s)\n",filename, linestart, lineend, 
		   symname, indirect, typenam);
	if (typenam[0])
	{
		enumTypes(usertype, typecallback, 0);
	}
	if (typeinfo && typeinfo->typedefname)
		printf("...%s\n", typeinfo->typedefname);
}
int __stdcall file_callback(char *name, int sys_inc, void **func, void **arg, 
							void *lines_used, int line_count)
{
	
		FILE *fp;
		FileText *ftext ;
		if (!name)
		{
			ftext = *arg;
			free(ftext->base);
			free(ftext);
			return 0;
		}
		// allocate
	    if (!sys_inc)
	        fp = srchpth(name, prm_x_searchpath, "r", FALSE)
	            ;
	    if (sys_inc || fp ==  NULL)
	    {
	        fp = srchpth(name, sys_x_searchpath, "r", TRUE);
	        if (fp == NULL && sys_inc)
	            fp = srchpth(name, prm_x_searchpath, "r", FALSE);
	    }
		if (fp == NULL)
			return 1;
		if (fp)
		{
			long len;
			char *str;
			fseek(fp, 0, SEEK_END);
			len = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			str = calloc(1, len+1);
			if (str)
			{
				// may read less because of removeal of '\r'
				len = fread(str, 1, len, fp);
				str[len] = 0;
			}
			fclose(fp);
			ftext = calloc(1, sizeof(FileText));
			ftext->base = ftext->current = str;
			*func = string_getline;
			*arg = ftext;
			return 0;
		}
		return 1;
}
int main(int argc, char* argv[])
{
	if (argv[1])
	{
        parse(argv[1],file_callback, NULL, NULL, NULL,
			   DEFINE_COUNT, defines, symcallback, 0);
	}
    return 0;
}
