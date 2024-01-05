/* 
IMAKE make utility
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

You may contact the author at:
	mailto::camille@bluegrass.net
 */
#include <stdio.h>
#include "utype.h"
#include "umem.h"
#include "phash.h"
#include "interp.h"
#include "input.h"
#include "register.h"
#include "macros.h"
#include "maker.h"
#include "imake.h"

extern HASHREC **hashtable;

static void DumpMacro(REGISTRY *macro)
{
    short *buffer;
    char *buf;
    buffer = AllocateMemory(INTERNALBUFSIZE *sizeof(short));
    buf = AllocateMemory(INTERNALBUFSIZE);
    CompressFile(buf, macro->name);
    printf("%25s = ", buf);
    pstrcpy(buffer, macro->x.macro);
    ExpandString(buffer);
    CompressFile(buf, buffer);
    printf("%s\n", buf);
    DeallocateMemory(buf);
    DeallocateMemory(buffer);
}

static void DumpPaths(REGISTRY *target)
{
    short **p;
    short *buffer;
    char *buf;
    buffer = AllocateMemory(INTERNALBUFSIZE *sizeof(short));
    buf = AllocateMemory(INTERNALBUFSIZE);
    CompressFile(buf, target->name);
    printf("%s: ", buf);
	pstrcpy(buffer, target->x.path);
	ExpandString(buffer);
	CompressFile(buf, buffer);
    printf("%s\n",buf);
    DeallocateMemory(buf);
    DeallocateMemory(buffer);
}
//-------------------------------------------------------------------------

static void DumpImplicits(REGISTRY *target)
{
    short **p;
    short *buffer;
    char *buf;
    buffer = AllocateMemory(INTERNALBUFSIZE *sizeof(short));
    buf = AllocateMemory(INTERNALBUFSIZE);
    CompressFile(buf, target->name);
    printf("%s:\n", buf);
	if (target->depends)
	{
	    pstrcpy(buffer, target->depends);
    	ExpandString(buffer);
	    CompressFile(buf, buffer);
    	printf("\tDependencies:\n\t%s\n\n",buf);
	}
	printf("\tCommands:\n");
    p = target->x.commands;
    while (*p)
    {
        pstrcpy(buffer,  *p);
        ExpandString(buffer);
        CompressFile(buf, buffer);
        printf("\t%s\n", buf);
        p++;
    }
    DeallocateMemory(buf);
    DeallocateMemory(buffer);
}

//-------------------------------------------------------------------------

static void DumpExplicits(REGISTRY *target)
{
    short **p;
    short *buffer;
    char *buf;
    buffer = AllocateMemory(INTERNALBUFSIZE *sizeof(short));
    buf = AllocateMemory(INTERNALBUFSIZE);
    CompressFile(buf, target->name);
    printf("%s:\n", buf);
    pstrcpy(buffer, target->depends);
    ExpandString(buffer);
    CompressFile(buf, buffer);
    printf("\tDependencies:\n\t%s\n\n\tCommands:\n", buf);
    p = target->x.commands;
    while (*p)
    {
        pstrcpy(buffer,  *p);
        ExpandString(buffer);
        CompressFile(buf, buffer);
        printf("\t%s\n", buf);
        p++;
    }
    DeallocateMemory(buf);
    DeallocateMemory(buffer);
}

//-------------------------------------------------------------------------

void DisplayMacros(void)
{
    int count = 0;
    int i;
    REGISTRY **r = 0;
    for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
        REGISTRY *p = hashtable[i];
        while (p)
        {
            if (p->type == R_MACRO && p->isdef)
                count++;
            p = p->link;
        }
    }
    r = AllocateMemory(count *sizeof(REGISTRY*));
    count = 0;
    for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
        REGISTRY *p = hashtable[i];
        while (p)
        {
            if (p->type == R_MACRO && p->isdef)
                r[count++] = p;
            p = p->link;
        }
    }
    for (i = 0; i < count; i++)
    {
        int j;
        for (j = i + 1; j < count; j++)
        if (pstrcmp(r[i]->name, r[j]->name) > 0)
        {
            REGISTRY *t = r[i];
            r[i] = r[j];
            r[j] = t;
        }
    }
    printf("Macro dump\n");
    for (i = 0; i < count; i++)
        DumpMacro(r[i]);

    DeallocateMemory(r);
}

void DisplayPaths(void)
{
    int count = 0;
    int i;
    REGISTRY **r = 0;
    for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
        REGISTRY *p = hashtable[i];
        while (p)
        {
            if (p->type == R_PATH)
                count++;
            p = p->link;
        }
    }
    if (!count)
        return ;
    r = AllocateMemory(count *sizeof(REGISTRY*));
    count = 0;
    for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
        REGISTRY *p = hashtable[i];
        while (p)
        {
            if (p->type == R_PATH)
                r[count++] = p;
            p = p->link;
        }
    }
    for (i = 0; i < count; i++)
    {
        int j;
        for (j = i + 1; j < count; j++)
        if (pstrcmp(r[i]->name, r[j]->name) > 0)
        {
            REGISTRY *t = r[i];
            r[i] = r[j];
            r[j] = t;
        }
    }
    printf("\nPaths\n");
    for (i = 0; i < count; i++)
        DumpPaths(r[i]);

    DeallocateMemory(r);
}

//-------------------------------------------------------------------------

void DisplayImplicits(void)
{
    int count = 0;
    int i;
    REGISTRY **r = 0;
    for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
        REGISTRY *p = hashtable[i];
        while (p)
        {
            if (p->type == R_IMPLICIT)
                count++;
            p = p->link;
        }
    }
    if (!count)
        return ;
    r = AllocateMemory(count *sizeof(REGISTRY*));
    count = 0;
    for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
        REGISTRY *p = hashtable[i];
        while (p)
        {
            if (p->type == R_IMPLICIT)
                r[count++] = p;
            p = p->link;
        }
    }
    for (i = 0; i < count; i++)
    {
        int j;
        for (j = i + 1; j < count; j++)
        if (pstrcmp(r[i]->name, r[j]->name) > 0)
        {
            REGISTRY *t = r[i];
            r[i] = r[j];
            r[j] = t;
        }
    }
    printf("\nImplicit rules\n");
    for (i = 0; i < count; i++)
        DumpImplicits(r[i]);

    DeallocateMemory(r);
}
//-------------------------------------------------------------------------

void DisplayExplicits(void)
{
    int count = 0;
    int i;
    REGISTRY **r = 0;
    for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
        REGISTRY *p = hashtable[i];
        while (p)
        {
            if (p->type == R_EXPLICIT)
                count++;
            p = p->link;
        }
    }
    if (!count)
        return ;
    r = AllocateMemory(count *sizeof(REGISTRY*));
    count = 0;
    for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
        REGISTRY *p = hashtable[i];
        while (p)
        {
            if (p->type == R_EXPLICIT)
                r[count++] = p;
            p = p->link;
        }
    }
    for (i = 0; i < count; i++)
    {
        int j;
        for (j = i + 1; j < count; j++)
        if (pstrcmp(r[i]->name, r[j]->name) > 0)
        {
            REGISTRY *t = r[i];
            r[i] = r[j];
            r[j] = t;
        }
    }
    printf("\nTargets:\n");
    for (i = 0; i < count; i++)
        DumpExplicits(r[i]);

    DeallocateMemory(r);
}

//-------------------------------------------------------------------------

void DisplayDefs(void)
{
    DisplayMacros();
	DisplayPaths();
    DisplayImplicits();
    DisplayExplicits();
}
