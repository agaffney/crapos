;nasm directive - 32 bit
bits 32

global load_idt

load_idt:
	; move last 4 bytes of stack (stack pointer "plus" 4, stack grows "down") into EDX (32-bit)
	mov edx, [esp + 4]
	lidt [edx]
	; enable interrupts
	sti
	ret

global keyboard_handler
extern x86_keyboard_handler

; We need to use the 'iretd' instruction when returning from an interrupt
; handler, so we create a wrapper function to make sure this happens
keyboard_handler:
	call    x86_keyboard_handler
	iretd
