; 32-bit x86 crtn.s

section .init
    ; GCC will put the contents of crtend.o's .init section here
    pop ebp
    ret

section .fini
    ; GCC will put the contents of crtend.o's .fini section here
    pop ebp
    ret
