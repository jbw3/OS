; boot.s

%include "paging.inc"

MBOOT_PAGE_ALIGN    equ 1 << 0      ; load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1 << 1      ; provide kernel with memory info
MBOOT_OFFSETS       equ 1 << 16     ; kernel offsets are provided in header
MBOOT_HEADER_MAGIC  equ 0x1BADB002  ; multiboot magic number

MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO | MBOOT_OFFSETS
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

; the kernel's virtual base address
KERNEL_VIRTUAL_BASE equ 0x0; TODO: map to higher half

; the index of the page table in the page directory
KERNEL_PAGE_TABLE_IDX equ (KERNEL_VIRTUAL_BASE >> 22)

; instructions in this file are 32-bit
[BITS 32]

extern loadStartAddr
extern loadEndAddr
extern bssEndAddr
extern start64

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
start	equ (_start - KERNEL_VIRTUAL_BASE)
_start:
	; set up page map
	mov ecx, (kernelPageMapStart - KERNEL_VIRTUAL_BASE)
	mov cr3, ecx

	; enable PAE flag (Physical Address Extension)
	mov ecx, cr4
	or ecx, 1 << 5
	mov cr4, ecx

	; set the long mode bit in the EFER MSR
	; (Extended Feature Enable Register, Model Specific Register)
	mov ecx, 0xC0000080
	rdmsr
	or eax, 1 << 8
	wrmsr

	; enable paging
	mov ecx, cr0
	or ecx, 1 << 31
	mov cr0, ecx

	; load GDT
	lgdt [gdt.struct]

	; update selectors
	mov ax, gdt.dataOffset
	mov ss, ax
	mov ds, ax
	mov es, ax
	jmp gdt.codeOffset:start64	; jump to long mode label

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Read-Only Data Section
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
section .rodata

gdt:
	dq 0 ; zero entry
.codeOffset: equ $ - gdt
	dq (1<<53) | (1<<47) | (1<<44) | (1<<43) | (1<<41) ; code segment
.dataOffset: equ $ - gdt
	dq (1<<47) | (1<<44) | (1<<41) ; data segment
.struct:
	dw $ - gdt - 1
	dq gdt

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Data Section
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
section .data

; kernel page map
align 4096
kernelPageMapStart:
dq ((kernelPageDirPtrStart - KERNEL_VIRTUAL_BASE) + (PAGE_MAP_RW | PAGE_MAP_PRESENT))
times (PAGE_TABLE_ENTRIES - 1) dq 0
kernelPageMapEnd:

; kernel page directory pointer
align 4096
kernelPageDirPtrStart:
dq ((kernelPageDirStart - KERNEL_VIRTUAL_BASE) + (PAGE_DIR_PTR_RW | PAGE_DIR_PTR_PRESENT))
times (PAGE_TABLE_ENTRIES - 1) dq 0
kernelPageDirPtrEnd:

; kernel page directory
align 4096
kernelPageDirStart:
; dq ((tempPageTableStart - KERNEL_VIRTUAL_BASE) + (PAGE_DIR_RW | PAGE_DIR_PRESENT))
; times (KERNEL_PAGE_TABLE_IDX - 1) dd 0
dq ((kernelPageTableStart - KERNEL_VIRTUAL_BASE) + (PAGE_DIR_RW | PAGE_DIR_PRESENT))
times (PAGE_TABLE_ENTRIES - KERNEL_PAGE_TABLE_IDX - 1) dq 0
kernelPageDirEnd:

; kernel page table
align 4096
kernelPageTableStart:
%assign address 0
%rep (256 + 7) ; 256 - 1 MiB, 7 - OS pages
dq (address | (PAGE_TABLE_RW | PAGE_TABLE_PRESENT))
%assign address address + 4096
%endrep
times (PAGE_TABLE_ENTRIES - (256 + 7)) dq 0
kernelPageTableEnd:

; temporary page table for identity mapping the kernel
; until we jump to the higher half; we only need to
; map the first page in the kernel which starts at
; physical address 0x100000 (1 MiB)
align 4096
tempPageTableStart:
times 256 dq 0
dq (0x100000 | (PAGE_TABLE_RW | PAGE_TABLE_PRESENT))
times (PAGE_TABLE_ENTRIES - 256 - 1) dq 0
tempPageTableEnd:

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
