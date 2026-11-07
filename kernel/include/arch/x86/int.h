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
extern void _asm_int_5(void);
extern void _asm_int_6(void);
extern void _asm_int_7(void);
extern void _asm_int_8(void);
extern void _asm_int_9(void);
extern void _asm_int_10(void);
extern void _asm_int_11(void);
extern void _asm_int_12(void);
extern void _asm_int_13(void);
extern void _asm_int_14(void);
extern void _asm_int_15(void);
extern void _asm_int_16(void);
extern void _asm_int_17(void);
extern void _asm_int_18(void);
extern void _asm_int_19(void);
extern void _asm_int_20(void);
extern void _asm_int_21(void);
extern void _asm_int_22(void);
extern void _asm_int_23(void);
extern void _asm_int_24(void);
extern void _asm_int_25(void);
extern void _asm_int_26(void);
extern void _asm_int_27(void);
extern void _asm_int_28(void);
extern void _asm_int_29(void);
extern void _asm_int_30(void);
extern void _asm_int_31(void);
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

// Stack frame shape for passing to interrupt handlers
typedef struct {
	uint32_t gs, fs, es, ds;                       // pushed by SAVE_REGS
	uint32_t edi, esi, ebp, _esp, ebx, edx, ecx, eax;   // pushal order
	uint32_t vector;
	uint32_t error_code;
	uint32_t eip, cs, eflags;                      // pushed by the CPU
} arch_x86_int_frame;

/*
 * Called at the start of arch_x86_irq_handler_default(),
 * arch_x86_cpu_exception_handler_default() and arch_x86_int_handler_syscall(),
 * with the vector number and the frame the handler was given. Only defined by
 * the in-kernel tests (KTEST=1 builds), which use it to look at the CPU state
 * inside a handler.
 *
 * Returns non-zero when the test caused this interrupt on purpose and wants it
 * handled as recoverable, which stops the CPU exception handler halting the
 * machine on a fatal vector. The other handlers ignore it.
 */
int ktest_int_hook(uint32_t vector, const arch_x86_int_frame * frame);

void arch_x86_irq_handler_default(const arch_x86_int_frame *);

void arch_x86_cpu_exception_handler_default(const arch_x86_int_frame *);

/*
 * Whether returning from a CPU exception vector is wrong: see int.c. Only CPU
 * exceptions (vectors 0-31) can be fatal, so this is 0 for any other vector.
 */
int arch_x86_cpu_exception_is_fatal(uint32_t vector);

/* The name of a CPU exception vector, for reports. Only vectors 0-31 have one. */
const char * arch_x86_cpu_exception_name(uint32_t vector);

/* Log every field of an interrupt frame on one debug line */
void arch_x86_int_debug_frame(const arch_x86_int_frame *);
