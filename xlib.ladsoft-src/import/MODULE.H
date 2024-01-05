/* 
IMPORT librarian
Copyright 2001-2011 David Lindauer.

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

You may contact the author at:
	mailto::camille@bluegrass.net
 */
#define MAX_NAMESIZE 32

#define SCAN 1
#define RESOLVE 2

/* Values returned by the record load function */
#define MODERR -1	/* Error this record */
#define MODQUIT -2	/* Never encountered */
#define MODIGNORE -3	/* Record was ignored */
#define LIBEND -4	/* End of library modules record found */

#define NAMEMAX 64

typedef struct _modlist_
{
    struct _modlist_ *link;
    char *name;
    char *modname;
    BYTE *data;
    long len;
    long offset;
} MODULE;

typedef struct _public_
{
    struct _public_ *link;
    char *name;
    MODULE *mod;
} PUBLIC;
