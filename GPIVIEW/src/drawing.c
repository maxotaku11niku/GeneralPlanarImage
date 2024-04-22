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
#include "graphicshw.h"
#include "drawing.h"
#include "x86strops.h"

__far unsigned char* PC98GDCPlanes[4] = { GDC_PLANE0, GDC_PLANE1, GDC_PLANE2, GDC_PLANE3 };

//Abstract function pointers
void (*DrawRectPortion)(GPIInfo* info, int x, int y);

void DrawRectPortionPC98(GPIInfo* info, int x, int y)
{
    int startpln = 0;
    if (info->hasMask) startpln = 1;
    int pw = info->byteWidth;
    int h = info->height;
    int sw = pw;
    if (sw > 80) sw = 80;
    int sh = h;
    if (sh > 400) sh = 400;
    int bx = x/8;
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

}

void InitDrawing(int type)
{
    switch (type)
    {
        case GRAPHICSHW_PC98_16:
            DrawRectPortion = DrawRectPortionPC98;
            break;
        case GRAPHICSHW_IBM_VGA:
            DrawRectPortion = DrawRectPortionVGA;
            break;
    }
}
