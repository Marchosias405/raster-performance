_Z21smooth_row_major_intoRK6RasterRS_:
.LFB3935:
	.cfi_startproc
	endbr64
	movq	(%rdi), %r11
	cmpq	$2, %r11
	jbe	.L8
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	pushq	%r12
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	pushq	%rbp
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	pushq	%rbx
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	movq	8(%rdi), %rbx
	cmpq	$2, %rbx
	jbe	.L6
	movq	16(%rdi), %rdi
	movq	16(%rsi), %r8
	subq	$1, %rbx
	movq	%r11, %r10
	leaq	0(,%r11,8), %rax
	vmovss	.LC0(%rip), %xmm1
	leaq	0(,%r11,4), %r13
	movl	$1, %r9d
	movq	%rdi, %rdx
	leaq	-8(%rax), %rsi
	leaq	8(%rdi), %rbp
	subq	%rax, %rdx
	leaq	8(%rdx), %r12
	.p2align 4,,10
	.p2align 3
.L3:
	movq	%r10, %rax
	addq	$1, %r9
	leaq	(%r12,%rsi), %rcx
	addq	%r11, %r10
	leaq	0(%rbp,%rsi), %rdx
	salq	$2, %rax
	.p2align 4,,10
	.p2align 3
.L4:
	vmovss	(%rcx), %xmm0
	vaddss	4(%rcx), %xmm0, %xmm0
	addq	$4, %rdx
	addq	$4, %rcx
	vaddss	4(%rcx), %xmm0, %xmm0
	vaddss	(%rdi,%rax), %xmm0, %xmm0
	vaddss	4(%rdi,%rax), %xmm0, %xmm0
	vaddss	8(%rdi,%rax), %xmm0, %xmm0
	vaddss	-4(%rdx), %xmm0, %xmm0
	vaddss	(%rdx), %xmm0, %xmm0
	vaddss	4(%rdx), %xmm0, %xmm0
	vdivss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 4(%r8,%rax)
	addq	$4, %rax
	cmpq	%rax, %rsi
	jne	.L4
	addq	%r13, %rsi
	cmpq	%rbx, %r9
	jne	.L3
.L6:
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L8:
	.cfi_restore 3
	.cfi_restore 6
	.cfi_restore 12
	.cfi_restore 13
	ret
	.cfi_endproc
.LFE3935:
	.size	_Z21smooth_row_major_intoRK6RasterRS_, .-_Z21smooth_row_major_intoRK6RasterRS_
