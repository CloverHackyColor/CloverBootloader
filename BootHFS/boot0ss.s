;
; This version of boot0 implements hybrid GUID/MBR partition scheme support
;
; Written by Tamás Kosárszky on 2008-03-10 and JrCs on 2013-05-08.
;
; Turbo added EFI System Partition boot support
;
; Added KillerJK's switchPass2 modifications
;
; JrCs added FAT32/exFAT System Partition boot support on GPT pure partition scheme
;

;
; boot0af and boot0ss share the same code except this ACTIVEFIRST definition
; boot0af - define ACTIVEFIRST
; boot0ss - do not define ACTIVEFIRST
;

%undef ACTIVEFIRST

%include "boot0.s"
