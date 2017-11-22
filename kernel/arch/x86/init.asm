;nasm directive - 32 bit
bits 32

section .text
        ;multiboot spec
        align 4
        dd 0x1BADB002            ;magic
        dd 0x00                  ;flags
        dd - (0x1BADB002 + 0x00) ;checksum. m+f+c should be zero

global start
extern kmain	        ;kmain is defined in the c file

start:
	cli 			;block interrupts
	mov esp, stack_space	;set stack pointer
	call kmain
	hlt		 	;halt the CPU

global inb

inb:
	mov edx, [esp + 4]
	; the AL register is the lower bits of the EAX register, which gcc will
	; automatically look at for an integer return value (per C calling conventions)
	in al, dx
	ret

global outb

outb:
	mov   edx, [esp + 4]
	mov   al, [esp + 4 + 4]
	out   dx, al
	ret

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

section .bss
resb 8192		;8KB for stack
stack_space:
