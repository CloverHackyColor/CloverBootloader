;-----------------------------------------------------------------------------
; ullshr.asm - long shift right
; Adapted from Visual Studio C runtime library
; Portions Copyright (c) Microsoft Corporation. All rights reserved. 
;-----------------------------------------------------------------------------
                .386
_TEXT           segment use32 para public 'CODE'
                public  __aullshr

;
; ullshr - long shift right
;
; Purpose:
;       Does a unsigned Long Shift Right
;       Shifts a long right any number of bits.
;
; Entry:
;       EDX:EAX - long value to be shifted
;       CL      - number of bits to shift by
;
; Exit:
;       EDX:EAX - shifted value
;
; Uses:
;       CL is destroyed.
;

__aullshr       proc    near
                assume  cs:_TEXT

;
; Handle shifts of 64 bits or more (if shifting 64 bits or more, the result
; depends only on the high order bit of edx).
;
        cmp     cl,64
        jae     short RETZERO

;
; Handle shifts of between 0 and 31 bits
;
        cmp     cl, 32
        jae     short MORE32
        shrd    eax,edx,cl
        shr     edx,cl
        ret

;
; Handle shifts of between 32 and 63 bits
;
MORE32:
        mov     eax,edx
        xor     edx,edx
        and     cl,31
        shr     eax,cl
        ret

;
; return 0 in edx:eax
;
RETZERO:
        xor     eax,eax
        xor     edx,edx
        ret

__aullshr       endp

_TEXT           ends
                end
