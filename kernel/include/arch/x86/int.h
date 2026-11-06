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
extern void _asm_int_40(void);
extern void _asm_int_41(void);
extern void _asm_int_42(void);
extern void _asm_int_43(void);
extern void _asm_int_44(void);
extern void _asm_int_45(void);
extern void _asm_int_46(void);
extern void _asm_int_47(void);

void _int_handler_default(uint32_t);

/*
 * Called at the start of _int_handler_default, with the vector number. Only
 * defined by the in-kernel tests (KTEST=1 builds), which use it to look at the
 * CPU state inside a handler.
 */
void ktest_int_hook(uint32_t vector);
