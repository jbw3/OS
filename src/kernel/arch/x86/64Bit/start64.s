; start of long mode

; instructions in this file are 64-bit
[BITS 64]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Text Section
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
section .text

global start64
start64:
	mov rax, 0x2f212f592f452f48
	mov qword [0xB8000], rax

.Linfinite:				; infinite loop
	cli
	hlt
	jmp .Linfinite
