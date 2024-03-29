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
#include <ctype.h>

int pstrncmp(short *str1, short *str2, int n)
{
    while (*str1++ ==  *str2++ && --n)
        ;
    if (!n)
        return 0;
    return (*--str1 >  *--str2) ? 1 :  - 1;

}

//-------------------------------------------------------------------------

int pstrcmp(short *str1, short *str2)
{
    while (*str1 &&  *str1 ==  *str2)
    {
        str1++;
        str2++;
    }
    if (*(str1) == 0)
        if (*(str2) == 0)
            return 0;
        else
            return  - 1;
    if (str1 > str2)
        return 1;
    else
        return  - 1;
}

//-------------------------------------------------------------------------

void pstrcpy(short *str1, short *str2)
{
    while (*str2)
        *str1++ =  *str2++;
    *str1 = 0;
}

//-------------------------------------------------------------------------

void pstrncpy(short *str1, short *str2, int len)
{
    memcpy(str1, str2, len *sizeof(short));
}

//-------------------------------------------------------------------------

void pstrcat(short *str1, short *str2)
{
    while (*str1++)
        ;
    str1--;
    while (*str2)
        *str1++ =  *str2++;
    *str1++ = 0;
}

//-------------------------------------------------------------------------

short *pstrchr(short *str, short ch)
{
    while (*str &&  *str != ch)
        str++;
    if (*str)
        return str;
    return 0;
}

//-------------------------------------------------------------------------

short *pstrrchr(short *str, short ch)
{
    short *start = str;
    while (*str++)
        ;
    str--;
    while (str != start - 1 &&  *str != ch)
        str--;
    if (str != start - 1)
        return str;
    return 0;
}

//-------------------------------------------------------------------------

int pstrlen(short *s)
{
    int len = 0;
    while (*s++)
        len++;
    return len;
}

//-------------------------------------------------------------------------

short *pstrstr(short *str1, short *str2)
{
    while (1)
    {
        short *pt = pstrchr(str1, str2[0]);
        if (!pt)
            return 0;
        if (!pstrncmp(pt, str2, pstrlen(str2)))
            return pt;
        str1 = pt + 1;
    }
}

/* warning!!!! does not cope with phi-text */
short *pstrupr(short *str)
{
    short *p = str;
    while (*p)
    {
        *p = toupper(*p);
        p++;
    }
}
