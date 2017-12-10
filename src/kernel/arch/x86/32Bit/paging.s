%include "paging.inc"

extern kernelPageDir
extern kernelPageTable1
extern kernelPageTable2

PAGE_DIR_INIT_ENTRY		equ PAGE_DIR_RW
PAGE_TABLE_INIT_ENTRY	equ PAGE_TABLE_RW

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; get the address of the kernel
; page dir
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global getKernelPageDir
getKernelPageDir:
	mov eax, kernelPageDir
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; get the address of a kernel page
; table
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global getKernelPageTable1
getKernelPageTable1:
	mov eax, kernelPageTable1
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; get the address of a kernel page
; table
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global getKernelPageTable2
getKernelPageTable2:
	mov eax, kernelPageTable2
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; sets the page directory
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global setPageDirectory
setPageDirectory:
	mov eax, [esp + 4]
	mov cr3, eax

	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; invalidate TLB for given address
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global invalidatePage
invalidatePage:
	mov eax, [esp + 4]
	invlpg [eax]

	ret
