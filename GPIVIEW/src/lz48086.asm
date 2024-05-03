;# Decompresses Y. Collet's LZ4 compressed stream data in 16-bit real mode.
;# Optimized for 8088/8086 CPUs.
;# Code by Trixter/Hornet (trixter@oldskool.org) on 20130105
;# Updated 20190617 -- thanks to Peter Ferrie, Terje Mathsen,
;# and Axel Kern for suggestions and improvements!
;# Updated 20190630: Fixed an alignment bug in lz4_decompress_small
;# Updated 20200314: Speed updates from Pavel Zagrebin
;# Adapted to GNU assembler by Maxim Hoxha

;#        IDEAL
;#        JUMPS ;needed because an early condition jump is > 128 bytes
;#        MODEL TPASCAL

.arch i8086,jumps ;#needed because an early condition jump is > 128 bytes
.intel_syntax noprefix
.code16
.text

;#PUBLIC  lz4_decompress, lz4_decompress_small

;# Must declare this in the code segment.
SHR4table:
        .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte 0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01
        .byte 0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02
        .byte 0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03
        .byte 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04
        .byte 0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05
        .byte 0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06
        .byte 0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07
        .byte 0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08
        .byte 0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09
        .byte 0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A
        .byte 0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B
        .byte 0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C
        .byte 0x0D,0x0D,0x0D,0x0D,0x0D,0x0D,0x0D,0x0D,0x0D,0x0D,0x0D,0x0D,0x0D,0x0D,0x0D,0x0D
        .byte 0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E
        .byte 0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F

;#---------------------------------------------------------------
;# function lz4_decompress(inb,outb:pointer):word;
;#
;# Decompresses an LZ4 stream file with a compressed chunk 64K or less in size.
;# Input:
;#   DS:SI Location of source data.  DWORD magic header and DWORD chunk size
;#         must be intact; it is best to load the entire LZ4 file into this
;#         location before calling this code.
;#
;# Output:
;#   ES:DI Decompressed data.  If using an entire 64K segment, decompression
;#         is "safe" because overruns will wrap around the segment.
;#   AX    Size of decompressed data.
;#
;# Trashes AX, BX, CX, DX, SI, DI
;#         ...so preserve what you need before calling this code.
;#---------------------------------------------------------------
;#Speed optimization history (decompression times in microseconds @ 4.77 MHz):
;#before segment fixups: shuttle 108976 text 48742 broken code, invalid output
;# after segment fixups: shuttle 112494 text 50940                 -
;# after match copy opt: shuttle 110971 text 49890                 +
;# after misc opt:       shuttle 109707 text 49056                 +
;# after rep stosb opt:  shuttle 104877 text 51435                 +
;# after rep stosw opt:  shuttle 104918 text 51412 robotron 365292 -+*
;# after match+RLE opt:  shuttle  94274 text 49641 robotron 345426 +++
;# after token unpack:   shuttle  93418 text 49140 robotron 342696 +++
;# after accum opt:      shuttle  91992 text 48213 robotron 336635 +++
;# after dx regswap opt: shuttle  90461 text 47218 robotron 330449 +++
;# after repmovsb only:  shuttle  96231 text 46472 robotron 333068 -+- aborted
;# after 1-byteRLE only: shuttle  96201 text 46472 robotron 333270 -+- aborted
;# after cmp cl, -> al,: shuttle  90091 text 46894 robotron 327713 +++
;# after likely(ll<15):  shuttle  89378 text 46487 robotron 323677 +++
;# after ll=0 removechk: shuttle  90880 text 47957 robotron 323375 --+ aborted
;# after likely(ml<15):  shuttle  89205 text 45388 robotron 317959 +++
;# after mov r,ax->xchg: shuttle  88462 text 44963 robotron 315099 +++
;# after es:movsw:       shuttle  90408 text 45295 robotron 321030 --- aborted
;# after mcopy shortcir: shuttle  89710 text 45597 robotron 319660 --- aborted
;# after rep es: movsb:  shuttle  88907 text 45076 robotron 316138 --- aborted
;# after main lp unroll: shuttle  86153 text 43502 robotron 307923 +++
;#Peter Ferrie is credited with the following suggestions/speedups:
;# remove unnecess. xor: shuttle  85781 text 43487 robotron 307660 +++
;# xor ax,ax->xchg ax,r: shuttle  85037 text 43035 robotron 304574 +++
;#Terje Mathisen is credited with the following suggestions/speedups:
;# RLE overshoot->adjus: shuttle  85022 text 43035 robotron 304571 +0+
;#---------------------------------------------------------------
;#Pavel Zagrebin is credited with the following speedups:
;# Changing the end-of-file comparison to self-modifying offset
;# push ds;pop ds->mov ds,bp
;# adc cx,cx;rep movsb->jnc
;# NOTE:  I can't explain it, but with no extraneous background interrupts,
;# timings are taking longer than normal on my IBM 5160.  So, we have to
;# reset our timing numbers here:
;# Old timings:          shuttle  85038 text 45720 robotron 307796 ---
;# After Pavel's speedups:
;# New timings:          shuttle  81982 text 43664 robotron 296081 +++

;# MH - Label names have been changed and the way some instructions are written has been changed, but this function is still pretty much unchanged
.p2align 2
.globl	lz4_decompress
;#PROC    lz4_decompress          NEAR
;#ARG     inb:DWORD, outb:DWORD
lz4_decompress:
        push    ds              ;#preserve compiler assumptions
;#        les     di,[outb]       ;#load target buffer, MH - es:di is being passed to the function
        push    di              ;#save original starting offset (in case != 0)
;#        lds     si,[inb]        ;#load source buffer, MH - ds:dl is being passed to the function
;#        add     si,4            ;#skip magic number, MH - No longer including this in output files
        cld                     ;#make strings copy forward
        mov     bx,offset SHR4table     ;#prepare BX for XLAT later on
        lodsw                   ;#load chunk size low 16-bit word
        mov     bp,ax           ;#BP = size of compressed chunk
        lodsw                   ;#load chunk size high 16-bit word
        add     bp,si           ;#BP = threshold to stop decompression
        or      ax,ax           ;#is high word non-zero?
        jnz     done_1          ;#If so, chunk too big or malformed, abort

starttoken:
        lodsb                   ;#grab token to AL
        mov     dx,ax           ;#preserve packed token in DX
        cs xlat                 ;#unpack upper 4 bits, faster than SHR reg,cl
        mov     cx,ax           ;#CX = unpacked literal length token
        jcxz    copymatches_1   ;#if CX = 0, no literals; try matches
        cmp     al,0x0F         ;#is it 15?
        jne     doliteralcopy1  ;#if so, build full length, else start copying
build1stcount:                  ;#this first count build is not the same
        lodsb                   ;#fall-through jump as the one in the main loop
        add     cx,ax           ;#because it is more likely that the very first
        cmp     al,0xFF         ;#length is 15 or more
        je      build1stcount
doliteralcopy1:
        rep     movsb           ;#src and dst might overlap so do this by bytes

;#At this point, we might be done; all LZ4 data ends with five literals and the
;#offset token is ignored.  If we're at the end of our compressed chunk, stop.

        cmp     si,bp           ;#are we at the end of our compressed chunk?
        movw    [cs:end_of_chunk_1+2],bp
                                ;#self-modifying cmp si,xxxx
        mov     bp,ds           ;#now we can use bp for restoring ds
        jae     done_1          ;#if so, jump to exit; otherwise, process match

copymatches_1:
        lodsw                   ;#AX = match offset
        xchg    dx,ax           ;#AX = packed token, DX = match offset
        and     al,0x0F         ;#unpack match length token
        cmp     al,0x0F         ;#is it 15?
        xchg    cx,ax           ;#(doesn't affect flags); don't need ax any more
        je      buildmcount     ;#if not, start copying, otherwise build count

domatchcopy_1:
        cmp     dx,2            ;#if match offset=1 or 2, we're repeating a value
        jbe     domatchfill     ;#if so, perform RLE expansion optimally
        xchg    si,ax           ;#ds:si saved
        mov     si,di
        sub     si,dx
        mov     dx,es
        mov     ds,dx           ;#ds:si points at match; es:di points at dest
        movsw                   ;#minimum match is 4 bytes; move them ourselves
        shr     cx,1
        jnc     even_1
        movsb
even_1:
        movsw
		rep     movsw           ;#cx contains count-4 so copy the rest
        xchg    si,ax
        mov     ds,bp

parsetoken_1:                   ;#CX always 0 here because of REP
        xchg    cx,ax           ;#zero ah here to benefit other reg loads
        lodsb                   ;#grab token to AL
        mov     dx,ax           ;#preserve packed token in DX
copyliterals_1:                 ;#next 5 lines are 8088-optimal, do not rearrange
        cs xlat                 ;#unpack upper 4 bits, faster than SHR reg,cl
        mov     cx,ax           ;#CX = unpacked literal length token
        jcxz    copymatches_1   ;#if CX = 0, no literals; try matches
        cmp     al,0x0F         ;#is it 15?
        je      buildlcount     ;#if so, build full length, else start copying
doliteralcopy_1:                ;#src and dst might overlap so do this by bytes
        rep     movsb           ;#if cx=0 nothing happens

;#At this point, we might be done; all LZ4 data ends with five literals and the
;#offset token is ignored.  If we're at the end of our compressed chunk, stop.

testformore:
end_of_chunk_1:
		cmp     si,256          ;#this constant is patched with the end address
        jb      copymatches_1   ;#if not, keep going
        jmp     done_1          ;#if so, end

domatchfill:
        je      domatchfill2    ;#if DX=2, RLE by word, else by byte
domatchfill1:
        mov     al,es:[di-1]    ;#load byte we are filling with
        mov     ah,al           ;#copy to ah so we can do 16-bit fills
        stosw                   ;#minimum match is 4 bytes, so we fill four
        stosw
        inc     cx              ;#round up for the shift
        shr     cx,1            ;#CX = remaining (count+1)/2
        rep     stosw           ;#includes odd byte - ok because LZ4 never ends with matches
        adc     di,-1           ;#Adjust dest unless original count was even
        jmp     parsetoken_1    ;#continue decompressing

domatchfill2:
        mov     ax,es:[di-2]    ;#load word we are filling with
        stosw                   ;#minimum match is 4 bytes, so we fill four
        stosw
        inc     cx              ;#round up for the shift
        shr     cx,1            ;#CX = remaining (count+1)/2
        rep     stosw           ;#includes odd byte - ok because LZ4 never ends with matches
        adc     di,-1           ;#Adjust dest unless original count was even
        jmp     parsetoken_1    ;#continue decompressing

buildlcount:                    ;#build full literal length count
        lodsb                   ;#get next literal count byte
        add     cx,ax           ;#increase count
        cmp     al,0xFF         ;#more count bytes to read?
        je      buildlcount
        jmp     doliteralcopy_1

buildmcount:                    ;#build full match length count - AX is 0
        lodsb                   ;#get next literal count byte
        add     cx,ax           ;#increase count
        cmp     al,0xFF         ;#more count bytes to read?
        je      buildmcount
        jmp     domatchcopy_1

done_1:
        pop     ax              ;#retrieve previous starting offset
        sub     di,ax           ;#subtract prev offset from where we are now
        xchg    ax,di           ;#AX = decompressed size
        pop     ds              ;#restore compiler assumptions
        ret

;#ENDP    lz4_decompress



;#---------------------------------------------------------------
;# function lz4_decompress_small(inb,outb:pointer):word; assembler;
;#
;# Same as LZ4_Decompress but optimized for size, not speed. Still pretty fast,
;# although roughly 30% slower than lz4_decompress and RLE sequences are not
;# optimally handled.  Same Input, Output, and Trashes as lz4_decompress.
;# Minus the Turbo Pascal preamble/postamble, assembles to 78 bytes.
;#---------------------------------------------------------------

;# MH - We won't assemble this one since we want speed and the function above isn't *that* big anyway
;#.p2align 2
;#.globl	lz4_decompress_small
;#PROC    lz4_decompress_small    NEAR
;#ARG     inb:DWORD, outb:DWORD
;#lz4_decompress_small
;#        push    ds              ;#preserve compiler assumptions
;#        les     di,[outb]       ;#load target buffer
;#        lds     si,[inb]        ;#load source buffer
;#        cld                     ;#make strings copy forward
;#        lodsw
;#        lodsw                   ;#skip magic number, smaller than "add si,4"
;#        lodsw                   ;#load chunk size low 16-bit word
;#        xchg    bx,ax           ;#BX = size of compressed chunk
;#        add     bx,si           ;#BX = threshold to stop decompression
;#        lodsw                   ;#load chunk size high 16-bit word
;#        or      ax,ax           ;#is high word non-zero?
;#        jnz     done_2          ;#If so, chunk too big or malformed, abort
;#parsetoken_2:                   ;#CX=0 here because of REP at end of loop
;#        lodsb                   ;#grab token to AL
;#        mov     dx,ax           ;#preserve packed token in DX
;#copyliterals_2:
;#        mov     cx,4            ;#set full CX reg to ensure CH is 0
;#        shr     al,cl           ;#unpack upper 4 bits
;#        call    buildfullcount  ;#build full literal count if necessary
;#doliteralcopy_2:                ;#src and dst might overlap so do this by bytes
;#        rep     movsb           ;#if cx=0 nothing happens

;#At this point, we might be done; all LZ4 data ends with five literals and the
;#offset token is ignored.  If we're at the end of our compressed chunk, stop.

;#        cmp     si,bx           ;#are we at the end of our compressed chunk?
;#        jae     done_2          ;#if so, jump to exit; otherwise, process match
;#copymatches_2:
;#        lodsw                   ;#AX = match offset
;#        xchg    dx,ax           ;#AX = packed token, DX = match offset
;#        and     al,0x0F         ;#unpack match length token
;#        call    buildfullcount  ;#build full match count if necessary
;#domatchcopy_2:
;#        push    ds
;#        push    si              ;#ds:si saved, xchg with ax would destroy ah
;#        mov     si,di
;#        sub     si,dx
;#        push    es
;#        pop     ds              ;#ds:si points at match; es:di points at dest
;#        add     cx,4            ;#minmatch = 4
                                  ;#Can't use MOVSWx2 because [es:di+1] is unknown
;#        rep     movsb           ;#copy match run if any left
;#        pop     si
;#        pop     ds              ;#ds:si restored
;#        jmp     parsetoken_2

;#buildfullcount:
                                  ;#CH has to be 0 here to ensure AH remains 0
;#        cmp     al,0x0F         ;#test if unpacked literal length token is 15?
;#        xchg    cx,ax           ;#CX = unpacked literal length token; flags unchanged
;#        jne     builddone       ;#if AL was not 15, we have nothing to build
;#buildloop:
;#        lodsb                   ;#load a byte
;#        add     cx,ax           ;#add it to the full count
;#        cmp     al,0xFF         ;#was it FF?
;#        je      buildloop       ;#if so, keep going
;#builddone:
;#        retn

;#done_2:
;#        subw    di,[outb]       ;#subtract original offset from where we are now
;#        xchg    ax,di           ;#AX = decompressed size
;#        pop     ds              ;#restore compiler assumptions
;#        ret

;#ENDP    lz4_decompress_small

;#ENDS    CODE

;#        END
