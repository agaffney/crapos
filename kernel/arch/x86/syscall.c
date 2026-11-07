#include <arch/x86/syscall.h>
#include <core/kprint.h>

// ktest_int_hook only exists in KTEST=1 builds; weak, so it is null otherwise
#pragma weak ktest_int_hook
#pragma weak ktest_syscall_result_hook

int arch_x86_int_handler_syscall(const arch_x86_int_frame * f) {
	if (ktest_int_hook) {
		ktest_int_hook(INT_SYSCALL, f);
	}
	kdebug("received syscall %d\n", f->vector);
	arch_x86_int_debug_frame(f);
	int result = 0;
	if (ktest_syscall_result_hook) {
		ktest_syscall_result_hook(result);
	}
	return result;
}
