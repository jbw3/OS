%include "paging.inc"

extern kernelPageDirStart
extern kernelPageDirEnd
extern kernelPageTableStart
extern kernelPageTableEnd

PAGE_DIR_INIT_ENTRY		equ PAGE_DIR_RW
PAGE_TABLE_INIT_ENTRY	equ PAGE_TABLE_RW

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; get the address of the start of
; the page dir
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global getKernelPageDirStart
getKernelPageDirStart:
	mov eax, kernelPageDirStart
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; get the address of the end of
; the page dir
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global getKernelPageDirEnd
getKernelPageDirEnd:
	mov eax, kernelPageDirEnd
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; get the address of the start of
; the page table
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global getKernelPageTableStart
getKernelPageTableStart:
	mov eax, kernelPageTableStart
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; get the address of the end of
; the page table
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global getKernelPageTableEnd
getKernelPageTableEnd:
	mov eax, kernelPageTableEnd
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; initialize the page directory
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global initPageDir
initPageDir:
	mov eax, kernelPageDirStart
.Lstart:
	mov dword [eax   ], PAGE_DIR_INIT_ENTRY
	mov dword [eax+ 4], PAGE_DIR_INIT_ENTRY
	mov dword [eax+ 8], PAGE_DIR_INIT_ENTRY
	mov dword [eax+12], PAGE_DIR_INIT_ENTRY
	mov dword [eax+16], PAGE_DIR_INIT_ENTRY
	mov dword [eax+20], PAGE_DIR_INIT_ENTRY
	mov dword [eax+24], PAGE_DIR_INIT_ENTRY
	mov dword [eax+28], PAGE_DIR_INIT_ENTRY
	mov dword [eax+32], PAGE_DIR_INIT_ENTRY
	mov dword [eax+36], PAGE_DIR_INIT_ENTRY
	mov dword [eax+40], PAGE_DIR_INIT_ENTRY
	mov dword [eax+44], PAGE_DIR_INIT_ENTRY
	mov dword [eax+48], PAGE_DIR_INIT_ENTRY
	mov dword [eax+52], PAGE_DIR_INIT_ENTRY
	mov dword [eax+56], PAGE_DIR_INIT_ENTRY
	mov dword [eax+60], PAGE_DIR_INIT_ENTRY
	add eax, 64
	cmp eax, kernelPageDirEnd
	jl .Lstart

	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; initialize a page table
; void initPageTable(void* addr);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global initPageTable
initPageTable:
	mov eax, [esp+4]	; get page table address
	mov ecx, 4096		; calculate the end of the page table
	add ecx, eax
.Lstart:
	mov dword [eax   ], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+ 4], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+ 8], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+12], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+16], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+20], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+24], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+28], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+32], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+36], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+40], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+44], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+48], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+52], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+56], PAGE_TABLE_INIT_ENTRY
	mov dword [eax+60], PAGE_TABLE_INIT_ENTRY
	add eax, 64
	cmp eax, ecx
	jl .Lstart

	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; enables paging
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global enablePaging
enablePaging:
	; load the page directory address
	mov eax, kernelPageDirStart
	mov cr3, eax

	; enable paging
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax

	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; disables paging
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global disablePaging
disablePaging:
	mov eax, cr0
	and eax, 0x7FFFFFFF
	mov cr0, eax

	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; check whether paging is enabled
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global isPagingEnabled
isPagingEnabled:
	mov eax, cr0
	shr eax, 31

	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; invalidate TLB for given address
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global invalidatePage
invalidatePage:
	mov eax, [esp + 4]
	invlpg [eax]

	ret
