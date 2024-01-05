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
#include <stdio.h>

/*
 * note that the environment list as an argument to main() 
 * is an extension to C, it is *not* something standard but is 
 * generally present in MSDOS C compilers.
 *
 * for portablity, use the 'getenv()' function to get environment variable
 * values
 */
 extern char * _osenv,*_environ ;
int main(int argc, char *argv[], char *env[])
{
  int i;
  char **ptr;

  printf("Argument list: %d\n",argc);
  for (i=0; i < argc; i++)
    printf("\t%d: \042%s\042\n",i,argv[i]);

  ptr = env;
  i = 0;
  printf("Environment:\n") ;
  while (*ptr)
    printf("\t%d: \042%s\042\n",i++,*ptr++);
}