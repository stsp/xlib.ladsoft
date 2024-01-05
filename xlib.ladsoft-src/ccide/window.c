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
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <float.h>
#include "helpid.h"
#include "header.h"
#include <dir.h>
#include "wargs.h"
#include "splash.h"
#include "..\version.h"
#include <sys\stat.h>

#define INITIAL_DOCK_COUNT 9

int initialIDs[INITIAL_DOCK_COUNT] = 
{
    DID_TABWND, DID_ERRORWND, DID_WATCHWND, DID_EDITTOOL, DID_NAVTOOL, DID_BUILDTOOL,
        DID_DEBUGTOOL, DID_BOOKMARKTOOL, DID_THREADSTOOL
};
CCD_params initialDocks[INITIAL_DOCK_COUNT] = 
{
    { // tabwnd
        DOCK_LEFT, DOCK_LEFT, FALSE, 0, 0, 0, 1000 *200, 
        {
            0, 0, 230, 200
        }
        , 
        {
            0, 0, 230, 200
        }
        , 
        {
            0, 0, 230, 200
        }
        , 
        {
            0, 0, 0, 0
        }
    }
    , 
    { //info wind
        DOCK_BOTTOM, DOCK_BOTTOM, FALSE, 0, 0, 0, 1000 *500, 
        {
            0, 0, 500, 180
        }
        , 
        {
            0, 0, 500, 180
        }
        , 
        {
            0, 0, 500, 180
        }
        , 
        {
            0, 0, 0, 0
        }
    }
    , 
    { // watch wind
        DOCK_BOTTOM, DOCK_BOTTOM, TRUE, 0, 0, 0, 1000 *500, 
        {
            0, 0, 500, 160
        }
        , 
        {
            0, 0, 500, 160
        }
        , 
        {
            0, 0, 500, 160
        }
        , 
        {
            0, 0, 0, 0
        }
    }
    ,
    { // edit tool
        DOCK_TOP, DOCK_TOP, FALSE, 0, 0, 0, 0, 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 0, 0
        }
    }
    , 
    { // nav tool
        DOCK_TOP, DOCK_TOP, FALSE, 0, 0, 0, 0, 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 0, 0
        }
    }
    , 
    { // build tool
        DOCK_TOP, DOCK_TOP, FALSE, 0, 1, 0, 0, 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 0, 0
        }
    }
    , 
    { // debug tool
        DOCK_TOP, DOCK_TOP, FALSE, 0, 1, 0, 0, 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 0, 0
        }
    }
    , 
    { // bookmark tool
        DOCK_TOP, DOCK_TOP, FALSE, 0, 1, 0, 0, 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 0, 0
        }
    }
    , 
    { // threads tool
        DOCK_TOP, DOCK_TOP, FALSE, 0, 1, 0, 0, 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 200, 30
        }
        , 
        {
            0, 0, 0, 0
        }
    }
    , 
};

void CreateDocks(void)
{
    dmgrSetInfo(initialIDs, initialDocks, INITIAL_DOCK_COUNT);
}