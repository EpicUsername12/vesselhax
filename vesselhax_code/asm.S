	.globl OSDriver_Register
OSDriver_Register:
	li %r0, 0x3200
	sc
	blr

	.globl OSDriver_CopyToSaveArea
OSDriver_CopyToSaveArea:
	li %r0, 0x4800
	sc
	blr

	.globl SC_KernelCopyData
SC_KernelCopyData:
	li %r0, 0x2500
	sc
	blr

	.globl KernelMemcpy
KernelMemcpy:
	li %r0, 0x2400
	sc
	blr

	.globl Syscall_0x26
Syscall_0x26:
	li %r0, 0x2600
	sc
	blr