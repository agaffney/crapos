#include <arch/x86/regs.h>

# Syscall (int 0x80) handler
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
