#pragma once

#include <stdint.h>

#define INT_SYSCALL 0x80

// Defined via ASM
extern void keyboard_handler(void);
extern void _default_int_handler(void);
extern void _asm_int_0(void);
extern void _asm_int_1(void);
extern void _asm_int_2(void);
extern void _asm_int_3(void);
extern void _asm_int_4(void);
//extern void _asm_int_32(void);
extern void _asm_int_33(void);
extern void _asm_int_34(void);
extern void _asm_int_35(void);
extern void _asm_int_36(void);
extern void _asm_int_37(void);
extern void _asm_int_38(void);
extern void _asm_int_39(void);
extern void _asm_int_128(void);

void default_int_handler(uint32_t);
