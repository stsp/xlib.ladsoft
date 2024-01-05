/* 
IMPORT librarian
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
#include <stdio.h>
#include <string.h>
#include "cmdline.h"
#include "umem.h"
#include "module.h"
#include "dict.h"

extern HASHREC **publichash;
extern MODULE *modules;

long dictofs, dictpages, pagesize = 16;
BYTE *dictionary;

int primes[] = 
{
    1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67,
        71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139,
        149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223,
        227, 229, 233, 239, 241, 251
};
#define PRIMESIZE sizeof(primes)/sizeof(int)

static void LibHash(const char *name, int blocks, HASH *h)
{
    int len = strlen(name);
    const char *pb = name,  *pe = name + len;
    const ushort blank = ' ';

    hv block_x = len | blank, bucket_d = len | blank;
    hv block_d = 0, bucket_x = 0;

    while (1)
    {
        ushort cback = *(--pe) | blank;
        ushort cfront = *(pb++) | blank;
        bucket_x = ROTR(bucket_x, 2) ^ cback;
        block_d = ROTL(block_d, 2) ^ cback;
        if (--len == 0)
            break;
        block_x = ROTL(block_x, 2) ^ cfront;
        bucket_d = ROTR(bucket_d, 2) ^ cfront;
    }
    h->block_x = block_x % blocks;
    h->block_d = block_d % blocks;
    h->block_d = MAX(h->block_d, 1);
    h->bucket_x = bucket_x % LIB_BUCKETS;
    h->bucket_d = bucket_d % LIB_BUCKETS;
    h->bucket_d = MAX(h->bucket_d, 1);
}

//-------------------------------------------------------------------------

void CalculateDictionary(void)
{
    int i, count = 0, size = 0;
    MODULE *m = modules;
    long pos = pagesize, dictpages2, dictpages1;
    for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
        PUBLIC *p = publichash[i];
        while (p)
        {
            int xx;
            count++;
            xx = strlen(p->name) + 1;
            if (xx &1)
                xx++;
            size += xx;
            p = p->link;
        }
    }
    dictpages1 = (count + LIB_BUCKETS) / LIB_BUCKETS;
    dictpages2 = (size + (512-38)) / (512-38) + 1;
    dictpages = dictpages1 > dictpages2 ? dictpages1 : dictpages2;

    restart: if (dictpages > primes[PRIMESIZE - 1])
        fatal("Library dictionary too large");
    for (i = 0; i < PRIMESIZE; i++)
        if (primes[i] >= dictpages)
            break;
    dictpages = primes[i];
    while (m)
    {
        long t;
        m->offset = pos;
        pos += m->len;
        t = pos % pagesize;
        t = t ? pagesize - t: 0;
        pos += t;
        m = m->link;
    }
    dictofs = pos + pagesize;
    dictofs = (dictofs + 511) &0xfffffe00;
    dictionary = AllocateMemory(dictpages *512);
    for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
        PUBLIC *p = publichash[i];
        while (p)
        {
            HASH lhr;
            BOOL put = FALSE;
            int startblock;
            LibHash(p->name, dictpages, &lhr);
            startblock = lhr.block_x;
            do
            {
                BYTE *dptr;
                int startbucket = lhr.bucket_x;
                dptr = dictionary + lhr.block_x * 512;
                do
                {
                    if (!dptr[lhr.bucket_x])
                    {
                        int start;
                        if (!dptr[LIB_FREEPOS])
                            start = 38;
                        else
                            start = dptr[LIB_FREEPOS] *2;
                        if (512-start >= strlen(p->name) + 3)
                        {
                            int i = strlen(p->name);
                            long ofs = p->mod->offset;
                            char *q = p->name;
                            dptr[lhr.bucket_x] = start / 2;
                            dptr[start++] = i;
                            while (i--)dptr[start++] =  *q++;
                            ofs /= pagesize;
                            dptr[start++] = (ofs) &0xff;
                            dptr[start++] = (ofs >> 8) &0xff;
                            ofs *= pagesize;
                            if (start &1)
                                start++;
                            if (start == 512)
                                start -= 2;
                            dptr[LIB_FREEPOS] = start / 2;
                            put = TRUE;
                            break;
                        }
                        else
                        {
                            dptr[LIB_FREEPOS] = 0xff;
                            goto nextblock;
                        }
                    }
                    lhr.bucket_x += lhr.bucket_d;
                    if (lhr.bucket_x >= LIB_BUCKETS)
                        lhr.bucket_x -= LIB_BUCKETS;
                }
                while (startbucket != lhr.bucket_x)
                    ;
                if (put)
                    break;
                nextblock: lhr.block_x += lhr.block_d;
                if (lhr.block_x >= dictpages)
                    lhr.block_x -= dictpages;
            }
            while (startblock != lhr.block_x)
                ;
            if (!put)
            {
                dictpages++;
                goto restart;
            }
            p = p->link;
        }
    }
}

//-------------------------------------------------------------------------

void WriteDictionary(FILE *file)
{
    int i;
    for (i = 0; i < dictpages; i++)
    {
        BYTE *p = dictionary + i * 512;
        fwrite(p, 1, 512, file);
    }
}
