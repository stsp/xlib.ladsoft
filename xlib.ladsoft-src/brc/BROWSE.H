/* 
Browse Linker
Copyright 2003-2011 David Lindauer.

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
#define BRS_SIGNATURE "$LS$"

#define BRS_STARTFUNC 0xc4
#define BRS_ENDFUNC 0xC5
#define BRS_STARTFILE 0xc0
#define BRS_VARIABLE 0xcc 
#define BRS_DEFINE 0xcd
#define BRS_BLOCKSTART 0xc8
#define BRS_BLOCKEND 0xc9

#define BRF_EXTERNAL 1
#define BRF_STATIC 2
#define BRF_TYPE 4

struct linedata
{
    struct linedata *link;
    int type;
    int startline;
    int endline;
	int funcendline;
    int charpos;
    int external;
    int blocklevel;
    int filenum;
	int typedata;
    char *hint;
};
struct symdata
{
    /* first two fields MUST be in this order */
    struct symdata *link;
    char *name;

    struct linedata *values;
    int externalcount;
    int globalcount;
    int argcount;
    int localcount;
    int fileoffs;
};
struct blocklist
{
    int start;
    int end;
    char **symnames;
    struct linedata **linedata;
    int max;
    int count;
};
