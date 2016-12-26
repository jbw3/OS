; start of long mode

; instructions in this file are 64-bit
[BITS 64]

extern kernelMain			; C++ code entry point

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Text Section
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
section .text

global start64
start64:
	; set up stack
	mov rsp, kernelStackStart

	cli						; disable interrupts

	; execute the kernel
	call kernelMain			; call kernelMain()

.Linfinite:					; infinite loop
	cli
	hlt
	jmp .Linfinite

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; BSS Section
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
section .bss

; kernel stack
global kernelStackStart
global kernelStackEnd
; the stack grows downward in memory so the start
; is at a higher address than the end
alignb 8
kernelStackEnd:
	resb 4096			; reserve 4 KiB of memory
kernelStackStart:
