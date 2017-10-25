; Wrapper to launch/clean up C/C++ programs

extern main
extern exit

section .text

	call main

	push eax	; main return code is argument for exit
	call exit
