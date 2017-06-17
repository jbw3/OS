; interrupt.s
; Define the 32 basic processor ISRs and the 16 custom IRQs

; This macro creates an ISR for an interrupt
; that does not push an error code onto the stack.
; To keep the stack frame consistent with interrupts
; that do push an error code, a dummy value of 0 is
; pushed onto the stack. The interrupt number is also pushed
; to the stack.
%macro ISR_NOERRCODE 1	; define a macro taking one parameter
global isr%1
isr%1:
	push 0				; push a dummy 0 value in place of an error code
	push %1				; push the interrupt number
	jmp isrCommonStub
%endmacro

; This macro creates an ISR for an interrupt
; that pushes an error code onto the stack.
; The interrupt number is also pushed to the stack.
%macro ISR_ERRCODE 1	; define a macro taking one parameter
global isr%1
isr%1:
	push %1				; push the interrupt number
	jmp isrCommonStub
%endmacro

; This macro creates a stub for an IRQ. The first parameter is the
; IRQ number, the second is the ISR number it is remapped to
%macro IRQ 2
global irq%1
irq%1:
	push 0				; push a dummy 0 value in place of an error code
	push %2				; push the interrupt number
	jmp irqCommonStub
%endmacro

; defined in isr.c
extern isrHandler

; This is the common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C interrupt handler, and
; restores the stack frame
isrCommonStub:
	pusha				; push edi, esi, ebp, esp, ebx, edx, ecx, eax

	mov ax, ds			; mov ds to lower 16-bits of eax
	push eax			; save the data segment descriptor

	mov ax, 16			; load the kernel data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov eax, esp		; push the stack pointer
	push eax

	call isrHandler		; call the C interrupt handler

	pop eax				; pop the stack pointer

	pop eax				; reload the original data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popa				; pop edi, esi, ebp, esp, ebx, edx, ecx, eax
	add esp, 8			; cleans up the pushed error code and interrupt number
	iret				; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP
						; (these are pushed automatically by the processor)

; defined in irq.c
extern irqHandler

; This is the common IRQ stub. It saves the processor state, sets
; up for kernel mode segments, calls the C interrupt handler, and
; restores the stack frame
irqCommonStub:
	pusha				; push edi, esi, ebp, esp, ebx, edx, ecx, eax

	mov ax, ds			; mov ds to lower 16-bits of eax
	push eax			; save the data segment descriptor

	mov ax, 16			; load the kernel data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov eax, esp		; push stack pointer as arg
	push eax

	call irqHandler

	pop eax				; pop stack pointer

	pop eax				; reload the original data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popa				; pop edi, esi, ebp, esp, ebx, edx, ecx, eax
	add esp, 8			; cleans up the pushed error code and interrupt number
	iret				; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP
						; (these are pushed automatically by the processor)

; define 32 basic ISRs
ISR_NOERRCODE   0
ISR_NOERRCODE   1
ISR_NOERRCODE   2
ISR_NOERRCODE   3
ISR_NOERRCODE   4
ISR_NOERRCODE   5
ISR_NOERRCODE   6
ISR_NOERRCODE   7
ISR_ERRCODE     8
ISR_NOERRCODE   9
ISR_ERRCODE    10
ISR_ERRCODE    11
ISR_ERRCODE    12
ISR_ERRCODE    13
ISR_ERRCODE    14
ISR_NOERRCODE  15
ISR_NOERRCODE  16
ISR_ERRCODE    17
ISR_NOERRCODE  18
ISR_NOERRCODE  19
ISR_NOERRCODE  20
ISR_NOERRCODE  21
ISR_NOERRCODE  22
ISR_NOERRCODE  23
ISR_NOERRCODE  24
ISR_NOERRCODE  25
ISR_NOERRCODE  26
ISR_NOERRCODE  27
ISR_NOERRCODE  28
ISR_NOERRCODE  29
ISR_ERRCODE    30
ISR_NOERRCODE  31
ISR_NOERRCODE 128

; define IRQs
IRQ  0, 32
IRQ  1, 33
IRQ  2, 34
IRQ  3, 35
IRQ  4, 36
IRQ  5, 37
IRQ  6, 38
IRQ  7, 39
IRQ  8, 40
IRQ  9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47
