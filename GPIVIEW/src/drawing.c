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
void (*DrawRectPortion)(GPIInfo* info, int x, int y);
void (*UpdateWithScroll)(GPIInfo* info, int dx, int dy);

void DrawRectPortionPC98(GPIInfo* info, int x, int y)
{
    int startpln = 0;
    if (info->hasMask) startpln = 1;
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
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            for (int j = 0; j < sh; j++)
            {
                Memcpy16Far(pptr + ((j + y) * pw + bx), (PC98GDCPlanes[i-startpln]) + j * 80, sw >> 1);
                (PC98GDCPlanes[i-startpln])[j * 80 + (sw >> 1)] = pptr[(j + y) * pw + bx + (sw >> 1)];
            }
        }
    }
    else
    {
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            for (int j = 0; j < sh; j++)
            {
                Memcpy16Far(pptr + ((j + y) * pw + bx), (PC98GDCPlanes[i-startpln]) + j * 80, sw >> 1);
            }
        }
    }
}

void DrawRectPortionVGA(GPIInfo* info, int x, int y)
{
    int startpln = 0;
    if (info->hasMask) startpln = 1;
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
    else if (newX + 625 > info->width)
    {
        newX = (info->width - 625) & 0xFFF0;
        dx = newX - oldX;
    }
    if (newY < 0)
    {
        newY = 0;
        dy = newY - oldY;
    }
    else if (newY + 400 > info->height)
    {
        newY = info->height - 400;
        dy = newY - oldY;
    }
    info->displayX = newX;
    info->displayY = newY;

    int startpln = 0;
    if (info->hasMask) startpln = 1;
    int pw = info->byteWidth;
    int sw = pw;
    if (sw > 80) sw = 80;
    int bx = newX/8;
    if (dy > 0)
    {
        int ty = 400 + oldY;
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            for (int j = ty; j < ty + dy; j++)
            {
                if ((((j * 80) + bx) & 0x7FFF) > ((((j * 80) + bx) + sw - 1) & 0x7FFF)) //Account for wraparound
                {
                    int pbx = 0x8000 - (((j * 80) + bx) & 0x7FFF);
                    MemcpyFar(pptr + (j * pw + bx), (PC98GDCPlanes[i-startpln]) + (((j * 80) + bx) & 0x7FFF), pbx);
                    MemcpyFar(pptr + (j * pw + bx + pbx), PC98GDCPlanes[i-startpln], sw-pbx);
                }
                else
                {
                    MemcpyFar(pptr + (j * pw + bx), (PC98GDCPlanes[i-startpln]) + (((j * 80) + bx) & 0x7FFF), sw);
                }
            }
        }
    }
    else if (dy < 0)
    {
        int by = oldY;
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            for (int j = by + dy; j < by; j++)
            {
                if ((((j * 80) + bx) & 0x7FFF) > ((((j * 80) + bx) + sw - 1) & 0x7FFF)) //Account for wraparound
                {
                    int pbx = 0x8000 - (((j * 80) + bx) & 0x7FFF);
                    MemcpyFar(pptr + (j * pw + bx), (PC98GDCPlanes[i-startpln]) + (((j * 80) + bx) & 0x7FFF), pbx);
                    MemcpyFar(pptr + (j * pw + bx + pbx), PC98GDCPlanes[i-startpln], sw-pbx);
                }
                else
                {
                    MemcpyFar(pptr + (j * pw + bx), (PC98GDCPlanes[i-startpln]) + (((j * 80) + bx) & 0x7FFF), sw);
                }
            }
        }
    }

    int sh = info->height;
    if (sh > 400) sh = 400;
    if (dx > 0)
    {
        int rbx = 80 + ((oldX/16) * 2);
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            for (int j = rbx; j < rbx + ((dx/16) * 2); j++)
            {
                for (int k = newY; k < newY + sh; k++)
                {
                    PC98GDCPlanes[i-startpln][((k * 80) + j) & 0x7FFF] = pptr[k * pw + j];
                }
            }
        }
    }
    else if (dx < 0)
    {
        int lbx = (oldX/16) * 2;
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            for (int j = lbx + ((dx/16) * 2); j < lbx; j++)
            {
                for (int k = newY; k < newY + sh; k++)
                {
                    PC98GDCPlanes[i-startpln][((k * 80) + j) & 0x7FFF] = pptr[k * pw + j];
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
    else if (newX + 633 > info->width)
    {
        newX = (info->width - 633) & 0xFFF8;
        dx = newX - oldX;
    }
    if (newY < 0)
    {
        newY = 0;
        dy = newY - oldY;
    }
    else if (newY + 480 > info->height)
    {
        newY = info->height - 480;
        dy = newY - oldY;
    }
    info->displayX = newX;
    info->displayY = newY;

    int startpln = 0;
    if (info->hasMask) startpln = 1;
    int pw = info->byteWidth;
    int sw = pw;
    if (sw > 80) sw = 80;
    int bx = newX/8;
    if (dy > 0)
    {
        int ty = 480 + oldY;
        unsigned int wplane = 0x0001;
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
    }
    else if (dy < 0)
    {
        int by = oldY;
        unsigned int wplane = 0x0001;
        for (int i = startpln; i < info->numPlanes; i++)
        {
            __far unsigned char* pptr = info->planes[i];
            VGASequencerAddress(VGA_SEQUENCER_ADDRESS_MAPMASK);
            VGASequencerDataWrite(wplane);
            for (int j = by + dy; j < by; j++)
            {
                MemcpyFar(pptr + (j * pw + bx), (VGA_BASE_1) + ((j * 80) + bx), sw);
            }
            wplane <<= 1;
        }
    }

    int sh = info->height;
    if (sh > 480) sh = 480;
    if (dx > 0)
    {
        int rbx = 80 + (oldX/8);
        unsigned int wplane = 0x0001;
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
    }
    else if (dx < 0)
    {
        int lbx = oldX/8;
        unsigned int wplane = 0x0001;
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
    }

    int newaX = newX/8;
    VGAScrollRaw((newY * 80) + newaX);
}

void InitDrawing(int type)
{
    switch (type)
    {
        case GRAPHICSHW_PC98_16:
            DrawRectPortion = DrawRectPortionPC98;
            UpdateWithScroll = UpdateWithScrollPC98;
            break;
        case GRAPHICSHW_IBM_VGA:
            DrawRectPortion = DrawRectPortionVGA;
            UpdateWithScroll = UpdateWithScrollVGA;
            break;
    }
}
