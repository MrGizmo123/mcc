	.text
	.attribute	4, 16
	.attribute	5, "rv32i2p1_m2p0_a2p1_c2p0"
	.file	"test.c"
	.globl	factorial                       # -- Begin function factorial
	.p2align	1
	.type	factorial,@function
factorial:                              # @factorial
# %bb.0:
	addi	sp, sp, -32
	sw	ra, 28(sp)                      # 4-byte Folded Spill
	sw	s0, 24(sp)                      # 4-byte Folded Spill
	addi	s0, sp, 32
	sw	a0, -16(s0)
	lw	a0, -16(s0)
	bnez	a0, .LBB0_2
	j	.LBB0_1
.LBB0_1:
	li	a0, 1
	sw	a0, -12(s0)
	j	.LBB0_3
.LBB0_2:
	lw	a0, -16(s0)
	sw	a0, -20(s0)                     # 4-byte Folded Spill
	addi	a0, a0, -1
	call	factorial
	mv	a1, a0
	lw	a0, -20(s0)                     # 4-byte Folded Reload
	mul	a0, a0, a1
	sw	a0, -12(s0)
	j	.LBB0_3
.LBB0_3:
	lw	a0, -12(s0)
	lw	ra, 28(sp)                      # 4-byte Folded Reload
	lw	s0, 24(sp)                      # 4-byte Folded Reload
	addi	sp, sp, 32
	ret
.Lfunc_end0:
	.size	factorial, .Lfunc_end0-factorial
                                        # -- End function
	.globl	main                            # -- Begin function main
	.p2align	1
	.type	main,@function
main:                                   # @main
# %bb.0:
	addi	sp, sp, -16
	sw	ra, 12(sp)                      # 4-byte Folded Spill
	sw	s0, 8(sp)                       # 4-byte Folded Spill
	addi	s0, sp, 16
	li	a0, 0
	sw	a0, -12(s0)
	li	a0, 6
	call	factorial
	sw	a0, -16(s0)
	lw	a0, -16(s0)
	lw	ra, 12(sp)                      # 4-byte Folded Reload
	lw	s0, 8(sp)                       # 4-byte Folded Reload
	addi	sp, sp, 16
	ret
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
                                        # -- End function
	.ident	"clang version 18.1.8"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym factorial
