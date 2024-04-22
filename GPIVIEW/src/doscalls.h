//MS-DOS system call interface
//Maxim Hoxha 2023-2024

#pragma once

#include "x86segments.h"

//Function error codes
#define DOSERROR_BADFUNCTION 0x0001
#define DOSERROR_FNOTFOUND 0x0002
#define DOSERROR_PNOTFOUND 0x0003
#define DOSERROR_TOOMANYFILES 0x0004
#define DOSERROR_ACCESSDENIED 0x0005
#define DOSERROR_BADHANDLE 0x0006
#define DOSERROR_NOMEMORYCONTROL 0x0007
#define DOSERROR_OUTOFMEMORY 0x0008
#define DOSERROR_BADMEMORYBLOCK 0x0009
#define DOSERROR_BADENV 0x000A
#define DOSERROR_BADFORMAT 0x000B
#define DOSERROR_BADACCESS 0x000C
#define DOSERROR_DEVICEDIFFERENT 0x0011
#define DOSERROR_NOMOREFILES 0x0012

typedef unsigned short doshandle;

//INT 21 function 00 - Terminate Program (do not actually use this, as it was superseded by function 4C)
inline void DOSLegacyTerminate()
{
    __asm volatile (
        "movb $0x00, %%ah\n\t"
        "int $0x21"
    : : : "%ah");
}

//INT 21 function 01 - Input Single Character From Console
inline char DOSConsoleReadChar()
{
    register volatile char ch __asm("%al");
    __asm volatile (
        "movb $0x01, %%ah\n\t"
        "int $0x21"
    : "=a" (ch) : );
    return ch;
}

//INT 21 function 02 - Output Single Character (ch) To Console
inline void DOSConsoleWriteChar(char ch)
{
    register volatile char c __asm("%dl");
    c = ch;
    __asm volatile (
        "movb $0x02, %%ah\n\t"
        "int $0x21"
    : : "d" (c) : "%ah");
}

//INT 21 function 03 - Input Single Character From Aux Port
inline char DOSAuxReadChar()
{
    register volatile char ch __asm("%al");
    __asm volatile (
        "movb $0x03, %%ah\n\t"
        "int $0x21"
    : "=a" (ch) : );
    return ch;
}

//INT 21 function 04 - Output Single Character (ch) To Aux Port
inline void DOSAuxWriteChar(char ch)
{
    register volatile char c __asm("%dl");
    c = ch;
    __asm volatile (
        "movb $0x04, %%ah\n\t"
        "int $0x21"
    : : "d" (c) : "%ah");
}

//INT 21 function 05 - Output Single Character (ch) To Printer Port
inline void DOSPrinterWriteChar(char ch)
{
    register volatile char c __asm("%dl");
    c = ch;
    __asm volatile (
        "movb $0x05, %%ah\n\t"
        "int $0x21"
    : : "d" (c) : "%ah");
}

//INT 21 function 09 - Output String (pointed to by strptr) To Console (must be terminated by '$')
inline void DOSConsoleWriteString(char* strptr)
{
    SetDS(GetSS());
    register volatile char* p __asm("%dx");
    p = strptr;
    __asm volatile (
        "movb $0x09, %%ah\n\t"
        "int $0x21"
    : : "d" (p) : "%ah");
}

//INT 21 function 3C - Create File (pathname pointed to by 'path' (null-terminated), attributes in 'attributes', returns a handle in 'handle', can return an error code)
inline int DOSCreateFile(const char* path, unsigned short attributes, doshandle* handle)
{
    SetDS(GetSS());
    register volatile const char* p __asm("%dx");
    register volatile unsigned short a __asm("%cx");
    register volatile doshandle h __asm("%ax");
    p = path;
    a = attributes;
    unsigned char errored;
    __asm volatile (
        "movb $0x3C, %%ah\n\t"
        "int $0x21\n\t"
        "sbbb %b1, %b1"
    : "=a" (h), "=r" (errored) : "d" (p), "c" (a));
    if (errored)
    {
        return h;
    }
    else
    {
        *handle = h;
        return 0;
    }
}
//Supporting defines
#define DOSFILE_ATTRIBUTE_READONLY 0x0001
#define DOSFILE_ATTRIBUTE_HIDDEN 0x0002
#define DOSFILE_ATTRIBUTE_SYSTEM 0x0004
#define DOSFILE_ATTRIBUTE_VOLUMELABEL 0x0008
#define DOSFILE_ATTRIBUTE_SUBDIR 0x0010
#define DOSFILE_ATTRIBUTE_ARCHIVE 0x0020

//INT 21 function 3D - Open File (pathname pointed to by 'path' (null-terminated), attribute in 'mode', returns a handle in 'handle', can return an error code)
inline int DOSOpenFile(const char* path, unsigned char mode, doshandle* handle)
{
    SetDS(GetSS());
    register volatile const char* p __asm("%dx");
    register volatile unsigned short m __asm("%al");
    register volatile doshandle h __asm("%ax");
    p = path;
    m = mode;
    unsigned char errored;
    __asm volatile (
        "movb $0x3D, %%ah\n\t"
        "int $0x21\n\t"
        "sbbb %b1, %b1"
    : "=a" (h), "=r" (errored) : "d" (p), "a" (m));
    if (errored)
    {
        return h;
    }
    else
    {
        *handle = h;
        return 0;
    }
}
//Supporting defines
#define DOSFILE_OPEN_READ 0x00
#define DOSFILE_OPEN_WRITE 0x01
#define DOSFILE_OPEN_READWRITE 0x02

//INT 21 function 3E - Close File (given by handle in 'handle', can return an error code)
inline int DOSCloseFile(doshandle handle)
{
    register volatile doshandle h __asm("%bx");
    h = handle;
    register volatile unsigned short errorcode __asm("%ax");
    __asm volatile (
        "movb $0x3E, %%ah\n\t"
        "int $0x21\n\t"
        "jc .end%=\n\t"
        "xorw %%ax, %%ax\n\t"
        ".end%=: "
    : "=a" (errorcode) : "b" (h));
    return errorcode;
}

//INT 21 function 3F - Read From a File (given by handle in 'handle', puts 'len' bytes into array pointed to by 'buffer', returns the number of bytes actually read in 'readbytes', can return an error code)
inline int DOSReadFile(doshandle handle, unsigned short len, __far void* buffer, unsigned short* readbytes)
{
    unsigned short ps = ((unsigned long)buffer) >> 16;
    unsigned char errored;
    register volatile doshandle h __asm("%bx");
    register volatile unsigned short l __asm("%cx");
    register volatile unsigned char* po __asm("%dx");
    h = handle;
    l = len;
    po = (unsigned char*)((unsigned short)buffer);
    register volatile unsigned short r __asm("%ax");
    __asm volatile (
        "mov %5, %%ds\n\t"
        "movb $0x3F, %%ah\n\t"
        "int $0x21\n\t"
        "sbbb %b1, %b1"
    : "=a" (r), "=r" (errored) : "b" (h), "c" (l), "d" (po), "rm" (ps) : "%ds");
    if (errored)
    {
        return r;
    }
    else
    {
        *readbytes = r;
        return 0;
    }
}

//INT 21 function 40 - Write To a File (given by handle in 'handle', puts 'len' bytes into the file from the array pointed to by 'buffer', returns the number of bytes actually written in 'writebytes', can return an error code)
inline int DOSWriteFile(doshandle handle, unsigned short len, __far const void* buffer, unsigned short* writebytes)
{
    unsigned short ps = ((unsigned long)buffer) >> 16;
    unsigned char errored;
    register volatile doshandle h __asm("%bx");
    register volatile unsigned short l __asm("%cx");
    register volatile const unsigned char* po __asm("%dx");
    h = handle;
    l = len;
    po = (unsigned char*)((unsigned short)buffer);
    register volatile unsigned short w __asm("%ax");
    __asm volatile (
        "mov %5, %%ds\n\t"
        "movb $0x40, %%ah\n\t"
        "int $0x21\n\t"
        "sbbb %b1, %b1"
    : "=a" (w), "=r" (errored) : "b" (h), "c" (l), "d" (po), "rm" (ps) : "%ds");
    if (errored)
    {
        return w;
    }
    else
    {
        *writebytes = w;
        return 0;
    }
}

//INT 21 function 41 - Delete File (pathname pointed to by 'path' (null-terminated), can return an error code)
inline int DOSDeleteFile(const char* path)
{
    SetDS(GetSS());
    register volatile const char* p __asm("%dx");
    p = path;
    register volatile unsigned short errorcode __asm("%ax");
    __asm volatile (
        "movb $0x41, %%ah\n\t"
        "int $0x21\n\t"
        "jc .end%=\n\t"
        "xorw %%ax, %%ax\n\t"
        ".end%=: "
    : "=a" (errorcode) : "d" (p));
    return errorcode;
}

//INT 21 function 42 - Seek In File (given by handle in 'handle', moves the file pointer by 'len' bytes, according to the method in 'method', returns the new file position in 'newpos', can return an error code)
inline int DOSSeekFile(doshandle handle, unsigned char method, unsigned long len, unsigned long* newpos)
{
    register volatile doshandle h __asm("%bx");
    register volatile unsigned char m __asm("%al");
    register volatile unsigned short lu __asm("%cx");
    register volatile unsigned short ll __asm("%dx");
    h = handle;
    m = method;
    ll = (unsigned short)len;
    lu = (unsigned short)(len >> 16);
    register volatile unsigned short pl __asm("%ax");
    register volatile unsigned short pu __asm("%dx");
    unsigned char errored;
    __asm volatile (
        "movb $0x42, %%ah\n\t"
        "int $0x21\n\t"
        "sbbb %b2, %b2"
    : "=a" (pl), "=d" (pu), "=r" (errored) : "b" (h), "a" (m), "c" (lu), "d" (ll));
    if (errored)
    {
        return pl;
    }
    else
    {
        unsigned long np = ((unsigned long)pl) | (((unsigned long)pu) << 16);
        *newpos = np;
        return 0;
    }
}
//Supporting defines
//From the beginning of the file
#define DOSFILE_SEEK_ABSOLUTE 0x00
//From the current file pointer position
#define DOSFILE_SEEK_RELATIVE 0x01
//From the end of the file
#define DOSFILE_SEEK_REVERSE 0x02

//INT 21 function 48 - Allocate Memory ('segsize' segments, can return an error code)
inline __far void* DOSMemAlloc(unsigned short segments)
{
    register volatile unsigned short size __asm("%bx");
    size = segments;
    register volatile unsigned short as __asm("%ax");
    unsigned char errored;
    __asm volatile (
        "movb $0x48, %%ah\n\t"
        "int $0x21\n\t"
        "sbbb %b1, %b1"
    : "=a" (as), "=r" (errored) : "b" (size));
    if (errored)
    {
        return (__far void*)as; //If this returns an address with nonzero offset and zero segment, it must have errored because there is no way it can be a legitimate pointer to allocated memory
    }
    else
    {
        return (__far void*)(((unsigned long)as) << 16);
    }
}

//INT 21 function 49 - Free Memory (pointed to by 'ptr', can return an error code)
inline int DOSMemFree(const __far void* ptr)
{
    unsigned short p = ((unsigned long)ptr) >> 16;
    register volatile unsigned short errorcode __asm("%ax");
    __asm volatile (
        "mov %1, %%es\n\t"
        "movb $0x49, %%ah\n\t"
        "int $0x21\n\t"
        "jc .end%=\n\t"
        "xorw %%ax, %%ax\n\t"
        ".end%=: "
    : "=a" (errorcode) : "rm" (p) : "%es");
    return errorcode;
}

//INT 21 function 4A - Reallocate Memory ('segments' segments as new size, pointed to by 'ptr', can return an error code)
inline int DOSMemRealloc(const __far void* ptr, unsigned short segments)
{
    register volatile unsigned short size __asm("%bx");
    size = segments;
    unsigned short p = ((unsigned long)ptr) >> 16;
    register volatile unsigned short errorcode __asm("%ax");
    __asm volatile (
        "mov %1, %%es\n\t"
        "movb $0x4A, %%ah\n\t"
        "int $0x21\n\t"
        "jc .end%=\n\t"
        "xorw %%ax, %%ax\n\t"
        ".end%=: "
    : "=a" (errorcode) : "rm" (p), "b" (size) : "%es");
    return errorcode;
}
