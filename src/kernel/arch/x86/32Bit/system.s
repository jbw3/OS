; several functions for debugging

%include "system.inc"

; GDT indices
KERNEL_CODE_SEGMENT_SELECTOR equ 0x08
KERNEL_DATA_SEGMENT_SELECTOR equ 0x10
USER_CODE_SEGMENT_SELECTOR equ 0x18
USER_DATA_SEGMENT_SELECTOR equ 0x20

; privilege levels
KERNEL_PL equ 0
USER_PL equ 3

extern kernelStackStart
extern kernelStackEnd

global clearInt
clearInt:
	cli
	ret

global setInt
setInt:
	sti
	ret

; get the stack pointer
global getStackPointer
getStackPointer:
	mov eax, esp
	ret

; get the start of the stack
global getStackStart
getStackStart:
	mov eax, kernelStackStart
	ret

; get the end of the stack
global getStackEnd
getStackEnd:
	mov eax, kernelStackEnd
	ret

; get the offset of the stack pointer from
; the start of the stack
global getStackOffset
getStackOffset:
	mov eax, kernelStackStart
	sub eax, esp
	ret

; get the CR2 register value
global getRegCR2
getRegCR2:
	mov eax, cr2
	ret

global switchToUserMode
switchToUserMode:
	push USER_DATA_SEGMENT_SELECTOR | USER_PL	; user mode stack segment selector
	push KERNEL_VIRTUAL_BASE - 4				; user mode stack pointer

	pushf										; user mode control flags
	and word [esp], 0xfffffdff					; disable interrupts in user mode

	push USER_CODE_SEGMENT_SELECTOR | USER_PL	; user mode code segment selector
	push 0										; instruction pointer to user mode code

	iret
