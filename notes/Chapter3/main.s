	.file	"main.c"
	.text
	.globl	set_val
	.type	set_val, @function
set_val:
.LFB0:
	.cfi_startproc
	endbr64
.L2:
	testq	%rdi, %rdi
	je	.L4
	movslq	12(%rdi), %rax
	movl	%esi, (%rdi,%rax,4)
	movq	16(%rdi), %rdi
	jmp	.L2
.L4:
	ret
	.cfi_endproc
.LFE0:
	.size	set_val, .-set_val
	.ident	"GCC: (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
