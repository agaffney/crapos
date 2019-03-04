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
.macro INTERRUPT num
.global _asm_int_\num
_asm_int_\num:
	SAVE_REGS
	# Push interrupt number onto the stack
	pushl $\num
	call _int_handler_default
	# Remove what we'd previously pushed onto the stack for consistency
	popl %eax
	RESTORE_REGS
	iret
.endm

# Generic PIC1 interrupt handler macro
.macro INTERRUPT_PIC1 num
.global _asm_int_\num
_asm_int_\num:
	SAVE_REGS
	# Push interrupt number onto the stack
	pushl $\num
	call _int_handler_default
	# Remove what we'd previously pushed onto the stack for consistency
	popl %eax
	# Notify PIC1 that we've handled the interrupt
	movb $0x20, %al
	outb %al, $0x20
	RESTORE_REGS
	iret
.endm

# Generic PIC2 interrupt handler macro
.macro INTERRUPT_PIC2 num
.global _asm_int_\num
_asm_int_\num:
	SAVE_REGS
	# Push interrupt number onto the stack
	pushl $\num
	call _int_handler_default
	# Remove what we'd previously pushed onto the stack for consistency
	popl %eax
	# Notify PIC1/PIC2 that we've handled the interrupt
	movb $0x20, %al
	# Notify PIC2 (slave)
	outb %al, $0xa0
	# Notify PIC1 (master)
	outb %al, $0x20
	RESTORE_REGS
	iret
.endm

# x86 CPU interrupts
INTERRUPT 0
INTERRUPT 1
INTERRUPT 2
INTERRUPT 3
INTERRUPT 4
INTERRUPT 5
INTERRUPT 6
INTERRUPT 7
INTERRUPT 8
INTERRUPT 9
INTERRUPT 10
INTERRUPT 11
INTERRUPT 12
INTERRUPT 13
INTERRUPT 14
INTERRUPT 15
INTERRUPT 16
INTERRUPT 17
INTERRUPT 18
INTERRUPT 19
INTERRUPT 20
INTERRUPT 21
INTERRUPT 22
INTERRUPT 23
INTERRUPT 24
INTERRUPT 25
INTERRUPT 26
INTERRUPT 27
INTERRUPT 28
INTERRUPT 29
INTERRUPT 30
INTERRUPT 31

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

# Syscall
INTERRUPT 128

