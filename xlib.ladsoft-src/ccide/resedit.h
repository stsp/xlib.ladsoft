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
#define UNDO_MAX 10

typedef struct {
    short reserved;
    short resType;
    short resCount;
} ICOCURSORHDR;

typedef struct {
    unsigned char width;
    unsigned char height;
    unsigned char colors;
    unsigned char unused;
    short hotspotX;
    short hotspotY;
    int DIBSize;
    int DIBOffset;
} ICOCURSORDESC, *PICOCURSORDESC;

typedef struct _imagedata {
    struct _imagedata *next;
    struct _imagedata *undo;
    enum { FT_BMP, FT_ICO, FT_CUR } type;
    int imageDirty : 1;
    int fileDirty : 1;
    int height;
    int width;
    int colors;
    int hotspotX;
    int hotspotY;
    int DIBSize;
    HBITMAP hbmpImage;
    HDC hdcImage;
    HBITMAP hbmpAndMask;
    HDC hdcAndMask;
    unsigned char *DIBInfo;
    DWORD rgbScreen;
} IMAGEDATA;

typedef struct _rubber
{
    enum { RT_LINE, RT_RECTANGLE, RT_ELLIPSE } type;
    int x;
    int y;
    int width;
    int height;
    int bmpwidth ;
    int bmpheight;
    HBITMAP hbmpRubber;
    HDC hdcRubber;
} RUBBER;

#define RGB_WHITE RGB(255,255,255)
#define RGB_BLACK RGB(0,0,0)
