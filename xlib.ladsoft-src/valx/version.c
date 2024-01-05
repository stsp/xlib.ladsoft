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
#include <windows.h>

/*
 * This basically pulls one of the version strings out of the stringtable block
 * of the version resource.
 *
 * We are doing it out the long way for compatibility with wdosx
 */
static int strsize(short *str)
{
   int count = 0 ;
   while (*str++) count++ ;
   return count ;
}
int VersionString(char *buf, short *key, short *lang)
{
   int rv = 0 ;
   HMODULE handle = GetModuleHandle(0) ;
   HRSRC rsrc = FindResource(handle,(LPCTSTR)1,RT_VERSION) ;
   DWORD size = SizeofResource(handle,rsrc) ;
   HGLOBAL global = LoadResource(handle,rsrc) ;
   if (global) {
      char *s = LockResource(global) ;
      if (s) {
         if (!memcmp(s+6,L"VS_VERSION_INFO",32)) {
            char *block = s + 0x5c ;
            if (!memcmp(block + 6,L"StringFileInfo",30)) {
               int xsize ;
               block = block + 0x24 ;
               while (memcmp(block+6,lang,strsize(lang)*2 + 2)) {
                  block = block + *(short *) block ;
                  if (block >= s+size)
                     goto done ;
               }
               xsize = *(short *)block - 0x24;
               block = block + strsize(lang) * 2 + 2 + 6 ;
               while (xsize>0 && memcmp(block+6,key,strsize(key)*2)) {
                  int len = *(short *)block ;
                  if (len %4) len += 2 ;
                  xsize -= len ;
                  block = block + len ;
               }
               if (xsize) {
                  short *aa ;
                  int len = *(short *)(block+2) ;
                  int len1 = 6 + strsize(key)*2 + 2;
                  if (len1 %4) len1 += 2 ;
                  aa = block +len1;
                  while (len--)  {
                     *buf++ = *aa ++ ;
                  }
                  *buf = 0 ;
                  rv = 1 ;

               }
            }
         }
done:
      }
      FreeResource(global) ;
   }
   return rv ;
}
