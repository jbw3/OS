; boot.s

%include "paging.inc"

MBOOT_PAGE_ALIGN	equ 1 << 0 		; load kernel and modules on a page boundary
MBOOT_MEM_INFO		equ 1 << 1 		; provide kernel with memory info
MBOOT_OFFSETS		equ 1 << 16		; kernel offsets are provided in header
MBOOT_HEADER_MAGIC	equ 0x1BADB002	; multiboot magic number

MBOOT_HEADER_FLAGS	equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO | MBOOT_OFFSETS
MBOOT_CHECKSUM		equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

; the kernel's virtual base address
KERNEL_VIRTUAL_BASE equ 0xC0000000

; the index of the page table in the page directory
KERNEL_PAGE_TABLE_IDX equ (KERNEL_VIRTUAL_BASE >> 22)

; instructions are 32-bit
[BITS 32]

extern loadStartAddr
extern loadEndAddr
extern bssEndAddr
extern _init				; global variable initialization
extern kernelMain			; C code entry point

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
	dd (mboot - KERNEL_VIRTUAL_BASE)
	dd loadStartAddr
	dd loadEndAddr
	dd bssEndAddr
	dd start

; kernel entry point
global start
start	equ (_start - KERNEL_VIRTUAL_BASE)
_start:
	; set up paging directory
	mov ecx, (kernelPageDirStart - KERNEL_VIRTUAL_BASE)
	mov cr3, ecx

	; enable paging
	mov ecx, cr0
	or ecx, 0x80000000
	mov cr0, ecx

	; jump to the higher half
	mov ecx, .higherHalf
	jmp ecx

.higherHalf:
	; unmap temporary identity mapped page
	mov dword [kernelPageDirStart + 0], 0
	invlpg [0]

	; set up stack
	mov esp, kernelStackStart	; this points the stack to the new stack area

	add ebx, KERNEL_VIRTUAL_BASE	; add virtual address offset to multiboot header pointer
	push ebx				; push multiboot header location (kernelMain param)
	push eax				; push multiboot magic number (kernelMain param)

	cli						; disable interrupts

	call _init				; call C++ global variable constructors

	; execute the kernel
	call kernelMain			; call kernelMain()

.Linfinite:					; infinite loop
	cli
	hlt
	jmp .Linfinite

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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Data Section
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
section .data

; kernel page directory
global kernelPageDirStart
global kernelPageDirEnd

align 4096
kernelPageDirStart:
dd ((tempPageTableStart - KERNEL_VIRTUAL_BASE) + (PAGE_DIR_RW | PAGE_DIR_PRESENT))
times (KERNEL_PAGE_TABLE_IDX - 1) dd 0
dd ((kernelPageTableStart - KERNEL_VIRTUAL_BASE) + (PAGE_DIR_RW | PAGE_DIR_PRESENT))
times (PAGE_TABLE_ENTRIES - KERNEL_PAGE_TABLE_IDX - 1) dd 0
kernelPageDirEnd:

; kernel page table
align 4096
kernelPageTableStart:
%assign address 0
%rep 768
dd (address | (PAGE_TABLE_RW | PAGE_TABLE_PRESENT))
%assign address address + 4096
%endrep
times (PAGE_TABLE_ENTRIES - 768) dd 0
kernelPageTableEnd:

; temporary page table for identity mapping the kernel
; until we jump to the higher half; we only need to
; map the first page in the kernel which starts at
; physical address 0x100000 (1 MiB)
align 4096
tempPageTableStart:
times 256 dd 0
dd (0x100000 | (PAGE_TABLE_RW | PAGE_TABLE_PRESENT))
times (1024 - 256 - 1) dd 0
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
alignb 4
kernelStackEnd:
	resb 4096			; reserve 4 KiB of memory
kernelStackStart:
