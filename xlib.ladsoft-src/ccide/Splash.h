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
/* DESCRIPTION    : Structures/definitions for splash screen functions        */
/* LANGUAGE       : "C"                                                       */
/* PLATFORM       : Windows 95/98/98SE/ME/NT4/2000                            */
/* AUTHOR         : Andrea Binello  <andrea.binello@tiscalinet.it>            */
/* LICENSE        : Freeware - Open Source                                    */
/* LAST UPDATE    : May 06, 2001                                              */
/******************************************************************************/

#ifndef __SPLASH_H__
    #define __SPLASH_H__

    #ifdef __cplusplus
        extern "C"
        {
        #endif 

        typedef struct
        {
            HWND hWndOwner;
            HINSTANCE hInstance;
            HINSTANCE hInstanceRes;
            LPSTR lpszResource;
            USHORT uTime;
            BOOL bCentered;
            BOOL bTopmost;
            BOOL bWait;
            BOOL bAbout;
            INT iPosX;
            INT iPosY;
            RECT bPos;
            LPSTR lpszButtonTitle;
            LPSTR lpszVersion;
        } SPLASH, FAR *LPSPLASH;

        VOID WINAPI SplashScreen(LPSPLASH lpSplash);

        #ifdef __cplusplus
        }
    #endif 

#endif
