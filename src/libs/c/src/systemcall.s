; system call interrupt number
%define INT_NUM 128

global systemCall0
systemCall0:
    mov eax, 0
    int INT_NUM

    ret

global systemCall3
systemCall3:
    mov eax, 3
    int INT_NUM

    ret
