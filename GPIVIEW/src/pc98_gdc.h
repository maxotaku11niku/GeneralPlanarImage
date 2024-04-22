//PC-98 basic GDC interface
//Maxim Hoxha 2023-2024

#pragma once

//Plane segments
#define GDC_PLANE0_SEGMENT 0xA800
#define GDC_PLANE1_SEGMENT 0xB000
#define GDC_PLANE2_SEGMENT 0xB800
#define GDC_PLANE3_SEGMENT 0xE000
//Use this to signify that the planes are being accessed in parallel (GRCG, EGC etc.)
#define GDC_PLANES_SEGMENT 0xA800
//Plane address pointers
#define GDC_PLANE0 ((unsigned __far char*)0xA8000000)
#define GDC_PLANE1 ((unsigned __far char*)0xB0000000)
#define GDC_PLANE2 ((unsigned __far char*)0xB8000000)
#define GDC_PLANE3 ((unsigned __far char*)0xE0000000)
//Use this to signify that the planes are being accessed in parallel (GRCG, EGC etc.)
#define GDC_PLANES ((unsigned __far char*)0xA8000000)

//OUTPORT 68 - Write GDC Mode 1
//Sets the graphics mode first part, can only change each attribute one at a time
inline void GDCSetMode1(unsigned char mode)
{
    volatile register unsigned char m __asm("%al");
    m = mode;
    __asm volatile (
        "outb %%al, $0x68"
    : : "a" (m));
}
//Supporting defines
#define GDC_MODE1_ATTRIBUTE4_VERTICALLINE 0x00
#define GDC_MODE1_ATTRIBUTE4_PATTERN      0x01
#define GDC_MODE1_COLOUR                  0x02
#define GDC_MODE1_MONOCHROME              0x03
#define GDC_MODE1_80COLUMN                0x04
#define GDC_MODE1_40COLUMN                0x05
#define GDC_MODE1_6x8CHARS                0x06
#define GDC_MODE1_7x13CHARS               0x07
#define GDC_MODE1_LINEDOUBLE_ON           0x08
#define GDC_MODE1_LINEDOUBLE_OFF          0x09
#define GDC_MODE1_CHARACCESS_CODE         0x0A
#define GDC_MODE1_CHARACCESS_BITMAP       0x0B
#define GDC_MODE1_MEMSWITCH_WRITE_ON      0x0C
#define GDC_MODE1_MEMSWITCH_WRITE_OFF     0x0D
#define GDC_MODE1_DISPLAY_ON              0x0E
#define GDC_MODE1_DISPLAY_OFF             0x0F

//OUTPORT 60 - Write GDC Text Command Parameter
//Writes a parameter for a text GDC command
inline void GDCWriteTextCommandParam(unsigned char param)
{
    volatile register unsigned char p __asm("%al");
    p = param;
    __asm volatile (
        "outb %%al, $0x60"
    : : "a" (p));
}

//OUTPORT 62 - Write GDC Text Command
//Writes to the text GDC to start a command
inline void GDCWriteTextCommand(unsigned char command)
{
    volatile register unsigned char c __asm("%al") = command;
    c = command;
    __asm volatile (
        "outb %%al, $0x62"
    : : "a" (c));
}
//Supporting defines (all of these commands will be abstracted behind proper functions)
//Reinitialises this GDC
#define GDC_COMMAND_RESET 0x00
//Sets display timing parameters, which can be VERY dangerous. If you don't know what you're doing, just use the BIOS functions.
#define GDC_COMMAND_SYNC_OFF 0x0E
#define GDC_COMMAND_SYNC_ON  0x0F
#define GDC_SYNC_SHRINK      0x01
#define GDC_SYNC_NOCHAR      0x02
#define GDC_SYNC_REFRESH     0x04
#define GDC_SYNC_INTERLACE   0x08
#define GDC_SYNC_NOFLASH     0x10
#define GDC_SYNC_NOGRAPH     0x20
//Start display of this layer
#define GDC_COMMAND_START 0x0D
//Stop display of this layer
#define GDC_COMMAND_STOP 0x0C
//Sets zoom values
#define GDC_COMMAND_ZOOM 0x46
//Sets which portions of the screen get displayed and which VRAM portions they correspond to, useful for hardware scrolling
#define GDC_COMMAND_SCROLL(startaddr) (0x70 | ((startaddr) & 0xF))
//Sets the text cursor format
#define GDC_COMMAND_CSRFORM 0x4B
//Sets the number of words per line
#define GDC_COMMAND_PITCH 0x47
//Reads out the position of the light pen
#define GDC_COMMAND_LPEN 0xC0
//Prepares a draw command
#define GDC_COMMAND_VECTW 0x4C
#define GDC_VECTW_DOT         0x00
#define GDC_VECTW_LINE        0x08
#define GDC_VECTW_TILE        0x10
#define GDC_VECTW_CIRCLE      0x20
#define GDC_VECTW_RECT        0x40
#define GDC_VECTW_SLANTEDTILE 0x90
//Executes a draw command
#define GDC_COMMAND_VECTE 0x6C
//Prepares a little pattern
#define GDC_COMMAND_TEXTW 0x78
//Draws the previously prepared pattern
#define GDC_COMMAND_TEXTE 0x68
//Sets the position of the cursor/drawing start position
#define GDC_COMMAND_CSRW 0x49
//Reads out the position of the cursor/drawing start position
#define GDC_COMMAND_CSRR 0xE0
//Sets the write mask register
#define GDC_COMMAND_MASK 0x4A
//Sets the write operation of any draw to VRAM
#define GDC_COMMAND_WRITE(mod) (0x20 | (mod))
//Sets the read operation of any read from VRAM
#define GDC_COMMAND_READ(mod) (0xA0 | (mod))
//GDC read/write mods
//DST = SRC
#define GDC_MOD_REPLACE 0x00
//DST = SRC XOR DST
#define GDC_MOD_XOR 0x01
//DST = !SRC AND DST
#define GDC_MOD_CLEAR 0x02
//DST = (!SRC AND DST) OR SRC
#define GDC_MOD_SET 0x03

//INPORT 60 - Read GDC Text Status
//Read the status of the text GDC
inline unsigned char GDCReadTextStatus()
{
    volatile register unsigned char status __asm("%al");
    __asm volatile ("inb $0x60, %al");
    return status;
}
//Supporting defines
#define GDC_STATUS_DATAREADY  0x01
#define GDC_STATUS_FIFO_FULL  0x02
#define GDC_STATUS_FIFO_EMPTY 0x04
//Only makes sense for the graphics GDC
#define GDC_STATUS_DRAWING 0x08
//Not applicable to the PC-98
#define GDC_STATUS_DMA_EXECUTE 0x10
//Please just use an interrupt if you need to VSYNC something
#define GDC_STATUS_VBLANK 0x20
//There is no HSYNC interrupt, however, so feel free to poll for HSYNC if you need to (or use a timer closely synced to the line frequency)
#define GDC_STATUS_HBLANK 0x40
//Not applicable to the PC-98 (and even if so, actually requires a CRT anyway)
#define GDC_STATUS_LIGHTPEN_DETECT 0x80

//INPORT 62 - Read GDC Text Command Data
//Read the data returned from a text GDC command
inline unsigned char GDCReadTextCommandData()
{
    volatile register unsigned char data __asm("%al");
    __asm volatile (
        "inb $0x62, %%al"
    : "=a" (data) : );
    return data;
}


//OUTPORT 64 - CRT Interrupt Reset
//IMPORTANT - call after you allow VSYNC interrupts to prevent a spinlock
inline void GDCInterruptReset()
{
    __asm volatile ("outb %al, $0x64"); //What's in al doesn't actually matter
}

//OUTPORT 6C - Set Border Colour
//This colour probably won't show up on an emulator or a modern flat display
inline void GDCSetBorderColour(unsigned char col)
{
    volatile register unsigned char c __asm("%al");
    c = col;
    __asm volatile (
        "outb %%al, $0x6C"
    : : "a" (c));
}

//OUTPORT 6A - Write GDC Mode 2
//Sets the graphics mode second part, can only change each attribute one at a time
inline void GDCSetMode2(unsigned char mode)
{
    volatile register unsigned char m __asm("%al");
    m = mode;
    __asm volatile (
        "outb %%al, $0x6A"
    : : "a" (m));
}
//Supporting defines
//You probably don't want to use this mode, as the VX supports 16 colours anyway
#define GDC_MODE2_8COLOURS 0x00
//The mode usually associated with the PC-98, though does require a model that supports it
#define GDC_MODE2_16COLOURS 0x01
//Requires the EGC (put a GDC_MODE2_MODIFY through first)
#define GDC_MODE2_GRCG 0x04
#define GDC_MODE2_EGC  0x05
//Requires a suitable hardware feature that can be modified through port 6A, otherwise these are useless
#define GDC_MODE2_NOMODIFY 0x06
#define GDC_MODE2_MODIFY   0x07
//Requires 256-colour hardware (either MATE/MULTI builtin, or H98, put a GDC_MODE2_MODIFY through first)
#define GDC_MODE2_256COLOURS_OFF  0x20
#define GDC_MODE2_256COLOURS_ON   0x21
#define GDC_MODE2_PAGE_DISCONNECT 0x68
#define GDC_MODE2_PAGE_CONNECT    0x69

//OUTPORT A0 - Write GDC Graphics Command Parameter
//Writes a parameter for a graphics GDC command
inline void GDCWriteGraphicsCommandParam(unsigned char param)
{
    volatile register unsigned char p __asm("%al");
    p = param;
    __asm volatile (
        "outb %%al, $0xA0"
    : : "a" (p));
}
//OUTPORT A2 - Write GDC Graphics Command
//Writes to the graphics GDC to start a command
inline void GDCWriteGraphicsCommand(unsigned char command)
{
    volatile register unsigned char c __asm("%al");
    c = command;
    __asm volatile (
        "outb %%al, $0xA2"
    : : "a" (c));
}
//OUTPORT A4 - Set Display Page
//There are two pages, this sets which page is shown
inline void GDCSetGraphicsDisplayPage(unsigned char page)
{
    volatile register unsigned char p __asm("%al");
    p = page;
    __asm volatile (
        "outb %%al, $0xA4"
    : : "a" (p));
}

//OUTPORT A6 - Set Draw Page
//There are two pages, this sets which page can be drawn to
inline void GDCSetGraphicsDrawPage(unsigned char page)
{
    volatile register unsigned char p __asm("%al");
    p = page;
    __asm volatile (
        "outb %%al, $0xA6"
    : : "a" (p));
}

//INPORT A0 - Read GDC Graphics Status
//#define gdc_readgraphicstatus(status) inportb(0xA0, status)
//Read the status of the graphics GDC
inline unsigned char GDCReadGraphicsStatus()
{
    volatile register unsigned char status __asm("%al");
    __asm volatile (
        "inb $0xA0, %%al"
    : "=a" (status) : );
    return status;
}
//INPORT A2 - Read GDC Graphics Command Data
//#define gdc_readgraphiccommanddata(data) inportb(0xA2, data)
//Read the data returned from a graphics GDC command
inline unsigned char GDCReadGraphicsCommandData()
{
    volatile register unsigned char data __asm("%al");
    __asm volatile (
        "inb $0xA2, %%al"
    : "=a" (data) : );
    return data;
}

//Resets the text GDC
inline void GDCResetText()
{
    GDCWriteTextCommand(GDC_COMMAND_RESET);
}

//Start displaying the text layer
inline void GDCStartText()
{
    GDCWriteTextCommand(GDC_COMMAND_START);
}

//Stop displaying the text layer
inline void GDCStopText()
{
    GDCWriteTextCommand(GDC_COMMAND_STOP);
}

//Input raw scroll parameters for the text layer
inline void GDCScrollRawText(unsigned char region, unsigned short startaddr, unsigned short numlines)
{
    unsigned char p0 = (unsigned char)startaddr;
    unsigned char p1 = (unsigned char)(startaddr >> 8) & 0x1F;
    unsigned char p2 = (unsigned char)(numlines << 4);
    unsigned char p3 = (unsigned char)(numlines >> 4) & 0x3F;
    GDCWriteTextCommand(GDC_COMMAND_SCROLL(region << 2));
    GDCWriteTextCommandParam(p0);
    GDCWriteTextCommandParam(p1);
    GDCWriteTextCommandParam(p2);
    GDCWriteTextCommandParam(p3);
}

//Resets the graphics GDC
inline void GDCResetGraphics()
{
    GDCWriteGraphicsCommand(GDC_COMMAND_RESET);
}

//Start displaying the graphics layer
inline void GDCStartGraphics()
{
    GDCWriteGraphicsCommand(GDC_COMMAND_START);
}

//Stop displaying the graphics layer
inline void GDCStopGraphics()
{
    GDCWriteGraphicsCommand(GDC_COMMAND_STOP);
}

//Input raw scroll parameters for the graphics layer
inline void GDCScrollRawGraphics(unsigned char region, unsigned long startaddr, unsigned short numlines)
{
    unsigned char p0 = (unsigned char)startaddr;
    unsigned char p1 = (unsigned char)(startaddr >> 8);
    unsigned char p2 = (unsigned char)(numlines << 4) | ((unsigned char)(startaddr >>16) & 0x3);
    unsigned char p3 = (unsigned char)(numlines >> 4) & 0x3F;
    GDCWriteGraphicsCommand(GDC_COMMAND_SCROLL(region << 2));
    GDCWriteGraphicsCommandParam(p0);
    GDCWriteGraphicsCommandParam(p1);
    GDCWriteGraphicsCommandParam(p2);
    GDCWriteGraphicsCommandParam(p3);
}

//Sets all 8 colours in the most basic palette. Only used if you're in 8-colour mode for some reason. Upper 4 bits for colours 0-3, lower 4 bits for colours 4-7.
inline void GDCSet8ColoursPalette(unsigned char col04, unsigned char col15, unsigned char col26, unsigned char col37)
{
    volatile register unsigned char c __asm("%al");
    c = col04;
    __asm volatile (
        "out %%al, $0xAE"
    : : "a" (c));
    c = col15;
    __asm volatile (
        "out %%al, $0xAA"
    : : "a" (c));
    c = col26;
    __asm volatile (
        "out %%al, $0xAC"
    : : "a" (c));
    c = col37;
    __asm volatile (
        "out %%al, $0xA8"
    : : "a" (c));
}

//Sets a single colour in the usual 16 colour palette. Each parameter is in the range 0x0-0xF
//If in 256-colour mode, each parameter is in the range 0x00-0xFF
inline void GDCSetPaletteColour(unsigned char index, unsigned char r, unsigned char g, unsigned char b)
{
    volatile register unsigned char i __asm("%al");
    i = index;
    __asm volatile (
        "out %%al, $0xA8"
    : : "a" (i));
    i = r;
    __asm volatile (
        "out %%al, $0xAC"
    : : "a" (i));
    i = g;
    __asm volatile (
        "out %%al, $0xAA"
    : : "a" (i));
    i = b;
    __asm volatile (
        "out %%al, $0xAE"
    : : "a" (i));
}

//Sets the start address and number of lines in the display region, use a byte offset address
void GDCSetDisplayRegion(unsigned int startaddr, unsigned int lineNumber);

//Set the vertical downsample factor
void GDCSetGraphicsLineScale(unsigned char scale);

/* Sets the display mode to some custom resolution and framerate. The display will be turned off after this, so reenable each desired GDC
 * 'width' and 'height' set the displayed width and height, 'scannedLines' determines the framerate as hsync frequency/scannedLines
 * This function currently has the following safe limits:
 * Width: 16 - 640 pixels, must be a multiple of 16
 * scannedLines - height (total vertical blanking period): 40 - 134
 * 50-70 Hz with hsync frequency at 24.82 kHz -> scannedLines: 355 - 496
 * 50-70 Hz with hsync frequency at 24.82 kHz -> height: 221 - 456
 * 50 Hz with hsync frequency at 24.82 kHz ->   height: 362 - 456 (scannedLines = 496)
 * 56.4 Hz with hsync frequency at 24.82 kHz -> height: 306 - 400 (scannedLines = 440) <- this is the default frequency for a PC-98
 * 60 Hz with hsync frequency at 24.82 kHz ->   height: 280 - 374 (scannedLines = 414)
 * 65 Hz with hsync frequency at 24.82 kHz ->   height: 248 - 342 (scannedLines = 382)
 * 70 Hz with hsync frequency at 24.82 kHz ->   height: 221 - 315 (scannedLines = 355)
 * WARNING: This function will NEVER check if the desired vsync frequency is safe for the monitor. You have been warned!
 */
int GDCSetDisplayMode(unsigned int width, unsigned int height, unsigned int scannedLines);
//Supporting defines
#define GDC_SETDISPMODE_ERROR_ZERODIMENSION  0x01
#define GDC_SETDISPMODE_ERROR_ZEROVLBANK     0x02
#define GDC_SETDISPMODE_ERROR_VBLANKTOOSHORT 0x03
#define GDC_SETDISPMODE_ERROR_VBLANKTOOLONG  0x04
#define GDC_SETDISPMODE_ERROR_HBLANKTOOSHORT 0x05

//Does a simple vertical scroll of the screen
void GDCScrollSimpleGraphics(unsigned int topline);

//Reprograms the GDC in such a way as to produce a vertical mosaic effect
void GDCVerticalMosaicEffect(unsigned char scale);

//Uses the GDC to draw a straight line
void GDCDrawLineGraphics(int x0, int y0, int x1, int y1);

//Uses the GDC to draw a rectangle outline
void GDCDrawRectangleGraphics(int x0, int y0, int x1, int y1);

//Uses the GDC to draw a filled rectangle
void GDCDrawFilledRectangleGraphics(int x0, int y0, int x1, int y1);

//Uses the GDC to draw a circular arc, angles 'as' and 'af' are in units of full revolutions/65536
void GDCDrawArcGraphics(int xc, int yc, int r, int as, int af);
