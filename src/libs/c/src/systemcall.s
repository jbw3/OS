; system call interrupt number
%define INT_NUM 128

global systemCallNumArgs
systemCallNumArgs:
    int INT_NUM
    ret
