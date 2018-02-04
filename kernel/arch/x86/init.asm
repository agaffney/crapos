;nasm directive - 32 bit
bits 32

global keyboard_handler
extern x86_keyboard_handler

; We need to use the 'iretd' instruction when returning from an interrupt
; handler, so we create a wrapper function to make sure this happens
keyboard_handler:
	call    x86_keyboard_handler
	iretd
