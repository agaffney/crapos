#include <arch/x86/regs.h>

# Syscall (int 0x80) handler
.global _asm_int_syscall
_asm_int_syscall:
	# Push dummy value for error code
	pushl $0x0
	# Push syscall number as vector
	push %eax
	SAVE_REGS
	# Push pointer to top of stack
	push %esp
	call arch_x86_int_handler_syscall
	# Move stack pointer past our handler arg
	addl $4, %esp
	# TODO: change this to save the current %eax somewhere else and restore it directly after RESTORE_REGS
	# Update saved eax value with current value
	movl %eax, 44(%esp)
	RESTORE_REGS
	# Move stack pointer past syscall number and error code
	addl $8, %esp
	iret
