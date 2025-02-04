	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 14, 0
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:                                ; %entry
	sub	sp, sp, #32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	.cfi_def_cfa_offset 32
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	mov	w8, #1093140480                 ; =0x41280000
	mov	w9, #3                          ; =0x3
Lloh0:
	adrp	x0, l_.str_int@PAGE
Lloh1:
	add	x0, x0, l_.str_int@PAGEOFF
	stp	w9, w8, [sp, #8]
	str	x9, [sp]
	bl	_printf
	ldr	s0, [sp, #12]
Lloh2:
	adrp	x0, l_.str_float@PAGE
Lloh3:
	add	x0, x0, l_.str_float@PAGEOFF
	fcvt	d0, s0
	str	d0, [sp]
	bl	_printf
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #32
	ret
	.loh AdrpAdd	Lloh2, Lloh3
	.loh AdrpAdd	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__const
l_.str_int:                             ; @.str_int
	.asciz	"%d\n"

l_.str_float:                           ; @.str_float
	.asciz	"%f\n"

.subsections_via_symbols
