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
	kdebug("frame info: gs=0x%x, fs=0x%x, es=0x%x, ds=0x%x, edi=0x%x, esi=0x%x, ebp=0x%x, ebx=0x%x, edx=0x%x, ecx=0x%x, eax=0x%x, eip=0x%x, cs=0x%x, eflags=0x%x\n", f->gs, f->fs, f->es, f->ds, f->edi, f->esi, f->ebp, f->ebx, f->edx, f->ecx, f->eax, f->eip, f->cs, f->eflags);
	int result = 0;
	if (ktest_syscall_result_hook) {
		ktest_syscall_result_hook(result);
	}
	return result;
}
