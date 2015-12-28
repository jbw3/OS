; boot.s

MBOOT_PAGE_ALIGN	equ 1 << 0 		; load kernel and modules on a page boundary
MBOOT_MEM_INFO		equ 1 << 1 		; provide kernel with memory info
MBOOT_HEADER_MAGIC	equ 0x1BADB002	; multiboot magic number

MBOOT_HEADER_FLAGS	equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM		equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

; instructions are 32-bit
[BITS 32]

global mboot	; make 'mboot' accessible from C
extern code		; start of the .text section
extern bss		; start of the .bss section
extern end		; end of the last loadable section

; this part must be 4-byte aligned
ALIGN 4
mboot:
	dd MBOOT_HEADER_MAGIC	; GRUB will search for this value on each 4-byte
							; boundary in the kernel file
	dd MBOOT_HEADER_FLAGS	; how GRUB should load your file/settings
	dd MBOOT_CHECKSUM		; to ensure the above values are correct

	dd mboot				; location of this descriptor
	dd code					; start of the kernel .text (code) section
	dd bss					; end of the kernel .data section
	dd end					; end of the kernel
	dd start				; kernel entry point (initial EIP)

global start				; kernel entry point
extern kernelMain			; C code entry point

start:
	mov esp, _sys_stack		; this points the stack to the new stack area
	push ebx				; load multiboot header location

	; execute the kernel
	cli						; disable interrupts
	call kernelMain			; call kernelMain()
	jmp $					; infinite loop

; Set up the segment registers:
; Kernel code descriptor offset: 8 B
; Kernel data descriptor offset: 16 B
; To set CS, we have to do a far jump. A far jump includes
; a segment as well as an offset
; Note: This is declared in C as "extern void gdtFlush(uint32_t);"
global gdtFlush
gdtFlush:
	mov eax, [esp+4]	; get the pointer to the GDT, passed as a parameter
	lgdt [eax]			; load the new GDT pointer

	mov ax, 16			; 16 is the offset in the GDT to the data segment
	mov ds, ax			; load all data segment selectors
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 8:.flush		; 8 is the offset to the code segment
.flush:
	ret

; Loads the IDT into the processor
; Note: This is declared in C as "extern void idtFlush(uint32_t);"
global idtFlush
idtFlush:
	mov eax, [esp+4] 	; get the pointer to the IDT, passed as a parameter
	lidt [eax]			; load the IDT pointer
	ret

; definition of BSS section that stores the stack
section .bss
	resb 8192				; reserve 8 KB of memory
_sys_stack:
