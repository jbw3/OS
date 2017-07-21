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

INTERRUPT_FLAG equ 0x200

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

; param1: user stack address
; param2: pointer to save current stack address
global switchToUserMode
switchToUserMode:
	; disable interrupts
	cli

	; save registers
	pusha

	; save current stack pointer
	mov eax, [esp + 40]
	mov [eax], esp

	; set up stack for interrupt return
	push USER_DATA_SEGMENT_SELECTOR | USER_PL	; user mode stack segment selector
	push dword [esp + 40]						; user mode stack pointer (function argument)
	pushf										; user mode control flags
	or dword [esp], INTERRUPT_FLAG
	push USER_CODE_SEGMENT_SELECTOR | USER_PL	; user mode code segment selector
	push 0										; instruction pointer to user mode code

	; set up segments
	mov eax, USER_DATA_SEGMENT_SELECTOR | USER_PL
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	iret

; param1: new stack address
; param2: pointer to save current stack address
global switchToProcessStack
switchToProcessStack:
	; save registers on current stack
	pusha

	; save current stack pointer
	mov eax, [esp + 40]
	mov [eax], esp

	; switch to new stack
	mov esp, [esp + 36]

	; restore registers
	popa

	ret
