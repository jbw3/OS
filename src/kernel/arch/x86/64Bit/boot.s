; boot.s

MBOOT_PAGE_ALIGN    equ 1 << 0      ; load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1 << 1      ; provide kernel with memory info
MBOOT_OFFSETS       equ 1 << 16     ; kernel offsets are provided in header
MBOOT_HEADER_MAGIC  equ 0x1BADB002  ; multiboot magic number

MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO | MBOOT_OFFSETS
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

[BITS 32]

extern loadStartAddr
extern loadEndAddr
extern bssEndAddr
extern _init                ; global variable initialization
extern kernelMain           ; C code entry point

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Text Section
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
section .text

; this part must be 4-byte aligned
align 4
mboot:
	dd MBOOT_HEADER_MAGIC	; GRUB will search for this value on each 4-byte
							; boundary in the kernel file
	dd MBOOT_HEADER_FLAGS	; how GRUB should load your file/settings
	dd MBOOT_CHECKSUM		; to ensure the above values are correct
	dd mboot
	dd loadStartAddr
	dd loadEndAddr
	dd bssEndAddr
	dd start

; kernel entry point
global start
start   equ _start
_start:

	; set up stack
	mov esp, kernelStackStart	; this points the stack to the new stack area

	mov dword [0xb8000], 0x2f4b2f4f
	hlt

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
