;
; Library/Uefi/IA32/ftol.asm
;
; UEFI implementation IA32 floating point to integer conversion intrinsics
;

.386

_TEXT segment use32 para public 'CODE'

  ; Export these symbols from this source
  public __ftol
  public __ftol2
  public __ftol2_sse

  ; __ftol
  ; Convert floating point to integer
  ; @param Float The floating point to convert to integer
  ; @return The integer converted from the floating point
  __ftol proc
    assume  cs:_TEXT
    fnstcw  word ptr [esp - 2]
    mov     ax, word ptr [esp - 2]
    or      ax, 0C00h
    mov     word ptr [esp - 4], ax
    fldcw   word ptr [esp - 4]
    fistp   qword ptr [esp - 12]
    fldcw   word ptr [esp - 2]
    mov     eax, dword ptr [esp - 12]
    mov     edx, dword ptr [esp - 8]
    ret
  __ftol endp


  ; __ftol2_sse
  ; Convert floating point to integer
  ; @param Float The floating point to convert to integer
  ; @return The integer converted from the floating point
  __ftol2_sse:

  ; __ftol2
  ; Convert floating point to integer
  ; @param Float The floating point to convert to integer
  ; @return The integer converted from the floating point
  __ftol2 proc
    assume  cs:_TEXT
    fnstcw  word ptr [esp - 2]
    mov     ax, word ptr [esp - 2]
    or      ax, 0C00h
    mov     word ptr [esp - 4], ax
    fldcw   word ptr [esp - 4]
    fistp   qword ptr [esp - 12]
    fldcw   word ptr [esp - 2]
    mov     eax, dword ptr [esp - 12]
    mov     edx, dword ptr [esp - 8]
    ret
  __ftol2 endp

_TEXT ends

end
