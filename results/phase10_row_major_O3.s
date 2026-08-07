_Z21smooth_row_major_intoRK6RasterRS_:
.LFB3935:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	andq	$-32, %rsp
	subq	$72, %rsp
	.cfi_offset 15, -24
	.cfi_offset 14, -32
	.cfi_offset 13, -40
	.cfi_offset 12, -48
	.cfi_offset 3, -56
	movq	(%rdi), %rbx
	movq	%rbx, 56(%rsp)
	cmpq	$2, %rbx
	jbe	.L31
	movq	8(%rdi), %rax
	cmpq	$2, %rax
	jbe	.L31
	subq	$1, %rax
	leaq	-2(%rbx), %r11
	movq	16(%rsi), %r8
	movq	16(%rdi), %rcx
	movq	%rax, 32(%rsp)
	leaq	-1(%rbx), %rax
	movq	%r11, %rdx
	leaq	0(,%rbx,4), %r14
	movq	%rax, -40(%rsp)
	leaq	-3(%rbx), %rax
	shrq	$3, %rdx
	movq	%rbx, %r13
	movq	%rax, 48(%rsp)
	movq	%r8, %rax
	salq	$5, %rdx
	movq	%rcx, %rdi
	subq	%r14, %rax
	movq	%rdx, -48(%rsp)
	movq	%r11, %rdx
	leaq	0(,%rbx,8), %rsi
	addq	$12, %rax
	leaq	(%rcx,%rsi), %r12
	andq	$-8, %rdx
	subq	$8, %rsi
	movq	%rax, 24(%rsp)
	movq	%r8, %rax
	subq	%rdx, %rbx
	leaq	1(%rdx), %r10
	subq	%rcx, %rax
	movq	%r11, -96(%rsp)
	andl	$7, %r11d
	vbroadcastss	.LC1(%rip), %ymm2
	movq	%rax, 16(%rsp)
	movq	%rbx, %rax
	leaq	-2(%rbx), %rbx
	vmovss	.LC1(%rip), %xmm1
	subq	$3, %rax
	movq	%rdx, -88(%rsp)
	leaq	(%rcx,%r14), %r15
	movq	%r10, -64(%rsp)
	movq	$1, 64(%rsp)
	movq	%r11, -56(%rsp)
	movq	%rax, -80(%rsp)
	movq	%rbx, -72(%rsp)
	movq	%r13, %rbx
	movq	%r14, %r13
	.p2align 4,,10
	.p2align 3
.L3:
	movq	56(%rsp), %rax
	addq	$1, 64(%rsp)
	movq	%rbx, %r14
	addq	%rax, %rbx
	cmpq	$2, 48(%rsp)
	jbe	.L12
	movq	24(%rsp), %rax
	leaq	4(%r12), %r9
	leaq	4(%rdi), %r10
	leaq	(%rax,%rsi), %rdx
	movq	%rdx, %rax
	subq	%r9, %rax
	cmpq	$32, %rax
	movq	%rdx, %rax
	seta	%r11b
	subq	%r10, %rax
	cmpq	$32, %rax
	seta	%al
	testb	%al, %r11b
	je	.L12
	cmpq	$32, 16(%rsp)
	jbe	.L12
	movq	56(%rsp), %r11
	movq	%r14, %rax
	subq	%r11, %rax
	cmpq	$6, 48(%rsp)
	movq	%rax, 40(%rsp)
	jbe	.L9
	leaq	8(%rdi), %rax
	leaq	4(%r15), %r11
	movq	%rbx, -16(%rsp)
	movq	%rax, -8(%rsp)
	leaq	8(%r15), %rax
	movq	-8(%rsp), %rbx
	movq	%rax, 8(%rsp)
	xorl	%eax, %eax
	movq	%r11, (%rsp)
	leaq	8(%r12), %r11
	movq	%r8, -24(%rsp)
	movq	(%rsp), %r8
	movq	%rcx, -32(%rsp)
	movq	8(%rsp), %rcx
	movq	%r13, 8(%rsp)
	movq	-48(%rsp), %r13
	.p2align 4,,10
	.p2align 3
.L10:
	vmovups	(%r10,%rax), %ymm4
	vaddps	(%rdi,%rax), %ymm4, %ymm0
	vaddps	(%rbx,%rax), %ymm0, %ymm0
	vaddps	(%r15,%rax), %ymm0, %ymm0
	vaddps	(%r8,%rax), %ymm0, %ymm0
	vaddps	(%rcx,%rax), %ymm0, %ymm0
	vaddps	(%r12,%rax), %ymm0, %ymm0
	vaddps	(%r9,%rax), %ymm0, %ymm0
	vaddps	(%r11,%rax), %ymm0, %ymm0
	vdivps	%ymm2, %ymm0, %ymm0
	vmovups	%ymm0, (%rdx,%rax)
	addq	$32, %rax
	cmpq	%r13, %rax
	jne	.L10
	cmpq	$0, -56(%rsp)
	movq	-16(%rsp), %rbx
	movq	-24(%rsp), %r8
	movq	-32(%rsp), %rcx
	movq	8(%rsp), %r13
	je	.L8
	cmpq	$2, -80(%rsp)
	movq	-72(%rsp), %rax
	jbe	.L16
	movq	-64(%rsp), %r11
	movq	-88(%rsp), %rdx
.L15:
	movq	40(%rsp), %r10
	leaq	(%rdx,%r14), %r9
	vbroadcastss	.LC1(%rip), %xmm3
	addq	%rdx, %r10
	addq	%rbx, %rdx
	vmovups	4(%rcx,%r10,4), %xmm5
	vaddps	(%rcx,%r10,4), %xmm5, %xmm0
	vaddps	8(%rcx,%r10,4), %xmm0, %xmm0
	vaddps	(%rcx,%r9,4), %xmm0, %xmm0
	vaddps	4(%rcx,%r9,4), %xmm0, %xmm0
	vaddps	8(%rcx,%r9,4), %xmm0, %xmm0
	vaddps	(%rcx,%rdx,4), %xmm0, %xmm0
	vaddps	4(%rcx,%rdx,4), %xmm0, %xmm0
	vaddps	8(%rcx,%rdx,4), %xmm0, %xmm0
	vdivps	%xmm3, %xmm0, %xmm0
	vmovups	%xmm0, 4(%r8,%r9,4)
	testb	$3, %al
	je	.L8
	andq	$-4, %rax
	addq	%r11, %rax
.L5:
	movq	40(%rsp), %r11
	leaq	(%r14,%rax), %rdx
	leaq	(%rbx,%rax), %r9
	leaq	(%r11,%rax), %r10
	vmovss	-4(%rcx,%r10,4), %xmm0
	vaddss	(%rcx,%r10,4), %xmm0, %xmm0
	vaddss	4(%rcx,%r10,4), %xmm0, %xmm0
	vaddss	-4(%rcx,%rdx,4), %xmm0, %xmm0
	vaddss	(%rcx,%rdx,4), %xmm0, %xmm0
	vaddss	4(%rcx,%rdx,4), %xmm0, %xmm0
	vaddss	-4(%rcx,%r9,4), %xmm0, %xmm0
	vaddss	(%rcx,%r9,4), %xmm0, %xmm0
	vaddss	4(%rcx,%r9,4), %xmm0, %xmm0
	movq	-40(%rsp), %r10
	vdivss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, (%r8,%rdx,4)
	leaq	1(%rax), %rdx
	cmpq	%r10, %rdx
	jnb	.L8
	leaq	(%r11,%rdx), %r10
	leaq	(%rdx,%r14), %r9
	addq	%rbx, %rdx
	addq	$2, %rax
	vmovss	-4(%rcx,%r10,4), %xmm0
	vaddss	(%rcx,%r10,4), %xmm0, %xmm0
	vaddss	4(%rcx,%r10,4), %xmm0, %xmm0
	vaddss	-4(%rcx,%r9,4), %xmm0, %xmm0
	vaddss	(%rcx,%r9,4), %xmm0, %xmm0
	vaddss	4(%rcx,%r9,4), %xmm0, %xmm0
	vaddss	-4(%rcx,%rdx,4), %xmm0, %xmm0
	vaddss	(%rcx,%rdx,4), %xmm0, %xmm0
	vaddss	4(%rcx,%rdx,4), %xmm0, %xmm0
	movq	-40(%rsp), %r10
	vdivss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, (%r8,%r9,4)
	cmpq	%r10, %rax
	jnb	.L8
	movq	%r11, %rdx
	leaq	(%r14,%rax), %r9
	addq	%rax, %rdx
	addq	%rbx, %rax
	vmovss	-4(%rcx,%rdx,4), %xmm0
	vaddss	(%rcx,%rdx,4), %xmm0, %xmm0
	vaddss	4(%rcx,%rdx,4), %xmm0, %xmm0
	vaddss	-4(%rcx,%r9,4), %xmm0, %xmm0
	vaddss	(%rcx,%r9,4), %xmm0, %xmm0
	vaddss	4(%rcx,%r9,4), %xmm0, %xmm0
	vaddss	-4(%rcx,%rax,4), %xmm0, %xmm0
	vaddss	(%rcx,%rax,4), %xmm0, %xmm0
	vaddss	4(%rcx,%rax,4), %xmm0, %xmm0
	vdivss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, (%r8,%r9,4)
.L8:
	movq	32(%rsp), %rdx
	addq	%r13, %rdi
	addq	%r13, %r12
	addq	%r13, %rsi
	addq	%r13, %r15
	cmpq	%rdx, 64(%rsp)
	jne	.L3
	vzeroupper
.L31:
	leaq	-40(%rbp), %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	ret
	.p2align 4,,10
	.p2align 3
.L12:
	.cfi_restore_state
	leaq	0(,%r14,4), %rax
	movq	%r12, %r9
	movq	%rdi, %rdx
	.p2align 4,,10
	.p2align 3
.L7:
	vmovss	4(%rdx), %xmm0
	vaddss	(%rdx), %xmm0, %xmm0
	addq	$4, %r9
	addq	$4, %rdx
	vaddss	4(%rdx), %xmm0, %xmm0
	vaddss	(%rcx,%rax), %xmm0, %xmm0
	vaddss	4(%rcx,%rax), %xmm0, %xmm0
	vaddss	8(%rcx,%rax), %xmm0, %xmm0
	vaddss	-4(%r9), %xmm0, %xmm0
	vaddss	(%r9), %xmm0, %xmm0
	vaddss	4(%r9), %xmm0, %xmm0
	vdivss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 4(%r8,%rax)
	addq	$4, %rax
	cmpq	%rsi, %rax
	jne	.L7
	jmp	.L8
.L9:
	movq	-96(%rsp), %rax
	movl	$1, %r11d
	xorl	%edx, %edx
	jmp	.L15
.L16:
	movq	-64(%rsp), %rax
	jmp	.L5
	.cfi_endproc
.LFE3935:
	.size	_Z21smooth_row_major_intoRK6RasterRS_, .-_Z21smooth_row_major_intoRK6RasterRS_
