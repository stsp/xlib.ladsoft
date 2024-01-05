/* 
VALX linker
Copyright 1997-2011 David Lindauer.

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

Original 16-bit linker written by:
David Troendle

You may contact the author of this derivative at:
	mailto::camille@bluegrass.net
 */
/* not valxing the source so I can copy it readily if I change it */
/* Handles name unmangling
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dos.h>
#include <time.h>

#include "langext.h"
#include "defines.h"
#include "types.h"
#include "subs.h"
#include "globals.h"

#define HASHTABLESIZE 1023

char *tn_void = "void";
char *tn_char = "char";
char *tn_int = "int";
char *tn_long = "long";
char *tn_longlong = "long long";
char *tn_short = "short ";
char *tn_unsigned = "unsigned ";
char *tn_ellipse = "...";
char *tn_float = "float";
char *tn_double = "double";
char *tn_longdouble = "long double";
char *tn_vol = " volatile ";
char *tn_const = " const ";
char *tn_class = "class ";
char *tn_struct = "struct ";
char *tn_union = "union ";
char *tn_fcomplex = "float complex";
char *tn_rcomplex = "double complex";
char *tn_lrcomplex = "long double complex";

    char *cpp_funcname_tab[] = 
    {
        "$bctr", "$bdtr", "$bnew", "$bdel", "$badd", "$bsub", "$bmul", "$bdiv",
            "$bshl", "$bshr", "$bmod", "$bequ", "$bneq", "$blt", "$bleq", 
            "$bgt", "$bgeq", "$basn", "$basadd", "$bassub", "$basmul", 
            "$basdiv", "$basmod", "$basshl", "$bsasshr", "$basand", "$basor", 
            "$basxor", "$binc", "$bdec", "$barray", "$bcall", "$bpstar", 
            "$barrow", "$bcoma", "$blor", "$bland", "$bnot", "$bor", "$band", "$bxor", 
            "$bcpl", "$bcast", "$bnwa", "$bdla",

    };
    char *xlate_tab[] = 
    {
        0, 0, "new", "delete", "+", "-", "*", "/", "<<", ">>", "%", "==", "!=",
            "<", "<=", ">", ">=", "=", "+=", "-=", "*=", "/=", "%=", "<<=", 
            ">>=", "&=", "|=", "^=", "++", "--", "[]", "()", "->*", "->", ",",
			"||", "&&", "!", "|", "&", "^", "~", "()", "new[]", "delete[]", 
    };
#define IT_THRESHOLD 2
#define IT_OV_THRESHOLD 4
#define IT_SIZE (sizeof(cpp_funcname_tab)/sizeof(char *))
 
static char * unmangcppfunc(char *buf, char *name, int firsttime) ;
static char * unmangcpptype(char *buf, char *name, int firsttime) ;
static char *unmang1(char *buf, char *name, int firsttime) ;

#define MAX_MANGLE_NAME_COUNT 36
static int manglenamecount = -1 ;
static char manglenames[MAX_MANGLE_NAME_COUNT][512] ;



static void cpp_unmang_intrins(char **buf, char **name, char *last)
{
   char cur[245],*p=cur,*q;
	int i;
	*p++ = *(*name)++;
	while (**name != '@' && **name != '$' && **name)
		*p++ = *(*name)++;
	*p = 0;
   if (cur[1] == 'o') {
	  strcpy(p, *name);
	  (*name) += strlen(*name);
      strcpy (*buf,"operator ") ;
      *buf += strlen(*buf) ;
      unmang1(*buf,cur+2,0) ;
   } else {
      for(i=0; i < IT_SIZE; i++)
         if (!strcmp(cur,cpp_funcname_tab[i]))
            break;
      if (i >= IT_SIZE)
         strcpy(*buf,cur);
      else {
         if (i < IT_THRESHOLD) {
            switch (i) {
               case 1:
                  *(*buf)++ = '~';
               case 0:
                  strcpy(*buf,last);
                  break;
            }
         }
         else {
               strcpy(*buf,"operator ");
               strcat(*buf,xlate_tab[i]);
         }
      }
   }
	*buf += strlen(*buf);
		
}
			
/* Argument unmangling for C++ */

static void unmangdollars(char *buf, char **nm)
{
	int quote = False ;
    if (**nm == '$' && *((*nm) +1) != 'q')
	{
		buf += strlen(buf) ;
		(*nm)+= 2 ;
			
		*buf++ = '(' ;
		if ((*nm)[-1] == 's') {
			quote = True ;
			*buf++ = '"' ;
		}
		while (**nm && **nm != '$')
			*buf++ = *(*nm)++ ;
		if (quote)
			*buf++ = '"' ;
		*buf++ = ')' ;
		*buf = 0 ;	
		if (**nm)
			(*nm)++ ;
	}	
}
static char *unmangptr(char *buf, char *name)
{
	char bf[256], *p = bf;
	while (*name == 'a' || *name == 'p' || *name == 'P')
	{
		if (*name == 'P')
		{
			strcpy(p, "far *"), name++;
			p += strlen(p);
		}
		else if (*name == 'p')
			*p++ = '*', name++;
		else
		{
			name++;
			*p++ = '[';
			while (isdigit(*name))
				*p++ = *name++;
			*p++ = ']';				
		}
	}
	*p = 0;
	name = unmang1(buf, name, False);
	strcat(buf, bf);
	return name;

}
		
/* Argument unmangling for C++ */
static char *unmang1(char *buf, char *name, int firsttime)
{
	int v;
	int cvol = 0, cconst = 0;
   char buf1[256],*p,buf2[256] ;
	while (*name == 'x' || *name == 'y') {
		if (*name == 'y')
			cvol++;
		if (*name == 'x')
			cconst++;
		name++;
	}
start:
		if (isdigit(*name)) {
			v = *name++ - '0';
			while (isdigit(*name)) 
				v = v*10+ *name++ - '0';
         if (name[0] == '#') {
            name++ ;
            while (*name && *name != '$' && *name != '#')
               *buf++ = *name++ ;
            *buf = 0 ;
            if (*name == '$') {
               name++ ;
               name = unmang1(buf,name,0) ;
            }
			buf += strlen(buf);
            name++ ;
         } else {
            char *s = buf ;
            while (v--) 
				if (*name == '@') {
					*buf++ = ':' ;
					*buf++ = ':' ;
					name++ ;
				} else			
	               *buf++ = *name++;
            *buf = 0;
            if (manglenamecount < MAX_MANGLE_NAME_COUNT)
                strcpy(manglenames[manglenamecount ++],s) ;
         }
		}
		else switch (*name++) {
            case 'Q':
			case 'q':
				{
                    char *s = buf;
	                if (!firsttime)
	                {
						if (name[-1] == 'Q')
		                    strcpy(buf, " (far *) ");
						else
							strcpy(buf, " (*) ");
	                    buf += strlen(buf);
	                }
	                name = unmangcppfunc(buf, name, False);
                    if (*(name - 1) == '$')
                    {
                        name = unmang1(buf1, name, False);
                        strcpy(buf2, s);
                        sprintf(s, "%s %s", buf1, buf2);
                        buf = s + strlen(s);
						if (*name == '$' && *(name +1) == 'q')
						{
	                        name = unmang1(buf, ++name, True);
						}
                    }
				}
                break;
         case 't':
              name = unmangcpptype(buf,name,False);
				break;	
			case 'u':
				strcpy(buf,"unsigned ");
				buf = buf+9;
				switch(*name++) {
					case 'i':
						strcpy(buf,tn_int);
						unmangdollars(buf,&name);
						break;
					case 'l':
						strcpy(buf,tn_long);
						unmangdollars(buf,&name);
						break;
               case 'L':
                  strcpy(buf,tn_longlong);
						unmangdollars(buf,&name);
						break;
					case 's':
						strcpy(buf,tn_short);
						unmangdollars(buf,&name);
						break;
					case 'c':
						strcpy(buf,tn_char);
						unmangdollars(buf,&name);
						break;
				}
				break;
			case 'M':
                if (*name == 'n') {
                    name++ ;
                    v = *name++ - '0' ;
                    if (v > 9)
                        v -= 7 ;
                    strcpy(buf1, manglenames[v]) ;
                    p = buf1 + strlen(buf1) ;
                } else {
                    v = *name++ - '0';
                    while (isdigit(*name)) 
                        v = v*10+ *name++ - '0';
                    p = buf1 ;
                    while (v--) {
                        if (*name == '@') {
                            name++;
                            *p++ = ':';
                            *p++ = ':';
                        } else
                            *p++ = *name++;
                        *p = 0;
                    }
                    if (manglenamecount < MAX_MANGLE_NAME_COUNT)
                        strcpy(manglenames[manglenamecount ++],buf1) ;
               }

                strcpy(p, "::*");
				buf2[0] = 0;
				if (name[0] == '$')
				{
	                name = unmang1(buf2, ++name, False);
                	sprintf(buf, "(%s) (%s)", buf1, buf2);
					if (*name == '$' && *(name +1) == 'q')
					{
                        name = unmang1(buf+strlen(buf), ++name, True);
					}
				}
				else
				{
	                name = unmang1(buf2, name, False);
	                sprintf(buf, "%s %s", buf2, buf1);
				}
				break;
            case 'n':
                    v = *name++ - '0' ;
                    if (v > 9)
                        v -= 7 ;
                    strcpy(buf, manglenames[v]) ;
                    break ;
			case 'v':
				strcpy(buf,tn_void);
				break;
         case 'F':
            strcpy(buf,tn_fcomplex);
						unmangdollars(buf,&name);
            break;
         case 'D':
            strcpy(buf,tn_rcomplex);
						unmangdollars(buf,&name);
            break;
         case 'G':
            strcpy(buf,tn_lrcomplex);
						unmangdollars(buf,&name);
            break;
			case 'f':
				strcpy(buf,tn_float);
						unmangdollars(buf,&name);
				break;
			case 'd':
				strcpy(buf,tn_double);
						unmangdollars(buf,&name);
				break;
			case 'g':
				strcpy(buf,tn_longdouble);
						unmangdollars(buf,&name);
				break;
			case 'i':
				strcpy(buf,tn_int);
						unmangdollars(buf,&name);
				break;
			case 'l':
				strcpy(buf,tn_long);
						unmangdollars(buf,&name);
				break;
         case 'L':
            strcpy(buf,tn_longlong);
						unmangdollars(buf,&name);
				break;
			case 's':
				strcpy(buf,tn_short);
						unmangdollars(buf,&name);
				break;
			case 'c':
				strcpy(buf,tn_char);
						unmangdollars(buf,&name);
				break;
			case 'p':
			case 'P':
				if (*name == 'q' || *name == 'Q')
					name = unmang1(buf, name, False);
				else
				{
					name = unmangptr(buf, name-1);
    	            buf = buf + strlen(buf);
				}
				break;
			case 'r':
				name = unmang1(buf, name,False);
				buf = buf + strlen(buf);
				*buf++='&';
				*buf = 0;
				break;
			case 'e':
				strcpy(buf,tn_ellipse);
				break;
			case '$':
				name--;
				return name;
		}
	if (cconst)
		strcat(buf, tn_const);
	if (cvol)
		strcat(buf, tn_vol);
	return name;
}
/* Unmangle an entire C++ function */
static char * unmangcppfunc(char *buf, char *name, int firsttime)
{
	int i;
	*buf++ = '(';
	while (*name && *name != '$') {
		name = unmang1(buf,name,firsttime);
		buf = buf+strlen(buf);
		if (*name && *name != '$') {
			*buf++ = ',';
		}	
		else {
			*buf++ = ')';	
		}
	}
	if (*name && *name == '$')
		name++;
	*buf=0;
	return name;
}
static char * unmangcpptype(char *buf, char *name, int firsttime)
{
	int i;
   *buf++ = '<';
   while (*name && *name != '$' && *name != '@' && *name != '#') {
		name = unmang1(buf,name,firsttime);
		buf = buf+strlen(buf);
      if (*name && *name != '$' && *name != '@' && *name != '#') {
			*buf++ = ',';
		}	
		else {
         *buf++ = '>';
		 *buf++ = ' ' ;  
		}
	}
   if (*name && *name == '$')
		name++;
	*buf=0;
	return name;
}
static void xlate_cppname(char **buf, char **name, char *lastname)
{
   char classname[256],*p=classname;
	*p = 0;
	if (**name == '@')
		(*name)++;
	if (**name == '$')
		cpp_unmang_intrins(buf,name,lastname);
	else {
      while (**name == '@') (*name)++ ;
      if (**name == '#')
         (*name)++ ;
		while (**name != '$' && **name) {
			if (**name == '@') {
				*(*buf)++ = ':';
				*(*buf)++ = ':';
				*p = 0;
		 		xlate_cppname(buf,name,classname);
			}
         else if (**name == '#')
            (*name)++ ;
         else
            *p++ = *(*buf)++ = *(*name)++;
		}
	}
	**buf = 0;
}
/* Name unmangling in general */
char *unmangle(char *name)
{
   static char val[512];
   char *buf = val ;
   char classname[256];
   strcpy(val, name);
   return val;
   val[0] = 0 ;
	classname[0] = 0;
    manglenamecount = 0 ;
   if (name[0] == '_') {
		strcpy(buf,&name[1]);
	}
	else 
      if (name[0] != '@' && name[0] != '#')
			strcpy(buf,name);
		else {
         int done = False ;
         xlate_cppname(&buf,&name,0);
         while (!done && *name) {
            if (*name == '$') {
               name++;
               done = !(*name == 't') ;
               name = unmang1(buf,name,True);
               buf += strlen(buf) ;
            }
            else if (*name == '@')  {
               name++ ;
               if (*name != '$' || name[1] == 'o' || name[1] == 'b') {
                  char aa[256] ;
                  buf[0] = 0 ;
                  strcpy(aa,val) ;
                  buf[0] = buf[1] = ':' ;
                  buf += 2 ;
                  buf[0] = 0 ;
                  xlate_cppname(&buf,&name,aa) ;
               } 
            }
            else if (*name == '#')
               name++ ;
            else {
               done = True ;
               *buf++ = 0;
            }
         }
		}
   return val ;
}