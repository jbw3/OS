; Wrapper to launch/clean up C/C++ programs

extern _init
extern main
extern exit

section .text

	call _init

	call main

	push eax	; main return code is argument for exit
	call exit
