# Macro to save general/segment registers to stack and set DS register to
# appropriate value for running kernel code
.macro SAVE_REGS
	# Push general registers to stack
	pushal
	# Push segment registers to stack
	push %ds
	push %es
	push %fs
	push %gs
	# Set current data segment to kernel code selector
	push %ebx
	mov $0x10, %bx
	mov %bx, %ds
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

# Generic interrupt handler macro
.global _asm_int_syscall
_asm_int_syscall:
	SAVE_REGS
	# Push syscall number onto the stack
	pushl %eax
	call _int_handler_syscall
	# Remove what we'd previously pushed onto the stack for consistency
	popl %eax
	RESTORE_REGS
	iret
