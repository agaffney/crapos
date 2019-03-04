#include <arch/x86/syscall.h>
#include <core/kprint.h>

int _int_handler_syscall(int syscall_no) {
	kdebug("syscall number = %d\n", syscall_no);
	return 0;
}
