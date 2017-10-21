;nasm directive - 32 bit
bits 32

global keyboard_handler
extern keyboard_handler_main

keyboard_handler:
	call    keyboard_handler_main
	iretd

