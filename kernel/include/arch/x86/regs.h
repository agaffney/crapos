#pragma once

/*
 * Register save/restore macros shared by the assembly interrupt stubs. This
 * header is only meant for .S files (which go through the C preprocessor), so
 * it is empty when included from C.
 */
#ifdef __ASSEMBLER__

# Macro to save general/segment registers to stack and load the kernel data
# segment selector into the data segment registers (ds, es, fs and gs)
.macro SAVE_REGS
	# Push general registers to stack
	pushal
	# Push segment registers to stack. A segment push may write only the low 16
	# bits of its 32-bit slot on some CPUs and leave stale stack contents in
	# the upper half, so clear that half to keep the frame fields exact. This is
	# four extra stores on every interrupt, which is cheap enough not to matter.
	push %ds
	movw $0, 2(%esp)
	push %es
	movw $0, 2(%esp)
	push %fs
	movw $0, 2(%esp)
	push %gs
	movw $0, 2(%esp)
	# Load the kernel data segment selector into every data segment register
	push %ebx
	mov $0x10, %bx
	mov %bx, %ds
	mov %bx, %es
	mov %bx, %fs
	mov %bx, %gs
	pop %ebx
	# The C ABI needs the direction flag clear whenever a function is called.
	# The interrupted code may have left it set, and an interrupt gate clears
	# only IF and TF, so clear it here. The iret restores the interrupted
	# code's flags, so it gets its own direction flag back.
	cld
.endm

# Macro to restore general/segment registers from stack when returning from
# interrupt handler
.macro RESTORE_REGS
	# Restore segment registers from stack
	pop %gs
	pop %fs
	pop %es
	pop %ds
	# Restore general registers from stack
	popal
.endm

#endif
