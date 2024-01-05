/* 
CC386 C Compiler
Copyright 1994-2011 David Lindauer.

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
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "ccerr.h"
#include "cvinfo.h"
int dbgblocknum;
void dbginit(void){}
void debug_startenum(SYM *sp){}
void debug_endenum(void){}
void debug_enumvalue(SYM *sp){}
void debug_outputtypedef(SYM *sp){}
void debug_outputtype(TYP *head){}
void debug_outfunc(SYM *sp){}
void debug_beginblock(TABLE lsyms){}
void debug_endblock(TABLE syms){}
void debug_outdata(SYM *sp, int BSS){}
void debug_dumpglobaltypedefs(void) {}
void output_obj_file(void){}
void dump_browsedata(char *s, int len){}
void outcode_file_init(void){}
struct amode *HandleIntrins(ENODE *node, int novalue)
{
    return 0;
}
