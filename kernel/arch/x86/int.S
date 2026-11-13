#include <arch/x86/regs.h>

# CPU exception handler macro
.macro INTERRUPT_EXCEPTION num
.global _asm_int_\num
_asm_int_\num:
	# Push dummy error code to the stack
	pushl $0x0
	INTERRUPT_EXCEPTION_COMMON \num
.endm

# CPU exception handler with error macro
.macro INTERRUPT_EXCEPTION_ERR num
.global _asm_int_\num
_asm_int_\num:
	# The error code has already been pushed to the stack by the CPU
	INTERRUPT_EXCEPTION_COMMON \num
.endm

.macro INTERRUPT_EXCEPTION_COMMON num
	# Push interrupt number onto the stack
	pushl $\num
	SAVE_REGS
	# Push pointer to top of stack
	push %esp
	call arch_x86_cpu_exception_handler_default
	# Move stack pointer past our handler arg
	addl $4, %esp
	RESTORE_REGS
	# Move stack pointer past vector and error code
	addl $8, %esp
	iret
.endm

# Generic PIC1 interrupt handler macro
.macro INTERRUPT_PIC1 num
.global _asm_int_\num
_asm_int_\num:
	# Push dummy error code to the stack
	pushl $0x0
	# Push interrupt number onto the stack
	pushl $\num
	SAVE_REGS
	# Push pointer to top of stack
	push %esp
	call arch_x86_irq_handler_default
	# Move stack pointer past our handler arg
	addl $4, %esp
	# Notify PIC1 that we've handled the interrupt
	pushl $(\num - 32)
	call arch_x86_pic_send_eoi
	addl $4, %esp
	RESTORE_REGS
	# Move stack pointer past vector and error code
	addl $8, %esp
	iret
.endm

# Generic PIC2 interrupt handler macro
.macro INTERRUPT_PIC2 num
.global _asm_int_\num
_asm_int_\num:
	# Push dummy error code to the stack
	pushl $0x0
	# Push interrupt number onto the stack
	pushl $\num
	SAVE_REGS
	# Push pointer to top of stack
	push %esp
	call arch_x86_irq_handler_default
	# Move stack pointer past our handler arg
	addl $4, %esp
	# Notify the PICs that we've handled the interrupt (slave first, then
	# master, unless it was spurious)
	pushl $(\num - 32)
	call arch_x86_pic_send_eoi
	addl $4, %esp
	RESTORE_REGS
	# Move stack pointer past vector and error code
	addl $8, %esp
	iret
.endm

# x86 CPU interrupts
INTERRUPT_EXCEPTION 0
INTERRUPT_EXCEPTION 1
INTERRUPT_EXCEPTION 2
INTERRUPT_EXCEPTION 3
INTERRUPT_EXCEPTION 4
INTERRUPT_EXCEPTION 5
INTERRUPT_EXCEPTION 6
INTERRUPT_EXCEPTION 7
INTERRUPT_EXCEPTION_ERR 8
INTERRUPT_EXCEPTION 9
INTERRUPT_EXCEPTION_ERR 10
INTERRUPT_EXCEPTION_ERR 11
INTERRUPT_EXCEPTION_ERR 12
INTERRUPT_EXCEPTION_ERR 13
INTERRUPT_EXCEPTION_ERR 14
INTERRUPT_EXCEPTION 15
INTERRUPT_EXCEPTION 16
INTERRUPT_EXCEPTION_ERR 17
INTERRUPT_EXCEPTION 18
INTERRUPT_EXCEPTION 19
INTERRUPT_EXCEPTION 20
INTERRUPT_EXCEPTION_ERR 21
INTERRUPT_EXCEPTION 22
INTERRUPT_EXCEPTION 23
INTERRUPT_EXCEPTION 24
INTERRUPT_EXCEPTION 25
INTERRUPT_EXCEPTION 26
INTERRUPT_EXCEPTION 27
INTERRUPT_EXCEPTION 28
INTERRUPT_EXCEPTION_ERR 29
INTERRUPT_EXCEPTION_ERR 30
INTERRUPT_EXCEPTION 31

# IRQ 0-15
#INTERRUPT_PIC1 32
INTERRUPT_PIC1 33
INTERRUPT_PIC1 34
INTERRUPT_PIC1 35
INTERRUPT_PIC1 36
INTERRUPT_PIC1 37
INTERRUPT_PIC1 38
INTERRUPT_PIC1 39
INTERRUPT_PIC2 40
INTERRUPT_PIC2 41
INTERRUPT_PIC2 42
INTERRUPT_PIC2 43
INTERRUPT_PIC2 44
INTERRUPT_PIC2 45
INTERRUPT_PIC2 46
INTERRUPT_PIC2 47
