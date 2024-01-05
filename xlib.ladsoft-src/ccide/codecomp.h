/* 
CCIDE
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
typedef struct _type
{
	struct _type *link;
	char *name;
	char *typedefname;
	enum Basictype type;
	struct _sym *syms;
	int t_const : 1;
	int t_volatile : 1;
	int t_restrict : 1;
} TYPE;

typedef struct _sym
{
	struct _sym *link;
	struct _type *type;
	char *name;
	char *filename;
	int startline;
	int endline;
} SYM;

void deleteSymsForFile(char *name);
TYPE *parse_lookup_symtype(char *name, char *module, int line);
int parse_lookup_prototype(char *proto, char *name, int *offsets);
void deleteFileData(char *name);


