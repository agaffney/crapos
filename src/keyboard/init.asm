;nasm directive - 32 bit
bits 32

global keyboard_handler
extern keyboard_handler_main

; We need to use the 'iretd' instruction when returning from an interrupt
; handler, so we create a wrapper function to make sure this happens
keyboard_handler:
	call    keyboard_handler_main
	iretd
