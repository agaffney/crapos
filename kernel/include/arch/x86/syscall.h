#pragma once

#include <arch/x86/int.h>

extern void _asm_int_syscall(void);

int arch_x86_int_handler_syscall(const arch_x86_int_frame *);

/*
 * Called by arch_x86_int_handler_syscall() with the value it is about to
 * return. Only defined by the in-kernel tests (KTEST=1 builds), so that they
 * can see the result without running the syscall again.
 */
void ktest_syscall_result_hook(int result);
