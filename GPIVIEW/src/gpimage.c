/* GPIVIEW - reference implementation of a viewer of GPI files for DOS based systems
 * Copyright (c) 2024 Maxim Hoxha
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * GPI reader and decoder
 */

#include "lz4.h"
#include "gpimage.h"

static const char magicNumber[3] = {'G', 'P', 'I'};

__far unsigned char* decompressionBuffer;
unsigned char filterBuffer[512];
unsigned char palette[256 * 3];

int OpenGPIFile(const char* path, GPIInfo* info)
{
    unsigned int result = DOSOpenFile(path, DOSFILE_OPEN_READ, &(info->handle));
    if (result)
    {
        return result; //Oops, DOS couldn't open the file
    }
    unsigned char header[GPI_HEADER_SIZE];
    unsigned short bytesRead;
    result = DOSReadFile(info->handle, GPI_HEADER_SIZE, (__far unsigned char*)header, &bytesRead);
    //Check magic number to determine if this is a genuine GPI file
    for (int i = 0; i < 3; i++)
    {
        if (header[i] != magicNumber[i])
        {
            DOSCloseFile(info->handle);
            return -1; //Oops, no it isn't
        }
    }
    info->flags              = header[0x03];
    unsigned short w         = *((unsigned short*)(&header[0x04])) + 1;
    unsigned short h         = *((unsigned short*)(&header[0x06])) + 1;
    unsigned short nT        = *((unsigned short*)(&header[0x08])) + 1;
    unsigned short planeMask = *((unsigned short*)(&header[0x0A]));
    info->width = w;
    info->height = h;
    info->numTiles = nT;
    unsigned short bw = (w + 7)/8;
    info->byteWidth = bw;
    unsigned long dh = ((unsigned long)h) * ((unsigned long)nT);
    info->decHeight = dh;
    unsigned long bpp = ((unsigned long)bw) * dh;
    info->bytesPerPlane = bpp;

    int np = 0;
    if (planeMask & 0x0100)
    {
        info->hasMask = 1;
        info->planes[np] = (__far unsigned char*)DOSMemAlloc((unsigned short)((bpp + 15) >> 4));
        np++;
    }
    else info->hasMask = 0;
    unsigned short pTest = 0x0001;
    for (int i = 0; i < 8; i++)
    {
        if (planeMask & pTest)
        {
            info->planes[np] = (__far unsigned char*)DOSMemAlloc((unsigned short)((bpp + 15) >> 4));
            np++;
        }
        pTest <<= 1;
    }
    info->numPlanes = np;
    if (info->numPlanes == 0)
    {
        DOSConsoleWriteString("GPI file doesn't have any colour planes, GPIVIEW cannot view it.\r\n$");
        return -2;
    }
    info->palSize = 0x0001 << np;
    if (info->hasMask) info->palSize >> 1;
    info->palette = palette;
    if (info->flags & GPI_BPC == GPI_BPC_8) //8 bits per channel is trivial to read in
    {
        result = DOSReadFile(info->handle, info->palSize * 3, (__far unsigned char*)palette, &bytesRead);
    }
    else //4 bits per channel is a bit harder to read in
    {
        unsigned char midpal[128 * 3];
        result = DOSReadFile(info->handle, (info->palSize/2) * 3, (__far unsigned char*)midpal, &bytesRead);
        int mpPtr = 0;
        for (int i = 0; i < info->palSize; i++)
        {
            unsigned char cb = midpal[mpPtr];
            mpPtr++;
            palette[i * 3] = (cb & 0xF) * 0x11;
            palette[i * 3 + 1] = ((cb >> 4) & 0x0F) * 0x11;
            cb = midpal[mpPtr];
            mpPtr++;
            palette[i * 3 + 2] = (cb & 0xF) * 0x11;
            i++;
            palette[i * 3] = ((cb >> 4) & 0x0F) * 0x11;
            cb = midpal[mpPtr];
            mpPtr++;
            palette[i * 3 + 1] = (cb & 0xF) * 0x11;
            palette[i * 3 + 2] = ((cb >> 4) & 0x0F) * 0x11;
        }
    }

    return 0; //File ready to read into planes
}

void DecompressGPIFile(GPIInfo* info)
{
    unsigned char filt = info->flags & GPI_FILTERED;
    unsigned short h = info->height;
    unsigned short pw = info->byteWidth;
    unsigned short dh = info->decHeight;
    unsigned long planeSize = info->bytesPerPlane;
    //Each plane is decompressed the same way
    decompressionBuffer = DOSMemAlloc((planeSize + 15) >> 4);
    for (int i = 0; i < info->numPlanes; i++)
    {
        unsigned long compressedSize = 0;
        unsigned short bytesRead;
        DOSReadFile(info->handle, (h+1)/2, (__far unsigned char*)filterBuffer, &bytesRead);
        DOSReadFile(info->handle, 4, (__far unsigned char*)decompressionBuffer, &bytesRead);
        compressedSize = *((__far unsigned long*)(&decompressionBuffer[0]));
        DOSReadFile(info->handle, compressedSize, (__far unsigned char*)(decompressionBuffer + 4), &bytesRead);
        __far unsigned char* pptr = info->planes[i];
        __far unsigned char* dptr = decompressionBuffer;
        unsigned int decSize = LZ4Decompress(pptr, dptr);

        //Defilter in-place
        pptr = info->planes[i];
        __far unsigned char* pptr1 = info->planes[i-1];
        __far unsigned char* pptr2 = info->planes[i-2];
        __far unsigned char* pptr3 = info->planes[i-3];
        __far unsigned char* pptr4 = info->planes[i-4];
        for (int j = 0; j < dh; j++)
        {
            int curFiltSpec = filterBuffer[j >> 1];
            curFiltSpec = (j & 1 ? curFiltSpec >> 4 : curFiltSpec) & 0xF;
            int curRow = j * pw;
            unsigned char carry;
            switch (curFiltSpec)
            {
                case 0x0:
                    break; //Nothing to be done
                case 0x1:
                    carry = 0x00;
                    for (int k = 0; k < pw; k++)
                    {
                        unsigned char in = pptr[k + curRow];
                        in ^= carry;
                        in ^= in >> 1;
                        in ^= in >> 2;
                        in ^= in >> 4;
                        carry = (in & 0x01) << 7;
                        pptr[k + curRow] = in;
                    }
                    break;
                case 0x2:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= pptr[k + curRow - pw];
                    }
                    break;
                case 0x3:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= pptr[k + curRow - pw * h];
                    }
                    break;
                case 0x4:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= pptr1[k + curRow];
                    }
                    break;
                case 0x5:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= pptr2[k + curRow];
                    }
                    break;
                case 0x6:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= pptr3[k + curRow];
                    }
                    break;
                case 0x7:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= pptr4[k + curRow];
                    }
                    break;
                case 0x8:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] = ~pptr[k + curRow];
                    }
                    break;
                case 0x9:
                    carry = 0x00;
                    for (int k = 0; k < pw; k++)
                    {
                        unsigned char in = ~pptr[k + curRow];
                        in ^= carry;
                        in ^= in >> 1;
                        in ^= in >> 2;
                        in ^= in >> 4;
                        carry = (in & 0x01) << 7;
                        pptr[k + curRow] = in;
                    }
                    break;
                case 0xA:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= ~pptr[k + curRow - pw];
                    }
                    break;
                case 0xB:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= ~pptr[k + curRow - pw * h];
                    }
                    break;
                case 0xC:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= ~pptr1[k + curRow];
                    }
                    break;
                case 0xD:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= ~pptr2[k + curRow];
                    }
                    break;
                case 0xE:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= ~pptr3[k + curRow];
                    }
                    break;
                case 0xF:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= ~pptr4[k + curRow];
                    }
                    break;
            }
        }
    }
    DOSMemFree(decompressionBuffer);
}

void CloseGPIFile(GPIInfo* info)
{
    DOSCloseFile(info->handle);
    for (int i = 0; i < info->numPlanes; i++)
    {
        DOSMemFree(info->planes[i]);
    }
    return;
}
