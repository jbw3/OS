; A test program
[BITS 32]

; call system interrupt 5 times
	mov eax, 5

start:
	int 128
	dec eax
	jnz start

	jmp $
