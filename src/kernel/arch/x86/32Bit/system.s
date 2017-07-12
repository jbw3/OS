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

; param1: user stack address
; param2: pointer to save current stack address
global switchToUserMode
switchToUserMode:
	; save registers
	pusha

	; save current stack pointer
	mov eax, [esp + 40]
	mov [eax], esp

	; set up stack for interrupt return
	push USER_DATA_SEGMENT_SELECTOR | USER_PL	; user mode stack segment selector
	push dword [esp + 40]						; user mode stack pointer (function argument)
	pushf										; user mode control flags
	push USER_CODE_SEGMENT_SELECTOR | USER_PL	; user mode code segment selector
	push 0										; instruction pointer to user mode code

	; disable interrupts before setting up segments
	; (do this after pushf so interrupts will be
	; enabled after iret)
	cli

	; set up segments
	mov eax, USER_DATA_SEGMENT_SELECTOR | USER_PL
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	iret

global forkProcess
forkProcess:
	mov ecx, [esp + 4] ; pageDirAddr
	mov edx, [esp + 8] ; currentStackAddr

	; return false for parent process
	; (this will get pushed to the stack and restored when
	; we switch back to the parent process)
	mov eax, 0

	; save registers
	pusha

	; save current stack pointer
	mov [edx], esp

	; switch page directory
	mov cr3, ecx

	; we're in the child process now, so get rid of pushed registers
	add esp, 32

	; return true for child process
	mov eax, 1
	ret

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
