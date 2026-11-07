#include <arch/x86/regs.h>

# Global variable for clock ticks
.section .data
.global _clock_ticks
_clock_ticks:
.long 0

.section .text
.global _asm_int_pit
.type _asm_int_pit, @function
_asm_int_pit:
	# Save every register and load the kernel data segments, as the other IRQ
	# stubs do, since arch_x86_pic_send_eoi is C code
	SAVE_REGS
	incl _clock_ticks
	# Notify the PIC that IRQ 0 is handled
	pushl $0
	call arch_x86_pic_send_eoi
	addl $4, %esp
	RESTORE_REGS
	iret
