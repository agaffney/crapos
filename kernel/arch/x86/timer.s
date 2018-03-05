# Global variable for clock ticks
.section .data
.global _clock_ticks
_clock_ticks:
.long 0

.section .text
.global _asm_int_32
.type _asm_int_32, @function
_asm_int_32:
	incl _clock_ticks
	mov $0x20, %al
	outb %al, $0x20
	iret
