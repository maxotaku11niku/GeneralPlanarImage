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
 * Graphics hardware interface
 */

#include "pc98_gdc.h"
#include "ibm_vga.h"
#include "graphicshw.h"

//Determines whether or not this program is running on a PC-98 by trying to call the printer BIOS
//This should work on an emulator without the official BIOS since BIOS calls are usually reimplemented
int IsOnPC98()
{
    unsigned int outres;
    __asm volatile (
        "mov $0x1000, %0\n\t"
        "clc\n\t" //According to a contributor to doslib, some IBM-compatible PC BIOSes set the carry flag on return from an invalid software interrupt
        "mov $0x1000, %%ax\n\t" //Those same BIOSes might return with ax != 0x1000, so we must account for them
        "int $0x1A\n\t" //Printer BIOS software interrupt vector (on a PC-98, usually invalid on other machines)
        "jc .buggyIBM%=\n\t" //We account for those weird BIOSes here (other BIOSes leave things alone, the PC-98 BIOS mucks around with ax but not the carry flag)
        "mov %%ax, %0\n\t"
        ".buggyIBM%=:"
    : "=rm" (outres) : : "%ax");
    if (outres != 0x1000) return 1; //Definitely a PC-98
    else return 0; //Not a PC-98
}
//For what it's worth, it does return the correct value on real IBM-compatible hardware (my laptop from 2019)

int FindGraphicsHardware()
{
    //Check which machine we're running on
    if (IsOnPC98())
    {
        return GRAPHICSHW_PC98_16;
    }
    else //Assume that if the machine is not any of the above it's a normal IBM-compatible PC with a VGA
    {
        return GRAPHICSHW_IBM_VGA;
    }
}

void VGAInitBasicGraphics()
{
    //Temporarily rely on the video BIOS
    __asm volatile (
        "movw $0x0012, %%ax\n\t"
        "int $0x10"
    : : : "%ax");
    //Set palette mapping to sensible values
    for (int i = 0; i < 16; i++)
    {
        VGAAttributeControllerAddress(i);
        VGAAttributeControllerDataWrite(i);
        VGAAttributeControllerAddress(i); //????
    }
    VGAAttributeControllerAddress(VGA_ATTRIBUTE_ADDRESS_NOLOAD | VGA_ATTRIBUTE_ADDRESS_COLOURSELECT);
    VGAAttributeControllerDataWrite(0);
}

void VGAResetBasicGraphics()
{
    //Temporarily rely on the video BIOS
    __asm volatile (
        "movw $0x0003, %%ax\n\t"
        "int $0x10"
    : : : "%ax");
}

void PC98InitBasicGraphics()
{
    GDCSetDisplayMode(640, 400, 440);
    GDCStopText();
    GDCStartGraphics();
    GDCSetGraphicsLineScale(1);
    GDCSetMode1(GDC_MODE1_LINEDOUBLE_ON);
    GDCSetMode1(GDC_MODE1_COLOUR);
    GDCSetGraphicsDisplayPage(0);
    GDCSetGraphicsDrawPage(0);
    GDCSetMode2(GDC_MODE2_16COLOURS);
    GDCSetDisplayRegion(0x0000, 400);
    GDCScrollSimpleGraphics(0);
}

void PC98ResetBasicGraphics()
{
    GDCSetPaletteColour(0x0, 0x0, 0x0, 0x0);
    GDCSetDisplayMode(640, 400, 440);
    GDCStartText();
    GDCStopGraphics();
}

int InitGraphicsHardware(int* type, GPIInfo* info)
{
    switch (*type)
    {
        case GRAPHICSHW_IBM_VGA:
            if (info->palSize > 16)
            {
                DOSConsoleWriteString("GPI file has more than 16 colours, but this hardware's method to display that many colours is not yet supported.\r\n$");
                return 0;
            }
            VGAInitBasicGraphics();
            VGADACWriteAddress(0);
            for (int i = 0; i < info->palSize; i++)
            {
                short r = info->palette[i * 3];
                r >>= 2;
                short g = info->palette[i * 3 + 1];
                g >>= 2;
                short b = info->palette[i * 3 + 2];
                b >>= 2;
                VGADACColourWrite(r, g, b);
            }
            return 1;
        case GRAPHICSHW_PC98_16:
            if (info->palSize > 16)
            {
                DOSConsoleWriteString("GPI file has more than 16 colours, but this hardware cannot display that many.\r\n$");
                return 0;
            }
            PC98InitBasicGraphics();
            for (int i = 0; i < info->palSize; i++)
            {
                short r = info->palette[i * 3];
                r += 0x08; r /= 0x11;
                short g = info->palette[i * 3 + 1];
                g += 0x08; g /= 0x11;
                short b = info->palette[i * 3 + 2];
                b += 0x08; b /= 0x11;
                GDCSetPaletteColour(i, r, g, b);
            }
            return 1;
        case GRAPHICSHW_PC98_MATE256:
            return 0; //TODO
        case GRAPHICSHW_PC98_H98:
            return 0; //TODO
    }
}

void ResetGraphicsHardware(int type)
{
    switch (type)
    {
        case GRAPHICSHW_IBM_VGA:
            VGAResetBasicGraphics();
            return; //TODO
        case GRAPHICSHW_PC98_16:
            PC98ResetBasicGraphics();
            return;
        case GRAPHICSHW_PC98_MATE256:
            PC98ResetBasicGraphics();
            return; //TODO
        case GRAPHICSHW_PC98_H98:
            PC98ResetBasicGraphics();
            return; //TODO
    }
}
