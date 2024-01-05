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
