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
 * Simple drawing functions
 */

#include "pc98_gdc.h"
#include "ibm_vga.h"
#include "graphicshw.h"
#include "drawing.h"
#include "x86strops.h"

__far unsigned char* PC98GDCPlanes[4] = { GDC_PLANE0, GDC_PLANE1, GDC_PLANE2, GDC_PLANE3 };

//Abstract function pointers
void (*ClearScreen)();
void (*DrawBackgroundPattern)();
void (*DrawRectPortion)(GPIInfo* info, int x, int y);
void (*UpdateWithScroll)(GPIInfo* info, int dx, int dy);

void ClearScreenPC98()
{
    Memset16Far(0x0000, GDC_PLANE0, 16000);
    Memset16Far(0x0000, GDC_PLANE1, 16000);
    Memset16Far(0x0000, GDC_PLANE2, 16000);
    Memset16Far(0x0000, GDC_PLANE3, 16000);
}

void ClearScreenVGA()
{
    VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
    VGASequencerDataWrite(0x0F);
    Memset16Far(0x0000, VGA_BASE_1, 19200);
}

//A little checkerboard pattern, exact colours depend on the palette
void DrawBackgroundPatternPC98()
{
    for (unsigned int i = 0; i < 25; i++)
    {
        Memset16Far(0xFF00, (GDC_PLANE0) + 1280 * i, 320);
        Memset16Far(0x00FF, (GDC_PLANE0) + 1280 * i + 640, 320);
    }
}

//A little checkerboard pattern, exact colours depend on the palette
void DrawBackgroundPatternVGA()
{
    VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
    VGASequencerDataWrite(0x01);
    for (unsigned int i = 0; i < 30; i++)
    {
        Memset16Far(0xFF00, (VGA_BASE_1) + 1280 * i, 320);
        Memset16Far(0x00FF, (VGA_BASE_1) + 1280 * i + 640, 320);
    }
}

void DrawRectPortionPC98(GPIInfo* info, int x, int y)
{
    int startpln = 0;
    int rootpln = 0;
    if (info->hasMask)
    {
        startpln = 2;
        rootpln = 1;
    }
    int pw = info->byteWidth;
    int h = info->height;
    int sw = pw;
    int bx = 0;
    if (sw > 80)
    {
        sw = 80;
        bx = x/8;
        if (bx + 80 > pw) bx = pw - 80;
        x = bx * 8;
    }
    else x = 0;
    int sh = h;
    if (sh > 400)
    {
        sh = 400;
        if (y + 400 > h) y = h - 400;
    }
    else y = 0;
    info->displayX = x;
    info->displayY = y;
    if (sw & 0x1)
    {
        if (info->hasMask) //Our background pattern only occupies one plane and is known in advance, so we can make a number of optimisations
        {
            __far unsigned char* mptr = info->planes[0];
            __far unsigned char* pptr = info->planes[1];
            for (int j = 0; j < sh; j++)
            {
                unsigned short checkpat;
                if (((j + y) & 0xF) < 8) checkpat = 0xFF00;
                else checkpat = 0x00FF;
                for (int k = 0; k < sw - 1; k += 2)
                {
                    unsigned short outshort = checkpat & (*((__far unsigned short*)(mptr + ((j + y) * pw + bx + k))));
                    outshort |= (*((__far unsigned short*)(pptr + ((j + y) * pw + bx + k))));
                    *((__far unsigned short*)((PC98GDCPlanes[0]) + j * 80 + k)) = outshort;
                }
                unsigned short outshort = checkpat & ((unsigned short)mptr[(j + y) * pw + bx + (sw - 1)]);
                outshort |= (unsigned short)pptr[(j + y) * pw + bx + (sw - 1)];
                (PC98GDCPlanes[0])[j * 80 + (sw - 1)] = (unsigned char)outshort;
            }
        }
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            for (int j = 0; j < sh; j++)
            {
                Memcpy16Far(pptr + ((j + y) * pw + bx), (PC98GDCPlanes[i-rootpln]) + j * 80, sw >> 1);
                (PC98GDCPlanes[i-rootpln])[j * 80 + (sw - 1)] = pptr[(j + y) * pw + bx + (sw - 1)];
            }
        }
    }
    else
    {
        if (info->hasMask) //Our background pattern only occupies one plane and is known in advance, so we can make a number of optimisations
        {
            __far unsigned char* mptr = info->planes[0];
            __far unsigned char* pptr = info->planes[1];
            for (int j = 0; j < sh; j++)
            {
                unsigned short checkpat;
                if (((j + y) & 0xF) < 8) checkpat = 0xFF00;
                else checkpat = 0x00FF;
                for (int k = 0; k < sw; k += 2)
                {
                    unsigned short outshort = checkpat & (*((__far unsigned short*)(mptr + ((j + y) * pw + bx + k))));
                    outshort |= (*((__far unsigned short*)(pptr + ((j + y) * pw + bx + k))));
                    *((__far unsigned short*)((PC98GDCPlanes[0]) + j * 80 + k)) = outshort;
                }
            }
        }
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            for (int j = 0; j < sh; j++)
            {
                Memcpy16Far(pptr + ((j + y) * pw + bx), (PC98GDCPlanes[i-rootpln]) + j * 80, sw >> 1);
            }
        }
    }
}

void DrawRectPortionVGA(GPIInfo* info, int x, int y)
{
    int startpln = 0;
    if (info->hasMask) startpln = 2;
    int pw = info->byteWidth;
    int h = info->height;
    int sw = pw;
    int bx = 0;
    if (sw > 80)
    {
        sw = 80;
        bx = x/8;
        if (bx + 80 > pw) bx = pw - 80;
        x = bx * 8;
    }
    else x = 0;
    int sh = h;
    if (sh > 480)
    {
        sh = 480;
        if (y + 480 > h) y = h - 480;
    }
    else y = 0;
    info->displayX = x;
    info->displayY = y;
    unsigned int wplane = 0x0001;
    if (sw & 0x1)
    {
        if (info->hasMask) //Our background pattern only occupies one plane and is known in advance, so we can make a number of optimisations
        {
            __far unsigned char* mptr = info->planes[0];
            __far unsigned char* pptr = info->planes[1];
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = 0; j < sh; j++)
            {
                unsigned short checkpat;
                if (((j + y) & 0xF) < 8) checkpat = 0xFF00;
                else checkpat = 0x00FF;
                for (int k = 0; k < sw - 1; k += 2)
                {
                    unsigned short outshort = checkpat & (*((__far unsigned short*)(mptr + ((j + y) * pw + bx + k))));
                    outshort |= (*((__far unsigned short*)(pptr + ((j + y) * pw + bx + k))));
                    *((__far unsigned short*)((VGA_BASE_1) + j * 80 + k)) = outshort;
                }
                unsigned short outshort = checkpat & ((unsigned short)mptr[(j + y) * pw + bx + (sw - 1)]);
                outshort |= (unsigned short)pptr[(j + y) * pw + bx + (sw - 1)];
                (VGA_BASE_1)[j * 80 + (sw - 1)] = (unsigned char)outshort;
            }
            wplane <<= 1;
        }
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = 0; j < sh; j++)
            {
                Memcpy16Far(pptr + ((j + y) * pw + bx), (VGA_BASE_1) + j * 80, sw >> 1);
                (VGA_BASE_1)[j * 80 + (sw >> 1)] = pptr[(j + y) * pw + bx + (sw >> 1)];
            }
            wplane <<= 1;
        }
    }
    else
    {
        if (info->hasMask) //Our background pattern only occupies one plane and is known in advance, so we can make a number of optimisations
        {
            __far unsigned char* mptr = info->planes[0];
            __far unsigned char* pptr = info->planes[1];
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = 0; j < sh; j++)
            {
                unsigned short checkpat;
                if (((j + y) & 0xF) < 8) checkpat = 0xFF00;
                else checkpat = 0x00FF;
                for (int k = 0; k < sw; k += 2)
                {
                    unsigned short outshort = checkpat & (*((__far unsigned short*)(mptr + ((j + y) * pw + bx + k))));
                    outshort |= (*((__far unsigned short*)(pptr + ((j + y) * pw + bx + k))));
                    *((__far unsigned short*)((VGA_BASE_1) + j * 80 + k)) = outshort;
                }
            }
            wplane <<= 1;
        }
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = 0; j < sh; j++)
            {
                Memcpy16Far(pptr + ((j + y) * pw + bx), (VGA_BASE_1) + j * 80, sw >> 1);
            }
            wplane <<= 1;
        }
    }
}

void UpdateWithScrollPC98(GPIInfo* info, int dx, int dy)
{
    if (info->width <= 640) dx = 0;
    if (info->height <= 400) dy = 0;
    if (dx == 0 && dy == 0) return; //Don't bother if nothing will change
    int oldX = info->displayX;
    int oldY = info->displayY;
    int newX = oldX + dx;
    int newY = oldY + dy;
    if (newX < 0)
    {
        newX = 0;
        dx = newX - oldX;
    }
    else if ((info->width > 640) && (newX + 625 > info->width))
    {
        newX = (info->width - 625) & 0xFFF0;
        dx = newX - oldX;
    }
    if (newY < 0)
    {
        newY = 0;
        dy = newY - oldY;
    }
    else if ((info->height > 400) && (newY + 400 > info->height))
    {
        newY = info->height - 400;
        dy = newY - oldY;
    }
    info->displayX = newX;
    info->displayY = newY;

    int startpln = 0;
    int rootpln = 0;
    if (info->hasMask)
    {
        startpln = 2;
        rootpln = 1;
    }
    int pw = info->byteWidth;
    int sw = pw;
    if (sw > 80) sw = 80;
    int margw = 80 - sw;
    int bx = newX/8;
    if (dy > 0)
    {
        int ty = 400 + oldY;
        if (info->hasMask)
        {
            __far unsigned char* mptr = info->planes[0];
            __far unsigned char* pptr = info->planes[1];
            for (int j = ty; j < ty + dy; j++)
            {
                unsigned char checkpat1, checkpat2;
                if ((j & 0xF) < 8)
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                else
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                for (int k = 0; k < sw; k++)
                {
                    unsigned char outbyte;
                    if (k & 1) outbyte = checkpat2 & mptr[j * pw + bx + k];
                    else outbyte = checkpat1 & mptr[j * pw + bx + k];
                    outbyte |= pptr[j * pw + bx + k];
                    (PC98GDCPlanes[0])[((j * 80) + bx + k) & 0x7FFF] = outbyte;
                }
            }
        }
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            for (int j = ty; j < ty + dy; j++)
            {
                if ((((j * 80) + bx) & 0x7FFF) > ((((j * 80) + bx) + sw - 1) & 0x7FFF)) //Account for wraparound
                {
                    int pbx = 0x8000 - (((j * 80) + bx) & 0x7FFF);
                    MemcpyFar(pptr + (j * pw + bx), (PC98GDCPlanes[i-rootpln]) + (((j * 80) + bx) & 0x7FFF), pbx);
                    MemcpyFar(pptr + (j * pw + bx + pbx), PC98GDCPlanes[i-rootpln], sw-pbx);
                }
                else
                {
                    MemcpyFar(pptr + (j * pw + bx), (PC98GDCPlanes[i-rootpln]) + (((j * 80) + bx) & 0x7FFF), sw);
                }
            }
        }
        //Fill rest of line if necessary
        if (sw < 80)
        {
            for (int j = ty; j < ty + dy; j++)
            {
                unsigned char checkpat1, checkpat2;
                if ((j & 0xF) < 8)
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                else
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                for (int k = sw; k < 80; k++)
                {
                    if (k & 1) (PC98GDCPlanes[0])[((j * 80) + bx + k) & 0x7FFF] = checkpat2;
                    else (PC98GDCPlanes[0])[((j * 80) + bx + k) & 0x7FFF] = checkpat1;
                }
            }
            for (int i = 1; i < 4; i++)
            {
                for (int j = ty; j < ty + dy; j++)
                {
                    if ((((j * 80) + bx + sw) & 0x7FFF) > ((((j * 80) + bx) + 79) & 0x7FFF)) //Account for wraparound
                    {
                        int pbx = 0x8000 - (((j * 80) + bx + sw) & 0x7FFF);
                        MemsetFar(0x00, (PC98GDCPlanes[i]) + (((j * 80) + bx + sw) & 0x7FFF), pbx);
                        MemsetFar(0x00, PC98GDCPlanes[i], margw-pbx);
                    }
                    else
                    {
                        MemsetFar(0x00, (PC98GDCPlanes[i]) + (((j * 80) + bx + sw) & 0x7FFF), margw);
                    }
                }
            }
        }
    }
    else if (dy < 0)
    {
        int by = oldY;
        if (info->hasMask)
        {
            __far unsigned char* mptr = info->planes[0];
            __far unsigned char* pptr = info->planes[1];
            for (int j = by + dy; j < by; j++)
            {
                unsigned char checkpat1, checkpat2;
                if ((j & 0xF) < 8)
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                else
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                for (int k = 0; k < sw; k++)
                {
                    unsigned char outbyte;
                    if (k & 1) outbyte = checkpat2 & mptr[j * pw + bx + k];
                    else outbyte = checkpat1 & mptr[j * pw + bx + k];
                    outbyte |= pptr[j * pw + bx + k];
                    (PC98GDCPlanes[0])[((j * 80) + bx + k) & 0x7FFF] = outbyte;
                }
            }
        }
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            for (int j = by + dy; j < by; j++)
            {
                if ((((j * 80) + bx) & 0x7FFF) > ((((j * 80) + bx) + sw - 1) & 0x7FFF)) //Account for wraparound
                {
                    int pbx = 0x8000 - (((j * 80) + bx) & 0x7FFF);
                    MemcpyFar(pptr + (j * pw + bx), (PC98GDCPlanes[i-rootpln]) + (((j * 80) + bx) & 0x7FFF), pbx);
                    MemcpyFar(pptr + (j * pw + bx + pbx), PC98GDCPlanes[i-rootpln], sw-pbx);
                }
                else
                {
                    MemcpyFar(pptr + (j * pw + bx), (PC98GDCPlanes[i-rootpln]) + (((j * 80) + bx) & 0x7FFF), sw);
                }
            }
        }
        //Fill rest of line if necessary
        if (sw < 80)
        {
            for (int j = by + dy; j < by; j++)
            {
                unsigned char checkpat1, checkpat2;
                if ((j & 0xF) < 8)
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                else
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                for (int k = sw; k < 80; k++)
                {
                    if (k & 1) (PC98GDCPlanes[0])[((j * 80) + bx + k) & 0x7FFF] = checkpat2;
                    else (PC98GDCPlanes[0])[((j * 80) + bx + k) & 0x7FFF] = checkpat1;
                }
            }
            for (int i = 1; i < 4; i++)
            {
                for (int j = by + dy; j < by; j++)
                {
                    if ((((j * 80) + bx + sw) & 0x7FFF) > ((((j * 80) + bx) + 79) & 0x7FFF)) //Account for wraparound
                    {
                        int pbx = 0x8000 - (((j * 80) + bx + sw) & 0x7FFF);
                        MemsetFar(0x00, (PC98GDCPlanes[i]) + (((j * 80) + bx + sw) & 0x7FFF), pbx);
                        MemsetFar(0x00, PC98GDCPlanes[i], margw-pbx);
                    }
                    else
                    {
                        MemsetFar(0x00, (PC98GDCPlanes[i]) + (((j * 80) + bx + sw) & 0x7FFF), margw);
                    }
                }
            }
        }
    }

    int sh = info->height;
    if (sh > 400) sh = 400;
    if (dx > 0)
    {
        int rbx = 80 + ((oldX/16) * 2);
        if (info->hasMask)
        {
            __far unsigned char* mptr = info->planes[0];
            __far unsigned char* pptr = info->planes[1];
            for (int j = rbx; j < rbx + ((dx/16) * 2); j++)
            {
                unsigned char checkpat1, checkpat2;
                if (j & 1)
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                else
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                for (int k = newY; k < newY + sh; k++)
                {
                    unsigned char outbyte;
                    if ((k & 0xF) < 8) outbyte = checkpat1 & mptr[k * pw + j];
                    else outbyte = checkpat2 & mptr[k * pw + j];
                    outbyte |= pptr[k * pw + j];
                    PC98GDCPlanes[0][((k * 80) + j) & 0x7FFF] = outbyte;
                }
            }
        }
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            for (int j = rbx; j < rbx + ((dx/16) * 2); j++)
            {
                for (int k = newY; k < newY + sh; k++)
                {
                    PC98GDCPlanes[i-rootpln][((k * 80) + j) & 0x7FFF] = pptr[k * pw + j];
                }
            }
        }
        //Fill rest of column if necessary
        if (sh < 400)
        {
            for (int j = rbx; j < rbx + ((dx/16) * 2); j++)
            {
                unsigned char checkpat1, checkpat2;
                if (j & 1)
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                else
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                for (int k = newY + sh; k < newY + 400; k++)
                {
                    if ((k & 0xF) < 8) PC98GDCPlanes[0][((k * 80) + j) & 0x7FFF] = checkpat1;
                    else PC98GDCPlanes[0][((k * 80) + j) & 0x7FFF] = checkpat2;
                }
            }
            for (int i = 1; i < 4; i++)
            {
                for (int j = rbx; j < rbx + ((dx/16) * 2); j++)
                {
                    for (int k = newY + sh; k < newY + 400; k++)
                    {
                        PC98GDCPlanes[i][((k * 80) + j) & 0x7FFF] = 0x00;
                    }
                }
            }
        }
    }
    else if (dx < 0)
    {
        int lbx = (oldX/16) * 2;
        if (info->hasMask)
        {
            __far unsigned char* mptr = info->planes[0];
            __far unsigned char* pptr = info->planes[1];
            for (int j = lbx + ((dx/16) * 2); j < lbx; j++)
            {
                unsigned char checkpat1, checkpat2;
                if (j & 1)
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                else
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                for (int k = newY; k < newY + sh; k++)
                {
                    unsigned char outbyte;
                    if ((k & 0xF) < 8) outbyte = checkpat1 & mptr[k * pw + j];
                    else outbyte = checkpat2 & mptr[k * pw + j];
                    outbyte |= pptr[k * pw + j];
                    PC98GDCPlanes[0][((k * 80) + j) & 0x7FFF] = outbyte;
                }
            }
        }
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            for (int j = lbx + ((dx/16) * 2); j < lbx; j++)
            {
                for (int k = newY; k < newY + sh; k++)
                {
                    PC98GDCPlanes[i-rootpln][((k * 80) + j) & 0x7FFF] = pptr[k * pw + j];
                }
            }
        }
        //Fill rest of column if necessary
        if (sh < 400)
        {
            for (int j = lbx + ((dx/16) * 2); j < lbx; j++)
            {
                unsigned char checkpat1, checkpat2;
                if (j & 1)
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                else
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                for (int k = newY + sh; k < newY + 400; k++)
                {
                    if ((k & 0xF) < 8) PC98GDCPlanes[0][((k * 80) + j) & 0x7FFF] = checkpat1;
                    else PC98GDCPlanes[0][((k * 80) + j) & 0x7FFF] = checkpat2;
                }
            }
            for (int i = 1; i < 4; i++)
            {
                for (int j = lbx + ((dx/16) * 2); j < lbx; j++)
                {
                    for (int k = newY + sh; k < newY + 400; k++)
                    {
                        PC98GDCPlanes[i][((k * 80) + j) & 0x7FFF] = 0x00;
                    }
                }
            }
        }
    }

    int newaX = newX/16;
    GDCScrollRawGraphics(0, (newY * 40) + newaX, 400);
}

void UpdateWithScrollVGA(GPIInfo* info, int dx, int dy)
{
    if (info->width <= 640) dx = 0;
    if (info->height <= 480) dy = 0;
    if (dx == 0 && dy == 0) return; //Don't bother if nothing will change
    int oldX = info->displayX;
    int oldY = info->displayY;
    int newX = oldX + dx;
    int newY = oldY + dy;
    if (newX < 0)
    {
        newX = 0;
        dx = newX - oldX;
    }
    else if ((info->width > 640) && (newX + 633 > info->width))
    {
        newX = (info->width - 633) & 0xFFF8;
        dx = newX - oldX;
    }
    if (newY < 0)
    {
        newY = 0;
        dy = newY - oldY;
    }
    else if ((info->height > 480) && (newY + 480 > info->height))
    {
        newY = info->height - 480;
        dy = newY - oldY;
    }
    info->displayX = newX;
    info->displayY = newY;

    int startpln = 0;
    if (info->hasMask) startpln = 2;
    int pw = info->byteWidth;
    int sw = pw;
    if (sw > 80) sw = 80;
    int margw = 80 - sw;
    int bx = newX/8;
    if (dy > 0)
    {
        int ty = 480 + oldY;
        unsigned int wplane = 0x0001;
        if (info->hasMask)
        {
            __far unsigned char* mptr = info->planes[0];
            __far unsigned char* pptr = info->planes[1];
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = ty; j < ty + dy; j++)
            {
                unsigned char checkpat1, checkpat2;
                if ((j & 0xF) < 8)
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                else
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                for (int k = 0; k < sw; k++)
                {
                    unsigned char outbyte;
                    if (k & 1) outbyte = checkpat2 & mptr[j * pw + bx + k];
                    else outbyte = checkpat1 & mptr[j * pw + bx + k];
                    outbyte |= pptr[j * pw + bx + k];
                    (VGA_BASE_1)[(j * 80) + bx + k] = outbyte;
                }
            }
            wplane <<= 1;
        }
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = ty; j < ty + dy; j++) //Rely on overflow wrapping
            {
                MemcpyFar(pptr + (j * pw + bx), (VGA_BASE_1) + ((j * 80) + bx), sw);
            }
            wplane <<= 1;
        }
        //Fill rest of line if necessary
        if (sw < 80)
        {
            wplane = 0x0001;
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = ty; j < ty + dy; j++)
            {
                unsigned char checkpat1, checkpat2;
                if ((j & 0xF) < 8)
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                else
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                for (int k = sw; k < 80; k++)
                {
                    if (k & 1) (VGA_BASE_1)[(j * 80) + bx + k] = checkpat2;
                    else (VGA_BASE_1)[(j * 80) + bx + k] = checkpat1;
                }
            }
            wplane = 0x000E;
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = ty; j < ty + dy; j++) //Rely on overflow wrapping
            {
                MemsetFar(0x00, (VGA_BASE_1) + ((j * 80) + bx + sw), margw);
            }
        }
    }
    else if (dy < 0)
    {
        int by = oldY;
        unsigned int wplane = 0x0001;
        if (info->hasMask)
        {
            __far unsigned char* mptr = info->planes[0];
            __far unsigned char* pptr = info->planes[1];
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = by + dy; j < by; j++)
            {
                unsigned char checkpat1, checkpat2;
                if ((j & 0xF) < 8)
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                else
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                for (int k = 0; k < sw; k++)
                {
                    unsigned char outbyte;
                    if (k & 1) outbyte = checkpat2 & mptr[j * pw + bx + k];
                    else outbyte = checkpat1 & mptr[j * pw + bx + k];
                    outbyte |= pptr[j * pw + bx + k];
                    (VGA_BASE_1)[(j * 80) + bx + k] = outbyte;
                }
            }
            wplane <<= 1;
        }
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = by + dy; j < by; j++) //Rely on overflow wrapping
            {
                MemcpyFar(pptr + (j * pw + bx), (VGA_BASE_1) + ((j * 80) + bx), sw);
            }
            wplane <<= 1;
        }
        //Fill rest of line if necessary
        if (sw < 80)
        {
            wplane = 0x0001;
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = by + dy; j < by; j++)
            {
                unsigned char checkpat1, checkpat2;
                if ((j & 0xF) < 8)
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                else
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                for (int k = sw; k < 80; k++)
                {
                    if (k & 1) (VGA_BASE_1)[(j * 80) + bx + k] = checkpat2;
                    else (VGA_BASE_1)[(j * 80) + bx + k] = checkpat1;
                }
            }
            wplane = 0x000E;
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = by + dy; j < by; j++) //Rely on overflow wrapping
            {
                MemsetFar(0x00, (VGA_BASE_1) + ((j * 80) + bx + sw), margw);
            }
        }
    }

    int sh = info->height;
    if (sh > 480) sh = 480;
    if (dx > 0)
    {
        int rbx = 80 + (oldX/8);
        unsigned int wplane = 0x0001;
        if (info->hasMask)
        {
            __far unsigned char* mptr = info->planes[0];
            __far unsigned char* pptr = info->planes[1];
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = rbx; j < rbx + (dx/8); j++)
            {
                unsigned char checkpat1, checkpat2;
                if (j & 1)
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                else
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                for (int k = newY; k < newY + sh; k++)
                {
                    unsigned char outbyte;
                    if ((k & 0xF) < 8) outbyte = checkpat1 & mptr[k * pw + j];
                    else outbyte = checkpat2 & mptr[k * pw + j];
                    outbyte |= pptr[k * pw + j];
                    (VGA_BASE_1)[(k * 80) + j] = outbyte;
                }
            }
            wplane <<= 1;
        }
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = rbx; j < rbx + (dx/8); j++)
            {
                for (int k = newY; k < newY + sh; k++)
                {
                    (VGA_BASE_1)[(k * 80) + j] = pptr[k * pw + j];
                }
            }
            wplane <<= 1;
        }
        //Fill rest of column if necessary
        if (sh < 480)
        {
            wplane = 0x0001;
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = rbx; j < rbx + (dx/8); j++)
            {
                unsigned char checkpat1, checkpat2;
                if (j & 1)
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                else
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                for (int k = newY + sh; k < newY + 480; k++)
                {
                    if ((k & 0xF) < 8) (VGA_BASE_1)[(k * 80) + j] = checkpat1;
                    else (VGA_BASE_1)[(k * 80) + j] = checkpat2;
                }
            }
            wplane = 0x000E;
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = rbx; j < rbx + (dx/8); j++)
            {
                for (int k = newY + sh; k < newY + 480; k++)
                {
                    (VGA_BASE_1)[(k * 80) + j] = 0x00;
                }
            }
        }
    }
    else if (dx < 0)
    {
        int lbx = oldX/8;
        unsigned int wplane = 0x0001;
        if (info->hasMask)
        {
            __far unsigned char* mptr = info->planes[0];
            __far unsigned char* pptr = info->planes[1];
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = lbx + (dx/8); j < lbx; j++)
            {
                unsigned char checkpat1, checkpat2;
                if (j & 1)
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                else
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                for (int k = newY; k < newY + sh; k++)
                {
                    unsigned char outbyte;
                    if ((k & 0xF) < 8) outbyte = checkpat1 & mptr[k * pw + j];
                    else outbyte = checkpat2 & mptr[k * pw + j];
                    outbyte |= pptr[k * pw + j];
                    (VGA_BASE_1)[(k * 80) + j] = outbyte;
                }
            }
            wplane <<= 1;
        }
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = lbx + (dx/8); j < lbx; j++)
            {
                for (int k = newY; k < newY + sh; k++)
                {
                    (VGA_BASE_1)[(k * 80) + j] = pptr[k * pw + j];
                }
            }
            wplane <<= 1;
        }
        //Fill rest of column if necessary
        if (sh < 480)
        {
            wplane = 0x0001;
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = lbx + (dx/8); j < lbx; j++)
            {
                unsigned char checkpat1, checkpat2;
                if (j & 1)
                {
                    checkpat1 = 0xFF;
                    checkpat2 = 0x00;
                }
                else
                {
                    checkpat1 = 0x00;
                    checkpat2 = 0xFF;
                }
                for (int k = newY + sh; k < newY + 480; k++)
                {
                    if ((k & 0xF) < 8) (VGA_BASE_1)[(k * 80) + j] = checkpat1;
                    else (VGA_BASE_1)[(k * 80) + j] = checkpat2;
                }
            }
            wplane = 0x000E;
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = lbx + (dx/8); j < lbx; j++)
            {
                for (int k = newY + sh; k < newY + 480; k++)
                {
                    (VGA_BASE_1)[(k * 80) + j] = 0x00;
                }
            }
        }
    }

    int newaX = newX/8;
    VGAScrollRaw((newY * 80) + newaX);
}

void InitDrawing(int type)
{
    switch (type)
    {
        case GRAPHICSHW_PC98_16:
            ClearScreen = ClearScreenPC98;
            DrawBackgroundPattern = DrawBackgroundPatternPC98;
            DrawRectPortion = DrawRectPortionPC98;
            UpdateWithScroll = UpdateWithScrollPC98;
            break;
        case GRAPHICSHW_IBM_VGA:
            ClearScreen = ClearScreenVGA;
            DrawBackgroundPattern = DrawBackgroundPatternVGA;
            DrawRectPortion = DrawRectPortionVGA;
            UpdateWithScroll = UpdateWithScrollVGA;
            break;
    }
}
