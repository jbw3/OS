argStart equ 8

global systemCall
systemCall:
	push ebx

	mov eax, [esp + argStart]		; funcPtr
	mov ebx, [esp + argStart + 4]	; numArgs

	mov ecx, ebx ; copy numArgs
	cmp ecx, 0
	jz .exec

	; push all arguments to the stack (in reverse order)
	mov edx, ebx	; copy num args
	shl edx, 2		; multiply by 4
	add edx, [esp + argStart + 8] ; add argsPtr offset
.pushLoop:
	sub edx, 4
	push dword [edx]
	dec ecx
	jnz .pushLoop

.exec:
	; execute system call
	call eax
	; DO NOT USE THE eax REGISTER AFTER THE SYSTEM CALL!!!
	; IT CONTAINS THE SYSTEM CALL'S RETURN VALUE!!!

	mov ecx, ebx ; copy numArgs
	cmp ecx, 0
	jz .end

	; pop arguments
.popLoop:
	pop edx
	dec ecx
	jnz .popLoop

.end:
	pop ebx
	ret
