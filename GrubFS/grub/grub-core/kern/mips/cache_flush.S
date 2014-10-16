#ifndef CACHE_OP_DEFINED
#define CACHE_OP_DEFINED 1
	.macro cache_op op addr
	.set mips3
	cache \op, \addr
	.set mips1
	.endm
	.macro sync_op
	.set mips3
	sync
	.set mips1
	.endm
#endif

	move $t2, $a0
	addu $t3, $a0, $a1
	srl $t2, $t2, 5
	sll $t2, $t2, 5
	addu $t3, $t3, 0x1f
	srl $t3, $t3, 5
	sll $t3, $t3, 5
	move $t0, $t2
	subu $t1, $t3, $t2
1:
	cache_op 1, 0($t0)
	/* All four ways.  */
#ifdef GRUB_MACHINE_MIPS_LOONGSON
	cache_op 1, 1($t0)
	cache_op 1, 2($t0)
	cache_op 1, 3($t0)
	addiu $t1, $t1, -0x20
	bne $t1, $zero, 1b
	 addiu $t0, $t0, 0x20

#else
	addiu $t1, $t1, -0x4
	bne $t1, $zero, 1b
	 addiu $t0, $t0, 0x4
#endif
	sync_op
	move $t0, $t2
	subu $t1, $t3, $t2
2:
	cache_op 0, 0($t0)
#ifdef GRUB_MACHINE_MIPS_LOONGSON
	addiu $t1, $t1, -0x20
	bne $t1, $zero, 2b
	 addiu $t0, $t0, 0x20
#else
	addiu $t1, $t1, -0x4
	bne $t1, $zero, 2b
	 addiu $t0, $t0, 0x4
#endif
	sync_op
