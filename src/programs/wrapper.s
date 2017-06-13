; Wrapper to launch/clean up C/C++ programs

extern main

section .text

	call main

	; haven't yet implemented a way to get back to kernel mode from
	; user mode, so just loop forever
	jmp $
