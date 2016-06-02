;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; calls cpuid instruction
; void getCpuInfo_x86(cpuinfo_x86_t*);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global getCpuInfo_x86
getCpuInfo_x86:
    ; cpuid returns info in eax, ebx, ecx, and edx.
    ; need to save registers first...
    ; caller-saved push eax
    ;push ebx    ; callee-saved
    ;push ecx
    ;push edx

    ; tmp test:
    ;mov dword eax, [esp+4]    ; eax <- &cpuinfo
    ;mov dword [eax], 0xa      ; cpuinfo->ten <- 0xa
    ;mov dword [eax+4], 0x21   ; cpuinfo->thirtyThree <- 0x21

    ; set up for first cpuid call
    xor eax, eax    ; eax <- 0x0000
    cpuid

    push eax    ; sp adjusted...

    mov dword eax, [esp+8]  ; eax <- &cpuinfo
    mov dword eax, [eax]    ; eax <- cpuinfo->vendorIdString
    mov dword [eax], ebx          ; cpuinfo->vendorIdString <- ebx
    mov dword [eax+4], edx
    mov dword [eax+8], ecx

    add esp, 0x4    ; "pop stack"

    ret
    ; restore registers
    ;pop edx
    ;pop ecx
    ;pop ebx
    ;pop eax