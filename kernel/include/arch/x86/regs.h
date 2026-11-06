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
	# Push segment registers to stack
	push %ds
	push %es
	push %fs
	push %gs
	# Load the kernel data segment selector into every data segment register
	push %ebx
	mov $0x10, %bx
	mov %bx, %ds
	mov %bx, %es
	mov %bx, %fs
	mov %bx, %gs
	pop %ebx
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
