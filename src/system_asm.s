; several functions for debugging

extern kernel_stack_start
extern kernel_stack_end
extern end

; get the stack pointer
global getStackPointer
getStackPointer:
    mov eax, esp
    ret

; get the start of the stack
global getStackStart
getStackStart:
    mov eax, kernel_stack_start
    ret

; get the end of the stack
global getStackEnd
getStackEnd:
    mov eax, kernel_stack_end
    ret

; get the offset of the stack pointer from
; the start of the stack
global getStackOffset
getStackOffset:
    mov eax, kernel_stack_start
    sub eax, esp
    ret

; get the kernel's end address
global getKernelEnd
getKernelEnd:
    mov eax, end
    ret

; get the CR2 register value
global getRegCR2
getRegCR2:
    mov eax, cr2
    ret