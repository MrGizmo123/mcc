	.file	"test.cpp"
	.text
	.p2align 4
	.globl	_Z6popcnti
	.type	_Z6popcnti, @function
_Z6popcnti:
.LFB0:
	.cfi_startproc
	movl	$32, %eax
	xorl	%edx, %edx
	.p2align 4
	.p2align 4
	.p2align 3
.L2:
	movl	%edi, %ecx
	sarl	%edi
	andl	$1, %ecx
	addl	%ecx, %edx
	subl	$1, %eax
	jne	.L2
	movl	%edx, %eax
	ret
	.cfi_endproc
.LFE0:
	.size	_Z6popcnti, .-_Z6popcnti
	.section	.text.startup,"ax",@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB1:
	.cfi_startproc
	movl	$32, %edx
	xorl	%ecx, %ecx
	movl	$10, %eax
	.p2align 4
	.p2align 4
	.p2align 3
.L6:
	movl	%eax, %esi
	sarl	%eax
	andl	$1, %esi
	addl	%esi, %ecx
	subl	$1, %edx
	jne	.L6
	movl	%ecx, %eax
	ret
	.cfi_endproc
.LFE1:
	.size	main, .-main
	.ident	"GCC: (GNU) 14.2.1 20250207"
	.section	.note.GNU-stack,"",@progbits
