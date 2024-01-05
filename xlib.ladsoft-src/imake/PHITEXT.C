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
#include <string.h>
#include <memory.h>
#include <ctype.h>
#include "utype.h"
#include "interp.h"

extern BYTE startchars[];
extern BYTE symchars[];
extern BYTE whitespacechars[];
extern BYTE commentchars[];

PHITEXT phiparms = 
{
    4, 0, 0x0f, 0, 0, 0, 0x7f
};
BOOL phiused = FALSE;
static BOOL putback = FALSE;

int currentphifg = 0;

static PHITEXT phidefaults = 
{
    4, 0, 0x0f, 0, 0, 0, 0x7f
};
static int pbchar = 0;

int repeat = 0;

void PhiInit(void)
{
    phiparms = phidefaults;
    putback = FALSE;
    phiused = FALSE;
    currentphifg = phidefaults.fgc;
}

//-------------------------------------------------------------------------

void BadString(void)
{
    Error("Bad escape sequence in string");
}

//-------------------------------------------------------------------------

static BOOL phichar(BYTE *ch, FILE *file)
{
    while (TRUE)
    {
        *ch = fgetc(file);
        if (feof(file))
            return (FALSE);
        if (*ch == 0x1f)
        {
            phiused = TRUE;
            continue;
        }
        break;
    }
    return (TRUE);
}

//-------------------------------------------------------------------------

BOOL philine(BYTE *buf, int len, FILE *file)
{
    if (len < 2 || !phichar(buf, file))
    {
        *buf = 0;
        return (FALSE);
    }
    while (len > 2)
    {
        if (*buf == 0x0a || (phiused && (*buf == 0x14 ||  *buf == 0x15 ||  *buf
            == 0x16)))
            break;
        if (!phichar(++buf, file))
            break;
        len--;
    }
    *buf++ = 0x0a;
    *buf++ = 0;
    return (TRUE);
}

//-------------------------------------------------------------------------

short parsechar(BYTE **buf)
{
    short rv;
    if (putback)
    {
        (*buf)++;
        putback = FALSE;
        return (pbchar);
    }
    if (!phiused)
        return (*(*buf)++);
    while (TRUE)
    {
        if (repeat)
        {
            repeat--;
            rv = phiparms.cwb + (phiparms.bank << 7);
            break;
        }
        rv = *(*buf)++;
        if (rv &0x80)
        {
            char grp = rv &0x70;
            switch (grp)
            {
                case 0:
                    {
                        int repeatlevel = 0;
                        (*buf)--;
                        while (((rv =  * * buf) &0xf0) == 0x80)
                        {
                            (*buf)++;
                            repeat += ((rv &0x0f) << (repeatlevel++ *4)) + 1;
                        }
                        repeat++;
                    }
                    break;
                case 0x10:
                    phiparms.bank = rv &0x0f;
                    break;
                case 0x20:
                case 0x30:
                    phiparms.attrib = rv &0x1f;
                    break;
                case 0x40:
                    phiparms.fgc = rv &0x0f;
                    break;
                case 0x50:
                    phiparms.bgc = rv &0x0f;
                    break;
                case 0x60:
                    phiparms.style = rv &0x0f;
                    break;
                case 0x70:
                    phiparms.size = rv &0x0f;
                    break;
            }
        }
        else
        {
            if (rv)
                phiparms.cwb = rv;
            if (rv &0x60)
            {
                if (phiparms.attrib &HIDDEN || phiparms.style)
                    continue;
                rv = rv + (phiparms.bank << 7);
            }
            else
                if (rv == 0x0a)
                    phiparms = phidefaults;
            break;
        }
    }
    return (rv);
}

//-------------------------------------------------------------------------

void putphiback(short ch)
{
    putback = TRUE;
    pbchar = ch;
}

//-------------------------------------------------------------------------

BOOL isstartchar(short val)
{
    int bit = 1 << (7-(val &0x07));
    int byte = val >> 3;
    return (startchars[byte] &bit);
}

//-------------------------------------------------------------------------

BOOL issymchar(short val)
{
    int bit = 1 << (7-(val &0x07));
    int byte = val >> 3;
    return (symchars[byte] &bit);
}

//-------------------------------------------------------------------------

BOOL iswhitespacechar(short val)
{
    int bit = 1 << (7-(val &0x07));
    int byte = val >> 3;
    return (whitespacechars[byte] &bit);
}

//-------------------------------------------------------------------------

BOOL iscommentchar(short val)
{
    int bit = 1 << (7-(val &0x07));
    int byte = val >> 3;
    return (commentchars[byte] &bit);
}

//-------------------------------------------------------------------------

static BYTE *getstringchar(BYTE *rv, BYTE *bufptr)
{
    putback = FALSE;
    if (phiused)
    {
        if (repeat)
        {
            repeat--;
            *rv = phiparms.cwb;
            return (bufptr);
        }
        if ((*bufptr > 0x7f) || phiparms.bank)
        {
            *rv =  *bufptr;
            if (*rv &0x80)
            {
                char grp =  *rv &0x70;
                switch (grp)
                {
                    case 0:
                        break;
                    case 0x10:
                        phiparms.bank =  *rv &0x0f;
                        break;
                    case 0x20:
                    case 0x30:
                        phiparms.attrib =  *rv &0x1f;
                        break;
                    case 0x40:
                        phiparms.fgc =  *rv &0x0f;
                        break;
                    case 0x50:
                        phiparms.bgc =  *rv &0x0f;
                        break;
                    case 0x60:
                        phiparms.style =  *rv &0x0f;
                        break;
                    case 0x70:
                        phiparms.size =  *rv &0x0f;
                        break;
                }
            }
            return (++bufptr);
        }
    }

    if (*bufptr == '\\')
    {
        bufptr++;
        if (isdigit(*bufptr) && (*bufptr < '8'))
        {
            unsigned temp = 0;
            while (isdigit(*bufptr) &&  *bufptr < '8')
                temp = (temp << 3) + (*bufptr++ - '0');
            if (temp > 255)
                BadString();

            *rv = (BYTE)temp;
        }
        else
        switch (*bufptr++)
        {
            case 'b':
                *rv = 8;
                break;
            case 'f':
                *rv = 12;
                break;
            case 'n':
                *rv = 10;
                break;
            case 'r':
                *rv = 13;
                break;
            case 't':
                *rv = 9;
                break;
            case '\'':
                *rv = '\'';
                break;
            case '"':
                *rv = '"';
                break;
            case '\\':
                *rv = '\\';
                break;
            case 'x':
                {
                    unsigned temp = 0;
                    while (isxdigit(*bufptr))
                    {
                        temp = (temp << 4) + (*bufptr - '0');
                        if (*bufptr++ > '9')
                            temp -= 7;
                    }
                    if (temp > 255)
                        BadString();
                    *rv = (BYTE)temp;
                }
                break;
            default:
                BadString();
        }
    }
    else
        *rv =  *bufptr++;
    return (bufptr);
}

//-------------------------------------------------------------------------

short getphistring(BYTE *obuf, BYTE **ibuf, short endchar)
{
    BYTE rv;
    if (phiused)
    {
        if (phidefaults.size != phiparms.size)
            *obuf++ = 0xf0 | phiparms.size;
        if (phidefaults.style != phiparms.style)
            *obuf++ = 0xe0 | phiparms.style;
        if (phidefaults.bgc != phiparms.bgc)
            *obuf++ = 0xd0 | phiparms.bgc;
        if (phidefaults.fgc != phiparms.fgc)
            *obuf++ = 0xc0 | phiparms.fgc;
        if (phidefaults.attrib != phiparms.attrib)
            *obuf++ = 0xa0 | phiparms.attrib;
        if (repeat)
        {
            BadString();
            repeat = 0;
            return (endchar);
        }
    }
    *ibuf = getstringchar(&rv,  *ibuf);
    while (rv && ((rv + (phiparms.bank << 7)) != endchar))
    {
        if (phiused && (rv < 0x20))
        {
            if ((rv == 0x0a))
            {
                while (((BYTE)*(obuf - 1)) &0x80)
                    obuf--;
            }
        }
        *obuf++ = rv;
        if (phiused && (rv == 0x0a))
        {
            if (phidefaults.size != phiparms.size)
                *obuf++ = 0xf0 | phiparms.size;
            if (phidefaults.style != phiparms.style)
                *obuf++ = 0xe0 | phiparms.style;
            if (phidefaults.bgc != phiparms.bgc)
                *obuf++ = 0xd0 | phiparms.bgc;
            if (phidefaults.fgc != phiparms.fgc)
                *obuf++ = 0xc0 | phiparms.fgc;
            if (phidefaults.attrib != phiparms.attrib)
                *obuf++ = 0xa0 | phiparms.attrib;
        }
        *ibuf = getstringchar(&rv,  *ibuf);
    }
    *obuf = 0;
    phiparms.cwb = rv;
    return (rv + (phiparms.bank << 7));
}

//-------------------------------------------------------------------------

long getphichar(BYTE **ibuf)
{
    BYTE rv;
    *ibuf = getstringchar(&rv,  *ibuf);
    if (!phiused)
        return (rv);
    while (rv &0x80)
        *ibuf = getstringchar(&rv,  *ibuf);
    if (!rv)
        return (0);
    phiparms.cwb = rv;
    return (((long)phiparms.size << 28) + ((long)phiparms.style << 24) + ((long)
        phiparms.fgc << 20) + ((long)phiparms.bgc << 16) + ((long)
        phiparms.attrib << 11) + ((long)phiparms.bank << 7) + (rv &0x7f));
}

//-------------------------------------------------------------------------

void phifg(int color, BYTE *file)
{
    currentphifg = color;
    if (phiused)
        fputc(color | 0xc0, file);
}

//-------------------------------------------------------------------------

short phiupper(short val)
{
    if (val > 0x60 && val < 0x7b)
        return val &~0x20;
    return (val);
}

//-------------------------------------------------------------------------

int installphichar(short curchar, BYTE *buf, int i)
{
    short rv = 0;
    int bank = curchar >> 7;
    int cwb = curchar &0x7f;
    buf = buf + i;
    if (!phiused)
    {
        *buf = curchar;
        return (1);
    }
    if (i)
    {
        buf--;
        if (((*buf) &0xf0) == 0x90)
        if ((*buf &0x0f) != bank)
        {
            *buf++ = 0x90 | bank;
        }
        else
            rv--;
        else
        {
            buf++;
            if (bank)
            {
                *buf++ = 0x90 | bank;
                rv++;
            }
        }
    }
    else
    if (bank)
    {
        *buf++ = 0x90 | bank;
        rv++;
    }
    *buf++ = cwb;
    rv++;
    if (bank)
    {
        *buf++ = 0x90 | bank;
        rv++;
    }
    return (rv);
}

//-------------------------------------------------------------------------

short phistreamtoflat(BYTE *out, BYTE *in, int size, BOOL useparms)
{
    int count = 0, fcount;
    long rv;
    PHITEXT oparms = phiparms;
    PHITEXT lastphiparms = phiparms;
    int orepeat = repeat;

    if (!phiused)
    {
        count = strlen(in);
        strcpy(out, in);
        return (count);
    }
    repeat = 0;
    while (rv = parsechar(&in))
    {
        if ((rv > 0x1f) && useparms)
            rv |= ((long)phiparms.attrib << 11) + ((long)phiparms.fgc << 16) + 
                ((long)phiparms.bgc << 20) + ((long)phiparms.style << 24) + (
                (long)phiparms.size << 28);
        if (rv == 0x0a || rv == 0x14 || rv == 0x15 || rv == 0x16)
            phiparms = lastphiparms;
        if (size > 2)
        {
            out[count++] = rv >> 24;
            out[count++] = (rv >> 16) &0xff;
        }
        if (size > 1)
            out[count++] = (rv >> 8) &0xff;
        out[count++] = (rv) &0xff;
        lastphiparms = phiparms;
    }
    fcount = count;
    out[count++] = 0;
    if (size > 1)
        out[count++] = 0;
    if (size > 2)
    {
        out[count++] = 0;
        out[count++] = 0;
    }

    repeat = orepeat;
    phiparms = oparms;
    return (fcount);
}

//-------------------------------------------------------------------------

int phicmp(BYTE *str1, BYTE *str2)
{
    BYTE buf1[400], buf2[400],  *p1 = buf1,  *p2 = buf2;
    int count1, count2;
    if (!phiused)
        return (strcmp(str1, str2));

    count1 = phistreamtoflat(buf1, str1, FALSE, FALSE);
    count2 = phistreamtoflat(buf2, str2, FALSE, FALSE);

    while (count1 && count2)
    {
        int val1 = ((*p1++) << 8) +  *p1++;
        int val2 = ((*p2++) << 8) +  *p2++;
        if (val1 > val2)
            return (1);
        if (val1 < val2)
            return ( - 1);
        count1--;
        count2--;
    }
    if (count1 == count2)
        return (0);
    if (count1)
        return (1);
    return (2);
}
