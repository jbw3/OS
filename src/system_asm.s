; several functions for debugging

extern kernelStackStart
extern kernelStackEnd

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
