#include <core/process.h>
#include <core/vmm.h>

PROCESS * proc_table[MAX_PROCS];

t_pid proc_current;

void process_init() {
	PROCESS * kernel_proc = kmalloc(sizeof(PROCESS), KMALLOC_ZERO);
	kernel_proc->pid = KERNEL_PID;
	proc_table[KERNEL_PID] = kernel_proc;
	proc_current = KERNEL_PID;
}
