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

#pragma once

#include "gpimage.h"

#define MACHINE_IBM  0x00 //IBM-compatible PC, must have legacy support (if it can't boot MS-DOS, it doesn't have legacy support)
#define MACHINE_PC98 0x10 //PC-98, Japan-exclusive legacy platform

#define GRAPHICSHW_IBM_VGA      0x00 //IBM PC, VGA-compatible
#define GRAPHICSHW_PC98_16      0x10 //PC-98, 16-out-of-4096 colour compatible
#define GRAPHICSHW_PC98_MATE256 0x11 //PC-98, 256 colour compatible (MATE)
#define GRAPHICSHW_PC98_H98     0x12 //PC-98, 256 colour compatible H98)
#define GRAPHICSHW_UNSUPPORTED  0xFF //Unrecognised graphics hardware

//Finds graphics hardware and reports back with what it found
int FindGraphicsHardware();
//Initialises found graphics hardware
int InitGraphicsHardware(int* type, GPIInfo* info);
//Resets graphics hardware back to defaults
void ResetGraphicsHardware(int type);
