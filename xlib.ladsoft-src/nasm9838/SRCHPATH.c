/* 
   Copyright 1994-2003 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  

   You may contact the author at:

   mailto::camille@bluegrass.net

   or by snail mail at:

   David Lindauer
   850 Washburn Ave Apt 99
   Louisville, KY 40222
*/
/*
 * Search for a file along a path list
 */
#include <stdio.h>
#include <string.h>
#include "cmdline.h"
/*
 * Pull the next path off the path search list
 */
static char *parsepath(char *path, char *buffer)
{
  char *pos = path;

  /* Quit if hit a ';' */
  while (*pos) {
		if (*pos == ';') {
			pos++;
			break;
    }
		*buffer++ = *pos++;
	}
  *buffer = 0;

  /* Return a null pointer if no more data */
  if (*pos)
	  return(pos);

  return(0);
}
/*
 * For each library:
 * Search local directory and all directories in the search path
 *  until it is found or run out of directories
 */
FILE *SearchPath(char *string, char *searchpath, char *mode)
{
		FILE *in;
		char *newpath = searchpath;

		/* Search local path */
    in = fopen((char *) string,mode);
		if (in) {
			return(in);
		}
		else {
			/* If no path specified we search along the search path */
         if (string[0] != '\\' && string[1] != ':') {
			  char buffer[200];
				while (newpath) {
					/* Create a file name along this path */
				  newpath = parsepath(newpath,buffer);
					if (buffer[strlen(buffer)-1] != '\\')
						strcat(buffer,"\\");
				  strcat(buffer, (char *)string);

					/* Check this path */
					in = fopen(buffer, mode);
					if (in) {
						strcpy(string,buffer);
						return(in);
					}
				}
			}
		}
	return(0);
}
