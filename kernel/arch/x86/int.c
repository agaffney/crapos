#include <arch/x86/idt.h>
#include <arch/x86/int.h>
#include <arch/x86/io.h>
#include <core/kprint.h>

// ktest_int_hook only exists in KTEST=1 builds; weak, so it is null otherwise
#pragma weak ktest_int_hook

void arch_x86_int_handler_default(const arch_x86_int_frame * f) {
	if (ktest_int_hook) {
		ktest_int_hook(f->vector, f);
	}
	kdebug("received interrupt %d, with error %d\n", f->vector, f->error_code);
	kdebug("frame info: gs=0x%x, fs=0x%x, es=0x%x, ds=0x%x, edi=0x%x, esi=0x%x, ebp=0x%x, ebx=0x%x, edx=0x%x, ecx=0x%x, eax=0x%x, eip=0x%x, cs=0x%x, eflags=0x%x\n", f->gs, f->fs, f->es, f->ds, f->edi, f->esi, f->ebp, f->ebx, f->edx, f->ecx, f->eax, f->eip, f->cs, f->eflags);
	if(f->vector >= PIC2_OFFSET_ADDR) {
		kdebug("IRQ %d\n", f->vector - PIC2_OFFSET_ADDR + 8);
	} else if(f->vector >= PIC1_OFFSET_ADDR) {
		kdebug("IRQ %d\n", f->vector - PIC1_OFFSET_ADDR);
	}
}
