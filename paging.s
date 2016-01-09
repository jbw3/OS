extern pageDirStart
extern pageDirEnd

PAGE_DIR_ADDRESS equ 0xFFFFF000
PAGE_DIR_RW      equ 0x00000002

PAGE_DIR_INIT_ENTRY equ PAGE_DIR_RW

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; get the address of the start of
; the page dir
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global getPageDirStart
getPageDirStart:
    mov eax, pageDirStart
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; get the address of the end of
; the page dir
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global getPageDirEnd
getPageDirEnd:
    mov eax, pageDirEnd
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; initialize the page directory
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global initPageDir
initPageDir:
    mov eax, pageDirStart
    mov ecx, PAGE_DIR_INIT_ENTRY

.Lstart
    mov [eax   ], ecx
    mov [eax+ 4], ecx
    mov [eax+ 8], ecx
    mov [eax+12], ecx
    mov [eax+16], ecx
    mov [eax+20], ecx
    mov [eax+24], ecx
    mov [eax+28], ecx
    add eax, 32
    cmp eax, pageDirEnd
    jl .Lstart

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; enables paging
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global enablePaging
enablePaging:
    ; load the page directory address
    mov eax, pageDirStart
    mov cr3, eax

    ; enable paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ret
