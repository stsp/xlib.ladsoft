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

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <ctype.h>
#include "operands.h"
#include "header.h"
#include "utype.h"

BOOL useim;

void SegmentDisplay(char *buffer, long value)
{
    sprintf(buffer, "%x", value);
}

//-------------------------------------------------------------------------

void FormatValue(char *buffer, OPERAND *record, uint segs, uint type)
{
    char buf[256];
    DEBUG_INFO *dbg;
    char *typetab,  *symtab;
    buf[0] = 0;
    if (record->address)
        segs++;
    FindSymbolByAddress(&dbg, &typetab, &symtab, NULL, record->address, buf);
    switch (type)
    {
        case SY_SIGNEDOFS:
            if (record->address < 0)
                sprintf(buffer, "-%x", (BYTE)ABS(record->address));
            else
                sprintf(buffer, "+%x", (BYTE)record->address);
            break;
        case SY_WORDOFS:
            sprintf(buffer, "+%lx", record->address);
            break;
        case SY_BYTEOFS:
            sprintf(buffer, "+%x", (BYTE)record->address);
            break;
        case SY_ABSOLUTE:
            if (buf[0])
                sprintf(buffer, "%lx:%s", record->address, buf);
            else
                sprintf(buffer, "%lx", record->address);
            break;
        case SY_SIGNEDIMM:
            if (record->address < 0)
                sprintf(buffer, "-%x", ABS(record->address));
            else
                sprintf(buffer, "+%x", record->address);
            break;
        case SY_WORDIMM:
            if (useim)
                sprintf(buffer, "%lx", record->address);
            else
                sprintf(buffer, "offset %lx", record->address);
            break;
        case SY_BYTEIMM:
            sprintf(buffer, "%x", (BYTE)(record->address));
            break;
        case SY_PORT:
            sprintf(buffer, "%x", (BYTE)record->address);
            break;
        case SY_INTR:
            sprintf(buffer, "%x", (BYTE)record->address);
            break;
        case SY_RETURN:
            sprintf(buffer, "%x", (uint)record->address);
            break;
        case SY_ABSBRANCH:
            if (buf[0])
                sprintf(buffer, "%lx:%s", record->address, buf);
            else
                sprintf(buffer, "%lx", record->address);
            break;
        case SY_LONGBRANCH:
            if (buf[0])
                sprintf(buffer, "%lx:%s", record->address, buf);
            else
                sprintf(buffer, "%lx", record->address);
            break;
        case SY_SHORTBRANCH:
            if (buf[0])
                sprintf(buffer, "%lx:%s", record->address, buf);
            else
                sprintf(buffer, "%lx", record->address);
            break;
        case SY_SHIFT:
            sprintf(buffer, "%x", (BYTE)record->address);
            break;
        case SY_SEGMENT:
            SegmentDisplay(buffer, record->segment);
            break;
    }
}

//-------------------------------------------------------------------------

void AddSymbol(OPERAND *record, uint type)
{
    record++;
    type++;
}
