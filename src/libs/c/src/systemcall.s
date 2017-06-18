; system call interrupt number
%define INT_NUM 128

global systemCall0
systemCall0:
    mov eax, [esp + 4]
    int INT_NUM

    ret
