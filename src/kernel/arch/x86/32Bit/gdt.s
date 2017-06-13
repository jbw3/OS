; Set up the segment registers:
; Kernel code descriptor offset: 8 B
; Kernel data descriptor offset: 16 B
; To set CS, we have to do a far jump. A far jump includes
; a segment as well as an offset
global loadGdt
loadGdt:
	mov eax, [esp+4]	; get the pointer to the GDT, passed as a parameter
	lgdt [eax]			; load the new GDT pointer

	mov ax, 16			; 16 is the offset in the GDT to the data segment
	mov ds, ax			; load all data segment selectors
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 8:.codeSegment	; 8 is the offset to the code segment
.codeSegment:
	ret
