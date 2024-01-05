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
#include "c_types.h"
typedef void (__stdcall  * sym_callback_t)(char *symname, char *filename, 
							   int linestart, int lineend, 
							   type_t *typeinfo, void *userdata);
typedef int (__stdcall  * file_callback_t)(char *filename, int sys_inc,
							   void **filefunc, void **filearg, 
							   unsigned char *linesused, int lines);

typedef void *(__stdcall * alloc_callback_t)(void *ptr_or_size, int alloc);

#ifndef DLLMAIN
void WINAPI _import parse(char *filename, file_callback_t fcallback,
				  	alloc_callback_t acallback,
				  	char *sysinc, char *userinc,
					int defineCount, char *defines[], sym_callback_t callback, 
					void *userdata);
int WINAPI _import enumTypes(type_t *type, sym_callback_t usercallback, void *userdata);
#endif
