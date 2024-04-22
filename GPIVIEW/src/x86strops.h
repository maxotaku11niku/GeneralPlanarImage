//x86 string instruction intrinsics
//Maxim Hoxha 2023-2024

#pragma once

#include "x86segments.h"

//Fast copy memory 8 bits at a time in the same segment
inline void Memcpy8Near(const void* src, void* dst, unsigned int count)
{
    unsigned short s = GetSS();
    SetDS(s); SetES(s);
    __asm volatile (
        "rep movsb"
    : "+c" (count), "+S" (src), "+D" (dst) : );
}

//Fast copy memory 16 bits at a time in the same segment
inline void Memcpy16Near(const void* src, void* dst, unsigned int count)
{
    unsigned short s = GetSS();
    SetDS(s); SetES(s);
    __asm volatile (
        "rep movsw"
    : "+c" (count), "+S" (src), "+D" (dst) : );
}

//Fast copy memory 32 bits at a time in the same segment
inline void Memcpy32Near(const void* src, void* dst, unsigned int count)
{
    unsigned short s = GetSS();
    SetDS(s); SetES(s);
    __asm volatile (
        "rep movsl"
    : "+c" (count), "+S" (src), "+D" (dst) : );
}

//Fast copy memory with count always in bytes in the same segment
inline void MemcpyNear(const void* src, void* dst, unsigned int count)
{
    unsigned short s = GetSS();
    SetDS(s); SetES(s);
    __asm volatile (
        "testw $0x0001, %2\n\t"
        "jz .aligned%=\n\t"
        "movsb\n\t"
        ".aligned%=: shrw $1, %2\n\t"
        "rep movsw"
    : "+S" (src), "+D" (dst), "+c" (count) : );
}

//Fast set memory 8 bits at a time in the same segment
inline void Memset8Near(unsigned char num, void* dst, unsigned int count)
{
    SetES(GetSS());
    __asm volatile (
        "rep stosb"
    : "+c" (count), "+D" (dst) : "a" (num));
}

//Fast set memory 16 bits at a time in the same segment
inline void Memset16Near(unsigned short num, void* dst, unsigned int count)
{
    SetES(GetSS());
    __asm volatile (
        "rep stosw"
    : "+c" (count), "+D" (dst) : "a" (num));
}

//Fast set memory 32 bits at a time in the same segment
inline void Memset32Near(unsigned long num, void* dst, unsigned int count)
{
    SetES(GetSS());
    __asm volatile (
        "rep stosl"
    : "+c" (count), "+D" (dst) : "a" (num));
}

//Fast set memory with count always in bytes in the same segment, can only fill with the same byte
inline void MemsetNear(unsigned char num, void* dst, unsigned int count)
{
    SetES(GetSS());
    __asm volatile (
        "testw $0x0001, %2\n\t"
        "jz .aligned%=\n\t"
        "stosb\n\t"
        ".aligned%=: shrw $1, %2\n\t"
        "rep stosw"
    : "+D" (dst), "+c" (count) : "a" (num) );
}

//Fast copy memory 8 bits at a time between different segments
inline void Memcpy8Far(__far const void* src, __far void* dst, unsigned int count)
{
    unsigned short srco = (unsigned short)((unsigned long)src);
    unsigned short dsto = (unsigned short)((unsigned long)dst);
    unsigned short srcs = ((unsigned long)src) >> 16;
    unsigned short dsts = ((unsigned long)dst) >> 16;
    __asm volatile (
        "mov %3, %%ds\n\t"
        "mov %4, %%es\n\t"
        "rep movsb"
    : "+c" (count), "+S" (srco), "+D" (dsto) : "rm" (srcs), "rm" (dsts) : "%ds", "%es");
}

//Fast copy memory 16 bits at a time between different segments
inline void Memcpy16Far(__far const void* src, __far void* dst, unsigned int count)
{
    unsigned short srco = (unsigned short)((unsigned long)src);
    unsigned short dsto = (unsigned short)((unsigned long)dst);
    unsigned short srcs = ((unsigned long)src) >> 16;
    unsigned short dsts = ((unsigned long)dst) >> 16;
    __asm volatile (
        "mov %3, %%ds\n\t"
        "mov %4, %%es\n\t"
        "rep movsw"
    : "+c" (count), "+S" (srco), "+D" (dsto) : "rm" (srcs), "rm" (dsts) : "%ds", "%es");
}

//Fast copy memory 32 bits at a time between different segments
inline void Memcpy32Far(__far const void* src, __far void* dst, unsigned int count)
{
    unsigned short srco = (unsigned short)((unsigned long)src);
    unsigned short dsto = (unsigned short)((unsigned long)dst);
    unsigned short srcs = ((unsigned long)src) >> 16;
    unsigned short dsts = ((unsigned long)dst) >> 16;
    __asm volatile (
        "mov %3, %%ds\n\t"
        "mov %4, %%es\n\t"
        "rep movsl"
    : "+c" (count), "+S" (srco), "+D" (dsto) : "rm" (srcs), "rm" (dsts) : "%ds", "%es");
}

//Fast copy memory with count always in bytes between different segments
inline void MemcpyFar(__far const void* src, __far void* dst, unsigned int count)
{
    unsigned short srco = (unsigned short)((unsigned long)src);
    unsigned short dsto = (unsigned short)((unsigned long)dst);
    unsigned short srcs = ((unsigned long)src) >> 16;
    unsigned short dsts = ((unsigned long)dst) >> 16;
    __asm volatile (
        "mov %3, %%ds\n\t"
        "mov %4, %%es\n\t"
        "testw $0x0001, %2\n\t"
        "jz .aligned%=\n\t"
        "movsb\n\t"
        ".aligned%=: shrw $1, %2\n\t"
        "rep movsw"
    : "+S" (srco), "+D" (dsto), "+c" (count) : "rm" (srcs), "rm" (dsts) : "%ds", "%es");
}

//Fast set memory 8 bits at a time to a different segment
inline void Memset8Far(unsigned char num, __far void* dst, unsigned int count)
{
    unsigned short dsto = (unsigned short)((unsigned long)dst);
    unsigned short dsts = ((unsigned long)dst) >> 16;
    __asm volatile (
        "mov %3, %%es\n\t"
        "rep stosb"
    : "+c" (count), "+D" (dsto) : "a" (num), "rm" (dsts) : "%es");
}

//Fast set memory 16 bits at a time to a different segment
inline void Memset16Far(unsigned short num, __far void* dst, unsigned int count)
{
    unsigned short dsto = (unsigned short)((unsigned long)dst);
    unsigned short dsts = ((unsigned long)dst) >> 16;
    __asm volatile (
        "mov %3, %%es\n\t"
        "rep stosw"
    : "+c" (count), "+D" (dsto) : "a" (num), "rm" (dsts) : "%es");
}

//Fast set memory 32 bits at a time to a different segment
inline void Memset32Far(unsigned long num, __far void* dst, unsigned int count)
{
    unsigned short dsto = (unsigned short)((unsigned long)dst);
    unsigned short dsts = ((unsigned long)dst) >> 16;
    __asm volatile (
        "mov %3, %%es\n\t"
        "rep stosl"
    : "+c" (count), "+D" (dsto) : "a" (num), "rm" (dsts) : "%es");
}

//Fast set memory with count always in bytes to a different segment, can only fill with the same byte
inline void MemsetFar(unsigned char num, __far void* dst, unsigned int count)
{
    unsigned short dsto = (unsigned short)((unsigned long)dst);
    unsigned short dsts = ((unsigned long)dst) >> 16;
    __asm volatile (
        "mov %3, %%es\n\t"
        "testw $0x0001, %1\n\t"
        "jz .aligned%=\n\t"
        "stosb\n\t"
        ".aligned%=: shrw $1, %1\n\t"
        "rep stosw"
    : "+D" (dst), "+c" (count) : "a" (num),  "rm" (dsts) : "%es");
}
