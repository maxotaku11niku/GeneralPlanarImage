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
 * Main function
 */

#include "doscalls.h"
#include "graphicshw.h"
#include "drawing.h"

void ShowHelp()
{
    DOSConsoleWriteString(
        "Usage:\r\n"
        "gpiview <filename> [options]\r\n\n"
        "Options:\r\n"
        "-h : Show this help.\r\n$"
    );
}


int main(int argc, char** argv)
{
    DOSConsoleWriteString("GPIVIEW - GPI Image Viewer for MS-DOS\r\n$");
    if (argc < 2)
    {
        DOSConsoleWriteString("Please put the file name of the file you want to view.\r\n$");
        ShowHelp();
        return 1;
    }
    GPIInfo imageInfo;
    if ((argv[1][0] == '-' || argv[1][0] == '/') && argv[1][1] == 'h')
    {
        ShowHelp();
        return 0;
    }
    if (OpenGPIFile(argv[1], &imageInfo))
    {
        DOSConsoleWriteString("Couldn't open the file.\r\n$");
        return 1;
    }

    int graphicshwType = FindGraphicsHardware();

    if (InitGraphicsHardware(&graphicshwType, &imageInfo))
    {
        InitDrawing(graphicshwType);
        int machinetype = graphicshwType & 0xF0;
        if (machinetype == MACHINE_PC98) //Initialise keyboard BIOS on PC-98
        {
            __asm volatile (
                "movb $0x03, %%ah\n\t"
                "int $0x18"
            : : : "%ah");
        }
        int kc = 0;

        DecompressGPIFile(&imageInfo);
        DrawRectPortion(&imageInfo, 0, 0);

        while(1)
        {
            switch (machinetype) //Use BIOS to get a keycode (funnily enough the process is similar between these two machines)
            {
                case MACHINE_PC98:
                    __asm volatile (
                        "movb $0x01, %%ah\n\t" //We need to check if the typeahead buffer actually has a key, since INT 18 function 0 blocks if it's empty (very bad!!)
                        "int $0x18\n\t"
                        "test %%bh, %%bh\n\t"
                        "jnz .buffull%=\n\t"
                        "movb $0x00, %%al\n\t" //Buffer empty -> return NULL code
                        "jmp .bufempty%=\n\t"
                        ".buffull%=: movb $0x00, %%ah\n\t"
                        "int $0x18\n\t"
                        ".bufempty%=: movb $0x00, %%ah\n\t" //Don't care about the scan code
                    : "=a" (kc) : : "%bh");
                    break;
                case MACHINE_IBM:
                    __asm volatile (
                        "movb $0x01, %%ah\n\t" //We need to check if the typeahead buffer actually has a key, since INT 16 function 0 blocks if it's empty (very bad!!)
                        "int $0x16\n\t"
                        "jnz .buffull%=\n\t"
                        "movb $0x00, %%al\n\t" //Buffer empty -> return NULL code
                        "jmp .bufempty%=\n\t"
                        ".buffull%=: movb $0x00, %%ah\n\t"
                        "int $0x16\n\t"
                        ".bufempty%=: movb $0x00, %%ah\n\t" //Don't care about the scan code
                    : "=a" (kc) :);
                    break;
            }
            if (kc == 0x1B) //Stop playback if ESC is pressed
            {
                break;
            }
            if ((kc & 0xDF) == 0x57) //Move up the image if W is pressed
            {
                UpdateWithScroll(&imageInfo, 0, -16);
            }
            if ((kc & 0xDF) == 0x53) //Move down the image if S is pressed
            {
                UpdateWithScroll(&imageInfo, 0, 16);
            }
            if ((kc & 0xDF) == 0x41) //Move left the image if A is pressed
            {
                UpdateWithScroll(&imageInfo, -16, 0);
            }
            if ((kc & 0xDF) == 0x44) //Move right the image if D is pressed
            {
                UpdateWithScroll(&imageInfo, 16, 0);
            }
        }

        ResetGraphicsHardware(graphicshwType);
        CloseGPIFile(&imageInfo);
        return 0;
    }
    else
    {
        CloseGPIFile(&imageInfo);
        return 2;
    }
}
