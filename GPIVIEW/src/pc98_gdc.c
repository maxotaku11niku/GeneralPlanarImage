//PC-98 basic GDC interface
//Maxim Hoxha 2023-2024

#include "pc98_gdc.h"

const unsigned char quadstoocts[12] = { 2, 5, 6, 1,
                                        3, 4, 7, 0,
                                        3, 5, 7, 1 };

unsigned int displayRegionStartAddress = 0;
unsigned char baseDisplayLineScale = 1;
unsigned short displayLines = 400;
unsigned short totalLines = 400;
unsigned short displayX = 640;
unsigned short displayPitch = 40;

void GDCSetDisplayRegion(unsigned int startaddr, unsigned int lineNumber)
{
    displayRegionStartAddress = startaddr >> 1; //Convert to GDC address
    totalLines = lineNumber;
}

void GDCSetGraphicsLineScale(unsigned char scale)
{
    GDCWriteGraphicsCommand(GDC_COMMAND_CSRFORM);
    GDCWriteGraphicsCommandParam(scale-1);
    baseDisplayLineScale = scale;
}

/* This function currently has the following safe limits:
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
int GDCSetDisplayMode(unsigned int width, unsigned int height, unsigned int scannedLines)
{
    if (width == 0 || height == 0) return GDC_SETDISPMODE_ERROR_ZERODIMENSION; //Trying to make the width or height zero?! You crazy bastard!
    else if (scannedLines <= height) return GDC_SETDISPMODE_ERROR_ZEROVLBANK; //Trying to make the VBLANK period zero or NEGATIVE?! You utter nincompoop!
    else if ((scannedLines - height) < 40) return GDC_SETDISPMODE_ERROR_VBLANKTOOSHORT; //Reject unsafely short VBLANK period
    else if ((scannedLines - height) > 134) return GDC_SETDISPMODE_ERROR_VBLANKTOOLONG; //Reject unsafely long VBLANK period
    else if (width > 640) return GDC_SETDISPMODE_ERROR_HBLANKTOOSHORT; //Reject unsafely short HBLANK period
    unsigned char is5MHz;
    __asm (
        "inb $0x31, %%al\n\t"
        "movb %%al, %0"
    : "=rm" (is5MHz) : : "%al");
    is5MHz = (~is5MHz) & 0x80; //Use this to figure out if the graphics GDC is clocked at 2.5MHz or 5MHz and adjust accordingly

    width = (width + 0xF) & 0xFFF0; //Round width up to nearest multiple of 16
    unsigned int pitch = width >> 4;
    unsigned int hfp = 5; //All these horizontal parameters are for a 2.5 MHz GDC, they are adjusted for a 5 MHz GDC
    unsigned int hs = 4;
    unsigned int hbp = 44 - pitch;
    if (hbp > 32) //Back porch setting has exceeded the safe limit -> start stretching the front porch instead
    {
        unsigned int fpp = hbp - 32;
        hfp += fpp;
        hbp = 32;
    }
    unsigned int pitch5 = 2 * pitch;
    unsigned int spitch5 = pitch5 - 2;
    unsigned int hfp5 = 2 * hfp - 1;
    unsigned int hs5 = 2 * hs - 1;
    unsigned int hbp5 =  2 * hbp - 1;
    unsigned int spitch = pitch - 2;
    hfp--; hs--; hbp--;
    unsigned int vfp = 7;
    unsigned int vs = 8;
    unsigned int vbp = scannedLines - 15 - height;
    if (vbp > 63) //Back porch setting has exceeded the safe limit -> start stretching the front porch instead
    {
        unsigned int fpp = vbp - 63;
        vfp += fpp;
        vbp = 63;
    }

    GDCWriteTextCommand(GDC_COMMAND_SYNC_OFF);
    GDCWriteTextCommandParam(0);
    GDCWriteTextCommandParam((unsigned char)spitch5);
    GDCWriteTextCommandParam(((unsigned char)hs5) | (((unsigned char)vs) << 5));
    GDCWriteTextCommandParam((((unsigned char)vs) >> 3) | (((unsigned char)hfp5) << 2));
    GDCWriteTextCommandParam((unsigned char)hbp5);
    GDCWriteTextCommandParam((unsigned char)vfp);
    GDCWriteTextCommandParam((unsigned char)height);
    GDCWriteTextCommandParam(((unsigned char)(height >> 8)) | (((unsigned char)vbp) << 2));
    GDCWriteTextCommand(GDC_COMMAND_PITCH);
    GDCWriteTextCommandParam((unsigned char)pitch5);

    GDCWriteGraphicsCommand(GDC_COMMAND_SYNC_OFF);
    GDCWriteGraphicsCommandParam(GDC_SYNC_NOCHAR | GDC_SYNC_REFRESH);
    if (is5MHz)
    {
        GDCWriteGraphicsCommandParam((unsigned char)spitch5);
        GDCWriteGraphicsCommandParam(((unsigned char)hs5) | (((unsigned char)vs) << 5));
        GDCWriteGraphicsCommandParam((((unsigned char)vs) >> 3) | (((unsigned char)hfp5) << 2));
        GDCWriteGraphicsCommandParam((unsigned char)hbp5);
        GDCWriteGraphicsCommandParam((unsigned char)vfp);
        GDCWriteGraphicsCommandParam((unsigned char)height);
        GDCWriteGraphicsCommandParam(((unsigned char)(height >> 8)) | (((unsigned char)vbp) << 2));
        GDCWriteGraphicsCommand(GDC_COMMAND_PITCH);
        GDCWriteGraphicsCommandParam((unsigned char)pitch);
    }
    else
    {
        GDCWriteGraphicsCommandParam((unsigned char)spitch);
        GDCWriteGraphicsCommandParam(((unsigned char)hs) | (((unsigned char)vs) << 5));
        GDCWriteGraphicsCommandParam((((unsigned char)vs) >> 3) | (((unsigned char)hfp) << 2));
        GDCWriteGraphicsCommandParam((unsigned char)hbp);
        GDCWriteGraphicsCommandParam((unsigned char)vfp);
        GDCWriteGraphicsCommandParam((unsigned char)height);
        GDCWriteGraphicsCommandParam(((unsigned char)(height >> 8)) | (((unsigned char)vbp) << 2));
        GDCWriteGraphicsCommand(GDC_COMMAND_PITCH);
        GDCWriteGraphicsCommandParam((unsigned char)pitch);
    }

    displayLines = height;
    displayX = width;
    displayPitch = pitch;
    return 0;
}


void GDCScrollSimpleGraphics(unsigned int topline)
{
    unsigned short startaddr = displayRegionStartAddress + displayPitch * topline;
    unsigned short numlinestop = totalLines - topline;
    unsigned char p0 = (unsigned char)startaddr;
    unsigned char p1 = (unsigned char)(startaddr >> 8);
    unsigned char p2 = (unsigned char)(numlinestop << 4);
    unsigned char p3 = (unsigned char)(numlinestop >> 4) & 0x3F;
    unsigned char p4 = (unsigned char)displayRegionStartAddress;
    unsigned char p5 = (unsigned char)(displayRegionStartAddress >> 8);
    unsigned char p6 = 0x00;
    unsigned char p7 = 0x20; //512 lines for second section
    GDCWriteGraphicsCommand(GDC_COMMAND_SCROLL(0));
    GDCWriteGraphicsCommandParam(p0);
    GDCWriteGraphicsCommandParam(p1);
    GDCWriteGraphicsCommandParam(p2);
    GDCWriteGraphicsCommandParam(p3);
    GDCWriteGraphicsCommandParam(p4);
    GDCWriteGraphicsCommandParam(p5);
    GDCWriteGraphicsCommandParam(p6);
    GDCWriteGraphicsCommandParam(p7);
}

void GDCVerticalMosaicEffect(unsigned char scale)
{
    unsigned int maxscale1 = 255 / displayPitch;
    //DOSBOX-X and Neko Project II seem to disagree over whether the maximum practical pitch is 255 or 127
    //The PC-9801 Programmer's Bible implies that the maximum pitch is 255, how does real hardware use this number for the graphics GDC?
    unsigned int maxscale2 = 32 / baseDisplayLineScale;
    unsigned int maxscale;
    if (maxscale1 < maxscale2) maxscale = maxscale1;
    else maxscale = maxscale2;
    if (scale > maxscale) scale = maxscale;
    unsigned char finalscale = scale * baseDisplayLineScale;
    unsigned char finalpitch = scale * displayPitch;
    GDCWriteGraphicsCommand(GDC_COMMAND_CSRFORM);
    GDCWriteGraphicsCommandParam(finalscale-1);
    GDCWriteGraphicsCommand(GDC_COMMAND_PITCH);
    GDCWriteGraphicsCommandParam(finalpitch);
}

void GDCDrawLineGraphics(int x0, int y0, int x1, int y1)
{
    //Set start point
    GDCWriteGraphicsCommand(GDC_COMMAND_WRITE(GDC_MOD_REPLACE));
    GDCWriteGraphicsCommand(GDC_COMMAND_TEXTW);
    GDCWriteGraphicsCommandParam(0xFF);
    GDCWriteGraphicsCommandParam(0xFF);
    unsigned short startaddr = (x0 >> 4) + y0 * displayPitch;
    unsigned char startdot = x0 & 0xF;
    GDCWriteGraphicsCommand(GDC_COMMAND_CSRW);
    GDCWriteGraphicsCommandParam((unsigned char)startaddr);
    GDCWriteGraphicsCommandParam((unsigned char)(startaddr >> 8));
    GDCWriteGraphicsCommandParam(startdot << 4);

    //Calculate octant and get correct deltas
    int dx = x1 - x0;
    int dy = y1 - y0;
    unsigned char quad;
    if (dx < 0)
    {
        if (dy < 0) quad = 1;
        else quad = 2;
    }
    else
    {
        if (dy < 0) quad = 0;
        else quad = 3;
    }
    int adx = dx < 0 ? -dx : dx;
    int ady = dy < 0 ? -dy : dy;
    unsigned char oct;
    if (adx < ady)
    {
        quad += 4;
    }
    else if (adx == ady)
    {
        quad += 8;
    }
    oct = quadstoocts[quad];
    int adi, add;
    switch (oct)
    {
        case 0: adi = ady; add = adx; break;
        case 1: adi = adx; add = ady; break;
        case 2: adi = adx; add = ady; break;
        case 3: adi = ady; add = adx; break;
        case 4: adi = ady; add = adx; break;
        case 5: adi = adx; add = ady; break;
        case 6: adi = adx; add = ady; break;
        case 7: adi = ady; add = adx; break;
    }

    //Calculate vector parameters
    int dc = adi;
    int d = 2 * add - adi;
    int d2 = 2 * (add - adi);
    int d1 = 2 * add;

    GDCWriteGraphicsCommand(GDC_COMMAND_VECTW);
    GDCWriteGraphicsCommandParam(GDC_VECTW_LINE | oct);
    GDCWriteGraphicsCommandParam((unsigned char)dc);
    GDCWriteGraphicsCommandParam((unsigned char)(dc >> 8) & 0x3F);
    GDCWriteGraphicsCommandParam((unsigned char)d);
    GDCWriteGraphicsCommandParam((unsigned char)(d >> 8) & 0x3F);
    GDCWriteGraphicsCommandParam((unsigned char)d2);
    GDCWriteGraphicsCommandParam((unsigned char)(d2 >> 8) & 0x3F);
    GDCWriteGraphicsCommandParam((unsigned char)d1);
    GDCWriteGraphicsCommandParam((unsigned char)(d1 >> 8) & 0x3F);
    GDCWriteGraphicsCommand(GDC_COMMAND_VECTE);
}

void GDCDrawRectangleGraphics(int x0, int y0, int x1, int y1)
{
    //Set start point
    if (x1 < x0)
    {
        int t = x0;
        x0 = x1;
        x1 = t;
    }
    if (y1 < y0)
    {
        int t = y0;
        y0 = y1;
        y1 = t;
    }
    GDCWriteGraphicsCommand(GDC_COMMAND_WRITE(GDC_MOD_REPLACE));
    GDCWriteGraphicsCommand(GDC_COMMAND_TEXTW);
    GDCWriteGraphicsCommandParam(0xFF);
    GDCWriteGraphicsCommandParam(0xFF);
    unsigned short startaddr = (x0 >> 4) + y0 * displayPitch;
    unsigned char startdot = x0 & 0xF;
    GDCWriteGraphicsCommand(GDC_COMMAND_CSRW);
    GDCWriteGraphicsCommandParam((unsigned char)startaddr);
    GDCWriteGraphicsCommandParam((unsigned char)(startaddr >> 8));
    GDCWriteGraphicsCommandParam(startdot << 4);

    //Calculate octant and get correct deltas
    unsigned char oct = 0;
    int dx = x1 - x0;
    int dy = y1 - y0;

    //Calculate vector parameters
    int dc = 3;
    int d = dy;
    int d2 = dx;
    int d1 = -1;
    int dm = dy;

    GDCWriteGraphicsCommand(GDC_COMMAND_VECTW);
    GDCWriteGraphicsCommandParam(GDC_VECTW_RECT | oct);
    GDCWriteGraphicsCommandParam((unsigned char)dc);
    GDCWriteGraphicsCommandParam((unsigned char)(dc >> 8) & 0x3F);
    GDCWriteGraphicsCommandParam((unsigned char)d);
    GDCWriteGraphicsCommandParam((unsigned char)(d >> 8) & 0x3F);
    GDCWriteGraphicsCommandParam((unsigned char)d2);
    GDCWriteGraphicsCommandParam((unsigned char)(d2 >> 8) & 0x3F);
    GDCWriteGraphicsCommandParam((unsigned char)d1);
    GDCWriteGraphicsCommandParam((unsigned char)(d1 >> 8) & 0x3F);
    GDCWriteGraphicsCommandParam((unsigned char)dm);
    GDCWriteGraphicsCommandParam((unsigned char)(d1 >> 8) & 0x3F);
    GDCWriteGraphicsCommand(GDC_COMMAND_VECTE);
}

void GDCDrawFilledRectangleGraphics(int x0, int y0, int x1, int y1)
{
    //Set start point
    if (x1 < x0)
    {
        int t = x0;
        x0 = x1;
        x1 = t;
    }
    if (y1 < y0)
    {
        int t = y0;
        y0 = y1;
        y1 = t;
    }
    GDCWriteGraphicsCommand(GDC_COMMAND_WRITE(GDC_MOD_REPLACE));
    GDCWriteGraphicsCommand(GDC_COMMAND_TEXTW);
    GDCWriteGraphicsCommandParam(0xFF);
    GDCWriteGraphicsCommandParam(0xFF);
    GDCWriteGraphicsCommandParam(0xFF);
    GDCWriteGraphicsCommandParam(0xFF);
    GDCWriteGraphicsCommandParam(0xFF);
    GDCWriteGraphicsCommandParam(0xFF);
    GDCWriteGraphicsCommandParam(0xFF);
    GDCWriteGraphicsCommandParam(0xFF);
    unsigned short startaddr = (x0 >> 4) + y0 * displayPitch;
    unsigned char startdot = x0 & 0xF;
    GDCWriteGraphicsCommand(GDC_COMMAND_CSRW);
    GDCWriteGraphicsCommandParam((unsigned char)startaddr);
    GDCWriteGraphicsCommandParam((unsigned char)(startaddr >> 8));
    GDCWriteGraphicsCommandParam(startdot << 4);

    //Calculate octant and get correct deltas
    unsigned char oct = 0;
    int dx = x1 - x0;
    int dy = y1 - y0;

    //Calculate vector parameters
    int dc = dx;
    int d = dy + 1;
    int d2 = dy + 1;

    GDCWriteGraphicsCommand(GDC_COMMAND_VECTW);
    GDCWriteGraphicsCommandParam(GDC_VECTW_TILE | oct);
    GDCWriteGraphicsCommandParam((unsigned char)dc);
    GDCWriteGraphicsCommandParam((unsigned char)(dc >> 8) & 0x3F);
    GDCWriteGraphicsCommandParam((unsigned char)d);
    GDCWriteGraphicsCommandParam((unsigned char)(d >> 8) & 0x3F);
    GDCWriteGraphicsCommandParam((unsigned char)d2);
    GDCWriteGraphicsCommandParam((unsigned char)(d2 >> 8) & 0x3F);
    GDCWriteGraphicsCommand(GDC_COMMAND_TEXTE);
}

void GDCDrawArcGraphics(int xc, int yc, int r, int as, int af)
{
    //stub
}
