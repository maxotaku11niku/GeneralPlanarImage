//VGA interface
//Maxim Hoxha 2024

#pragma once

//VRAM segments
#define VGA_BASE_0_SEGMENT 0xA000
#define VGA_BASE_1_SEGMENT 0xA000
#define VGA_BASE_2_SEGMENT 0xB000
#define VGA_BASE_3_SEGMENT 0xB800

//VRAM address pointers
#define VGA_BASE_0 ((unsigned __far char*)0xA0000000)
#define VGA_BASE_1 ((unsigned __far char*)0xA0000000)
#define VGA_BASE_2 ((unsigned __far char*)0xB0000000)
#define VGA_BASE_3 ((unsigned __far char*)0xB8000000)

//OUTPORT 03C2 - Miscellaneous Output Register Write
inline void VGAMiscOutputWrite(unsigned char mode)
{
    volatile register unsigned char m __asm("%al");
    m = mode;
    __asm volatile (
        "movw $0x03C2, %%dx\n\t"
        "outb %%al, %%dx"
    : : "a" (m) : "%dx");
}
//INPORT 03CC - Miscellaneous Output Register Read
inline unsigned char VGAMiscOutputRead()
{
    volatile register unsigned char data __asm("%al");
    __asm volatile (
        "movw $0x03CC, %%dx\n\t"
        "inb %%dx, %%al"
    : "=a" (data) : : "%dx");
    return data;
}
//Supporting defines
#define VGA_MISCOUT_IOADDR_3B   0x00
#define VGA_MISCOUT_IOADDR_3D   0x01
#define VGA_MISCOUT_RAM_DISABLE 0x00
#define VGA_MISCOUT_RAM_ENABLE  0x02
#define VGA_MISCOUT_CLOCK_25MHZ 0x00
#define VGA_MISCOUT_CLOCK_28MHZ 0x04
#define VGA_MISCOUT_CLOCK_EXT1  0x08
#define VGA_MISCOUT_CLOCK_EXT2  0x0C
#define VGA_MISCOUT_OEPAGE_LOW  0x00
#define VGA_MISCOUT_OEPAGE_HIGH 0x20
#define VGA_MISCOUT_HSYNCP_POS  0x00
#define VGA_MISCOUT_HSYNCP_NEG  0x40
#define VGA_MISCOUT_VSYNCP_POS  0x00
#define VGA_MISCOUT_VSYNCP_NEG  0x80

//INPORT 03C2 - Input Status 0
inline unsigned char VGAInputStatus0()
{
    volatile register unsigned char data __asm("%al");
    __asm volatile (
        "movw $0x03C2, %%dx\n\t"
        "inb %%dx, %%al"
    : "=a" (data) : : "%dx");
    return data;
}
//Supporting defines
#define VGA_INSTAT0_SS 0x10

//INPORT 03DA - Input Status 1
inline unsigned char VGAInputStatus1()
{
    volatile register unsigned char data __asm("%al");
    __asm volatile (
        "movw $0x03DA, %%dx\n\t"
        "inb %%dx, %%al"
    : "=a" (data) : : "%dx");
    return data;
}
//Supporting defines
#define VGA_INSTAT1_VBLANK    0x08
#define VGA_INSTAT1_NODISPLAY 0x01

//OUTPORT 03C0 - Attribute Controller Address
inline void VGAAttributeControllerAddress(unsigned char addr)
{
    volatile register unsigned char a __asm("%al");
    a = addr;
    __asm volatile (
        "movw $0x03C0, %%dx\n\t"
        "outb %%al, %%dx"
    : : "a" (a) : "%dx");
}
//Supporting defines
#define VGA_ATTRIBUTE_ADDRESS_NOLOAD       0x20
#define VGA_ATTRIBUTE_ADDRESS_PAL(n)       ((n) & 0xF)
#define VGA_ATTRIBUTE_ADDRESS_MODECTRL     0x10
#define VGA_ATTRIBUTE_ADDRESS_OVERSCANCLR  0x11
#define VGA_ATTRIBUTE_ADDRESS_PLANEENABLE  0x12
#define VGA_ATTRIBUTE_ADDRESS_PIXELPAN     0x13
#define VGA_ATTRIBUTE_ADDRESS_COLOURSELECT 0x14

//OUTPORT 03C1 - Attribute Controller Data Write
inline void VGAAttributeControllerDataWrite(unsigned char data)
{
    volatile register unsigned char d __asm("%al");
    d = data;
    __asm volatile (
        "movw $0x03C1, %%dx\n\t"
        "outb %%al, %%dx"
    : : "a" (d) : "%dx");
}
//INPORT 03C1 - Attribute Controller Data Read
inline unsigned char VGAAttributeControllerDataRead()
{
    volatile register unsigned char data __asm("%al");
    __asm volatile (
        "movw $0x03C1, %%dx\n\t"
        "inb %%dx, %%al"
    : "=a" (data) : : "%dx");
    return data;
}
//Supporting defines
//VGA_ATTRIBUTE_ADDRESS_MODECTRL
#define VGA_ATTRIBUTE_MODECTRL_GRAPHICSENABLE 0x01
#define VGA_ATTRIBUTE_MODECTRL_MONOCHROME     0x02
#define VGA_ATTRIBUTE_MODECTRL_LINEGRAPHICS   0x04
#define VGA_ATTRIBUTE_MODECTRL_BLINKENABLE    0x08
#define VGA_ATTRIBUTE_MODECTRL_SPLITPAN       0x20
#define VGA_ATTRIBUTE_MODECTRL_256COLOUR      0x40
#define VGA_ATTRIBUTE_MODECTRL_PALBITS54CSEL  0x80

//OUTPORT 03C4 - Sequencer Address
inline void VGASequencerAddress(unsigned char addr)
{
    volatile register unsigned char a __asm("%al");
    a = addr;
    __asm volatile (
        "movw $0x03C4, %%dx\n\t"
        "outb %%al, %%dx"
    : : "a" (a) : "%dx");
}
//Supporting defines
#define VGA_SEQUENCER_ADDRESS_RESET     0x00
#define VGA_SEQUENCER_ADDRESS_CLOCKMODE 0x01
#define VGA_SEQUENCER_ADDRESS_MAPMASK   0x02
#define VGA_SEQUENCER_ADDRESS_CHARMAP   0x03
#define VGA_SEQUENCER_ADDRESS_MEMMODE   0x04

//OUTPORT 03C5 - Sequencer Data Write
inline void VGASequencerDataWrite(unsigned char data)
{
    volatile register unsigned char d __asm("%al");
    d = data;
    __asm volatile (
        "movw $0x03C5, %%dx\n\t"
        "outb %%al, %%dx"
    : : "a" (d) : "%dx");
}
//INPORT 03C5 - Sequencer Data Read
inline unsigned char VGASequencerDataRead()
{
    volatile register unsigned char data __asm("%al");
    __asm volatile (
        "movw $0x03C5, %%dx\n\t"
        "inb %%dx, %%al"
    : "=a" (data) : : "%dx");
    return data;
}
//Supporting defines
//VGA_SEQUENCER_ADDRESS_RESET
#define VGA_SEQUENCER_RESET_ASYNC   0x00
#define VGA_SEQUENCER_NORESET_ASYNC 0x01
#define VGA_SEQUENCER_RESET_SYNC    0x00
#define VGA_SEQUENCER_NORESET_SYNC  0x02
//VGA_SEQUENCER_ADDRESS_CLOCKMODE
#define VGA_SEQUENCER_CLOCKMODE_9PIXPC         0x00
#define VGA_SEQUENCER_CLOCKMODE_8PIXPC         0x01
#define VGA_SEQUENCER_CLOCKMODE_LOADEVERYCHAR  0x00
#define VGA_SEQUENCER_CLOCKMODE_LOADEVERY2CHAR 0x04
#define VGA_SEQUENCER_CLOCKMODE_LOADEVERY4CHAR 0x10
#define VGA_SEQUENCER_CLOCKMODE_PIXCLOCK1X     0x00
#define VGA_SEQUENCER_CLOCKMODE_PIXCLOCK0P5X   0x08
#define VGA_SEQUENCER_CLOCKMODE_SCREENDISABLE  0x20
//VGA_SEQUENCER_ADDRESS_CHARMAP
#define VGA_SEQUENCER_CHARMAP_A(n) ((((n) & 0x3) | (((n) & 0x4) << 1)) << 2)
#define VGA_SEQUENCER_CHARMAP_B(n) (((n) & 0x3) | (((n) & 0x4) << 2))
#define VGA_SEQUENCER_CHARMAP_BASE0 0x0000
#define VGA_SEQUENCER_CHARMAP_BASE1 0x4000
#define VGA_SEQUENCER_CHARMAP_BASE2 0x8000
#define VGA_SEQUENCER_CHARMAP_BASE3 0xC000
#define VGA_SEQUENCER_CHARMAP_BASE4 0x2000
#define VGA_SEQUENCER_CHARMAP_BASE5 0x6000
#define VGA_SEQUENCER_CHARMAP_BASE6 0xA000
#define VGA_SEQUENCER_CHARMAP_BASE7 0xE000
//VGA_SEQUENCER_ADDRESS_MEMMODE
#define VGA_SEQUENCER_MEMMODE_EXTENDED       0x02
#define VGA_SEQUENCER_MEMMODE_ODDEVENDISABLE 0x04
#define VGA_SEQUENCER_MEMMODE_CHAIN4         0x08

//OUTPORT 03C8 - DAC Write Address
inline void VGADACWriteAddress(unsigned char addr)
{
    volatile register unsigned char a __asm("%al");
    a = addr;
    __asm volatile (
        "movw $0x03C8, %%dx\n\t"
        "outb %%al, %%dx"
    : : "a" (a) : "%dx");
}

//OUTPORT 03C7 - DAC Read Address
inline void VGADACReadAddress(unsigned char addr)
{
    volatile register unsigned char a __asm("%al");
    a = addr;
    __asm volatile (
        "movw $0x03C7, %%dx\n\t"
        "outb %%al, %%dx"
    : : "a" (a) : "%dx");
}

//OUTPORT 03C9 - DAC Data Write
inline void VGADACDataWrite(unsigned char data)
{
    volatile register unsigned char d __asm("%al");
    d = data;
    __asm volatile (
        "movw $0x03C9, %%dx\n\t"
        "outb %%al, %%dx"
    : : "a" (d) : "%dx");
}
//INPORT 03C9 - DAC Data Read
inline unsigned char VGADACDataRead()
{
    volatile register unsigned char data __asm("%al");
    __asm volatile (
        "movw $0x03C9, %%dx\n\t"
        "inb %%dx, %%al"
    : "=a" (data) : : "%dx");
    return data;
}

//INPORT 03C7 - DAC State Read
inline unsigned char VGADACStateRead()
{
    volatile register unsigned char data __asm("%al");
    __asm volatile (
        "movw $0x03C7, %%dx\n\t"
        "inb %%dx, %%al"
    : "=a" (data) : : "%dx");
    return data;
}

//Writes one colour to the DAC, write the index first before calling this. Calling multiple times one after another sets sequential colours in the palette
inline void VGADACColourWrite(unsigned char r, unsigned char g, unsigned char b)
{
    volatile register unsigned char d __asm("%al");
    d = r;
    __asm volatile (
        "movw $0x03C9, %%dx\n\t"
        "outb %%al, %%dx"
    : : "a" (d) : "%dx");
    d = g;
    __asm volatile (
        "outb %%al, %%dx"
    : : "a" (d));
    d = b;
    __asm volatile (
        "outb %%al, %%dx"
    : : "a" (d));
}

//OUTPORT 03CE - Graphics Controller Address
inline void VGAGraphicsControllerAddress(unsigned char addr)
{
    volatile register unsigned char a __asm("%al");
    a = addr;
    __asm volatile (
        "movw $0x03CE, %%dx\n\t"
        "outb %%al, %%dx"
    : : "a" (a) : "%dx");
}
//Supporting defines
#define VGA_GRAPHICS_ADDRESS_SETRESET       0x00
#define VGA_GRAPHICS_ADDRESS_SETRESETENABLE 0x01
#define VGA_GRAPHICS_ADDRESS_COLOURCOMPARE  0x02
#define VGA_GRAPHICS_ADDRESS_ROTATE         0x03
#define VGA_GRAPHICS_ADDRESS_READPLANE      0x04
#define VGA_GRAPHICS_ADDRESS_MODE           0x05
#define VGA_GRAPHICS_ADDRESS_MISC           0x06
#define VGA_GRAPHICS_ADDRESS_COLOURIDC      0x07
#define VGA_GRAPHICS_ADDRESS_MASK           0x08

//OUTPORT 03CF - Graphics Controller Data Write
inline void VGAGraphicsControllerDataWrite(unsigned char data)
{
    volatile register unsigned char d __asm("%al");
    d = data;
    __asm volatile (
        "movw $0x03CF, %%dx\n\t"
        "outb %%al, %%dx"
    : : "a" (d) : "%dx");
}
//INPORT 03CF - Graphics Controller Data Read
inline unsigned char VGAGraphicsControllerDataRead()
{
    volatile register unsigned char data __asm("%al");
    __asm volatile (
        "movw $0x03CF, %%dx\n\t"
        "inb %%dx, %%al"
    : "=a" (data) : : "%dx");
    return data;
}
//Supporting defines
//VGA_GRAPHICS_ADDRESS_ROTATE
#define VGA_GRAPHICS_ROTATE_OP_NOP 0x00
#define VGA_GRAPHICS_ROTATE_OP_AND 0x08
#define VGA_GRAPHICS_ROTATE_OP_OR  0x10
#define VGA_GRAPHICS_ROTATE_OP_XOR 0x18
#define VGA_GRAPHICS_ROTATE_COUNT(n) ((n) & 0x7)
//VGA_GRAPHICS_ADDRESS_MODE
#define VGA_GRAPHICS_MODE_WRITE0          0x00
#define VGA_GRAPHICS_MODE_WRITE1          0x01
#define VGA_GRAPHICS_MODE_WRITE2          0x02
#define VGA_GRAPHICS_MODE_WRITE3          0x03
#define VGA_GRAPHICS_MODE_READ0           0x00
#define VGA_GRAPHICS_MODE_READ1           0x08
#define VGA_GRAPHICS_MODE_ODDEVENDISABLE  0x10
#define VGA_GRAPHICS_MODE_SHIFTINTERLEAVE 0x20
#define VGA_GRAPHICS_MODE_SHIFT256        0x40
//VGA_GRAPHICS_ADDRESS_MISC
#define VGA_GRAPHICS_MISC_GRAPHICSENABLE 0x01
#define VGA_GRAPHICS_MISC_CHAINODDEVEN   0x02
#define VGA_GRAPHICS_MISC_MEMMAP0        0x00
#define VGA_GRAPHICS_MISC_MEMMAP1        0x04
#define VGA_GRAPHICS_MISC_MEMMAP2        0x08
#define VGA_GRAPHICS_MISC_MEMMAP3        0x0C

//OUTPORT 03D4 - CRTC Address
inline void VGACRTCAddress(unsigned char addr)
{
    volatile register unsigned char a __asm("%al");
    a = addr;
    __asm volatile (
        "movw $0x03D4, %%dx\n\t"
        "outb %%al, %%dx"
    : : "a" (a) : "%dx");
}
//Supporting defines
#define VGA_CRTC_ADDRESS_HTOTAL       0x00
#define VGA_CRTC_ADDRESS_HDISPLAYEND  0x01
#define VGA_CRTC_ADDRESS_HBLANKSTART  0x02
#define VGA_CRTC_ADDRESS_HBLANKEND    0x03
#define VGA_CRTC_ADDRESS_HSYNCSTART   0x04
#define VGA_CRTC_ADDRESS_HSYNCEND     0x05
#define VGA_CRTC_ADDRESS_VTOTAL       0x06
#define VGA_CRTC_ADDRESS_OVERFLOW     0x07
#define VGA_CRTC_ADDRESS_PRESETRSCAN  0x08
#define VGA_CRTC_ADDRESS_MAXLINE      0x09
#define VGA_CRTC_ADDRESS_CSTART       0x0A
#define VGA_CRTC_ADDRESS_CEND         0x0B
#define VGA_CRTC_ADDRESS_SADDRHI      0x0C
#define VGA_CRTC_ADDRESS_SADDRLO      0x0D
#define VGA_CRTC_ADDRESS_CADDRHI      0x0E
#define VGA_CRTC_ADDRESS_CADDRLO      0x0F
#define VGA_CRTC_ADDRESS_VSYNCSTART   0x10
#define VGA_CRTC_ADDRESS_VSYNCEND     0x11
#define VGA_CRTC_ADDRESS_VDISPLAYEND  0x12
#define VGA_CRTC_ADDRESS_LINEPITCH    0x13
#define VGA_CRTC_ADDRESS_UNDERLINELOC 0x14
#define VGA_CRTC_ADDRESS_VBLANKSTART  0x15
#define VGA_CRTC_ADDRESS_VBLANKEND    0x16
#define VGA_CRTC_ADDRESS_MODE         0x17
#define VGA_CRTC_ADDRESS_COMPLINE     0x18

//OUTPORT 03D5 - CRTC Data Write
inline void VGACRTCDataWrite(unsigned char data)
{
    volatile register unsigned char d __asm("%al");
    d = data;
    __asm volatile (
        "movw $0x03D5, %%dx\n\t"
        "outb %%al, %%dx"
    : : "a" (d) : "%dx");
}
//INPORT 03D5 - CRTC Data Read
inline unsigned char VGACRTCDataRead()
{
    volatile register unsigned char data __asm("%al");
    __asm volatile (
        "movw $0x03D5, %%dx\n\t"
        "inb %%dx, %%al"
    : "=a" (data) : : "%dx");
    return data;
}
//Supporting defines
