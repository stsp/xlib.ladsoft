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

#include "header.h"
#include "pefile.h"
static struct pe_header_struct PEHead;
static int base;
int IsPEFile(char *filename)
{
    FILE *fil = fopen(filename, "rb");
    char buf[64];
    long pos;
    if (!fil)
        return 0;
    memset(buf, 64, 0);
    fread(buf, 64, 1, fil);
    pos = *(int*)(buf + 0x3c);
    fseek(fil, pos, SEEK_SET);
    fread(buf, 2, 1, fil);
    fclose(fil);

    if (buf[0] == 'P' && buf[1] == 'E')
        return 1;
    return  - 1;

} 
/* assumes FindExitProcessAddress has been called */
int GetEntryPoint(void)
{
    return PEHead.entry_point + base;
}

//-------------------------------------------------------------------------

int FindExitProcessAddress(HANDLE hProcess, int imagebase)
{
    struct pe_import_dir_struct PEImport;
    int dir_address;

    base = imagebase;
    ReadProcessMemory(hProcess, (LPVOID)(imagebase + 0x3c), (LPVOID)
        &dir_address, 4, 0);

    ReadProcessMemory(hProcess, (LPVOID)(imagebase + dir_address), (LPVOID)
        &PEHead, sizeof(struct pe_header_struct), 0);
    dir_address = PEHead.import_rva;
    if (dir_address == 0)
        return 0;
    do
    {
        ReadProcessMemory(hProcess, (LPVOID)(imagebase + dir_address), (LPVOID)
            &PEImport, sizeof(struct pe_import_dir_struct), 0);
        if (PEImport.dllName)
        {
            char buf[256];
            ReadProcessMemory(hProcess, (LPVOID)(imagebase + PEImport.dllName),
                (LPVOID)buf, 256, 0);
            if (!xstricmpz(buf, "KERNEL32.DLL"))
            {
                int namepos = PEImport.thunkPos2 + imagebase;
                int addrpos = PEImport.thunkPos + imagebase;
                do
                {
                    int nametext;
                    ReadProcessMemory(hProcess, (LPVOID)namepos, buf, 4, 0);
                    nametext = *(int*)buf + imagebase;
                    if (nametext == 0)
                        return 0;
                    ReadProcessMemory(hProcess, (LPVOID)nametext, buf, 256, 0);
                    if (!strcmp(buf + 2, "ExitProcess"))
                    {
                        ReadProcessMemory(hProcess, (LPVOID)addrpos, buf, 4, 0);
                        return *(int*)buf;
                    } namepos += 4;
                    addrpos += 4;
                }
                while (TRUE);

            }
            dir_address += sizeof(struct pe_import_dir_struct);
        }

    }
    while (PEImport.dllName)
        ;

    return 0;
}
//-------------------------------------------------------------------------

int FindLSCRTLExitAddress(HANDLE hProcess, int imagebase)
{
    struct pe_import_dir_struct PEImport;
    int dir_address;

    base = imagebase;
    ReadProcessMemory(hProcess, (LPVOID)(imagebase + 0x3c), (LPVOID)
        &dir_address, 4, 0);

    ReadProcessMemory(hProcess, (LPVOID)(imagebase + dir_address), (LPVOID)
        &PEHead, sizeof(struct pe_header_struct), 0);
    dir_address = PEHead.import_rva;
    if (dir_address == 0)
        return 0;
    do
    {
        ReadProcessMemory(hProcess, (LPVOID)(imagebase + dir_address), (LPVOID)
            &PEImport, sizeof(struct pe_import_dir_struct), 0);
        if (PEImport.dllName)
        {
            char buf[256];
            ReadProcessMemory(hProcess, (LPVOID)(imagebase + PEImport.dllName),
                (LPVOID)buf, 256, 0);
            if (!xstricmpz(buf, "KERNEL32.DLL"))
            {
                int namepos = PEImport.thunkPos2 + imagebase;
                int addrpos = PEImport.thunkPos + imagebase;
                do
                {
                    int nametext;
                    ReadProcessMemory(hProcess, (LPVOID)namepos, buf, 4, 0);
                    nametext = *(int*)buf + imagebase;
                    if (nametext == 0)
                        return 0;
                    ReadProcessMemory(hProcess, (LPVOID)nametext, buf, 256, 0);
                    if (!strcmp(buf + 2, "ExitProcess"))
                    {
                        ReadProcessMemory(hProcess, (LPVOID)addrpos, buf, 4, 0);
                        return *(int*)buf;
                    } namepos += 4;
                    addrpos += 4;
                }
                while (TRUE);

            }
            dir_address += sizeof(struct pe_import_dir_struct);
        }

    }
    while (PEImport.dllName)
        ;

    return 0;
}
