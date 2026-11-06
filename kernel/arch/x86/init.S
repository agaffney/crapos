#include <arch/x86/regs.h>

.section .text

.global keyboard_handler
.type keyboard_handler, @function
# We need to use the 'iret' instruction when returning from an interrupt
# handler, so we create a wrapper function to make sure this happens
keyboard_handler:
	SAVE_REGS
	call x86_keyboard_handler
	RESTORE_REGS
	iret
