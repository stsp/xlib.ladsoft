#include <windows.h>

#include <stdio.h>
    typedef struct _imagedata {
        struct _imagedata *next;
        enum { FT_BMP, FT_ICO, FT_CUR } type;
        int height;
        int width;
        int colors;
        int DIBSize;
        unsigned char DIBInfo[1];
    } IMAGEDATA;
    
    static int validDIB(
        LPBITMAPINFO DIB,
        DWORD DIBSize,
        int bitmap)
    {
        DWORD ANDMaskBytes;
        DWORD XORMaskBytes;
        DWORD ColorTableBytes;
        DWORD height;

        if (DIBSize < sizeof(BITMAPINFOHEADER))
            return FALSE;
    
        if (DIB->bmiHeader.biSize != sizeof(BITMAPINFOHEADER))
            return FALSE;
    
        if (DIB->bmiHeader.biPlanes != 1)
            return FALSE;
    
        if (DIB->bmiHeader.biBitCount != 1 &&
                DIB->bmiHeader.biBitCount != 4 &&
                DIB->bmiHeader.biBitCount != 8 &&
                DIB->bmiHeader.biBitCount != 24)
            return FALSE;
    
        if (bitmap) {
            height = DIB->bmiHeader.biHeight;
            ANDMaskBytes = 0;
        }
        else {
            height = DIB->bmiHeader.biHeight / 2;
            ANDMaskBytes = (((DIB->bmiHeader.biWidth + 31) & 0xffffffe0) >> 3) *
                    Height;
        }
    
        colorTableBytes = (1 << DIB->bmiHeader.biBitCount) * sizeof(RGBQUAD);
        XORMaskBytes = ((((DIB->bmiHeader.biWidth * DIB->bmiHeader.biBitCount) +
                31) & 0xffffffe0) >> 3) * height;
    
        if (DIB->bmiHeader.biSizeImage &&
                DIB->bmiHeader.biSizeImage != XORMaskBytes + ANDMaskBytes)
            return FALSE;
    
        if (DIBSize != sizeof(BITMAPINFOHEADER) + ColorTableBytes +
                XORMaskBytes + ANDMaskBytes)
            return FALSE;
    
        return TRUE;
    }
    
    IMAGEDATA *LoadBitmapFile(char *name)
    {
        FILE *bmpFile;
        DWORD fileSize;
        DWORD DIBSize;
        BITMAPFILEHEADER bfh;
        BITMAPINFOHEADER bih;
        INT colors;
        IMAGEDATA *id;
        
        bmpFile = fopen(name, "rb");
        if (!bmpFile)
            return NULL;

        fseek(bmpFile, 0 , SEEK_END);
        fileSize = ftell(bmpFile);
        fseek(bmpFile, 0, SEEK_SET);
    
        fread(&bfh, 1, sizeof(BITMAPFILEHEADER), bmpFile);
    
        if (bfh.bfType != 0x4D42 || bfh.bfSize != dwFileSize) {
            fclose(bmpFile);
            return NULL;
        }
    
        fread(&bih, 1, sizeof(BITMAPINFOHEADER), bmpFile);

        DIBSize = fileSize - sizeof(BITMAPFILEHEADER);
    
        if (!validDIB, &bih, DIBSize, TRUE)
        {
            fclose(bmpFile);
            return NULL;
        }
        /* maximum image = 256 * 256 */
        if (bih.biWidth > 256 || bih.biHeight > 256) {
            fclose(bmpFile);
            return NULL;
        }
    
        switch (bih.biBitCount) {
            case 1:
                colors = 2;
                break;
    
            case 4:
                colors = 16;
                break;

            case 8:
                colors = 256;
                break;

            case 24:
                colors = 0;
                break;                
            default:
                fclose(bmpFile);
                return NULL;
        }
        id = calloc(1, DIBSize + sizeof(IMAGEDATA) - 1);
        if (!id)
        {
            fclose(bmpFile);
            return NULL;
        }        
        id->type = FT_BMP;
        id->height = bih.biHeight;
        id->width = bih.biWidth;
        id->colors = colors;
        id->DIBSize = DIBSize;

        /* now read the dib */    
        fseek(bmpFile, sizeof(BITMAPFILEHEADER), SEEK_SET);
        fread(id->DIBInfo, 1, DIBSize, bmpFile);

        fclose(bmpFile);
        return id;
    }
    int SaveBitmapFile(char *name, IMAGEDATA *image)
    {
        BITMAPFILEHEADER bfh;
        FILE *bmpFile;    
        /*
         * Open the file for writing.
         */
        bmpFile = fopen(name, "wb");
        if (!bmpFile)
            return FALSE;
        bfh.bfType = 0x4D42;
        bfh.bfSize = sizeof(BITMAPFILEHEADER) + image->DIBSize;
        bfh.bfReserved1 = 0;
        bfh.bfReserved2 = 0;
        bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
                (image->colors * sizeof(RGBQUAD));
    
        fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), bmpFile);
        fwrite(&image->DIBInfo, 1, image->DIBSize, bmpFile);

        fclose(bmpFile);
    
        return TRUE;
    }
    /****************************************************************************/
    /*                                                                          */
    /*                 Copyright (C) 1987-1996 Microsoft Corp.                */
    /*                           All Rights Reserved                            */
    /*                                                                          */
    /****************************************************************************/
    /****************************** Module Header *******************************
    * Module Name: rwicocur.c
    *
    * Routines to read and write icon and cursor files.
    *
    * History:
    *
    ****************************************************************************/
    
    #include "imagedit.h"
    
    #include <io.h>
    #include <fcntl.h>                          // For NT fstat().
    #include <sys\types.h>                      // For fstat() types.
    #include <sys\stat.h>                       // For fstat() function.
    
    
    
    /************************************************************************
    * LoadIconCursorFile
    *
    * Loads the specified icon or cursor file.  It reads the images into
    * a list, then prompts for which one to open initially.
    *
    * Arguments:
    *
    * History:
    *
    ************************************************************************/
    
    BOOL LoadIconCursorFile(
        PSTR pszFullFileName,
        BOOL fIcon)
    {
        HFILE hf;
        INT i;
        PIMAGEINFO pImage;
        LPBITMAPINFO lpBitmapInfo;
        HANDLE hDIB;                    // Handle to DIB bits.
        OFSTRUCT OfStruct;
        struct stat FileStatus;
        ICOCURSORHDR hdr;               // Header structure of icon/cursor file.
        INT nImages;
        PICOCURSORDESC aIcoCurDesc;     // Array of ico/cur descriptors.
        DWORD dwFilePos;
        DWORD dwFileSize;
        INT iType;
    
        if ((hf = (HFILE)OpenFile(pszFullFileName, (LPOFSTRUCT)&OfStruct, OF_READ))
                == (HFILE)-1) {
            Message(MSG_CANTOPEN, pszFullFileName);
            return FALSE;
        }
    
        fstat((INT)_open_osfhandle((long)(hf), (int)(O_RDONLY)), &FileStatus);
        dwFileSize = (DWORD)FileStatus.st_size;
    
        ImageLinkFreeList();
    
        if (fIcon)
            iType = FT_ICON;
        else
            iType = FT_CURSOR;
    
        /*
         * Read the Icon/Cursor File header.
         */
        if (!MyFileRead(hf, (LPSTR)&hdr, sizeof(ICOCURSORHDR),
                pszFullFileName, iType))
            goto Error1;
    
        if (hdr.iReserved != 0) {
            Message(MSG_BADICOCURFILE, pszFullFileName);
            goto Error1;
        }
    
        /*
         * Get number of images in the file.
         */
        nImages = hdr.iResourceCount;
    
        if (!nImages || nImages > MAXIMAGES) {
            Message(MSG_BADICOCURFILE, pszFullFileName);
            goto Error1;
        }
    
        if (hdr.iResourceType != 1 && hdr.iResourceType != 2) {
            Message(MSG_BADICOCURFILE, pszFullFileName);
            goto Error1;
        }
    
        /*
         * Allocate room for the descriptor records.
         */
        if (!(aIcoCurDesc = (PICOCURSORDESC)MyAlloc(
                sizeof(ICOCURSORDESC) * nImages)))
            goto Error1;
    
        /*
         * Read in the descriptor records.
         */
        if (!MyFileRead(hf, (LPSTR)aIcoCurDesc, sizeof(ICOCURSORDESC) * nImages,
                pszFullFileName, iType))
            goto Error2;
    
        /*
         * Get the current file position (after the descriptors).  This
         * should be the start of the DIB's.
         */
        dwFilePos = (DWORD)SetFilePointer((HANDLE)hf, 0, NULL, (DWORD)1);
    
        /*
         * Validate the descriptor records.
         */
        for (i = 0; i < nImages; i++) {
            /*
             * Make sure the DIB's are sequential (not overlapping)
             * and they all fit within the file.
             */
            if (aIcoCurDesc[i].DIBOffset != dwFilePos ||
                    dwFilePos + aIcoCurDesc[i].DIBSize > dwFileSize) {
                Message(MSG_BADICOCURFILE, pszFullFileName);
                goto Error2;
            }
    
            /*
             * Jump to the next DIB.
             */
            dwFilePos += aIcoCurDesc[i].DIBSize;
        }
    
        for (i = 0; i < nImages; i++) {
            pImage = ImageLinkAlloc(NULL, 0, 0,
                    aIcoCurDesc[i].iHotspotX, aIcoCurDesc[i].iHotspotY,
                    (aIcoCurDesc[i].iColorCount == (BYTE)8) ?
                    aIcoCurDesc[i].iColorCount : 0);
    
            if (!pImage)
                goto Error3;
    
            /*
             * Allocate space for the DIB for this image.
             */
            if (!(hDIB = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                    (DWORD)aIcoCurDesc[i].DIBSize))) {
                Message(MSG_OUTOFMEMORY);
                goto Error3;
            }
    
            pImage->DIBSize = aIcoCurDesc[i].DIBSize;
            pImage->DIBhandle = hDIB;
            pImage->DIBPtr = (LPSTR)GlobalLock(hDIB);
        }
    
        for (pImage = gpImageHead; pImage != NULL; pImage = pImage->pImageNext) {
            if (!MyFileRead(hf, pImage->DIBPtr, (DWORD)pImage->DIBSize,
                    pszFullFileName, iType))
                goto Error3;
    
            lpBitmapInfo = (LPBITMAPINFO)pImage->DIBPtr;
    
            if (!IsValidDIB(lpBitmapInfo, pImage->DIBSize, TRUE)) {
                Message(MSG_BADICOCURFILE, pszFullFileName);
                goto Error3;
            }
    
            /*
             * Fill the x and y size fields in image node from
             * information in the DIB header.
             */
            pImage->cx = (INT)lpBitmapInfo->bmiHeader.biWidth;
            pImage->cy = (INT)lpBitmapInfo->bmiHeader.biHeight / 2;
            if (pImage->nColors == 0)
                pImage->nColors = (1 << lpBitmapInfo->bmiHeader.biBitCount);
    
            pImage->pDevice = DeviceLinkFind(
                    fIcon ? gpIconDeviceHead : gpCursorDeviceHead,
                    pImage->nColors, pImage->cx, pImage->cy);
        }
    
        _lclose((HFILE)hf);
    
        fFileDirty = FALSE;
        SetFileName(pszFullFileName);
        giType = iType;
    
        gnImages = nImages;
    
        /*
         * Update the PropBar and the Toolbox so that they show
         * information about the opened file.  We do this now just
         * in case the user cancels out of the Image Select Dialog.
         */
        PropBarUpdate();
        ToolboxUpdate();
    
        /*
         * Open up an image.  If there are multiple images in the file,
         * show the Image Select dialog.  We also show the Image Select
         * dialog if the file only has one image but it is not for a known
         * device.
         */
        if (gnImages > 1 || !gpImageHead->pDevice)
            ImageSelectDialog();
        else
            ImageOpen2(gpImageHead);
    
        return TRUE;
    
    Error3:
        ImageLinkFreeList();
    
    Error2:
        MyFree(aIcoCurDesc);
    
    Error1:
        _lclose((HFILE)hf);
    
        return FALSE;
    }
    
    
    
    /************************************************************************
    * IsValidDIB
    *
    * This function determines if the given DIB is valid or not.  It does
    * this without touching memory outside the bounds of the cbDIBSize
    * passed in or the size of a BITMAPINFOHEADER, whichever is smaller.
    * Note that even if the DIB is valid, however, the current image
    * editor might not be able to edit it (the size might be too big, for
    * instance).
    *
    * Arguments:
    *   LPBITMAPINFO pDIB - Points to the DIB.
    *   DWORD cbDIBSize   - The size of the DIB.
    *   BOOL fIcoCur      - TRUE if this is an icon or cursor.  This effects
    *                       whether an AND mask is expected to be in the DIB.
    *
    * History:
    *
    ************************************************************************/
    
    
    
    
    /************************************************************************
    * SaveIconCursorFile
    *
    *
    *
    * Arguments:
    *
    * Returns:
    *   TRUE if successful, FALSE otherwise.
    *
    * History:
    *
    ************************************************************************/
    
    BOOL SaveIconCursorFile(
        PSTR pszFullFileName,
        INT iType)
    {
        ICOCURSORHDR IcoCurHdr;     // Header structure of icon/cursor file.
        ICOCURSORDESC IcoCurDesc;   // Icon/cursor descriptor struct.
        HCURSOR hcurOld;            // Handle to old cursor.
        PIMAGEINFO pImage;          // Pointer to node in image list.
        DWORD iBitsOffset;          // Offset of the actual DIB bits for image.
        HFILE hf;
        OFSTRUCT OfStruct;
    
        hcurOld = SetCursor(hcurWait);
    
        /*
         * Save the bits of the current image.
         */
        ImageSave();
    
        /*
         * Open the file for writing.
         */
        if ((hf = (HFILE)OpenFile(pszFullFileName, &OfStruct, OF_CREATE | OF_READWRITE))
                == (HFILE)-1) {
            Message(MSG_CANTCREATE, pszFullFileName);
            goto Error1;
        }
    
        /*
         * This is crucial since this helps distinguish a 3.0 icon/cursor
         * from an old, old (2.1 format) icon/cursor, which has meaningful
         * information in this WORD.
         */
        IcoCurHdr.iReserved = (WORD)0;
    
        if (iType == FT_ICON)
            IcoCurHdr.iResourceType = 1;        // Icon type.
        else
            IcoCurHdr.iResourceType = 2;        // Cursor type.
    
        IcoCurHdr.iResourceCount = (WORD)gnImages;
    
        /*
         * Write the header to disk.
         */
        if (!MyFileWrite(hf, (LPSTR)&IcoCurHdr, sizeof(ICOCURSORHDR),
                pszFullFileName))
            goto Error2;
    
        /*
         * Write all the descriptors.
         */
        iBitsOffset = sizeof(ICOCURSORHDR) + (gnImages * sizeof(ICOCURSORDESC));
        for (pImage = gpImageHead; pImage; pImage = pImage->pImageNext) {
            IcoCurDesc.iWidth = (BYTE)pImage->cx;
            IcoCurDesc.iHeight = (BYTE)pImage->cy;
            IcoCurDesc.iColorCount = (giType == FT_ICON) ?
                    (BYTE)pImage->nColors : (BYTE)0;
            IcoCurDesc.iUnused = 0;
            IcoCurDesc.iHotspotX = (WORD)pImage->iHotspotX;
            IcoCurDesc.iHotspotY = (WORD)pImage->iHotspotY;
            IcoCurDesc.DIBSize = pImage->DIBSize;
            IcoCurDesc.DIBOffset = iBitsOffset;
    
            if (!MyFileWrite(hf, (LPSTR)&IcoCurDesc, sizeof(ICOCURSORDESC),
                    pszFullFileName))
                goto Error2;
    
            iBitsOffset += IcoCurDesc.DIBSize;
        }
    
        /*
         * Now write the DIB's.
         */
        for (pImage = gpImageHead; pImage; pImage = pImage->pImageNext) {
            if (!MyFileWrite(hf, (LPSTR)pImage->DIBPtr,
                    (DWORD)pImage->DIBSize, pszFullFileName))
                goto Error2;
        }
    
        _lclose((HFILE)hf);
    
        fFileDirty = FALSE;
        SetFileName(pszFullFileName);
    
        SetCursor(hcurOld);
    
        return TRUE;
    
    Error2:
        _lclose((HFILE)hf);
    
    Error1:
        SetCursor(hcurOld);
    
        return FALSE;
    }
